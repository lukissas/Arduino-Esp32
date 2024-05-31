#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ESP32";
const char* password = "SantoDomingo";

const char* esp32Server = "http://192.168.4.1/device1";

const byte btn = 23;
const byte led = 22;
bool isLedOn = false;
String answer; 

void setup(){
    Serial.begin(115200);

    pinMode(btn, INPUT_PULLDOWN);
    pinMode(led, OUTPUT);

    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Conectado a la red con la ip: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void loop(){
    answer = getRequest(esp32Server);
    Serial.println("Respuesta del Esp32 Servidor: ");
    Serial.println(answer);
    delay(1000); 

    if (digitalRead(btn) == LOW && !isLedOn) {
        digitalWrite(led, LOW);
        isLedOn = true;
        Serial.println("\nled encendido!");
        answer = getRequest(esp32Server);
        Serial.println("Respuesta del Esp32 Servidor: ");
        Serial.println(answer);
        delay(250);
    } else {
        digitalWrite(led, HIGH);
    }
}

String getRequest(const char* serverName) {
    HTTPClient http;
    http.begin(serverName);

    int httpResponseCode = http.GET();

    String payload = "...";

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();

    return payload; 
}
