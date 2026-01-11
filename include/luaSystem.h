/* Start Header ************************************************************************/
/*!
\file       luaSystem.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       Ocotber, 23rd, 2025
\brief      declaration of the LuaSystem class to handle Lua scripting integration

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <iostream>
#include "GameObject.h"
#include "input.h"
#include "Component.h"
#include "messageBus.h"
#include "GameObjectManager.h"
#include "GUISystem.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class LuaSystem {
public:
    LuaSystem() = default;
    ~LuaSystem() = default;

	void init(); //initialize Lua state
    void cleanup(); //clean up Lua state
    bool runString(const std::string& code); //execute Lua code from a string
	bool runScript(const std::string& filename); //execute Lua code from a file
	float getGlobalNumber(const std::string& name); //to get the global number variable from Lua
    bool updateObjectScript(const std::string& tableName, GameObject* obj, float deltaTime); //update the script 
    bool loadScriptForObject(const std::string& filename, const std::string& tableName); //load script
	static int Lua_getPosition(lua_State* L); //get position of object
	static int Lua_setPosition(lua_State* L); //set position of object
	static int Lua_IsKeyHeld(lua_State* L); //check if key is held
	static int Lua_SendInputEvent(lua_State* L); //send input event
	void setMessageBus(MessageBus* bus) { messageBus = bus; } //set message bus
	void update(GameObjectManager& manager, float deltaTime); //update all Lua scripts

private:
	lua_State* L = nullptr; //pointer to Lua state
	MessageBus* messageBus = nullptr; //pointer to message bus
	//std::unique_ptr<GameObjectManager> manager; //pointer to game object manager
	GUISystem* guiSystem = nullptr; //pointer to GUI system
};
