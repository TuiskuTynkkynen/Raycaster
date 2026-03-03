function AddMiniaudio()
	files {
		"miniaudio/miniaudio/miniaudio.h",
		"miniaudio/miniaudio/extras/decoders/libopus/*"
	}

	includedirs{
        "libogg/include",
        "libopus/include",
        "opusfile/include"
	}

	links { "opus" }

	filter { "system:linux" }
		links { "lpthread", "lm", "ldl" }
end
