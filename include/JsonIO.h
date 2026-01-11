/* Start Header ************************************************************************/
/*!
\file		JsonIO.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Declarations for JSON loading/saving utilities for the WaterBound engine.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>     
#include <filesystem>
#include <iostream>
#include <array>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp>


#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>

#include "GameObjectManager.h"
#include "Component.h"
#include "renderer.h"

/*!
 * \namespace Paths
 * \brief Provides OS-aware helpers to locate engine directories relative
 *        to the executable (EXE) directory.
 *
 * These utilities ensure that the engine can load configs, shaders and scenes
 * correctly regardless of where the executable is deployed.
 */
namespace Paths {
	std::filesystem::path exe_dir();
	std::filesystem::path shaders();
	std::filesystem::path scenes();
	std::filesystem::path config();
	std::filesystem::path scripts();
	void ensure_dir(const std::filesystem::path& p);
}

/*!
 * \namespace JsonIO
 * \brief Collection of JSON reading/writing helpers using RapidJSON.
 *
 * All JSON parsing, serialization and data extraction utilities live here.
 * This keeps GameObjectManager and engine systems clean and independent of
 * RapidJSON internals.
 */
namespace JsonIO {

	// ---- Path helpers ----
	std::string runtimeScenePath(const std::string& name);
	std::string sourceScenePath(const std::string& name);
	std::string projectSourceRoot();

	// ---- Shape string helpers ----
	const char* ShapeToStr(shape s);
	shape StrToShape(const std::string& s);

	// ---- Collision Response Mode helper
	const char* CollisionResponseModeToStr(CollisionResponseMode mode);
	CollisionResponseMode StrToCollisionResponseMode(const std::string& s);

	// ---- JSON file IO helpers ----
	bool syncSceneToRuntime(const std::string& name);
	bool AtomicMove(const std::string& tmp, const std::string& dst, std::string* err = nullptr);
	bool ReadFileToDocument(const std::string& path, rapidjson::Document& doc, std::string* err = nullptr);
	bool WriteDocumentToFile(const std::string& path, rapidjson::Document& doc, bool pretty = true, std::string* err = nullptr);

	const rapidjson::Value* GetObj(const rapidjson::Value& parent, const char* key);

	bool GetString(const rapidjson::Value& obj, const char* key, std::string& out);

	bool GetBool(const rapidjson::Value& obj, const char* key, bool& out);

	bool GetInt(const rapidjson::Value& obj, const char* key, int& out);
	bool GetFloat(const rapidjson::Value& obj, const char* key, float& out);

	inline int   GetIntOr(const rapidjson::Value& obj, const char* key, int   def) { int v; return GetInt(obj, key, v) ? v : def; }
	inline float GetFloatOr(const rapidjson::Value& obj, const char* key, float def) { float v; return GetFloat(obj, key, v) ? v : def; }
	inline bool  GetBoolOr(const rapidjson::Value& obj, const char* key, bool  def) { bool v; return GetBool(obj, key, v) ? v : def; }

	// ---------- Fixed-size array helpers (3 components) ----------
	bool GetArray3f(const rapidjson::Value& obj, const char* key, std::array<float, 3>& out);
	void PutArray3f(rapidjson::Value& obj, const char* key, const std::array<float, 3>& v, rapidjson::Document::AllocatorType& a);

	// ---------- glm::vec3 helpers ----------
	bool ReadVec3(const rapidjson::Value& obj, const char* key, glm::vec3& out);
	void WriteVec3(rapidjson::Value& obj, const char* key, const glm::vec3& v, rapidjson::Document::AllocatorType& a);


	bool ReadVec3(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z);
	void WriteVec3(rapidjson::Value& obj, const char* key, float x, float y, float z, rapidjson::Document::AllocatorType& a);

	// ---- RGBA (Color) helpers ----

	// Read a 4-component float color array [r,g,b,a] from JSON
	bool ReadColor(const rapidjson::Value& obj,
		const char* key,
		float& r, float& g, float& b, float& alpha);

	// Write a 4-component float color array [r,g,b,a] to JSON
	void WriteColor(rapidjson::Value& obj,
		const char* key,
		float r, float g, float b, float alpha,
		rapidjson::Document::AllocatorType& alloc);

	// Read a glm::vec4 color (r,g,b,a) from JSON
	bool ReadColor(const rapidjson::Value& obj,
		const char* key,
		glm::vec4& out);

	// Write a glm::vec4 color (r,g,b,a) to JSON
	void WriteColor(rapidjson::Value& obj,
		const char* key,
		const glm::vec4& v,
		rapidjson::Document::AllocatorType& alloc);


	// ---- for undo/redo in editor ----
	std::string serializeGameObj(GameObject* obj);
	GameObject* deserializeGameObj(GameObjectManager& manager, const std::string& jsonStr);
	// ---- END ----

	/* ------ moved to components.h ------ */
	// for the indexing in animation vector
	//inline std::vector<std::string> stateNames = { "Idle", "Walking", "Jumping", "Falling", "Shooting" };
	/* ------- END ------- */
}
