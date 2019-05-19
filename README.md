# Security fest 2019 - n00Badge
Here you go, all the code and files that exist as a part of the official Security fest 2019 badge we call "n00Badge". First of all, you should know that I'm a complete n00b (hence the name) and this is my first ever project designing a PCB from scratch. I've learned a lot and it was a lot of fun with more headaches than I could count. I recommend everyone wanting to get into electronics to do what I did, get an idea what you want to do and then, like Nike say, Just do it!

The schematics and PCB design are all made in [KiCAD](http://kicad-pcb.org/) which is an Open Source Electrionics design suite. There's a lot of PCB footprints and schematic symbols for KiCAD and it's easy to make your own. There's other suites available but I use KiCAD because it was my first result on Google. 

I wanted to do something cool for Security fest 2019 and I wanted to do it myself from scratch. So I got my hands on my first ESP8266 board, the ESP-12F. It's a WiFi IoT thingy and can both work as an Access Point and a WiFi client and you can get it for 3 USD, or about 50 SEK if you buy it in Sweden. Now I have a WiFi capable microcontroller. But I wanted more. So I bought a Nokia 5110 LCD screen which costs about the same as the ESP-12F. This became my head requirements for the project:

  * Nokia 5110 LCD
  * ESP8266
  * Powered by micro USB

Simple enough. I built a prototype that could be powered by 5V micro USB. Although the components require 3.3V, but with a voltage regulator (LD1117V33) this wasn't an issue. The result was this:

![First Prototype](https://github.com/bewniac/securityfest-badge-2019/blob/master/images/First_prototype.jpg)

Not so pretty, I know. But it worked. But then I wanted to create my own PCB prototype, with as much hand-solderable components as possible as I've never done SMD soldering before. I created my first schematic and PCB design in KiCAD. 

![Schematic prototype](https://github.com/bewniac/securityfest-badge-2019/blob/master/images/Prototype1_schematic.png)
![PCB Prototype](https://github.com/bewniac/securityfest-badge-2019/blob/master/images/PCB_Prototype1.png)

Still not pretty, but much better than my hand-made prototype and very fun to do! I ordered 10 PCBs from https://jlcpcb.com/ and the components I was missing from https://lcsc.com/ and waited. When it arrived I was extatic! My first PCB! I soldered all the components as soon as I got it (although I had to fix some errors with the backlight LEDs at first, I forget a GND route to the backlight LEDs). 

![PCB](https://github.com/bewniac/securityfest-badge-2019/blob/master/images/Prototype_PCB2.jpg)

When I did some programming trying to come up with some fun stuff to add I found I was missing something, buttons for user interaction. So I had to modify the badge a bit. I used the ADC (Analog to Digital Converter) pin to add two buttons and controll them with one pin. This can be done by adding resistors of different value to the GND leg of each button. The analog voltage is converted by the pin to a digital number. The resistors will reduce the voltage to different values on each button, hence the digital number will be differ between the buttons and I can make descisions in the code based on those values. Next I had a brilliant idea, it has to be battery powered! A Li-Po battery should do the trick. But we also like to charge the Li-Po battery. So I modified my prototype by hand, added buttons, a Li-Po battery and a Li-Po charger circuit (from Adafruit, which is open hardware and the schematics are avaiable on their website). The result was this:

![PCB_MOD](https://github.com/bewniac/securityfest-badge-2019/blob/master/images/Prototype_PCB_mod.jpg)

So now I got a prototype I feel pretty good about to work on. It runs on battery and the battery can be charged, AWESOME! Now I want to make the final design. A square badge feels so boring and we've got a cool logo to work with. So I started to create a schematic including the charging curcuit (based on the Adafruit schematic) and the buttons. The final schematic:

![Final schematic](https://github.com/bewniac/securityfest-badge-2019/blob/master/images/Final_schematic.png)

And the final PCB design:

![Final PCB](https://github.com/bewniac/securityfest-badge-2019/blob/master/images/Final_PCB.png)
