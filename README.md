# Simon
Embedded programming using ATmega328P as a microcontroller, AVR as a platform architecture, and Arduino Uno as a hardware platform.

It uses 4 LEDs and 5 pushbutton switches (4 for reading input and 1 for waking up the program from an idle state explained shortly). The output pins are connected to pins PB0 ~ PB3 of Arduino Uno through jumper wires, which are connected to each LED. The input pins are connected to pins PC2 ~ PC5 and to 4 pushbutton switches, which also are connected to each LED.

One additional connection with PB0 and an extra pushbutton switch is needed. This circuit serves as having the program run into idle mode when there is no input for a certain amount of time.

No hardware switch debouncing is needed here.
