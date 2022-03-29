
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "SSAO.h"
#include <glad/glad.h>
#include "Resources.h"
#include "Error.h"

void DefferedRenderer::init(const std::string& basePath, const std::shared_ptr<Scene> scenePtr) {
	glEnable(GL_DEBUG_OUTPUT); // Modern error callback functionnality
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // For recovering the line where the error occurs, set a debugger breakpoint in DebugMessageCallback
	glDebugMessageCallback(debugMessageCallback, 0); // Specifies the function to call when an error message is generated.
	glEnable(GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
							 
										  // GPU resources
	
	loadShaderProgram(basePath);
	loadTextures(basePath);
	genGBuffer();
	
	size_t numOfMeshes = scenePtr->numOfMeshes();
	for (size_t i = 0; i < numOfMeshes; i++)
		m_vaos.push_back(toGPU(scenePtr->mesh(i)));
	
}
void DefferedRenderer::setResolution(int width, int height) {
	glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
	this->widthW = width;
	this->heightW = height;
}

void DefferedRenderer::loadShaderProgram(const std::string& basePath) {
	m_geometryShaderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_geometryShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "SSAO_g_bufferVS.glsl",
			shaderPath + "SSAO_g_bufferFS.glsl");
	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading shader program]") + e.what());
	}
	
	m_LShaderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_LShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "defferedVS.glsl",
			shaderPath + "LightingSSAOFS.glsl");
		
		m_LShaderProgramPtr->set("gPosition", 0);
		m_LShaderProgramPtr->set("gNormal", 1);
		m_LShaderProgramPtr->set("gAlbedoRoughness", 2);
		m_LShaderProgramPtr->set("gNormalGlobal", 3);
		m_LShaderProgramPtr->set("ssao", 4);

	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}

	m_ssaoShaderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_ssaoShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "defferedVS.glsl",
			shaderPath + "SSAO_FS.glsl");
			
		m_ssaoShaderProgramPtr->set("gPosition", 0);
		m_ssaoShaderProgramPtr->set("gNormal", 1);
		m_ssaoShaderProgramPtr->set("texNoise", 2);
	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}

	m_ssaoBlurShaderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_ssaoBlurShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "defferedVS.glsl",
			shaderPath + "SSAO_BLURFS.glsl");
			
		
		m_ssaoBlurShaderProgramPtr->set("ssao", 0);
	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}

	m_ssdoBlurShaderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_ssdoBlurShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "defferedVS.glsl",
			shaderPath + "SSDO_BLURFS.glsl");
			
		m_ssdoBlurShaderProgramPtr->set("Li", 0);
		m_ssdoBlurShaderProgramPtr->set("lightPass", 1);

		
	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}

	m_indirectLShaderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_indirectLShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "defferedVS.glsl",
			shaderPath + "indirectLSSDOFS.glsl");
			
		m_indirectLShaderProgramPtr->set("gPosition", 0);
		m_indirectLShaderProgramPtr->set("gNormal", 1);
		m_indirectLShaderProgramPtr->set("texNoise", 2);
		m_indirectLShaderProgramPtr->set("lightPass", 3);
	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}

	m_directLShaderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_directLShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "defferedVS.glsl",
			shaderPath + "directLSSDOFS.glsl");
			
		m_directLShaderProgramPtr->set("gPosition", 0);
		m_directLShaderProgramPtr->set("gNormal", 1);
		m_directLShaderProgramPtr->set("gAlbedoRoughness", 2);
		m_directLShaderProgramPtr->set("gNormalGlobal", 3);

	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}
	
	m_displayDefferedSahderProgramPtr.reset();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_displayDefferedSahderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "defferedVS.glsl",
			shaderPath + "displayDefferedFS.glsl");
			
		m_displayDefferedSahderProgramPtr->set("gPosition", 0);
		m_displayDefferedSahderProgramPtr->set("gNormal", 1);
		m_displayDefferedSahderProgramPtr->set("gAlbedoRoughness", 2);
	}
	catch (std::exception& e) {
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}
}


