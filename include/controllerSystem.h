/* Start Header ************************************************************************/
/*!
\file       controllerSystem.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       November, 4th, 2025
\brief      Handles input events and letting C++ respond directly or forwarding to Lua scripts.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "subscriber.h"
#include "luaSystem.h"
#include <iostream>

//for input
class PlayerControllerSystem : public subscriber {
public:
	void onNotify(const Message& msg) override {
		if (msg.type == "KeyPressed") {
			auto keyEvent = std::any_cast<KeyEvent>(msg.payload);
			if (keyEvent.key == "SPACE") {
				std::cout << "Bullet shooting!\n";
			}
			else if (keyEvent.key == "W") {
				std::cout << "Player moving upwards!\n";
			}
			else if (keyEvent.key == "S") {
				std::cout << "Player moving downwards!\n";
			}
			else if (keyEvent.key == "A") {
				std::cout << "Player moving left side!\n";
			}
			else if (keyEvent.key == "D") {
				std::cout << "Player moving right side!\n";
			}
			else if (keyEvent.key == "0") {
				std::cout << "Toggling UI!\n";
			}
            else if (keyEvent.key == "B") {
                std::cout << "Player is jumping!\n";
            }
		}
	}
};

//for lua
class LuaSubscriber : public subscriber {
public:
    LuaSubscriber(lua_State* L) : L(L) {} //to store lua state 

    void onNotify(const Message& msg) override { //handling message
        if (!L) return; //lua state is null

        //push global Lua function if it exists
        lua_getglobal(L, "onInputEvent"); 

        if (lua_isfunction(L, -1)) {
            try {
				auto keyEvent = std::any_cast<KeyEvent>(msg.payload); //get payload

				lua_pushstring(L, msg.type.c_str()); //push message type
				lua_pushstring(L, keyEvent.key.c_str()); //push key
				lua_pushnumber(L, keyEvent.x); //push x position
				lua_pushnumber(L, keyEvent.y); //push y position

                //call Lua function (4 args, 0 return)
                if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
                    std::cerr << "Lua error: " << lua_tostring(L, -1) << "\n";
                    lua_pop(L, 1); //to remove error message from stack
                }
            }
            catch (const std::bad_any_cast&) { //payload is not an event 
                std::cerr << "Invalid KeyEvent payload in LuaSubscriber\n";
            }
        }
        else {
            lua_pop(L, 1); //remove non-function
        }
    }

private:
    lua_State* L;
};
