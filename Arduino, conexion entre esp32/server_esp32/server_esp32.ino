#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32";
const char* password = "SantoDomingo";

WebServer server(80);

void setup() {
    Serial.begin(115200);
    delay(50);

    WiFi.softAP(ssid, password);
    IPAddress ip = WiFi.softAPIP();

    Serial.print("Nombre de mi red esp32: ");
    Serial.println(ssid);
    Serial.print("La ip es: ");
    Serial.println(ip);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/register", HTTP_POST, handleRegister);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Servidor HTTP iniciado");
}

void loop() {
    server.handleClient();
}

void handleRoot() {
    String html = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>Registrar Dispositivo</title>\
    </head>\
    <body>\
        <h1>Registrar Dispositivo</h1>\
        <form action=\"/register\" method=\"post\">\
            <label for=\"deviceName\">Nombre del Dispositivo:</label><br>\
            <input type=\"text\" id=\"deviceName\" name=\"deviceName\" required><br><br>\
            <label for=\"deviceType\">Tipo del Dispositivo:</label><br>\
            <select id=\"deviceType\" name=\"deviceType\" required>\
                <option value=\"Analogico\">Analogico</option>\
                <option value=\"Digital\">Digital</option>\
            </select><br><br>\
            <input type=\"submit\" value=\"Registrar\">\
        </form>\
    </body>\
    </html>";
    server.send(200, "text/html", html);
}

void handleRegister() {
    if (server.hasArg("deviceName") && server.hasArg("deviceType")) {
        String deviceName = server.arg("deviceName");
        String deviceType = server.arg("deviceType");
        Serial.println("Dispositivo registrado:");
        Serial.println("Nombre: " + deviceName);
        Serial.println("Tipo: " + deviceType);

        String response = "Dispositivo registrado:<br>Nombre: " + deviceName + "<br>Tipo: " + deviceType;
        server.send(200, "text/html", response);
    } else {
        server.send(400, "text/html", "Solicitud inválida");
    }
}

void handleNotFound() {
    server.send(404, "text/html", "Not found");
}
