#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "meshClass.h"
#include "ShaderHandler.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <unordered_map>

void setLightValuesShader_3DModel(Shader& lightingShader);
vector<glm::vec3>pointLightPos = {
		glm::vec3(1.0f,  1.0f, 1.0f),
		glm::vec3(2.3f, 3.3f, 4.0f),
};
bool once = false;
int numMeshes = 0;

struct TextureDataGl {
	unsigned char* data;
	int width, height, channels;
	std::string name;
};
std::unordered_map<std::string, TextureDataGl> g_TextureQueue;
static std::mutex g_TextureQueueMutex;

static void stbi_dataFromImage(string path, string directory);
static void uploadTextureToGl(const TextureDataGl& texData, unsigned int texID);

class Model {
public:
	Model(const char* path) {
		loadModel(path);
		std::cout << "No. of Model textures are :- " << textures_loaded.size() << endl;
		std::cout << "No. of meshes it has " << meshes.size();
	}
	//main function that draws the entire model by drawing the individual meshes
	void Draw(Shader& shader) {
		if (!once) {
			setLightValuesShader_3DModel(shader);
			once = true;
		}
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].draw(shader);
		}
	}
private:
	//model data
	vector<Mesh>meshes;
	string directory;
	vector<Texture>textures_loaded;

	void loadModel(string path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		//aiProcess_Triangulate transforms model's primitive shape to a triangle
		//aiProcess_FlipUVs flips the texture coordinates on the y-axis where necessary

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "Error::Assimp::" << importer.GetErrorString() << endl;
			return;
		}

		directory = path.substr(0, path.find_last_of('/'));//path has the location of the .obj file this func. extracts the folder in which the file is kept

		processNode(scene->mRootNode, scene);//processNode() func. will recursively go through all the nodes in the binary tree in which the model is kept

		//Launch threads to load images
		std::vector<std::thread> threads;
		for (auto& thisTex : textures_loaded) {
			threads.emplace_back(stbi_dataFromImage, thisTex.path, directory);
		}

		//Wait for all threads to finish
		for (auto& thread : threads) {
			thread.join();
			std::cout << "Thread terminated" << endl;
		}

		std::cout << "No. of texture data loaded into vector " << g_TextureQueue.size() << endl << endl;

		std::lock_guard<std::mutex> lock(g_TextureQueueMutex);
		int i = 0;
		for (auto& texture : textures_loaded) {
			TextureDataGl& tex = g_TextureQueue[texture.path];
			uploadTextureToGl(tex, texture.id);
			stbi_image_free(tex.data);
		}

		for (auto& texture : textures_loaded) {
			std::cout << texture.path << endl;
		}
	}

	//this makes the final mesh data which is the vector of meshes
	void processNode(aiNode* node, const aiScene* scene) {
		//process all the node's meshes
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];//mMeshes of the aiNode* node is a pointer to the array of indexes to the array meshes(mMeshes array of type aiMesh) of aiScene* scene
			//i is the indices to the mesh in the node ONLY. mMeshes[i] is the index to the mesh in the mMeshes[] in the scene node
			meshes.push_back(processMesh(mesh, scene));//pushes the mesh objects onto the back of the vector of meshes
		}
		//doing the same of the every children of the root(or the node we just passed in the function)
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	//meshes vector is of type mesh, so we have the convert the assimp mesh object into our mesh object
	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		vector<Vertex>vertices;
		vector<unsigned int>indices;
		vector<Texture>textures;

		//extracting all data for the vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			//extracting the position
			glm::vec3 vec;
			vec.x = mesh->mVertices[i].x;
			vec.y = mesh->mVertices[i].y;
			vec.z = mesh->mVertices[i].z;
			vertex.pos = vec;
			//extracting the normals
			vec.x = mesh->mNormals[i].x;
			vec.y = mesh->mNormals[i].y;
			vec.z = mesh->mNormals[i].z;
			vertex.normal = vec;
			//extracting texture coordinates
			if (mesh->mTextureCoords[0])//check if the mesh contains a texture
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.texCoords = vec;
			}
			else
				vertex.texCoords = glm::vec2(0.0f, 0.0f);
			vertices.push_back(vertex);
		}

		//extracting all the indices
		//a meshes is made of array of primitive faces(like triangle or square) but as we used aiProcess_Triangulate, the primitive of the meshes(or faces) is a triangle
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);//indices are for vertices in one mesh
			}
		}

		//extracting material for a mesh
		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];//a mesh has only one material, mesh->mMaterialIndex gives the index of the this mesh's material in the scenes mMaterials array

			//extracting the diffuse textures
			vector<Texture>diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::cout << "Diffuse textures are "<<endl;
			for (auto& tex : textures)
				std::cout << tex.path << endl;

			//extracting the specular textures
			vector<Texture>specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

			std::cout << "Specular textures are " << endl;
			for (auto& tex : textures) {
				if (tex.type == "specular")
					std::cout << tex.path << endl;
			}
		}
		return Mesh(vertices, textures, indices);
	}
	vector<Texture>loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector<Texture>textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);//here we get location of the texture file(.jpg) with respect the directory where the model is stored, here "directory"
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip) {
			    Texture texture;
			    unsigned int texID;
				glGenTextures(1, &texID);
				texture.id = texID;
			    texture.type = typeName;
			    texture.path = string(str.C_Str());
			    textures_loaded.push_back(texture);
			    textures.push_back(texture);
			}
		}
		return textures;
	}
};

