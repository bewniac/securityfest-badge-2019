// https://arduino-esp8266.readthedocs.io/en/
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <FS.h>
// https://github.com/bbx10/Adafruit-PCD8544-Nokia-5110-LCD-library/tree/esp8266
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// https://gitlab.com/painlessMesh/painlessMesh
#include <painlessMesh.h>
#include "bitmaps.h"
#include "snake.h"

#define   MESH_PREFIX     "SECFEST_19"
#define   MESH_PASSWORD   "SECFEST_19_PSK"
#define   MESH_PORT       5555

#define BUTTON_ADC 400
#define LCD_MAX_CHARS 84

// Mesh
painlessMesh  mesh;
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);

/*
   GPIO14 - Serial clock out (SCLK)
   GPIO13 - Serial data out (DIN)
   GPIO12 - Data/Command select (D/C)
   GPIO05 - LCD chip select (CS)
   GPIO04 - LCD reset (RST)
*/
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);

// Variables

// Variables used for timing. Delay() sucks and break shit.
long lastTimeButton = 0;
long buttonDelay = 150;
long secretDelay = 10000;
long lastTimeSecret = 0;
bool scrolling = false;
long lastTimeScrolling = 0;
long scrollingDelay = 150;
int scrollPos = 0;
bool printingLongText = false;
String textLeft = "";
String macAddr = WiFi.softAPmacAddress().c_str();
int maxRows = 6;
String menu_items[5] = {"Logo", "Schedule", "Snake", "Network", "Secret"};
String net_menu[3] = {"Network", "Scan", "Exit"};
String *CurrentMenu = menu_items;
String *networks_menu;
int CurrentMenuSize = (sizeof(menu_items) / sizeof(String));
int CurrentItem = 0;
int bPressed = 0;
String schedule[26] = {"-- MAY 23 --", "08:30 Registration and breakfast", "09:00 Hello and welcome", "09:20 Shira Shamban", "10:20 Christoffer Jerkeby", "11:20 Olle Segerdahl", "12:30 Lunch", "13:30 Hugo Hirsh", "14:30 Csaba Fitzl", "15:20 Coffee break", "15:45 Alex Inführ", "16:45 Samit Anwer", "17:45 Calle Svensson", "19:45 Dinner, drinks and awards", "21:45 After party!", "-- MAY 24 --", "08:30 Breakfast", "09:00 Dave Lewis", "10:15 David Fiser", "11:30 Dennis Kolegov", "12:30 Lunch", "13:30 Hanno Böck", "14:45 Himanshu Mehta", "15:45 Mazin Ahmed", "16:45 Goodbye <3", "Exit."};
String secret = "UNSET";
uint32_t masterNode = 3157003985;

// Used to catch messages on the mesh network.
void receivedCallback( uint32_t from, String &msg ) {
  String recv = msg;
  if (recv.startsWith("secret:")) {
    secret = recv;
    printLongText(secret);
  } else {
    printLongText(msg);
  }
}

int ADC_Button = A0; // ADC - Analog digital converter pin for buttons

void setup(void) {
  pinMode(ADC_Button, INPUT);

  // Serial for debugging
  Serial.begin(115200);
  Serial.println();
  Serial.println("N00badge v0.1");
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.setDebugMsgTypes( ERROR | COMMUNICATION );
  mesh.onReceive(&receivedCallback);
  mesh.setContainsRoot();

  // Display setup
  display.begin(60);
  display.setTextSize(0);

  PrintMenu(CurrentMenu);
}

void loop(void) {
  // Mesh update
  mesh.update();

  long timeSecret = millis();
  if ((timeSecret - lastTimeSecret) >= secretDelay) {
    lastTimeSecret = timeSecret;
    if (secret == "UNSET") {
      String msg = "giveme";
      mesh.sendSingle(masterNode, msg);
    }
  }

  long timeButton = millis();
  if ((timeButton - lastTimeButton) >= buttonDelay) {
    lastTimeButton = timeButton;
    bPressed = analogRead(ADC_Button);
    buttonPress(bPressed);
  }

  // Scrolling text
  long timeScrolling = millis();
  if ((timeScrolling - lastTimeScrolling) >= scrollingDelay) {
    if (scrolling) {
      display.setCursor(0, (8 * (CurrentItem % 6)));
      display.setTextColor(WHITE, BLACK);
      for (int i = 0; i < 14; i++) {
        display.print(CurrentMenu[CurrentItem][scrollPos + i]);
      }
      scrollPos++;
      lastTimeScrolling = timeScrolling;
      if (CurrentMenu[CurrentItem].length() == scrollPos) {
        scrolling = false;
        scrollPos = 0;
      }
      display.display();
    }
  }

}
void buttonPress(int button) {
  if (100 < button) { // A button is pressed
    // If button is pressed then we should restore scrollPos and stop scrolling.
    scrollPos = 0;
    scrolling = false;
    if ( button < BUTTON_ADC ) {  // Action button
      if (printingLongText) {
        printLongText(textLeft);
      } else {
        Action(CurrentMenu);
      }
    }
    if (BUTTON_ADC < button) { // Move button
      CurrentItem = ((CurrentItem + 1) % CurrentMenuSize);
      PrintMenu(CurrentMenu);
    }
  }
}

