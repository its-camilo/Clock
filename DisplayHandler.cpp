#include "DisplayHandler.h"

DisplayHandler::DisplayHandler(Adafruit_SSD1306& disp):display(disp) {}

void DisplayHandler::showTime(int hour, int min, int sec) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);

    display.print("SET TIME");

    display.setTextSize(2);

    display.setCursor(10, 30);
    display.printf("%02d:%02d:%02d", hour, min, sec);
    display.display();
}

void DisplayHandler::showSensorReads(float h, float t)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(22, 15);
    display.print("Humedad: ");
    display.print(h);
    display.print("%");
    display.setCursor(11, 45);
    display.print("Temperatura: ");
    display.print(t);
    display.print("C");
    display.display();
}
