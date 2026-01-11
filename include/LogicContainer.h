/* Start Header ************************************************************************/
/*!
\file		LogicContainer.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Declares the LogicContainer system responsible for managing simple
            per-object finite state machines. This includes handling transitions
            between Idle, Walking, Jumping, and Falling states based on input,
            physics state, and gameplay events. The LogicContainer also supports
            player actions such as spawning bullets.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

class GameObject;
class GameObjectManager;

#include "Component.h"
#include "input.h"

/*!
 * \class LogicContainer
 * \brief A lightweight gameplay logic system for handling finite-state
 *        behaviors of game objects.
 *
 * This class bridges the Input system, Physics, Transform, and StateMachine
 * components. It evaluates per-object state transitions (Idle, Walking,
 * Jumping, Falling) and applies context-specific behavior each frame.
 *
 * The LogicContainer also handles special actions such as firing bullets.
 */
class LogicContainer {
public:

    /*!
     * \brief Main per-frame state update function for a GameObject.
     *
     * Evaluates input, updates the object�s state machine, performs state
     * transitions, and triggers gameplay actions such as jumping or firing.
     *
     * \param obj      Reference to the GameObject whose logic is being updated.
     * \param dt       Delta time for the current frame.
     * \param manager  Reference to the GameObjectManager used for spawning
     *                 or retrieving other objects (e.g., bullets).
     */
    void update(GameObject& obj, float dt /*, GameObjectManager& manager */);

private:

    /*!
     * \brief Performs a state transition on a GameObject�s StateMachine.
     *
     * Resets the state timer, updates the current state, and logs the transition
     * to both console and UI for debugging.
     *
     * \param obj   The object to transition.
     * \param next  The PlayerState to move into.
     */
    static void enter(GameObject& obj, PlayerState next);

    /*!
     * \brief Behavior function executed when the object is in Idle state.
     *
     * Handles transitions to Walking when movement keys are pressed,
     * and Jumping when the player triggers a jump.
     *
     * \param obj The object in Idle state.
     * \param dt  Time elapsed for the frame.
     */
    static void tickIdle(GameObject& obj, float dt);

    /*!
     * \brief Behavior function executed when the object is in Walking state.
     *
     * Handles movement-based transitions, orientation updates, jumping,
     * and detecting transitions into Falling when the object leaves the ground.
     *
     * \param obj The object in Walking state.
     * \param dt  Time elapsed for the frame.
     */
    static void tickWalking(GameObject& obj, float dt);

    /*!
     * \brief Behavior function executed when the object is in Jumping state.
     *
     * Monitors vertical velocity and transitions to Falling once the upward
     * movement ends.
     *
     * \param obj The object in Jumping state.
     * \param dt  Time elapsed for the frame.
     */
    static void tickJumping(GameObject& obj, float dt);

    /*!
     * \brief Behavior function executed when the object is in Falling state.
     *
     * Detects landing and transitions into either Walking or Idle depending
     * on which movement keys are currently held.
     *
     * \param obj The object in Falling state.
     * \param dt  Time elapsed for the frame.
     */
    static void tickFalling(GameObject& obj, float dt);

    /*!
     * \brief Spawns a bullet near the object and initializes its velocity.
     *
     * Uses the object�s facing direction and transform to position the bullet.
     * Only works if a GameObject named "bullet" exists in the manager.
     *
     * \param obj      The object performing the action (typically the player).
     * \param manager  The GameObjectManager used to retrieve the bullet object.
     */
    //static void trySpawnBulletNear(GameObject& obj, GameObjectManager& manager);\

	static void tickDead(GameObject& obj, float dt);
};

