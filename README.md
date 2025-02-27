# Raycaster

## Dependencies
External dependencies should be added to Raycaster-Core/Source/Libraries  
- [GLFW](https://www.glfw.org/) OpenGL library for window creation  
  - Version 3.3.9  
  - In Raycaster-Core/Source/Libraries/GLWF  
- [Glad](https://glad.dav1d.de/) OpenGL loading library  
  - For gl version 3.3  
  - No extensions
  - In Raycaster-Core/Source/Libraries/glad  
- [GLM](https://github.com/g-truc/glm/releases/tag/1.0.1) OpenGL mathematics library  
  - Version 1.0.1  
  - In Raycaster-Core/Source/Libraries/glm  
- [stb](https://github.com/nothings/stb/blob/master/stb_image.h) for image parsing  
  -  Just the stb_image header file  
  -  In Raycaster-Core/Source/Libraries/utils  
- [FreeType](https://freetype.org/) for font parsing  
  -  Version 2.13.2  
  -  Cannot be in seperate directory  
  -  In Raycaster-Core/Source/Libraries  
- [miniaudio](https://github.com/mackron/miniaudio) for handling audio  
  -  Version 0.11.21  
  -  Just the header file  
  -  In Raycaster-Core/Source/Libraries/miniaudio  