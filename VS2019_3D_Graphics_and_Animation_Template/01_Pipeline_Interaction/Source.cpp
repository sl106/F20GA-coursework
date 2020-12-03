// 3D Graphics and Animation - Main Template
// Visual Studio 2019
// Last Changed 01/10/2019

#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")
#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <iterator>
using namespace std;

#include <GL/glew.h>		// Add library to extend OpenGL to newest version
#include <GLFW/glfw3.h> // Add library to launch a window
#include <GLM/glm.hpp>	// Add helper maths library
#include <GLM/gtx/transform.hpp>

#include <stb_image.h> // Add library to load images for textures

#include "Mesh.h" // Simplest mesh holder and OBJ loader - can update more - from https://github.com/BennyQBD/ModernOpenGLTutorial

// MAIN FUNCTIONS
void setupRender();
void startup();
void update(GLfloat currentTime);
void render(GLfloat currentTime);
void endProgram();

// HELPER FUNCTIONS OPENGL
void makeTexture(string& fileName, GLuint& texture);
void hintsGLFW();
string readShader(string name);
void checkErrorShader(GLuint shader);
void errorCallbackGLFW(int error, const char* description);
void debugGL();
static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam);

// CALLBACK FUNCTIONS FOR WINDOW
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);

// VARIABLES
GLFWwindow* window; // Keep track of the window
int windowWidth = 640;
int windowHeight = 480;
bool running = true;																		// Are we still running?
glm::mat4 proj_matrix;																	// Projection Matrix
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f); // Week 5 lecture
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float aspect = (float)windowWidth / (float)windowHeight;
float fovy = 45.0f;
bool keyStatus[1024];
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLuint program;
GLint proj_location;
const int TOTAL = 5;																																													 // total number of objects
glm::vec3 body_position, lid_position, box_position, handle_position, ball_position, modelPosition;						 //variables for positions of each object
glm::vec3 modelPositions[TOTAL] = { body_position, lid_position, box_position, handle_position, ball_position }; // array of position variables
glm::vec3 modelRotation;
Mesh toy_body, toy_lid, toy_box, toy_handle, toy_ball;																											//add the toy's different parts as object
Mesh meshes[TOTAL] = { toy_body, toy_lid, toy_box, toy_handle, toy_ball };																		//put them in a list
string obj_names[TOTAL] = { "toy_body.obj", "toy_lid.obj", "toy_box.obj", "toy_handle.obj", "toy_ball.obj" }; //array of names of object
string tex_names[TOTAL] = { "T_body.jpg", "T_lid.jpg", "T_box.jpg", "T_handle.jpg", "T_ball.jpg" };						//array of names of texture images
GLuint texture[TOTAL];
GLint tex_location;
// FPS camera variables
GLfloat yaw = -90.0f;								 // init pointing to inside
GLfloat pitch = 0.0f;								 // start centered
GLfloat lastX = windowWidth / 2.0f;	 // start middle screen
GLfloat lastY = windowHeight / 2.0f; // start middle screen
bool firstMouse = true;

//light
glm::vec3 lightDisp = glm::vec3(-2.2f, 1.0f, 2.0f);
// glm::vec3 lightDisp = glm::vec3(5.0f, 3.0f, 8.0f);
float ka = 1.0f;
glm::vec3 ia = glm::vec3(0.5f, 0.5f, 0.5f);
//float kd = 1.0f;
glm::vec3 id = glm::vec3(1.0f, 1.0f, 1.0f);
//float ks = 1.0f;
glm::vec3 is = is = glm::vec3(0.3f, 0.3f, 0.3f);
//float shininess = 1.5f;

