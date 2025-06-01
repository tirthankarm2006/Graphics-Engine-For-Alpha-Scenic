# Graphics-Engine-For-Alpha-Scenic
This will be the design of the Graphics engine in Alpha Scenic, check for another repository for Alpha Scenic main engine(which is incomplete now).
Uses OpenGL(modern OpenGL/Glad) for the rendering, 
GLFW for window management, OpenGL Mathematics(glm),
Assimp for model/texture loading from .obj objects, 
Stbi_Image with std::thread for faster parallel texture loading, 
imgui for the GUI

I am still learning so some of the codes like the model.h uses the exact same code form learnopengl.com though I have optimized the method in a new model class ("newModelClass.h") that
use std::thread for parallel texture loading and for the same reason this engine does not use all concepts from 3D Graphics Programming like post processing, PBR, animations.

About the game engine I am making, Alpha Scenic, I have not written any code there as I have to first grasp the entire 3D Graphics and how a game engine pipeline works so it is going to take 
quite a lot of time :) and it will use many other libraries not mentioned here like spdlog and I will have to first learn all of these libraries to be able to fully develop the game engine.

# Other requirements
Some libraries have not been included so you would have to set it up on your own:-
Assimp and glm
You can go head and clone them from their respective repository and put the name of the folders as "assimpLib" and "glmLib" respectively otherwise it won't work.

Next think is that this project is made on Visual Studio 2019. Other versions may create problems.

# Few things when using
> You can load your own 3D model but make sure to properly make an object in the main file and that the textures are in the right folder otherwise Assimp won't understand.
> There are two model classes "newModelClass.h" and "model.h". You can use which you want by including it in "main2.hpp"
> There are two set of lighting parameters, one for the cubes another for the models. The lighting can be changed only from the code. Change the parameters for model from which ever
  model class you are using and the parameters for the cubes can be changed from the main file(go to the end of the file).
