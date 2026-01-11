/**
 * W20PP Window Features Example
 * 
 * Demonstrates advanced window features including:
 * - Window transparency and opacity
 * - Always-on-top (topmost)
 * - Size constraints
 * - Window state management
 * - Style manipulation
 */

#include "../include/w20pp/w20pp.hpp"
#include <format>

#if defined(_WIN32) && !defined(_CONSOLE)
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
#else
int main() {
#endif
    using namespace w20pp;

    Application app;

    // Create window with advanced features
    auto result = app.create_window({
        .title = L"W20PP - Window Features Demo",
        .size = {.width = 800, .height = 600},
        .theme = Theme::System,
        .resizable = true,
        .min_size = {400, 300},     // Minimum size constraint
        .max_size = {1600, 1200}    // Maximum size constraint
    });

    if (!result) {
        (void)show_message_box(result.error().format(), L"Error", 
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& window = result.value();

    // Track current state
    static BYTE current_opacity = 255;
    static bool is_topmost = false;
    static bool borders_visible = true;

    // Paint handler to show current state and instructions
    window.on_paint([&window](PaintEvent& evt) {
        RECT rc;
        GetClientRect(evt.hwnd, &rc);
        
        // Fill background with a nice gradient-like effect
        HBRUSH bg = CreateSolidBrush(RGB(25, 35, 50));
        FillRect(evt.hdc, &rc, bg);
        DeleteObject(bg);
        
        SetBkMode(evt.hdc, TRANSPARENT);
        SetTextColor(evt.hdc, RGB(200, 210, 220));
        
        HFONT font = CreateFontW(
            18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
        HFONT oldFont = (HFONT)SelectObject(evt.hdc, font);
        
        int y = 20;
        int lineHeight = 26;
        
        auto drawLine = [&](const std::wstring& text, COLORREF color = RGB(200, 210, 220)) {
            SetTextColor(evt.hdc, color);
            TextOutW(evt.hdc, 20, y, text.c_str(), (int)text.length());
            y += lineHeight;
        };
        
        auto drawHeader = [&](const std::wstring& text) {
            drawLine(text, RGB(120, 180, 255));
        };
        
        drawHeader(L"W20PP Window Features Demo");
        drawLine(L"===========================");
        y += 10;
        
        drawHeader(L"Keyboard Controls:");
        drawLine(L"  [+/-] Adjust opacity (current: " + 
                 std::to_wstring(current_opacity) + L")");
        drawLine(L"  [T]   Toggle always-on-top (current: " + 
                 std::wstring(is_topmost ? L"ON" : L"OFF") + L")");
        drawLine(L"  [B]   Toggle borders (current: " + 
                 std::wstring(borders_visible ? L"ON" : L"OFF") + L")");
        drawLine(L"  [C]   Center window on screen");
        drawLine(L"  [M]   Maximize window");
        drawLine(L"  [N]   Minimize window");
        drawLine(L"  [R]   Restore window");
        y += 10;
        
        drawHeader(L"Current Window State:");
        auto state = window.get_state();
        std::wstring stateStr;
        switch (state) {
            case WindowState::Normal: stateStr = L"Normal"; break;
            case WindowState::Minimized: stateStr = L"Minimized"; break;
            case WindowState::Maximized: stateStr = L"Maximized"; break;
            case WindowState::Hidden: stateStr = L"Hidden"; break;
        }
        drawLine(L"  State: " + stateStr);
        
        auto pos = window.get_position();
        auto size = window.get_size();
        drawLine(std::format(L"  Position: ({}, {})", pos.x, pos.y));
        drawLine(std::format(L"  Size: {}x{}", size.width, size.height));
        drawLine(std::format(L"  DPI: {} (scale: {:.2f}x)", 
            window.get_dpi(), window.get_dpi_scale()));
        
        auto monitor = window.get_monitor();
        if (monitor) {
            y += 10;
            drawHeader(L"Current Monitor:");
            drawLine(std::format(L"  Device: {}", monitor->device_name));
            drawLine(std::format(L"  Bounds: {}x{} at ({}, {})", 
                monitor->bounds.width, monitor->bounds.height,
                monitor->bounds.x, monitor->bounds.y));
            drawLine(std::format(L"  Work Area: {}x{}", 
                monitor->work_area.width, monitor->work_area.height));
            drawLine(std::format(L"  DPI: {} | Primary: {}", 
                monitor->dpi, monitor->is_primary ? L"Yes" : L"No"));
        }
        
        y += 10;
        drawHeader(L"Size Constraints:");
        auto minSz = window.get_min_size();
        auto maxSz = window.get_max_size();
        drawLine(std::format(L"  Min: {}x{}", minSz.width, minSz.height));
        drawLine(std::format(L"  Max: {}x{}", maxSz.width, maxSz.height));
        
        SelectObject(evt.hdc, oldFont);
        DeleteObject(font);
    });

    // Keyboard controls
    window.on_key_down([&window](KeyEvent& evt) {
        switch (evt.virtual_key) {
            case VK_OEM_PLUS:
            case VK_ADD:
                // Increase opacity
                if (current_opacity < 255) {
                    current_opacity = static_cast<BYTE>(
                        (std::min)(255, current_opacity + 15));
                    window.set_opacity(current_opacity);
                    window.invalidate();
                }
                break;
                
            case VK_OEM_MINUS:
            case VK_SUBTRACT:
                // Decrease opacity
                if (current_opacity > 30) {
                    current_opacity = static_cast<BYTE>(
                        (std::max)(30, current_opacity - 15));
                    window.set_opacity(current_opacity);
                    window.invalidate();
                }
                break;
                
            case 'T':
                // Toggle topmost
                is_topmost = !is_topmost;
                window.set_topmost(is_topmost);
                window.invalidate();
                break;
                
            case 'B':
                // Toggle borders
                borders_visible = !borders_visible;
                if (borders_visible) {
                    window.add_style(WS_CAPTION | WS_THICKFRAME);
                } else {
                    window.remove_style(WS_CAPTION | WS_THICKFRAME);
                }
                window.invalidate();
                break;
                
            case 'C':
                // Center on screen
                window.center_on_screen();
                window.invalidate();
                break;
                
            case 'M':
                // Maximize
                window.maximize();
                break;
                
            case 'N':
                // Minimize
                window.minimize();
                break;
                
            case 'R':
                // Restore
                window.restore();
                break;
                
            case VK_ESCAPE:
                window.close();
                break;
        }
    });

    // Refresh display when size/move changes
    window.on_size([&window](SizeEvent&) {
        window.invalidate();
    });

    window.on_move([&window](MoveEvent&) {
        window.invalidate();
    });

    window.show();
    window.center_on_screen();
    
    return app.run();
}
