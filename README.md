# Raycaster

A small framework for creating interactive applications and raycaster demo application written in in C++.

## Building

This project uses the [Premake](https://premake.github.io/) build system which is included as prebuilt binaries.
Other dependencies will get downloaded as git submodules or during the build process. Start by cloning the repository 
with `git clone --recursive https://github.com/TuiskuTynkkynen/Raycaster.git`
and follow the instructions for your platform.

### Windows  

- Prerequisites  
  - MSVC C++ toolset  
  - Python (version >= 3.3)  
- Run [Scripts/Setup-Windows.bat](Scripts/Setup-Windows.bat)  
- Open the generated Visual Studio Solution 

### Linux

- Prerequisites  
  - GNU Make  
  - Clang  
  - Python (version >= 3.3)  
  - X11 and xkbcommon development packages [for GLFW](https://www.glfw.org/docs/latest/compile_guide.html#compile_deps)  
  - OpenGL development package  
- Run [Scripts/Setup-Linux.sh](Scripts/Setup-Linux.sh)  
- Run `make` in the project root directory  