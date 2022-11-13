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
    vTaskDelay(100 / portTICK_PERIOD_MS);
    _serial->begin(_baud, SERIAL_8N1, RXDN, TXDN);

    vTaskDelay(400 / portTICK_PERIOD_MS); //Pause for effect
    flushReads();
    
    //Reset Nextion Display
    vTaskDelay(100 / portTICK_PERIOD_MS);
    writeCmd("rest");
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
    if(_xSerialWriteSemaphore != NULL ){
      // if(_xSerialWriteSemaphore != NULL ){
        if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) ==
            pdTRUE) {
          _serial->print(command + _cmdTerminator);
          xSemaphoreGive(_xSerialWriteSemaphore);
        } else {
          // sLog.send("writeCmd Semaphore fail");
        }
      // }
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
  
