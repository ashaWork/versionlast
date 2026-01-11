/* Start Header ************************************************************************/
/*!
\file       menuBar.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of the top menu bar in game.
            Also include the setting up of colours for different group in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifdef _DEBUG
#include "Editor/editorManager.h"

MenuBar::MenuBar(AssetBrowser& aBrowser, Editor::AssetBrowserState& Astate, Editor::MenuBarState& Mstate, Editor::ObjSelectionState& Ostate, Editor::SceneState& sceneState):
    m_assetBrowser(aBrowser),
    m_AssetBrowserState(Astate),
    m_menuBarState(Mstate),
    m_objSelectionState(Ostate),
    m_sceneState(sceneState) {}

void MenuBar::render(GameObjectManager& manager){
    ImGui::BeginMainMenuBar();

    /* ------------------- File Menu ------------------- */
    drawFileMenu(manager); // create/load/duplicate/save

    /* ------------------- Edit Menu ------------------- */
    drawEditMenu(); // undo/redo

    /* ------------------- Simulation ------------------- */
    drawSimulationMenu(manager); // play/pause/resume/stop simulation

    /* ------------------- Editor Theme ------------------- */
    drawThemeMenu(); // change theme

    ImGui::EndMainMenuBar();
}

void MenuBar::toggleSimulation(GameObjectManager& manager) {

    if (EditorManager::isEditingMode()) playSimulation(manager);
    else stopSimulation(manager);
}

void MenuBar::pauseResumeSimul() {
    if (!EditorManager::isPaused()) pauseSimulation();
    else if (EditorManager::isPaused()) resumeSimulation();
}

void MenuBar::drawFileMenu(GameObjectManager& manager) {
    if (ImGui::BeginMenu("File")) {

        // create new scene
        if (ImGui::MenuItem("New Scene")) {
            m_menuBarState.showNewScenePopup = true;
        }

        // duplicate scene
        if (ImGui::MenuItem("Duplicate Scene")) {
            m_menuBarState.showDuplicatePopup = true;
        }

        // save current scene to file
        if (ImGui::MenuItem("Save Scene", "CTRL+S")) {
            //manager.saveScene(m_currentSceneName);
            manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);
        }

        // load an existing scene
        if (ImGui::BeginMenu("Load Scene")) {

            // load through the assets to find scene files (all files saved in "Scene" folder)
            for (const Editor::Asset& asset : m_assetBrowser.getAssets()) {
                if (asset.folder == m_AssetBrowserState.currentFolder + "Scene") {
                    // create a submenu for the scenes
                    if (ImGui::MenuItem(asset.name.c_str())) {
                        m_menuBarState.sceneToLoad = asset.name;
                        m_menuBarState.showLoadScenePopup = true;
                    }
                }
            }
            ImGui::EndMenu();

        }

        ImGui::EndMenu();
    }
}

void MenuBar::drawEditMenu() {
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, UndoRedoManager::Instance().canUndo())) {
            UndoRedoManager::Instance().undo();
        }

        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, UndoRedoManager::Instance().canRedo())) {
            UndoRedoManager::Instance().redo();
        }
        ImGui::EndMenu();
    }
}

void MenuBar::drawSimulationMenu(GameObjectManager& manager) {
    if (ImGui::BeginMenu("Simulation")) {
        // show "PLAY" when editing (not playing simulation)
        if (ImGui::MenuItem("Play", "F5", false, EditorManager::isEditingMode())) {
            playSimulation(manager);
        }

        // show PAUSE when not editing and playing simulation
        if (ImGui::MenuItem("Pause", "CTRL+F5", false, !EditorManager::isPaused()&&!EditorManager::isEditingMode())) {
            pauseSimulation();
        }

        // show RESUME when not editing and simulation paused
        if (ImGui::MenuItem("Resume", "CTRL+F5", false, EditorManager::isPaused()&&!EditorManager::isEditingMode())) {
            resumeSimulation();
        }

        // show STOP when playing simulation (not editing)
        if (ImGui::MenuItem("Stop", "F5", false, !EditorManager::isEditingMode())) {
            stopSimulation(manager);
        }

        ImGui::EndMenu();
    }
}

