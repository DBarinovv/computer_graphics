#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

int main(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1024, 768, "Tutorial 04 - Colored Cube", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // background color

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Stellated octahedron (± 1/2, ± 1/2, 0) (0, 0, ± 1 / √2) (± 1, 0, ± 1 / √2) (0, ± 1, ± 1 / √2)
	const GLfloat ver1[] = {  0.5f, 0.5f, 0.0f };          // A
	const GLfloat ver2[] = { -0.5f, -0.5f, 0.0f };         // B
	const GLfloat ver3[] = {  0.5f, -0.5f, 0.0f };         // C
	const GLfloat ver4[] = { -0.5f, 0.5f, 0.0f };          // D

	const GLfloat ver5[] = { 0.0f, 0.0f, 1 / sqrt(2) };    // E
	const GLfloat ver6[] = { 0.0f, 0.0f, -1 / sqrt(2) };   // F

	const GLfloat ver7[] = { 1.0f, 0.0f, 1 / sqrt(2) };    // G
	const GLfloat ver8[] = { -1.0f, 0.0f, 1 / sqrt(2) };   // H
	const GLfloat ver9[] = { 1.0f, 0.0f, -1 / sqrt(2) };   // I
	const GLfloat ver10[] = { -1.0f, 0.0f, -1 / sqrt(2) }; // J

	const GLfloat ver11[] = { 0.0f, 1.0f, 1 / sqrt(2) };   // K
	const GLfloat ver12[] = { 0.0f, -1.0f, 1 / sqrt(2) };  // L
	const GLfloat ver13[] = { 0.0f, 1.0f, -1 / sqrt(2) };  // M
	const GLfloat ver14[] = { 0.0f, -1.0f, -1 / sqrt(2) }; // N

	auto stellated_octahedron =
	{
		ver1, ver5, ver11,
		ver1, ver7, ver5,
		ver1, ver3, ver7,
		ver1, ver4, ver11,
		ver3, ver7, ver5,
		ver3, ver5, ver12,
		ver3, ver12, ver2,
		ver2, ver12, ver5,
		ver2, ver5, ver8,
		ver2, ver8, ver4,
		ver4, ver8, ver5,
		ver4, ver5, ver11,

		ver1, ver13, ver4,
		ver1, ver6, ver13,
		ver1, ver9, ver6,
		ver1, ver3, ver9,
		ver3, ver6, ver9,
		ver3, ver14, ver6,
		ver3, ver2, ver14,
		ver2, ver6, ver14,
		ver2, ver10, ver6,
		ver2, ver4, ver10,
		ver4, ver10, ver6,
		ver4, ver6, ver13
	};

	std::vector<GLfloat> res;
	for (const auto& i: stellated_octahedron)
	{
		res.push_back(i[0]);
		res.push_back(i[1]);
		res.push_back(i[2]);
	}

	GLfloat g_vertex_buffer_data[2048];
	std::copy(res.begin(), res.end(), g_vertex_buffer_data);

	// One color for each vertex. They were generated randomly.
	static const GLfloat g_color_buffer_data[] = {
		#include "colors.h"
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	float i = 0.0;
	bool dir = true;
	do {
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
		i += 0.001;
		glm::mat4 View = glm::lookAt(
			glm::vec3(5 * cos(i), 0, 5 * sin(i)),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);

		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model = glm::mat4(1.0f);
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen

		glUseProgram(programID); // Use our shader

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, res.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	glfwTerminate();

	return 0;
}
