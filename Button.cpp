#include "Button.h"

#define BUTTON_TRESHOLD 300


Button::Button(int pin) {
  _pin = pin;
  _lastPressTime = 0;
  pinMode(_pin, INPUT);
}

boolean Button::pressed() {
  boolean retVal = false;

  if (digitalRead(_pin) == HIGH) {
    if (_lastPressTime + BUTTON_TRESHOLD < millis()) {
      retVal = true;
      _lastPressTime = millis();
    }
  }

  return retVal;
}

