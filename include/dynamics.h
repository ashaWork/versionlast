/* Start Header ************************************************************************/
/*!
\file       dynamics.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       November, 7th, 2025
\brief      Defines the Force and Dynamics structures along with the
            DynamicsSystem class responsible for handling physics-based
            motion and force integration in the game.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "mathlib.h"
#include <vector>
/**
 * @struct Force
 * @brief  physical force applied to an object
 *
 * the structure defines a force with direction, magnitude, lifetime,
 * and activation state to use in physics simulations
 */
struct Force
{
    Vector3D direction; // normalized
    float magnitude;
    float lifetime;     // in seconds (-1 for infinite)
    bool active = true;
};
/**
 * @struct Dynamics
 * @brief Contains all dynamic properties for a physical object
 *
 * structure holds linear and angular motion properties, 
 * mass information,
 * and accumulated forces for physics simulation
 */
struct Dynamics
{
    // Linear motion properties
    Vector3D position{ 0.0f, 0.0f, 0.0f };
    Vector3D velocity{ 0.0f, 0.0f, 0.0f };
    Vector3D acceleration{ 0.0f, 0.0f, 0.0f };

    // Angular motion properties
    Vector3D angularVelocity{ 0.0f, 0.0f, 0.0f };
    Vector3D angularAcceleration{ 0.0f, 0.0f, 0.0f };

    // Mass properties
    float mass = 1.0f;
    float inverseMass = 1.0f;

    std::vector<Force> forces;
};

/**
 * @class DynamicsSystem
 * @brief Handles physics simulation for dynamic objects
 *
 * This class provides static methods managing forces and integrating
 * physics equations to simulate object motion
 */
class DynamicsSystem
{
public:
    /**
     * @brief Applies a new force to a dynamics object
     */
    static void AddForce(Dynamics& dyn, const Vector3D& direction, float magnitude, float lifetime = -1.0f);
    /**
     * @brief Updates all forces
     */
    static void UpdateForces(Dynamics& dyn, float deltaTime);
    /**
     * @brief update position and velocity of objects
     */
    static void Integrate(Dynamics& dyn, float deltaTime, float gravityValue, bool onGround);
};
