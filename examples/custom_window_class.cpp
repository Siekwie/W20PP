/**
 * W20PP Custom Window Class Example
 * 
 * Demonstrates creating and using custom window classes:
 * - Register custom window class with specific styles
 * - Create multiple windows from custom classes
 * - Different window configurations per class
 */

#include "../include/w20pp/w20pp.hpp"
#include <format>
#include <vector>

#if defined(_WIN32) && !defined(_CONSOLE)
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
#else
int main() {
#endif
    using namespace w20pp;

    Application app;

    // Register a custom window class with no-redraw style
    auto toolWindowClassResult = app.register_class({
        .class_name = L"W20PP_ToolWindow",
        .cursor = LoadCursor(nullptr, IDC_HAND),
        .background = CreateSolidBrush(RGB(40, 45, 55)),
        .style = CS_DBLCLKS  // Only double-click tracking, no redraw
    });

    if (!toolWindowClassResult) {
        (void)show_message_box(toolWindowClassResult.error().format(), L"Error",
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& toolWindowClass = toolWindowClassResult.value();

    // Register another custom class for popup-style windows
    auto popupClassResult = app.register_class({
        .class_name = L"W20PP_Popup",
        .background = CreateSolidBrush(RGB(50, 30, 60)),
        .style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW
    });

    if (!popupClassResult) {
        (void)show_message_box(popupClassResult.error().format(), L"Error",
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& popupClass = popupClassResult.value();

    // Create the main window using default class
    auto mainResult = app.create_window({
        .title = L"W20PP - Custom Window Classes (Main)",
        .size = {.width = 800, .height = 600},
        .position = Point{100, 100},
        .theme = Theme::System
    });

    if (!mainResult) {
        (void)show_message_box(mainResult.error().format(), L"Error",
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& mainWindow = mainResult.value();

    // Create a tool window using our custom class
    auto toolResult = app.create_window({
        .title = L"Tool Window (Custom Class)",
        .size = {.width = 300, .height = 200},
        .position = Point{920, 100},
        .theme = Theme::Dark,
        .resizable = true,
        .maximizable = false,
        .minimizable = false
    }, &toolWindowClass);

    if (!toolResult) {
        (void)show_message_box(toolResult.error().format(), L"Error",
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& toolWindow = toolResult.value();

    // Create a popup-style window using custom styles
    WindowStyles popupStyles;
    popupStyles.popup().topmost().layered();
    
    auto popupResult = app.create_window({
        .title = L"",  // Popups often have no title
        .size = {.width = 250, .height = 150},
        .position = Point{920, 320},
        .theme = Theme::Dark,
        .custom_styles = popupStyles,
        .opacity = 230
    }, &popupClass);

    if (!popupResult) {
        (void)show_message_box(popupResult.error().format(), L"Error",
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& popupWindow = popupResult.value();

    // Setup main window
    mainWindow.on_paint([](PaintEvent& evt) {
        RECT rc;
        GetClientRect(evt.hwnd, &rc);
        
        HBRUSH bg = CreateSolidBrush(RGB(30, 35, 45));
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
        
        drawLine(L"Custom Window Classes Demo", RGB(100, 180, 255));
        drawLine(L"==========================");
        y += 10;
        
        drawLine(L"This demo shows three windows created with different classes:");
        y += 10;
        
        drawLine(L"1. Main Window (Default Class)", RGB(150, 220, 150));
        drawLine(L"   - Standard overlapped window");
        drawLine(L"   - Full window controls");
        drawLine(L"   - Default cursor and background");
        y += 10;
        
        drawLine(L"2. Tool Window (Custom 'W20PP_ToolWindow' Class)", RGB(220, 180, 100));
        drawLine(L"   - Hand cursor");
        drawLine(L"   - Dark background");
        drawLine(L"   - No maximize/minimize buttons");
        y += 10;
        
        drawLine(L"3. Popup Window (Custom 'W20PP_Popup' Class)", RGB(200, 150, 220));
        drawLine(L"   - Borderless popup style");
        drawLine(L"   - Drop shadow");
        drawLine(L"   - Always on top");
        drawLine(L"   - Semi-transparent");
        y += 20;
        
        drawLine(L"Press ESC on any window to close all windows.");
        
        SelectObject(evt.hdc, oldFont);
        DeleteObject(font);
    });

    // Setup tool window
    toolWindow.on_paint([](PaintEvent& evt) {
        RECT rc;
        GetClientRect(evt.hwnd, &rc);
        
        SetBkMode(evt.hdc, TRANSPARENT);
        SetTextColor(evt.hdc, RGB(220, 200, 150));
        
        HFONT font = CreateFontW(
            16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Consolas"
        );
        HFONT oldFont = (HFONT)SelectObject(evt.hdc, font);
        
        TextOutW(evt.hdc, 15, 15, L"Tool Window", 11);
        TextOutW(evt.hdc, 15, 40, L"Custom class with", 17);
        TextOutW(evt.hdc, 15, 60, L"hand cursor", 11);
        TextOutW(evt.hdc, 15, 100, L"Double-click me!", 16);
        
        SelectObject(evt.hdc, oldFont);
        DeleteObject(font);
    });

    toolWindow.on_mouse_double_click([](MouseEvent& evt) {
        (void)show_message_box(L"You double-clicked the tool window!",
            L"Double Click", MessageBoxType::Ok, MessageBoxIcon::Information, evt.hwnd);
    });

    // Setup popup window
    popupWindow.on_paint([](PaintEvent& evt) {
        RECT rc;
        GetClientRect(evt.hwnd, &rc);
        
        // Draw a rounded-looking border
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(180, 130, 220));
        HPEN oldPen = (HPEN)SelectObject(evt.hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(evt.hdc, GetStockObject(NULL_BRUSH));
        Rectangle(evt.hdc, 0, 0, rc.right, rc.bottom);
        SelectObject(evt.hdc, oldPen);
        SelectObject(evt.hdc, oldBrush);
        DeleteObject(pen);
        
        SetBkMode(evt.hdc, TRANSPARENT);
        SetTextColor(evt.hdc, RGB(200, 180, 230));
        
        HFONT font = CreateFontW(
            14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
        HFONT oldFont = (HFONT)SelectObject(evt.hdc, font);
        
        TextOutW(evt.hdc, 15, 15, L"Popup Window", 12);
        TextOutW(evt.hdc, 15, 40, L"- Borderless", 12);
        TextOutW(evt.hdc, 15, 58, L"- Drop shadow", 13);
        TextOutW(evt.hdc, 15, 76, L"- Always on top", 15);
        TextOutW(evt.hdc, 15, 94, L"- Semi-transparent", 18);
        TextOutW(evt.hdc, 15, 120, L"Drag to move!", 13);
        
        SelectObject(evt.hdc, oldFont);
        DeleteObject(font);
    });

    // Allow dragging the popup window
    static bool dragging = false;
    static Point drag_start;
    static Point window_start;

    popupWindow.on_mouse_down([&popupWindow](MouseEvent& evt) {
        if (evt.left_button) {
            dragging = true;
            drag_start = evt.screen_pos;
            window_start = popupWindow.get_position();
            SetCapture(evt.hwnd);
        }
    });

    popupWindow.on_mouse_move([&popupWindow](MouseEvent& evt) {
        if (dragging) {
            int dx = evt.screen_pos.x - drag_start.x;
            int dy = evt.screen_pos.y - drag_start.y;
            popupWindow.set_position({window_start.x + dx, window_start.y + dy});
        }
    });

    popupWindow.on_mouse_up([](MouseEvent&) {
        if (dragging) {
            dragging = false;
            ReleaseCapture();
        }
    });

    // Close all windows when ESC is pressed on any window
    auto closeAll = [&](KeyEvent& evt) {
        if (evt.virtual_key == VK_ESCAPE) {
            mainWindow.close();
        }
    };

    mainWindow.on_key_down(closeAll);
    toolWindow.on_key_down(closeAll);
    popupWindow.on_key_down(closeAll);

    // Show all windows
    mainWindow.show();
    toolWindow.show();
    popupWindow.show();

    return app.run();
}
