/* Include the library */
#include <HCMotor.h>
#include <IRremote.h>
#include <elapsedMillis.h>

//Define variables
int x = 0;
int y = 0;
int dir;
int Speed;


int RPWM_Output = 5; // Arduino PWM output pin 5; connect to IBT-2 pin 1 (RPWM)
int LPWM_Output = 6; // Arduino PWM output pin 6; connect to IBT-2 pin 2 (LPWM)

/* Pins used for Stepper Motor */
#define DIR_PIN 8 //Connect to drive modules 'direction' input.
#define CLK_PIN 9 //Connect to drive modules 'step' or 'CLK' input.

//Set up timing variables
#define speed_adjust_t 4
#define speed_slow_t 2

elapsedMillis forward_time;
elapsedMillis backward_time;
elapsedMillis neutral_time;
elapsedMillis print_time;


const int buttonPinRight = 3; 
int buttonStateRight = 0;

const int buttonPinLeft = 4; 
int buttonStateLeft = 0;

const int buttonPinF = 10; /*pink wire*/
int buttonStateF = 0;

const int buttonPinB = 11; /*yellow wire*/
int buttonStateB = 0;

const int buttonPinL = 12; /*orange wire*/
int buttonStateL = 0;

const int buttonPinR = 7; /*green wire*/
int buttonStateR = 0;

//Set IR remote pin and LED pin
int RECV_PIN = 2;
int LED = 13;

//Set machine state for remote switch
bool machine = true;

//IR remote code
long off1  = 0x00FFA25D;
long on1 = 0x00FF22DD;
long off2 = 0x00FF629D;
long on2 = 0x00FF02FD;
long off3 = 0x00FFE21D;
long on3 = 0x00FFC23D;

IRrecv irrecv(RECV_PIN);
decode_results results;

HCMotor HCMotor;

void setup()
{
  pinMode(RECV_PIN, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  pinMode(RPWM_Output, OUTPUT);
  pinMode(LPWM_Output, OUTPUT);
  
  /* Initialise the library */
  HCMotor.Init();
 
  /* Attach motor 0 to digital pins 8 & 9. The first parameter specifies the 
     motor number, the second is the motor type, and the third and forth are the 
     digital pins that will control the motor */
  HCMotor.attach(0, STEPPER, CLK_PIN, DIR_PIN);
 
  /* Set the number of steps to continuous so the the motor is always turning whilst 
     not int he dead zone*/
  HCMotor.Steps(0,CONTINUOUS);

  pinMode (buttonPinLeft,INPUT);
  pinMode (buttonPinRight,INPUT);
  pinMode (buttonPinF,INPUT);
  pinMode (buttonPinL,INPUT);
  pinMode (buttonPinR,INPUT);
  pinMode (buttonPinB,INPUT);

  irrecv.enableIRIn();

}
 
void loop()
{   
  //Condition for remote shutoff switch
  if (irrecv.decode(&results)) 
   {
    
    if (results.value == on1 or results.value == on2 or results.value == on3 ){
      machine = true;
    }
    else if (results.value == off1 or results.value == off2 or results.value == off3 ){
      machine = false;
    }     
    irrecv.resume(); // Receive the next value
  }

//If machine is enable, run
if (machine == true)
  {digitalWrite(LED,LOW);
  buttonStateLeft = digitalRead(buttonPinLeft);
  buttonStateRight = digitalRead(buttonPinRight);
  buttonStateF = digitalRead(buttonPinF);
  buttonStateB = digitalRead(buttonPinB);
  buttonStateL = digitalRead(buttonPinL);
  buttonStateR = digitalRead(buttonPinR);
  
  //Foward, speed of drive motor is controlled between 0 to 255
  if (buttonStateF == HIGH){
    dir = 1;
    if (forward_time > speed_adjust_t){
      forward_time = 0;
      
      x++;
      if (x>255){
        x = 255;
      }
    }
  }

  //Backward
  if (buttonStateB == HIGH){
    dir = 0;
    if (backward_time > speed_adjust_t){
      backward_time = 0;
      
      y++;
      if (y>255){
        y = 255;
      }
    }
  }

  //Neutral state
  if (buttonStateF == LOW and buttonStateB == LOW){
    if (neutral_time > speed_slow_t){
      neutral_time = 0;
      x= x-2;
      y= y -2;
      if (x <0){
        x = 0;
      }
      if (y<0){
        y = 0;
      }
      }
  }

  //Make sure the device is not running before switch direction
  if (x != 0 and buttonStateB ==HIGH){
    dir = 1;
    if (backward_time > speed_slow_t){
      backward_time = 0;
      x--;
      if (x<0){
        x=0;
      }
    }
  }

   if (y != 0 and buttonStateF ==HIGH){
    dir = 0;
    if (forward_time > speed_slow_t){
      forward_time = 0;
      y--;
      if (y<0){
        y=0;
      }
    }
  }

  //Direction to go forward
  if (dir == 1){
    analogWrite(RPWM_Output,x); 
  }

  //Direction to go backward
  if (dir == 0){
    analogWrite(LPWM_Output,y);
  }

  //Steering Left
    if (buttonStateL == HIGH)
  {
    HCMotor.Direction(0,FORWARD);
    Speed = 20; // The smaller the speed, the faster the stepper motor steers
  }
  
  //Steering Right
  else if (buttonStateR == HIGH)
  {
    HCMotor.Direction(0,REVERSE);
    Speed = 20;
  }

  //Steering neutral
  else if (buttonStateR == LOW and buttonStateL == LOW)
  {
    Speed = 0;
    
  }

  //This is for steering back to neutral position
  if (buttonStateL == HIGH && buttonStateLeft == HIGH)
  { HCMotor.Direction(0,FORWARD);
    Speed = 0;}
  if (buttonStateL == LOW && buttonStateLeft == HIGH)
  { HCMotor.Direction(0,REVERSE);
    Speed = 10;
    delay(280);
  }
    
   if (buttonStateR == HIGH && buttonStateRight == HIGH)
  { HCMotor.Direction(0,REVERSE);
    Speed = 0;
  }
  if (buttonStateR == LOW && buttonStateRight == HIGH)
  { HCMotor.Direction(0,FORWARD);
    Speed = 10;
    delay(250);
  }
HCMotor.DutyCycle(0, Speed);
}

//If machine is disable, turn LED on and do nothing
if (machine == false){
  digitalWrite(LED,HIGH);
  analogWrite(RPWM_Output,0);
  analogWrite(LPWM_Output,0);
  HCMotor.DutyCycle(0, 0);
  x=0;
  y=0;
}
}
