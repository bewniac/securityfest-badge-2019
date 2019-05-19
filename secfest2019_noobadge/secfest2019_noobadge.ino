// https://arduino-esp8266.readthedocs.io/en/
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
// https://github.com/bbx10/Adafruit-PCD8544-Nokia-5110-LCD-library/tree/esp8266
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// https://gitlab.com/painlessMesh/painlessMesh
#include <painlessMesh.h>
#include "bitmaps.h"
#include "snake.h"

// Mesh 
painlessMesh  mesh; 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
String groups[50] = ["hexorbase", "waffit", "sulley", "edb", "owtf", "splint", "rlogin-scanner", "canari", "netbios-share-scanner", "rpdscan", "goodork", "btscanner", "tor-browser-en", "htshells", "nishang", "urlcrazy", "extracthosts", "zarp", "shellcodecs", "dnmap", "wireless-ids", "halcyon", "thc-smartbrute", "malwaredetect", "ircsnapshot", "spipscan", "dumpacl", "dirs3arch", "fuzzdb", "fs-nyarl", "padbuster", "tilt", "arduino", "netmap", "blueranger", "speedpwn", "grokevt", "acccheck", "hdmi-sniff", "snmpattack", "googlesub", "httpsniff", "snoopy-ng", "obfsproxy", "artillery", "hotpatch", "easy-creds", "crypthook", "inundator", "ctunnel"];
int groupSize[50];
int groupIndex = 0;
String nodeSecrets[200][3];
/*
*   nodeSecrets[NodeIdIndex][0 = NodeId, 1=Group, 2=Secret]
*   if groupSize[groupIndex] == 4 then groups[groupIndex] is full. groupIndex++
*   secret:GROUPNAME:1-83247671d6207372b47e5039ea2c1103356afe16238cb7e4a9bb3e0b0803858c3aa386
*/

void initChallenge() {
  for (int i=0; i<200; i++) {
    for (int j=0; j<3; j++) {
      nodeSecrets[i][j] = "";
    }
  }
  for (int i=0; i<50; i++) {
    groupSize[i] = 0;
  }
}

// Web server
ESP8266WebServer httpServer(80);

/* 
 * GPIO14 - Serial clock out (SCLK)
 * GPIO13 - Serial data out (DIN)
 * GPIO12 - Data/Command select (D/C)
 * GPIO05 - LCD chip select (CS)
 * GPIO04 - LCD reset (RST)
 */
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);

// Buttons and pins
int OUTPIN = 16; // GPIO16 - Free PIN
int ADC_Button = A0; // ADC - Analog digital converter pin for buttons
int LED = 2; // Onboard LED, LOW turns LED on for some reason.

// Variables 
String macAddr = WiFi.softAPmacAddress().c_str();
String ssid = "SECFEST_19";
String passwd = "SECFEST_19";
int maxRows = 6;
String fs_menu[2] = {"List", "Exit"};
String menu_items[6] = {"Logo", "Schedule" , "Files", "Snake", "Network", "Secret"};
String net_menu[3] = {"Network", "Scan", "Exit"};
String *CurrentMenu = menu_items;
String *networks_menu;
int CurrentMenuSize = (sizeof(menu_items) / sizeof(String));
int CurrentItem = 0;
int bPressed = 0;
String schedule[9] = { "08:30 - Opening", "09:00 - Dude where's my car", "10:00 - WTF Did you say", "11:00 - Foobar", "12:00 - Hurry up and buy", "13:00 - LUNCH", "14:00 - MORE LUNCH", "15:00 - BEER", "Exit" };
String secret = "";

// Used to catch messages on the mesh network.
void receivedCallback( uint32_t from, String &msg ) {
  String tmp = msg.c_str();
  if (tmp.substring(0,7) == "secret:") {
    secret = tmp.substring(7); 
  } else {
    // Implement screen turning black and white
    printLongText(msg.c_str());
  }
}

