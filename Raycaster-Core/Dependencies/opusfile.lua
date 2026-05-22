project "Opusfile"
    kind "StaticLib"
    language "C"
    staticruntime "off"
    warnings "off"

    files {
        "opusfile/src/*.c"
    }

    includedirs {
        "libogg/include",
        "libopus/include",
        "opusfile/include"
    }

    targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")
    
    filter "system:not emscripten"
        files {
            -- libogg
            "libogg/src/*.c",

            -- libopus
            "libopus/src/*.c",
            "libopus/celt/*.c",
            "libopus/silk/*.c",
            "libopus/silk/float/*.c",
         }
 
        includedirs {
            -- libopus
            "libopus/src",
            "libopus/celt",
            "libopus/silk",
            "libopus/silk/float/",
        }

        defines {
            "OPUS_BUILD",
            "USE_ALLOCA"
        }

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
