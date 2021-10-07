/*
 * Dan Levy, September 2021.
 *
 * WIS lab tape drive control code.
 * 
 * This program controls the two stepper motors 
 * and synchronizes them so that the tension in the tape is maintained constant.
 *
 */
 
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"
#include <Encoder.h>
#include <FastPID.h>

// The pins for the optical encoder
Encoder myEnc(7,6);
// Encoder counter. Total number of counts is about 44000. I take a maximum of 40000.
long counter = 0; 

// Initial tension set point in percent.
unsigned int setpoint_percent = 25;
unsigned int setpoint = 10000;

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 8=full step, 4=half step etc.
#define MICROSTEPS 8

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

float Kp=10, Ki=2, Kd=0, Hz=1;
int output_bits = 16;
bool output_signed = true;
long output;

unsigned long int loop_start_time;

FastPID myPID(Kp, Ki, Kd, Hz, output_bits, output_signed);

void setup() {    
    Serial.begin(9600);
    top_stepper.begin(RPM, MICROSTEPS);
    bottom_stepper.begin(RPM, MICROSTEPS);    
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);

    // Release tension and set counter to zero
    controller.rotate(-10, 10);
    delay(500);
    myEnc.readAndReset();
    
}

void loop() { 

    loop_start_time = millis();

    if (Serial.available()>0) {      
      String cmd = Serial.readString();
      Serial.println(cmd);
    }    
    /*
    // pause and allow the motor to be moved by hand
    //bottom_stepper.disable();    

    // Get user input for tension (encoder position) setpoint
    //if (Serial.available()) {
    //  setpoint_percent = Serial.readString().toInt();
    //  setpoint = setpoint_percent * 40000/100;      
    //}
    
    long feedback = counter;
    float error = setpoint-feedback;
    
    //float feedback_percent = feedback*100/40000;

    /*if (abs(error/setpoint)>0.1) {      
      output = 100*abs(error)/error;
    }
    else {
      output = myPID.step(setpoint, feedback);   
    }*/   
    
    /*
    counter = myEnc.read();
          
    //Serial.print("Set point: ");
    //Serial.println(setpoint);
    //Serial.print("Error: ");
    //Serial.println(error);
    //Serial.print("Feedback: ");
    Serial.println(feedback);
    //Serial.print("Output: ");
    //Serial.println(output*0.05);       
    
    if (counter>30000) {
      Serial.println("End of tape!");
      while (Serial.available() == 0) {
      }
      Serial.readString();
      windTop();      
    }
    else {     
      controller.rotate(-12.0, -12.0);      
      
      Serial.println(feedback);
      
      if (feedback<setpoint*0.9) {
        while (feedback<setpoint) {      
          counter = myEnc.read();
          feedback = counter;
          if (counter > 30000) {
            break;        
          }              
          bottom_stepper.rotate(-3.6);
        }
      }
  
      if (feedback>setpoint*1.1) {
        while (feedback>setpoint*0.9) {          
          counter = myEnc.read();
          feedback = counter;
          if (counter > 30000) {
            break;
          }
          top_stepper.rotate(-3.6);
        }
      }
  
      //delay(100-(millis()-loop_start_time));
      //delay(1000-(millis()-loop_start_time));
      //delay(1000);
      
  
      /*
      
      bottom_stepper.disable();
      
      top_stepper.rotate(15.0);
  
      bottom_stepper.enable();
      bottom_stepper.rotate(15.0);       
    }*/  
}


void windTop() {
    top_stepper.setRPM(120);      
    bottom_stepper.setRPM(120);            
    while (counter < 35000) {
      controller.rotate(3.6, 3.6);      
      counter = myEnc.read();
    }
}
