/*********
   Rui Santos
   Complete project details at http://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include "settings.h"
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <SPIFFS.h>

#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

#include <FastLED.h>

#define NUM_LEDS_CHEVRONS 8
#define NUM_LEDS_CHEVRONS_FINAL 1
#define NUM_LEDS_RAMPS 8

//MP3
SoftwareSerial mp3(MP3_RX, MP3_TX, false); //, 256);
DFRobotDFPlayerMini myDFPlayer;


//chevron
CRGB ledsChevron[NUM_LEDS_CHEVRONS];
CRGB ledsChevronFINAL[NUM_LEDS_CHEVRONS_FINAL];
//ramp
CRGB ledsRamps[NUM_LEDS_RAMPS];
#define FASTLED_SHOW_CORE 0
#define BRIGHTNESS 200

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001008)
#warning "Requires FastLED 3.1.8 or later; check github for latest code."
#endif

// choose the ntp server that serves your timezone
#define NTP_OFFSET 2 * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "0.fr.pool.ntp.org"   //  NTP SERVER

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//led
// lestsrip 1 (9 leds)
// ledstrip 2 (8 leds) Ramp_Lights is the output pin wired to the ramp lights (this assumes all lights are connected to a single pin).

/*********STARGATE************/

int MICROSTEP = 16;
int CAL_STEP1 = MICROSTEP;
int CAL_STEP2 = CAL_STEP1 * 10;

/*
   Populate Chevrons[] array with output pins according to each chevron in dialling order.
   Note: pins 0 & 1 cannot be used while the Arduino is connected to a PC
   For this reason the pins defined by default start from 2.
 */
int ramps_led[] = {3,4,2,5,1,6,0,7};
int ramps_led_size = 8;

//int Chevrons[] = {7,8,9,3,4,5,10,2,6};
//int Chevrons[] = {5,6,7,10,2,3,8,11,4};
//int Chevrons[] =   {3,2,1,0 ,8,7,6,5 ,4};
int Chevrons[] =   {3,2,1,5,6,7,0,8,4};

//int Chevrons[] = {3,2,1,0,8,7,6,5,4};


//Populate Ring_Chevrons[] array with output pins according to each chevron in clockwise (or anticlockwise) order.
//int Ring_Chevrons[] = {2,3,4,5,6,7,8,9,10};
//int Ring_Chevrons[] = {11,10,2,3,4,5,6,7,8};
int Ring_Chevrons[] =   {8,7,6,5,4,3,2,1,0};

//Chevron_Locked should be set to the last chevron in the dialling sequence
int Chevron_Locked = Chevrons[8];

//Set Cal to 0 for calibration and then dialling.
//Set Cal to 3 to bypass calibration and jump to dialling.
int Cal = 3;

//The calibration point is set 3% higher than the average. Adjust this number if you experience problems with calibration.
int Calibration_Percent = 3;

//How long should the locked chevron be lit before continuing dialling. Default is 1500 (1.5 seconds).
int ChevronLocked = 500;

//How many steps the stepper motor must turn per symbol.
//This value is overritten by the calibrate function.
//If you bypass calibration this value must be set accordingly.
float Step_Per_Symbol = 30.77*MICROSTEP;

//How many steps to move the stepper motor for the chevron.
int Chevron_Step = 10*MICROSTEP;

//Set Ring_Display to a number between 1 and 7 for predefined display functions.
//Set Ring_Display to 0 for calibration or dialling.
int Ring_Display = 0;

int R=255, G=165, B=0;

//Sample Startgate addresses. un-comment the address you want.
//Abydos
int Address[] = {27,7,15,32,12,30,1,0,0};
int Address_Length = 7;

int Address_Abydos[] = {27,7,15,32,12,30,1};
int R_Abydos=255, G_Abydos=165, B_Abydos=0;

//Othala (Asgard homeworld)
int Address_Asgard[] = {11,27,23,16,33,3,9,1};
int R_Asgard=0, G_Asgard=255, B_Asgard=0;

//Destiny
int Address_Destiny[] = {6,17,21,31,35,24,5,11,1};
int R_Destiny=0, G_Destiny=0, B_Destiny=255;

//Other variable definitions. No need to change these.
float LDR_avg = 0.0;
float LDR_cal = 0.0;
int Dialling = 0;
int Ring = 0;
float Step_increment = 0;

/*********STARGATE************/

void setup() {
        Serial.begin(115200);
        mp3.begin(115200);
        SPIFFS.begin();
        setupPinModes();
        ChevronstopRoll();
        GatestopRoll();
// We start by connecting to a WiFi network
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);
// Wifi with
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
                Serial.println("Connection Failed! Rebooting...");
                delay(500);
                ESP.restart();
        }
        setupOTA();
        initSettings();

// start server and time client
        server.begin();
        timeClient.begin();
// Blink onboard LED to signify its connected
        blink(2);
//led
//Initialize the lib for the ledstrip
        FastLED.addLeds<WS2812B, DATA_PIN_CHEVRONS, GRB>(ledsChevron, NUM_LEDS_CHEVRONS).setCorrection( TypicalLEDStrip );
        FastLED.addLeds<WS2812B, DATA_PIN_CHEVRONS_FINAL, GRB>(ledsChevronFINAL, NUM_LEDS_CHEVRONS_FINAL).setCorrection( TypicalLEDStrip );
        FastLED.addLeds<WS2812B, DATA_PIN_RAMP, GRB>(ledsRamps, NUM_LEDS_RAMPS).setCorrection( TypicalLEDStrip );
        FastLED.setBrightness(  BRIGHTNESS );
        FastLED.setDither( 0 );
        FastLED.show();
