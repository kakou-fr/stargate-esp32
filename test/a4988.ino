// This “sketch” by jed on https://forum.pololu.com 1 from one
// posted by decrDude at A4988 Dev Board
// also using the guide decrDude posted at
// A4983 and A4988 Getting Started Guide

// I’ve said it before and I’ll repeat it: MANY thanks to
// decrDude for his help teaching the rest of us how to use
// steppers, Pololu controller boards, and the Arduino
// together.

// jed’s changes:
// Special code to move 45 degrees at a time, wait 2 seconds,
// move another 45 degrees

#define stepPin 3
#define dirPin 2
#define enablePin 7
#define MicroStep1Pin 6
#define MicroStep2Pin 5
#define MicroStep3Pin 4

#define numMsInOneSec 1000
#define numMicroSecInOneMs 1000
#define stepPulseWidthInMicroSec 2
#define setupTimeInMicroSec 1

#define inputBufferSize 128

int serialCharIn;
char serialInString[inputBufferSize];
int serialInIndex = 0;

unsigned long timeBetweenInputPollsInMicroSec = ( (unsigned long)(numMsInOneSec /4) * numMicroSecInOneMs ); // 250 = 1/4th of a second
unsigned long timeBetweenStepsInMicroSec = (1 * numMicroSecInOneMs);

unsigned long loopCheck = 0;

boolean lineReady = false;
boolean successfullyParsed = false;
boolean currentDirection = false;
boolean shouldStep = true;

boolean speedChanged = true;

void setCurrentDirection(boolean dir)
{
        if(dir == false)
        {
                digitalWrite(dirPin, LOW);
        } else {
                digitalWrite(dirPin, HIGH);
        }
        currentDirection = dir;
        delayMicroseconds(setupTimeInMicroSec);
}

void changeDirection()
{
        setCurrentDirection(!currentDirection);
}

void enableStepper(int isEnabled)
{
        if(isEnabled)
        {
                digitalWrite(enablePin, LOW); // enable HIGH = stepper driver OFF
        } else {
                digitalWrite(enablePin, HIGH); // enable HIGH = stepper driver OFF
        }
// wait a few microseconds for the enable to take effect
// (That isn’t in the spec sheet I just added it for sanity.)
        delayMicroseconds(2);
}

void takeSingleStep()
{
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepPulseWidthInMicroSec);
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepPulseWidthInMicroSec);
        digitalWrite(stepPin, LOW);
}

void setFullStep()
{
        digitalWrite(MicroStep1Pin, LOW);
        digitalWrite(MicroStep2Pin, LOW);
        digitalWrite(MicroStep3Pin, LOW);
        delayMicroseconds(setupTimeInMicroSec);
}

void setHalfStep()
{
        digitalWrite(MicroStep1Pin, HIGH);
        digitalWrite(MicroStep2Pin, LOW);
        digitalWrite(MicroStep3Pin, LOW);
        delayMicroseconds(setupTimeInMicroSec);
}

void setQuarterStep()
{
        digitalWrite(MicroStep1Pin, LOW);
        digitalWrite(MicroStep2Pin, HIGH);
        digitalWrite(MicroStep3Pin, LOW);
        delayMicroseconds(setupTimeInMicroSec);
}

void setEighthStep()
{
        digitalWrite(MicroStep1Pin, HIGH);
        digitalWrite(MicroStep2Pin, HIGH);
        digitalWrite(MicroStep3Pin, LOW);
        delayMicroseconds(setupTimeInMicroSec);
}

void setSixteenthStep()
{
        digitalWrite(MicroStep1Pin, HIGH);
        digitalWrite(MicroStep2Pin, HIGH);
        digitalWrite(MicroStep3Pin, HIGH);
        delayMicroseconds(setupTimeInMicroSec);
}

