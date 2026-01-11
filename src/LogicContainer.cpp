/* Start Header ************************************************************************/
/*!
\file		LogicContainer.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief		Implements the LogicContainer class. Handles player logic transitions
            such as Idle, Walking, and Jumping based on input and state.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "LogicContainer.h"
#include "GameObject.h"
#include "input.h"
#include "Systems.h"
#include "Component.h"
#include "GameObjectManager.h"

// ============================================================================
// Helper: convert PlayerState enum to readable string
// ============================================================================
/*!
 * \brief Converts a PlayerState enum into a human-readable C-string.
 * \param s The PlayerState to convert.
 * \return A string literal representing the state.
 */
static const char* ToString(PlayerState s)
{
    switch (s)
    {
    case PlayerState::Idle:    return "Idle";
    case PlayerState::Walking: return "Walking";
    case PlayerState::Jumping: return "Jumping";
    case PlayerState::Falling: return "Falling";
    case PlayerState::Shooting:return "Shooting";
    default:                   return "Unknown";
    }
}

/*!
 * \brief Checks if any movement key (W/A/S/D) is currently held.
 * \return True if one or more movement keys are held.
 */
static inline bool anyMoveHeld() {
    return InputHandler::isKeyHeld(GLFW_KEY_W) ||
        InputHandler::isKeyHeld(GLFW_KEY_A) ||
        InputHandler::isKeyHeld(GLFW_KEY_S) ||
        InputHandler::isKeyHeld(GLFW_KEY_D);
}

// ============================================================================
// FSM Transition Helper
// ============================================================================
/*!
 * \brief Applies a state transition to the object�s StateMachine component.
 *
 * Resets state timer, logs the transition to ImGui & console,
 * and performs any enter-state side effects.
 *
 * \param obj Reference to the GameObject owning the FSM.
 * \param next The next PlayerState to enter.
 */
void LogicContainer::enter(GameObject& obj, PlayerState next)
{
    if (auto* fsm = obj.getComponent<StateMachine>())
    {
        if (fsm->state != next)
        {
            // reset animation frame
            if (Animation* a = obj.getComponent<Animation>()) {

                //a->animState[static_cast<int>(fsm->state)].currentFrameColumn = a->animState[static_cast<int>(fsm->state)].initialFrame.x;
                //a->animState[static_cast<int>(fsm->state)].currentFrameRow = a->animState[static_cast<int>(fsm->state)].initialFrame.y;
                int stateIndex = static_cast<int>(fsm->state);

                a->animState[stateIndex].currentFrameColumn =
                    static_cast<int>(a->animState[stateIndex].initialFrame.x);

                a->animState[stateIndex].currentFrameRow =
                    static_cast<int>(a->animState[stateIndex].initialFrame.y);
            }
            std::string msg = "[LogicContainer] "
                + obj.getObjectName() + " -> " + ToString(next);
            DebugLog::addMessage(msg, DebugMode::PlaySimul);
            std::cout << msg << std::endl;


            //get or add AudioComponent
            AudioComponent* audio = obj.getComponent<AudioComponent>();
            if (!audio) {
                audio = obj.addComponent<AudioComponent>();
            }

            if (!audio) {
                std::cerr << "[LogicContainer] Failed to get/create AudioComponent for "
                    << obj.getObjectName() << std::endl;
                fsm->state = next;
                fsm->stateTime = 0.0f;
                return;  // Early return if audio component is invalid
            }

            //if prev state was walking
            if (fsm->state == PlayerState::Walking) {

                //stop footsteps when leaving walking state

                AudioChannel* footsteps = audio->getChannel("footsteps");
                if (footsteps && footsteps->state == AudioState::Playing) {
                    footsteps->isPendingStop = true;
                    footsteps->fadeOutOnStop = true;
                    footsteps->fadeOutDuration = 0.15f;
                }
            }

            //if prev state was falling, means exiting falling state(just landed prolly)
            if (fsm->state == PlayerState::Falling) {
                AudioChannel* landing = audio->getOrCreateChannel("landing");
                
                //play sound when exiting falling state
                landing->audioFile = "assets/audio/landing.wav";
                landing->loop = false;
                landing->volume = 1.0f;
                landing->isPendingPlay = true;
            }

            //play a sound depending on next state
            switch (next) {
            case PlayerState::Walking : {//jump jump
                AudioChannel* footsteps = audio->getOrCreateChannel("footsteps");
                footsteps->audioFile = "assets/audio/footsteps.wav";
                footsteps->loop = true;
                footsteps->volume = 0.5f;
                footsteps->isPendingPlay = true;
                break;
            }
            case PlayerState::Jumping: {
                AudioChannel* jump = audio->getOrCreateChannel("jump");
                jump->audioFile = "assets/audio/jump.wav";
                jump->loop = false;
                jump->volume = 0.7f;
                jump->isPendingPlay = true;
                break;
            }
            case PlayerState::Falling: {
                //play falling sound
                break;
            }
            case PlayerState::Idle: {
                //play some idle sound if needed
                break;
            }
            default:
                //prolly nothing happens
                break;   
            }


        }

        fsm->state = next;
        fsm->stateTime = 0.0f;
    }
}

