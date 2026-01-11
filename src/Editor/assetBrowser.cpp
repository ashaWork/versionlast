/* Start Header ************************************************************************/
/*!
\file       assetBrowser.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of Asset Browser and hot loading of
            assets from the assets folder. Include the actions that be done on a asset 
            (rename/delete/show in explorer/replace).

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Systems.h"
#include "Editor/editorManager.h"

AssetBrowser::AssetBrowser(Editor::AssetBrowserState& Astate, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate) :
    m_AssetBrowserState(Astate),
    m_objSelectionState(Ostate),
    m_menuBarState(Mstate)
{}

void AssetBrowser::render() {
    m_AssetBrowserState.hoverFolder = "";

    ImGui::Begin("Assets");

    // refresh to update assets from the actual asset folder
    // not really needed since we have auto refresh, but still good to have
    if (ImGui::Button("Refresh")) {
        EditorManager::assetChanged();
    }


    /* ------------------ Auto update if changes in project directory ---------------------*/
    // TEMPORARY
    // Check top-level folder got any updates (the asset folder)
    std::filesystem::file_time_type currentTopLevel = std::filesystem::last_write_time(m_AssetBrowserState.currentFolder);
    bool needsRefresh = false;

    if (currentTopLevel != m_AssetBrowserState.lastTopLevelRefresh) {
        m_AssetBrowserState.lastTopLevelRefresh = currentTopLevel;
        needsRefresh = true;
    }

    // Now check subfolders recursively (cheap guard: only if assetsChangedFlag is false)
    std::filesystem::file_time_type newestTime = m_AssetBrowserState.lastRefresh;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetBrowserState.currentFolder)) {
        auto t = std::filesystem::last_write_time(entry);
        if (t > newestTime) {
            newestTime = t;
            needsRefresh = true;
        }
    }

    // update and reload if necessary
    if (needsRefresh) {
        m_AssetBrowserState.lastRefresh = newestTime;
        EditorManager::assetChanged();
    }
    /* ------------------ End ---------------------*/


    ImGui::Separator();

    if (!m_AssetBrowserState.currentViewFolder.empty()) {
        if (ImGui::Button("< Back")) {
            m_AssetBrowserState.currentViewFolder.clear();
        }

        // if inside a folder and drag and drop a file, add it into the folder
        m_AssetBrowserState.hoverFolder = m_AssetBrowserState.currentViewFolder;

        ImGui::SameLine();
        ImGui::Text("Folder: %s", m_AssetBrowserState.currentViewFolder.c_str());
        ImGui::Separator();
    }

    float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

    // show folders & files directly under Assets folder
    if (m_AssetBrowserState.currentViewFolder.empty()) {
        for (const std::string& folder : m_AssetBrowserState.subFolders) {
            ImGui::PushID(folder.c_str());

            ImGui::BeginGroup();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100, 40));

            // folder button
            if (ImGui::Button(folder.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize))) {
                m_AssetBrowserState.currentViewFolder = folder;
            }

            // see if mouse is on which folder (for dropping of files from file explorer)
            if (ImGui::IsItemHovered()) {
                m_AssetBrowserState.hoverFolder = folder;
            }

            ImGui::PopStyleVar();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_AssetBrowserState.iconSize);
            ImGui::Text("%s", folder.c_str());
            ImGui::PopTextWrapPos();

            ImGui::EndGroup();

            float lastButtonX2 = ImGui::GetItemRectMax().x;
            float nextButtonX2 = lastButtonX2 + m_AssetBrowserState.iconSize;
            if (nextButtonX2 < windowVisibleX2) ImGui::SameLine();

            ImGui::PopID();
        }
    }

    int index{};
    for (const Editor::Asset& asset : m_assets) {
        std::string assetFolderName = std::filesystem::path(asset.folder).filename().string();

        if (m_AssetBrowserState.currentFolder.empty()) {
            // if at root (Asset folder), show only folders
            //if (asset.folder != m_AssetBrowserState.currentFolder) continue;
            continue;
        }
        else {
            // if file is directly inside Assets, put them in Misc
            if (m_AssetBrowserState.currentViewFolder == "Misc" && assetFolderName == "assets") assetFolderName = "Misc";

            // if in subfolder, show assets in that folder only
            if (assetFolderName != m_AssetBrowserState.currentViewFolder) continue;
        }

        std::string fullPath = asset.folder + "/" + asset.name;

        ImGui::PushID(index++);

        // highlight if the asset is the current scene
        if (asset.name == Editor::sceneState.currentSceneName) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.4f, 0.7f, 1.0f));
            /*ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.7f, 1.0f));*/
        }

        // display button and the name of the file
        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100, 40));

        // can consider changing to image in the future
        if(ImGui::Button(asset.name.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize))) {
            std::string extension;
            std::filesystem::path a = asset.name;      // or asset.path if you want full path
            std::string ext = a.extension().string();
            size_t dotPos = asset.name.find_last_of('.');
            if (dotPos != std::string::npos) extension = asset.name.substr(dotPos + 1);
            std::cout << asset.name << " button clicked!!!!!" << std::endl;
            if (extension == "png" || extension == "jpg" || extension == "jpeg")
            {
                TileMapSystem::filename = asset.path;
                /*std::cout << asset.name << std::endl;
                std::cout << asset.path << std::endl;*/
            }
        }

        ImGui::PopStyleVar();

        // pop style for the highlight current scene
        if (asset.name == Editor::sceneState.currentSceneName) {
            ImGui::PopStyleColor();
        }

        // RIGHT CLICK asset and show context menu
        if (ImGui::BeginPopupContextItem("AssetContextMenu")) {
            // show this asset in window explorer
            if (ImGui::MenuItem("Show in Explorer")) {
                // convert filepath to absolute to open in Window Explorer
                std::filesystem::path absPath = std::filesystem::absolute(fullPath);
                std::string absPathStr = absPath.string();

                std::replace(absPathStr.begin(), absPathStr.end(), '/', '\\');

                std::string command = "explorer /select,\"" + absPathStr + "\"";
                std::cout << "Running: " << command << std::endl;
                system(command.c_str());
            }

            // open popup window and ask whether to delete this current asset (delete not for prefab YET)
            if (assetFolderName != "Prefab") {
                if (ImGui::MenuItem("Delete Asset")) {
                    m_AssetBrowserState.fullPathToDelete = fullPath;
                    m_AssetBrowserState.showDeletePopup = true;
                }
            }

            // open popup window and rename this current asset
            if (ImGui::MenuItem("Rename Asset")) {
                m_AssetBrowserState.fullPathToRename = fullPath;
                m_AssetBrowserState.showRenamePopup = true;

                // pre-fill the current name
                std::strncpy(m_AssetBrowserState.newNameBuffer, asset.name.c_str(), sizeof(m_AssetBrowserState.newNameBuffer));
            }

            // open popup window and replace this current asset (not for prefab)
            if (assetFolderName != "Prefab" && assetFolderName != "Scene") {
                if (ImGui::MenuItem("Replace Asset")) {
                    m_AssetBrowserState.fullPathToReplace = fullPath;
                    m_AssetBrowserState.showReplacePopup = true;
                    m_AssetBrowserState.replacePathBuffer[0] = '\0';
                }
            }

            ImGui::EndPopup();
        }

        // DOUBLE click logic
        if (ImGui::IsItemHovered() && InputHandler::isMouseLeftDoubleClicked()) {
            // if its a scene file, load the scene
            if (assetFolderName == "Scene") {
                m_menuBarState.sceneToLoad = asset.name;
                m_menuBarState.showLoadScenePopup = true;
            }

            // if its a prefab, convert to temp game obj for editing in inspector
            if (assetFolderName == "Prefab") {
                // get ID of the double clicked prefab by its filename
                std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(asset.name);
                if (prefabID.empty()) {
                    DebugLog::addMessage("Cannot find prefab ID for " + asset.name + ".\n");
                    return;
                }

                // set the selected prefab to double clicked prefab, later will show in inspector
                std::unique_ptr<GameObject> tempPrefab = PrefabManager::Instance().createTempPrefabObj(prefabID);
                if (tempPrefab) {
                    m_objSelectionState.selectedObject = nullptr;
                    m_objSelectionState.selectedIndex = -1;
                    m_objSelectionState.selectedPrefab = std::move(tempPrefab);

                    DebugLog::addMessage("Open Prefab " + m_objSelectionState.selectedPrefab->getObjectName() + ".\n");
                }
                else {
                    DebugLog::addMessage("Failed to open prefab " + prefabID + " object for editing.\n");
                }

            }
        }

        // make sure this text wraps if it's too long
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_AssetBrowserState.iconSize);
        ImGui::Text("%s", asset.name.c_str());
        ImGui::PopTextWrapPos();

        ImGui::EndGroup();

        // calculate whether the next button should be displayed in the same line or next line based on this window size
        float lastButtonX2 = ImGui::GetItemRectMax().x;
        float nextButtonX2 = lastButtonX2 + m_AssetBrowserState.iconSize;
        if (nextButtonX2 < windowVisibleX2) ImGui::SameLine();


        /* ----------- Drag and Drop Logic ----------- */

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            // drag a texture to inspector/hierarchy window
            ImGui::SetDragDropPayload("ASSET", asset.path.c_str(), asset.path.size() + 1);
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        /* ---------- END ---------- */

        ImGui::PopID();
    }


    ImGui::End();

    /* -------- Asset pop up modals call -------- */
    assetDeletePopup();
    assetRenamePopup();
    assetReplacePopup();
    /* -------- END -------- */
}

