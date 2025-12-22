function AddGlad()
	files 
	{
		"glad/include/**.h",
		"glad/src/gl.c",
	}

	includedirs
	{
		"glad/include"
	}

   filter { "system:linux" }
      links { "GL" }

   filter { "system:windows" }
      links { "OpenGL32" }
end