// ============================================================================
// State: Idle
// ============================================================================
/*!
 * \brief Per-frame update for Idle state.
 *
 * Handles:
 * - Jumping (B key)
 * - Transition to Walking when movement keys are held
 *
 * \param obj Player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickIdle(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    // jump
    if (InputHandler::isKeyTriggered(GLFW_KEY_B) && physics->onGround) {
        physics->velY = physics->jumpForce;
        physics->onGround = false;
        enter(obj, PlayerState::Jumping);
        DebugLog::addMessage("FSM: Jump", DebugMode::PlaySimul);
    }
    // walk
    else if (anyMoveHeld() && obj.getObjectPrefabID() != "eaa2ba42-971a-413b-b8c4-99b2f5ab674d" && obj.getObjectPrefabID() != "f04819ff-270c-41b4-8387-73382eb85103") {
        enter(obj, PlayerState::Walking);
    }
}

// ============================================================================
// State: Walking
// ============================================================================
/*!
 * \brief Per-frame update for Walking state.
 *
 * Handles:
 * - Facing direction based on A/D keys
 * - Returning to Idle when no movement keys are held
 * - Transition to Falling when leaving ground
 * - Jumping (B key)
 *
 * \param obj The player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickWalking(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    auto* fsm = obj.getComponent<StateMachine>();
    if (!physics || !fsm) return;

    if (InputHandler::isKeyHeld(GLFW_KEY_A)) fsm->facingRight = false;
    if (InputHandler::isKeyHeld(GLFW_KEY_D)) fsm->facingRight = true;

    // if its water source, play the sound once
    if (obj.getObjectPrefabID() == "f04819ff-270c-41b4-8387-73382eb85103" && fsm->state == PlayerState::Walking) {
        AudioComponent* audio = obj.getComponent<AudioComponent>();
        if (audio) {
            AudioChannel* getWater = audio->getDefaultChannel();

            if (getWater && getWater->state != AudioState::Playing && !getWater->isPendingPlay) {
                getWater->loop = false;
                getWater->volume = 0.5f;
                getWater->isPendingPlay = true;

				fsm->state = PlayerState::Falling;
            }
        }
    }
        
    if (!anyMoveHeld() && !(fsm->fixState)) {
        enter(obj, PlayerState::Idle);
        return;
    }
    if (!physics->onGround && physics->velY < 0.0f) {
        enter(obj, PlayerState::Falling);
        return;
    }
    if (InputHandler::isKeyTriggered(GLFW_KEY_B) && physics->onGround) {
        physics->velY = physics->jumpForce;
        physics->onGround = false;
        enter(obj, PlayerState::Jumping);
        DebugLog::addMessage("FSM: Jump", DebugMode::PlaySimul);
        return;
    }
}

// ============================================================================
// State: Jumping
// ============================================================================
/*!
 * \brief Per-frame update for Jumping state.
 *
 * Switches to Falling once vertical velocity becomes negative.
 *
 * \param obj The player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickJumping(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    if (physics->velY <= 0.0f) {
        enter(obj, PlayerState::Falling);
    }
}

// ============================================================================
// State: Falling
// ============================================================================
/*!
 * \brief Per-frame update for Falling state.
 *
 * When the player lands:
 * - enters Walking if movement keys held
 * - otherwise enters Idle
 *
 * \param obj The player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickFalling(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    if (physics->onGround) {
        if (anyMoveHeld()) enter(obj, PlayerState::Walking);
        else               enter(obj, PlayerState::Idle);
    }
}

// ============================================================================
// State: Dead
// ============================================================================
/*!
 * \brief Per-frame update for Dead state.
 *
 * Disables movement and handles respawn logic after a delay.
 *
 * \param obj The player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickDead(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    auto* fsm = obj.getComponent<StateMachine>();
    if (!fsm) return;
    // 1) Disable player movement
    if (physics) {
        physics->velX = 0.0f;
        
        physics->velY = 0.0f;
		physics->onGround = true;

    }
    // 2) After some delay, respawn or reload level
    //const float respawnDelay = 1.5f; // seconds
    //if (fsm->stateTime > respawnDelay) {
    //    // respawn logic here
    //    
    //}
}
// ============================================================================
// Shooting Logic
// ============================================================================
/*!
 * \brief Spawns a bullet near the player and gives it initial velocity.
 *
 * Uses the player's Transform and facing direction to position the bullet.
 *
 * \param obj The player.
 * \param manager Reference to GameObjectManager for accessing existing bullet object.
 */