void AssetBrowser::loadAssetsFromDirectory() {
    m_assets.clear(); // clear existing assets before loading new ones
    m_AssetBrowserState.subFolders.clear();

    for (const auto& entry : std::filesystem::directory_iterator(m_AssetBrowserState.currentFolder)) {
        if (entry.is_directory()) {
            std::string folderName = entry.path().filename().string();
            if (folderName == "resources" || folderName == "scripting" || folderName == "shaders") continue;
            m_AssetBrowserState.subFolders.push_back(folderName);
        }
    }

    // for all files directly in Assets
    m_AssetBrowserState.subFolders.push_back("Misc");

    // iterate over all files and folders in this directory and push all files to m_assets (flat list)
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(m_AssetBrowserState.currentFolder)) {
        if (entry.is_regular_file()) {
            Editor::Asset asset;
            std::string filename = entry.path().filename().string();
            std::string extension = entry.path().extension().string();

            // skip unwanted files
            if (filename == Editor::sceneState.tempSceneName) continue;
            if (filename == "prefab_registry.json") continue;
            if (extension == ".vert" || extension == ".frag" || extension == ".lua") continue;

            asset.name = std::move(filename);
            asset.path = entry.path().string();
            asset.folder = entry.path().parent_path().string(); // folder in assets that contains this file, for future filtering

            ///* ---- Sample for the asset struct ---- */
            // filename: control_scene.json
            // folder: assets/Scene ->asset+immediate parent folder
            // path: assets/Scene\control_scene.json
            ///* ---- END ---- */

            /*std::cout << "Asset Name: " << asset.name <<
                "\n Asset Path: " << asset.path <<
                "\n Asset Folder: " << asset.folder << std::endl;*/
            m_assets.push_back(asset);
        }
    }

    DebugLog::addMessage("Assets loaded from " + m_AssetBrowserState.currentFolder);
}