/****/
        fillAll(255, 0, 0);
        delay(200);
        fillAll(0, 255, 0);
        delay(200);
        fillAll(0, 0, 255);
        delay(200);

        ClearAllLedData();
        FastLED.show();
        for (uint32_t i = 0; i < NUM_LEDS_CHEVRONS; i++) {
                setPixel(1,i, R, G, B);
        }
        for (uint32_t i = 0; i < NUM_LEDS_CHEVRONS_FINAL; i++) {
                setPixel(2,i, R, G, B);
        }
        for (uint32_t i = 0; i < NUM_LEDS_RAMPS; i++) {
                setPixel(3,i, 255, 255, 255);
        }
        FastLED.show();
        FastLED.delay(200);
        ClearAllLedData();
/**/

/********STARGATE************/
        pinMode(Calibrate_LED, OUTPUT);
        pinMode(LDR,INPUT);
        if (Ring_Display > 0) {
                Cal = 3;
                Dialling = 10;
        }else if (Cal == 3) {
                Dialling = 1;
        }else{
                Cal = 0;
                Dialling = 0;
                Ring_Display = 0;
        }
/********STARGATE************/
/*******   MP3   ************/
        Serial.println();
        Serial.println(F("DFRobot DFPlayer Mini Demo"));
        Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

        if (!myDFPlayer.begin(mp3, true, false)) { //Use softwareSerial to communicate with mp3.
                Serial.println(F("Unable to begin:"));
                Serial.println(F("1.Please recheck the connection!"));
                Serial.println(F("2.Please insert the SD card!"));
                while(true) {
                        delay(0); // Code to compatible with ESP8266 watch dog.
                }
        }
        Serial.println(F("DFPlayer Mini online."));

        myDFPlayer.volume(10); //Set volume value. From 0 to 30
        myDFPlayer.play(1); //Play the first mp3
/*******   MP3   ************/

}


void playSound(int x) {
  if (x == 0){// Listen for a command to stop playing from DHD or gate
    myDFPlayer.stop();
  }

  else if (x == 1) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(1); // Gate start and turn
    }
  }
  else if (x == 2) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(2); // Chevron Lock
    }
  }
  else if (x == 3) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(3); // Wormhole Activate
    }

  }
  else if (x == 4) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(4); // Wormhole De-activate
    }
  }
  else if (x == 5) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(5); // Event Horizon (puddle sfx)
    }
  }
  else if (x == 6) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(6); // DHD 1st Address
    }
  }
  else if (x == 7) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(7); // DHD 2nd Address
    }
  }
  else if (x == 8) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(8); // DHD 3rd Address
    }
  }
  else if (x == 9) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(9); // DHD 4th Address
    }
  }
  else if (x == 10) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(10); // DHD 5th Address
    }
  }
  else if (x == 11) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(11); // DHD 6th Address
    }
  }
  else if (x == 12) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(12); // DHD 7th Address
    }
  }
  else if (x == 13) {
    if (x != 0 && myDFPlayer.available()) {
      myDFPlayer.play(13); // DHD red button's LEDs activate
    }
  }
}

uint8_t thishue = 0;
uint8_t deltahue = 7;

int nothing_to_do=0;
int dialing = 0;
void loop(){
        // if OTA called we need this
        ArduinoOTA.handle();
        WiFiClient client = server.available();                  // listen for incoming clients

        if(dialing) {
                if (Cal < 3) {
                        calibrate();
                }else if (Dialling <= Address_Length) {
                        dial(Address[Dialling++]);
                        if (Dialling == Address_Length) {
                                GaterollFORWARD(Chevron_Step);
                                ChevronstopRoll();
                                Serial.println("Wormhole Established");
                                for (int tmp_chevron1 = 0; tmp_chevron1 < 9; tmp_chevron1++) {
                                        ledChevron(Chevrons[tmp_chevron1], HIGH);
                                }
                                ledRamp(HIGH);
                                delay(10000);
                                for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron++) {
                                        ledChevron(Chevrons[tmp_chevron], LOW);
                                }
                                ledRamp(LOW);
                                Serial.println("Wormhole Disengaged");
                                /********/
                                dialing=0;
                                Dialling=0;
                                /******/
                        }
                        if(Dialling>11) {
                                /********/
                                dialing=0;
                                Dialling=0;
                                /******/
                        }
                }else if (Ring_Display == 1) {
                        if ((Ring < 1) || (Ring > 9)) {
                                Ring = 1;
                        }
                        ring_lights(120);
                        Ring++;
                        if (Ring >= 10) {
                                Ring = 1;
                        }
                }else if (Ring_Display == 2) {
                        if ((Ring < 1) || (Ring > 18)) {
                                Ring = 1;
                        }
                        ring_chase_lights(150);
                        Ring++;
                        if (Ring >= 19) {
                                Ring = 1;
                        }
                }else if (Ring_Display == 3) {
                        if ((Ring < 1) || (Ring > 8)) {
                                Ring = 1;
                        }
                        ring_loop(150);
                        Ring++;
                        if (Ring >= 9) {
                                Ring = 1;
                        }
                }else if (Ring_Display == 4) {
                        ring_lights_random(150);
                }else if (Ring_Display == 5) {
                        if ((Ring < 1) || (Ring > 14)) {
                                Ring = 1;
                        }
                        ring_lights_snake(150);
                        Ring++;
                        if (Ring >= 14) {
                                Ring = 1;
                        }
                }else if (Ring_Display == 6) {
                        ring_lights_random_triangle(150);
                }else if (Ring_Display == 7) {
                        if ((Ring < 1) || (Ring > 9)) {
                                Ring = 1;
                        }
                        ring_lights_triangle(500);
                        Ring++;
                        if (Ring > 9) {
                                Ring = 1;
                        }
                }
        }else{
                if (client) {                 // If a new client connects,
                        clientRequest(client);
                }
                if(nothing_to_do) {
                        fill_rainbow(ledsChevron,NUM_LEDS_CHEVRONS, thishue, deltahue);
                        fill_rainbow(ledsChevronFINAL,NUM_LEDS_CHEVRONS_FINAL, thishue, deltahue);
                        fill_rainbow(ledsRamps,NUM_LEDS_RAMPS, thishue, deltahue);

                        EVERY_N_MILLISECONDS( 20 ) {
                                thishue++;
                        }                                   // slowly cycle the "base color" through the rainbow
                        FastLED.show();
                }
        }
}

