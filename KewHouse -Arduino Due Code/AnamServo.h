#include "Arduino.h"

#define servoMax 2200              // Maximum position for servo
#define servoMin 200               // Minimum position for servo

class AnamServo
{
  public:

  String servoName;
  int id;
  int relayPin;
  int dataPin;
  int servoSpeed;
  int startPos;
  int firstPos;
  int secondPos;

  int currentPos;        // Current position of the servo (in pulselength)
  int gotoPos;           // Goto position - servo will move towards this position

  long startTime;        // Starting time for this script
  long lastPulse;        // the time in milliseconds of the last pulse

  bool done;

  
    //constructor
    AnamServo(String _servoName, int _id, int _relayPin, int _dataPin, int _servoSpeed, int _startPos, int _firstPos, int _secondPos)
    {
      servoName = _servoName;
      id = _id;
      relayPin = _relayPin;
      dataPin = _dataPin;
      servoSpeed = _servoSpeed;
      startPos = _startPos;
      firstPos = _firstPos;
      secondPos = _secondPos;
      done = false;
    }
    
    //int servoStartPos = 1150;//1150        // starting position for servo  (1500 is the middle, 2 rots is 500pulse , so 2000 + to get to the end of the line
    //int servoFirstPos = 550;        // open position  // good medium for the model to remain open enough
    //int servoSecondPos = 1150;        // end position

    //int servoRelay = 5;              // pin for relay to trigger
    //int servoPin = 6;                // servo connected to this digital pin
    //int servoSpeed = 1;               // current position will change by this value every pulse

    void setup()
    {
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, LOW);
      pinMode(dataPin, OUTPUT);
    }

    void openStart()
    {
      //Serial.println(servoName + " open start");
      currentPos = startPos;  // set current position to the startint position
      gotoPos = firstPos;     // set goto position to the first position
      lastPulse = 0;
      digitalWrite(relayPin, HIGH);
      done = false;
    }

    void closeStart()
    {
      //Serial.println(servoName + " close start");
      currentPos = firstPos;  // set current position to the startint position
      gotoPos = secondPos;     // set goto position to the first position
      lastPulse = 0;
      done = false;
    }
    
    bool update()
    {
      // we check if we need to increment the current position
      if (currentPos < gotoPos)
      {
        // check if it would overshoot
        if (currentPos + servoSpeed > gotoPos)
        {
          currentPos = gotoPos;
        }
        else
        {
          currentPos += servoSpeed;
        }
      }

      // we check if we need to deveremtn the current position
      else if (currentPos > gotoPos)
      {
        // check if it would overshoot
        if (currentPos - servoSpeed < gotoPos)
        {
          currentPos = gotoPos;
        }
        else
        {
          currentPos -= servoSpeed;
        }
      }

      // make sure the current position is not more or less than min or max
      if (currentPos > servoMax)
      {
        currentPos = servoMax;
      }
      else if (currentPos < servoMin)
      {
        currentPos = servoMin;
      }

      // now we have to actually set the goto position on the servo
      digitalWrite(dataPin, HIGH);               // Turn the motor on
      delayMicroseconds(currentPos);          // Length of the pulse sets the motor position
      digitalWrite(dataPin, LOW);                // Turn the motor off
    }

    /**
       Call this every frame in the main script, it will call the update function every 20 ms.
    */
    bool callUpdate()
    {

      // EVER 20 MS SEND A PULSE OF EACH SERVO
      long t = millis();
      if (t - lastPulse >= 20)
      {
        // update last pulse
        lastPulse = t;

        // do the servo update
        if (done == false) {
          update();
        }

//        Serial.print("Pos: ");
//        Serial.print(currentPos);
//        Serial.print(" - GotoPos ");
//        Serial.print(gotoPos);
//        if (id == 1) {
//          Serial.print(" --- ");
//        } else {
//          Serial.println("");
//        }

        // check if the model is closed again
        if (currentPos == secondPos)
        {
          digitalWrite(relayPin, LOW);
          done = true;
          return done;
        }

        // check if opening sequence is done
        if (currentPos == firstPos)
        {
          done = true;
          return done;
        }


      }
      return done;
    }

};
