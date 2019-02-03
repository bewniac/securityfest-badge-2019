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

int maxRows = 6;
String fs_menu[2] = {"List", "Exit"};
String menu_items[8] = {"Logo", "Schedule" , "Files", "Snake", "Network", "Something", "Something", "Darkside"};
String net_menu[3] = {"Network conf.", "Scan", "Exit"};
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
  httpServer.send(200, "text/html", schedule[0]);
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

String ssid = "SECFEST_19";
String passwd = "SECFEST_19";

void setup(void) {
  // Setting up buttons
  pinMode(OUTPIN, OUTPUT);
  pinMode(ADC_Button, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); // Turn LED off.

  // Serial for debugging
  Serial.begin(115200);

  /* WiFi Soft Accesspoint setup*/
  boolean result = WiFi.softAP(ssid, passwd);
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
  //animateLogo();
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
      delay(60);
      Action(currentMenu);
    }
    if (400 < bPressed) { // Move button
      //Serial.println(bPressed);
      CurrentItem = ((CurrentItem + 1) % CurrentMenuSize);
      delay(60);
      PrintMenu(currentMenu);
    }
  }
}
void printLongText(String ltext) {
  int maxChars = 84;
  int pages = ltext.length()/maxChars;
  int currentPage = 0;
  display.clearDisplay();
  for (int i=0; i<maxChars+1; i++) {
    display.print(ltext[i]);
  }
  display.display();
  while (currentPage <= pages) {
    delay(50);
    bPressed = analogRead(ADC_Button);
    if (600 < bPressed) {
      // Move button is pressed, change page
      delay(60);
      display.clearDisplay();
      currentPage++;
      int startChar = currentPage*maxChars;
      if (ltext.length() < (startChar+84)) {
        for (int i=startChar; i<ltext.length(); i++) {
          if(ltext[startChar] == ' ') {
            i = i+1;
          }
          display.print(ltext[i]);
        } 
      }
      else {
        for (int i=startChar; i<(startChar+84); i++) {
          display.print(ltext[i]);
        }
      }
      display.display();
    }
    
  }
}
void printSchedule(void) {
  //printLongText(text);
}
void PrintMenu(String *menu) {
  display.clearDisplay();
  int firstItem = CurrentItem - (CurrentItem%6);
  
  for (int i = firstItem; i < CurrentMenuSize; i++) {
    if (CurrentItem == i) {
      display.setTextColor(WHITE, BLACK);
    } else {
      display.setTextColor(BLACK, WHITE);
    }
    if (menu[i].length() < 14) {
      display.println(menu[i]);
    }
    else {
      for (int c=0; c<14; c++) {
        display.print(menu[i][c]);
      }
    }
  }
  if (currentMenu == menu_items) {
    display.drawBitmap(40, 0, logo, 48, 48, 1);
  }
  display.display();
  if (14 < menu[CurrentItem].length()) {
    int startChar = 0;
    display.setTextColor(WHITE, BLACK);
    while(14 < (menu[CurrentItem].length()-startChar)) {
      display.setCursor(0, (8*(CurrentItem % 6)));
      startChar++;
      for (int c=startChar; c<(startChar+14); c++) {
        
        display.print(menu[CurrentItem][c]);
      }
      bPressed = analogRead(ADC_Button);
      delay(150);
      if (100 < bPressed) {
        display.display();
        display.setTextColor(BLACK, WHITE);
        return;
      }
      display.display();
    }   
  }
  display.display();
  display.setTextColor(BLACK, WHITE);
}

void Action(String *menu) {
  if (currentMenu == menu_items) {
    switch (CurrentItem) {
      case 0:
        printLogo();
        break;
      case 1:
        CurrentItem = 0;
        currentMenu = schedule;
        CurrentMenuSize = (sizeof(schedule) / sizeof(String));
        PrintMenu(currentMenu);
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
        CurrentItem = 0;
        currentMenu = net_menu;
        CurrentMenuSize = (sizeof(net_menu) / sizeof(String));
        PrintMenu(currentMenu);
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
  } else if (currentMenu == net_menu) {
    boolean result;
    switch (CurrentItem) {
      case 0:
        printNetwork();
        break;
      case 1:
        display.clearDisplay();
        display.println("Wait for it...");
        display.display();
        ScanNetwork();
        /* When scanning is complete create a access point once more */
        WiFi.mode(WIFI_AP);
        result = WiFi.softAP(ssid, passwd);
        if (result) {
          Serial.println("Success setting up AP");
        }
        else {
          Serial.println("Failed setting up AP");
        }
        break;
      case 2:
        CurrentItem = 0;
        CurrentMenuSize = (sizeof(menu_items) / sizeof(String));
        currentMenu = menu_items;
        PrintMenu(currentMenu);
        break;
      default:
        PrintMenu(currentMenu);
    }
  } else if (currentMenu == schedule) {
    switch(CurrentItem) {
      case 0: // Opening
        printLongText("Now Johan will open the conference and say hello to everyone. He will tell you a bunch of jokes and all will be fine and dandy. Love you all.");
        break;
      case 1: // Keynote
        printLongText("Now Johan will open the conference and say hello to everyone. He will tell you a bunch of jokes and all will be fine and dandy. Love you all.");
        break;
      case 2: // Second talk
        printLongText("Now Johan will open the conference and say hello to everyone. He will tell you a bunch of jokes and all will be fine and dandy. Love you all.");
        break;
      case 3: // Third talk
        printLongText("Now Johan will open the conference and say hello to everyone. He will tell you a bunch of jokes and all will be fine and dandy. Love you all.");
        break;
      case 8: // EXIT
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
void ScanNetwork() {
  display.clearDisplay();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    display.println(WiFi.SSID(i));
  }
  display.display();
  
}
void printNetwork(void) {
  display.clearDisplay();
  display.println("SSID: " + ssid);
  display.println("PSK: " + passwd);
  display.println("IP: " + String(WiFi.softAPIP()));
  display.display();
}

void snake(void) {
  Game g(ADC_Button);
  g.Run(display);
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
