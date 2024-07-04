#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

const char* WIFI_SSID = "";
const char* WIFI_PASS = "";

AsyncWebServer server(80);

fs::LittleFSFS WebsiteFS;
fs::LittleFSFS ConfigFS;

void setupPartition(fs::LittleFSFS& fs, const char* partition_label, const char* mount_point) {
    Serial.printf("Mounting \"%s\" partition: ", partition_label);

    bool paritition_is_mounted = fs.begin(true, mount_point, 10, partition_label);

    if (paritition_is_mounted) {
        Serial.println("success");
    } else
        Serial.println("failed");
}

void setupPartitions() {
    setupPartition(WebsiteFS, "website", "/website");
    setupPartition(ConfigFS, "config", "/config");
}

void setupConfigFile() {
    bool configfile_does_not_exist = !ConfigFS.exists("/config.txt");

    Serial.print("Checking \"config.txt\": ");

    if (configfile_does_not_exist) {
        Serial.print("File does not exist. Creating a new one...");

        File f = ConfigFS.open("/config.txt", "w");
        if (!f) {
            Serial.println("failed");
            return;
        }
        f.println("Config");
        f.close();
        Serial.println("success");
    } else {
        Serial.println("exist");
    }
}

void setupWiFi() {
    Serial.print("Connecting WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(250);
    }
    Serial.println();
}

void setupWebServer() {
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest* req) {
        AsyncWebServerResponse* res = req->beginResponse(ConfigFS, "/config.txt", "text/plain");
        req->send(res);
    });

    server.on("/format_config", HTTP_GET, [](AsyncWebServerRequest* req) {
        Serial.println("Formatting config partition");
        ConfigFS.format();
        req->redirect("/index.html");
    });

    server.serveStatic("/", WebsiteFS, "/").setDefaultFile("/index.html");
    server.begin();
}

void showWelcomeText() {
    String      ipString = WiFi.localIP().toString();
    const char* ip       = ipString.c_str();

    Serial.printf("Open your browser and navigate to:\r\n");
    Serial.printf("http://%s/index.html will be served from \"website\" partition\r\n", ip);
    Serial.printf("http://%s/config shows the content of the file \"config.txt\" from \"config\" partition\r\n", ip);
    Serial.printf("http://%s/format_config formats the \"config\" partition.\r\n", ip);
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(false);

    setupWiFi();

    setupPartitions();
    setupConfigFile();

    setupWebServer();

    showWelcomeText();
}

void loop() {
}