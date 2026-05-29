# Raycaster

A small framework for creating interactive applications and raycaster demo application written in C++.

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
  - GNU Autotools and GNU Make  
  - Clang  
  - Python (version >= 3.3)  
  - X11 and xkbcommon development packages for [GLFW](https://www.glfw.org/docs/latest/compile_guide.html#compile_deps)  
  - OpenGL development package  
- Run [Scripts/Setup-Linux.sh](Scripts/Setup-Linux.sh)  
- Run `make` in the project root directory  

### Web

- Web builds are supported only on Linux  
- Prerequisites  
  - GNU Autotools and GNU Make  
  - Python (version >= 3.3)  
  - [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)  
- Source the Emscripten SDK
- Run [Scripts/Setup-Web.sh](Scripts/Setup-Web.sh)  
- Run `emmake make` in the project root directory  
