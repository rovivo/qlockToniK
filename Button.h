#ifndef BUTTON_H
#define BUTTON_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class Button {
public:
  Button(int pin);

  boolean pressed();

private:
  int _pin;
  unsigned long _lastPressTime;
};

#endif

