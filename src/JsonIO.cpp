/* Start Header ************************************************************************/
/*!
\file		JsonIO.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Implementation of JSON helpers and scene serialization for the WaterBound engine.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/	

#include "JsonIO.h"
#include "GameObjectManager.h"
#include "renderer.h"

#ifndef RUNTIME_SCENE_DIR_R
#define RUNTIME_SCENE_DIR_R "./assets/Scene"
#endif
#ifndef SOURCE_ASSETS_DIR_R
#define SOURCE_ASSETS_DIR_R "./projects/WaterBound/assets"
#endif
#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "./projects/WaterBound/assets"
#endif
//==============================================================================
// Paths
//==============================================================================

/**
 * \namespace Paths
 * \brief OS-aware helpers to resolve folders relative to the executable.
 *
 * These functions let the engine run regardless of where the EXE lives by
 * computing paths at runtime. Use these for loading configs, scripts, and
 * other resources that may be bundled alongside the executable.
 */
namespace Paths {

    /**
     * \brief Returns the directory that contains the running executable.
     * \return Executable folder as std::filesystem::path.
     *
     * \note On Windows, uses \c GetModuleFileNameW .
     *       On Linux, resolves \c /proc/self/exe .
     */
    std::filesystem::path exe_dir() {
#ifdef _WIN32
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        return std::filesystem::path(buf).parent_path();
#else
        return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
    }

    /**
     * \brief Convenience: path to the \c shaders directory next to the EXE.
     */
    std::filesystem::path shaders() { return exe_dir() / "shaders"; }

    /**
     * \brief Convenience: path to the \c shaders/Scene directory next to the EXE.
     */
    std::filesystem::path scenes() { return shaders() / "Scene"; }

    /**
     * \brief Resolves the configuration file location.
     * \return \c config.json next to the EXE if present; otherwise \c resources/config.json .
     */
    std::filesystem::path config() {
        auto direct = exe_dir() / "config.json";
        if (std::filesystem::exists(direct))
            return direct;
        return exe_dir() / "resources" / "config.json";
    }

    /**
     * \brief Convenience: path to the \c scripting directory next to the EXE.
     */
    std::filesystem::path scripts() { return exe_dir() / "scripting"; }

    /**
     * \brief Ensures a directory exists (creates it if needed).
     * \param p Directory path to create.
     *
     * \note Failures are swallowed via \c std::error_code; it is safe to call
     *       even if the directory already exists.
     */
    void ensure_dir(const std::filesystem::path& p) {
        std::error_code ec;
        std::filesystem::create_directories(p, ec);
    }

} // namespace Paths


//==============================================================================
// JsonIO
//==============================================================================

/**
 * \namespace JsonIO
 * \brief Thin abstraction layer over RapidJSON plus engine-specific helpers.
 *
 * Responsibilities:
 * - Safe JSON read/write with good error messages
 * - Atomic file replacement for saves
 * - Small value helpers (bool/int/float/vec/color)
 * - String <-> enum conversions for engine types (e.g., shape)
 * - Scene path indirections for live-edit workflows
 */
namespace JsonIO {

    /// \name Scene path helpers
    /// @{

    /**
     * \brief Resolves runtime scene file path.
     * \param name Scene file name (e.g., \c level01.json ).
     * \return Absolute/relative path into \c RUNTIME_SCENE_DIR_R .
     */
    std::string JsonIO::runtimeScenePath(const std::string& name) {
        return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
    }

    /**
     * \brief Resolves source (project) scene file path.
     * \param name Scene file name (e.g., \c level01.json ).
     * \return Absolute/relative path into \c SOURCE_SCENE_DIR_R .
     */
    std::string JsonIO::sourceScenePath(const std::string& name) {
        return std::string(SOURCE_ASSETS_DIR_R) + "/Scene/" + name;
    }

    /**
     * \brief Returns the project source root for shaders-related assets.
     * \return Value of \c SOURCE_DIR_R .
     */
    std::string projectSourceRoot() {
        return std::string(SOURCE_DIR_R);
    }

    /// @}


    /**
     * \brief Copies a scene from the project source folder into the runtime folder.
     * \param name Scene filename (e.g., \c level01.json ).
     * \return \c true on success, \c false otherwise.
     *
     * Creates the destination directory if needed and overwrites existing files.
     * Useful in editor workflows so content creators don't need to manually copy.
     */
    bool syncSceneToRuntime(const std::string& name) {
        const std::string src = sourceScenePath(name);
        const std::string dst = runtimeScenePath(name);

        std::error_code ec;
        const auto parent = std::filesystem::path(dst).parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent, ec);

        std::filesystem::copy_file(
            src, dst,
            std::filesystem::copy_options::overwrite_existing, ec
        );

