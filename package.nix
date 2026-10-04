{ pkgs, ... }:
let
  buildTarget =
    { name, target }:
    pkgs.stdenv.mkDerivation {
      pname = name;
      version = "0.1-dev";

      src = ./.;

      buildInputs = with pkgs; [
        ncurses
      ];
      # nativeBuildInputs = with pkgs; [ gnumake ] ++ (if pkgs.stdenv.isLinux then [ gcc ] else [ clang ]);

      buildPhase = ''
        make ${target}
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp ./out/${target} $out/bin/${name}
      '';

      meta = {
        description = "Cross-platform multiplayer TUI game";
        license = pkgs.lib.licenses.gpl3;
        mainProgram = name;
      };
    };
in
rec {
  amaze = buildTarget {
    name = "amaze";
    target = "client";
  };
  amaze-server = buildTarget {
    name = "amaze-server";
    target = "server";
  };
  default = amaze;
}
