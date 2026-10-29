{
  pkgs,
  build ? "host",
  cross-pkgs ? { },
  ...
}:
let
  is_dynamic = build == "dynamic";
  is_mixed = build == "mixed";
  is_darwin = pkgs.stdenv.hostPlatform.isDarwin;
  is_linux = pkgs.stdenv.hostPlatform.isLinux;
  is_aarch64 = pkgs.stdenv.hostPlatform.isAarch64;

  version = "0.1-dev";

  buildTarget =
    { name, target }:
    pkgs.stdenv.mkDerivation {
      inherit version;
      pname = name;
      src = ./.;

      __noChroot = is_darwin;
      nativeBuildInput = pkgs.lib.optionals (is_dynamic && is_linux) [ pkgs.patchelf ];
      buildInputs = if is_darwin then [ (pkgs.darwinMinVersionHook "11.0") ] else [ pkgs.ncurses ];

      preBuild = pkgs.lib.optionalString is_darwin ''
        XSDK=$(env -u DEVELOPER_DIR -u SDKROOT /usr/bin/xcrun --sdk macosx --show-sdk-path)
        mkdir -p stubs
        ln -s "$XSDK"/usr/lib/libncurses*.tbd stubs/
        export NIX_LDFLAGS="$NIX_LDFLAGS -L$PWD/stubs"
      '';
      buildPhase = ''
        runHook preBuild
        make ${pkgs.lib.optionalString is_darwin "\"EXTRA_FLAGS=-mmacosx-version-min=10.13\""} ${target}
        runHook postBuild
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp ./out/${target} $out/bin/${name}
      '';
      postFixup = pkgs.lib.optionalString (is_linux && is_dynamic) ''
        patchelf --set-interpreter ${
          if is_aarch64 then "/lib/ld-linux-aarch64.so.1" else "/lib64/ld-linux-x86-64.so.2"
        } $out/bin/${name}
        patchelf --remove-rpath $out/bin/${name}
      '';

      meta = {
        description = "Cross-platform multiplayer TUI game";
        license = pkgs.lib.licenses.agpl3Plus;
        mainProgram = name;
      };
    };
  combineTargets =
    { name, targets }:
    pkgs.stdenv.mkDerivation {
      inherit version;
      pname = name;
      dontUnpack = true;
      nativeBuildInputs = [
        pkgs.cctools
        pkgs.darwin.autoSignDarwinBinariesHook
      ];
      buildPhase = ''
        lipo -create ${pkgs.lib.concatStringsSep " " targets} -output ${name}
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp ${name} $out/bin/${name}
      '';
      meta = {
        description = "Cross-platform multiplayer TUI game";
        license = pkgs.lib.licenses.agpl3Plus;
        mainProgram = name;
      };
    };

  client-name = "amaze";
  server-name = "amaze-server";

  client =
    if (is_mixed && is_darwin) then
      combineTargets {
        name = client-name;
        targets = [
          "${cross-pkgs.cross-dynamic.aarch64-darwin.${client-name}}/bin/${client-name}"
          "${cross-pkgs.cross-dynamic.x86_64-darwin.${client-name}}/bin/${client-name}"
        ];
      }
    else
      buildTarget {
        name = client-name;
        target = "client";
      };
  server =
    if (is_mixed && is_darwin) then
      combineTargets {
        name = server-name;
        targets = [
          "${cross-pkgs.cross-dynamic.aarch64-darwin.${server-name}}/bin/${server-name}"
          "${cross-pkgs.cross-dynamic.x86_64-darwin.${server-name}}/bin/${server-name}"
        ];
      }
    else
      buildTarget {
        name = server-name;
        target = "server";
      };
in
{
  ${client-name} = client;
  ${server-name} = server;
  default = client;
}
