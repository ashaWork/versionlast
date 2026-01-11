/* Start Header ************************************************************************/
/*!
\file		main.cpp
\author     Hugo Low Ren Hao, low.h, 2402272, 50%
\author     Varick, v.teo, 2403417, 50%
\par        low.h@digipen.edu
\par        v.teo@digipen.edu
\date		October, 1st, 2025
\brief      Main entry point for the OpenGL application.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "CoreEngine.h"

#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#endif

// This allows for memory leak detection in debug builds
#if defined(DEBUG) | defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char** argv){
#if defined(DEBUG) | defined(_DEBUG)
    // Enable run-time memory check for debug builds
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Initialize the crash logger
    CrashLog::Init("crash_log.txt");

    bool forceWindowed = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--windowed")
        {
            forceWindowed = true;
        }
    }

    // Create the engine using a smart pointer for automatic cleanup
    auto engine = std::make_unique<CoreEngine>();

    try {
        engine->Init(forceWindowed);
        engine->Run();
    }
    catch (const std::exception& e) {
        CrashLog::WriteException(e.what());
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        engine->Shutdown();
        return -1;
    }
    catch (...) {
        CrashLog::WriteException("Unknown exception");
        std::cerr << "Unhandled unknown exception\n";
        engine->Shutdown();
        return -1;
    }

    // Gracefully shut down the engine and logger
    engine->Shutdown();
    CrashLog::Shutdown();
    return 0;
}