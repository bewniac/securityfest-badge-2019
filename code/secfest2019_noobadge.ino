#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "bitmaps.h"
#include "pages.h"
#include "snake.h"
#include <FS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>

// GPIO14 - Serial clock out (SCLK)
// GPIO13 - Serial data out (DIN)
// GPIO12 - Data/Command select (D/C)
// GPIO05 - LCD chip select (CS)
// GPIO04 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);

int OUTPIN = 16; // GPIO16 - Free PIN
int ADC_Button = A0; // ADC - Analog digital converter pin for buttons
int LED = 2; // Onboard LED, LOW turns LED on for some reason.

String fs_menu[2] = {"1. List", "2. Exit"};
String menu_items[5] = {"1. Logo", "2. Schedule" , "3. Files", "4. Snake", "5. Network"};
String *currentMenu = menu_items;
int CurrentMenuSize = (sizeof(menu_items) / sizeof(String));
int CurrentItem = 0;
int bPressed = 0;

// Web server and updater
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void handleRoot() {
  httpServer.send(200, "text/plain", startpage);
}
void handleSchedule() {
  httpServer.send(200, "text/html", schedule);
}
void handleFiles() {
  if (httpServer.args() > 0) {
    if (httpServer.argName(0) == "file") {
      if (httpServer.arg("file") == "") {
        httpServer.send(200, "text/plain", "Missing file.");
      }
      else {
        SPIFFS.begin();
        String filename = httpServer.arg("file");
        filename.trim();
        /*if (filename == "flag.txt") {
          httpServer.send(200, "text/plain", "Not really so simple.");
        }*/
        File file = SPIFFS.open("/" + filename, "r");
        if (!file) {
          httpServer.send(200, "text/plain", "Error reading file.");
        }
        else {
          if (httpServer.arg("type") == "jpg") {
            httpServer.streamFile(file, "image/jpeg");
            httpServer.send(200, "text/plain", "Here you go.");
          }
          if (httpServer.arg("type") == "txt") {
            String fileContent = "";
            for (int i = 0; i < file.size(); i++) {
              fileContent += (char)file.read();
            }
            httpServer.send(200, "text/plain", fileContent);
          }
          else {
            httpServer.send(200, "text/plain", "Missing type");
          }
        }
      }
    }
    else {
      httpServer.send(200, "text/plain", "Unknown parameter.");
    }
  } else {
    httpServer.send(200, "text/plain", "Directory / \n" + getFiles());
  }
}

void setup(void) {
  // Setting up buttons
  pinMode(OUTPIN, OUTPUT);
  pinMode(ADC_Button, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); // Turn LED off.

  // Serial for debugging
  Serial.begin(115200);

  /* WiFi Soft Accesspoint setup*/
  boolean result = WiFi.softAP("SECFEST_19", "hejsansvejsan");
  if (result) {
    Serial.println("Success setting up AP");
  }
  else {
    Serial.println("Failed setting up AP");
  }
  // Display setup
  display.begin(100);
  display.setTextSize(0);

  httpUpdater.setup(&httpServer);
  httpServer.on("/", handleRoot);
  httpServer.on("/schedule", handleSchedule);
  httpServer.on("/filesystem", handleFiles);
  httpServer.begin();
  PrintMenu(currentMenu);
}

void loop(void) {
  // Need delay keep wifi running while reading ADC button. Somehow they dont work at the same time.
  delay(100);
  //Webserver client handle
  httpServer.handleClient();

  bPressed = analogRead(ADC_Button);
  if (300 < bPressed) { // A button is pressed
    if ( bPressed < 400 ) {  // Action button
      //Serial.println(bPressed);
      delay(300);
      Action(currentMenu);
    }
    if (400 < bPressed) { // Move button
      //Serial.println(bPressed);
      CurrentItem = ((CurrentItem + 1) % CurrentMenuSize);
      delay(300);
      PrintMenu(currentMenu);
    }
  }
}

void PrintMenu(String *menu) {
  display.clearDisplay();
  for (int i = 0; i < CurrentMenuSize; i++) {
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
        currentMenu = fs_menu;
        CurrentMenuSize = (sizeof(fs_menu) / sizeof(String));
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
  } else if (currentMenu == fs_menu) {
    File flag, f;
    Dir dir;
    String userInput;
    switch (CurrentItem) {
      case 0:
        SPIFFS.begin();
        // Present list of files on FS
        display.clearDisplay();
        dir = SPIFFS.openDir("/");
        display.println("Filename");
        while (dir.next()) {
          display.print(dir.fileName() + "\n");
        }
        display.display();
        if (Serial.available()) {
          while (Serial.available() > 0) {
            userInput += char(Serial.read());

          }
          userInput.trim();
          flag = SPIFFS.open(userInput, "r");
          if (!flag) {
            Serial.println("ERROR: Can't open file");
          }
          Serial.println("----- CONTENT -----");
          for (int i = 0; i < flag.size(); i++) {
            Serial.print((char)flag.read());
          }
          Serial.println("\n----- CONTENT -----");
        }
        break;
      case 1:
        CurrentItem = 0;
        CurrentMenuSize = (sizeof(menu_items) / sizeof(String));
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
  display.println(WiFi.softAPIP());
  display.display();
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
  for (int i = -48; i < 85; i++) {
    display.clearDisplay();
    display.drawBitmap(i, 0, logo, 48, 48, 1);
    display.display();
    delay(50);
  }
}

String getFiles() {
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");
  String fileList = "";
  while (dir.next()) {
    fileList += dir.fileName() + "\n";
  }
  return fileList;
}
