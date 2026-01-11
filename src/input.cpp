/* Start Header ************************************************************************/
/*!
\file       input.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 15th, 2025
\brief      This source file implements the InputHandler class, which centralizes input
			handling for GLFW. It maintains arrays of current and previous states for
			keyboard keys and mouse buttons, enabling detection of triggered, released,
			held, and combo inputs.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "editor/editorManager.h"
#include "renderer.h"
#include "input.h"

GLFWwindow* InputHandler::m_window = nullptr;
AppConfig InputHandler::m_cfg;

bool InputHandler::m_keys[1024] = { false };
bool InputHandler::m_keysPrevious[1024] = { false };

bool InputHandler::m_mouseButtons[8] = { false };
bool InputHandler::m_mouseButtonsPrevious[8] = { false };
double InputHandler::m_lastClickTime[8] = { 0.0 };
Vector2D InputHandler::m_mousePosition = Vector2D(0.0f);
Vector2D InputHandler::m_mousePositionPrevious = Vector2D(0.0f);
Vector2D InputHandler::m_mouseDragStartPos[8] = { false };
bool InputHandler::m_isDragging[8] = { false };
bool InputHandler::m_isCursorInside = false;

float InputHandler::m_scrollOffset = 0.0f;
double InputHandler::m_doubleClickThreshold = 0.25;

// initialize input handler with window and set up callbacks
void InputHandler::init(GLFWwindow* window, const AppConfig& cfg) {
	m_window = window;
	m_cfg = cfg;

	// set up the callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetCursorEnterCallback(window, CursorEnterCallback);
	glfwSetScrollCallback(window, scrollCallback);
#ifdef _DEBUG
	glfwSetDropCallback(window, dropCallback);
#endif
}

// update input states (call once per frame)
void InputHandler::update() {
	// store previous states
	memcpy(m_keysPrevious, m_keys, sizeof(m_keys));
	memcpy(m_mouseButtonsPrevious, m_mouseButtons, sizeof(m_mouseButtons));
	m_mousePositionPrevious = m_mousePosition;

	// reset scroll each frame
	m_scrollOffset = 0.0f;

	glfwPollEvents();
}


/* ----- KEYBOARD ----- */
// check if key is triggered (pressed this frame)
bool InputHandler::isKeyTriggered(int key) {
	if (key < 0 || key >= 1024) return false;

	// only TRUE if the key is pressed this frame & was NOT pressed last frame
	return (m_keys[key] && !m_keysPrevious[key]);
}

// check if key is released (released this frame)
bool InputHandler::isKeyReleased(int key) {
	if (key < 0 || key >= 1024) return false;

	// only TRUE if the key is released this frame & was pressed last frame
	return (!m_keys[key] && m_keysPrevious[key]);
}

// check if key is held (currently down)
bool InputHandler::isKeyHeld(int key) {
	if (key < 0 || key >= 1024) return false;

	// TRUE if the key is currently down
	return m_keys[key];
}

// check if both keys are triggered (default is left shift + another given key)
bool InputHandler::isComboKeyTriggered(int key1, int key2) {
	if (key1 < 0 || key1 >= 1024) return false;
	if (key2 < 0 || key2 >= 1024) return false;

	// key1 is held, key2 just pressed this frame
	if (m_keys[key1] && (m_keys[key2] && !m_keysPrevious[key2])) {
		return true;
	}
	// key2 is held, key1 just pressed this frame
	if (m_keys[key2] && (m_keys[key1] && !m_keysPrevious[key1])) {
		return true;
	}

	return false;
}

// check if both keys are being held down (default is left shift + another given key)
bool InputHandler::isComboKeyHeld(int key1, int key2) {
	if (key1 < 0 || key1 >= 1024) return false;
	if (key2 < 0 || key2 >= 1024) return false;

	return (m_keys[key1] && m_keys[key2]);
}

/* ----- MOUSE ----- */
// check if mouse button is clicked (pressed this frame)
bool InputHandler::isMouseButtonClicked(int button) {
	if (button < 0 || button >= 8) return false;

	// only TRUE if the button is pressed this frame & was NOT pressed last frame
	return (m_mouseButtons[button] && !m_mouseButtonsPrevious[button]);
}

