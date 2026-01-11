/* Start Header ************************************************************************/
/*!
\file        shader.hpp
\author      Seow Sin Le, s.sinle, 2401084
\par         s.sinle@digipen.edu
\date        October, 1st, 2025
\brief       Enable shader files for graphics

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once

#include <GL/glew.h>
#include <string>
#define _GET_GL_ERROR { GLenum err = glGetError(); std::cout << "[OpenGL Error] " << glewGetErrorString(err) << std::endl; }

/**
 * @brief Loads and compiles vertex and fragment shaders from given file paths, links them into a shader program.
 *
 * @param vertex_file_path Path to the vertex shader source file.
 * @param fragment_file_path Path to the fragment shader source file.
 * @return GLuint ID of the created shader program.
 */
GLuint LoadShaders(const std::string& p_vertexShaderCode, const std::string& p_fragmentShaderCode, bool p_loadFromFile);

/**
 * @brief Loads separable vertex and fragment shaders and sets up a program pipeline.
 *
 * @param vertex_file_path Path to the vertex shader source file.
 * @param fragment_file_path Path to the fragment shader source file.
 * @param programIDs Output array to store vertex and fragment shader program IDs (size must be 2).
 * @return GLuint ID of the OpenGL pipeline object.
 */
GLuint LoadPipeline(const char* vertex_file_path, const char* fragment_file_path, GLuint* programIDs);