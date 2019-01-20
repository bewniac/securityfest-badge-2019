# Security fest 2019 - Official badge
Here you go, all the code and files that exist as a part of the official Security fest badge "N00Badge". First of all you should know that I'm a complete newbie and this is my first ever project designing a PCB from scratch. I've learned a lot and it was a lot of fun with more headaches. I recommend everyone wanting to get into electronics to do exactly this, get an idea what you want to do and do it!

The schematics and PCB design are all made in [KiCAD](http://kicad-pcb.org/) which is an Open Source Electrionics design suite and there is a lot of footprints and schematic symbols for a lot of components out there. You could choose to go with Eagle, I haven't tried it but heard a lot of nice things about it. I choose KiCAD because it was the first google result.

We wanted to do something cool for Security fest 2019 and we wanted to do it ourselves. So I got my hands on my first ESP8266 board, the ESP-12F. It's a WiFi IoT thingy and can both work as an Access Point or a client and you can get it for 3 USD, or about 50 SEK if you buy it in Sweden. However I felt like that wasn't enough. I wanted a screen! So I bought a Nokia 5110 LCD screen which costs about the same as the ESP-12F. So those two were my main components at the beginning of the development stage. I made a list of what I felt was a requirement.

  * Display
  * ESP8266
  * Powered by micro USB. 

Simple enough. I built a prototype that could be powered by 5V micro USB. Although the components require 3.3V, but with a voltage regulator (LD1117V33) this wasn't an issue. The result was this:

![First Prototype](https://github.com/bewniac/securityfest-badge-2019/blob/master/First_prototype.jpg)

Not so pretty, I know. But it worked. But then I wanted to create my own PCB prototype, mainly with components I could solder by hand as I'm never done surface mount soldering myself. So I created my first schematic and PCB design in KiCAD. 

![Schematic prototype](https://github.com/bewniac/securityfest-badge-2019/blob/master/Prototype1_schematic.png)
![PCB Prototype](https://github.com/bewniac/securityfest-badge-2019/blob/master/PCB_Prototype1.png)
