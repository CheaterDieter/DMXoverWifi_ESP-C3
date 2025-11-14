/*

  DMX Write

  This sketch allows you to write DMX data to a DMX listener using a standard
  DMX shield, such SparkFun ESP32 Thing Plus DMX to LED Shield. This sketch was
  made for the Arduino framework!

  Created 10 September 2021
  By Mitch Weisbrod

  https://github.com/someweisguy/esp_dmx

*/
#include <Arduino.h>
#include <esp_dmx.h>

int led = 8;

/* First, lets define the hardware pins that we are using with our ESP32. We
  need to define which pin is transmitting data and which pin is receiving data.
  DMX circuits also often need to be told when we are transmitting and when we
  are receiving data. We can do this by defining an enable pin. */
int transmitPin = 4;
int receivePin = 5;
int enablePin = 2;

/* Next, lets decide which DMX port to use. The ESP32 has either 2 or 3 ports.
  Port 0 is typically used to transmit serial data back to your Serial Monitor,
  so we shouldn't use that port. Lets use port 1! */
dmx_port_t dmxPort = 1;

/* Now we want somewhere to store our DMX data. Since a single packet of DMX
  data can be up to 513 bytes long, we want our array to be at least that long.
  This library knows that the max DMX packet size is 513, so we can fill in the
  array size with `DMX_PACKET_SIZE`. */
byte data[DMX_PACKET_SIZE];

/* This variable will allow us to update our packet and print to the Serial
  Monitor at a regular interval. */
unsigned long lastUpdate = millis();

void setup() {
  /* Start the serial connection back to the computer so that we can log
    messages to the Serial Monitor. Lets set the baud rate to 115200. */
  Serial.begin(115200);

  /* Now we will install the DMX driver! We'll tell it which DMX port to use,
    what device configuration to use, and what DMX personalities it should have.
    If you aren't sure which configuration to use, you can use the macros
    `DMX_CONFIG_DEFAULT` to set the configuration to its default settings.
    Because the device is being setup as a DMX controller, this device won't use
    any DMX personalities. */
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_personality_t personalities[] = {};
  int personality_count = 0;
  dmx_driver_install(dmxPort, &config, personalities, personality_count);

  /* Now set the DMX hardware pins to the pins that we want to use and setup
    will be complete! */
  dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);
}

void loop() {
  static bool state = false;
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate >= 1000) {
    state = !state;
    data[1] = state ? 255 : 0; // DMX Kanal 1 blinken
    dmx_write(dmxPort, data, DMX_PACKET_SIZE); // Schreibe DMX-Daten in den Puffer
    Serial.printf("DMX Kanal 1: %d\n", data[1]);
    lastUpdate = now;
  }

  
  dmx_send_num(dmxPort, DMX_PACKET_SIZE); // Sende DMX-Paket



  /* Wait until the DMX packet is sent before continuing. This isn't strictly
    necessary, but it can help keep the timing of your DMX packets more regular.
    The `DMX_TIMEOUT_TICK` macro is defined by the library and is a reasonable
    amount of time to wait for a DMX packet to be sent. */
  dmx_wait_sent(dmxPort, DMX_TIMEOUT_TICK);
}

