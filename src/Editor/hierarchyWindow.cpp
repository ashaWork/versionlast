/* Start Header ************************************************************************/
/*!
\file       hierarchyWindow.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of Hierarchy in editor. Also
            include the right click context menu for game object manipulation (dup/delete)
            and creation of new object/prefab.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Editor/editorManager.h"

HierarchyWindow::HierarchyWindow(Editor::ObjSelectionState& Ostate) : m_objSelectionState(Ostate) {}

void HierarchyWindow::render(GameObjectManager& manager) {
    ImGui::Begin("Hierarchy");

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    // by default just set to no children
    ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Leaf;

    for (size_t i = 0; i < gameObjects.size(); i++) {
        // if no children (in the future)
    //    baseFlags |= ImGuiTreeNodeFlags_Leaf;

        // highlight selected row
        if (i == m_objSelectionState.selectedIndex) {
            baseFlags |= ImGuiTreeNodeFlags_Selected;
        }
        else {
            baseFlags &= ~ImGuiTreeNodeFlags_Selected;
        }

        // draw tree node for each object
        if (ImGui::TreeNodeEx(gameObjects[i]->getObjectName().c_str(), baseFlags)) {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                m_objSelectionState.selectedIndex = static_cast<int>(i);
                m_objSelectionState.selectedObject = gameObjects[i];
            }

            /* ------- right click context menu ------- */
            showContextMenu(manager, gameObjects[i], static_cast<int>(i));

            ImGui::TreePop();

        }
    }

    if (gameObjects.empty()) {
        m_objSelectionState.selectedObject = nullptr;
        m_objSelectionState.draggedObject = nullptr;
        m_objSelectionState.selectedIndex = -1;

        ImGui::Text("No GameObjects available");
        if (ImGui::Button("Create Empty")) {
            AddObjWindow::createEmpty(manager);
        }
    }

    /* ----------- Receive Dropped Prefab from Asset Browser ----------- */
    // make entire hierarchy viewport a invisible button that can receive asset
    //ImGuiID targetID = ImGui::GetID("HierarchyWindowDropTarget");
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::InvisibleButton("##drop_target", ImGui::GetContentRegionAvail());

    // accept the prefab drop
    handlePrefabDragDrop(manager);
    /* ---------- END --------- */

    ImGui::End();

    /* ---------- Popup Modal Call --------- */
    objDeletePopup(manager);
    createPrefabPopup();
    /* ---------- END --------- */
}

