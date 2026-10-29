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

      targets = [
        "x86_64-linux-gnu.2.31"
        "aarch64-linux-gnu.2.31"
        # Static linking is not supported yet
        # "x86_64-linux-musl"
        # "aarch64-linux-musl"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forTargets = nixpkgs.lib.genAttrs [
        "gnu64"
        "aarch64-multiplatform"
        "aarch64-darwin"
        "x86_64-darwin"
      ];
      forHosts = nixpkgs.lib.genAttrs hosts;
    in
    {
      devShells = forHosts (system: import ./shell.nix { pkgs = import nixpkgs { inherit system; }; });
      packages = forHosts (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        import ./package.nix { inherit pkgs; }
        // rec {
          cross-dynamic = forTargets (
            cross-system:
            import ./package.nix {
              pkgs = pkgs.pkgsCross.${cross-system};
              build = "dynamic";
            }
          );
          cross-mixed = {
            x86_64-aarch64-darwin = import ./package.nix {
              inherit pkgs;
              build = "mixed";
              cross-pkgs = { inherit cross-dynamic; };
            };
          };
        }
      );
    };
}
