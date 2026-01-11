/* Start Header ************************************************************************/
/*!
\file        Component.h
\author      Seow Sin Le, s.sinle, 2401084, 20%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 40%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 15%
\author      Asha Mathyalakan, asha.m, 2402886, 25%
             Total - 100%
\par         s.sinle@digipen.edu
\par         low.h@digipen.edu
\par		 p.lin@digipen.edu
\par         asha.m@digipen.edu
\date        October, 1st, 2025
\brief       This file contains all the component definitions for the Component-based architecture.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <memory> //for unique_ptr
#include "renderer.h"
#include "mathlib.h"
#include "font.h"
#include "dynamics.h"


// at least it works!!!
namespace JsonIO {
    // for the indexing in animation vector
    inline std::vector<std::string> stateNames = { "Idle", "Walking", "Jumping", "Falling", "Shooting", "Dead" };
}

// basic component class
class Component {
public:
    // virtual just means that the child class can overwrite 
    // these functions
    virtual ~Component() = default;

    virtual std::unique_ptr<Component> clone() const = 0;
};

// Transform component to store position, rotation, and scale
struct Transform : public Component {
public:
    Transform() = default;
    Transform(float px, float py, float pz, float rot, float sx, float sy, float sz)
        : x(px), y(py), z(pz), rotation(rot), scaleX(sx), scaleY(sy), scaleZ(sz), mdlWorld(1) {
    }

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Transform>(*this);
    }

    float x{ 0.f }, y{ 0.f }, z{ 0.f };
    float rotation = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ{ 0.f };
    bool flipX = false; // flip scaleX when change direction, true means flip to left
    glm::mat4 mdlWorld = 1.f;
};

// Render component to store rendering-related properties
struct Render : public Component {
public:
    Render() : texHDL{ 0 } {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Render>(*this);
    }

public:
    bool visible = true;
    bool hasTex = false;
    bool isTransparent = false;
    bool hasAnimation = false;
    //bool wireframe = false;
    // sorry but i need this for real time texture update
    // wait for boss sin le to help me settle this - kelly :)
    bool texChanged = false;
    GLuint texHDL;
    std::string texFile;
    glm::vec3 clr{ 1,1,1 };
    renderer::model modelRef;
};

struct FontComponent : public Component {
    FontComponent() = default;

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<FontComponent>(*this);
    }

    //GLuint texHdl;
    Font::FontMdl mdl = Font::fontMdls[0];
    std::string word = "";
    float scale = 1;
    glm::vec3 clr{ 0.f,0.f,0.f };
    int fontType = 1;

    std::string fontPath = "assets/ARIAL.TTF";
};

// animation for different state
struct AnimateState {
    bool loop = true;
    Vector2D initialFrame{}, lastFrame{};
    int currentFrameColumn{}, currentFrameRow{}, totalColumn{ 1 }, totalRow{ 1 };
    float frameTime = 0.1f, frameTimer{};
    GLuint texHDL{};
    std::string texFile{};

    // do not serialize this
    bool texChanged = false;
};

struct Animation : public Component
{
    Animation() { animState.resize(JsonIO::stateNames.size()); } // set animState to be of equal size of statenames
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Animation>(*this);
    }

    bool runItBack = true;
    std::vector<AnimateState> animState; // to store the animation data for dff state, index by playerstate
};

// Physics component to store velocity and other physics-related properties
struct Physics : public Component {
public:

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Physics>(*this);
    }

    bool physicsFlag = true;
    float moveSpeed = 1.0f;

    Dynamics dynamics;

    bool canMove = true;  // Add this flag


    float velX = 0.0f;
    float velY = 0.0f;
    float gravity = -9.8f;

    //Jump 
    float damping = 0.98f;
    float jumpForce = 100.0f;
    float floorY = -7.5f;
    bool onGround = false;

    //water bullet
    bool alive = false;
    float lifeTimer = 0.0f;
    float maxLifetime = 3.0f;
    Vector2D originalPos{ 0, 0 };
    Vector2D originalVel{ 0, 0 };
    bool isOriginalStateSet = false;

    //bool inWaterTriggered; // true only when W is pressed

    bool inWater = false;
    bool buoancy = false;

    //const float waterBounce = 60.0f;   // upward velocity when entering water

};

// Input component to store input-related properties
struct Input : public Component {
public:
    //Input(float speed = 0.0f) : moveSpeed(speed) {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Input>(*this);
    }

public:
    //float moveSpeed = 0.0f;

};

// enum class used for CollisionSystem, to determine to stop/push by force (currently not force ig)
enum class CollisionResponseMode {
    StopWhenCollide, // if collide by another obj, just stop
    MoveWhenCollide // if collide by another obj, apply "fake" force and kena push
};
//

// CollisionInfo component to store collision-related information
struct CollisionInfo : public Component {
public:

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<CollisionInfo>(*this);
    }


    bool collisionFlag = true; // whether objest can collide with another obj
    shape colliderType = shape::square;
    Vector2D colliderSize{ 0,0 }; // size of the bounding box, if collider is circle then x will be diamater
    bool autoFitScale = true; // auto-fit the collider with the size of the mesh (i.e. obj bigger -> collider bigger)
    CollisionResponseMode collisionRes = CollisionResponseMode::StopWhenCollide;

    // run-time calculation, do not de/serialize
    bool collided = false; // check whether obj alr collided
    float timeOfImpact = 0.0f;
    Vector2D normal{ 0,0 };
    Vector2D contactPoint = { 0,0 };
    float penetration = 0.0f; //for it to not overlap 
};

// For imgui(testing)
struct UIComponent : public Component {
    //idk empty as of now
    //idk how i can use it
};

//for lua scripting
//to know which object to run
struct LuaScript : public Component {
public:
    LuaScript(const std::string& file)
        : filename(file) {
        // Extract just the filename without extension to use as scriptName
        size_t lastSlash = file.find_last_of("/\\");
        std::string fname = (lastSlash != std::string::npos) ? file.substr(lastSlash + 1) : file;

        size_t lastDot = fname.find_last_of('.');
        scriptName = (lastDot != std::string::npos) ? fname.substr(0, lastDot) : fname;
    }

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<LuaScript>(*this);
    }

    std::string filename;   // Full path to the Lua file
    std::string scriptName; // Just the object name (e.g., "bullet", "player")
};

// Logic Container (FSM)
enum class PlayerState { Idle, Walking, Jumping, Falling, Shooting, Dead };

struct StateMachine : public Component {
    PlayerState state = PlayerState::Idle;
    float       stateTime = 0.f;
    bool        facingRight = true;
    bool        fixState = false;
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<StateMachine>(*this);
    }
};

//Audio component
struct FadeInfo {
    bool isFading = false;
    bool isFadingIn = false;
    float startVolume = 0.0f;
    float targetVolume = 0.0f;
    float fadeDuration = 0.0f;
    float fadeTimer = 0.0f;

    float defaultFadeInDuration = 1.0f;
    float defaultFadeOutDuration = 1.0f;
};

enum class AudioState {
    Stopped,
    Playing,
    Paused
};

//AudioChannel serves as the real storage for sounds and their data.
struct AudioChannel {
    FMOD::Channel* channel = nullptr;
    std::string audioFile;
    AudioState state = AudioState::Stopped;

    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool muted = false;

    bool fadeInOnStart = false;
    float fadeInDuration = 1.0f;
    bool fadeOutOnStop = false;
    float fadeOutDuration = 1.0f;
    FadeInfo fadeInfo;

    bool isPendingPlay = false;
    bool isPendingStop = false;
    bool playOnStart = false;
};

//enum class soundID : int;
//enum class AudioState;
class FMOD::Channel;

//Audio component stores multiple Audio channels
//sort of acts as a manager of AudioChannels
struct AudioComponent : public Component {
public:
    AudioComponent() = default;

    /*AudioComponent(const std::string& file, float vol = 1.0f, bool shouldLoop = false, bool playStart = false)
        : audioFile(file)
        , volume(vol)
        , loop(shouldLoop)
        , playOnStart(playStart)
    {}*/
    //need to change this to make it work with multiple sounds

    AudioComponent(const std::string& file, float vol = 1.0f,
        bool shouldLoop = false, bool playStart = false)
    {
        // Create "default" channel with these parameters
        AudioChannel& ch = audioChannels["default"];
        ch.audioFile = file;
        ch.volume = vol;
        ch.loop = shouldLoop;
        ch.playOnStart = playStart;
    }

    std::unique_ptr<Component> clone() const override {
        auto cloned = std::make_unique<AudioComponent>();

        for (const auto& pair : audioChannels) {
            cloned->audioChannels[pair.first] = pair.second;
            cloned->audioChannels[pair.first].channel = nullptr;
            cloned->audioChannels[pair.first].state = AudioState::Stopped;
            cloned->audioChannels[pair.first].fadeInfo.isFading = false;
        }

        return cloned;
    }

    // Audio ID - which sound to play
    //std::string audioFile;

    //prolly need a map for 1 component to multiple sounds
    std::map<std::string, AudioChannel> audioChannels;

    //soundID soundId = static_cast<soundID>(0);

    AudioChannel* getChannel(const std::string& channelName) {
        auto it = audioChannels.find(channelName);
        if (it != audioChannels.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    const AudioChannel* getChannel(const std::string& channelName) const {
        auto it = audioChannels.find(channelName);
        if (it != audioChannels.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    //get or create channel
    AudioChannel* getOrCreateChannel(const std::string& name) {
        return &audioChannels[name];
    }

    //check if channel exists
    bool hasChannel(const std::string& name) const {
        return audioChannels.find(name) != audioChannels.end();
    }

    //remove a channel
    void removeChannel(const std::string& name) {
        auto it = audioChannels.find(name);
        if (it != audioChannels.end()) {
            //stop the channel if playing
            if (it->second.channel) {
                it->second.isPendingStop = true;
            }
            audioChannels.erase(it);
        }
    }

    //idk if we need this
    AudioChannel* getDefaultChannel() {
        return getOrCreateChannel("default");
    }

    void updateFades(float deltaTime) {
        for (auto& pair : audioChannels) {
            AudioChannel& ch = pair.second;

            if (!ch.fadeInfo.isFading || !ch.channel) return;

            ch.fadeInfo.fadeTimer += deltaTime;

            float progress = ch.fadeInfo.fadeTimer / ch.fadeInfo.fadeDuration;
            if (progress > 1.0f) progress = 1.0f;

            float currentVolume;
            if (ch.fadeInfo.isFadingIn) {
                currentVolume = ch.fadeInfo.startVolume + (ch.fadeInfo.targetVolume - ch.fadeInfo.startVolume) * progress;
                std::cerr << "sound fading" << currentVolume << std::endl;
            }
            else {
                currentVolume = ch.fadeInfo.startVolume + (ch.fadeInfo.targetVolume - ch.fadeInfo.startVolume) * progress;
            }

            FMODErrorCheck(ch.channel->setVolume(currentVolume));

            if (progress >= 1.0f) {
                ch.fadeInfo.isFading = false;

                if (!ch.fadeInfo.isFadingIn && ch.fadeInfo.targetVolume == 0.0f) {
                    ch.channel->stop();
                    ch.channel = nullptr;
                }
            }

        }

    }
};

// tile map component
struct TileMap : public Component
{
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<TileMap>(*this);
    }

    //std::string filename{};
    size_t tilesetTexID = 0;

    //how big is the tile
    float tileW = 3.0f;
    float tileH = 3.0f;

    int rows = 50;
    int columns = 50;

    //size_t tilesetRows = 1, tilesetCols = 1;

    struct TileKey {
        int x;
        int y;

        bool operator==(const TileKey& other) const {
            return x == other.x && y == other.y;
        }
    };

    struct TileKeyHash {
        std::size_t operator()(const TileKey& k) const noexcept {
            return (std::hash<int>()(k.x) << 1) ^ std::hash<int>()(k.y);
        }
    };
    std::unordered_map<TileKey, std::string, TileKeyHash> tiles;

    std::string getTile(int x, int y) const {
        TileKey key{ x, y };
        auto it = tiles.find(key);
        return (it != tiles.end()) ? it->second : ""; // -1 = empty
    }

    void setTile(int x, int y, std::string tileID) {
        if (x >= -columns && x < columns && y >= -rows && y < rows) {
            tiles[{x, y}] = tileID;
        }
    }

    void clearTile(int x, int y) {
        tiles.erase({ x, y });
    }
};