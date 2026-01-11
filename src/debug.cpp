/* Start Header ************************************************************************/
/*!
\file		debug.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 1st, 2025
\brief      Crash logging and lightweight performance utilities.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "debug.h"
#include "JsonIO.h"
// ======================================================================================
// Internal helpers (anonymous namespace): thread-safe file append + time formatting
// ======================================================================================
namespace {
    // Shared singletons hidden in anonymous namespace
    std::mutex& LogMutex() { static std::mutex m; return m; }
    std::string& LogPath() { static std::string p = "crash_log.txt"; return p; }
    bool& Inited() { static bool inited = false; return inited; }

    /**
     * @brief Produce a local-time timestamp "YYYY-MM-DD HH:MM:SS".
     */
    std::string Timestamp()
    {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    /**
     * @brief Append a single line to the log file with a timestamp.
     *        Thread-safe via a mutex.
     * @param line Text to append (without trailing newline).
     */
    void AppendLine(const std::string& line)
    {
        std::lock_guard<std::mutex> lock(LogMutex());
        std::ofstream out(LogPath(), std::ios::app);
        if (!out) return; // if logging fails, avoid throwing in a crash path
        out << "[" << Timestamp() << "] " << line << "\n";
    }

    // --------- signal/exception/terminate handlers (log then abort) ----------

    /**
     * @brief POSIX signal handler. Logs signal type then aborts.
     */
    void SigHandler(int sig)
    {
        switch (sig) {
        case SIGSEGV: AppendLine("Crash: SIGSEGV (segmentation fault)"); break;
        case SIGABRT: AppendLine("Crash: SIGABRT (abort)"); break;
        default:      AppendLine("Crash: signal " + std::to_string(sig)); break;
        }
        std::abort(); // terminate immediately after logging
    }

#ifdef _WIN32
    /**
     * @brief Windows SEH filter for unhandled exceptions (e.g., access violation).
     *        Logs a generic SEH message and returns control to the OS.
     */
    LONG WINAPI SehFilter(EXCEPTION_POINTERS* info)
    {
        (void)info;
        AppendLine("Crash: Unhandled SEH (Windows structured exception)");
        return EXCEPTION_EXECUTE_HANDLER; // hand back to OS after logging
    }
#endif

    /**
     * @brief C++ terminate handler. Logs and aborts (likely due to unhandled exception).
     */
    void TerminateHandler()
    {
        AppendLine("Crash: std::terminate called (likely unhandled exception)");
        std::abort();
    }
} // anonymous namespace

// ======================================================================================
// CrashLog API
// ======================================================================================
namespace CrashLog {

    /**
     * @brief Initialize crash logging and register failure handlers.
     * @param logPath Optional path to the log file; defaults to "crash_log.txt".
     *
     * Safe to call multiple times; subsequent calls are ignored.
     */
    void Init(const char* logPath)
    {
        if (Inited()) return;
        if (logPath && *logPath) LogPath() = logPath;

        // Mark the start of the process in the log
        AppendLine("===== Application start =====");

        // Hook C++ terminate
        std::set_terminate(::TerminateHandler);

        // Register POSIX signals
        std::signal(SIGSEGV, ::SigHandler);
        std::signal(SIGABRT, ::SigHandler);

#ifdef _WIN32
        // Windows SEH for access violations and similar
        SetUnhandledExceptionFilter(::SehFilter);
#endif

        Inited() = true;
    }

    /**
     * @brief Write an informational line (prefixed with "Note: ") to the log.
     */
    void WriteLine(const char* msg)
    {
        if (!msg) return;
        AppendLine(std::string("Note: ") + msg);
    }

    /**
     * @brief Write details of an unhandled C++ exception message to the log.
     * @param what Exception what() string (may be null).
     */
    void WriteException(const char* what)
    {
        std::string line = "Unhandled C++ exception: ";
        line += (what ? what : "(null)");
        AppendLine(line);
    }

    /**
     * @brief Mark the end of the application in the log.
     */
    void Shutdown()
    {
        AppendLine("===== Application end =====");
    }
} // namespace CrashLog


// ======================================================================================
// Perf utilities: delta-time, rolling FPS, and window title updates
// ======================================================================================
namespace Perf {

    /**
     * @brief Update frame delta time and compute rolling FPS over an interval.
     *
     * Internally tracks previous time, a frame counter, and a start time for
     * the current FPS window. When the window exceeds @p fps_calc_interval,
     * the FPS value is refreshed as (frames / elapsed).
     *
     * @param[out] delta  Seconds elapsed since last call.
     * @param[out] fps    Frames per second (updated at the chosen interval).
     * @param[in]  fps_calc_interval  Seconds between FPS recomputations.
     *                                Clamped to [0, 10]. Use 0 for per-frame updates.
     */
    void UpdateTime(double& delta, double& fps, double fps_calc_interval) {
        // Track previous frame time
        static double prev_time = glfwGetTime();
        const  double curr_time = glfwGetTime();
        delta = curr_time - prev_time;
        prev_time = curr_time;

        // Rolling FPS accumulators
        static double count = 0.0;
        static double start_time = curr_time;
        const  double elapsed_time = curr_time - start_time;

        ++count;

        // Keep the interval within a reasonable range
        if (fps_calc_interval < 0.0) fps_calc_interval = 0.0;
        if (fps_calc_interval > 10.0) fps_calc_interval = 10.0;

        // Recompute FPS when the interval has passed
        if (elapsed_time > fps_calc_interval) {
            fps = count / elapsed_time;
            start_time = curr_time;
            count = 0.0;
        }
    }

