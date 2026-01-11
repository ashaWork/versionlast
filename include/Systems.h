/* Start Header ************************************************************************/
/*!
\file        Systems.h
\author      Seow Sin Le, s.sinle, 2401084, 40%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 45%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 15%
\par         s.sinle@digipen.edu
\par		 low.h@digipen.edu
\par		 p.lin@digipen.edu
\date        October, 1st, 2025
\brief       System functions to run the logic for the components

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "renderer.h" //RenderSystem needs the full definition of the renderer
#include "input.h" //InputSystem needs access to the InputHandler
#include "GameObjectManager.h"
//#include <stb_image.h>
#include "collision.h"
#include "imgui_internal.h" // for docking in UISystem
//#include <ui.h>
#include "Editor/editorManager.h"
#include "font.h"
#include "messageBus.h"
#include "controllerSystem.h"
#include <LogicContainer.h>
#include "audio.h"

//forward declaration
struct renderer;

//all systems need to have an update function

// Input system - reads user input and updates velocity accordingly
class InputSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus);
};

// Physics system - updates position based on velocity and applies gravity
class PhysicsSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus);

private:
	bool m_stepMode = false;
	bool m_stepReq = false;
};

/*!***********************************************************************
\brief
	render system

*************************************************************************/
class RenderSystem {
	GLuint fbo{};
	GLuint texture{};
	GLuint depthBuffer{};

	int fboWidth = 0;
	int fboHeight = 0;

	void createFBO();
public:
	/*!***********************************************************************
	\brief
		initializes textures for all objects with render component

	\param[in] manager
		array of all objects

	*************************************************************************/
	void init(GameObjectManager& manager, int fboW, int fboH);
	/*!***********************************************************************
	\brief
		resend matrix data to the shaders to recalculated object position and orientation to draw for the new frame

	\param[in] manager
		array of all objects

	*************************************************************************/
	void update(GameObjectManager& manager, float const& deltaTime);
	//moved to ResourceManager
	//GLuint uploadtex(std::string const& filename, bool& isTransparent);
	void renderTex(GameObject* object, float const& deltaTime);
	void renderNoTex(GameObject* object);
	void renderFBO();
	void resizeFBO(int width, int height);
	GLuint getTexture() const { return texture; }

	//not implemented yet, hopefully tri break can do
	//void renderCollision(Collision::AABB const& box, glm::vec3 const& clr);
	//GLuint getTexture() const { return texture; }
	void batchingSetUp(GameObjectManager& manager, float const& deltaTime);

	void fboAspectRatio(int& width, int& height) const;
	~RenderSystem();
	//bool batchRebuild = true;
	static std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> objectWithTex2;
private:
	std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> objectWithTex;
	std::unordered_map<shape, std::vector<renderer::InstanceData>> objectWithoutTex;
};

/*!***********************************************************************
\brief
	Tile Map system

*************************************************************************/
class TileMapSystem
{
public:
	/*!***********************************************************************
	\brief
		update cycle to push the data in tiles to objectWithTex2 to be rendered in render system

	\param[in] manager
		array of all objects

	*************************************************************************/
	void update(GameObjectManager& manager);

	/*!***********************************************************************
	\brief
		update the tiles for the object selected with the texture selected

	\param[in] obj
		object with TileMap component

	*************************************************************************/
	void tileUpdate(GameObject* obj);
	//std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> objectWithTex2{};

	/*!***********************************************************************
	\brief
		to store the filename and filepath of the asset that the mouse has clicked
		and use it to paint the tile map

	*************************************************************************/
	static std::string filename;
};

class FontSystem {
public:
	void init(GameObjectManager& manager);
	void update(GameObjectManager& manager, double fps);
	void RenderText(GLuint& s, std::string text, float x, float y, float scale, glm::vec3 color, GameObject* object);

	// sorry very scuffed but can remove aft submission
	static inline bool showFPS = false;
};

/*!***********************************************************************
\brief
	transform system

*************************************************************************/
class TransformSystem {
public:
	/*!***********************************************************************
	\brief
		updating the new position, scale and orientation of the object

	\param[in] GameObjectManager manager
		array of all objects

	\param[in] deltaTime
		tracking of the world time

	*************************************************************************/
	//void update(GameObjectManager& manager, const float& deltaTime);
};

// Collision system - detects and resolves collisions between game objects
class CollisionSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
};

// only allow editor in debug mode
#ifdef _DEBUG
// UI system - handles UI rendering and interactions using ImGui
class UISystem {
public:
	void init(GLFWwindow* window, RenderSystem* renderer);
	~UISystem();
	void update(GameObjectManager& manager);

	static void toggleUI() { showUI = !showUI; } // temp toggle function
	static bool isShowUI() { return showUI; }

private:
	void dockingSetUp();
	void renderMultipleViewPorts();
	//UI m_UI;
	EditorManager m_UI;
	static bool showUI;
	bool m_firstTime = true;
	RenderSystem* m_renderer = nullptr;
};
#else
namespace EditorManager {
	inline bool isEditingMode() { return false; }
	inline bool isPaused() { return false; }
}
namespace UISystem {
	inline bool isShowUI() { return false; }
}
#endif

class LogicSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
};

class AudioSystem {
public:
	void init(GameObjectManager & manager);
	void update(GameObjectManager& manager, float deltaTime);
	void cleanup(GameObjectManager& manager);

	static void initializeSceneAudio(GameObjectManager& manager);

private:
	void processAudio(GameObject* obj, AudioChannel* audio, float deltaTime);
};