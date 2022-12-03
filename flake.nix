{
  description = "A template for Nix based C++ project setup.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/22.11";

    utils.url = "github:numtide/flake-utils";
    utils.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    "x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin"
  ] (system: let pkgs = import nixpkgs {
                   inherit system;
                 };
             in {
               devShell = pkgs.mkShell rec {
                 name = "http_asio";

                 packages = with pkgs; [
                   # Development Tools
                   clang-tools
                   clang_14
                   cmake
                   cmakeCurses
                   boost179
                   spdlog
                   python3
                   fmt
                 ];
                 shellHook = ''export _JAVA_AWT_WM_NONREPARENTING=1; export PS1="\e[0;31m[\u@\h \W]\$ \e[m "'';
               };
             });
}