void initSettings(){
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.println("Place this IP address into a browser window");
        // make an ip address you can read
        IPAddress myIP = WiFi.localIP();
        ipStr = String(myIP[0])+"."+String(myIP[1])+"."+String(myIP[2])+"."+String(myIP[3]);
}

void setupPinModes(){
        pinMode(GateDirPin, OUTPUT);  // set Stepper direction pin mode
        pinMode(GateStepPin, OUTPUT); // set Stepper step mode
        pinMode(GateEnablePin, OUTPUT); // set Stepper enable pin
        pinMode(GateMicroStep1Pin, OUTPUT); //set Microstep 1 config
        pinMode(GateMicroStep2Pin, OUTPUT); //set Microstep 2 config
        pinMode(GateMicroStep3Pin, OUTPUT); //set Microstep 3 config
        pinMode(ChevronDirPin, OUTPUT);  // set Stepper direction pin mode
        pinMode(ChevronStepPin, OUTPUT); // set Stepper step mode
        pinMode(ChevronEnablePin, OUTPUT); // set Stepper enable pin
        pinMode(ChevronMicroStep1Pin, OUTPUT); //set Microstep 1 config
        pinMode(ChevronMicroStep2Pin, OUTPUT); //set Microstep 2 config
        pinMode(ChevronMicroStep3Pin, OUTPUT); //set Microstep 3 config

        pinMode(LED, OUTPUT);     // ready LED

        digitalWrite(GateMicroStep1Pin, HIGH); // Initialized with microGateStepPing off
        digitalWrite(GateMicroStep2Pin, HIGH); // Initialized with microGateStepPing off
        digitalWrite(GateMicroStep3Pin, HIGH); // Initialized with microGateStepPing off
        digitalWrite(ChevronMicroStep1Pin, HIGH); // Initialized with microGateStepPing off
        digitalWrite(ChevronMicroStep2Pin, HIGH); // Initialized with microGateStepPing off
        digitalWrite(ChevronMicroStep3Pin, HIGH); // Initialized with microGateStepPing off

}

void resetESPdaily(){
        // once a day do a restart if noInit in not false is. (sometimes the esp hangs up)
        if (formattedTime=="00:00:02" && noInit == true) {
                ESP.restart();
        }
}

void blink(int blinks) {
        for (int i = 0; i <= blinks; i++) {
                digitalWrite(LED, HIGH);
                delay(300);
                digitalWrite(LED, LOW);
                delay(300);
        }
}
/*** lEDS ****/
//Clears the data for all configured ledstrip
void  ClearAllLedData() {
        for (word ledNr = 0; ledNr < NUM_LEDS_CHEVRONS; ledNr++) {
                setPixel(1,ledNr,0,0,0);
        }
        setPixel(2,0,0,0,0);
        for (word ledNr = 0; ledNr < NUM_LEDS_RAMPS; ledNr++) {
                setPixel(3,ledNr,0,0,0);
        }
        FastLED.show();
}

/********/
void GaterollBACKWARD(int doSteps) {
        digitalWrite(GateEnablePin, LOW);
        digitalWrite(GateDirPin, LOW);
        for (int i=1; i <= doSteps; i++) {
                currPos++;
//TODO test fin de position
                digitalWrite(GateStepPin, HIGH);
                delayMicroseconds(motorSpeed);
                digitalWrite(GateStepPin,LOW );
                delayMicroseconds(motorSpeed);
        }
        digitalWrite(GateEnablePin, HIGH);
        if (debugPrint ==true) {
                Serial.println("Down to position " + String(currPos));
        }
        GatestopRoll();
}
void GaterollFORWARD(int doSteps) {
        digitalWrite(GateEnablePin, LOW);
        digitalWrite(GateDirPin, HIGH);
        for (int i=1; i <= doSteps; i++) {
                currPos--;
//TODO test fin de position
                digitalWrite(GateStepPin, HIGH);
                delayMicroseconds(motorSpeed);
                digitalWrite(GateStepPin,LOW );
                delayMicroseconds(motorSpeed);

        }
        digitalWrite(GateEnablePin, HIGH);
        if (debugPrint ==true) {
                Serial.println("up to position " + String(currPos));
        }
        GatestopRoll();
}

void GatestopRoll(){
        // write current position to EEprom
        if (debugPrint ==true) {
                Serial.println("Stop");
        }
        digitalWrite(GateEnablePin, LOW);
        digitalWrite(GateDirPin, HIGH);
        delay(100);
        digitalWrite(GateEnablePin, HIGH);
}

