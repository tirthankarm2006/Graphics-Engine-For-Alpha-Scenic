#include "main2.hpp"

using namespace glm;

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
unsigned int loadCubeMap(vector<std::string>faces, bool isFlipped);
unsigned int loadTexture(const char* path, bool isFlipped);
void setLightValuesShader(Shader* lightShader);

Camera camera(glm::vec3(4.0f, 2.0f, 8.0f));//camera object

const unsigned int scr_width = 1600;
const unsigned int scr_height = 900;

float deltaTime = 0.0f;
float lastTime = 0.0f;
float currentTime = 0.0f;

float lastX = scr_width / 2, lastY = scr_height / 2;
bool firstMouse = true;

static bool SpecOn = false;
static bool skybox_DirLightOn = false;
static bool setPointLights = false;
static bool setSpotLight = false;
static bool uiNotUse = false;
static float cutOffAngle =12.5f;
static float outerCutOffAngle = 17.5f;
static int i = 1;//for cube select
static float scaleModel = 1.0f;

static glm::vec3 spotPos;
static glm::vec3 spotDir;

vector<glm::vec3> cubePositions;
vector<glm::vec3>pointLightPositions;
vector<Model> models;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);// tells glfw the minimum version of opengl we will use is 3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);// tells glfw the maximum version of opengl we will use is 3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "Project Bruh", NULL, NULL);//opens a window but one frame only
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//glad loads all opengl function pointers
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//disables cursor when 'window' is 
	glfwSetCursorPosCallback(window, mouse_callback);//as soon as the window encounters mouse movement, this func. will automatically call mouse_callback() func.
	glfwSetScrollCallback(window, scroll_callback);
	
	//Shader myShader("VertexShaderCode.vs", "FragmentShaderCode.fsc");//path wrt to the include library provided in the project properties
	Shader cubeMapShader("cubeMap.vs", "cubeMap.fsc");//shader program object for cubemap
	Shader lightCubeShader("lightSourceContainer.vs", "lightSourceContainer.fsc");//shader program for light Source container
	Shader lightShader("lightSourceContainer.vs", "lightSource.fsc");//shader program for light source
	Shader shaderFor3DModel("lightSourceContainer.vs", "3DModelFS_Lighting.fsc");
	//Shader depthTestShader("lightSourceContainer.vs", "depthTest.fsc");

	Model ourModel("Resources/3Dmodels/backpack/backpack.obj");
	//Model ourModel("Resources/3Dmodels/GrozaGun/groza2.obj");
	//Model ourModel("Resources/3Dmodels/MasterChief1/masterChiefModel.glb");
	//Model ourModel("Resources/3Dmodels/MasterChief2/masterChiefModel2.glb");
	//Model ourModel("Resources/3Dmodels/MasterChief3/Master Chief/Master Chief.fbx");
	//Model ourModel("Resources/3Dmodels/Milenia(EldenRing)/Malenia.blend");
	
	//Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;// Enable Gamepad Controls
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;// IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
	
	//data for all 6 vertices of one cube
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	//locations wrt camera of the each cubes
	cubePositions = { 
	glm::vec3(2.786f, 2.339f, 4.570f),  
    glm::vec3(2.892f, 1.447f, 2.107f), 
	glm::vec3(5.804f, 2.329f, 7.126f),
	glm::vec3(6.249f, 1.768f, 4.464f)
	};
	
	spotPos = glm::vec3(1.0, -1.0, 1.0);
	spotDir = glm::vec3(1.0, 1.0, 1.0);

	pointLightPositions = {
		glm::vec3(1.0f,  1.0f, 1.0f),
		glm::vec3(2.3f, 3.3f, 4.0f),
	};

	unsigned int VBO, VAO;//VAO and VBO for the cube
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);//creates a vertex buffer object(VBO) of id as 'VBO'
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//position attribute
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);//LAST ARGUMENT IS 0 AS NO OFFSET
	glEnableVertexAttribArray(0);
	//normal attributes of the surfaces
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
	//VAO for the light(the vertices stays the same)
	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);//we only need to bind it as the data is already filled before
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//vertices for cubemap
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	//vertex array buffers for cubemap
	unsigned int cbVAO, cbVBO;
	glGenVertexArrays(1,&cbVAO);
	glGenBuffers(1, &cbVBO);
	glBindVertexArray(cbVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cbVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);//unbinding

	//Till now we just stored the data containing location of vertices of the triangles into the GPU
	//and defined a way to access the data from the GPU.

	//loading cubemap textures
	  /*GL_TEXTURE_CUBE_MAP_POSITIVE_X	Right
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X	Left
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y	Top
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	Bottom
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z	Back
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	Front*/
	vector<std::string>cubemap1{
		"Resources/cubemap1/right.jpg",
		"Resources/cubemap1/left.jpg",
		"Resources/cubemap1/top.jpg",
		"Resources/cubemap1/bottom.jpg",
		"Resources/cubemap1/front.jpg",
		"Resources/cubemap1/back.jpg"
	};
	
	unsigned int cubeMapTex = loadCubeMap(cubemap1, false);//cubemap texture ID
	cubeMapShader.useShader();
	cubeMapShader.setInt("skyBox",0);

	//generating texture ID for cube
	unsigned int cubeTexDiffuse = loadTexture("Resources/container2.png", false);
	unsigned int cubeTexSpecular = loadTexture("Resources/container2_specular.png", false);//texture without the wooden part for specular map
	unsigned int cubeTexMatrixEmission = loadTexture("Resources/matrix_emissive.jpg", false);

	//setting neccesary data in the light shader for the different light
	setLightValuesShader(&lightShader);

	lightShader.useShader();
	lightShader.setFloat("material.shininess", 32.0f);
	//assigning right texture to the right sampler2D
	lightShader.setInt("material.diffuse", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeTexDiffuse);
	lightShader.setInt("material.specular", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cubeTexSpecular);//passing the right texture ID to the right sampler2D
	lightShader.setInt("material.emissive", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, cubeTexMatrixEmission);

	lightCubeShader.useShader();
	lightCubeShader.setVec3("lightColor", glm::vec3(1.0, 1.0, 1.0));

	glm::mat4 model = glm::mat4(1.0f);//all identity matrices
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	
	while(!glfwWindowShouldClose(window))//main loop
	{
		processInput(window);

		glClearColor(0.1f, 0.2f, 0.2f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//all ImGui things first
		ImGui::Text("Select Cube Number");
		ImGui::InputInt(" ", &i);
		if (i > cubePositions.size())
			i = cubePositions.size();
		else if (i < 1)
			i = 1;
		ImGui::Text("Position of box %d", i);
		ImGui::SliderFloat("X", &cubePositions[i - 1].x, -50.0f, 50.0f);
		ImGui::SliderFloat("Y", &cubePositions[i - 1].y, -50.0f, 50.0f);
		ImGui::SliderFloat("Z", &cubePositions[i - 1].z, -50.0f, 50.0f);

		lightShader.useShader();
		if (ImGui::CollapsingHeader("Lighting Specifications"))
		{
			ImGui::Checkbox("Turn On Specular Map", &SpecOn);
			ImGui::Text("Set smoothness of edges of Spot Light");
			ImGui::SliderFloat("Cut Off angle", &cutOffAngle, 0.0f, outerCutOffAngle);
			ImGui::SliderFloat("Outer Cut Off angle", &outerCutOffAngle, 0.0f, 15.0f);
			    lightShader.setFloat("spotlight.cutOff", glm::cos(glm::radians(cutOffAngle)));
			    lightShader.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(outerCutOffAngle)));//cut off angles for spot light
				
			ImGui::Checkbox("Turn On Directional Lighting", &skybox_DirLightOn);
			    lightShader.setBool("setDirecLight", skybox_DirLightOn);
				
			ImGui::Checkbox("Turn On Point lights", &setPointLights);
			    lightShader.setBool("setPointLights", setPointLights);
				
		    ImGui::Checkbox("Turn On Spot light", &setSpotLight);
				lightShader.setBool("setSpotLight", setSpotLight);
				
		}
		if (ImGui::Button("Turn Off Cursor and focus")) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			uiNotUse = false;
		}
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
			ImGui::Text("  Press M to turn On Cursor");

		//starting to render other things
		view = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.zoom), (float)scr_width / scr_height, 0.1f, 100.0f);

		//setting the binded textures again for cubes as it was changed when drawing the model(as we used same texture types GL_TEXTURE0 later that changed the data at the location)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexDiffuse);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, cubeTexSpecular);//passing the right texture ID to the right sampler2D
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, cubeTexMatrixEmission);

		glBindVertexArray(VAO);
		lightShader.useShader();
		lightShader.setMatrix4("projection", projection);//setting the projection matrix into the shader
		lightShader.setMatrix4("view", view);
		lightShader.setVec3("viewPos", camera.Position);
		lightShader.setVec3("spotlight.position", camera.Position);
		lightShader.setVec3("spotlight.spotDir", camera.Front);

		for (int i = 0; i < cubePositions.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);//translating the models according to the data given int 'cubePositions[]'		
			//updating the model matric in vertex shader with this code's model matrix
			model = glm::rotate(model, (float)glfwGetTime() * i * 0.1f, glm::vec3(glfwGetTime() * i + 0.15, glfwGetTime() * i + 0.2, glfwGetTime() * i + 0.1));
			lightShader.setMatrix4("model", model);
			lightShader.setInt("specOn", SpecOn);
			//drawing a cube with no EBO and using glDrawArrays
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		lightCubeShader.useShader();
		lightCubeShader.setMatrix4("projection", projection);
		lightCubeShader.setMatrix4("view", view);
		//model matrix for our light translated to vec3 lightPos
		if (setPointLights) {
			for (int i = 0; i < pointLightPositions.size(); i++) {
				model = glm::mat4(1.0f);
				model = glm::translate(model, pointLightPositions[i]);
				model = glm::scale(model, glm::vec3(0.08));
				lightCubeShader.setMatrix4("model", model);
				glBindVertexArray(lightCubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		//loading the 3D model
		shaderFor3DModel.useShader();
		shaderFor3DModel.setMatrix4("projection", projection);
		shaderFor3DModel.setMatrix4("view", view);
		shaderFor3DModel.setVec3("viewPos", camera.Position);
		shaderFor3DModel.setVec3("spotlight.position", camera.Position);
		shaderFor3DModel.setVec3("spotlight.spotDir", camera.Front);
		shaderFor3DModel.setFloat("spotlight.cutOff", glm::cos(glm::radians(cutOffAngle)));
		shaderFor3DModel.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(outerCutOffAngle)));//cut off angles for spot light
		shaderFor3DModel.setBool("setDirecLight", skybox_DirLightOn);
		shaderFor3DModel.setBool("setSpotLight", setSpotLight);
		shaderFor3DModel.setBool("setPointLights", setPointLights);
		shaderFor3DModel.setBool("specOn", SpecOn);
		ImGui::Text("Scale up size of Model");
		ImGui::SliderFloat("  ", &scaleModel, 0.01, 10);
	    glm:vec3 Pos(0.0f, 2.0f, -5.0f);
		ImGui::Text("Position of Model");
		ImGui::SliderFloat("Model X", &Pos.x, -50.0f, 50.0f);
		ImGui::SliderFloat("Model Y", &Pos.y, -50.0f, 50.0f);
		ImGui::SliderFloat("Model Z", &Pos.z, -50.0f, 50.0f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, Pos); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(scaleModel));
		shaderFor3DModel.setMatrix4("model", model);
		ourModel.Draw(shaderFor3DModel);

		//loading and displaying our cubemap
		if (skybox_DirLightOn) {
			glDepthFunc(GL_LEQUAL);
			cubeMapShader.useShader();
			//view = camera.GetViewMatrix();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix()));//removes the translation part as we don't want the camera to move wrt to cubemap
			cubeMapShader.setMatrix4("view", view);
			cubeMapShader.setMatrix4("projection", projection);
			glBindVertexArray(cbVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);//uses concept of double buffer to render and display, swaps the buffer which is already rendered
		glfwPollEvents();//checks if any events are triggered and updates the window state

		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &cbVAO);
	glDeleteBuffers(1, &cbVBO);

	glfwDestroyWindow(window);//window is destroyed
	glfwTerminate();//glfw is terminated
	return 0;
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		uiNotUse = true;
	}
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)//function has to to have these exact parameters
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = 0;
	float yOffset = 0;
	if (!uiNotUse) {
		xOffset = xpos - lastX;
		yOffset = lastY - ypos;
	}
	lastX = xpos;
	lastY = ypos;
	
	camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window,double xOffset, double yOffset)//this function has to have these exact parameters
{
	camera.ProcessScrollMovement(static_cast<float>(yOffset));
}

