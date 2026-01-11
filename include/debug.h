/* Start Header ************************************************************************/
/*!
\file		debug.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 1st, 2025
\brief      Declarations for crash logging and performance utilities.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <deque>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <csignal>
#include <mutex>
#include <ctime>

// ======================================================================================
// Perf namespace: frame timing & FPS display
// ======================================================================================
namespace Perf {

    /**
     * @brief Update frame delta time and compute rolling FPS.
     *
     * Call once per frame. Internally maintains state across calls.
     *
     * @param[out] delta   Seconds between frames.
     * @param[out] fps     Frames per second, smoothed over `fps_calc_interval`.
     * @param[in]  fps_calc_interval Interval (seconds) over which FPS is averaged.
     *                               Default = 0.5. Clamped to [0, 10].
     */
    void UpdateTime(double& delta, double& fps, double fps_calc_interval = 0.5);


    void UpdateTime(double& delta, double& fps, double fps_calc_interval,
        double fixed_dt, int& steps_out, double& alpha_out,
        int max_steps_per_frame = 8);

    /**
     * @brief Update the GLFW window title with a base title and optional FPS.
     *
     * @param window   GLFW window handle.
     * @param title    Base title string.
     * @param fps      Current frames per second (calculated by UpdateTime).
     * @param showFps  If true, appends " | FPS: xx.x" to the title.
     */
    void UpdateWindowTitle(GLFWwindow* window,
        const std::string& title,
        double fps,
        bool showFps);

} // namespace Perf

// ======================================================================================
// CrashLog namespace: crash logging & signal handling
// ======================================================================================
namespace CrashLog {

    /**
     * @brief Initialize crash logging and install handlers.
     *
     * Safe to call once at program start. Creates/uses a log file (default "crash_log.txt")
     * and writes a start marker. Registers terminate handler, signal handlers,
     * and (on Windows) SEH filter.
     *
     * @param logPath Optional file path for log output.
     */
    void Init(const char* logPath = "crash_log.txt");

    /**
     * @brief Write a custom informational line to the crash log.
     *
     * @param msg Message string to log. Ignored if null.
     */
    void WriteLine(const char* msg);

    /**
     * @brief Log an unhandled C++ exception message.
     *
     * Typically called inside a catch block if you want to record exception details.
     *
     * @param what The `what()` string from std::exception, or custom message.
     */
    void WriteException(const char* what);

    /**
     * @brief Mark the end of the application in the log file.
     *
     * Safe to omit; the file will close automatically when the process exits.
     */
    void Shutdown();
}


// ======================================================================================
// DebugInput: lightweight input event logger (call from your GLFW callbacks)
// ======================================================================================


namespace DebugInput {

    /**
     * @brief Enable or disable input event logging.
     *
     * When enabled, events passed through the DebugInput callbacks will be
     * formatted and written to the crash log, console (debug builds),
     * and stored in an internal ring buffer for later retrieval.
     *
     * @param on True to enable logging, false to disable.
     */
    void SetEnabled(bool on);

    /**
     * @brief Check whether input event logging is currently enabled.
     *
     * @return True if logging is enabled, false otherwise.
     */
    bool Enabled();

    /**
     * @brief Handle a keyboard event from GLFW.
     *
     * Call this inside your GLFW key callback. Logs the key name,
     * scancode, action (press/release/repeat), and modifiers.
     *
     * @param window   The GLFW window associated with the event.
     * @param key      The keyboard key (GLFW key code).
     * @param scancode Platform-specific scancode.
     * @param action   The key action (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
     * @param mods     Modifier flags (bitfield for shift, ctrl, alt, etc.).
     */
    void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * @brief Handle a mouse button event from GLFW.
     *
     * Call this inside your GLFW mouse button callback. Logs button
     * identity (left/right), action (press/release), modifiers, and
     * the cursor position at the time of the click.
     *
     * @param window The GLFW window associated with the event.
     * @param button The mouse button code (e.g. GLFW_MOUSE_BUTTON_LEFT).
     * @param action The mouse action (GLFW_PRESS or GLFW_RELEASE).
     * @param mods   Modifier flags (bitfield for shift, ctrl, alt, etc.).
     */
    void OnMouseButton(GLFWwindow* window, int button, int action, int mods);

    /**
     * @brief Handle a mouse cursor movement event from GLFW.
     *
     * Call this inside your GLFW cursor position callback. Logs
     * the updated cursor coordinates in window client space.
     *
     * @param window The GLFW window associated with the event.
     * @param xpos   The new x-coordinate of the cursor.
     * @param ypos   The new y-coordinate of the cursor.
     */
    void OnCursorPos(GLFWwindow* window, double xpos, double ypos);

    /**
     * @brief Handle a scroll wheel (or gesture) event from GLFW.
     *
     * Call this inside your GLFW scroll callback. Logs horizontal
     * and vertical scroll offsets.
     *
     * @param window  The GLFW window associated with the event.
     * @param xoffset Scroll offset along the x-axis.
     * @param yoffset Scroll offset along the y-axis.
     */
    void OnScroll(GLFWwindow* window, double xoffset, double yoffset);

    /**
     * @brief Retrieve the most recent logged input events.
     *
     * Returns up to @p max of the latest event strings stored in
     * the internal ring buffer. Useful for displaying in an on-screen
     * HUD or ImGui debug window.
     *
     * @param max Maximum number of recent messages to retrieve.
     * @return A deque of recent log lines (newest last).
     */
    std::deque<std::string> Recent(size_t max = 32);
}

/*!
\namespace DebugDiag
\brief Contains debugging and diagnostic tools for verifying engine paths.

The DebugDiag namespace provides utility functions that help validate the
engine's installation directories, asset folders, and configuration files.
These tools are useful for diagnosing missing assets or incorrect project
structure during development.
*/
namespace DebugDiag {

	void AuditRuntimePaths();

}