{
  description = "A weatherstation prototype.";
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        nproc = builtins.readFile (pkgs.runCommand "nproc" { } "nproc | tr -d '\n' > $out");
      in
      {
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [
            gnumake
            gcc-arm-embedded-9
            bossa
            git
            bear
            git
            (python37.withPackages (p: with p; [
              pyserial
              breathe
              sphinx_rtd_theme
            ]))
            doxygen
          ];
          MAKEFLAGS = "-j${nproc}";
          DEVELHELP = 1;
        };
      }
    );
}
