#pragma once

#include <Arduino.h>
#include <Servo.h>

void drawNumber(float bx, float by, int num, float scale);
void lift(uint16_t lift_amount);
void bogenUZS(float bx, float by, float radius, int start, int ende, float sqee);
void bogenGZS(float bx, float by, float radius, int start, int ende, float sqee);
void drawTo(double pX, double pY);
