// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <cmath>

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


const float PI = pi<float>();
const float EPSILON = 1e-6;

// Initial radius : 10
float radius = 10;
// Initial camera horizontal angle : on +X axis (+Z is up. Always look at the origin)
float horizontalAngle = 0;
// Initial camera vertical angle : on xy plane (+Z is up. Always look at the origin)
float verticalAngle = PI * 0.5;
// Initial Field of View
float initialFoV = 45.0f;

float angleSpeed = 3.0f;
float distSpeed = 15.0f;



void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Move closer
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		radius -= deltaTime * distSpeed;
	}
	// Move farther
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		radius += deltaTime * distSpeed;
	}
	// Restrict the radius >= 0
	radius = max(radius, EPSILON);

	// Rotates the camera to the left
	if (glfwGetKey( window, GLFW_KEY_A) == GLFW_PRESS){
		horizontalAngle -= deltaTime * angleSpeed;
	}
	// Rotates the camera to the right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		horizontalAngle += deltaTime * angleSpeed;
	}
	// Make the horizontalAngle loop within [0, PI * 2]
	horizontalAngle = fmod(horizontalAngle + PI * 2, PI * 2);

	// radially rotates the camera up
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		verticalAngle -= deltaTime * angleSpeed;
	}
	// radially rotates the camera down
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		verticalAngle += deltaTime * angleSpeed;
	}
	// Restrict the verticalAngle within [0, PI]
	verticalAngle = glm::clamp(verticalAngle, EPSILON, PI - EPSILON);
	
	glm::vec3 position = glm::vec3(
		radius * sin(verticalAngle) * cos(horizontalAngle),
		radius * sin(verticalAngle) * sin(horizontalAngle),
		radius * cos(verticalAngle)
	);
	
	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								glm::vec3(), // and looks here : at the same position, plus "direction"
								glm::vec3(0, 0, 1)  // Head is up (set to 0,0,1 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}