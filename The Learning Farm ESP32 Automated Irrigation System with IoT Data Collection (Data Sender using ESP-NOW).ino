//----------------------------------------------------------------
// The Learning Farm ESP32 Automated Irrigation System with IoT Data Collection (Data Sender using ESP-NOW)
//
// Vincent Tjoa, July 3, 2025

#include <DHT.h>
#include <DHT_U.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// === Relay Setup ===
//Connect 5V 3A adapter's positive terminal to ESP32's VIN pin and relay DC+ terminal, and share adapter, ESP32, and relay grounds
#define RELAY_IN1 13 //Connect relay's IN1 pin/terminal to ESP32 GPIO 13/D13 pin
#define RELAY_IN2 14 //Connect relay's IN2 pin/terminal to GPIO 14/D14 pin
#define RELAY_IN3 2 //Connect relay's IN3 pin/terminal to GPIO 2/D2 pin
#define RELAY_IN4 18 //Connect relay's IN4 pin/terminal to GPIO 18/D18 pin

// === DHT22 Setup ===
#define DHTPIN 25 //Connect DHT22 Module's OUT pin to ESP32 GPIO 25/D25 pin
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// === LED Pins ===
const int ledGreen = 19; //Connect LED's anode to ESP32 GPIO 19/D19 pin, then connect <500Ω resistor from LED's cathode to ESP32 GND
const int ledRed = 23; //Connect LED's anode to ESP32 GPIO 23/D23 pin, then connect <500Ω resistor from LED's cathode to ESP32 GND
const int ledOrange = 27; //Connect LED's anode to ESP32 GPIO 27/D27 pin, then connect <500Ω resistor from LED's cathode to ESP32 GND
const int ledRainbow = 26; //Connect LED's anode to ESP32 GPIO 26/D26 pin, then connect <500Ω resistor from LED's cathode to ESP32 GND

// === Soil Moisture Setup ===
const int moisture_pin1 = 33; //Connect capacitive soil moisture sensor's AOUT pin to ESP32 GPIO 33/D33 pin
const int moisture_pin2 = 32; //Connect capacitive soil moisture sensor's AOUT pin to ESP32 GPIO 32/D32 pin
const int moisture_pin3 = 35; //Connect capacitive soil moisture sensor's AOUT pin to ESP32 GPIO 35/D35 pin
const int moisture_pin4 = 34; //Connect capacitive soil moisture sensor's AOUT pin to ESP32 GPIO 34/D34 pin
int moisture_raw1 = 0, moisture_raw2 = 0, moisture_raw3 = 0, moisture_raw4 = 0;
int moisture_percent1 = 0, moisture_percent2 = 0, moisture_percent3 = 0, moisture_percent4 = 0;

// === OLED Setup ===
//Connect OLED's SCL and SDA pins to ESP32 GPIO 22/D22 pin and GPIO 21/D21 pin respectively
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === ESP-NOW Setup ===
uint8_t receiverMac[] = {0xCC, 0xDB, 0xA7, 0x9D, 0x77, 0xAC};
esp_now_peer_info_t peerInfo;

typedef struct struct_message {
  float temperature; 
  float humidity;
  int moisture1;
  int moisture2;
  int moisture3;
  int moisture4;
} struct_message;

struct_message sensorData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("ESP-NOW Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  delay(100);

  // Set WiFi channel (optional if matching receiver's channel)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Failed to add ESP-NOW peer");
  } else {
    Serial.println("✅ ESP-NOW peer added");
  }
}

// === Moisture Mapping Function ===
int mapMoisture(int raw) {
  int percent = ((3600 - raw) / (3600.0 - 1600.0)) * 100;
  return constrain(percent, 0, 100);
}

// === LED Blink Struct ===
struct BlinkState {
  unsigned long previousMillis = 0;
  int blinkCount = 0;
  int blinkLimit = 0;
  bool ledState = false;
};

BlinkState greenBlink, redBlink, orangeBlink, rainbowBlink;

void handleBlink(int pin, BlinkState &state, int totalBlinks);

