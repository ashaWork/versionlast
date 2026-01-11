/* Start Header ************************************************************************/
/*!
\file       prefabManager.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       November, 07th, 2025
\brief      A class that load and manage the prefabs by maintaining a prefab registry.
			When applying prefab changes to instances -> serialize a prefab copy before
			saving -> compare with obj to know which are the override data -> selectively
			apply changes on those data that is not overriden

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "../projects/WaterBound/assets"
#endif
#include "prefabManager.h"
#include "JsonIO.h"

// -- helpers --
namespace {

	void removeComponentByName(GameObject* obj, const std::string& compName) {
		if (compName == "Transform") obj->removeComponent<Transform>();
		else if (compName == "Render") obj->removeComponent<Render>();
		else if (compName == "Input") obj->removeComponent<Input>();
		else if (compName == "Physics") obj->removeComponent<Physics>();
		else if (compName == "CollisionInfo") obj->removeComponent<CollisionInfo>();
		else if (compName == "Animation") obj->removeComponent<Animation>();
		else if (compName == "StateMachine") obj->removeComponent<StateMachine>();
		else if (compName == "AudioComponent") obj->removeComponent<AudioComponent>();
		else if (compName == "FontComponent") obj->removeComponent<FontComponent>();
		else if (compName == "TileMap") obj->removeComponent<TileMap>();
	}

	void removeAllComponents(GameObject* obj) {
		obj->removeComponent<Transform>();
		obj->removeComponent<Render>();
		obj->removeComponent<Input>();
		obj->removeComponent<Physics>();
		obj->removeComponent<CollisionInfo>();
		obj->removeComponent<Animation>();
		obj->removeComponent<StateMachine>();
		obj->removeComponent<AudioComponent>();
		obj->removeComponent<FontComponent>();
		obj->removeComponent<TileMap>();
	}

	void applyComponentFromJson(GameObject* obj, const char* compName, const rapidjson::Value& compData, rapidjson::Document::AllocatorType& allocator, const std::function<void(GameObject*, const rapidjson::Value&)>& func) {

		rapidjson::Value temp(rapidjson::kObjectType);
		rapidjson::Value copy;

		copy.CopyFrom(compData, allocator);
		temp.AddMember(rapidjson::Value(compName, allocator), copy, allocator);

		func(obj, temp);
	}
}

//static PlayerState parsePlayerState(const std::string& s) {
//	if (s == "Walking")  return PlayerState::Walking;
//	if (s == "Jumping")  return PlayerState::Jumping;
//	if (s == "Falling")  return PlayerState::Falling;
//	if (s == "Shooting") return PlayerState::Shooting;
//	if (s == "Dead")     return PlayerState::Dead;
//	return PlayerState::Idle; // default/fallback
//}

static const char* playerStateToStr(PlayerState state) {
	switch (state) {
	case PlayerState::Walking:  return "Walking";
	case PlayerState::Jumping:  return "Jumping";
	case PlayerState::Falling:  return "Falling";
	case PlayerState::Shooting: return "Shooting";
	case PlayerState::Dead:     return "Dead";
	default:                    return "Idle";
	}
}

PrefabManager& PrefabManager::Instance() {
	static PrefabManager instance;
	return instance;
}

void PrefabManager::loadPrefabRegistry() {
	rapidjson::Document doc;
	std::string err;

	std::string filename = std::string(RUNTIME_DIR_R) + registryFile;

	if (!JsonIO::ReadFileToDocument(filename, doc, &err)) {
		DebugLog::addMessage("Prefab registry load failed: " + err);
		return;
	}

	int version{ 1 };
	if (doc.HasMember("version") && doc["version"].IsInt())
		version = doc["version"].GetInt();
	if (version != 1)
		DebugLog::addMessage("Warning: unsupported prefab registry version " + version);

	m_prefabRegistry.clear();

	if (!doc.HasMember("prefabIndex") || !doc["prefabIndex"].IsArray()) {
		DebugLog::addMessage("Prefab registry JSON missing PREFABINDEX array.");
		return;
	}

	for (auto& jPrefab : doc["prefabIndex"].GetArray()) {
		if (!jPrefab.IsObject()) continue;

		std::string id = (jPrefab.HasMember("id") && jPrefab["id"].IsString()) ? jPrefab["id"].GetString() : "";

		std::string name = (jPrefab.HasMember("name") && jPrefab["name"].IsString())
			? jPrefab["name"].GetString()
			: ("Prefab_" + std::to_string(m_prefabRegistry.size()));

		std::string path = (jPrefab.HasMember("path") && jPrefab["path"].IsString()) ? jPrefab["path"].GetString() : "";

		if (!id.empty()) m_prefabRegistry[id] = { name, path };
	}

	std::cout << "Prefab registry loaded: " << filename << " (" << m_prefabRegistry.size() << " prefabs)\n";
}

void PrefabManager::savePrefabRegistry() {
	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	// version number (same as loader)
	doc.AddMember("version", 1, a);

	// Create the prefabIndex array
	rapidjson::Value prefabArray(rapidjson::kArrayType);

	for (const auto& [id, prefab] : m_prefabRegistry) {
		rapidjson::Value prefabObj(rapidjson::kObjectType);

		prefabObj.AddMember("id", rapidjson::Value(id.c_str(), a), a);
		prefabObj.AddMember("name", rapidjson::Value(prefab.name.c_str(), a), a);
		prefabObj.AddMember("path", rapidjson::Value(prefab.path.c_str(), a), a);

		prefabArray.PushBack(prefabObj, a);
	}

	doc.AddMember("prefabIndex", prefabArray, a);

	std::string err;
	// write to run time file
	std::string runtime = std::string(RUNTIME_DIR_R) + registryFile;
	std::string dir = std::string(SOURCE_DIR_R) + registryFile;

	JsonIO::WriteDocumentToFile(runtime, doc, true, &err);
	JsonIO::WriteDocumentToFile(dir, doc, true, &err);

	std::cout << "Prefab registry saved at " << registryFile << "\n";
}

const std::string& PrefabManager::getPrefabPath(const std::string& prefabID) const {
	return m_prefabRegistry.at(prefabID).path;
}

bool PrefabManager::setPrefabPath(const std::string& prefabID, const std::string& newName) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		DebugLog::addMessage("Prefab ID " + prefabID + "not found.");
		return false;
	}

	std::filesystem::path path{ newName };
	std::string newNameWPath = path.filename().string();

	iterator->second.path = "/Prefab/" + newNameWPath;
	savePrefabRegistry();

	DebugLog::addMessage("Updated prefab path for ID " + prefabID + " -> " + newNameWPath);
	return true;
}

std::string PrefabManager::getPrefabFilename(const std::string& prefabID) const {
	auto it = m_prefabRegistry.find(prefabID);
	if (it == m_prefabRegistry.end()) {
		DebugLog::addMessage("Error: Prefab ID '" + prefabID + "' not found.");
		return {};
	}

	// extract filename from path
	return std::filesystem::path(it->second.path).filename().string();  // e.g. "platform.json"
}

std::string& PrefabManager::getPrefabName(const std::string& prefabID) {
	return m_prefabRegistry[prefabID].name;
}

const std::string& PrefabManager::getPrefabName(const std::string& prefabID) const {
	return m_prefabRegistry.at(prefabID).name;
}

std::string PrefabManager::findPrefabIDByFilename(const std::filesystem::path& filename) {
	std::string f{ filename.filename().string() };

	for (const auto& [id, info] : m_prefabRegistry) {
		if (std::filesystem::path(info.path).filename().string() == f)
			return id;
	}

	return "";
}

const rapidjson::Document* PrefabManager::getPrefabJson(const std::string& prefabID) const {
	auto iterator = m_prefabCache.find(prefabID);

	return (iterator != m_prefabCache.end()) ? &iterator->second : nullptr;
}

const rapidjson::Value* PrefabManager::getPrefabJsonComp(const rapidjson::Document* prefabDoc, const std::string& compName) const {
	// if prefab dont have components then return
	if (!prefabDoc || !prefabDoc->HasMember("components")) return nullptr;

	// if prefab dont have the given components
	const auto& comps = (*prefabDoc)["components"];

	return comps.HasMember(compName.c_str()) ? &comps[compName.c_str()] : nullptr;
}

GameObject* PrefabManager::instantiate(GameObject* go, GameObjectManager& manager) {
	const std::string& id = go->getObjectPrefabID();
	auto iterator = m_prefabRegistry.find(id);
	if (iterator == m_prefabRegistry.end() || id.empty()) {
		DebugLog::addMessage("PrefabManager::instantiate() failed: prefab ID not found: " + id);
		return nullptr;
	}

	//const std::string& path = iterator->second.path;
	const std::string& path = std::string(RUNTIME_DIR_R) + iterator->second.path;

	rapidjson::Document doc;
	auto cacheIterator = m_prefabCache.find(id);
	if (cacheIterator != m_prefabCache.end()) {
		// if json is loaded previously, use it
		doc.CopyFrom(cacheIterator->second, doc.GetAllocator());
	}
	else {
		// if json never load before, read from disk
		std::string err;
		if (!JsonIO::ReadFileToDocument(path, doc, &err)) {
			DebugLog::addMessage("Failed to load prefab file '" + path + "': " + err);
			return nullptr;
		}

		m_prefabCache[id].CopyFrom(doc, m_prefabCache[id].GetAllocator());
	}

	if (doc.HasMember("layer") && doc["layer"].IsInt()) {
		int layerID = doc["layer"].GetInt();
		manager.assignObjectToLayer(go, layerID);
	}

	if (doc.HasMember("components") && doc["components"].IsObject()) {
		applyPrefabComponents(go, doc["components"]);
	}

	return go;
}

std::unique_ptr<GameObject> PrefabManager::createTempPrefabObj(const std::string& prefabID) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		DebugLog::addMessage("Prefab not found in registry.");
		return nullptr;
	}

	const std::string& path = std::string(RUNTIME_DIR_R) + iterator->second.path;

	rapidjson::Document doc;
	auto cacheIterator = m_prefabCache.find(prefabID);
	if (cacheIterator != m_prefabCache.end()) {
		// if json is loaded previously, use it
		doc.CopyFrom(cacheIterator->second, doc.GetAllocator());
	}
	else {
		// if json never load before, read from disk
		std::string err;
		if (!JsonIO::ReadFileToDocument(path, doc, &err)) {
			DebugLog::addMessage("Failed to load prefab file '" + path + "': " + err);
			return nullptr;
		}

		m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());
	}

	if (!doc.HasMember("components") || !doc["components"].IsObject()) {
		DebugLog::addMessage("Prefab file '" + path + "' missing COMPONENT object.");
		return nullptr;
	}

	// cache original prefab state before editing (for comparison after saving later)
	m_prefabBeforeEdit[prefabID].CopyFrom(doc, m_prefabBeforeEdit[prefabID].GetAllocator());

	// create a new temp obj
	std::unique_ptr<GameObject> tempGO = GameObjectManager::createTempGameObject(iterator->second.name);
	tempGO->getObjectPrefabID() = prefabID;

	if (doc.HasMember("layer") && doc["layer"].IsInt()) {
		int layerID = doc["layer"].GetInt();
		// store the layer value, do not register!!!
		tempGO->setLayer(layerID);
	}

	// apply prefab changes on the obj
	applyPrefabComponents(tempGO.get(), doc["components"]);

	return tempGO;
}

bool PrefabManager::savePrefab(const GameObject* prefabObj) {
	if (!prefabObj) return false;

	const std::string& prefabID = prefabObj->getObjectPrefabID();
	if (prefabID.empty()) return false;

	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	// prefab id
	doc.AddMember("id", rapidjson::Value(prefabID.c_str(), a), a);
	// name
	doc.AddMember("name", rapidjson::Value(prefabObj->getObjectName().c_str(), a), a);

	int layerID = prefabObj->getLayer();
	if (layerID != -1 && layerID != 1) {  // Only save if not default game layer
		doc.AddMember("layer", layerID, a);
	}

	// components container
	rapidjson::Value comps(rapidjson::kObjectType);
	serializeComponents(prefabObj, comps, a);

	doc.AddMember("components", comps, a);

	// save to runtime & source
	std::string err;
	JsonIO::WriteDocumentToFile(std::string(RUNTIME_DIR_R) + getPrefabPath(prefabID), doc, true, &err);
	JsonIO::WriteDocumentToFile(std::string(SOURCE_DIR_R) + getPrefabPath(prefabID), doc, true, &err);

	// update cache
	m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());

	// update registry incase of name change
	savePrefabRegistry();

	return true;
}

GameObject* PrefabManager::revertToPrefab(GameObject* obj, GameObjectManager& manager) {
	if (!obj || obj->getObjectPrefabID().empty()) return obj;

	// store scene data first (preserve position)
	Transform* t = obj->getComponent<Transform>();
	float posX = t ? t->x : 0.f;
	float posY = t ? t->y : 0.f;
	float posZ = t ? t->z : 0.f;

	std::string name = obj->getObjectName();
	int layer = obj->getLayer();

	// remove existing components
	removeAllComponents(obj);

	// reapply prefab
	instantiate(obj, manager);

	// restore the position
	if (Transform* newT = obj->getComponent<Transform>()) {
		newT->x = posX;
		newT->y = posY;
		newT->z = posZ;
	}

	obj->getObjectName() = name;
	manager.assignObjectToLayer(obj, layer);

	// reiniatialize
	manager.initializeSceneResources();

	return obj;
}

// apply prefab changes to all existing instances in current scene
int PrefabManager::applyToAllInstances(const std::string& prefabID, GameObjectManager& manager) {
	if (prefabID.empty()) return 0;

	auto oldI = m_prefabBeforeEdit.find(prefabID); // before edit
	auto newI = m_prefabCache.find(prefabID); // aft edit and save

	if (oldI == m_prefabBeforeEdit.end()) {
		// use current cache as old prefab
		if (newI != m_prefabCache.end()) {
			m_prefabBeforeEdit[prefabID].CopyFrom(newI->second, m_prefabBeforeEdit[prefabID].GetAllocator());
			oldI = m_prefabBeforeEdit.find(prefabID);
		}
		else {
			// load from disk as fallback
			const std::string& path = std::string(RUNTIME_DIR_R) + getPrefabPath(prefabID);
			rapidjson::Document diskDoc;
			std::string err;

			if (JsonIO::ReadFileToDocument(path, diskDoc, &err)) {
				m_prefabBeforeEdit[prefabID].CopyFrom(diskDoc, m_prefabBeforeEdit[prefabID].GetAllocator());
				oldI = m_prefabBeforeEdit.find(prefabID);
			}
		}
	}

	// if no cached before editing state found for prefab, fall back to original obj state
	if (oldI == m_prefabBeforeEdit.end()) {
		// PLEASE DONT HAPPEN
		DebugLog::addMessage("No cached state found for prefab.");
		return applyAllInstancesFallback(prefabID, manager);
	}

	auto* oldComps = JsonIO::GetObj(oldI->second, "components");
	auto* newComps = JsonIO::GetObj(newI->second, "components");
	if (!oldComps || !newComps) return 0;

	// get layer data
	int oldLayer = JsonIO::GetIntOr(oldI->second, "layer", 1);
	int newLayer = JsonIO::GetIntOr(newI->second, "layer", 1);

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	std::vector<GameObject*> instances;
	for (GameObject* obj : gameObjects) {
		if (obj->getObjectPrefabID() == prefabID)
			instances.push_back(obj);
	}

	int count{};

	// apply to all instances (that ref the prefab)
	for (GameObject* obj : instances) {
		if (obj->getObjectPrefabID() != prefabID) continue;

		std::string objName = obj->getObjectName();
		int objLayer = obj->getLayer();

		// apply component changes with override checking
		applyPrefabComponentsSelective(obj, *oldComps, *newComps);

		// restore its name
		obj->getObjectName() = objName;
		manager.assignObjectToLayer(obj, (objLayer == oldLayer) ? newLayer : objLayer);

		//if (Render* r = obj->getComponent<Render>()) {
		//	if (r->hasTex && !r->texFile.empty()) {
		//		r->texChanged = true;
		//	}
		//}

		++count;
	}

	manager.initializeSceneResources();
	m_prefabBeforeEdit[prefabID].CopyFrom(newI->second, m_prefabBeforeEdit[prefabID].GetAllocator());
	//m_prefabBeforeEdit.erase(prefabID);

	return count;
}

int PrefabManager::applyAllInstancesFallback(const std::string& prefabID, GameObjectManager& manager) {
	auto newI = m_prefabCache.find(prefabID);
	if (newI == m_prefabCache.end()) return 0;

	const rapidjson::Document& newPrefabDoc = newI->second;
	if (!newPrefabDoc.HasMember("components")) return 0;

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	int count = 0;
	for (GameObject* obj : gameObjects) {
		if (obj->getObjectPrefabID() != prefabID) continue;

		// Serialize current state
		rapidjson::Document objDoc;
		objDoc.SetObject();
		auto& a = objDoc.GetAllocator();

		rapidjson::Value comps(rapidjson::kObjectType);
		serializeComponents(obj, comps, a);
		objDoc.AddMember("components", comps, a);

		// clear all components
		removeAllComponents(obj);

		// apply prefab first
		applyPrefabComponents(obj, newPrefabDoc["components"]);

		// override with scene data
		if (objDoc.HasMember("components")) {
			applyPrefabComponents(obj, objDoc["components"]);
		}

		if (Render* r = obj->getComponent<Render>()) {
			if (r->hasTex && !r->texFile.empty()) r->texChanged = true;
		}

		++count;
	}

	manager.initializeSceneResources();
	return count;
}

bool PrefabManager::createPrefabFromGameObj(GameObject* go, const std::string& name, bool makeRef) {
	if (!go) return false;

	std::string prefabID = generateUUID();

	// if Prefab folder not created yet, create one
	std::filesystem::path preFabDir = SOURCE_DIR_R + prefabFolder;
	if (!std::filesystem::exists(preFabDir)) {
		std::filesystem::create_directories(preFabDir);
	}

	// if name is not given, use the name of the game obj
	std::string prefabName = (name.empty()) ? go->getObjectName() : name;
	std::string filename = prefabFolder + "/" + prefabName + ".json";
	std::filesystem::path prefabPath = preFabDir / prefabName;

	auto pathExistsInRegistry = [this](const std::string& path) -> bool {
		for (const auto& [id, info] : m_prefabRegistry) {
			if (info.path == path) return true;
		}
		return false;
		};

	// Rename if path already registered
	if (pathExistsInRegistry(filename)) {
		int counter = 1;
		std::string newFilename;
		do {
			newFilename = prefabFolder + "/" + prefabName + "_" + std::to_string(counter) + ".json";
			counter++;
		} while (pathExistsInRegistry(newFilename));
		filename = newFilename;
	}

	m_prefabRegistry[prefabID] = { prefabName, filename };

	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	// prefab id
	doc.AddMember("id", rapidjson::Value(prefabID.c_str(), a), a);
	// name
	doc.AddMember("name", rapidjson::Value(prefabName.c_str(), a), a);

	int layerID = go->getLayer();
	if (layerID != -1 && layerID != 1) {
		doc.AddMember("layer", layerID, a);
	}

	rapidjson::Value comps(rapidjson::kObjectType);
	serializeComponents(go, comps, a);

	doc.AddMember("components", comps, a);

	/* -------- Save new prefab to a new .json file ------- */
	std::string err;
	JsonIO::WriteDocumentToFile(std::string(RUNTIME_DIR_R) + filename, doc, true, &err);
	JsonIO::WriteDocumentToFile(std::string(SOURCE_DIR_R) + filename, doc, true, &err);
	/* -------- END ------- */

	// save in prefab registry file
	savePrefabRegistry();

	// set this game object to be reference to the created prefab
	if (makeRef) go->getObjectPrefabID() = prefabID;

	// update cache
	m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());

	return true;
}

