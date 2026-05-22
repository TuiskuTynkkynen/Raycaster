project "Raycaster-App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"

    files { 
        "Source/**.h",
        "Source/**.cpp",
    }

    includedirs {
        "Source/",
        "Source/*/",
    }

    RaycasterCore.include()

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    postbuildcommands "{COPYDIR} Assets ../Binaries/%{OutputDir}/%{prj.name}/Assets"

    filter "system:windows"
        systemversion "latest"

    filter "system:emscripten"
        linkoptions {
            "--preload-file " .. path.getabsolute("Assets") .. "@/Assets/",
        }

    filter "configurations:Debug"
        defines { "DEBUG", "LOG_ENABLE" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE", "LOG_ENABLE", "LOG_PREFIX_LEVEL=1"  }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"
