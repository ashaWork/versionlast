/* Start Header ************************************************************************/
/*!
\file       editorManager.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for Editor Manager class. Initialize the other
            editor class to allow rendering. Also handle shortcut keys in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Editor/editorManager.h"

EditorManager::EditorManager() :
    addObjWindow(Editor::circleInput, Editor::rectangleInput),
    assetBrowser(Editor::assetBrowserState, Editor::objSelectionState, Editor::menuBarState),
    editorDebugWindow(DebugMode::Editor),
    playDebugWindow(DebugMode::PlaySimul),
    hierarchyWindow(Editor::objSelectionState),
    inspectorWindow(Editor::objSelectionState, Editor::gizmoState),
    menuBar(assetBrowser, Editor::assetBrowserState, Editor::menuBarState, Editor::objSelectionState, Editor::sceneState),
    performanceWindow(),
    sceneWindow(assetBrowser, Editor::objSelectionState, Editor::menuBarState, Editor::sceneWindowState, Editor::sceneState, Editor::gizmoState) {}

void EditorManager::update(GameObjectManager& manager) {
    if (assetsChangedFlag) {
        assetBrowser.loadAssetsFromDirectory();
        assetsChangedFlag = false;
    }

    handleShortcutKeys(manager);

    addObjWindow.render(manager);
    assetBrowser.render();
    editorDebugWindow.render();
    hierarchyWindow.render(manager);
    inspectorWindow.render(manager);
    menuBar.render(manager);
    performanceWindow.render();

    // only show play info when not editing
    if (!isEditing) playDebugWindow.render();
}

void EditorManager::renderScene(GLuint texture, float aspectRatio, GameObjectManager& manager) {
    sceneWindow.render(texture, aspectRatio, manager);
}

void EditorManager::handleShortcutKeys(GameObjectManager& manager) {
    // block shortcut if typing
    if (ImGui::GetIO().WantTextInput) {
        return;
    }

    /* --------------- "Q" -> Deselect Obj --------------- */
    if (InputHandler::isKeyTriggered(GLFW_KEY_Q)) {
        if (Editor::objSelectionState.selectedObject) {
            sceneWindow.resetSelection();
        }
    }
    /* --------------- END --------------- */

    /* --------------- "Y" -> Toggle Grid --------------- */
    if (InputHandler::isKeyTriggered(GLFW_KEY_U)) {
        Editor::gizmoState.showGrid = !Editor::gizmoState.showGrid;
    }
    /* --------------- END --------------- */

    /* --------------- GIZMO CONTROL --------------- */
    if (Editor::objSelectionState.selectedObject && !ImGuizmo::IsUsing()) {
        if (InputHandler::isKeyTriggered(GLFW_KEY_W)) {
            Editor::gizmoState.currentOp = Editor::GIZMO_TRANSLATE;
        }
        if (InputHandler::isKeyTriggered(GLFW_KEY_E)) {
            Editor::gizmoState.currentOp = Editor::GIZMO_ROTATE;
        }
        if (InputHandler::isKeyTriggered(GLFW_KEY_R)) {
            Editor::gizmoState.currentOp = Editor::GIZMO_SCALE;
        }
        if (InputHandler::isKeyTriggered(GLFW_KEY_T)) {
            Editor::gizmoState.currentOp = Editor::GIZMO_NONE;
        }
    }
    /* --------------- END --------------- */


    /* --------------- "DEL" -> Delete Game Object --------------- */
    if (InputHandler::isKeyTriggered(GLFW_KEY_DELETE)) {
        if (Editor::objSelectionState.selectedObject) {
            Editor::objSelectionState.showDeletePopup = true;
        }
    }
    /* --------------- END --------------- */


    /* --------------- "CTRL" + "D" -> Duplicate Game Object --------------- */
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_D) ||
        InputHandler::isComboKeyTriggered(GLFW_KEY_D, GLFW_KEY_RIGHT_CONTROL)) {
        if (Editor::objSelectionState.selectedObject) {
            AddObjWindow::dupObj(manager, Editor::objSelectionState.selectedObject);
        }
    }
    /* --------------- END --------------- */


    /* ------------- "CTRL" + "Z" -> Undo ------------- */
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_Z) ||
        InputHandler::isComboKeyTriggered(GLFW_KEY_Z, GLFW_KEY_RIGHT_CONTROL)) {
        if (isEditingMode() && UndoRedoManager::Instance().canUndo()) {
            UndoRedoManager::Instance().undo();
        }
    }
    /* --------------- END --------------- */


    /* ------------- "CTRL" + "Y" -> Redo ------------- */
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_Y) ||
        InputHandler::isComboKeyTriggered(GLFW_KEY_Y, GLFW_KEY_RIGHT_CONTROL)) {
        if (isEditingMode() && UndoRedoManager::Instance().canRedo()) {
            UndoRedoManager::Instance().redo();
        }
    }
    /* --------------- END --------------- */


    /* --------------- "CTRL" + "S" -> Save Scene --------------- */
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_S) ||
            InputHandler::isComboKeyTriggered(GLFW_KEY_S, GLFW_KEY_RIGHT_CONTROL)) {
            if (isEditingMode()) {
                manager.saveScene(JsonIO::sourceScenePath(sceneWindow.getCurrentSceneName()));
                JsonIO::syncSceneToRuntime(sceneWindow.getCurrentSceneName());
            }
            else {
                DebugLog::addMessage("Playing simulation. File cannot be saved.");
                DebugLog::addMessage("Playing simulation. File cannot be saved.", DebugMode::PlaySimul);
            }
        }
    /* --------------- END --------------- */


    /* ------------- "CTRL" + "F5" -> Pause/Resume Simulation ------------- */
    /* ------------- "F5" -> Play/Stop Simulation ------------- */
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_F5) ||
        InputHandler::isComboKeyTriggered(GLFW_KEY_F5, GLFW_KEY_RIGHT_CONTROL)) {
        if (!isEditing) menuBar.pauseResumeSimul();
    }
    else if (InputHandler::isKeyTriggered(GLFW_KEY_F5)) {
        menuBar.toggleSimulation(manager);
    }
    /* --------------- END --------------- */
}
#endif