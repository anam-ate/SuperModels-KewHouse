

//============================================================================//
//===== Settings variables

const int TRIG_PIN = 2;         // trigger pin
const int ECHO_PIN = 3;         // echo pin


//============================================================================//
//===== Local variables
long duration, distanceCm;


//============================================================================//
//===== Setup - call this in the global setup function
void setupDistanceSensor() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

//============================================================================//
//===== Reads the distance from the distance sensor
int readDistance() {
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // get the pulse length, convert it to the distance and return it
  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCm = duration / 29.1 / 2;
  if (distanceCm < 1) return 100;
  return distanceCm;
}
