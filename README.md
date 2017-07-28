# HiFive1-GPIO
Demonstrate I/O driven interrupts on SiFive's FE310 chip, as used on the HiFive1 dev board.

## Usage

This program sets up PWM0 as a 1.19MHz clock source (pin 5, blue LED). Pin 19 is set up as a GPIO input source and raises an interrupt on a rising edge where the interrupt handler toggles the output of pin 18. 
