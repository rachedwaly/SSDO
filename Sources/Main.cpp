// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>
#include <filesystem>

namespace fs = std::filesystem;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Resources.h"
#include "Error.h"
#include "Console.h"
#include "MeshLoader.h"
#include "Scene.h"
#include "Image.h"
#include "Rasterizer.h"
#include "RayTracer.h"

#include "Material.h"
#include "SSAO.h"

using namespace std;




#define DEFFERED
//#define RASTERIZER


// Window parameters
static GLFWwindow* windowPtr = nullptr;
static std::shared_ptr<Scene> scenePtr;
#ifdef RASTERIZER
static std::shared_ptr<Rasterizer> rasterizerPtr;
static std::shared_ptr<RayTracer> rayTracerPtr;
#endif



#ifdef DEFFERED
static std::shared_ptr<DefferedRenderer> DefferedPtr;
static bool isDisplaySSAO(false);
#endif








// Camera control variables
static glm::vec3 center = glm::vec3(0.0); // To update based on the mesh position
static float meshScale = 1.0; // To update based on the mesh size, so that navigation runs at scale
static bool isRotating(false);
static bool isPanning(false);
static bool isZooming(false);
static double baseX(0.0), baseY(0.0);
static glm::vec3 baseTrans(0.0);
static glm::vec3 baseRot(0.0);

// Files
static std::string basePath;
static std::string meshFilename;

// Raytraced rendering
static bool isDisplayRaytracing(false);

void clear();

void printHelp() {
	Console::print(std::string("Help:\n")
		+ "\tMouse commands:\n"
		+ "\t* Left button: rotate camera\n"
		+ "\t* Middle button: zoom\n"
		+ "\t* Right button: pan camera\n"
		+ "\tKeyboard commands:\n"
		+ "\t* ESC: quit the program\n"
		+ "\t* H: print this help\n"
		+ "\t* F: decrease field of view\n"
		+ "\t* G: increase field of view\n"
#ifdef RASTERIZER
		+ "\t* F12: reload GPU shaders\n"
		+ "\t* TAB: switch between rasterization and ray tracing display\n"
		+ "\t* K: execute ray tracing with accelerating data structure\n"
		+ "\t* SPACE: execute basic ray tracing without accelerating data structure\n");
#endif

#ifdef DEFFERED
	+"\t* S: toggle screen space feature can be SSDO or SSAO\n"
	+ "\t* F1: activate SSAO mode in the scene\n"
	+ "\t* F2: activate standard Deffered mode in the scene\n"
	+ "\t* F3: activate SSDO mode in the scene\n"
	+ "\t* F4: show only the SSAO texture on the red channel\n");
#endif

}

/// Adjust the ray tracer target resolution and runs it.
void raytrace() {
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
#ifdef RASTERIZER
	rayTracerPtr->setResolution(width, height);
	rayTracerPtr->render(scenePtr);
#endif


}

void raytraceAABB() {
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
#ifdef RASTERIZER
	rayTracerPtr->setResolution(width, height);
	rayTracerPtr->renderWithAABB(scenePtr);
#endif
}

/// Executed each time a key is entered.
void keyCallback(GLFWwindow* windowPtr, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_H) {
			printHelp();
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(windowPtr, true); // Closes the application if the escape key is pressed
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_F12) {
#ifdef RASTERIZER
			rasterizerPtr->loadShaderProgram(basePath);
#endif
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
			scenePtr->camera()->setFoV(std::max(5.f, scenePtr->camera()->getFoV() - 5.f));
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_G) {
			scenePtr->camera()->setFoV(std::min(120.f, scenePtr->camera()->getFoV() + 5.f));
		}

#ifdef DEFFERED
		else if (action == GLFW_PRESS && key == GLFW_KEY_S) {
			DefferedPtr->toggleSSFeature();
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
		Console::print("SSAO with 1 directional light");
		DefferedPtr->activateSSAO();
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_F2) {
		Console::print("Standard deffered shading with 31 directional lights");
		DefferedPtr->activateDeffered();
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_F3) {
		Console::print("SSDO with 1 directional light");
		DefferedPtr->activateSSDO();
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_F4) {
		Console::print("SSAO texture only");
		DefferedPtr->activateSSAOTexture();
		}
		else {
			printHelp();
		}
