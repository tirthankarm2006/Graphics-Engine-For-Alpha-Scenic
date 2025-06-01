#ifndef SHADER_H
#define SHADER_H

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
using namespace std;

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>


class Shader
{
public:
	unsigned int shaderID;//ID of the shader program

	/*
	Shader(const char* vertexPath, const char* fragPath);//constructor that reads and builds the shader
	//void useShader();//use/activate shader
	//uitility uniform functions
	void setBool(const std::string& name, bool value) const;//name is the name of the uniform variable in the shader
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	*/
	Shader(const char* vertexPath, const char* fragPath)
	{
		//retreive the vertex/fragment source code from filePath
		string vertexCode;
		string fragCode;
		ifstream vShaderFile;
		ifstream fShaderFile;
		//ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		try {
			//open files from the paths
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragPath);
			//creating String streams to read content from the file
			stringstream vShaderStream, fShaderStream;
			//reading file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			//close the file handlers
			vShaderFile.close();
			fShaderFile.close();
			//convert the streams into strings
			vertexCode = vShaderStream.str();
			fragCode = fShaderStream.str();
		}
		catch (ifstream::failure e) {
			cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << endl;
		}
		//converting the strings into const char*
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragCode.c_str();

		//making shader objects and compiling them
		unsigned int vertex, fragment;

		//vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkStatus(vertex, "VERTEX");
		//fragment shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkStatus(fragment, "FRAGMENT");
		//Linking the shader program
		shaderID = glCreateProgram();
		glAttachShader(shaderID, vertex);
		glAttachShader(shaderID, fragment);
		glLinkProgram(shaderID);
		checkStatus(shaderID, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	void useShader() {
		glUseProgram(shaderID);
	}
	void setBool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);//1i as the unform variable vec1 and integer type
	}
	void setInt(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
	}
	void setMatrix4(const std::string& name, glm::mat4 trans) {
		glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, glm::value_ptr(trans));
		//2nd argument tells how many matrix we want to pass, 3rd argument tells whether we want to transpose the matrix
		//last argument gives the final matrix to opengl
	}
	void setVec3(const std::string& name, const glm::vec3 &value) const {
		glUniform3fv(glGetUniformLocation(shaderID, name.c_str()),1 , &value[0]);//3fv means 3 float vector
	}
	void setVec3(const std::string& name, const float x, const float y, const float z) const {
		glm::vec3 value = glm::vec3(x,y,z);
		glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);//3fv means 3 float vector
	}
	void setVec4(const std::string& name, const glm::vec4& value) const {
		glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);//3fv means 3 float vector
	}
    private:
		void checkStatus(unsigned int objName, string type) {
			int success;
			char infolog[1024];
			if (type != "Program"){
				glGetShaderiv(objName, GL_COMPILE_STATUS, &success);
				if (!success) {
					glGetShaderInfoLog(objName, 1024, NULL, infolog);
					cout << "ERROR::SHADER::COMPILATION_FAILED::TYPE::" <<type<<"\n"<< infolog << endl;
				}
			}
			else {
				glGetProgramiv(objName, GL_LINK_STATUS, &success);
				if (!success) {
					glGetProgramInfoLog(objName, 1024, NULL, infolog);
					cout << "ERROR::PROGRAM::LINKING_FAILED::TYPE::" << type << "\n" << infolog << endl;
				}
			}
		}
};
#endif // !SHADER_H