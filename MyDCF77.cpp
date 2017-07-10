/**
 * MyDCF77.h
 * Klasse fuer die Auswertung des DCF77-Zeitsignals.
 * Diese Klasse geht von einem 'sauberen' Signal aus.
 * Bei schlechten Empfangsbedingungen muesste man eine
 * Unschaerfe zulassen.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.5
 * @created  1.11.2011
 * @updated  18.1.2013
 *
 * Versionshistorie:
 * V 1.1:   - Fehler in der Array-Laenge und in toString() behoben.
 * V 1.1.1: - signal() public gemacht.
 * V 1.2:   - Signal geglaettet.
 * V 1.3:   - Strings in PROGMEM ausgelagert.
 * V 1.4:   - Signal kann invertiert werden (fuer manche Empfaenger noetig) - danke an Dominik.
 * V 1.5:   - Optimierung hinsichtlich Speicherbedarf.
 */
#include "MyDCF77.h"

// #define DEBUG 

/**
 * Initialisierung mit dem Pin, an dem das Signal des Empfaengers anliegt
 */
MyDCF77::MyDCF77(byte signalPin) {
  _signalPin = signalPin;
#ifndef MyDCF77_SIGNAL_IS_ANALOG
  pinMode(_signalPin, INPUT);
#endif

  for(byte i=0; i<MYDCF77_TELEGRAMMLAENGE; i++) {
    _bits[i] = 0;
  }
  _bitsPointer = 0;
  _oldSignalState = false;
  _signalStartedAtMillis = 0;
  _signalStoppedAtMillis = 0;
  
  _meanpointer = 0;
}

/**
 * Liegt ein Signal vom Empfaenger an?
 */
boolean MyDCF77::signal() {
  boolean val;
#ifdef MyDCF77_SIGNAL_IS_ANALOG
  val = analogRead(_signalPin) > MyDCF77_ANALOG_SIGNAL_TRESHOLD;
#else
  #ifdef MYDCF77_SIGNAL_IS_INVERTED
    val = digitalRead(_signalPin) == LOW;
  #else
    val = digitalRead(_signalPin) == HIGH;
  #endif
#endif
  _meanvalues[_meanpointer] = val;
  _meanpointer++;
  if(_meanpointer == MYDCF77_DCF_MEAN_COUNT) {
    _meanpointer = 0;
  }
  int count = 0;
  for(byte i=0; i<MYDCF77_DCF_MEAN_COUNT; i++) {
    if(_meanvalues[i]) {
      count++;
    }
  }
  return (count > (MYDCF77_DCF_MEAN_COUNT / 2));
}

/**
 * Aufsammeln der Bits des DCF77-Signals. Im Prinzip ein grosser Zustandsautomat.
 * Zurueckgegeben wird ein Wahrheitswert. 
 * TRUE bedeutet, das Zeittelegramm wurde korrekt ausgewertet, die Zeitdaten 
 * koennen mit den Gettern abgerufen werden.
 * FALSE bedeutet, die Auswertung laeuft oder war falsch, die Getter liefern
 * alte Informationen.
 */
