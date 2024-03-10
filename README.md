# Vent-Axia-ESP32
Connect ESP32 as Vent-Axia MVHR remote unit 

This is just in Proof-of-concept stage!!!

At this time, this module can read Vent-Axia cable remote messages and responses and logs them to the Serial interface
![image](https://github.com/aelias-eu/vent-axia-esp32/assets/71124636/03550e64-5662-4d78-b4a7-6e9f22637992)

## Schematics
The circuit uses MAX3232 circuit to convert between MCU voltage levels (3V3) and RS232 (+/- 10V).

The ESP-in-the-middle processes packets and sends them to designated receiver.

This configuration allows logging and also modifying data.

![image](https://github.com/aelias-eu/vent-axia-esp32/assets/71124636/4025540a-7d5e-4c01-b48e-c003a1b3a923)

The J4 connector is designed to connect a bunch of DS18B20 or DS18S20 sensors to measure temperatures at each input/output of the MVHR.
(*This will be implemented later on this mdule as this is for me currently provided by another ESP*).
But this is a good feature to see the Energetic benefits of having an MVHR, or to calculate thermal efficiency :) 

Just for a sneak-peek - this is how the temperature infos about MVHR look for me in Homeassistant:
![image](https://github.com/aelias-eu/vent-axia-esp32/assets/71124636/799e5ed4-24c8-4c1f-8c5f-05acf7e479dd)


# Todo
## Software
 - [x] Read and process keyboard status
 - [x] Read and process display messages from MVHR
 - [ ] Test keyboard command insertion
 - [ ] Test display hijacking -> sending custom messages
 - [ ] Add support for DS18x20 sensors for temperature measurement
 - [ ] Add OTG support
 - [ ] Add MQTT Interface
 - [ ] Maybe? Add Web Interface
 - [ ] Maybe? Add CO (Carbon-Monoxide) level measurement 
## Hardware
 - [x] Add schematics
 - [x] Add support for OneWire sensors
 - [ ] Maybe? Add relay output to control high flow request
 - [ ] Maybe? Add analog-out (0-10V) to control flow 
 - [ ] Design the PCB