bool PrefabManager::deletePrefab(const std::string prefabID) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		DebugLog::addMessage("PrefabManager::deleteprefab: failed � prefab ID not found: " + prefabID);
		return false;
	}

	// remove prefab from prefab registry and prefab cache
	m_prefabRegistry.erase(iterator);
	m_prefabCache.erase(prefabID);

	// save updated prefab registry to file
	savePrefabRegistry();

	return true;
}

std::string PrefabManager::generateUUID() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned> dis1(0, 15); // 0 - F
	std::uniform_int_distribution<unsigned> dis2(8, 11); // variant

	std::stringstream uuid;
	int i;
	for (i = 0; i < 8; ++i) uuid << std::hex << dis1(gen); // first 8 hex
	uuid << "-";
	for (i = 0; i < 4; ++i) uuid << std::hex << dis1(gen); // next 4 digits
	uuid << "-";
	uuid << 4; // version 4
	for (i = 0; i < 3; ++i) uuid << std::hex << dis1(gen); // next 3 digits
	uuid << "-";
	uuid << std::hex << dis2(gen); // variant
	for (i = 0; i < 3; ++i) uuid << std::hex << dis1(gen); // next 3 digits
	uuid << "-";
	for (i = 0; i < 12; ++i) uuid << std::hex << dis1(gen); // last 12 digits

	return uuid.str();
}