void handleBlink(int pin, BlinkState &state, int totalBlinks) {
  unsigned long currentMillis = millis();
  const int onTime = 200;
  const int offTime = 100;

  // If blinking is finished
  if (state.blinkCount >= state.blinkLimit * 2) {
    digitalWrite(pin, LOW);
    // Update blink limit if changed
    if (totalBlinks != state.blinkLimit) {
      state.blinkCount = 0;
      state.blinkLimit = totalBlinks;
      state.previousMillis = currentMillis;
      state.ledState = false;
    }
    return;
  }

  // Start new blink cycle if needed
  if (state.blinkLimit == 0 && totalBlinks > 0) {
    state.blinkCount = 0;
    state.blinkLimit = totalBlinks;
    state.previousMillis = currentMillis;
    state.ledState = false;
  }

  int interval = state.ledState ? onTime : offTime;
  if (currentMillis - state.previousMillis >= interval) {
    state.previousMillis = currentMillis;
    state.ledState = !state.ledState;
    digitalWrite(pin, state.ledState ? HIGH : LOW);
    state.blinkCount++;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  dht.begin();
  delay(1000);

  pinMode(RELAY_IN1, OUTPUT);
  pinMode(RELAY_IN2, OUTPUT);
  pinMode(RELAY_IN3, OUTPUT);
  pinMode(RELAY_IN4, OUTPUT);

  digitalWrite(RELAY_IN1, HIGH);
  digitalWrite(RELAY_IN2, HIGH);
  digitalWrite(RELAY_IN3, HIGH);
  digitalWrite(RELAY_IN4, HIGH);

  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledOrange, OUTPUT);
  pinMode(ledRainbow, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("❌ SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 30);
  display.println("Booting...");
  display.display();
  delay(1000);
  display.clearDisplay();

  initESPNow(); //Initialize ESP-NOW protocol
}

void loop() {
  float dhtTemp = dht.readTemperature();
  float humidity = dht.readHumidity();

  moisture_raw1 = analogRead(moisture_pin1); //reads analog value (ADC) from capacitive soil moisture sensor 1
  moisture_raw2 = analogRead(moisture_pin2); //reads analog value (ADC) from capacitive soil moisture sensor 2
  moisture_raw3 = analogRead(moisture_pin3); //reads analog value (ADC) from capacitive soil moisture sensor 3
  moisture_raw4 = analogRead(moisture_pin4); //reads analog value (ADC) from capacitive soil moisture sensor 4

  moisture_percent1 = mapMoisture(moisture_raw1); //calls the mapMoisture function to read soil moisture percentage from capacitive soil moisture sensor 1
  moisture_percent2 = mapMoisture(moisture_raw2); //calls the mapMoisture function to read soil moisture percentage from capacitive soil moisture sensor 2
  moisture_percent3 = mapMoisture(moisture_raw3); //calls the mapMoisture function to read soil moisture percentage from capacitive soil moisture sensor 3
  moisture_percent4 = mapMoisture(moisture_raw4); //calls the mapMoisture function to read soil moisture percentage from capacitive soil moisture sensor 4

  Serial.printf("\nTemp: %.1f C, Humidity: %.1f %%\n", dhtTemp, humidity);
  Serial.printf("Moisture 1: %d %% (raw %d)\n", moisture_percent1, moisture_raw1);
  Serial.printf("Moisture 2: %d %% (raw %d)\n", moisture_percent2, moisture_raw2);
  Serial.printf("Moisture 3: %d %% (raw %d)\n", moisture_percent3, moisture_raw3);
  Serial.printf("Moisture 4: %d %% (raw %d)\n", moisture_percent4, moisture_raw4);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0); display.printf("Temp : %.1f C\n", dhtTemp);
  display.setCursor(0, 10); display.printf("Humidity : %.1f %%\n", humidity);
  display.setCursor(0, 20); display.printf("Moist 1: %d %%\n", moisture_percent1);
  display.setCursor(0, 30); display.printf("Moist 2: %d %%\n", moisture_percent2);
  display.setCursor(0, 40); display.printf("Moist 3: %d %%\n", moisture_percent3);
  display.setCursor(0, 50); display.printf("Moist 4: %d %%\n", moisture_percent4);
  display.display();

  handleBlink(ledGreen, greenBlink, moisture_percent1 / 20);
  handleBlink(ledRed, redBlink, moisture_percent2 / 20);
  handleBlink(ledOrange, orangeBlink, moisture_percent3 / 20);
  handleBlink(ledRainbow, rainbowBlink, moisture_percent4 / 20);


  // Relay Conditions
  bool moist1_low = (moisture_percent1 < 40);
  bool moist2_low = (moisture_percent2 < 40);

  // Pump (Relay IN3) ON if either soil 1 or 2 is dry
  if (moist1_low || moist2_low) {
    digitalWrite(RELAY_IN3, LOW);  // ON (active-low)
  } else {
    digitalWrite(RELAY_IN3, HIGH); // OFF
  }

  // Valve 1 (Relay IN1) ON if moist1 > 40 and moist2 < 40
  if (!moist1_low && moist2_low) {
    digitalWrite(RELAY_IN1, LOW);  // ON
  } else {
    digitalWrite(RELAY_IN1, HIGH); // OFF
  }

  // Valve 2 (Relay IN2) ON if moist2 > 40 and moist1 < 40
  if (!moist2_low && moist1_low) {
    digitalWrite(RELAY_IN2, LOW);  // ON
  } else {
    digitalWrite(RELAY_IN2, HIGH); // OFF
  }

  // Relay IN4 not used or set to OFF
  digitalWrite(RELAY_IN4, HIGH);// Relay logic (active-low)

  // === Send Data via ESP-NOW ===
  sensorData.temperature = dhtTemp; //sends temperature reading to receiver ESP32
  sensorData.humidity = humidity; //sends humidity reading to receiver ESP32
  sensorData.moisture1 = moisture_percent1; //sends capacitive soil moisture sensor 1 reading to receiver ESP32
  sensorData.moisture2 = moisture_percent2; //sends capacitive soil moisture sensor 2 reading to receiver ESP32
  sensorData.moisture3 = moisture_percent3; //sends capacitive soil moisture sensor 3 reading to receiver ESP32
  sensorData.moisture4 = moisture_percent4; //sends capacitive soil moisture sensor 4 reading to receiver ESP32

  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&sensorData, sizeof(sensorData));
  if (result == ESP_OK) {
    Serial.println("✅ ESP-NOW Data sent");
  } else {
    Serial.printf("❌ ESP-NOW Send Error (%d)\n", result);
  }

  delay(10000);

}