// Prints out new connections on serial
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
  /* Testing for master badge. Only master badge should send messages. 
   * Send single message per new connection. 
   * Implement to send a new part of the secret each time */
  //String msg = "secret:1-83247671d6207372b47e5039ea2c1103356afe16238cb7e4a9bb3e0b0803858c3aa386";
  //mesh.sendSingle(nodeId, msg);
}


// Web server handles
void handleRoot() {
  httpServer.send(200, "text/plain", "SSSSecret: " + secret);
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
        if (filename == "flag.txt") {
          if (!httpServer.authenticate("admin", "Securityfest2019Sup3rS3cuReP4$$w0rd")) { // REMOVE PASSWORD AFTER GOING PUBLIC BECAUSE SECURITY IS KEY
            return httpServer.requestAuthentication();;
          }
        }
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

  mesh.init(ssid, passwd);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onReceive(&receivedCallback);
  
  // Display setup
  display.begin(60);
  display.setTextSize(0);

  // Web server
  httpServer.on("/", handleRoot);
  httpServer.on("/filesystem", handleFiles);
  httpServer.begin();
  PrintMenu(CurrentMenu);
  
}
// Variables used for timing. Delay() sucks and break shit. 
long lastTimeButton = 0;
long buttonDelay = 150;
bool scrolling = false;
long lastTimeScrolling = 0;
long scrollingDelay = 150;
int scrollPos = 0;
bool printingLongText = false;
String textLeft = "";

#define BUTTON_ADC 400
#define LCD_MAX_CHARS 84

void loop(void) {
  // Mesh update
  mesh.update();

  // Listen for http requests
  httpServer.handleClient();
  
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
      display.setCursor(0, (8*(CurrentItem % 6)));
      display.setTextColor(WHITE, BLACK);
      for (int i=0; i<14; i++) {
        display.print(CurrentMenu[CurrentItem][scrollPos+i]);
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
    textLeft = text.substring(84,text.length());
    Serial.println(textLeft);
    for (int i=0; i<LCD_MAX_CHARS; i++) {
      display.print(text.substring(0,LCD_MAX_CHARS)[i]);
    }
  } else {
    for (int i=0; i<text.length(); i++) {
      display.print(text[i]);
    }
    printingLongText = false;
  }
  display.display();
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
        CurrentItem = 0;
        CurrentMenu = fs_menu;
        CurrentMenuSize = (sizeof(fs_menu) / sizeof(String));
        PrintMenu(CurrentMenu);
        break;
      case 3:
        snake();
        break;
      case 4:
        CurrentItem = 0;
        CurrentMenu = net_menu;
        CurrentMenuSize = (sizeof(net_menu) / sizeof(String));
        PrintMenu(CurrentMenu);
        break;
      case 5:
        display.clearDisplay();
        display.print(secret);
        display.display();
        break;
      default:
        PrintMenu(CurrentMenu);
    }
  } else if (CurrentMenu == fs_menu) {
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
        CurrentMenu = menu_items;
        PrintMenu(CurrentMenu);
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
        result = WiFi.softAP(ssid, passwd);
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
      switch(CurrentItem) {
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
    String text = "Now Johan will open the conference and say hello to everyone. He will tell you a bunch of jokes and all will be fine and dandy. Love you all.";
    switch(CurrentItem) {
      case 0: // Opening
        printLongText(text);        
        break;
      case 1: // Keynote
        printLongText(text);    
        break;
      case 2: // Second talk
        printLongText(text);       
        break;
      case 3: // Third talk
        printLongText(text);
        break;
      case 8: // EXIT
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
  networks_menu = new String[numberOfNetworks+1];
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
  display.println("SSID: \n" + ssid);
  display.println("PSK: \n" + passwd);
  display.println("IP: \n" + IPAddress_toString(WiFi.softAPIP()));
  display.display();
}
String IPAddress_toString(IPAddress ip) {
  String IpAddress = "";
  for (int i=0; i<4; i++) {
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

String getFiles() {
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");
  String fileList = "";
  while (dir.next()) {
    fileList += dir.fileName() + "\n";
  }
  return fileList;
}
