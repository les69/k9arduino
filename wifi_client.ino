#include <Adafruit_ESP8266.h>
#include <SoftwareSerial.h>

#include <aJSON.h>
#define SSID "Netgear00M" //this must be read from the sd-card
#define PASS "Orione2000%" //this must be read from the sd-card
#define HOST "192.168.1.5" //this must be read from the sd-card
#define PORT 8000
#define ESP_RX   3
#define ESP_TX   4
#define ESP_RST  8
#define URI "/message/new/"

SoftwareSerial softser(ESP_RX, ESP_TX);

// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
Adafruit_ESP8266 wifi(&softser, &Serial, ESP_RST);


void setup() {
  // put your setup code here, to run once:
  softser.begin(9600); // Soft serial connection to ESP8266
  Serial.begin(9600); while (!Serial);
  delay(1000); // UART serial debug




}

void loop() {

  char buffer[50];
  boolean isOffline = false;
  //Serial.print(F("Connecting to WiFi..."));
  //wifi.println("AT+CWJAP=\"D-Link\",\"Orione2000%\"");

  if (wifi.connectToAP(F(SSID), F(PASS)) || !isOffline) {

    delay(1000);
    if (wifi.readLine(buffer, sizeof(buffer))) {
      //Serial.println(buffer);
      wifi.find(); // Discard the 'OK' that follows
      wifi.println("AT+CIPMUX=0"); // configure for single connection,
      //we should only be connected to one SMTP server
      wifi.find();
      wifi.closeTCP(); // close any open TCP connections
      wifi.find();

      if (wifi.connectTCP(F(HOST), PORT)) {
        Serial.print(F("OK\nRequesting page..."));

        //here we listen for new messages and we send them to our web server
        while (!isOffline) {
          aJsonObject* json = aJson.createObject();
          aJson.addItemToObject(json, "message", aJson.createItem("hello there"));
          aJson.addItemToObject(json, "user", aJson.createItem("les"));
          char *json_String = aJson.print(json);
          wifi.httpPost("192.168.1.5", "/message/new/", aJson.print(json));
          isOffline = wifi.find(F("ERROR"));
          delay(100);
        }
        wifi.closeTCP();
      }
      else { // TCP connect failed
        Serial.println(F("Connection fail"));
      }
    }
  }
  else{
    //do flash save here
    Serial.println("Flash mode initiated");
  }
  delay(5000);
  //wifi.softReset();


}
