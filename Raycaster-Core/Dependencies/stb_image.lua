function AddStbImage()
	os.mkdir("utils/stb")
	os.execute("curl \"https://raw.githubusercontent.com/nothings/stb/f1c79c02822848a9bed4315b12c8c8f3761e1296/stb_image.h\" -o utils/stb/stb_image.h")
	
	files
	{
		"utils/stb/stb_image.h",
	}
end
