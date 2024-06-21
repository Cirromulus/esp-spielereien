#include "setTime.hpp"
#include "Arduino.h"
#include <Wire.h>
#include <DS1307RTC.h>

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

tmElements_t tm;

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

bool setTimeFromString(const char* date, const char* time) {
    bool parse=false;
    bool config=false;

    // check validity of given strings
    if (getDate(date) && getTime(time)) {
        parse = true;
        // and configure the RTC with this info
        if (RTC.write(tm)) {
            config = true;
            setTime(tm.Hour,tm.Minute,tm.Second,tm.Day,tm.Month,tm.Year);
        }
    }

    if (parse && config) {
        Serial.print("DS1307 configured!\n Time=\"");
        Serial.print(time);
        Serial.print("\", Date=\"");
        Serial.print(date);
        Serial.println("\"");
        return true;
    } else if (parse) {
        Serial.println("DS1307 Communication Error :-{");
        Serial.println("Please check your circuitry");
    } else {
        Serial.print("Could not parse info, Time=\"");
        Serial.print(time);
        Serial.print("\", Date=\"");
        Serial.print(date);
        Serial.println("\"");
    }
    return false;
}

bool readInto(char* into, const unsigned max_bufsize)
{
    static constexpr uint32_t readline_timeout_seconds = 10;
    // static_assert(std::numeric_limits<decltype(millis())>::max() >= readline_timeout_seconds * 1000);

    uint8_t count = 0;
    while(count < max_bufsize) {
        const auto startWait = millis();
        while(!Serial.available())
        {
            if (millis() - startWait > readline_timeout_seconds * 1000)
            {
                Serial.println("Timeout... resetting");
                return false;
            }
        };
        into[count] = Serial.read();

        if(into[count] == '*')  // Special character to indicate remote's readyness!
            continue;           // this char will be ignored in buffer.

        Serial.print(into[count]);

        if(into[count] == '\r')
            break;
        if(into[count] == '\n')
            break;
        if(!isPrintable(into[count]))
        {
            Serial.println("character broken...? resetting");
            return false;
        }
        count++;
    }
    into[count] = 0;
    return true;
}

bool setTimeFromSerial() {
    static constexpr unsigned max_bufsize = 32;
    char date[max_bufsize+1];
    char time[max_bufsize+1];
    Serial.print("Date like '");
    Serial.print(__DATE__);
    Serial.println("':");
    if (!readInto(date, max_bufsize))
        return false;
    Serial.print("\ngot string '");
    Serial.print(date);
    Serial.println("'");
    Serial.print("\nok, now time like '");
    Serial.print(__TIME__);
    Serial.println("':");
    if (!readInto(time, max_bufsize))
        return false;
    Serial.print("\ngot string '");
    Serial.print(time);
    Serial.println("'\n");
    return setTimeFromString(date,time);
}

bool setTimeFromBuild() {
    return setTimeFromString(__DATE__,__TIME__);
}
