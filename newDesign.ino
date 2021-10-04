#include "Thread.h"

int StepDIRPin = 2;
int StepPin = 4;
int SleepPin = 5;
int StepDIRPin1 = 9;
int StepPin1 = 13;
int SleepPin1 = 12;
#define encoderPinA 7
#define encoderPinB 6


int speedParam = 0; 
int sleepPinDir = 1; // 1=forward (HIGH), 0=reverse (LOW)
int movement = 1;
volatile unsigned long counter =0;


//My simple Thread
Thread motor1Thread = Thread();
Thread motor2Thread = Thread();


/*----------------------------------------------------------------------------*/
/* Get a single character from the host                                       */
/*----------------------------------------------------------------------------*/
static int getb(void)
{  while (!(Serial.available() > 0));  /* Wait for data to arrive             */
   return Serial.read();               /* Return next character               */
}                                      /*  end: getb()                        */
/*----------------------------------------------------------------------------*/
/* Get current date/time and set the system reset time                        */
/*----------------------------------------------------------------------------*/
void getSpeed(void)
{  
  int t;                           /* Current system time                 */
   int c;                              /* Input character                     */
   do                                  /* Until end of message                */
   {  //Serial.println("?T");            /* Send time query to host via USB     */
      t = 0;                           /* Initialize time value               */
   //   while ('T' != getb()) ;          /* Watch for start of time response    */
      while (('0' <= (c = getb())) && (c <= '9')) /* Is this a decimal digit? */
      {  
        t = 10 * t + (c & 0x0F);      /* If so, build time value             */
      }                                /*  end: building time value           */
   }  while (c != '!');                /* Until a valid time is received      */
 //  set_time(ss(t));                    /* Calculate and save reset time       */
 speedParam = t;
 //Serial.println(speedParam);
}                    

// callback for myThread
void motor1Callback(){
 // static bool ledStatus = false;
 // ledStatus = !ledStatus;

 // digitalWrite(ledPin, ledStatus);

       
  Serial.print("COOL! motor1 running on: ");
  Serial.println(micros());
  
    digitalWrite(StepPin, LOW);
     delay(2);

}

void motor2Callback()
{
    digitalWrite(StepPin1, LOW);
    delay(2);
}


void setup() {
  //Declaring motor pin as output
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(StepDIRPin, OUTPUT);
  pinMode(StepPin, OUTPUT);
  pinMode(SleepPin, OUTPUT);
  pinMode(StepDIRPin1, OUTPUT);
  pinMode(StepPin1, OUTPUT);
  pinMode(SleepPin1, OUTPUT);
  Serial.begin (9600);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), isr,CHANGE);

  
  motor1Thread.onRun(motor1Callback);
  motor1Thread.setInterval(4000); 
  
  motor2Thread.onRun(motor2Callback);
  motor2Thread.setInterval(2500);
}

String parseCmd()
{
  String readString;

  while (Serial.available())
  {
    delay(1);
    if (Serial.available() > 0)
    {
      char c = Serial.read();
       
      //if (isDigit(c))
      if ('T' == c) //getb())
      {
       // speedParam = Serial.parseInt();
       // Serial.println(speedParam);
          getSpeed();
      }
      else if (isControl(c))
      {
        break;
      }

      readString += c;
    }
  }

}

void loop() 
{
  
  String Q = parseCmd();
  if (Q == "go")
  {
    sleepPinDir = 1;
    speedParam = 0;
    movement = 1;
  }
  else if (Q == "rev")
  {
    sleepPinDir = 1;
    speedParam = 200;
    movement = -1;
  }
  else if (Q == "stop")
  {
    sleepPinDir = 0;
    speedParam = 0;
    movement = 0;
  }



  //Fading the DC motor
//  analogWrite(motor1_pin,speedParam);
//  digitalWrite(DIR2Pin, HIGH);
  
  //Fading the DC motor
 // analogWrite(motor2_pin,speedParam);
  if (movement == 1)
  {
 //    digitalWrite(DIR1Pin, HIGH);
     digitalWrite(StepDIRPin, HIGH);
     digitalWrite(StepDIRPin1, LOW);
  }
  else if (movement == -1)
  {
 //    digitalWrite(DIR1Pin, HIGH);
     digitalWrite(StepDIRPin, LOW);
     digitalWrite(StepDIRPin1, HIGH);
  }
 

  digitalWrite(SleepPin, sleepPinDir);
  digitalWrite(SleepPin1, sleepPinDir);

  digitalWrite(StepPin, HIGH);
  digitalWrite(StepPin1, HIGH);
  
  if (movement != 0)
  {
     // checks if thread should run
      if(motor1Thread.shouldRun())
        motor1Thread.run();
        
      if(motor2Thread.shouldRun())
        motor2Thread.run();
  }
  Serial.println(counter);//"counter=" + String(counter));
 //delayMicroseconds(2500);//(5100); //(1250);
  delay(2);

}

//Interrupts
void isr(){

    int channel_A = digitalRead(encoderPinA);
    int channel_B = digitalRead(encoderPinB);

   if(channel_A == channel_B){
    counter++;
   }
   else{
    counter--;
   }
}
