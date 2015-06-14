#include <Adafruit_ESP8266.h>
#include <SoftwareSerial.h>
#define SSID "NETGEAR00" //this must be read from the sd-card
#define PASS "Orione2000%" //this must be read from the sd-card
//#define SSID "esp8266_network" //this must be read from the sd-card
//#define PASS "esp8266_project"
#define HOST "192.168.1.2" //this must be read from the sd-card
#define PORT 8000
#define ESP_RX   3
#define ESP_TX   4
#define ESP_RST  8
#define URI "/message/new/"

SoftwareSerial softser(ESP_RX, ESP_TX);
boolean isOffline;

// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
Adafruit_ESP8266 wifi(&softser, &Serial, ESP_RST);


void setup() {
  // put your setup code here, to run once:
  softser.begin(9600); // Soft serial connection to ESP8266
  Serial.begin(9600); while (!Serial);
  delay(1000); // UART serial debug
  isOffline = true;
  
  
}

void loop() {

   isOffline = !wifi.connectToAP(F(SSID), F(PASS));
   delay(500);

  if (!isOffline) {

      wifi.find(); // Discard the 'OK' that follows
      delay(500);
      while (wifi.connectTCP(F(HOST), PORT)) {
        //here we listen for new messages and we send them to our web server
          char *json_string = createJson("les","hello there from esp8266");
          delay(500);
          
          //we don't want to send null objects
          if(strlen(json_string) > 2)
               isOffline = wifi.httpPost(HOST, "/message/new/", json_string);
          delay(100);
          
          if(!isOffline){
             Serial.println(F("Connection fail"));
             //save data somewhere
          }
          free(json_string);
        }
        wifi.closeTCP();
        delay(500);
      }
  else{
    //do flash save here
    Serial.println("Flash mode initiated");
    
    //at the end we re-try conenction
    isOffline = wifi.connectToAP(F(SSID), F(PASS));
  }
  delay(5000);
  //wifi.softReset(); {message:user:}



}
char* createJson(const char *user, const char *message){

  char *json = new char[16+strlen(user)+strlen(message)];
  strcpy(json,"");
  strcat(json,"{\"message\":\"");
  strcat(json,message);
  strcat(json,",\",\"user\":\"");
  strcat(json,user);
  strcat(json,"\"}");
  return json;
    
}
