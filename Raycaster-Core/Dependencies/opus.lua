project "Opus"
    kind "StaticLib"
    language "C"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"
    warnings "off"
    
    
    files {
		-- libogg
		"libogg/src/*.c",

		-- libopus
		"libopus/src/*.c",
        "libopus/celt/*.c",
        "libopus/silk/*.c",
        "libopus/silk/float/*.c",

        -- opusfile
        "opusfile/src/*.c"
	}

	includedirs {
		-- libogg
		"libogg/include",

		-- libopus
		"libopus/include",
		"libopus/src",
		"libopus/celt",
		"libopus/silk",
        "libopus/silk/float/",

        -- opusfile
		"opusfile/include"
	}

	defines {
        "OPUS_BUILD",
        "USE_ALLOCA"
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