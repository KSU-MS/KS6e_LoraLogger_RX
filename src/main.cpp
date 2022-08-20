#include <Arduino.h>
#include <RadioLib.h>
int pin_cs = 10;
int pin_dio0 = 2;
int pin_nrst = 4;
int pin_dio1 = 3;
SX1276 radio = new Module(pin_cs, pin_dio0, pin_nrst, pin_dio1);
void setFlag(void);
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
  Serial.print(F("[SX1278] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
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

    // you can also read received data as byte array
    /*
      byte byteArr[8];
      int state = radio.readData(byteArr, 8);
    */

    if (state == RADIOLIB_ERR_NONE) {

      Serial.println(str);
    }
    // put module back to listen mode
    radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }

}