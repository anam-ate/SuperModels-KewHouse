//================================Bird Variables=======================================================================Servo birdServo;  // create servo object to control a servo
#define relayBird 10 // bird relay
#define birdservo_pin 9 // bird servo pin

class birdy
{
  public:
  
    int birdDuration; //seconds x 1000 mseconds 10secs 
    unsigned long previousMillisBird = 0;
    long birdStartingTime;
    int servo0; // first pos // 1000 
    int servo180; // second pos  //2000 
    int inc; //speed original 20
    int pos = servo0;

    //CONSTRUCTOR  list of variables you want to define outside the class in the main script
    birdy(int _birdDuration, int _servo0, int _servo180, int _inc)
    {
      birdDuration = _birdDuration; // length of bird duration
      servo0  = _servo0; // start position of bird
      servo180 = _servo180; // end position of bird
      inc = _inc; // speed of birdy
    }

    void birdStart() 
    {
      birdStartingTime = millis();
    }

    // if you call this instead of millis() in this script it will give you the milliseconds since you called (startBird)
    long birdMillis() 
    {
      return millis() - birdStartingTime;
    }

    void setup()
    {
      pinMode(birdservo_pin, OUTPUT);  //
      digitalWrite(birdservo_pin, HIGH);
      pinMode(relayBird, OUTPUT);
      digitalWrite(relayBird, LOW);

    }
    bool birdSequence()
    {
      pos += inc;
      //unsigned long currentMillis = millis();
      if (birdMillis() <= 1000 * birdDuration) { //overall duration of birds
        digitalWrite(relayBird, HIGH); // trigger relay on and thus servo

        if (pos > servo180)
        {
          //Serial.println("REVERSE!");
          pos = servo180;
          inc *= -1;

        }
        else if (pos < servo0)
        {

          //Serial.println("FORWARD!");
          pos = servo0;
          inc *= -1;
        }
        digitalWrite(birdservo_pin, HIGH);
        delayMicroseconds(pos);
        digitalWrite(birdservo_pin, LOW);
        delay(20);
      } else {
        digitalWrite(relayBird, LOW);
        return true;
      }
      return false;
    }
};