    void UpdateTime(double& delta, double& fps, double fps_calc_interval,
        double fixed_dt, int& steps_out, double& alpha_out,
        int max_steps_per_frame)
    {
        // 1) Update variable delta & FPS using the original function
        UpdateTime(delta, fps, fps_calc_interval);

        // 2) Accumulate time and emit a bounded number of fixed steps
        static double accumulator = 0.0;

        // Optional: clamp extremely large deltas (e.g., when the window was dragged/paused)
        const double max_frame_delta = 0.25; // avoid accumulating >250ms in one go
        const double clamped_delta = (delta > max_frame_delta) ? max_frame_delta : delta;

        accumulator += clamped_delta;

        // Safety: fixed_dt must be positive
        if (fixed_dt <= 0.0) fixed_dt = 1.0 / 60.0;

        int steps = 0;
        while (accumulator >= fixed_dt && steps < max_steps_per_frame) {
            accumulator -= fixed_dt;
            ++steps;
        }

        steps_out = steps;
        alpha_out = accumulator / fixed_dt; // for render interpolation (0..1)
    }

    /**
     * @brief Update the OS window title with a base title and optional FPS figure.
     * @param window  GLFW window handle.
     * @param title   Base text to display.
     * @param fps     Frames per second (from UpdateTime).
     * @param showFps If true, appends " | FPS: xx.x" to the title.
     */
    void UpdateWindowTitle(GLFWwindow* window,
        const std::string& title,
        double fps,
        bool showFps) {
        if (!window) return;

        std::ostringstream oss;
        oss << title;
        if (showFps) {
            oss << " | FPS: " << std::fixed << std::setprecision(1) << fps;
        }
        glfwSetWindowTitle(window, oss.str().c_str());
    }

} // namespace Perf


// ======================================================================================
// DebugInput implementation
// ======================================================================================

namespace {
    /**
     * @brief Tracks whether input debugging is currently enabled.
     *
     * @return Reference to a static boolean flag that persists across calls.
     */
    bool& dbgInputOn() { static bool v = false; return v; }

    /**
     * @brief Holds a queue of recent input/debug event messages.
     *
     * @return Reference to a static deque containing recent event strings.
     */
    std::deque<std::string>& dbgQ() { static std::deque<std::string> q; return q; }

    /**
     * @brief Pushes a debug message into the queue and writes it out.
     *
     * Adds the message to the recent-events deque (keeping only the last 128 entries),
     * echoes to console when `_DEBUG` is defined, and appends the line to the crash log.
     *
     * @param s The message string to log.
     */
    void dbg_push_line(const std::string& s) {
        auto& q = dbgQ();
        q.push_back(s);
        while (q.size() > 128) q.pop_front(); // limit buffer size

#ifdef _DEBUG
        std::cerr << s << std::endl; // print to console in debug builds
#endif
        CrashLog::WriteLine(s.c_str()); // write to crash log
    }

    /**
     * @brief Returns a human-readable string for a GLFW key/mouse action.
     *
     * @param a The GLFW action constant (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
     * @return Corresponding action name as a string literal.
     */
    const char* action_name(int a) {
        switch (a) {
        case GLFW_PRESS:   return "PRESS";
        case GLFW_RELEASE: return "RELEASE";
        case GLFW_REPEAT:  return "REPEAT";
        default:           return "UNKNOWN";
        }
    }
} // anonymous namespace


namespace DebugInput {

    /**
     * @brief Enable or disable input debugging.
     *
     * @param on True to enable, false to disable.
     */
    void SetEnabled(bool on) { dbgInputOn() = on; }

    /**
     * @brief Query if input debugging is enabled.
     *
     * @return True if enabled, false if disabled.
     */
    bool Enabled() { return dbgInputOn(); }

    /**
     * @brief Get a copy of the most recent input events.
     *
     * @param max Maximum number of recent messages to return.
     * @return A deque of recent input/debug messages (up to @p max entries).
     */
    std::deque<std::string> Recent(size_t max) {
        auto& q = dbgQ();
        std::deque<std::string> out;
        const size_t n = std::min(max, q.size());
        std::copy(q.end() - static_cast<long>(n), q.end(), std::back_inserter(out));
        return out;
    }

    /**
     * @brief Log a keyboard event.
     *
     * Generates a formatted string including the key, scancode, action (press/release/repeat),
     * and active modifier keys. This is called from a GLFW key callback.
     *
     * @param window The GLFW window receiving the event.
     * @param key The key that was pressed, repeated, or released.
     * @param scancode Platform-specific scancode.
     * @param action The key action (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
     * @param mods Modifier flags (e.g., shift, ctrl, alt).
     */
    void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (!dbgInputOn()) return;
        (void)window;

