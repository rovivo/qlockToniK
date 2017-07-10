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
#ifndef MYDCF77_H
#define MYDCF77_H

// Bei einer Verwendung eines analogen Pins die
// naechsten zwei Zeilen einkommentieren.
// #define MyDCF77_SIGNAL_IS_ANALOG
// #define MyDCF77_ANALOG_SIGNAL_TRESHOLD 600

// Als Synchronisierungs-Marke wird eine Pause von 1 Sekunde laenge 
// 'gesendet'. Allerdings haben wir ja vorher noch eine Absenkung vom 
// Checkbit P3 gehabt, daher 1800.
#define MYDCF77_SYNC_PAUSE_TRESHOLD 1700
#define MYDCF77_200MS_TRESHOLD 185
#define MYDCF77_100MS_TRESHOLD 85
#define MYDCF77_TELEGRAMMLAENGE 59

/**
 * Ist das Signal invertiert (z.B. ELV-Empfaenger).
 * Dann die folgende Zeile einkommentieren.
 */
#define MYDCF77_SIGNAL_IS_INVERTED

/**
 * Anzahl der Glaettungspunkte.
 */
#define MYDCF77_DCF_MEAN_COUNT 8

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class MyDCF77 {
public:
  MyDCF77(byte signalPin);

  boolean poll();

  byte getMinutes();
  byte getHours();

  byte getDate();
  byte getDayOfWeek();
  byte getMonth();
  byte getYear();

  char* asString();

  boolean signal();

private:
  byte _signalPin;
  boolean _meanvalues[MYDCF77_DCF_MEAN_COUNT];
  byte _meanpointer;

  byte _minutes;
  byte _hours;

  byte _date;
  byte _dayOfWeek;
  byte _month;
  byte _year;

  char _cDateTime[17];

  byte _bits[MYDCF77_TELEGRAMMLAENGE];
  byte _bitsPointer;

  boolean decode();

  // Hilfsvariablen fuer den Zustandsautomaten
  boolean _oldSignalState;
  unsigned long _signalStartedAtMillis;
  unsigned long _signalStoppedAtMillis;
};

#endif


