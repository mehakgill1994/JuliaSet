// Assignment1_OpenGL.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_helper.h"

//window client area size

const int ClientAreaWidth = 800;
const int ClientAreaHeight = 800;

//global variables

float cReal, cImag;

struct Vertex
{
	float position[2];
};

GLFWwindow *window;
GLuint gpuProgram;
GLuint vertexBuffer, indexBuffer, vertexArray;


//locations of uniform variables in the GPU program
GLuint locMatProjection;

//globals for time measurement
LARGE_INTEGER clockFreq;
LARGE_INTEGER prevTime;
unsigned int texture;\
float counter = 0;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

bool InitWindow();

bool LoadShaders();
bool loadTextures();
void CreateSquare();
void InitUniforms();
void InitInputAssembler();
void InitRasterizer();
void InitPerSampleProcessing();

void MainLoop();
void Render();
void Cleanup();

int main()
{
	if (!glfwInit())
	{
		std::cerr << "Failed to load GLFW!" << std::endl;
		return 1;
	}

	if (!InitWindow())
	{
		std::cerr << "Failed to create window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to load GLEW!" << std::endl;
		glfwTerminate();
		return 1;
	}

	//init time measurement
	::QueryPerformanceFrequency(&clockFreq);
	::QueryPerformanceCounter(&prevTime);

	if (!LoadShaders())
	{
		std::cerr << "Failed to load shaders!" << std::endl;
		glfwTerminate();
		return 1;
	}

	if (!loadTextures())
	{
		Cleanup();
		return 1;
	}

	CreateSquare();
	InitUniforms();
	InitInputAssembler();
	InitRasterizer();
	InitPerSampleProcessing();

	MainLoop();

	Cleanup();

	glfwTerminate();
	return 0;
}

bool InitWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(ClientAreaWidth, ClientAreaHeight, "Assignment 1", nullptr, nullptr);

	if (!window)
		return false;

	//set the window's OpenGL context as the current OpenGL context

	glfwMakeContextCurrent(window);

	//set event handlers for the window

	glfwSetKeyCallback(window, key_callback);

	//the parameter 1 means VSync is enabled
	//change to 0 to disable VSync

	glfwSwapInterval(1);

	return true;
}

bool loadTextures()
{
	int width, height, nrChannels;
	unsigned char *data = stbi_load("texture.jpg", &width, &height, &nrChannels, 0);

	/*unsigned int texture;*/
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	return true;
}


bool LoadShaders()
{
	//load shaders

	GLenum vertexShader, fragmentShader;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	bool success = false;

	if (try_compile_shader_from_file(vertexShader, "VertexShader.glsl") &&
		try_compile_shader_from_file(fragmentShader, "FragmentShader.glsl"))
	{
		gpuProgram = glCreateProgram();

		glAttachShader(gpuProgram, vertexShader);
		glAttachShader(gpuProgram, fragmentShader);

		success = try_link_program(gpuProgram);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return success;
}

void CreateSquare()
{
	//each vertex is 2 floats for the position

	Vertex vertexBufferData[] = {
		{ -2.0f, +2.0f },
		{ -2.0f, -2.0f },
		{ +2.0f, +2.0f },
		{ +2.0f, -2.0f }
	};

	//this is going to be a triangle strip, so we don't need an index buffer

	//create vertex buffer and store data into it

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, 0);
}

void InitUniforms()
{
	//TODO
	locMatProjection = glGetUniformLocation(gpuProgram, "matProjection");
	glm::mat4 matProjection = glm::frustum(-2.f, +2.f, -2.f, +2.f, 1.f, 10.f);
	glProgramUniformMatrix4fv(gpuProgram, locMatProjection, 1, GL_FALSE, glm::value_ptr(matProjection));

	int MaxIterationLocation = glGetUniformLocation(gpuProgram, "MaxIterations");
	glUseProgram(gpuProgram);
	glUniform1f(MaxIterationLocation, 30);


	/*int cLocation = glGetUniformLocation(gpuProgram, "c");
	glUseProgram(gpuProgram);
	glUniform2f(cLocation, cReal, cImag);*/
}

void InitInputAssembler()
{
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	//index		num_components	type				normalize?	stride			offset
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float[2]), (void*)0);
	glEnableVertexAttribArray(0);
}

void InitRasterizer()
{
	glViewport(0, 0, ClientAreaWidth, ClientAreaHeight);
	glEnable(GL_CULL_FACE);
}

void InitPerSampleProcessing()
{
	glClearColor(0.6f, 0.7f, 0.9f, 1.0f);
	glDisable(GL_DEPTH_TEST);
}

void Cleanup()
{
	glDeleteProgram(gpuProgram);
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteBuffers(1, &vertexBuffer);
}

void Render()
{
	//clear the color buffer

	glClear(GL_COLOR_BUFFER_BIT);

	//bind whatever needs to be bound

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBindVertexArray(vertexArray);
	glUseProgram(gpuProgram);

	//draw the triangles

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//swap the back and front buffers

	glfwSwapBuffers(window);

	//update angle using time

	//TODO
	LARGE_INTEGER currentTime;
	::QueryPerformanceCounter(&currentTime);

	float elapsedTime = (float)(double(currentTime.QuadPart - prevTime.QuadPart) / clockFreq.QuadPart);
	
	cReal = -0.7885 * cos(0.1 * 22 / 7 + counter);
	cImag = -0.7885 * sin(0.1 * 22 / 7 + counter);
	counter += 0.002;

	prevTime = currentTime;

	/*cReal = -0.835;
	cImag = -0.2321;*/
	int cLocation = glGetUniformLocation(gpuProgram, "c");
	glUseProgram(gpuProgram);
	glUniform2f(cLocation, cReal, cImag);
}

void MainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		Render();
		glfwPollEvents();
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action = GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);
	}
}










