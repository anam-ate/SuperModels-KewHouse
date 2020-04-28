#include <Wire.h>
#include <SPI.h>
#include "AnamServo.h"
#include "sensor.h"
#include "buttonTrigger.h"
#include "chimney.h"
#include "bird.h"
#include "lights.h"

//#define DEBUG 1                 // comment this out when you want to remove all the print statements
//#define DEBUG_SEQUENCE_PICKER   // uncomment this to manually select sequences
//#define DEBUG_STATE_PICKER      // uncomment this to manually select states


//====================================================================================================================//
//===== Buttons
bool buttonPushed = false;



//====================================================================================================================//
//===== States
int state = -1;                 // keeps track of the states (-1 is default state (bird and button))
// 0 is a special state for chaning states in the sequence
// 1 and up are content states
int internalSequenceState = 0;  // keeps track of everyting within states
int timer;                      // timer for states (stores start time for states)
bool timerDone = false;


//====================================================================================================================//
//===== Sequences
int randomSeqPicker = 0;        // selected sequence
int routineIndex = 0;           // current index in the routine arrays (previously was x)
int RoutineA [19] = {22, 11, 19, 17, 20, 12, 18, 21, 27, 4, 31, 18, 10, 30, 12, 3, 3, 3, 3}; //23 sequence 1
int RoutineB [19] = {32, 19, 17, 9, 11, 13, 15, 7, 5, 8, 6, 4, 31, 18, 10, 30, 12, 33, 33}; //Sequence kevin 2//{32, 11, 17, 19, 20, 12, 18, 24, 17, 9, 29, 13, 11, 15, 28, 4, 31, 18, 10, 30, 12, 33, 33}; //23
int RoutineC [19] = {22, 11, 19, 17, 20, 12, 18, 21, 28, 4, 31, 18, 10, 30, 12, 3, 3, 3, 3}; //23 sequence 1
int lenRoutine = 19;
int RoutineTest [1] = {1};
//int RoutineA [4] = {1, 2, 3, 4};
//int RoutineB [4] = {3, 4, 5, 6};
//int RoutineC [4] = {4, 2, 6, 1};
//int lenRoutine = 4;



//====================================================================================================================//
//===== Birdy
//birdy(int _birdDuration, int _servo0, int _servo180, int _inc)
birdy sensorBirdySlow(6, 1000, 2000, 20); // slow flap
int birdState = 0; // flap bird state within range of sensor

// light colour value
int r = 255;
int g = 77;
int b = 0;
int w = 32;
//====================================================================================================================//
//===== Class instances
//AnamServo(String _servoName, int _id, int _relayPin, int _dataPin, int _servoSpeed, int _startPos, int _firstPos, int _secondPos)
AnamServo verticalServo("Vertical", 1, 7, 8, 1, 1300, 700, 1300); // 7,8 //12, 42
AnamServo slideServo("Slide", 2, 5, 6, 1, 1150, 550, 1150);  // 5,6

//Lights(int _numberofPixels, int _dataPinLight, int _rVal, int _gVal, int _bVal, int _wVal, int _pixelfadeSpeed)
Lights firePlace(1, 26, r, g, b, w, 1); //26
Lights parentsRoom(1, 28, r, g, b, w, 1); //28
Lights mainHouse(5, 24, r, g, b, w, 1); // 24
//Lights streetLight(5, r, g, 77, 0, 32, 1); // ? pin 

chimneySmoke chimney01(20);


//====================================================================================================================//
//===== Plinth lights
int ledPin = 11;    // leds inside plinth
int fadeValue = 0;
long flashTimer = 0;            // used if something goes wrong
bool plinthLightState = false;  // same



//====================================================================================================================//
//===== Helper functions

inline void debugPrint(const char* str)
{
#ifdef DEBUG
    Serial.println(F(str));
#endif
}

inline void debugPrint(int i)
{
#ifdef DEBUG
    Serial.println(i);
#endif
}

inline void debugPrint(float f)
{
#ifdef DEBUG
    Serial.println(f);
#endif
}

inline void wireTransfer(byte b)
{
    Wire.beginTransmission(20);
    Wire.write(b);
    Wire.endTransmission();
}

inline void stateMover(long t, int s)
{
    timer = t;
    state = s;
}

inline void stateCompleted()
{
    routineIndex++;
    state = 0;
    internalSequenceState = 0;
}


