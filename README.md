# TLF_ESP32_Systems
AgriSync.ID C++ code on Arduino IDE for my ESP32-based Automated Irrigation System with IoT Data Collection

Start Date: February 2025

End Date: ongoing

Location: The Learning Farm (TLF), Kawungluwuk Village, Sukaresmi, Cianjur Regency, West Java, Indonesia

Description:

To confirm the feasability of a microcontroller-based automated irrigation system with ESP32s in Javanese highland farming conditions.
Various ESP32 prototypes and an automated irrigation system were implemented in TLF, collecting data on water usage, electricity usage, crop growth, crop health, etc.

This repository possesses the code required to operate these systems.

//----------------------------------------------------------------

The Learning Farm ESP32 Automated Irrigation System with IoT Data Collection (Data Sender using ESP-NOW) notes:

This system uses 5V relays instead of MOSFETs for 12V DC Water pump and 12V DC Solenoid valve control Operation: Uses a 5V 3A AC-to-DC adapter for the ESP32 and 5V relay system, and a 12V 15A AC-to-DC adapter for the water pump and solenoid valves

This system utilizes a DOIT ESP32 Devkit V1, DHT22 temp & humidity sensor, SSD1306 0.96" OLED display, 4 LEDs, 4 capacitive soil moisture sensors, and a 5V quad-relay module (for water pumps and solenoid valves). It operates an automated irrigation system by actuating the water pump and solenoid valves based off of each soil moisture sensor's readings

I used the ESP-NOW protocol to transmit data to a receiver ESP32, which then transmits the collected data to Adafruit IO cloud (I will post its code in another file)

Both the OLED display and MCP sensor is wired up to the ESP32’s I2C bus through each module’s SCL and SDA lines, displaying all the data collected by all the sensors in real-time. Each module connected to the ESP32 is powered with 3.3V, not 5V (+ terminal of each sensor to 3.3v rail on breadboard, and - terminal to ESP32 GND breadboard rail)

The LEDs blink or stays off depending on the soil moisture percentage. In this configuration, each LED is paired with a capacitive soil moisture sensor.

When soil moisture percentage is 0-19%, it stays off. When soil moisture percentage is 20-39%, it blinks once. When soil moisture percentage is 40-59%, it blinks twice. When soil moisture percentage is 60-79%, it blinks thrice. When soil moisture percentage is 80-100%, it blinks four to five times.

Each blink consists of 200ms ON and 100ms OFF. Each cycle runs totalBlinks × 2 (because each blink is 1 ON + 1 OFF). Once blinking finishes, the LED stays off until a new blink count is triggered.

When calibrating the sensors, the capacitive soil moisture sensors transmits a raw value of around 3600 when dry, and around 1600 when wet.

This code was created in Arduino IDE, and was directly flashed onto the ESP32 from the IDE. I also may have had the help of AI in the creation of this code :)

//----------------------------------------------------------------

The Learning Farm ESP32 Node MCU IoT Data Collection (Data Receiver using ESP-NOW) notes:

This system does not have any peripherals, and is only connected to a 5V power source to function. It uses The Learning Farm's local WiFi channel, and directly transmits all data it collects to a cloud network (adafruit IO) I used the ESP-NOW protocol to transmit data to this receiver ESP32 from the sender ESP32, which then transmits the collected data to Adafruit IO cloud It sends the data from a DHT22 sensor and 4 capacitive soil moisture sensors from the other ESP32 system

This code was created in Arduino IDE, and was directly flashed onto the ESP32 from the IDE.
