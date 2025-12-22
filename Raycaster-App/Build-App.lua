project "Raycaster-App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    files { 
        "Source/**.h", "Source/**.cpp",
        "Source/Shaders/**.glsl",
        "Source/Textures/**",
        "Source/Libraries/GLFW/**",
        "Source/Libraries/glad/**"
    }

    includedirs
    {
        "Source",

	    "Source/Shaders",
	    "Source/Textures",

	    -- Include Core
	    "../Raycaster-Core/Source",
        "../Raycaster-Core/Dependencies/GLFW/include",
	    "../Raycaster-Core/Dependencies/glad/include",
	    "../Raycaster-Core/Dependencies/glm",
	    "../Raycaster-Core/Dependencies/FreeType/include",
	    "../Raycaster-Core/Dependencies/miniaudio",
	    "../Raycaster-Core/Dependencies/utils",
    }	

    links
    {
        "Raycaster-Core",
        "Raycaster-Dependencies",
    }

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }

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