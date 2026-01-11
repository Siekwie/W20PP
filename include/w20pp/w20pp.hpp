#pragma once
/**
 * W20PP: Modern C++20 Windows UI Framework
 * 
 * A lightweight, header-only framework for Windows UI development.
 * - Pure RAII design (no macros, no exceptions)
 * - Declarative UI syntax using C++20 designated initializers
 * - Zero external dependencies
 * - Built-in dark mode, DPI awareness, and accessibility
 * 
 * @version 0.2.0
 * @phase Phase 2: Window Management & Lifecycle
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <dwmapi.h>
#include <shellscalingapi.h>
#include <windowsx.h>

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
#include <unordered_map>
#include <vector>
#include <variant>
#include <any>
#include <format>
#include <mutex>
#include <array>

namespace w20pp {

// -----------------------------------------------------------------------------
// Version Information
// -----------------------------------------------------------------------------
inline constexpr int version_major = 0;
inline constexpr int version_minor = 2;
inline constexpr int version_patch = 0;

// -----------------------------------------------------------------------------
// Error Handling
// -----------------------------------------------------------------------------
enum class ErrorCode {
    Success = 0,
    WindowCreationFailed,
    ClassRegistrationFailed,
    InvalidHandle,
    SystemError,
    IconLoadFailed,
    InvalidParameter
};

struct Error {
    ErrorCode code;
    DWORD win32_error;
    std::wstring message;
    
    [[nodiscard]] std::wstring format() const {
        return std::format(L"Error [{}]: {} (Win32: {})", 
            static_cast<int>(code), message, win32_error);
    }
};

template<typename T>
using Result = std::expected<T, Error>;

inline Result<void> make_success() {
    return {};
}

inline std::unexpected<Error> make_error(ErrorCode code, std::wstring_view msg) {
    return std::unexpected(Error{code, GetLastError(), std::wstring(msg)});
}

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------
class Window;
class Application;
class WindowClass;

// -----------------------------------------------------------------------------
// Geometry Types
// -----------------------------------------------------------------------------
struct Point {
    int x = 0;
    int y = 0;
    
    [[nodiscard]] POINT to_native() const noexcept { return {x, y}; }
    [[nodiscard]] static Point from_native(const POINT& pt) noexcept { 
        return {pt.x, pt.y}; 
    }
    [[nodiscard]] static Point from_lparam(LPARAM lp) noexcept {
        return {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
    }
    
    [[nodiscard]] bool operator==(const Point& other) const noexcept = default;
};

struct Size {
    int width = 0;
    int height = 0;
    
    [[nodiscard]] SIZE to_native() const noexcept { 
        return {static_cast<LONG>(width), static_cast<LONG>(height)}; 
    }
    [[nodiscard]] static Size from_native(const SIZE& sz) noexcept { 
        return {sz.cx, sz.cy}; 
    }
    [[nodiscard]] static Size from_lparam(LPARAM lp) noexcept {
        return {LOWORD(lp), HIWORD(lp)};
    }
    
    [[nodiscard]] bool operator==(const Size& other) const noexcept = default;
};

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    
    [[nodiscard]] int left() const noexcept { return x; }
    [[nodiscard]] int top() const noexcept { return y; }
    [[nodiscard]] int right() const noexcept { return x + width; }
    [[nodiscard]] int bottom() const noexcept { return y + height; }
    [[nodiscard]] Point position() const noexcept { return {x, y}; }
    [[nodiscard]] Size size() const noexcept { return {width, height}; }
    [[nodiscard]] Point center() const noexcept { 
        return {x + width / 2, y + height / 2}; 
    }
    
    [[nodiscard]] RECT to_native() const noexcept { 
        return {x, y, x + width, y + height}; 
    }
    [[nodiscard]] static Rect from_native(const RECT& rc) noexcept { 
        return {rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top}; 
    }
    
    [[nodiscard]] bool contains(const Point& pt) const noexcept {
        return pt.x >= x && pt.x < x + width && pt.y >= y && pt.y < y + height;
    }
    
    [[nodiscard]] bool operator==(const Rect& other) const noexcept = default;
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

[[nodiscard]] inline int scale_for_dpi(int value, HWND hwnd) noexcept {
    return static_cast<int>(value * get_dpi_scale(hwnd));
}

// -----------------------------------------------------------------------------
// Monitor Information
// -----------------------------------------------------------------------------
struct MonitorInfo {
    Rect bounds;
    Rect work_area;
    bool is_primary = false;
    int dpi = 96;
    std::wstring device_name;
};

[[nodiscard]] inline std::vector<MonitorInfo> get_all_monitors() {
    std::vector<MonitorInfo> monitors;
    
    EnumDisplayMonitors(nullptr, nullptr, 
        [](HMONITOR hmon, HDC, LPRECT, LPARAM lparam) -> BOOL {
            auto* list = reinterpret_cast<std::vector<MonitorInfo>*>(lparam);
            
            MONITORINFOEXW mi = {};
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfoW(hmon, &mi)) {
                MonitorInfo info;
                info.bounds = Rect::from_native(mi.rcMonitor);
                info.work_area = Rect::from_native(mi.rcWork);
                info.is_primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
                info.device_name = mi.szDevice;
                
                UINT dpiX = 96, dpiY = 96;
                GetDpiForMonitor(hmon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
                info.dpi = static_cast<int>(dpiX);
                
                list->push_back(info);
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&monitors));
    
    return monitors;
}

[[nodiscard]] inline std::optional<MonitorInfo> get_primary_monitor() {
    auto monitors = get_all_monitors();
    for (const auto& mon : monitors) {
        if (mon.is_primary) return mon;
    }
    return std::nullopt;
}

[[nodiscard]] inline std::optional<MonitorInfo> get_monitor_from_point(const Point& pt) {
    POINT native_pt = pt.to_native();
    HMONITOR hmon = MonitorFromPoint(native_pt, MONITOR_DEFAULTTONEAREST);
    if (!hmon) return std::nullopt;
    
    MONITORINFOEXW mi = {};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(hmon, &mi)) return std::nullopt;
    
    MonitorInfo info;
    info.bounds = Rect::from_native(mi.rcMonitor);
    info.work_area = Rect::from_native(mi.rcWork);
    info.is_primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
    info.device_name = mi.szDevice;
    
    UINT dpiX = 96, dpiY = 96;
    GetDpiForMonitor(hmon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    info.dpi = static_cast<int>(dpiX);
    
    return info;
}

// -----------------------------------------------------------------------------
// Window State
// -----------------------------------------------------------------------------
enum class WindowState {
    Normal,
    Minimized,
    Maximized,
    Hidden
};

// -----------------------------------------------------------------------------
// Event System - Type-Safe Event Data
// -----------------------------------------------------------------------------

// Base event structure
struct Event {
    HWND hwnd = nullptr;
    UINT message = 0;
    WPARAM wparam = 0;
    LPARAM lparam = 0;
    bool handled = false;
};

// Specific event types with typed parameters
struct CloseEvent : Event {
    bool cancel = false;
};

struct SizeEvent : Event {
    Size new_size;
    enum class Type { Restored, Minimized, Maximized, MaxShow, MaxHide } type;
};

struct MoveEvent : Event {
    Point new_position;
};

struct DpiChangedEvent : Event {
    int new_dpi;
    Rect suggested_rect;
};

struct FocusEvent : Event {
    HWND other_window;
    bool gained; // true = gained focus, false = lost focus
};

struct CreateEvent : Event {
    CREATESTRUCTW* create_struct;
};

struct DestroyEvent : Event {};

struct PaintEvent : Event {
    HDC hdc;
    Rect paint_rect;
};

struct MouseEvent : Event {
    Point position;      // Client coordinates
    Point screen_pos;    // Screen coordinates
    bool left_button;
    bool right_button;
    bool middle_button;
    bool ctrl_key;
    bool shift_key;
};

struct MouseWheelEvent : MouseEvent {
    int delta;           // Positive = scroll up, negative = scroll down
    bool horizontal;     // True if horizontal scroll
};

struct KeyEvent : Event {
    int virtual_key;
    int scan_code;
    int repeat_count;
    bool extended_key;
    bool alt_down;
    bool was_down;       // Key was already down before this message
    bool is_released;    // Key is being released (WM_KEYUP)
};

struct CharEvent : Event {
    wchar_t character;
    int repeat_count;
};

struct CommandEvent : Event {
    WORD id;
    WORD code;
    HWND control;
};

struct TimerEvent : Event {
    UINT_PTR timer_id;
};

// -----------------------------------------------------------------------------
// Event Handler Types
// -----------------------------------------------------------------------------
using EventHandler = std::function<void(Event&)>;
using CloseHandler = std::function<void(CloseEvent&)>;
using SizeHandler = std::function<void(SizeEvent&)>;
using MoveHandler = std::function<void(MoveEvent&)>;
using DpiHandler = std::function<void(DpiChangedEvent&)>;
using FocusHandler = std::function<void(FocusEvent&)>;
using CreateHandler = std::function<void(CreateEvent&)>;
using DestroyHandler = std::function<void(DestroyEvent&)>;
using PaintHandler = std::function<void(PaintEvent&)>;
using MouseHandler = std::function<void(MouseEvent&)>;
using MouseWheelHandler = std::function<void(MouseWheelEvent&)>;
using KeyHandler = std::function<void(KeyEvent&)>;
using CharHandler = std::function<void(CharEvent&)>;
using CommandHandler = std::function<void(CommandEvent&)>;
using TimerHandler = std::function<void(TimerEvent&)>;

// Generic message handler for custom messages
using MessageHandler = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM, bool&)>;

// -----------------------------------------------------------------------------
// Icon Handle RAII Wrapper
// -----------------------------------------------------------------------------
class Icon {
public:
    Icon() noexcept = default;
    
    explicit Icon(HICON handle, bool owned = true) noexcept 
        : handle_(handle), owned_(owned) {}
    
    ~Icon() {
        if (handle_ && owned_) {
            DestroyIcon(handle_);
        }
    }
    
    Icon(Icon&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr))
        , owned_(std::exchange(other.owned_, false)) {}
    
    Icon& operator=(Icon&& other) noexcept {
        if (this != &other) {
            if (handle_ && owned_) DestroyIcon(handle_);
            handle_ = std::exchange(other.handle_, nullptr);
            owned_ = std::exchange(other.owned_, false);
        }
        return *this;
    }
    
    Icon(const Icon&) = delete;
    Icon& operator=(const Icon&) = delete;
    
    [[nodiscard]] HICON handle() const noexcept { return handle_; }
    [[nodiscard]] bool valid() const noexcept { return handle_ != nullptr; }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    
    // Load icon from resource ID
    [[nodiscard]] static Result<Icon> from_resource(HINSTANCE instance, int resource_id) {
        HICON h = LoadIconW(instance, MAKEINTRESOURCEW(resource_id));
        if (!h) return make_error(ErrorCode::IconLoadFailed, L"Failed to load icon from resource");
        return Icon(h, false); // Resource icons should not be destroyed
    }
    
    // Load icon from file
    [[nodiscard]] static Result<Icon> from_file(std::wstring_view path, Size size = {0, 0}) {
        HICON h = static_cast<HICON>(LoadImageW(
            nullptr, path.data(), IMAGE_ICON,
            size.width, size.height,
            LR_LOADFROMFILE | (size.width == 0 ? LR_DEFAULTSIZE : 0)
        ));
        if (!h) return make_error(ErrorCode::IconLoadFailed, L"Failed to load icon from file");
        return Icon(h, true);
    }
    
    // Load system icon
    [[nodiscard]] static Icon system_icon(LPCWSTR icon_id) noexcept {
        return Icon(LoadIconW(nullptr, icon_id), false);
    }
    
private:
    HICON handle_ = nullptr;
    bool owned_ = true;
};

// -----------------------------------------------------------------------------
// Window Style Helpers
// -----------------------------------------------------------------------------
struct WindowStyles {
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_style = 0;
    
    // Builder pattern for common style configurations
    WindowStyles& overlapped() { style = WS_OVERLAPPEDWINDOW; return *this; }
    WindowStyles& popup() { style = WS_POPUP | WS_BORDER; return *this; }
    WindowStyles& child() { style = WS_CHILD | WS_VISIBLE; return *this; }
    
    WindowStyles& no_resize() { style &= ~WS_THICKFRAME; return *this; }
    WindowStyles& no_maximize() { style &= ~WS_MAXIMIZEBOX; return *this; }
    WindowStyles& no_minimize() { style &= ~WS_MINIMIZEBOX; return *this; }
    WindowStyles& no_caption() { style &= ~WS_CAPTION; return *this; }
    WindowStyles& no_border() { style &= ~WS_BORDER; return *this; }
    
    WindowStyles& visible() { style |= WS_VISIBLE; return *this; }
    WindowStyles& disabled() { style |= WS_DISABLED; return *this; }
    WindowStyles& clip_children() { style |= WS_CLIPCHILDREN; return *this; }
    WindowStyles& clip_siblings() { style |= WS_CLIPSIBLINGS; return *this; }
    
    // Extended styles
    WindowStyles& topmost() { ex_style |= WS_EX_TOPMOST; return *this; }
    WindowStyles& tool_window() { ex_style |= WS_EX_TOOLWINDOW; return *this; }
    WindowStyles& transparent_click() { ex_style |= WS_EX_TRANSPARENT; return *this; }
    WindowStyles& layered() { ex_style |= WS_EX_LAYERED; return *this; }
    WindowStyles& no_activate() { ex_style |= WS_EX_NOACTIVATE; return *this; }
    WindowStyles& composited() { ex_style |= WS_EX_COMPOSITED; return *this; }
    WindowStyles& app_window() { ex_style |= WS_EX_APPWINDOW; return *this; }
    WindowStyles& right_to_left() { ex_style |= WS_EX_RTLREADING | WS_EX_LAYOUTRTL; return *this; }
};

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
    bool start_visible = true;
    WindowState initial_state = WindowState::Normal;
    std::optional<WindowStyles> custom_styles = std::nullopt;
    
    // Advanced options
    HWND parent = nullptr;
    HICON icon = nullptr;
    HICON icon_small = nullptr;
    BYTE opacity = 255;          // 0-255, used with WS_EX_LAYERED
    bool always_on_top = false;
    Size min_size = {0, 0};      // Minimum window size constraint
    Size max_size = {0, 0};      // Maximum window size constraint (0 = no limit)
};

// -----------------------------------------------------------------------------
// Window Class Configuration
// -----------------------------------------------------------------------------
struct WindowClassConfig {
    std::wstring class_name;
    HICON icon = nullptr;
    HICON icon_small = nullptr;
    HCURSOR cursor = nullptr;
    HBRUSH background = nullptr;
    UINT style = CS_HREDRAW | CS_VREDRAW;
    int extra_class_bytes = 0;
    int extra_window_bytes = 0;
};

// -----------------------------------------------------------------------------
// Event Handler Storage (per window)
// -----------------------------------------------------------------------------
class EventHandlers {
public:
    CloseHandler on_close;
    SizeHandler on_size;
    MoveHandler on_move;
    DpiHandler on_dpi_changed;
    FocusHandler on_focus;
    CreateHandler on_create;
    DestroyHandler on_destroy;
    PaintHandler on_paint;
    
    // Mouse events
    MouseHandler on_mouse_move;
    MouseHandler on_mouse_down;
    MouseHandler on_mouse_up;
    MouseHandler on_mouse_double_click;
    MouseWheelHandler on_mouse_wheel;
    MouseHandler on_mouse_enter;
    MouseHandler on_mouse_leave;
    
    // Keyboard events
    KeyHandler on_key_down;
    KeyHandler on_key_up;
    CharHandler on_char;
    
    // Other events
    CommandHandler on_command;
    TimerHandler on_timer;
    
    // Custom message handlers
    std::unordered_map<UINT, MessageHandler> custom_handlers;
    
    // General pre/post message hooks
    MessageHandler pre_message;
    MessageHandler post_message;
    
    // Size constraints
    Size min_size = {0, 0};
    Size max_size = {0, 0};
    
    // Mouse tracking state
    bool mouse_tracked = false;
};

// -----------------------------------------------------------------------------
// Window Class (RAII wrapper for HWND)
// -----------------------------------------------------------------------------
class Window {
public:
    Window() noexcept = default;
    
    explicit Window(HWND handle) noexcept : handle_(handle) {
        // Store this pointer in window's user data for callback routing
        if (handle_) {
            SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        }
    }
    
    ~Window() {
        if (handle_ && owns_handle_) {
            SetWindowLongPtrW(handle_, GWLP_USERDATA, 0);
            DestroyWindow(handle_);
        }
    }

    // Move-only semantics
    Window(Window&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr))
        , owns_handle_(std::exchange(other.owns_handle_, false))
        , handlers_(std::move(other.handlers_)) {
        if (handle_) {
            SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        }
    }
    
    Window& operator=(Window&& other) noexcept {
        if (this != &other) {
            if (handle_ && owns_handle_) {
                SetWindowLongPtrW(handle_, GWLP_USERDATA, 0);
                DestroyWindow(handle_);
            }
            handle_ = std::exchange(other.handle_, nullptr);
            owns_handle_ = std::exchange(other.owns_handle_, false);
            handlers_ = std::move(other.handlers_);
            if (handle_) {
                SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            }
        }
        return *this;
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Handle access
    [[nodiscard]] HWND handle() const noexcept { return handle_; }
    [[nodiscard]] bool valid() const noexcept { return handle_ != nullptr && IsWindow(handle_); }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    
    // Static lookup for window from HWND
    [[nodiscard]] static Window* from_hwnd(HWND hwnd) noexcept {
        if (!hwnd) return nullptr;
        return reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    // ----- Window State -----
    void show() const noexcept { ShowWindow(handle_, SW_SHOW); }
    void hide() const noexcept { ShowWindow(handle_, SW_HIDE); }
    void maximize() const noexcept { ShowWindow(handle_, SW_MAXIMIZE); }
    void minimize() const noexcept { ShowWindow(handle_, SW_MINIMIZE); }
    void restore() const noexcept { ShowWindow(handle_, SW_RESTORE); }
    void activate() const noexcept { SetForegroundWindow(handle_); }
    void focus() const noexcept { SetFocus(handle_); }
    void close() const noexcept { PostMessageW(handle_, WM_CLOSE, 0, 0); }
    
    [[nodiscard]] WindowState get_state() const noexcept {
        if (!IsWindowVisible(handle_)) return WindowState::Hidden;
        if (IsIconic(handle_)) return WindowState::Minimized;
        if (IsZoomed(handle_)) return WindowState::Maximized;
        return WindowState::Normal;
    }
    
    [[nodiscard]] bool is_visible() const noexcept { return IsWindowVisible(handle_) != 0; }
    [[nodiscard]] bool is_minimized() const noexcept { return IsIconic(handle_) != 0; }
    [[nodiscard]] bool is_maximized() const noexcept { return IsZoomed(handle_) != 0; }
    [[nodiscard]] bool has_focus() const noexcept { return GetFocus() == handle_; }
    [[nodiscard]] bool is_active() const noexcept { return GetForegroundWindow() == handle_; }
    [[nodiscard]] bool is_enabled() const noexcept { return IsWindowEnabled(handle_) != 0; }
    
    void enable(bool enabled = true) const noexcept { EnableWindow(handle_, enabled ? TRUE : FALSE); }
    void disable() const noexcept { enable(false); }

    // ----- Geometry -----
    [[nodiscard]] Rect get_client_rect() const noexcept {
        RECT rc;
        GetClientRect(handle_, &rc);
        return Rect::from_native(rc);
    }
    
    [[nodiscard]] Rect get_window_rect() const noexcept {
        RECT rc;
        GetWindowRect(handle_, &rc);
        return Rect::from_native(rc);
    }
    
    [[nodiscard]] Point get_position() const noexcept {
        RECT rc;
        GetWindowRect(handle_, &rc);
        return {rc.left, rc.top};
    }
    
    [[nodiscard]] Size get_size() const noexcept {
        RECT rc;
        GetWindowRect(handle_, &rc);
        return {rc.right - rc.left, rc.bottom - rc.top};
    }
    
    void set_position(const Point& pos) const noexcept {
        SetWindowPos(handle_, nullptr, pos.x, pos.y, 0, 0, 
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    void set_size(const Size& size) const noexcept {
        SetWindowPos(handle_, nullptr, 0, 0, size.width, size.height, 
                    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    void set_bounds(const Rect& rect) const noexcept {
        SetWindowPos(handle_, nullptr, rect.x, rect.y, rect.width, rect.height, 
                    SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    void center_on_screen() const noexcept {
        if (auto mon = get_monitor_from_point(get_position())) {
            auto work = mon->work_area;
            auto size = get_size();
            set_position({
                work.x + (work.width - size.width) / 2,
                work.y + (work.height - size.height) / 2
            });
        }
    }
    
    void center_on_parent() const noexcept {
        HWND parent = GetParent(handle_);
        if (!parent) {
            center_on_screen();
            return;
        }
        RECT parent_rect;
        GetWindowRect(parent, &parent_rect);
        auto parent_bounds = Rect::from_native(parent_rect);
        auto size = get_size();
        set_position({
            parent_bounds.x + (parent_bounds.width - size.width) / 2,
            parent_bounds.y + (parent_bounds.height - size.height) / 2
        });
    }

    // ----- Title -----
    void set_title(std::wstring_view title) const noexcept {
        SetWindowTextW(handle_, title.data());
    }
    
    [[nodiscard]] std::wstring get_title() const {
        int len = GetWindowTextLengthW(handle_);
        if (len == 0) return {};
        std::wstring title(len + 1, L'\0');
        GetWindowTextW(handle_, title.data(), len + 1);
        title.resize(len);
        return title;
    }

    // ----- Theme & Appearance -----
    void apply_dark_mode(bool enable) const noexcept {
        BOOL value = enable ? TRUE : FALSE;
        DwmSetWindowAttribute(handle_, DWMWA_USE_IMMERSIVE_DARK_MODE, 
                             &value, sizeof(value));
    }
    
    void set_opacity(BYTE alpha) const noexcept {
        // Ensure layered style
        LONG_PTR ex_style = GetWindowLongPtrW(handle_, GWL_EXSTYLE);
        if (!(ex_style & WS_EX_LAYERED)) {
            SetWindowLongPtrW(handle_, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);
        }
        SetLayeredWindowAttributes(handle_, 0, alpha, LWA_ALPHA);
    }
    
    [[nodiscard]] BYTE get_opacity() const noexcept {
        BYTE alpha = 255;
        DWORD flags = 0;
        GetLayeredWindowAttributes(handle_, nullptr, &alpha, &flags);
        return alpha;
    }
    
    void set_transparent_color(COLORREF color) const noexcept {
        LONG_PTR ex_style = GetWindowLongPtrW(handle_, GWL_EXSTYLE);
        if (!(ex_style & WS_EX_LAYERED)) {
            SetWindowLongPtrW(handle_, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);
        }
        SetLayeredWindowAttributes(handle_, color, 0, LWA_COLORKEY);
    }

    // ----- Window Layering -----
    void set_topmost(bool topmost) const noexcept {
        SetWindowPos(handle_, topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                    0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    
    [[nodiscard]] bool is_topmost() const noexcept {
        return (GetWindowLongPtrW(handle_, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
    }
    
    void bring_to_top() const noexcept {
        BringWindowToTop(handle_);
    }

    // ----- Icons -----
    void set_icon(HICON icon) const noexcept {
        SendMessageW(handle_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
    }
    
    void set_icon_small(HICON icon) const noexcept {
        SendMessageW(handle_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
    }
    
    void set_icons(HICON big_icon, HICON small_icon) const noexcept {
        set_icon(big_icon);
        set_icon_small(small_icon);
    }

    // ----- DPI -----
    [[nodiscard]] int get_dpi() const noexcept {
        return get_dpi_for_window(handle_);
    }
    
    [[nodiscard]] float get_dpi_scale() const noexcept {
        return w20pp::get_dpi_scale(handle_);
    }
    
    [[nodiscard]] int scale_value(int value) const noexcept {
        return scale_for_dpi(value, handle_);
    }

    // ----- Monitor -----
    [[nodiscard]] std::optional<MonitorInfo> get_monitor() const noexcept {
        return get_monitor_from_point(get_position());
    }

    // ----- Size Constraints -----
    void set_min_size(const Size& size) noexcept {
        handlers_.min_size = size;
    }
    
    void set_max_size(const Size& size) noexcept {
        handlers_.max_size = size;
    }
    
    [[nodiscard]] Size get_min_size() const noexcept {
        return handlers_.min_size;
    }
    
    [[nodiscard]] Size get_max_size() const noexcept {
        return handlers_.max_size;
    }

    // ----- Styles -----
    [[nodiscard]] DWORD get_style() const noexcept {
        return static_cast<DWORD>(GetWindowLongPtrW(handle_, GWL_STYLE));
    }
    
    [[nodiscard]] DWORD get_ex_style() const noexcept {
        return static_cast<DWORD>(GetWindowLongPtrW(handle_, GWL_EXSTYLE));
    }
    
    void set_style(DWORD style) const noexcept {
        SetWindowLongPtrW(handle_, GWL_STYLE, style);
        // Refresh the frame
        SetWindowPos(handle_, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    
    void set_ex_style(DWORD ex_style) const noexcept {
        SetWindowLongPtrW(handle_, GWL_EXSTYLE, ex_style);
        SetWindowPos(handle_, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    
    void add_style(DWORD style) const noexcept {
        set_style(get_style() | style);
    }
    
    void remove_style(DWORD style) const noexcept {
        set_style(get_style() & ~style);
    }
    
    void add_ex_style(DWORD ex_style) const noexcept {
        set_ex_style(get_ex_style() | ex_style);
    }
    
    void remove_ex_style(DWORD ex_style) const noexcept {
        set_ex_style(get_ex_style() & ~ex_style);
    }

    // ----- Event Handler Registration -----
    Window& on_close(CloseHandler handler) { handlers_.on_close = std::move(handler); return *this; }
    Window& on_size(SizeHandler handler) { handlers_.on_size = std::move(handler); return *this; }
    Window& on_move(MoveHandler handler) { handlers_.on_move = std::move(handler); return *this; }
    Window& on_dpi_changed(DpiHandler handler) { handlers_.on_dpi_changed = std::move(handler); return *this; }
    Window& on_focus(FocusHandler handler) { handlers_.on_focus = std::move(handler); return *this; }
    Window& on_create(CreateHandler handler) { handlers_.on_create = std::move(handler); return *this; }
    Window& on_destroy(DestroyHandler handler) { handlers_.on_destroy = std::move(handler); return *this; }
    Window& on_paint(PaintHandler handler) { handlers_.on_paint = std::move(handler); return *this; }
    
    Window& on_mouse_move(MouseHandler handler) { handlers_.on_mouse_move = std::move(handler); return *this; }
    Window& on_mouse_down(MouseHandler handler) { handlers_.on_mouse_down = std::move(handler); return *this; }
    Window& on_mouse_up(MouseHandler handler) { handlers_.on_mouse_up = std::move(handler); return *this; }
    Window& on_mouse_double_click(MouseHandler handler) { handlers_.on_mouse_double_click = std::move(handler); return *this; }
    Window& on_mouse_wheel(MouseWheelHandler handler) { handlers_.on_mouse_wheel = std::move(handler); return *this; }
    Window& on_mouse_enter(MouseHandler handler) { handlers_.on_mouse_enter = std::move(handler); return *this; }
    Window& on_mouse_leave(MouseHandler handler) { handlers_.on_mouse_leave = std::move(handler); return *this; }
    
    Window& on_key_down(KeyHandler handler) { handlers_.on_key_down = std::move(handler); return *this; }
    Window& on_key_up(KeyHandler handler) { handlers_.on_key_up = std::move(handler); return *this; }
    Window& on_char(CharHandler handler) { handlers_.on_char = std::move(handler); return *this; }
    
    Window& on_command(CommandHandler handler) { handlers_.on_command = std::move(handler); return *this; }
    Window& on_timer(TimerHandler handler) { handlers_.on_timer = std::move(handler); return *this; }
    
    Window& on_message(UINT msg, MessageHandler handler) { 
        handlers_.custom_handlers[msg] = std::move(handler); 
        return *this; 
    }
    
    Window& pre_message_hook(MessageHandler handler) {
        handlers_.pre_message = std::move(handler);
        return *this;
    }
    
    Window& post_message_hook(MessageHandler handler) {
        handlers_.post_message = std::move(handler);
        return *this;
    }

    // ----- Timers -----
    [[nodiscard]] UINT_PTR set_timer(UINT_PTR id, UINT interval_ms) const noexcept {
        return SetTimer(handle_, id, interval_ms, nullptr);
    }
    
    void kill_timer(UINT_PTR id) const noexcept {
        KillTimer(handle_, id);
    }

    // ----- Invalidation -----
    void invalidate(bool erase_background = true) const noexcept {
        InvalidateRect(handle_, nullptr, erase_background ? TRUE : FALSE);
    }
    
    void invalidate(const Rect& rect, bool erase_background = true) const noexcept {
        RECT rc = rect.to_native();
        InvalidateRect(handle_, &rc, erase_background ? TRUE : FALSE);
    }
    
    void update() const noexcept {
        UpdateWindow(handle_);
    }
    
    void redraw() const noexcept {
        RedrawWindow(handle_, nullptr, nullptr, 
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    }

    // ----- Message Processing (internal) -----
    LRESULT process_message(UINT msg, WPARAM wparam, LPARAM lparam) {
        bool handled = false;
        
        // Pre-message hook
        if (handlers_.pre_message) {
            LRESULT result = handlers_.pre_message(handle_, msg, wparam, lparam, handled);
            if (handled) return result;
        }
        
        // Check custom handlers first
        if (auto it = handlers_.custom_handlers.find(msg); 
            it != handlers_.custom_handlers.end()) {
            LRESULT result = it->second(handle_, msg, wparam, lparam, handled);
            if (handled) return result;
        }
        
        // Handle built-in events
        LRESULT result = handle_message(msg, wparam, lparam, handled);
        
        // Post-message hook
        if (handlers_.post_message && !handled) {
            bool post_handled = false;
            LRESULT post_result = handlers_.post_message(handle_, msg, wparam, lparam, post_handled);
            if (post_handled) return post_result;
        }
        
        if (handled) return result;
        return DefWindowProcW(handle_, msg, wparam, lparam);
    }

private:
    HWND handle_ = nullptr;
    bool owns_handle_ = true;
    mutable EventHandlers handlers_;
    
    // Helper to create mouse event from message params
    MouseEvent make_mouse_event(UINT msg, WPARAM wparam, LPARAM lparam) const {
        MouseEvent evt;
        evt.hwnd = handle_;
        evt.message = msg;
        evt.wparam = wparam;
        evt.lparam = lparam;
        evt.position = Point::from_lparam(lparam);
        
        POINT screen_pt = evt.position.to_native();
        ClientToScreen(handle_, &screen_pt);
        evt.screen_pos = Point::from_native(screen_pt);
        
        evt.left_button = (wparam & MK_LBUTTON) != 0;
        evt.right_button = (wparam & MK_RBUTTON) != 0;
        evt.middle_button = (wparam & MK_MBUTTON) != 0;
        evt.ctrl_key = (wparam & MK_CONTROL) != 0;
        evt.shift_key = (wparam & MK_SHIFT) != 0;
        
        return evt;
    }
    
    // Helper to create key event from message params
    KeyEvent make_key_event(UINT msg, WPARAM wparam, LPARAM lparam) const {
        KeyEvent evt;
        evt.hwnd = handle_;
        evt.message = msg;
        evt.wparam = wparam;
        evt.lparam = lparam;
        evt.virtual_key = static_cast<int>(wparam);
        evt.scan_code = static_cast<int>((lparam >> 16) & 0xFF);
        evt.repeat_count = static_cast<int>(lparam & 0xFFFF);
        evt.extended_key = (lparam & (1 << 24)) != 0;
        evt.alt_down = (lparam & (1 << 29)) != 0;
        evt.was_down = (lparam & (1 << 30)) != 0;
        evt.is_released = (msg == WM_KEYUP || msg == WM_SYSKEYUP);
        return evt;
    }
    
    LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam, bool& handled) {
        switch (msg) {
            case WM_CLOSE: {
                if (handlers_.on_close) {
                    CloseEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    handlers_.on_close(evt);
                    if (evt.cancel) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_SIZE: {
                if (handlers_.on_size) {
                    SizeEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.new_size = Size::from_lparam(lparam);
                    switch (wparam) {
                        case SIZE_RESTORED: evt.type = SizeEvent::Type::Restored; break;
                        case SIZE_MINIMIZED: evt.type = SizeEvent::Type::Minimized; break;
                        case SIZE_MAXIMIZED: evt.type = SizeEvent::Type::Maximized; break;
                        case SIZE_MAXSHOW: evt.type = SizeEvent::Type::MaxShow; break;
                        case SIZE_MAXHIDE: evt.type = SizeEvent::Type::MaxHide; break;
                    }
                    handlers_.on_size(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_MOVE: {
                if (handlers_.on_move) {
                    MoveEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.new_position = Point::from_lparam(lparam);
                    handlers_.on_move(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_DPICHANGED: {
                auto* rect = reinterpret_cast<RECT*>(lparam);
                SetWindowPos(handle_, nullptr,
                    rect->left, rect->top,
                    rect->right - rect->left,
                    rect->bottom - rect->top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
                
                if (handlers_.on_dpi_changed) {
                    DpiChangedEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.new_dpi = HIWORD(wparam);
                    evt.suggested_rect = Rect::from_native(*rect);
                    handlers_.on_dpi_changed(evt);
                }
                handled = true;
                return 0;
            }
            
            case WM_SETFOCUS:
            case WM_KILLFOCUS: {
                if (handlers_.on_focus) {
                    FocusEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.other_window = reinterpret_cast<HWND>(wparam);
                    evt.gained = (msg == WM_SETFOCUS);
                    handlers_.on_focus(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_PAINT: {
                if (handlers_.on_paint) {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(handle_, &ps);
                    
                    PaintEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.hdc = hdc;
                    evt.paint_rect = Rect::from_native(ps.rcPaint);
                    handlers_.on_paint(evt);
                    
                    EndPaint(handle_, &ps);
                    handled = true;
                    return 0;
                }
                break;
            }
            
            case WM_DESTROY: {
                if (handlers_.on_destroy) {
                    DestroyEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    handlers_.on_destroy(evt);
                }
                PostQuitMessage(0);
                handled = true;
                return 0;
            }
            
            // Mouse events
            case WM_MOUSEMOVE: {
                // Track mouse for enter/leave events
                if (!handlers_.mouse_tracked && (handlers_.on_mouse_enter || handlers_.on_mouse_leave)) {
                    TRACKMOUSEEVENT tme = {};
                    tme.cbSize = sizeof(tme);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = handle_;
                    TrackMouseEvent(&tme);
                    handlers_.mouse_tracked = true;
                    
                    if (handlers_.on_mouse_enter) {
                        auto evt = make_mouse_event(msg, wparam, lparam);
                        handlers_.on_mouse_enter(evt);
                    }
                }
                
                if (handlers_.on_mouse_move) {
                    auto evt = make_mouse_event(msg, wparam, lparam);
                    handlers_.on_mouse_move(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_MOUSELEAVE: {
                handlers_.mouse_tracked = false;
                if (handlers_.on_mouse_leave) {
                    MouseEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    handlers_.on_mouse_leave(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN: {
                if (handlers_.on_mouse_down) {
                    auto evt = make_mouse_event(msg, wparam, lparam);
                    handlers_.on_mouse_down(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP: {
                if (handlers_.on_mouse_up) {
                    auto evt = make_mouse_event(msg, wparam, lparam);
                    handlers_.on_mouse_up(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK: {
                if (handlers_.on_mouse_double_click) {
                    auto evt = make_mouse_event(msg, wparam, lparam);
                    handlers_.on_mouse_double_click(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_MOUSEWHEEL:
            case WM_MOUSEHWHEEL: {
                if (handlers_.on_mouse_wheel) {
                    MouseWheelEvent evt;
                    static_cast<MouseEvent&>(evt) = make_mouse_event(msg, wparam, lparam);
                    evt.delta = GET_WHEEL_DELTA_WPARAM(wparam);
                    evt.horizontal = (msg == WM_MOUSEHWHEEL);
                    handlers_.on_mouse_wheel(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            // Keyboard events
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                if (handlers_.on_key_down) {
                    auto evt = make_key_event(msg, wparam, lparam);
                    handlers_.on_key_down(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                if (handlers_.on_key_up) {
                    auto evt = make_key_event(msg, wparam, lparam);
                    handlers_.on_key_up(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_CHAR: {
                if (handlers_.on_char) {
                    CharEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.character = static_cast<wchar_t>(wparam);
                    evt.repeat_count = static_cast<int>(lparam & 0xFFFF);
                    handlers_.on_char(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_COMMAND: {
                if (handlers_.on_command) {
                    CommandEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.id = LOWORD(wparam);
                    evt.code = HIWORD(wparam);
                    evt.control = reinterpret_cast<HWND>(lparam);
                    handlers_.on_command(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_TIMER: {
                if (handlers_.on_timer) {
                    TimerEvent evt;
                    evt.hwnd = handle_;
                    evt.message = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    evt.timer_id = wparam;
                    handlers_.on_timer(evt);
                    if (evt.handled) {
                        handled = true;
                        return 0;
                    }
                }
                break;
            }
            
            case WM_GETMINMAXINFO: {
                if (handlers_.min_size.width > 0 || handlers_.min_size.height > 0 ||
                    handlers_.max_size.width > 0 || handlers_.max_size.height > 0) {
                    auto* mmi = reinterpret_cast<MINMAXINFO*>(lparam);
                    if (handlers_.min_size.width > 0) 
                        mmi->ptMinTrackSize.x = handlers_.min_size.width;
                    if (handlers_.min_size.height > 0) 
                        mmi->ptMinTrackSize.y = handlers_.min_size.height;
                    if (handlers_.max_size.width > 0) 
                        mmi->ptMaxTrackSize.x = handlers_.max_size.width;
                    if (handlers_.max_size.height > 0) 
                        mmi->ptMaxTrackSize.y = handlers_.max_size.height;
                    handled = true;
                    return 0;
                }
                break;
            }
        }
        
        return 0;
    }
};

// -----------------------------------------------------------------------------
// Window Class Registration
// -----------------------------------------------------------------------------
class WindowClass {
public:
    WindowClass() noexcept = default;
    
    explicit WindowClass(std::wstring name, ATOM atom, HINSTANCE instance) noexcept
        : name_(std::move(name)), atom_(atom), instance_(instance) {}
    
    ~WindowClass() {
        if (atom_ != 0 && instance_) {
            UnregisterClassW(name_.c_str(), instance_);
        }
    }
    
    WindowClass(WindowClass&& other) noexcept
        : name_(std::move(other.name_))
        , atom_(std::exchange(other.atom_, static_cast<ATOM>(0)))
        , instance_(std::exchange(other.instance_, nullptr)) {}
    
    WindowClass& operator=(WindowClass&& other) noexcept {
        if (this != &other) {
            if (atom_ != 0 && instance_) {
                UnregisterClassW(name_.c_str(), instance_);
            }
            name_ = std::move(other.name_);
            atom_ = std::exchange(other.atom_, static_cast<ATOM>(0));
            instance_ = std::exchange(other.instance_, nullptr);
        }
        return *this;
    }
    
    WindowClass(const WindowClass&) = delete;
    WindowClass& operator=(const WindowClass&) = delete;
    
    [[nodiscard]] const std::wstring& name() const noexcept { return name_; }
    [[nodiscard]] ATOM atom() const noexcept { return atom_; }
    [[nodiscard]] bool valid() const noexcept { return atom_ != 0; }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    
private:
    std::wstring name_;
    ATOM atom_ = 0;
    HINSTANCE instance_ = nullptr;
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
    
    [[nodiscard]] HINSTANCE instance() const noexcept { return instance_; }

    // Register a custom window class
    [[nodiscard]] Result<WindowClass> register_class(const WindowClassConfig& config) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = config.style;
        wc.lpfnWndProc = window_proc;
        wc.cbClsExtra = config.extra_class_bytes;
        wc.cbWndExtra = config.extra_window_bytes;
        wc.hInstance = instance_;
        wc.hIcon = config.icon;
        wc.hIconSm = config.icon_small;
        wc.hCursor = config.cursor ? config.cursor : LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = config.background ? config.background : 
                          reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = config.class_name.c_str();
        
        ATOM atom = RegisterClassExW(&wc);
        if (!atom) {
            return make_error(ErrorCode::ClassRegistrationFailed, 
                L"Failed to register window class: " + config.class_name);
        }
        
        return WindowClass(config.class_name, atom, instance_);
    }

    [[nodiscard]] Result<Window> create_window(const WindowConfig& config, 
                                               const WindowClass* custom_class = nullptr) {
        // Use default class if none provided
        const wchar_t* class_name = L"W20PP_Window";
        
        if (custom_class && custom_class->valid()) {
            class_name = custom_class->name().c_str();
        } else {
            // Register default class if needed
            if (!default_class_registered_) {
                WNDCLASSEXW wc = {};
                wc.cbSize = sizeof(wc);
                wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
                wc.lpfnWndProc = window_proc;
                wc.hInstance = instance_;
                wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
                wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
                wc.lpszClassName = class_name;

                if (!RegisterClassExW(&wc)) {
                    return make_error(ErrorCode::ClassRegistrationFailed,
                        L"Failed to register default window class");
                }
                default_class_registered_ = true;
            }
        }

        // Calculate styles
        DWORD style = WS_OVERLAPPEDWINDOW;
        DWORD ex_style = 0;
        
        if (config.custom_styles) {
            style = config.custom_styles->style;
            ex_style = config.custom_styles->ex_style;
        } else {
            if (!config.resizable) style &= ~WS_THICKFRAME;
            if (!config.maximizable) style &= ~WS_MAXIMIZEBOX;
            if (!config.minimizable) style &= ~WS_MINIMIZEBOX;
            if (!config.show_in_taskbar) ex_style |= WS_EX_TOOLWINDOW;
            if (config.always_on_top) ex_style |= WS_EX_TOPMOST;
            if (config.opacity < 255) ex_style |= WS_EX_LAYERED;
        }

        int x = config.position.has_value() ? config.position->x : CW_USEDEFAULT;
        int y = config.position.has_value() ? config.position->y : CW_USEDEFAULT;

        HWND hwnd = CreateWindowExW(
            ex_style,
            class_name,
            config.title.data(),
            style,
            x, y,
            config.size.width, config.size.height,
            config.parent, nullptr, instance_, nullptr
        );

        if (!hwnd) {
            return make_error(ErrorCode::WindowCreationFailed, L"Failed to create window");
        }

        Window window(hwnd);

        // Apply theme
        bool use_dark = (config.theme == Theme::Dark) ||
                       (config.theme == Theme::System && is_dark_mode_enabled());
        window.apply_dark_mode(use_dark);
        
        // Apply icons
        if (config.icon) window.set_icon(config.icon);
        if (config.icon_small) window.set_icon_small(config.icon_small);
        
        // Apply opacity
        if (config.opacity < 255) {
            window.set_opacity(config.opacity);
        }
        
        // Apply size constraints
        if (config.min_size.width > 0 || config.min_size.height > 0) {
            window.set_min_size(config.min_size);
        }
        if (config.max_size.width > 0 || config.max_size.height > 0) {
            window.set_max_size(config.max_size);
        }
        
        // Apply initial state
        if (config.start_visible) {
            switch (config.initial_state) {
                case WindowState::Normal:
                    window.show();
                    break;
                case WindowState::Minimized:
                    ShowWindow(hwnd, SW_SHOWMINIMIZED);
                    break;
                case WindowState::Maximized:
                    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
                    break;
                case WindowState::Hidden:
                    // Don't show
                    break;
            }
        }

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
    
    // Process pending messages without blocking
    bool process_messages() {
        MSG msg = {};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return true;
    }

private:
    HINSTANCE instance_ = nullptr;
    bool default_class_registered_ = false;

    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, 
                                        WPARAM wparam, LPARAM lparam) {
        // Try to get the Window object from HWND
        Window* window = Window::from_hwnd(hwnd);
        
        if (window) {
            return window->process_message(msg, wparam, lparam);
        }
        
        // Default handling for messages before Window is associated
        if (msg == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
};

// -----------------------------------------------------------------------------
// Utility Functions
// -----------------------------------------------------------------------------

// Message box helpers
enum class MessageBoxType {
    Ok = MB_OK,
    OkCancel = MB_OKCANCEL,
    YesNo = MB_YESNO,
    YesNoCancel = MB_YESNOCANCEL,
    RetryCancel = MB_RETRYCANCEL,
    AbortRetryIgnore = MB_ABORTRETRYIGNORE
};

enum class MessageBoxIcon {
    None = 0,
    Information = MB_ICONINFORMATION,
    Warning = MB_ICONWARNING,
    Error = MB_ICONERROR,
    Question = MB_ICONQUESTION
};

enum class MessageBoxResult {
    Ok = IDOK,
    Cancel = IDCANCEL,
    Yes = IDYES,
    No = IDNO,
    Abort = IDABORT,
    Retry = IDRETRY,
    Ignore = IDIGNORE,
    TryAgain = IDTRYAGAIN,
    Continue = IDCONTINUE
};

[[nodiscard]] inline MessageBoxResult show_message_box(
    std::wstring_view message,
    std::wstring_view title = L"Message",
    MessageBoxType type = MessageBoxType::Ok,
    MessageBoxIcon icon = MessageBoxIcon::Information,
    HWND parent = nullptr) {
    
    int result = MessageBoxW(parent, message.data(), title.data(),
        static_cast<UINT>(type) | static_cast<UINT>(icon));
    return static_cast<MessageBoxResult>(result);
}

// Clipboard helpers
[[nodiscard]] inline bool clipboard_set_text(std::wstring_view text, HWND owner = nullptr) {
    if (!OpenClipboard(owner)) return false;
    EmptyClipboard();
    
    size_t size = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) {
        CloseClipboard();
        return false;
    }
    
    wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
    if (!pMem) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    
    memcpy(pMem, text.data(), size);
    GlobalUnlock(hMem);
    
    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    return true;
}

[[nodiscard]] inline std::optional<std::wstring> clipboard_get_text(HWND owner = nullptr) {
    if (!OpenClipboard(owner)) return std::nullopt;
    
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        CloseClipboard();
        return std::nullopt;
    }
    
    wchar_t* pData = static_cast<wchar_t*>(GlobalLock(hData));
    if (!pData) {
        CloseClipboard();
        return std::nullopt;
    }
    
    std::wstring text(pData);
    GlobalUnlock(hData);
    CloseClipboard();
    
    return text;
}

} // namespace w20pp
