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

    // get the date and time the compiler was run
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

bool setTimeFromSerial() {
    static constexpr unsigned max_bufsize = 32;
    char date[max_bufsize+1];
    char time[max_bufsize+1];
    char c;
    uint8_t count = 0;
    while(count < max_bufsize) {
        while(!Serial.available()){};
        if((c = Serial.read()) == '\n')
            break;
        date[count++] = c;
        Serial.print(c);
    }
    date[count] = 0;
    Serial.print(c);
    Serial.println("ok, now time:");
    count = 0;
    while(count < max_bufsize) {
        while(!Serial.available()){};
        if((c = Serial.read()) == '\n')
            break;
        time[count++] = c;
        Serial.print(c);
    }
    time[count] = 0;
    Serial.println(c);

    return setTimeFromString(date,time);
}

bool setTimeFromBuild() {
    return setTimeFromString(__DATE__,__TIME__);
}
