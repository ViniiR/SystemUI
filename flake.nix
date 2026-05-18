{
    inputs.nixpkgs.url = "github:nixos/nixpkgs/afbbf774e2087c3d734266c22f96fca2e78d3620";
    outputs = {
        self,
        nixpkgs,
    }: let
        system = "x86_64-linux";
        pkgs = import nixpkgs {inherit system;};
        lib = pkgs.lib;
    in {
        devShells.${system}.default = pkgs.mkShell rec {
            NIX_ENFORCE_PURITY = 0;
            LD_LIBRARY_PATH = lib.makeLibraryPath packages;

            # Packages available in the User's shell
            packages = with pkgs; [
                pkg-config
                cmake
                gcc
                gtk4
                icon-library
                zsh
            ];

            shellHook = ''
                export SHELL=${pkgs.zsh}/bin/zsh
                exec zsh;
            '';
        };
    };
}