void HierarchyWindow::handlePrefabDragDrop(GameObjectManager& manager) {
    // receive prefab and make it into a game obj referecing the prefab
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
            const char* prefabPath = static_cast<const char*>(payload->Data);
            std::filesystem::path path = prefabPath;
            std::string folder = path.parent_path().filename().string();

            // only accept if its in prefab folder
            if (folder == "Prefab") {
                std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(prefabPath);

                std::string objName = PrefabManager::Instance().getPrefabName(prefabID);
                GameObject* prefabObj = manager.createGameObject(objName + "(" + std::to_string(manager.getGameObjectCount()) + ")");
                prefabObj->getObjectPrefabID() = prefabID;

                if (prefabObj) {
                    PrefabManager::Instance().instantiate(prefabObj, manager);
                    Render* render = prefabObj->getComponent<Render>();
                    if (render) render->texChanged = true;

                    DebugLog::addMessage("Added game obj from prefab\n");
                }
                else {
                    Editor::popupState.prefabFileTypePopup = true;
                    Editor::popupState.filename = path.filename().string();
                    DebugLog::addMessage("Failed to receive\n");
                }
            }
            else {
                Editor::popupState.prefabFileTypePopup = true;
                Editor::popupState.filename = path.filename().string();

                DebugLog::addMessage("Wrong prefab file received: " + path.filename().string() + "\n");
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void HierarchyWindow::showContextMenu(GameObjectManager& manager, GameObject* obj, int objIndex) {
    /* ------- right click context menu ------- */
    if (ImGui::BeginPopupContextItem()) {

        if (ImGui::MenuItem("Duplicate")) {
            AddObjWindow::dupObj(manager, obj);
        }

        if (ImGui::MenuItem("Delete Object")) {
            m_objSelectionState.selectedIndex = objIndex;
            m_objSelectionState.selectedObject = obj;
            m_objSelectionState.showDeletePopup = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::MenuItem("Save as Prefab")) {
            m_objSelectionState.selectedIndex = objIndex;
            m_objSelectionState.selectedObject = obj;
            m_objSelectionState.showCreatePrefabPopup = true;
        }

        // if selected object is ref a prefab
        if (!obj->getObjectPrefabID().empty()) {
            if (ImGui::MenuItem("Edit Prefab")) {
                std::unique_ptr<GameObject> tempPrefab = PrefabManager::Instance().createTempPrefabObj(obj->getObjectPrefabID());
                if (tempPrefab) {
                    m_objSelectionState.selectedObject = nullptr;
                    m_objSelectionState.selectedIndex = -1;
                    m_objSelectionState.selectedPrefab = std::move(tempPrefab);

                    DebugLog::addMessage("Open Prefab " + m_objSelectionState.selectedPrefab->getObjectName() + ".");
                }
                else {
                    DebugLog::addMessage("Failed to open prefab " + obj->getObjectPrefabID() + " object for editing.\n");
                }
            }

            if (ImGui::MenuItem("Revert to Prefab")) {
                obj = PrefabManager::Instance().revertToPrefab(obj, manager);

                DebugLog::addMessage("Reverted " + obj->getObjectName() + " to its prefab" + obj->getObjectPrefabID() + ".");
            }

            if (ImGui::MenuItem("Unpack Prefab")) {
                obj->getObjectPrefabID().clear();
                DebugLog::addMessage("Unpacked prefab: " + obj->getObjectName() + " is now a regular object.");
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::MenuItem("Create Empty")) {
            AddObjWindow::createEmpty(manager);
        }

        if (ImGui::MenuItem("Create Object")) {
            AddObjWindow::createRec(manager);
        }

        ImGui::EndPopup();
    }
}

void HierarchyWindow::objDeletePopup(GameObjectManager& manager) {
    // open popup if flagged
    if (m_objSelectionState.showDeletePopup) {
        ImGui::OpenPopup("Confirm Delete Object");
        m_objSelectionState.showDeletePopup = false;
    }


    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    /*---------------- Cannot delete current loaded scene ----------------*/
    // delete confirm popup
    if (ImGui::BeginPopupModal("Confirm Delete Object", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this object?");

        if (ImGui::Button("Delete") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            std::string name = m_objSelectionState.selectedObject->getObjectName();

            //manager.deleteGameObject(m_objSelectionState.selectedObject); -> CREATE DELETE CMD will handle the deletion

            // call CREATE DELETE CMD to settle the deletion, and save in a stack (for undo)
            auto cmd = std::make_unique<DeleteObjCmd>(manager, m_objSelectionState.selectedObject);
            UndoRedoManager::Instance().executeCmd(std::move(cmd));

            m_objSelectionState.selectedObject = nullptr;
            m_objSelectionState.draggedObject = nullptr;
            m_objSelectionState.selectedIndex = -1;

            DebugLog::addMessage("Object " + name + " deleted\n");

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void HierarchyWindow::createPrefabPopup() {
    // open popup if flagged
    if (m_objSelectionState.showCreatePrefabPopup) {
        ImGui::OpenPopup("Confirm Create Prefab");
        m_objSelectionState.showCreatePrefabPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    /*---------------- Cannot delete current loaded scene ----------------*/
    // delete confirm popup
    if (ImGui::BeginPopupModal("Confirm Create Prefab", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Do you want to pack this object into the new prefab?");

        if (ImGui::Button("Create and Pack")) {

            if (PrefabManager::Instance().createPrefabFromGameObj(m_objSelectionState.selectedObject, "", true))
                DebugLog::addMessage("Prefab created.\n");
            else DebugLog::addMessage("Prefab not created.\n");

            // attach the prefab id to the obj in json (auto save)
            /*manager.saveScene(JsonIO::sourceScenePath(Editor::sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(Editor::sceneState.currentSceneName);*/

            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Create but Do Not Pack")) {

            if (PrefabManager::Instance().createPrefabFromGameObj(m_objSelectionState.selectedObject))
                DebugLog::addMessage("Prefab created.\n");
            else DebugLog::addMessage("Prefab not created.\n");

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
#endif