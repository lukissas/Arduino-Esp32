#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ESP32";
const char* password = "SantoDomingo";

const char* esp32Server = "http://192.168.4.1/list";

const byte btn = 23;
const byte led = 22;
bool isLedOn = false;
String answer; 

void setup(){
    Serial.begin(115200);

    pinMode(btn, INPUT_PULLDOWN);
    pinMode(led, OUTPUT);

    WiFi.begin(ssid, password);
    Serial.println("Conectando");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Conectado a la red con la IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void loop(){
    answer = getRequest(esp32Server);
    Serial.println("Respuesta del ESP32 Servidor:");
    Serial.println(answer);
    delay(5000); // Cambié a 5 segundos para menos spam en el Serial Monitor

    if (digitalRead(btn) == LOW && !isLedOn) {
        digitalWrite(led, LOW);
        isLedOn = true;
        Serial.println("\nLED encendido!");
        answer = getRequest(esp32Server);
        Serial.println("Respuesta del ESP32 Servidor:");
        Serial.println(answer);
        delay(250);
    } else {
        digitalWrite(led, HIGH);
        isLedOn = false;
    }
}

String getRequest(const char* serverName) {
    HTTPClient http;
    Serial.print("Haciendo solicitud GET a: ");
    Serial.println(serverName);
    
    http.begin(serverName);
    int httpResponseCode = http.GET();
    String payload = "...";

    if (httpResponseCode > 0) {
        Serial.print("Código de respuesta HTTP: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
        Serial.println("Respuesta del servidor:");
        Serial.println(payload);
    } else {
        Serial.print("Código de error HTTP: ");
        Serial.println(httpResponseCode);
        Serial.println("Posibles causas: URL incorrecta, servidor no disponible, recurso no encontrado.");
    }
    http.end();

    return payload;
}

