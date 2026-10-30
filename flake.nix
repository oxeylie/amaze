{
  description = "Amaze game flake & dev environnement";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs =
    { nixpkgs, ... }:
    let
      hosts = [
        "aarch64-darwin"
        "aarch64-linux"
        "x86_64-linux"
      ];

      linux-targets = [
        "x86_64-linux-gnu.2.31"
        "aarch64-linux-gnu.2.31"
        "x86_64-linux-musl"
        "aarch64-linux-musl"
      ];

      darwin-targets = [
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      all-targets = (linux-targets ++ darwin-targets);

      forTargets = nixpkgs.lib.genAttrs all-targets;
      forHosts = nixpkgs.lib.genAttrs hosts;
    in
    {
      devShells = forHosts (system: import ./shell.nix { pkgs = import nixpkgs { inherit system; }; });
      legacyPackages = forHosts (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
          p = import ./package.nix { inherit pkgs; };
          host-pkgs = p.pkgsForTarget "host";
          cross-pkgs = forTargets (target: p.pkgsForTarget target);
          combined-pkgs = {
            x86_64-aarch64-darwin = p.combineTargets darwin-targets cross-pkgs;
          };
        in
        host-pkgs
        // rec {
          default = host-pkgs.amaze;
          cross = cross-pkgs // combined-pkgs;
          group = p.groupTargets (builtins.attrNames cross) cross;
        }
      );
    };
}