        if (ec) {
            std::cerr << "Scene sync failed: " << src << " -> " << dst
                << " (" << ec.message() << ")\n";
            return false;
        }
        std::cout << "Scene synced: " << dst << "\n";
        return true;
    }


    /// \name Shape conversion helpers
    /// @{

    /**
     * \brief Converts a \c shape enum to its JSON string representation.
     * \param s Engine shape enum.
     * \return \c "square" , \c "circle" , or \c "triangle" (defaults to \c "square" ).
     */
    const char* ShapeToStr(shape s) {
        switch (s) {
        case shape::square:   return "square";
        case shape::circle:   return "circle";
        case shape::triangle: return "triangle";
        default:              return "square";
        }
    }

    /**
     * \brief Converts a JSON string into a \c shape enum.
     * \param s JSON string (\c "square" , \c "circle" , \c "triangle" ).
     * \return Parsed enum; defaults to \c shape::square .
     */
    shape StrToShape(const std::string& s) {
        if (s == "circle")   return shape::circle;
        if (s == "triangle") return shape::triangle;
        return shape::square;
    }

    /// @}


    /**
     * \brief Converts a CollisionResponseMode enum to a string.
     * \param mode The collision response mode enum.
     * \return String representation ("StopWhenCollide" or "MoveWhenCollide").
     */
    const char* CollisionResponseModeToStr(CollisionResponseMode mode) {
        switch (mode) {
        case CollisionResponseMode::StopWhenCollide: return "StopWhenCollide";
        case CollisionResponseMode::MoveWhenCollide: return "MoveWhenCollide";
        default:                                    return "StopWhenCollide";
        }
    }

    /**
     * \brief Converts a JSON string into a CollisionResponseMode enum.
     * \param s JSON string ("StopWhenCollide" or "MoveWhenCollide").
     * \return Parsed enum; defaults to CollisionResponseMode::StopWhenCollide.
     */
    CollisionResponseMode StrToCollisionResponseMode(const std::string& s) {
        if (s == "MoveWhenCollide") return CollisionResponseMode::MoveWhenCollide;
        return CollisionResponseMode::StopWhenCollide;
    }


    /**
     * \brief Atomically replaces a target file with a temporary file.
     * \param tmp Path to the temporary file (already fully written and flushed).
     * \param dst Final destination path to replace.
     * \param err Optional error string out.
     * \return \c true on success, \c false otherwise.
     *
     * Strategy:
     * 1) Try remove(\c dst)   (ignore errors)
     * 2) Try rename(\c tmp,\c dst)
     * 3) If rename fails (not uncommon on Windows), copy(\c tmp -> \c dst) and delete \c tmp .
     */
    bool AtomicMove(const std::string& tmp, const std::string& dst, std::string* err) {
        std::error_code ec;
        std::filesystem::remove(dst, ec);
        std::filesystem::rename(tmp, dst, ec);
        if (ec) {
            std::filesystem::copy_file(tmp, dst,
                std::filesystem::copy_options::overwrite_existing, ec);
            std::filesystem::remove(tmp, ec);
            if (ec) { if (err) *err = "Move failed: " + ec.message(); return false; }
        }
        return true;
    }


    /**
     * \brief Reads a JSON file from disk and parses it into a RapidJSON document.
     * \param path File path to load.
     * \param doc  RapidJSON document to populate.
     * \param err  Optional error string out (on failure).
     * \return \c true on success, \c false if file open, parse, or root type fails.
     *
     * \note On parse error, \c err includes offset and human-readable message.
     */
    bool ReadFileToDocument(const std::string& path,
        rapidjson::Document& doc,
        std::string* err) {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) { if (err) *err = "Could not open " + path; return false; }

        rapidjson::IStreamWrapper isw(ifs);
        doc.ParseStream(isw);

        if (doc.HasParseError()) {
            if (err) *err = std::string("JSON parse error @")
                + std::to_string(doc.GetErrorOffset()) + ": "
                + rapidjson::GetParseError_En(doc.GetParseError());
            return false;
        }
        if (!doc.IsObject()) { if (err) *err = "Root JSON must be an object"; return false; }
        return true;
    }


    /**
     * \brief Writes a RapidJSON document to disk (pretty or compact) using a temp file.
     * \param path   Destination file path.
     * \param doc    RapidJSON document to write.
     * \param pretty If \c true , use \c PrettyWriter with 2-space indent.
     * \param err    Optional error string out (on failure).
     * \return \c true on success, \c false if writing or atomic move fails.
     *
     * \details The function creates the parent directory if needed, writes to
     *          \c path + ".tmp" first, then atomically replaces the target.
     */
    bool WriteDocumentToFile(const std::string& path,
        rapidjson::Document& doc,
        bool pretty,
        std::string* err) {
        std::error_code ec;
        const auto parent = std::filesystem::path(path).parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent, ec);

        const std::string tmp = path + ".tmp";
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs) { if (err) *err = "Cannot open temp file for writing: " + tmp; return false; }

        rapidjson::OStreamWrapper osw(ofs);
        if (pretty) {
            rapidjson::PrettyWriter<rapidjson::OStreamWrapper> w(osw);
            w.SetIndent(' ', 2);
            doc.Accept(w);
        }
        else {
            rapidjson::Writer<rapidjson::OStreamWrapper> w(osw);
            doc.Accept(w);
        }

        ofs.flush();
        ofs.close();
        return AtomicMove(tmp, path, err);
    }


    /// \name JSON small-value helpers
    /// @{

    /**
     * \brief Fetches a sub-value by key if the parent is an object.
     * \param parent Object to read from.
     * \param key    Null-terminated key string.
     * \return Pointer to value if present and parent is object; otherwise \c nullptr .
     */
    const rapidjson::Value* GetObj(const rapidjson::Value& parent, const char* key) {
        if (!parent.IsObject() || !parent.HasMember(key)) return nullptr;
        return &parent[key];
    }

    /**
     * \brief Reads a string field from a JSON object.
     * \param obj  JSON object.
     * \param key  Field name.
     * \param out  Output string on success.
     * \return \c true if present and string; otherwise \c false .
     */
    bool GetString(const rapidjson::Value& obj, const char* key, std::string& out) {
        if (auto* v = GetObj(obj, key); v && v->IsString()) { out = v->GetString(); return true; }
        return false;
    }

    /**
     * \brief Reads a boolean-like field (bool, int, or number) from a JSON object.
     * \param obj JSON object.
     * \param key Field name.
     * \param out Output boolean (non-zero numeric treated as true).
     * \return \c true on success, \c false otherwise.
     */
    bool GetBool(const rapidjson::Value& obj, const char* key, bool& out) {
        if (auto* v = GetObj(obj, key)) {
            if (v->IsBool()) { out = v->GetBool();   return true; }
            if (v->IsInt()) { out = (v->GetInt() != 0);  return true; }
            if (v->IsNumber()) { out = (v->GetDouble() != 0.0); return true; }
        }
        return false;
    }

    /**
     * \brief Reads an integer field from a JSON object.
     */
    bool GetInt(const rapidjson::Value& obj, const char* key, int& out) {
        if (auto* v = GetObj(obj, key); v && v->IsInt()) { out = v->GetInt(); return true; }
        return false;
    }

    /**
     * \brief Reads a float field from a JSON object.
     * \details Accepts either float or generic number; performs safe cast.
     */
    bool GetFloat(const rapidjson::Value& obj, const char* key, float& out) {
        if (auto* v = GetObj(obj, key)) {
            if (v->IsFloat()) { out = v->GetFloat();  return true; }
            if (v->IsNumber()) { out = static_cast<float>(v->GetDouble()); return true; }
        }
        return false;
    }

    /// @}


    /// \name Vec3 helpers
    /// @{

    /**
     * \brief Reads an array of exactly 3 numeric values into \c out .
     * \param obj JSON object.
     * \param key Field name.
     * \param out Output array \c {x,y,z} .
     * \return \c true on success; \c false if shape/type mismatch.
     */
    bool GetArray3f(const rapidjson::Value& obj, const char* key, std::array<float, 3>& out) {
        auto* v = GetObj(obj, key);
        if (!v || !v->IsArray() || v->Size() != 3) return false;
        for (rapidjson::SizeType i = 0; i < 3; ++i) {
            if (!(*v)[i].IsNumber()) return false;
            out[i] = static_cast<float>((*v)[i].GetDouble());
        }
        return true;
    }

    /**
     * \brief Writes a 3-float array field under \c key into \c obj .
     */
    void PutArray3f(rapidjson::Value& obj, const char* key, const std::array<float, 3>& v, rapidjson::Document::AllocatorType& a) {
        rapidjson::Value arr(rapidjson::kArrayType);
        arr.PushBack(rapidjson::Value().SetFloat(v[0]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[1]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[2]), a);
        obj.AddMember(rapidjson::Value(key, a), arr, a);
    }

    /**
     * \brief Reads a 3-float array field into a \c glm::vec3 .
     */
    bool ReadVec3(const rapidjson::Value& obj, const char* key, glm::vec3& out) {
        std::array<float, 3> tmp{};
        if (!GetArray3f(obj, key, tmp)) return false;
        out = glm::vec3(tmp[0], tmp[1], tmp[2]);
        return true;
    }

    /**
     * \brief Writes a \c glm::vec3 to JSON as a 3-float array.
     */
    void WriteVec3(rapidjson::Value& obj, const char* key, const glm::vec3& v, rapidjson::Document::AllocatorType& a) {
        PutArray3f(obj, key, { v.x, v.y, v.z }, a);
    }

    /**
     * \brief Reads a 3-float array field into three separate floats.
     */
    bool ReadVec3(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z) {
        std::array<float, 3> tmp{};
        if (!GetArray3f(obj, key, tmp)) return false;
        x = tmp[0]; y = tmp[1]; z = tmp[2];
        return true;
    }

    /**
     * \brief Writes three floats as a 3-float array field.
     */
    void WriteVec3(rapidjson::Value& obj, const char* key, float x, float y, float z, rapidjson::Document::AllocatorType& a) {
        PutArray3f(obj, key, { x, y, z }, a);
    }

    /// @}


    /// \name Color helpers (RGBA)
    /// @{

    /**
     * \brief Reads an RGBA array field into separate components.
     * \return \c true on success; requires exactly four numeric elements.
     */
    bool ReadColor(const rapidjson::Value& obj, const char* key, float& r, float& g, float& b, float& a) {
        const auto* v = GetObj(obj, key);
        if (!v || !v->IsArray() || v->Size() != 4) return false;

        for (rapidjson::SizeType i = 0; i < 4; ++i) {
            if (!(*v)[i].IsNumber()) return false;
        }

        r = static_cast<float>((*v)[0].GetDouble());
        g = static_cast<float>((*v)[1].GetDouble());
        b = static_cast<float>((*v)[2].GetDouble());
        a = static_cast<float>((*v)[3].GetDouble());
        return true;
    }

    /**
     * \brief Writes RGBA components as a 4-float array field.
     */
    void WriteColor(rapidjson::Value& obj, const char* key, float r, float g, float b, float a, rapidjson::Document::AllocatorType& alloc) {
        rapidjson::Value arr(rapidjson::kArrayType);
        arr.PushBack(rapidjson::Value().SetFloat(r), alloc);
        arr.PushBack(rapidjson::Value().SetFloat(g), alloc);
        arr.PushBack(rapidjson::Value().SetFloat(b), alloc);
        arr.PushBack(rapidjson::Value().SetFloat(a), alloc);
        obj.AddMember(rapidjson::Value(key, alloc), arr, alloc);
    }

    /**
     * \brief Reads RGBA into a \c glm::vec4 .
     */
    bool ReadColor(const rapidjson::Value& obj, const char* key, glm::vec4& out) {
        float r, g, b, a;
        if (!ReadColor(obj, key, r, g, b, a)) return false;
        out = glm::vec4(r, g, b, a);
        return true;
    }

    /**
     * \brief Writes a \c glm::vec4 as a 4-float RGBA array.
     */
    void WriteColor(rapidjson::Value& obj, const char* key, const glm::vec4& v, rapidjson::Document::AllocatorType& alloc) {
        WriteColor(obj, key, v.r, v.g, v.b, v.a, alloc);
    }

    /// @}

    /* ---------- Saving obj state in json for undo/redo ---------- */
    std::string serializeGameObj(GameObject* obj) {
        if (!obj) return "";

        using namespace rapidjson; // save my life

        Document doc;
        doc.SetObject();
        auto& a = doc.GetAllocator();

        // Basic info
        doc.AddMember("name", Value(obj->getObjectName().c_str(), a), a);
        doc.AddMember("prefabID", Value(obj->getObjectPrefabID().c_str(), a), a);
        doc.AddMember("layer", obj->getLayer(), a);

        // Transform
        if (Transform* t = obj->getComponent<Transform>()) {
            Value jt(kObjectType);
            JsonIO::WriteVec3(jt, "pos", t->x, t->y, t->z, a);
            jt.AddMember("rotation", Value().SetFloat(t->rotation), a);
            JsonIO::WriteVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ, a);
            doc.AddMember("Transform", jt, a);
        }

        // Render
        if (Render* r = obj->getComponent<Render>()) {
            Value jr(kObjectType);

            jr.AddMember("shape", Value(JsonIO::ShapeToStr(r->modelRef.shape), a),a);
            jr.AddMember("hasTex", Value().SetBool(r->hasTex), a);
            if (r->hasTex && !r->texFile.empty()) {
                jr.AddMember("texture", Value(r->texFile.c_str(), a), a);
            }

            // 0/1 to mirror prior text formats
            jr.AddMember("hasAnimation", Value(r->hasAnimation ? 1 : 0), a);
            JsonIO::WriteVec3(jr, "clr", r->clr.x, r->clr.y, r->clr.z, a);

            doc.AddMember("Render", jr, a);
        }

        // Physics
        if (Physics* p = obj->getComponent<Physics>()) {
            Value jp(kObjectType);

            // lazy to type, but no need to convert boolean to 1/0 should be fine for the obj still
            jp.AddMember("physicsFlag", p->physicsFlag, a);
            jp.AddMember("moveSpeed", p->moveSpeed, a);
            jp.AddMember("jumpForce", p->jumpForce, a);
            jp.AddMember("gravity", p->gravity, a);
            jp.AddMember("damping", p->damping, a);
            jp.AddMember("mass", p->dynamics.mass, a);
            jp.AddMember("buoyancy", p->buoancy, a);

            doc.AddMember("Physics", jp, a);
        }

        // Collision
        if (CollisionInfo* c = obj->getComponent<CollisionInfo>()) {
            Value jc(kObjectType);

            jc.AddMember("collisionFlag", c->collisionFlag, a);
            jc.AddMember("autoFitScale", c->autoFitScale, a);
            jc.AddMember("width", c->colliderSize.x, a);
            jc.AddMember("height", c->colliderSize.y, a);

            Value collisionResStr(CollisionResponseModeToStr(c->collisionRes), a);
            jc.AddMember("collisionRes", collisionResStr, a);

            doc.AddMember("Collision", jc, a);
        }

        // Input
        if (obj->hasComponent<Input>()) {
            doc.AddMember("Input", true, a);
        }

        // Font
        if (FontComponent* f = obj->getComponent<FontComponent>()) {
            Value jf(kObjectType);

            jf.AddMember("word", Value(f->word.c_str(), a), a);
            jf.AddMember("scale", f->scale, a);
            jf.AddMember("fontType", f->fontType, a);
            JsonIO::WriteVec3(jf, "clr", f->clr.x, f->clr.y, f->clr.z, a);

            doc.AddMember("Font", jf, a);
        }

        // Audio
        if (AudioComponent* audio = obj->getComponent<AudioComponent>()) {
            Value ja(kObjectType);

            AudioChannel* ac = audio->getDefaultChannel();

            // CRITICAL FIX: Only serialize if "default" channel exists
            if (ac) {
                ja.AddMember("audioFile", Value(ac->audioFile.c_str(), a), a);
                ja.AddMember("loop", ac->loop, a);
                ja.AddMember("playOnStart", ac->playOnStart, a);
                ja.AddMember("volume", ac->volume, a);
                ja.AddMember("pitch", ac->pitch, a);
                ja.AddMember("fadeInOnStart", ac->fadeInOnStart, a);
                ja.AddMember("fadeInDuration", ac->fadeInDuration, a);
                ja.AddMember("fadeOutOnStop", ac->fadeOutOnStop, a);
                ja.AddMember("fadeOutDuration", ac->fadeOutDuration, a);

                doc.AddMember("Audio", ja, a);
            }
        }

        if (StateMachine* sm = obj->getComponent<StateMachine>()) {
            if (Animation* anim = obj->getComponent<Animation>()) {
                Value js(kObjectType);

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

                doc.AddMember("StateMachine", js, a);
            }
        }

        // TileMap
        if (TileMap* tm = obj->getComponent<TileMap>()) {
            Value jt(kObjectType);

            jt.AddMember("tileW", tm->tileW, a);
            jt.AddMember("tileH", tm->tileH, a);
            jt.AddMember("columns", tm->columns, a);
            jt.AddMember("rows", tm->rows, a);

            // Serialize tiles array
            Value tileArray(kArrayType);
            for (const auto& [key, id] : tm->tiles) {
                Value t(kObjectType);
                t.AddMember("x", key.x, a);
                t.AddMember("y", key.y, a);
                t.AddMember("id", rapidjson::Value(id.c_str(), a), a);
                tileArray.PushBack(t, a);
            }
            jt.AddMember("Tiles", tileArray, a);

            doc.AddMember("TileMap", jt, a);
        }

        // convert to string
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    GameObject* deserializeGameObj(GameObjectManager& manager, const std::string& jsonStr) {
        if (jsonStr.empty()) return nullptr;

        using namespace rapidjson;

        try {
            Document doc;
            doc.Parse(jsonStr.c_str());
            if (doc.HasParseError()) return nullptr;

            std::string name = doc.HasMember("name") ? doc["name"].GetString() : "Unnamed";
            GameObject* obj = manager.createGameObject(name);
            if (!obj) return nullptr;

            if (doc.HasMember("prefabID")) obj->getObjectPrefabID() = doc["prefabID"].GetString();

            if (doc.HasMember("layer")) manager.assignObjectToLayer(obj, doc["layer"].GetInt());

            // Transform
            if (doc.HasMember("Transform")) {
                const Value& jt = doc["Transform"];
                Transform* t = obj->addComponent<Transform>();

                ReadVec3(jt, "pos", t->x, t->y, t->z);
                GetFloat(jt, "rotation", t->rotation);
                ReadVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ);
            }

            // Render
            if (doc.HasMember("Render")) {
                const Value& jr = doc["Render"];
                Render* r = obj->addComponent<Render>();

                std::string shp = (jr.HasMember("shape") && jr["shape"].IsString())
                    ? jr["shape"].GetString()
                    : "square";
                r->modelRef = renderer::models[static_cast<int>(JsonIO::StrToShape(shp))];

                // hasTex / texture (accept bool or 0/1)
                if (jr.HasMember("hasTex")) {
                    r->hasTex = jr["hasTex"].IsBool()
                        ? jr["hasTex"].GetBool()
                        : (jr["hasTex"].GetInt() != 0);
                }
                else {
                    r->hasTex = false;
                }

                if (r->hasTex && jr.HasMember("texture") && jr["texture"].IsString()) {
                    r->texFile = jr["texture"].GetString();
                    r->texHDL = 0;         // will be uploaded later by renderer
                    r->texChanged = true;     // flag for upload
                }
                else {
                    r->texFile.clear();
                    r->texHDL = 0;
                    r->texChanged = false;
                }

                if (jr.HasMember("hasAnimation")) {
                    r->hasAnimation = jr["hasAnimation"].IsBool()
                        ? jr["hasAnimation"].GetBool()
                        : (jr["hasAnimation"].GetInt() != 0);
                }
                else {
                    r->hasAnimation = false;
                }

                ReadVec3(jr, "clr", r->clr.x, r->clr.y, r->clr.z);
            }

            // Physics
            if (doc.HasMember("Physics")) {
                const Value& jp = doc["Physics"];
                Physics* p = obj->addComponent<Physics>();

                p->physicsFlag = jp["physicsFlag"].GetBool();
                p->moveSpeed = jp["moveSpeed"].GetFloat();
                p->jumpForce = jp["jumpForce"].GetFloat();
                p->gravity = jp["gravity"].GetFloat();
                p->damping = jp["damping"].GetFloat();
                p->dynamics.mass = jp["mass"].GetFloat();
                p->buoancy = jp["buoyancy"].GetBool();
            }

            // Collision
            if (doc.HasMember("Collision")) {
                const Value& jc = doc["Collision"];
                CollisionInfo* c = obj->addComponent<CollisionInfo>();

                c->collisionFlag = jc["collisionFlag"].GetBool();
                c->autoFitScale = jc["autoFitScale"].GetBool();
                c->colliderSize.x = jc["width"].GetFloat();
                c->colliderSize.y = jc["height"].GetFloat();

                if (jc.HasMember("collisionRes")) {
                    c->collisionRes = StrToCollisionResponseMode(jc["collisionRes"].GetString());
                }
            }

            // Input
            if (doc.HasMember("Input") && doc["Input"].GetBool()) {
                obj->addComponent<Input>();
            }

            // Font
            if (doc.HasMember("Font")) {
                const Value& jf = doc["Font"];
                FontComponent* f = obj->addComponent<FontComponent>();

                f->word = jf["word"].GetString();
                f->scale = jf["scale"].GetFloat();
                f->fontType = jf["fontType"].GetInt();
                ReadVec3(jf, "clr", f->clr.x, f->clr.y, f->clr.z);
            }

            // Audio
            if (doc.HasMember("Audio")) {
                const Value& ja = doc["Audio"];
                AudioComponent* ac = obj->addComponent<AudioComponent>();
                AudioChannel* a = ac->getDefaultChannel();

                a->audioFile = ja["audioFile"].GetString();
                a->loop = ja["loop"].GetBool();
                a->playOnStart = ja["playOnStart"].GetBool();
                a->volume = ja["volume"].GetFloat();
                a->pitch = ja["pitch"].GetFloat();
                a->fadeInOnStart = ja["fadeInOnStart"].GetBool();
                a->fadeInDuration = ja["fadeInDuration"].GetFloat();
                a->fadeOutOnStop = ja["fadeOutOnStop"].GetBool();
                a->fadeOutDuration = ja["fadeOutDuration"].GetFloat();
            }

            // State Machine with Animation
            if (doc.HasMember("StateMachine")) {
                const Value& smObj = doc["StateMachine"];
                StateMachine* sm = obj->addComponent<StateMachine>();
                Animation* anim = obj->addComponent<Animation>();
                anim->animState.resize(stateNames.size());

                if (sm && anim) {
                    for (size_t i = 0; i < stateNames.size() && i < anim->animState.size(); ++i) {
                        if (smObj.HasMember(stateNames[i].c_str())) {
                            const Value& stateObj = smObj[stateNames[i].c_str()];

                            if (stateObj.HasMember("animState")) {
                                const Value& animStateObj = stateObj["animState"];
                                AnimateState& animState = anim->animState[i];

                                animState.texFile = animStateObj["texture"].GetString();
                                animState.loop = animStateObj["loop"].IsBool() ? animStateObj["loop"].GetBool() : (animStateObj["loop"].GetInt() != 0);
                                animState.currentFrameColumn = animStateObj["currentFrameColumn"].GetInt();
                                animState.currentFrameRow = animStateObj["currentFrameRow"].GetInt();
                                animState.totalColumn = animStateObj["totalColumn"].GetInt();
                                animState.totalRow = animStateObj["totalRow"].GetInt();
                                animState.frameTime = animStateObj["frameTime"].GetFloat();
                                animState.initialFrame.x = (float)animStateObj["initialFrameCol"].GetInt();
                                animState.initialFrame.y = (float)animStateObj["initialFrameRow"].GetInt();
                                animState.lastFrame.x = (float)animStateObj["lastFrameCol"].GetInt();
                                animState.lastFrame.y = (float)animStateObj["lastFrameRow"].GetInt();
                                animState.texChanged = true; // magic boolean!!
                            }
                        }
                    }
                }
            }

            // TileMap
            if (doc.HasMember("TileMap"))
            {
                const Value& tmj = doc["TileMap"];
                TileMap* tm = obj->addComponent<TileMap>();

                if (tmj.HasMember("tileW"))    tm->tileW = tmj["tileW"].GetFloat();
                if (tmj.HasMember("tileH"))    tm->tileH = tmj["tileH"].GetFloat();
                if (tmj.HasMember("columns"))  tm->columns = tmj["columns"].GetInt();
                if (tmj.HasMember("rows"))     tm->rows = tmj["rows"].GetInt();

                tm->tiles.clear();

                if (tmj.HasMember("tiles") && tmj["tiles"].IsArray()) {
                    for (const auto& t : tmj["tiles"].GetArray()) {
                        int x = t["x"].GetInt();
                        int y = t["y"].GetInt();
                        std::string id = t["id"].GetString();
                        tm->setTile(x, y, id);
                    }
                }
            }

            return obj;
        }
        catch (...) {
            DebugLog::addMessage("Failed to deserialize GameObject with RapidJSON.");
            return nullptr;
        }
    }
    /* ---------- END ---------- */

} // namespace JsonIO



