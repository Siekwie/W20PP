/**
 * W20PP Basic Window Example
 * 
 * Demonstrates creating a simple window with the W20PP framework.
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
        MessageBoxW(nullptr, result.error().message.c_str(), 
                   L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    auto& window = result.value();
    window.show();

    return app.run();
}
