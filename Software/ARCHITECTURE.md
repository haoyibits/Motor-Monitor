# Application and register-driver architecture

The C1 refactor keeps all register-level peripheral implementations in
`Drivers/Register_base` unchanged. It separates application policy from hardware
effects so UI behavior can be tested and changed without rewriting MCU drivers.

## Dependency direction

```text
main / event adapter
        |
        +--> Application (event queue, config transaction, motor state)
        +--> OLED UI integration
        +--> motor service / BSP
                    |
                    +--> register drivers --> CMSIS
```

`motor.c` reports protection notifications through a callback. The application
adapter converts those notifications into queued application events, and only
that adapter selects an OLED popup. The motor layer therefore has no dependency
on menu data or the OLED UI implementation.

## Runtime rules

- The SysTick ISR only advances the millisecond clock.
- The main loop scans debounced input, posts due work, and drains the event queue.
- Current processing is posted before encoder and UI work.
- OLED input processing and rendering run from one 50 Hz application event.
- Queue overflow is counted; posting never blocks.

## Configuration editing contract

- Opening an editable window copies `motor_config` into a draft transaction.
- Up/Down modifies a typed UI value, never the live persisted structure.
- Enter validates, commits, and writes the configuration to flash once.
- Back or timeout cancels the draft and restores the displayed live value.
- Integer UI values use 32-bit storage, so the 60000 ms restart delay is valid.

## C1 boundary

C1 does not change peripheral register programming, pin assignments, ADC-DMA
initialization order, PWM timer calculations, flash commands, interrupt vectors,
or the linker/startup files. Those are candidates for a separately reviewed C2.

## C2 register-driver contract

- Register drivers include their own header and direct dependencies; they never
  include the application-level `bsp.h` umbrella.
- Fallible driver operations return `DriverStatus`. Timeout, invalid argument,
  bus busy, I/O failure, and device-not-ready are distinct outcomes.
- SPI, UART, DMA, I2C, and flash polling is bounded. A missing peripheral can no
  longer trap startup or configuration save in an infinite loop.
- The RCC configuration names the PLL source explicitly, validates clock-tree
  limits, switches through HSI while reconfiguring, and keeps derived clocks in
  sync with the actual AHB/APB dividers.
- W25Q128 operations use the configuration passed by the caller, validate flash
  and page boundaries, verify WEL, wait internally once, and propagate failures
  to the motor configuration service.
- I2C follows the STM32F4 one-, two-, and multi-byte receive sequences and can
  pulse SCL/reinitialize the peripheral after a stuck BUSY condition.
- DMA stream flags use the STM32F4's irregular flag layout. Burst settings are
  written to `DMA_SxCR`, not the FIFO control register.
- ADC averaging consumes the completed DMA half-buffer in the ISR, so the main
  loop no longer reads a buffer while DMA is rewriting it.
- Encoder position is accumulated with counter-wrap correction; direction is
  zero when no movement was measured.

## Hardware validation checklist

1. Boot with valid and erased flash; confirm safe motor-disabled startup.
2. Navigate Main > Motor Settings > PWM Control and PID Speed Control.
3. For each value window, verify Up/Down, Enter commit, Back cancel, and timeout cancel.
4. Set restart delay above 32767 ms and reboot to verify persistence.
5. Select Disabled, STM32, and FPGA sources and verify enable/direction behavior.
6. Trigger overcurrent and verify fault, restart, and maximum-attempt notifications.
7. Hold each button and check debounce, navigation rate, and UI responsiveness.
8. Disconnect the OLED and flash separately; verify startup and UI remain live.
9. Force an I2C BUSY/SDA-low condition and verify the recovery clock pulses.
10. Run the encoder through counter rollover in both directions and verify RPM.
11. Test both HSI and an actual board HSE configuration before enabling HSE in BSP.
