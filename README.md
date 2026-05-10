# STM32 Two-Axis Machine Control

STM32-based embedded control project for a two-axis machine driven by two stepper motors. The system uses potentiometer-based ADC speed control, interrupt-driven limit switch safety, SPI motor-driver communication, UART manual control, and a Python GUI for direct user operation.

This project was completed for MTE 325 and focused on applying embedded systems concepts to a real hardware platform: peripheral configuration, polling vs interrupts, ADC characterization, motor control, UART communication, and system-level safety handling.

![Two-Axis Machine](Machine_img1.HEIC)

## Demo

Demo folder:  
https://drive.google.com/drive/folders/1DSZOzzM9O2JOJqB40VQ0yBSDtRi_iyZa?usp=sharing

Demo videos:
- Demo 1: https://drive.google.com/file/d/1I3mCT2xIy3bBejYWqJA6apu2jjCAlniW/
- Demo 2: https://drive.google.com/file/d/1OqJbgLzihHthyge3sKhTlL_7apw86IUl/
- Demo 3: https://drive.google.com/file/d/1VmHeQlyNQaXavxj6MBrBOQC8uks0vpJQ/

## Tech Stack

- Microcontroller: STM32 Nucleo-F401RE
- Motor driver shield: X-NUCLEO-IHM02A1
- Motor driver IC: L6470 stepper motor driver
- Programming language: C
- Development environment: VS Code + PlatformIO
- Firmware libraries: STM32 HAL, L6470 motor driver library
- Communication interfaces: SPI, UART, GPIO EXTI, ADC
- PC control tool: Python GUI
- Python libraries/tools: PySerial, Tkinter
- Debug/test tools: serial terminal, oscilloscope, digital multimeter, bench power supply

## Project Goals

- Control a two-axis machine using two independently driven stepper motors
- Use ADC readings from potentiometers to control motor speed in real time
- Use limit switches to prevent the machine from driving past physical boundaries
- Compare polling and interrupt-based synchronization for embedded response timing
- Implement safe direction changes using hard stops before reversing motor direction
- Add UART-based manual control using serial commands and a Python GUI
- Handle edge cases such as startup at a limit switch, switch bounce, and fault conditions
- Document the hardware wiring, pin mapping, control logic, and testing process clearly

## What We Built

The final system combines automatic and manual control modes.

In normal mode, both motors run continuously within the machine boundaries. Each axis has its speed controlled by a potentiometer connected to the STM32 ADC. When a limit switch is triggered, the firmware commands the affected motor to move away from the switch, keeping the platform inside the safe motion range.

In manual mode, a Python GUI sends UART commands to the STM32 so the user can move or stop each axis directly. The system still respects the limit switches in manual mode, so the user cannot intentionally drive the platform past its mechanical limits.

The firmware includes:
- ADC scan conversion for two potentiometers
- Real-time speed mapping from ADC values to motor speed commands
- GPIO external interrupts for four normally closed limit switches
- SPI communication with the X-NUCLEO-IHM02A1 motor driver shield
- UART command parsing for manual control
- Fault detection if both switches on the same axis are triggered
- Fault LED output and reset-based recovery
- Direction tracking variables to prevent synchronization conflicts between ADC updates and limit-switch interrupts

## Hardware and Materials

### Main hardware

- STM32 Nucleo-F401RE development board
- X-NUCLEO-IHM02A1 motor driver shield
- 2-axis machine platform
- 2 stepper motors
  - Motor 0: horizontal axis
  - Motor 1: vertical axis
- 2 potentiometers
  - Potentiometer A: horizontal-axis speed control
  - Potentiometer B: vertical-axis speed control
- 4 normally closed limit switches
  - X-left
  - X-right
  - Y-top
  - Y-bottom
- Fault LED
- 1 kΩ resistor for the LED circuit
- Breadboard
- Jumper wires
- Micro-USB cable for programming and UART communication
- External DC power supply

### Test and debugging equipment

- Oscilloscope
- Digital multimeter
- Signal generator
- Bench power supply
- PC/laptop running VS Code, PlatformIO, and Python

## System Architecture

The STM32 reads two potentiometers through ADC1 and uses the readings to update motor speed. Limit switches are connected as GPIO external interrupts so the system can quickly respond to unsafe travel limits. Motor commands are sent to the X-NUCLEO-IHM02A1 shield over SPI. UART is used for serial communication between the STM32 and the Python manual-control GUI.

A full connection block diagram is included in the repository:

- `Connection_block_diagram.pdf`
- `Connection_block_diagram/`

![Connections](Connections.HEIC)

## Pin Mapping

### ADC inputs

| Function | STM32 Pin | ADC Channel | Purpose |
|---|---:|---:|---|
| Potentiometer A | PA0 / A0 | ADC1_CH0 | Horizontal-axis speed control |
| Potentiometer B | PB0 / A3 | ADC1_CH8 | Vertical-axis speed control |

ADC configuration:
- ADC peripheral: ADC1
- Resolution: 8-bit
- Conversion mode: scan conversion, 2 channels
- Sampling time: 15 cycles
- ADC clock: PCLK2 / 4 = 21 MHz
- ADC raw range: 0 to 255

### Limit switches

| Limit Switch | STM32 Pin | EXTI Line | Action |
|---|---:|---:|---|
| X-left | PA8 / D7 | EXTI8 | Reverse/stop horizontal axis away from left limit |
| X-right | PA9 / D8 | EXTI9 | Reverse/stop horizontal axis away from right limit |
| Y-top | PA10 / D2 | EXTI10 | Reverse/stop vertical axis away from top limit |
| Y-bottom | PB6 / D10 | EXTI6 | Reverse/stop vertical axis away from bottom limit |

Limit switch configuration:
- Normally closed switches
- 3.3 V bias
- Falling-edge interrupt trigger
- Pulldown configuration used in firmware
- Debounce handled in software before accepting a switch event

### Fault LED

| Function | STM32 Pin | Purpose |
|---|---:|---|
| Fault LED | PB8 / D15 | Turns on when a hard fault is detected |

The LED turns on when `fault_detected = true`. The system must be reset to clear the fault.

### UART manual control

| Function | STM32 Pin | Setting |
|---|---:|---|
| USART2 TX | PA2 | 115200 baud |
| USART2 RX | PA3 | 115200 baud |

UART settings:
- Baud rate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: None
- Flow control: None

### SPI motor-driver interface

The STM32 communicates with the X-NUCLEO-IHM02A1 shield through SPI. The shield uses L6470 stepper motor driver ICs in a daisy-chain configuration.

Common motor commands used:
- `Run`
- `Stop`
- `HardStop`
- Direction/status checks
- L6470 status-register reads

## Motor Control

### Axis mapping

| Motor | Axis | Mechanism | Speed Range |
|---|---|---|---:|
| Motor 0 | Horizontal axis | Belt drive | 0 to 7,000 |
| Motor 1 | Vertical axis | Lead screw | 0 to 70,000 |

The two speed ranges are different because the horizontal and vertical mechanical systems behave differently. The horizontal belt-driven axis moves faster at lower command values, while the vertical lead-screw axis requires a larger speed command range for practical motion.

### ADC-to-speed mapping

The potentiometers produce analog voltages that are converted into 8-bit ADC values from 0 to 255.

The firmware maps these ADC values into motor speed commands:

```c
horizontal_speed = ADC_x * (7000 - 0) / 255;
vertical_speed   = ADC_y * (70000 - 0) / 255;
