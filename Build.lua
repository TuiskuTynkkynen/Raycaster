workspace "Raycaster"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Raycaster-App"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Core"
	include "Raycaster-Core/Build-Core.lua"
group ""

include "Raycaster-App/Build-App.lua"
