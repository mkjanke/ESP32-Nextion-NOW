#include "nextionInterface.h"

  /// @brief Class to handle communication with Nextion device.
  /// @param serial Serial port to which Nextion is attached
  /// @param baud Baud rate to be used for communication with Nextion
  myNextionInterface::myNextionInterface(HardwareSerial& serial, unsigned long baud) {
    _serial = &serial;
    _baud = baud;

    // Initialize semaphores
    _xSerialWriteSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(_xSerialWriteSemaphore);
    _xSerialReadSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(_xSerialReadSemaphore);
  }

  /// @brief Initialize Nextion Interface 
  ///        Flush serial interface, reset Nextion display
  /// @return true on success, false on failure.
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

  /// @brief Read and throw away serial input until no bytes or timeout
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

  /// @brief Write Nextion formatted 'Number' to Nextion display object 'val' property
  ///        via serial port
  /// @param _componentName Name of Nextion object/component
  /// @param _val Number to write
  void myNextionInterface::writeNum(const String& _componentName, uint32_t _val) {
    String _command = _componentName + "=" +_val;
    if(_xSerialWriteSemaphore != NULL ){
      if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) ==
          pdTRUE) {
        _serial->print(_command + _cmdTerminator);
        xSemaphoreGive(_xSerialWriteSemaphore);
      }
    }
  } //writeNum()

  /// @brief Write Nextion Arduino String object to Nextion display object 'txt' property 
  ///        via serial port
  /// @param command Name of Nextion object/component
  /// @param txt String to write
  void myNextionInterface::writeStr(const String& command, const String& txt) {
    String _command = command + "=\"" + txt + "\"";
    if(_xSerialWriteSemaphore != NULL ){
      if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) ==
          pdTRUE) {
        _serial->print(_command + _cmdTerminator);
        xSemaphoreGive(_xSerialWriteSemaphore);
      }
    }
  } //writeStr()

  /// @brief Write generic Nextion command to Nextion display via serial port
  /// @param command Name of Nextion object/component
  void myNextionInterface::writeCmd(const String& command) {
    if(_xSerialWriteSemaphore != NULL ){
        if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) ==
            pdTRUE) {
          _serial->print(command + _cmdTerminator);
          xSemaphoreGive(_xSerialWriteSemaphore);
        }
    }
  } //writeCmd()


  /// @brief Listen for data from Nextion device 
  ///        Call in a task or the loop() function periodically.
  /// @param _nexBytes std::string in which to place bytes from Nextion
  /// @param _size Max number of bytes to read
  /// @return Number of bytes read or 'false' if no bytes read
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
  
