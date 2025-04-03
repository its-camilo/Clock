#pragma once

#include "Global.h"

class Clock {
public:
    int currentHour, currentMin, currentSec;
    Clock();
    void init(int hour, int min); 
    void updateTime(); 
};