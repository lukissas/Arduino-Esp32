#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// WiFi
const char* ssid = "ESP32";
const char* password = "SantoDomingo";
const int maxDevices = 5;

// MQTT
const char* mqtt_server = "0969a134f3274e87870416b38ae8aa59.s1.eu.hivemq.cloud"; // URL del clúster MQTT
const int mqtt_port = 8883; // Puerto TLS MQTT
const char* mqtt_user = "Lucas"; // Usuario MQTT
const char* mqtt_password = "874681Lu"; // Contraseña MQTT
const char* mqtt_topic = "esp32/devices";

#define BOT_TOKEN "7458068961:AAGKIm267e8v8_eJUpvTWqKkg07niaBhMdE" 
#define CHAT_ID "6154482804"  

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
UniversalTelegramBot bot(BOT_TOKEN, espClient);

WebServer server(80);
Preferences preferences;

int deviceCount = 0;

void setup() {
    Serial.begin(115200);
    delay(50);

    WiFi.softAP(ssid, password);
    IPAddress ip = WiFi.softAPIP();

    Serial.print("Nombre de mi red ESP32: ");
    Serial.println(ssid);
    Serial.print("La IP es: ");
    Serial.println(ip);

    preferences.begin("deviceData", false);
    deviceCount = preferences.getInt("deviceCount", 0);

    espClient.setInsecure(); // Usar conexión insegura (puedes cambiar esto si tienes el certificado de CA)

    mqttClient.setServer(mqtt_server, mqtt_port);
    connectToMQTT();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/register", HTTP_POST, handleRegister);
    server.on("/list", HTTP_GET, handleList);
    server.on("/delete", HTTP_GET, handleDelete);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Servidor HTTP iniciado");
}

void loop() {
    server.handleClient();
    if (!mqttClient.connected()) {
        connectToMQTT();
    }
    mqttClient.loop();
}

void connectToMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("Conectando a MQTT...");
        if (mqttClient.connect("ESP32Client", mqtt_user, mqtt_password)) { // Conectar con usuario y contraseña
            Serial.println("conectado");
        } else {
            Serial.print("falló, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" intentar de nuevo en 5 segundos");
            delay(5000);
        }
    }
}