float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}
void DefferedRenderer::genGBuffer() {
	
	glGenFramebuffers(1, &gBuffer);
	//bind the gbuffer to modify it
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	//position as texture buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, widthW, heightW, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	//normal as texture buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, widthW, heightW, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);


	//roughness and shininess as texture buffer
	glGenTextures(1, &gAlbedoRoughness);
	glBindTexture(GL_TEXTURE_2D, gAlbedoRoughness);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthW, heightW, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoRoughness, 0);


	//normalGlobal as texture buffer
	glGenTextures(1, &gNormalGlobal);
	glBindTexture(GL_TEXTURE_2D, gNormalGlobal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, widthW, heightW, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gNormalGlobal, 0);

	//attaching textures output
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4,attachments);

	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, widthW, heightW);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);



	
	
	//unbind frameBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	glGenFramebuffers(1, &ssaoFBO); 
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	
	// SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthW, heightW, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	
	//SSAO BLUR buffer
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	
	
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthW, heightW, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &directLightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, directLightFBO);

	// Direct pass of light color buffer
	glGenTextures(1, &directLColorBuffer);
	glBindTexture(GL_TEXTURE_2D, directLColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthW, heightW, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, directLColorBuffer, 0);


	glGenFramebuffers(1, &indirectLightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, indirectLightFBO);

	// indrect pass of light color buffer
	glGenTextures(1, &indirectLColorBuffer);
	glBindTexture(GL_TEXTURE_2D, indirectLColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthW, heightW, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, indirectLColorBuffer, 0);


	
	kernelSize = 128;
	for (unsigned int i = 0; i < kernelSize; ++i)
	{
		float r1 = ((float)rand() / (RAND_MAX));
		float r2 = ((float)rand() / (RAND_MAX));
		float r3 = ((float)rand() / (RAND_MAX));
		float r4 = ((float)rand() / (RAND_MAX));
		glm::vec3 sample(r1 * 2.0 - 1.0,r2 * 2.0 - 1.0, r3);
		sample = glm::normalize(sample);
		sample *= r4;
		float scale = float(i) / kernelSize;

		//scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// generate noise texture
	// ----------------------

	for (unsigned int i = 0; i < 16; i++)
	{
		float r1 = ((float)rand() / (RAND_MAX));
		float r2 = ((float)rand() / (RAND_MAX));
		glm::vec3 noise(r1*2.0-1.0 , r2*2.0-1.0 , 0.0f); 
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

void DefferedRenderer::GeometryPass(std::shared_ptr<Scene> scenePtr) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	size_t numOfMeshes = scenePtr->numOfMeshes();



	//bind the gBuffer 
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	//clearing the gBuffer 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	//assigning textures uniforms
	glActiveTexture(GL_TEXTURE0);
	m_geometryShaderProgramPtr->set("textureAlbedo", 0);
	glBindTexture(GL_TEXTURE_2D, textureAlbedo);
	glActiveTexture(GL_TEXTURE1);
	m_geometryShaderProgramPtr->set("textureRoughness", 1);
	glBindTexture(GL_TEXTURE_2D, textureRoughness);
	glActiveTexture(GL_TEXTURE2);
	m_geometryShaderProgramPtr->set("textureAlbedo1", 2);
	glBindTexture(GL_TEXTURE_2D, textureAlbedo1);
	glActiveTexture(GL_TEXTURE3);
	m_geometryShaderProgramPtr->set("textureRoughness1", 3);
	glBindTexture(GL_TEXTURE_2D, textureRoughness1);
	glActiveTexture(GL_TEXTURE3);
	m_geometryShaderProgramPtr->set("textureAlbedo2", 3);
	glBindTexture(GL_TEXTURE_2D, textureAlbedo2);
	glActiveTexture(GL_TEXTURE4);
	m_geometryShaderProgramPtr->set("textureRoughness2", 4);
	glBindTexture(GL_TEXTURE_2D, textureRoughness2);




	int s = 0;
	for (size_t i = 0; i < numOfMeshes; i++) {
		std::shared_ptr<Mesh> meshi = scenePtr->mesh(i);
		bool b = meshi->hasTexture();
		m_geometryShaderProgramPtr->set("s", s);
		s = (s + 1) % 3;
		m_geometryShaderProgramPtr->set("hasTexture", b);
		if (!b) {
			m_geometryShaderProgramPtr->set("material.albedo", meshi->getMaterial()->albedo);
			m_geometryShaderProgramPtr->set("material.roughness", meshi->getMaterial()->roughness);
		}
		
		glm::mat4 projectionMatrix = scenePtr->camera()->computeProjectionMatrix();
		m_geometryShaderProgramPtr->set("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
		glm::mat4 modelMatrix = meshi->computeTransformMatrix();
		glm::mat4 modelMatrixnormals = glm::transpose(glm::inverse(modelMatrix));
		m_geometryShaderProgramPtr->set("modelMatnormals", modelMatrixnormals);
		glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix();
		m_geometryShaderProgramPtr->set("viewMat", glm::transpose(glm::inverse(viewMatrix)));

		glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
		m_geometryShaderProgramPtr->set("modelViewMat", modelViewMatrix);
		m_geometryShaderProgramPtr->set("normalMat", normalMatrix);
		draw(i, meshi->triangleIndices().size());
	}

	m_geometryShaderProgramPtr->stop();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void DefferedRenderer::SSAOTexturePass(std::shared_ptr<Scene> scenePtr) {
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	glClear(GL_COLOR_BUFFER_BIT);
	// Send kernel + rotation 
	for (unsigned int i = 0; i < ssaoKernel.size(); ++i) {
		m_ssaoShaderProgramPtr->set("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}
	m_ssaoShaderProgramPtr->set("projection", scenePtr->camera()->computeProjectionMatrix());
	m_ssaoShaderProgramPtr->set("noiseScale", glm::vec2((float)widthW / 4.0, (float)heightW / 4.0));
	m_ssaoShaderProgramPtr->set("kernelSize", kernelSize);
	m_ssaoShaderProgramPtr->set("radius", radius);
	m_ssaoShaderProgramPtr->set("bias", bias);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);

	renderQuadToScreen();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_indirectLShaderProgramPtr->stop();
}

void DefferedRenderer::SSAOBlurPass(std::shared_ptr<Scene> scenePtr) {
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	m_ssaoBlurShaderProgramPtr->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	renderQuadToScreen();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DefferedRenderer::SSAOLightingPass(std::shared_ptr<Scene> scenePtr) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition); 
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoRoughness);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gNormalGlobal);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

	glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix();
	glm::mat4 viewMat = inverse(viewMatrix);

	m_LShaderProgramPtr->set("withSSAO", withSSFeature);
	m_LShaderProgramPtr->set("viewPos", vec3(viewMat[3]));
	m_LShaderProgramPtr->set("numOfLights", (int)scenePtr->numOfLights());

	for (int i = 0; i <1; i++) {
		m_LShaderProgramPtr->set("lights[" + std::to_string(i) + "].direction", scenePtr->light(i)->getDirection());
		m_LShaderProgramPtr->set("lights[" + std::to_string(i) + "].color", scenePtr->light(i)->getColor());
		m_LShaderProgramPtr->set("lights[" + std::to_string(i) + "].intensity", scenePtr->light(i)->getIntensity());
	}

	renderQuadToScreen();

	m_LShaderProgramPtr->stop();
}

void DefferedRenderer::SSDOLightingPass(std::shared_ptr<Scene> scenePtr)
{
	glBindFramebuffer(GL_FRAMEBUFFER, directLightFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition); //define gPosition in header
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);	//define gNormal in header
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoRoughness);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gNormalGlobal);


	glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix();
	glm::mat4 viewMat = inverse(viewMatrix);
	m_directLShaderProgramPtr->set("viewPos", vec3(viewMat[3]));
	m_directLShaderProgramPtr->set("numOfLights", (int)scenePtr->numOfLights());
	for (int i = 0; i < 1; i++) {
		m_directLShaderProgramPtr->set("lights[" + std::to_string(i) + "].direction", scenePtr->light(i)->getDirection());
		m_directLShaderProgramPtr->set("lights[" + std::to_string(i) + "].color", scenePtr->light(i)->getColor());
		m_directLShaderProgramPtr->set("lights[" + std::to_string(i) + "].intensity", scenePtr->light(i)->getIntensity());
	}

	renderQuadToScreen();

	m_directLShaderProgramPtr->stop();
}

