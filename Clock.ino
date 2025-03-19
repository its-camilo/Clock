#include "Global.h"

//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <JC_Button.h>
#include <DHT.h>
// You can add these definitions before including WiFi libraries
#define WIFI_TX_BUFFER_SIZE 512  // Reduce from default
#define WIFI_RX_BUFFER_SIZE 512  // Reduce from default
#include <WiFi.h>
#include <HTTPClient.h> // Agregar la biblioteca HTTP para enviar solicitudes HTTP

#include "Clock.h"
#include "DisplayHandler.h"
#include "BluetoothSerial.h"
BluetoothSerial BT;

Button setBtn(12), minBtn(5), hourBtn(23);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int DHTPin = 4;
#define DHTTYPE DHT22
DHT dht(DHTPin, DHTTYPE);

const unsigned int pressedTime=1500;
const unsigned int incrementTime =300;

unsigned long lastPressedMinTime, lastPressedHourTime;
boolean minIncrementActive=false,hourIncrementActive=false;

Clock myClock;
DisplayHandler displayHandler(display);

/* create a hardware timer */
hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {
    myClock.updateTime();
}

unsigned long currentMillis;

// Configuración WiFi y ThingSpeak
// Store constant strings in program memory
const char* ssid = "Tinker Bell";
const char* password = "Peterpan2023";
const char* thingspeakApiKey = "PKRPTFXQV5F6GXAI"; 
const char* thingspeakServer = "http://api.thingspeak.com/update";

// Función para enviar datos a ThingSpeak y parpadear el LED
void sendToThingSpeak(float temperature, float humidity) {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(14, HIGH);
    HTTPClient http;
    String url = String(thingspeakServer) + "?api_key=" + thingspeakApiKey 
                 + "&field1=" + String(temperature) 
                 + "&field2=" + String(humidity);
    
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println(httpResponseCode);
    } else {
      Serial.println(httpResponseCode);
    }
    http.end();

    delay(300);
    digitalWrite(14, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("1");

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      for (;;) ;
  }
  
  pinMode(14, OUTPUT);
  BT.begin("ESP32");

  dht.begin();

  setBtn.begin();
  minBtn.begin();
  hourBtn.begin();

  myClock.init(12, 0); 

  //----- timer settings ---------------------//
  // Set timer frequency to 1Mhz
  timer = timerBegin(1000000);
  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer);
  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter).
  timerAlarm(timer, 1000000, true, 0);
  //-----------------------------------------//
}

void loop() {
  currentMillis = millis();
 
  setBtn.read();
  minBtn.read();
  hourBtn.read();

  updateDisplay();
}

void updateDisplay() {
  static int previousMillis = millis();
  static int step = 0;
  
  if(setBtn.isReleased()) {
    switch(step) {
      case 0:
        displayHandler.showTime(myClock.currentHour, myClock.currentMin, myClock.currentSec);
        if(currentMillis - previousMillis >= 5000) {
          step = 1;
          previousMillis = currentMillis;
        }
        break;

      case 1:
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (isnan(h) || isnan(t)) {
        }
        displayHandler.showSensorReads(h, t);  
        
        // Enviar datos a ThingSpeak cada 15 segundos para no exceder el límite
        static unsigned long lastThingSpeakSend = 0;
        if (currentMillis - lastThingSpeakSend >= 15000) {
          sendToThingSpeak(t, h);
          lastThingSpeakSend = currentMillis;
        }
        
        if(currentMillis - previousMillis >= 5000) {
          step = 0;
          previousMillis = currentMillis;
        }
        break;
    }
  } else {
    displayHandler.showTime(myClock.currentHour, myClock.currentMin, myClock.currentSec);
    step = 0;
    previousMillis = currentMillis;

    if (minBtn.wasPressed()) {
      myClock.init(myClock.currentHour, (myClock.currentMin + 1) % 60);
      lastPressedMinTime = currentMillis;
      minIncrementActive = false;
    }

    if (hourBtn.wasPressed()) {
      myClock.init((myClock.currentHour + 1) % 24, myClock.currentMin);
      lastPressedHourTime = currentMillis;
      hourIncrementActive = false;
    }

    if (minBtn.isPressed()) {
      if (currentMillis - lastPressedMinTime > pressedTime) {
        minIncrementActive = true;
        lastPressedMinTime = currentMillis;
        myClock.init(myClock.currentHour, (myClock.currentMin + 1) % 60);
      }
      if ((currentMillis - lastPressedMinTime > incrementTime) && minIncrementActive) {
        lastPressedMinTime = currentMillis;
        myClock.init(myClock.currentHour, (myClock.currentMin + 1) % 60);
      }
    }

    if (hourBtn.isPressed()) {
      if (currentMillis - lastPressedHourTime > pressedTime) {
        hourIncrementActive = true;
        lastPressedHourTime = currentMillis;
        myClock.init((myClock.currentHour + 1) % 24, myClock.currentMin);
      }
      if ((currentMillis - lastPressedHourTime > incrementTime) && hourIncrementActive) {
        lastPressedHourTime = currentMillis;
        myClock.init((myClock.currentHour + 1) % 24, myClock.currentMin);
      }
    }
  }
}
