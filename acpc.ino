#define BAUDRATE 57600 // for serial communications
#define VOLTMIN 1.0 // when relay connects outlets
#define VOLTMAX 140.0 // when relay disconnects outlets
#define VOLTHYSTERESIS 5.0 // this many volts closer to ideal before relay closes
#define RELAYMINTIME 1000 // minimum milliseconds relay must stay open before closing again

#include <Adafruit_NeoPixel.h>

#define STRIP1PIN 4 // pin number of LED strip 1
#define STRIP1NUM 60 // number of LEDs on strip

#define VOLTSPIN A0 // where to sense voltage
#define AMPSPIN A1 // where to sense amperage
#define RELAYPIN 2 // normally open relay for AC power
#define VOLTCOEFF 4.1666 // 500 at 120vac
#define VOLTOFFSET 0.0 // when voltage is 0 this is the ADC value
#define AMPCOEFF 174.4 // 39 0.1A   252 at 1.28A    171 at 0.82A
#define AMPOFFSET 28.0 // when nothing plugged in, this is the ADC value

#define MAXPOWER 250.0 // wattage at which all LEDs are lit up

#define OVERSAMPLING 25 // how many times to average analogReads

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(STRIP1NUM, STRIP1PIN, NEO_GRB + NEO_KHZ800);
uint32_t offColor = 0; // the color used for pixels when "off"
uint32_t onColor = 0; // the color used for pixels when "on"

float ampsADC, voltsADC;
float volts, amps, watts;
unsigned long lastRelayTime = 0; // remember the last time we flipped relay
unsigned long lastLoopTime;
int patternPos = 0; // for strip1 animation
int fullness = 50;

void animateLeds(float fullness) { // fullness is 0.0 to 1.0
  unsigned long lastStrip1Time = millis();
  int numLit = constrain(STRIP1NUM * fullness,0,STRIP1NUM-2);
  for (int i = 0; i < STRIP1NUM; i++) {
    if (i <= numLit) {
    strip1.setPixelColor(STRIP1NUM-(i+patternPos)%STRIP1NUM-1,onColor);
    } else {
    strip1.setPixelColor(STRIP1NUM-(i+patternPos)%STRIP1NUM-1,offColor);
    }
  }
  patternPos += 1;
  strip1.show();
}

void updateRelay(float volts) {
  if (digitalRead(RELAYPIN)) { // if relay is closed
    if ((volts > VOLTMAX) || (volts < VOLTMIN)) {
      digitalWrite(RELAYPIN,LOW); // open relay
      lastRelayTime = millis(); // store when it opened
    }
  } else // relay must be open
  if (millis() - lastRelayTime > RELAYMINTIME) { // if it has been opened for long enough
    if ((volts > VOLTMIN + VOLTHYSTERESIS) && (volts < VOLTMAX - VOLTHYSTERESIS)) {
      digitalWrite(RELAYPIN,HIGH); // close relay
    }
  }
}

void setup() {
  Serial.begin(57600);
  Serial.println("AC PedalCharger display and relay box");
  pinMode(RELAYPIN,OUTPUT);
  strip1.begin();
  offColor = strip1.Color(10, 10, 10);
  onColor = strip1.Color(200, 200, 200);
  for (int i = 0; i < STRIP1NUM; i++) strip1.setPixelColor(i,offColor);
  strip1.show();
  lastLoopTime = millis();
}

void loop() {
  voltsADC = averageRead(A0);
  volts = (voltsADC - VOLTOFFSET) / VOLTCOEFF;
  ampsADC = averageRead(A1);
  amps = (ampsADC - AMPOFFSET) / AMPCOEFF;
  watts = amps * volts;
  Serial.print(volts,1);
  Serial.print("V (");
  Serial.print(voltsADC,1);
  Serial.print(")  ");
  Serial.print(amps,2);
  Serial.print("A (");
  Serial.print(ampsADC,1);
  Serial.print(")  ");
  Serial.print(watts,1);
  Serial.print("W  ");
  Serial.println(millis() - lastLoopTime);
  lastLoopTime = millis();
  /*if (Serial.available()) {
    fullness = Serial.parseInt();
    while (Serial.available()) byte inByte = Serial.read();
  animateLeds((float)fullness / 100.0);
  }*/
  animateLeds(watts/MAXPOWER);
  updateRelay(volts);
}

float averageRead(byte pin) {
  int adder = 0;
  for (int i = 0; i < OVERSAMPLING; i++) {
    adder += analogRead(pin);
    delayMicroseconds(666);
  }
  return ((float)adder/OVERSAMPLING);
}
