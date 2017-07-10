/**
 * DCF77Helper.cpp
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
#include "DCF77Helper.h"

//#define DEBUG

/**
 * Initialisierung und das Array 'falsch' vorbelegen
 */
DCF77Helper::DCF77Helper() {
  _cursor = 0;
  for(int i=0; i<DCF77HELPER_MAX_SAMPLES; i++) {
    _zeitstempelDcf77[i] = TimeStamp();
    _zeitstempelDcf77[i].set(i, i, i, i, i, i);
    
    _zeitstempelRtc[i] = TimeStamp();
    _zeitstempelRtc[i].set(10000, 0, 0, 0, 0, 0);
  }
}

/**
 * Einen neuen Sample hinzufuegen
 */
void DCF77Helper::addSample(MyDCF77 dcf77, DS1307 ds1307) {
#ifdef DEBUG
  Serial.print("Adding sample from dcf77: ");
#endif
  _zeitstempelDcf77[_cursor].setFrom(dcf77);
#ifdef DEBUG
  Serial.println(_zeitstempelDcf77[_cursor].asString());
  Serial.print("Adding sample from ds1307: ");
#endif
  _zeitstempelRtc[_cursor].setFrom(ds1307);
#ifdef DEBUG
  Serial.println(_zeitstempelRtc[_cursor].asString());
  Serial.flush();
#endif
  _cursor++;
  if(_cursor == DCF77HELPER_MAX_SAMPLES) {
    _cursor = 0;
  }
}

/**
 * Die Samples vergleichen
 */
boolean DCF77Helper::samplesOk() {
  boolean ret = true;
  for(int i=0; i<DCF77HELPER_MAX_SAMPLES-1; i++) {
    // Teste den Minutenabstand zwischen den Zeitstempeln...
    if((_zeitstempelDcf77[i].getMinutesOfDay() - _zeitstempelDcf77[i+1].getMinutesOfDay()) != (_zeitstempelRtc[i].getMinutesOfDay() - _zeitstempelRtc[i+1].getMinutesOfDay())) {
#ifdef DEBUG
      Serial.print("Diff #");
      Serial.print(i);
      Serial.print(" distance is wrong (");
      Serial.print(_zeitstempelDcf77[i].getMinutesOfDay() - _zeitstempelDcf77[i+1].getMinutesOfDay());
      Serial.print("!=");
      Serial.print(_zeitstempelRtc[i].getMinutesOfDay() - _zeitstempelRtc[i+1].getMinutesOfDay());
      Serial.println(").");
      Serial.flush();
#endif    
      ret = false;
    } 
    else {
#ifdef DEBUG
      Serial.print("Diff #");
      Serial.print(i);
      Serial.print(" distance is ok (");
      Serial.print(_zeitstempelDcf77[i].getMinutesOfDay() - _zeitstempelDcf77[i+1].getMinutesOfDay());
      Serial.print("==");
      Serial.print(_zeitstempelRtc[i].getMinutesOfDay() - _zeitstempelRtc[i+1].getMinutesOfDay());
      Serial.println(").");
      Serial.flush();
#endif    
    }
  }   

  return ret;
}

