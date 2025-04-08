# Clock

### Repositorio de un programa desarrollado en C++ para ejecutarse en una placa ESP32. El proyecto integra los siguientes componentes:
- Una pantalla electrónica para visualizar datos en tiempo real.
- Un sensor DHT22 para medir temperatura y humedad ambiental.
- Tres botones para interacción con el usuario (ajuste de parámetros).
- Un LED como indicador visual.
 
El sistema incluye funcionalidades de reloj integrado, que inicia en 12:00 al encenderse y permite:
 - Mostrar la hora actual en la pantalla.
 - Ajustar horas y minutos mediante los botones (incremento programado).

El código combina la lectura del sensor, la gestión del reloj y la interacción con los componentes, mostrando los datos en la pantalla y activando el LED bajo condiciones específicas. Incluye esquemas de conexión y guía de configuración.

### Repository for a C++ program designed to run on an ESP32 microcontroller. The project integrates the following components:
- An electronic display for real-time data visualization.
- A DHT22 sensor to measure ambient temperature and humidity.
- Three buttons for user interaction (parameter adjustment).
- An LED for visual feedback (e.g., system status or alerts).

The system features an embedded clock functionality, which:
 - Starts at 12:00 on power-up.
 - Displays the current time on the screen.
 - Allows time adjustment (hours/minutes) via the buttons (incremental control).

The code synchronizes sensor readings, clock management, and component interaction, displaying data on the screen and triggering the LED based on predefined conditions. Includes wiring diagrams and setup instructions.

Nota/Note: La imagen adjunta muestra la conexión de los componentes / The attached image displays the component conection.

![Image](https://github.com/user-attachments/assets/0aa15cd4-fa61-4447-8ab6-10438bd9e862)
![Image](https://github.com/user-attachments/assets/4f8c65d9-be53-4c49-99c6-b9d0de8be723) 
 
### Subida de datos a ThingSpeak: Los valores de temperatura y humedad se envían a un canal de ThingSpeak personalizable.
 - El usuario puede seleccionar su canal modificando la API Key y el ID del canal en el archivo Clock.ino.
 - La imagen adjunta muestra los datos recopilados en el canal predeterminado.

### ThingSpeak data upload: Temperature and humidity values are sent to a customizable ThingSpeak channel.
 - Users can select their channel by updating the API Key and Channel ID in the Clock.ino file.
 - The attached image displays data from the default channel.

![Image](https://github.com/user-attachments/assets/39d0a57e-d0d4-45ad-babc-ce387ae1e7f7)

### El reloj integrado cuenta con control remoto mediante una interfaz web. Para su uso:
- Configuración de red:
 - El ESP32 se conecta a una red WiFi definida por el usuario (SSID y contraseña modificables en el archivo Clock.ino).
- Acceso al terminal web:
 - Desde cualquier dispositivo conectado a la misma red (celular, computadora, tablet, etc.), ingrese a reloj.local/terminal en el navegador.
- Comandos disponibles:
 - Show: Muestra en pantalla la hora actual, temperatura y humedad registradas.
 - Set hh:mm: Ajusta la hora del reloj al valor especificado (ej. Set 15:30), sin necesidad de usar los botones físicos.

Las imágenes adjuntas ilustran ejemplos prácticos de la interfaz del terminal y su funcionamiento.

### The embedded clock features remote control via a web interface. Usage steps:
- Network setup:
 - The ESP32 connects to a user-defined WiFi network (SSID and password can be updated in the Clock.ino file).
- Web terminal access:
 - From any device on the same network (phone, computer, tablet, etc.), navigate to reloj.local/terminal in a browser.
- Available commands:
 - Show: Displays the current time, temperature, and humidity readings.
 - Set hh:mm: Sets the clock to the specified time (e.g., Set 15:30), bypassing physical buttons.

Attached images demonstrate the web terminal interface and command examples.

Nota/Note: Las imágenes muestran el uso del terminal web / Images show the web terminal in action.

![Image](https://github.com/user-attachments/assets/0da4fbaf-8cb9-4b13-bb99-2bbcb9ef6f52)
![Image](https://github.com/user-attachments/assets/f5b4e6e0-efd6-4b14-86c9-5c3103d9af1e)
