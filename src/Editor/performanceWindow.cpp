/* Start Header ************************************************************************/
/*!
\file       gameDebugLog.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of the performance for all the
            systems.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Editor/editorManager.h"

void PerformanceWindow::render(){
    ImGui::Begin("Performance");
    //time frame:
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate);
    ImGui::Separator();
    //average time --can add for other functions also just need to add 4 lines of codes into the function start and end(see InputSystem::Update in system.cpp)
    double totalMs = 0.0;
    for (auto& timer : g_SystemTimers) totalMs += timer.ms;
    //loop through all timers and display
    for (auto& timer : g_SystemTimers) {
        float percent = totalMs > 0.0 ? (float)((timer.ms / totalMs) * 100.0) : 0.0f;
        ImGui::Text("%s: %.3f ms (%.1f%%)", timer.name.c_str(), timer.ms, percent);
        ImGui::ProgressBar(percent / 100.0f, ImVec2(0.0f, 0.0f));
    }
    g_SystemTimers.clear(); // clear timers for next frame
    ImGui::End();
}
#endif