//==============================================================================
// GameObjectManager JSON serialization
//==============================================================================

/**
 * \brief Serializes all current game objects (and components) to a JSON file.
 * \param filename Destination JSON path.
 * \param isNew    If \c true , writes only a version header (empty scene shell).
 *
 * \details
 * Layout (version 1):
 * \code{.json}
 * {
 *   "version": 1,
 *   "objects": [
 *     {
 *       "name": "player",
 *       "components": {
 *         "Transform": { "pos":[x,y,z], "rotation":r, "scale":[sx,sy,sz] },
 *         "Render":    { "shape":"square|circle|triangle", "hasTex":true, "texture":"..." , "hasAnimation":0|1 },
 *         "Input":     {},
 *         "Physics":   { "inWater":0|1, "buoyancy":0|1 },
 *         "CollisionInfo": {},
 *         "Animation": { "loop":0|1, "currentFrameColumn":i, "currentFrameRow":i, "totalColumn":i, "totalRow":i, "frameTime":f }
 *       }
 *     }
 *   ]
 * }
 * \endcode
 */
void GameObjectManager::saveToJson(const std::string& filename, bool isNew) const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& a = doc.GetAllocator();

    // Version header
    doc.AddMember("version", rapidjson::Value(1), a);

    if (!isNew) {
        rapidjson::Value objects(rapidjson::kArrayType);

        for (auto const& kv : m_gameObjects) {
            const GameObject* obj = kv.second.get();

            rapidjson::Value jObj(rapidjson::kObjectType);

            // Object name
            jObj.AddMember("name",
                rapidjson::Value(obj->getObjectName().c_str(), a),
                a);

            // Components container
            rapidjson::Value comps(rapidjson::kObjectType);

            // ---- Transform ----
            if (auto t = obj->getComponent<Transform>()) {
                rapidjson::Value jt(rapidjson::kObjectType);

                // pos / rotation / scale
                JsonIO::WriteVec3(jt, "pos", t->x, t->y, t->z, a);
                jt.AddMember("rotation", rapidjson::Value().SetFloat(t->rotation), a);
                JsonIO::WriteVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ, a);

                comps.AddMember("Transform", jt, a);
            }

            // ---- Render ----
            if (auto r = obj->getComponent<Render>()) {
                rapidjson::Value jr(rapidjson::kObjectType);

                // shape
                jr.AddMember("shape",
                    rapidjson::Value(JsonIO::ShapeToStr(r->modelRef.shape), a),
                    a);

                // hasTex / texture
                jr.AddMember("hasTex", rapidjson::Value().SetBool(r->hasTex), a);
                if (r->hasTex && !r->texFile.empty()) {
                    jr.AddMember("texture",
                        rapidjson::Value(r->texFile.c_str(), a),
                        a);
                }

                // 0/1 to mirror prior text formats
                jr.AddMember("hasAnimation", rapidjson::Value(r->hasAnimation ? 1 : 0), a);

                // Optional future flags:
                // jr.AddMember("visible",     rapidjson::Value().SetBool(r->visible), a);
                // jr.AddMember("transparent", rapidjson::Value().SetBool(r->isTransparent), a);
                // jr.AddMember("wireframe",   rapidjson::Value().SetBool(r->wireframe), a);

                comps.AddMember("Render", jr, a);
            }

            // ---- Input (presence only) ----
            if (obj->hasComponent<Input>()) {
                comps.AddMember("Input", rapidjson::Value(rapidjson::kObjectType), a);
            }

            // ---- Physics ----
            if (auto p = obj->getComponent<Physics>()) {
                rapidjson::Value jp(rapidjson::kObjectType);
                // store as 0/1 to mirror .txt
                jp.AddMember("inWater", rapidjson::Value(p->inWater ? 1 : 0), a);
                jp.AddMember("buoyancy", rapidjson::Value(p->buoancy ? 1 : 0), a); // member is 'buoancy' in struct
                comps.AddMember("Physics", jp, a);
            }

            // ---- CollisionInfo (presence only) ----
            if (obj->hasComponent<CollisionInfo>()) {
                comps.AddMember("CollisionInfo", rapidjson::Value(rapidjson::kObjectType), a);
            }

            // ---- Animation ----
            /*if (auto an = obj->getComponent<Animation>()) {
                rapidjson::Value ja(rapidjson::kObjectType);
                ja.AddMember("loop", rapidjson::Value(an->loop ? 1 : 0), a);
                ja.AddMember("currentFrameColumn", rapidjson::Value(an->currentFrameColumn), a);
                ja.AddMember("currentFrameRow", rapidjson::Value(an->currentFrameRow), a);
                ja.AddMember("totalColumn", rapidjson::Value(an->totalColumn), a);
                ja.AddMember("totalRow", rapidjson::Value(an->totalRow), a);
                ja.AddMember("frameTime", rapidjson::Value().SetFloat(an->frameTime), a);
                comps.AddMember("Animation", ja, a);
            }*/

            jObj.AddMember("components", comps, a);
            objects.PushBack(jObj, a);
        }

        // Attach the objects array
        doc.AddMember("objects", objects, a);
    }

    std::string err;
    if (!JsonIO::WriteDocumentToFile(filename, doc, /*pretty=*/true, &err)) {
        std::cerr << "Scene save failed: " << err << "\n";
    }
    else {
        std::cout << "Scene saved (JSON): " << filename << "\n";
    }
}


