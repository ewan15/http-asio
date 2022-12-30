{ lib
, llvmPackages_14
, clang_14
, cmake
, cmakeCurses
, spdlog
, boost
, python3
, fmt
, openssl
}:

llvmPackages_14.stdenv.mkDerivation rec {
  pname = "http_server";
  version = "beta";

  src = ./.;

  nativeBuildInputs = [
  ];
  buildInputs = [
    clang_14
    cmake
    cmakeCurses
    boost
    spdlog
    python3
    fmt
    openssl
  ];

  cmakeFlags = [
    "-DENABLE_TESTING=OFF"
    "-DENABLE_INSTALL=ON"
  ];

  meta = with lib; {
    homepage = "https://github.com/ewan15/http-asio";
    description = ''
      A template for Nix based C++ project setup.";
    '';
    platforms = with platforms; linux ++ darwin;
  };
}
