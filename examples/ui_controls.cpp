/**
 * W20PP UI Controls Example
 * 
 * Demonstrates the Phase 3 UI control system including:
 * - Buttons
 * - Labels
 * - TextBox controls
 * - Event handling for controls
 * - Dynamic UI updates
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

    // Create main window
    auto result = app.create_window({
        .title = L"W20PP - UI Controls Demo",
        .size = {.width = 600, .height = 500},
        .theme = Theme::System,
        .resizable = true,
        .min_size = {.width = 500, .height = 400}
    });

    if (!result) {
        (void)show_message_box(result.error().format(), L"Error", 
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& window = result.value();
    HWND hwnd = window.handle();
    
    // State for UI interactions
    static int click_count = 0;
    static std::wstring status_text = L"Ready";
    
    // Create UI controls directly after window creation
    static Button btn_click;
    static Button btn_disable;
    static Button btn_clear;
    static Label lbl_title;
    static Label lbl_name;
    static Label lbl_status;
    static TextBox txt_input;
    static TextBox txt_output;
    static Font title_font;
    
    // Title label
    auto lbl_title_result = Label::create(hwnd, {
        .bounds = {20, 20, 560, 40},
        .text = L"W20PP UI Controls Demo",
        .id = 1
    });
    if (lbl_title_result) {
        lbl_title = std::move(lbl_title_result.value());
        // Create larger font for title
        title_font = Font::create(24, FW_BOLD, false, false, L"Segoe UI");
        if (title_font) {
            lbl_title.set_font(title_font.handle());
        }
    }
    
    // Name label
    auto lbl_name_result = Label::create(hwnd, {
        .bounds = {20, 80, 100, 25},
        .text = L"Your Name:",
        .id = 2
    });
    if (lbl_name_result) {
        lbl_name = std::move(lbl_name_result.value());
    }
    
    // Text input
    auto txt_input_result = TextBox::create(hwnd, {
        .bounds = {130, 78, 250, 25},
        .text = L"",
        .id = 100
    });
    if (txt_input_result) {
        txt_input = std::move(txt_input_result.value());
    }
    
    // Click button
    auto btn_click_result = Button::create(hwnd, {
        .bounds = {400, 78, 100, 30},
        .text = L"Click Me!",
        .id = 200,
        .default_button = true
    });
    if (btn_click_result) {
        btn_click = std::move(btn_click_result.value());
    }
    
    // Output label
    (void)Label::create(hwnd, {
        .bounds = {20, 120, 560, 25},
        .text = L"Output:",
        .id = 3
    });
    
    // Multiline text output
    auto txt_output_result = TextBox::create(hwnd, {
        .bounds = {20, 150, 560, 200},
        .text = L"Welcome to W20PP!\r\nClick the button or type your name and press Enter.",
        .id = 101,
        .multiline = true,
        .read_only = true,
        .auto_vscroll = true
    });
    if (txt_output_result) {
        txt_output = std::move(txt_output_result.value());
    }
    
    // Disable button
    auto btn_disable_result = Button::create(hwnd, {
        .bounds = {20, 370, 120, 35},
        .text = L"Disable Input",
        .id = 201
    });
    if (btn_disable_result) {
        btn_disable = std::move(btn_disable_result.value());
    }
    
    // Clear button
    auto btn_clear_result = Button::create(hwnd, {
        .bounds = {150, 370, 120, 35},
        .text = L"Clear Output",
        .id = 202
    });
    if (btn_clear_result) {
        btn_clear = std::move(btn_clear_result.value());
    }
    
    // Status label
    auto lbl_status_result = Label::create(hwnd, {
        .bounds = {20, 420, 560, 25},
        .text = L"Status: Ready",
        .id = 4
    });
    if (lbl_status_result) {
        lbl_status = std::move(lbl_status_result.value());
    }
    
    // Handle command events (button clicks, etc.)
    window.on_command([](CommandEvent& evt) {
        switch (evt.id) {
            case 200: { // Click Me button
                click_count++;
                std::wstring name = txt_input.get_text();
                
                if (name.empty()) {
                    txt_output.append_text(std::format(
                        L"\r\n[{}] Button clicked! (No name entered)",
                        click_count
                    ).c_str());
                } else {
                    txt_output.append_text(std::format(
                        L"\r\n[{}] Hello, {}!",
                        click_count,
                        name
                    ).c_str());
                }
                
                status_text = std::format(L"Status: Button clicked {} times", click_count);
                lbl_status.set_text(status_text.c_str());
                evt.handled = true;
                break;
            }
            
            case 201: { // Disable/Enable Input button
                bool currently_enabled = txt_input.is_enabled();
                txt_input.enable(!currently_enabled);
                btn_disable.set_text(currently_enabled ? L"Enable Input" : L"Disable Input");
                
                status_text = currently_enabled ? 
                    L"Status: Input disabled" : L"Status: Input enabled";
                lbl_status.set_text(status_text.c_str());
                evt.handled = true;
                break;
            }
            
            case 202: { // Clear Output button
                txt_output.clear();
                txt_output.set_text(L"Output cleared.");
                click_count = 0;
                
                status_text = L"Status: Output cleared";
                lbl_status.set_text(status_text.c_str());
                evt.handled = true;
                break;
            }
            
            case 100: { // Text input - handle changes
                if (evt.code == EN_CHANGE) {
                    std::wstring name = txt_input.get_text();
                    if (!name.empty()) {
                        status_text = std::format(L"Status: Typing... ({})", name);
                        lbl_status.set_text(status_text.c_str());
                    }
                }
                break;
            }
        }
    });
    
    // Handle keyboard shortcuts
    window.on_key_down([&window](KeyEvent& evt) {
        if (evt.virtual_key == VK_ESCAPE) {
            window.close();
            evt.handled = true;
        } else if (evt.virtual_key == VK_RETURN && !evt.alt_down) {
            // Simulate button click on Enter
            if (txt_input.has_focus()) {
                btn_click.click();
                evt.handled = true;
            }
        } else if (evt.virtual_key == 'L' && evt.alt_down) {
            // Alt+L to focus input
            txt_input.focus();
            evt.handled = true;
        }
    });
    
    // Handle close with confirmation
    window.on_close([](CloseEvent& evt) {
        auto result = show_message_box(
            L"Are you sure you want to exit?",
            L"Confirm Exit",
            MessageBoxType::YesNo,
            MessageBoxIcon::Question,
            evt.hwnd
        );
        if (result == MessageBoxResult::No) {
            evt.cancel = true;
        }
    });
    
    // Resize handler to adjust control layout
    window.on_size([](SizeEvent& evt) {
        if (evt.type == SizeEvent::Type::Minimized) return;
        
        int width = evt.new_size.width;
        int height = evt.new_size.height;
        
        // Adjust controls to new size
        if (lbl_title.valid()) {
            lbl_title.set_bounds({20, 20, width - 40, 40});
        }
        
        if (txt_input.valid()) {
            txt_input.set_bounds({130, 78, width - 270, 25});
        }
        
        if (btn_click.valid()) {
            btn_click.set_bounds({width - 120, 78, 100, 30});
        }
        
        if (txt_output.valid()) {
            txt_output.set_bounds({20, 150, width - 40, height - 230});
        }
        
        if (lbl_status.valid()) {
            lbl_status.set_bounds({20, height - 60, width - 40, 25});
        }
        
        if (btn_clear.valid()) {
            btn_clear.set_bounds({150, height - 110, 120, 35});
        }
        
        if (btn_disable.valid()) {
            btn_disable.set_bounds({20, height - 110, 120, 35});
        }
    });
    
    window.center_on_screen();
    window.show();
    
    // Give focus to the text input
    txt_input.focus();

    return app.run();
}
