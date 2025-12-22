function AddMiniaudio()
	files
	{
		"miniaudio/miniaudio/miniaudio.h",
	}

	filter { "system:linux" }
		links { "lpthread", "lm", "ldl" }
end
