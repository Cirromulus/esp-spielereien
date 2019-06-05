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
#define OUR_timezone    0                     // localtime with UTC difference in minutes
#define DS3231_ADDRESSE 0x68


const char * ssid = "BehindertesWLANFuerDenDrucker";
const char * password = "0~0~0}{0~";

#define WARM 2
#define KALT 0
#define SDA 1
#define SCL 3
#define AW_MAX 1024

uint16_t kalt = 0, warm = 0;
uint16_t kaltMax = AW_MAX, warmTarget = AW_MAX;
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
void clearI2CBus();

void setup() {
    pinMode(WARM, OUTPUT);
    pinMode(KALT, OUTPUT);

    clearI2CBus();
    Wire.begin(SDA, SCL);

    setSyncProvider(RTC.get);                     // the function to get the time from the RTC
    if (timeStatus() != timeSet){
      blinkIP();
      return;
    }
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
    MDNS.begin("DualLED");
    myIp = WiFi.localIP();
    startup = false;        //to prevent blinking the IP
    server.on("/", []()
    {
        if(server.hasArg("warm"))
        {
            warmTarget = std::strtoul(server.arg("warm").c_str(), nullptr, 10);
        }
        if(server.hasArg("kalt"))
        {
            kaltTarget = std::strtoul(server.arg("kalt").c_str(), nullptr, 10);
        }
        time_t myTime = RTC.get();
        time_t sRise = sm.sunRise();
        time_t sSet  = sm.sunSet();
        char answer[150] ;
        sprintf(answer, "warm: %04d kalt: %04d\nnow: %lu, rise: %l, set: %l (%s)\n(/setClock?ts=[timestamp])",
                warmTarget, kaltTarget, myTime, sRise - myTime, sSet - myTime, (myTime > sRise && myTime < sSet) ? "day" : "night");

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
            setLight();
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
    if(counter++ > 0x0FFF)
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

    time_t now = RTC.get();
    if(now > sm.sunRise() && now < sm.sunSet())
    {
        //hell!
        analogWrite(WARM, warm = warmTarget);
        analogWrite(KALT, kalt = kaltTarget);
    }
    else
    {
        analogWrite(WARM, warm = warmTarget);
        analogWrite(KALT, kalt = 0);
    }
}

void clearI2CBus()
{
    //Serial.print(digitalRead(PIN_SCL));    //should be HIGH
    //Serial.println(digitalRead(PIN_SDA));   //should be HIGH, is LOW on stuck I2C bus
    pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
    pinMode(SCL, INPUT_PULLUP);

    if(digitalRead(SCL) == HIGH && digitalRead(SDA) == LOW) {
          //Serial.println("reset");
          pinMode(SDA, OUTPUT);      // is connected to SDA
          digitalWrite(SDA, LOW);
          delay(2000);               //maybe too long
          pinMode(SDA, INPUT);       // reset pin
          delay(50);
    }
    //Serial.print(digitalRead(26));    // HIGH
    //Serial.println(digitalRead(25));  // HIGH
}
