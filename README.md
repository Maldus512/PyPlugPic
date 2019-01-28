# PyPlug Current Sensor Firmware

This firmware is meant to be flashed on PyPlug's PIC16F15325. The role of the PIC MCU on the board is to continuously read and convert ADC values from the current sensor; this is done 5 times per second. Those values are then conveniently polled by a higher level ESP32, which can avoid to poll an ADC by itself. You can find PyPlug's ESP32 implementation [here](https://github.com/Maldus512/PyPlugESP32).

## Communication Protocol

Communication to the PIC passes through an UART interface running at 9600 bps. From the ESP32, RX line is on GPIO 16 and TX line is on GPIO 17.
The communication protocol uses custom *ATCOMMANDS*, short strings starting with the AT prefix.
The following commands are accepted by the PIC:

- *ATON*: turns on the load.
- *ATOFF*: turns off the load.
- *ATSTATE*: prints the current state (i.e. whether the load is on or off).
- *ATPRINT*: instructs the PIC to print a recap of the whole system every second, using the format `state=x, current=y, adc=z, cal=t, consumption=u`, where:
  - *state*: 1 or 0, to indicate on or off.
  - *current*: the current reading in Amperes.
  - *adc*: a single adc reading (meaningless, kept for debug purposes).
  - *cal*: the current calibration values for adc readings.
  - *consumption*: accumulated power consumption in Watt-hour.

  Sending any other character will interrupt this process.

- *ATRESET*: resets every value on the MCU. Mainly for debug purposes.
- *ATREAD*: returns the current current reading in Amperes.
- *ATPOWER*: returns the accumulated power consumption in Watt-hour (updated every minute)
- *ATZERO*: resets the power accumulator.

Every command is processed when the newline (`\n`, or ASCII code 10) character is received. If the command is correctly processed a newline is sent as acknowledgement (after eventual return values).

## Notification Pin

Additionally, when the PIC reads a null current value a line corresponding to GPIO4 on the ESP32 is pulled low, to be left high when a current flow is detected. This can serve as a notification system to wake up a sleeping ESP32 when activity is detected.