/**
 * \brief Loads a scene from a JSON file and rebuilds all game objects.
 * \param filename Source JSON path.
 *
 * \details
 * Behavior:
 * - Validates version (warns on mismatch, still attempts to load).
 * - Clears existing scene before creation.
 * - Accepts booleans and numeric 0/1 for legacy compatibility.
 * - Defers texture upload by setting \c texHDL=0 and \c texChanged=true when needed.
 */
void GameObjectManager::loadFromJson(const std::string& filename) {
    rapidjson::Document doc;
    std::string err;
    if (!JsonIO::ReadFileToDocument(filename, doc, &err)) {
        std::cerr << "Scene load failed: " << err << "\n";
        return;
    }

    // Version check (non-fatal warning)
    int version = 1;
    if (doc.HasMember("version") && doc["version"].IsInt())
        version = doc["version"].GetInt();
    if (version != 1)
        std::cerr << "Warning: unsupported scene version " << version << "\n";

    // Start fresh
    m_gameObjects.clear();

    // Validate presence of objects array
    if (!doc.HasMember("objects") || !doc["objects"].IsArray()) {
        std::cerr << "Scene JSON missing 'objects' array.\n";
        return;
    }

    // Rebuild objects
    for (auto& jObj : doc["objects"].GetArray()) {
        if (!jObj.IsObject()) continue;

        std::string name = (jObj.HasMember("name") && jObj["name"].IsString())
            ? jObj["name"].GetString()
            : ("Object_" + std::to_string(m_gameObjects.size()));

        GameObject* go = createGameObject(name);

        if (!jObj.HasMember("components") || !jObj["components"].IsObject())
            continue;

        const auto& comps = jObj["components"];

        // ---- Transform ----
        if (comps.HasMember("Transform") && comps["Transform"].IsObject()) {
            const auto& jt = comps["Transform"];
            auto* t = go->addComponent<Transform>();

            (void)JsonIO::ReadVec3(jt, "pos", t->x, t->y, t->z);
            (void)JsonIO::GetFloat(jt, "rotation", t->rotation);
            (void)JsonIO::ReadVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ);
        }

        // ---- Render ----
        if (comps.HasMember("Render") && comps["Render"].IsObject()) {
            const auto& jr = comps["Render"];
            auto* r = go->addComponent<Render>();

            // shape
            std::string shp = (jr.HasMember("shape") && jr["shape"].IsString())
                ? jr["shape"].GetString()
                : "square";
            r->modelRef = renderer::models[static_cast<int>(JsonIO::StrToShape(shp))];

            // hasTex / texture (accept bool or 0/1)
            if (jr.HasMember("hasTex")) {
                r->hasTex = jr["hasTex"].IsBool()
                    ? jr["hasTex"].GetBool()
                    : (jr["hasTex"].GetInt() != 0);
            }
            else {
                r->hasTex = false;
            }

            if (r->hasTex && jr.HasMember("texture") && jr["texture"].IsString()) {
                r->texFile = jr["texture"].GetString();
                r->texHDL = 0;         // will be uploaded later by renderer
                r->texChanged = true;     // flag for upload
            }
            else {
                r->texFile.clear();
                r->texHDL = 0;
                r->texChanged = false;
            }

            if (jr.HasMember("hasAnimation")) {
                r->hasAnimation = jr["hasAnimation"].IsBool()
                    ? jr["hasAnimation"].GetBool()
                    : (jr["hasAnimation"].GetInt() != 0);
            }
            else {
                r->hasAnimation = false;
            }

            // Optional future flags:
            // if (jr.HasMember("visible")     && jr["visible"].IsBool())     r->visible       = jr["visible"].GetBool();
            // if (jr.HasMember("transparent") && jr["transparent"].IsBool()) r->isTransparent = jr["transparent"].GetBool();
            // if (jr.HasMember("wireframe")   && jr["wireframe"].IsBool())   r->wireframe     = jr["wireframe"].GetBool();
        }

        // ---- Input (presence only) ----
        if (comps.HasMember("Input") && comps["Input"].IsObject()) {
            go->addComponent<Input>();
        }

        // ---- Physics ----
        if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
            const auto& jp = comps["Physics"];
            auto* p = go->addComponent<Physics>();
            if (jp.HasMember("inWater")) {
                p->inWater = jp["inWater"].IsBool()
                    ? jp["inWater"].GetBool()
                    : (jp["inWater"].GetInt() != 0);
            }
            if (jp.HasMember("buoyancy")) {
                p->buoancy = jp["buoyancy"].IsBool()
                    ? jp["buoyancy"].GetBool()
                    : (jp["buoyancy"].GetInt() != 0);
            }
        }

        // ---- CollisionInfo (presence only) ----
        if (comps.HasMember("CollisionInfo") && comps["CollisionInfo"].IsObject()) {
            go->addComponent<CollisionInfo>();
        }

        // ---- Animation ----
        /*if (comps.HasMember("Animation") && comps["Animation"].IsObject()) {
            const auto& ja = comps["Animation"];
            auto* a = go->addComponent<Animation>();
            if (ja.HasMember("loop")) {
                a->loop = ja["loop"].IsBool()
                    ? ja["loop"].GetBool()
                    : (ja["loop"].GetInt() != 0);
            }
            if (ja.HasMember("currentFrameColumn") && ja["currentFrameColumn"].IsInt())
                a->currentFrameColumn = ja["currentFrameColumn"].GetInt();
            if (ja.HasMember("currentFrameRow") && ja["currentFrameRow"].IsInt())
                a->currentFrameRow = ja["currentFrameRow"].GetInt();
            if (ja.HasMember("totalColumn") && ja["totalColumn"].IsInt())
                a->totalColumn = ja["totalColumn"].GetInt();
            if (ja.HasMember("totalRow") && ja["totalRow"].IsInt())
                a->totalRow = ja["totalRow"].GetInt();
            if (ja.HasMember("frameTime") && ja["frameTime"].IsNumber())
                a->frameTime = ja["frameTime"].GetFloat();
        }*/
    }

    std::cout << "Scene loaded (JSON): " << filename << "\n";
}