/********/
void ChevronrollBACKWARD(int doSteps) {
        digitalWrite(ChevronEnablePin, LOW);
        digitalWrite(ChevronDirPin, LOW);
        for (int i=1; i <= doSteps; i++) {
                currPos++;
//TODO test fin de position
                digitalWrite(ChevronStepPin, HIGH);
                delayMicroseconds(motorSpeed);
                digitalWrite(ChevronStepPin,LOW );
                delayMicroseconds(motorSpeed);
        }
        digitalWrite(ChevronEnablePin, HIGH);
        if (debugPrint ==true) {
                Serial.println("Down to position " + String(currPos));
        }
        ChevronstopRoll();
}
void ChevronrollFORWARD(int doSteps) {
        digitalWrite(ChevronEnablePin, LOW);
        digitalWrite(ChevronDirPin, HIGH);
        for (int i=1; i <= doSteps; i++) {
                currPos--;
//TODO test fin de position
                digitalWrite(ChevronStepPin, HIGH);
                delayMicroseconds(motorSpeed);
                digitalWrite(ChevronStepPin,LOW );
                delayMicroseconds(motorSpeed);

        }
        digitalWrite(ChevronEnablePin, HIGH);
        if (debugPrint ==true) {
                Serial.println("up to position " + String(currPos));
        }
        ChevronstopRoll();
}

void ChevronstopRoll(){
        // write current position to EEprom
        if (debugPrint ==true) {
                Serial.println("Stop");
        }
        digitalWrite(ChevronEnablePin, LOW);
        digitalWrite(ChevronDirPin, HIGH);
        delay(100);
        digitalWrite(ChevronEnablePin, HIGH);
}
/*******************/

void setupOTA(){
        ArduinoOTA.setHostname("ESP32_Stepper"); // Hostname for OTA
        ArduinoOTA.setPassword(my_OTA_PW);   // set in credidentials.h
        ArduinoOTA
        .onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                        type = "sketch";
                else // U_SPIFFS
                        type = "filesystem";

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Serial.println("Start updating " + type);
        })
        .onEnd([]() {
                Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

        ArduinoOTA.begin(); // Start OTA
}

/*****************************/

void fillAll(int r, int g, int b){
        ClearAllLedData();
        FastLED.show();
        for (uint32_t i = 0; i < NUM_LEDS_CHEVRONS; i++) {
                setPixel(1,i, r, g, b);
        }
        for (uint32_t i = 0; i < NUM_LEDS_CHEVRONS_FINAL; i++) {
                setPixel(2,i, r, g, b);
        }
        for (uint32_t i = 0; i < NUM_LEDS_RAMPS; i++) {
                setPixel(3,i, r, g, b);
        }
        FastLED.show();
}

void setPixel(int ledstrip, int num, int r, int g, int b)
{
        if(ledstrip==1)
                ledsChevron[num].setRGB(r,g,b);
        else if(ledstrip==2)
                ledsChevronFINAL[num].setRGB(r,g,b);
        else
                ledsRamps[num].setRGB(r,g,b);
}

void ledChevron(int num, int etat){
        if(etat == LOW) {
                if(num<=3) {
                        ledsChevron[num].setRGB(0,0,0);
                }else if(num>=5) {
                        ledsChevron[num-1].setRGB(0,0,0);
                }else{
                        ledsChevronFINAL[0].setRGB(0,0,0);
                }
        }else{
                if(num<=3) {
                        ledsChevron[num].setRGB(R,G,B);
                }else if(num>=5) {
                        ledsChevron[num-1].setRGB(R,G,B);
                }else{
                        ledsChevronFINAL[0].setRGB(R,G,B);
                }
        }
        FastLED.show();
}

void ledRamp(int num, int etat){
        if(etat == LOW)
                ledsRamps[num].setRGB(0,0,0);
        else
                ledsRamps[num].setRGB(255,255,255);
        FastLED.show();
}

void ledRamp(int etat){
        if(etat==HIGH) {
                for(int i=0; i<(ramps_led_size-1); i+=2) {
                        ledRamp(ramps_led[i],etat);
                        ledRamp(ramps_led[i+1],etat);
                        FastLED.show();
                        delay(200);
                }
        }else{
                for(int i=0; i<(ramps_led_size-1); i+=2) {
                        ledRamp(ramps_led[7-i],etat);
                        ledRamp(ramps_led[7-(i+1)],etat);
                        FastLED.show();
                        delay(200);
                }
        }
}

/******* STARGATE ************/
void dial(int Chevron) {
        Serial.print("Chevron ");
        Serial.print(Chevron);
        Serial.println(" Encoded");
        int Steps_Turn = 0;
        if (Chevron == 1) {
                Steps_Turn = round(Step_Per_Symbol * (Address[(Chevron - 1)] - 1));
                GaterollBACKWARD(Steps_Turn);
        }else{
                Steps_Turn = Address[(Chevron - 1)] - Address[(Chevron - 2)];
                if ((Chevron % 2) == 0) {
                        if  (Steps_Turn < 0) {
                                Steps_Turn = Steps_Turn * -1;
                        }else{
                                Steps_Turn = 39 - Steps_Turn;
                        }
                        Steps_Turn = round(Step_Per_Symbol * Steps_Turn);
                        GaterollFORWARD(Steps_Turn);
                }else{

                        if  (Steps_Turn < 0) {
                                Steps_Turn = 39 + Steps_Turn;
                        }
                        Steps_Turn = round(Step_Per_Symbol * Steps_Turn);
                        GaterollBACKWARD(Steps_Turn);
                }
        }
        GatestopRoll();
        if ((Dialling == 9) && (Address[(Chevron - 1)] != 1)) {
                for (int tmp_fail_safe_gate = 0; tmp_fail_safe_gate <= 100; tmp_fail_safe_gate++) {
                        int tmp_fail_safe_chevron = random(7);
                        ledChevron(Chevrons[tmp_fail_safe_chevron], LOW);
                        GaterollBACKWARD(5*16);
                        GatestopRoll();
                        ledChevron(Chevrons[tmp_fail_safe_chevron], HIGH);
                }
                for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron++) {
                        ledChevron(Chevrons[tmp_chevron], LOW);
                }
                Dialling = 10;
        }else{
                Serial.print("Chevron ");
                Serial.print(Chevron);
                Serial.println(" Locked");
                ledChevron(Chevron_Locked, HIGH);
                ChevronrollBACKWARD(Chevron_Step);
                ChevronstopRoll();
        }
        if (Dialling < Address_Length) {
                delay(ChevronLocked);
                ChevronrollFORWARD(Chevron_Step);
                ChevronstopRoll();
                ledChevron(Chevron_Locked, LOW);
                delay(20);
                if ((Address_Length < 8) || (Dialling < 4)) {
                        ledChevron(Chevrons[(Chevron - 1)], HIGH);
                }else if ((Address_Length == 8) && ((Dialling >= 4) && (Dialling <= Address_Length))) {
                        if (Dialling == 4) {
                                ledChevron(Chevrons[(6)], HIGH);
                        }else if (Dialling == 5) {
                                ledChevron(Chevrons[(3)], HIGH);
                        }else if (Dialling == 6) {
                                ledChevron(Chevrons[(4)], HIGH);
                        }else if (Dialling == 7) {
                                ledChevron(Chevrons[(5)], HIGH);
                        }
                }else if ((Address_Length == 9) && ((Dialling >= 4) && (Dialling <= Address_Length))) {
                        if (Dialling == 4) {
                                ledChevron(Chevrons[(6)], HIGH);
                        }else if (Dialling == 5) {
                                ledChevron(Chevrons[(7)], HIGH);
                        }else if (Dialling == 6) {
                                ledChevron(Chevrons[(3)], HIGH);
                        }else if (Dialling == 7) {
                                ledChevron(Chevrons[(4)], HIGH);
                        }else if (Dialling == 8) {
                                ledChevron(Chevrons[(5)], HIGH);
                        }
                }
        }
}

