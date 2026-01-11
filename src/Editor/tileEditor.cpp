/* Start Header ************************************************************************/
/*!
\file       tileEditor.cpp
\author     Seow Sin Le, s.sinle, 2401084
			- 100%
\par		cheeqing.tan@digipen.edu
\date       November, 26th, 2025
\brief      Definitions of the functions for loading textures onto a tilemap

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Systems.h"
std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> RenderSystem::objectWithTex2{};

void TileMapSystem::tileUpdate(GameObject* obj)
{
#ifdef _DEBUG
	TileMap* tm = obj->getComponent<TileMap>();
	Transform* transform = obj->getComponent<Transform>();
	Vector2D world = InputHandler::getMousePositionInImGuiViewport(Editor::sceneWindowState.scenePos, Editor::sceneWindowState.sceneSize);
	int col = (int)floor((world.x - transform->x) / tm->tileW);
	int row = (int)floor((world.y - transform->y) / tm->tileH);

	if(SceneWindow::isSceneHovered())
		if (!TileMapSystem::filename.empty())
			if (col >= -tm->columns && col < tm->columns &&
				row >= -tm->rows && row < tm->rows)
			{
				// apply selected tile
				if (tm->getTile(col, row) != filename)
					tm->setTile(col, row, filename);
				else
					tm->clearTile(col, row);

			}
#endif
}

void TileMapSystem::update(GameObjectManager& manager)
{
	std::vector<GameObject*> objects;
	manager.getAllGameObjects(objects);
	RenderSystem::objectWithTex2.clear();
	
	
	for (GameObject* obj : objects)
	{
		if (!obj->hasComponent<TileMap>() || !obj->hasComponent<Transform>())
			continue;


		if (InputHandler::isMouseLeftClicked())
		{
			//Vector2D world = InputHandler::getMousePositionInImGuiViewport(Editor::sceneWindowState.scenePos, Editor::sceneWindowState.sceneSize);
			//std::cout << world.x << " | " << world.y << std::endl;
			//DebugLog::addMessage("mouse pos: " + std::to_string(world.x) + " | " + std::to_string(world.y) + "\n");
			tileUpdate(obj);
		}

		TileMap* tm = obj->getComponent<TileMap>();
		Transform* transform = obj->getComponent<Transform>(); //assume that all objects has transform

		for (const auto& [tileKey, tileID] : tm->tiles)
		{
			renderer::InstanceData data;
			glm::vec3 pos{ transform->x + tileKey.x * tm->tileW + tm->tileW * 0.5f, transform->y + tileKey.y * tm->tileH + tm->tileH * 0.5f, transform->z };
			glm::mat4 mdl{ 1 };
			mdl = glm::translate(mdl, pos);
			mdl = glm::scale(mdl, glm::vec3(tm->tileW, tm->tileH, 1.f));
			data.model = mdl;
			data.color = glm::vec4(0, 0, 0, 1);
			data.texParams = glm::vec4(0, 0, 1, 1); //no slicing
			TextureData texID = ResourceManager::getInstance().getTexture(tileID);
			RenderSystem::objectWithTex2[BatchKey{ shape::square, texID.id }].push_back(data);
		}
	}
}