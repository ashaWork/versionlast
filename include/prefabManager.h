/* Start Header ************************************************************************/
/*!
\file       prefabManager.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       November, 07th, 2025
\brief      A class that load and manage the prefabs by maintaining a prefab registry.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <unordered_map>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include "GameObjectManager.h"

//struct PrefabData {
//	std::string name;
//	std::string id;
//};

/*
HOW TO USE THIS
- PrefabManager::Instance() -> to access the singleton instance
e.g. PrefabManager::Instance().instantiate("player1"); -> to instantiate a prefab with id "player1"
*/

class PrefabManager {
public:
	// singleton access
	static PrefabManager& Instance();

	/* -------- Registry Manage -------- */
	// load the registry
	void loadPrefabRegistry();

	// save the registry (e.g. aft renaming, will auto call)
	void savePrefabRegistry();
	/* -------- END -------- */


	/* -------- Setter & Getter -------- */
	/*std::string& getPrefabPath(const std::string& prefabID);*/
	// path name with the parent folder (e.g. Prefab/abc.json)
	const std::string& getPrefabPath(const std::string& prefabID) const;
	bool setPrefabPath(const std::string& prefabID, const std::string& newName);

	// name without the parent folder (e.g. abc.json)
	std::string getPrefabFilename(const std::string& prefabID) const;

	std::string& getPrefabName(const std::string& prefabID);
	const std::string& getPrefabName(const std::string& prefabID) const;

	// find prefabid by filename, this will auto remove the parent folder
	std::string findPrefabIDByFilename(const std::filesystem::path& filename);
	/* -------- END -------- */


	/* -------- Getting JSON -------- */
	// looks up a prefab’s JSON document by its ID in opened json cache
	const rapidjson::Document* getPrefabJson(const std::string& prefabID) const;

	// retrieves component’s JSON data inside a prefab’s JSON document
	const rapidjson::Value* getPrefabJsonComp(const rapidjson::Document* prefabDoc, const std::string& compName) const;
	/* -------- END -------- */


	/* -------- Prefab with game obj -------- */
	//GameObject* instantiate(const std::string& id, GameObjectManager& manager);
	GameObject* instantiate(GameObject* go, GameObjectManager& manager);

	// convert prefab to a temp game obj to allow editting in editor
	std::unique_ptr<GameObject> createTempPrefabObj(const std::string& prefabID);

	// save current state of prefab to its file
	bool savePrefab(const GameObject* prefabObj); // prefabObj is a temp obj rep the prefab

	// revert game object back to follow prefab
	GameObject* revertToPrefab(GameObject* obj, GameObjectManager& manager);

	// apply prefab changes to all existing instances in current scene and return affected counts
	int applyToAllInstances(const std::string& prefabID, GameObjectManager& manager);

	// create a prefab out of a game obj
	bool createPrefabFromGameObj(GameObject* go, const std::string& name = "", /* whether to make this game obj ref to the game obj */ bool makeRef = false);
	/* -------- END -------- */


	// DO NOT USE THIS, INCOMPLETE
	bool deletePrefab(const std::string prefabID);

private:
	struct PrefabInfo {
		std::string name;
		std::string path;
	};

	// prefab registry mapping prefab ID to PrefabInfo
	// format {prefabID: {name, path}}
	std::unordered_map<std::string, PrefabInfo> m_prefabRegistry;

	// prefab jsons cache aft its first load
	std::unordered_map<std::string, rapidjson::Document> m_prefabCache;

	// cache original prefab state before editing (for comparing later)
	std::unordered_map<std::string, rapidjson::Document> m_prefabBeforeEdit;

	std::string prefabFolder = "/Prefab";
	std::string registryFile = "/Prefab/prefab_registry.json";

	/* -------- Helper Functions -------- */
	// generate UUID for creation of the prefab
	std::string generateUUID();

	// apply components from JSON
	void applyPrefabComponents(GameObject* obj, const rapidjson::Value& comps);

	// serialize components from JSON
	void serializeComponents(const GameObject* obj, rapidjson::Value& comps, rapidjson::Document::AllocatorType& a);

	// scene data override all
	int applyAllInstancesFallback(const std::string& prefabID, GameObjectManager& manager);

	// compare 2 json values
	bool valuesEqual(const rapidjson::Value& lhs, const rapidjson::Value& rhs) const;

	// apply prefab changes based on comparison result
	void applyPrefabComponentsSelective(GameObject* obj, const rapidjson::Value& oldPrefabComps, const rapidjson::Value& newPrefabComps);
	/* -------- END -------- */
};