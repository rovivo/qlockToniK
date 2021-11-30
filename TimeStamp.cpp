#include "TimeStamp.h"

// #define DEBUG

bool h12Flag2;
bool pmFlag2;
bool century2 = false;

TimeStamp::TimeStamp() {
}

TimeStamp::TimeStamp(int minutes, int hours, int date, int dayOfWeek, int month, int year) {
  set(minutes, hours, date, dayOfWeek, month, year);
}

TimeStamp::TimeStamp(DS3231 ds3231){
  setFrom(ds3231);
}

int TimeStamp::getMinutes(){
  return _minutes;
}

int TimeStamp::getMinutesOfDay() {
  return _minutes + 60 * _hours;
}

// Minuten des Tages ohne die Beruecksichtigung von 12/24 Stunden 
// (fuer den Wecker)...
int TimeStamp::getMinutesOf12HoursDay() {
  int h = _hours;
  while(h > 12) {
    h -= 12;
  }
  return _minutes + 60 * h;
}

int TimeStamp::getHours(){
  return _hours;
}

int TimeStamp::getDate(){
  return _date;
}

int TimeStamp::getDayOfWeek(){
  return _dayOfWeek;
}

int TimeStamp::getMonth(){
  return _month;
}

int TimeStamp::getYear(){
  return _year;
}

void TimeStamp::setFrom(DS3231 ds3231){
  _minutes = ds3231.getMinute();
  _hours = ds3231.getHour(h12Flag2, pmFlag2);
  _date = ds3231.getDate();
  _dayOfWeek = ds3231.getDoW();
  _month = ds3231.getMonth(century2);
  _year = ds3231.getYear();
}

void TimeStamp::set(int minutes, int hours, int date, int dayOfWeek, int month, int year){
  _minutes = minutes;
  _hours = hours;
  _date = date;
  _dayOfWeek = dayOfWeek;
  _month = month;
  _year = year;
}

/**
 * Die Minuten erhoehen.
 */
void TimeStamp::incMinutes() {
  _minutes++;
  if(_minutes > 59) {
    _minutes = 0;
  }
}
 
/**
 * Die Stunden erhoehen.
 */
void TimeStamp::incHours() {
  _hours++;
  if(_hours > 23) {
    _hours = 0;
  }
}

/**
 * Die Zeit als String bekommen
 */
char* TimeStamp::asString() {
  _cDateTime[0] = 0;
  char temp[5];

  // build the string...        
  if (_hours < 10) {
    sprintf(temp, "0%d:", _hours);
    strncat(_cDateTime, temp, strlen(temp));
  } 
  else {
    sprintf(temp, "%d:", _hours);
    strncat(_cDateTime, temp, strlen(temp));
  }

  if (_minutes < 10) {
    sprintf(temp, "0%d ", _minutes);
    strncat(_cDateTime, temp, strlen(temp));
  } 
  else {
    sprintf(temp, "%d ", _minutes);
    strncat(_cDateTime, temp, strlen(temp));
  }

  if (_date < 10) {
    sprintf(temp, "0%d.", _date);
    strncat(_cDateTime, temp, strlen(temp));
  } 
  else {
    sprintf(temp, "%d.", _date);
    strncat(_cDateTime, temp, strlen(temp));
  }

  if (_month < 10) {
    sprintf(temp, "0%d.", _month);
    strncat(_cDateTime, temp, strlen(temp));
  } 
  else {
    sprintf(temp, "%d.", _month);
    strncat(_cDateTime, temp, strlen(temp));
  }

  sprintf(temp, "%d", _year);
  strncat(_cDateTime, temp, strlen(temp));

  return _cDateTime;
}

