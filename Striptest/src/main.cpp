#include <Adafruit_NeoPixel.h>
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

#define PIN 2

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(72*4, PIN, NEO_GRB + NEO_KHZ800);
//AsyncUDP udp;

enum class Mode : uint8_t
{
    ip,
    colorWipe,
    theaterChase,
    rainbow,
    rainbowCycle,
    none
};

static const char* modeNames[] =
{
    "ip",
    "colorWipe",
    "theaterChase",
    "rainbow",
    "rainbowCycle",
    "none"
};

Mode activeMode = Mode::ip;
uint8_t r = 126, g = 126, b = 200;
ESP8266WebServer server(80);
IPAddress myIp;

uint32_t Wheel(byte WheelPos);
void theaterChaseRainbow(uint8_t wait);
void theaterChase(uint32_t c, uint8_t wait);
void rainbowCycle(uint8_t wait);
void rainbow(uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);

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

bool HandleAndDelay(uint8_t wait)
{
    unsigned long prev = millis();
    Mode bak = activeMode;
    server.handleClient();
    unsigned long now = millis();

    if(now - prev < wait)
        delay(wait - (now-prev));
    return bak != activeMode;
}

void setup() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        while(1) {
            delay(500);
        }
    }
    myIp = WiFi.localIP();
    server.on("/", [](){
        bool modeWrong = false;
        if(server.hasArg("mode"))
        {
            uint8_t rawMode = std::strtoul(server.arg("mode").c_str(), nullptr, 10);
            if(rawMode > static_cast<uint8_t>(Mode::none))
            {
                modeWrong = true;
            }
            else
            {
                activeMode = static_cast<Mode>(rawMode);
            }
        }
        if(server.hasArg("r"))
        {
            r = std::strtoul(server.arg("r").c_str(), nullptr, 10);
        }
        if(server.hasArg("g"))
        {
            g = std::strtoul(server.arg("g").c_str(), nullptr, 10);
        }
        if(server.hasArg("b"))
        {
            b = std::strtoul(server.arg("b").c_str(), nullptr, 10);
        }

        char answer[50] ;
        sprintf(answer, "Mode: %s\nr: %03d g: %03d b: %03d\n%s", modeNames[static_cast<uint8_t>(activeMode)], r, g, b, modeWrong ? "invalid mode" : " ");

        server.send(200, "text/plain", answer);
    });

    server.onNotFound(handleNotFound);
    server.begin();
}

void loop() {
    switch(activeMode)
    {
        case Mode::ip:
            for(uint16_t i=0; i<myIp[3]; i++) {
                strip.setPixelColor(i, strip.Color(0, (i % 10) > 4 ? g : 0, (i % 10) > 4 ? 0 : b));
            }
            strip.show();
            activeMode = Mode::none;
            break;
        case Mode::colorWipe:
            colorWipe(strip.Color(r, g, b), 50);
            break;
        case Mode::theaterChase:
            theaterChase(strip.Color(r, g, b), 50);
            break;
        case Mode::rainbow:
            rainbow(18);
            break;
        case Mode::rainbowCycle:
            rainbowCycle(18);
            break;
        default:
        break;
    }
    server.handleClient();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    if(HandleAndDelay(wait))
        return;
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    if(HandleAndDelay(wait))
        return;
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    if(HandleAndDelay(wait))
        return;
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      if(HandleAndDelay(wait))
          return;

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      if(HandleAndDelay(wait))
          return;

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
