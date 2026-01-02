function AddStbImage()
	local stat = os.stat("utils/stb/stb_image.h")
	if stat == nil or stat.size == 0 then
		os.mkdir("utils/stb")
		os.execute("curl \"https://raw.githubusercontent.com/nothings/stb/f1c79c02822848a9bed4315b12c8c8f3761e1296/stb_image.h\" -o utils/stb/stb_image.h")
	end
	
	files
	{
		"utils/stb/stb_image.h",
	}
end
