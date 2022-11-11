#include "espnow.h"

#include <Arduino.h>

// MAC Address of the receiver
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

TaskHandle_t xhandleSerialWriteHandle = NULL;
TaskHandle_t xhandleHeartbeat = NULL;

static QueueHandle_t send_to_Serial_queue;

char uptimeBuffer[20];  // scratch space for storing formatted 'uptime' string

// Calculate uptime & populate uptime buffer for future use
void uptime() {
  // Constants for uptime calculations
  static const uint32_t millis_in_day = 1000 * 60 * 60 * 24;
  static const uint32_t millis_in_hour = 1000 * 60 * 60;
  static const uint32_t millis_in_minute = 1000 * 60;

  uint8_t days = millis() / (millis_in_day);
  uint8_t hours = (millis() - (days * millis_in_day)) / millis_in_hour;
  uint8_t minutes =
      (millis() - (days * millis_in_day) - (hours * millis_in_hour)) /
      millis_in_minute;
  snprintf(uptimeBuffer, sizeof(uptimeBuffer), "Uptime: %2dd%2dh%2dm", days,
           hours, minutes);
}
// Tasks

// Dequeue message from ESP_NOW recieve queue and output to Serial
// If Nextion command, send to nextion
void writeToSerial(void *parameter) {
  char receiveBuffer[ESP_BUFFER_SIZE];
  // String jsonrecv = "";
  // jsonrecv.reserve(ESP_BUFFER_SIZE + 2);
  StaticJsonDocument<ESP_BUFFER_SIZE + 32> doc;

  for (;;) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // Dequeue
    if (xQueueReceive(send_to_Serial_queue, receiveBuffer, portMAX_DELAY) == pdTRUE) {
      // Stuff ESP_NOW data into JSON doc
      // 'doc' automatically cleared by deserializeJson function
      DeserializationError err = deserializeJson(doc, receiveBuffer);
      if (err) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(err.c_str());
      } else {
        // Valid JSON doc, now test if relevent to us
        if(doc.containsKey("D") && doc.containsKey("CMD"))
          if (doc["D"] == "ESP-Display") {
            String command = doc["CMD"];
            Serial.println(command);
            myNex.writeCmd(command);
          }
      }
      for (size_t _i = 0; _i < ESP_BUFFER_SIZE; _i++) {
        receiveBuffer[_i] = 0;
      }
    }
  }
}

// Send heartbeat out to ESP_NOW broadcastAddress
void espnowHeartbeat(void *parameter) {
  String jsonMessage = "";
  jsonMessage.reserve(ESP_BUFFER_SIZE + 2);
  StaticJsonDocument<ESP_BUFFER_SIZE> doc;
  for (;;) {
    uint64_t now = esp_timer_get_time() / 1000 / 1000;
    doc.clear();
    jsonMessage.clear();
    doc["D"] = DEVICE_NAME;
    doc["T"] = long(now);
    doc["S"] = uxTaskGetStackHighWaterMark(xhandleSerialWriteHandle);
    doc["R"] = uxTaskGetStackHighWaterMark(xhandleHeartbeat);
    doc["H"] = esp_get_minimum_free_heap_size();
    doc["Q"] = uxQueueMessagesWaiting(send_to_Serial_queue);

    serializeJson(doc, jsonMessage);  // Convert JsonDoc to JSON string
    int result = espNowSend(jsonMessage);
    // Test for success
    if (result != ESP_OK) {
      Serial.print("Error sending data: ");
      Serial.println(esp_err_to_name(result));
    }
    char buffer[48];
    snprintf(buffer, sizeof(buffer), "S:%ib\\rH:%ib\\rH:%i",
             uxTaskGetStackHighWaterMark(xhandleSerialWriteHandle),
             uxTaskGetStackHighWaterMark(xhandleHeartbeat),
            //  uxTaskGetStackHighWaterMark(xcheckWiFiHandle),
             esp_get_minimum_free_heap_size());

    myNex.writeStr(NEXT_ESPOUT_WIDGET, buffer);
    myNex.writeNum("heartbeat", 1);
    
    uptime();
    myNex.writeStr(NEXT_UPTIME_WIDGET, uptimeBuffer);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

// Callbacks

// Callback function for messaged recieved from ESP-NOW
// Copy message to serial queue
// xQueueSend copyies ESP_BUFFER_SIZE bytes to queue
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // char recvBuffer[ESP_BUFFER_SIZE] = {0};
  if (len <= ESP_BUFFER_SIZE) {
    if (xQueueSend(send_to_Serial_queue, (void *)incomingData, 0) != pdTRUE) {
      Serial.println("Error sending to queue");
    }
  }
}

//Send to ESP-NOW peers (broadcast address)
int espNowSend(const String& message){
    esp_err_t result =
        esp_now_send(broadcastAddress, (uint8_t *)message.c_str(), message.length() + 2);
    return result;
}

//Send to ESP-NOW peers (broadcast address)
int espNowSend(const JsonDocument& doc){
  String jsonMessage = "";
  serializeJson(doc, jsonMessage);  // Convert JsonDoc to JSON string
  esp_err_t result =
        esp_now_send(broadcastAddress, (uint8_t *)jsonMessage.c_str(), jsonMessage.length() + 2);
  return result;
}

// Initialize ESP_NOW interface. Call once from setup()
bool initEspNow() {
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return false;
  } else {
    Serial.println("ESP-NOW initialized");
  }
  // Register Callbacks
  // esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return false;
  }
  // Set up queues and tasks
  send_to_Serial_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, ESP_BUFFER_SIZE);
  if (send_to_Serial_queue == NULL) {
    Serial.println("Create Queue failed");
    return false;
  }
  xTaskCreate(espnowHeartbeat, "Heartbeat Handler", 2400, NULL, 4, &xhandleHeartbeat);
  xTaskCreate(writeToSerial, "Serial Write Handler", 2400, NULL, 4, &xhandleSerialWriteHandle);

  return true;
}