void PrefabManager::applyPrefabComponents(GameObject* go, const rapidjson::Value& comps) {
	//std::cout << go->getObjectName() << "data\n\n";
	// ---- Transform ----
	if (comps.HasMember("Transform") && comps["Transform"].IsObject()) {
		const auto& jt = comps["Transform"];
		auto* t = go->addComponent<Transform>();

		if (jt.HasMember("pos") && jt["pos"].IsArray() && jt["pos"].Size() == 3) {
			t->x = jt["pos"][0].GetFloat();
			t->y = jt["pos"][1].GetFloat();
			t->z = jt["pos"][2].GetFloat();

			//std::cout << t->x << " " << t->y << " " << t->z << "pos\n\n";
		}
		if (jt.HasMember("rotation") && jt["rotation"].IsNumber()) {
			t->rotation = jt["rotation"].GetFloat();

			//std::cout << t->rotation << "rotation\n\n";
		}
		if (jt.HasMember("scale") && jt["scale"].IsArray() && jt["scale"].Size() == 3) {
			t->scaleX = jt["scale"][0].GetFloat();
			t->scaleY = jt["scale"][1].GetFloat();
			t->scaleZ = jt["scale"][2].GetFloat();

			//std::cout << t->scaleX << " " << t->scaleY << " " << t->scaleZ << "scale\n\n";
		}
	}

	// ---- Render ----
	if (comps.HasMember("Render") && comps["Render"].IsObject()) {
		const auto& jr = comps["Render"];
		auto* r = go->addComponent<Render>();

		// shape
		std::string shp = (jr.HasMember("shape") && jr["shape"].IsString())
			? jr["shape"].GetString()
			: "square";
		r->modelRef = renderer::models[static_cast<int>(
			JsonIO::GetString(jr, "shape", shp) ? JsonIO::StrToShape(shp) : shape::square)];

		// hasTex / texture
		r->hasTex = JsonIO::GetBoolOr(jr, "hasTex", false);
		if (r->hasTex && JsonIO::GetString(jr, "texture", r->texFile)) {
			r->texHDL = 0;
			r->texChanged = false;
		}

		r->hasAnimation = JsonIO::GetBoolOr(jr, "hasAnimation", false);
		JsonIO::ReadVec3(jr, "clr", r->clr.r, r->clr.g, r->clr.b);

		/*std::cout << r->hasTex << " has tex\n\n";
		std::cout << r->hasAnimation << " has animation\n\n";*/
	}

	// ---- Input (presence only) ----
	if (comps.HasMember("Input") && comps["Input"].IsObject()) {
		go->addComponent<Input>();
	}

	// ---- Physics ----
	if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
		const auto& jp = comps["Physics"];
		auto* p = go->addComponent<Physics>();

		p->physicsFlag = JsonIO::GetBoolOr(jp, "physicsFlag", true);
		p->moveSpeed = JsonIO::GetFloatOr(jp, "moveSpeed", 0.f);
		p->damping = JsonIO::GetFloatOr(jp, "damping", 0.f);
		p->jumpForce = JsonIO::GetFloatOr(jp, "jumpForce", 0.f);
		p->dynamics.mass = JsonIO::GetFloatOr(jp, "mass", 1.f);
		p->inWater = JsonIO::GetBoolOr(jp, "inWater", false);
		p->buoancy = JsonIO::GetBoolOr(jp, "buoyancy", false);
	}

	// ---- CollisionInfo (presence only) ----
	if (comps.HasMember("CollisionInfo") && comps["CollisionInfo"].IsObject()) {
		const auto& jc = comps["CollisionInfo"];
		auto* c = go->addComponent<CollisionInfo>();

		c->collisionFlag = JsonIO::GetBoolOr(jc, "collisionFlag", true);
		c->autoFitScale = JsonIO::GetBoolOr(jc, "autoFitScale", false);
		std::string shp;
		c->colliderType = JsonIO::GetString(jc, "colliderType", shp) ? JsonIO::StrToShape(shp) : shape::square;
		std::string res;
		c->collisionRes = JsonIO::GetString(jc, "collisionRes", res) ? JsonIO::StrToCollisionResponseMode(res) : CollisionResponseMode::StopWhenCollide;
		if (jc.HasMember("colliderSize") && jc["colliderSize"].IsArray() && jc["colliderSize"].Size() >= 2) {
			c->colliderSize.x = jc["colliderSize"][0].GetFloat();
			c->colliderSize.y = jc["colliderSize"][1].GetFloat();
		}
	}

	// ---- State Machine ----
	if (comps.HasMember("StateMachine") && comps["StateMachine"].IsObject()) {
		const auto& smj = comps["StateMachine"];
		auto* sm = go->addComponent<StateMachine>();

		std::string state;
		sm->state = /*JsonIO::GetString(smj, "state", state) ? parsePlayerState(state) :*/ PlayerState::Idle;
		sm->facingRight = JsonIO::GetBoolOr(smj, "facingRight", true);
		sm->stateTime = JsonIO::GetFloatOr(smj, "stateTime", 0.f);

		/* ---- Animation based on state ---- */
			// Helper lambda to parse a single animation state
		auto parseAnimState = [](const rapidjson::Value& animJson) -> AnimateState {
			AnimateState state;

			if (animJson.HasMember("texture") && animJson["texture"].IsString())
				state.texFile = animJson["texture"].GetString();

			if (animJson.HasMember("loop")) {
				if (animJson["loop"].IsBool())
					state.loop = animJson["loop"].GetBool();
				else if (animJson["loop"].IsInt())
					state.loop = (animJson["loop"].GetInt() != 0);
			}

			if (animJson.HasMember("totalColumn") && animJson["totalColumn"].IsInt())
				state.totalColumn = animJson["totalColumn"].GetInt();

			if (animJson.HasMember("totalRow") && animJson["totalRow"].IsInt())
				state.totalRow = animJson["totalRow"].GetInt();

			if (animJson.HasMember("frameTime") && animJson["frameTime"].IsNumber())
				state.frameTime = static_cast<float>(animJson["frameTime"].GetDouble());

			if (animJson.HasMember("initialFramCol") && animJson["initialFramCol"].IsNumber())
				state.initialFrame.x = static_cast<float>(animJson["initialFramCol"].GetDouble());

			if (animJson.HasMember("initialFramRow") && animJson["initialFramRow"].IsNumber())
				state.initialFrame.y = static_cast<float>(animJson["initialFramRow"].GetDouble());

			if (animJson.HasMember("lastFramCol") && animJson["lastFramCol"].IsNumber())
				state.lastFrame.x = static_cast<float>(animJson["lastFramCol"].GetDouble());

			if (animJson.HasMember("lastFramRow") && animJson["lastFramRow"].IsNumber())
				state.lastFrame.y = static_cast<float>(animJson["lastFramRow"].GetDouble());

			return state;
			};

		// parse each state's animation in the order: Idle, Walking, Jumping, Falling, Shooting
		// match the PlayerState enum order
		std::vector<std::string>stateNames = JsonIO::stateNames;

		bool hasAnyAnimState = false;
		for (size_t i = 0; i < stateNames.size(); ++i) {
			if (smj.HasMember(stateNames[i].c_str()) && smj[stateNames[i].c_str()].IsObject()) {
				const rapidjson::Value& stateObj = smj[stateNames[i].c_str()];
				if (stateObj.HasMember("animState") && stateObj["animState"].IsObject()) {
					hasAnyAnimState = true;
					break;
				}
			}
		}

		// Add Animation component if needed
		Animation* anim = nullptr;
		if (hasAnyAnimState) {
			anim = go->addComponent<Animation>();
			anim->animState.resize(stateNames.size());

			// Parse each state's animation
			for (size_t i = 0; i < stateNames.size(); ++i) {
				if (smj.HasMember(stateNames[i].c_str()) && smj[stateNames[i].c_str()].IsObject()) {
					const rapidjson::Value& stateObj = smj[stateNames[i].c_str()];
					if (stateObj.HasMember("animState") && stateObj["animState"].IsObject()) {
						AnimateState parsed = parseAnimState(stateObj["animState"]);
						anim->animState[i] = parsed;
						anim->animState[i].texChanged = true;
					}
				}
			}
		}
	}

	// ---- TileMapComponent ----
	if (comps.HasMember("TileMap") && comps["TileMap"].IsObject())
	{
		const auto& tmj = comps["TileMap"];
		auto* tm = go->addComponent<TileMap>();

		if (tmj.HasMember("tileW") && tmj["tileW"].IsNumber())
			tm->tileW = tmj["tileW"].GetFloat();

		if (tmj.HasMember("tileH") && tmj["tileH"].IsNumber())
			tm->tileH = tmj["tileH"].GetFloat();

		if (tmj.HasMember("columns") && tmj["columns"].IsNumber())
			tm->columns = tmj["columns"].GetInt();

		if (tmj.HasMember("rows") && tmj["rows"].IsNumber())
			tm->rows = tmj["rows"].GetInt();

		// --- IMPORTANT: Deserialize actual tile data ---
		tm->tiles.clear();  // wipe previous tile data

		if (tmj.HasMember("tiles") && tmj["tiles"].IsArray())
		{
			for (const auto& tile : tmj["Tiles"].GetArray())
			{
				if (!tile.IsObject()) continue;

				int x = tile.HasMember("x") ? tile["x"].GetInt() : 0;
				int y = tile.HasMember("y") ? tile["y"].GetInt() : 0;
				std::string id = tile["id"].GetString();

				tm->setTile(x, y, id);
			}
		}
	}

	// ---- AudioComponent ----
	if (comps.HasMember("AudioComponent") && comps["AudioComponent"].IsObject()) {
		const auto& ja = comps["AudioComponent"];
		auto* audio = go->addComponent<AudioComponent>();

		AudioChannel* ac = audio->getDefaultChannel();

		JsonIO::GetString(ja, "audioFile", ac->audioFile);
		ac->volume = JsonIO::GetFloatOr(ja, "volume", 1.f);
		ac->pitch = JsonIO::GetFloatOr(ja, "pitch", 1.f);
		ac->loop = JsonIO::GetBoolOr(ja, "loop", false);
		ac->playOnStart = JsonIO::GetBoolOr(ja, "playOnStart", false);
		ac->muted = JsonIO::GetBoolOr(ja, "muted", false);
		ac->fadeInOnStart = JsonIO::GetBoolOr(ja, "fadeInOnStart", false);
		ac->fadeInDuration = JsonIO::GetFloatOr(ja, "fadeInDuration", 0.f);
		ac->fadeOutOnStop = JsonIO::GetBoolOr(ja, "fadeOutOnStop", false);
		ac->fadeOutDuration = JsonIO::GetFloatOr(ja, "fadeOutDuration", 0.f);
	}

	// ---- FontComponent ----
	if (comps.HasMember("FontComponent") && comps["FontComponent"].IsObject()) {
		const auto& jf = comps["FontComponent"];
		auto* fc = go->addComponent<FontComponent>();

		JsonIO::GetString(jf, "word", fc->word);
		fc->scale = JsonIO::GetFloatOr(jf, "scale", 1.f);
		fc->fontType = JsonIO::GetIntOr(jf, "fontType", 0);
		JsonIO::ReadVec3(jf, "color", fc->clr.r, fc->clr.g, fc->clr.b);
	}
}

