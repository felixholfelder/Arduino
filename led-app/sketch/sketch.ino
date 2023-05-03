#include <WebSocketsServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <WS2812FX.h>
#include <WiFiManager.h>

#define STRIP_PIN 2             // PIN D4 on ESP-8266
#define STRIP_LED_NUMBER 57

DynamicJsonDocument doc(512);
StaticJsonDocument<200> staticDoc;
WS2812FX strip = WS2812FX(STRIP_LED_NUMBER, STRIP_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
WiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  initStrip();
  initWifiManager();
  initWebsocket();
  Serial.println("Setup done!");
}

void loop() {
  webSocket.loop();
  server.handleClient();
  strip.service();
}

void initStrip() {
  strip.init();
  strip.setBrightness(255);
  setStripReady();
  Serial.println("Strip-setup done!");
}

void initWifiManager() {
  wifiManager.autoConnect("AutoConnectAP");
}

void initWebsocket() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  server.begin();
}

void setStripReady() {
  stopStrip();
  strip.setColor(YELLOW);
  strip.start();
}

void setStripConnected() {
  stopStrip();
  strip.setColor(GREEN);
  strip.start();
}

void setStripError() {
  stopStrip();
  strip.setColor(RED);
  strip.start();
}

void stopStrip() {
  strip.stop();
  strip.strip_off();
  strip.setMode(NO_OPTIONS);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t welength) {
  switch(type) {
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connection from ", num);
      Serial.println(ip.toString());
      setStripConnected();
    }
    break;

    case WStype_TEXT: {
      Serial.printf("[%u] Text: %s\n", num, payload);
      webSocket.sendTXT(num, payload);
      String value = (const char *)payload;
      handleRequest(value);
    }
    break;
  }
}

void handleRequest(String value) {
  stopStrip();
  DeserializationError error = deserializeJson(doc, value);
  uint8_t currentMode = doc["mode"];
  uint8_t redColor = doc["red"];
  uint8_t greenColor = doc["green"];
  uint8_t blueColor = doc["blue"];
  if(currentMode != 0) {
    strip.setMode(currentMode);
  }
  if(redColor != 0 || greenColor != 0 || blueColor != 0) {
    strip.setColor(redColor, greenColor, blueColor);
  }
  strip.start();
}