void handleRoot() {
    String html = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>Registrar Dispositivo</title>\
        <style>\
            body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }\
            .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\
            h1 { text-align: center; color: #333; }\
            form { display: flex; flex-direction: column; }\
            label { margin-bottom: 10px; color: #555; }\
            input, select { padding: 10px; margin-bottom: 20px; border: 1px solid #ccc; border-radius: 5px; }\
            input[type='submit'] { background-color: #007BFF; color: white; border: none; cursor: pointer; }\
            input[type='submit']:hover { background-color: #0056b3; }\
            a { text-decoration: none; color: #007BFF; }\
            a:hover { text-decoration: underline; }\
            .device-list { margin-top: 20px; }\
            .device-list p { background: #e9ecef; padding: 10px; border-radius: 5px; }\
        </style>\
    </head>\
    <body>\
        <div class='container'>\
            <h1>Registrar Dispositivo</h1>\
            <form action=\"/register\" method=\"post\">\
                <label for=\"deviceName\">Nombre del Dispositivo:</label>\
                <input type=\"text\" id=\"deviceName\" name=\"deviceName\" required>\
                <label for=\"deviceType\">Tipo del Dispositivo:</label>\
                <select id=\"deviceType\" name=\"deviceType\" required>\
                    <option value=\"Sensor\">Sensor</option>\
                    <option value=\"Actuador\">Actuador</option>\
                    <option value=\"Controlador\">Controlador</option>\
                </select>\
                <input type=\"submit\" value=\"Registrar\">\
            </form>\
            <div class='device-list'>\
                <h2>Dispositivos Registrados</h2>\
                <a href=\"/list\">Ver Lista de Dispositivos</a>\
            </div>\
        </div>\
    </body>\
    </html>";
    server.send(200, "text/html", html);
}

void handleRegister() {
    if (deviceCount >= maxDevices) {
        String response = "<!DOCTYPE html>\
        <html>\
        <head>\
            <title>Límite Alcanzado</title>\
            <style>\
                body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }\
                .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); text-align: center; }\
                h1 { color: #333; }\
                p { color: #555; }\
                a { text-decoration: none; color: #007BFF; }\
                a:hover { text-decoration: underline; }\
            </style>\
        </head>\
        <body>\
            <div class='container'>\
                <h1>Límite de Dispositivos Alcanzado</h1>\
                <p>No se pueden registrar más de " + String(maxDevices) + " dispositivos.</p>\
                <a href=\"/\">Volver a la página principal</a>\
            </div>\
        </body>\
        </html>";
        server.send(200, "text/html", response);
        return;
    }

    if (server.hasArg("deviceName") && server.hasArg("deviceType")) {
        String deviceName = server.arg("deviceName");
        String deviceType = server.arg("deviceType");

        deviceCount++;
        String deviceKey = "device" + String(deviceCount);

        preferences.putString((deviceKey + "_name").c_str(), deviceName);
        preferences.putString((deviceKey + "_type").c_str(), deviceType);
        preferences.putInt("deviceCount", deviceCount);

        Serial.println("Dispositivo registrado:");
        Serial.println("Nombre: " + deviceName);
        Serial.println("Tipo: " + deviceType);

        String message = "Dispositivo registrado:\nNombre: " + deviceName + "\nTipo: " + deviceType;
        bot.sendMessage(CHAT_ID, message, "");
        mqttClient.publish(mqtt_topic, message.c_str());

        String response = "<!DOCTYPE html>\
        <html>\
        <head>\
            <title>Dispositivo Registrado</title>\
            <style>\
                body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }\
                .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); text-align: center; }\
                h1 { color: #333; }\
                p { color: #555; }\
                a { text-decoration: none; color: #007BFF; }\
                a:hover { text-decoration: underline; }\
            </style>\
        </head>\
        <body>\
            <div class='container'>\
                <h1>Dispositivo Registrado</h1>\
                <p>Nombre: " + deviceName + "</p>\
                <p>Tipo: " + deviceType + "</p>\
                <a href=\"/\">Registrar Otro Dispositivo</a>\
            </div>\
        </body>\
        </html>";
        server.send(200, "text/html", response);
    } else {
        server.send(400, "text/html", "Solicitud inválida");
    }
}

void handleList() {
    String response = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>Lista de Dispositivos</title>\
        <style>\
            body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }\
            .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\
            h1 { text-align: center; color: #333; }\
            .device { background: #e9ecef; padding: 10px; border-radius: 5px; margin-bottom: 10px; }\
            .device .delete-link { float: right; color: red; font-weight: bold; text-decoration: none; }\
            a { text-decoration: none; color: #007BFF; display: block; text-align: center; margin-top: 20px; }\
            a:hover { text-decoration: underline; }\
        </style>\
    </head>\
    <body>\
        <div class='container'>\
            <h1>Lista de Dispositivos Registrados</h1>";

    for (int i = 1; i <= deviceCount; i++) {
        String deviceKey = "device" + String(i);
        String deviceName = preferences.getString((deviceKey + "_name").c_str(), "N/A");
        String deviceType = preferences.getString((deviceKey + "_type").c_str(), "N/A");

        response += "<div class='device'><p>Dispositivo " + String(i) + ":</p>";
        response += "<p>Nombre: " + deviceName + "</p>";
        response += "<p>Tipo: " + deviceType + "</p>";
        response += "<a href=\"/delete?deviceId=" + String(i) + "\" class=\"delete-link\" onclick=\"return confirm('¿Estás seguro de que quieres eliminar este dispositivo?');\">&times;</a></div>";
    }

    response += "<a href=\"/\">Registrar Otro Dispositivo</a>\
        </div>\
    </body>\
    </html>";

    server.send(200, "text/html", response);
}

void handleDelete() {
    if (server.hasArg("deviceId")) {
        int deviceId = server.arg("deviceId").toInt();

        if (deviceId > 0 && deviceId <= deviceCount) {
            String deviceName = preferences.getString(("device" + String(deviceId) + "_name").c_str());
            String deviceType = preferences.getString(("device" + String(deviceId) + "_type").c_str());

            for (int i = deviceId; i < deviceCount; i++) {
                String currentKey = "device" + String(i);
                String nextKey = "device" + String(i + 1);
                String nextName = preferences.getString((nextKey + "_name").c_str());
                String nextType = preferences.getString((nextKey + "_type").c_str());

                preferences.putString((currentKey + "_name").c_str(), nextName);
                preferences.putString((currentKey + "_type").c_str(), nextType);
            }

            String lastKey = "device" + String(deviceCount);
            preferences.remove((lastKey + "_name").c_str());
            preferences.remove((lastKey + "_type").c_str());

            deviceCount--;
            preferences.putInt("deviceCount", deviceCount);

            String message = "Dispositivo eliminado:\nNombre: " + deviceName + "\nTipo: " + deviceType;
            bot.sendMessage(CHAT_ID, message, "");
            mqttClient.publish(mqtt_topic, message.c_str());

            server.sendHeader("Location", "/list");
            server.send(303);
        } else {
            server.send(400, "text/html", "ID de dispositivo inválido");
        }
    } else {
        server.send(400, "text/html", "Solicitud inválida");
    }
}

void handleNotFound() {
    server.send(404, "text/html", "Not found");
}
