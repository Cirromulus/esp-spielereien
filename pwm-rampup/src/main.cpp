#include <Arduino.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

static constexpr unsigned potpin = PA0;  // analog pin used to connect the potentiometer
static constexpr uint8_t  ANALOG_IN_RESOLUTION = 12;    // bits
static constexpr uint16_t ANALOG_MAX_VALUE = (1 << ANALOG_IN_RESOLUTION) - 1;
static constexpr uint16_t SERVO_MIN_MS = 900;
static constexpr uint16_t SERVO_MAX_MS = 2000;
static constexpr uint16_t STEP_SIZE_MS = 2;
static constexpr uint16_t STEPS_PER_S = 20;
uint16_t val_set_ms, val_target_ms;    // variable to read the value from the analog pin

void setup() {
    analogReadResolution(ANALOG_IN_RESOLUTION);
    val_set_ms = 900;
    myservo.attach(PB0);  // attaches the servo on pin 9 to the servo object
    myservo.writeMicroseconds(val_set_ms);
    delay(1000);    //wait for the ESC/Servo to power up
}

void loop() {
    val_target_ms = map(analogRead(potpin), 0, ANALOG_MAX_VALUE,
                        SERVO_MIN_MS, SERVO_MAX_MS);

    // low pass for ramp-up
    if(val_target_ms > val_set_ms) {
        // Cap the increase
        val_set_ms += min(STEP_SIZE_MS, static_cast<uint16_t>(val_target_ms - val_set_ms));
        delay(1000/STEPS_PER_S);                        // waits for the servo to get there
    }
    else
        val_set_ms = val_target_ms;

    myservo.writeMicroseconds(val_set_ms);          // sets the servo position according to the scaled value
}