//void LogicContainer::trySpawnBulletNear(GameObject& obj, GameObjectManager& manager) {
//    auto* playerT = obj.getComponent<Transform>();
//    auto* fsm = obj.getComponent<StateMachine>();
//    if (!playerT || !fsm) return;
//
//    GameObject* bullet = manager.getGameObject("bullet");
//    if (!bullet) return;
//
//    Transform* t = bullet->getComponent<Transform>();
//    Physics* p = bullet->getComponent<Physics>();
//    if (!t || !p) return;
//
//    const bool right = fsm->facingRight;
//    t->x = playerT->x + (right ? 0.8f : -0.8f);
//    t->y = playerT->y + 0.3f;
//    t->z = 0.02f;
//    t->scaleX = t->scaleY = t->scaleZ = 0.5f;
//
//    p->velX = right ? 6.0f : -6.0f;
//    p->velY = 5.0f;
//    p->onGround = false;
//
//    DebugLog::addMessage("[LogicContainer] player fired bullet", DebugMode::PlaySimul);
//    std::cout << "[LogicContainer] player fired bullet\n";
//}

// ============================================================================
// Main Update
// ============================================================================
/*!
 * \brief Main per-frame update for the LogicContainer.
 *
 * Responsibilities:
 * - Increase state time
 * - Handle shooting (SPACE key)
 * - Run per-state tick function (Idle/Walking/Jumping/Falling)
 * \param obj The player GameObject being updated.
 * \param dt Delta time in seconds.
 * \param manager GameObjectManager used for spawning bullets.
 */
void LogicContainer::update(GameObject& obj, float dt /*, GameObjectManager& manager*/) {
    auto* fsm = obj.getComponent<StateMachine>();
    if (!fsm) return;

    fsm->stateTime += dt;

    // Immediate action: shooting
    /*if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE)) {
        trySpawnBulletNear(obj, manager);
    }*/

    switch (fsm->state) {
    case PlayerState::Idle:     tickIdle(obj, dt);     break;
    case PlayerState::Walking:  tickWalking(obj, dt);  break;
    case PlayerState::Jumping:  tickJumping(obj, dt);  break;
    case PlayerState::Falling:  tickFalling(obj, dt);  break;
    case PlayerState::Dead:     tickDead(obj, dt);     break;
    default: break;
    }
}