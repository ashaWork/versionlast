/* Start Header ************************************************************************/
/*!
\file        renderer.h
\author      Seow Sin Le, s.sinle, 2401084
\par         s.sinle@digipen.edu
\date        October, 1st, 2025
\brief       initializes the meshes and camera

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <shader.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <map>
#include <input.h>

#include "ResourceManager.h"

/*!***********************************************************************
\brief
	enum class for shapes (for both render and collision components)

*************************************************************************/
enum class shape
{
	square = 0,
	circle,
	triangle,
};

/*!***********************************************************************
\brief
	enum class for camera modes (one for gameplay, one for editor)

*************************************************************************/
enum class cameraMode {
	GAME = 0,
	EDITOR
};

struct BatchKey {
	shape meshType;
	GLuint texID;

	bool operator==(const BatchKey& other) const noexcept {
		return meshType == other.meshType && texID == other.texID;
	}
};

struct BatchKeyHash {
	std::size_t operator()(const BatchKey& k) const noexcept {
		// simple hash combine
		return std::hash<int>()((int)k.meshType) ^ (std::hash<GLuint>()(k.texID) << 1);
	}
};

/*!***********************************************************************
\brief
	struct for everything to do with rendering

	for(pair(batch, std::vector<instancedata>) pair : objectWithtex)
		render
	for(pair(batch, std::vector<instancedata>) pair : objectWithouttex)
		render

	for each batch key
		

*************************************************************************/
struct renderer
{
	/*!***********************************************************************
	\brief
		Data layout for one instance (transform, color, texture offset/scale)
	*************************************************************************/
	struct InstanceData
	{
		glm::mat4 model;      
		glm::vec4 color;      
		glm::vec4 texParams;  
	};
	/*!***********************************************************************
	\brief
		initializes the setups for the meshes and shader programs

	\param[in] w
		width of screen

	\param[in] h
		height of screen

	*************************************************************************/
	static void init(int w, int h);

	// Update camera aspect when window size changes
	static void onResize(int w, int h);

	/*!***********************************************************************
	\brief
		struct for everything to do with meshes

	*************************************************************************/
	struct model
	{
		shape shape;
		GLenum primitive_type;
		GLuint vaoid;
		GLuint vbo;
		GLuint ebo;
		GLuint elem_cnt;
		GLuint draw_cnt;
		GLuint primitive_cnt;

		GLuint instanceVBO = 0;
		GLsizei maxInstances = 5000; 

		model() : shape(shape::square), primitive_type(GL_TRIANGLES), vaoid(0), vbo(0), ebo(0), elem_cnt(0), draw_cnt(0), primitive_cnt(0) {}
	};
	/*!***********************************************************************
	\brief
		Sets up instancing attributes for a given model

	\param[in] mdl
		the mesh/model that is being drawn
	*************************************************************************/
	static void setup_instance_attributes(model& mdl);

	/*!***********************************************************************
	\brief
		Draws multiple instances of a model in one call

	\param[in]
		the mesh/model that is being drawn

	\param[in] instances
		relevent data to show how i should draw my mesh
	*************************************************************************/
	static void drawInstances(model& mdl, const std::vector<InstanceData>& instances);
	/*!***********************************************************************
	\brief
		storing of shader program
	*************************************************************************/
	static std::vector<GLuint> shdr_pgm;
	/*!***********************************************************************
	\brief
		setting up square mesh

	\param[in] clr
		taking in RGB colors

	\return model
		returns a mesh

	*************************************************************************/
	static model setup_square(glm::vec3 clr);
	/*!***********************************************************************
	\brief
		setting up circle mesh

	\param[in] clr
		taking in RGB colors

	\param[in] slices
		the amount of slices/amount of triangles in the circle

	*************************************************************************/
	static model setup_circle(glm::vec3 clr, int slices);
	/*!***********************************************************************
	\brief
		loading of shader programs into the system

	*************************************************************************/
	static void setup_shdrpgm();//const glm::mat4& p_modelMat);

	static void cleanup();

	/*!***********************************************************************
	\brief
		array of all the available meshes

	*************************************************************************/
	static std::vector<model>models;

	/*!***********************************************************************
	\brief
		struct that contains all the data for a camera

	*************************************************************************/
	struct camera
	{
		GLfloat ar;
		int width;
		glm::vec3 campos;
		float zoom;
		float rot;
		glm::vec3 up;
		glm::mat4 view;
		glm::mat4 proj;
		cameraMode mode; // camera mode: game or editor
		float zoomFactor;
		float minZoom;
		float maxZoom;
		/*!***********************************************************************
		\brief
			initializes all the data for the camera

		\param[in] w
			width of the screen

		\param[in] h
			height of the screen

		*************************************************************************/
		void init(int w, int h);
		/*!***********************************************************************
		\brief
			updating view and orthographic projection matrix

		*************************************************************************/
		void update();
		/*!***********************************************************************
		\brief
			move camera based on given delta

		\param[in] delta
			how much to move

		*************************************************************************/
		void pan(const Vector2D& delta);
		/*!***********************************************************************
		\brief
			zoom in/out based on scroll delta

		\param[in] scrollDelta
			how much to zoom in/out (scroll)
		*************************************************************************/
		void zoomInOut(float scrollDelta);

	};

	/*!***********************************************************************
	\brief
		global camera

	*************************************************************************/
	static camera cam;
	static camera editorCam;
};