void test_calibrate() {
        digitalWrite(Calibrate_LED, HIGH);
        Serial.print("Reading 1 : ");
        int LDR_1 = analogRead(LDR);
        delay(2000);
        LDR_1 = analogRead(LDR);
        Serial.println(LDR_1);
        digitalWrite(Calibrate_LED, LOW);
        Serial.print("Reading 1 : ");
        LDR_1 = analogRead(LDR);
        delay(2000);
        LDR_1 = analogRead(LDR);
        Serial.println(LDR_1);

/**/
        digitalWrite(Calibrate_LED, HIGH);
        for (int i=0; i<100; i++) {
                GaterollBACKWARD(16);
                LDR_1 = analogRead(LDR);
                Serial.println(LDR_1);
                delay(200);
                if (LDR_1 > 100)
                        break;
        }
        digitalWrite(Calibrate_LED, LOW);


}

void calibrate() {
        if (Cal == 0) {
                digitalWrite(Calibrate_LED, HIGH);
                Serial.print("Reading 1 : ");
                int LDR_1 = analogRead(LDR);
                delay(2000);
                LDR_1 = analogRead(LDR);
                Serial.println(LDR_1);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 2 : ");
                int LDR_2 = analogRead(LDR);
                Serial.println(LDR_2);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 3 : ");
                int LDR_3 = analogRead(LDR);
                Serial.println(LDR_3);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 4 : ");
                int LDR_4 = analogRead(LDR);
                Serial.println(LDR_4);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 5 : ");
                int LDR_5 = analogRead(LDR);
                Serial.println(LDR_5);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 6 : ");
                int LDR_6 = analogRead(LDR);
                Serial.println(LDR_6);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 7 : ");
                int LDR_7 = analogRead(LDR);
                Serial.println(LDR_7);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 8 : ");
                int LDR_8 = analogRead(LDR);
                Serial.println(LDR_8);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 9 : ");
                int LDR_9 = analogRead(LDR);
                Serial.println(LDR_9);
                GaterollBACKWARD(CAL_STEP2);
                Serial.print("Reading 10 : ");
                int LDR_10 = analogRead(LDR);
                Serial.println(LDR_10);
                LDR_avg = (LDR_1 + LDR_2 + LDR_3 + LDR_4 + LDR_5 + LDR_6 + LDR_7 + LDR_8 + LDR_9 + LDR_10) / 10;

                //Calculate average ldr value and set calibration point 3% higher
                LDR_cal = LDR_avg + ((LDR_avg / 100) * Calibration_Percent);

                Serial.print("LDR Average : ");
                Serial.println(LDR_avg);
                Serial.print("LDR Calibrate Point : ");
                Serial.println(LDR_cal);
                GatestopRoll();
                Cal = 1;
        }else if (Cal == 1) {
                digitalWrite(Calibrate_LED, HIGH);
                GaterollFORWARD(CAL_STEP1);
                GatestopRoll();
                int tmp_cal = analogRead(LDR);
                if(tmp_cal > LDR_cal) {
                        Serial.print("LDR Calibrated : ");
                        Serial.println(tmp_cal);
                        Cal = 2;
                        for (int tmp_chevron1 = 0; tmp_chevron1 < 9; tmp_chevron1++) {
                                ledChevron(Chevrons[tmp_chevron1], HIGH);
                        }
                        delay(1000);
                        for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron++) {
                                ledChevron(Chevrons[tmp_chevron], LOW);
                        }
                }
        }else{
                digitalWrite(Calibrate_LED, HIGH);
                GaterollFORWARD(CAL_STEP1);
                GatestopRoll();
                Step_increment++;
                int tmp_cal = analogRead(LDR);
                if((tmp_cal > LDR_cal) && (Step_increment > 30)) {
                        // 39 symbols
                        Step_Per_Symbol = Step_increment / 39;
                        Serial.print("Steps Calculate : ");
                        Serial.println(Step_Per_Symbol);
                        Serial.print("Total Steps : ");
                        Serial.println(Step_increment);
                        digitalWrite(Calibrate_LED, LOW);
                        Cal = 3;
                        Dialling = 1;
                        for (int tmp_chevron1 = 0; tmp_chevron1 < 9; tmp_chevron1++) {
                                ledChevron(Chevrons[tmp_chevron1], HIGH);
                        }
                        delay(1000);
                        for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron++) {
                                ledChevron(Chevrons[tmp_chevron], LOW);
                        }
                        delay(4000);
                }else if (Step_increment == 133) {
                        ledChevron(Ring_Chevrons[5], HIGH);
                }else if (Step_increment == 266) {
                        ledChevron(Ring_Chevrons[6], HIGH);
                }else if (Step_increment == 399) {
                        ledChevron(Ring_Chevrons[7], HIGH);
                }else if (Step_increment == 532) {
                        ledChevron(Ring_Chevrons[8], HIGH);
                }else if (Step_increment == 665) {
                        ledChevron(Ring_Chevrons[0], HIGH);
                }else if (Step_increment == 798) {
                        ledChevron(Ring_Chevrons[1], HIGH);
                }else if (Step_increment == 931) {
                        ledChevron(Ring_Chevrons[2], HIGH);
                }else if (Step_increment == 1064) {
                        ledChevron(Ring_Chevrons[3], HIGH);
                }
        }
}

