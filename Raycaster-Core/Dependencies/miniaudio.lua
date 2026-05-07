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

	links { "Opus" }

	filter { "system:linux" }
		links { "lpthread", "lm", "ldl" }
end
