/*
 * Using accelerated motion ("linear speed") in nonblocking mode
 *
 2020-09-12: Modified for the sealer so that you send a bitmask for which motors
 to move. All motors move at the same speed in unison.

 Can only move positive if the limit switch is not hit.

 Hit the button and it goes down, pauses, then goes up.
 Hit button during seal to retract immediately
*/

#define PAUSETIME 5             /* pause in seconds */
#define DOWNTRAVEL 360          /* how many steps to retract */

#include <Arduino.h>

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
// Target RPM for cruise speed
#define RPM 100
// Acceleration and deceleration values are always in FULL steps / s^2
#define MOTOR_ACCEL 3000
#define MOTOR_DECEL 3000

// Microstepping mode. If you hardwired it to save pins, set to the same value here.
#define MICROSTEPS 16

#define DIR 6
#define STEP 3
#define ENABLE 8

#define STARTPIN 18

/*
 * Choose one of the sections below that match your board
 */

//#include "DRV8834.h"
//#define M0 10
//#define M1 11
//DRV8834 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, M0, M1);

#include "A4988.h"
A4988 stepper(MOTOR_STEPS, DIR, 3, ENABLE);

// #include "DRV8825.h"
// #define MODE0 10
// #define MODE1 11
// #define MODE2 12
// DRV8825 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MODE0, MODE1, MODE2);

// #include "DRV8880.h"
// #define M0 10
// #define M1 11
// #define TRQ0 6
// #define TRQ1 7
// DRV8880 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, M0, M1, TRQ0, TRQ1);

// #include "BasicStepperDriver.h" // generic
// BasicStepperDriver stepper(DIR, STEP);

void setup() {
    Serial.begin(115200);

    pinMode(STARTPIN, INPUT_PULLUP);
    stepper.begin(RPM, MICROSTEPS);
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    
    stepper.setEnableActiveState(LOW);
    // set current level (for DRV8880 only). Valid percent values are 25, 50, 75 or 100.
    // stepper.setCurrent(100);

    /*
     * Set LINEAR_SPEED (accelerated) profile.
     */
    stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);

    Serial.println("START");
    /* startrotate is nonblocking
     * Using non-blocking mode to print out the step intervals.
     * We could have just as easily replace everything below this line with 
     * stepper.rotate(360);
     */
}

void loop() {
  unsigned long end_pause;
  unsigned int start_pin = digitalRead(STARTPIN);
  static unsigned int debounce;

  if(!start_pin){
    debounce++;
    if(debounce > 20){
      debounce = 20;
    }
  }else{
    debounce=0;
  }
  if(debounce==20){
    Serial.print("Start\n");
    stepper.enable();
    stepper.rotate(1000);        /* go home */
    stepper.rotate(-DOWNTRAVEL);       /* move to seal position */
    end_pause = millis()+ PAUSETIME*1000;
    debounce=0;
    while(millis() < end_pause && debounce < 20){
      if(!digitalRead(STARTPIN)){
        debounce++;
      }else{
        debounce=0;
      }
    }
    stepper.rotate(DOWNTRAVEL);        /* go back home */
  }
}
