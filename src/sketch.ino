/* index.html muss in den Speicher geladen werden mit 
pio run --target uploadfs --upload-port COM10
*/



#include <Arduino.h>
#include <esp_dmx.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

int led = 8;

/* First, lets define the hardware pins that we are using with our ESP32. We
  need to define which pin is transmitting data and which pin is receiving data.
  DMX circuits also often need to be told when we are transmitting and when we
  are receiving data. We can do this by defining an enable pin. */
int transmitPin = 5;
int receivePin = 6;
int enablePin = 7;

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

const char* ssid = "DMX-AP";
const char* password = "dmx12345";
WebServer server(80);
bool dmxEnabled = true;





void setup() {
  /* Start the serial connection back to the computer so that we can log
    messages to the Serial Monitor. Lets set the baud rate to 115200. */
  Serial.begin(115200);
  SPIFFS.begin(true);

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

  
 // Für Betrieb als Access Point
  WiFi.softAP(ssid, password);

  // Für Betrieb als Client
  /*
   WiFi.begin("FRITZ!Box 6890 HR", "XXXXXXXXX");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nMit WLAN verbunden, IP-Adresse: ");
  Serial.println(WiFi.localIP());
*/


  server.on("/", handleRoot);
  server.on("/dmx", handleDMX);
  server.begin();
  Serial.println("HTTP API server started");
}

void loop() {
  server.handleClient();
  dmx_write(dmxPort, data, DMX_PACKET_SIZE);
  dmx_send_num(dmxPort, DMX_PACKET_SIZE);
  dmx_wait_sent(dmxPort, DMX_TIMEOUT_TICK);
}

void handleDMX() {
  String datastring = "";
  for ( int i = 1; i <= 255; i++)
  {
    String argName = String(i);
    if (server.hasArg(argName)) {
      int v = server.arg(argName).toInt();
      data[i] = constrain(v, 0, 255);
    }
  }
  // datastring als 512 Hex-Zahlen ausgeben
  for (int i = 1; i <= 512; i++) {
    if (data[i] < 16) datastring += "0"; // führende Null
    datastring += String(data[i], HEX);
    if (i < 512) datastring += " ";
  }
  datastring.toUpperCase();
  server.send(200, "text/plain", datastring);
  Serial.println(datastring);
}

void handleRoot() {
  if (!SPIFFS.begin(true)) {
    server.send(500, "text/plain", "SPIFFS Fehler");
    return;
  }
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "index.html nicht gefunden");
    return;
  }
  String html = file.readString();
  server.send(200, "text/html", html);
  file.close();
}