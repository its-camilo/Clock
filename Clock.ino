#include "Global.h"
#include <JC_Button.h>
#include <DHT.h>
#define WIFI_TX_BUFFER_SIZE 512  
#define WIFI_RX_BUFFER_SIZE 512  
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>  // Necesitas instalar esta biblioteca

#include "Clock.h"
#include "DisplayHandler.h"

#include <HTTPClient.h>

// Configuración existente
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

hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {
    myClock.updateTime();
}

unsigned long currentMillis;

const char* ssid = "itscamilo";
const char* password = "itscamilo";
const char* thingspeakApiKey = "PKRPTFXQV5F6GXAI"; 
const char* thingspeakServer = "http://api.thingspeak.com/update";

// Crear servidor web en puerto 80
WebServer server(80);
// WebSocket server en puerto 81
WebSocketsServer webSocket = WebSocketsServer(81);

// Variables globales para las lecturas del sensor
float currentTemperature = 0;
float currentHumidity = 0;

// Buffer para mensajes seriales
String serialBuffer = "";
const int MAX_SERIAL_BUFFER = 1000; // Limitar el tamaño del buffer

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
      Serial.print("ThingSpeak response: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error en la petición HTTP: ");
      Serial.println(httpResponseCode);
    }
    http.end();

    delay(300);
    digitalWrite(14, LOW);
  } else {
    Serial.println("WiFi desconectado");
  }
}

// Función para manejar eventos de WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Conectado desde %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      {
        // Procesar comandos recibidos del cliente web
        String command = String((char*)payload);
        Serial.printf("Comando recibido: %s\n", command.c_str());
        
        // Procesar comandos específicos
        if (command.equals("Show")) {
          // Formatear hora actual con ceros a la izquierda si es necesario
          String formattedHour = (myClock.currentHour < 10 ? "0" : "") + String(myClock.currentHour);
          String formattedMin = (myClock.currentMin < 10 ? "0" : "") + String(myClock.currentMin);
          String formattedSec = (myClock.currentSec < 10 ? "0" : "") + String(myClock.currentSec);
          
          // Crear mensaje con todos los datos actuales
          String statusMessage = "Datos actuales \n";
          statusMessage += "Hora: " + formattedHour + ":" + formattedMin + ":" + formattedSec + "\n";
          statusMessage += "Temperatura: " + String(currentTemperature, 1) + "°C\n";
          statusMessage += "Humedad: " + String(currentHumidity, 1) + "%\n";
          
          // Enviar mensaje al cliente que envió el comando
          webSocket.sendTXT(num, statusMessage);
          
          // También mostrar en Serial para depuración
          Serial.println(statusMessage);
        }
        else if (command.startsWith("Set")) {
          // Extraer el formato de hora hh:mm
          String timeStr = command.substring(4); // Obtiene la parte después de "Set "
          timeStr.trim(); // Elimina espacios en blanco
    
          // Verificar si el formato es correcto (hh:mm)
          if (timeStr.length() == 5 && timeStr.charAt(2) == ':') {
            // Extraer horas y minutos
            int hh = timeStr.substring(0, 2).toInt();
            int mm = timeStr.substring(3, 5).toInt();
        
            // Validar que los valores sean correctos
            if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) {
              // Llamar a la función para inicializar el reloj
              myClock.init(hh, mm);
              webSocket.sendTXT(num, "Reloj configurado a " + timeStr);
            } 
            else {
                webSocket.sendTXT(num, "Error: formato de hora inválido. Use hh:mm (00-23:00-59)");
            }
          }
            else {
                webSocket.sendTXT(num, "Error: formato incorrecto. Use 'Set hh:mm'");
            }
        }
        else {
          // Respuesta genérica para comandos no reconocidos
          webSocket.sendTXT(num, "Comando no reconocido: " + command);
        }
      }
      break;
  }
}

// Agrega esto al final de tu función setup()
void redirectSerial() {
  // Redirigir Serial.print() a WebSocket
  freopen(NULL, "w", stdout);
  setvbuf(stdout, NULL, _IONBF, 0);
  
  // Sustituir la implementación de Serial.write
  Serial.flush();
}

// Manejador para la página principal
void handleRoot() {
  // Código existente
}

// Manejador para la interfaz de terminal
void handleTerminal() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Terminal Web</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: 'Courier New', monospace;
            margin: 0;
            padding: 20px;
            background-color: #1e1e1e;
            color: #ddd;
        }
        #terminal {
            background-color: #000;
            color: #0f0;
            padding: 10px;
            height: 400px;
            overflow-y: auto;
            border-radius: 5px;
            font-size: 14px;
            line-height: 1.5;
        }
        #commandInput {
            width: 100%;
            background-color: #333;
            color: #fff;
            border: none;
            padding: 10px;
            margin-top: 10px;
            border-radius: 3px;
            font-family: 'Courier New', monospace;
        }
        h1 {
            color: #0f0;
        }
        .time {
            color: #aaa;
            font-size: 12px;
            margin-right: 5px;
        }
    </style>