void printLongText(String text) {
  printingLongText = true;
  display.setTextColor(BLACK, WHITE);
  display.clearDisplay();
  if (text.length() > LCD_MAX_CHARS) {
    textLeft = text.substring(84, text.length());
    String line = text.substring(0, LCD_MAX_CHARS);
    line.trim();
    display.print(line);

  } else {
    for (int i = 0; i < text.length(); i++) {
      display.print(text[i]);
    }
    printingLongText = false;
  }
  display.display();
}

void PrintMenu(String *menu) {
  display.clearDisplay();
  int firstItem = CurrentItem - (CurrentItem % 6);
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
      for (int c = 0; c < 14; c++) {
        display.print(menu[i][c]);
      }
    }
  }
  if (CurrentMenu == menu_items) {
    display.drawBitmap(40, 0, logo, 48, 48, 1);
  }
  display.display();
  if (13 < menu[CurrentItem].length()) {
    scrolling = true;
  }
  display.display();
  display.setTextColor(BLACK, WHITE);
}

void Action(String *menu) {
  if (CurrentMenu == menu_items) {
    switch (CurrentItem) {
      case 0:
        printLogo();
        break;
      case 1:
        CurrentItem = 0;
        CurrentMenu = schedule;
        CurrentMenuSize = (sizeof(schedule) / sizeof(String));
        PrintMenu(CurrentMenu);
        break;
      case 2:
        snake();
        break;
      case 3:
        CurrentItem = 0;
        CurrentMenu = net_menu;
        CurrentMenuSize = (sizeof(net_menu) / sizeof(String));
        PrintMenu(CurrentMenu);
        break;
      case 4:
        printLongText(secret);
        break;
      default:
        PrintMenu(CurrentMenu);
    }
  } else if (CurrentMenu == net_menu) {
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
        result = WiFi.softAP(MESH_PREFIX, MESH_PASSWORD);
        if (result) {
          Serial.println("Success setting up AP");
        }
        else {
          Serial.println("Failed setting up AP");
        }
        CurrentItem = 0;
        CurrentMenuSize = sizeof(networks_menu);
        CurrentMenu = networks_menu;
        PrintMenu(CurrentMenu);
        break;
      case 2:
        CurrentItem = 0;
        CurrentMenuSize = (sizeof(menu_items) / sizeof(String));
        CurrentMenu = menu_items;
        PrintMenu(CurrentMenu);
        break;
      default:
        PrintMenu(CurrentMenu);
    }
  } else if (CurrentMenu == networks_menu) {
    switch (CurrentItem) {
      case 3:
        CurrentItem = 0;
        CurrentMenuSize = (sizeof(net_menu) / sizeof(String));
        CurrentMenu = net_menu;
        PrintMenu(CurrentMenu);
        break;
      default:
        PrintMenu(CurrentMenu);
    }
  } else if (CurrentMenu == schedule) {
    switch (CurrentItem) {
      case 25: // EXIT
        CurrentItem = 0;
        CurrentMenuSize = (sizeof(menu_items) / sizeof(String));
        CurrentMenu = menu_items;
        PrintMenu(CurrentMenu);
        break;
      default:
        PrintMenu(CurrentMenu);
    }
  }
}


void ScanNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int numberOfNetworks = WiFi.scanNetworks();
  String encType = "";
  display.clearDisplay();
  display.display();
  networks_menu = new String[numberOfNetworks + 1];
  for (int i = 0; i < numberOfNetworks; i++)
  {
    int et = WiFi.encryptionType(i);
    if (et ==  ENC_TYPE_WEP) {
      encType = "WEP";
    } else if (et == ENC_TYPE_TKIP) {
      encType = "WPA";
    } else if (et == ENC_TYPE_CCMP) {
      encType = "WPA2";
    } else if (et == ENC_TYPE_NONE) {
      encType = "Open";
    }
    networks_menu[i] = String(i + 1) + ": " + WiFi.SSID(i).c_str() + " Ch:" + WiFi.channel(i) + "(" + WiFi.RSSI(i) + "dBm) " + encType;
  }
  networks_menu[numberOfNetworks] = "Exit";
}
void printNetwork(void) {
  display.clearDisplay();
  display.println(MESH_PREFIX);
  display.println(MESH_PASSWORD);
  display.println(IPAddress_toString(WiFi.softAPIP()));
  display.display();
}
String IPAddress_toString(IPAddress ip) {
  String IpAddress = "";
  for (int i = 0; i < 4; i++) {
    IpAddress += String(ip[i]);
    if (i < 3) {
      IpAddress += ".";
    }
  }
  return IpAddress;
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
