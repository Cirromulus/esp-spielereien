#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string.h>

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
        char answer[50] ;
        sprintf(answer, "l: %04d r: %04d\n", l, r);
        analogWrite(LINKS, l);
        analogWrite(RECHTS, r);
        server.send(200, "text/plain", answer);
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
