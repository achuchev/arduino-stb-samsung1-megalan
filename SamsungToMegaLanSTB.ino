/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

IRsend irsend;
int RECV_PIN = 2; // From the TV send IR device
int STATUS_PIN = 13; // For the LED
int STB_ON_OFF_PIN = A0; // B/Pb cable on the STB
decode_results results;
IRrecv irrecv(RECV_PIN);

unsigned long codeValueReceived; // The code value if not raw
int codeLen = 32;
boolean stbStatusLastOn = false;

//////////// Codes Set Top Box MegaLan
// Navigation Controls
const unsigned long codeNecNavUp = 0xFE7A85;
const unsigned long codeNecNavDown = 0xFE6A95;
const unsigned long codeNecNavRight = 0xFE1AE5;
const unsigned long codeNecNavLeft = 0xFEDA25;
// Regular Buttons
const unsigned long codeNecChannelUp = 0xFEF807;
const unsigned long codeNecChannelDown = 0xFE3AC5;
const unsigned long codeNecVolUp = 0xFE7887;
const unsigned long codeNecVolDown = 0xFEFA05;
const unsigned long codeNecEnter = 0xFE5AA5;
const unsigned long codeNecC = 0xFE58A7;
const unsigned long codeNecPower = 0xFE50AF; // Send it twice
const unsigned long codeNecKey0 = 0xFEC03F;
const unsigned long codeNecKey1 = 0xFE708F;
const unsigned long codeNecKey2 = 0xFE609F;
const unsigned long codeNecKey3 = 0xFEF00F;
const unsigned long codeNecKey4 = 0xFE48B7;
const unsigned long codeNecKey5 = 0xFEE01F;
const unsigned long codeNecKey6 = 0xFEC837;
const unsigned long codeNecKey7 = 0xFE6897;
const unsigned long codeNecKey8 = 0xFE40BF;
const unsigned long codeNecKey9 = 0xFEE817;

//////////// Codes Samsung 1+ provider
const unsigned long  codeSamsungNavUp = 0x10EF50AF;
const unsigned long codeSamsungNavDown = 0x10EFD02F;
const unsigned long codeSamsungNavLeft = 0x10EFB04F;
const unsigned long codeSamsungNavRight = 0x10EF30CF;
const unsigned long codeSamsungNavEnter = 0x10EF9867;
const unsigned long codeSamsungPower = 0x10EFF00F;
const unsigned long codeSamsungChannelList = 0x10EF28D7;
const unsigned long codeSamsungKey0 = 0x10EF906F;
const unsigned long codeSamsungKey1 = 0x10EF00FF;
const unsigned long codeSamsungKey2 = 0x10EF807F;
const unsigned long codeSamsungKey3 = 0x10EF40BF;
const unsigned long codeSamsungKey4 = 0x10EFC03F;
const unsigned long codeSamsungKey5 = 0x10EF20DF;
const unsigned long codeSamsungKey6 = 0x10EFA05F;
const unsigned long codeSamsungKey7 = 0x10EF609F;
const unsigned long codeSamsungKey8 = 0x10EFE01F;
const unsigned long codeSamsungKey9 = 0x10EF10EF;
const unsigned long codeSamsungKeyPreCh = 0x10EFE817;

void setup()
{
  // Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

#define STB_NUM_READS 100
int readSTBStatusPin(){
  // read multiple values and sort them to take the mode
  int sortedValues[STB_NUM_READS];
  for(int i=0;i<STB_NUM_READS;i++){
    int value = analogRead(STB_ON_OFF_PIN);
    int j;
    if(value<sortedValues[0] || i==0){
      j=0; //insert at first position
    }
    else{
      for(j=1;j<i;j++){
        if(sortedValues[j-1]<=value && sortedValues[j]>=value){
          // j is insert position
          break;
        }
      }
    }
    for(int k=i;k>j;k--){
      // move all values higher than current reading up one position
      sortedValues[k]=sortedValues[k-1];
    }
    sortedValues[j]=value; //insert current reading
  }
  //return scaled mode of 10 values
  float returnval = 0;
  for(int i=STB_NUM_READS/2-5;i<(STB_NUM_READS/2+5);i++){
    returnval +=sortedValues[i];
  }
  returnval = returnval/10;
  return returnval*1100/1023;
}


void software_Reset()
// Restarts program from beginning but
// does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");
}

