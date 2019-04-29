#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string.h>
#include "Wire.h"
#define DS3231_ADDRESSE 0x68


const char * ssid = "BehindertesWLANFuerDenDrucker";
const char * password = "0~0~0}{0~";

#define LINKS 2
#define RECHTS 0

uint16_t l = 0, r = 512;
bool startup = true;

ESP8266WebServer server(80);
IPAddress myIp;

void blinkIP();
void handleAndDelay(uint16_t wait);
byte decToBcd(byte val) {
// Dezimal Zahl zu binary coded decimal (BCD) umwandeln
  return((val/10*16) + (val%10));
}
byte bcdToDec(byte val) {
// BCD (binary coded decimal) in Dezimal Zahl umwandeln
  return((val/16*10) + (val%16));
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
    pinMode(LINKS, OUTPUT);
    pinMode(RECHTS, OUTPUT);

    analogWrite(LINKS, l);
    analogWrite(RECHTS, r);

    Wire.begin(1, 3);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        l = l ? 0 : 512;
        r = r ? 0 : 512;
        analogWrite(LINKS, l);
        analogWrite(RECHTS, r);
    }

    analogWrite(LINKS, l = 0);
    analogWrite(RECHTS, r = 0);
    MDNS.begin("MultiLED");
    myIp = WiFi.localIP();
    server.on("/", []()
    {
        startup = false;
        if(server.hasArg("l"))
        {
            l = std::strtoul(server.arg("l").c_str(), nullptr, 10);
        }
        if(server.hasArg("r"))
        {
            r = std::strtoul(server.arg("r").c_str(), nullptr, 10);
        }
        Wire.beginTransmission(DS3231_ADDRESSE);
        Wire.write(0); // DS3231 Register zu 00h
        Wire.endTransmission();
        Wire.requestFrom(DS3231_ADDRESSE, 7); // 7 Byte Daten vom DS3231 holen
        byte sekunde = bcdToDec(Wire.read() & 0x7f);
        bcdToDec(Wire.read());
        bcdToDec(Wire.read() & 0x3f);
        bcdToDec(Wire.read());
        bcdToDec(Wire.read());
        bcdToDec(Wire.read());
        bcdToDec(Wire.read());

        char answer[50] ;
        sprintf(answer, "l: %04d r: %04d\nSekunden: %d", l, r, sekunde);

        analogWrite(LINKS, l);
        analogWrite(RECHTS, r);
        server.send(200, "text/plain", answer);
    });

    server.on("/setClock", []()
    {
        if(server.hasArg("ts"))
        {
            unsigned long newTS = std::strtoul(server.arg("ts").c_str(), nullptr, 10);
            char answer[50] ;
            sprintf(answer, "new timestamp: %d", ts);
            server.send(200, "text/plain", answer);
        }
        server.send(200, "text/plain", "no timestamp");
    });

    server.onNotFound(handleNotFound);
    server.begin();
}

void loop() {
    server.handleClient();
    if(startup)
        blinkIP();
}

void blinkIP()
{
    analogWrite(LINKS, 0);
    analogWrite(RECHTS, 0);
    while(startup)
    {
        for(uint8_t i = 0; i < myIp[3]; i++)
        {
            analogWrite(LINKS, 250);
            analogWrite(RECHTS, 250);
            handleAndDelay(200);
            analogWrite(LINKS, 0);
            analogWrite(RECHTS, 0);
            handleAndDelay(200);
            if(!startup)
                return;
        }
        handleAndDelay(800);
    }
}

void handleAndDelay(uint16_t wait)
{
    unsigned long target = millis() + wait;

    while(millis() < target)
    {
        server.handleClient();
    }
}
