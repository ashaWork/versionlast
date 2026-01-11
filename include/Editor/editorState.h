/* Start Header ************************************************************************/
/*!
\file       editorState.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Useful structs for the the editor, e.g. selection of objects, width and height
			of a viewport, etc. Also there is a magic vector to allow adding of components
			in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#pragma once
#include <string>
#include <memory>
#include <filesystem>
#include <imgui.h>
#include <ImGuizmo.h>
#include "GameObjectManager.h"

namespace Editor {
	/* ----------------- Menu Bar -----------------*/
	struct MenuBarState {
		bool showNewScenePopup = false;
		bool showLoadScenePopup = false;
		bool showDuplicatePopup = false;

		std::string sceneToLoad;
	};
	/* ----------------- Menu Bar End -----------------*/


	/* ----------------- Game Object Selection -----------------*/
	struct ObjSelectionState {
		GameObject* selectedObject = nullptr; // currently selected object
		int selectedIndex = -1; // index of selected object in the list

		std::unique_ptr<GameObject> selectedPrefab;

		GameObject* draggedObject = nullptr; // currently dragged object

		bool aspectRatioLock = true; // lock aspect ratio for resizing
		bool ratioSet = false;
		float ratioY = 1.0f;
		float ratioZ = 1.0f;

		bool showDeletePopup = false;
		bool showCreatePrefabPopup = false;
	};
	/* ----------------- Game Object Selection End -----------------*/


	struct ShapeControlState {
		float inputX = 0.0f;
		float inputY = 0.0f;
		float inputZ = 0.0f;
	};


	/* ----------------- Scene Viewport -----------------*/
	struct SceneWindowState {
		ImVec2 scenePos;  // top left corner of viewport
		ImVec2 sceneSize; // size of viewport
	};
	/* ----------------- Scene Viewport End -----------------*/

	/* ----------------- Assets -----------------*/
	// migrate to asset manager in the future
	struct Asset {
		std::string name;
		std::string path;
		std::string folder; // in case there is a parent folder (e.g. textures/) for filtering in the future
	};

	struct AssetBrowserState {
		std::string currentFolder = "assets/"; // we have an asset folder yayy
		std::filesystem::file_time_type lastTopLevelRefresh; // top level folder timestamp
		std::filesystem::file_time_type lastRefresh;
		std::string currentViewFolder;
		std::vector<std::string> subFolders;

		// for dropping of files from file explorer
		std::string hoverFolder;

		// for deleting asset via browser
		std::string fullPathToDelete;
		bool showDeletePopup = false;

		// for renaming asset via browser
		std::string fullPathToRename;
		bool showRenamePopup = false;
		char newNameBuffer[128] = "\0";

		std::string fullPathToReplace;
		bool showReplacePopup = false;
		char replacePathBuffer[128] = "\0";

		float iconSize = 100.0f;
		float buttonPadding = 20.0f; // adding padding made my code ugly
	};
	/* ----------------- Assets End -----------------*/

	
	/* ----------------- Scene State ---------------- */
	struct SceneState {
		std::string currentSceneName = "";
		std::string tempSceneName = "temp_scene.json"; // for simulation
	};
	/* ----------------- Scene End -----------------*/


	/* ----------------- Popup End -----------------*/
	struct PopupState {
		bool audioFileTypePopup = false;
		bool textureFileTypePopup = false;
		bool prefabFileTypePopup = false;
		std::string filename;
		std::string message;
	};
	/* ----------------- Popup End -----------------*/


	/* ----------------- Gizmo ---------------- */
	enum GizmoOperation {
		GIZMO_NONE = -1,
		GIZMO_TRANSLATE = ImGuizmo::TRANSLATE,
		GIZMO_ROTATE = ImGuizmo::ROTATE,
		GIZMO_SCALE = ImGuizmo::SCALE
	};

	struct GizmoState {
		GizmoOperation currentOp = GIZMO_NONE; // ImGuizmo::OPERATION
		ImGuizmo::MODE currentMode = ImGuizmo::WORLD; // do not change

		// snapping
		bool useSnap = false;
		float snapValues[3] = { 1.f, 1.f, 1.f };
		float snapAngle = 15.f;

		// grid
		bool showGrid = false;
		int gridSpacing = 1;
		Vector2D gridOffset{ 0.0f, 0.0f };
	};
	/* ----------------- Gizmo End -----------------*/


	/* ----------------- Component Entry ---------------- */
	struct ComponentEntry {
		std::string name;
		std::function<void(GameObject*)>addComp; // func to add component (hlp3 is now my fav module)
		std::function<bool(const GameObject*)>hasComp; // func to check if has component
	};
	/* ----------------- Component End ---------------- */

	inline MenuBarState menuBarState;
	inline ObjSelectionState objSelectionState;
	inline ShapeControlState circleInput;
	inline ShapeControlState rectangleInput;
	inline SceneWindowState sceneWindowState;
	inline AssetBrowserState assetBrowserState;
	inline SceneState sceneState;
	inline PopupState popupState;
	inline GizmoState gizmoState;


	/* ----------------- Component Registry ---------------- */
	// add useful func here to check/add components for Editor Inspector
	// do not add transform component here, every obj should by default has a transform alr
	inline static std::vector<ComponentEntry> componentRegistry{
		{"Render",
			[](GameObject* obj) {
			Render* r = obj->addComponent<Render>();
			r->modelRef = renderer::models[static_cast<int>(shape::square)];
			DebugLog::addMessage("Added Render component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Render>(); }
		},

		{"Collision",
			[] (GameObject* obj) {
			CollisionInfo* c = obj->addComponent<CollisionInfo>();

			Transform* t = obj->getComponent<Transform>();
			if (t) {
				c->colliderSize.x = t->scaleX;
				c->colliderSize.y = t->scaleY;
			}

			Render* r = obj->getComponent<Render>();
			if (r) {
				c->colliderType = r->modelRef.shape;
			}

			DebugLog::addMessage("Added Collision component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<CollisionInfo>(); }
		},

		{"Physics",
			[] (GameObject* obj) {
			obj->addComponent<Physics>();
			DebugLog::addMessage("Added Physics component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Physics>(); }
		},

		{"Input Control",
			[](GameObject* obj) {
			obj->addComponent<Input>();
			DebugLog::addMessage("Added Input component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Input>(); }
		},

		//{"Animation",
		//	[](GameObject* obj) {
		//	// animation need render component
		//	/*Render* r = obj->getComponent<Render>();
		//	if (!r) r = obj->addComponent<Render>();
		//	DebugLog::addMessage("Added Render component for " + obj->getObjectName());*/

		//	/*r->hasAnimation = true;
		//	r->hasTex = true;*/

		//	obj->addComponent<Animation>();
		//	DebugLog::addMessage("Added Animation component for " + obj->getObjectName());
		//	},
		//	[](const GameObject* obj) { return obj->hasComponent<Animation>();}
		//},
		{"State Machine",
			[](GameObject* obj) {
			obj->addComponent<StateMachine>();
			DebugLog::addMessage("Added State Machine for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<StateMachine>(); }
		},
		{"Font",
			[](GameObject* obj) {
			obj->addComponent<FontComponent>();
			DebugLog::addMessage("Added Font component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<FontComponent>(); }
		},
		{"Audio",
			[](GameObject* obj) {
			obj->addComponent<AudioComponent>();
			DebugLog::addMessage("Added Audio component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<AudioComponent>(); }
		},
		{"Tile Map",
			[](GameObject* obj) {
			obj->addComponent<TileMap>();
			DebugLog::addMessage("Added Tile Map component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<TileMap>(); }
		}
	};
	/* ----------------- Component End ---------------- */
}
#endif