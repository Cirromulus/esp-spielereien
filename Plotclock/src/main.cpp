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

  TimeChangeRule myDST = {"CEST", week_t::Second, Sun, Mar, 2, +120};
  TimeChangeRule mySTD = {"CET",  week_t::First, Sun, Nov, 2, +60};
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
      if (!setTimeFromBuild()) {
          Serial.println("Time lib could not set the time");
          setTime(0,0,0,0,0,0);
      }
  }
#else
  // Set current time only the first to values, hh,mm are needed
  setTime(19,38,0,0,0,0);
#endif


  drawTo(rest_position[0], rest_position[1]);
#ifndef GLOWINDARK
  servo_lift.attach(SERVOPINLIFT);  //  lifting servo
  lift(LIFT0);
#else
  pinMode(SERVOPINLIFT, OUTPUT);
  lift(LIFT1);
#endif
  servo_left.attach(SERVOPINLEFT);  //  left servo
  servo_right.attach(SERVOPINRIGHT);  //  right servo
}

void loop()
{

#ifdef CALIBRATION
    while(true) {
        /*
          // Servohorns will have 90° between movements, parallel to x and y axis
          lift(LIFT2);
          drawTo(-3, 29.2);
          delay(500);
          drawTo(74.1, 28);
          delay(750);
          lift(LIFT0);
          delay(1000);
      */
      //square
      drawTo(5, 15);
      delay(2000);
      drawTo(5, 50);
      delay(500);
      drawTo(70, 50);
      delay(500);
      drawTo(70,15);
      delay(500);
  }
#endif

    time_t t = now();
    #ifdef REALTIMECLOCK
        t = timezone.toLocal(t);
    #endif

  if (last_min != minute()) {
#ifndef GLOWINDARK
    if (!servo_lift.attached()) servo_lift.attach(SERVOPINLIFT);
#endif
    if (!servo_left.attached()) servo_left.attach(SERVOPINLEFT);
    if (!servo_right.attached()) servo_right.attach(SERVOPINRIGHT);
    delayMicroseconds(PATH_WRITE_DELAY_US);

#ifndef GLOWINDARK
    lift(LIFT0);
    drawNumber( 3, 3, 111, 1);   //wipe
#endif
    drawNumber(char_offset_x[0], char_offset_y, hour(t) / 10, 0.9);
    drawNumber(char_offset_x[1], char_offset_y, hour(t) % 10, 0.9);

    drawNumber(char_offset_x[2], char_offset_y, 11, 0.9);    // dots

    drawNumber(char_offset_x[3], char_offset_y, minute(t) / 10, 0.9);
    drawNumber(char_offset_x[4], char_offset_y, minute(t) % 10, 0.9);

    delayMicroseconds(PATH_WRITE_DELAY_US);

    lift(LIFT2);
    drawTo(rest_position[0], rest_position[1]);   // pen rest
    lift(LIFT1);

    last_min = minute();

    // prevent servos glitching
#ifndef GLOWINDARK
    servo_lift.detach();
#endif
    servo_left.detach();
    servo_right.detach();
  }

#ifdef REALTIMECLOCK
    if (Serial) {
        setTimeFromSerial();
    }
#endif


}
