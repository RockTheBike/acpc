#define BAUDRATE 57600 // for serial communications
#define VOLTMIN 100.0 // when relay connects outlets
#define VOLTMAX 135.0 // when relay disconnects outlets
#define VOLTHYSTERESIS 5.0 // this many volts closer to ideal before relay closes

#include <Adafruit_NeoPixel.h>

#define STRIP1PIN 4 // pin number of LED strip
#define NUMLEDS 65 // number of LEDs on strip
#define VOLTSPIN A0 // where to sense voltage
#define AMPSPIN A1 // where to sense amperage
#define RELAYPIN 2 // normally open relay for AC power
#define VOLTCOEFF 4.1666 // 500 at 120vac
#define VOLTOFFSET 0.0 // when voltage is 0 this is the ADC value
#define AMPCOEFF 174.4 // 39 0.1A   252 at 1.28A    171 at 0.82A
#define AMPOFFSET 28.0 // when nothing plugged in, this is the ADC value

#define OVERSAMPLING 50 // how many times to average analogReads

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUMLEDS, STRIP1PIN, NEO_GRB + NEO_KHZ800);
uint32_t offColor = 0; // the color used for pixels when "off"

float ampsADC, voltsADC;
float volts, amps;

void setup() {
  Serial.begin(57600);
  Serial.println("AC PedalCharger display and relay box");
  pinMode(RELAYPIN,OUTPUT);
  strip1.begin();
  offColor = strip1.Color(10, 10, 10);
  for (int i = 0; i < NUMLEDS; i++) strip1.setPixelColor(i,offColor);
  strip1.show();
}


void loop() {
  for (int i = 0; i < NUMLEDS; i++) strip1.setPixelColor(i,strip1.Color(i,0,70-i));
  strip1.show();

  voltsADC = averageRead(A0);
  volts = (voltsADC - VOLTOFFSET) / VOLTCOEFF;
  ampsADC = averageRead(A1);
  amps = (ampsADC - AMPOFFSET) / AMPCOEFF;
  Serial.print(volts,1);
  Serial.print("V (");
  Serial.print(voltsADC,1);
  Serial.print(")  ");
  Serial.print(amps,2);
  Serial.print("A (");
  Serial.print(ampsADC,1);
  Serial.println(")  ");
}

float averageRead(byte pin) {
  int adder = 0;
  for (int i = 0; i < OVERSAMPLING; i++) {
    adder += analogRead(pin);
    delayMicroseconds(666);
  }
  return ((float)adder/OVERSAMPLING);
}