//====================================================================================================================//
//===== Setup Function
void setup()
{

    // Setup Serial and Wire comms
    Wire.begin();
    Serial.begin(19200);

    // Setup the birdy
    sensorBirdySlow.setup();

    // Setup distance sensor and button
    setupDistanceSensor();
    buttonTriggerSetup();

    // Class instance setups
    verticalServo.setup();
    slideServo.setup();
    chimney01.setup();
    firePlace.setup();
    parentsRoom.setup();
    mainHouse.setup();
    //streetLight.setup();
    
    // Plinth light
    fadeValue = 0; // start with no light
    pinMode(ledPin, OUTPUT);
    analogWrite(ledPin, fadeValue);

    // seed for random function
    randomSeed(analogRead(0));

    debugPrint("Ready...");
}


//====================================================================================================================//
//===== Loop Function
void loop()
{
    //==== Default State ===============================================================================================
    if (state == -1) // Default state - Bird sensor and Button code
    {

#ifdef DEBUG_SEQUENCE_PICKER

        if (Serial.available())
        {
            randomSeqPicker = Serial.parseInt();
            if (randomSeqPicker < 1) return;
            if (randomSeqPicker > 3) return;
            state = 0;
            analogWrite(ledPin, 150);
            debugPrint("sequence chosen");
            debugPrint("I received: ");
            debugPrint(randomSeqPicker);
        }

#endif

#ifndef DEBUG_SEQUENCE_PICKER
#ifdef DEBUG_STATE_PICKER

        if (Serial.available())
        {
            int temp = Serial.parseInt();
            if (temp < 1) return;
            if (temp > 33) return;
            routineIndex = 0;
            randomSeqPicker = 0;
            RoutineTest[0] = temp;
            lenRoutine = 1;
            state = 0;
        }

#endif
#endif

#ifndef DEBUG_SEQUENCE_PICKER
#ifndef DEBUG_STATE_PICKER

        // Check if button is pushed
        if (buttonTriggerSensor() == true)
        {
            buttonPushed = true;
            debugPrint("button pushed");
        }

        // only do this if the button is pushed and the routines have ended or the bird sensor state isn't triggered
        if (buttonPushed == true && birdState == 0)
        {
            buttonPushed = false;

            analogWrite(ledPin, 150);  // turn plinth lights on
            randomSeqPicker = 1 + random(1, 99) % 3; // CHOOSE A ROUTINE RANDOML
            state = 0; // change state sequence choice state

            debugPrint("button pushed");
            debugPrint("random picked: ");
            debugPrint(randomSeqPicker);
            debugPrint("sequence chosen");
        }

        //===============================SENSOR TRIGGER BIRD========================================
        else if (birdState == 0 && readDistance() < 20) // if the bells haven't started and the sensor reads
        {
            sensorBirdySlow.birdStart();
            birdState = 3;

            wireTransfer(1); // send trigger to stop construction track

            debugPrint("sent med birdies to play");
            debugPrint("birds flap");
            debugPrint(readDistance());
        }


        else if (birdState == 3 ) // bell duration extra to account for milliseconds of bell sound runover
        {

            bool birdDone = sensorBirdySlow.birdSequence(); // play bird movement
            if (birdDone == true)
            {
                wireTransfer(2);  // send trigger to stop construction track

                debugPrint("end birds");
                birdState = 4;
            }
        }
        else if (birdState == 4) // else if timer run out and bellstate == 1
        {
            debugPrint("birds over");

            birdState = 0;
        }

#endif
#endif

    } // ENDIF state == -1


    //==== Chose Sequence State ========================================================================================
    else if (state == 0)
    {
        // check if sequence is completed
        if (routineIndex >= lenRoutine) {
            // reset everything and return to default state
            debugPrint("Sequence completed. Returning to default state.");
            state = -1;
            routineIndex = 0;
        }
        else
        {
            // go to the next state in the sequence
            if (randomSeqPicker == 1)
            {
                debugPrint("Trigger routine A");
                stateMover(millis(), RoutineA[routineIndex]);
            }
            else if (randomSeqPicker == 2)
            {
                debugPrint("Trigger routine B");
                stateMover(millis(), RoutineB[routineIndex]);
            }
            else if (randomSeqPicker == 3)
            {
                debugPrint("Trigger routine C");
                stateMover(millis(), RoutineC[routineIndex]);
            }
            else if (randomSeqPicker == 0) // for testing purposes
            {
                debugPrint("Trigger test routine");
                stateMover(millis(), RoutineTest[routineIndex]);
            }
            else
            {
                // shouldn't happen...
                // but if it does, return to default state
                debugPrint("This shouldn't have happened. randomSeqPicker out of random range.");
                if (routineIndex == 0)
                {
                    state = -1;
                    routineIndex = 0;
                }
                else
                {
                    // flash the plinth lights to indicate an error
                    if (millis() >= flashTimer + 1000)
                    {
                        flashTimer = millis();
                        analogWrite(ledPin, plinthLightState ? 255 : 0);
                    }
                }
            }
        }

    } // ENDIF state == 0



    //==================================================================================================================
    //==== Content States Start ========================================================================================
    //==================================================================================================================

    //==== THE BIRD STATE MEDIUM =======================================================================================
    else if (state == 1)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            debugPrint("timer up");

            sensorBirdySlow.birdStart();
            internalSequenceState = 1;
            wireTransfer(3);  // send trigger to stop construction track
            debugPrint("send sequence 1 to play");
            debugPrint("bird start");
        }

        else if (internalSequenceState == 1)
        {
            bool birdDone = sensorBirdySlow.birdSequence();
            if (birdDone == true)
            {
                debugPrint("end birds");
                debugPrint("bird done");
                debugPrint("timer is up get out of state 3");

                stateCompleted();
            }
        }
    }



    //==== KEW HOUSE OPENS AND THEN CLOSES AFTER TIME STATE ============================================================
    else if (state == 2)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            verticalServo.openStart();
            slideServo.openStart();

            debugPrint("open kew");

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool servoVertDone = verticalServo.callUpdate();
            bool servoSlideDone = slideServo.callUpdate();
            if (servoVertDone == true && servoSlideDone == true)
            {
                internalSequenceState = 2;
                timer = millis();
            }
        }
        else if (internalSequenceState == 2 && millis() >= timer + 8 * 1000)
        {
            verticalServo.closeStart();
            slideServo.closeStart();
            internalSequenceState = 3;
        }
        else if (internalSequenceState == 3)
        {
            bool servoVertDone = verticalServo.callUpdate();
            bool servoSlideDone = slideServo.callUpdate();
            if (servoVertDone == true && servoSlideDone == true)
            {
                debugPrint("End house closed");

                stateCompleted();
            }
        }

    }



    //==== LIGHTS OFF STATE ============================================================================================
    else if (state == 3)
    {
        debugPrint("turn plinth lights off");

        analogWrite(ledPin, 0);
        wireTransfer(4);  // 

        debugPrint("sequence 01 stop");
        debugPrint("resetting");

        stateCompleted();
    }



    //==== CHIMNEY IN ISOLATION=========================================================================================
    else if (state == 4)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            debugPrint("chimney start");
            debugPrint("start chimney ");

            chimney01.chimneyStart();

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool chimneyDone = chimney01.chimneySequence();
            if (chimneyDone == true)
            {
                debugPrint("chimney done");

                stateCompleted();
            }
        }
    }



    //==== VERTICAL HOUSE OPENS=========================================================================================
    else if (state == 5)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            debugPrint("open vertical house");

            verticalServo.openStart();
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool servoVertDone = verticalServo.callUpdate();
            if (servoVertDone == true)
            {
                debugPrint("open vertical house finished");

                stateCompleted();
            }
        }
    }



    //==== VERTICAL HOUSE CLOSE ========================================================================================
    else if (state == 6)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {

            debugPrint("close vertical house");

            verticalServo.closeStart();
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool servoVertDone = verticalServo.callUpdate();
            if (servoVertDone == true)
            {
                debugPrint("close vertical house finished");

                stateCompleted();
            }
        }
    }



    //==== HORIZONTAL HOUSE OPENS=======================================================================================
    else if (state == 7)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            debugPrint("open horizontal house");

            slideServo.openStart();
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool servoSlideDone = slideServo.callUpdate();
            if (servoSlideDone == true)
            {
                debugPrint("open horizontal house finished");

                stateCompleted();
            }
        }
    }



    //==== HORIZONTAL HOUSE CLOSE ======================================================================================
    else if (state == 8)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            debugPrint("close horizontal house");

            slideServo.closeStart();
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool servoSlideDone = slideServo.callUpdate();
            if (servoSlideDone == true)
            {
                debugPrint("closed");

                stateCompleted();
            }
        }

    }



    //==== FIRE PLACE ON ===============================================================================================
    else if (state == 9)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            firePlace.fadeUpStart(0, 8);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = firePlace.lightSequence();
            if (lightsFadeUpDone == true)
            {
                debugPrint("living room on");

                stateCompleted();
            }
        }
    }



    //==== FIRE PLACE OFF ==============================================================================================
    else if (state == 10)
    {

        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            firePlace.fadeDownStart(0, 8);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = firePlace.lightSequence();
            if (lightsFadeDownDone == true)
            {
                debugPrint("living room off");

                stateCompleted();
            }
        }
    }



    //==== PARENTS BEDROOM ON ==========================================================================================
    else if (state == 11)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            parentsRoom.fadeUpStart(0, 12);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = parentsRoom.lightSequence();
            if (lightsFadeUpDone == true)
            {
                debugPrint("parents bedroom light on");

                stateCompleted();
            }
        }
    }



    //==== PARENTS BEDROOM OFF =========================================================================================
    else if (state == 12)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            parentsRoom.fadeDownStart(0, 10);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = parentsRoom.lightSequence();
            if (lightsFadeDownDone == true)
            {
                debugPrint("parents bedroom light off");

                stateCompleted();
            }
        }
    }



    //==== PLAYROOM ON =================================================================================================
    else if (state == 13)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeUpStart(0, 15);
            mainHouse.fadeUpStart(1, 15);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = mainHouse.lightSequence();
            bool lightsFadeUpDone2 = mainHouse.lightSequence();
            if (lightsFadeUpDone == true && lightsFadeUpDone2 == true)
            {
                debugPrint("playroom light on");

                stateCompleted();
            }
        }
    }



    //==== PLAYROOM OFF ================================================================================================
    else if (state == 14)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeDownStart(0, 15);
            mainHouse.fadeDownStart(1, 15);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = mainHouse.lightSequence();

            if (lightsFadeDownDone == true )
            {
                debugPrint("playroom light off");

                stateCompleted();
            }
        }
    }



    //==== WORKSHOP ON =================================================================================================
    else if (state == 15)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeUpStart(2, 15);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = mainHouse.lightSequence();

            if (lightsFadeUpDone == true)
            {
                debugPrint("workshop light on");

                stateCompleted();
            }
        }
    }



    //==== WORKSHOP OFF ================================================================================================
    else if (state == 16)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeDownStart(2, 15);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = mainHouse.lightSequence();

            if (lightsFadeDownDone == true)
            {
                debugPrint("workshop light off");

                stateCompleted();
            }
        }
    }



    //==== KITCHEN ON ==================================================================================================
    else if (state == 17)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeUpStart(3, 25);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = mainHouse.lightSequence();

            if (lightsFadeUpDone == true)
            {
                debugPrint("kitchen light on");

                stateCompleted();
            }
        }
    }



    //==== KITCHEN OFF =================================================================================================
    else if (state == 18)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeDownStart(3, 15);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = mainHouse.lightSequence();

            if (lightsFadeDownDone == true)
            {

                debugPrint("kitchen light off");

                stateCompleted();
            }
        }
    }



    //==== KIDS BEDROOM ON =============================================================================================
    else if (state == 19)
    {

        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {

            mainHouse.fadeUpStart(4, 10);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = mainHouse.lightSequence();

            if (lightsFadeUpDone == true)
            {

                debugPrint("kids bedroom light on");

                stateCompleted();
            }
        }
    }



    //==== KIDS BEDROOM OFF ============================================================================================
    else if (state == 20)
    {

        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {

            mainHouse.fadeDownStart(4, 10);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = mainHouse.lightSequence();

            if (lightsFadeDownDone == true)
            {
                debugPrint("kids bedroom light off");

                stateCompleted();
            }
        }
    }



    //==== ALL FADE ON =================================================================================================
    else if (state == 21)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            firePlace.fadeUpStart(0, 10);
            parentsRoom.fadeUpStart(0, 10);
            mainHouse.fadeUpStart(0, 10);
            mainHouse.fadeUpStart(1, 10);
            mainHouse.fadeUpStart(2, 10);
            mainHouse.fadeUpStart(3, 10);
            mainHouse.fadeUpStart(4, 10);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = firePlace.lightSequence();
            bool lightsFadeUpDone2 = parentsRoom.lightSequence();
            bool lightsFadeUpDone3 = mainHouse.lightSequence();

            if (lightsFadeUpDone == true && lightsFadeUpDone2 == true && lightsFadeUpDone3 == true)
            {
                debugPrint("ALL LIGHTS ON");

                stateCompleted();
            }
        }
    }



    //==== ALL FADE OFF ================================================================================================
    else if (state == 22)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            firePlace.fadeDownStart(0, 10);
            parentsRoom.fadeDownStart(0, 10);
            mainHouse.fadeDownStart(0, 10);
            mainHouse.fadeDownStart(1, 10);
            mainHouse.fadeDownStart(2, 10);
            mainHouse.fadeDownStart(3, 10);
            mainHouse.fadeDownStart(4, 10);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = firePlace.lightSequence();
            bool lightsFadeUpDone2 = parentsRoom.lightSequence();
            bool lightsFadeUpDone3 = mainHouse.lightSequence();


            if (lightsFadeUpDone == true && lightsFadeUpDone2 == true && lightsFadeUpDone3 == true)
            {
                wireTransfer(3);  // send trigger to stop construction track

                debugPrint("sequence 01 start");
                debugPrint("ALL LIGHTS OFF");
                debugPrint("send 3 to trigger sequence 1 and 2");

                stateCompleted();
            }
        }
    }



    //==== 8 SECOND DEAD PAUSE =========================================================================================
    else if (state == 23)
    {
        if (internalSequenceState == 0 && millis() >= timer + 8 * 1000)
        {
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            debugPrint("8 second pause over");

            stateCompleted();
        }
    }



    //==== 5 SECOND DEAD PAUSE =========================================================================================
    else if (state == 24)
    {
        if (internalSequenceState == 0 && millis() >= timer + 5 * 1000)
        {
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            debugPrint("5 second pause over");

            stateCompleted();
        }
    }



    //==== 10 SECOND DEAD PAUSE ========================================================================================
    else if (state == 25)
    {
        if (internalSequenceState == 0 && millis() >= timer + 10 * 1000)
        {
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            debugPrint("10 second pause over");

            stateCompleted();
        }
    }



    //==== 3 SECOND DEAD PAUSE =========================================================================================
    else if (state == 26)
    {
        if (internalSequenceState == 0 && millis() >= timer + 3 * 1000)
        {
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            debugPrint("3 second pause over");

            stateCompleted();
        }
    }



    //==== KEW HOUSE OPENS PART SLIDE THEN VERTICAL THEN CLOSES SIMULTANEOUSLY =========================================
    else if (state == 27)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            slideServo.openStart();

            debugPrint("open kew");

            internalSequenceState = 1;
            timer = millis();
        }

        else if (internalSequenceState == 1)
        {
            bool servoSlideDone;
            bool servoVertDone;

            servoSlideDone = slideServo.callUpdate();
            if (millis() >= timer + 5 * 1000 && timerDone == false )
            {
                verticalServo.openStart();
                timerDone = true; // tell me when the 5 seconds is up

                debugPrint("timer done begin vertical servo");

            }
            if (timerDone == true)
            {
                servoVertDone = verticalServo.callUpdate();

                debugPrint("start vertical servo moving");


            }
            if (servoVertDone == true && servoSlideDone == true)
            {

                debugPrint("both open");

                internalSequenceState = 2;
                timerDone  = false; // reset telling me when the timer is done
                timer = millis();
            }

        }

        //PAUSE BEFORE CLOSE
        else if (internalSequenceState == 2 && millis() >= timer + 10 * 1000)
        {

            debugPrint("pause up");

            verticalServo.closeStart();
            slideServo.closeStart();
            internalSequenceState = 3;
        }
        else if (internalSequenceState == 3)
        {
            bool servoVertDone = verticalServo.callUpdate();
            bool servoSlideDone = slideServo.callUpdate();
            if (servoVertDone == true && servoSlideDone == true)
            {
                debugPrint("End house closed");

                stateCompleted();
            }
        }

    }



    //==== KEW HOUSE OPENS PART SLIDE THEN VERTICAL THEN CLOSES SIMULTANEOUSLY  version 2===============================
    else if (state == 28)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            verticalServo.openStart();

            debugPrint("open kew");

            internalSequenceState = 1;
            timer = millis();
        }

        else if (internalSequenceState == 1)
        {
            bool servoSlideDone;
            bool servoVertDone;
            servoVertDone = verticalServo.callUpdate();

            if (millis() >= timer + 5 * 1000 && timerDone == false )
            {
                slideServo.openStart();
                timerDone = true; // tell me when the 5 seconds is up

                debugPrint("timer done begin vertical servo");
            }
            if (timerDone == true)
            {
                servoSlideDone = slideServo.callUpdate();

                debugPrint("start vertical servo moving");
            }
            if (servoVertDone == true && servoSlideDone == true)
            {
                debugPrint("both open");

                internalSequenceState = 2;
                timerDone  = false; // reset telling me when the timer is done
                timer = millis();
            }

        }

        //PAUSE BEFORE CLOSE
        else if (internalSequenceState == 2 && millis() >= timer + 10 * 1000)
        {
            debugPrint("pause up");

            verticalServo.closeStart();
            slideServo.closeStart();
            internalSequenceState = 3;
        }
        else if (internalSequenceState == 3)
        {
            bool servoVertDone = verticalServo.callUpdate();
            bool servoSlideDone = slideServo.callUpdate();
            if (servoVertDone == true && servoSlideDone == true)
            {
                debugPrint("End house closed");

                stateCompleted();
            }
        }

    }



    //==== KIDS BEDROOM ON  FASTER======================================================================================
    else if (state == 29)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeUpStart(4, 1);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = mainHouse.lightSequence();

            if (lightsFadeUpDone == true)
            {
                debugPrint("kids bedroom light on");

                stateCompleted();
            }
        }
    }



    //==== KIDS BEDROOM FASTER =========================================================================================
    else if (state == 30)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeDownStart(4, 1);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = mainHouse.lightSequence();

            if (lightsFadeDownDone == true)
            {
                debugPrint("kids bedroom light off");

                stateCompleted();
            }
        }
    }



    //==== PLAY ROOM AND WORKSHOP LIGHTS OFF ===========================================================================
    else if (state == 31)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            mainHouse.fadeDownStart(0, 8);
            mainHouse.fadeDownStart(1, 8);
            mainHouse.fadeDownStart(2, 8);
            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeDownDone = mainHouse.lightSequence();

            if (lightsFadeDownDone == true )
            {
                debugPrint("playroom and workshop light off");

                stateCompleted();
            }
        }
    }



    //==== ALL FADE OFF  ===============================================================================================
    else if (state == 32)
    {
        if (internalSequenceState == 0 && millis() >= timer + 1 * 1000)
        {
            firePlace.fadeDownStart(0, 10);
            parentsRoom.fadeDownStart(0, 10);
            mainHouse.fadeDownStart(0, 10);
            mainHouse.fadeDownStart(1, 10);
            mainHouse.fadeDownStart(2, 10);
            mainHouse.fadeDownStart(3, 10);
            mainHouse.fadeDownStart(4, 10);

            internalSequenceState = 1;
        }
        else if (internalSequenceState == 1)
        {
            bool lightsFadeUpDone = firePlace.lightSequence();
            bool lightsFadeUpDone2 = parentsRoom.lightSequence();
            bool lightsFadeUpDone3 = mainHouse.lightSequence();

            if (lightsFadeUpDone == true && lightsFadeUpDone2 == true && lightsFadeUpDone3 == true)
            {
                wireTransfer(5); 

                debugPrint("sequence 02 start");
                debugPrint("ALL LIGHTS OFF");

                stateCompleted();
            }
        }
    }



    //==== LIGHTS OFF STATE 2 ==========================================================================================
    else if (state == 33)
    {
        if (millis() >= timer + 2500)
        {
            analogWrite(ledPin, 0);
            wireTransfer(6);  // send trigger to stop construction track

            debugPrint("turn plinth lights off");
            debugPrint("sequence 2 stop");

            stateCompleted();
        }
    }

    else
    {
        // shouldn't happen...
        // but if it does, return to default state
        debugPrint("This shouldn't have happened. state out of random range.");
        // flash the plinth lights to indicate an error
        if (millis() >= flashTimer + 1000)
        {
            flashTimer = millis();
            analogWrite(ledPin, plinthLightState ? 255 : 0);
        }
    }
}
