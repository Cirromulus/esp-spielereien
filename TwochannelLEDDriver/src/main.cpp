#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string.h>
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h>
#include <DS3232RTC.h>
#include <sunMoon.h>

#define OUR_latitude    53.075600
#define OUR_longtitude  8.801781
#define OUR_timezone    120                     // localtime with UTC difference in minutes
#define DS3231_ADDRESSE 0x68


const char * ssid = "BehindertesWLANFuerDenDrucker";
const char * password = "0~0~0}{0~";

#define WARM 2
#define KALT 0
#define AW_MAX 1024

uint16_t kalt = 0, warm = 0;
bool startup = true;

ESP8266WebServer server(80);
DS3232RTC RTC(false);
sunMoon sm;
IPAddress myIp;

void blinkIP();
void handleAndDelay(uint16_t wait);
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
void setLight();

void setup() {
    pinMode(WARM, OUTPUT);
    pinMode(KALT, OUTPUT);

    Wire.begin(1, 3);

    setSyncProvider(RTC.get);                     // the function to get the time from the RTC
    if (timeStatus() != timeSet)
      return;
    sm.init(OUR_timezone, OUR_latitude, OUR_longtitude);
    setLight();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        /*
        l = l ? 0 : 512;
        r = r ? 0 : 512;
        analogWrite(WARM, l);
        analogWrite(KALT, r);
        */

    }

    //analogWrite(WARM, l = 0);
    //analogWrite(KALT, r = 0);
    MDNS.begin("MultiLED");
    myIp = WiFi.localIP();
    startup = false;        //to prevent blinking the IP
    server.on("/", []()
    {

        if(server.hasArg("warm"))
        {
            warm = std::strtoul(server.arg("warm").c_str(), nullptr, 10);
        }
        if(server.hasArg("kalt"))
        {
            kalt = std::strtoul(server.arg("kalt").c_str(), nullptr, 10);
        }
        time_t myTime = RTC.get();
        time_t sRise = sm.sunRise();
        time_t sSet  = sm.sunSet();
        char answer[100] ;
        sprintf(answer, "warm: %04d kalt: %04d\nnow: %lu, rise: %lu, set: %lu", warm, kalt, myTime, sRise, sSet);

        analogWrite(WARM, warm);
        analogWrite(KALT, kalt);
        server.send(200, "text/plain", answer);
    });

    server.on("/setClock", []()
    {
        if(server.hasArg("ts"))
        {
            time_t ts = std::strtoul(server.arg("ts").c_str(), nullptr, 10);
            char answer[50] ;
            sprintf(answer, "new timestamp: %lu", ts);
            RTC.set(ts);
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
    static uint32_t counter = 0;
    if(counter++ > 0xFFFFF)
    {
        setLight();
        counter = 0;
    }
}

void blinkIP()
{
    analogWrite(WARM, 0);
    analogWrite(KALT, 0);
    while(startup)
    {
        for(uint8_t i = 0; i < myIp[3]; i++)
        {
            analogWrite(WARM, 250);
            analogWrite(KALT, 250);
            handleAndDelay(200);
            analogWrite(WARM, 0);
            analogWrite(KALT, 0);
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


void setLight()
{
    analogWrite(WARM, warm = AW_MAX);
    time_t now = RTC.get();
    if(now > sm.sunRise() && now < sm.sunSet())
    {
        //hell!
        analogWrite(KALT, kalt = AW_MAX);
    }
    else
    {
        analogWrite(KALT, kalt = 0);
    }
}
