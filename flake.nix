{
  description = "A template for Nix based C++ project setup.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/22.11";

    utils.url = "github:numtide/flake-utils";
    utils.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs =
    { self
    , nixpkgs
    , ...
    }
      @inputs: inputs.utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };

      http_server = pkgs.callPackage ./default.nix { };

      dockerImage = pkgs.dockerTools.buildImage {
        name = "http_server";
        config = { Cmd = [ "${http_server}/bin/http_server" ]; };
      };
    in
    {
      packages = {
        http_server = http_server;
        docker = dockerImage;
      };

      devShell = pkgs.mkShell rec {
        name = "http_asio";

        packages = with pkgs; [
          # Development Tools
          clang-tools
          clang_14
          cmake
          cmakeCurses
          boost
          spdlog
          python3
          fmt
        ];
        shellHook = ''export _JAVA_AWT_WM_NONREPARENTING=1; export PS1="\e[0;31m[\u@\h \W]\$ \e[m "'';
      };

      defaultPackage = http_server;
      docker = dockerImage;
    });
}
