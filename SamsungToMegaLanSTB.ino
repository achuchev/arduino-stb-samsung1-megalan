/*
  First You need to remove the build-in RobotIRremote library from /Applications/Arduino.app/Contents/Java/libraries
*/

#include <IRremote.h>

#define DEBUG_ENABLED // Set to DEBUG_ENABLED to enable debug mode
#define TIME_BETWEEN_TRANSMITTION 500

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
const unsigned long codeNecPower = 0xFF02FD; // Requres to send codeNecDoNotRepeat
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
const unsigned long codeNecSpecialRepeatSequence = 0xFFFFFFFF;


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
const unsigned long codeSamsungSimpleRCKeyUp = 0xE0E006F9;
const unsigned long codeSamsungSimpleRCKeyDown = 0xE0E08679;
const unsigned long codeSamsungSimpleRCOk = 0xE0E016E9;
const unsigned long codeSamsungSimpleRCReturn = 0xE0E01AE5;

// Key Combination Variables
#define KEY_COMBINATION_COUNT 4
int lastPressedKeysIndex = 0;
unsigned long timeOfTheLastPressedKeySequance = 0;
unsigned long lastPressedKeys[KEY_COMBINATION_COUNT];
unsigned long keyCombinationTVSleepTimer60[KEY_COMBINATION_COUNT] = {codeSamsungNavLeft, codeSamsungNavRight, codeSamsungNavLeft, codeSamsungNavRight};
unsigned long keyCombinationTVSleepTimer30[KEY_COMBINATION_COUNT] = {codeSamsungNavLeft, codeSamsungNavRight, codeSamsungNavLeft, codeSamsungNavLeft};
unsigned long keyCombinationTVScreenOff[KEY_COMBINATION_COUNT] = {codeSamsungNavLeft, codeSamsungNavLeft, codeSamsungNavRight, codeSamsungNavRight};
unsigned long keyCombinationSTBPowenOnOFF[KEY_COMBINATION_COUNT] = {codeSamsungNavRight, codeSamsungNavLeft, codeSamsungNavRight, codeSamsungNavLeft};

unsigned long invalidCode = 0xFFFFFF;

void setup()
{
  resetLastPassedKeysArray();
#ifdef DEBUG_ENABLED
  Serial.begin(9600); // Enable Debug
#endif
  irrecv.enableIRIn(); // Start the IR receiver
}

void software_Reset()
// Restarts program from beginning but
// does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");
}

