// Reference the SPI Library
#include <SPI.h>
// Reference the I2C Library
#include <Wire.h>
// Reference the HMC5883L Compass Library
#include <HMC5883L.h>

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// ### -- 1.8 SPI TFT --
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
// http://learn.adafruit.com/adafruit-gfx-graphics-library/overview

//#define sclk 13   // SCL
//#define mosi 11   // SDA
#define cs   7     // CS 10
#define dc   9      // DC 9
#define rst  8      // RES 8


Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
HMC5883L compass;

int TFTrotation = 3;
int MBisOK = 0;
int RFisOK = 0;
int CompassisOK = 0;

int error = 0;
int error1 = 0;

int speedLeft;
int speedRight;
int direc;
int compassValue;

#include <JeeLib.h> // https://github.com/jcw/jeelib


// Fixed RF12 settings
#define MYNODE 10 //node ID of the receiever
#define freq RF12_433MHZ //frequency
#define group 210 //network group

#define USE_ACK           // Enable ACKs, comment out to disable
#define RETRY_PERIOD 5    // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT 5     // Maximum number of times to retry
#define ACK_TIME 10       // Number of milliseconds to wait for an ack

MilliTimer sendTimer;
byte needToSend;

 typedef struct {
          int rx1; // sensor value
          int rx2; // sensor value
          int rx3; // tx voltage
          int rx4; // tx voltage
 } rxPayload;
 rxPayload rx;
 
  typedef struct {
          int tx1; // sensor value
          int tx2; // sensor value
          int tx3; // tx voltage
          int tx4; // tx voltage
 } txPayload;
 txPayload tx;
 
 // Wait a few milliseconds for proper ACK
 #ifdef USE_ACK
  static byte waitForAck() {
   MilliTimer ackTimer;
   while (!ackTimer.poll(ACK_TIME)) {
     if (rf12_recvDone() && rf12_crc == 0 &&
        rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | MYNODE))
        return 1;
     }
   return 0;
  }
 #endif
 
 int nodeID; //node ID of tx, extracted from RF datapacket. Not transmitted as part of structure

// ### -- Serial variables --
unsigned long serialdata;
unsigned long data;
int inbyte;
int answer;

void setup()
{
  Serial.begin(9600);
  tftInit();
  compassInit();
  
  robot_Welcome();

  rf12_initialize(MYNODE,freq,group); // Initialize RFM12 with settings defined above 
  // Adjust low battery voltage to 2.2V UNTESTED!!!!!!!!!!!!!!!!!!!!!
  rf12_control(0xC040);
 while(1){
  if (MBisOK == 0){
    robot_MBTest();
  }
  if (MBisOK == 1 && RFisOK == 0){
    robot_RFTest();
  }
  if (RFisOK == 1 && CompassisOK == 0){
    robot_CompassTest();
    break;
  }
  delay(500);
 }
 delay(1000);
tft_setBackground();
}

void loop()
{
 compassValue = compassRead();
 drawCompass(compassValue);
 pointTo(180, 255);
  tx.tx1 = 1;
  tx.tx2 = compassValue;
  tx.tx3 = speedLeft;
  tx.tx4 = speedRight;
  rf12_sendStart(RF12_HDR_ACK, &tx, sizeof tx); 
 delay(10);
}

void tftInit(){
  tft.initR(INITR_BLACKTAB); 
  tft.fillScreen(BLACK);
  tft.setRotation(TFTrotation);
}

void compassInit(){
  Wire.begin();
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  compass.SetScale(1.3); // Set the scale of the compass.
  compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
}

void tft_setBackground() {
  tft.fillScreen(BLACK);
  tft.fillRect(tft.width()/8-14, tft.height()/8-14, tft.width()*3/4+28, tft.height()*3/4+28, GREEN);
  tft.fillRect(tft.width()/8-12, tft.height()/8-12, tft.width()*3/4+24, tft.height()*3/4+24, BLACK);
}