void DefferedRenderer::SSDO(std::shared_ptr<Scene> scenePtr)
{
	glBindFramebuffer(GL_FRAMEBUFFER, indirectLightFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	// Send kernel + rotation 
	for (unsigned int i = 0; i < ssaoKernel.size(); ++i) {
		m_indirectLShaderProgramPtr->set("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}
	m_indirectLShaderProgramPtr->set("projection", scenePtr->camera()->computeProjectionMatrix());
	m_indirectLShaderProgramPtr->set("noiseScale", glm::vec2((float)widthW / 4.0, (float)heightW / 4.0));
	m_indirectLShaderProgramPtr->set("kernelSize", kernelSize);
	m_indirectLShaderProgramPtr->set("radius", radius);
	m_indirectLShaderProgramPtr->set("bias", bias);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, directLColorBuffer);

	renderQuadToScreen();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_indirectLShaderProgramPtr->stop();
}


void DefferedRenderer::defferedLightingPass(std::shared_ptr<Scene> scenePtr) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition); 
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalGlobal);	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoRoughness);

	glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix();
	glm::mat4 viewMat = inverse(viewMatrix);
	m_displayDefferedSahderProgramPtr->set("viewPos", vec3(viewMat[3]));
	m_displayDefferedSahderProgramPtr->set("viewMat", glm::transpose(glm::inverse(viewMatrix)));//to remove

	m_displayDefferedSahderProgramPtr->set("numOfLights", (int)scenePtr->numOfLights());

	for (int i = 0; i < scenePtr->numOfLights(); i++) {
		m_displayDefferedSahderProgramPtr->set("lights[" + std::to_string(i) + "].direction", scenePtr->light(i)->getDirection());
		m_displayDefferedSahderProgramPtr->set("lights[" + std::to_string(i) + "].color", scenePtr->light(i)->getColor());
		m_displayDefferedSahderProgramPtr->set("lights[" + std::to_string(i) + "].intensity", scenePtr->light(i)->getIntensity());
	}



	renderQuadToScreen();

	m_displayDefferedSahderProgramPtr->stop();
}
void DefferedRenderer::SSDOBlurPass(std::shared_ptr<Scene> scenePtr)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	m_ssdoBlurShaderProgramPtr->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, indirectLColorBuffer);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, directLColorBuffer);
	m_ssdoBlurShaderProgramPtr->set("withSSDO", withSSFeature);
	renderQuadToScreen();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void DefferedRenderer::render(std::shared_ptr<Scene> scenePtr) {
	if (SSAOactive) {
		GeometryPass(scenePtr);
		SSAOTexturePass(scenePtr);
		SSAOBlurPass(scenePtr);
		SSAOLightingPass(scenePtr);
	}

	if (SSDOactive) {
		GeometryPass(scenePtr);
		SSDOLightingPass(scenePtr);
		SSDO(scenePtr);
		SSDOBlurPass(scenePtr);
	}

	if (defferedActive) {
		GeometryPass(scenePtr);
		defferedLightingPass(scenePtr);

	}
	if (showOnlySSAOTexture) {
		GeometryPass(scenePtr);
		renderSSAOTexture(scenePtr);
	}
	
}