//worker thread
void stbi_dataFromImage(string path, string directory) {
	//full file path by adding the path in directory(where is kept) and the path of the texture wrt to directory
	string fullFilePath = directory + '/' + path;

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(fullFilePath.c_str(), &width, &height, &nrChannels, 0);

	if (data) {
		std::cout << "Model textures loaded at " << path << endl;
	}
	else {
		data = stbi_load("Resources/3Dmodels/default_texture.png", &width, &height, &nrChannels, 0);
		std::cout << "Could not load Model texture at " << path << endl;
		std::cout << "  Loaded default texture" << endl << endl;
	}

	std::lock_guard<std::mutex> lock(g_TextureQueueMutex);
	g_TextureQueue.emplace(path, TextureDataGl{data, width, height, nrChannels, path});
}

void uploadTextureToGl(const TextureDataGl& texData, unsigned int texID) {
	glBindTexture(GL_TEXTURE_2D, texID);
	
	GLenum format;
	if (texData.channels == 1)
		format = GL_RED;
	else if (texData.channels == 2)
		format = GL_RG;
	else if (texData.channels == 3)
		format = GL_RGB;
	else if (texData.channels == 4)
		format = GL_RGBA;
	else
		format = GL_RGB;

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, texData.width, texData.height, 0, format, GL_UNSIGNED_BYTE, texData.data);
	//2nd argument is 0 because we want to set texture for the base level minmap
	//3rd argument tell what type of format we want to store the texture, our image has only rgb values
	//4th and 5th argument gives the width and height of the texture
	//the 7th and 8th argument specifies the format and the datatype of the source image. The image was loaded with rgb values and stored them as chars(bytes)
	//the last argument is the actual data
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void setLightValuesShader_3DModel(Shader& lightingShader) {
	lightingShader.useShader();
	lightingShader.setVec3("dLight.direction", 1.0f, -1.0f, 1.0f);
	lightingShader.setVec3("dLight.ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("dLight.diffuse", 1.5f, 1.5f, 1.5f);
	lightingShader.setVec3("dLight.specular", 0.6f, 0.6f, 0.6f);
	// point light 1
	lightingShader.setVec3("pointLights[0].position", pointLightPos[0]);
	lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[0].diffuse", 0.6f, 0.6f, 2.0f);
	lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[0].constant", 1.0f);
	lightingShader.setFloat("pointLights[0].linearCoeff", 0.09f);
	lightingShader.setFloat("pointLights[0].quadCoeff", 0.032f);
	// point light 2
	lightingShader.setVec3("pointLights[1].position", pointLightPos[1]);
	lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[1].diffuse", 2.0f, 1.0f, 1.4f);
	lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[1].constant", 1.0f);
	lightingShader.setFloat("pointLights[1].linearCoeff", 0.09f);
	lightingShader.setFloat("pointLights[1].quadCoeff", 0.032f);
	// spotLight
	lightingShader.setVec3("spotlight.ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("spotlight.diffuse", 2.0f, 2.0f, 2.0f);
	lightingShader.setVec3("spotlight.specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("spotlight.constant", 1.0f);
	lightingShader.setFloat("spotlight.linearCoeff", 0.09f);
	lightingShader.setFloat("spotlight.quadCoeff", 0.032f);
	lightingShader.setFloat("material.shininess", 2.0f);
}
