/* Start Header ************************************************************************
\file       config.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       October, 23rd, 2025
\brief      Configuration structures and JSON loader (RapidJSON).
* End Header **************************************************************************/
#pragma once

#include <cstdio>
#include <string>
#include <cstring>
#include <algorithm>
#include <fstream>                
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <filesystem>

struct AppConfig {
    // window
    int         width = 1600;
    int         height = 900;
    std::string title = "Tanuki Lab";
    bool        vsync = true;
    bool        fullscreen = false;

    // graphics
    float       clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

    // debug
    bool        show_input_debug = false;
    bool        show_fps_in_title = true;
};

bool LoadConfig(const std::string& path, AppConfig& out, std::string* err = nullptr);

/* (Optional) global mirrors if other code reads them directly */
extern bool  gShowFpsInTitle;
extern bool  gShowInputDebug;
extern float gClearColor[4];

bool SaveConfig(const std::string& path,
    const AppConfig& cfg,
    std::string* err = nullptr,
    bool pretty = true);


