/* Start Header ************************************************************************/
/*!
\file       message.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       November, 4th, 2025
\brief      Defines struct for Message used in the Message Bus system.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <string>
#include <any>

class GameObject; //forward declaration

struct Message {
    std::string type;
    GameObject* sender;
    std::any payload;

    Message(const std::string& t, GameObject* s, std::any p = {})
        : type(t), sender(s), payload(p) {}
};