// check if left mouse button is clicked (pressed this frame)
bool InputHandler::isMouseLeftClicked() {
	return (m_mouseButtons[GLFW_MOUSE_BUTTON_LEFT] && !m_mouseButtonsPrevious[GLFW_MOUSE_BUTTON_LEFT]);
}

// check if left mouse button is being held down (currently down)
bool InputHandler::isMouseLeftHeld() {
	return m_mouseButtons[GLFW_MOUSE_BUTTON_LEFT];
}

// check if left mouse button is released (released this frame)
bool InputHandler::isMouseLeftReleased() {
	return (!m_mouseButtons[GLFW_MOUSE_BUTTON_LEFT] && m_mouseButtonsPrevious[GLFW_MOUSE_BUTTON_LEFT]);
}

// check if left mouse button is double clicked (within threshold time)
bool InputHandler::isMouseLeftDoubleClicked() {
	return isMouseButtonDoubleClicked(GLFW_MOUSE_BUTTON_LEFT);
}

// check if the given mouse button is double clicked (within threshold time)
bool InputHandler::isMouseButtonDoubleClicked(int button) {
	if (button < 0 || button >= 8) return false;

	if (m_mouseButtons[button] && !m_mouseButtonsPrevious[button]) {
		double currentTime = glfwGetTime();
		if ((currentTime - m_lastClickTime[button]) < m_doubleClickThreshold) { // double click if within the threshold
			m_lastClickTime[button] = 0.0; // reset last click time
			return true;
		}
		m_lastClickTime[button] = currentTime; // update last click time
	}
	return false;
}

// check if mouse is being dragged (left button held and moving)
bool InputHandler::isMouseDragging(int button) {
	if (button < 0 || button >= 8) return false;
	return m_isDragging[button];
}

// get change in mouse position since drag started
Vector2D InputHandler::getMouseDragDelta(int button) {
	if (!m_isDragging) return Vector2D(0.0f);

	return Vec_Sub(&m_mousePosition, &m_mouseDragStartPos[button]);
}

// get mouse position when drag started
Vector2D InputHandler::getMouseDragStartPos(int button) {
	return m_mouseDragStartPos[button];
}

//Vector2D InputHandler::getMouseDeltaWorld() {
//	Vector2D delta = getMouseDelta();
//
//	if (delta.x == 0.0f && delta.y == 0.0f) return Vector2D(0.0f);
//
//	// convert screen space to normalized
//	float normX = delta.x / (m_cfg.width / 2.f);
//	float normY = -delta.y / (m_cfg.height / 2.f);
//
//	float scaleX = (renderer::cam.width / 9.1f);
//	float scaleY = ((renderer::cam.width / renderer::cam.ar) / 9.1f);
//
//	// apply camera position and zoom
//	float worldX = (normX * scaleX) / renderer::cam.zoom;
//	float worldY = (normY * scaleY) / renderer::cam.zoom;
//
//	return Vector2D(worldX, worldY);
//}

// get current cursor position
Vector2D InputHandler::getMousePosition() {
	return m_mousePosition;
}

// get current cursor x position
float InputHandler::getMouseX() {
	return m_mousePosition.x;
}

// get current cursor y position
float InputHandler::getMouseY() {
	return m_mousePosition.y;
}

// get change in mouse position since last frame
Vector2D InputHandler::getMouseDelta() {
	return Vec_Sub(&m_mousePosition, &m_mousePositionPrevious);
}

// check if cursor is inside window
bool InputHandler::isCursorInsideWindow() {
	return m_isCursorInside;
}

