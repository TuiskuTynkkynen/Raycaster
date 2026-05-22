project "glm"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"
    warnings "off"

    files {
        "glm/glm/**.h",
        "glm/glm/**.hpp",
        "glm/glm/**.cpp",
        "glm/glm/**.inl",
    }

    includedirs {
        "glm",
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
