### PyPlug Current Sensor Firmware

This firmware is meant to be flashed on PyPlug's PIC16F15325. The role of the PIC MCU on the board is to continously read and convert ADC values from the current sensor; the higher level ESP32 can then conviently poll those values, avoiding to poll an ADC by itself.
The conversion is carried on 5 times per second.

#### Communication Protocol

Communication to the PIC passes through an UART interface running at 9600. From the ESP32, rx line is on GPIO 16 and tx line is on GPIO 17.
The communication protocol uses custom *ATCOMMANDS*, short strings starting with the AT prefix.
The following commands are accepted by the PIC:

- *ATON*: turns on the load.
- *ATOFF*: turns off the load.
- *ATSTATE*: prints the current state (i.e. whether the load is on or off).
- *ATPRINT*: instructs the PIC to print a recap of the whole system every second, using the format `state=x, current=y, adc=z, cal=t, consumption=u`, where:
    
    - *state* is 1 or 0, to indicate on or off.
    - *current* is the current reading in Amperes.
    - *adc* is a single adc reading (meaningless, kept for debug purposes).
    - *cal* is the current calibration values for adc readings.
    - *consumption* is the accumulated power consumption in Watts/hour.

. Sending any other character interrupts this process.
- *ATRESET*: resets every value on the MCU. Mainly for debug purposes.
- *ATREAD*: returns the current current reading in Amperes.
- *ATPOWER*: returns the accumulated power consumption in Watts/hour (updated every minute)
- *ATZERO*: resets the power accumulator.

Every command is processed when the newline ('\n', or ASCII code 10) character is received. If the command is correctly processed a newline is send as acknowledgement (after eventual return values).

#### Notification Pin

Additionally when the PIC reads a null current value a line corresponding to GPIO4 on the ESP32 is pulled low, to be left high when a current flow is detected. This can serve as a notification system to wake up a sleeping ESP32 when activity is detected.

