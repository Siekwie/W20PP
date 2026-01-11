# W20PP Examples

# Basic window example (Windows GUI application)
add_executable(basic_window WIN32 basic_window.cpp)
target_link_libraries(basic_window PRIVATE W20PP::W20PP)

# Console example (for debugging with console output)
add_executable(basic_window_console basic_window.cpp)
target_compile_definitions(basic_window_console PRIVATE _CONSOLE)
target_link_libraries(basic_window_console PRIVATE W20PP::W20PP)
