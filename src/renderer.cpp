/* Start Header ************************************************************************/
/*!
\file        renderer.cpp
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
#include "renderer.h"

std::vector<renderer::model> renderer::models;
//std::map<std::string, renderer::object> renderer::objects;
std::vector<GLuint> renderer::shdr_pgm;
renderer::camera renderer::cam;
renderer::camera renderer::editorCam;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

void renderer::init(int w, int h)
{
	// set up gameplay camera
	cam.init(w, h);
	cam.mode = cameraMode::GAME;

	// set up editor camera
	editorCam.init(w, h);
	editorCam.mode = cameraMode::EDITOR;

	models.push_back(setup_square(glm::vec3(1.f, 1.f, 1.f)));
	models.push_back(setup_circle(glm::vec3(1.f, 0.5f, 0.5f), 20));
	models.push_back(setup_circle(glm::vec3(0.f, 0.f, 0.f), 20));
	setup_shdrpgm();
}

void renderer::onResize(int w, int h)
{
	// Update gameplay camera
	cam.width = w;
	cam.ar = static_cast<float>(w) / static_cast<float>(h);

	// Update editor camera too so picking is consistent there as well
	editorCam.width = w;
	editorCam.ar = cam.ar;
}


renderer::model renderer::setup_square(glm::vec3 clr)
{
	model mdl;
	std::array<glm::vec2, 4> pos_vtx{ // vertex position attributes
	glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.5f),
	glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, -0.5f)
	};

	std::array<glm::vec3, 4> clr_vtx{
	clr, clr,
	clr,clr
	};

	std::array<glm::vec2, 4> texpos{
	glm::vec2(1.f, 0.f), glm::vec2(1.f,1.f),
	glm::vec2(0.f, 1.f), glm::vec2(0.f, 0.f)
	};

	
	GLsizei position_data_offset = 0;
	GLsizei position_attribute_size = sizeof(glm::vec2);
	GLsizei position_data_size = position_attribute_size * static_cast<GLsizei>(pos_vtx.size());
	GLsizei color_data_offset = position_data_size;
	GLsizei color_attribute_size = sizeof(glm::vec3);
	GLsizei color_data_size = color_attribute_size * static_cast<GLsizei>(clr_vtx.size());
	GLsizei texture_data_offset = position_data_size + color_data_size;
	GLsizei texture_attibute_size = sizeof(glm::vec2);
	GLsizei texture_data_size = texture_attibute_size * static_cast<GLsizei>(texpos.size());

	glCreateBuffers(1, &mdl.vbo);
	glNamedBufferStorage(mdl.vbo, position_data_size + color_data_size + texture_data_size, nullptr, GL_DYNAMIC_STORAGE_BIT);                     // immutable
	glNamedBufferSubData(mdl.vbo, 0, position_data_size, pos_vtx.data());   // upload positions
	glNamedBufferSubData(mdl.vbo, color_data_offset, color_data_size, clr_vtx.data());   // upload colors
	glNamedBufferSubData(mdl.vbo, texture_data_offset, texture_data_size, texpos.data());   // upload colors

	glCreateVertexArrays(1, &mdl.vaoid);
	glEnableVertexArrayAttrib(mdl.vaoid, 0);
	glVertexArrayVertexBuffer(mdl.vaoid, 0, mdl.vbo, position_data_offset, position_attribute_size);
	glVertexArrayAttribFormat(mdl.vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 0, 0);

	glEnableVertexArrayAttrib(mdl.vaoid, 1);
	glVertexArrayVertexBuffer(mdl.vaoid, 1, mdl.vbo, color_data_offset, color_attribute_size);
	glVertexArrayAttribFormat(mdl.vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 1, 1);

	glEnableVertexArrayAttrib(mdl.vaoid, 2);
	glVertexArrayVertexBuffer(mdl.vaoid, 2, mdl.vbo, texture_data_offset, texture_attibute_size);
	glVertexArrayAttribFormat(mdl.vaoid, 2, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 2, 2);

	//---------------------------------------------------------------------------------
	//idk what im looking at, but hopefully gpt dun screw me over, to be refactored
	//---------------------------------------------------------------------------------
	// --- Setup instance buffer for per-instance attributes ---
	glGenBuffers(1, &mdl.instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mdl.instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, mdl.maxInstances * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);

	// Setup layout in VAO
	setup_instance_attributes(mdl);

	mdl.primitive_type = GL_TRIANGLES;
	std::array<GLushort, 6> idx_vtx{ 0, 1, 2, 2, 3, 0 };
	mdl.elem_cnt = static_cast<GLuint>(idx_vtx.size());
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(GLushort) * mdl.elem_cnt, reinterpret_cast<GLvoid*>(idx_vtx.data()), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(mdl.vaoid, ebo_hdl);
	mdl.ebo = ebo_hdl;
	glBindVertexArray(0);
	mdl.draw_cnt = static_cast<GLsizei>(pos_vtx.size());
	mdl.primitive_cnt = mdl.elem_cnt;
	mdl.shape = shape::square;
	setup_instance_attributes(mdl);
	return mdl;
}

renderer::model renderer::setup_circle(glm::vec3 clr, int slices)
{
	std::vector<glm::vec2> pos_vtx;
	std::vector<glm::vec3> clr_vtx;
	pos_vtx.reserve(slices + 2);
	clr_vtx.reserve(slices + 2);
	pos_vtx.emplace_back(0, 0);
	clr_vtx.emplace_back(clr);
	for (int i = 0; i < slices + 1; ++i)
	{
		float theta = glm::radians(360.0f * i / slices);
		pos_vtx.emplace_back(cos(theta), sin(theta));
		if (i == slices)
		{
			clr_vtx.emplace_back(clr_vtx[1]);
		}
		else
		{
			clr_vtx.emplace_back(clr.r, clr.g, clr.b);

		}
	}
	model mdl;
	GLsizei pos_size = static_cast<GLsizei>(sizeof(glm::vec2) * pos_vtx.size());
	GLsizei clr_size = static_cast<GLsizei>(sizeof(glm::vec3) * clr_vtx.size());
	// set up VAO as in GLApp::points_model
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, pos_size + clr_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(vbo_hdl, 0, pos_size, pos_vtx.data());
	glNamedBufferSubData(vbo_hdl, pos_size, clr_size, clr_vtx.data());

	glCreateVertexArrays(1, &mdl.vaoid);
	glEnableVertexArrayAttrib(mdl.vaoid, 0);
	glVertexArrayVertexBuffer(mdl.vaoid, 0, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(mdl.vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 0, 0);
	//gernerate color
	glEnableVertexArrayAttrib(mdl.vaoid, 1);
	glVertexArrayVertexBuffer(mdl.vaoid, 1, vbo_hdl, pos_size, sizeof(glm::vec3));
	glVertexArrayAttribFormat(mdl.vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 1, 1);

	glGenBuffers(1, &mdl.instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mdl.instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, mdl.maxInstances * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
	glBindVertexArray(0);
	// and then set up the rest of object mdl ...
	mdl.primitive_type = GL_TRIANGLE_FAN;
	mdl.draw_cnt = static_cast<GLuint>(pos_vtx.size()); // number of vertices
	mdl.elem_cnt = 0;       // no EBO
	mdl.ebo = 0;
	mdl.primitive_cnt = slices; // number of primitives
	mdl.shape = shape::circle;
	setup_instance_attributes(mdl);
	return mdl;
}
/*
* 
* create vao + vbo of quad
* 
* vector of mat4 - transforms
* 
* foreach object
*	transforms[i] = object.transform
* 
* for(inti = 0; i < transforms.size; i +=100
*	size = std::min(100, transforms.size - i);
*	gluniform(size, transforms[i])
* 
*	gldrawinstance(vao, size)
*/


