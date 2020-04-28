#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

//#define DATA_PINHouse   24 //24
//#define DATA_PINMoving   28
//#define DATA_PINFirePlace  26

#define MinBrightness 0    //value 0-255
#define MaxBrightness 255  //value 0-255
#define MAX_LEDS 5

class Lights
{
  public:
    int numberofPixels; // total pixels on strip
    int dataPinLight;  // which digi pin
    int pixelID; // pin id for turning on lights on certain strips
    int durationFade;
    int rVal;   //255, 77, 0, 32,
    int gVal;
    int bVal;
    int wVal;
    int currentValue[MAX_LEDS];  // stores the current brightness of the pixel
    int gotoValue[MAX_LEDS];  // stores the destination position of the pixel
    int durationFadeTime[MAX_LEDS];
    unsigned long LastFadeTime[MAX_LEDS];
    int pixelfadeSpeed; // the speed at which the fading occurs

    unsigned long LightstartTime;       // you set this to millis() when you want it to start, i.e. when the sensor triggers (just once though)

    //DECLARE THE STRIPS FOR THE HOUSE, FIREPLACE AND THE PARENTS BEDROOM
    Adafruit_NeoPixel* stripLights; // initialise strip outside of constructor as public make strip a pointer


    //CONSTRUCTOR  list of variables you want to define outside the class in the main script
    Lights(int _numberofPixels, int _dataPinLight, int _rVal, int _gVal, int _bVal, int _wVal, int _pixelfadeSpeed) // call this function in the main script, this becomes an instance of the Lights constructor
    {
      numberofPixels = _numberofPixels;
      dataPinLight = _dataPinLight;
      rVal = _rVal;
      gVal = _gVal;
      bVal = _bVal;
      wVal = _wVal;
      pixelfadeSpeed = _pixelfadeSpeed;
      stripLights = new Adafruit_NeoPixel(numberofPixels, dataPinLight, NEO_GRBW + NEO_KHZ800);

      for (uint16_t i = 0; i < numberofPixels; i++) // initialise the values of the pixels to 0
      {
        currentValue[i] = 0;
        gotoValue[i] = 0;
        durationFadeTime[i] = 0;
        LastFadeTime[i] = 0;
      }
    }

    void setup()
    {

      // put your setup code here, to run once:
      stripLights->begin(); // Setup LED strip and set brightness

      for (uint16_t i = 0; i < numberofPixels; i++)
      {
        stripLights->setPixelColor(i, 0, 0, 0, 0);
      }

      stripLights->setBrightness(MaxBrightness); // always have the brightness set to 255

      stripLights->show(); // Initialize all pixels to 'off'

    }

    void fadeUpStart(int pixelID, int durationFade)
    {
      //Serial.println("fade up begin");
      gotoValue[pixelID] = 255;  // the value the lights should go to when they fade up
      durationFadeTime[pixelID] = durationFade;
    }

    void fadeDownStart(int pixelID, int durationFade)
    {
      //Serial.println("fade down begin");
      gotoValue[pixelID] = 0;  // the value the lights should go to when they fade down
      durationFadeTime[pixelID] = durationFade;
    }

    // currentValue/fadeduration
    bool lightSequence()
    {

      for (uint16_t i = 0; i < numberofPixels; i++)
      {
        if (millis() - LastFadeTime[i] > durationFadeTime[i]) // durationofFadeTime is the interval at which I want each fadeup step to happen (in this case 1sec or whatever durationfadetime is set too)
        {

          if (currentValue[i] < gotoValue[i])
          {
            currentValue[i] += pixelfadeSpeed;
            if (currentValue[i] >= gotoValue[i] )
            {
              currentValue[i] = gotoValue[i];
            }
            //Serial.println("fade upper");
          }

          if (currentValue[i] > gotoValue[i])
          {
            currentValue[i] -= pixelfadeSpeed;

            if (currentValue[i] <= gotoValue[i])
            {
              currentValue[i] = gotoValue[i];
            }
            //Serial.println("fade downer");
          }

          //Serial.println(currentValue[i]);
          stripLights->setPixelColor(i, currentValue[i] * rVal / 255, currentValue[i] * gVal / 255, currentValue[i] * bVal / 255, currentValue[i] * wVal / 255); //warm white
          stripLights->show();
          LastFadeTime[i] = millis();

        }
      }
      ///checker: check if the final gotovalue is reached then return true
      bool done = true;
      for (uint16_t i = 0; i < numberofPixels; i++)
      {
        if (currentValue[i] != gotoValue[i])
          done = false;
      }
 
      return done;
    }

};