Vector2D InputHandler::getMousePositionInImGuiViewport(const ImVec2& scenePos, const ImVec2& sceneSize) {
	ImVec2 mousePos = ImGui::GetMousePos();//screen position, not viewport
	//std::cout << "scenePos: " << scenePos.x << " | " << scenePos.y << std::endl;

	Vector2D localMouse = { 0.f, 0.f };
	localMouse.x = mousePos.x - scenePos.x - 0.5f * sceneSize.x;// Range:[-sceneSize/2, sceneSize/2] right is +ve
	localMouse.y = 0.5f * sceneSize.y + scenePos.y - mousePos.y;// Range:[-sceneSize/2, sceneSize/2] up is +ve

	//std::cout << "local Mouse is at: " << localMouse.x << " | " << localMouse.y << std::endl;

	Vector2D scale;
	scale.x = (2.f * renderer::editorCam.ar * renderer::editorCam.zoom) / sceneSize.x;
	scale.y = (2.f * renderer::editorCam.zoom) / sceneSize.y;

	//std::cout << "world scale is: " << scale.x << " | " << scale.y << std::endl;

	Vector2D world;
	world.x = (localMouse.x * scale.x) + renderer::editorCam.campos.x;
	world.y = (localMouse.y * scale.y) + renderer::editorCam.campos.y;

	//std::cout << "campos is: " << renderer::cam.campos.x << " | " << renderer::cam.campos.y << std::endl;
	//std::cout << "worldpos is: " << world.x << " | " << world.y << std::endl;

	return world;
}

Vector2D InputHandler::getMouseDeltaWorldInViewport(const ImVec2& sceneSize)
{
	Vector2D delta = getMouseDelta();

	Vector2D scale;
	scale.x = (2.f * renderer::editorCam.ar * renderer::editorCam.zoom) / sceneSize.x;
	scale.y = (2.f * renderer::editorCam.zoom) / sceneSize.y;

	//std::cout << "world scale is: " << scale.x << " | " << scale.y << std::endl;

	Vector2D world;
	world.x = delta.x * scale.x;
	world.y = -delta.y * scale.y;

	//std::cout << "campos is: " << renderer::cam.campos.x << " | " << renderer::cam.campos.y << std::endl;
	//std::cout << "worldpos is: " << world.x << " | " << world.y << std::endl;

	return world;
}

//// get cursor world postion
//Vector2D InputHandler::getWorldMousePosition() {
//	float mx = getMouseX();
//	float my = getMouseY();
//
//	my = m_cfg.height - my;
//
//	// convert screen space to normalized
//	float normX = (mx - m_cfg.width / 2.f) / (m_cfg.width / 2.f);
//	float normY = (my - m_cfg.height / 2.f) / (m_cfg.height / 2.f);
//
//	float scaleX = (renderer::cam.width / 9.1f);
//	float scaleY = ((renderer::cam.width / renderer::cam.ar) / 9.1f);
//
//	// apply camera position and zoom
//	float worldX = (normX * scaleX) / renderer::cam.zoom + renderer::cam.campos.x;
//	float worldY = (normY * scaleY) / renderer::cam.zoom + renderer::cam.campos.y;
//
//	return Vector2D(worldX, worldY);
//}
Vector2D InputHandler::getWorldMousePosition() {
	float mx = getMouseX();
	float my = getMouseY();

	// Flip Y coordinate (screen origin top-left, world origin center)
	my = m_cfg.height - my;

	// Convert screen space to normalized device coordinates [-1, 1]
	float normX = (mx - m_cfg.width / 2.f) / (m_cfg.width / 2.f);
	float normY = (my - m_cfg.height / 2.f) / (m_cfg.height / 2.f);

	// Use camera's actual orthographic bounds instead of magic numbers
	// The camera's ortho projection is: glm::ortho(-zoom * ar, zoom * ar, -zoom, zoom, -1.f, 1.f)
	float worldWidth = renderer::cam.zoom * renderer::cam.ar * 2.0f;  // Total width in world units
	float worldHeight = renderer::cam.zoom * 2.0f;                     // Total height in world units

	// Convert normalized coords to world space
	float worldX = normX * (worldWidth / 2.0f) + renderer::cam.campos.x;
	float worldY = normY * (worldHeight / 2.0f) + renderer::cam.campos.y;

	return Vector2D(worldX, worldY);
}

// get world cursor x position
float InputHandler::getWorldMouseX() {
	return getWorldMousePosition().x;
}

// get world cursor y position
float InputHandler::getWorldMouseY() {
	return getWorldMousePosition().y;
}

// get scroll offset since last frame
float InputHandler::getMouseScroll() {
	return m_scrollOffset;
}