void DefferedRenderer::renderSSAOTexture(std::shared_ptr<Scene> scenePtr) {
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	// Send kernel + rotation 
	for (unsigned int i = 0; i < ssaoKernel.size(); ++i) {
		m_ssaoShaderProgramPtr->set("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}
	m_ssaoShaderProgramPtr->set("projection", scenePtr->camera()->computeProjectionMatrix());
	m_ssaoShaderProgramPtr->set("noiseScale", glm::vec2((float)widthW / 4.0, (float)heightW / 4.0));
	m_ssaoShaderProgramPtr->set("kernelSize", kernelSize);
	m_ssaoShaderProgramPtr->set("radius", radius);
	m_ssaoShaderProgramPtr->set("bias", bias);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	renderQuadToScreen();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	m_ssaoBlurShaderProgramPtr->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	renderQuadToScreen();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DefferedRenderer::loadTextures(const std::string& basePath) {
	std::string texturePath = basePath + "/" + DEFAULT_MATERIAL_DIRNAME + "Chesterfield/Base_Color.png";
	textureAlbedo = loadTextureFromFileToGPU(texturePath);
	texturePath= basePath + "/" + DEFAULT_MATERIAL_DIRNAME + "Chesterfield/Roughness.png";
	textureRoughness = loadTextureFromFileToGPU(texturePath,1);

	texturePath = basePath + "/" + DEFAULT_MATERIAL_DIRNAME + "Old_Heavy_Victorian_Handmade/Base_Color.png";
	textureAlbedo1 = loadTextureFromFileToGPU(texturePath);
	texturePath = basePath + "/" + DEFAULT_MATERIAL_DIRNAME + "Old_Heavy_Victorian_Handmade/Roughness.png";
	textureRoughness1 = loadTextureFromFileToGPU(texturePath, 1);

	texturePath = basePath + "/" + DEFAULT_MATERIAL_DIRNAME + "Sci_Fi_Storage_Unit_Wall/Base_Color.png";
	textureAlbedo2 = loadTextureFromFileToGPU(texturePath);
	texturePath = basePath + "/" + DEFAULT_MATERIAL_DIRNAME + "Sci_Fi_Storage_Unit_Wall/Roughness.png";
	textureRoughness2 = loadTextureFromFileToGPU(texturePath, 1);
}



void DefferedRenderer::renderQuadToScreen() {
	if (VAO == 0) {
		float vertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //used drawArrays because of the nature of data, stacked positions + text coordinates
	glBindVertexArray(0);

}

void DefferedRenderer::draw(size_t meshId, size_t triangleCount) {
	glBindVertexArray(m_vaos[meshId]); // Activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei> (triangleCount * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}

GLuint DefferedRenderer::genGPUBuffer(size_t elementSize, size_t numElements, const void* data) {
	GLuint vbo;
	glCreateBuffers(1, &vbo); // Generate a GPU buffer to store the positions of the vertices
	size_t size = elementSize * numElements; // Gather the size of the buffer from the CPU-side vector
	glNamedBufferStorage(vbo, size, NULL, GL_DYNAMIC_STORAGE_BIT); // Create a data store on the GPU
	glNamedBufferSubData(vbo, 0, size, data); // Fill the data store from a CPU array
	return vbo;
}

GLuint DefferedRenderer::genGPUVertexArray(GLuint posVbo, GLuint ibo,GLuint textVbo, bool hasNormals, GLuint normalVbo) {
	GLuint vao;
	glCreateVertexArrays(1, &vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	if (hasNormals) {
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	}
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, textVbo);
	glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0); // Desactive the VAO just created. Will be activated at rendering time.
	return vao;
}


GLuint DefferedRenderer::toGPU(std::shared_ptr<Mesh> meshPtr) {
	GLuint posVbo = genGPUBuffer(3 * sizeof(float), meshPtr->vertexPositions().size(), meshPtr->vertexPositions().data()); // Position GPU vertex buffer
	GLuint normalVbo = genGPUBuffer(3 * sizeof(float), meshPtr->vertexNormals().size(), meshPtr->vertexNormals().data()); // Normal GPU vertex buffer
	GLuint ibo = genGPUBuffer(sizeof(glm::uvec3), meshPtr->triangleIndices().size(), meshPtr->triangleIndices().data()); // triangle GPU index buffer
	GLuint textVbo= genGPUBuffer(2*sizeof(float), meshPtr->textCoords().size(), meshPtr->textCoords().data());
	
	m_posVbos.push_back(posVbo);
	m_normalVbos.push_back(normalVbo);
	m_ibos.push_back(ibo);
	m_textVbos.push_back(textVbo);


	GLuint vao = genGPUVertexArray(posVbo, ibo,textVbo, true, normalVbo);
	return vao;
}

void DefferedRenderer::clear() {
	for (unsigned int i = 0; i < m_posVbos.size(); i++) {
		GLuint vbo = m_posVbos[i];
		glDeleteBuffers(1, &vbo);
	}
	m_posVbos.clear();
	for (unsigned int i = 0; i < m_normalVbos.size(); i++) {
		GLuint vbo = m_normalVbos[i];
		glDeleteBuffers(1, &vbo);
	}
	m_normalVbos.clear();
	for (unsigned int i = 0; i < m_ibos.size(); i++) {
		GLuint ibo = m_ibos[i];
		glDeleteBuffers(1, &ibo);
	}
	m_ibos.clear();
	for (unsigned int i = 0; i < m_textVbos.size(); i++) {
		GLuint textVbo = m_textVbos[i];
		glDeleteBuffers(1, &textVbo);
	}
	m_ibos.clear();
	for (unsigned int i = 0; i < m_vaos.size(); i++) {
		GLuint vao = m_vaos[i];
		glDeleteVertexArrays(1, &vao);
	}
	m_vaos.clear();
}

GLuint DefferedRenderer::loadTextureFromFileToGPU(const std::string& filename,int type) {
	int width, height, numComponents;
	// Loading the image in CPU memory using stbd_image
	unsigned char* data=nullptr;
	float* dataf=nullptr;
	
	if (type == 0) {
		data = stbi_load(filename.c_str(),
			&width,
			&height,
			&numComponents, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
			0);
	}
	else {
		dataf = stbi_loadf(filename.c_str(),
			&width,
			&height,
			&numComponents, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
			0);
	}
	// Create a texture in GPU memory
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Uploading the image data to GPU memory
	if (type == 0) {
		glTexImage2D(GL_TEXTURE_2D,
			0,
			(numComponents == 1 ? GL_RED : (numComponents == 3 ? GL_RGB : GL_RGBA)),
			width,
			height,
			0,
			(numComponents == 1 ? GL_RED : (numComponents == 3 ? GL_RGB : GL_RGBA)),
			GL_UNSIGNED_BYTE,
			data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D,
			0,
			(numComponents == 1 ? GL_RED : (numComponents == 3 ? GL_RGB : GL_RGBA)),
			width,
			height,
			0,
			(numComponents == 1 ? GL_RED : (numComponents == 3 ? GL_RGB : GL_RGBA)),
			GL_FLOAT,
			dataf);
	}
	// Generating mipmaps for filtered texture fetch
	glGenerateMipmap(GL_TEXTURE_2D);
	// Freeing the now useless CPU memory
	stbi_image_free(data);
	stbi_image_free(dataf);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;

}

