# Nextion ESP-NOW interface

ESP-NOW interface to Nextion Serial Displays. 
* Listens for JSON-formatted data on ESP-NOW and forwards to Nextion serial port. 
* Listens on Nextion serial port and forwards messages to ESP-NOW as JSON-formatted data

Uses: 
* ArduinoJson

## ESP_NOW message format:
Message sent to Nextion via ESP-NOW format:
* {"D": "ESP DEVICE NAME", "CMD": "Nextion Command"}
 
Example:
To send a nextion command "page0.Ah1.val=123" to the Nextion (update Nextion object Ah1 on page0 to the value 123):
* {"D": "ESP-Display", "CMD": "page0.Ah1.val=123"}

Message from Nextion sent via ESP-NOW format:
* JSON>> {"D":"ESP-DEVICE-NAME","MSG":"Hex message converted to ASCII representation"}\r\n

Exmple: 
Message from Nextion '66 00' (Nextion 'sendme' response)
* JSON>> {"D":"ESP-Test","MSG":"66 00 FF FF FF "}\r\n'

Note that Nextion responses are formatted and sent as ASCII representation of binary/hex emitted by Nextion

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
