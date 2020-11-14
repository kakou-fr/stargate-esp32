// define variables

//MINI MP3 PLAYER
int8_t MP3_RX = 15; //4;
int8_t MP3_TX = 4; //15;

/*
//LedStrip
#define DATA_PIN1   12 // GPIO12 / D6 on WeMos/NodeMCU ESP8266
#define DATA_PIN2   13 // D7
*/

int LED = 2;          // LED Feedback GPIO 12
//Calibrate_LED is the output pin wired to the calibration led.
//int Calibrate_LED = 11;
int Calibrate_LED = 23;
//LDR is the analogue input pin wired to the light dependant resistor.
int LDR = 36;

#define DATA_PIN_CHEVRONS 13
#define DATA_PIN_CHEVRONS_FINAL 12
#define DATA_PIN_RAMP 14

//stepper Gate
int GateDirPin = 16; //18;       // Direction GPIO
int GateStepPin = 17; //19;      // Step GPIO
int GateEnablePin = 5; //21;     // Stepper enable pin
int GateMicroStep1Pin = 21; // Stepper MS1
int GateMicroStep2Pin = 19; // Stepper MS2
int GateMicroStep3Pin = 18; // Stepper MS3

// stepper Chevron
int ChevronDirPin = 22;       // Direction GPIO
int ChevronStepPin = 27;      // Step GPIO
int ChevronEnablePin = 26;     // Stepper enable pin
int ChevronMicroStep1Pin = 25; //33; // Stepper MS1
int ChevronMicroStep2Pin = 33; //25; // Stepper MS2
int ChevronMicroStep3Pin = 32; //26; // Stepper MS3

//speed
int motorSpeed = 200;      // Set step delay for motor in microseconds (smaller is faster)
int currPos = 0;
int currPosChevron = 0;
int oneRotation = 3200; // 200 x 1.8 degrees per step = 360

int maxSteps = 2300;   // maximum steps

int value = 0;
bool debugPrint = true;

String ipStr = "";
String respMsg = "";
String formattedTime = "";
bool noInit = true;

// 1  Gate start and turn
// 2  Chevron Lock
// 3  Wormhole Activate
// 4  Wormhole De-activate
// 5  Event Horizon (puddle sfx)
// 6  DHD 1st Address
// 7  DHD 2nd Address
// 8  DHD 3rd Address
// 9  DHD 4th Address
// 10  DHD 5th Address
// 11  DHD 6th Address
// 12  DHD 7th Address
// 13  DHD red button's LEDs activate
// 14  stargate generic