void drawCompassBase(){
	tft.drawCircle(tft.width()/2,tft.height()/2,50,YELLOW);
	tft.drawLine(tft.width()/2,30,tft.width()/2,20,GREEN);
}
void drawCompassDire(int16_t dire){
	static uint8_t x_old;
	static uint8_t y_old;
	static uint8_t x_t_old;
	static uint8_t y_t_old;

	uint8_t x=60*sin(dire/360.0*6.28)+tft.width()/2;
	uint8_t x_t=40*sin(dire/360.0*6.28)+tft.width()/2;
	uint8_t y=60*cos(dire/360.0*6.28)+tft.height()/2;
	uint8_t y_t=40*cos(dire/360.0*6.28)+tft.height()/2;

	//tft.drawLine(x_t_old,y_t_old,x_old,y_old,BLUE);
        tft.drawLine(x_t_old,y_t_old,x_old,y_old,BLACK);
	tft.drawLine(x_t,y_t,x,y,GREEN);

        tft.fillRect(tft.width()/2-20, tft.height()/2-20, 40, 20, BLACK);
        tft.setTextSize(2);
        tft.setTextColor(GREEN);
        tft.setCursor(tft.width()/2-20, tft.height()/2-20);
        tft.print(dire);

	x_old=x;
	y_old=y;
	x_t_old=x_t;
	y_t_old=y_t;
}

void drawCompass(uint16_t value){
	drawCompassBase();
	drawCompassDire(value);
}

void robot_Welcome(){
  tft_setBackground();
   
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.setCursor(20, 20);
  tft.print("MARVIN-ino");
  tft.setCursor(20, 30);
  tft.print("__________");
  tft.setTextSize(1);
  tft.setCursor(40, 60);
  tft.print("Skynet dans");
  tft.setCursor(40, 80);
  tft.print("ta maison !");
  delay(2000);
  tft.fillRect(tft.width()/8-10, tft.height()/8-10, tft.width()*3/4+20, tft.height()*3/4+20, BLACK);
}

void robot_MBTest(){
  tft.setTextColor(WHITE);
  tft.setCursor(10, 10);
  tft.print("# Liaison Motors Board : ");
  Serial.print("1/");
  delay(100);
  answer = getSerial();
  switch(answer)
  {
  case 42:
    {
      tft.fillRect(10, 20, tft.width()/2, 10, BLACK);
      tft.setCursor(10, 20);
      tft.setTextColor(GREEN);
      tft.print("OK.");
      MBisOK = 1;
      break;
    }
  }
}

void robot_CompassTest(){
  tft.setTextColor(WHITE);
  tft.setCursor(10, 50);
  tft.print("# Boussole : ");
  
  
  int heading = compassRead();
  if(heading >= 0){
    tft.fillRect(10, 60, tft.width()/2, 10, BLACK);
    tft.setCursor(10, 60);
    tft.setTextColor(GREEN);
    tft.print("OK.");
    CompassisOK = 1;
  }
}

void robot_RFTest(){
  tft.setTextColor(WHITE);
  tft.setCursor(10, 30);
  tft.print("# Liaison Base Station : ");
  
  tft.fillRect(10, 40, tft.width()/2, 10, BLACK);
  tft.setCursor(10, 40);
  tft.setTextColor(YELLOW);
  tft.print("SENDING...");
        
  tx.tx1 = 42;
  tx.tx2 = 42;
  tx.tx3 = 42;
  tx.tx4 = 42;
  //delay(250);
  if (sendTimer.poll(100))
    needToSend = 1;
  if (needToSend && rf12_canSend()) {
    needToSend = 0;

    rfwrite(); // Send data via RF 
    
  }
  if (rf12_recvDone() && rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0) {
    nodeID = rf12_hdr & 0x1F; // get node ID
    rx = *(rxPayload*) rf12_data;
    int value1 = rx.rx1;
    int value2 = rx.rx2;
    int value3 = rx.rx3;
    int value4 = rx.rx4;
    int rxsize = sizeof rx;
    if(nodeID == 30){
      if(value1 == 42){
        tft.fillRect(10, 40, tft.width()/2, 10, BLACK);
        tft.setCursor(10, 40);
        tft.setTextColor(GREEN);
        tft.print("OK.");
        RFisOK = 1;
      }
    }
//  Serial.println('received a packet');
  }
}

