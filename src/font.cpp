#include "font.h"
/* Start Header ************************************************************************/
/*!
\file        font.cpp
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

//std::vector<FT_Face> Font::face;
//FT_Library Font::ft;
//int Font::fontTypes = 3;
//std::map<char, Font::Character> Font::Characters;
//std::vector<std::string> Font::fonts;
//std::vector<std::map<char, Font::Character>> Font::characters;

std::vector<Font::FontMdl> Font::fontMdls;
GLuint Font::fontShaders;

int Font::init()
{
    /*fonts.push_back("shaders/Orange Knight.ttf");
    fonts.push_back("shaders/ARIAL.TTF");
    fonts.push_back("shaders/times.ttf");
    fontTypes = fonts.size();
    face.resize(fontTypes);
    characters.resize(fontTypes);
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    for (int i = 0; i < fontTypes; ++i)
    {
        if (FT_New_Face(ft, fonts[i].c_str(), 0, &face[i]))
        {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return -1;
        }
        FT_Set_Pixel_Sizes(face[i], 0, 48);
        uploadFontTex(i);
    }
    */

    //fontShaders = LoadShaders("shaders/font.vert", "shaders/font.frag", true);
	fontShaders = ResourceManager::getInstance().getShader("shaders/font.vert", "shaders/font.frag");
    fontMdls.push_back(fontMeshInit());

    std::cout << "Font system initialized (shader and mesh)" << std::endl;
    return 0;
}

Font::FontMdl Font::fontMeshInit()
{
    FontMdl mdl;
    glGenVertexArrays(1, &mdl.vao);
    glGenBuffers(1, &mdl.vbo);
    glBindVertexArray(mdl.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mdl.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return mdl;
}

//int Font::update(size_t fontElement)
//{
//    if (FT_Load_Char(face[fontElement], 'X', FT_LOAD_RENDER))
//    {
//        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//        return -1;
//    }
//}


//moved to resourcemanager.cpp
// 
//void Font::uploadFontTex(size_t fontElement)
//{
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
//
//    for (unsigned char c = 0; c < 255; c++)
//    {
//        // load character glyph 
//        if (FT_Load_Char(face[fontElement], c, FT_LOAD_RENDER))
//        {
//            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//            continue;
//        }
//        // generate texture
//        GLuint texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            face[fontElement]->glyph->bitmap.width,
//            face[fontElement]->glyph->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            face[fontElement]->glyph->bitmap.buffer
//        );
//        // set texture options
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        // now store character for later use
//        Character character = {
//            texture,
//            glm::ivec2(face[fontElement]->glyph->bitmap.width, face[fontElement]->glyph->bitmap.rows),
//            glm::ivec2(face[fontElement]->glyph->bitmap_left, face[fontElement]->glyph->bitmap_top),
//            face[fontElement]->glyph->advance.x
//        };
//        characters[fontElement].insert(std::pair<char, Character>(c, character));
//    }
//}

void Font::freeFonts()
{
    /*for(int i = 0; i < fontTypes; ++i)
        FT_Done_Face(face[i]);
    FT_Done_FreeType(ft);*/

    //Clean up font VAOs/VBOs
    for (auto& mdl : fontMdls) {
        if (mdl.vao) glDeleteVertexArrays(1, &mdl.vao);
        if (mdl.vbo) glDeleteBuffers(1, &mdl.vbo);
    }
    fontMdls.clear();

    //Delete shader program
    if (fontShaders) {
        glDeleteProgram(fontShaders);
        fontShaders = 0;
    }
}