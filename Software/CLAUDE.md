# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a bare-metal STM32F407VGT6 motor monitoring system. The project is designed for embedded systems learning and experimentation without HAL or LL libraries - only pure C with register-level programming.

## Build System

This project uses CMake with ARM GCC toolchain:

```bash
# Configure build (from project root)
cmake -B build

# Build the project
cmake --build build

# Alternative: use make in build directory
cd build && make
```

Build outputs:
- `motor_monitor.elf` - Main executable
- `motor_monitor.hex` - Intel HEX format for programming
- `motor_monitor.bin` - Binary format
- `motor_monitor.map` - Memory map file

## Architecture

### Core Structure
- **Bare-metal approach**: No HAL/LL libraries, direct register programming
- **Modular design**: Hardware drivers in `Drivers/Register_base/`
- **Event-driven**: Main loop handles scanning and event processing
- **Real-time monitoring**: ADC-DMA for continuous current sensing

### Key Components

1. **Board Support Package (BSP)**: `Inc/bsp.h`, `Src/bsp.c`
   - Central hardware configuration
   - System initialization sequence
   - Hardware pin definitions

2. **Register-based Drivers**: `Drivers/Register_base/`
   - Low-level hardware abstraction
   - Each peripheral has separate .h/.c files
   - Direct register manipulation for optimal performance

3. **Event System**: `Inc/event.h`, `Src/event.c`
   - Motor control state machine
   - Button handling
   - Periodic scanning and monitoring

4. **Main Application**: `Src/main.c`
   - Simple initialization and main loop
   - SEGGER RTT debug output

### Hardware Configuration

**Target MCU**: STM32F407VGT6
- Clock: Configurable HSI (16MHz) or HSE (8MHz external crystal)
- Motor control: Motor enable + direction pins (PB0, PB1, PE7)
- Encoder: Quadrature encoder on TIM2 (PA2, PA3)
- Current sensing: ADC1 with DMA on PA0
- User interface: 4 buttons on PE9-PE12
- Communication: UART2 on PD5/PD6 for FPGA interface
- OLED Display: I2C-based SSD1306 for user interface
- Debug: SEGGER RTT for real-time terminal

**Critical ADC-DMA Sequence**: The ADC-DMA initialization requires a specific sequence:
1. Configure DMA before ADC
2. Enable DMA stream before ADC
3. Set ADC_CR2_DDS bit for continuous DMA requests
4. Start ADC conversion last

### File Organization

```
Inc/                    # Application headers
├── bsp.h              # Board support package
├── event.h            # Event handling system
└── irq.h              # Interrupt handlers

Src/                    # Application source
├── main.c             # Entry point
├── bsp.c              # BSP implementation  
├── event.c            # Event system
├── irq.c              # Interrupt handlers
├── syscall.c          # System calls
└── sysmem.c           # Memory management

Drivers/Register_base/  # Hardware drivers
├── Inc/               # Driver headers
└── Src/               # Driver implementations

Drivers/CMSIS/         # ARM CMSIS and STM32 device files
├── RTT/               # SEGGER RTT for debug output

Driver/OLED_UI_Core/  # Advanced OLED UI library (HAL-based)

```

## Development Workflow

1. **Modify hardware drivers** in `Drivers/Register_base/` for new peripherals
2. **Update BSP configuration** in `bsp.h`/`bsp.c` for pin assignments
3. **Add event handlers** in `event.c` for new functionality
4. **Use SEGGER RTT** for debug output instead of printf
5. **Test with build** after any changes to ensure compilation

## Git Configuration

This is haoyibits' personal project. All commits must use the following identity:
- **Name**: haoyibits
- **Email**: haoyi.chen@studenti.polito.it

When creating commits, always ensure they are authored by the project owner, not AI assistants. The local Git configuration has been set to use the correct identity.

## Debug and Monitoring

- **SEGGER RTT**: Real-time terminal output (Channel 0)
- **Current monitoring**: 200-sample ADC buffer with DMA
- **Encoder feedback**: Position and speed calculation
- **Button states**: Debounced input handling

## Important Notes

- **No HAL dependency**: All drivers are register-based implementations
- **Real-time constraints**: ADC sampling at ~500kHz, avoid blocking operations
- **Power efficiency**: Designed for continuous monitoring applications
- **Safety features**: Current threshold protection, emergency stop capability
- **Comments requirement**: When you need to add comments, it should be in english with doxygen style comments.

## OLED UI Library

