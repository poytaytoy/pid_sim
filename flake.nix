{
  description = "A cross-platform development environment for OpenGL";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # Build Tools
            cmake
            pkg-config
            gnumake

            # OpenGL Core Libraries
            libGL
            glfw
            glew
            glm
            python314Packages.glad
            imgui

            # X11 Dependencies (Often required for GLFW/Windowing)
            xorg.libX11
            xorg.libXcursor
            xorg.libXrandr
            xorg.libXinerama
            xorg.libXi
            xorg.libXext
          ];

          shellHook = ''
            export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${
              pkgs.lib.makeLibraryPath (with pkgs; [
                libGL
                glfw
                xorg.libX11
              ])
            }"
            echo "--- OpenGL Development Environment Loaded ---"
            echo "GLFW, GLEW, GLM, and GLAD are available."
          '';
        };
      }
    );
}