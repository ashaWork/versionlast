/* Start Header ************************************************************************/
/*!
\file       addObjWindow.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 90%
            Pearly Lin Lee Ying, p.lin, 2401591
            - 10% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of rendering the window and the adding of different objects in
            the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Editor/editorManager.h"

AddObjWindow::AddObjWindow(Editor::ShapeControlState& Cshape, Editor::ShapeControlState& Rshape) :
    m_circleInput(Cshape),
    m_rectangleInput(Rshape) {}

void AddObjWindow::render(GameObjectManager& manager){
    ImGui::Begin("Create Object");

    //ImGui::PushItemWidth(150.0f);

    /* --------------- Add Rectangle Logic --------------- */
    {
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::InputFloat("##X1", &m_rectangleInput.inputX, 1.f);

        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##Y1", &m_rectangleInput.inputY, 1.f);

        ImGui::Text("Z");
        ImGui::SameLine();
        ImGui::InputFloat("##Z1", &m_rectangleInput.inputZ, 1.f);
        ImGui::Spacing();


        if (ImGui::Button("Add Rectangle")) {
            createRec(manager, m_rectangleInput.inputX, m_rectangleInput.inputY, m_rectangleInput.inputZ);
        }
    }
    /* --------------- END --------------- */

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    /* --------------- Add Circle Logic --------------- */
    {
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::InputFloat("##X", &m_circleInput.inputX, 1.f);

        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##Y", &m_circleInput.inputY, 1.f);

        ImGui::Text("Z");
        ImGui::SameLine();
        ImGui::InputFloat("##Z", &m_circleInput.inputZ, 1.f);
        ImGui::Spacing();

        //ImGui::PopItemWidth();
        if (ImGui::Button("Add Circle")) {
            createCir(manager, m_circleInput.inputX, m_circleInput.inputY, m_circleInput.inputZ);
        }

        /* --------------- END --------------- */
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    //// spawning objects (double up every click)
    static int count = 0;
    if (ImGui::Button("Stress Test (Do not click)")) {
        int spawnCount = (count == 0) ? 1 : count * 2;
        if (spawnCount == 2048)
            spawnCount = 2500; //cap at 400 objects

        // if player2 not exist then create player2
        if (!manager.getGameObject("player2")) {
            GameObject* player2 = manager.createGameObject("player2");

            player2->addComponent<Transform>();
            player2->addComponent<Physics>();
            Render* render = player2->addComponent<Render>();

            render->modelRef = renderer::models[static_cast<int>(shape::square)];
            render->hasTex = true;
            render->texFile = "./assets/Sample_tilemap.jpg";
            render->texChanged = true;
        }

        for (int i = count; i < spawnCount; ++i) {
            std::string cloneName = "Player_Clone(" + std::to_string(i + 1) + ")";
            GameObject* newObj = manager.CloneGameObject("player2", cloneName);
            newObj->changeAutoMove(true); // set to true so it auto move

            Transform* transform = newObj->getComponent<Transform>();
            Physics* physics = newObj->getComponent<Physics>();
            if (!physics) {
                physics = newObj->addComponent<Physics>();
            }

            float randomX = (rand() % 2000) / 100.0f;
            float randomY = (rand() % 1000) / 100.0f;     // 0.0 to 9.99
            float rotation = (rand() % 36000) / 100.0f;   // 0.0 to 359.99
            float scaleX = ((rand() % 300) / 100.0f) + 1; // 1.00 to 3.00
            float scaleY = ((rand() % 300) / 100.0f) + 1;
            float velX = ((rand() % 300) / 100.0f) - 1;   // -1.00 to 2.00
            float velY = ((rand() % 300) / 100.0f) - 1;

            // avoid zero velocity
            if (velX == 0.0f) velX = 0.5f;
            if (velY == 0.0f) velY = 0.5f;

            // make sure 
            if (rand() % 2 == 0)
                randomX = -randomX;
            if (rand() % 2 == 1)
                randomY = -randomY;

            transform->x = randomX;
            transform->y = randomY;
            transform->rotation = rotation;
            transform->scaleX = scaleX;
            transform->scaleY = scaleY;

            physics->gravity = 0.0;
            physics->dynamics.velocity.x = velX;
            physics->dynamics.velocity.y = velY;
        }

        count = spawnCount;
    }

    ImGui::End();
}

void AddObjWindow::createEmpty(GameObjectManager& manager, float x, float y, float z) {
    std::string name = "Empty_" + std::to_string(manager.getGameObjectCount());
    GameObject* obj = manager.createGameObject(name);

    Transform* transform = obj->addComponent<Transform>();
    if (!transform) {
        DebugLog::addMessage("Error creating a game object. Component doesn't exist.");
        return;
    }

    transform->x = x;
    transform->y = y;
    transform->z = z;

    // record the create command (for undo)
    auto cmd = std::make_unique<CreateObjectCmd>(manager, name);
    UndoRedoManager::Instance().executeCmd(std::move(cmd));

    DebugLog::addMessage("Empty created at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")\n");
}

void AddObjWindow::createRec(GameObjectManager& manager, float x, float y, float z) {
    std::string name = "Rectangle_" + std::to_string(manager.getGameObjectCount());
    GameObject* recobj = manager.createGameObject(name);

    Transform* transform = recobj->addComponent<Transform>();
    Render* render = recobj->addComponent<Render>();
    if (!transform || !render) {
        DebugLog::addMessage("Error creating a game object. Component doesn't exist.");
        return;
    }
    
    render->modelRef = renderer::models[static_cast<int>(shape::square)];

    transform->x = x;
    transform->y = y;
    transform->z = z;

    if (!recobj) {
        DebugLog::addMessage("Error creating game object.");
        return;
    }

    // record the create command (for undo)
    auto cmd = std::make_unique<CreateObjectCmd>(manager, name);
    UndoRedoManager::Instance().executeCmd(std::move(cmd));

    DebugLog::addMessage("Rectangle created at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")\n");
}


void AddObjWindow::createCir(GameObjectManager& manager, float x, float y, float z) {
    std::string name = "Circle_" + std::to_string(manager.getGameObjectCount());
    GameObject* circleObj = manager.createGameObject(name);

    Transform* transform = circleObj->addComponent<Transform>();
    Render* render = circleObj->addComponent<Render>();
    if (!transform || !render) {
        DebugLog::addMessage("Error creating a game object. Component doesn't exist.");
        return;
    }

    render->modelRef = renderer::models[static_cast<int>(shape::circle)];

    transform->x = x;
    transform->y = y;
    transform->z = z;

    // record the create command (for undo)
    auto cmd = std::make_unique<CreateObjectCmd>(manager, name);
    UndoRedoManager::Instance().executeCmd(std::move(cmd));

    DebugLog::addMessage("Circle created at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")\n");
}

void AddObjWindow::dupObj(GameObjectManager& manager, GameObject* oriObj) {
    std::string name = oriObj->getObjectName() + std::to_string(manager.getGameObjectCount());
    GameObject* dupObj = manager.CloneGameObject(oriObj->getObjectName(), name);

    if (Transform* oriT = oriObj->getComponent<Transform>()) {
        Transform* dupT = dupObj->getComponent<Transform>();

        // add offset from original object
        dupT->x = oriT->x + 1.f;
        dupT->y = oriT->y + 1.f;
    }

    // record the create command (for undo)
    auto cmd = std::make_unique<CreateObjectCmd>(manager, name);
    UndoRedoManager::Instance().executeCmd(std::move(cmd));

    Editor::objSelectionState.selectedObject = dupObj;

    DebugLog::addMessage("Object duplicated!");
}
#endif