void checkSTBStatus(){
  // Check the voltage level on B/Pb cable on the STB
  // above 2.5V(AVG = 612) means that the STB is OFF
  // below 2.0V(AVG = 308) means that the STB is ON

  int val = readSTBStatusPin();
  // Serial.print(val);
  // Serial.print(" ");

  if (val>=100){
    // The STB is OFF but the TV(arduino) is started, so we have to start the STB
    if (stbStatusLastOn==false){
      // Send Power signal to STB
      sendMegaLanCode(codeNecPower, 1);
      stbStatusLastOn = true;
      delay(10000);
      // FIXME do not reset Arduino. For some reason if you turn on the TV the receiving of IR codes is blocked.
      software_Reset();
    }
    // Serial.println("OFF");
  }
  else{
    // Serial.println("ON");
  }
  // Serial.println("EXIT");
}

void sendMegaLanCode(unsigned long code, int repeat){
  //  Serial.print("Send MegaLan codeValue=");
  //  Serial.print(code, HEX);
  //  Serial.print(" codeLen=");
  //  Serial.println(codeLen, DEC);

  for (int i=0; i<repeat; i++){
    irsend.sendNEC(code, codeLen);
    delay(50); // Wait a bit between retransmissions
  }
}


void storeCode(decode_results *results) {
  // FIXME: No need to have this method, but first find out what actualy it does
  codeValueReceived = results->value;
}

void loop() {



  if (irrecv.decode(&results)) {
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results);
    irrecv.resume(); // resume receiver
    digitalWrite(STATUS_PIN, LOW);

    // Serial.print("Received: ");
    // Serial.println(codeValueReceived, HEX);

    switch (codeValueReceived) {
    case codeSamsungNavUp:
      {
        sendMegaLanCode(codeNecNavUp, 2);
        break;
      }
    case codeSamsungNavDown:
      {
        sendMegaLanCode(codeNecNavDown, 2);
        break;
      }
    case codeSamsungNavLeft:
      {
        sendMegaLanCode(codeNecNavLeft, 1);
        break;
      }
    case codeSamsungNavRight:
      {
        sendMegaLanCode(codeNecNavRight, 1);
        break;
      }
    case codeSamsungNavEnter:
      {
        sendMegaLanCode(codeNecEnter, 1);
        break;
      }
    case codeSamsungPower:
      {
        sendMegaLanCode(codeNecPower, 1);
        break;
      }
    case codeSamsungChannelList:
      {
        sendMegaLanCode(codeNecC, 2);
        break;
      }
    case codeSamsungKey0:
      {
        sendMegaLanCode(codeNecKey0, 1);
        break;
      }
    case codeSamsungKey1:
      {
        sendMegaLanCode(codeNecKey1, 1);
        break;
      }
    case codeSamsungKey2:
      {
        sendMegaLanCode(codeNecKey2, 1);
        break;
      }
    case codeSamsungKey3:
      {
        sendMegaLanCode(codeNecKey3, 1);
        break;
      }
    case codeSamsungKey4:
      {
        sendMegaLanCode(codeNecKey4, 1);
        break;
      }
    case codeSamsungKey5:
      {
        sendMegaLanCode(codeNecKey5, 1);
        break;
      }
    case codeSamsungKey6:
      {
        sendMegaLanCode(codeNecKey6, 1);
        break;
      }
    case codeSamsungKey7:
      {
        sendMegaLanCode(codeNecKey7, 1);
        break;
      }
    case codeSamsungKey8:
      {
        sendMegaLanCode(codeNecKey8, 1);
        break;
      }
    case codeSamsungKey9:
      {
        sendMegaLanCode(codeNecKey9, 1);
        break;
      }
    }
    irrecv.enableIRIn();
  }

  checkSTBStatus();
}
