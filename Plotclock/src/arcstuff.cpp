#include "arcstuff.hpp"
#include "config.hpp"

#ifndef GLOWINDARK
    extern Servo servo_lift;
#endif
extern Servo servo_left;
extern Servo servo_right;

static unsigned servoLift = 1500;  //also misused as "is writing" state
static double lastX = rest_position[0];
static double lastY = rest_position[1];

// Writing numeral with bx by being the bottom left originpoint. Scale 1 equals a 20 mm high font.
// The structure follows this principle: move to first startpoint of the numeral, lift down, draw numeral, lift up
void drawNumber(float bx, float by, int num, float scale) {

  switch (num) {

  case 0:
    drawTo(bx + 12 * scale, by + 6 * scale);
    lift(LIFT0);
    bogenGZS(bx + 7 * scale, by + 10 * scale, 10 * scale, -0.8, 6.7, 0.5);
    lift(LIFT1);
    break;
  case 1:

    drawTo(bx + 3 * scale, by + 15 * scale);
    lift(LIFT0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(LIFT1);
    break;
  case 2:
    drawTo(bx + 2 * scale, by + 12 * scale);
    lift(LIFT0);
    bogenUZS(bx + 8 * scale, by + 14 * scale, 6 * scale, 3, -0.8, 1);
    drawTo(bx + 1 * scale, by + 0 * scale);
    drawTo(bx + 12 * scale, by + 0 * scale);
    lift(LIFT1);
    break;
  case 3:
    drawTo(bx + 2 * scale, by + 17 * scale);
    lift(LIFT0);
    bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 3, -2, 1);
    bogenUZS(bx + 5 * scale, by + 5 * scale, 5 * scale, 1.57, -3, 1);
    lift(LIFT1);
    break;
  case 4:
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(LIFT0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 6 * scale);
    drawTo(bx + 12 * scale, by + 6 * scale);
    lift(LIFT1);
    break;
  case 5:
    drawTo(bx + 2 * scale, by + 5 * scale);
    lift(LIFT0);
    bogenGZS(bx + 5 * scale, by + 6 * scale, 6 * scale, -2.5, 2, 1);
    drawTo(bx + 5 * scale, by + 20 * scale);
    drawTo(bx + 12 * scale, by + 20 * scale);
    lift(LIFT1);
    break;
  case 6:
    drawTo(bx + 2 * scale, by + 10 * scale);
    lift(LIFT0);
    bogenUZS(bx + 7 * scale, by + 6 * scale, 6 * scale, 2, -4.4, 1);
    drawTo(bx + 11 * scale, by + 20 * scale);
    lift(LIFT1);
    break;
  case 7:
    drawTo(bx + 2 * scale, by + 20 * scale);
    lift(LIFT0);
    drawTo(bx + 12 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 0);
    lift(LIFT1);
    break;
  case 8:
    drawTo(bx + 5 * scale, by + 10 * scale);
    lift(LIFT0);
    bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 4.7, -1.6, 1);
    bogenGZS(bx + 5 * scale, by + 5 * scale, 5 * scale, -4.7, 2, 1);
    lift(LIFT1);
    break;

  case 9:
    drawTo(bx + 9 * scale, by + 11 * scale);
    lift(LIFT0);
    bogenUZS(bx + 7 * scale, by + 15 * scale, 5 * scale, 4, -0.5, 1);
    drawTo(bx + 5 * scale, by + 0);
    lift(LIFT1);
    break;

  case 111:

    lift(LIFT0);
    drawTo(70, 46);
    drawTo(65, 43);

    drawTo(65, 49);
    drawTo(5, 49);
    drawTo(5, 45);
    drawTo(65, 45);
    drawTo(65, 40);

    drawTo(5, 40);
    drawTo(5, 35);
    drawTo(65, 35);
    drawTo(65, 30);

    drawTo(5, 30);
    drawTo(5, 25);
    drawTo(65, 25);
    drawTo(65, 20);

    drawTo(5, 20);
    drawTo(60, 44);

    drawTo(75.2, 47);
    lift(LIFT2);

    break;

  case 11:
    drawTo(bx + 5 * scale, by + 15 * scale);
    lift(LIFT0);
    bogenGZS(bx + 5 * scale, by + 15 * scale, 0.1 * scale, 1, -1, 1);
    delay(PATH_WRITE_DELAY << 5);
    lift(LIFT1);
    drawTo(bx + 5 * scale, by + 5 * scale);
    lift(LIFT0);
    bogenGZS(bx + 5 * scale, by + 5 * scale, 0.1 * scale, 1, -1, 1);
    delay(PATH_WRITE_DELAY << 5);
    lift(LIFT1);
    break;

  }
}



void lift(uint16_t lift_amount) {
#ifndef GLOWINDARK
    if (servoLift >= lift_amount) {
      while (servoLift >= lift_amount)
      {
        servoLift--;
        servo_lift.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }
    else {
      while (servoLift <= lift_amount) {
        servoLift++;
        servo_lift.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }
    delay(PATH_WRITE_DELAY);  //ease the transition
#else

    delay(PATH_WRITE_DELAY << 3);  //ease the transition
    servoLift = lift_amount;
    digitalWrite(SERVOPINLIFT, lift_amount == LIFT0);
#endif
}


void bogenUZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = -0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
    radius * sin(start + count) + by);
    count += inkr;
  }
  while ((start + count) > ende);

}

void bogenGZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = 0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
    radius * sin(start + count) + by);
    count += inkr;
  }
  while ((start + count) <= ende);
}

double return_angle(double a, double b, double c) {
  // cosine rule for angle between c and a
  return acos((a * a + c * c - b * b) / (2 * a * c));
}

void set_XY(double Tx, double Ty)
{
  double dx, dy, c, a1, a2, Hx, Hy;

  // calculate triangle between pen, servoLeft and arm joint
  // cartesian dx/dy
  dx = Tx - O1X;
  dy = Ty - O1Y;

  // polar lemgth (c) and angle (a1)
  c = sqrt(dx * dx + dy * dy); //
  a1 = atan2(dy, dx); //
  a2 = return_angle(L1, L2, c);

  servo_left.writeMicroseconds(floor(((a2 + a1 - M_PI) * SERVOFAKTORLEFT) + SERVOLEFTNULL));

  // calculate joinr arm point for triangle of the right servo arm
  a2 = return_angle(L2, L1, c);
  Hx = Tx + L3 * cos((a1 - a2 + 0.621) + M_PI); //36,5°
  Hy = Ty + L3 * sin((a1 - a2 + 0.621) + M_PI);

  // calculate triangle between pen joint, servoRight and arm joint
  dx = Hx - O2X;
  dy = Hy - O2Y;

  c = sqrt(dx * dx + dy * dy);
  a1 = atan2(dy, dx);
  a2 = return_angle(L1, (L2 - L3), c);

  servo_right.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTORRIGHT) + SERVORIGHTNULL));

}

void drawTo(double pX, double pY) {
  double dx, dy, c;
  int i;
  uint16_t delay_per_step = PATH_WRITE_DELAY*1000;

  // dx dy of new point
  dx = pX - lastX;
  dy = pY - lastY;
  //path lenght in mm, times 4 equals 4 steps per mm
  c = floor(4 * sqrt(dx * dx + dy * dy));

  if (c < 1) {
      delay_per_step >>= 2;
      c = 1;
  }

  for (i = 0; i <= c; i++) {
    // draw line point by point
    set_XY(lastX + (i * dx / c), lastY + (i * dy / c));
    if(servoLift == LIFT0)
        delayMicroseconds(delay_per_step);
  }

  lastX = pX;
  lastY = pY;
}
