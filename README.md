ESP-IDF Death Watch
====================

The application is lovingly called Death Watch because it serves a very simple purpose: 
it reports in-house activities so that some action can be taken should occupants seize to seem alive.

#### Rationale

Although there may be many reasons do this, in my case this IoT project solves the 
*What if I die, in or out of the house, and the pet(s) are left unattended and nobody knows?* dilemma.  It could equally well serve to monitor senile/disabled/elderly individuals (although there are probably better tools).

There are obviously many ways to solve this problem, 
but I wanted something that's more reliable than my phone. 
In fact, I wanted it to rely on as little hardware as possible, 
so I didn't involve any of my own servers or privately hosted code, 
relying instead only on WiFi and Google. 

#### What it does

##### In my application, the ESP-32 board connects to:

* A **motion sensor** (ie HC-SR501) - detects human motion, placed where pets will not trip it (garage in my case)
* An **ultrasonic rangefinder** (ie US-100) - detects garage door position (both for fun and to serve its purpose).  Intended to be mounted on the ceiling pointed down, this can then identify 3 states: 
    * Door open (short distance to door)
    * Door closed (long distance to floor)
    * Car present (medium distance to car)
* A **temperature sensor** (DS18B20) - measures garage temperature just for fun.

##### It then does the following:

* **Monitors** above sensors and collects data.
* **Reports** said data via an HTTPS (SSL) request (eg: to Google Forms)
* Also provides a **webserver** that allows said data to be retrieved via HTTP, if you want (JSON).


##### What you do with the data is up to you, but with Google Forms, the idea is to:

* Have time-series data in a spreadsheet (for example, number of motion events for each hour) 
* Run a scheduled Google Script that will send email (for example) when no motion has been detected for X hours.
  * The script could first email you as a warning, and after a further wait - send email to your designated first responders.
  * The script could have a whitelist spreadsheet (eg vacation days when no alarming is needed)
* Whatever else.



#### Circuit

![Circuit](/help/circuit.png?raw=true "Circuit")


The wiring is trivial, sensors are connected to power, ground and GPIO pins, except for the following:

* US-100 rangefinder takes 5V input (taken from 5V pin) and outputs 5V signal, so I run it through a resistor voltage divider (2.2k and 1k because that's what I had) before connecting to GPIO.
* HC-SR501 motion detector takes 5V (taken from 5V pin) but outputs 3.3v, so it's just connected directly to GPIO.
* DS18B20 temperature sensor connects directly to 3.3v power/ground pins and to GPIO for signal, except that a >3k resistor is used to pull-up. 

All of the above are standard wiring connections and can be found on the interwebs.  
ADC1 pins are used for all of the above since WiFi is used (see defaults). 

#### Credits

* https://github.com/feelfreelinux/ds18b20
* https://github.com/cmmakerclub/esp32-webserver
* https://github.com/Ebiroll/esp32_ultra
* ESP32-IDF Examples


#### Prerequisites
This application is meant to be used with `Espressif IoT Development Framework`_ (ESP-IDF) which should be properly installed. 

Please check ESP-IDF docs for getting started instructions.



