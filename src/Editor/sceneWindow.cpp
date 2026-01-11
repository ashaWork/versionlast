/* Start Header ************************************************************************/
/*!
\file       sceneWindow.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of Scene in editor as well as the
            popup for scene manipulation (load/create/duplicate/save). Rendering of gizmo
            is also included here.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifdef _DEBUG
#include "Editor/editorManager.h"

bool SceneWindow::s_isSceneHovered = false;

SceneWindow::SceneWindow(AssetBrowser& aBrowser, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate, Editor::SceneWindowState& Sstate, Editor::SceneState& sceneState, Editor::GizmoState& gizmoState) :
    m_assetBrowser(aBrowser), 
    m_objSelectionState(Ostate),
    m_menuBarState(Mstate),
    m_sceneWindowState(Sstate),
    m_sceneState(sceneState),
    m_gizmoState(gizmoState) {}

void SceneWindow::render(GLuint texture, float aspectRatio, GameObjectManager& manager) {
    /* ---------- Draw Editor Camera Window --------- */
    editorCameraControls();
    /* ---------- END --------- */


    ImGui::Begin("Scene", nullptr);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 size = avail;

    // calculate how much to scale based on aspect ratio
    float availAspect = avail.x / avail.y;
    if (availAspect > aspectRatio) size.x = avail.y * aspectRatio;
    else size.y = avail.x / aspectRatio;

    // center in the available space
    ImVec2 cursor = ImGui::GetCursorPos();
    ImVec2 offset = { (avail.x - size.x) * 0.5f, (avail.y - size.y) * 0.5f };
    if (offset.x < 0) offset.x = 0;
    if (offset.y < 0) offset.y = 0;
    ImGui::SetCursorPos({ cursor.x + offset.x, cursor.y + offset.y });


    // draw the scene in a child window to prevent moving/resizing this child viewport
    ImGui::BeginChild("SceneViewport", size, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Image((void*)(intptr_t)texture, size, ImVec2(0, 1), ImVec2(1, 0));

    m_sceneWindowState.scenePos = ImGui::GetItemRectMin();
    m_sceneWindowState.sceneSize = ImGui::GetItemRectSize();

    s_isSceneHovered = ImGui::IsWindowHovered();

    if (ImGui::IsWindowHovered())
    {
        // Pan with middle mouse button (and in editing mode)
        if (InputHandler::isMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE) && EditorManager::isEditingMode())
        {
            renderer::editorCam.pan(InputHandler::getMouseDeltaWorldInViewport(m_sceneWindowState.sceneSize));
        }

        // zoom in/out with middle mouse button (and in editing mode)
        if (InputHandler::getMouseScroll() != 0.f && EditorManager::isEditingMode())
        {
            renderer::editorCam.zoomInOut(InputHandler::getMouseScroll());
        }

        // left click to select obj
        if (InputHandler::isMouseLeftClicked() && !ImGuizmo::IsUsing()) {
            objPicking(manager);
        }

        // drag to reposition obj
        if (InputHandler::isMouseDragging() && !ImGuizmo::IsUsing()) {
            objDragging();
        }

        if (InputHandler::isMouseLeftReleased()) {
            if (UndoRedoManager::Instance().isEditingTransform() && m_objSelectionState.draggedObject) {
                UndoRedoManager::Instance().endTransformEdit(manager, m_objSelectionState.draggedObject);
            }
        }

    }
    
    /* ---------- ImGuizmo --------- */
    ImGuizmo::BeginFrame();
    // show grid
    if (m_gizmoState.showGrid) {
        drawGrid(renderer::editorCam.view, renderer::editorCam.proj, manager);
    }
    // render gizmo
    renderGizmo(manager);
    /* ---------- END --------- */

    ImGui::EndChild();

    ImGui::End();


    /* ---------- Popup Modal Call --------- */
    newScenePopup(manager);
    duplicateScenePopup(manager);
    loadScenePopup(manager);
    /* ---------- END --------- */
}

