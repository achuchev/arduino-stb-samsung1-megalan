/*
   IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
   An IR LED must be connected to Arduino PWM pin 3.
   Version 0.1 July, 2009
   Copyright 2009 Ken Shirriff
   http://arcfn.com
*/

#include <IRremote.h>

#define DEBUG
#define DEBUG_ENABLED_// Set to DEBUG to enable debug mode

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
const unsigned long codeNecNavUp = 0xFFE21D;
const unsigned long codeNecNavDown = 0xFFD22D;
const unsigned long codeNecNavRight = 0xFF52AD;
const unsigned long codeNecNavLeft = 0xFF12ED;
// Regular Buttons
const unsigned long codeNecChannelUp = 0xFFD02F;
const unsigned long codeNecChannelDown = 0xFFA857;
const unsigned long codeNecVolUp = 0xFF30CF;
const unsigned long codeNecVolDown = 0xFF18E7;
const unsigned long codeNecOk = 0xFF926D;
const unsigned long codeNecBack = 0xFF32CD;
const unsigned long codeNecPower = 0xFF02FD;
const unsigned long codeNecKey0 = 0xFF00FF;
const unsigned long codeNecKey1 = 0xFF807F;
const unsigned long codeNecKey2 = 0xFF40BF;
const unsigned long codeNecKey3 = 0xFFC03F;
const unsigned long codeNecKey4 = 0xFF20DF;
const unsigned long codeNecKey5 = 0xFFA05F;
const unsigned long codeNecKey6 = 0xFF609F;
const unsigned long codeNecKey7 = 0xFFE01F;
const unsigned long codeNecKey8 = 0xFF10EF;
const unsigned long codeNecKey9 = 0xFF906F;

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

//////////// Codes Samsung from Simple Remote Control
const unsigned long codeSamsungSimpleRCKeyTools = 0xE0E0D22D;
const unsigned long codeSamsungSimpleRCKeyDown = 0xE0E08679;
const unsigned long codeSamsungSimpleRCOk = 0xE0E016E9;
const unsigned long codeSamsungSimpleRCReturn = 0xE0E01AE5;

// Key Combination Variables
#define KEY_COMBINATION_COUNT 4
int lastPressedKeysIndex = 0;
unsigned long lastPressedKeys[KEY_COMBINATION_COUNT];
unsigned long keyCombinationTVSleepTimer[KEY_COMBINATION_COUNT] = {codeSamsungNavLeft, codeSamsungNavRight, codeSamsungNavLeft, codeSamsungNavRight};
unsigned long invalidCode = 0xFFFFFF;

void setup()
{
  resetArray();
  //Serial.begin(9600); // Enable Debug
  irrecv.enableIRIn(); // Start the IR receiver
}

void software_Reset()
// Restarts program from beginning but
// does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");
}

void sendMegaLanCode(unsigned long code, bool useNEC = true, int repeat = 1) {
#ifdef DEBUG_ENABLED
  Serial.print("Send IR Code: ");

  Serial.print(code, HEX);
  Serial.print(" codeLen:");
  Serial.print(codeLen, DEC);
  Serial.print(" Type: ");
#endif

  for (int i = 0; i < repeat; i++) {
    if (useNEC == true) {
      irsend.sendNEC(code, codeLen);
#ifdef DEBUG_ENABLED
      Serial.println("NEC");
#endif
    } else {
      irsend.sendSAMSUNG(code, codeLen);
#ifdef DEBUG_ENABLED
      Serial.println("Samsung");
#endif
    }
  }
  delay(50); // Wait a bit between retransmissions
}

void storeCode(decode_results *results) {
  // FIXME: No need to have this method, but first find out what actualy it does
  codeValueReceived = results->value;
}

bool compareArrays(unsigned long actualArray[], unsigned long expectedArray[], int n) {
  for (int i = 0; i < n; i++) {
    if (actualArray[i] != invalidCode) {
      if (actualArray[i] != expectedArray[i]) {
        resetArray();
        return false;
      }
    } else {
      return false;
    }
  }
  resetArray();
  return true;
}

