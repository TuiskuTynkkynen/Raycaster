require "glad"
require "GLFW"
require "glm"
require "FreeType"
require "miniaudio"
require "stb_image"

project "Raycaster-Dependencies"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	targetdir "Binaries/%{cfg.buildcfg}"
	staticruntime "off"
	warnings "off"
	
	AddGLFW()
	AddGlad()
	AddGLM()
	AddFreeType()
	AddMiniaudio()
	AddStbImage()
   	
	targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
	objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

	filter "system:windows"
		systemversion "latest"
		defines { "_CRT_SECURE_NO_WARNINGS" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "RELEASE" }
		runtime "Release"
		optimize "On"
		symbols "On"

	filter "configurations:Dist"
		defines { "DIST" }
		runtime "Release"
		optimize "On"
		symbols "Off"