{ pkgs, ... }:
let
  inherit (pkgs) lib;
  major_v = "0";
  minor_v = "1";
  appendix = "dev";
  version = if appendix != null then "${major_v}.${minor_v}-${appendix}" else "${major_v}.${minor_v}";

  hostArch = target: builtins.head (lib.splitString "." target);
  safeName = builtins.replaceStrings [ "." ] [ "_" ];

  mkMeta = name: version: {
    inherit version;
    meta = {
      description = "Cross-platform multiplayer TUI game";
      license = pkgs.lib.licenses.agpl3Plus;
      mainProgram = name;
    };
  };

  mkNcurses =
    {
      host-target,
      static ? true,
      setup,
      cc,
      ar,
      ranlib,
    }:
    pkgs.stdenv.mkDerivation {
      inherit (pkgs.ncurses) version src;
      pname = "ncurses-${lib.optionalString static "static-"}${safeName host-target}";
      dontFixup = true; # no strip/patchelf on foreign-arch files

      configurePhase = ''
        runHook preConfigure
        ${setup}
        ./configure \
          --prefix=$out \
          --build=$(sh ./config.guess) \
          --host=${hostArch host-target} \
          --with-build-cc=${pkgs.stdenv.cc}/bin/cc \
          CC=${cc} AR=${ar} RANLIB=${ranlib} \
          ${
            if static then
              "--without-shared --with-normal"
            else
              "--with-shared --without-normal --with-abi-version=6"
          } --without-debug \
          --enable-widec --enable-overwrite --disable-stripping \
          --without-cxx --without-cxx-binding --without-ada \
          --without-manpages --without-tests --without-progs \
          --with-terminfo-dirs=/etc/terminfo:/lib/terminfo:/usr/share/terminfo \
          --with-default-terminfo-dir=/usr/share/terminfo
        find . -name Makefile -exec sed -i 's/,-stats,-lc//' {} +
        runHook postConfigure
      '';

      buildPhase = ''
        runHook preBuild
        make -j$NIX_BUILD_CORES
        runHook postBuild
      '';

      installPhase =
        let
          ext = if static then "a" else "so";
        in
        ''
          runHook preInstall
          make install.libs install.includes
          ln -s libncursesw.${ext} $out/lib/libncurses.${ext}
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

      version_flags = ''-DMAJOR_V=\"${major_v}\" -DMINOR_V=\"${minor_v}\" ${
        if appendix != null then ''-DAPPENDIX=\"${appendix}\"'' else "-UAPPENDIX"
      }'';

      # Only needed by darwin
      arch =
        {
          aarch64-darwin = "arm64";
          x86_64-darwin = "x86_64";
        }
        .${host-target};

      # Only needed by linux
      zig-cc = pkgs.writeShellScript "zigcc-${safeName host-target}" ''exec ${pkgs.zig}/bin/zig cc -target ${host-target} "$@"'';
      zig-ar = pkgs.writeShellScript "zigar" ''exec ${pkgs.zig}/bin/zig ar "$@"'';
      zig-ranlib = pkgs.writeShellScript "zigranlib" ''exec ${pkgs.zig}/bin/zig ranlib "$@"'';
      zig-setup = ''
        export ZIG_GLOBAL_CACHE_DIR=$TMPDIR/zig-global
        export ZIG_LOCAL_CACHE_DIR=$TMPDIR/zig-local
      '';

      # Compile a static version of ncurses
      ncurses = mkNcurses {
        static = targetIsStatic;
        cc = zig-cc;
        ar = zig-ar;
        ranlib = zig-ranlib;
        setup = zig-setup;
        inherit host-target;
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
            ${zig-setup}
          ''}
          ${lib.optionalString (targetIsDarwin && pkgs.stdenv.hostPlatform.isDarwin) ''
            unset DEVELOPER_DIR SDKROOT NIX_LDFLAGS
            XSDK=$(/usr/bin/xcrun --sdk macosx --show-sdk-path)
            XLD=$(/usr/bin/xcrun --find ld)
          ''}
        '';

        buildPhase = ''
          runHook preBuild
          make -j$NIX_BUILD_CORES ${
            linuxTargetStr "CC=${zig-cc}" + darwinTargetStr "CC=/usr/bin/clang"
          } EXTRA_FLAGS='${
            darwinTargetStr ''-arch ${arch} -isysroot "'$XSDK'" --ld-path="'$XLD'" -mmacosx-version-min=${
              if host-target == "x86_64-darwin" then "10.13" else "11.0"
            }''
            + linuxTargetStr "-I${ncurses}/include -L${ncurses}/lib -O2 -g0 -fno-sanitize=undefined -s -Wl,--strip-all ${lib.optionalString targetIsStatic "-static"}"
          } ${version_flags}' ${make-target}
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/bin
          cp ./out/${name} $out/bin/${name}
          runHook postInstall
        '';

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

  groupPkgs =
    { name, target-pkgs }:
    pkgs.stdenv.mkDerivation (
      {
        pname = "${name}-group";
        dontUnpack = true;
        dontBuild = true;
        dontFixup = true;

        installPhase = ''
          mkdir -p $out/bin
          ${lib.concatMapStringsSep "\n" (
            p: "cp ${p.path}/bin/${name} $out/bin/${name}-${safeName p.target}"
          ) target-pkgs}
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

  groupTargets =
    target-hosts: cross-pkgs:
    let
      tagged-pkgs = map (
        t:
        lib.mapAttrs (_: path: {
          target = t;
          inherit path;
        }) cross-pkgs.${t}
      ) target-hosts;
    in
    lib.mapAttrs (name: target-pkgs: groupPkgs { inherit name target-pkgs; }) (
      lib.zipAttrs tagged-pkgs
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
