/* Start Header ************************************************************************/
/*!
\file		CoreEngine.h
\author     Hugo Low Ren Hao, low.h, 2402272
\par        low.h@digipen.edu
\date		October, 1, 2025
\brief		Declaration for the CoreEngine class.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <GL/glew.h>
#include <stdexcept>
#include <iostream>
#include "config.h"
#include "debug.h"
#include "input.h"
#include "audio.h"
#include "Systems.h"
#include "luaSystem.h"
#include "ResourceManager.h"
#include "layerManager.h"
#include "prefabManager.h"
#include "GUISystem.h"
#include "messageBus.h"
#include "JsonIO.h"


class CoreEngine {
public:
    CoreEngine();
    ~CoreEngine();

    // Initializes all engine systems and the window
    void Init(bool forceWindowed = false);

    // Runs the main game loop
    void Run();

    // Shuts down all engine systems
    void Shutdown();

    //for resume and pause
    static void windowIconifyCallback(GLFWwindow* window, int iconified);
    static void windowFocusCallback(GLFWwindow* window, int focused);

private:
    void GameLoop();
    void Update(float deltaTime);
    double m_fixedDt = 1.0 / 60.0; // 60 Hz simulation

    // Window and timing variables
    GLFWwindow* m_window;
    double m_fps;
    double m_delta;
    std::string m_title;
    bool m_isRunning;

    bool m_isFullscreen = false;
    int  m_windowedWidth = 0;
    int  m_windowedHeight = 0;
    int  m_windowPosX = 100;
    int  m_windowPosY = 100;
    bool m_forceWindowed = false;
	bool m_isPaused; //for alt-tab pause

    //hardcoded bgm player
    FMOD::Channel* m_bgmChannel = nullptr;
    std::string m_bgmFilePath = "assets/audio/bg.wav";

    void startBGM();
    void stopBGM();
    void pauseBGM();
    void resumeBGM();
    
    void toggleFullscreen();

    std::unique_ptr<GUISystem> m_guiSystem;


    // Core Engine Systems
    std::unique_ptr<GameObjectManager> m_manager;
    std::unique_ptr<InputSystem> m_inputSystem;
    std::unique_ptr<CollisionSystem> m_collisionSystem;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<RenderSystem> m_renderSystem;
#ifdef _DEBUG
    std::unique_ptr<UISystem> m_uiSystem;
#endif
    std::unique_ptr<LuaSystem> m_luaSystem;
    std::unique_ptr<FontSystem> m_fontSystem;
    std::unique_ptr<LogicSystem> m_logicSystem;
    std::unique_ptr<MessageBus> m_messageBus;
    std::unique_ptr<PlayerControllerSystem> m_playerController;
    std::unique_ptr<AudioSystem> m_audioSystem;
    std::unique_ptr<TileMapSystem> m_tileMapSystem;
};
