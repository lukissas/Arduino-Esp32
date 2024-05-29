#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ESP32";
const char* password = "SantoDomingo";

const char* esp32Server = "http://192.168.4.1/register";

const byte btn = 23;
const byte led = 22;
bool isLedOn = false;
String answer; 

void setup() {
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

void loop() {
    if (digitalRead(btn) == LOW && !isLedOn) {
        digitalWrite(led, LOW);
        isLedOn = true;
        Serial.println("\nled encendido!");
        sendDataToServer("LED", "Encendido");
        delay(250);
    } else if (digitalRead(btn) == HIGH && isLedOn) {
        digitalWrite(led, HIGH);
        isLedOn = false;
        sendDataToServer("LED", "Apagado");
        delay(250);
    }
}

void sendDataToServer(String deviceName, String deviceType) {
    if(WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(esp32Server);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String httpRequestData = "deviceName=" + deviceName + "&deviceType=" + deviceType;
        int httpResponseCode = http.POST(httpRequestData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Respuesta del servidor: " + response);
        } else {
            Serial.println("Error en la respuesta: " + String(httpResponseCode));
        }
        http.end();
    } else {
        Serial.println("Error en la conexión WiFi");
    }
}
