workspace "Raycaster"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Raycaster-App"
   fatalwarnings "All"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      flags { "MultiProcessorCompile" }
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

   -- Workspace-wide build options for web builds
   filter "system:emscripten"
        system "emscripten"
        architecture "wasm32"
        targetextension ".html"
        buildoptions {
            "-pthread",
            "-s SHARED_MEMORY=1",
            "-Wno-deprecated-declarations",
        }

    filter "system:emscripten"
        linkoptions {
            "-s WASM=1",
            "-s USE_GLFW=3",
            "-s FULL_ES3=1",
            "-pthread",
            "-s SHARED_MEMORY=1",
            "-s PTHREAD_POOL_SIZE=navigator.hardwareConcurrency",
            "-s ALLOW_MEMORY_GROWTH=1",
            "--preload-file " .. path.getabsolute("Raycaster-App") .. "/Assets@./Assets/",
        }

    filter { "system:emscripten", "configurations:not Dist" }
        linkoptions {
            "-g3",
            "-fdebug-compilation-dir=.",
            "-fdebug-prefix-map=" .. path.getabsolute(".") .. "=.",
            "-s ASSERTIONS=2",
            "-s SAFE_HEAP=1",
            "-s STACK_OVERFLOW_CHECK=2",
            "-s GL_DEBUG=1",
        }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Core"
	include "Raycaster-Core/Build-Core.lua"
group ""

include "Raycaster-App/Build-App.lua"
