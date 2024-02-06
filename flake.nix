{
  description = "C++ Development Shell";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:

      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      with pkgs;
      {
        devShells.default = mkShell.override { stdenv = clang14Stdenv; }
          {
            buildInputs = [
              pkgs.clang
              pkgs.clang-tools
              pkgs.pkg-config
              pkgs.cmake
              pkgs.ninja
              pkgs.git
              (pkgs.gnutls.override { cxxBindings = true; })
              pkgs.libtasn1
              pkgs.libidn2
              pkgs.p11-kit
              pkgs.openssl
              pkgs.yaml-cpp
              pkgs.pkgs.zlib
              pkgs.zstd
              pkgs.protobuf3_20
              pkgs.icu70
              pkgs.lksctp-tools
              pkgs.hwloc
              pkgs.libsystemtap
              pkgs.libxfs
              pkgs.ragel
              pkgs.lksctp-tools
              pkgs.c-ares
              pkgs.lz4
              pkgs.numactl
              (pkgs.boost183.override { enableShared = false; })
              pkgs.fmt
            ];
          };
      }
    );
}
