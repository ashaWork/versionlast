/* Start Header ************************************************************************/
/*!
\file        fontTypes.h
\author      Hugo Low Ren Hao, low.h, 2402272
\par         low.h@digipen.edu
\date        November, 20th, 2025
\brief       initializes the fonts

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

//storing of each character for the chosen font
struct FontCharacter {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};