</head>
<body>
    <h1>ESP32 Terminal Web</h1>
    <div id="terminal"></div>
    <input type="text" id="commandInput" placeholder="Escribe un comando y presiona Enter" />
    
    <script>
        const terminal = document.getElementById('terminal');
        const commandInput = document.getElementById('commandInput');
        let socket;
        
        // Función para conectar al WebSocket
        function connectWebSocket() {
            // Usar la dirección actual del servidor pero con puerto diferente
            const host = window.location.hostname;
            socket = new WebSocket(`ws://${host}:81`);
            
            socket.onopen = function(e) {
                addToTerminal("Conexion establecida", "system");
            };
            
            socket.onclose = function(e) {
                addToTerminal("Conexion cerrada. Reconectando en 5 segundos...", "system");
                setTimeout(connectWebSocket, 5000);
            };
            
            socket.onerror = function(e) {
                addToTerminal("Error de conexion", "error");
            };
            
            socket.onmessage = function(e) {
                addToTerminal(e.data, "receive");
            };
        }
        
        // Función para agregar texto al terminal
        function addToTerminal(text, type) {
            const timestamp = new Date().toLocaleTimeString();
            const entry = document.createElement('div');
            entry.innerHTML = `<span class="time">[${timestamp}]</span> ${text}`;
            
            if (type === "system") {
                entry.style.color = "#ff9900";
            } else if (type === "error") {
                entry.style.color = "#ff3333";
            } else if (type === "send") {
                entry.style.color = "#3399ff";
            }
            
            terminal.appendChild(entry);
            terminal.scrollTop = terminal.scrollHeight;
        }
        
        // Manejar entrada de comandos
        commandInput.addEventListener('keydown', function(e) {
            if (e.key === 'Enter') {
                const command = commandInput.value;
                if (command) {
                    addToTerminal(`> ${command}`, "send");
                    socket.send(command);
                    commandInput.value = '';
                }
            }
        });
        
        // Iniciar conexión WebSocket
        connectWebSocket();
        
        // Solicitar datos periódicamente
        setInterval(function() {
            fetch('/api')
            .then(response => response.json())
            .then(data => {
                const statusLine = `Hora: ${data.hour}:${data.minute}:${data.second} | Temp: ${data.temperature}C | Hum: ${data.humidity}%`;
                document.title = `ESP32 Terminal (${data.hour}:${data.minute}:${data.second})`;
                // Actualizar solo si cambió el estado
                if (terminal.lastChild && !terminal.lastChild.classList.contains('status')) {
                    const status = document.createElement('div');
                    status.classList.add('status');
                    status.style.color = "#7799ff";
                    status.style.fontSize = "12px";
                    status.style.borderBottom = "1px dashed #333";
                    status.innerHTML = statusLine;
                    terminal.appendChild(status);
                    terminal.scrollTop = terminal.scrollHeight;
                }
            });
        }, 1000);
    </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

// Devolver datos en formato JSON para aplicaciones
void handleAPI() {
  String json = "{\"hour\":" + String(myClock.currentHour) + 
                ",\"minute\":" + String(myClock.currentMin) + 
                ",\"second\":" + String(myClock.currentSec) + 
                ",\"temperature\":" + String(currentTemperature) + 
                ",\"humidity\":" + String(currentHumidity) + "}";
  server.send(200, "application/json", json);
}

// Redireccionar la salida Serial al WebSocket
void serialOutput(const char *message) {
  serialBuffer += message;
  
  // Limitar el tamaño del buffer
  if (serialBuffer.length() > MAX_SERIAL_BUFFER) {
    serialBuffer = serialBuffer.substring(serialBuffer.length() - MAX_SERIAL_BUFFER);
  }
  
  // Enviar a todos los clientes WebSocket conectados
  webSocket.broadcastTXT(message);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando monitor de reloj con WiFi");

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a WiFi. IP: ");
  Serial.println(WiFi.localIP());

  // Configurar mDNS para acceso fácil
  if (MDNS.begin("reloj")) {
    Serial.println("mDNS iniciado - accede a http://reloj.local");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("No se pudo iniciar la pantalla OLED");
      for (;;) ;
  }
  
  pinMode(14, OUTPUT);

  dht.begin();

  setBtn.begin();
  minBtn.begin();
  hourBtn.begin();

  myClock.init(12, 0); 

  // Usando la versión simple del temporizador compatible con tu biblioteca
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);

  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/terminal", handleTerminal);  // Nueva ruta para la terminal web
  server.on("/api", handleAPI);  // Endpoint para obtener datos en formato JSON
  
  // Iniciar servidor web
  server.begin();
  
  // Iniciar WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("Servidor HTTP iniciado");
  Serial.println("Terminal WebSocket iniciado en puerto 81");
  Serial.println("Accede a http://reloj.local/terminal para ver la terminal web");
}

void loop() {
  currentMillis = millis();
  server.handleClient();  // Manejar clientes web
  webSocket.loop();       // Manejar conexiones WebSocket
 
  setBtn.read();
  minBtn.read();
  hourBtn.read();

  updateDisplay();
  
  // Ejemplo de mensaje serial periódico para ver en la terminal web
  static unsigned long lastSerialOutput = 0;
  if (currentMillis - lastSerialOutput >= 5000) {
    Serial.println("Estado: Hora=" + String(myClock.currentHour) + ":" + 
                  (myClock.currentMin < 10 ? "0" : "") + String(myClock.currentMin) + ":" + 
                  (myClock.currentSec < 10 ? "0" : "") + String(myClock.currentSec) + 
                  " Temp=" + String(currentTemperature) + "C Hum=" + String(currentHumidity) + "%");
    lastSerialOutput = currentMillis;
  }
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
        currentHumidity = dht.readHumidity();
        currentTemperature = dht.readTemperature();
        if (isnan(currentHumidity) || isnan(currentTemperature)) {
          Serial.println("Error al leer del sensor DHT");
        }
        displayHandler.showSensorReads(currentHumidity, currentTemperature);  
        
        static unsigned long lastThingSpeakSend = 0;
        if (currentMillis - lastThingSpeakSend >= 15000) {
          sendToThingSpeak(currentTemperature, currentHumidity);
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