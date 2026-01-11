/* Start Header ************************************************************************/
/*!
\file       performance.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
			- 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawPerformance, drawControls, drawDebug, shapeControl
			for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "performance.h"

std::vector<SystemTimer> g_SystemTimers;

void LogSystemTimersEveryInterval(float deltaTime, double intervalSeconds){
    static double accumulator = 0.0;
    accumulator += deltaTime;

    if (accumulator >= intervalSeconds){
        accumulator = 0.0;

        double totalMs = 0.0;
        for (auto& timer : g_SystemTimers)
            totalMs += timer.ms;

        std::cout << "\n=== System Performance Report(every 15 seconds) ===\n";
        for (auto& timer : g_SystemTimers){
            float percent = totalMs > 0.0 ? (float)((timer.ms / totalMs) * 100.0) : 0.0f;
            std::cout << timer.name << ": " << timer.ms << " ms (" << percent << "%)" << std::endl;
        }
        std::cout << "=================================\n" << std::endl;
    }
    g_SystemTimers.clear();
}
