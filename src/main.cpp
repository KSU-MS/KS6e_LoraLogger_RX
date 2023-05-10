#include <Arduino.h>
#include <RadioLib.h>

int pin_cs = 10;
int pin_dio0 = 16;
int pin_nrst = 4;
int pin_dio1 = 17;
int pin_rx_enable = 8;
int pin_tx_enable = 9;
SX1276 radio = new Module(pin_cs, pin_dio0, pin_nrst, pin_dio1);

void setup() {
  delay(500); //Wait for ESP32 to be able to print
  Serial.begin(115200);

  // initialize SX1278 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  int state = radio.begin(915.0);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
  radio.setRfSwitchPins(pin_rx_enable, pin_tx_enable);


//   Serial.print(F("[SX1276] Initializing ... "));
//   //int state = radio.begin(); //-121dBm
//   //int state = radio.begin(868.0); //-20dBm
//   int state = radio.begin(915.0); //-23dBm
//   radio.disableAddressFiltering();
//   if (state == RADIOLIB_ERR_NONE) {
//     Serial.println(F("init success!"));
//   } else {
//     Serial.print(F("failed, code "));
//     Serial.println(state);
//     while (true);
//   }

//   // set output power to 10 dBm (accepted range is -3 - 17 dBm)
//   // NOTE: 20 dBm value allows high power operation, but transmission
//   //       duty cycle MUST NOT exceed 1%
// //  if (radio.setOutputPower(20) == ERR_INVALID_OUTPUT_POWER) {
// //    Serial.println(F("Selected output power is invalid for this module!"));
// //    while (true);
// //  }

//   // some modules have an external RF switch
//   // controlled via two pins (RX enable, TX enable)
//   // to enable automatic control of the switch,
//   // call the following method
//   radio.setDio0Action(setFlag);
//   int pin_rx_enable = 8;
//   int pin_tx_enable = 9;
//   radio.setRfSwitchPins(pin_rx_enable, pin_tx_enable);
//     // start listening for LoRa packets
//   Serial.print(F("[SX1276] Starting to listen ... "));
//   state = radio.startReceive();
//   if (state == RADIOLIB_ERR_NONE) {
//     Serial.println(F("success!"));
//   } else {
//     Serial.print(F("failed, code "));
//     Serial.println(state);
//     while (true);
//   }
// }

// int counter = 0;

// // flag to indicate that a packet was received
// volatile bool receivedFlag = false;

// // disable interrupt when it's not needed
// volatile bool enableInterrupt = true;

// // this function is called when a complete packet
// // is received by the module
// // IMPORTANT: this function MUST be 'void' type
// //            and MUST NOT have any arguments!
// #if defined(ESP8266) || defined(ESP32)
//   ICACHE_RAM_ATTR
// #endif
// void setFlag(void) {
//   // check if the interrupt is enabled
//   if(!enableInterrupt) {
//     return;
//   }

//   // we got a packet, set the flag
//   receivedFlag = true;
}


void loop() {
   Serial.print(F("[SX1276] Waiting for incoming transmission ... "));

  // you can receive data as an Arduino String
  // NOTE: receive() is a blocking method!
  //       See example ReceiveInterrupt for details
  //       on non-blocking reception method.
  String str;
  int state = radio.receive(str);

  // you can also receive data as byte array
  /*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */

  if (state == RADIOLIB_ERR_NONE) {
    // packet was successfully received
    Serial.println(F("success!"));

    // print the data of the packet
    Serial.print(F("[SX1276] Data:\t\t\t"));
    Serial.println(str);

    // print the RSSI (Received Signal Strength Indicator)
    // of the last received packet
    Serial.print(F("[SX1276] RSSI:\t\t\t"));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    // print the SNR (Signal-to-Noise Ratio)
    // of the last received packet
    Serial.print(F("[SX1276] SNR:\t\t\t"));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

    // print frequency error
    // of the last received packet
    Serial.print(F("[SX1276] Frequency error:\t"));
    Serial.print(radio.getFrequencyError());
    Serial.println(F(" Hz"));

  } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    // timeout occurred while waiting for a packet
    Serial.println(F("timeout!"));

  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("CRC error!"));

  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);

  }
  
  

  // // check if the flag is set
  // if(receivedFlag) {
  //   // disable the interrupt service routine while
  //   // processing the data
  //   enableInterrupt = false;

  //   // reset flag
  //   receivedFlag = false;

  //   // you can read received data as an Arduino String
  //   String str;
  //   int state = radio.readData(str);

  //   // you can also read received data as byte array
  //   /*
  //     byte byteArr[8];
  //     int state = radio.readData(byteArr, 8);
  //   */

  //   if (state == RADIOLIB_ERR_NONE) {

  //     Serial.println(str);
  //   }
  //   // put module back to listen mode
  //   radio.startReceive();

  //   // we're ready to receive more packets,
  //   // enable interrupt service routine
  //   enableInterrupt = true;
  // }
}