        const char* print = glfwGetKeyName(key, scancode); // may be null
        std::ostringstream oss;
        oss << "Key "
            << (print ? print : "<non-printable>")
            << " (" << key << ") "
            << action_name(action)
            << " mods=" << mods;
        dbg_push_line(oss.str());
    }

    /**
     * @brief Log a mouse button event.
     *
     * Generates a formatted string with button (left/right), action (press/release),
     * modifier keys, and cursor position at the time of the click.
     *
     * @param window The GLFW window receiving the event.
     * @param button The mouse button (e.g., GLFW_MOUSE_BUTTON_LEFT).
     * @param action The action (GLFW_PRESS or GLFW_RELEASE).
     * @param mods Modifier flags (shift, ctrl, alt).
     */
    void OnMouseButton(GLFWwindow* window, int button, int action, int mods) {
        if (!dbgInputOn()) return;
        (void)window;

        const char* bname = (button == GLFW_MOUSE_BUTTON_LEFT) ? "LMB" :
            (button == GLFW_MOUSE_BUTTON_RIGHT) ? "RMB" : "MB";
        std::ostringstream oss;
        oss << "Mouse " << bname << " " << action_name(action) << " mods=" << mods;

        // Attach cursor coordinates at the time of the click
        double x = 0.0, y = 0.0;
        if (window) glfwGetCursorPos(window, &x, &y);
        oss << " @(" << std::fixed << std::setprecision(1) << x << "," << y << ")";
        dbg_push_line(oss.str());
    }

    /**
     * @brief Log mouse cursor movement.
     *
     * Reports the cursor’s current x and y coordinates within the window client area.
     *
     * @param window The GLFW window receiving the event.
     * @param xpos Cursor x-coordinate.
     * @param ypos Cursor y-coordinate.
     */
    void OnCursorPos(GLFWwindow* window, double xpos, double ypos) {
        if (!dbgInputOn()) return;
        (void)window;

        std::ostringstream oss;
        oss << "Cursor (" << std::fixed << std::setprecision(1)
            << xpos << ", " << ypos << ")";
        dbg_push_line(oss.str());
    }

    /**
     * @brief Log a mouse scroll (wheel/gesture) event.
     *
     * Reports scroll offsets along the x and y axes.
     *
     * @param window The GLFW window receiving the event.
     * @param xoffset Scroll offset in x-direction.
     * @param yoffset Scroll offset in y-direction.
     */
    void OnScroll(GLFWwindow* window, double xoffset, double yoffset) {
        if (!dbgInputOn()) return;
        (void)window;

        std::ostringstream oss;
        oss << "Scroll dx=" << xoffset << " dy=" << yoffset;
        dbg_push_line(oss.str());
    }
}

/*!
\brief Audits and prints the engine's runtime directory structure.

This function checks the existence of important directories used by the
WaterBound engine at runtime. It prints out the resolved path for each
directory and indicates whether the directory exists. It can be used to
diagnose missing folders such as shaders, scenes, scripting, or resources.

The function also optionally lists a few files (up to 5) from each directory
to help verify that expected contents are present. This is useful during
debugging when the game cannot locate required assets.
*/
namespace DebugDiag {

    void AuditRuntimePaths()
    {
        using std::cout; using std::cerr; using std::endl;
        namespace fs = std::filesystem;

        auto exe = Paths::exe_dir();
        auto cfg = Paths::config();
        auto sh = Paths::shaders();
        auto scn = Paths::scenes();
        auto scr = exe / "scripting";
        auto res = exe / "resources";

        cout << "[Audit] exe_dir: " << exe.string() << endl;
        cout << "[Audit] config path: " << cfg.string() << (fs::exists(cfg) ? "  (OK)" : "  (MISSING)") << endl;
        cout << "[Audit] shaders dir: " << sh.string() << (fs::exists(sh) ? "  (OK)" : "  (MISSING)") << endl;
        cout << "[Audit] scenes dir: " << scn.string() << (fs::exists(scn) ? "  (OK)" : "  (MISSING)") << endl;
        cout << "[Audit] scripting dir: " << scr.string() << (fs::exists(scr) ? "  (OK)" : "  (MISSING)") << endl;
        cout << "[Audit] resources dir: " << res.string() << (fs::exists(res) ? "  (OK)" : "  (MISSING)") << endl;

        // Optional: list a few files from each folder to confirm contents
        auto listFew = [](const fs::path& dir, const char* label) {
            if (!fs::exists(dir)) return;
            int shown = 0;
            for (auto& e : fs::directory_iterator(dir)) {
                if (shown++ >= 5) break;
                std::cout << "  - [" << label << "] " << e.path().filename().string() << "\n";
            }
            };
        listFew(scn, "scene");
        listFew(scr, "lua");
        listFew(res, "res");
    }
}