void ring_lights(int Ring_Delay) {
        ledChevron(Ring_Chevrons[(Ring - 1)], HIGH);
        delay(20);
        if (Ring == 1) {
                ledChevron(Ring_Chevrons[(8)], LOW);
        }else{
                ledChevron(Ring_Chevrons[(Ring - 2)], LOW);
        }
        delay(Ring_Delay);
}

void ring_chase_lights(int Ring_Delay) {
        if (Ring < 10) {
                ledChevron(Ring_Chevrons[(Ring - 1)], HIGH);
        }else{
                ledChevron(Ring_Chevrons[(Ring - 10)], LOW);
        }
        delay(Ring_Delay);
}

void ring_loop(int Ring_Delay) {
        if (Ring == 1) {
                ledChevron(Ring_Chevrons[0], HIGH);
                ledChevron(Ring_Chevrons[8], HIGH);
                ledChevron(Ring_Chevrons[1], LOW);
                ledChevron(Ring_Chevrons[7], LOW);
        }else if ((Ring == 2) || (Ring == 8)) {
                ledChevron(Ring_Chevrons[1], HIGH);
                ledChevron(Ring_Chevrons[7], HIGH);
                ledChevron(Ring_Chevrons[0], LOW);
                ledChevron(Ring_Chevrons[2], LOW);
                ledChevron(Ring_Chevrons[6], LOW);
                ledChevron(Ring_Chevrons[8], LOW);
        }else if ((Ring == 3) || (Ring == 7)) {
                ledChevron(Ring_Chevrons[2], HIGH);
                ledChevron(Ring_Chevrons[6], HIGH);
                ledChevron(Ring_Chevrons[1], LOW);
                ledChevron(Ring_Chevrons[3], LOW);
                ledChevron(Ring_Chevrons[5], LOW);
                ledChevron(Ring_Chevrons[7], LOW);
        }else if ((Ring == 4) || (Ring == 6)) {
                ledChevron(Ring_Chevrons[3], HIGH);
                ledChevron(Ring_Chevrons[5], HIGH);
                ledChevron(Ring_Chevrons[2], LOW);
                ledChevron(Ring_Chevrons[4], LOW);
                ledChevron(Ring_Chevrons[6], LOW);
        }else if (Ring == 5) {
                ledChevron(Ring_Chevrons[4], HIGH);
                ledChevron(Ring_Chevrons[3], LOW);
                ledChevron(Ring_Chevrons[5], LOW);
        }
        delay(Ring_Delay);
}

void ring_lights_random(int Ring_Delay){
        for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron++) {
                ledChevron(Chevrons[tmp_chevron], LOW);
        }
        ledChevron(Ring_Chevrons[random(9)], HIGH);
        delay(Ring_Delay);
}

void ring_lights_snake(int Ring_Delay){
        int tmp_Ring = Ring - 4;
        if (tmp_Ring <= 0) {
                tmp_Ring = 9+ tmp_Ring;
        }
        ledChevron(Ring_Chevrons[(tmp_Ring - 1)], LOW);
        if (Ring <= 9) {
                ledChevron(Ring_Chevrons[(Ring - 1)], HIGH);
        }
        delay(Ring_Delay);
}

