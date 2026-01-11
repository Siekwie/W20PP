#pragma once
/**
 * W20PP: Modern C++20 Windows UI Framework
 * 
 * A lightweight, header-only framework for Windows UI development.
 * - Pure RAII design (no macros, no exceptions)
 * - Declarative UI syntax using C++20 designated initializers
 * - Zero external dependencies
 * - Built-in dark mode, DPI awareness, and accessibility
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <dwmapi.h>
#include <shellscalingapi.h>

// DWMWA_USE_IMMERSIVE_DARK_MODE requires Windows 10 2004+ SDK
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#include <concepts>
#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <expected>
#include <span>

namespace w20pp {

// -----------------------------------------------------------------------------
// Version Information
// -----------------------------------------------------------------------------
inline constexpr int version_major = 0;
inline constexpr int version_minor = 1;
inline constexpr int version_patch = 0;

// -----------------------------------------------------------------------------
// Error Handling
// -----------------------------------------------------------------------------
enum class ErrorCode {
    Success = 0,
    WindowCreationFailed,
    ClassRegistrationFailed,
    InvalidHandle,
    SystemError
};

struct Error {
    ErrorCode code;
    DWORD win32_error;
    std::wstring message;
};

template<typename T>
using Result = std::expected<T, Error>;

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------
class Window;
class Application;

// -----------------------------------------------------------------------------
// Geometry Types
// -----------------------------------------------------------------------------
struct Point {
    int x = 0;
    int y = 0;
};

struct Size {
    int width = 0;
    int height = 0;
};

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// -----------------------------------------------------------------------------
// Theme Support
// -----------------------------------------------------------------------------
enum class Theme {
    System,
    Light,
    Dark
};

[[nodiscard]] inline bool is_dark_mode_enabled() noexcept {
    HKEY key;
    if (RegOpenKeyExW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            0, KEY_READ, &key) == ERROR_SUCCESS) {
        DWORD value = 1;
        DWORD size = sizeof(value);
        RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, nullptr,
                        reinterpret_cast<LPBYTE>(&value), &size);
        RegCloseKey(key);
        return value == 0;
    }
    return false;
}

// -----------------------------------------------------------------------------
// DPI Awareness
// -----------------------------------------------------------------------------
inline void enable_dpi_awareness() noexcept {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

[[nodiscard]] inline int get_dpi_for_window(HWND hwnd) noexcept {
    return static_cast<int>(GetDpiForWindow(hwnd));
}

[[nodiscard]] inline float get_dpi_scale(HWND hwnd) noexcept {
    return static_cast<float>(get_dpi_for_window(hwnd)) / 96.0f;
}

// -----------------------------------------------------------------------------
// Window Configuration
// -----------------------------------------------------------------------------
struct WindowConfig {
    std::wstring_view title = L"W20PP Window";
    Size size = {800, 600};
    std::optional<Point> position = std::nullopt;
    Theme theme = Theme::System;
    bool resizable = true;
    bool maximizable = true;
    bool minimizable = true;
    bool show_in_taskbar = true;
};

// -----------------------------------------------------------------------------
// Window Class (RAII wrapper for HWND)
// -----------------------------------------------------------------------------
class Window {
public:
    Window() noexcept = default;
    
    explicit Window(HWND handle) noexcept : handle_(handle) {}
    
    ~Window() {
        if (handle_ && owns_handle_) {
            DestroyWindow(handle_);
        }
    }

    // Move-only semantics
    Window(Window&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr))
        , owns_handle_(std::exchange(other.owns_handle_, false)) {}
    
    Window& operator=(Window&& other) noexcept {
        if (this != &other) {
            if (handle_ && owns_handle_) {
                DestroyWindow(handle_);
            }
            handle_ = std::exchange(other.handle_, nullptr);
            owns_handle_ = std::exchange(other.owns_handle_, false);
        }
        return *this;
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] HWND handle() const noexcept { return handle_; }
    [[nodiscard]] bool valid() const noexcept { return handle_ != nullptr; }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }

    void show() const noexcept { ShowWindow(handle_, SW_SHOW); }
    void hide() const noexcept { ShowWindow(handle_, SW_HIDE); }
    void maximize() const noexcept { ShowWindow(handle_, SW_MAXIMIZE); }
    void minimize() const noexcept { ShowWindow(handle_, SW_MINIMIZE); }
    void restore() const noexcept { ShowWindow(handle_, SW_RESTORE); }

    [[nodiscard]] Rect get_client_rect() const noexcept {
        RECT rc;
        GetClientRect(handle_, &rc);
        return {rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top};
    }

    void set_title(std::wstring_view title) const noexcept {
        SetWindowTextW(handle_, title.data());
    }

    void apply_dark_mode(bool enable) const noexcept {
        BOOL value = enable ? TRUE : FALSE;
        DwmSetWindowAttribute(handle_, DWMWA_USE_IMMERSIVE_DARK_MODE, 
                             &value, sizeof(value));
    }

private:
    HWND handle_ = nullptr;
    bool owns_handle_ = true;
};

// -----------------------------------------------------------------------------
// Application Class
// -----------------------------------------------------------------------------
class Application {
public:
    Application() {
        enable_dpi_awareness();
        instance_ = GetModuleHandleW(nullptr);
    }

    [[nodiscard]] Result<Window> create_window(const WindowConfig& config) {
        static const wchar_t* class_name = L"W20PP_Window";
        static bool registered = false;

        if (!registered) {
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(wc);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = window_proc;
            wc.hInstance = instance_;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            wc.lpszClassName = class_name;

            if (!RegisterClassExW(&wc)) {
                return std::unexpected(Error{
                    ErrorCode::ClassRegistrationFailed,
                    GetLastError(),
                    L"Failed to register window class"
                });
            }
            registered = true;
        }

        DWORD style = WS_OVERLAPPEDWINDOW;
        if (!config.resizable) style &= ~WS_THICKFRAME;
        if (!config.maximizable) style &= ~WS_MAXIMIZEBOX;
        if (!config.minimizable) style &= ~WS_MINIMIZEBOX;

        int x = config.position.has_value() ? config.position->x : CW_USEDEFAULT;
        int y = config.position.has_value() ? config.position->y : CW_USEDEFAULT;

        HWND hwnd = CreateWindowExW(
            config.show_in_taskbar ? 0 : WS_EX_TOOLWINDOW,
            class_name,
            config.title.data(),
            style,
            x, y,
            config.size.width, config.size.height,
            nullptr, nullptr, instance_, nullptr
        );

        if (!hwnd) {
            return std::unexpected(Error{
                ErrorCode::WindowCreationFailed,
                GetLastError(),
                L"Failed to create window"
            });
        }

        Window window(hwnd);

        // Apply theme
        bool use_dark = (config.theme == Theme::Dark) ||
                       (config.theme == Theme::System && is_dark_mode_enabled());
        window.apply_dark_mode(use_dark);

        return window;
    }

    int run() {
        MSG msg = {};
        while (GetMessageW(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

    void quit(int exit_code = 0) noexcept {
        PostQuitMessage(exit_code);
    }

private:
    HINSTANCE instance_ = nullptr;

    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, 
                                        WPARAM wparam, LPARAM lparam) {
        switch (msg) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_DPICHANGED: {
                auto* rect = reinterpret_cast<RECT*>(lparam);
                SetWindowPos(hwnd, nullptr,
                    rect->left, rect->top,
                    rect->right - rect->left,
                    rect->bottom - rect->top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
                return 0;
            }
        }
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
};

} // namespace w20pp
