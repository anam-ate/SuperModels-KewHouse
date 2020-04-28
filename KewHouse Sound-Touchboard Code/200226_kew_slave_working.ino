#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <FreeStack.h> 
#include <SFEMP3Shield.h>

// mp3 variables
SFEMP3Shield MP3player;
// sd card instantiation
SdFat sd;
// ==========================================bird Whistle song stuff==============================================================================
#define Birdsong 0 // bird content
int durationTrackBird = 5;  //duration of bird sound
bool birdSoundTrigger1 = false;   // boolean for turning bird sound on 
bool birdSoundTrigger2 = false;  // boolean for turning bird sound off
long SoundBirdstartTime;  // start millis() for sequence 
bool birdplaying = false;
// restart the time for the bird sequence
void SoundBirdStart() 
{
  SoundBirdstartTime = millis();
}

long SoundBird1Millis() 
{
  return millis() - SoundBirdstartTime;
}
 //SOFTWARE RESET ARDUINO
void(* resetFunc) (void) = 0; //declare reset function @ address 0
// ==========================================SEQUENCE 01==============================================================================
#define sequence01 1 // wake up morning content WEEKDAY
int durationTrackSequence01 = 152;  
bool sequence01SoundTrigger1 = false;   
bool sequence01SoundTrigger2 = false;  
long sequence01startTime;  // start millis() for sequence 
bool sequence01playing = false;
// restart the time for the bird sequence
void sequence01Start() 
{
  sequence01startTime = millis();
}

long sequence01Millis() 
{
  return millis() - sequence01startTime;
}

// ==========================================SEQUENCE 02==============================================================================
#define sequence02 2 // wake up morning content WEEKEND
int durationTrackSequence02 = 255;  
bool sequence02SoundTrigger1 = false;   
bool sequence02SoundTrigger2 = false;  
long sequence02startTime;  // start millis() for sequence 
bool sequence02playing = false;
// restart the time for the bird sequence
void sequence02Start() 
{
  sequence02startTime = millis();
}

long sequence02Millis() 
{
  return millis() - sequence02startTime;
}


//=========================================== main setup ===================================================================================
void setup() 
{
  Wire.begin(20);                // join i2c bus with address 
  Wire.onReceive(receiveEvent); // register event from Due to Touchboard receive trigger data
  Serial.begin(19200);           // start serial for output
  //Serial.println("starting to receive shit");
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt(); // sound stuff 
  MP3player.begin();   
  MP3player.setVolume(10,10); // volume control for later fade this up and down 
  SoundBirdstartTime = 0; // reset the start time
  sequence01startTime = 0;
  sequence02startTime = 0;

}

//===================================main trigger loop for sounds ==============================================================
void loop() 
{
  

}

//receive data from the due, to trigger the mp3 files
void receiveEvent(int howMany)   // special event function for wire 
{
  while (1 < Wire.available()) 
  { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
  }
  int y = Wire.read();    // receive byte as an integer
  //Serial.print("Received Trigger: ");
  //Serial.println(y);         // print the integer
  
  // trigger bird whistle song from Due==================================================================
  if (y == 1)
  {
   // Serial.println("y is 1");
     MP3player.playTrack(Birdsong);

  }
  
  //stop bird whistle song from Due====================================================================
  if (y == 2)
  {
    MP3player.stopTrack();
  }
   
   
   // TRIGGER SEQUENCE 01 =====================================================================

  if(y == 3)
  {
    MP3player.playTrack(sequence01);

  }
  // STOP SEQUENCE 01 ====================================================================
  if(y == 4)
  {
    MP3player.stopTrack();

  }
  
  // TRIGGER SEQUENCE 02 =====================================================================

  if(y == 5)
  {
    MP3player.playTrack(sequence02);

  }
  // STOP SEQUENCE 02 ====================================================================
  if(y == 6)
  {
    MP3player.stopTrack();

  }

  
}
