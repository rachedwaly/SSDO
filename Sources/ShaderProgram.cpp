// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "ShaderProgram.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <exception>
#include <ios>

using namespace std;

// Create a GPU program i.e., a graphics pipeline
ShaderProgram::ShaderProgram () : m_id (glCreateProgram ()) {}


ShaderProgram::~ShaderProgram () {
	glDeleteProgram (m_id); 
}

std::string ShaderProgram::file2String (const std::string & filename) {
	std::ifstream input (filename.c_str ());
	if (!input)
		throw std::ios_base::failure ("[Shader Program][file2String] Error: cannot open " + filename);
	std::stringstream buffer;
	buffer << input.rdbuf ();
	return buffer.str ();
}

void ShaderProgram::loadShader (GLenum type, const std::string & shaderFilename) {
	GLuint shader = glCreateShader (type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
	std::string shaderSourceString = file2String (shaderFilename); // Loads the shader source from a file to a C++ string
	const GLchar * shaderSource = (const GLchar *)shaderSourceString.c_str (); // Interface the C++ string through a C pointer
	glShaderSource (shader, 1, &shaderSource, NULL); // Load the vertex shader source code
	int  success;
	char infoLog[512];

	glCompileShader (shader);  // THe GPU driver compile the shader
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	glAttachShader (m_id, shader); // Set the vertex shader as the one ot be used with the program/pipeline
	glDeleteShader (shader);
}

std::shared_ptr<ShaderProgram> ShaderProgram::genBasicShaderProgram (const std::string & vertexShaderFilename,
															 	 	 const std::string & fragmentShaderFilename) {
	std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram> ();
	shaderProgramPtr->loadShader (GL_VERTEX_SHADER, vertexShaderFilename);
	shaderProgramPtr->loadShader (GL_FRAGMENT_SHADER, fragmentShaderFilename);
	shaderProgramPtr->link ();
	return shaderProgramPtr;
}