//---------------------------------------------------------------------------------
//idk what im looking at, but hopefully gpt dun screw me over, to be refactored
//---------------------------------------------------------------------------------
void renderer::setup_instance_attributes(model& mdl)
{
	glBindVertexArray(mdl.vaoid);
	glBindBuffer(GL_ARRAY_BUFFER, mdl.instanceVBO);

	GLsizei stride = sizeof(InstanceData);
	std::size_t offset = 0;

	// mat4 model: locations 3,4,5,6 (each is a vec4)
	for (int i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(offset + i * sizeof(glm::vec4)));
		glVertexAttribDivisor(3 + i, 1);
	}
	offset += sizeof(glm::mat4);

	// color: location 7
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	glVertexAttribDivisor(7, 1);
	offset += sizeof(glm::vec4);

	// texParams: location 8 (still present for untextured; harmless)
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	glVertexAttribDivisor(8, 1);

	glBindVertexArray(0);
}

//---------------------------------------------------------------------------------
//idk what im looking at, but hopefully gpt dun screw me over, to be refactored
//---------------------------------------------------------------------------------
void renderer::drawInstances(model& mdl, const std::vector<InstanceData>& instances)
{
	if (instances.empty())
		return;

	glBindBuffer(GL_ARRAY_BUFFER, mdl.instanceVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		instances.size() * sizeof(InstanceData),
		instances.data());

	glBindVertexArray(mdl.vaoid);

	if (mdl.elem_cnt > 0) {
		// Indexed path (square)
		glDrawElementsInstanced(GL_TRIANGLES,
			mdl.elem_cnt,
			GL_UNSIGNED_SHORT,
			nullptr,
			static_cast<GLsizei>(instances.size()));
	}
	else {
		// Non-indexed path (circle)
		glDrawArraysInstanced(mdl.primitive_type,
			0,
			mdl.draw_cnt,
			static_cast<GLsizei>(instances.size()));
	}

	glBindVertexArray(0);
}