void PrefabManager::serializeComponents(const GameObject* obj, rapidjson::Value& comps, rapidjson::Document::AllocatorType& a) {
	//// ---- Transform ----
	if (auto t = obj->getComponent<Transform>()) {
		rapidjson::Value jt(rapidjson::kObjectType);

		JsonIO::WriteVec3(jt, "pos", t->x, t->y, t->z, a);
		jt.AddMember("rotation", t->rotation, a);
		JsonIO::WriteVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ, a);

		comps.AddMember("Transform", jt, a);
	}

	//// ---- Render ----
	if (auto r = obj->getComponent<Render>()) {
		rapidjson::Value jr(rapidjson::kObjectType);

		jr.AddMember("shape", rapidjson::Value(JsonIO::ShapeToStr(r->modelRef.shape), a), a);
		jr.AddMember("hasTex", r->hasTex ? 1 : 0, a);
		if (r->hasTex && !r->texFile.empty())
			jr.AddMember("texture", rapidjson::Value(r->texFile.c_str(), a), a);
		jr.AddMember("hasAnimation", r->hasAnimation ? 1 : 0, a);
		if (r->clr.r != 1.f || r->clr.g != 1.f || r->clr.b != 1.f) {
			JsonIO::WriteVec3(jr, "clr", r->clr.r, r->clr.g, r->clr.b, a);
		}

		comps.AddMember("Render", jr, a);
	}

	if (auto i = obj->getComponent<Input>()) {
		comps.AddMember("Input", rapidjson::Value(rapidjson::kObjectType), a);
	}

	//// ---- Physics ----
	if (auto p = obj->getComponent<Physics>()) {
		rapidjson::Value jp(rapidjson::kObjectType);

		jp.AddMember("physicsFlag", p->physicsFlag ? 1 : 0, a);

		jp.AddMember("moveSpeed", p->moveSpeed, a);
		jp.AddMember("jumpForce", p->jumpForce, a);
		jp.AddMember("damping", p->damping, a);
		jp.AddMember("mass", p->dynamics.mass, a);

		jp.AddMember("inWater", p->inWater ? 1 : 0, a);
		jp.AddMember("buoyancy", p->buoancy ? 1 : 0, a);

		comps.AddMember("Physics", jp, a);
	}

	//// ---- Collision ----
	if (auto c = obj->getComponent<CollisionInfo>()) {
		rapidjson::Value jc(rapidjson::kObjectType);

		jc.AddMember("collisionFlag", c->collisionFlag ? 1 : 0, a);
		jc.AddMember("autoFitScale", c->autoFitScale ? 1 : 0, a);

		JsonIO::PutArray3f(jc, "colliderSize", { c->colliderSize.x, c->colliderSize.y, 0.f }, a);

		jc.AddMember("colliderType", rapidjson::Value(JsonIO::ShapeToStr(c->colliderType), a), a);

		jc.AddMember("collisionRes", rapidjson::Value(JsonIO::CollisionResponseModeToStr(c->collisionRes), a), a);

		comps.AddMember("CollisionInfo", jc, a);
	}

	//// ---- StateMachine ----
	if (auto sm = obj->getComponent<StateMachine>()) {
		rapidjson::Value js(rapidjson::kObjectType);

		js.AddMember("state", rapidjson::Value(playerStateToStr(sm->state), a), a);
		js.AddMember("facingRight", sm->facingRight ? 1 : 0, a);
		js.AddMember("stateTime", sm->stateTime, a);

		/* ------ Animation for states ----- */
		if (auto anim = obj->getComponent<Animation>()) {
			std::vector<std::string> stateNames = JsonIO::stateNames;
			for (size_t i = 0; i < stateNames.size() && i < anim->animState.size(); ++i) {
				const AnimateState& animState = anim->animState[i];

				rapidjson::Value stateObj(rapidjson::kObjectType);
				rapidjson::Value animStateObj(rapidjson::kObjectType);

				animStateObj.AddMember("texture", rapidjson::Value(animState.texFile.c_str(), a), a);
				animStateObj.AddMember("loop", rapidjson::Value(animState.loop ? 1 : 0), a);
				animStateObj.AddMember("totalColumn", rapidjson::Value(animState.totalColumn), a);
				animStateObj.AddMember("totalRow", rapidjson::Value(animState.totalRow), a);
				animStateObj.AddMember("frameTime", rapidjson::Value(animState.frameTime), a);
				animStateObj.AddMember("initialFramCol", rapidjson::Value(animState.initialFrame.x), a);
				animStateObj.AddMember("initialFramRow", rapidjson::Value(animState.initialFrame.y), a);
				animStateObj.AddMember("lastFramCol", rapidjson::Value(animState.lastFrame.x), a);
				animStateObj.AddMember("lastFramRow", rapidjson::Value(animState.lastFrame.y), a);
				stateObj.AddMember("animState", animStateObj, a);

				js.AddMember(rapidjson::Value(stateNames[i].c_str(), a), stateObj, a);
			}
		}
		/* ------ END ------ */

		comps.AddMember("StateMachine", js, a);
	}

	// AudioComponent
	if (auto audio = obj->getComponent<AudioComponent>()) {
		rapidjson::Value ja(rapidjson::kObjectType);

		const AudioChannel* ac = audio->getChannel("default");

		// CRITICAL FIX: Only serialize if "default" channel exists
		if (ac) {
			ja.AddMember("audioFile", rapidjson::Value(ac->audioFile.c_str(), a), a);
			ja.AddMember("volume", ac->volume, a);
			ja.AddMember("pitch", ac->pitch, a);
			ja.AddMember("loop", ac->loop ? 1 : 0, a);
			ja.AddMember("playOnStart", ac->playOnStart ? 1 : 0, a);
			ja.AddMember("muted", ac->muted ? 1 : 0, a);
			ja.AddMember("fadeInOnStart", ac->fadeInOnStart ? 1 : 0, a);
			ja.AddMember("fadeInDuration", ac->fadeInDuration, a);
			ja.AddMember("fadeOutOnStop", ac->fadeOutOnStop ? 1 : 0, a);
			ja.AddMember("fadeOutDuration", ac->fadeOutDuration, a);

			comps.AddMember("AudioComponent", ja, a);
		}
	}

	// FontComponent
	if (auto fc = obj->getComponent<FontComponent>()) {
		rapidjson::Value jf(rapidjson::kObjectType);
		jf.AddMember("word", rapidjson::Value(fc->word.c_str(), a), a);
		jf.AddMember("scale", fc->scale, a);
		jf.AddMember("fontType", fc->fontType, a);
		rapidjson::Value clr(rapidjson::kArrayType);
		clr.PushBack(fc->clr.r, a).PushBack(fc->clr.g, a).PushBack(fc->clr.b, a);
		jf.AddMember("color", clr, a);
		comps.AddMember("FontComponent", jf, a);
	}

	// TileMapComponent
	if (auto tm = obj->getComponent<TileMap>())
	{
		rapidjson::Value jt(rapidjson::kObjectType);
		jt.AddMember("tileW", tm->tileW, a);
		jt.AddMember("tileH", tm->tileH, a);
		jt.AddMember("columns", tm->columns, a);
		jt.AddMember("rows", tm->rows, a);

		// Create tiles array
		rapidjson::Value tileArray(rapidjson::kArrayType);

		for (const auto& [key, id] : tm->tiles)
		{
			rapidjson::Value tileObj(rapidjson::kObjectType);

			tileObj.AddMember("x", key.x, a);
			tileObj.AddMember("y", key.y, a);
			tileObj.AddMember("id", rapidjson::Value(id.c_str(), a), a);

			tileArray.PushBack(tileObj, a);
		}

		jt.AddMember("tiles", tileArray, a);

		// IMPORTANT: Add TileMap to components JSON
		comps.AddMember("TileMap", jt, a);
	}
}

