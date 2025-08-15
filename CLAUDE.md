# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Kaleidoscope is a flexible firmware framework for Arduino-powered keyboards, particularly Keyboardio keyboards. It uses a plugin-based architecture that allows users to customize keyboard behavior through modular components.

## Development Commands

### Build Commands
- **Compile a sketch**: `make compile` (from sketch directory)
- **Flash firmware**: `make flash` (from sketch directory)
- **Clean build**: `make clean`
- **Check code style**: `make check-code-style`
- **Format code**: `make format` (uses astyle)

### Preonic-Specific Commands
For the preonic_cesare configuration, there are convenience shell scripts:
- **Compile**: `./compile.sh` (sets KALEIDOSCOPE_DIR automatically)
- **Flash**: `./flash.sh` (compiles and flashes firmware)

### Testing
- **Run simulator tests**: `make simulator-tests`
- **Run specific test directory**: `make simulator-tests TEST_PATH=hid`
- **Run tests in Docker**: `make docker-simulator-tests`
- **Run individual test**: Navigate to test directory and run make

### Setup
- **Initial setup**: `make setup` (installs toolchain, arduino-cli, and platform support)
- **Update dependencies**: `make update`

## Architecture

### Core Components
- **src/Kaleidoscope.h**: Main header file that includes all core components
- **src/kaleidoscope/Runtime.{h,cpp}**: Core runtime that manages the plugin system and event dispatch
- **src/kaleidoscope/layers.{h,cpp}**: Layer management system for keymaps
- **src/kaleidoscope/hooks.{h,cpp}**: Event handler system that connects plugins

### Plugin System
Kaleidoscope uses an event-driven plugin architecture:
- Plugins inherit from `kaleidoscope::Plugin`
- Event handlers like `onKeyEvent()`, `beforeReportingState()`, `onFocusEvent()` allow plugins to respond to various events
- Plugins are registered via `KALEIDOSCOPE_INIT_PLUGINS()` macro in sketches
- Plugin order matters - earlier plugins process events first

### Device Support
- Hardware abstraction through device-specific classes in `src/kaleidoscope/device/`
- Platform-specific implementations in `plugins/Kaleidoscope-Hardware-*/`
- Supports AVR (ATmega32U4), ARM (SAMD, GD32), and virtual (simulator) platforms

### Key Concepts
- **KeyEvent**: Container for key state changes with `addr` (physical location), `state` (current/previous), and `key` (Key value)
- **EventHandlerResult**: Plugins return OK, ABORT, or EVENT_CONSUMED to control event flow
- **Layers**: Up to 32 keyboard layouts that can be activated/deactivated
- **Focus**: Serial protocol for host communication (configuration, debugging)

## Directory Structure
- **examples/**: Example sketches demonstrating features and device support
- **plugins/**: Individual plugin modules (LED effects, macros, etc.)
- **src/**: Core Kaleidoscope source code
- **testing/**: Test infrastructure and GoogleTest integration
- **tests/**: Test cases using .ktest format
- **docs/**: Documentation (Sphinx-based)
- **bin/**: Build and development scripts

## Working with Sketches
- Sketches require a `sketch.yaml` or `sketch.json` file specifying the FQBN (Fully Qualified Board Name)
- Main sketch file must match directory name (e.g., `MySketch/MySketch.ino`)
- Use absolute paths in Makefiles when needed
- The build system uses arduino-cli under the hood

## Important Notes
- Always check existing code style and conventions before making changes
- Plugin initialization order can affect behavior significantly
- The simulator is used for automated testing and doesn't support all hardware features
- When creating new plugins, follow the patterns in existing plugins
- Focus commands use a simple text protocol: `command [args]\n`