/*
  //For manual calibration of soil moisture wet & dry values, switch this with the "moisture_percentx = mapMoisture(moisture_rawx);"" functions

  moisture_raw1 = analogRead(moisture_pin1);
  // Updated mapping for soil_moisture_1: 1550 (wet) -> 100%, 3700 (dry) -> 0%
  moisture_percent1 = ((3600 - moisture_raw1) / (3600.0 - 1600.0)) * 100;
  moisture_percent1 = constrain(moisture_percent1, 0, 100); // Clamp between 0–100

  moisture_raw2 = analogRead(moisture_pin2);
  // Updated mapping for soil_moisture_2: 1550 (wet) -> 100%, 3700 (dry) -> 0%
  moisture_percent2 = ((3600 - moisture_raw2) / (3600.0 - 1600.0)) * 100;
  moisture_percent2 = constrain(moisture_percent2, 0, 100); // Clamp between 0–100

  moisture_raw3 = analogRead(moisture_pin3);
  // Updated mapping for soil_moisture_3: 1550 (wet) -> 100%, 3700 (dry) -> 0%
  moisture_percent3 = ((3600 - moisture_raw3) / (3600.0 - 1600.0) * 100);
  moisture_percent3 = constrain(moisture_percent3, 0, 100); // Clamp between 0–100

  moisture_raw4 = analogRead(moisture_pin4);
  // Updated mapping for soil_moisture_4: 1550 (wet) -> 100%, 3300 (dry) -> 0%
  moisture_percent4 = ((3600 - moisture_raw4) / (3600.0 - 1600.0)) * 100;
  moisture_percent4 = constrain(moisture_percent4, 0, 100); // Clamp between 0–100
*/
