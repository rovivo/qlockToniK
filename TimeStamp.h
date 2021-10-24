#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "MyDCF77.h"
#include <DS3231.h>

class TimeStamp {
public:
  TimeStamp();
  TimeStamp(int minutes, int hours, int date, int dayOfWeek, int month, int year);
  TimeStamp(MyDCF77 dcf77);
  TimeStamp(DS3231 ds1307);

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
  void setFrom(DS3231 ds1307);
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
