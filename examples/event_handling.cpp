/**
 * W20PP Event Handling Example
 * 
 * Demonstrates the type-safe event system with various event handlers.
 * Shows mouse, keyboard, focus, and window lifecycle events.
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

    // Create window with event handling configuration
    auto result = app.create_window({
        .title = L"W20PP - Event Handling Demo",
        .size = {.width = 1024, .height = 768},
        .theme = Theme::System,
        .resizable = true
    });

    if (!result) {
        (void)show_message_box(result.error().format(), L"Error", 
                        MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& window = result.value();

    // Track some state for display
    static std::wstring last_event = L"No events yet";
    static Point last_mouse_pos = {0, 0};
    static int key_press_count = 0;

    // ----- Window Lifecycle Events -----
    
    window.on_close([](CloseEvent& evt) {
        // Ask for confirmation before closing
        auto result = show_message_box(
            L"Are you sure you want to close the window?",
            L"Confirm Close",
            MessageBoxType::YesNo,
            MessageBoxIcon::Question,
            evt.hwnd
        );
        if (result == MessageBoxResult::No) {
            evt.cancel = true;  // Prevent window from closing
        }
    });

    window.on_size([&window](SizeEvent& evt) {
        last_event = std::format(L"Resized to {}x{}", 
            evt.new_size.width, evt.new_size.height);
        
        // Update window title with size
        window.set_title(std::format(L"W20PP - Event Demo [{}x{}]",
            evt.new_size.width, evt.new_size.height));
    });

    window.on_move([](MoveEvent& evt) {
        last_event = std::format(L"Moved to ({}, {})", 
            evt.new_position.x, evt.new_position.y);
    });

    window.on_focus([](FocusEvent& evt) {
        if (evt.gained) {
            last_event = L"Window gained focus";
        } else {
            last_event = L"Window lost focus";
        }
    });

    window.on_dpi_changed([](DpiChangedEvent& evt) {
        last_event = std::format(L"DPI changed to {}", evt.new_dpi);
    });

    // ----- Mouse Events -----

    window.on_mouse_move([](MouseEvent& evt) {
        last_mouse_pos = evt.position;
    });

    window.on_mouse_down([](MouseEvent& evt) {
        std::wstring button;
        if (evt.left_button) button = L"Left";
        else if (evt.right_button) button = L"Right";
        else if (evt.middle_button) button = L"Middle";
        
        last_event = std::format(L"{} mouse button down at ({}, {})",
            button, evt.position.x, evt.position.y);
    });

    window.on_mouse_up([](MouseEvent& evt) {
        last_event = std::format(L"Mouse button up at ({}, {})",
            evt.position.x, evt.position.y);
    });

    window.on_mouse_double_click([](MouseEvent& evt) {
        last_event = std::format(L"Double-click at ({}, {})",
            evt.position.x, evt.position.y);
    });

    window.on_mouse_wheel([](MouseWheelEvent& evt) {
        std::wstring direction = evt.delta > 0 ? L"up" : L"down";
        std::wstring axis = evt.horizontal ? L"horizontal" : L"vertical";
        last_event = std::format(L"Mouse wheel {} ({}, delta: {})",
            direction, axis, evt.delta);
    });

    window.on_mouse_enter([](MouseEvent&) {
        last_event = L"Mouse entered window";
    });

    window.on_mouse_leave([](MouseEvent&) {
        last_event = L"Mouse left window";
    });

    // ----- Keyboard Events -----

    window.on_key_down([](KeyEvent& evt) {
        key_press_count++;
        last_event = std::format(L"Key down: VK={} (total: {})",
            evt.virtual_key, key_press_count);
        
        // Handle specific keys
        if (evt.virtual_key == VK_ESCAPE) {
            // Post close message on Escape
            PostMessageW(evt.hwnd, WM_CLOSE, 0, 0);
        }
    });

    window.on_char([](CharEvent& evt) {
        if (evt.character >= 32) {  // Printable characters
            last_event = std::format(L"Character typed: '{}'", evt.character);
        }
    });

    // ----- Paint Event -----

    window.on_paint([](PaintEvent& evt) {
        // Get client rect
        RECT rc;
        GetClientRect(evt.hwnd, &rc);
        
        // Fill background
        HBRUSH bg = CreateSolidBrush(RGB(30, 30, 40));
        FillRect(evt.hdc, &rc, bg);
        DeleteObject(bg);
        
        // Set text properties
        SetBkMode(evt.hdc, TRANSPARENT);
        SetTextColor(evt.hdc, RGB(220, 220, 230));
        
        // Create font
        HFONT font = CreateFontW(
            20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Consolas"
        );
        HFONT oldFont = (HFONT)SelectObject(evt.hdc, font);
        
        // Draw instructions
        int y = 20;
        int lineHeight = 28;
        
        auto drawLine = [&](const std::wstring& text) {
            TextOutW(evt.hdc, 20, y, text.c_str(), (int)text.length());
            y += lineHeight;
        };
        
        drawLine(L"W20PP Event Handling Demo");
        drawLine(L"========================");
        y += 10;
        drawLine(L"Instructions:");
        drawLine(L"  - Move the mouse to track position");
        drawLine(L"  - Click, double-click, or scroll");
        drawLine(L"  - Type on the keyboard");
        drawLine(L"  - Resize or move the window");
        drawLine(L"  - Press ESC to close (with confirmation)");
        y += 20;
        drawLine(L"Last Event:");
        SetTextColor(evt.hdc, RGB(100, 200, 255));
        drawLine(std::format(L"  {}", last_event));
        y += 10;
        SetTextColor(evt.hdc, RGB(220, 220, 230));
        drawLine(std::format(L"Mouse Position: ({}, {})", 
            last_mouse_pos.x, last_mouse_pos.y));
        drawLine(std::format(L"Key Press Count: {}", key_press_count));
        
        // Cleanup
        SelectObject(evt.hdc, oldFont);
        DeleteObject(font);
    });

    // Set up a timer to refresh the display
   (void)window.set_timer(1, 50);  // 50ms refresh rate
    
    window.on_timer([&window](TimerEvent& evt) {
        if (evt.timer_id == 1) {
            window.invalidate();
        }
    });

    window.show();
    return app.run();
}
