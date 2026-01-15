# Embedded Reflex Game on ATmega328 (CS/EE 120B)

An embedded systems project written in Embedded C for the ATmega328. This is a real-time reflex game that runs without an operating system and demonstrates scheduling, state machines, and hardware interfacing under resource constraints.

## What it does
- Runs a real-time game loop with responsive button input
- Renders animation frames on an SSD1306 128x64 OLED over I2C
- Displays score on an LCD
- Plays audio feedback on a buzzer (jump + game over)
- Uses a timer-driven scheduler with multiple concurrent state-machine tasks

## Hardware
- ATmega328 (or equivalent board exposing I2C + GPIO)
- SSD1306 128x64 OLED (I2C)
- Character LCD (e.g., 16x2)
- Push button (jump input)
- Piezo buzzer
- Breadboard + jumper wires

## Project structure
- Main application logic is in the C source files in this repo.
- The architecture is organized as concurrent state-machine tasks coordinated by a timer-driven scheduler.

## Implementation notes
Key engineering challenges in this project:
- Maintaining smooth OLED frame updates over I2C within tight timing constraints
- Implementing collision detection without breaking animation responsiveness
- Debugging timing issues between input polling and display refresh
- Keeping memory usage predictable for sprite / frame rendering

## Demo 
https://www.youtube.com/watch?v=yyZTAf4vM1w


## Acknowledgements
Built for CS/EE 120B (Custom Laboratory).
