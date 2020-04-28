
//ChimneyVariables============================================================================

#define humidiferRelay 13 //HUMIDIFIER

//=============================================================================================


class chimneySmoke
{
  public:
    int chinmeyDuration;
    unsigned long previousMillisChimney = 0; //store previous millis for delay of servo movemebts
    long chimneyStartingTime;

    //CONSTRUCTOR  list of variables you want to define outside the class in the main script
    chimneySmoke(int _chinmeyDuration)
    {
      chinmeyDuration = _chinmeyDuration;
    }
    void setup() 
    {

      pinMode(humidiferRelay, OUTPUT);

      digitalWrite(humidiferRelay, LOW); // RELAY OFF ON START HUMIDIFIER

    }

    // you have to call this once (only once!) when you want it to start
    void chimneyStart() 
    {
      chimneyStartingTime = millis();
    }

    // if you call this instead of millis() in this script it will give you the milliseconds since you called (startBird)
    long chimneyMillis() 
    {
      return millis() - chimneyStartingTime;
    }

    bool chimneySequence()
    {

      if (chimneyMillis() <= 1000 * chinmeyDuration)
      { //overall duration of birds

        digitalWrite(humidiferRelay, HIGH); // trigger humidifier

        return false;
      }
      else
      {

        digitalWrite(humidiferRelay, LOW);
        return true;
      }
      return false;
    }

}; // always end a class with a semicolon, i have no idea why google this
