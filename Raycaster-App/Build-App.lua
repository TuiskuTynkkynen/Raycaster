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
          "../Raycaster-Core/Source/Libraries/",
          "../Raycaster-Core/Source/Libraries/GLFW/include",
          "../Raycaster-Core/Source/Libraries/glad/include",
          "../Raycaster-Core/Source/Libraries/glad/src",
   }	

   libdirs
   {
      "../Raycaster-Core/Source/Libraries/GLFW/include",
      "../Raycaster-Core/Source/Libraries/GLFW/lib-vc2022",
   }

   links
   {
      "Raycaster-Core",
      "glfw3",
   }

   filter { "system:windows" }
      links { "OpenGL32" }

   filter { "system:not windows" }
      links { "GL" }
	
   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

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