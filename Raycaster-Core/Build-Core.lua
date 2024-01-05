project "Raycaster-Core"
   kind "StaticLib"
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
      "Source/Libraries/",
      "Source/Libraries/GLFW/include",
      "Source/Libraries/glad/include",
      "Source/Libraries/glad/src",
   }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

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