void SceneWindow::renderGizmo(GameObjectManager& manager) {
    // early return if no obj selected OR playing simulation OR gizmo is off
    if (!m_objSelectionState.selectedObject || !EditorManager::isEditingMode()) return;
    if (m_gizmoState.currentOp == Editor::GIZMO_NONE) return;

    GameObject* selected = m_objSelectionState.selectedObject;
    Transform* t = selected->getComponent<Transform>();
    if (!t) return;

    // for 2D editors
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();
    // define the screen area where gizmo will appear (follow the Scene viewport)
    ImGuizmo::SetRect(m_sceneWindowState.scenePos.x, m_sceneWindowState.scenePos.y,
        m_sceneWindowState.sceneSize.x, m_sceneWindowState.sceneSize.y); 

    glm::mat4 view = renderer::editorCam.view;
    glm::mat4 proj = renderer::editorCam.proj;

    // set up transform matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(t->x, t->y, t->z));
    model = glm::rotate(model, glm::radians(t->rotation), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(t->scaleX, t->scaleY, t->scaleZ));

    //Mat44 model;
    //Mat44_Identity(&model);
    //
    //// Create individual transformation matrices using your mathlib
    //Mat44 translationMat, rotationMat, scaleMat;
    //
    //// Translation
    //Mat44_Trans(&translationMat, t->x, t->y, t->z);
    //
    //// Rotation (Z-axis for 2D)
    //Mat44_RotZ(&rotationMat, DegToRad(t->rotation));
    //
    //// Scale
    //Mat44_Scale(&scaleMat, t->scaleX, t->scaleY, t->scaleZ);
    //
    //// Combine: model = translation * rotation * scale
    //Mat44 temp;
    //Mat44_Con_cat(&temp, &rotationMat, &scaleMat);
    //Mat44_Con_cat(&model, &translationMat, &temp);

    // set up snapping
    float* snapPtr = nullptr;
    float snapValue[3];
    if (m_gizmoState.useSnap) {
        if (m_gizmoState.currentOp == Editor::GIZMO_ROTATE) {
            snapValue[0] = snapValue[1] = snapValue[2] = m_gizmoState.snapAngle;
        }
        else {
            snapValue[0] = m_gizmoState.snapValues[0];
            snapValue[1] = m_gizmoState.snapValues[1];
            snapValue[2] = m_gizmoState.snapValues[2];
        }
        snapPtr = snapValue;
    }

    static bool wasUsingGizmo = false;
    bool isUsingGizmo = ImGuizmo::IsUsing();

    // Detect gizmo drag start
    if (isUsingGizmo && !wasUsingGizmo) {
        UndoRedoManager::Instance().beginTransformEdit(selected);
    }
    
    // draw gizmo and settle the changes
    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
        static_cast<ImGuizmo::OPERATION>(m_gizmoState.currentOp), m_gizmoState.currentMode,
        glm::value_ptr(model), nullptr, snapPtr)) {

        // break matrix back into t, r, s
        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model),
            glm::value_ptr(translation),
            glm::value_ptr(rotation),
            glm::value_ptr(scale));

        // update the object
        t->x = translation.x;
        t->y = translation.y;
        t->z = translation.z;
        t->rotation = rotation.z;
        t->scaleX = scale.x;
        t->scaleY = scale.y;
        t->scaleZ = scale.z;

        // if collider size set to auto-fit, update its size to match mesh size
        if (CollisionInfo* collision = selected->getComponent<CollisionInfo>()) {
            if (collision->autoFitScale) {
                collision->colliderSize.x = t->scaleX;
                collision->colliderSize.y = t->scaleY;
            }
        }
    }

    if (!isUsingGizmo && wasUsingGizmo) {
        UndoRedoManager::Instance().endTransformEdit(manager, selected);
    }
    wasUsingGizmo = isUsingGizmo;

    /*DebugLog::addMessage(
        "Gizmo - Op: " + std::to_string(m_gizmoState.currentOp) +
        ", Mode: " + std::to_string(m_gizmoState.currentMode) +
        ", IsOver: " + std::to_string(ImGuizmo::IsOver()) +
        ", IsUsing: " + std::to_string(ImGuizmo::IsUsing()) +
        ", Bounds: " + std::to_string(m_sceneWindowState.scenePos.x) + "," +
        std::to_string(m_sceneWindowState.scenePos.y) + " " +
        std::to_string(m_sceneWindowState.sceneSize.x) + "x" +
        std::to_string(m_sceneWindowState.sceneSize.y) + "\n"
    );*/
}

