/* Start Header ************************************************************************/
/*!
\file       performance.h
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

#pragma once
#include <string>
#include <vector>
#include <iostream>

//struct for storing system timer info
struct SystemTimer {
	std::string name;
	double ms;
};

extern std::vector<SystemTimer> g_SystemTimers;
void LogSystemTimersEveryInterval(float deltaTime, double intervalSeconds = 15.0); //for console
