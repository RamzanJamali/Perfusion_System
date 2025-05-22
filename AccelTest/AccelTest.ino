/*
 * Using accelerated motion ("linear speed") in nonblocking mode
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#include <Arduino.h>

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
// Target RPM for cruise speed
#define RPM 10
// Acceleration and deceleration values are always in FULL steps / s^2
#define MOTOR_ACCEL 10
#define MOTOR_DECEL 10

// Microstepping mode. If you hardwired it to save pins, set to the same value here.
#define MICROSTEPS 32

#define DIR 8
#define STEP 9
#define ENABLE_PIN 10
#define SLEEP 13 // optional (just delete SLEEP from everywhere if not used)

/*
 * Choose one of the sections below that match your board
 */

//#include "DRV8834.h"
//#define M0 10
//#define M1 11
//DRV8834 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, M0, M1);

// #include "A4988.h"
// #define MS1 10
// #define MS2 11
// #define MS3 12
// A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MS1, MS2, MS3);

 #include "DRV8825.h"
 #define MODE0 10
 #define MODE1 11
 #define MODE2 12
 DRV8825 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MODE0, MODE1, MODE2);

// #include "DRV8880.h"
// #define M0 10
// #define M1 11
// #define TRQ0 6
// #define TRQ1 7
// DRV8880 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, M0, M1, TRQ0, TRQ1);

// #include "BasicStepperDriver.h" // generic
// BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

#define Relay 5

// commands for arduino to operate. The structure is as follows: 1. Stepper motor, 2. Relay,
//String commands[] = {"STOP_MOTOR", "OFF"};
String inputString = "";
const char delimiter = ',';
const int maxCommands = 2;
String Commands[maxCommands] = {"STOP_MOTOR", "OFF"};

bool connected = 1;

void setup() {
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);

    pinMode(Relay, OUTPUT);

    Serial.begin(115200);
    //Serial.setTimeout(500);
    stepper.begin(RPM, MICROSTEPS);
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);
    stepper.enable();
    // set current level (for DRV8880 only). Valid percent values are 25, 50, 75 or 100.
    // stepper.setCurrent(100);

    /*
     * Set LINEAR_SPEED (accelerated) profile.
     */
    stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);

    Serial.println("OK");
    /*
     * Using non-blocking mode to print out the step intervals.
     * We could have just as easily replace everything below this line with 
     * stepper.rotate(360);
     */
     //stepper.startRotate(0.05625);
     //delay(5000);
   
}

void loop() {
   if (Serial.available()) {
    inputString = Serial.readStringUntil('\n'); // Read until newline
    inputString.trim();
    if (inputString == "STATUS"){
      Status();
    }
    else if (inputString != Commands[0]+","+Commands[1]) {
      CommandParser(inputString, Commands);
      Serial.println(Commands[0]+","+Commands[1]);
    }
    else{
    }
   }

    if (Commands[0] == "RUN_CLOCKWISE") {
        //Serial.println("Motor running clockwise.");
        RunClockwise();
        
      } else if (Commands[0] == "RUN_COUNTERCLOCKWISE") {
        //Serial.println("Motor running counterclockwise.");
        RunCounterClockwise();

      } else if (Commands[0] == "STOP_MOTOR"){
         //Serial.println("Motor stopped.");
         StopMotor();
         
      } else {
         //Serial.println("Unknown command.");
      }
   RelayControl(Commands[1]);

   
}


void RunClockwise(){
   digitalWrite(ENABLE_PIN, LOW);
   stepper.rotate(-360);
   //delay(1000);
}

void RunCounterClockwise(){
   digitalWrite(ENABLE_PIN, LOW);
   stepper.rotate(360);
   //delay(1000);
}

void StopMotor(){
   digitalWrite(ENABLE_PIN, HIGH);
}


void RelayControl(String state){
   if (state=="ON"){
      digitalWrite(Relay, HIGH);
      //Serial.println("Relay is turned ON.\n");
   }

   else if (state=="OFF") {
      digitalWrite(Relay, LOW);
      //Serial.println("Relay is turned OFF.\n");
   }

   else {
      //Serial.println("Wrong parameter!\n");
   }
}


void CommandParser(String inputString, String *Commands) {
   int index = 0;

   while (inputString.length() > 0 && index < maxCommands) {
      int delimiterIndex = inputString.indexOf(delimiter);
      if (delimiterIndex == -1) {
        Commands[index++] = inputString;
        break;
      } else {
        Commands[index++] = inputString.substring(0, delimiterIndex);
        inputString = inputString.substring(delimiterIndex + 1);
      }
    }
}


void Status(){
   Serial.println(Commands[0]+","+Commands[1]);
}