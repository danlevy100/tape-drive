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

#define INPUT_SIZE 30
int direct = 1;
bool fast_forward = false;

// The pins for the optical encoder
Encoder myEnc(7,6);
// Encoder counter. Total number of counts is about 44000. I take a maximum of 40000.
long counter = 0; 

// Initial tension set point in percent.
unsigned int setpoint = 20000;

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
    Serial.setTimeout(10);
    top_stepper.begin(RPM, MICROSTEPS);
    bottom_stepper.begin(RPM, MICROSTEPS);    
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);

    // Release tension and set counter to zero    
    release_tension();

    top_stepper.disable();
    bottom_stepper.disable();
}

void single_step(int direct) {
    
    top_stepper.setRPM(60);
    bottom_stepper.setRPM(60);      
    
    counter = myEnc.read();   
    
    /*if (counter>30000) {
      Serial.println("End of tape!");
      while (Serial.available() == 0) {
      }
      Serial.readString();
      wind_up();      
    }*/
       
    controller.rotate(-12.0*direct, -12.0*direct);           
    
    if (counter<setpoint*0.9) {
      while (counter<setpoint) {      
        counter = myEnc.read();        
        if (counter > 30000) {
          break;        
        }              
        if (direct == 1) {
          bottom_stepper.rotate(-3.6);         
        } else {
          top_stepper.rotate(3.6);
        }
      }
    }

    if (counter>setpoint*1.1) {
      while (counter>setpoint) {          
        counter = myEnc.read();        
        if (counter > 30000) {
          break;
        }
        if (direct == 1) {
          top_stepper.rotate(-1.2);                    
        } else {
          bottom_stepper.rotate(1.2);
        }
        
      }
    }    
  
}

void ff_step(int direct) {
        
    if (direct == -1) {     
      top_stepper.enable();
      bottom_stepper.disable();    
      top_stepper.rotate(3.6);            
    
    } else if (direct == 1) {         
      bottom_stepper.enable();      
      top_stepper.disable();    
      bottom_stepper.rotate(-3.6);    
    }    
}

void release_tension() {
  top_stepper.enable();
  bottom_stepper.enable();

  long counter_before = myEnc.read();
  long counter_after = counter_before + 1;

  while (counter_before != counter_after) { 
    counter_before = myEnc.read();
    controller.rotate(-1, 1);   
    counter_after = myEnc.read();    
  } 
  
  //controller.rotate(-30, 30);
  top_stepper.disable();
  bottom_stepper.disable();
  myEnc.readAndReset();  
}

void loop() {
    
    // Get user input, format is command:direction
    
    counter = myEnc.read();
    Serial.println(counter);   
    
    if (Serial.available()) {      
      //String cmd = Serial.readStringUntil(':'); // Format is command:direction:speed:    
      //String direct = Serial.readStringUntil(':');   
          
      //Serial.println(cmd);
      //Serial.println(direct);     

      // Get next command from Serial (add 1 for final 0)
      char input[INPUT_SIZE + 1];
      byte size = Serial.readBytes(input, INPUT_SIZE);
      // Add the final 0 to end the C string
      input[size] = 0;
      
      // Read command
      char* token = strtok(input, ":");     
      String cmd = token;
      //Serial.println(cmd);
      token = strtok(NULL, ":");      
      direct = atoi(token);            
      //Serial.println(direct);
      //token = strtok(NULL, ":");
      //int rot_speed = atoi(token);               
      //Serial.println(rot_speed);     

      // Deal with command
      
      if (cmd == "ff_step") {
        fast_forward = true;
        top_stepper.setRPM(120);        
        bottom_stepper.setRPM(120);        
      }

      else if (cmd == "single_step") {
        top_stepper.enable();
        bottom_stepper.enable();       
        
        // Release tension and set counter to zero
        //controller.rotate(-5, 5);
        // delay(50);
        //myEnc.readAndReset();  
        single_step(direct);

        fast_forward = false;        
      }
      else if (cmd == "stop") {
        top_stepper.disable();
        bottom_stepper.disable();
        fast_forward = false;        
      }
      else if (cmd == "release_tension") {
        release_tension();
      }
    }

    else if (fast_forward) {
      ff_step(direct);      
    }
      
}
