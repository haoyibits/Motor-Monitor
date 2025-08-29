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

## OLED UI Library Usage

The project integrates a professional OLED UI library for motor monitoring interface, providing a complete menu-driven user experience.

### Motor Monitor UI Design

**Main Interface (Tile-based Navigation)**:
- **Motor Settings** - PWM duty cycle, enable/disable, direction control
- **Overcurrent Protection** - Current limit settings and protection enable
- **Device Info** - Hardware specifications and system information

### UI Architecture

**File Structure**:
```
Drivers/OLED_UI_Core/
├── Driver/
│   ├── Hardware_Driver/    # Low-level OLED communication (I2C-based)
│   └── Software_Driver/    # Graphics engine and font rendering
├── OLED_UI/               # High-level UI framework
│   ├── OLED_UI.h/c           # Core UI engine and menu system
│   ├── OLED_UI_MenuData.h/c  # Motor monitor menu definitions
└── OLED_UI_Launcher.h/c   # UI initialization and integration
```

### Menu System Implementation

**Main Menu Configuration**:
```c
MenuItem MainMenuItems[] = {
    {.General_item_text = "Motor Settings", .Tiles_Icon = Image_motor},
    {.General_item_text = "Overcurrent Protection", .Tiles_Icon = Image_protection},
    {.General_item_text = "Device Info", .Tiles_Icon = Image_device_info},
    {.General_item_text = NULL}  // Terminator
};
```

**Interactive Windows**:
- **PWM Duty Cycle Window**: Float slider (0-100%) with progress bar
- **Current Limit Window**: Float slider (0.1-5.0A) with visual feedback
- **Brightness Window**: Integer adjustment (5-255) for display control

### Custom Icons

**Motor Monitor Specific Icons (32x32)**:
- `Image_motor[]` - Circular motor with center shaft representation
- `Image_protection[]` - Shield-based protection symbol with electrical elements
- `Image_device_info[]` - Chip outline with information indicators

### UI Integration

**Initialization Sequence**:
1. Initialize OLED hardware driver (I2C-based SSD1309)
2. Setup UI timer integration with SysTick system
3. Configure button input mapping (4-button navigation)
4. Load main menu page and start UI main loop

**Event Handling**:
- **Button Navigation**: Up/Down for selection, Enter for confirm, Back for return
- **Long Press Support**: Accelerated navigation for numeric adjustments
- **Smooth Animations**: PID-curve based transitions between menu items
- **Real-time Updates**: Motor parameters reflected instantly in UI

### UI Performance Features

- **Tile-based Main Menu**: Horizontal scrolling with smooth animations
- **List-based Submenus**: Vertical navigation with cursor highlighting
- **Double-buffered Rendering**: Smooth visual updates without flicker
- **Partial Screen Updates**: Efficient rendering for better performance
- **Font System**: Multiple sizes (8px, 12px, 16px, 20px) with Chinese support

### Integration with Motor Control

**Variable Binding**:
```c
float motor_pwm_duty = 50.0f;      // PWM duty cycle (0-100%)
float motor_current_limit = 2.0f;   // Current limit (A)
bool motor_enabled = false;         // Motor enable state
bool motor_direction = false;       // Motor direction (CW/CCW)
```

**Real-time Monitoring**:
- UI automatically reflects changes in motor control variables
- Interactive sliders provide immediate feedback
- Protection settings are enforced in real-time motor control logic

### Usage Guidelines

**Adding New Menu Items**:
1. Define MenuItem structure with appropriate callbacks
2. Create MenuWindow for interactive controls if needed
3. Add menu page configuration with proper navigation hierarchy
4. Update menu data arrays and ensure proper termination

**Custom Window Creation**:
```c
MenuWindow CustomWindow = {
    .General_Width = 80,
    .General_Height = 28,
    .Text_String = "Parameter Name",
    .Text_FontSize = OLED_UI_FONT_12,
    .General_WindowType = WINDOW_ROUNDRECTANGLE,
    .Prob_Data_Float = &your_variable,
    .Prob_DataStep = 0.1f,
    .Prob_MinData = 0.0f,
    .Prob_MaxData = 100.0f
};
```

**Timer Integration**:
- UI refresh rate: 20ms (50Hz) via SysTick interrupt
- Animation frame timing: Optimized for smooth 50fps performance
- Event polling: Integrated with main application event loop

This UI system provides a professional, industrial-grade interface suitable for motor monitoring and control applications, with full customization capabilities for specific hardware requirements.


# 代码重构整理计划

## 第一阶段：目录结构简化重组 (1.5小时)

