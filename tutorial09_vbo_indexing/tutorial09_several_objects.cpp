// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");
	GLuint RectTexture = loadBMP_custom("uvtexture1.bmp");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// A flag to tell the fragment shader to render models in green
	GLuint OnlyGreenID  = glGetUniformLocation(programID, "OnlyGreen");

	// A flag to indicate whether show specular and diffuse components
	bool specularDiffuseOn = true;
	GLuint SpecularDiffuseOnID  = glGetUniformLocation(programID, "SpecularDiffuseOn");

	// L key's state of last frame
	int lastL = GLFW_RELEASE;

	// Prepare a rectangle on the z=0 plane
	static const float rect_width = 6.25;
	static const GLfloat rect_vertex_buffer_data[] = { 
		-1.0f * rect_width / 2, -1.0f * rect_width / 2, 0.0f,
		 1.0f * rect_width / 2, -1.0f * rect_width / 2, 0.0f,
		 1.0f * rect_width / 2,  1.0f * rect_width / 2, 0.0f,
		 1.0f * rect_width / 2,  1.0f * rect_width / 2, 0.0f,
		-1.0f * rect_width / 2,  1.0f * rect_width / 2, 0.0f,
		-1.0f * rect_width / 2, -1.0f * rect_width / 2, 0.0f,
	};

	static const GLfloat rect_uv_buffer_data[] = { 
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
	};

	static const GLfloat rect_normal_buffer_data[] = { 
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};

	GLuint rectvertexbuffer;
	glGenBuffers(1, &rectvertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, rectvertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertex_buffer_data), rect_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint rectuvbuffer;
	glGenBuffers(1, &rectuvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, rectuvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect_uv_buffer_data), rect_uv_buffer_data, GL_STATIC_DRAW);

	GLuint rectnormalbuffer;
	glGenBuffers(1, &rectnormalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, rectnormalbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect_normal_buffer_data), rect_normal_buffer_data, GL_STATIC_DRAW);

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

	// The model matrices for 4 Suzanne objects
	glm::mat4 ModelMatrices[4];
	for (int i = 0; i < 4; i++) {
		ModelMatrices[i] = glm::mat4();
		ModelMatrices[i] = glm::rotate(ModelMatrices[i], pi<float>() / 2, glm::vec3(1, 0, 0));
		ModelMatrices[i] = glm::rotate(ModelMatrices[i], pi<float>() / 2 * i, glm::vec3(0, 1, 0));
		ModelMatrices[i] = glm::translate(ModelMatrices[i], glm::vec3(0.0f, 1.0f, 1.87f));
	}

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do{

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the P and V matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		int curL = glfwGetKey(window, GLFW_KEY_L);
		if (lastL == GLFW_PRESS && curL == GLFW_RELEASE){
			specularDiffuseOn ^= 1;
		}
		lastL = curL;
		glUniform1i(SpecularDiffuseOnID, specularDiffuseOn);

		////// Start of the rendering of the rectangle //////

		// Use our shader
		glUseProgram(programID);

		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		glm::mat4 ModelMatrixRect = glm::mat4(1.0);
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrixRect;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixRect[0][0]);


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RectTexture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// Let the fragment shader draw everything in green
		// Comment it since we have texture now
		//glUniform1i(OnlyGreenID, 1);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, rectvertexbuffer);
		glVertexAttribPointer(
			vertexPosition_modelspaceID, // The attribute we want to configure
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, rectuvbuffer);
		glVertexAttribPointer(
			vertexUVID,                       // The attribute we want to configure
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, rectnormalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,        // The attribute we want to configure
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Make this rectangle double-sided
		glDisable(GL_CULL_FACE);

		// Draw the rectangle !
		glDrawArrays(GL_TRIANGLES, 0, 2*3); // 6 indices starting at 0 -> 2 triangles

		glUniform1i(OnlyGreenID, 0);
		glEnable(GL_CULL_FACE);

		////// End of rendering of the rectangle //////
		////// Start of the rendering of the 4 Suzanne objects //////

		// In our very specific case, the 2 objects use the same shader.
		// So it's useless to re-bind the "programID" shader, since it's already the current one.
		//glUseProgram(programID);
		
		// Similarly : don't re-set the light position and camera matrix in programID,
		// it's still valid !
		// *** You would have to do it if you used another shader ! ***
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		
		// Again : this is already done, but this only works because we use the same shader.
		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);
		
		
		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Switch to Suzanne's texture
		glBindTexture(GL_TEXTURE_2D, Texture);

		for (int i = 0; i < 4; i++) {
			// BUT the Model matrix is different (and the MVP too)
			glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrices[i];

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrices[i][0][0]);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
		}

		////// End of rendering of the 4 Suzanne objects //////

		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

