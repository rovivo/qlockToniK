//#define DEBUG

// Die Geschwindigkeit der seriellen Schnittstelle. Default: 57600
  #define SERIAL_SPEED 57600

  #define ENABLE_LDR

  #define SKIP_BLANK_LINES

#include <Wire.h> // Wire library fuer I2C
//#include "DS1307.h"
#include <DS3231.h>
#include "MyDCF77.h"
#include "ShiftRegister.h"
#include "Button.h"
#include "LDR.h"
#include "DCF77Helper.h"

/**
 * Die Real-Time-Clock
 */
//DS3231 ds1307(0x68);
DS3231 ds1307;
byte helperSeconds;
bool h12Flag;
bool pmFlag;
bool century = false;
/**
 * Der serielle Ausgang zu den Multiplexern
 */
#define outputEnablePin   3
ShiftRegister shiftRegister(10, 12, 11);
byte linesToWrite = 10;
/**
 * Die Funkuhr.
 */
#define dcf77Signal   9
MyDCF77 dcf77(dcf77Signal);
DCF77Helper dcf77Helper;
/**
 * Das Rechtecksignal der RTC fuer den Interrupt
 */
#define rtcSQWLed     4
/**
 * Der Lautsprecher
 */
#define SPEAKER  99
#define SPEAKER_FREQUENCY 200000
/**
 * Variablen fuer den Alarm.
 */

/**
 * Der Helligkeitssensor
 */
#ifdef ENABLE_LDR
  LDR ldr(A3);
#endif
/**
 * Die Helligkeit (default: MAX_BRIGHTNESS)
 */
int brightness = MAX_BRIGHTNESS;
/**
 * Die Helligkeit zum Anzeigen
 */
int brightnessToDisplay;
/**
 * Die Zeit, die mit LDR::MAX_BRIGHTNESS multipliziert auf 
 * eine Zeile aufgeteilt wird in Mikrosekunden.
 * (default = 50)
 */
#define PWM_DURATION 50
/**
 * Die Tasten
 */
Button minutesPlusButton(5);
Button hoursPlusButton(6);
Button modeChangeButton(7);
/**
 * Die Modi
 */
//ohne Alarm!
//#ifndef ENABLE_ALARM
  #define NORMAL     0
  #define SECONDS    1
  #define BRIGHTNESS 2
  #define SCRAMBLE   3
  #define BLANK      4
  #define ALL        5
  #define MAX        9 //maximum of modes
  int mode = NORMAL;

// Merker fuer den Modus vor der Abschaltung...
int lastMode = mode;

// Ueber die Wire-Library festgelegt:
// Arduino analog input 4 = I2C SDA
// Arduino analog input 5 = I2C SCL

// Die Matrix, eine Art Bildschirmspeicher
word matrix[16];

// Hilfsvariable, da I2C und Interrupts nicht zusammenspielen
volatile boolean needsUpdateFromRtc = true;

// Hilfsvariable, um das Display auf den Kopf zu stellen
#ifdef UPSIDE_DOWN
#define DISPLAY_SHIFT  9-
#else
#define DISPLAY_SHIFT  0+
#endif

/**
 * Hier werden die Grafiken fuer die Zahlen der
 * Sekundenanzeige includiert.
 */
#include "Zahlen.h"

/**
 * Hier werden die Definitionen der Woerter
 * includiert. Die Woerter definieren die LEDs,
 * die pro Wort aufleuchten sollen. Daher muss
 * diese Definition mit dem Folienplot ueber-
 * einstimmen.
 */
   #include "Woerter_DE.h"

/**
 * Hier wird die Datei includiert, die die Eck-Leds setzt.
 * So kann man sie leichter anpassen (wenn man sich beim
 * anloeten vertan hat) und hat bei Software-Updates weniger 
 * Probleme.
 */
#ifdef SPLIT_SIDE_DOWN
  #include "SetCornersSplitSideDown.h"
#else
  #include "SetCorners.h"
  //#include "SetCornersCCW.h"
#endif

/**
 * Hier wird die Datei includiert, die fuer
 * das Setzten der Minuten zustaendig ist.
 */
