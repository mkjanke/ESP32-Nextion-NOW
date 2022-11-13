#ifndef SETTINGS_H
#define SETTINGS_H


#define HEARTBEAT 1000L         // Sensor and WiFi loop delay (ms)
#define DEVICE_NAME "ESP-Test"

// Nextion Serial configuration
// May need to use Serial2, depending on ESP32 module and pinouts
#define NEXTION_SERIAL Serial1
#define NEXTION_BAUD 115200
#define RXDN 19
#define TXDN 21

// Nextion Widgets
#define NEXT_UPTIME_WIDGET "page0.upTxt.txt" // Name of Uptime widget on Nextion display
#define NEXT_ESPOUT_WIDGET "Setup.ESP.txt"   // Widget to display ESP debug output

//ESP-NOW configuration
#define ESPNOW_QUEUE_SIZE 10
#define ESP_BUFFER_SIZE 200

#endif //SETTINS_H