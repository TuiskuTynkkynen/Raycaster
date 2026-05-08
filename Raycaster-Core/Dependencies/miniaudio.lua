function AddMiniaudio()
	files {
		"miniaudio/miniaudio/miniaudio.c",
		"miniaudio/miniaudio/extras/decoders/libopus/*"
	}

	includedirs{
        "libogg/include",
        "libopus/include",
        "opusfile/include"
	}

	links { "Opusfile" }

	filter { "system:linux" }
		links { "lpthread", "lm", "ldl" }
end
