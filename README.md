# RC Car – Embedded Wireless Control System
## Project Overview
This project demonstrates an entry-level systems and embedded software engineering design using two Raspberry Pi Picos. One Pico reads joystick input and wirelessly transmits control signals using the nRF24L01+ radio module. The second Pico receives the data and drives a four-wheel RC car using PWM-controlled motor drivers. Designed with modularity, signal integrity, and power isolation in mind, this project integrates embedded C programming, communication protocols, real-time motor control, and low-level hardware interfaces - showcasing practical skills in both system-level planning and embedded firmware development.

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

- 1x 70x90mm perfboard, 2x 20x80mm perfboard

- 10x M2 x 6mm screws

- 4x M3 x 6mm screws

- 4x M3 x 16mm screws

- 6x M3 x 30mm screws

- Pin headers

## Budget Report

| Component                             | Qty Used | Qty Bought | Cost Paid | Cost Used |
|--------------------------------------|----------|-------------|-----------|-----------|
| Raspberry Pi Pico                    | 2        | 2           | $14.00    | $14.00    |
| nRF24L01+ Modules                    | 2        | 4           | $7.89     | $3.95     |
| nRF24L01+ Breakout Boards            | 2        | 4           | $7.99     | $4.00     |
| DRV8833 Motor Drivers                | 2        | 5           | $8.59     | $3.44     |
| 2S LiPo Batteries (2pack) + Charger  | 1        | 1           | $24.99    | $24.99    |
| TT Motors + Wheels (4-Pack)          | 4        | 4           | $8.99     | $8.99     |
| JST Female Connectors (20AWG)        | 2        | 20          | $7.19     | $0.72     |
| M3 Screws/Nuts/Washers (2240 pcs)    | 14       | 2240        | $24.99    | $0.16     |
| M2 Screws (1140 pcs)                 | 10       | 1140        | $6.98     | $0.06     |
| 100µF + 0.1µF Capacitors             | 4        | -           | $0.00***     | $0.00  |
| Joystick Module                      | 1        | -           | $0.00*    | $0.00     |
| 3D Printed Chassis                   | 1        | -           | $0.00**   | $0.00     |
| LEDs + Resistors                     | 4        | -           | $0.00***     | $0.00  |
| Splicing Connectors (1→2, 1→3)       | 4        | 26          | $14.98    | $2.30     |
| Perfboards (70x90, 20x80 x2)         | 3        | 32          | $6.29     | $0.59     |
| Dupont Wires (F-F, M-M)              | 18       | 120         | $6.98     | $1.05     |

**Total Spent:** $139.86  
**Total Used Value in Build:** ~$64.25

\* Donated by a friend; found online for ~$6.29  
\** Printed for free at CSUN using university resources (PLA)  
\*** Already had

## Equipment Report

| Equipment                                    | Qty Bought | Qty Used | Cost Paid | Cost Used |
|---------------------------------------------|------------|----------|-----------|-----------|
| No-Clean Soldering Flux Paste (2-pack, 10CC each) | 1          | 1        | $9.99     | ~$1.50    |
| Fanttik Soldering Iron Kit                  | 1          | 1        | $56.20    | $56.20    |
| Kotto Fume Extractor + Helping Hands        | 1          | 1        | $49.99    | $49.99    |
| 63/37 Solder Wire (100g, 0.6mm)             | 1          | 1        | $7.99     | ~$2.00    |

**Total Spent on Equipment:** $124.17  
**Estimated Value Used:** ~$109.69

## Block Diagram

The following diagram illustrates the full system architecture, including the transmitter (joystick module) and receiver (RC car):

![RC Car Block Diagram](https://github.com/user-attachments/assets/c1271378-2d14-41d3-b2f8-7e34a096a80f)
*Figure: System-level block diagram showing data flow, power paths, and communication interfaces.*

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

// 2000 and 2200 values were deadzones given for neutral based on idle joystick value readings. Due to budget-friendly motors, values are limited to max and min as gears and radio lock out and have to be power cycled.

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

## Final Outcome   
![IMG_0824](https://github.com/user-attachments/assets/67a352bc-edb3-443e-88b5-43f37788e64c)
![IMG_0825](https://github.com/user-attachments/assets/c99b8c46-d7ae-4c15-add2-dd56e45bde64)
![IMG_0826](https://github.com/user-attachments/assets/3c8a57c5-8fa1-452b-82aa-b23e469c46cb)
![IMG_0827](https://github.com/user-attachments/assets/87de3be6-0aac-4e59-8e75-88b9533cf88d)
![IMG_0828](https://github.com/user-attachments/assets/4f6e6a28-f22b-4658-a68b-a7ad3f0dfef9)

## Credits & References
- Raspberry Pi Pico SDK documentation

- nRF24L01+ datasheet and communication protocols

*Special thank you to CSUN's Creator Maker Studio for allowing me to use their services free of charge, this project would have not been possible to complete without their help*
