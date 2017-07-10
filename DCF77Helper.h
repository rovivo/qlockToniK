/**
 * DCF77Helper.h
 * Klasse um die Qualitaet der DCF77-Samples zu verbessern. Dazu wird der Minutenabstand
 * zwischen den empfangenen DCF77-Samples mit Hilfe der DS1307 Echtzeituhr verglichen.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.4
 * @created  19.3.2011
 * @updated  13.2.2012
 *
 * Versionshistorie:
 * V 1.1  - Fehler in der Initialisierung behoben.
 * V 1.2  - Umstellung auf TimeStamps und Verbesserung des Plausibilitaetstests.
 * V 1.3  - Fehler im samplesOk() behoben.
 * V 1.4  - Kompatibilitaet zu Arduino-IDE 1.0 hergestellt.
 */
#ifndef DCF77HELPER_H
#define DCF77HELPER_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "DS1307.h"
#include "MyDCF77.h"
#include "TimeStamp.h"

// Wieviele Samples muessen stimmen, damit das DCF77-Telegramm als gueltig zaehlt?
#define DCF77HELPER_MAX_SAMPLES 4

class DCF77Helper {
public:
  DCF77Helper();

  void addSample(MyDCF77 dcf77, DS1307 ds1307);
  boolean samplesOk();

private:
  int _cursor;
  TimeStamp _zeitstempelDcf77[DCF77HELPER_MAX_SAMPLES];
  TimeStamp _zeitstempelRtc[DCF77HELPER_MAX_SAMPLES];
};

#endif

