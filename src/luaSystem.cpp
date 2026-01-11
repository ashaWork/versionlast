/* Start Header ************************************************************************/
/*!
\file       lua.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       Ocotber, 23rd, 2025
\brief

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "luaSystem.h"

void LuaSystem::init() {
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "Input_isKeyHeld", Lua_IsKeyHeld);
    lua_register(L, "getPosition", Lua_getPosition);
    lua_register(L, "setPosition", Lua_setPosition);

    //for message
    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, Lua_SendInputEvent, 1);
    lua_setglobal(L, "SendInputEvent");
    //manager = std::make_unique<GameObjectManager>();
}

void LuaSystem::cleanup() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

//function exposed to Lua
int LuaSystem::Lua_IsKeyHeld(lua_State* L) {
    const char* keyStr = lua_tostring(L, 1); //get string from Lua

    //convert string to GLFW key code
    int key = -1;
    if (strcmp(keyStr, "W") == 0) key = GLFW_KEY_W;
    else if (strcmp(keyStr, "A") == 0) key = GLFW_KEY_A;
    else if (strcmp(keyStr, "S") == 0) key = GLFW_KEY_S;
    else if (strcmp(keyStr, "D") == 0) key = GLFW_KEY_D;
    else if (strcmp(keyStr, "SPACE") == 0) key = GLFW_KEY_SPACE;
    else if (strcmp(keyStr, "B") == 0) key = GLFW_KEY_B;
    //add more keys if needed in the future

    bool held = false;
    if (key != -1) {
        held = InputHandler::isKeyHeld(key);
    }

    lua_pushboolean(L, held); //push result to Lua
    return 1; //number of return values
}

bool LuaSystem::runString(const std::string& code) {
    if (!L) return false;
    return luaL_dostring(L, code.c_str()) == LUA_OK;
}

bool LuaSystem::runScript(const std::string& filename) {
    if (!L) return false;
    return luaL_dofile(L, filename.c_str()) == LUA_OK;
}

float LuaSystem::getGlobalNumber(const std::string& name) {
    if (!L) return 0.0f;
    lua_getglobal(L, name.c_str());
    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        return 0.0f;
    }
    float val = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);
    return val;
}

//get position of object from Lua
int LuaSystem::Lua_getPosition(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj) return 0;

    //get the object Transform component
    Transform* transform = obj->getComponent<Transform>();
    if (!transform)
        return 0;

    lua_pushnumber(L, transform->x);
    lua_pushnumber(L, transform->y);
    //lua_pushnumber(L, transform->z); //not needed for 2D
    return 2; //return x and y
}

//set position of object from Lua
int LuaSystem::Lua_setPosition(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj)
        return 0;

    Transform* transform = obj->getComponent<Transform>();
    if (!transform)
        return 0;

    float x = static_cast<float>(lua_tonumber(L, 2));
    float y = static_cast<float>(lua_tonumber(L, 3));
    //float z = static_cast<float>(lua_tonumber(L, 4)); //idk if needed cause in the transform i see there is z

    transform->x = x;
    transform->y = y;
    //transform->z = z; //idk if needed cause in the transform i see there is z

    return 0;
}

bool LuaSystem::loadScriptForObject(const std::string& filename, const std::string& tableName) {
    if (!L) return false;

    //reload only if file modified
    static std::unordered_map<std::string, std::filesystem::file_time_type> fileTimes;
    auto currentTime = std::filesystem::last_write_time(filename);
    bool reload = false;

    //check if file have changed to know if need reload
    if (fileTimes.find(filename) == fileTimes.end() || fileTimes[filename] != currentTime) {
        fileTimes[filename] = currentTime;
        reload = true;
    }

    if (reload) {
        //load and execute the script file
        if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
            std::cerr << "Lua error loading " << filename << ": "
                << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }

        // The script should return a table, which is now on top of the stack
        if (!lua_istable(L, -1)) {
            std::cerr << "[Lua] Error: Script " << filename
                << " did not return a table. Make sure it has 'return tableName' at the end."
                << std::endl;
            lua_pop(L, 1);
            return false;
        }

        // Set the returned table as a global with the object name
        lua_setglobal(L, tableName.c_str());

        std::cout << "[Lua] Reloaded script: " << filename << std::endl;
    }

    return true;
}


bool LuaSystem::updateObjectScript(const std::string& tableName, GameObject* obj, float deltaTime) {
    if (!L || !obj) return false;

    lua_getglobal(L, tableName.c_str()); //get table
    lua_getfield(L, -1, "Update");       //get table.Update from lua script
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return false;
    }

    lua_pushlightuserdata(L, obj);
    lua_pushnumber(L, deltaTime);

    //call the Update function with 2 arguments and 0 return values
    if (lua_pcall(L, 2, 0, 0) != LUA_OK)
        std::cerr << "Lua Update error: " << lua_tostring(L, -1) << std::endl;

    lua_pop(L, 1); //pop table

    return true;
}

//for message
int LuaSystem::Lua_SendInputEvent(lua_State* L) {
    LuaSystem* self = static_cast<LuaSystem*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!self || !self->messageBus) return 0;

    const char* key = lua_tostring(L, 1);
    float x = static_cast<float>(lua_tonumber(L, 2));
    float y = static_cast<float>(lua_tonumber(L, 3));

    //to set the keys 
    KeyEvent evt;
    evt.key = key ? key : "";
    evt.pressed = true;
    evt.x = x;
    evt.y = y;

    self->messageBus->publish(Message("KeyPressed", nullptr, evt));

    return 0;
}

//lua updates for all objects with LuaScript component
void LuaSystem::update(GameObjectManager& manager, float deltaTime) {
    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    for (GameObject* obj : gameObjects) {
        if (obj->hasComponent<LuaScript>()) {
            LuaScript* scriptComp = obj->getComponent<LuaScript>();
            updateObjectScript(scriptComp->scriptName, obj, deltaTime);
        }
    }
}
