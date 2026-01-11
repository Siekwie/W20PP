/**
 * W20PP Multi-Monitor Example
 * 
 * Demonstrates multi-monitor support and DPI awareness:
 * - Enumerate all connected monitors
 * - Display monitor properties
 * - Move window between monitors
 * - Handle DPI changes
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

    // Get all monitors at startup
    static std::vector<MonitorInfo> monitors = get_all_monitors();
    static int current_monitor_index = 0;

    auto result = app.create_window({
        .title = L"W20PP - Multi-Monitor Demo",
        .size = {.width = 700, .height = 500},
        .theme = Theme::System,
        .resizable = true
    });

    if (!result) {
        (void)show_message_box(result.error().format(), L"Error", 
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& window = result.value();

    // Find which monitor we're currently on
    auto updateCurrentMonitor = [&]() {
        auto pos = window.get_position();
        for (size_t i = 0; i < monitors.size(); i++) {
            if (monitors[i].bounds.contains(pos)) {
                current_monitor_index = static_cast<int>(i);
                break;
            }
        }
    };
    updateCurrentMonitor();

    // Paint handler
    window.on_paint([&window](PaintEvent& evt) {
        RECT rc;
        GetClientRect(evt.hwnd, &rc);
        
        // Background
        HBRUSH bg = CreateSolidBrush(RGB(20, 25, 35));
        FillRect(evt.hdc, &rc, bg);
        DeleteObject(bg);
        
        SetBkMode(evt.hdc, TRANSPARENT);
        
        HFONT font = CreateFontW(
            window.scale_value(16), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Consolas"
        );
        HFONT oldFont = (HFONT)SelectObject(evt.hdc, font);
        
        int y = window.scale_value(15);
        int lineHeight = window.scale_value(22);
        int x = window.scale_value(15);
        
        auto drawLine = [&](const std::wstring& text, COLORREF color = RGB(200, 210, 220)) {
            SetTextColor(evt.hdc, color);
            TextOutW(evt.hdc, x, y, text.c_str(), (int)text.length());
            y += lineHeight;
        };
        
        drawLine(L"Multi-Monitor Information", RGB(100, 180, 255));
        drawLine(std::format(L"Total Monitors: {}", monitors.size()));
        drawLine(std::format(L"Current DPI: {} (Scale: {:.0f}%)", 
            window.get_dpi(), window.get_dpi_scale() * 100));
        y += lineHeight / 2;
        
        // Draw each monitor's info
        for (size_t i = 0; i < monitors.size(); i++) {
            const auto& mon = monitors[i];
            bool isCurrent = (i == static_cast<size_t>(current_monitor_index));
            
            COLORREF headerColor = isCurrent ? RGB(100, 255, 150) : RGB(180, 180, 190);
            drawLine(std::format(L"Monitor {} {} {}", 
                i + 1,
                mon.is_primary ? L"(Primary)" : L"",
                isCurrent ? L"<-- Current" : L""), headerColor);
            
            drawLine(std::format(L"  Device: {}", mon.device_name));
            drawLine(std::format(L"  Bounds: {}x{} at ({}, {})",
                mon.bounds.width, mon.bounds.height,
                mon.bounds.x, mon.bounds.y));
            drawLine(std::format(L"  Work Area: {}x{} at ({}, {})",
                mon.work_area.width, mon.work_area.height,
                mon.work_area.x, mon.work_area.y));
            drawLine(std::format(L"  DPI: {}", mon.dpi));
            y += lineHeight / 2;
        }
        
        y += lineHeight / 2;
        drawLine(L"Keyboard Controls:", RGB(100, 180, 255));
        drawLine(L"  [1-9] Move to monitor 1-9");
        drawLine(L"  [C]   Center on current monitor");
        drawLine(L"  [R]   Refresh monitor list");
        drawLine(L"  [ESC] Close");
        
        SelectObject(evt.hdc, oldFont);
        DeleteObject(font);
    });

    // Handle DPI changes
    window.on_dpi_changed([&window](DpiChangedEvent& evt) {
        window.set_title(std::format(L"W20PP - Multi-Monitor Demo [DPI: {}]", evt.new_dpi));
        window.invalidate();
    });

    // Handle movement to update current monitor
    window.on_move([&](MoveEvent&) {
        updateCurrentMonitor();
        window.invalidate();
    });

    // Keyboard controls
    window.on_key_down([&window, &updateCurrentMonitor](KeyEvent& evt) {
        // Number keys 1-9 to move to specific monitor
        if (evt.virtual_key >= '1' && evt.virtual_key <= '9') {
            int monitorIndex = evt.virtual_key - '1';
            if (monitorIndex < static_cast<int>(monitors.size())) {
                const auto& mon = monitors[monitorIndex];
                auto size = window.get_size();
                
                // Center window on target monitor's work area
                int x = mon.work_area.x + (mon.work_area.width - size.width) / 2;
                int y = mon.work_area.y + (mon.work_area.height - size.height) / 2;
                
                window.set_position({x, y});
                current_monitor_index = monitorIndex;
                window.invalidate();
            }
            return;
        }
        
        switch (evt.virtual_key) {
            case 'C':
                // Center on current monitor
                if (current_monitor_index < static_cast<int>(monitors.size())) {
                    const auto& mon = monitors[current_monitor_index];
                    auto size = window.get_size();
                    int x = mon.work_area.x + (mon.work_area.width - size.width) / 2;
                    int y = mon.work_area.y + (mon.work_area.height - size.height) / 2;
                    window.set_position({x, y});
                    window.invalidate();
                }
                break;
                
            case 'R':
                // Refresh monitor list
                monitors = get_all_monitors();
                updateCurrentMonitor();
                window.invalidate();
                break;
                
            case VK_ESCAPE:
                window.close();
                break;
        }
    });

    window.show();
    window.center_on_screen();
    
    return app.run();
}