int compassRead()
{
  // Retrive the raw values from the compass (not scaled).
  MagnetometerRaw raw = compass.ReadRawAxis();
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  
  // Values are accessed like so:
  int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)

  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(scaled.YAxis, scaled.XAxis);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: 2� 37' W, which is 2.617 Degrees, or (which we need) 0.0456752665 radians, I will use 0.0457
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.0457;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  // Normally we would delay the application by 66ms to allow the loop
  // to run at 15Hz (default bandwidth for the HMC5883L).
  // However since we have a long serial out (104ms at 9600) we will let
  // it run at its natural speed.
  // delay(66);
  
  // Output the data.
  return headingDegrees;

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

  if (diff > 0) {
    // keep the right wheel spinning,
    // change the speed of the left wheel
    speedLeft = 255 - diff;
    speedRight = 255;
  } else {
    // keep the right left spinning,
    // change the speed of the left wheel
    speedLeft = 255;
    speedRight = 255 + diff;
  }
  // write out to the motors
  // motorsWrite(2, speedLeft, speedRight); // A FAIRE
  
  tft.fillRect(tft.width()/2-20, tft.height()/2+20, 50, 20, BLACK);
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(tft.width()/2-20, tft.height()/2+20);
  tft.print(diff);
  
  tft.fillRect(10, 10, 40, 20, BLACK);
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(10, 10);
  tft.print(speedLeft);
  
  tft.fillRect(tft.width()-40, 10, 40, 20, BLACK);
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(tft.width()-40, 10);
  tft.print(speedRight);
  
//  Serial.print("2/");Serial.print(speedLeft);Serial.print("/");Serial.print(speedRight);Serial.print("/");
  motorsWrite(speedLeft, speedRight);

}



void pointTo(int angle, int speed){
  int target=angle;
  target=target%360;
  if(target<0){
    target+=360;
    }
  int direction=angle;
  
  int compassValue=compassRead();
  int diff=target-compassValue;
  if(diff<-180) 
    diff += 360;
  else if(diff> 180) 
    diff -= 360;
  direction=-diff;
  
  if(direction>0){
    speedLeft = speed;
    speedRight = -speed;
//    motorsWrite(speedLeft,speedRight);//right
    delay(10);
  }else{
    speedLeft = -speed;
    speedRight = speed;
//    motorsWrite(speedLeft,speedRight);//left
    delay(10);
  }
  
		
  if(abs(diff)<5){
    speedLeft = 0;
    speedRight = 0;
//    motorsWrite(speedLeft,speedRight); // stop
    //return;
  }
  
  motorsWrite(speedLeft,speedRight);//right
  
  tft.fillRect(tft.width()/2-20, tft.height()/2+20, 50, 20, BLACK);
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(tft.width()/2-20, tft.height()/2+20);
  tft.print(diff);
  
  tft.fillRect(15, 10, 30, 20, BLACK);
  tft.setTextSize(1);
  tft.setTextColor(RED);
  tft.setCursor(15, 10);
  tft.print(speedLeft);
  
  tft.fillRect(tft.width()-40, 10, 30, 20, BLACK);
  tft.setTextSize(1);
  tft.setTextColor(RED);
  tft.setCursor(tft.width()-40, 10);
  tft.print(speedRight);
}

void turn(int angle, int speed){
  int originalAngle=compassRead();
  int target=originalAngle+angle;
  pointTo(target, speed);
}

// ### --  MOTORS -- ##
void motorsWrite(int speedLeft,int speedRight){
  Serial.print("2/");Serial.print(speedLeft);Serial.print("/");Serial.print(speedRight);Serial.print("/");
}

void motorsStop(){
  motorsWrite(0,0);
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

//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//-------------------------------------------------------------------------------------------------
 static void rfwrite(){
  #ifdef USE_ACK
   for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
//     rf12_sleep(-1);              // Wake up RF module
      while (!rf12_canSend())
      rf12_recvDone();
      rf12_sendStart(RF12_HDR_ACK, &tx, sizeof tx); 
      rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
      byte acked = waitForAck();  // Wait for ACK
//      rf12_sleep(0);              // Put RF module to sleep
      if (acked) { return; }      // Return if ACK received
  
   Sleepy::loseSomeTime(10);     // If no ack received wait and try again
   }
  #else
//     rf12_sleep(-1);              // Wake up RF module
     while (!rf12_canSend())
     rf12_recvDone();
     rf12_sendStart(0, &tx, sizeof tx); 
     rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
//     rf12_sleep(0);              // Put RF module to sleep
     return;
  #endif
 }
