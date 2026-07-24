{
    inputs.nixpkgs.url = "github:nixos/nixpkgs/95ca1e203c0750115fd4a6f17d5a245dfe6b1edd";
    outputs = {
        self,
        nixpkgs,
    }: let
        system = "x86_64-linux";
        pkgs = import nixpkgs {inherit system;};
        lib = pkgs.lib;
        daemon_name = "com.vinii.VGSController";
        binary_name = "vgscontroller";
    in {
        packages.${system}.default = pkgs.stdenv.mkDerivation rec {
            pname = binary_name;
            version = "0.1";

            src = ./.;

            nativeBuildInputs = with pkgs; [
                pkg-config
                cmake
                gcc

                # rustc
                # cargo
            ];
            buildInputs = with pkgs; [
                systemd
                dbus
            ];

            installPhase = ''
                runHook preInstall

                mkdir -p $out/bin
                mv system_ui $out/bin/${pname}

                runHook postInstall
            '';

            postInstall = ''
                # Launch D-Bus service
                mkdir -p $out/share/dbus-1/system-services
                cat <<END > $out/share/dbus-1/system-services/${daemon_name}.service
                [D-BUS Service]
                Name=${daemon_name}
                Exec=$out/bin/${pname}
                User=root
                SystemdService=${pname}.service
                END
                
                # D-Bus config file
                mkdir -p $out/share/dbus-1/system.d
                cp ${./dbus.conf.xml} $out/share/dbus-1/system.d/${daemon_name}.conf
            '';
        };
        nixosModules.default = {...}: let
            package = self.packages.${pkgs.stdenv.hostPlatform.system}.default;
        in {
            services.dbus.packages = [package];
            # environment.systemPackages = [package];

            systemd.services.${binary_name} = {
                description = "VGSController Daemon";
                wantedBy = ["multi-user.target"];
                after = ["dbus.service"];
                wants = ["dbus.service"];
                serviceConfig = {
                    Type = "dbus";
                    BusName = daemon_name;
                    ExecStart = "${package}/bin/${binary_name}";
                    Restart = "always";
                };
            };
        };
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