// Hochdeutsch:
   #include "SetMinutes_DE_DE.h"

/**
 * Hier wird die Datei includiert, die fuer
 * das Setzten der Stunde zustaendig ist.
 */
// Deutsch:
   #include "SetHours_DE.h"

/**
 * Initialisierung. setup() wird einmal zu Beginn aufgerufen, wenn der
 * Arduino Strom bekommt.
 */
void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.println("Qlockthree is initialazing...");

#ifdef DEBUG
  Serial.println("... and starting in debug-mode...");
#endif
  Serial.flush();

  // starte Wire-Library als I2C-Bus Master
  Wire.begin();

  // Inhalt der Shift Register loeschen...
  clearMatrix();
  // Setze Output-Enable der Shift-Register auf LOW->enabled
  pinMode(outputEnablePin, OUTPUT);
  digitalWrite(outputEnablePin, LOW);
  // analogWrite(outputEnablePin, brightness); // Schwerer Muell! Bewirkt Flackern!

  // DCF77-Pins konfigurieren
  pinMode(dcf77Signal, INPUT);
  digitalWrite(dcf77Signal, HIGH);

  // DS1307-Pins konfigurieren
  pinMode(rtcSQWLed, OUTPUT);
  digitalWrite(rtcSQWLed, LOW);
      
  // DCF77-LED drei Mal als 'Hello' blinken lassen
  for(int i=0; i<3; i++) {
    tone(SPEAKER, SPEAKER_FREQUENCY); 
    delay(100);

    noTone(SPEAKER);
    delay(100);    
  }

  // Matrix loeschen
  clearScreenBuffer();

  // 1 Hz-SQW-Signal einschalten
  helperSeconds = ds1307.getSecond();

  Serial.print("Time: ");
  Serial.print(ds1307.getHour(h12Flag, pmFlag));
  Serial.print(":");
  Serial.print(ds1307.getMinute());
  Serial.print(":");
  Serial.println(ds1307.getSecond());
  Serial.print("Date: ");
  Serial.print(ds1307.getDate());
  Serial.print(".");
  Serial.print(ds1307.getMonth(century));
  Serial.print(".");
  Serial.println(ds1307.getYear());
  Serial.flush();


  // den Interrupt konfigurieren
  // nicht mehr CHANGE, das sind 2 pro Sekunde
  // RISING ist einer pro Sekunde, das reicht
  attachInterrupt(0, updateFromRtc, RISING);

  // Werte vom LDR einlesen, da die ersten nichts taugen...
#ifdef ENABLE_LDR
  for(int i=0; i<1000; i++) {
    ldr.value();
  }  
#endif

#ifdef ENABLE_ALARM
  // Alarm-Sachen
  isBeeping = false;
  showAlarmTimeTimer = 0;
  toneIsOn = false;
#endif

  // rtcSQWLed-LED drei Mal als 'Hello' blinken lassen
  for(int i=0; i<3; i++) {
    digitalWrite(rtcSQWLed, HIGH);
    tone(SPEAKER, SPEAKER_FREQUENCY); 
    delay(100);    
    digitalWrite(rtcSQWLed, LOW);
    noTone(SPEAKER);
    delay(100);    
  }

  Serial.println("Finished initialazing (VOID Setup) and going in loop!");
  Serial.flush();
}

/**
 * loop() wird endlos auf alle Ewigkeit vom Microcontroller durchlaufen
 */
