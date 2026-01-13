# W20PP: A Modern C++20 Windows UI Framework

**Version 0.3.0** - Phase 3: UI Controls (In Progress)

## The Problem

MFC is obsolete, Qt is bloated (150MB+ dependencies), and Win32++ hasn't evolved past C++14. Windows developers lack a lightweight, modern option.

## The Solution

A header-only C++20/23 framework that provides:

- **Pure RAII Design**: No macros, no exceptions, automatic resource management
- **Declarative UI Syntax**: C++20 designated initializers for clean, readable code
- **Single-Header Distribution**: Zero external dependencies, just include and go
- **Modern Features**: Dark mode, DPI awareness, type-safe events, UI controls
- **Performance First**: Compile times in seconds, not minutes

## Current Status

### Phase 1: Foundation ✓ Complete

- Application class with message loop
- RAII Window wrapper with move semantics
- Error handling using `std::expected<T, Error>`
- DPI awareness (per-monitor v2)
- Dark mode support

### Phase 2: Window Management ✓ Complete

- Comprehensive event system (mouse, keyboard, lifecycle)
- Window state management (minimize, maximize, restore)
- Multi-monitor support with DPI handling
- Custom window classes
- Window styling and transparency

### Phase 3: UI Controls (In Progress - 75% Complete)

- **GDI Resource Management**: RAII wrappers for Font, Brush, Pen, DC
- **Button Control**: Push buttons with default button support
- **Label Control**: Static text with alignment options
- **TextBox Control**: Single/multi-line input, password mode, read-only
- **CheckBox Control**: Standard and tri-state checkboxes
- **RadioButton Control**: Mutually exclusive option selection
- **ComboBox Control**: Dropdown lists with multiple styles
- **ListBox Control**: Single and multi-select lists
- **Base Control Class**: Common functionality for all controls
- Coming soon: ProgressBar, Slider, GroupBox, and more

## Quick Example

```cpp
#include <w20pp/w20pp.hpp>

int main() {
    using namespace w20pp;

    Application app;

    // Create window with declarative syntax
    auto result = app.create_window({
        .title = L"Hello W20PP",
        .size = {800, 600},
        .theme = Theme::System
    });

    if (!result) return 1;
    auto& window = result.value();

    // Create UI controls (after window creation)
    static Button button;
    static Label label;
    static TextBox textbox;
    HWND hwnd = window.handle();

    // Create a label
    auto lblResult = Label::create(hwnd, {
        .bounds = {20, 20, 200, 25},
        .text = L"Enter your name:",
        .id = 1
    });
    if (lblResult) label = std::move(lblResult.value());

    // Create a text input
    auto txtResult = TextBox::create(hwnd, {
        .bounds = {20, 50, 250, 25},
        .id = 100
    });
    if (txtResult) textbox = std::move(txtResult.value());

    // Create a button
    auto btnResult = Button::create(hwnd, {
        .bounds = {20, 90, 100, 30},
        .text = L"Greet",
        .id = 200,
        .default_button = true
    });
    if (btnResult) button = std::move(btnResult.value());

    // Handle button clicks
    window.on_command([](CommandEvent& evt) {
        if (evt.id == 200) {
            std::wstring name = textbox.get_text();
            show_message_box(
                std::format(L"Hello, {}!", name),
                L"Greeting"
            );
        }
    });

    window.show();
    window.center_on_screen();

    return app.run();
}
```

## Key Features

### Type-Safe Event System

```cpp
window.on_close([](CloseEvent& evt) {
    evt.cancel = true;  // Prevent closing
})
.on_size([](SizeEvent& evt) {
    // Resize handling with typed parameters
})
.on_key_down([](KeyEvent& evt) {
    if (evt.virtual_key == VK_ESCAPE) {
        // Handle Escape key
    }
});
```

### RAII Resource Management

```cpp
// Automatic cleanup, no manual resource management
auto font = Font::create(16, FW_BOLD, false, false, L"Segoe UI");
auto brush = Brush::solid(RGB(100, 150, 200));
auto pen = Pen::solid(2, RGB(255, 0, 0));

button.set_font(font.handle());
// Resources automatically released
```

### Multi-Monitor & DPI Support

```cpp
// Enumerate all monitors
auto monitors = get_all_monitors();
for (const auto& mon : monitors) {
    // mon.dpi, mon.bounds, mon.work_area
}

// DPI-aware scaling
int scaled = window.scale_value(16);  // Scales with DPI
window.on_dpi_changed([](DpiChangedEvent& evt) {
    // Handle DPI changes automatically
});
```

## Building

### Requirements

- Windows 10 or later
- C++20 compatible compiler (MSVC 2019+, Clang 12+, GCC 11+)
- CMake 3.20+

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/w20pp.git
cd w20pp

# Build with CMake
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run examples
./bin/Release/ui_controls.exe
./bin/Release/event_handling.exe
```

## Documentation

- [Phase 2 Guide](docs/PHASE2_GUIDE.md) - Window management and events
- [Phase 3 Guide](docs/PHASE3_GUIDE.md) - UI controls and GDI resources
- [Development Plan](docs/PLAN.md) - Roadmap and progress
- [Examples](examples/) - Sample applications

## Why W20PP?

| Feature       | W20PP       | MFC         | Qt      | Win32++     |
| ------------- | ----------- | ----------- | ------- | ----------- |
| C++ Standard  | C++20/23    | C++98       | C++17   | C++11       |
| Distribution  | Header-only | Library     | 150MB+  | Header-only |
| Dependencies  | Zero        | Windows SDK | Many    | Windows SDK |
| RAII          | Full        | Partial     | Full    | Partial     |
| Compile Time  | Seconds     | Minutes     | Minutes | Seconds     |
| Type Safety   | Strong      | Weak        | Strong  | Medium      |
| Modern Syntax | Yes         | No          | Partial | No          |

## Roadmap

- **Phase 4**: Layout system (Flow, Grid, Flexbox-like)
- **Phase 5**: Styling & theming system
- **Phase 6**: Graphics & rendering (Direct2D integration)
- **Phase 7**: Advanced input handling
- **Phase 8**: Accessibility support
- **Phase 9**: Data binding & MVVM
- **Phase 10**: Platform integration (notifications, system tray)

## Contributing

Contributions are welcome! This is an active development project. See [PLAN.md](docs/PLAN.md) for current priorities.

## License

MIT License - See [LICENSE](LICENSE) file for details.

---

**Status**: Active Development | **Version**: 0.3.0 | **Phase**: 3 of 12
