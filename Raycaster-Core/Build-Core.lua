project "Raycaster-Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { 
      "Source/**.h", "Source/**.cpp",
      "Source/Libraries/**",
      "Source/Libraries/GLFW/**",
      "Source/Libraries/glad/**",
   }

   includedirs
   {
      "Source",
      "Source/Libraries/",
      "Source/Libraries/freetype/",
      "Source/Libraries/dlg/",
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