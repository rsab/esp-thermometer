#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>
#include "index_html.h"
#include "config_html.h"

#define ONE_WIRE_BUS D4
#define CONFIG_FILE  "/config.json"
#define PARAM_LEN    220

const unsigned long PUSH_INTERVAL = 30000;

char otlpUrl[PARAM_LEN]  = "";
char otlpAuth[PARAM_LEN] = "";
bool shouldSaveConfig     = false;
bool shouldRestart        = false;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
ESP8266WebServer server(80);
unsigned long lastPush = 0;


void loadConfig() {
  if (!LittleFS.exists(CONFIG_FILE)) return;
  File f = LittleFS.open(CONFIG_FILE, "r");
  if (!f) return;
  JsonDocument doc;
  if (deserializeJson(doc, f) == DeserializationError::Ok) {
    strlcpy(otlpUrl,  doc["otlp_url"]  | "", PARAM_LEN);
    strlcpy(otlpAuth, doc["otlp_auth"] | "", PARAM_LEN);
  }
  f.close();
}

void saveConfig(const char* url, const char* auth) {
  File f = LittleFS.open(CONFIG_FILE, "w");
  if (!f) {
    Serial.println("Config save failed: could not open file");
    return;
  }
  JsonDocument doc;
  doc["otlp_url"]  = url;
  doc["otlp_auth"] = auth;
  serializeJson(doc, f);
  f.close();
  Serial.println("Config saved");
}

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    server.send(200, "application/json", "{\"error\":\"sensor disconnected\"}");
    return;
  }

  String json = "{\"temperature\":" + String(tempC, 2) + "}";
  server.send(200, "application/json", json);
}

void handleConfig() {
  if (server.method() == HTTP_POST) {
    String url  = server.arg("otlp_url");
    String auth = server.arg("otlp_auth");
    url.toCharArray(otlpUrl, PARAM_LEN);
    if (auth.length() > 0) auth.toCharArray(otlpAuth, PARAM_LEN);
    saveConfig(otlpUrl, otlpAuth);
    server.sendHeader("Location", "/config");
    server.send(303);
    return;
  }

  String page = FPSTR(CONFIG_HTML);
  page.replace("%URL%", String(otlpUrl));
  server.send(200, "text/html", page);
}

void handleResetConfig() {
  LittleFS.remove(CONFIG_FILE);
  WiFiManager wm;
  wm.resetSettings();
  server.send(200, "text/plain", "Config cleared. Restarting into setup portal...");
  shouldRestart = true;
}

void pushOTLP() {
  if (strlen(otlpUrl) == 0 || strlen(otlpAuth) == 0) return;

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC == DEVICE_DISCONNECTED_C) return;

  time_t now = time(nullptr);
  String tsNano = String((unsigned long)now) + "000000000";

  String body =
    "{\"resourceMetrics\":[{\"resource\":{\"attributes\":[{\"key\":\"service.name\",\"value\":{\"stringValue\":\"thermometer\"}}]},"
    "\"scopeMetrics\":[{\"metrics\":[{\"name\":\"temperature_celsius\",\"gauge\":{\"dataPoints\":[{"
    "\"asDouble\":" + String(tempC, 2) + ","
    "\"timeUnixNano\":\"" + tsNano + "\""
    "}]}}]}]}]}";

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, otlpUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", otlpAuth);

  int code = http.POST(body);
  Serial.println("OTLP push: HTTP " + String(code));
  http.end();
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  LittleFS.begin();
  loadConfig();

  WiFiManagerParameter paramUrl(
    "otlp_url", "OTLP Endpoint URL", otlpUrl, PARAM_LEN);
  WiFiManagerParameter paramAuth(
    "otlp_auth", "OTLP Authorization (Basic ...)", otlpAuth, PARAM_LEN);

  WiFiManager wm;
  wm.setSaveConfigCallback([]() { shouldSaveConfig = true; });
  wm.addParameter(&paramUrl);
  wm.addParameter(&paramAuth);
  wm.setConfigPortalTimeout(180);

  if (!wm.autoConnect("Thermometer-Setup")) {
    ESP.restart();
  }

  if (shouldSaveConfig) {
    strlcpy(otlpUrl,  paramUrl.getValue(),  PARAM_LEN);
    strlcpy(otlpAuth, paramAuth.getValue(), PARAM_LEN);
    saveConfig(otlpUrl, otlpAuth);
  }

  configTime(0, 0, "pool.ntp.org");
  Serial.print("Syncing NTP");
  while (time(nullptr) < 1000000000UL) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" done");

  server.on("/", handleRoot);
  server.on("/temperature", handleTemperature);
  server.on("/config", handleConfig);
  server.on("/reset", handleResetConfig);
  server.begin();

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("HTTP server started");
}

void loop() {
  if (shouldRestart) ESP.restart();

  server.handleClient();

  if (millis() - lastPush >= PUSH_INTERVAL) {
    lastPush = millis();
    pushOTLP();
  }
}
