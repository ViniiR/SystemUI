{
    inputs.nixpkgs.url = "github:nixos/nixpkgs/95ca1e203c0750115fd4a6f17d5a245dfe6b1edd";
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

            RUST_SRC_PATH = "${pkgs.rust.packages.stable.rustPlatform.rustLibSrc}";

            # Packages available in the User's shell
            packages = with pkgs; [
                pkg-config
                cmake
                gcc
                gtk4
                rustc
                systemd
                cargo
                rust-analyzer
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
