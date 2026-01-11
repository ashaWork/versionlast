/* Start Header ************************************************************************/
/*!
\file        font.h
\author      Seow Sin Le, s.sinle, 2401084
\par         s.sinle@digipen.edu
\date        November, 1st, 2025
\brief       initializes the fonts

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <glm/glm.hpp>
#include <map>
#include <shader.hpp>

#include "fontTypes.h"
#include "ResourceManager.h"

struct Font {
    struct FontMdl {
        GLuint vao, vbo;
    };

    //struct Character {
    //    GLuint TextureID;  // ID handle of the glyph texture
    //    glm::ivec2   Size;       // Size of glyph
    //    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    //    GLuint Advance;    // Offset to advance to next glyph
    //    FontMdl mesh;
    //};

    using Character = FontCharacter;

    static std::vector<FontMdl> fontMdls;
    static GLuint fontShaders;
    static int fontTypes;

    static std::vector<FT_Face> face;
    static FT_Library ft;
    static void freeFonts();

    /*static std::map<char, Character> Characters;
    static std::vector<std::map<char, Character>> characters;
    static int init();
    static int update(size_t fontElement);
    static FontMdl fontMeshInit();
    static void uploadFontTex(size_t fontElement);
    static std::vector<std::string> fonts;*/


    static int init();
    static FontMdl fontMeshInit();
};