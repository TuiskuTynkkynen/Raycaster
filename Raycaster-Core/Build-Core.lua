project "Raycaster-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    files { 
        "Source/**.h", "Source/**.cpp",
    }

    includedirs
    {
        "Source",
        "Dependencies/GLFW/include",
        "Dependencies/glad/include",
        "Dependencies/glm",
        "Dependencies/FreeType/include",
        "Dependencies/miniaudio",
        "Dependencies/utils",
    }

    links {
        "Raycaster-Dependencies",
        "GLFW",
        "glad",
        "FreeType",
    }

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { }

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


group "Core/Dependencies"
    include "Dependencies/Build-Dependencies.lua"
group ""
