/*
 * Dan Levy, September 2021.
 *
 * WIS lab tape drive control code.
 * 
 * This program controls the two stepper motors 
 * and synchronizes them so that the tension in the tape is maintained constant.
 *
 */


/**************** MAX31855K_Thermocouple_Digitizer_Example.ino *****************
 *                                                                             *
 * MAX31855K Thermocouple Breakout Example Code                                *
 * brent@sparkfun.com                                                          *
 * March 26th 2015                                                             *
 * https://github.com/sparkfun/MAX31855K_Thermocouple_Digitizer                *
 *                                                                             *
 * Use the "serial monitor" window to read a temperature sensor.               *
 *                                                                             *
 * Circuit:                                                                    *
 * MAX31855K breakout attached to the following pins                           *
 *  SS:   pin 10                                                               *
 *  MOSI: pin 11 (NC)                                                          *
 *  MISO: pin 12                                                               *
 *  SCK:  pin 13                                                               *
 *  VCC:  pin 14                                                               *
 *  GND:  pin 15                                                               *
 *                                                                             *
 *                                                                             *
 * Development environment specifics:                                          *
 * 1.6.4                                                                       *
 * Arduino Pro Mini 328 3.3V/8MHz                                              *
 *                                                                             *
 * This code is beerware; if you see me (or any other SparkFun employee) at    *
 * the local, and you've found our code helpful, please buy us a round!        *
 * Distributed as-is; no warranty is given.                                    *
 ******************************************************************************/

#include <SparkFunMAX31855k.h> // Using the max31855k driver
#include <SPI.h>  // Included here too due Arduino IDE; Used in above header

const uint8_t CHIP_SELECT_PIN = 0; // Using standard CS line (SS)
const uint8_t CHIP_SELECT1_PIN = 1; // Using standard CS line (SS)
// SCK & MISO are defined by Arduiino

// Instantiate an instance of the SparkFunMAX31855k class.  There are 4
// parameters, but only the first is required. The parameters are (in order):
// chip select pin, VCC pin, 0V pin, debug flag (bool).
// Here are some examples. Only use 1 at a time unless you have more than one
// sensor.


// This version relies on power and doesn't print debug messages
SparkFunMAX31855k probe(CHIP_SELECT_PIN); // Motor 0
SparkFunMAX31855k probe1(CHIP_SELECT1_PIN); // Motor 1

// This example doesn't use IO to power the IC, but has serial debug enabled
//SparkFunMAX31855k probe(CHIP_SELECT_PIN, NONE, NONE, true);

// These define's must be placed at the beginning before #include "TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#define USE_TIMER_1     true

#if ( defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)  || \
        defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MINI) ||    defined(ARDUINO_AVR_ETHERNET) || \
        defined(ARDUINO_AVR_FIO) || defined(ARDUINO_AVR_BT)   || defined(ARDUINO_AVR_LILYPAD) || defined(ARDUINO_AVR_PRO)      || \
        defined(ARDUINO_AVR_NG) || defined(ARDUINO_AVR_UNO_WIFI_DEV_ED) || defined(ARDUINO_AVR_DUEMILANOVE) || defined(ARDUINO_AVR_FEATHER328P) || \
        defined(ARDUINO_AVR_METRO) || defined(ARDUINO_AVR_PROTRINKET5) || defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_AVR_PROTRINKET5FTDI) || \
        defined(ARDUINO_AVR_PROTRINKET3FTDI) )
  #define USE_TIMER_2     true
  #warning Using Timer1
#else          
  #define USE_TIMER_3     true
  #warning Using Timer3
#endif

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "TimerInterrupt.h"

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN     13
#endif

unsigned int outputPin1 = LED_BUILTIN;
unsigned int outputPin  = A0;

#define TIMER1_INTERVAL_MS    10

/* End of timer interrupt definitions */

#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"
#include <Encoder.h>

#define INPUT_SIZE 30
int direct = 1;
bool fast_forward = false;
bool EOT = false; 

// The pins for the optical encoder
Encoder myEnc(7,6);
// Encoder encoder_reading. Total number of counts is about 44000. I take a maximum of 40000.
//long encoder_reading = 0; 

// Initial tension set point in percent.
//unsigned int encoder_setpoint;

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

//volatile float temperature;

void TimerHandler1(unsigned int outputPin = LED_BUILTIN)
{
  // Read the temperature in Celsius
  float temperature = probe.readTempC();
  //float temperature1 = probe1.readTempC();

  long encoder_reading = myEnc.read();
  String transmit_response = ""; 
  transmit_response = transmit_response + encoder_reading + "," + EOT + "," + String(temperature);
  Serial.println(transmit_response);    
    
  
  /*if (!isnan(temperature)) {
    Serial.print("Temp[C]=");
    Serial.println(temperature);
    Serial.print("Temp1[C]=");
    Serial.println(temperature1);
  }*/
}


