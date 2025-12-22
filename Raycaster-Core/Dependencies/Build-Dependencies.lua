require "glm"
require "miniaudio"
require "stb_image"

project "Raycaster-Dependencies"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"
    warnings "off"

    AddGLM()
    AddMiniaudio()
    AddStbImage()
        
    targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "_CRT_SECURE_NO_WARNINGS" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        runtime "Release"
        optimize "On"
        symbols "Off"

include "GLFW.lua"
include "glad.lua"
include "FreeType.lua"
