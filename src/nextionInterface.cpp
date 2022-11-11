#include "nextionInterface.h"

  myNextionInterface::myNextionInterface(HardwareSerial& serial, unsigned long baud) {
    _serial = &serial;
    _baud = baud;

    // Initialize semaphores
    _xSerialWriteSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(_xSerialWriteSemaphore);

    _xSerialReadSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(_xSerialReadSemaphore);
  }

  bool myNextionInterface::begin() {
    // sLog.send("Starting myNex", true);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    _serial->begin(_baud, SERIAL_8N1, RXDN, TXDN);

    vTaskDelay(400 / portTICK_PERIOD_MS); //Pause for effect
    flushReads();
    
    //Reset Nextion
    vTaskDelay(100 / portTICK_PERIOD_MS);
    // sLog.send("Resetting Display");
    writeCmd("rest");

      // Set up queues and tasks
    read_from_Nextion_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, ESP_BUFFER_SIZE);
    if (read_from_Nextion_queue == NULL) {
      Serial.println("Create Queue failed");
      return false;
    }
    return true;
  }

  // Read and throw away serial input until no bytes (or timeout)
  void myNextionInterface::flushReads(){
    if(_xSerialReadSemaphore != NULL ){
      if (xSemaphoreTake(_xSerialReadSemaphore, 400 / portTICK_PERIOD_MS)) {
        unsigned long _timer = millis();
        while ((_serial->available() > 0) && (millis() - _timer) < 400L ) {
          _serial->read();  // Start with clear serial port.
        }
        xSemaphoreGive(_xSerialReadSemaphore);
      }
    }
  }

  // Thread safe writes (I think...)
  void myNextionInterface::writeNum(const String& _componentName, uint32_t _val) {
    String _command = _componentName + "=" +_val;
    // sLog.send("writeNum: " + _command);
    if(_xSerialWriteSemaphore != NULL ){
      if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) ==
          pdTRUE) {
        _serial->print(_command + _cmdTerminator);
        xSemaphoreGive(_xSerialWriteSemaphore);
      } else {
        // sLog.send("writeNum Semaphore fail");
      }
    }
  } //writeNum()

  void myNextionInterface::writeStr(const String& command, const String& txt) {
    String _command = command + "=\"" + txt + "\"";
    // sLog.send("writeStr: " + _command);
    if(_xSerialWriteSemaphore != NULL ){
      if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) ==
          pdTRUE) {
        _serial->print(_command + _cmdTerminator);
        xSemaphoreGive(_xSerialWriteSemaphore);
      } else {
        // sLog.send("writeNum Semaphore fail");
      }
    }
  } //writeStr()

  void myNextionInterface::writeCmd(const String& command) {
    // sLog.send("writeCmd: " + command);
    if(_xSerialWriteSemaphore != NULL ){
      if(_xSerialWriteSemaphore != NULL ){
        if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) ==
            pdTRUE) {
          _serial->print(command + _cmdTerminator);
          xSemaphoreGive(_xSerialWriteSemaphore);
        } else {
          // sLog.send("writeCmd Semaphore fail");
        }
      }
    }
  } //writeCmd()

  // This gets called in a task or the loop().
  // Pass std::string and maximum bytes to be read
  // Returns actual bytes read and populated string
  // or 'false' if no bytes read
  int myNextionInterface::listen(std::string& _nexBytes, uint8_t _size) {
    if(_xSerialReadSemaphore != NULL ){
      if (xSemaphoreTake(_xSerialReadSemaphore, 100 / portTICK_PERIOD_MS)) {
        if (_serial->available() > 0) {
          int _byte_read = 0;
          uint8_t _terminatorCount = 0;       // Expect three '\xFF' bytes at end of each Nextion command 
          unsigned long _timer = millis();

          while ((_nexBytes.length() < _size) && (_terminatorCount < 3) && (millis() - _timer) < 400L) {
            _byte_read = _serial->read();
            if (_byte_read != -1) {           // HardwareSerial::read returns -1 if no bytes available
              _nexBytes.push_back(_byte_read);
            }
            if (_byte_read == '\xFF') _terminatorCount++;
          }
          xSemaphoreGive(_xSerialReadSemaphore);
          // Queue
          if (_nexBytes.length() <= ESP_BUFFER_SIZE) {
            // Convert to ASCII triplets and Queue
              if (_nexBytes.length() > 3) {
                std::string _hexString;
                char _x[3] = {};
                _hexString.reserve(_nexBytes.length() * 3);
                for (const auto& item : _nexBytes) {
                  sprintf(_x, "%02X ", item);
                  _hexString += _x;
                }
                Serial.println((String)_hexString.c_str());
                if (xQueueSend(read_from_Nextion_queue, (void *)_hexString.c_str(), 0) != pdTRUE) {
                  Serial.println("Error sending to Nextion queue");
                }
              }
          }
          return _nexBytes.length();
        }
        xSemaphoreGive(_xSerialReadSemaphore);
        return false;
      } else {
        return false;
      }
    }
    else
      return false;
  }  //listen()
  