int main()
{
	if (!glfwInit())
	{ // Checking for GLFW
		cout << "Could not initialise GLFW...";
		return 0;
	}

	glfwSetErrorCallback(errorCallbackGLFW); // Setup a function to catch and display all GLFW errors.

	hintsGLFW(); // Setup glfw with various hints.

	// Start a window using GLFW
	string title = "My OpenGL Application";

	// Fullscreen
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	windowWidth = mode->width;
	windowHeight = mode->height;																																				//fullscreen
	window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), glfwGetPrimaryMonitor(), NULL); // fullscreen

	// Window
	// window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
	if (!window)
	{ // Window or OpenGL context creation failed
		cout << "Could not initialise GLFW...";
		endProgram();
		return 0;
	}

	glfwMakeContextCurrent(window); // making the OpenGL context current

	// Start GLEW (note: always initialise GLEW after creating your window context.)
	glewExperimental = GL_TRUE; // hack: catching them all - forcing newest debug callback (glDebugMessageCallback)
	GLenum errGLEW = glewInit();
	if (GLEW_OK != errGLEW)
	{ // Problems starting GLEW?
		cout << "Could not initialise GLEW...";
		endProgram();
		return 0;
	}

	debugGL(); // Setup callback to catch openGL errors.

	// Setup all the message loop callbacks.
	glfwSetWindowSizeCallback(window, onResizeCallback);			 // Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);								 // Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback); // Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);		 // Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);			 // Set callback for mouse wheel.
																														 //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor. Fullscreen
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Set mouse cursor FPS fullscreen.

	setupRender(); // setup some render variables.
	startup();		 // Setup all necessary information for startup (aka. load texture, shaders, models, etc).

	do
	{																								// run until the window is closed
		GLfloat currentTime = (GLfloat)glfwGetTime(); // retrieve timelapse
		deltaTime = currentTime - lastTime;						// Calculate delta time
		lastTime = currentTime;												// Save for next frame calculations.
		glfwPollEvents();															// poll callbacks
		update(currentTime);													// update (physics, animation, structures, etc)
		render(currentTime);													// call render function.

		glfwSwapBuffers(window); // swap buffers (avoid flickering and tearing)

		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE); // exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram(); // Close and clean everything up...

	cout << "\nPress any key to continue...\n";
	cin.ignore();
	cin.get(); // delay closing console to read debugging errors.

	return 0;
}

void errorCallbackGLFW(int error, const char* description)
{
	cout << "Error GLFW: " << description << "\n";
}

void hintsGLFW()
{
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Create context in debug mode - for debug message callback
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
}

void endProgram()
{
	glfwMakeContextCurrent(window); // destroys window handler
	glfwTerminate();								// destroys all windows and releases resources.
}

void setupRender()
{
	glfwSwapInterval(1); // Ony render when synced (V SYNC)

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 2);
	glfwWindowHint(GLFW_STEREO, GL_FALSE);
}

void startup()
{

	// Load each object
	for (int i = 0; i < TOTAL; i++)
	{
		meshes[i].LoadModel(obj_names[i]);
	}

	program = glCreateProgram();

	string vs_text = readShader("vs_model.glsl");
	const char* vs_source = vs_text.c_str();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	checkErrorShader(vs);
	glAttachShader(program, vs);

	string fs_text = readShader("fs_model.glsl");
	const char* fs_source = fs_text.c_str();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	checkErrorShader(fs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glUseProgram(program);

	// Start from the centre
	modelRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < TOTAL; i++)
	{
		modelPositions[i] = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	// A few optimizations.
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Calculate proj_matrix for the first time.
	aspect = (float)windowWidth / (float)windowHeight;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);

	//generate textures for each objects
	for (int i = 0; i < TOTAL; i++)
	{
		makeTexture(tex_names[i], texture[i]);
	}
}

//helper function for texture
void makeTexture(string& fileName, GLuint& texture)
{
	//generate texture
	// Load Texture OPENGL 4.3
	string name = fileName;
	glGenTextures(1, &texture);
	tex_location = glGetUniformLocation(program, "tex");

	// Load image Information.
	int iWidth, iHeight, iChannels;
	unsigned char* iData = stbi_load(name.c_str(), &iWidth, &iHeight, &iChannels, 0);

	if (iData)
	{
		// Load and create a texture
		glBindTexture(GL_TEXTURE_2D, texture); // All upcoming operations now have effect on this texture object
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, iWidth, iHeight);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidth, iHeight, GL_RGB, GL_UNSIGNED_BYTE, iData);

		// Set the texture wrapping parameters (next lecture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set texture filtering parameters (next lecture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Generate mipmaps (next lecture)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		//error handleing
		cout << "Could not load texture image: " << fileName << "\n";
	}
}

