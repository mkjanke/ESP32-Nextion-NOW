#ifndef SETTINGS_H
#define SETTINGS_H

#define WIFI_SSID "IoT"
#define WIFI_PASSWORD "WTR54G2222"

#define HEARTBEAT 1000L         // Sensor and WiFi loop delay (ms)
#define DEVICE_NAME "ESP-Test"

// Nextion Serial configuration
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

// BlueTooth UUID's
#define SERVICE_UUID        "611f9238-3915-11ec-8d3d-0242ac130003"  // 9238
#define MESSAGE_UUID        "6bcbec08-7fbb-11ec-a8a3-0242ac120002"  // 0002 String - Nextion Command
#define EVENT_UUID          "6bcbee60-7fbb-11ec-a8a3-0242ac120002"
#define UPTIME_UUID         "611f96f2-3915-11ec-8d3d-0242ac130003"  // 96f2 String


#endif //SETTINS_H