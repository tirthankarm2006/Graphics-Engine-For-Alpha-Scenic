#include<glad/glad.h>
#include<glfw3.h>
#include<config.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

typedef enum {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
}camera_movement;

const float Yaw = -90.0f;
const float Pitch = 0.0f;
const float Speed = 2.5f;
const float Sensitivity = 0.1f;
const float ZoomFov = 45.0f;//angle for the frustum in perspective projection

class Camera {
public:
	glm::vec3 Front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 Position;
	//glm::vec3 worldUp;
	//euler's angles
	float yaw;
	float pitch;
	//camera's movement
	float movementSpeed;
	float mouseSensitivity;
	float zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f), float yaw =Yaw, float pitch = Pitch): Front(glm::vec3(0.0f,0.0f,-1.0f)), mouseSensitivity(Sensitivity), movementSpeed(Speed), zoom(ZoomFov)
	{
		Position = position;
		this->up = up;
		this->yaw = yaw;
		this->pitch = pitch;
		updateCameraVectors();//assigns the vectors Position, right, up, front
	}
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float pitch, float yaw) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), mouseSensitivity(Sensitivity), movementSpeed(Speed), zoom(ZoomFov)
	{
		Position = glm::vec3(posX, posY, posZ);
		up = glm::vec3(upX, upY, upZ);
		this->pitch = pitch;
		this->yaw = yaw;
	}

	glm::mat4 GetViewMatrix() {
		return glm::lookAt(Position, Position+Front, up);
	}

	void ProcessMouseMovement(double xOffset, double yOffset, bool constrainPitch= true)//function has to to have these exact parameters
	{
		
		xOffset *= mouseSensitivity;
		yOffset *= mouseSensitivity;

		yaw += xOffset;
		pitch += yOffset;

		if(constrainPitch)
		{
			if (pitch > 89.9f)
				pitch = 89.9f;
			if (pitch < -89.9f)
				pitch = -89.9f;
		}
		updateCameraVectors();
	}

	void ProcessScrollMovement(float yOffset)//this function has to have these exact parameters
	{
		zoom -= (float)yOffset;
		if (zoom < 1.0f)
			zoom = 1.0f;
		if (zoom > 50.0f)
			zoom = 50.0f;
	}

	void ProcessKeyboard(camera_movement direction, float deltaTime)
	{
		float velocity = movementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += velocity * Front;
		if (direction == BACKWARD)
			Position -= velocity * Front;
		if (direction == RIGHT)
			Position += right*velocity;
		if (direction == LEFT)
			Position -= right * velocity;
		if (direction == DOWN)
			Position -= velocity * up;
		if (direction == UP)
			Position += velocity * up;
	}

private:
	void updateCameraVectors() {
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		Front = glm::normalize(direction);
		right = glm::normalize(glm::cross(Front, up));
	}
};