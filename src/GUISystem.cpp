/* Start Header ************************************************************************/
/*!
\file       GUISystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 10% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 90% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\date       November, 7th, 2025
\brief      Handles initialization, updates, and GUI interactions such as
            creating/removing menu buttons and transitioning between
            MENU, CONTROl and PLAYING states.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "GUISystem.h"
#include "Systems.h"
#include "input.h"
#include "CoreEngine.h"
#include <iostream>
#include <fstream>

static std::string runtimeScenePath(std::string const& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}

GUISystem::GUISystem() :
    m_currentState(GameState::MENU), 
    m_previousState(GameState::MENU),
    m_stateBeforePause(GameState::MENU),
    g_manager(nullptr),
    g_luaSystem(nullptr) {
    std::cout << "[GUI] GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "[GUI] GUISystem destructor called" << std::endl;
}

void GUISystem::init(GameObjectManager& manager, LuaSystem& luaSystem) {
    std::cout << "[GUI] GUISystem::init started" << std::endl;

    // Load the menu scene - this will populate button data in the manager
    loadScreen(manager, "menu_scene.json");

    g_manager = &manager;
    g_luaSystem = &luaSystem;
    std::cout << "[GUI] GUISystem::init completed successfully" << std::endl;
}

//update function to check for button clicking
void GUISystem::update(GameObjectManager& manager) {
    //get mouse position
    Vector2D worldMousePos = InputHandler::getWorldMousePosition();
    Vector2D worldPos = Vec_Set(worldMousePos.x, worldMousePos.y);

    if (UISystem::isShowUI()) return; //skip if UI is shown

    if (!InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
		//when hover over button we can add hover effect here
    }

    else {
        //loop through all buttons in the map
        for (auto& pair : m_buttons) {
            const std::string& name = pair.first;
            GameObject* btn = pair.second;

            if (!btn) continue;

            if (isPointInButton(worldPos, btn)) {
                //handle button action dynamically based on name
                if (name == "play_button") {
                    m_currentState = GameState::PLAYING;
					startGame(manager); //start game
                    return; //stop checking after a click
                }
                else if (name == "exit_button") {
                    //stopGame();
                    m_currentState = GameState::EXITCONFIRMATION;
                    loadScreen(manager, "confirmation_scene.json");
                    return; //stop checking after a click
                }
                else if (name == "control_button") {
                    m_currentState = GameState::CONTROL;
                    loadScreen(manager, "control_scene.json");
                    return; //stop checking after a click
                }
                //idk if we need more buttons
            }
        }
    }
}

//updating the control screen 
void GUISystem::controlUpdate(GameObjectManager& manager) {
	//get mouse position
    Vector2D worldMousePos = InputHandler::getWorldMousePosition();
    Vector2D worldPos = Vec_Set(worldMousePos.x, worldMousePos.y);

    if (UISystem::isShowUI()) return; //skip if UI is shown

    if (!InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
        //when hover over button we can add hover effect here
    }
    else {
        //loop through all buttons in the map
        for (auto& pair : m_buttons) {
            const std::string& name = pair.first;
            GameObject* btn = pair.second;

            if (!btn) continue;

            if (isPointInButton(worldPos, btn)) {
                //handle button action dynamically based on name
                if (name == "back_button") {
                    if (m_previousState == GameState::PAUSED) {
                        loadScreen(manager, "pause_scene.json");
						m_currentState = GameState::PAUSED;
                    }
                    else {
                        m_currentState = GameState::MENU;
                        loadScreen(manager, "menu_scene.json");
                    }                    
                    return; //stop checking after a click
                }
                //idk if we need more buttons
            }
        }
    }
}

//updating for exit confirmation
void GUISystem::exitUpdate(GameObjectManager& manager) {
    //get mouse position
    Vector2D worldMousePos = InputHandler::getWorldMousePosition();
    Vector2D worldPos = Vec_Set(worldMousePos.x, worldMousePos.y);

    if (UISystem::isShowUI()) return; //skip if UI is shown

    if (!InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
        //when hover over button we can add hover effect here
    }
    else {
        //loop through all buttons in the map
        for (auto& pair : m_buttons) {
            const std::string& name = pair.first;
            GameObject* btn = pair.second;

            if (!btn) continue;

            if (isPointInButton(worldPos, btn)) {
                //handle button action dynamically based on name
                if (name == "yes_button") {
                    m_currentState = GameState::EXIT;
                    stopGame();
                    return; //stop checking after a click
                }
                else if(name == "no_button") {
                    m_currentState = GameState::MENU;
                    loadScreen(manager, "menu_scene.json");
                    return; //stop checking after a click
				}
                //idk if we need more buttons
            }
        }
    }
}

//updating for pause confirmation
void GUISystem::pauseUpdate(GameObjectManager& manager) {
    //get mouse position
    Vector2D worldMousePos = InputHandler::getWorldMousePosition();
    Vector2D worldPos = Vec_Set(worldMousePos.x, worldMousePos.y);

    if (UISystem::isShowUI()) return; //skip if UI is shown

    if (!InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
        //when hover over button we can add hover effect here
    }
    else {
        //loop through all buttons in the map
        for (auto& pair : m_buttons) {
            const std::string& name = pair.first;
            GameObject* btn = pair.second;

            if (!btn) continue;

            if (isPointInButton(worldPos, btn)) {
                //handle button action dynamically based on name
                if (name == "control_button") {
					m_previousState = m_currentState; //set to pause state before going to control
                    m_currentState = GameState::CONTROL;
                    loadScreen(manager, "control_scene.json");
                    return; //stop checking after a click
                }
                else if (name == "resume_button") {
                    if (m_stateBeforePause == GameState::PLAYING) {
                        m_currentState = GameState::PLAYING;
						//idk as of now we dont have a resume screen to load back to playing
                    }
                    else if (m_stateBeforePause == GameState::MENU){
                        m_currentState = GameState::MENU;
                        loadScreen(manager, "menu_scene.json");
                    }
                    //can have more?
                    return; //stop checking after a click
                }
                else if (name == "menu_button") {
                    m_currentState = GameState::MENU;
                    loadScreen(manager, "menu_scene.json");
                    return; //stop checking after a click
                }
                //idk if we need more buttons
            }
        }
    }
}

//creation of buttons
//can be use for creating buttons in game also
void GUISystem::createButtons(GameObjectManager& manager) {
    //clear previous references
    m_buttons.clear();

	//get all game objects from manager
    std::vector<GameObject*> allObjs;
    manager.getAllGameObjects(allObjs);

    if (allObjs.empty()) return; //check if its empty

	//loop through all objects to find buttons
    for (GameObject* obj : allObjs){
		//get object name
        const std::string name = obj->getObjectName();
        //only consider objects that have both transform and render
        if (!obj->hasComponent<Transform>()) continue;

        //find if its a button
        bool isButton = (name.find("_button") != std::string::npos);
        if (!isButton) {
            manager.assignObjectToLayer(obj, 0); // assign all non-buttons to layer 0
            continue;
        }
        //for font
        if (obj->hasComponent<FontComponent>()) {
            manager.assignObjectToLayer(obj, 1); // or any GUI layer
        }
        else {
            //for button
            manager.assignObjectToLayer(obj, 2);
        }

        m_buttons[name] = obj;
    }
    std::cout << "[GUI] Menu buttons created successfully" << std::endl;
}

//removing of buttons 
void GUISystem::removeButtons(GameObjectManager& manager) {
    //delete button objects
    for (auto& pair : m_buttons) {
		GameObject* btn = pair.second; //get button object
        if (btn) {
			manager.deleteGameObject(btn); //delete from manager
        }
    }
    m_buttons.clear(); //clear map
}

void GUISystem::startGame(GameObjectManager& manager) {
    std::cout << "[GUI] Play button clicked - starting game" << std::endl;
    
    // Remove menu buttons
    removeButtons(manager);

    // Load the game scene
    manager.init();
    manager.initializeSceneResources();

    // Change game state
    m_currentState = GameState::PLAYING;

    std::vector<GameObject*> gameObjects;
    g_manager->getAllGameObjects(gameObjects);
    //iterate over all Lua scripts in the folder
    for (const auto& file : std::filesystem::directory_iterator("./assets/scripting")) {
        if (file.path().extension() != ".lua") continue; //have to be .lua files if not it will skip
        std::string filename = file.path().filename().string();

        //strip ".lua" to get object name
        std::string objectName = filename.substr(0, filename.size() - 4);

        GameObject* obj = g_manager->getGameObject(objectName);
        if (obj) {
            obj->addComponent<LuaScript>(file.path().string()); //add LuaScript component to object
            //load Lua script into Lua state
            if (!g_luaSystem->loadScriptForObject(file.path().string(), objectName)) {
                std::cerr << "[Lua] Failed to load script for " << objectName << std::endl;
            }
            else {
                std::cout << "[Lua] Loaded script " << filename << " for object " << objectName << std::endl;
            }
        }
    }
    std::cout << "[GUI] Game started - loaded json" << std::endl;
}

//stop game and quit
void GUISystem::stopGame() {
    std::cout << "[GUI] Stop button clicked - quitting application" << std::endl;

    // Close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
}

//check if mouse is pointing the button
bool GUISystem::isPointInButton(const Vector2D& point, GameObject* button) {
    if (!button || !button->hasComponent<Transform>()) return false;

    Transform* transform = button->getComponent<Transform>();

    // Simple AABB collision check for button
    float halfWidth = transform->scaleX * 0.5f;
    float halfHeight = transform->scaleY * 0.5f;

    return (point.x >= transform->x - halfWidth &&
        point.x <= transform->x + halfWidth &&
        point.y >= transform->y - halfHeight &&
        point.y <= transform->y + halfHeight);
}

//load a specific screen using the filename
void GUISystem::loadScreen(GameObjectManager& manager, const std::string& sceneName) {
    std::cout << "[GUI] Load Screen: " << sceneName << std::endl;
    removeButtons(manager);
    manager.loadScene(runtimeScenePath(sceneName));
    manager.initializeSceneResources(); //to get resources loaded
    createButtons(manager); //loading scene
    std::cout << "[GUI] Load Screen created successfully!" << std::endl;
}