void resetArray() {
  for (int i = 0; i < KEY_COMBINATION_COUNT; i++ ) {
    lastPressedKeys[i] = invalidCode;
  }
  lastPressedKeysIndex = 0;
}

bool isKeyCombination(unsigned long code) {
  lastPressedKeys[lastPressedKeysIndex] = code;
  if (lastPressedKeysIndex == (KEY_COMBINATION_COUNT - 1)) {
    lastPressedKeysIndex = 0;
  } else {
    lastPressedKeysIndex++;
  }
#ifdef DEBUG_ENABLED
  Serial.println("Key Combination Start");
#endif

  // FIXME: For some reason the code below is needed, otherwise the program will not work:(
  for (int i = 0; i < KEY_COMBINATION_COUNT; i++ ) {
    //    unsigned long az = lastPressedKeys[i];
    Serial.println(0xFFFFFF, HEX);
  }

#ifdef DEBUG_ENABLED
  Serial.println("Key Combination End");
#endif

  bool isArrayEqual = compareArrays(lastPressedKeys, keyCombinationTVSleepTimer, KEY_COMBINATION_COUNT);
  if (isArrayEqual == true) {
    return true;
  }
  return false;
}

void sendTVSleepTimer() {
#ifdef DEBUG_ENABLED
  Serial.println("Sending Key Combination");
#endif
#define TIME_BETWEEN_TRANSMITTION 250

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyTools, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCOk, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCOk, false);

  delay(TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCReturn, false);
}

void loop() {
  if (irrecv.decode(&results)) {
    storeCode(&results);
    irrecv.resume(); // resume receiver
#ifdef DEBUG_ENABLED
    Serial.print("Received: ");
    Serial.println(codeValueReceived, HEX);
#endif

    if (isKeyCombination(codeValueReceived) == true) {
      sendTVSleepTimer();
    } else {
      switch (codeValueReceived) {
        case codeSamsungNavUp:
          {
            sendMegaLanCode(codeNecNavUp);
            break;
          }
        case codeSamsungNavDown:
          {
            sendMegaLanCode(codeNecNavDown);
            break;
          }
        case codeSamsungNavLeft:
          {
            sendMegaLanCode(codeNecNavLeft);
            break;
          }
        case codeSamsungNavRight:
          {
            sendMegaLanCode(codeNecNavRight);
            break;
          }
        case codeSamsungNavEnter:
          {
            sendMegaLanCode(codeNecOk);
            break;
          }
        case codeSamsungPower:
          {
            sendMegaLanCode(codeNecPower);
            break;
          }
        case codeSamsungChannelList:
          {
            sendMegaLanCode(codeNecBack);
            break;
          }
        case codeSamsungKey0:
          {
            sendMegaLanCode(codeNecKey0);
            break;
          }
        case codeSamsungKey1:
          {
            sendMegaLanCode(codeNecKey1);
            break;
          }
        case codeSamsungKey2:
          {
            sendMegaLanCode(codeNecKey2);
            break;
          }
        case codeSamsungKey3:
          {
            sendMegaLanCode(codeNecKey3);
            break;
          }
        case codeSamsungKey4:
          {
            sendMegaLanCode(codeNecKey4);
            break;
          }
        case codeSamsungKey5:
          {
            sendMegaLanCode(codeNecKey5);
            break;
          }
        case codeSamsungKey6:
          {
            sendMegaLanCode(codeNecKey6);
            break;
          }
        case codeSamsungKey7:
          {
            sendMegaLanCode(codeNecKey7);
            break;
          }
        case codeSamsungKey8:
          {
            sendMegaLanCode(codeNecKey8);
            break;
          }
        case codeSamsungKey9:
          {
            sendMegaLanCode(codeNecKey9);
            break;
          }
      }
    }
    irrecv.enableIRIn();
  }
}