void renderer::setup_shdrpgm()
{

	GLuint shader1 = ResourceManager::getInstance().getShader("shaders/hasTex.vert", "shaders/hasTex.frag");
	std::cout << "Shader program ID (hasTex): " << shader1 << std::endl;
	GLuint shader2 = ResourceManager::getInstance().getShader("shaders/noTex.vert", "shaders/noTex.frag");
	std::cout << "Shader program ID (noTex): " << shader2 << std::endl;

	shdr_pgm.push_back(shader1);
	shdr_pgm.push_back(shader2);

}

void renderer::camera::init(int w, int h)
{
	ar = static_cast<float>(w) / h;
	width = w;
	rot = 0;
	zoom = 10;
	campos = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);
	zoomFactor = 0.1f;
	minZoom = 0.1f;
	maxZoom = 50.f;
}

void renderer::camera::update()
{
	if (mode == cameraMode::GAME)
	{
		// for gameplay camera, always focus at the center of the view
		view = glm::lookAt(campos, glm::vec3(0, 0, -1), up);
	}
	else
	{
		// for editor camera, center move with camera
		view = glm::lookAt(campos, glm::vec3(campos.x, campos.y, -1.f), up);
	}

	proj = glm::ortho(-zoom * ar, zoom * ar, -zoom, zoom, -1.f, 1.f);
	
}

void renderer::camera::pan(const Vector2D& delta) {
	campos -= glm::vec3(delta.x, delta.y, 0.f);
}

void renderer::camera::zoomInOut(float scrollDelta) {
	zoom *= 1.0f - scrollDelta * zoomFactor;

	// clamp zoom
	zoom = std::max(minZoom, zoom);
	zoom = std::min(maxZoom, zoom);
}

void renderer::cleanup() {
	// Clean up all models
	for (auto& model : models) {
		if (model.vaoid) glDeleteVertexArrays(1, &model.vaoid);
		if (model.vbo) glDeleteBuffers(1, &model.vbo);
		if (model.ebo) glDeleteBuffers(1, &model.ebo);
		if (model.instanceVBO) glDeleteBuffers(1, &model.instanceVBO);
	}
	models.clear();

	// Clean up shader programs
	for (GLuint program : shdr_pgm) {
		if (program) glDeleteProgram(program);
	}
	shdr_pgm.clear();
	std::cout << "Renderer cleanup complete\n";
}