#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<string>
#include<vector>
#include<ShaderHandler.h>

using namespace std;

struct Vertex {
	glm::vec3 pos;//position vector
	glm::vec3 normal;//normal vector
	glm::vec2 texCoords;//a texture coordinate vector
};

struct Texture {
	unsigned int id;//texture ids
    std::string type;//example diffuse type or specular type(like we used different textures for diffuse maps and specular maps)
	std::string path;//path/directory where the texture is stored
};

class Mesh {
public:
	//Mesh data
	vector<Vertex>vertices;
	vector<Texture>textures;
	vector<unsigned int>indices;//indices required in index buffering

	Mesh(vector<Vertex>vertices, vector<Texture>textures, vector<unsigned int>indices) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	}

	void draw(Shader& shader) {
		unsigned int diffuseNr = 0;
		unsigned int specularNr = 0;
		shader.useShader();
		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);//activating the right texture
			string number;
			string name = textures[i].type;
			
			if (name == "diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "specular")
				number = std::to_string(specularNr++);

			shader.setInt(("material." + name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		//draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
private:
	//render data
	unsigned int VAO, VBO, EBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		//normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		//texture coordinates
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

		glBindVertexArray(0);
	}
};