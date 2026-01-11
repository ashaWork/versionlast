/* Start Header ************************************************************************/
/*!
\file       editorManager.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 95%
			Pearly Lin Lee Ying, p.lin, 2401591
			- 5% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Multiple class for the rendering of different ImGui viewports for editor.
			Each class will have its own utility functions to aid in the purpose it served.
			EditorManager will be the only public interface needed to update in UISystem.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#pragma once
#include <imgui.h> //for use of imgui in UISystem
#include <imgui_impl_glfw.h> //for use of imgui in UISystem
#include <imgui_impl_opengl3.h> //for use of imgui in UISystem
#include <iostream>
#include <algorithm>
#include "Editor/editorState.h"
#include "Editor/gameDebugLog.h"
#include "Editor/undoRedo.h"
#include "collision.h"
#include "GameObjectManager.h"
#include "JsonIO.h"
#include "prefabManager.h"
#include "performance.h"

#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "../projects/WaterBound/assets"
#endif

// add obj window & obj creation+deletion func
class AddObjWindow {
public:
	AddObjWindow(Editor::ShapeControlState& Cshape, Editor::ShapeControlState& Rshape);
	void render(GameObjectManager& manager);

	static void createEmpty(GameObjectManager& manager, float x = 0, float y = 0, float z = 0);

	static void createRec(GameObjectManager& manager, float x = 0, float y = 0, float z = 0);

	static void createCir(GameObjectManager& manager, float x = 0, float y = 0, float z = 0);

	static void dupObj(GameObjectManager& manager, GameObject* oriObj);

private:
	Editor::ShapeControlState& m_circleInput;
	Editor::ShapeControlState& m_rectangleInput;
};

// hot loaded asset browser
class AssetBrowser {
public:
	AssetBrowser(Editor::AssetBrowserState& Astate, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate);

	void render();
	void loadAssetsFromDirectory();

	// return the loaded list of assets (to check duplicated name when creating new scene)
	const std::vector<Editor::Asset>& getAssets() const { return m_assets; }

private:
	void assetDeletePopup();
	void assetRenamePopup();
	void assetReplacePopup();

	Editor::AssetBrowserState& m_AssetBrowserState;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::MenuBarState& m_menuBarState;
	std::vector<Editor::Asset> m_assets;
};

// Debug console
class DebugWindow {
public:
	DebugWindow(DebugMode Dmode);
	void render();

private:
	DebugMode mode;
};

// Hierarchy (object list)
class HierarchyWindow {
public:
	HierarchyWindow(Editor::ObjSelectionState& Ostate);
	void render(GameObjectManager& manager);

private:
	void handlePrefabDragDrop(GameObjectManager& manager);
	void showContextMenu(GameObjectManager& manager, GameObject* obj, int objIndex);

	void objDeletePopup(GameObjectManager& manager);
	void createPrefabPopup();

	Editor::ObjSelectionState& m_objSelectionState;
};

// Inspector (property editor)
class InspectorWindow {
public:
	InspectorWindow(Editor::ObjSelectionState& Ostate, Editor::GizmoState& gState);
	void render(GameObjectManager& manager);

private:
	void addComponent(GameObject* selected);
	void renderGizmoCtrl();
	void renderTransform(GameObject* selected);
	void renderRender(GameObject* selected);
	void renderStateMachine(GameObject* selected);
	void renderCollision(GameObject* selected, GameObjectManager& manager);
	void renderInput(GameObject* selected);
	void renderPhysics(GameObject* selected, bool canMove);
	void renderFont(GameObject* selected);
	void renderAudio(GameObject* selected);
	void renderTileMap(GameObject* selected);

	void wrongFileTypePopup(const std::string& filename);

	Editor::ObjSelectionState& m_objSelectionState;
	Editor::GizmoState& m_gizmoState;
};

// Top menu bar
class MenuBar {
public:
	MenuBar(AssetBrowser& aBrowser, Editor::AssetBrowserState& Astate, Editor::MenuBarState& Mstate, Editor::ObjSelectionState& Ostate, Editor::SceneState& sceneState);
	void render(GameObjectManager& manager);
	void toggleSimulation(GameObjectManager& manager); // play/stop simulation
	void pauseResumeSimul();

private:
	void drawFileMenu(GameObjectManager& manager);
	void drawEditMenu();
	void drawSimulationMenu(GameObjectManager& manager);
	void drawThemeMenu();

	void playSimulation(GameObjectManager& manager);
	void stopSimulation(GameObjectManager& manager);
	void pauseSimulation();
	void resumeSimulation();

	void themeSinLeDark();
	void themeSinLeLight();
	void themeForest();

	AssetBrowser& m_assetBrowser;
	Editor::AssetBrowserState& m_AssetBrowserState;
	Editor::MenuBarState& m_menuBarState;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::SceneState& m_sceneState;
};

// Performance
class PerformanceWindow {
public:
	void render();
};

// Scene Window & Editor Camera Window
class SceneWindow {
public:
	SceneWindow(AssetBrowser& aBrowser, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate, Editor::SceneWindowState& Sstate, Editor::SceneState& sceneState, Editor::GizmoState& gizmoState);

	void render(GLuint texture, float aspectRatio, GameObjectManager& manager);

	std::string getCurrentSceneName() const { return m_sceneState.currentSceneName; }

	// reusable func to clear obj selections
	void resetSelection();

	static bool isSceneHovered() { return s_isSceneHovered; }

private:
	/* --------- Render -------- */
	void renderGizmo(GameObjectManager& manager);
	void drawGrid(const glm::mat4& view, const glm::mat4& proj, GameObjectManager& manager);
	void editorCameraControls();
	/* -------- END -------- */


	/* --------- Scene Management -------- */
	void newScene(GameObjectManager& manager);
	// dup current scene and open
	void duplicateScene(GameObjectManager& manager);
	// load existing scene
	void loadScene(GameObjectManager& manager);
	/* -------- END -------- */

	/* --------- Pop up Modals -------- */
	void newScenePopup(GameObjectManager& manager);
	void duplicateScenePopup(GameObjectManager& manager);
	void loadScenePopup(GameObjectManager& manager);
	/* -------- END -------- */


	/* -------- Object Click and Drag -------- */
	// handle click and select logic and collision
	void objPicking(GameObjectManager& manager);
	// handle dragging of the obj (obj will follow cursor)
	void objDragging() const;
	/* -------- END -------- */

	AssetBrowser& m_assetBrowser;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::MenuBarState& m_menuBarState;
	Editor::SceneWindowState& m_sceneWindowState;
	Editor::SceneState& m_sceneState;
	Editor::GizmoState& m_gizmoState;

	static bool s_isSceneHovered;
};




// the big laoban for ALL editor viewports
class EditorManager {
public:
	EditorManager();

	/* -------- Render all editor viewports ------- */
	void update(GameObjectManager& manager);
	void renderScene(GLuint texture, float aspectRatio, GameObjectManager& manager);
	/* -------- END -------- */

	static void assetChanged() { assetsChangedFlag = true; } // set to true to reload assets in UI
	static bool isEditingMode() { return isEditing; }
	static void toggleEditing(bool e) { isEditing = e; } // if true then editing mode, dun play animation, obj dun move

	static bool isPaused() { return pausedSimul; }
	static void togglePause(bool e) { pausedSimul = e; }

private:
	void handleShortcutKeys(GameObjectManager& manager);

	AddObjWindow addObjWindow;
	AssetBrowser assetBrowser;
	DebugWindow editorDebugWindow;
	DebugWindow playDebugWindow;
	HierarchyWindow hierarchyWindow;
	InspectorWindow inspectorWindow;
	MenuBar menuBar;
	PerformanceWindow performanceWindow;
	SceneWindow sceneWindow;

	static inline bool isEditing = false;
	static inline bool pausedSimul = false;
	static inline bool assetsChangedFlag = true;
};
#endif