unsigned int loadTexture(const char* path, bool isFlipped) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	int width, height, nrChannels;

	stbi_set_flip_vertically_on_load(isFlipped);

	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 2)
			format = GL_RG;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		else
			format = GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//2nd argument is 0 because we want to set texture for the base level minmap
		//3rd argument tell what type of format we want to store the texture, our image has only rgb values
		//4th and 5th argument gives the width and height of the texture
		//the 7th and 8th argument specifies the format and the datatype of the source image. The image was loaded with rgb values and stored them as chars(bytes)
		//the last argument is the actual data
	}
	else
		std::cout << "Could not load texture at "<<path;
	stbi_image_free(data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texture;
}

unsigned int loadCubeMap(vector<std::string> faces, bool isFlipped) {
	unsigned int cubeMap;
	glGenTextures(1, &cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(isFlipped);
	for (int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else {
			std::cout << "Cubemap texture failed to load at " << faces[i];
		}
		stbi_image_free(data);
	}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return cubeMap;
}

void setLightValuesShader(Shader* lightingShader) {
	lightingShader->useShader();
	lightingShader->setVec3("dLight.direction", 1.0f, -1.0f, 1.0f);
	lightingShader->setVec3("dLight.ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->setVec3("dLight.diffuse", 1.5f, 1.4f, 1.4f);
	lightingShader->setVec3("dLight.specular", 0.6f, 0.6f, 0.6f);
	// point light 1
	lightingShader->setVec3("pointLights[0].position", pointLightPositions[0]);
	lightingShader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->setVec3("pointLights[0].diffuse", 0.6f, 0.6f, 2.0f);
	lightingShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setFloat("pointLights[0].constant", 1.0f);
	lightingShader->setFloat("pointLights[0].linearCoeff", 0.09f);
	lightingShader->setFloat("pointLights[0].quadCoeff", 0.032f);
	// point light 2
	lightingShader->setVec3("pointLights[1].position", pointLightPositions[1]);
	lightingShader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->setVec3("pointLights[1].diffuse", 2.0f, 1.0f, 1.4f);
	lightingShader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setFloat("pointLights[1].constant", 1.0f);
	lightingShader->setFloat("pointLights[1].linearCoeff", 0.09f);
	lightingShader->setFloat("pointLights[1].quadCoeff", 0.032f);
	// spotLight
	lightingShader->setVec3("spotlight.ambient", 0.0f, 0.0f, 0.0f);
	lightingShader->setVec3("spotlight.diffuse", 2.0f, 2.0f, 2.0f);
	lightingShader->setVec3("spotlight.specular", 1.0f, 1.0f, 1.0f);
	lightingShader->setFloat("spotlight.constant", 1.0f);
	lightingShader->setFloat("spotlight.linearCoeff", 0.09f);
	lightingShader->setFloat("spotlight.quadCoeff", 0.032f);
}