void SceneWindow::drawGrid(const glm::mat4& view, const glm::mat4& proj, GameObjectManager& manager) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    for (GameObject* obj : gameObjects)
    {
        if (!obj->hasComponent<TileMap>() || !obj->hasComponent<Transform>())
            continue;

        TileMap* tm = obj->getComponent<TileMap>();
        Transform* transform = obj->getComponent<Transform>();
        int gridLineX = tm->columns;
        int gridLineY = tm->rows;

        ImU32 gridColor = IM_COL32(100, 100, 100, 100); // dark dark for the normal grid
        ImU32 axisColorX = IM_COL32(255, 80, 80, 150); // red for x axis
        ImU32 axisColorY = IM_COL32(80, 255, 80, 150); // green for y-axis

        // convert world space to screen space (with offset)
        auto worldToScreen = [&](float x, float y, float z) -> ImVec2 {
            glm::vec4 worldPos(x * tm->tileW + m_gizmoState.gridOffset.x + transform->x,
                y * tm->tileH + m_gizmoState.gridOffset.y + transform->y,
                z, 1.0f);
            glm::vec4 clipPos = proj * view * worldPos;

            if (clipPos.w != 0.0f) {
                clipPos /= clipPos.w;
            }

            float screenX = m_sceneWindowState.scenePos.x +
                (clipPos.x * 0.5f + 0.5f) * m_sceneWindowState.sceneSize.x;
            float screenY = m_sceneWindowState.scenePos.y +
                (1.0f - (clipPos.y * 0.5f + 0.5f)) * m_sceneWindowState.sceneSize.y;

            return ImVec2(screenX, screenY);
            };

        float extentX = (float)(gridLineX * m_gizmoState.gridSpacing);
        float extentY = (float)(gridLineY * m_gizmoState.gridSpacing);

        // draw vertical lines
        for (int i = -gridLineX; i <= gridLineX; ++i) {
            float x = (float)(i * m_gizmoState.gridSpacing);
            ImVec2 start = worldToScreen(x, -extentY, transform->z);
            ImVec2 end = worldToScreen(x, extentY, transform->z);

            ImU32 color = (i == 0) ? axisColorY : gridColor;
            float thickness = (i == 0) ? 2.0f : 1.0f;

            drawList->AddLine(start, end, color, thickness);
        }

        // draw horizontal lines
        for (int i = -gridLineY; i <= gridLineY; ++i) {
            float y = (float)(i * m_gizmoState.gridSpacing);
            ImVec2 start = worldToScreen(-extentX, y, transform->z);
            ImVec2 end = worldToScreen(extentX, y, transform->z);

            ImU32 color = (i == 0) ? axisColorX : gridColor;
            float thickness = (i == 0) ? 2.0f : 1.0f;

            drawList->AddLine(start, end, color, thickness);
        }
    }

}

