#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === OLED ===
#define OLED_RESET LED_BUILTIN  // = D4 = Pin2
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
// === /OLED ===

const char* ssid = "Turminator";
const char* password = "lkwpeter,.-123";
MDNSResponder mdns;

ESP8266WebServer server(80);

// static const uint8_t D0   = 16;
// static const uint8_t D1   = 5;
// static const uint8_t D2   = 4;
// static const uint8_t D3   = 0;
// static const uint8_t D4   = 2;   // blue LED on nodeMCU
// static const uint8_t D5   = 14;
// static const uint8_t D6   = 12;
// static const uint8_t D7   = 13;
// static const uint8_t D8   = 15;
// static const uint8_t D9   = 3;
// static const uint8_t D10  = 1;
const int led = D4;

// === handle HTTP ============================================================
void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}
void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
// ============================================================================

void setupOLED() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello from:");
  display.println("Turminator");
  display.display();
}

void setupSerial() {
  Serial.begin(115200);
}

void setupDIO() {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
}

void setupWifi() {
  WiFi.begin(ssid, password);
}

void waitForWifi() {
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMDNS() {
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("mDNS started");
  }
}

void startHTTPD() {
  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

///////////////////////////////////////////////////////////////////////////////

void setup(void){
  setupDIO();
  setupSerial();
  setupWifi();
  setupOLED();
  waitForWifi();

  display.println("IP: ");
  display.println(WiFi.localIP());
  display.display();

  setupMDNS();
  startHTTPD();
}

void loop(void){
  server.handleClient();
}
