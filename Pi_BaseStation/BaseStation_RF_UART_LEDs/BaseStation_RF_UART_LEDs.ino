//----------------------------------------------------------------------------------------------------------------------
// Requires Arduino IDE with arduino-tiny core: http://code.google.com/p/arduino-tiny/
//----------------------------------------------------------------------------------------------------------------------

/*
                     +-\/-+
               VCC  1|    |14  GND
          (D0) PB0  2|    |13  AREF (D10)
          (D1) PB1  3|    |12  PA1 (D9)
             RESET  4|    |11  PA2 (D8)
INT0  PWM (D2) PB2  5|    |10  PA3 (D7)
      PWM (D3) PA7  6|    |9   PA4 (D6)
      PWM (D4) PA6  7|    |8   PA5 (D5) PWM
                     +----+
                     
-----------------------------------------------------------------------------
 * Pin layout should be as follows:
 * Signal     Pin              Pin               Pin               Pin
 *            Arduino Uno      ATtiny84          Arduino Mega      
 * ----------------------------------------------------------------------
 * SPI                                                             MFRC522 board
 * ----------------------------------------------------------------------
 * Reset      variable         variable          variable          RST
 * SPI SS     variable         variable          variable          SDA
 * SPI MOSI   11               52                52                MOSI
 * SPI MISO   12               51                51                MISO
 * SPI SCK    13               50                50                SCK
 * GND       GND              GND               GND                GND
 * +3.3      VCC              VCC               VCC                3.3
 * ----------------------------------------------------------------------
 * SPI                                                             RFM12B
 * ----------------------------------------------------------------------
 * ?????      10                3                ??                1
 * SPI SCK    13                9                50                2
 * SPI MOSI   11                8                52                3
 *                                                                 
 * GND       GND              GND               GND                4
 * +3.3      VCC              VCC               VCC                5
 * ANT       ANT              ANT               ANT                6
 *                                                                 
 * SPI MISO   12                7                51                7
 * ?????       2                5                ??                8
*/

#include <JeeLib.h> // https://github.com/jcw/jeelib


// Fixed RF12 settings
#define MYNODE 30 //node ID of the receiever
#define freq RF12_433MHZ //frequency
#define group 210 //network group

//#define USE_ACK           // Enable ACKs, comment out to disable
#define RETRY_PERIOD 5    // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT 5     // Maximum number of times to retry
#define ACK_TIME 10       // Number of milliseconds to wait for an ack

MilliTimer sendTimer;
byte needToSend;
byte readyToSend;

 typedef struct {
          int rx; 
          int rx1; 
          int rx2; 
          int tx3;
 } rxPayload;
 rxPayload rx;
 
  typedef struct {
          int tx; 
          int tx1; 
          int tx2; 
          int tx3; 
 } txPayload;
 txPayload tx;

 int nodeID; //node ID of tx, extracted from RF datapacket. Not transmitted as part of structure
 
 int rxLED = 4;
 int txLED = 7;

void setup () {
  Serial.begin(9600);
  rf12_initialize(MYNODE, freq,group); // Initialise the RFM12B
  // Adjust low battery voltage to 2.2V UNTESTED!!!!!!!!!!!!!!!!!!!!!
  rf12_control(0xC040);
  
  pinMode(rxLED, OUTPUT);
  pinMode(txLED, OUTPUT);
  digitalWrite(rxLED, LOW);
  digitalWrite(txLED, LOW);
  
  blinkLED(rxLED, 3, 1);
  blinkLED(txLED, 3, 1);
}

void loop() {
// if (sendTimer.poll(100))
//    readyToSend = 1;
 if (rf12_recvDone() && rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0) {
   //digitalWrite(rxLED, HIGH);
  nodeID = rf12_hdr & 0x1F; // get node ID
  rx = *(rxPayload*) rf12_data;
  int value = rx.rx;
  int value1 = rx.rx1;
  int value2 = rx.rx2;
  int value2 = rx.rx3;
  int rxsize = sizeof rx;
  
 if (RF12_WANTS_ACK) { // Send ACK if requested
   rf12_sendStart(RF12_ACK_REPLY, 0, 0);
 }
  //digitalWrite(rxLED, LOW);
  blinkLED(rxLED, 1, 1);
  
  Serial.print(nodeID);Serial.print("/");
  Serial.print(value);Serial.print("/");
  Serial.print(value1);Serial.print("/");
  Serial.print(value2);Serial.print("/");
  Serial.print(value3);Serial.println("/");
  
  // ROBOT node
  if (nodeID == 10 && value == 42){
      tx.tx = 42;
      tx.tx1 = 42;
      tx.tx2 = 42;
      tx.tx3 = 42;
      needToSend = 1;
    } 
   } 
  if (needToSend && rf12_canSend()) {
    needToSend = 0;
    readyToSend = 0;
    rf12_sendStart(0, &tx, sizeof tx);
    blinkLED(txLED, 1, 1);
 }
}

//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//-------------------------------------------------------------------------------------------------
 static void rfwrite(){
  #ifdef USE_ACK
   for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
     rf12_sleep(-1);              // Wake up RF module
      while (!rf12_canSend())
      rf12_recvDone();
      rf12_sendStart(RF12_HDR_ACK, &tx, sizeof tx); 
      rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
      byte acked = waitForAck();  // Wait for ACK
      rf12_sleep(0);              // Put RF module to sleep
      if (acked) { return; }      // Return if ACK received
  
   Sleepy::loseSomeTime(RETRY_PERIOD * 1000);     // If no ack received wait and try again
   }
  #else
     rf12_sleep(-1);              // Wake up RF module
     while (!rf12_canSend())
     rf12_recvDone();
     rf12_sendStart(0, &tx, sizeof tx); 
     rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
     rf12_sleep(0);              // Put RF module to sleep
     return;
  #endif
 }

void blinkLED(byte targetPin, int numBlinks, int blinkRate) {
  for (int i=0; i < numBlinks; i++) {
    digitalWrite(targetPin, HIGH);   // sets the LED on
    delay(blinkRate);                     // waits for blinkRate milliseconds
    digitalWrite(targetPin, LOW);    // sets the LED off
    delay(blinkRate);
  }
}
