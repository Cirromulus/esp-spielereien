#include <Arduino.h>
// Plotclock
// cc - by Johannes Heberlein 2014
// v 1.02
// thingiverse.com/joo   wiki.fablab-nuernberg.de
// units: mm; microseconds; radians
// origin: bottom left of drawing surface
// time library see http://playground.arduino.cc/Code/time
// RTC  library see http://playground.arduino.cc/Code/time
//               or http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
// Change log:
// 1.01  Release by joo at https://github.com/9a/plotclock
// 1.02  Additional features implemented by Dave:
//       - added ability to calibrate servofaktor seperately for left and right servos
//       - added code to support DS1307, DS1337 and DS3231 real time clock chips
//       - see http://www.pjrc.com/teensy/td_libs_DS1307RTC.html for how to hook up the real time clock



#include <Timezone.h>    // https://github.com/JChristensen/Timezone
#include <Servo.h>

#include "config.hpp"
#include "arcstuff.hpp"

#ifdef REALTIMECLOCK
// for instructions on how to hook up a real time clock,
// see here -> http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
// DS1307RTC works with the DS1307, DS1337 and DS3231 real time clock chips.
// Please run the SetTime example to initialize the time on new RTC chips and begin running.

#include <Wire.h>
#include <DS1307RTC.h> // see http://playground.arduino.cc/Code/time
#include "setTime.hpp"

TimeChangeRule mySTD = {"CEST", week_t::Last, Sun, Mar, 2, +120};
TimeChangeRule myDST = {"CET",  week_t::Last, Sun, Oct, 2, +60};
Timezone timezone(myDST, mySTD);
#endif

#ifndef GLOWINDARK
  Servo servo_lift;
#endif
Servo servo_left;
Servo servo_right;

int last_min = -1;

void setup()
{
#ifdef REALTIMECLOCK
  Serial.begin(115200);
  //while (!Serial) { ; } // wait for serial port to connect. Needed for Leonardo only
  tmElements_t tm;
  if (RTC.read(tm))
  {
    setSyncProvider(RTC.get);
    setSyncInterval(60);
    time_t rtc_time = RTC.get();
    setTime(rtc_time);
    Serial.print("DS1307 time can be read OK: ");
    Serial.println(rtc_time);
  }
  else
  {
      Serial.println("Forgot Time! Setting it to the time of build");
      if (!setTimeFromBuild()) {
          Serial.println("Time lib could not set the time");
          setTime(0,0,0,0,0,0);
      }
      // Indicate that by drawing all dots on screen
      for (unsigned i = 0; i < 4; i++)
      {
        drawNumber(char_offset_x[i], char_offset_y, 11, FONT_SCALE);    // dots
      }
  }
  Serial.println("To set time, first enter date, then time (UTC!)");
  Serial.println("In the format:");
  Serial.println(__DATE__);
  Serial.println(__TIME__);
  if (timezone.locIsDST(RTC.get()))
  {
    Serial.println("By the way, the RTC suggests that we are currently in DST");
  }
#else
  // Set current time only the first to values, hh,mm are needed
  setTime(19,38,0,0,0,0);
#endif

  servo_left.attach(SERVOPINLEFT);
  servo_right.attach(SERVOPINRIGHT);
  drawTo(rest_position[0], rest_position[1]);
  servo_left.detach();
  servo_right.detach();

#ifndef GLOWINDARK
  servo_lift.attach(SERVOPINLIFT);  //  lifting servo
  lift(LIFT_DRAW);
#else
  pinMode(SERVOPINLIFT, OUTPUT);
  lift(LIFT_HIGH);
#endif

#ifdef SWITCHPIN
  pinMode(SWITCHPIN, INPUT_PULLUP);
#endif

}

void loop()
{
  #ifdef REALTIMECLOCK
    if (Serial.available()) {
        setTimeFromSerial();
    }
  #endif

#ifdef CALIBRATION
    while(true) {
      // Servohorns will have 90° between movements, parallel to x and y axis
      lift(LIFT_SWEP);
      drawTo(-3, 29.2);
      delay(500);
      lift(LIFT_DRAW);
      drawTo(74.1, 28);
      delay(1000);
      //square
      // drawTo(5, 15);
      // delay(2000);
      // drawTo(5, 50);
      // delay(500);
      // drawTo(70, 50);
      // delay(500);
      // drawTo(70,15);
      // delay(500);
  }
#endif

  #ifdef SWITCHPIN
    if (!digitalRead(SWITCHPIN))
    {
      // switch is "on", so don't write to screen
      return;
    }
  #endif

  time_t t = now();
  #ifdef REALTIMECLOCK
      // Serial.print("utcIsDST: ");
      // Serial.println(timezone.utcIsDST(t));
      // Serial.print("localIsDST: ");
      // Serial.println(timezone.locIsDST(t));

      // TimeChangeRule *tcr_used;
      t = timezone.toLocal(t);//, &tcr_used);
      // Serial.print(tcr_used->abbrev);
      // Serial.print(": ");
      // Serial.println(tcr_used->offset);

  #endif

  if (last_min != minute()) {
#ifndef GLOWINDARK
    if (!servo_lift.attached()) servo_lift.attach(SERVOPINLIFT);
#endif
    if (!servo_left.attached()) servo_left.attach(SERVOPINLEFT);
    if (!servo_right.attached()) servo_right.attach(SERVOPINRIGHT);
    delayMicroseconds(PATH_WRITE_DELAY_US);

#ifndef GLOWINDARK
    lift(LIFT_DRAW);
    drawNumber( 3, 3, 111, 1);   //wipe
#endif
    drawNumber(char_offset_x[0], char_offset_y, hour(t) / 10, FONT_SCALE);
    drawNumber(char_offset_x[1], char_offset_y, hour(t) % 10, FONT_SCALE);

    drawNumber(char_offset_x[2], char_offset_y, 11, FONT_SCALE);    // dots

    drawNumber(char_offset_x[3], char_offset_y, minute(t) / 10, FONT_SCALE);
    drawNumber(char_offset_x[4], char_offset_y, minute(t) % 10, FONT_SCALE);

    lift(LIFT_SWEP);
    drawTo(rest_position[0], rest_position[1]);   // pen rest
    lift(LIFT_HIGH);

    last_min = minute();

    // prevent servos glitching
#ifndef GLOWINDARK
    servo_lift.detach();
#endif
    servo_left.detach();
    servo_right.detach();
  }
}
