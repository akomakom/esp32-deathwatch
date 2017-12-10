ESP-IDF Death Watch for the ESP32
====================

This **IoT** project is lovingly called Death Watch because it serves a very simple purpose: 
it reports in-house activities so that some action can be taken should occupants seize to seem alive. 
It is designed to be run on the [Espressif ESP32](http://espressif.com/en/products/hardware/esp32/overview) 
and compiled using the ESP-IDF framework. 

#### Rationale

Although there may be many reasons do this, in my case this IoT project solves the 
*What if I die (or am otherwise detained), in or out of the house, and the pet(s) are left unattended and nobody knows?* dilemma.  It could equally well serve to monitor senile/disabled/elderly individuals (although there are probably better tools).

There are obviously many ways to solve this problem, 
but I wanted something that's more reliable than my phone. 
In fact, I wanted it to rely on as little hardware as possible, 
so I didn't involve any of my own servers or privately hosted code, 
relying instead only on WiFi and Google. 

#### Setup

##### The ESP-32 board is connected to:

* A **motion sensor** (ie HC-SR501) - detects human motion, placed where pets will not trip it (garage in my case)
* An **ultrasonic rangefinder** (ie US-100) - detects garage door position (both for fun and to serve its purpose).  Intended to be mounted on the ceiling pointed down, this can then identify 3 states: 
    * Door open (short distance to door)
    * Door closed (long distance to floor)
    * Car present (medium distance to car)
* A **temperature sensor** (DS18B20) - measures garage temperature just for fun.

##### It then does the following:

* **Monitors** above sensors and collects data.
* **Reports** said data via an HTTPS (SSL) POST request (eg: to Google Forms) 
* Also provides a **webserver** that allows said data to be retrieved via HTTP, if you want (JSON).


##### What you do with the data is up to you, but with Google Forms, the idea is to:

* Have time-series data in a spreadsheet (for example, number of motion events for each hour) 
* Run a scheduled Google Script that will send email (for example) when no motion has been detected for X hours.
  * The script could first email you as a warning, and after a further wait - send email to your designated first responders.
  * The script could have a whitelist spreadsheet (eg vacation days when no alarming is needed)
  * The script also sends reminders if the garage door is left open.
* Whatever else.

The above is fully implemented in google docs, I'll share the code once I figure out how to anonymize it. 
![Spreadsheet](/help/gdoc-screenshot2.png?raw=true "Spreadsheet")
![Spreadsheet](/help/gdoc-screenshot.png?raw=true "Spreadsheet")



#### Circuit

![Circuit](/help/circuit.png?raw=true "Circuit")


The wiring is trivial, sensors are connected to power, ground and GPIO pins, except for the following:

* US-100 rangefinder takes 5V input (taken from 5V pin) and outputs 5V signal (I think), so I run it through a resistor voltage divider (2.2k and 1k because that's what I had) before connecting to GPIO.  UPDATE: it proved to be very unstable at long range (300 cm), and I had to add a lot of code to ignore bad readings.  At this time I am powering it from 3.3v and the ECHO pin is connected directly to GPIO (no divider).  That doesn't make it any more stable, but the wiring is simpler.
* HC-SR501 motion detector takes 5V (taken from 5V pin) but outputs 3.3v, so it's just connected directly to GPIO.
* DS18B20 temperature sensor connects directly to 3.3v power/ground pins and to GPIO for signal, except that a >=3K resistor is used to pull-up (connects signal wire to 3.3v) 

All of the above are standard wiring connections and can be found on the interwebs.  
ADC1 pins are used for all of the above since WiFi is used (see defaults). 

#### Credits

Various components were adapted from:

* https://github.com/feelfreelinux/ds18b20
* https://github.com/cmmakerclub/esp32-webserver
* https://github.com/Ebiroll/esp32_ultra
* ESP32-IDF Examples


#### Prerequisites
This application is meant to be used with `Espressif IoT Development Framework`_ (ESP-IDF) which should be properly installed. 

Please check ESP-IDF docs for getting started instructions.

#### Additional Possibilities:

* Motion detected LED (either controlled by GPIO or directly form the motion sensor pin, depending on power requirements).
* Car position indicator (far enough into garage), probably with a second ultrasonic module.  This would have to be measuring very frequently to be useful and will need a bright light of some kind to indicate position.
* Configuration via the built-in webserver (flash storage for settings).  
* Control of garage door via webserver (relay to button).  Possibly even automatically close the door if left open.

