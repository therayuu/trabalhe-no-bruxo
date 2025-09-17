{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = with pkgs; [
    gnumake

    pandoc
    texlive.combined.scheme-small
  ];
}

