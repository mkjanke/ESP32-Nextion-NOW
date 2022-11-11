#include <Arduino.h>
#include <ArduinoJson.h>
// #include <ArduinoOTA.h>
#include <WiFi.h>
#include "espnow.h"
#include "settings.h"
#include "nextionInterface.h"

// May need to use Serial2, depending on ESP32 module and pinouts
myNextionInterface myNex(NEXTION_SERIAL, NEXTION_BAUD);
void handleNextion(void* );
TaskHandle_t xhandleNextionHandle = NULL;

void setup() {
  // Serial
  vTaskDelay(10000);
  Serial.begin(115200);
  Serial.printf("%s Is Now Woke!\n", DEVICE_NAME);

  // Set WIFI mode to STA mode
  WiFi.mode(WIFI_STA); 

  myNex.begin();        // Initialize Nextion interface

  if (!initEspNow()) {  // Start esp-now  
    return;
  };

  xTaskCreate(handleNextion, "Nextion Handler", 3000, NULL, 6, &xhandleNextionHandle);
}

void loop() {
  // vTaskDelay(10000/portTICK_PERIOD_MS);
  // Serial.println("Heartbeat");
}

// Read incoming events (messages) from Nextion
// Forward filtered events to BLEinterface
//
// Check for command from BLE interface
// write command to Nextion Display
void handleNextion(void* parameter) {
  
  StaticJsonDocument<ESP_BUFFER_SIZE> doc;

  // Events we care about
  const char filter[] = {'\x65', '\x66', '\x67', '\x68',
                         '\x70', '\x71', '\x86', '\x87',
                         '\xAA'};

  std::string _bytes;  // Raw bytes returned from Nextion, incl. \xFF terminaters
  _bytes.reserve(48);

  std::string _hexString;  // _bytes converted to space delimited ASCII chars
                           // I.E. 1A B4 E4 FF FF FF
  char _x[3] = {};

  vTaskDelay(100 / portTICK_PERIOD_MS);

  for (;;) {  // ever
    // Check for incoming event from Nextion
    if (_bytes.length() > 0) _bytes.clear();
    if (_hexString.length() > 0) _hexString.clear();

    int _len = myNex.listen(_bytes, 48);
    if (_len) {
      if (_len > 3) {
        _hexString.reserve(_len * 3);
        for (const auto& item : _bytes) {
          sprintf(_x, "%02X ", item);
          _hexString += _x;
        }
        // sLog.send((String)"handleNextion returned: " + _hexString.c_str());
        Serial.println((String)"handleNextion returned: " + _hexString.c_str());
        
        // If we see interesting event from Nextion, forward to BLE interface
        // and to ESP-NOW.
        for (size_t i = 0; i < sizeof filter; i++) {
          if (_bytes[0] == filter[i]) {
            doc.clear();
            doc["D"] = DEVICE_NAME;
            doc["MSG"] = _hexString.c_str();
            espNowSend((JsonDocument&)doc);

        //     // bleIF.writeEvent(_hexString);

        //     // if (_bytes[0] == '\x70') {
        //     //   bleIF.writeMessage(_hexString);
        //     // }
        //     // if (_bytes[0] == '\x71') {
        //     //   bleIF.writeMessage(_hexString);
        //     // }

          }
        }
      } else {
        // sLog.send("handleNextion: Short read");
        myNex.flushReads();
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);  // Should never reach this.
}  // handleNextion()
