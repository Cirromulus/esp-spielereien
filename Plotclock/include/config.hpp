#pragma once

#include <inttypes.h>

// delete or mark the next line as comment if you don't need these
//#define CALIBRATION      // enable calibration mode
#define REALTIMECLOCK    // enable real time clock
#define GLOWINDARK

// When in calibration mode, adjust the following factor until the servos move exactly 90 degrees
#define SERVOFAKTORLEFT 655
#define SERVOFAKTORRIGHT 655

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that
// the servo arms are at all times parallel
// either to the X or Y axis
#define SERVOLEFTNULL 2200
#define SERVORIGHTNULL 920

//#define SERVOPINLIFT  PB10
//#define SERVOPINLEFT  PB1
//#define SERVOPINRIGHT PB0

// lift positions of lifting servo
#define LIFT0 1080 // on drawing surface
#define LIFT1 925  // between numbers
#define LIFT2 700  // going towards sweeper

// speed of liftimg arm, higher is slower
#define LIFTSPEED 1500
static constexpr uint32_t PATH_WRITE_DELAY_US=2050;

// length of arms
#define L1 33.0
#define L2 54.0 //55.1
#define L3 12 //13.2

// origin points of left and right servo
// this is a lie, but negative y values are unsupported
#define EXTRA_Y_OFFS 0
#define O1X 24
#define O1Y (-29+EXTRA_Y_OFFS)
#define O2X 46
#define O2Y (-29+EXTRA_Y_OFFS)

// pen_offset = {5, 19, 28, 34, 48};
static constexpr unsigned char_offset_x [] = {5, 21, 33, 44, 57};
static constexpr unsigned char_offset_y = 13+EXTRA_Y_OFFS;
// pen rest = {74.2, 47.5};
static constexpr unsigned rest_position [] = {40, 0};