/* ----- CALLBACKS ----- */
//did not use mods, did not use scancode, did not use window
// key callback function (GLFW will call this automatically)
void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)mods;
	(void)scancode;
	(void)window;
	if (key < 0 || key >= 1024) return;

	// is key is pressed, change bool in m_key to TRUE, if released then FALSE
	if (action == GLFW_PRESS) {
		m_keys[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		m_keys[key] = false;
	}
}

//did not use mods, did not use window
// mouse button callback function (GLFW will call this automatically)
void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	(void)mods;
	(void)window;
	if (button < 0 || button >= 8) return;

	if (action == GLFW_PRESS) {
		m_mouseButtons[button] = true;
		m_isDragging[button] = true;
		m_mouseDragStartPos[button] = m_mousePosition;
	}
	else if (action == GLFW_RELEASE) {
		m_mouseButtons[button] = false;
		m_isDragging[button] = false;
	}
}
//did not use window

// cursor position callback function (GLFW will call this automatically)
void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	(void)window;
	m_mousePosition = Vector2D(static_cast<float>(xpos), static_cast<float>(ypos));
}
//did not use window

// cursor enter/leave callback function (GLFW will call this automatically)
void InputHandler::CursorEnterCallback(GLFWwindow* window, int entered) {
	(void)window;
	if (entered) {
		// Cursor entered the window
		m_isCursorInside = true;
	}
	else {
		// Cursor left the window
		m_isCursorInside = false;

	}
}
//did not use window, did not use xoffset

// scroll callback function (GLFW will call this automatically)
void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	(void)window;
	(void)xoffset;
	m_scrollOffset = static_cast<float>(yoffset);
}

void InputHandler::setWindowSize(int width, int height)
{
	m_cfg.width = width;
	m_cfg.height = height;
}


#ifdef _DEBUG
void InputHandler::dropCallback(GLFWwindow* window, int count, const char** paths) {
	(void)window;

    std::filesystem::path buildDir(RUNTIME_DIR_R); // "./assets"
    std::filesystem::path projectDir(SOURCE_DIR_R);  // "./projects/WaterBound/assets"

	// check if dropping onto a specific folder in editor
	std::string targetFolder = Editor::assetBrowserState.hoverFolder;

	for (int i = 0; i < count; ++i) {
		std::filesystem::path src(paths[i]);
		std::string ext = src.extension().string();
		std::filesystem::path dstBuild = buildDir / src.filename();
		std::filesystem::path dstProject = projectDir / src.filename();

		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		// Auto-detect folder based on extension if no hover target
		if (targetFolder.empty()) {
			// if audio file, auto add to Audio
			if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") {
				targetFolder = "Audio";
			}
			// if textures, auto add to textures
			else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
				targetFolder = "Texture";
			}
		}
		else if (targetFolder == "Misc") targetFolder.clear();

		if (!targetFolder.empty()) {
			dstBuild = buildDir / targetFolder / src.filename();
			dstProject = projectDir / targetFolder / src.filename();
		}

		try {
			// Copy to build/debug folder
			if (std::filesystem::exists(dstBuild)) {
				std::filesystem::remove_all(dstBuild);
			}
			if (std::filesystem::is_directory(src)) {
				std::filesystem::copy(src, dstBuild, std::filesystem::copy_options::recursive);
			}
			else {
				std::filesystem::copy_file(src, dstBuild, std::filesystem::copy_options::overwrite_existing);
			}

			// Copy to project folder
			if (std::filesystem::exists(dstProject)) {
				std::filesystem::remove_all(dstProject);
			}
			if (std::filesystem::is_directory(src)) {
				std::filesystem::copy(src, dstProject, std::filesystem::copy_options::recursive);
			}
			else {
				std::filesystem::copy_file(src, dstProject, std::filesystem::copy_options::overwrite_existing);
			}

			EditorManager::assetChanged(); // flag to reload assets
			DebugLog::addMessage("Added asset " + src.string() + " to " + dstBuild.string() + " and " + dstProject.string() + "\n");
		}
		catch (const std::filesystem::filesystem_error& e) {
			DebugLog::addMessage("Failed to add asset " + src.string() + ": " + e.what() + "\n");
		}
	}
}
#endif