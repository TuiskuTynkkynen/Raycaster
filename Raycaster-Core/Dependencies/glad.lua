project "glad"
   kind "StaticLib"
   language "C++"
   cppdialect "C++23"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"
   warnings "off"

   files 
   {
        "glad/include/**.h",
        "glad/src/gl.c",
   }

   includedirs
   {
        "glad/include"
   }
       
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
