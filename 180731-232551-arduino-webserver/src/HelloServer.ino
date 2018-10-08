#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncUDP.h>

const char * ssid[] = {"SecurityCam13", "WLAN nicht verfuegbar."};
const char * password[] = {"VollerEmpfang", "Ich habe kein Passwort."};
const uint8_t APs = 2;
const char myName[] = "esp8266-dehumidifier";

ESP8266WebServer server(80);
AsyncUDP udp;

const int led = 2;
const int input = 0;
bool ledOn = false;
unsigned long lastUdpBeacon = 0;

void handleRoot() {
  char response[] = "esp8266dehumidifier\nled X is xxx. input x is xxx.";
  response[24] = led + '0';
  response[40] = input + '0';

  if(server.hasArg("led"))
  {
      ledOn = server.arg("led") == "1";
      digitalWrite(led, !ledOn);
  }
  Serial.print("Led ");
  if(ledOn)
  {
      Serial.println("on");
      memcpy(&response[29], "on. ", 4);
  }
  else
  {
      Serial.println("off");
      memcpy(&response[29], "off.", 4);
  }
  if(!digitalRead(input))
  {
      memcpy(&response[45], "on. ", 4);
  }
  else
  {
      memcpy(&response[45], "off.", 4);
  }
  server.send(200, "text/plain", response);
}

void handleNotFound(){
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  pinMode(input, INPUT_PULLUP);
  digitalWrite(led, 1);
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("");

  // Wait for connection
  uint8_t ap;
  for(ap = 0; ap < APs; ap++)
  {
      Serial.print("Trying to connect to ");
      Serial.println(ssid[ap]);
      WiFi.begin(ssid[ap], password[ap]);
      for(uint8_t tries = 30; WiFi.status() != WL_CONNECTED && WiFi.status() != WL_CONNECT_FAILED && tries != 0; tries--) {
          delay(500);
          Serial.println(tries);
      }
      if(WiFi.status() == WL_CONNECTED)
      {
          goto init;
      }
  }
  while(true)
  {
      Serial.println("WiFi connect failed.");
      delay(1000);
  }

init:
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid[ap]);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(myName)) {
    Serial.print("MDNS responder started as ");
    Serial.println(myName);
  }

  server.on("/", handleRoot);

  server.on("/pin", [](){
    server.send(200, "text/plain", !digitalRead(input) ? "1" : "0");
  });
  server.on("/led", [](){
    if(server.hasArg("on"))
    {
        ledOn = true;
        digitalWrite(led, !ledOn);
    }
    else if(server.hasArg("off"))
    {
        ledOn = false;
        digitalWrite(led, !ledOn);
    }
    server.send(200, "text/plain", ledOn ? "1" : "0");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  if(millis() - lastUdpBeacon > 5000)
  {
      udp.broadcastTo(myName, 6666);
      lastUdpBeacon = millis();
  }
}
