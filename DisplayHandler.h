#pragma once

#include "Global.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayHandler {
public:
    DisplayHandler(Adafruit_SSD1306& display);
    void showTime(int hour, int min, int sec);
    void showSensorReads(float h, float t);
private:
    Adafruit_SSD1306& display;
};
