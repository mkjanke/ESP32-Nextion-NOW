# Nextion ESP-NOW interface

ESP-NOW interface to Nextion Serial Displays. 
* Listens for JSON-formatted data on ESP-NOW and forwards to Nextion serial port. 
* Listens on Nextion serial port and forwards messages to ESP-NOW as JSON-formatted data

Uses: 
* ArduinoJson

Key components:

* ESP_NOW callback:
  * Receive data from ESP Now, send to serial queue:
     * Incoming ESP-NOW packet --> onDataRecv --> send_to_Serial_queue
  
* Task writeToNextion:
  * Dequeue data from serial queue, send to serial port:
   * send_to_Serial_queue --> writeToNextion --> Nextion Serial Port

* Task handleNextion:
  * Recieve data from Nextion serial port, send to ESP-NOW queue:
   * handleNextion --> espNowSend() --> send_to_EspNow_queue

* Task writeToEspNow:
  * Dequeue data from ESP-NOW queue, forward to ESP-NOW:
    * send_to_EspNow_Queue --> writeToEspNow() --> Outgoing ESP-NOW packet
  
* Task espnowHeartbeat:
  * Send uptime and stack data to send_to_EspNow_Queue
  * Send uptime and stack data to Nextion serial port
