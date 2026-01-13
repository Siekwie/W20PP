# W20PP Changelog

All notable changes to this project will be documented in this file.

## [0.3.0] - 2026-01-13

### Phase 3: UI Controls & Components (75% Complete)

#### Added - GDI Resource Management
- **Font**: RAII wrapper for HFONT with automatic cleanup
  - `Font::create()` - Create custom fonts with weight, style, face
  - `Font::system_font()` - Get default system font
- **Brush**: RAII wrapper for HBRUSH
  - `Brush::solid()` - Create solid color brushes
  - `Brush::system()` - Get system brushes
- **Pen**: RAII wrapper for HPEN
  - `Pen::create()` - Create pens with style, width, color
  - `Pen::solid()` - Create solid pens
- **DeviceContext**: RAII wrapper for HDC
  - `DeviceContext::get()` - Get DC for window with automatic release
- **Scoped GDI Selection**: Automatic object restoration
  - `ScopedFont`, `ScopedBrush`, `ScopedPen` - RAII selection helpers
- **Color Utilities**: Type-safe color management
  - `Color::from_rgb()`, `Color::to_colorref()`
  - Common color constants (black, white, red, green, blue, etc.)

#### Added - Base Control Infrastructure
- **Control Base Class**: Common functionality for all UI controls
  - Visibility control (show, hide, is_visible)
  - Enable/disable state management
  - Text get/set operations
  - Geometry management (bounds, position, size)
  - Font management
  - Focus control
  - Control ID management
  - Move-only semantics for proper resource management
- **ControlConfig**: Base configuration structure
  - Bounds, text, visibility, enabled state, control ID
  - Used by all control types via inheritance

#### Added - UI Controls
- **Button Control**
  - Push buttons with text
  - Default button support (activated by Enter key)
  - Programmatic click support
  - Check/uncheck for toggle functionality
  
- **Label Control**
  - Static text display
  - Alignment options (left, center, right)
  - Single-line and multi-line support
  - Custom font support
  
- **TextBox Control**
  - Single-line and multi-line text input
  - Password mode (character masking)
  - Read-only mode
  - Auto-scroll (horizontal and vertical)
  - Maximum length limiting
  - Text selection and manipulation
  - Undo support
  - Append text functionality
  
- **CheckBox Control**
  - Standard two-state checkboxes
  - Tri-state support (checked, unchecked, indeterminate)
  - State management with type-safe enum
  - Automatic state toggling
  
- **RadioButton Control**
  - Mutually exclusive option selection
  - Auto-radio mode (automatically unchecks siblings)
  - Manual radio mode for custom grouping
  - Checked state management
  
- **ComboBox Control**
  - Three styles: DropDown, Simple, DropDownList
  - Item management (add, insert, remove, clear)
  - Sorting support
  - Selection management
  - Item search functionality
  - Multi-select support
  
- **ListBox Control**
  - Single and multi-select modes
  - Sorted and unsorted lists
  - Item management (add, insert, remove, clear)
  - Selection management (single and multiple)
  - Item search functionality
  - Scrollbar support

#### Added - Examples
- **ui_controls.cpp**: Comprehensive interactive demo
  - Button, Label, TextBox demonstration
  - Event handling examples
  - Dynamic UI updates
  - Keyboard shortcuts
  - Window resizing with layout adjustment
  
- **advanced_controls.cpp**: Complete controls showcase
  - All 7 control types in one application
  - CheckBox (including tri-state)
  - RadioButton groups
  - ComboBox with multiple items
  - ListBox with multi-select
  - Form submission example
  - Real-time feedback

#### Added - Documentation
- **PHASE3_GUIDE.md**: Comprehensive Phase 3 documentation
  - GDI resource management guide
  - Control usage examples for all types
  - Event handling patterns
  - Best practices and common pitfalls
  - Complete working examples
  
- **CHANGELOG.md**: This file
  - Version history
  - Feature tracking
  - Breaking changes documentation

#### Updated
- **README.md**: 
  - Updated to version 0.3.0
  - Added Phase 3 progress (75% complete)
  - Enhanced quick example with UI controls
  - Updated feature comparison table
  
- **PLAN.md**:
  - Marked 7 basic controls as complete
  - Updated timeline estimates
  - Revised completion percentages
  - Added Phase 3 documentation tracking

#### Technical Improvements
- All controls use move-only semantics for proper resource management
- Consistent error handling with `Result<T>` types
- Type-safe event handling with specific event structures
- Declarative configuration using C++20 designated initializers
- Zero-overhead abstractions over Win32 APIs
- Comprehensive const-correctness
- [[nodiscard]] attributes for error-prone APIs

---

## [0.2.0] - Previous Release

### Phase 2: Window Management & Lifecycle (Complete)

#### Added
- Type-safe event system (mouse, keyboard, window lifecycle)
- Window state management (minimize, maximize, restore)
- Multi-monitor support with DPI handling
- Custom window classes
- Window styling and transparency
- Comprehensive event handlers
- Size constraints (min/max)
- Window layering and z-order control

---

## [0.1.0] - Initial Release

### Phase 1: Foundation (Complete)

#### Added
- Application class with message loop
- RAII Window wrapper with move semantics
- Error handling using `std::expected<T, Error>`
- DPI awareness (per-monitor v2)
- Dark mode support
- Basic window creation and management
- CMake build system
- Initial examples

---

## Roadmap

### Phase 4: Layout System (Planned)
- Flow layout (horizontal/vertical stacking)
- Grid layout (rows and columns)
- Flexbox-like layout
- Automatic sizing and positioning
- Margins and padding
- Alignment and anchoring

### Phase 5: Styling & Theming (Planned)
- Theme system (Light, Dark, Custom)
- Runtime theme switching
- Color palette management
- Custom styling API
- Visual effects (shadows, gradients)

### Phase 6: Graphics & Rendering (Planned)
- Direct2D integration
- 2D drawing API
- Custom paint handlers
- Hardware acceleration
- Image loading and rendering

### Phase 7: Advanced Input (Planned)
- Touch and gesture support
- Drag and drop
- Clipboard operations (enhanced)
- Keyboard shortcuts system

### Phase 8: Accessibility (Planned)
- Screen reader support
- Keyboard navigation
- High contrast mode
- WCAG compliance

---

## Version History

- **0.3.0** (2026-01-13): Phase 3 UI Controls - 75% Complete
- **0.2.0** (Previous): Phase 2 Window Management - Complete
- **0.1.0** (Initial): Phase 1 Foundation - Complete

---

## Breaking Changes

### 0.3.0
- None (additive release)

### 0.2.0
- Window creation now returns `Result<Window>` instead of `std::optional<Window>`
- Event handlers now use specific event types instead of generic parameters

### 0.1.0
- Initial release, no breaking changes