boolean MyDCF77::poll() {
  boolean retVal = false;

  if(signal()) {
    if(!_oldSignalState) {
      // vorher kein Signal -> ansteigende Flanke -> Sekundenbeginn
      _oldSignalState = true;
      _signalStartedAtMillis = millis();
    }
  } 
  else { // if signal()
    if (_oldSignalState) {
      // vorher Signal -> abfallende Flanke -> Bitinfo fertig (Signaldauer 100ms = 0; 200ms = 1)
      unsigned long duration = millis()-_signalStartedAtMillis;
      if(duration > MYDCF77_200MS_TRESHOLD) {
        // Bit ist 1
#ifdef DEBUG
        Serial.print("1");
        Serial.flush();
#endif
        if(_bitsPointer < MYDCF77_TELEGRAMMLAENGE) {
          _bits[_bitsPointer] = 1;
        }
        _bitsPointer++;
        _oldSignalState = false;
        _signalStoppedAtMillis = millis();
      } 
      else if (duration > MYDCF77_100MS_TRESHOLD) {
        // Bit ist 0
#ifdef DEBUG
        Serial.print("0");
        Serial.flush();
#endif
        if(_bitsPointer < MYDCF77_TELEGRAMMLAENGE) {
          _bits[_bitsPointer] = 0;
        }
        _bitsPointer++;
        _oldSignalState = false;
        _signalStoppedAtMillis = millis();
      } 
      else {
        // Schrott empfangen
#ifdef DEBUG
        Serial.print("-");
        Serial.flush();
#endif
      }
    }
  }

  // Synczeitpunkt erreicht oder Telegrammlaenge korrekt...
  if(millis()-_signalStoppedAtMillis > MYDCF77_SYNC_PAUSE_TRESHOLD) {
#ifdef DEBUG
    Serial.println();
    Serial.print(F("Duration (pause): "));
    Serial.println(millis()-_signalStoppedAtMillis);
    Serial.print(F("Bitcount: "));
    Serial.println(_bitsPointer);
    Serial.flush();
#endif

    if(_bitsPointer == MYDCF77_TELEGRAMMLAENGE) {
      retVal = decode();
    }

    // Bitarray und Pointer zuruecksetzen...
    for(byte i=0; i<MYDCF77_TELEGRAMMLAENGE; i++) {
      _bits[i] = 0;
    }
    _bitsPointer = 0;
    _oldSignalState = false;
    _signalStartedAtMillis = millis();
    _signalStoppedAtMillis = millis();
    
#ifdef DEBUG
    // Von oben nach unten lesen. Zuerst Bitnummer, dann Bedeutung.
    Serial.println(F("          1111111111222222222233333333334444444444555555555"));
    Serial.println(F("01234567890123456789012345678901234567890123456789012345678"));
    Serial.println(F("---------------RAZZASMMMMMMMPSSSSSSPDDDDDDWWWMMMMMYYYYYYYYP"));
    Serial.println(F("                1122 12481241124812212481212412481124812483"));
    Serial.println(F("                         000     00     00       0    0000 "));
    Serial.flush();
#endif    
  }

  return retVal;
}

/**
 * Decodierung des Telegramms...
 */
