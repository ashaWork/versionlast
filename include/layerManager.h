/* Start Header ************************************************************************/
/*!
\file       layerManager.h
\author     Hugo Low Ren Hao, low.h, 2402272

\par        low.h@digipen.edu
\date       November, 1, 2025
\brief      This file contains the declaration of the LayerManager class.
			It manages multiple layers and assigns GameObjects to them.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once

#include <memory>
#include <map>

#include "layer.h"

class GameObject;
class GameObjectManager;

class LayerManager {
public:
	//default constructor
	LayerManager();

	//create a new layer
	Layer* createLayer(const std::string& name, int layerID);

	//get layer by id
	Layer* getLayer(int layerID);

	//get layer by name
	Layer* getLayerByName(const std::string& name);

	//assign object to layer
	bool assignObjectToLayer(GameObject* obj, int layerID);

	//remove object from layer
	void removeObjectFromLayer(GameObject* obj);

	//get object's layer
	int getObjectLayer(GameObject* obj) const;

	//get all layers
	std::vector<Layer*> getAllLayers() const;

	//clear all objects from all layers
	void clearAllLayers();


private:
	//map of layerID to Layer object
	std::map<int, std::unique_ptr<Layer>> m_layers;

	//map of layer name to layerID for lookup
	std::map<std::string, int> m_layerNameToID;
};