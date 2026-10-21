# Next reconstruction pass

1. Build pad/via coordinate inventory from top/bottom copper + drills.
2. Identify the footprints for SIPO, MUX16, U1, DS18B20, LED0..2, LH1/RH1.
3. Trace and name only the first high-value nets:
   - GND
   - VCC / 3V3
   - 5V
   - 12V/VIN
   - I2C SDA/SCL
   - SPI MISO/MOSI/SCK/CS
   - 74HC595 data/clock/latch/OE/MR
   - CD74HC4067 S0..S3/EN/SIG
4. Cross-check those nets against the physical board and firmware HWINFO.
5. Generate first KiCad schematic sheet only after these core nets are stable.