void loop() {  

  // Dimmung.
#ifdef ENABLE_LDR
  brightness = ldr.brightness();
#endif

  // via Interrupt gesetzt ueber Flanke des SQW-Signals von der RTC
  if (needsUpdateFromRtc) {    
    needsUpdateFromRtc = false;
    // die Zeit verursacht ein kurzes Flackern. Wir muessen
    // sie aber nicht immer lesen, im Modus 'normal' alle 60 Sekunden,
    // im Modus 'seconds' alle Sekunde, sonst garnicht.
    helperSeconds++;
    if(helperSeconds == 60) {
      helperSeconds = 0;
    }
    
    // Zeit einlesen...
    switch(mode) {
      case NORMAL:
#ifdef ENABLE_ALARM
      case ALARM:
        if(isBeeping) {
        }
#endif      
        if(helperSeconds == 0) {
          helperSeconds = ds1307.getSecond();
        }
        break;
      case SECONDS:
      case BLANK:
        helperSeconds = ds1307.getSecond();
        break;
      // andere Modi egal...
    }

    // Bildschirmpuffer beschreiben...
    switch (mode) {
      case NORMAL:
        clearScreenBuffer();
        setMinutes(ds1307.getHour(h12Flag, pmFlag), ds1307.getMinute());
        setCorners(ds1307.getMinute());
        break;   
      case SECONDS:
        clearScreenBuffer();
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= ziffern[ds1307.getSecond() / 10][i] << 11;
          matrix[1 + i] |= ziffern[ds1307.getSecond() % 10][i] << 5;
        }
        break;
      case SCRAMBLE:
        scrambleScreenBuffer();
        break;
      case BLANK:
        clearScreenBuffer();
        break;
      case BRIGHTNESS:
        clearScreenBuffer();
        brightnessToDisplay = map(brightness, 1, MAX_BRIGHTNESS, 0, 9);
        for(int x=0; x<brightnessToDisplay; x++) {
          for(int y=0; y<=x; y++) {
            matrix[9-y] |= 1 << (14-x);
          }
        }
      break;
      case ALL:
        setAllScreenBuffer();
        break;
      }
  }

  // Taste Minuten++ (brighness++) gedrueckt?
  if (minutesPlusButton.pressed()) {
#ifdef DEBUG
    Serial.println("\nMinutes plus pressed...");
    Serial.flush();    
#endif      
    needsUpdateFromRtc = true;
    switch(mode) {
      case NORMAL:
		ds1307.setMinute(ds1307.getMinute() + 1);   
        ds1307.setSecond(0);
        helperSeconds = 0;  
#ifdef DEBUG
        Serial.print("M is now ");
        Serial.println(ds1307.getMinute());
        Serial.flush();
#endif      
#ifdef ENABLE_ALARM
      case ALARM:
        alarmTime.incMinutes();
        showAlarmTimeTimer = 10;
#ifdef DEBUG
        Serial.print("A is now ");
        Serial.println(alarmTime.asString());
        Serial.flush();
#endif      
      break;
#endif      
      break;
      case BRIGHTNESS:
        if(brightness < MAX_BRIGHTNESS) {
          brightness++;
        }
      break;
    }
  }
  
  // Taste Stunden++ (brightness--) gedrueckt?
  if (hoursPlusButton.pressed()) {
#ifdef DEBUG
    Serial.println("\nHours plus pressed...");
    Serial.flush();
#endif      
    needsUpdateFromRtc = true;
    switch(mode) {
      case NORMAL:
		ds1307.setHour(ds1307.getHour(h12Flag, pmFlag) + 1);
        ds1307.setSecond(0);
        helperSeconds = 0;
#ifdef DEBUG
        Serial.print("H is now ");
        Serial.println(ds1307.getHour(h12Flag, pmFlag));
        Serial.flush();
#endif      
      break;
#ifdef ENABLE_ALARM
      case ALARM:
        alarmTime.incHours();
        showAlarmTimeTimer = 10;
#ifdef DEBUG
        Serial.print("A is now ");
        Serial.println(alarmTime.asString());
        Serial.flush();
#endif      
      break;
#endif      
      case BRIGHTNESS:
        if(brightness > 2) {
          brightness--;
        }
      break;
    }
  }
  
  // Taste Moduswechsel gedrueckt?
  if (modeChangeButton.pressed()) {
    mode++;
    if(mode == MAX) {
      mode = 0;
    }
    if((mode == NORMAL) || (mode == SECONDS)) {
      linesToWrite = 10;
    } else {
      linesToWrite = 16;
    }
#ifdef ENABLE_ALARM
    if(mode == ALARM) {
      showAlarmTimeTimer = 10;
    } else {
      isBeeping = false;
      if(toneIsOn) {
        noTone(SPEAKER);
        toneIsOn = false;
      }
    }
#endif

#ifdef DEBUG
    Serial.print("\nChange mode pressed, mode is now ");
    Serial.print(mode);
    Serial.println("...");
    Serial.flush();
#endif      
    needsUpdateFromRtc = true;
    
    if(mode == BLANK) {
      digitalWrite(outputEnablePin, HIGH);
    } else {
      digitalWrite(outputEnablePin, LOW);
    }
    
    lastMode = mode;
  }
 
  // Die Matrix auf die LEDs multiplexen
  if(mode != BLANK) {
#ifdef SPLIT_SIDE_DOWN
    writeScreenBufferToMatrixSplitSideDown();
#else
    writeScreenBufferToMatrix();
#endif
  }

  // DCF77-Empfaenger abfragen
  if(dcf77.poll()) {
#ifdef DEBUG
    Serial.print("Captured: ");
    Serial.println(dcf77.asString());
    Serial.flush();
#endif
    dcf77Helper.addSample(dcf77, ds1307);
    // stimmen die Abstaende im Array?
    if(dcf77Helper.samplesOk()) {
      ds1307.setSecond(0);
      ds1307.setMinute(dcf77.getMinutes());
      ds1307.setHour(dcf77.getHours());
      // wir setzen auch das Datum, dann kann man, wenn man moechte,
      // auf das Datum eingehen (spezielle Nachrichten an speziellen
      // Tagen). Allerdings ist das Datum bisher ungeprueft!
      ds1307.setDate(dcf77.getDate());
      ds1307.setDoW(dcf77.getDayOfWeek());
      ds1307.setMonth(dcf77.getMonth());
      // die DS1307 moechte das Jahr zweistellig
      int y = dcf77.getYear();
      while (y > 100) {
        y = y-100;
      }
      ds1307.setYear(y);
#ifdef DEBUG
      Serial.println("DCF77-Time written to DS1307.");
      Serial.flush();
#endif    
    }
    else {
#ifdef DEBUG
      Serial.println("DCF77-Time trashed because wrong distances between timestamps.");
      Serial.flush();
#endif    
    }
  }
}

