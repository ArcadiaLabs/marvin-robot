#include <Wire.h>

// ### -- Adafruit Motor Shield V2
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
// http://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Select which 'port' M1, M2, M3 or M4.
Adafruit_DCMotor *LeftMotor = AFMS.getMotor(1);
Adafruit_DCMotor *RightMotor = AFMS.getMotor(2);
// You can also make another motor on port M2
//Adafruit_DCMotor *myOtherMotor = AFMS.getMotor(2);

int speedLeft;
int speedRight;

unsigned long serialdata;
unsigned long data;
int inbyte;

int rxLED = 13;

void setup()
{
  Serial.begin(9600);
  motorsInit();

  pinMode(rxLED, OUTPUT); 
  blinkLED(rxLED, 3, 10);
}

void loop()
{

  parseCommand();
  delay(1);
  
}

void motorsInit(){
  AFMS.begin();
}

void motorsWrite(int speedLeft, int speedRight){
  
  LeftMotor->run(RELEASE);
  RightMotor->run(RELEASE);
  
  if (speedLeft > 0){
    LeftMotor->run(FORWARD); 
    LeftMotor->setSpeed(speedLeft);
  }
  else if(speedLeft < 0){
    int speedLeftInt = abs(speedLeft);
    LeftMotor->run(BACKWARD); 
    LeftMotor->setSpeed(speedLeftInt);
  }
  else if(speedLeft = 0){
    LeftMotor->run(RELEASE);
  }
    
  if (speedRight > 0){
    RightMotor->run(FORWARD); 
    RightMotor->setSpeed(speedRight); 
  }
  else if(speedRight < 0){
    int speedRightInt = abs(speedRight);
    RightMotor->run(BACKWARD); 
    RightMotor->setSpeed(speedRightInt);
  }
  else if(speedRight = 0){
    RightMotor->run(RELEASE);
  }
  
  // turn on motor
  //LeftMotor->run(RELEASE);
  //RightMotor->run(RELEASE);
}

void motorWrite(int motor, int motorSpeed){
  if (motor == 1){
    LeftMotor->setSpeed(abs(motorSpeed));
    if (motorSpeed > 0){
      LeftMotor->run(FORWARD);
    }
    else if (motorSpeed < 0){
      LeftMotor->run(BACKWARD);
    }
    else if (motorSpeed == 0){
      LeftMotor->run(RELEASE);
    }
  }
  else if (motor == 2){
    RightMotor->setSpeed(abs(motorSpeed));
    if (motorSpeed > 0){
      RightMotor->run(FORWARD);
    }
    else if (motorSpeed < 0){
      RightMotor->run(BACKWARD);
    }
    else if (motorSpeed == 0){
      RightMotor->run(RELEASE);
    }    
  }
}
void motorsStop(){
  LeftMotor->run(FORWARD);
  RightMotor->run(FORWARD);
  LeftMotor->setSpeed(0);
  RightMotor->setSpeed(0);
}

void motorStop(int motor){
  if (motor == 1){
    LeftMotor->run(FORWARD);
    LeftMotor->setSpeed(0);
  }
  else if (motor == 2){
    RightMotor->run(FORWARD);
    RightMotor->setSpeed(0);
  }
}

void motorsRelease(){
  LeftMotor->setSpeed(0);
  RightMotor->setSpeed(0);
  LeftMotor->run(RELEASE);
  RightMotor->run(RELEASE);
}

void motorRelease(int motor){
  if (motor == 1){
    LeftMotor->setSpeed(0);
    LeftMotor->run(RELEASE);
  }
  else if (motor == 2){
    RightMotor->setSpeed(0);
    RightMotor->run(RELEASE);
  }
}

void moveForward(int speed){
  motorsWrite(speed,speed);
}
void moveBackward(int speed){
  motorsWrite(speed,speed);
}
void turnLeft(int speed){
  motorsWrite(speed,255);
}
void turnRight(int speed){
  motorsWrite(255,speed);
}

void followHeading(uint16_t compassValue, uint16_t value){
  // how many degrees are we off
  int diff = compassValue - value;

  // modify degress
    
  if (diff > 180)
    diff = -360 + diff;
  else if (diff < -180)
    diff = 360 + diff;

  // Make the robot turn to its proper orientation
  diff = map(diff, -180, 180, -255, 255);

  if(abs(diff)<5){
    speedLeft = 0;
    speedRight = 0;
  } else if (diff > 0) {
    // keep the right wheel spinning,
    // change the speed of the left wheel
    speedLeft = 255 - diff;
    speedRight = 255;
  } else if (diff < 0) {
    // keep the right left spinning,
    // change the speed of the left wheel
    speedLeft = 255;
    speedRight = 255 + diff;    
  }  
  motorsWrite(speedLeft, speedRight);
}

void pointTo(int compassValue, int angle){
  int target=angle;
  uint8_t speed=80;
  target=target%360;
  if(target<0){
    target+=360;
    }
  int direction=angle;
  while(1){
    if(direction>0){
      motorsWrite(speed,-speed);//right
    }else{
      motorsWrite(-speed,speed);//left
    }
    int diff=target-compassValue;
    if(diff<-180) 
      diff += 360;
    else if(diff> 180) 
      diff -= 360;
    direction=-diff;
  		
    if(abs(diff)<5){
      motorsWrite(0,0);
    }
  }
}

void turn(int compassValue, int angle){
  int target=compassValue+angle;
  pointTo(compassValue, target);
}

void parseCommand(){
  //getSerial();
  int Command = getSerial();
  switch(Command)
  {
    case 1:{   // identifier : 1 -> 42
      Serial.print("42/");
      blinkLED(rxLED, 3, 10);
      break;
    } 
    case 2:{   // motorsWrite(int speedLeft, int speedRight)
      //getSerial();
      speedLeft = getSerial();
      //getSerial();
      speedRight = getSerial();
      motorsWrite(speedLeft, speedRight);
      blinkLED(rxLED, 1, 10);
      break;
    }
  }
}

//long getSerial()
//{
//  serialdata = 0;
//  data = 0;
//  while (Serial.available()) {
//    while (inbyte != '/')
//    {
//      inbyte = Serial.read(); 
//      // inbyte = Serial.read()-'0';
//      if (inbyte > 0 && inbyte != '/')
//      {
//       
//        data = serialdata * 10 + inbyte - '0';
//      }
//    }
//  }
//  inbyte = 0;
//  serialdata = 0;
//  return data;
//}
long getSerial()
{
  serialdata = 0;
  while (inbyte != '/')
  {
    inbyte = Serial.read(); 
    // inbyte = Serial.read()-'0';
    if (inbyte > 0 && inbyte != '/')
    {
     
      serialdata = serialdata * 10 + inbyte - '0';
    }
  }
  inbyte = 0;
  return serialdata;
}

void blinkLED(byte targetPin, int numBlinks, int blinkRate) {
  for (int i=0; i < numBlinks; i++) {
    digitalWrite(targetPin, HIGH);   // sets the LED on
    delay(blinkRate);                     // waits for blinkRate milliseconds
    digitalWrite(targetPin, LOW);    // sets the LED off
    delay(blinkRate);
  }
}
