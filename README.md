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

Connections:
 - DHT22 → GPIO 25 (OUT)
 - Soil Moisture Sensors → AOUT to GPIO 33, 32, 35, 34
 - OLED → SDA to GPIO 21, SCL to GPIO 22 (I²C)
 - Relays → IN1 (13), IN2 (14), IN3 (2), IN4 (18)
 - LEDs → Green (19), Red (23), Orange (27), Rainbow (26)
 - 5V 3A adapter → VIN and relay VCC (shared ground)

Power and data flow summary:
 - ESP32 powered by 5V adapter through VIN.
 - Sensors powered by 3.3V (signal logic) and relays by 5V.
 - Data flows from DHT22 + soil sensors → ESP32 → OLED + relays → receiver ESP32 via ESP-NOW.
 - Sensor values displayed and transmitted every 10 seconds for irrigation automation and data logging.

This program was written and uploaded via the Arduino IDE, using the following libraries: DHT, Adafruit_GFX, Adafruit_SSD1306, WiFi, and esp_now.

//----------------------------------------------------------------

The Learning Farm ESP32 Node MCU IoT Data Collection (Data Receiver using ESP-NOW) notes:

This system does not have any peripherals, and is only connected to a 5V power source to function. It uses The Learning Farm's local WiFi channel, and directly transmits all data it collects to a cloud network (adafruit IO) I used the ESP-NOW protocol to transmit data to this receiver ESP32 from the sender ESP32, which then transmits the collected data to Adafruit IO cloud It sends the data from a DHT22 sensor and 4 capacitive soil moisture sensors from the other ESP32 system

Connections:
 - ESP32 NodeMCU powered by 5V via USB or external 5V adapter.
 - zero sensors and peripherals (only board)
 - Wi-Fi credentials and Adafruit IO keys configured in the code for automatic upload (not added in the code).

Power and data flow summary:
 - ESP32 NodeMCU powered by 5V (logic 3.3V internally).
 - Data flows wirelessly from remote ESP32 sensor nodes → ESP32 NodeMCU (via ESP-NOW) → Wi-Fi → Adafruit IO.
 - Receives temperature, humidity, and four soil moisture readings from multiple field nodes.
 - The ESP32 prints received values via serial monitor and uploads them to corresponding Adafruit IO feeds.

This program was written and uploaded via the Arduino IDE, using libraries for each module (WiFi, esp_now, AdafruitIO_WiFi).
