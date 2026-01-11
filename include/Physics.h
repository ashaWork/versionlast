/* Start Header ************************************************************************/
/*!
\file       Physics.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       November, 7th, 2025
\brief      defines the PhysicsForces struct which provides static methods
            for handling game physics including gravity application,
            movement damping, jumping, shooting, and bullet object pooling.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include "gameObject.h"
#include "dynamics.h"
#include "GameObjectManager.h"

class GameObjectManager;

/**
 * @struct PhysicsForces
 * @brief Handles  physics operations for game objects
 *
 * Provides static methods for applying gravity, damping, updating positions,
 * and handling game-specific physics like jumping and shooting.
 */
struct PhysicsForces {
    /**
    * @brief Applies gravity force to a physics object
    */
    static void applyGravity(Physics* physics);
    /**
     * @brief Applies velocity damping to reduce movement over time
     */
    static void applyDamping(Physics* physics);
    /**
     * @brief Updates transform position based on physics simulation
     */
    static void updatePosition(Transform* transform, Physics* physics, float deltaTime);
    /**
     * @brief Applies jump force to object
     */
    static void jump(GameObject* object);           // Apply jump force
    /**
     * @brief shoots a bullet from  origin (player)
     */
    static void shoot(GameObject* bullet, const Transform* origin);  // Shoot bullet from origin

    /*static void applyDynamics(Physics* physics, float deltaTime);*/
    /**
     * @brief Finds an available/free bullet from the object pool
     */
    static GameObject* findAvailableBullet(GameObjectManager& manager);
    /**
     * @brief Deactivates a bullet and returns it to the object pool
     */
    static void deactivateBullet(GameObject* bullet);
};