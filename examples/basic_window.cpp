/**
 * W20PP Basic Window Example
 * 
 * Demonstrates creating a simple window with the W20PP framework.
 * This is the minimal "Hello World" example.
 */

#include "../include/w20pp/w20pp.hpp"

#if defined(_WIN32) && !defined(_CONSOLE)
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
#else
int main() {
#endif
    using namespace w20pp;

    Application app;

    // Declarative window configuration using C++20 designated initializers
    auto result = app.create_window({
        .title = L"W20PP - Basic Window",
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
    
    // Add a simple paint handler
    window.on_paint([](PaintEvent& evt) {
        RECT rc;
        GetClientRect(evt.hwnd, &rc);
        
        // Fill with a pleasant dark background
        HBRUSH bg = CreateSolidBrush(RGB(30, 35, 45));
        FillRect(evt.hdc, &rc, bg);
        DeleteObject(bg);
        
        // Draw centered text
        SetBkMode(evt.hdc, TRANSPARENT);
        SetTextColor(evt.hdc, RGB(200, 210, 220));
        
        HFONT font = CreateFontW(
            24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
        HFONT oldFont = (HFONT)SelectObject(evt.hdc, font);
        
        const wchar_t* text = L"Welcome to W20PP!";
        DrawTextW(evt.hdc, text, -1, &rc, 
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(evt.hdc, oldFont);
        DeleteObject(font);
    });
    
    // Handle close with ESC key
    window.on_key_down([&window](KeyEvent& evt) {
        if (evt.virtual_key == VK_ESCAPE) {
            window.close();
        }
    });
    
    window.show();
    window.center_on_screen();

    return app.run();
}
