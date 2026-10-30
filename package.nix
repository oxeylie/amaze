{ pkgs, ... }:
let
  is_darwin = pkgs.stdenv.hostPlatform.isDarwin;
  is_linux = pkgs.stdenv.hostPlatform.isLinux;
  is_aarch64 = pkgs.stdenv.hostPlatform.isAarch64;

  version = "0.1-dev";

  mkMeta = pname: version: {
    inherit version pname;
    meta = {
      description = "Cross-platform multiplayer TUI game";
      license = pkgs.lib.licenses.agpl3Plus;
      mainProgram = pname;
    };
  };

  mkPkgForTarget =
    {
      name,
      make-target,
      host-target,
    }:
    let
      linux_list = pkgs.lib.optionals is_linux;
      darwin_str = pkgs.lib.optionalString is_darwin;
      linux_str = pkgs.lib.optionalString is_linux;
      interpreter = if is_aarch64 then "/lib/ld-linux-aarch64.so.1" else "/lib64/ld-linux-x86-64.so.2";
      arch =
        {
          aarch64-darwin = "arm64";
          x86_64-darwin = "x86_64";
        }
        .${host-target};
    in
    pkgs.stdenv.mkDerivation (
      {
        src = ./.;
        __noChroot = is_darwin;
        nativeBuildInputs = [ ] ++ linux_list [ pkgs.patchelf ];
        buildInputs = [ ] ++ linux_list [ pkgs.ncurses ];
        buildPhase = ''
          ${darwin_str ''
            XSDK=$(env -u DEVELOPER_DIR -u SDKROOT /usr/bin/xcrun --sdk macosx --show-sdk-path)
            mkdir -p stubs
            ln -s "$XSDK"/usr/lib/libncurses*.tbd stubs/
            export NIX_LDFLAGS="$NIX_LDFLAGS -L$PWD/stubs"
          ''}
          make ${darwin_str ''"EXTRA_FLAGS=-arch ${arch} -mmacosx-version-min=10.13"''} ${make-target}
        '';
        installPhase = ''
          mkdir -p $out/bin
          cp ./out/${make-target} $out/bin/${name}
        '';
        postFixup = linux_str ''
          patchelf --set-interpreter ${interpreter} $out/bin/${name}
          patchelf --remove-rpath $out/bin/${name}
        '';
      }
      // mkMeta name version
    );
  combinePkgs =
    {
      name,
      target-pkgs,
    }:
    pkgs.stdenv.mkDerivation (
      {
        dontUnpack = true; # No src
        nativeBuildInputs = [
          pkgs.cctools
          pkgs.darwin.autoSignDarwinBinariesHook
        ];
        buildPhase = ''
          lipo -create ${pkgs.lib.concatMapStringsSep " " (p: "${p}/bin/${name}") target-pkgs} -output ${name}
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
    targets: cross-pkgs:
    pkgs.lib.mapAttrs (name: target-pkgs: combinePkgs { inherit name target-pkgs; }) (
      pkgs.lib.zipAttrs (map (t: cross-pkgs.${t}) targets)
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
