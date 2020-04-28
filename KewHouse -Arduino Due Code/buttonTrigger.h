
//variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
const int buttonPin = 4;     // the number of the pushbutton pin


void buttonTriggerSetup()
{
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

}

bool buttonTriggerSensor()
{
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);  //read state of push button

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH)
  {
    //Serial.println("button pushed");
    return true;
  } else {
    //Serial.println("button not pushed");
    return false;


  }

}
