//
//  main.cpp
//  Check Max Supported OpenGL version
//
//  Created by CGIS on 30/09/2020.
//  Copyright © 2020 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 1000;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
bool showDepthMap;
bool pressedKeys[1024];
bool mouseLocked = false;

gps::Camera myCamera(
	glm::vec3(0.0f, 2.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));


double xAnterior = 0;
double yAnterior = 0;
bool faAnimatie = false;
bool faRotatie = false;

float cameraSpeed = 0.1f;
float projectionNear = 0.1f;
float projectionFar = 2147483647.0f;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
bool wireframe = false;
float rotatieY, rotatieX;
float unghiAnimatie = 0, scaleSize = 0;
bool mareste = 0;
glm::mat4 model, copieModel;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix, copieNormalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::mat4 lightRotation;

gps::Model3D scena, ground;
gps::Shader myCustomShader;
gps::Model3D cottage, boat;


GLuint shadowMapFBO;
GLuint depthMapTexture;
GLfloat lightAngle = 0;


glm::vec4 clearColor{ 1.0f, 1.0f, 1.0f, 1.0f };

GLfloat angleY = 0;

gps::Shader skyDomeShader;
gps::Shader depthMapShader, shaderUmbre;
gps::Model3D skyDome, nanosuit;

GLuint lightColorLocSky, lightDirLocSky, projectionLocSky,
normalMatrixLocSky, viewLocSky, modelLocSky;


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

//void initSkyBox()
//{
//	std::vector<const GLchar*> faces;
//	faces.push_back("skybox/right.tga");
//	faces.push_back("skybox/left.tga");
//	faces.push_back("skybox/top.tga");
//	faces.push_back("skybox/bottom.tga");
//	faces.push_back("skybox/back.tga");
//	faces.push_back("skybox/front.tga");
//	mySkyBox.Load(faces);
//}

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	retina_height = height;
	retina_width = width;
    glViewport(0, 0, width, height);

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, projectionNear, projectionFar);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	projectionLocSky = glGetUniformLocation(skyDomeShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLocSky, 1, GL_FALSE, glm::value_ptr(projection));

	glUniformMatrix4fv(glGetUniformLocation(shaderUmbre.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	//glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	///trebuie verificat aici daca e bine!
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	//printf("%lf   %lf\n", xpos, ypos);
	double deltaX = xpos - xAnterior;
	double deltaY = yAnterior - ypos;
	xAnterior = xpos;
	yAnterior = ypos;
	

	float deltaYaw = deltaX * cameraSpeed;
	float deltaPitch = deltaY * cameraSpeed;

	myCamera.rotate(deltaPitch, deltaYaw);

}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}
bool initOpenGLWindow()
{
	if (!glfwInit())
	{
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow)
	{
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}



void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}


glm::mat4 computeLightSpaceTrMatrix()
{
	//glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.5f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 6.0f;
	//glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, 10 * far_plane);
	const GLfloat bigValue = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-bigValue, bigValue, -bigValue, bigValue, near_plane, bigValue * far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q])
	{
		angleY = -1.0f;
		///fa ceva la apasare de q
		model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	if (pressedKeys[GLFW_KEY_E])
	{
		angleY = 1.0f;
		//printf("S-a apasat E\n");
		model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	if (pressedKeys[GLFW_KEY_W])
	{
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S])
	{
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A])
	{
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D])
	{
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_U])
	{
		faAnimatie = true;
	}
	if(pressedKeys[GLFW_KEY_I])
	{
		faAnimatie = false;
	}
	if (pressedKeys[GLFW_KEY_Y])
	{
		faRotatie = false;
	}
	if (pressedKeys[GLFW_KEY_T])
	{
		faRotatie = true;
	}

	if (pressedKeys[GLFW_KEY_F]) {
		wireframe = true;
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	
	if (pressedKeys[GLFW_KEY_G])
	{
		wireframe = false;
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}



	if (pressedKeys[GLFW_KEY_L])
	{////da capture la mouse
		glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (pressedKeys[GLFW_KEY_K])
	{
		glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);///ca sa dea uncapture la mouse
	}
}

void initObjects()
{
	//tree.LoadModel("objects/Tree/Tree.obj");
	//tree.LoadModel("objects/una/Scene.obj");
	
	//ground.LoadModel("objects/RockGround/RockGround.obj");

	
	//cottage.LoadModel("objects/cottage/untitled.obj");
	//cottage.LoadModel("objects/cottage/Cottage_FREE.obj");
	//printf("A AJUNS AICI\n");
	
	//skyDome.LoadModel("objects/cottage/Cottage_FREE.obj");
	//skyDome.LoadModel("objects/Tree/Tree.obj");

	scena.LoadModel("objects/Scene/FinalSceneOhneSky.obj");
	boat.LoadModel("objects/boat/woodBoat.obj");
	skyDome.LoadModel("objects/aSky/aSky.obj");
	nanosuit.LoadModel("objects/nanosuit/nanosuit.obj");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();

	/*skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();*/

	//skyDomeShader.loadShader("shaders/shader.vert", "shaders/shader.frag");
	skyDomeShader.loadShader("shaders/skyDomeShader.vert", "shaders/skyDomeShader.frag");
	skyDomeShader.useShaderProgram();

	shaderUmbre.loadShader("shaders/shaderUmbre.vert", "shaders/shaderUmbre.frag");
	shaderUmbre.useShaderProgram();

	depthMapShader.loadShader("shaders/shader.vert", "shaders/shader.frag");
	depthMapShader.useShaderProgram();
}


