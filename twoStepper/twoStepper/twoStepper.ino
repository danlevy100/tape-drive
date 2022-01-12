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

int act = 0;

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
    controller.rotate(-10, 10);
    delay(500);    
    myEnc.readAndReset();

    top_stepper.disable();
    bottom_stepper.disable();
}




void step(int direct) {
    
    long feedback = counter;
    float error = setpoint-feedback;
    
    // float feedback_percent = feedback*100/40000;   
    
    counter = myEnc.read();          
   
    Serial.print("Feedback: ");
    Serial.println(feedback);   
    
    /*if (counter>30000) {
      Serial.println("End of tape!");
      while (Serial.available() == 0) {
      }
      Serial.readString();
      wind_up();      
    }*/
       
    controller.rotate(-12.0*direct, -12.0*direct);           
    
    if (feedback<setpoint*0.9) {
      while (feedback<setpoint) {      
        counter = myEnc.read();
        feedback = counter;
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

    if (feedback>setpoint*1.1) {
      while (feedback>setpoint*1) {          
        counter = myEnc.read();
        feedback = counter;
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

void wind_up() {
    top_stepper.setRPM(120);
    bottom_stepper.disable();
    while (counter < 20000) {      
      top_stepper.rotate(3.6);            
      counter = myEnc.read();      
    }
    controller.rotate(-7.2,0.0);
    bottom_stepper.enable();
}

void loop() { 

    loop_start_time = millis();
    
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
      Serial.println(cmd);
      token = strtok(NULL, ":");      
      direct = atoi(token);            
      Serial.println(direct);
      //token = strtok(NULL, ":");
      //int rot_speed = atoi(token);               
      //Serial.println(rot_speed);      
      
      if (cmd.equals("wind_up")) {        
        wind_up();    
      }

      if (cmd.equals("step")) {
        top_stepper.enable();
        bottom_stepper.enable();
        
        // Release tension and set counter to zero
        controller.rotate(-10, 10);
        delay(500);
        myEnc.readAndReset();  
        
        act = 1;
      }
      if (cmd.equals("stop")) {
        top_stepper.disable();
        bottom_stepper.disable();
        act = 0;
      }
    }

    if (act == 1) {
      Serial.println("Rotate 1 Hz");
      step(direct);
      act = 0; 
    }    
    
    //delay(100-(millis()-loop_start_time));
    delay(1000-(millis()-loop_start_time));
    //delay(1000);        
}