bool PrefabManager::valuesEqual(const rapidjson::Value& lhs, const rapidjson::Value& rhs) const {
	// make sure lhs and rhs are the same type before continue
	if (lhs.GetType() != rhs.GetType()) return false;

	if (lhs.IsNull()) return rhs.IsNull();

	if (lhs.IsBool()) return lhs.GetBool() == rhs.GetBool();

	if (lhs.IsInt()) return lhs.GetInt() == rhs.GetInt();

	if (lhs.IsDouble()) {
		return std::abs(lhs.GetDouble() - rhs.GetDouble()) < 0.0001;
	}

	if (lhs.IsString()) {
		return std::strcmp(lhs.GetString(), rhs.GetString()) == 0;
	}

	if (lhs.IsArray()) {
		if (lhs.Size() != rhs.Size()) return false;
		for (rapidjson::SizeType i = 0; i < lhs.Size(); ++i) {
			if (!valuesEqual(lhs[i], rhs[i])) return false;
		}
		return true;
	}

	if (lhs.IsObject()) {
		if (lhs.MemberCount() != rhs.MemberCount()) return false;
		for (auto it = lhs.MemberBegin(); it != lhs.MemberEnd(); ++it) {
			if (!rhs.HasMember(it->name.GetString())) return false;
			if (!valuesEqual(it->value, rhs[it->name.GetString()])) return false;
		}
		return true;
	}

	return false;
}