void update(GLfloat currentTime)
{
	if (keyStatus[GLFW_KEY_LEFT])
		modelRotation.y += 0.05f;
	if (keyStatus[GLFW_KEY_RIGHT])
		modelRotation.y -= 0.05f;
	if (keyStatus[GLFW_KEY_UP])
		modelRotation.x += 0.05f;
	if (keyStatus[GLFW_KEY_DOWN])
		modelRotation.x -= 0.05f;
	//scale up all parts
	if (keyStatus[GLFW_KEY_X])
	{
		for (int i = 0; i < TOTAL; i++)
		{
			modelPositions[i].z += 0.05f;
		}
	}
	//scale down all parts
	if (keyStatus[GLFW_KEY_Y])
	{
		for (int i = 0; i < TOTAL; i++)
		{
			modelPositions[i].z -= 0.05f;
		}
	}
	//animate product disassemble
	if (keyStatus[GLFW_KEY_M])
	{
		if (modelPositions[0].y < 0.90f)
		{
			modelPositions[0].y += 0.01f;
			modelPositions[1].y += 0.01f;
			modelPositions[0].x += 0.01f;
			modelPositions[3].x += 0.01f;
			modelPositions[4].x += 0.015f;
		}
	} //animate product assemble
	if (keyStatus[GLFW_KEY_N])
	{
		if (modelPositions[0].y > 0.0f)
		{
			modelPositions[0].y -= 0.01f;
			modelPositions[1].y -= 0.01f;
			modelPositions[0].x -= 0.01f;
			modelPositions[3].x -= 0.01f;
			modelPositions[4].x -= 0.015f;
		}
	}
	//move light position
	if (keyStatus[GLFW_KEY_O])
		lightDisp.x += 0.02f;
	if (keyStatus[GLFW_KEY_P])
		lightDisp.x -= 0.02f;

	GLfloat cameraSpeed = 1.0f * deltaTime;
	if (keyStatus[GLFW_KEY_W])
		cameraPosition += cameraSpeed * cameraFront;
	if (keyStatus[GLFW_KEY_S])
		cameraPosition -= cameraSpeed * cameraFront;
	if (keyStatus[GLFW_KEY_A])
		cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keyStatus[GLFW_KEY_D])
		cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void render(GLfloat currentTime)
{
	// Clear colour buffer
	glm::vec4 backgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

	// Clear deep buffer
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &one);

	// Enable blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader programs
	glUseProgram(program);

	// Setup camera
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition,							 // eye
		cameraPosition + cameraFront, // centre
		cameraUp);										 // up

//bind texture and draw each object
	for (int i = 0; i < TOTAL; i++)
	{
		// Bind textures and samplers
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glUniform1i(tex_location, 0);

		// Do some translations, rotations and scaling
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), modelPositions[i]);
		modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

		glm::mat4 mv_matrix = viewMatrix * modelMatrix;

		glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "proj_matrix"), 1, GL_FALSE, &proj_matrix[0][0]);

		meshes[i].Draw();
	}

	//light uniforms
	glUniform4f(glGetUniformLocation(program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
	glUniform4f(glGetUniformLocation(program, "lightPosition"), lightDisp.x, lightDisp.y, lightDisp.z, 1.0);
	glUniform4f(glGetUniformLocation(program, "ia"), ia.r, ia.g, ia.b, 1.0);
	glUniform1f(glGetUniformLocation(program, "ka"), ka);
	glUniform4f(glGetUniformLocation(program, "id"), id.r, id.g, id.b, 1.0);
	glUniform1f(glGetUniformLocation(program, "kd"), 1.0f);
	glUniform4f(glGetUniformLocation(program, "is"), is.r, is.g, is.b, 1.0);
	glUniform1f(glGetUniformLocation(program, "ks"), 1.0f);
	glUniform1f(glGetUniformLocation(program, "shininess"), 32.0f);
}

void onResizeCallback(GLFWwindow* window, int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	aspect = (float)w / (float)h;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		keyStatus[key] = true;
	else if (action == GLFW_RELEASE)
		keyStatus[key] = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
}

void onMouseMoveCallback(GLFWwindow* window, double x, double y)
{
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);

	if (firstMouse)
	{
		lastX = (float)mouseX;
		lastY = (float)mouseY;
		firstMouse = false;
	}

	GLfloat xoffset = mouseX - lastX;
	GLfloat yoffset = lastY - mouseY; // Reversed
	lastX = (float)mouseX;
	lastY = (float)mouseY;

	GLfloat sensitivity = 0.03f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// check for pitch out of bounds otherwise screen gets flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(front);
}

static void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	int yoffsetInt = static_cast<int>(yoffset);
}

void debugGL()
{
	//Output some debugging information
	cout << "VENDOR: " << (char*)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char*)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char*)glGetString(GL_RENDERER) << endl;

	// Enable Opengl Debug
	//glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam)
{

	cout << "---------------------opengl-callback------------" << endl;
	cout << "Message: " << message << endl;
	cout << "type: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << " --- ";

	cout << "id: " << id << " --- ";
	cout << "severity: ";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		cout << "NOTIFICATION";
	}
	cout << endl;
	cout << "-----------------------------------------" << endl;
}

string readShader(string name)
{
	string vs_text;
	std::ifstream vs_file(name);

	string vs_line;
	if (vs_file.is_open())
	{

		while (getline(vs_file, vs_line))
		{
			vs_text += vs_line;
			vs_text += '\n';
		}
		vs_file.close();
	}
	return vs_text;
}

void checkErrorShader(GLuint shader)
{
	// Get log lenght
	GLint maxLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	std::vector<GLchar> errorLog(maxLength);

	if (maxLength > 1)
	{
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		cout << "--------------Shader compilation error-------------\n";
		cout << errorLog.data();
	}
}