void ring_lights_random_triangle(int Ring_Delay){
        for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron++) {
                ledChevron(Chevrons[tmp_chevron], LOW);
        }
        int tmp_Ring = random(1, 9);
        int tmp_Ring_2 = tmp_Ring - 3;
        int tmp_Ring_3 = tmp_Ring + 3;
        if (tmp_Ring_2 <= 0 ) {
                tmp_Ring_2 = 9 + tmp_Ring_2;
        }
        if (tmp_Ring_3 >= 10 ) {
                tmp_Ring_3 = tmp_Ring_3 - 9;
        }
        ledChevron(Ring_Chevrons[(tmp_Ring - 1)], HIGH);
        ledChevron(Ring_Chevrons[(tmp_Ring_2 - 1)], HIGH);
        ledChevron(Ring_Chevrons[(tmp_Ring_3 - 1)], HIGH);
        delay(Ring_Delay);
}

void ring_lights_triangle(int Ring_Delay){
        for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron++) {
                ledChevron(Chevrons[tmp_chevron], LOW);
        }
        int tmp_Ring_2 = Ring - 3;
        int tmp_Ring_3 = Ring + 3;
        if (tmp_Ring_2 <= 0 ) {
                tmp_Ring_2 = 9 + tmp_Ring_2;
        }
        if (tmp_Ring_3 >= 10 ) {
                tmp_Ring_3 = tmp_Ring_3 - 9;
        }
        ledChevron(Ring_Chevrons[(Ring - 1)], HIGH);
        ledChevron(Ring_Chevrons[(tmp_Ring_2 - 1)], HIGH);
        ledChevron(Ring_Chevrons[(tmp_Ring_3 - 1)], HIGH);
        delay(Ring_Delay);
}
/******* STARGATE ************/

String outputRamp_LightsState = "off";
String outputChevron_LightsState = "off";
String outputRAINBOW_LightsState = "off";

