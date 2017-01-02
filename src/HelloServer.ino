#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mcp_can.h>

// === OLED ===
#define OLED_RESET LED_BUILTIN  // = D4 = Pin2
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
// === /OLED ===

// === MCP2515 ===
MCP_CAN CAN(D3);    // Set CS to pin15 (=D8)
// === /MCP2515 ===

// === Networking / WIFI ===
const char* ssid = "Turminator";
const char* password = "lkwpeter,.-123";
MDNSResponder mdns;
ESP8266WebServer server(80);
// === /Networking / WIF ===

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

void initCAN() {
    Serial.print("Initializing CAN ...");

    // CAN Bus on BMW K51 has 500kbps
    while(CAN_OK != CAN.begin(CAN_500KBPS)) {
        Serial.print(".");
        delay(100);
    }

    // Mode
    //CAN.mcp2515_modifyRegister(MCP_CANCTRL,MODE_MASK,MODE_LOOPBACK); // MODE_NORMAL MODE_LOOPBACK

    //Set filter masks
    CAN.init_Mask(0, 0, 0xfff);
    CAN.init_Mask(1, 0, 0xfff);

    // Set filters
    CAN.init_Filt(0, 0, 0x2D0); // ZFE
    CAN.init_Filt(0, 0, 0x2A0);

    // Print in CSV format
    Serial.println("time,CAN-ID,b0,b1,b2,b3,b4,b5,b6,b7");
    Serial.println();

    Serial.println("CAN initialized.");
}

void printCAN() {
    unsigned char length = 0;
    unsigned char data[8];

    // Print in CSV format
    if(CAN_MSGAVAIL == CAN.checkReceive()){
        CAN.readMsgBuf(&length, data);
        Serial.print(millis());
        Serial.print(",");
        Serial.print(CAN.getCanId(), HEX);
        for(int i = 0; i<length; i++) {
            Serial.print(",");
            if( data[i] < 0x10) {
                Serial.print("0");
            }
            Serial.print(data[i], HEX);
        }
        Serial.println("");
    }
}

///////////////////////////////////////////////////////////////////////////////

void setup(void) {
    setupDIO();
    setupSerial();
    setupWifi();
    setupOLED();

    display.println("initCAN...");
    display.display();
    initCAN();
    display.println("DONE!");
    display.display();

    waitForWifi();
    // display.println("IP: ");
    // display.println(WiFi.localIP());
    // display.display();

    setupMDNS();
    startHTTPD();
}

unsigned long attempt = 0;
void loop(void) {
    // HTTP
    server.handleClient();

    attempt = attempt + 1;
    if (attempt % 32 == 0) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("run #");
        display.println(attempt);
        display.display();
    }

    // CAN
    printCAN();
}
