/* Start Header ************************************************************************/
/*!
\file       ResourceManager.h
\author     Hugo Low Ren Hao, low.h, 2402272

\par        low.h@digipen.edu
\date       October, 30, 2025
\brief      This file contains the declaration of the ResourceManager class. It loads resources
            from files on-demand, stores them in memory to avoid redundant loading, and
            manages their lifecycle.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "GL/glew.h"
#include <../lib/FMOD/fmod.hpp>
#include "stb_image.h"
#include "shader.hpp"
#include "AudioUtility.h"
#include "fontTypes.h"

struct TextureData {
    GLuint id = 0;
    bool isTransparent = false;
};

struct FontData {
    FT_Face face = nullptr;
	std::map<unsigned char, FontCharacter> characters;
	bool isLoaded = false;
};

class ResourceManager {
public:
    /**
     * @brief Gets the singleton instance of the ResourceManager.
     */
    static ResourceManager& getInstance();

    /**
     * @brief Initializes the manager. Call this once at startup.
     * @param fmodSystem A pointer to the FMOD system from AudioHandler.
     */
    void init(FMOD::System* fmodSystem);

    /**
     * @brief Cleans up and releases all loaded resources. Call this at shutdown.
     */
    void shutdown();

    /**
     * @brief Gets a texture. Loads it from file if not in cache.
     * @param path The filepath to the texture.
     * @return A struct containing the texture ID and transparency info.
     */
    TextureData getTexture(const std::string& path);

    /**
     * @brief Gets a sound. Loads it from file if not in cache.
     * @param path The filepath to the sound.
     * @param loop Whether the sound should loop (FMOD_LOOP_NORMAL).
     * @return A pointer to the FMOD::Sound object.
     */
    FMOD::Sound* getSound(const std::string& path, bool loop = false);

    /**
     * @brief Gets a shader program. Loads and links it if not in cache.
     * @param vertPath The filepath to the vertex shader.
     * @param fragPath The filepath to the fragment shader.
     * @return The GLuint program ID.
     */
    GLuint getShader(const std::string& vertPath, const std::string& fragPath);

    const FontData& getFont(const std::string& relativePath);

    std::vector<std::string> getLoadedFontPaths() const;

    // Delete copy/move constructors and assignment operators
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;


private:
    ResourceManager() = default;  // Private constructor
    ~ResourceManager() = default; // Private destructor

    // Private helper functions to load from disk
    TextureData loadTextureFromFile(const std::string& path);
    FontData loadFontFromFile(const std::string& path);
    FMOD::Sound* loadSoundFromFile(const std::string& path, bool loop);

    // FMOD system instance (required for sound creation)
    FMOD::System* m_fmodSystem = nullptr;
    FT_Library m_ftLibrary = nullptr;

    // Caches for loaded resources
    std::unordered_map<std::string, TextureData> m_textureCache;
    std::unordered_map<std::string, FMOD::Sound*> m_audioCache;
    std::unordered_map<std::string, GLuint> m_shaderCache;
    std::unordered_map<std::string, FontData> m_fontCache;


};
