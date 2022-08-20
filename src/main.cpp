#include <Arduino.h>
#include <RadioLib.h>
#include <Metro.h>
//#define DEBUG
int pin_cs = 10;
int pin_dio0 = 2;
int pin_nrst = 4;
int pin_dio1 = 3;
SX1276 radio = new Module(pin_cs, pin_dio0, pin_nrst, pin_dio1);
int packvoltage=0,invertercurrent=0,torquereq=0,motorrpm=0;
int motortemp=0;
int invertertemp=0;
int accel1_=0,accel2_=0,brake1_=0;
uint64_t invfaults=0;
Metro sendToPcTimer = Metro(1);
void sendToPC(int* data1, int* data2, int* data3,int* data4,int* data5,int* data6,int* data7);
void setFlag(void);
byte buf[10];
void setup() {
  delay(500); //Wait for ESP32 to be able to print
  Serial.begin(115200);
  Serial.print(F("[SX1276] Initializing ... "));
  //int state = radio.begin(); //-121dBm
  //int state = radio.begin(868.0); //-20dBm
  int state = radio.begin(915.0); //-23dBm
  radio.disableAddressFiltering();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("init success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set output power to 10 dBm (accepted range is -3 - 17 dBm)
  // NOTE: 20 dBm value allows high power operation, but transmission
  //       duty cycle MUST NOT exceed 1%
//  if (radio.setOutputPower(20) == ERR_INVALID_OUTPUT_POWER) {
//    Serial.println(F("Selected output power is invalid for this module!"));
//    while (true);
//  }

  // some modules have an external RF switch
  // controlled via two pins (RX enable, TX enable)
  // to enable automatic control of the switch,
  // call the following method
  radio.setDio0Action(setFlag);
  int pin_rx_enable = 5;
  int pin_tx_enable = 6;
  radio.setRfSwitchPins(pin_rx_enable, pin_tx_enable);
    // start listening for LoRa packets
  //Serial.print(F("[SX1278] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    //Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
}

int counter = 0;

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}


void loop() {
  // check if the flag is set
  if(receivedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    String str;
    int state = radio.readData(str);
    int spaceindex=str.indexOf(",");
    packvoltage=(str.substring(0,spaceindex).toInt()); //Serial.println(packvoltage);
      str=str.substring(spaceindex+1); //
        spaceindex=str.indexOf(",");
    motortemp=(str.substring(0,spaceindex).toInt()); //Serial.println(motortemp);
      str=str.substring(spaceindex+1); //
        spaceindex=str.indexOf(",");
    invertertemp=(str.substring(0,spaceindex).toInt()); //Serial.println(invertertemp);
      str=str.substring(spaceindex+1);
        spaceindex=str.indexOf(",");
    invfaults=(str.substring(0,spaceindex).toInt()); //Serial.println(invfaults);
      str=str.substring(spaceindex+1);
        spaceindex=str.indexOf(",");
    accel1_=(str.substring(0,spaceindex).toInt()); //Serial.println(accel1_);
      str=str.substring(spaceindex+1);
        spaceindex=str.indexOf(",");
    accel2_=(str.substring(0,spaceindex).toInt()); //Serial.println(accel2_);
      str=str.substring(spaceindex+1);
        spaceindex=str.indexOf(",");
    brake1_=(str.substring(0,spaceindex).toInt()); //Serial.println(brake1_);
      str=str.substring(spaceindex+1);
        spaceindex=str.indexOf(",");
    invertercurrent=(str.substring(0,spaceindex).toInt()); //Serial.println(invertercurrent);
      str=str.substring(spaceindex+1);
        spaceindex=str.indexOf(",");
    torquereq=(str.substring(0,spaceindex).toInt()); //Serial.println(torquereq);
      str=str.substring(spaceindex+1);
        spaceindex=str.indexOf(",");
    motorrpm=(str.substring(0,spaceindex).toInt()); //Serial.println(motorrpm);
      str=str.substring(spaceindex+1);
    // you can also read received data as byte array
    /*
      byte byteArr[8];
      int state = radio.readData(byteArr, 8);
    */

    if (state == RADIOLIB_ERR_NONE) {

      //Serial.println(str);
    }
    // put module back to listen mode
    radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
    
  }
  if(sendToPcTimer.check()){
    sendToPC(&accel1_,&accel2_,&brake1_,&motortemp,&motorrpm,&packvoltage,&torquereq);
  }
}
void sendToPC(int* data1, int* data2, int* data3,int* data4,int* data5,int* data6,int* data7)
{
  #ifdef DEBUG
  Serial.printf("%d, %d, %d\n",data1,data2,data3);
  #endif
  byte* byteData1 = (byte*)(data1);
  byte* byteData2 = (byte*)(data2);
  byte* byteData3 = (byte*)(data3);
  byte* byteData4 = (byte*)(data4);
  byte* byteData5 = (byte*)(data5);
  byte* byteData6 = (byte*)(data6);
  byte* byteData7 = (byte*)(data7);
  byte buf[14] = {byteData1[0], byteData1[1],
                 byteData2[0], byteData2[1],
                 byteData3[0], byteData3[1],
                 byteData4[0], byteData4[1],       
                 byteData5[0], byteData5[1],
                 byteData6[0], byteData6[1],byteData7[0], byteData7[1]};
  Serial.write(buf, 14);
}
