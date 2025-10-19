//----------------------------------------------------------------
// The Learning Farm ESP32 Node MCU IoT Data Collection (Data Receiver using ESP-NOW)
//
// Vincent Tjoa, July 3, 2025

#include <AdafruitIO_WiFi.h>
#include <WiFi.h>
#include <esp_now.h>

// Replace with your network credentials
#define WIFI_SSID "xxxxx" //TLF WiFi SSID
#define WIFI_PASS "xxxxx" //TLF WiFi Password

// Replace with your Adafruit IO credentials
#define IO_USERNAME  "xxxxx" //Adafruit IO TLF username
#define IO_KEY       "xxxxx" //Adafruit IO TLF Key

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// === Define Adafruit IO Feeds ===
AdafruitIO_Feed *temperature_feed = io.feed("DHT22 Temperature");
AdafruitIO_Feed *humidity_feed = io.feed("DHT22 Humidity");
AdafruitIO_Feed *moisture1_feed = io.feed("Soil Moisture 1");
AdafruitIO_Feed *moisture2_feed = io.feed("Soil Moisture 2");
AdafruitIO_Feed *moisture3_feed = io.feed("Soil Moisture 3");
AdafruitIO_Feed *moisture4_feed = io.feed("Soil Moisture 4");

// === Data Structure must match sender ===
typedef struct struct_message {
  float temperature;
  float humidity;
  int moisture1;
  int moisture2;
  int moisture3;
  int moisture4;
} struct_message;


struct_message incomingData;


// === Callback: runs every time data is received ===
void OnDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           recvInfo->src_addr[0], recvInfo->src_addr[1], recvInfo->src_addr[2],
           recvInfo->src_addr[3], recvInfo->src_addr[4], recvInfo->src_addr[5]);
 
  Serial.print("Received data from: ");
  Serial.println(macStr);

  // Make sure the received data size matches the struct
  if (len == sizeof(struct_message)) {
    memcpy(&incomingData, data, sizeof(incomingData));
   
    Serial.println("Parsed data:");
    Serial.printf("  Temperature : %.1f C\n", incomingData.temperature);
    Serial.printf("  Humidity    : %.1f %%\n", incomingData.humidity);
    Serial.printf("  Moisture 1  : %d %%\n", incomingData.moisture1);
    Serial.printf("  Moisture 2  : %d %%\n", incomingData.moisture2);
    Serial.printf("  Moisture 3  : %d %%\n", incomingData.moisture3);
    Serial.printf("  Moisture 4  : %d %%\n", incomingData.moisture4);
    Serial.println();

    // === Send data to Adafruit IO ===
    temperature_feed->save(incomingData.temperature);
    humidity_feed->save(incomingData.humidity);
    moisture1_feed->save(incomingData.moisture1);
    moisture2_feed->save(incomingData.moisture2);
    moisture3_feed->save(incomingData.moisture3);
    moisture4_feed->save(incomingData.moisture4);

  } else {
    Serial.println("Received data size mismatch!");
  }
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // ESP-NOW requires station mode

  // Start Adafruit IO connection
  io.connect();

  // Wait for Adafruit IO connection
unsigned long startAttempt = millis();
while(io.status() < AIO_CONNECTED) {
  Serial.print(".");
  io.run();  // ensure it's processing network
  delay(500);

  if (millis() - startAttempt > 15000) {
    Serial.println("\n❌ Connection Timeout. Check Wi-Fi or AIO credentials.");
    break;
    }
  }
  Serial.println();
  Serial.println("Connected to Adafruit IO");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the receive callback
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW Receiver Ready");
}


void loop() {
  io.run();  // Keep Adafruit IO connection alive
 
  /*Serial.println("test");
  Serial.println(WiFi.channel()); //to check the WiFi channel the system is present in
  delay(5000);
  */
}
