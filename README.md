# RC Car – Embedded Wireless Control System
## Project Overview
This project demonstrates an entry-level systems and embedded software engineering design using two Raspberry Pi Picos. One Pico reads joystick input and wirelessly transmits control signals using the nRF24L01+ radio module. The second Pico receives the data and drives a four-wheel RC car using PWM-controlled motor drivers. Designed with modularity, signal integrity, and power isolation in mind, this project integrates embedded C programming, communication protocols, real-time motor control, and low-level hardware interfaces — showcasing practical skills in both system-level planning and embedded firmware development.

## System Architecture
### TX Side (Transmitter):

- Raspberry Pi Pico reads joystick X/Y (12-bit ADC)

- Sends 4-byte payload via SPI to nRF24L01+

- Powered by 7.4V 2S LiPo → 5V buck → VSYS

### RX Side (Receiver):

- Raspberry Pi Pico receives joystick values via nRF24L01+

- Controls four TT motors via two DRV8833 motor drivers

- Separate buck converters power logic and motor subsystems

### Communication:

- nRF24L01+ (SPI-based, CE pulsed to trigger TX, RX checks STATUS)

- 4-byte payload: 2 bytes X, 2 bytes Y (scaled 12-bit ADC values)

### Components
- 2x Raspberry Pi Pico

- 2x nRF24L01+ radio modules with breakout adapters

- 4x TT gear motors (3–6V)

- 2x DRV8833 dual H-bridge motor drivers

- 2x 2S LiPo battery (7.4V nominal)

- 2x Buck converters

- 100µF electrolytic + 0.1µF ceramic capacitors (on both TX & RX)

- Joystick module (2-axis analog + button)

- 3D printed chassis (STL and SLDPRT files included)

- 2x Red LED and 1x Green LED

- 3x 330 ohm resistor

- 2x JST Plug Connector Female

- 2x 1 to 2 Splicing connectors, 2x 1 to 3 Splicing connectors

- Female to male, male to male wires

## Functional Requirements
- Wireless transmission of joystick position from TX to RX

- Convert X/Y position into directional motor control

- Ensure proper power separation for logic vs motors

- Implement neutral zone and direction-switch lockout

- Maintain system stability under noisy conditions

## Interface Summary
| **Interface** | **Role** |
| :---: | :---: |
| SPI | nRF24L01+ communication |
| ADC | Read joystick values |
| GPIO | Motor control |
| PWM | Motor speed control |

## Motor Control Logic
PWM Wrap: 1561
Duty cycle calculated using:

// Forward
duty = (2000 - xvalue) / 2000.0f * 1796.0f;

// Reverse
duty = (xvalue - 2200) / (4095.0f - 2200.0f) * 1796.0f;

// 2000 and 2200 values were deadzones given for neutral based on idle joystick value readings. Due to cheap motors, values are limited to max and min as gears and radio lock out and have to be power cycled.

## Direction Table
| **Action** | **X Range** | **Y Range** | **Motor Behavior** |
| :---: | :---: | :---: | :---: |
| Foward | < 100 | Neutral | All Wheels Forward |
| Reverse | > 4000 | Neutral | All wheels reverse |
| Turn Right | Neutral | < 100 | Left wheels forward, right wheels reverse |
| Turn Left | Neutral | > 4000 | Right wheels forward, left wheels reverse |
| Neutral | 500-3500 | 500-3500 | All motors stop |

## Power Design
- TX Side: 7.4V LiPo → 5V Buck → Pico (VSYS) and nRF24L01+

- RX Side:

  - Buck #1 → 5V to Pico and radio

  - Buck #2 → 5V to DRV8833 drivers

- Shared ground maintained across all components

## Troubleshooting & Debugging
Problem: RX Freeze During Non-Max and Non-Min Joystick Input
- Occurs when X is between 350-1800 and 2500-3900

- Does not occur when joystick is fully maxed or mined out (e.g., < 100 or > 4000)

- Suspected cause: current spike or transient noise interfering with SPI or the Pico

Problem: RX/TX will ACK only about 3/10 trys and locks out for a couple seconds

## Fixes Implemented
- Locked values to < 100 and > 4000 (Min and Max respectively) to make sure gears and radio never locks

- Added 100µF electrolytic and 0.1µF ceramic capacitors at power lines which increased ACK to roughly 8/10 trys but still locks out

- Isolated logic and motor power rails to make sure pico's power is steady

## Future Improvements
- Packet validation and sequence tracking
  - Make sure its a complete packet
  - The data is not corrupted
  - Verify data come from TX and not random noise

- Add display feedback or sensor integration

- Use IRQs instead of polling for RX

- Add Ultrasonic sensor for emergency stops 

## Credits & References
Raspberry Pi Pico SDK documentation

nRF24L01+ datasheet and communication protocols

DRV8833 motor driver datasheet

