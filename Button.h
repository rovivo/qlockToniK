/**
 * Button.h
 * Kleine Klasse zum Entprellen der Tasten.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.1
 * @created  18.2.2011
 * @updated  13.2.2012
 *
 * Versionshistorie:
 * V 1.1:  - Kompatibilitaet zu Arduino-IDE 1.0 hergestellt.
 */
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

