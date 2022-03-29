// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <glad/glad.h>
#include <string>

#include "Scene.h"
#include "Mesh.h"
#include "Image.h"
#include "ShaderProgram.h"

class DefferedRenderer {
public:

	inline DefferedRenderer() {}

	virtual ~DefferedRenderer() {}

	/// OpenGL context, shader pipeline initialization and GPU ressources (vertex buffers, textures, etc)
	void init(const std::string& basepath, const std::shared_ptr<Scene> scenePtr);
	void setResolution(int width, int height);
	/// Loads and compile the programmable shader pipeline
	void loadShaderProgram(const std::string& basePath);
	void render(std::shared_ptr<Scene> scenePtr);
	void renderSSAOTexture(std::shared_ptr<Scene> scenePtr);

	void clear();
	void genGBuffer();
	void loadTextures(const std::string& basePath);
	void renderQuadToScreen();
	void GeometryPass(std::shared_ptr<Scene> scenePtr);
	void SSAOTexturePass(std::shared_ptr<Scene> scenePtr);
	void SSAOBlurPass(std::shared_ptr<Scene> scenePtr);
	void SSAOLightingPass(std::shared_ptr<Scene> scenePtr);
	void SSDOLightingPass(std::shared_ptr<Scene> scenePtr);
	void SSDO(std::shared_ptr<Scene> scenePtr);
	void defferedLightingPass(std::shared_ptr<Scene> scenePtr);
	void SSDOBlurPass(std::shared_ptr<Scene> scenePtr);

	void toggleSSFeature() {
		withSSFeature = !withSSFeature;
	}

	void activateDeffered() {
		defferedActive = true;
		SSDOactive = false;
		SSAOactive = false;
		showOnlySSAOTexture = false;
	}

	void activateSSAO() {
		defferedActive = false;
		SSDOactive = false;
		SSAOactive = true;
		showOnlySSAOTexture = false;
	}

	void activateSSDO() {
		defferedActive = false;
		SSDOactive = true;
		SSAOactive = false;
		showOnlySSAOTexture = false;
	}

	void activateSSAOTexture() {
		defferedActive = false;
		SSDOactive = false;
		SSAOactive = false;
		showOnlySSAOTexture = true;
	}


	GLuint loadTextureFromFileToGPU(const std::string& filename,int type=0);

private:
	GLuint genGPUBuffer(size_t elementSize, size_t numElements, const void* data);
	GLuint genGPUVertexArray(GLuint posVbo, GLuint ibo, GLuint textVbo, bool hasNormals, GLuint normalVbo);
	GLuint toGPU(std::shared_ptr<Mesh> meshPtr);
	void draw(size_t meshId, size_t triangleCount);

	/// Pointer to GPU shader pipeline i.e., set of shaders structured in a GPU program
	std::shared_ptr<ShaderProgram> m_geometryShaderProgramPtr; // A GPU program contains at least a vertex shader and a fragment shader
	std::shared_ptr<ShaderProgram> m_LShaderProgramPtr; // Full screen quad shader program, for displaying 2D color images
	std::shared_ptr<ShaderProgram> m_directLShaderProgramPtr; // Full screen quad shader program, for displaying 2D color images
	std::shared_ptr<ShaderProgram> m_ssaoShaderProgramPtr;
	std::shared_ptr<ShaderProgram> m_ssaoBlurShaderProgramPtr;
	std::shared_ptr<ShaderProgram> m_indirectLShaderProgramPtr;
	std::shared_ptr<ShaderProgram> m_displayDefferedSahderProgramPtr;
	std::shared_ptr<ShaderProgram> m_ssdoBlurShaderProgramPtr;
	GLuint m_displayImageTex; // Texture storing the image to display in non-rasterization mode
	GLuint m_screenQuadVao;  // Full-screen quad drawn when displaying an image (no scene rasterization) 

	std::vector<GLuint> m_vaos;
	std::vector<GLuint> m_posVbos;
	std::vector<GLuint> m_normalVbos;
	std::vector<GLuint> m_ibos;
	std::vector<GLuint> m_textVbos;
	
	int widthW, heightW;
	unsigned int gBuffer;
	unsigned int rboDepth;
	unsigned int gPosition, gNormal, gAlbedoRoughness, gNormalGlobal; //texture buffers generated to fill in the geometry shader
	unsigned int textureAlbedo; //texture of roughness to pass as uniform to  the geometry shader
	unsigned int textureRoughness; //texture of roughness to pass as uniform to  the geometry shader

	unsigned int textureAlbedo1; //texture of roughness to pass as uniform to  the geometry shader
	unsigned int textureRoughness1; //texture of roughness to pass as uniform to  the geometry shader

	unsigned int textureAlbedo2; //texture of roughness to pass as uniform to  the geometry shader
	unsigned int textureRoughness2; //texture of roughness to pass as uniform to  the geometry shader
	
	unsigned int VAO=0, VBO; //

	//ssao
	unsigned int ssaoFBO, ssaoBlurFBO, directLightFBO, indirectLightFBO;
	unsigned int ssaoColorBuffer, ssaoColorBufferBlur, directLColorBuffer, indirectLColorBuffer;
	unsigned int noiseTexture;
	int kernelSize;
	std::vector<glm::vec3> ssaoKernel = {};
	std::vector<glm::vec3> ssaoNoise = {};

	bool withSSFeature = true; //with screen space feature enable ssdo or ssao depends on the mode

	bool SSAOactive = true;
	bool SSDOactive = false;
	bool defferedActive = false;
	
	bool showOnlySSAOTexture=false;

	float radius = 0.35f;
	float bias = 0.025f;

};

