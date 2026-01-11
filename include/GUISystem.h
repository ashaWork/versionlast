/* Start Header ************************************************************************/
/*!
\file       GUISystem.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 10% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 90% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\date       November, 7th, 2025
\brief      Consists of declarations for managing creation and removal
//          of menu buttons, detects button clicks, and updates game state
//          accordingly.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "GameObjectManager.h"
#include "input.h"
#include <string>
#include <unordered_map>

class LuaSystem;

enum class GameState {
    MENU,
    PLAYING,
    CONTROL,
    PAUSED,
    EXIT,
    EXITCONFIRMATION,
	LEVELS //maybe for future use
};

class GUISystem {
public:
    GUISystem();
    ~GUISystem();

    void init(GameObjectManager& manager, LuaSystem& luaSystem);
	void update(GameObjectManager& manager); //update menu system
	void controlUpdate(GameObjectManager& manager); //update control screen
	void exitUpdate(GameObjectManager& manager); //update exit confirmation screen
	void pauseUpdate(GameObjectManager& manager); //update pause screen
    void loadScreen(GameObjectManager& manager, const std::string& sceneName); //load a screen

    GameState getCurrentState() const { return m_currentState; } //check current state
	GameState getPreviousState() const { return m_previousState; } //check previous state
	void setCurrentState(GameState state) { m_currentState = state; } //set current state
	void setPreviousState(GameState state) { m_previousState = state; } //set previous state
	void setStateBeforePause(GameState state) { m_stateBeforePause = state; } //set state before pause

private:
    void createButtons(GameObjectManager& manager); //create menu buttons
    void removeButtons(GameObjectManager& manager); //remove menu buttons
	void startGame(GameObjectManager& manager); //start game
    void stopGame(); //stop game	

    bool isPointInButton(const Vector2D& point, GameObject* button);

    std::unordered_map<std::string, GameObject*> m_buttons; //for storing buttons 

    GameState m_currentState;
	GameState m_previousState;
    GameState m_stateBeforePause;
    GameObjectManager* g_manager = nullptr;
    LuaSystem* g_luaSystem = nullptr;
};