void setup() {
  Serial.begin(9600);
  Serial.println("\nBeginning...");
  delay(50);  // Let IC stabilize or first readings will be garbage

  ITimer1.init();

  // Using ATmega328 used in UNO => 16MHz CPU clock ,
  // For 16-bit timer 1, 3, 4 and 5, set frequency from 0.2385 to some KHz
  // For 8-bit timer 2 (prescaler up to 1024, set frequency from 61.5Hz to some KHz

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1, outputPin1))
  {
    Serial.print(F("Starting  ITimer1 OK, millis() = ")); Serial.println(millis());

#if (TIMER_INTERRUPT_DEBUG > 1)    
    Serial.print(F("OutputPin1 = ")); Serial.print(outputPin1);
    Serial.print(F(" address: ")); Serial.println((uint32_t) &outputPin1 );
#endif    
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

  Serial.setTimeout(10);
    top_stepper.begin(RPM, MICROSTEPS);
    bottom_stepper.begin(RPM, MICROSTEPS);    
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);

    // Release tension and set encoder_reading to zero    
    release_tension();

    top_stepper.disable();
    bottom_stepper.disable();

}

void single_step(int direct, long encoder_reading, unsigned int encoder_setpoint) {    
    
    top_stepper.setRPM(120);
    bottom_stepper.setRPM(120);

    top_stepper.enable();
    bottom_stepper.enable(); 

    // make step
    controller.rotate(-12.0*direct, 12.0*direct);   

    // correct for tension
    int i=0;
    while (encoder_reading<encoder_setpoint*0.95 || encoder_reading>encoder_setpoint*1.05) {
      if (encoder_reading<encoder_setpoint*0.95) {      
        if (direct == 1) {
          bottom_stepper.rotate(0.6);         
        } else {
          top_stepper.rotate(0.6);
        }
        if (myEnc.read() <= encoder_reading) {
          i = i+1; // Could not increase tension at this step. When i>=5, break loop.
        }
      }    
                  
      if (encoder_reading>encoder_setpoint*1.05) {                  
        if (direct == 1) {
          top_stepper.rotate(-0.6);                    
        } else {
          bottom_stepper.rotate(-0.6);          
        }             
        if (myEnc.read() >= encoder_reading) {
          i = i+1; // Could not release tension at this step. When i>=5, break loop.
        }     
      }      
      encoder_reading = myEnc.read();     
      
      if (i>=100) {
        // Error! Cannot set tension properly. Perhaps the motors are not working, tape is torn or end of tape?
        break;
      }
    }      
    EOT = false;    
}

void ff_step(int direct) {
  
    top_stepper.setRPM(120);        
    bottom_stepper.setRPM(120);        
        
    if (direct == -1) {     
      top_stepper.enable();
      bottom_stepper.disable();    
      top_stepper.rotate(3.6);
      if (myEnc.read() > 25000) {
        bottom_stepper.enable();
        bottom_stepper.rotate(30);
        bottom_stepper.disable();
      }
    
    } else if (direct == 1) {         
      bottom_stepper.enable();      
      top_stepper.disable();    
      bottom_stepper.rotate(3.6);
      if (myEnc.read() > 25000) {
        top_stepper.enable();
        top_stepper.rotate(30);
        top_stepper.disable();
      }
    }
}

void release_tension() {
  top_stepper.enable();
  bottom_stepper.enable();

  long encoder_reading_before = myEnc.read();
  long encoder_reading_after = encoder_reading_before + 1;

  while (encoder_reading_before != encoder_reading_after) { 
    encoder_reading_before = myEnc.read();
    controller.rotate(-1, 1);   
    encoder_reading_after = myEnc.read();    
  }   
  
  top_stepper.disable();
  bottom_stepper.disable();
  myEnc.readAndReset();  
}

void loop() {
    
    // Get user input, format is command:direction
    
    long encoder_reading = myEnc.read();
    /*String transmit_response = ""; 
    transmit_response = transmit_response + encoder_reading + "," + EOT + "," + String(temperature);
    Serial.println(transmit_response);    */
    
    if (Serial.available()) {      
      //String cmd = Serial.readStringUntil(':'); // Format is command:direction:tension:    
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
      token = strtok(NULL, ":");      
      direct = atoi(token);            
      token = strtok(NULL, ":");
      long encoder_setpoint = atoi(token);
      encoder_setpoint = encoder_setpoint * 40000 / 100; // Convert from percent to real encoder values              

      // Deal with command
      
      if (cmd == "ff_step") {
        fast_forward = true;        
      }

      else if (cmd == "single_step") { 
        
        single_step(direct, encoder_reading, encoder_setpoint);
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

    else if (fast_forward && !EOT) {      
      if ( ((encoder_reading > 35000) and (direct==1)) or ((encoder_reading > 25000) and (direct==-1)) ) {
        EOT = true;
      } else {
        ff_step(direct);
      }
    }
      
}