void SceneWindow::editorCameraControls() {
    // edit camera position and zoom for editor camera
    if (ImGui::Begin("Editor Camera")) {
        // change the centre pos of the camera
        ImGui::DragFloat3("Position", glm::value_ptr(renderer::editorCam.campos), 0.1f);

        // change zoom level of the camera
        ImGui::DragFloat("Zoom", &renderer::editorCam.zoom, 0.1f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

        // change zoom factor (how fast zoom in/out)
        ImGui::DragFloat("Zoom Factor", &renderer::editorCam.zoomFactor, 0.01f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

        // set min zoom range (able to zoom in closer than using mouse)
        ImGui::DragFloat("Min Zoom", &renderer::editorCam.minZoom, 0.1f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

        // set max zoom range (able to zoom out further than using mouse)
        ImGui::DragFloat("Max Zoom", &renderer::editorCam.maxZoom, 0.1f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

        ImGui::Separator();
        if (ImGui::Button("Reset Camera")) {
            renderer::editorCam.campos = glm::vec3(0.f);
            renderer::editorCam.zoom = 10.f;
        }
    }
    ImGui::End();

}

void SceneWindow::newScene(GameObjectManager& manager) {
    //std::string folder = std::string(SOURCE_SCENE_DIR_R);
    std::string base = "untitled_scene";
    std::string extension = ".json";
    std::string newSceneName;

    int copy = 1;
    bool nameExists;
    do {
        nameExists = false;
        newSceneName = base + "(" + std::to_string(copy) + ")" + extension;

        for (const Editor::Asset& asset : m_assetBrowser.getAssets()) {
            if (asset.name == std::filesystem::path(newSceneName).filename().string()) {
                nameExists = true;
                break;
            }
        }

        copy++;
    } while (nameExists);

    m_sceneState.currentSceneName = newSceneName;
    manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName), true);
    JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);
    EditorManager::assetChanged();

    resetSelection();

    DebugLog::addMessage("New scene " + m_sceneState.currentSceneName + " created.\n");
}

// dup current scene and open
void SceneWindow::duplicateScene(GameObjectManager& manager) {
    if (!m_sceneState.currentSceneName.empty()) {
        std::filesystem::path originalName(m_sceneState.currentSceneName);
        std::string folder = originalName.parent_path().string();
        std::string filename = originalName.stem().string();
        std::string extension = originalName.extension().string();

        // check for existing names and find a new unique name
        std::string newSceneName;
        int copy = 1;
        bool nameExists;
        do {
            nameExists = false;
            newSceneName = folder + "/" + filename + "_copy(" + std::to_string(copy) + ")" + extension;

            for (const Editor::Asset& asset : m_assetBrowser.getAssets()) {
                if (asset.name == std::filesystem::path(newSceneName).filename().string()) {
                    nameExists = true;
                    break;
                }
            }

            copy++;
        } while (nameExists);

        //manager.saveToFile(newSceneName);
        m_sceneState.currentSceneName = newSceneName;
        manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
        JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

        // if current scene got /, remove it (to compare and highlight this asset in browser)
        if (m_sceneState.currentSceneName[0] == '/') {
            m_sceneState.currentSceneName.erase(0, 1);
        }
        EditorManager::assetChanged();

        resetSelection();
        DebugLog::addMessage("Scene duplicated to: " + newSceneName + "\n");
    }
    else {
        DebugLog::addMessage("No current scene to duplicate.\n");
    }
}

// load existing scene
void SceneWindow::loadScene(GameObjectManager& manager) {
    // reset cam pos
    renderer::editorCam.campos = glm::vec3(0.f);
    renderer::editorCam.zoom = 10.f;

    resetSelection();

    // clear all undo/redo
    UndoRedoManager::Instance().clear();

    //manager.loadFromFile(m_MenuBarState.sceneToLoad);
    manager.loadScene(JsonIO::runtimeScenePath(m_menuBarState.sceneToLoad));
    manager.initializeSceneResources();

    m_sceneState.currentSceneName = std::move(m_menuBarState.sceneToLoad); 

    DebugLog::addMessage("Loaded scene: " + m_sceneState.currentSceneName + "\n");
}

void SceneWindow::newScenePopup(GameObjectManager& manager) {
    // open popup if flagged
    if (m_menuBarState.showNewScenePopup) {
        if (EditorManager::isEditingMode()) ImGui::OpenPopup("Confirm New Scene"); // only load scene when not playing simulation
        else {
            DebugLog::addMessage("Playing simulation, cannot load another scene.\n");
            DebugLog::addMessage("Playing simulation, cannot load another scene.\n", DebugMode::PlaySimul);
        }
        m_menuBarState.showNewScenePopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Create a new scene? Unsaved changes will be lost.");

        // Save and open new scene
        if (ImGui::Button("Save and Create New") || InputHandler::isKeyTriggered(GLFW_KEY_1)) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

            // create new scene
            newScene(manager);

            // load new scene
            m_menuBarState.sceneToLoad = m_sceneState.currentSceneName;
            loadScene(manager);

            ImGui::CloseCurrentPopup();
        }

        // Discard changes and open new scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes") || InputHandler::isKeyTriggered(GLFW_KEY_2)) {

            // create new scene
            newScene(manager);

            // load new scene
            m_menuBarState.sceneToLoad = m_sceneState.currentSceneName;
            loadScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Cancel duplicate and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || InputHandler::isKeyTriggered(GLFW_KEY_3)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SceneWindow::duplicateScenePopup(GameObjectManager& manager) {
    if (m_menuBarState.showDuplicatePopup) {
        m_menuBarState.showDuplicatePopup = false;
        if (EditorManager::isEditingMode()) ImGui::OpenPopup("Confirm Duplicate Scene"); // only duplicate scene when not playing simulation
        else {
            DebugLog::addMessage("Playing simulation, cannot duplicate scene.\n");
            DebugLog::addMessage("Playing simulation, cannot duplicate scene.\n", DebugMode::PlaySimul);
        }
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm Duplicate Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Duplicate and open new a scene? Unsaved changes will be lost.");

        // Save and open newly duplicated scene
        if (ImGui::Button("Save and Duplicate") || InputHandler::isKeyTriggered(GLFW_KEY_1)) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

            duplicateScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Discard changes and open newly duplicated scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes") || InputHandler::isKeyTriggered(GLFW_KEY_2)) {

            duplicateScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Cancel duplicate and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || InputHandler::isKeyTriggered(GLFW_KEY_3)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SceneWindow::loadScenePopup(GameObjectManager& manager) {
    if (m_menuBarState.showLoadScenePopup) {
        m_menuBarState.showLoadScenePopup = false;
        if (EditorManager::isEditingMode()) ImGui::OpenPopup("Confirm Load Scene"); // only load scene when not playing simulation
        else {
            DebugLog::addMessage("Playing simulation, cannot load scene.\n");
            DebugLog::addMessage("Playing simulation, cannot load scene.\n", DebugMode::PlaySimul);
        }
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        // show different title if reloading current scene
        std::string title = (m_menuBarState.sceneToLoad == m_sceneState.currentSceneName) ? "Reload current scene? Unsaved changes will be lost." : "Load another scene? Unsaved changes will be lost";

        ImGui::Text(title.c_str());

        // Save and load another scene
        if (ImGui::Button("Save and Load") || InputHandler::isKeyTriggered(GLFW_KEY_1)) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

            loadScene(manager);
            /*std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            for (GameObject* object : gameObjects) {
                Render* r = object->getComponent<Render>();
                if (r) {
                    r->texChanged = true;
                }
            }*/
            ImGui::CloseCurrentPopup();
        }

        // Discard changes and load selected scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes") || InputHandler::isKeyTriggered(GLFW_KEY_2)) {
            loadScene(manager);

            /*std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            for (GameObject* object : gameObjects) {
                Render* r = object->getComponent<Render>();
                if (r) {
                    r->texChanged = true;
                }
            }*/
            ImGui::CloseCurrentPopup();
        }

        // Cancel loading and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || InputHandler::isKeyTriggered(GLFW_KEY_3)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

/*
Future implementation (jic i forget)
- Add snapping to grid or axis (e.g. hold Shift for 1-unit steps)

- Highlight selected object while dragging

- Undo/redo
*/
void SceneWindow::objPicking(GameObjectManager& manager) {
    Vector2D mousePos = InputHandler::getMousePositionInImGuiViewport(m_sceneWindowState.scenePos, m_sceneWindowState.sceneSize);

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    GameObject* selected = nullptr;
    GameObject* topmost = nullptr;
    float highestZ = -INFINITY;

    for (auto* obj : gameObjects) {
        // dont let user click and select platform
        if (obj->getObjectName() == "platform") continue;

        Transform* transform = obj->getComponent<Transform>();
        if (!transform) continue;

        Collision::AABB aabb = Collision::getObjectAABB(transform);
        Collision::Circle mousePoint(mousePos, 0.2f);

        // check if mouse point intersects with the game obj
        if (Collision::CollisionIntersection_CircleAABB_Static(mousePoint, aabb)) {
            if (transform->z > highestZ) {
                highestZ = transform->z;
                topmost = obj;
            }
        }
    }

    selected = topmost;
    // if something is selected, update selection state
    if (selected) {
        m_objSelectionState.selectedPrefab = nullptr;
        m_objSelectionState.selectedObject = selected;
        m_objSelectionState.draggedObject = selected;

        UndoRedoManager::Instance().beginTransformEdit(selected);

        auto iterator = std::find(gameObjects.begin(), gameObjects.end(), selected);
        if (iterator != gameObjects.end()) {
            m_objSelectionState.selectedIndex = static_cast<int>(std::distance(gameObjects.begin(), iterator));
        }

        DebugLog::addMessage(selected->getObjectName() + " selected");
    }
    else {
        // if mouse click somewhere else, deselect current dragged object
        m_objSelectionState.draggedObject = nullptr;
    }
}

void SceneWindow::objDragging() const {
    // early return if no obj is selected
    GameObject* obj = m_objSelectionState.draggedObject;
    if (!obj) return;

    Transform* transform = obj->getComponent<Transform>();
    if (!transform) return;

    // change current object position based on mouse delta
    Vector2D mousePos = InputHandler::getMouseDeltaWorldInViewport(m_sceneWindowState.sceneSize);
    transform->x += mousePos.x;
    transform->y += mousePos.y;

    // if obj has physics, do not follow gravity while dragging
    Physics* physics = obj->getComponent<Physics>();
    if (!physics) return;

    physics->velY = 0.0f;
}

void SceneWindow::resetSelection() {
    m_objSelectionState.selectedObject = nullptr;
    m_objSelectionState.draggedObject = nullptr;
    m_objSelectionState.selectedPrefab = nullptr;
    m_objSelectionState.selectedIndex = -1;
}
#endif