void AssetBrowser::assetDeletePopup() {
    // open popup if flagged
    if (m_AssetBrowserState.showDeletePopup) {
        ImGui::OpenPopup("Confirm Delete Asset");
        m_AssetBrowserState.showDeletePopup = false;
    }


    /*---------------- Delete Confirmation ----------------*/
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm Delete Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this asset?");

        if (ImGui::Button("Delete") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {

            if (!m_AssetBrowserState.fullPathToDelete.empty() &&
                std::filesystem::exists(m_AssetBrowserState.fullPathToDelete)) {
               /* std::cout << "\n\nfirst fullpath: " << m_AssetBrowserState.fullPathToDelete << "\n\n";*/
                std::filesystem::remove(m_AssetBrowserState.fullPathToDelete);
                EditorManager::assetChanged(); // to reload assets

                // delete from the project folder as well
                std::filesystem::path fullPath(m_AssetBrowserState.fullPathToDelete);
                std::string filename = fullPath.filename().string();

                // split the "assets/" part to get relative path
                std::string relativePath;
                size_t pos = m_AssetBrowserState.fullPathToDelete.find("assets/");
                if (pos != std::string::npos) {
                    relativePath = m_AssetBrowserState.fullPathToDelete.substr(pos + strlen("assets/"));
                }

                // combine relative path to delete
                std::filesystem::path projectPath = std::string(SOURCE_DIR_R) + "/" + relativePath;
                
                if (std::filesystem::exists(projectPath)) {
                    std::filesystem::remove(projectPath);
                    /*std::cout << "\n\nDeleted asset (Project): " << projectPath.string() << "\n\n";*/
                }

                /* -------- Prefab Delete (WIP) -------- */
                std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(relativePath);
                if (!prefabID.empty()) {
                    PrefabManager::Instance().deletePrefab(prefabID);
                }
                /* -------- END --------- */

                DebugLog::addMessage("Deleted asset: " + m_AssetBrowserState.fullPathToDelete + "\n");
            }

            m_AssetBrowserState.fullPathToDelete.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No")) {
            m_AssetBrowserState.fullPathToDelete.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    /*-------------------- END -------------------*/


    /*---------------- Cannot delete current loaded scene ----------------*/
    if (!m_AssetBrowserState.fullPathToDelete.empty() &&
        Editor::sceneState.currentSceneName == std::filesystem::path(m_AssetBrowserState.fullPathToDelete).filename().string())
    {
        ImGui::OpenPopup("Cannot Delete Current Scene");
        m_AssetBrowserState.fullPathToDelete.clear(); // clear so the check doesn't repeat
    }

    if (ImGui::BeginPopupModal("Cannot Delete Current Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Cannot delete the currently loaded scene!");

        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    /*-------------------- END -------------------*/
}

void AssetBrowser::assetRenamePopup() {
    // open popup if flagged
    if (m_AssetBrowserState.showRenamePopup) {
        ImGui::OpenPopup("Rename Asset");
        m_AssetBrowserState.showRenamePopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Rename Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new name for the asset");

        ImGui::InputText("##newName", m_AssetBrowserState.newNameBuffer, sizeof(m_AssetBrowserState.newNameBuffer));

        if (ImGui::Button("Rename") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            if (!m_AssetBrowserState.fullPathToRename.empty()) {
                std::filesystem::path old = m_AssetBrowserState.fullPathToRename;
                std::filesystem::path newPath = old.parent_path() / m_AssetBrowserState.newNameBuffer;
                // Debug/shaders/Prefab\\platform.json

                // keep the extension if not provided
                if (newPath.extension().empty())
                    newPath.replace_extension(old.extension());

                if (!std::filesystem::exists(newPath)) {
                    // if renamed asset is current scene, rename current scene name as well
                    if (Editor::sceneState.currentSceneName == old.filename().string()) {
                        Editor::sceneState.currentSceneName = newPath.filename().string();
                    }

                    std::filesystem::rename(old, newPath);
                    EditorManager::assetChanged(); // reload assets to reflect new name

                    // change in the project folder as well
                    std::filesystem::path sourceDir(SOURCE_DIR_R);

                    // split the "assets/" part to get relative path
                    std::string relative;
                    size_t pos = old.string().find("assets/");
                    if (pos != std::string::npos) {
                        relative = old.string().substr(pos + strlen("assets/"));
                    }
                    else {
                        relative = old.filename().string();
                    }

                    // combine relative path to rename
                    std::filesystem::path projectOld = sourceDir / relative;
                    std::filesystem::path projectNew = projectOld.parent_path() / newPath.filename();
                    if (std::filesystem::exists(projectOld)) {
                        std::filesystem::rename(projectOld, projectNew);
                    }


                    /* -------- Changing Prefab Name + Registry Entry -------- */

                    // get prefabid by the path
                    std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(old);
                    if (!prefabID.empty()) {
                        // change prefab name and save
                        PrefabManager::Instance().setPrefabPath(prefabID, newPath.string());
                    }

                    /* ------- END ------- */

                    DebugLog::addMessage("Renamed asset: " + old.string() + " -> " + newPath.string() + "\n");
                }
                else {
                    DebugLog::addMessage("Failed to rename: file already exists.\n");
                }
            }

            m_AssetBrowserState.fullPathToRename.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            m_AssetBrowserState.fullPathToRename.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void AssetBrowser::assetReplacePopup() {
    if (m_AssetBrowserState.showReplacePopup) {
        ImGui::OpenPopup("Replace Asset");
        m_AssetBrowserState.showReplacePopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Replace Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select a new file to replace the asset:\n");

        ImGui::InputText("File Path", m_AssetBrowserState.replacePathBuffer, sizeof(m_AssetBrowserState.replacePathBuffer));

        ImGui::Text("Please enter the absolute path without \"\"");

        if (ImGui::Button("Replace") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            std::filesystem::path old = m_AssetBrowserState.fullPathToReplace;
            std::filesystem::path newPath = m_AssetBrowserState.replacePathBuffer;

            if (std::filesystem::exists(newPath)) {
                std::filesystem::copy_file(newPath, old, std::filesystem::copy_options::overwrite_existing);
                EditorManager::assetChanged();
                DebugLog::addMessage("Replaced asset: " + old.string() + " with " + newPath.string() + "\n");
            }
            else {
                DebugLog::addMessage("Replacement file does not exist.\n");
            }

            m_AssetBrowserState.fullPathToReplace.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_AssetBrowserState.fullPathToReplace.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
#endif