### 1.1 创建简洁的目录结构
```
Software/
├── app/                    # 应用层
│   ├── inc/               # 应用头文件
│   └── src/               # 应用源文件
├── drivers/               # 硬件驱动层
│   ├── mcu_peripherals/   # MCU外设驱动 (原Register_base)
│   │   ├── inc/
│   │   └── src/
│   ├── external_devices/  # 外部设备驱动
│   │   ├── inc/
│   │   └── src/
│   ├── ui_core/          # UI驱动核心 (保持现有OLED_UI_Core)
│   └── cmsis/            # ARM CMSIS
├── ui/                   # UI界面层 (新增)
│   ├── ui_menu.h/c       # UI菜单定义和配置
│   └── ui_callback.h/c   # UI回调函数实现
└── config/
    └── board_config.h    # 硬件配置
    └── system_includes.h # 集中头文件管理
```

### 1.2 文件迁移方案
- `Inc/` + `Src/` → `app/inc/` + `app/src/`
- `Drivers/Register_base/` → `drivers/mcu_peripherals/`
- `Drivers/OLED_UI_Core/` → `drivers/ui_core/` (保持不变)
- 从现有UI代码中提取菜单配置 → `ui/ui_menu.h/c`
- 从现有UI代码中提取回调函数 → `ui/ui_callback.h/c`

### 1.3 集中头文件管理
- 创建 `config/system_includes.h` 作为主头文件
- 在 `system_includes.h` 中包含所有必要的头文件：
  - CMSIS标准头文件
  - MCU外设驱动头文件  
  - 外部设备驱动头文件
  - UI核心头文件
  - 应用层头文件
- 所有 `.c` 文件只需包含 `#include "system_includes.h"`
- 简化依赖管理，避免重复包含问题

## 第二阶段：UI层分离重构 (2小时)

### 2.1 UI菜单配置分离 (`ui/ui_menu.h/c`)
- 提取所有MenuItem数组定义
- 提取MenuWindow配置
- 提取菜单层次结构定义
- 提取图标和资源定义
- **只包含数据结构和配置，无业务逻辑**

### 2.2 UI回调函数分离 (`ui/ui_callback.h/c`)
- 提取所有菜单项回调函数
- 提取数值调节回调
- 提取按键处理回调
- 提取与应用层交互的接口函数
- **包含所有UI相关的业务逻辑处理**

### 2.3 UI驱动层保持不变 (`drivers/ui_core/`)
- OLED硬件驱动保持原样
- 图形引擎保持原样
- UI核心引擎保持原样
- 仅修改对菜单配置和回调的引用路径

## 第三阶段：应用层和驱动层整理 (2小时)

### 3.1 应用层代码整理
- **main.c**: 保持简洁，添加清晰的模块初始化顺序
- **event.c**: 整理事件处理，分类组织
- **motor.c**: 电机控制逻辑优化
- **irq.c**: 中断处理优化

### 3.2 MCU驱动层规范化
- 统一驱动接口命名风格
- 添加英文doxygen注释
- 清理未使用的函数
- 确保每个外设模块独立

### 3.3 硬件配置集中化
- 创建`config/board_config.h`
- 从bsp.h中提取纯硬件配置
- 引脚定义、时钟配置集中管理

## 第四阶段：命名规范统一 (1小时)

### 4.1 目录和文件命名
- 统一使用`snake_case`
- 重命名不规范的目录和文件

### 4.2 代码命名规范
- 函数: `module_function_name()`
- 变量: `snake_case`  
- 统一注释为英文

## 第五阶段：构建系统更新 (0.5小时)

### 5.1 更新CMakeLists.txt
- 修复模板变量和拼写错误
- 更新新的目录路径
- 添加新的UI层文件

### 5.2 更新文档
- 更新CLAUDE.md中的架构说明
- 反映UI层分离的设计

## 架构特点

**简化的三层结构**：
```
应用层 (app/)         ← 业务逻辑，直接调用驱动
    ↓
驱动层 (drivers/)     ← 寄存器级硬件操作 + UI驱动
    ↓  
CMSIS层              ← ARM标准定义
```

**UI层分离设计**：
- `drivers/ui_core/`: UI驱动和图形引擎 (不变)
- `ui/ui_menu`: UI界面配置和布局 (配置层)
- `ui/ui_callback`: UI业务逻辑处理 (逻辑层)

**核心设计原则**：
- 应用层直接访问寄存器驱动，无抽象层开销
- UI驱动最小化改动，保持稳定
- 配置和回调分离，便于维护
- 保持bare-metal的性能优势

## 预期成果

- **UI层清晰分离**: 配置和回调分开，便于维护
- **最小化UI驱动改动**: 保持现有UI驱动稳定
- **统一代码规范**: 项目范围内的命名和注释统一
- **优化的目录结构**: 三层架构清晰，职责明确

估计总用时：6-7小时，重点关注UI层分离，最小化对现有UI驱动的影响。