void PrefabManager::applyPrefabComponentsSelective(GameObject* obj, const rapidjson::Value& oldPrefabComps, const rapidjson::Value& newPrefabComps) {
	rapidjson::Document objDoc;
	objDoc.SetObject();
	auto& a = objDoc.GetAllocator();

	rapidjson::Value objComps(rapidjson::kObjectType);
	serializeComponents(obj, objComps, a);

	// track which components exist in new prefab
	std::unordered_set<std::string> newPrefabCompSet;
	for (auto it = newPrefabComps.MemberBegin(); it != newPrefabComps.MemberEnd(); ++it) {
		newPrefabCompSet.insert(it->name.GetString());
	}

	// remove component that no longer in new prefab (means removed) if not overridden
	for (auto i = objComps.MemberBegin(); i != objComps.MemberEnd(); ++i) {
		const char* compName = i->name.GetString();

		if (oldPrefabComps.HasMember(compName) && !newPrefabCompSet.count(compName)) {
			// check if instance has overridden this component
			const rapidjson::Value& objCompData = i->value;
			const rapidjson::Value& oldPrefabCompData = oldPrefabComps[compName];

			// if instance data match old prefab, NOT overridden
			bool wasOverridden = !valuesEqual(objCompData, oldPrefabCompData);

			if (!wasOverridden) {
				// remove if instance no override prefab
				removeComponentByName(obj, compName);
				/*DebugLog::addMessage(std::string("Removed component '") + compName + "' from instance (removed from prefab)");*/
			}
			else {
				// instance has override data, keep it
				/*DebugLog::addMessage(std::string("Kept overridden component '") + compName + "' on instance despite prefab removal");*/
			}
		}
	}


	/* ---------- Special Case -> Animation Component check ---------- */
	// check if Animation was removed from the prefab StateMachine
	bool oldPrefabHadAnimation = false;
	bool newPrefabHasAnimation = false;

	// check if old prefab has a statemachine
	if (oldPrefabComps.HasMember("StateMachine") && oldPrefabComps["StateMachine"].IsObject()) {
		const auto& oldSM = oldPrefabComps["StateMachine"];

		// check if any state has animState
		for (const auto& stateName : JsonIO::stateNames) {
			if (oldSM.HasMember(stateName.c_str()) && oldSM[stateName.c_str()].IsObject()) {
				const auto& stateObj = oldSM[stateName.c_str()];

				if (stateObj.HasMember("animState") && stateObj["animState"].IsObject()) {
					oldPrefabHadAnimation = true;
					break;
				}

			}
		}
	}

	// check if new prefab has a statemachine
	if (newPrefabComps.HasMember("StateMachine") && newPrefabComps["StateMachine"].IsObject()) {
		const auto& newSM = newPrefabComps["StateMachine"];
		std::vector<std::string> stateNames = JsonIO::stateNames;
		for (const auto& stateName : stateNames) {
			if (newSM.HasMember(stateName.c_str()) && newSM[stateName.c_str()].IsObject()) {
				const auto& stateObj = newSM[stateName.c_str()];

				if (stateObj.HasMember("animState") && stateObj["animState"].IsObject()) {
					newPrefabHasAnimation = true;
					break;
				}

			}
		}
	}

	// check if object matches old prefab (no override)
	/*
	  for now just assume that if both instance and old prefab has it, means its not
	  overriden (means if animation is removed in prefab -> instance also no more)
	*/
	bool noOverride = false;
	if (obj->getComponent<Animation>() && oldPrefabHadAnimation && objComps.HasMember("StateMachine")) {
		noOverride = true;
	}

	// remove Animation if removed from prefab and obj didnt override it
	if (oldPrefabHadAnimation && !newPrefabHasAnimation && noOverride) {
		obj->removeComponent<Animation>();
		/*DebugLog::addMessage("Removed Animation component from instance (removed from prefab)");*/
	}
	/* -------------------- END -------------------- */

	// for each component in new prefab
	for (auto compI = newPrefabComps.MemberBegin(); compI != newPrefabComps.MemberEnd(); ++compI) {
		const char* compName = compI->name.GetString();
		const rapidjson::Value& newCompData = compI->value;

		// if new prefab has a new component
		if (!oldPrefabComps.HasMember(compName)) {
			// remove if exists (to start fresh)
			removeComponentByName(obj, compName);

			applyComponentFromJson(obj, compName, newCompData, a, [this](GameObject* o, const rapidjson::Value& v) { applyPrefabComponents(o, v);});

			continue;
		}


		const rapidjson::Value& oldCompData = oldPrefabComps[compName];


		// if obj doesnt have this component
		if (!objComps.HasMember(compName)) {
			/*std::cout << "  Object missing component '" << compName << "', applying from prefab\n";*/

			applyComponentFromJson(obj, compName, newCompData, a, [this](GameObject* o, const rapidjson::Value& v) { applyPrefabComponents(o, v); });

			continue;
		}


		const rapidjson::Value& objCompData = objComps[compName];


		// component exists in all three -> compare properties
		rapidjson::Value finalCompData(rapidjson::kObjectType);
		for (auto propIt = newCompData.MemberBegin(); propIt != newCompData.MemberEnd(); ++propIt) {
			const char* propName = propIt->name.GetString();
			const rapidjson::Value& newValue = propIt->value;
			const rapidjson::Value* sourceValue = nullptr;

			// property is NEW in the prefab
			if (!oldCompData.HasMember(propName) || !objCompData.HasMember(propName)) {
				/*std::cout << "    Property '" << compName << "." << propName << "' is NEW - using prefab value\n";*/
				sourceValue = &newValue;
			}
			else {
				const rapidjson::Value& oldValue = oldCompData[propName];
				const rapidjson::Value& objValue = objCompData[propName];

				// use new prefab if object matches old prefab (means no override)
				sourceValue = valuesEqual(objValue, oldValue) ? &newValue : &objValue;
			}

			rapidjson::Value copy;
			copy.CopyFrom(*sourceValue, a);
			finalCompData.AddMember(rapidjson::Value(propName, a), copy, a);
		}

		removeComponentByName(obj, compName);
		rapidjson::Value temp(rapidjson::kObjectType);
		temp.AddMember(rapidjson::Value(compName, a), finalCompData, a);
		applyPrefabComponents(obj, temp);
	}
}