/**
 * Aenderung der Anzeige als Funktion fuer den Interrupt, der ueber das SQW-Signal 
 * der Real-Time-Clock gesetzt wird. Da die Wire-Bibliothek benutzt wird, kann man
 * den Interrupt nicht direkt benutzen, sondern muss eine Hilfsvariable setzen, die
 * dann in loop() ausgewertet wird.
 */
void updateFromRtc() {
  needsUpdateFromRtc = true;
}

/**
 * Ein Zufallsmuster erzeugen (zum Testen der LEDs)
 */
void scrambleScreenBuffer() {
  for (int i = 0; i < 16; i++) {
    matrix[i] = random(65536);
  }
}

/**
 * Die Matrix loeschen (zum Strom sparen, DCF77-Empfang
 * verbessern etc.)
 */
void clearScreenBuffer() {
  for (int i = 0; i < 16; i++) {
    matrix[i] = 0;
  }
}

/**
 * Die Matrix komplett einschalten (zum Testen der LEDs)
 */
void setAllScreenBuffer() {
  for (int i = 0; i < 16; i++) {
    matrix[i] = 65535;
  }
}

/**
 * Die Matrix ausgeben
 */
void writeScreenBufferToMatrix() {
  word row = 1;  

  // Jetzt die Daten...
  // wir brauchen keine 16, 10 wuerden reichen... dann gehen aber nicht 
  // mehr alle Modi! Also via Variable, die im Modus-Wechsel geaendert wird...
  for (word k = 0; k < linesToWrite; k++) { 
#ifdef SKIP_BLANK_LINES
    if(matrix[DISPLAY_SHIFT k] != 0) {
#endif
      // Zeile einschalten...
      shiftRegister.prepareShiftregisterWrite();
      shiftRegister.shiftOut(~matrix[DISPLAY_SHIFT k]);
      shiftRegister.shiftOut(row);
      shiftRegister.finishShiftregisterWrite();    

      // hier kann man versuchen, das Taktverhaeltnis zu aendern (Anzeit)...
      // delayMicroseconds mit Werten <= 3 macht Probleme...
      if(brightness != 0) {
        delayMicroseconds(brightness*PWM_DURATION);
      }

      // Zeile ausschalten (einfach ganze Matrix gegenlaeufig schalten)...
      shiftRegister.prepareShiftregisterWrite();
      shiftRegister.shiftOut(65535);
      shiftRegister.shiftOut(0);
      shiftRegister.finishShiftregisterWrite();    
  
      // hier kann man versuchen, das Taktverhaeltnis zu aendern (Auszeit)...
      // delayMicroseconds mit Werten <= 3 macht Probleme...
      if(brightness != MAX_BRIGHTNESS) {
        delayMicroseconds((MAX_BRIGHTNESS-brightness)*PWM_DURATION);
      }

#ifdef SKIP_BLANK_LINES
    }
#endif
    
    row = row << 1;
  }
}

