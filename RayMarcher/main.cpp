#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>

#include "Shader.h"
#include "ComputeShader.h"
#include "Camera.h"

constexpr auto PI = 3.1415926535f;

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

GLFWwindow* window;
Camera camera;

double dTime = 0.0;
double lastTime = 0.0;

const float speed = 5.0f;
const float sensitivity = 0.0008f;
bool cursorHidden = true;
glm::vec2 mousePos(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

GLFWwindow* Initialize(int width, int height, const char* title, int vsync);
void SetupBuffers(GLuint& VAO);
void SetupTexture(GLuint width, GLuint height, GLuint& texture);
void GetComputeGroupInfo();
void KeyBoardInput();
void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

float clamp(float n, float l, float h)
{
	float t = (n > h) ? h : n;
	return (t < l) ? l : t;
}

void printMat4(const glm::mat4& m)
{
	std::cout << std::fixed << std::setprecision(2);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << m[j][i] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

void main()
{
	window = Initialize(WINDOW_WIDTH, WINDOW_HEIGHT, "RayMarcher", 1);

	Shader shader("C:/Users/jonat/source/repos/RayMarcher/RayMarcher/vertex.glsl", "C:/Users/jonat/source/repos/RayMarcher/RayMarcher/fragment.glsl");
	ComputeShader computeShader("C:/Users/jonat/source/repos/RayMarcher/RayMarcher/compute.glsl");

	GLuint QuadVAO;
	SetupBuffers(QuadVAO);

	GLuint texWidth = WINDOW_WIDTH, texHeight = WINDOW_HEIGHT;
	GLuint texture;
	SetupTexture(texWidth, texHeight, texture);

	GetComputeGroupInfo();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glm::mat4 projection = glm::perspective(PI / 2, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.01f, 10000.0f);
	glm::mat4 invProjection = glm::inverse(projection);

	camera = Camera(glm::vec3(0.0f, 0.0f, -10.0f));

	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		dTime = currentTime - lastTime;
		lastTime = currentTime;

		KeyBoardInput();

		computeShader.use();
		computeShader.setMat4("cameraToWorld", glm::inverse(camera.GetViewMatrix()));
		computeShader.setMat4("invProjection", invProjection);
		computeShader.dispatch(texWidth, texHeight, 1);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();

		glBindVertexArray(QuadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
}

glm::vec3 erot(glm::vec3 p, glm::vec3 ax, float ro)
{
	return glm::mix(glm::dot(p, ax) * ax, p, cos(ro)) + sin(ro) * glm::cross(ax, p);
}

GLFWwindow* Initialize(int width, int height, const char* title, int vsync)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL)
	{
		std::cout << "GLFW: failed to create window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, MouseMoveCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSwapInterval(vsync);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD: failed to load" << std::endl;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	return window;
}

void SetupBuffers(GLuint& VAO)
{
	GLfloat quad[] = {
	-1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	 1.0f,  1.0f, 1.0f, 1.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f
	};

	GLuint VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
}

void SetupTexture(GLuint width, GLuint height, GLuint& texture)
{
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float* data = new float[4 * width * height];
	memset(data, 0.0f, 4 * width * height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void GetComputeGroupInfo()
{
	int workGroupCount[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCount[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCount[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCount[2]);

	std::cout << "Work group count: " << workGroupCount[0] << " " << workGroupCount[1] << " " << workGroupCount[2] << std::endl;

	int workGroupSize[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSize[2]);

	std::cout << "Work group size: " << workGroupSize[0] << " " << workGroupSize[1] << " " << workGroupSize[2] << std::endl;

	int workGroupInvocations;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGroupInvocations);

	std::cout << "Work group invocations: " << workGroupInvocations << std::endl;
}

void KeyBoardInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, cursorHidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		cursorHidden = !cursorHidden;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, dTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, dTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, dTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, dTime);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.ProcessKeyboard(UPWARD, dTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.ProcessKeyboard(DOWNWARD, dTime);
	}
	
}

void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	float offsetx = mousePos.x - xpos;
	float offsety = mousePos.y - ypos;
	mousePos = glm::vec2(float(xpos), float(ypos));

	if (cursorHidden) {
		camera.ProcessMouseMovement(-offsetx, offsety);
	}
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;
}