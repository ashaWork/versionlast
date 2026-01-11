/* Start Header ************************************************************************/
/*!
\file		GameObject.cpp
\author     Hugo Low Ren Hao, low.h, 2402272
\par        low.h@digipen.edu
\date		October, 1st, 2025
\brief      This file contains the GameObject class. A GameObject is an entity in the
            game world that can have multiple components attached to it.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "GameObject.h"

//constructor
GameObject::GameObject(const std::string& name, const std::string& prefabID) : m_name(name), m_prefabID(prefabID) { autoMove = false;  }

const std::string& GameObject::getObjectName() const {
    return m_name;
}

std::string& GameObject::getObjectName() {
    return m_name;
}

const std::string& GameObject::getObjectPrefabID() const {
    return m_prefabID;
}

std::string& GameObject::getObjectPrefabID() {
    return m_prefabID;
}

std::unique_ptr<GameObject> GameObject::clone(const std::string& name) const {

	std::unique_ptr<GameObject> newObject = std::make_unique<GameObject>(name);

    newObject->m_prefabID = this->m_prefabID;
	newObject->setLayer(this->m_layerID);

    // Iterate through all components of the source object.
	for (const auto& pair : m_components) { // Pair is [type_index, unique_ptr<Component>]
        const std::unique_ptr<Component>& sourceComponent = pair.second;

        if (sourceComponent) {
            // Ask each component to clone itself.
            std::unique_ptr<Component> newComponent = sourceComponent->clone();

            // Add the newly created component copy to the new GameObject.
            // We use the same type_index key from the original.
            newObject->m_components[pair.first] = std::move(newComponent);
        }
    }

    return newObject;
}

// Layering functions
void GameObject::setLayer(int layerID) {
    m_layerID = layerID;
}

const int& GameObject::getLayer() const {
    return m_layerID;
}

bool GameObject::isOnLayer(int layerID) const {
    return m_layerID == layerID;
}

//remove component of type T

//use like obj->removeComponent<Transform>();