#include "LDR.h"

// #define DEBUG

/**
 * Initialisierung mit dem Pin, an dem der LDR haengt
 */
LDR::LDR(byte pin) {
  _pin = pin;
  _meanpointer = 0;
#ifdef LDR_AUTOSCALE
  _min = 1023;
  _max = 0;
#else
  _min = 0;
  _max = 1023;
#endif
}

/**
 * Welchen Wert hat der LDR?
 */
int LDR::value() {
  int val = analogRead(_pin);
  // glaetten
  _meanvalues[_meanpointer] = val;
  _meanpointer++;
  if(_meanpointer == LDR_MEAN_COUNT) {
    _meanpointer = 0;
  }
  long ret = 0;
  for(int i=0; i<LDR_MEAN_COUNT; i++) {
    ret += _meanvalues[i];
  }
  return ret/LDR_MEAN_COUNT;
}

/**
 * Welche Helligkeit haben wir?
 */
int LDR::brightness() {
  int val = value();
#ifdef LDR_AUTOSCALE
    if(val < _min) {
      _min = val;
    }
    if(val > _max) {
      _max = val;
    }
#else
  val = constrain(val, _min, _max);
#endif
  int b = map(val, _min, _max, MAX_BRIGHTNESS, 1);
#ifdef DEBUG
    Serial.print(F(" _min: "));
    Serial.print(_min);
    Serial.print(F(" _max: "));
    Serial.print(_max);
    Serial.print(F(" ldr: "));
    Serial.print(val);
    Serial.print(F(" brightness: "));
    Serial.println(b);
#endif
  return b;
}