void clientRequest(WiFiClient client)
{
        Serial.println("==========================");
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = ""; // make a String to hold incoming data from the client
        while (client.connected()) { // loop while the client's connected
                if (client.available()) { // if there's bytes to read from the client,
                        char c = client.read(); // read a byte, then
                        Serial.write(c); // print it out the serial monitor
                        header += c;
                        if (c == '\n') { // if the byte is a newline character
                                // if the current line is blank, you got two newline characters in a row.
                                // that's the end of the client HTTP request, so send a response:
                                if (currentLine.length() == 0) {
                                        if (header.indexOf("GET /") >= 0) {
                                                String URI = midString(header,"GET ", " ");
                                                if(loadFromSpiffs(URI,client)) break;
                                        }
                                        if (header.indexOf("POST /dialstatus") >= 0) {
                                                Serial.print("Dialling : "); Serial.print(Dialling);
                                                client.println(F("HTTP/1.1 204 No Content"));
                                                client.println(F("Content-Type: text/html"));
                                                client.println(F("Content-Length: 0"));
                                                client.println(F("Connection: close"));
                                                client.println();
                                                client.flush();
                                                client.stop();
                                                break;
                                                /*
                                                   if DialProgram.is_dialing:
                                                   self.send_response(200, '1')
                                                   else:
                                                   self.send_response(204, '0')
                                                   return
                                                 */
                                        }
                                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                                        // and a content-type so the client knows what's coming, then a blank line:
                                        client.println("HTTP/1.1 200 OK");
                                        client.println("Content-type:text/html");
                                        client.println("Connection: close");
                                        client.println();
// turns the GPIOs on and off
                                        if (header.indexOf("POST /RAINBOW/on") >= 0) {
                                                outputRAINBOW_LightsState="on";
                                                nothing_to_do=1;
                                        }
                                        if (header.indexOf("POST /RAINBOW/off") >= 0) {
                                                nothing_to_do=0;
                                                outputRAINBOW_LightsState="off";
                                                ClearAllLedData();
                                                FastLED.show();
                                        }
                                        /**/
                                        if (header.indexOf("POST /dialing/on/Abydos") >= 0) {
                                                memcpy(Address,Address_Abydos,7*sizeof(int));
                                                Address_Length=7;
                                                R=R_Abydos; G=G_Abydos; B=B_Abydos;
                                                dialing=1;
                                        }
                                        if (header.indexOf("POST /dialing/on/Asgard") >= 0) {
                                                memcpy(Address,Address_Asgard,8*sizeof(int));
                                                Address_Length=8;
                                                R=R_Asgard; G=G_Asgard; B=B_Asgard;
                                                dialing=1;
                                        }
                                        if (header.indexOf("POST /dialing/on/Destiny") >= 0) {
                                                memcpy(Address,Address_Destiny,9*sizeof(int));
                                                Address_Length=9;
                                                R=R_Destiny; G=G_Destiny; B=B_Destiny;
                                                dialing=1;
                                        }
                                        /**/
                                        if (header.indexOf("POST /CB/on") >= 0) {
                                                GaterollBACKWARD(39*Step_Per_Symbol);
                                        }
// turns the GPIOs on and off
                                        if (header.indexOf("POST /CF/on") >= 0) {
                                                GaterollFORWARD(39*Step_Per_Symbol);
                                        }
                                        // turns the GPIOs on and off
                                        if (header.indexOf("POST /Ramp_Lights/on") >= 0) {
                                                Serial.println("GPIO Ramp_Lights on");
                                                outputRamp_LightsState = "on";
                                                ledRamp(HIGH);
                                        } else if (header.indexOf("POST /Ramp_Lights/off") >= 0) {
                                                Serial.println("GPIO Ramp_Lights off");
                                                outputRamp_LightsState = "off";
                                                ledRamp(LOW);
                                        }
                                        // turns the GPIOs on and off
                                        if (header.indexOf("POST /Ramp_Chevrons/on") >= 0) {
                                                Serial.println("GPIO Ramp_Lights on");
                                                outputChevron_LightsState = "on";
                                                ClearAllLedData();
                                                FastLED.show();
                                                for (uint32_t i = 0; i < 4; i++) {
                                                        setPixel(1,i, R, G, B);
                                                        FastLED.show();
                                                        delay(200);
                                                }
                                                setPixel(2,0, R, G, B);
                                                FastLED.show();
                                                delay(200);
                                                for (uint32_t i = 4; i < NUM_LEDS_CHEVRONS; i++) {
                                                        setPixel(1,i, R, G, B);
                                                        FastLED.show();
                                                        delay(200);
                                                }
                                        } else if (header.indexOf("POST /Ramp_Chevrons/off") >= 0) {
                                                Serial.println("GPIO Ramp_Lights off");
                                                outputChevron_LightsState = "off";
                                                for (uint32_t i = NUM_LEDS_CHEVRONS-1; i > 3; i--) {
                                                        setPixel(1,i, 0, 0, 0);
                                                        FastLED.show();
                                                        delay(200);
                                                }
                                                setPixel(2,0, 0, 0, 0);
                                                FastLED.show();
                                                delay(200);
                                                for (uint32_t i = 3; i >= 0; i--) {
                                                        setPixel(1,i, 0, 0, 0);
                                                        FastLED.show();
                                                        delay(200);
                                                }
                                        } else if (header.indexOf("POST /calibrate") >= 0) {
                                                Cal=0;
                                                while(Cal!=2)
                                                        calibrate();
                                                Cal=3;
                                        } else if (header.indexOf("POST /update") >= 0) {
                                                //
                                                String line="";
                                                while (client.available() && (c = client.read())!=-1) {
                                                        Serial.write(c); // print it out the serial monitor
                                                        line += c;
                                                }
                                                Serial.print("==>"); Serial.println(line);
                                                // {"anim":2,"sequence":[2,3,4,6,18,17,30]}
                                                String tmp = midString(line,"[","]");
                                                R=R_Abydos; G=G_Abydos; B=B_Abydos;
                                                dialing=1;
                                                Address_Length= 0;
                                                while(tmp.indexOf(",")!=-1) {
                                                        Address[ Address_Length ] = tmp.substring(0,tmp.indexOf(",")).toInt();
                                                        Address_Length++;
                                                        tmp = tmp.substring(tmp.indexOf(",")+1);
                                                }
                                                Address[ Address_Length ] = tmp.toInt();
                                                Address_Length++;
                                                dialing=1;
                                        }
                                        break;
                                } else { // if you got a newline, then clear currentLine
                                        currentLine = "";
                                }
                        } else if (c != '\r') { // if you got anything else but a carriage return character,
                                currentLine += c; // add it to the end of the currentLine
                        }
                }

        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
}

String midString(String str, String start, String finish){
        int locStart = str.indexOf(start);
        if (locStart==-1) return "";
        locStart += start.length();
        int locFinish = str.indexOf(finish, locStart);
        if (locFinish==-1) return "";
        return str.substring(locStart, locFinish);
}


bool loadFromSpiffs(String path, WiFiClient client){
        String dataType = "text/plain";
        if(path.endsWith("/")) path += "index.htm";

        if(!SPIFFS.exists(path)) return false;
        if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
        else if(path.endsWith(".svg")) dataType = "image/svg+xml";
        else if(path.endsWith(".html")) dataType = "text/html";
        else if(path.endsWith(".htm")) dataType = "text/html";
        else if(path.endsWith(".css")) dataType = "text/css";
        else if(path.endsWith(".js")) dataType = "application/javascript";
        else if(path.endsWith(".png")) dataType = "image/png";
        else if(path.endsWith(".gif")) dataType = "image/gif";
        else if(path.endsWith(".jpg")) dataType = "image/jpeg";
        else if(path.endsWith(".ico")) dataType = "image/x-icon";
        else if(path.endsWith(".xml")) dataType = "text/xml";
        else if(path.endsWith(".pdf")) dataType = "application/pdf";
        else if(path.endsWith(".zip")) dataType = "application/zip";
        File dataFile = SPIFFS.open(path.c_str(), "r");

        /*if (server.hasArg("download")) dataType = "application/octet-stream";
           if (server.streamFile(dataFile, dataType) != dataFile.size()) {
           }
         */
        //client.print(F("POST "));
        //client.print(path);
        client.println("HTTP/1.1 200 OK");
        client.print(F("Content-Type: "));
        client.println(dataType);
        client.print(F("Host: "));
        client.println(WiFi.localIP());
        if(!(path.endsWith(".css") || path.endsWith("/")) )
                client.println(F("Cache-Control: max-age=864000"));
        client.println(F("Connection: close"));
        client.print(F("Content-Length: "));
        client.println(dataFile.size());
        client.println();
        while (dataFile.available()) {
                client.write(dataFile.read());
        }
        client.flush();
        dataFile.close();
        client.stop();

        return true;
}