#endif



#ifdef RASTERIZER
		else if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {

			isDisplayRaytracing = !isDisplayRaytracing;
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {

			raytrace();

		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_K) {
			raytraceAABB();
		}
		else {
			printHelp();
		}
#endif
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height) / 2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating) {
		glm::vec3 dRot(-dy * M_PI, dx * M_PI, 0.0);
		scenePtr->camera()->setRotation(baseRot + dRot);
	}
	else if (isPanning) {
		scenePtr->camera()->setTranslation(baseTrans + meshScale * glm::vec3(dx, dy, 0.0));
	}
	else if (isZooming) {
		scenePtr->camera()->setTranslation(baseTrans + meshScale * glm::vec3(0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (!isRotating) {
			isRotating = true;
			glfwGetCursorPos(window, &baseX, &baseY);
			baseRot = scenePtr->camera()->getRotation();
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		isRotating = false;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (!isPanning) {
			isPanning = true;
			glfwGetCursorPos(window, &baseX, &baseY);
			baseTrans = scenePtr->camera()->getTranslation();
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		isPanning = false;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		if (!isZooming) {
			isZooming = true;
			glfwGetCursorPos(window, &baseX, &baseY);
			baseTrans = scenePtr->camera()->getTranslation();
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
		isZooming = false;
	}
}

/// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window. 
void windowSizeCallback(GLFWwindow* windowPtr, int width, int height) {
	scenePtr->camera()->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
#ifdef RASTERIZER
	rasterizerPtr->setResolution(width, height);
	rayTracerPtr->setResolution(width, height);
#endif // RASTERIZER

#ifdef DEFFERED
	DefferedPtr->setResolution(width, height);
#endif // !RASTERIZER
	
}

void initGLFW() {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit()) {
		Console::print("ERROR: Failed to init GLFW");
		std::exit(EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 16); // Enable OpenGL multisampling (MSAA)

	// Create the window
	windowPtr = glfwCreateWindow(1024, 768, BASE_WINDOW_TITLE.c_str(), nullptr, nullptr);
	if (!windowPtr) {
		Console::print("ERROR: Failed to open window");
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent(windowPtr);

	// Connect the callbacks for interactive control 
	glfwSetWindowSizeCallback(windowPtr, windowSizeCallback);
	glfwSetKeyCallback(windowPtr, keyCallback);
	glfwSetCursorPosCallback(windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback(windowPtr, mouseButtonCallback);
}


void initScene() {
	scenePtr = std::make_shared<Scene>();
	scenePtr->setBackgroundColor(glm::vec3(0.1f, 0.5f, 0.95f));

	// Mesh
	auto meshPtr = std::make_shared<Mesh>();
	try {
		MeshLoader::loadOFF(meshFilename, meshPtr);
	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading mesh]") + e.what());
	}
	


	
	
	meshPtr->computeBoundingSphere(center, meshScale);
	
#ifdef DEFFERED
	
	
	std::string relativePathMesh = basePath + MODELS_PATH;
	std::string p = relativePathMesh + "plane.off";


	std::srand(10);
	
	//adding lights for SSDO & DefferedRenderer
	auto light1 = std::make_shared<LightSource>(LightSource(glm::vec3(0.7, -0.7, -1.0), glm::vec3(1.0, 1.0, 1.0), 10.f));
	scenePtr->add(light1);
	
	//adding light for deffered shading to show the use of such a technique
	for (unsigned int i = 0; i <30; i++)
	{

		float xPos = static_cast<float>(((std::rand() % 100) / 100.0) * 10.0 - 10.0);
		float yPos = static_cast<float>(((std::rand() % 100) / 100.0) *10.0 - 5.0);
		float zPos = static_cast<float>(((std::rand() % 100) / 100.0) * 10.0 - 5.0);


		float rColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5);
		float gColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5);
		float bColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5);


		auto light = std::make_shared<LightSource>(LightSource(glm::vec3(xPos, yPos, zPos), glm::vec3(rColor, gColor, bColor), 10.f));
		scenePtr->add(light);
	}
	
	
	meshPtr->setTranslation(glm::vec3(0, -0.6,0));
	scenePtr->add(meshPtr);

	auto meshPtr1 = std::make_shared<Mesh>();
	MeshLoader::loadOFF(meshFilename, meshPtr1);
	meshPtr1->setTranslation(glm::vec3(0.0, -0.6, 2.5));
	scenePtr->add(meshPtr1);
	std::string dragon = relativePathMesh + "dragon.off";
	
	auto meshPtr4 = std::make_shared<Mesh>();
	MeshLoader::loadOFF(dragon, meshPtr4);
	meshPtr4->setTranslation(glm::vec3(0.0, 0.0, -0.35));
	meshPtr4->setScale(meshScale * 3);
	scenePtr->add(meshPtr4);
	
	auto meshPtr2 = std::make_shared<Mesh>();
	MeshLoader::loadOFF(meshFilename, meshPtr2);
	meshPtr2->setTranslation(glm::vec3(2.5, -0.6, 0.5));
	scenePtr->add(meshPtr2);

	auto meshPtr3 = std::make_shared<Mesh>();
	MeshLoader::loadOFF(meshFilename, meshPtr3);
	meshPtr3->setTranslation(glm::vec3(-2.5, -0.6, 0.5));
	scenePtr->add(meshPtr3);


	
	
	auto material2 = std::make_shared<Material>(Material(glm::vec3(1.0, 0.0, 0.0), 64.f, 0.4f, 0.5f, 0.15f));
	auto material1 = std::make_shared<Material>(Material(glm::vec3(1.0, 1.0, 1.0), 64.f, 0.4f, 0.5f, 0.15f));
	auto meshGround = std::make_shared<Mesh>();
	auto meshWall = std::make_shared<Mesh>();

	meshGround->setMaterial(material1);
	meshWall->setMaterial(material2);
	

	

	//std::string s = basePath + "/Resources/Models/plane.off";
	MeshLoader::loadOFF(p, meshGround);
	MeshLoader::loadOFF(p, meshWall);
	//WALL
	meshWall->setScale(meshScale * 2);
	meshWall->setTranslation(vec3(0.0, 0.0, -1.0));
	scenePtr->add(meshWall);
	scenePtr->add(material1);
	//Ground
	meshGround->setTranslation(vec3(0.0, 0.0, -0.5 ));
	meshGround->setRotation(vec3(glm::radians(-90.f), 0.0, glm::radians(180.f)));
	meshGround->setScale(meshScale * 2);
	scenePtr->add(meshGround);
	scenePtr->add(material2);




#endif // !DEFFERED

	
#ifdef RASTERIZER
	
	auto light1 = std::make_shared<LightSource>(LightSource(glm::vec3(0.0, -0.7, -0.7), glm::vec3(1.0, 1.0, 1.0), 10.f));
	scenePtr->add(light1);


	auto light2 = std::make_shared<LightSource>(LightSource(glm::vec3(0.0, -1.0, 0.0), glm::vec3(1.0, 0.0, 0.0), 10.f));
	scenePtr->add(light2);
	
	scenePtr->add(meshPtr);
	//adding materila to the mesh
	auto material1 = std::make_shared<Material>(Material(glm::vec3(0.7, 0.7, 0.7), 64.f, 0.4f, 0.5f, 0.15f));
	scenePtr->add(material1);

	auto meshGround = std::make_shared<Mesh>();
	auto meshWall = std::make_shared<Mesh>();

	std::string s = basePath + "/../../Resources/Models/plane.off";
	//std::string s = basePath + "/Resources/Models/plane.off";




	MeshLoader::loadOFF(s, meshGround);
	MeshLoader::loadOFF(s, meshWall);

	//WALL
	meshWall->setScale(meshScale * 2);
	meshWall->setTranslation(vec3(0.0, 0.5, -1.0));
	scenePtr->add(meshWall);
	scenePtr->add(material1);

	//meshWall->setTranslation(vec3(-1.0, 0.0, 0.0));
	//meshWall


	//Ground
	auto material2 = std::make_shared<Material>(Material(glm::vec3(0.5, 0.5, 0.5), 64.f, 0.4f, 0.5f, 0.15f));
	meshGround->setTranslation(vec3(0.0, 0.0, -0.5));
	meshGround->setRotation(vec3(glm::radians(-90.f), 0.0, 0.0));
	meshGround->setScale(meshScale * 2);
	scenePtr->add(meshGround);
	scenePtr->add(material2);

#endif // RASTERIZER




	// Camera
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
	auto cameraPtr = std::make_shared<Camera>();
	cameraPtr->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	cameraPtr->setTranslation(center + glm::vec3(0.0, 0.0, 6.0 * meshScale));
	cameraPtr->setNear(0.1f);
	cameraPtr->setFar(100.f * meshScale);
	scenePtr->set(cameraPtr);
}

void init() {
	initGLFW(); // Windowing system
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // Load extensions for modern OpenGL
		exitOnCriticalError("[Failed to initialize OpenGL context]");
	initScene(); // Actual scene to render
#ifdef RASTERIZER
	rasterizerPtr = make_shared<Rasterizer>();
	rasterizerPtr->init(basePath, scenePtr); // Mut be called before creating the scene, to generate an OpenGL context and allow mesh VBOs
	rayTracerPtr = make_shared<RayTracer>();
	rayTracerPtr->init(scenePtr);
#endif // RASTERIZER



#ifdef DEFFERED
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
	DefferedPtr = make_shared<DefferedRenderer>();
	DefferedPtr->setResolution(width, height);
	DefferedPtr->init(basePath, scenePtr);
#endif // DEFFERED


}

void clear() {
	glfwDestroyWindow(windowPtr);
	glfwTerminate();
}


// The main rendering call
void render() {
#ifdef RASTERIZER
	if (isDisplayRaytracing) {
		//rasterizerPtr->display(rayTracerPtr->image()); //the rasterizer is displaying what is calculated by the cpu
	}
	else
		rasterizerPtr->render(scenePtr);
#endif // RASTERIZER




#ifdef DEFFERED
	DefferedPtr->render(scenePtr);
#endif // DEFFERED




	
}

// Update any accessible variable based on the current time
void update(float currentTime) {
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	static float lastTime = 0.f;
	static unsigned int frameCount = 0;
	static float fpsTime = currentTime;
	static unsigned int FPS = 0;
	float elapsedTime = currentTime - initialTime;
	float dt = currentTime - lastTime;
	if (frameCount == 99) {
		float delai = (currentTime - fpsTime) / 100;
		FPS = static_cast<unsigned int> (1.f / delai);
		frameCount = 0;
		fpsTime = currentTime;
	}
	std::string titleWithFPS = BASE_WINDOW_TITLE + " - " + std::to_string(FPS) + "FPS";
	glfwSetWindowTitle(windowPtr, titleWithFPS.c_str());
	lastTime = currentTime;
	frameCount++;
}

void usage(const char* command) {
	Console::print("Usage : " + std::string(command) + " [<meshfile.off>]");
	std::exit(EXIT_FAILURE);
}

void parseCommandLine(int argc, char** argv) {
	if (argc > 3)
		usage(argv[0]);
	fs::path appPath = argv[0];
	basePath = appPath.parent_path().string();
	meshFilename = basePath + "/" + (argc >= 2 ? argv[1] : DEFAULT_MESH_FILENAME);
}

int main(int argc, char** argv) {
	parseCommandLine(argc, argv);
	init();

	while (!glfwWindowShouldClose(windowPtr)) {
		update(static_cast<float> (glfwGetTime()));
		render();
		glfwSwapBuffers(windowPtr); //read the buffer of the gpu
		glfwPollEvents(); //windows events
	}
	clear();
	Console::print("Quit");
	return EXIT_SUCCESS;
}

