# Graphics-Engine-For-Alpha-Scenic
This will be the design of the Graphics engine in Alpha Scenic, check for another repository for Alpha Scenic main engine(which is incomplete now).
Uses OpenGL(modern OpenGL/Glad) for the rendering, 
GLFW for window management, OpenGL Mathematics(glm),
Assimp for model/texture loading from .obj objects, 
Stbi_Image with std::thread for faster parallel texture loading, 
imgui for the GUI

# Other requirements
Some libraries have not been included so you would have to set it up on your own:-
Assimp and glm
You can go head and clone them from their respective repository and put the name of the folders as "assimpLib" and "glmLib" respectively otherwise it won't work.

Next think is that this project is made on Visual Studio 2019. Other versions may create problems.
