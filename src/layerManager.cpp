/* Start Header ************************************************************************/
/*!
\file       layerManager.cpp
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

#include"GameObject.h"
#include "layerManager.h"

LayerManager::LayerManager() {
	//for now got 3 layers by default
	//bg, game, UI
	createLayer("Background", 0);
	createLayer("Game", 1);
	createLayer("UI", 2);
	//can create more during run time if needed
}

Layer* LayerManager::createLayer(const std::string& name, int layerID) {
	//check if layerID already exists
	if(m_layers.find(layerID) != m_layers.end()) {
		std::cerr << "Layer Manager Error: Layer ID " << layerID << " already exists. Cannot create layer '" << name << "'.\n";
		return nullptr;
	}

	//create new layer and insert into map
	auto newLayer = std::make_unique<Layer>(name, layerID);
	Layer* layerPtr = newLayer.get();
	m_layers[layerID] = std::move(newLayer);
	m_layerNameToID[name] = layerID;

	return layerPtr;
}

Layer* LayerManager::getLayer(int layerID) {
	//find by ID
	auto it = m_layers.find(layerID);
	if(it != m_layers.end()) {
		return it->second.get();
	}

	std::cerr << "Layer Manager Warning: Layer ID " << layerID << " not found.\n";
	return nullptr;
}

Layer* LayerManager::getLayerByName(const std::string& name) {
	//find layerID by name
	auto it = m_layerNameToID.find(name);
	if(it != m_layerNameToID.end()) {
		int layerID = it->second;
		return getLayer(layerID);
	}
	std::cerr << "Layer Manager Warning: Layer name '" << name << "' not found.\n";
	return nullptr;
}

bool LayerManager::assignObjectToLayer(GameObject* obj, int layerID) {
	if(!obj) {
		std::cerr << "Layer Manager Error: Attempted to assign a null GameObject to layer " << layerID << ".\n";
		return false;
	}

	Layer* layer = getLayer(layerID);
	if(!layer) {
		std::cerr << "Layer Manager Error: Attempted to assign GameObject to non-existent layer " << layerID << ".\n";
		return false;
	}

	//remove object from its current layer first
	//add to new layer
	removeObjectFromLayer(obj);
	layer->addGameObject(obj);

	return true;
}

void LayerManager::removeObjectFromLayer(GameObject* obj) {
	if (!obj) {
		std::cerr << "Layer Manager Error: Attempted to remove a null GameObject from its layer.\n";
		return;
	}

	//remove if found in any layer
	for(auto& layer : m_layers) {
		layer.second->removeGameObject(obj);
	}
}

int LayerManager::getObjectLayer(GameObject* obj) const {
	if(!obj) {
		std::cerr << "Layer Manager Error: Attempted to get layer of a null GameObject.\n";
		return -1; //invalid layer
	}

	//search through all layers to find the object
	for(const auto& layer : m_layers) {
		//if have the object
		if(layer.second->hasObject(obj)) {
			return layer.first; //return layerID
		}
	}

	//otherwise not found
	return -1;
}

std::vector<Layer*> LayerManager::getAllLayers() const {
	std::vector<Layer*> layers;
	layers.reserve(m_layers.size());

	for(const auto& layer : m_layers) {
		layers.push_back(layer.second.get());
	}

	return layers;
}

void LayerManager::clearAllLayers() {
	for(auto& layer : m_layers) {
		layer.second->clear();
	}
}