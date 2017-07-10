/**
 * LDR.h
 * Klasse fuer den Zugriff auf einen lichtabhaengigen Widerstand.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.2
 * @created  18.3.2012
 * @updated  29.1.2013
 *
 * Versionshistorie:
 * V 1.1:  - Optimierung hinsichtlich Speicherbedarf.
 * V 1.2:  - LDR-Werte constrained.
 */
#ifndef LDR_H
#define LDR_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

/*
 * Sollen die Werte vom LDR automatisch angepasst werden? Sonst muss man die Werte messen und
 * die ldr_min- und ldr_max-Variablen weiter unten anpassen. Bei einem Neustart der QlockTwo kann
 * das Display flackern, dann muss man einmal ueber den LDR 'wischen', damit er verschiedene
 * Messwerte bekommt.
 */
#define LDR_AUTOSCALE
/**
 * Die maximale Helligkeit (default = 9)
 */
#define MAX_BRIGHTNESS 9
/**
 * Die Menge der Werte fuer die Glaettung. Mehr Werte bringen bessere Ergebnisse aber
 * brauchen auch mehr Zeit. (Gleitender Mittelwert)
 */
#define LDR_MEAN_COUNT 32

class LDR {
public:
  LDR(byte pin);

  int value();
  int brightness();

private:
  byte _pin;
  int _meanvalues[LDR_MEAN_COUNT];
  byte _meanpointer;
  /**
   * Die Maximalwerte vom LDR. Ueber den Messwert vom
   * LDR und diesen Grenzen wird die Helligkeit bestimmt.
   * Idealerweise misst man einmal im #define-DEBUG-Mode
   * die Grenzen aus.
   * Oder man verwendet das #define-LDR_AUTOSCALE-Feature.
   */
  int _min;
  int _max;
};

#endif

