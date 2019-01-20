#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "bitmaps.h"
#include "pages.h"
#include "snake.h"

// Software SPI (slower updates, more flexible pin options):
// GPIO14 - Serial clock out (SCLK)
// GPIO13 - Serial data out (DIN)
// GPIO12 - Data/Command select (D/C)
// GPIO05 - LCD chip select (CS)
// GPIO04 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain; charset=utf-8", startpage);
  
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

int OUTPIN = 16; // GPIO16 - Free PIN
int ADC_Button = A0; // ADC - Analog digital converter pin for buttons

String led_items[3] = {"1. ON", "2. OFF", "3. EXIT"};
String menu_items[5] = {"1. Logo", "2. Schedule" , "3. LED", "4. Snake", "5. Network"};
String *currentMenu = menu_items;
int CurrentMenuSize = (sizeof(menu_items)/sizeof(String));
int CurrentItem = 0;

int ConnClients = 0;
String mac = WiFi.softAPmacAddress().c_str();

const char* ssid = "Securityfest_badge";
const char* password = "esp_86F3EB";

void setup(void) {    
  pinMode(OUTPIN, OUTPUT);
  pinMode(ADC_Button, INPUT);
  
  Serial.begin(115200);

  IPAddress local_IP(10,11,12,13);
  IPAddress gateway(10,11,12,13);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(mac);

  display.begin(60);
  
  server.on("/", handleRoot);
  server.on("/schedule", []() {
    server.send(200, "text/html; charset=utf-8", schedule);
  });
  // POST TO DEVICE WHICH IS THE NEXT TALK
  server.on("/api/nexttalk", []() {
     if (server.method() == HTTP_POST) {
        String data = "";
        for (int i=0; i<server.args(); i++) {
          if (server.argName(i) == "talk") {
            data+=server.arg(i);
          }
        }
        if (data == "") {
          server.send(418, "text/html; charset=utf-8", "I'm a teapot!");
        }
        display.clearDisplay();
        display.print(data);
        display.display();
        
     } else {
        server.send(418, "text/html; charset=utf-8", "I'm a teapot!");
     }
  });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  
  PrintMenu(currentMenu);
}

void loop(void) {
  server.handleClient();
  int bPressed = analogRead(ADC_Button);
  if (300 < bPressed) { // A button
    if ( bPressed < 400 ) {  // Action button
      Serial.println(bPressed);
      delay(300);
      Action(currentMenu);
    }
    if (600 < bPressed) { // Move button 
      Serial.println(bPressed);
      CurrentItem = ((CurrentItem+1) % CurrentMenuSize);
      delay(300);
      PrintMenu(currentMenu);
    }
  } 
}

void PrintMenu(String *menu) {
  display.clearDisplay();
  for (int i=0; i<CurrentMenuSize; i++) {
    if (CurrentItem == i) {
      display.print("* ");
    } else {
      display.print("  ");  
    }
    display.println(menu[i]);
  }
  display.display();
}

void Action(String *menu) {
  if (currentMenu == menu_items) {
      switch (CurrentItem) {
        case 0:
          printLogo();
          break;
        case 1:
          printSchedule();
          break;
        case 2:
          CurrentItem = 0;
          currentMenu = led_items;
          CurrentMenuSize = (sizeof(led_items)/sizeof(String));
          PrintMenu(currentMenu);
          break;
        case 3:
          snake();
          break;
        case 4:
          printNetwork();
          break;
        default:
          PrintMenu(currentMenu);
      }
  } else if (currentMenu == led_items) {
      switch (CurrentItem) {
        case 0: // ON
          digitalWrite(OUTPIN, HIGH);
          break;
        case 1: // OFF
          digitalWrite(OUTPIN, LOW);
          break;
        case 2:
          CurrentItem = 0;
          CurrentMenuSize = (sizeof(menu_items)/sizeof(String));
          currentMenu = menu_items;
          PrintMenu(currentMenu);
          break;
        default:
          PrintMenu(currentMenu); 
      }
  }
}

void printNetwork(void) {
  display.clearDisplay();
  display.println("SSID: " + String(ssid));
  display.println("PSK: " + String(password));
  display.println(WiFi.softAPIP());
}

void snake(void) {
  Game g(ADC_Button);
  g.Run(display);
}
void printSchedule(void) {
  display.clearDisplay();
  display.println("TBD");
  display.display();
}

void printLogo(void) {
  display.clearDisplay();
  display.drawBitmap(18, 0, logo, 48, 48, 1);
  display.display();
}

void animateLogo(void) {
  for (int i=-48; i<85; i++) {
    display.clearDisplay();
    display.drawBitmap(i, 0, logo, 48, 48, 1);
    display.display();
    delay(50);
  }
}
