#ifndef ESPNOW_H
#define ESPNOW_H

#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "settings.h"
#include "nextionInterface.h"

bool initEspNow();
int espNowSend(const String&);
int espNowSend(const JsonDocument&);
extern myNextionInterface myNex;

#endif