/*
 * Simple demo, should work with any driver board
 *
 * Connect STEP, DIR as indicated
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"
#include <Encoder.h>
#include <FastPID.h>

// The pins for the optical encoder
Encoder myEnc(7,6);
long counter = 0;

int setpoint = 5000;

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 1500

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=half step, 2=full step etc.
#define MICROSTEPS 2

// top is top motor, bottom is bottom motor.

// All the wires needed for full functionality
#define top_DIR 2
#define top_STEP 4
#define top_SLEEP 5

#define bottom_DIR 9
#define bottom_STEP 13
#define bottom_SLEEP 12

BasicStepperDriver top_stepper(MOTOR_STEPS, top_DIR, top_STEP, top_SLEEP);
BasicStepperDriver bottom_stepper(MOTOR_STEPS, bottom_DIR, bottom_STEP, bottom_SLEEP);

MultiDriver controller(top_stepper, bottom_stepper);

float Kp=0.05, Ki=0.005, Kd=0, Hz=1;
int output_bits = 16;
bool output_signed = true;
long output;

FastPID myPID(Kp, Ki, Kd, Hz, output_bits, output_signed);

void setup() {    
    top_stepper.begin(RPM, MICROSTEPS);
    bottom_stepper.begin(RPM, MICROSTEPS);    
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);
    
}

void loop() { 

    // pause and allow the motor to be moved by hand
    //bottom_stepper.disable();    

    if (Serial.available()) {
      setpoint = Serial.parseInt();
      Serial.readString();      
    }
    
    int feedback = counter/2;
    float error = setpoint-feedback;

    if (abs(error/setpoint)>0.1) {      
      output = abs(error)/error;
    }
    else {
      output = myPID.step(setpoint, feedback) / 100;    
    }   
    
    counter = myEnc.read();
       
    Serial.println(feedback);   
    
    controller.rotate(-7.2+output, -7.2);   
}
