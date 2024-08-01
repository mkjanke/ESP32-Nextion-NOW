#ifndef NEXTIONINTERFACE_H
#define NEXTIONINTERFACE_H

#include <Arduino.h>
#include "settings.h"

class myNextionInterface {
 private:
  HardwareSerial* _serial;
  unsigned long _baud;
  const char _cmdTerminator[3] = {'\xFF', '\xFF', '\xFF'};
  
  // Avoid smashing Nextion with overlapping reads and writes
  SemaphoreHandle_t _xSerialWriteSemaphore =  NULL;
  SemaphoreHandle_t _xSerialReadSemaphore = NULL;


 public:
  myNextionInterface(HardwareSerial&, unsigned long);

  bool begin();
  void flushReads();

  void writeNum(const String&, uint32_t);
  void writeStr(const String&, const String&); 
  void writeCmd(const String&);

  int listen(std::string&, uint8_t);
};

#endif  // NEXTIONINTERFACE_H