void MenuBar::drawThemeMenu() {
    if (ImGui::BeginMenu("Theme")) {

        if (ImGui::MenuItem("Dark")) ImGui::StyleColorsDark();
        if (ImGui::MenuItem("Light")) ImGui::StyleColorsLight();
        if (ImGui::MenuItem("Classic")) ImGui::StyleColorsClassic();
        if (ImGui::MenuItem("Sin Le Dark")) themeSinLeDark();
        if (ImGui::MenuItem("Sin Le Light")) themeSinLeLight();
        if (ImGui::MenuItem("Forest")) themeForest();
       
        ImGui::EndMenu();
    }
}

void MenuBar::playSimulation(GameObjectManager& manager) {
    EditorManager::toggleEditing(false);

    // save current state to a temp scene file
    manager.saveScene(JsonIO::runtimeScenePath(m_sceneState.tempSceneName));

    manager.initializeSimulationResources();

    DebugLog::addMessage("Simulation started.\n");
}

void MenuBar::stopSimulation(GameObjectManager& manager) {
    EditorManager::toggleEditing(true);
    manager.cleanupSimulationResources();

    // load back the temp scene file to restore state
    manager.loadScene(JsonIO::runtimeScenePath(m_sceneState.tempSceneName));

    manager.initializeSceneResources();

    // clear all undo/redo
    UndoRedoManager::Instance().clear();


    m_objSelectionState.selectedObject = nullptr;
    m_objSelectionState.draggedObject = nullptr;
    m_objSelectionState.selectedPrefab = nullptr;
    m_objSelectionState.selectedIndex = -1;

    DebugLog::clearPlaySimulMsg();

    // remove temp scene file
    const auto tempFull = JsonIO::runtimeScenePath(m_sceneState.tempSceneName);
    if (std::filesystem::exists(tempFull))
        std::filesystem::remove(tempFull);
    DebugLog::addMessage("Simulation stopped.\n");
}

void MenuBar::pauseSimulation() {
    EditorManager::togglePause(true);

    DebugLog::addMessage("Simulation paused.\n");
}

void MenuBar::resumeSimulation() {
    EditorManager::togglePause(false);

    DebugLog::addMessage("Simulation resumed.\n");
}

void MenuBar::themeSinLeDark() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // start with dark theme
    ImGui::StyleColorsDark();

    // button
    colors[ImGuiCol_Button] = ImVec4(0.75f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.45f, 0.65f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.65f, 0.25f, 0.45f, 1.00f);

    // header
    colors[ImGuiCol_Header] = ImVec4(0.75f, 0.35f, 0.55f, 0.45f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.45f, 0.65f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.65f, 0.25f, 0.45f, 1.00f);

    // slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.45f, 0.65f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.75f, 0.35f, 0.55f, 1.00f);

    // tab
    colors[ImGuiCol_Tab] = ImVec4(0.88f, 0.70f, 0.80f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.75f, 0.85f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.80f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.80f, 0.85f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.65f, 0.75f, 1.00f);

    // pinkie borders
    colors[ImGuiCol_Border] = ImVec4(0.40f, 0.20f, 0.30f, 1.00f);
}

void MenuBar::themeSinLeLight() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Light pink backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.90f, 0.92f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.92f, 0.87f, 0.89f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.94f, 0.89f, 0.91f, 0.98f);

    // Dark text for contrast
    colors[ImGuiCol_Text] = ImVec4(0.20f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.45f, 0.48f, 1.00f);

    // Soft pink buttons
    colors[ImGuiCol_Button] = ImVec4(0.85f, 0.65f, 0.75f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.70f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.60f, 0.70f, 1.00f);

    // Input fields
    colors[ImGuiCol_FrameBg] = ImVec4(0.88f, 0.78f, 0.82f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.92f, 0.82f, 0.86f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.96f, 0.86f, 0.90f, 1.00f);

    // tab
    colors[ImGuiCol_Tab] = ImVec4(0.88f, 0.70f, 0.80f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.75f, 0.85f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.80f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.80f, 0.85f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.65f, 0.75f, 1.00f);

    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
}

void MenuBar::themeForest() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Dark forest green background
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.12f, 0.08f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.14f, 0.10f, 1.00f);

    // Light tan text
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.88f, 0.80f, 1.00f);

    // Moss green buttons
    colors[ImGuiCol_Button] = ImVec4(0.35f, 0.55f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.65f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);

    // Dark green frames
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.18f, 0.12f, 1.00f);
}

#endif