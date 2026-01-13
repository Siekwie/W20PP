# W20PP Examples

# Helper function to add examples
function(add_w20pp_example name)
    # Windows GUI application
    add_executable(${name} WIN32 ${name}.cpp)
    target_link_libraries(${name} PRIVATE W20PP::W20PP msimg32)
    
    # Console version (for debugging with console output)
    add_executable(${name}_console ${name}.cpp)
    target_compile_definitions(${name}_console PRIVATE _CONSOLE)
    target_link_libraries(${name}_console PRIVATE W20PP::W20PP msimg32)
endfunction()

# Phase 1: Basic Examples
add_w20pp_example(basic_window)

# Phase 2: Window Management & Lifecycle Examples
add_w20pp_example(event_handling)
add_w20pp_example(window_features)
add_w20pp_example(multi_monitor)
add_w20pp_example(custom_window_class)

# Phase 3: UI Controls Examples
add_w20pp_example(ui_controls)
add_w20pp_example(advanced_controls)
