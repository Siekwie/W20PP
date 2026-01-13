/**
 * W20PP Advanced Controls Example
 * 
 * Demonstrates all Phase 3 controls:
 * - Buttons, Labels, TextBoxes
 * - CheckBoxes (including tri-state)
 * - Radio Buttons
 * - ComboBox (dropdown)
 * - ListBox (single and multi-select)
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

    auto result = app.create_window({
        .title = L"W20PP - Advanced Controls Demo",
        .size = {700, 600},
        .theme = Theme::System,
        .resizable = true,
        .min_size = {600, 500}
    });

    if (!result) {
        (void)show_message_box(result.error().format(), L"Error", 
                               MessageBoxType::Ok, MessageBoxIcon::Error);
        return 1;
    }

    auto& window = result.value();
    HWND hwnd = window.handle();
    
    // Control storage
    static Label lbl_title;
    static CheckBox chk_option1;
    static CheckBox chk_option2;
    static CheckBox chk_tristate;
    static RadioButton radio_option1;
    static RadioButton radio_option2;
    static RadioButton radio_option3;
    static ComboBox combo_colors;
    static ListBox list_items;
    static TextBox txt_output;
    static Button btn_submit;
    static Button btn_clear;
    static Font title_font;
    
    // Create all controls directly
    
    // Title
    auto title_result = Label::create(hwnd, {
        .bounds = {20, 10, 660, 35},
        .text = L"Advanced Controls Demo",
        .id = 1
    });
    if (title_result) {
        lbl_title = std::move(title_result.value());
        title_font = Font::create(20, FW_BOLD);
        if (title_font) lbl_title.set_font(title_font.handle());
    }
    
    // CheckBoxes section
    (void)Label::create(hwnd, {
        .bounds = {20, 55, 200, 20},
        .text = L"CheckBoxes:",
        .id = 2
    });
    
    auto chk1_result = CheckBox::create(hwnd, {
        .bounds = {30, 80, 150, 20},
        .text = L"Enable Feature A",
        .id = 100
    });
    if (chk1_result) chk_option1 = std::move(chk1_result.value());
    
    auto chk2_result = CheckBox::create(hwnd, {
        .bounds = {30, 105, 150, 20},
        .text = L"Enable Feature B",
        .id = 101,
        .checked = true
    });
    if (chk2_result) chk_option2 = std::move(chk2_result.value());
    
    auto chk3_result = CheckBox::create(hwnd, {
        .bounds = {30, 130, 150, 20},
        .text = L"Tri-State Option",
        .id = 102,
        .three_state = true
    });
    if (chk3_result) {
        chk_tristate = std::move(chk3_result.value());
        chk_tristate.set_indeterminate();
    }
    
    // Radio buttons section
    (void)Label::create(hwnd, {
        .bounds = {220, 55, 200, 20},
        .text = L"Radio Buttons (Choose One):",
        .id = 3
    });
    
    auto radio1_result = RadioButton::create(hwnd, {
        .bounds = {230, 80, 100, 20},
        .text = L"Option 1",
        .id = 200,
        .checked = true
    });
    if (radio1_result) radio_option1 = std::move(radio1_result.value());
    
    auto radio2_result = RadioButton::create(hwnd, {
        .bounds = {230, 105, 100, 20},
        .text = L"Option 2",
        .id = 201
    });
    if (radio2_result) radio_option2 = std::move(radio2_result.value());
    
    auto radio3_result = RadioButton::create(hwnd, {
        .bounds = {230, 130, 100, 20},
        .text = L"Option 3",
        .id = 202
    });
    if (radio3_result) radio_option3 = std::move(radio3_result.value());
    
    // ComboBox section
    (void)Label::create(hwnd, {
        .bounds = {420, 55, 200, 20},
        .text = L"Select Color:",
        .id = 4
    });
    
    auto combo_result = ComboBox::create(hwnd, {
        .bounds = {420, 80, 150, 200},
        .id = 300,
        .items = {L"Red", L"Green", L"Blue", L"Yellow", L"Purple", L"Orange"},
        .selected_index = 0,
        .style = ComboBoxConfig::Style::DropDownList
    });
    if (combo_result) combo_colors = std::move(combo_result.value());
    
    // ListBox section
    (void)Label::create(hwnd, {
        .bounds = {20, 165, 200, 20},
        .text = L"Select Items (Multi-select):",
        .id = 5
    });
    
    auto list_result = ListBox::create(hwnd, {
        .bounds = {20, 190, 200, 150},
        .id = 400,
        .items = {
            L"Apple", L"Banana", L"Cherry", L"Date", 
            L"Elderberry", L"Fig", L"Grape", L"Honeydew"
        },
        .multi_select = true,
        .sorted = true
    });
    if (list_result) list_items = std::move(list_result.value());
    
    // Output text box
    (void)Label::create(hwnd, {
        .bounds = {230, 165, 200, 20},
        .text = L"Output:",
        .id = 6
    });
    
    auto output_result = TextBox::create(hwnd, {
        .bounds = {230, 190, 450, 150},
        .text = L"Welcome! Make selections and click Submit to see results.",
        .id = 500,
        .multiline = true,
        .read_only = true,
        .auto_vscroll = true
    });
    if (output_result) txt_output = std::move(output_result.value());
    
    // Buttons
    auto submit_result = Button::create(hwnd, {
        .bounds = {20, 360, 120, 35},
        .text = L"Submit",
        .id = 600,
        .default_button = true
    });
    if (submit_result) btn_submit = std::move(submit_result.value());
    
    auto clear_result = Button::create(hwnd, {
        .bounds = {150, 360, 120, 35},
        .text = L"Clear Output",
        .id = 601
    });
    if (clear_result) btn_clear = std::move(clear_result.value());
    
    // Handle command events
    window.on_command([](CommandEvent& evt) {
        switch (evt.id) {
            case 600: { // Submit button
                std::wstring output = L"=== Form Submission ===\r\n\r\n";
                
                // CheckBoxes
                output += L"CheckBoxes:\r\n";
                output += std::format(L"  Feature A: {}\r\n", 
                    chk_option1.is_checked() ? L"Enabled" : L"Disabled");
                output += std::format(L"  Feature B: {}\r\n", 
                    chk_option2.is_checked() ? L"Enabled" : L"Disabled");
                
                auto tristate = chk_tristate.get_state();
                std::wstring tristate_str = 
                    (tristate == CheckBox::State::Checked) ? L"Checked" :
                    (tristate == CheckBox::State::Unchecked) ? L"Unchecked" :
                    L"Indeterminate";
                output += std::format(L"  Tri-State: {}\r\n\r\n", tristate_str);
                
                // Radio buttons
                output += L"Radio Selection: ";
                if (radio_option1.is_checked()) output += L"Option 1\r\n\r\n";
                else if (radio_option2.is_checked()) output += L"Option 2\r\n\r\n";
                else if (radio_option3.is_checked()) output += L"Option 3\r\n\r\n";
                
                // ComboBox
                output += std::format(L"Selected Color: {}\r\n\r\n", 
                    combo_colors.get_selected_text());
                
                // ListBox
                auto selected = list_items.get_selected_indices();
                output += std::format(L"Selected Items ({}):\r\n", selected.size());
                for (int idx : selected) {
                    output += std::format(L"  - {}\r\n", list_items.get_item_text(idx));
                }
                
                txt_output.set_text(output);
                evt.handled = true;
                break;
            }
            
            case 601: { // Clear button
                txt_output.set_text(L"Output cleared. Make new selections.");
                evt.handled = true;
                break;
            }
            
            case 100: // CheckBox 1
            case 101: // CheckBox 2
            case 102: // CheckBox 3 (tri-state)
                if (evt.code == BN_CLICKED) {
                    txt_output.append_text(L"\r\nCheckBox state changed.");
                }
                break;
                
            case 200: // Radio 1
            case 201: // Radio 2
            case 202: // Radio 3
                if (evt.code == BN_CLICKED) {
                    txt_output.append_text(L"\r\nRadio button selection changed.");
                }
                break;
                
            case 300: // ComboBox
                if (evt.code == CBN_SELCHANGE) {
                    std::wstring color = combo_colors.get_selected_text();
                    txt_output.append_text(
                        std::format(L"\r\nColor changed to: {}", color).c_str()
                    );
                }
                break;
                
            case 400: // ListBox
                if (evt.code == LBN_SELCHANGE) {
                    auto count = list_items.get_selected_indices().size();
                    txt_output.append_text(
                        std::format(L"\r\nSelected {} items.", count).c_str()
                    );
                }
                break;
        }
    });
    
    // Keyboard shortcuts
    window.on_key_down([&window](KeyEvent& evt) {
        if (evt.virtual_key == VK_ESCAPE) {
            window.close();
            evt.handled = true;
        } else if (evt.virtual_key == VK_RETURN && (GetKeyState(VK_CONTROL) & 0x8000)) {
            // Ctrl+Enter to submit
            btn_submit.click();
            evt.handled = true;
        }
    });
    
    // Handle resize
    window.on_size([](SizeEvent& evt) {
        if (evt.type == SizeEvent::Type::Minimized) return;
        
        int width = evt.new_size.width;
        int height = evt.new_size.height;
        
        // Adjust layout for new size
        if (lbl_title.valid()) {
            lbl_title.set_bounds({20, 10, width - 40, 35});
        }
        
        if (txt_output.valid()) {
            txt_output.set_bounds({230, 190, width - 250, height - 280});
        }
        
        if (btn_submit.valid()) {
            btn_submit.set_bounds({20, height - 70, 120, 35});
        }
        
        if (btn_clear.valid()) {
            btn_clear.set_bounds({150, height - 70, 120, 35});
        }
    });
    
    window.center_on_screen();
    window.show();

    return app.run();
}