void sendMegaLanCode(unsigned long code, bool useNEC = true, int delayBeforeTransmit = 0, int repeat = 1 ) {

#ifdef DEBUG_ENABLED
  Serial.print("Sleep: ");
  Serial.print(delayBeforeTransmit, DEC);
  Serial.print(" Send IR Code: ");
  Serial.print(code, HEX);
  Serial.print(" codeLen:");
  Serial.print(codeLen, DEC);
  Serial.print(" Type: ");
#endif
  delay(delayBeforeTransmit);

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

bool compareArrays(unsigned long actualArray[], unsigned long expectedArray[], int n, bool &isSequance) {
  for (int i = 0; i < n; i++) {
    if (actualArray[i] != invalidCode) {
      if (actualArray[i] != expectedArray[i]) {
        // invalid sequance
        isSequance = false;
        //resetLastPassedKeysArray();
        return false;
      } else {
        // not full, but valid sequance
      }
    } else {
      return false;
    }
  }
  // the sequance is fully correct
  return true;
}

void resetLastPassedKeysArray() {
  for (int i = 0; i < KEY_COMBINATION_COUNT; i++ ) {
    lastPressedKeys[i] = invalidCode;
  }
  lastPressedKeysIndex = 0;
}

bool isKeyCombination(unsigned long code) {
  unsigned int  timeSinceLastSequanceKeyPressed = 0;
  if (timeOfTheLastPressedKeySequance == 0) {
    timeOfTheLastPressedKeySequance = millis();
  }

  timeSinceLastSequanceKeyPressed = millis()  - timeOfTheLastPressedKeySequance;

#ifdef DEBUG_ENABLED
  Serial.print("Milis since last key pressed: ");
  Serial.println(timeSinceLastSequanceKeyPressed);
#endif

  if (timeSinceLastSequanceKeyPressed >= 5000) {
    // Too much time since last sequance key was pressed
#ifdef DEBUG_ENABLED
    Serial.println("Too much time since last sequance key was pressed. Resetting the sequance.");
#endif
    timeOfTheLastPressedKeySequance = 0;
    resetLastPassedKeysArray();
  }
  else {
    timeOfTheLastPressedKeySequance = millis();
  }

  lastPressedKeys[lastPressedKeysIndex] = code;
  if (lastPressedKeysIndex == (KEY_COMBINATION_COUNT - 1)) {
    lastPressedKeysIndex = 0;
  } else {
    lastPressedKeysIndex++;
  }
#ifdef DEBUG_ENABLED
  Serial.println("lastPressedKeys Start");
#endif
  // FIXME: For some reason the code below is needed, otherwise the program will not work:(
  for (int i = 0; i < KEY_COMBINATION_COUNT; i++ ) {
    Serial.println(lastPressedKeys[i], HEX);
  }
#ifdef DEBUG_ENABLED
  Serial.println("lastPressedKeys End");
#endif

  bool isCombination = false;
  bool isAnySequance = false;

  // TV sleep timer 60 min
  bool isSequance = true;
  if (compareArrays(lastPressedKeys, keyCombinationTVSleepTimer60, KEY_COMBINATION_COUNT, isSequance)) {
    sendTVSleepTimer(true);
    isCombination = true;
  }
  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  // TV sleep timer 30 min
  isSequance = true;
  if (compareArrays(lastPressedKeys, keyCombinationTVSleepTimer30, KEY_COMBINATION_COUNT, isSequance)) {
    sendTVSleepTimer(false);
    isCombination = true;
  }
  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  // STB on/off
  isSequance = true;
  if (compareArrays(lastPressedKeys, keyCombinationSTBPowenOnOFF, KEY_COMBINATION_COUNT, isSequance)) {
    sendSTBPowerOnOff();
    isCombination = true;
  }
  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  //TV Screen off
  isSequance = true;
  if (compareArrays(lastPressedKeys, keyCombinationTVScreenOff, KEY_COMBINATION_COUNT, isSequance)) {
    sendTVScreenOff();
    isCombination = true;
  }
  if (isSequance || isAnySequance) {
    isAnySequance = true;
  }

  if (isCombination || !isAnySequance) {
    resetLastPassedKeysArray();
  }

  if (isCombination) {
    return true;
  }
  return false;

#ifdef DEBUG_ENABLED
  Serial.print("isCombination: ");
  Serial.println(isCombination);
  Serial.print("isAnySequance: ");
  Serial.println(isAnySequance);

  Serial.println("lastPressedKeys Start");
  for (int i = 0; i < KEY_COMBINATION_COUNT; i++ ) {
    Serial.println(lastPressedKeys[i], HEX);
  }
  Serial.println("lastPressedKeys End");
#endif
  return false;
}

void sendSTBPowerOnOff() {
#ifdef DEBUG_ENABLED
  Serial.println("Sending STB Power On Off Signal.");
#endif
  sendMegaLanCode(codeNecPower);
  sendMegaLanCode(codeNecPower);
}

void sendTVSleepTimer(bool is60min) {
#ifdef DEBUG_ENABLED
  Serial.println("Sending sendTVScreenOff Combination");
#endif
  sendMegaLanCode(codeSamsungSimpleRCKeyTools, false, 1500);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false, TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false, TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false, TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCOk, false, TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCKeyDown, false, TIME_BETWEEN_TRANSMITTION);

  if (is60min == true) {
    sendMegaLanCode(codeSamsungSimpleRCKeyDown, false, TIME_BETWEEN_TRANSMITTION);
  }
  sendMegaLanCode(codeSamsungSimpleRCOk, false, TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCReturn, false, TIME_BETWEEN_TRANSMITTION);
}

void sendTVScreenOff() {
#ifdef DEBUG_ENABLED
  Serial.println("Sending sendTVSleepTimer Combination");
#endif
  sendMegaLanCode(codeSamsungSimpleRCKeyTools, false,  1500);
  sendMegaLanCode(codeSamsungSimpleRCKeyUp, false, TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCOk, false, TIME_BETWEEN_TRANSMITTION);
  sendMegaLanCode(codeSamsungSimpleRCReturn, false, TIME_BETWEEN_TRANSMITTION);
}


void loop() {
  if (irrecv.decode(&results)) {
    storeCode(&results);
    irrecv.resume(); // resume receiver
#ifdef DEBUG_ENABLED
    Serial.print("Received: ");
    Serial.println(codeValueReceived, HEX);
#endif
    if (!isKeyCombination(codeValueReceived)) {
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
            sendSTBPowerOnOff();
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
