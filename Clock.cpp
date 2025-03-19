#include "Clock.h"

Clock::Clock() {
    currentSec = 0;
}

void Clock::init(int hour, int min) {
    currentHour = hour;
    currentMin = min;
}

void Clock::updateTime() {
  currentSec++;
  if (currentSec >= 60) {
    currentSec = 0;
    currentMin++;
    if (currentMin >= 60) {
      currentMin = 0;
      currentHour = (currentHour + 1) % 24;
    }
  }
}
