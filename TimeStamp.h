/**
 * TimeStamp.h
 * Klasse fuer die Kapselung eines Zeitstempels.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.4
 * @created  2.3.2011
 * @updated  2.7.2012
 *
 * Versionshistorie:
 * V 1.1:  - Fehler in toString() behoben.
 * V 1.2:  - Kompatibilitaet zu Arduino-IDE 1.0 hergestellt.
 * V 1.3:  - neuer Konstruktor, neue Methoden.
 * V 1.4:  - getMinutesOf12HoursDay eingefuehrt.
 */
#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "MyDCF77.h"
#include "DS1307.h"

class TimeStamp {
public:
  TimeStamp();
  TimeStamp(int minutes, int hours, int date, int dayOfWeek, int month, int year);
  TimeStamp(MyDCF77 dcf77);
  TimeStamp(DS1307 ds1307);

  void incMinutes();
  void incHours();

  int getMinutes();
  int getMinutesOfDay();
  int getMinutesOf12HoursDay();
  int getHours();

  int getDate();
  int getDayOfWeek();
  int getMonth();
  int getYear();

  void setFrom(MyDCF77 dcf77);
  void setFrom(DS1307 ds1307);
  void set(int minutes, int hours, int date, int dayOfWeek, int month, int year);

  char* asString();

private:
  int _minutes;
  int _hours;

  int _date;
  int _dayOfWeek;
  int _month;
  int _year;
  
  char _cDateTime[17];
};

#endif