boolean MyDCF77::decode() {
  int c = 0; // bitcount for checkbit
  boolean ok = true;

#ifdef DEBUG
  Serial.println(F("Decoding telegram..."));
  Serial.flush();
#endif

  if(_bits[20] != 1) {
    ok = false;
#ifdef DEBUG
    Serial.println(F("Check-bit S failed."));
    Serial.flush();
#endif
  }

  if(_bits[17] == _bits[18]) {
    ok = false;
#ifdef DEBUG
    Serial.println(F("Check Z1 != Z2 failed."));
    Serial.flush();
#endif
  }

  //
  // minutes
  //
  _minutes = 0;
  c = 0;
  if(_bits[21] == 1) {
    c++;
    _minutes += _bits[21] * 1;
  }
  if(_bits[22] == 1) {
    c++;
    _minutes += _bits[22] * 2;
  }
  if(_bits[23] == 1) {
    c++;
    _minutes += _bits[23] * 4;
  }
  if(_bits[24] == 1) {
    c++;
    _minutes += _bits[24] * 8;
  }
  if(_bits[25] == 1) {
    c++;
    _minutes += _bits[25] * 10;
  }
  if(_bits[26]) {
    c++;
    _minutes += _bits[26] * 20;
  }
  if(_bits[27]) {
    c++;
    _minutes += _bits[27] * 40;
  }
#ifdef DEBUG
  Serial.print(F("Minutes: "));
  Serial.println(_minutes);
  Serial.flush();
#endif
  if((c + _bits[28]) % 2 != 0) {
    ok = false;
#ifdef DEBUG
    Serial.println(F("Check-bit P1: minutes failed."));
    Serial.flush();
#endif
  }

  //
  // hour
  //
  _hours = 0;
  c = 0;
  if(_bits[29] == 1) {
    c++;
    _hours += _bits[29] * 1;
  }
  if (_bits[30] == 1) {
    c++;
    _hours += _bits[30] * 2;
  }
  if (_bits[31] == 1) {
    c++;
    _hours += _bits[31] * 4;
  }
  if (_bits[32] == 1) {
    c++;
    _hours += _bits[32] * 8;
  }
  if (_bits[33] == 1) {
    c++;
    _hours += _bits[33] * 10;
  }
  if (_bits[34] == 1) {
    c++;
    _hours += _bits[34] * 20;
  }
#ifdef DEBUG
  Serial.print(F("Hours: "));
  Serial.println(_hours);
  Serial.flush();
#endif
  if((c + _bits[35]) % 2 != 0) {
    ok = false;
#ifdef DEBUG
    Serial.println(F("Check-bit P2: hours failed."));
    Serial.flush();
#endif
  }

  //
  // date
  //
  _date = 0;
  c = 0;
  if(_bits[36] == 1) {
    c++;
    _date += _bits[36] * 1;
  }
  if(_bits[37] == 1) {
    c++;
    _date += _bits[37] * 2;
  }
  if(_bits[38] == 1) {
    c++;
    _date += _bits[38] * 4;
  }
  if(_bits[39] == 1) {
    c++;
    _date += _bits[39] * 8;
  }
  if(_bits[40] == 1) {
    c++;
    _date += _bits[40] * 10;
  }
  if(_bits[41] == 1) {
    c++;
    _date += _bits[41] * 20;
  }
#ifdef DEBUG
  Serial.print(F("Date: "));
  Serial.println(_date);
  Serial.flush();
#endif

  //
  // day of week
  //
  _dayOfWeek = 0;
  if(_bits[42] == 1) {
    c++;
    _dayOfWeek += _bits[42] * 1;
  }
  if(_bits[43] == 1) {
    c++;
    _dayOfWeek += _bits[43] * 2;
  }
  if(_bits[44] == 1) {
    c++;
    _dayOfWeek += _bits[44] * 4;
  }
#ifdef DEBUG
  Serial.print(F("Day of week: "));
  Serial.println(_dayOfWeek);
  Serial.flush();
#endif

  //
  // month
  //
  _month = 0;
  if(_bits[45] == 1) {
    c++;
    _month += _bits[45] * 1;
  }
  if(_bits[46] == 1) {
    c++;
    _month += _bits[46] * 2;
  }
  if(_bits[47] == 1) {
    c++;
    _month += _bits[47] * 4;
  }
  if(_bits[48] == 1) {
    c++;
    _month += _bits[48] * 8;
  }
  if(_bits[49] == 1) {
    c++;
    _month += _bits[49] * 10;
  }
#ifdef DEBUG
  Serial.print(F("Month: "));
  Serial.println(_month);
  Serial.flush();
#endif

  //
  // year
  //
  _year = 0;
  if(_bits[50] == 1) {
    c++;
    _year += _bits[50] * 1;
  }
  if(_bits[51] == 1) {
    c++;
    _year += _bits[51] * 2;
  }
  if(_bits[52] == 1) {
    c++;
    _year += _bits[52] * 4;
  }
  if(_bits[53] == 1) {
    c++;
    _year += _bits[53] * 8;
  }
  if(_bits[54] == 1) {
    c++;
    _year += _bits[54] * 10;
  }
  if(_bits[55] == 1) {
    c++;
    _year += _bits[55] * 20;
  }
  if(_bits[56] == 1) {
    c++;
    _year += _bits[56] * 40;
  }
  if(_bits[57] == 1) {
    c++;
    _year += _bits[57] * 80;
  }

  
           
         
#ifdef DEBUG
                         
           
    
   
  
      
  Serial.print(F("Year: "));
  Serial.println(_year);
  Serial.flush();
#endif

  if((c + _bits[58]) % 2 != 0) {
    ok = false;
#ifdef DEBUG
    Serial.println(F("Check-bit P3: date failed."));
    Serial.flush();
#endif
  }

  if (!ok) {
    // discard date...
    _minutes = 0;
    _hours = 0;
    _date = 0;
    _dayOfWeek = 0;
    _month = 0;
    _year = 0;
  }

  return ok;
}

/**
 * Das Zeittelegramm als String bekommen
 */
char* MyDCF77::asString() {
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

//
// Getter
//
byte MyDCF77::getMinutes() {
  return _minutes;
}

byte MyDCF77::getHours() {
  return _hours;
}

byte MyDCF77::getDate() {
  return _date;
}

byte MyDCF77::getDayOfWeek() {
  return _dayOfWeek;
}

byte MyDCF77::getMonth() {
  return _month;
}

byte MyDCF77::getYear() {
  return _year;
}