/* we don’t need this code but might need to refer to it
 *********** start of commented code
   bool makeChangesBasedOnSerial()
   {
   boolean output = true;
   String tempString = serialInString;
   String hzStartString = “StepsInHz=”;
   if(tempString == “ChangeDirection\n”)
   {
   changeDirection();
   } else if (tempString == “EnableStepper\n”) {
   enableStepper(true);
   } else if (tempString == “DisableStepper\n”) {
   enableStepper(false);
   } else if (tempString == “FullStep\n”) {
   setFullStep();
   } else if (tempString == “HalfStep\n”) {
   setHalfStep();
   } else if (tempString == “QuarterStep\n”) {
   setQuarterStep();
   } else if (tempString == “EighthStep\n”) {
   setEighthStep();
   } else if (tempString == “SixteenthStep\n”) {
   setSixteenthStep();
   } else if (tempString == “SteppingOn\n”) {
   shouldStep = true;
   } else if (tempString == “SteppingOff\n”) {
   shouldStep = false;
   } else if (tempString.startsWith( hzStartString ) && tempString.endsWith("\n") ) {
   // must be less than 1,000,000
   String hzstr = tempString.substring(hzStartString.length(), tempString.length() -1);
   char hzCharArray[inputBufferSize];
   hzstr.toCharArray(hzCharArray, inputBufferSize);
   // Serial.println(hzstr);
   long parsed = atol(hzCharArray);
   // Serial.println(parsed);
   if(parsed != 0 && parsed < (numMicroSecInOneMsnumMsInOneSec) )
   {
   timeBetweenStepsInMicroSec = ((unsigned long)numMicroSecInOneMsnumMsInOneSec) / (unsigned long) parsed;
   = true;
   } else {
   output = false;
   }
   } else {
   output = false;
   }
   return output;
   }
 *************** end of commented code ************
 */

void setup()
{
// We set the enable pin to be an output
        pinMode(enablePin, OUTPUT); // then we set it HIGH so that the board is disabled until we
        pinMode(stepPin, OUTPUT);
        pinMode(dirPin, OUTPUT);

        pinMode(MicroStep1Pin, OUTPUT);
        pinMode(MicroStep2Pin, OUTPUT);
        pinMode(MicroStep3Pin, OUTPUT);

// get into a known state.
        enableStepper(false);
// we set the direction pin in an arbitrary direction.
        setCurrentDirection(false);
        setSixteenthStep();

        enableStepper(true);
// we set the direction pin in an arbitrary direction.
        setCurrentDirection(true);
        timeBetweenStepsInMicroSec = (1 * numMicroSecInOneMs); // set speed
// timeBetweenStepsInMicroSec = (3 * numMicroSecInOneMs); // slower speed
// timeBetweenStepsInMicroSec = (numMicroSecInOneMs / 2); // faster speed
}

void loop()
{
        int j;
        int loops, ramp;
        int rampspeed=4; // bigger numbers = quicker ramp

// we need to move 45 degrees until 360 degrees is done
        for (loops=0; loops<(360/45); loops++) {
                enableStepper(true);
// now we ramp up the stepper then move it smoothly
// 200 steps * 16 microstepping = 3,200 / 8 loops = 400
// so will ramp up for 50, speed for 300, ramp down for 50
                for (ramp=0; ramp<50; ramp++) {
// wait extra time
                        delay((50-ramp)/rampspeed);
                        takeSingleStep();
                        delayMicroseconds(timeBetweenStepsInMicroSec);
                }

                for(j=0; j<200; j++) {
                        takeSingleStep();
                        delayMicroseconds(timeBetweenStepsInMicroSec);
                }

                for (ramp=0; ramp<50; ramp++) {
                        // wait extra time
                        delay((ramp+1)/rampspeed);
                        takeSingleStep();
                        delayMicroseconds(timeBetweenStepsInMicroSec);
                }
// now disable the stepper and wait two seconds
                enableStepper(false);
                delay(2000); // in miliseconds
        }

// that’s all, folks!
        enableStepper(false);
        delay(8000); // delay before going again
}
