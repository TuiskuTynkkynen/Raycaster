project "miniaudio"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"
    warnings "off"
    
    files {
		"miniaudio/miniaudio/miniaudio.c",
		"miniaudio/miniaudio/extras/decoders/libopus/*"
	}
    
	includedirs {
        "libogg/include",
        "libopus/include",
        "opusfile/include"
	}

    links { "Opusfile" }

    targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

	filter { "system:linux" }
		links { "lpthread", "lm", "ldl" }

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
