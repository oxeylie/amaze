{ pkgs, ... }:
let
  inherit (pkgs) lib;

  version = "0.1-dev";

  mkMeta = name: version: {
    inherit version;
    meta = {
      description = "Cross-platform multiplayer TUI game";
      license = pkgs.lib.licenses.agpl3Plus;
      mainProgram = name;
    };
  };

  safeName = builtins.replaceStrings [ "." ] [ "_" ];
  mkNcurseStatic =
    {
      host-target,
      setup,
      cc,
      ar,
      ranlib,
    }:
    let
      host-arch = builtins.head (lib.splitString "." host-target); # drop ".2.31"
    in
    pkgs.stdenv.mkDerivation {
      inherit (pkgs.ncurses) version src;
      pname = "ncurses-static-${safeName host-target}";
      dontFixup = true; # no strip/patchelf on foreign-arch files

      configurePhase = ''
        runHook preConfigure
        ${setup}
        ./configure \
          --prefix=$out \
          --build=$(sh ./config.guess) \
          --host=${host-arch} \
          --with-build-cc=${pkgs.stdenv.cc}/bin/cc \
          CC=${cc} AR=${ar} RANLIB=${ranlib} \
          --without-shared --with-normal --without-debug \
          --enable-widec --enable-overwrite --disable-stripping \
          --without-cxx --without-cxx-binding --without-ada \
          --without-manpages --without-tests --without-progs \
          --with-terminfo-dirs=/etc/terminfo:/lib/terminfo:/usr/share/terminfo \
          --with-default-terminfo-dir=/usr/share/terminfo
        runHook postConfigure
      '';

      buildPhase = ''
        runHook preBuild
        make -j$NIX_BUILD_CORES
        runHook postBuild
      '';

      installPhase = ''
        runHook preInstall
        make install.libs install.includes
        # your Makefile says -lncurses; only .a files live here, so the
        # linker can't pick a .so by accident
        ln -s libncursesw.a $out/lib/libncurses.a
        runHook postInstall
      '';
    };

  mkPkgForTarget =
    {
      name,
      make-target,
      host-target,
    }:
    let
      targetIsLinux = lib.hasInfix "linux" host-target;
      targetIsDarwin = lib.hasInfix "darwin" host-target;
      targetIsStatic = lib.hasInfix "musl" host-target;
      targetIsHost = host-target == "host";
      hostTargetList = lib.optionals targetIsHost;
      linuxTargetList = lib.optionals targetIsLinux;
      darwinTargetStr = lib.optionalString targetIsDarwin;
      linuxTargetStr = lib.optionalString targetIsLinux;

      # Only needed by darwin
      arch =
        {
          aarch64-darwin = "arm64";
          x86_64-darwin = "x86_64";
        }
        .${host-target};

      # Only needed by linux
      cc = pkgs.writeShellScript "zigcc-${safeName host-target}" ''exec ${pkgs.zig}/bin/zig cc -target ${host-target} "$@"'';
      ar = pkgs.writeShellScript "zigar" ''exec ${pkgs.zig}/bin/zig ar "$@"'';
      ranlib = pkgs.writeShellScript "zigranlib" ''exec ${pkgs.zig}/bin/zig ranlib "$@"'';
      setup = ''
        export ZIG_GLOBAL_CACHE_DIR=$TMPDIR/zig-global
        export ZIG_LOCAL_CACHE_DIR=$TMPDIR/zig-local
      '';

      # Compile a static version of ncurses
      ncurses = mkNcurseStatic {
        inherit
          host-target
          cc
          ar
          ranlib
          setup
          ;
      };
    in
    pkgs.stdenv.mkDerivation (
      {
        pname = "${name}-${safeName host-target}";
        src = ./.;
        __noChroot = pkgs.stdenv.hostPlatform.isDarwin; # Needed since we need to call for Xcode
        dontFixup = targetIsLinux;

        nativeBuildInputs = [ ] ++ linuxTargetList [ pkgs.patchelf ];
        buildInputs = [ ] ++ hostTargetList [ pkgs.ncurses ];

        preBuild = ''
          ${linuxTargetStr ''
            ${setup}
          ''}
          ${lib.optionalString ((targetIsDarwin || targetIsHost) && pkgs.stdenv.hostPlatform.isDarwin) ''
            XSDK=$(env -u DEVELOPER_DIR -u SDKROOT /usr/bin/xcrun --sdk macosx --show-sdk-path)
            mkdir -p stubs
            ln -s "$XSDK"/usr/lib/libncurses*.tbd stubs/
            export NIX_LDFLAGS="$NIX_LDFLAGS -L$PWD/stubs"
          ''}
        '';

        buildPhase = ''
          runHook preBuild
          make -j$NIX_BUILD_CORES ${linuxTargetStr "CC=${cc}"} EXTRA_FLAGS="${
            darwinTargetStr "-arch ${arch} -mmacosx-version-min=11.0"
            + linuxTargetStr "-I${ncurses}/include -L${ncurses}/lib -O2 -g0 -fno-sanitize=undefined -s -Wl,--strip-all ${lib.optionalString targetIsStatic "-static"}"
          }" ${make-target}
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/bin
          cp ./out/${make-target} $out/bin/${name}
          runHook postInstall
        '';

        doInstallCheck = !targetIsHost;
        disallowedReferences = [
          pkgs.zig
          ncurses
        ];
      }
      // mkMeta name version
    );
  combinePkgs =
    {
      name,
      target-hosts,
      target-pkgs,
    }:
    pkgs.stdenv.mkDerivation (
      {
        pname = "${name}-${lib.concatMapStringsSep "-" (target: safeName target) target-hosts}";

        dontUnpack = true; # No src
        nativeBuildInputs = [
          pkgs.cctools
          pkgs.darwin.autoSignDarwinBinariesHook
        ];

        buildPhase = ''
          lipo -create ${lib.concatMapStringsSep " " (p: "${p}/bin/${name}") target-pkgs} -output ${name}
        '';

        installPhase = ''
          mkdir -p $out/bin
          cp ${name} $out/bin/${name}
        '';
      }
      // mkMeta name version
    );
in
{
  combineTargets =
    target-hosts: cross-pkgs:
    lib.mapAttrs (name: target-pkgs: combinePkgs { inherit name target-pkgs target-hosts; }) (
      lib.zipAttrs (map (t: cross-pkgs.${t}) target-hosts)
    );

  pkgsForTarget = target: {
    "amaze" = mkPkgForTarget {
      name = "amaze";
      make-target = "client";
      host-target = target;
    };
    "amaze-server" = mkPkgForTarget {
      name = "amaze-server";
      make-target = "server";
      host-target = target;
    };
  };
}