/**
 * Die Matrix ausgeben mir gedrehten Shift Rgistern
 * bei den Anoden...
 */
void writeScreenBufferToMatrixSplitSideDown() {
  word row = 1 << 8;
  for (word k = 0; k < 8; k++) { 
#ifdef SKIP_BLANK_LINES
    if(matrix[DISPLAY_SHIFT k] != 0) {
#endif
      // Zeile einschalten...
      shiftRegister.prepareShiftregisterWrite();
      shiftRegister.shiftOut(~matrix[DISPLAY_SHIFT k]);
      shiftRegister.shiftOut(row);
      shiftRegister.finishShiftregisterWrite();    

      // hier kann man versuchen, das Taktverhaeltnis zu aendern (Anzeit)...
      // delayMicroseconds mit Werten <= 3 macht Probleme...
      if(brightness != 0) {
        delayMicroseconds(brightness*PWM_DURATION);
      }

      // Zeile ausschalten...
      shiftRegister.prepareShiftregisterWrite();
      shiftRegister.shiftOut(65535);
      shiftRegister.shiftOut(0);
      shiftRegister.finishShiftregisterWrite();    
  
      // hier kann man versuchen, das Taktverhaeltnis zu aendern (Auszeit)...
      // delayMicroseconds mit Werten <= 3 macht Probleme...
      if(brightness != MAX_BRIGHTNESS) {
        delayMicroseconds((MAX_BRIGHTNESS-brightness)*PWM_DURATION);
      }

#ifdef SKIP_BLANK_LINES
    }
#endif
    
    row = row << 1;
  }

  row = 1;
  for (word k = 8; k < linesToWrite; k++) { 
#ifdef SKIP_BLANK_LINES
    if(matrix[DISPLAY_SHIFT k] != 0) {
#endif
      // Zeile einschalten...
      shiftRegister.prepareShiftregisterWrite();
      shiftRegister.shiftOut(~matrix[DISPLAY_SHIFT k]);
      shiftRegister.shiftOut(row);
      shiftRegister.finishShiftregisterWrite();    

      // hier kann man versuchen, das Taktverhaeltnis zu aendern (Anzeit)...
      // delayMicroseconds mit Werten <= 3 macht Probleme...
      if(brightness != 0) {
        delayMicroseconds(brightness*PWM_DURATION);
      }

      // Zeile ausschalten...
      shiftRegister.prepareShiftregisterWrite();
      shiftRegister.shiftOut(65535);
      shiftRegister.shiftOut(0);
      shiftRegister.finishShiftregisterWrite();    
  
      // hier kann man versuchen, das Taktverhaeltnis zu aendern (Auszeit)...
      // delayMicroseconds mit Werten <= 3 macht Probleme...
      if(brightness != MAX_BRIGHTNESS) {
        delayMicroseconds((MAX_BRIGHTNESS-brightness)*PWM_DURATION);
      }

#ifdef SKIP_BLANK_LINES
    }
#endif
    
    row = row << 1;
  }
}

/**
 * Die Matrix (Shift-Register) loeschen...
 */
void clearMatrix() {
  word row = 1;  

  for (word k = 0; k < 16; k++) { 
    shiftRegister.prepareShiftregisterWrite();
    shiftRegister.shiftOut(65535);
    shiftRegister.shiftOut(row);
    shiftRegister.finishShiftregisterWrite();        
    row = row << 1;
  }
}