void initUniforms()
{
	shaderUmbre.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shaderUmbre.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(shaderUmbre.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(shaderUmbre.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, projectionNear, projectionFar);
	projectionLoc = glGetUniformLocation(shaderUmbre.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	//lightDir = glm::vec3(100.0f, 100.0f, 100.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(shaderUmbre.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
	//glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(0.0f));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(shaderUmbre.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));




	myCustomShader.useShaderProgram();

	modelLocSky = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLocSky, 1, GL_FALSE, glm::value_ptr(model));

	viewLocSky = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLocSky, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrixLocSky = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLocSky, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projectionLocSky = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLocSky, 1, GL_FALSE, glm::value_ptr(projection));


	lightDirLocSky = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLocSky, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	lightColorLocSky = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLocSky, 1, glm::value_ptr(lightColor));
}






void cleanup() {

	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

//void faAnimatie(gps::Model3D &ceva3D, gps::Shader &cevaShader, float vitezaRotatie)
//{
//	const float scaleFactor = 0.01f;
//	const float maxScale = 1.5f;
//	cevaShader.useShaderProgram();
//	unghiAnimatie += vitezaRotatie;
//	if (mareste)
//	{
//		scaleSize += scaleFactor;
//		if (scaleSize > maxScale)
//			mareste = !mareste;
//	}
//	else
//	{
//		scaleSize -= scaleFactor;
//			if (scaleSize < -maxScale)
//				mareste = !mareste;
//	}
//	///fa ceva la apasare de q
//	copieModel = model;
//	copieNormalMatrix = normalMatrix;
//
//	//model = glm::scale(model, glm::vec3(scaleSize, scaleSize, scaleSize)); 
//	model = glm::rotate(model, glm::radians(unghiAnimatie), glm::vec3(0.0f, 1.0f, 0.0f));
//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
//	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
//
//	ceva3D.Draw(cevaShader);
//	
//	///readuc la loc lucrurile
//	model = copieModel;
//	normalMatrix = copieNormalMatrix;
//	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//}






glm::vec3 findCenterBoat()
{
	int size = 0;
	glm::vec3 center(0.0f);
	std::vector<gps::Mesh> meshes = boat.getMeshes();
	for (int i = 0; i < meshes.size(); i++)
	{
		for (const gps::Vertex& vertex : meshes[i].vertices) {
			center += vertex.Position;
		}
		
		size += meshes[i].vertices.size();
	}
	

	center /= static_cast<float>(size);

	return center;
}
int ok = 1;
float angle = 0.0f;
float radius = 10.0f;
float circleSpeed = 0.01f;
float doiPi = ((glm::pi<float>()) * 2);
float periodCircle = doiPi / circleSpeed;
float sfertPerioada = doiPi / 4;

float trX = 0, trY = 0, trZ = 0;
float rotateX = 0.0f, rotateY = 90.0f, rotateZ = 0.0f;
float scalarZ = 0.1f;
float scalarX = 0.04f;

void faAnimatieBoat(gps::Shader shaderTrimis)
{///barca este indreptata pe axa Z
	///axa X este perpendicualra cu barca
	///axa Y este triviala
	
	shaderTrimis.useShaderProgram();
	
	copieModel = model;
	copieNormalMatrix = normalMatrix;
	model = glm::translate(model, glm::vec3(0.0f, -0.18f, 10.0f));
	
	/*trZ += scalarZ;
	if (trZ > 25.0f)
		scalarZ = -scalarZ;
	if (trZ < 0.0f)
		scalarZ = -scalarZ;


	trX += scalarX;
	if (trX > 5.0f)
		scalarX = -scalarX;
	if (trX < -5.0f)
		scalarX = -scalarX;



	model = glm::translate(model, glm::vec3(trX, trY, trZ));*/


	///aici se misca in cerc
	///angle va ajunge la 2 * pi pe cand face o rotatie completa
	if (angle > doiPi )
	{
		angle = 0.0f;
		//printf("S-A RESETAT UNGHIUL ADICA A FACUT BARCA O ROTATIE\n");
	}
	
	float x = radius * cos(angle);
	float y = radius * sin(angle);
	model = glm::translate(model, glm::vec3(x, 0.0f, y));
	angle += circleSpeed;
	
	
	//periodCircle = ( glm::pi<float>() ) * 2 / circleSpeed;
	//printf("CirclePeriod: %f\n", periodCircle);
	//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 25.0f));

	rotateY += 360.0f / periodCircle;
	if (rotateY > 360.0f)
	{
		rotateY = 0.0f;
		//printf("RESET ROTATEY\n");
	}


	glm::vec3 center = findCenterBoat();
	model = glm::translate(model, center);
	float aux = -rotateY + 90.0f;
	model = glm::rotate(model, glm::radians(aux), glm::vec3(0.0f, 1.0f, 0.0f));///aici fac rotirea

	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));///pt tile la boat
	
	model = glm::translate(model, -center);

	//(glGetUniformLocation(shaderUmbre.shaderProgram, "shadowMap")
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shaderTrimis.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(glGetUniformLocation(shaderTrimis.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	boat.Draw(shaderTrimis);


	model = copieModel;
	normalMatrix = copieNormalMatrix;
	glUniformMatrix3fv(glGetUniformLocation(shaderTrimis.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shaderTrimis.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
}


//void faCameraAnimatie()
//{
//	///aici fac trransformari la model
//	model = glm::translate(model, glm::vec3(transfX, 0.0f, 0.0f));
//
//	transfX += cv;
//	if (transfX > 10.0f)
//		cv = -cv;
//
//	view = myCamera.getViewMatrix();
//	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//	glUniformMatrix4fv(viewLocSky, 1, GL_FALSE, glm::value_ptr(view));
//
//	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
//	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
//	glUniformMatrix3fv(normalMatrixLocSky, 1, GL_FALSE, glm::value_ptr(normalMatrix));
//}
float transfX = 0.5f;
float cv = 0.1f;
void faAnimatieCamera()
{
	if (faRotatie)
		myCamera.rotate(0, 0.1f);


	if (!faAnimatie)
		return;
	transfX += cv;
	if (transfX > 10.0f)
		cv = -cv;
	if (transfX < -10.0f)
		cv = -cv;
	myCamera.move(gps::MOVE_LEFT, cv);

	
	
}


void drawObjects(gps::Shader shader, bool depthPass)
{

	shader.useShaderProgram();
	
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		///
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass)
	{
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

			//
		glUniformMatrix3fv(normalMatrixLocSky, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	
	faAnimatieCamera();
	nanosuit.Draw(shader);
	scena.Draw(shader);
	faAnimatieBoat(shader);

	glUniform1i(glGetUniformLocation(shader.shaderProgram, "eSky"),1);
	skyDome.Draw(shader);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "eSky"), 0);
	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass)
	{
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLocSky, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	
}



/// pt a schimba luminozitatea in scena se schimba valoarea
/// la "dist" in fragment shader din cauza
/// marimii mare a scenei pe care o am
void renderScene() {
	////TODO drawing code
	////glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	////glClear(GL_COLOR_BUFFER_BIT);
	//myCustomShader.useShaderProgram();
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//////
	//view = myCamera.getViewMatrix();
	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	//////

	//scena.Draw(myCustomShader);
	//nanosuit.Draw(myCustomShader);
	//faAnimatieBoat(); 
	////boat.Draw(myCustomShader);

	/////aici afiseasa skyDome
	//skyDomeShader.useShaderProgram();

	//glUniformMatrix4fv(viewLocSky, 1, GL_FALSE, glm::value_ptr(view));
	//glUniformMatrix3fv(normalMatrixLocSky, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//glUniform3fv(lightDirLocSky, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	//skyDome.Draw(skyDomeShader);
	/////aici se intoarce la shaderul normal
	//myCustomShader.useShaderProgram();

	//faAnimatie(ground, myCustomShader, 0.5f);///pot adauga si scaling
	//ground.Draw(myCustomShader);
	

	/*skyDomeShader.useShaderProgram();

	glUniformMatrix4fv(viewLocSky, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix3fv(normalMatrixLocSky, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glUniform3fv(lightDirLocSky, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	skyDome.Draw(skyDomeShader);*/
	//tree.Draw(skyDomeShader);///DECI NU SHADER UL E DE VINA
	//skyDome.Draw(myCustomShader);

	//cottage.Draw(myCustomShader);

	//mySkyBox.Draw(skyboxShader, view, projection);
	//myCustomShader.useShaderProgram();///pt ca functia de sus schimba shaderul la skyBoxShader



	
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthMapShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (true)
	{
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderUmbre.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		//
		glUniformMatrix4fv(viewLocSky, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
		//
		glUniform3fv(lightDirLocSky, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
		
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(shaderUmbre.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(shaderUmbre.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(shaderUmbre, false);
	}
	
}

void initFBO()
{
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow())
	{
		glfwTerminate();
		return 1;
	}
	
	

	///aici pun initurile
	//initSkyBox();
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		
		processMovement();
		renderScene();

		glfwSwapBuffers(glWindow);
		glfwPollEvents();		
	}
	
	cleanup();
	return 0;
}