The project includes an advanced OLED UI library located in `Drivers/OLED_UI_Core/` that provides comprehensive menu-driven interface capabilities.

### OLED_UI_Core Architecture

**Library Structure**: `Drivers/OLED_UI_Core/HAL/OLED_UI_Core/`
```
Driver/
├── Hardware_Driver/    # Low-level OLED communication
│   ├── OLED_driver.h/c     # SSD1306/SH1106 drivers with SPI support
│   └── OLED_UI_Driver.h/c  # Hardware abstraction layer
└── Software_Driver/    # Graphics and text rendering
    ├── OLED.h/c           # Display buffer and drawing functions
    └── OLED_Fonts.h/c     # Font rendering system

OLED_UI/               # High-level UI framework
├── OLED_UI.h/c           # Menu system and event handling
├── OLED_UI_MenuData.h/c  # Menu structure definitions
└── misc.h/c              # Utility functions
```

### Key Features

1. **Multi-level Menu System**
   - Hierarchical menu navigation with breadcrumbs
   - Configurable menu items with icons and actions
   - Smooth scrolling animations and transitions
   - Support for radio buttons, functions, and submenus

2. **Rich Graphics Engine**
   - Multiple font sizes (8px, 12px, 16px, 20px)
   - Chinese character support with mixed text rendering
   - Geometric shapes: rectangles, circles, triangles, arcs
   - Bitmap image display with area clipping
   - 3D cube rendering capabilities

3. **Input System**
   - 4-button navigation (Up/Down/Enter/Back)
   - Encoder support for smooth selection
   - Long press detection with customizable timing
   - Interrupt-based input handling via Timer1

4. **Display Features**
   - 128x64 SSD1306/SH1106 OLED support
   - Double-buffered rendering for smooth updates
   - Dark/Light mode switching
   - Brightness control
   - Partial screen updates for efficiency

### Current Implementation (HAL-based)

The library currently uses STM32 HAL and is configured for **SPI communication**:
- Hardware SPI with DMA support
- GPIO pins for DC, Reset, and CS control
- Timer-based UI refresh (TIM1 interrupt)
- HAL GPIO functions for button input

### Migration Requirements for Register-Based I2C

To integrate with this project's bare-metal architecture:

1. **Hardware Driver Adaptation** (`OLED_driver.h/c`):
   - Replace HAL SPI calls with register-based I2C using `i2c_oled.h`
   - Remove SPI-specific pins (MOSI, CLK, CS)
   - Adapt for I2C address-based communication (0x3C/0x3D)
   - Update command/data sending to use I2C protocol

2. **Hardware Abstraction** (`OLED_UI_Driver.h/c`):
   - Replace HAL button reading with register-based GPIO
   - Integrate with existing button driver (`button.h`)
   - Replace HAL timer with existing SysTick or TIM implementation
   - Adapt encoder interface to use existing encoder driver

3. **System Integration**:
   - Remove HAL dependencies from all source files
   - Update includes to use STM32F407xx register definitions
   - Integrate UI timer with existing event system
   - Adapt delay functions to use existing implementations

### Migration Benefits

- **Feature-rich UI**: Professional menu system with animations
- **Proven codebase**: Mature library with extensive graphics capabilities
- **Consistent architecture**: Maintains separation between hardware and software layers
- **Performance optimized**: Double-buffered rendering and partial updates

## Software Timer Architecture

The project implements an excellent software timer system using SysTick that serves as the foundation for all periodic tasks.

### Timer System Design

**Core Components**:
- **SysTick Timer**: 1ms interrupt frequency with minimal ISR overhead
- **Software Timer Structure**: Lightweight timer objects with auto-reload capability
- **Event-driven Architecture**: Main loop polling with timer expiration checks

**File Organization**:
```
Drivers/Register_base/
├── systick.h/c           # Core SysTick implementation (1ms base)
Src/
├── irq.c                 # Hardware interrupt handlers (minimal processing)
├── event.c               # Application event handlers (business logic)
└── main.c                # Main loop scheduling (scan_check)
```


### Architecture Principles

**Interrupt Handling**:
- **Fast ISR**: SysTick handler contains only `system_tick_ms++`
- **Deferred Processing**: Complex logic in main loop handlers
- **Priority Ordering**: Critical tasks execute before UI updates

**Code Organization**:
- **irq.c**: Hardware interrupt responses only
- **event.c**: Application logic and state management  
- **Separation of Concerns**: Clear distinction between hardware and software layers



