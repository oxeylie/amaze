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
        # Static linking is not supported yet
        # "x86_64-linux-musl"
        # "aarch64-linux-musl"
      ];

      darwin-targets = [
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forTargets = nixpkgs.lib.genAttrs (linux-targets ++ darwin-targets);
      forHosts = nixpkgs.lib.genAttrs hosts;
    in
    {
      devShells = forHosts (system: import ./shell.nix { pkgs = import nixpkgs { inherit system; }; });
      packages = forHosts (
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
        // {
          default = host-pkgs.amaze;
          cross = cross-pkgs // combined-pkgs;
        }
      );
    };
}
