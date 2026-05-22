include "Dependencies/Build-Dependencies.lua"

RaycasterCore = {}

local base_path = path.getabsolute("./")
function RaycasterCore.include()
    includedirs {
        base_path .. "/Source",
    }
    
    links "Raycaster-Core"

    RaycasterCoreDependencies.include()

    filter "system:emscripten"
        linkoptions {
            "--pre-js " .. base_path .. "/Source/Platform/Web/pre.js",
        }

    filter {}
end

project "Raycaster-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"
    
    files {
        "Source/**.h",
        "Source/**.cpp",
    }

    includedirs {
        "Source",
    }

    RaycasterCoreDependencies.include()
    
    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"

    filter "system:emscripten"
        linkoptions {
            "--pre-js " .. path.getabsolute("Source/Platform/Web/pre.js"),
        }

    filter "configurations:Debug"
        defines { "DEBUG", "LOG_ENABLE"}
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE", "LOG_ENABLE", "LOG_PREFIX_LEVEL=1"}
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"
