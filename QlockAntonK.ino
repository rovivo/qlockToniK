/**
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  2.0.1
 * @created  1.11.2011
 * @updated  6.7.2012
 *
 * TO-DOs:
 * - ganzes Datum vom DCF77 in DS1307 schreiben; Ausgabe von 'Specials' an bestimmten
 *   Datumswerten (Geburtstag etc.) - fehlt noch. Das Problem ist die Ueberpruefung
 *   des DCF77-Signals. Soll das Datum auch stimmen werden viel mehr Telegramme
 *   verworfen und die Synczeit wird viel laenger. Ausserdem sollte man die Moeglichkeit
 *   habe, das Datum von Hand einstellen zu koennen.
 *
 * Versionshistorie:
 * V 1.1:   - DCF77 auf reine Zeit ohne Strings umgestellt.
 *          - Ganzes Datum wird in die DS1307 geschrieben (wieder verworfen).
 * V 1.2:   - Schnelles Schreiben der Shift-Register eingefuehrt.
 * V 1.3:   - Auf eigene DCF77-Bibliothek umgestellt (MyDCF77) und DCF77Helper verbessert.
 * V 1.4:   - Glimmen/Nachleuchten dank Tipp von Volker beseitigt (http://www.mikrocontroller.net/topic/209988).
 *          - Erneute Fehlerbereinigung in der DCF77-Bibliothek.
 * V 1.5:   - Die Helligkeit laesst sich ueber die Variable 'brightness' einstellen (Bereich 1-9) bzw.
 *            ueber den Compiler-Schalter 'ENABLE_LDR' die Regelung einschalten.
 * V 1.6:   - Fehler in der DCF77Helper-Bibliothek behoben.
 * V 1.7:   - SKIP_BLANK_LINES eingefuehrt, damit hellere LEDs (Danke an Johannes).
 * V 1.8:   - UPSIDE_DOWN eingefuehrt, damit man die Kabel beim Anoden-Multiplexer nicht kreuzen muss.
 * V 1.9:   - Diverse Verbesserungen und Fehlerbehebungen, die das Flackern drastisch reduzieren.
 * V 1.9.1: - Fehler behoben, der die Nutzung eines analogen Pins fuer den DCF77-Empfaenger verhindert hat.
 * V 1.9.2: - Timing-Parameter fuer die PWM ueber define anpassbar.
 *          - LDR-Grenzwerte automatisch anpassbar.
 *          - Schreibfehler in setHoures()->setHours() behoben.
 *          - Kompatibilitaet zu Arduino-IDE 1.0 hergestellt.
 * V 1.9.3: - Glimmen wieder behoben.
 * V 1.9.4: - Modus zum Einstellen der Helligkeit eingefuehrt.
 * V 2.0:   - Sprachrelevante Dinge ausgelagert, so kann man mehr Sprachen haben und einfach durch einkommentieren aktivieren.
 *          - setWords in setMinutes und setMinutes in setCorners umbenannt, weil es mehr Sinn ergibt.
 *          - setCorners in eigene Datei ausgelagert, weil viele Bastler sich vertun und in der Routine Aenderungen vornehmen muessen.
 *          - LDR in eigene Klasse ausgelagert und Werte geglaettet. Am Anfang werden 1000 Werte gelesen, damit er sich einpegelt.
 *          - Signal vom DCF77-Empfaenger geglaettet, damit nicht ein einzelner falscher Peak das Telegramm zerstoert. 
 *          - Status-LEDs koennen ueber DEFINEs ausgeschaltet werden.
 *          - DCF77- und SQW-LED blinken am Anfang drei Mal als 'Hello', damit man ohne Serial-Monitor sehen kann, ob der ATMEGA anlaeuft.
 *          - Serial-Output ist immer an, kleine Statusmeldung waehrend der Initialisierung, damit man beim Bespielen des ATMEGA ohne weitere 
 *            Elektronik sehen kann, ob das Programm drauf ist und laeuft.
 *          - Aenderung, die ein Stellen der Zeit nach dem DCF77-Signal auch im Modus 'BLANK' ermoeglicht - in diesem Modus ist der Empfang
 *            weniger gestoert, da die LED-Matrix abgeschaltet ist.
 *          - Zeitgesteuertes Abschalten des Displays eingefuehrt (Stromsparen in der Nacht/Schlafzimmer/besserer Empfang des DCF77-Empfaengers).
 *          - Speaker auf D13 eingefuehrt.
 *          - Alarm-Mode eingefuehrt.
 *          - Bayrische Sprachvariante (DE_BA): 'viertel nach Zehn', aber 'dreiviertel Elf'.
 * V 2.0.1: - DCF77-Tresholds angepasst.
 */

/*
 * Den DEBUG-Schalter gibt es in allen Bibiliotheken. Wird er eingeschaltet, werden ueber den
 * 'Serial-Monitor' der Arduino-Umgebung Informationen ausgegeben. Die Geschwindigkeit im
 * Serial-Monitor muss mit der hier angegeben uebereinstimmen.
 * Default: ausgeschaltet
 */
//#define DEBUG
// Die Geschwindigkeit der seriellen Schnittstelle. Default: 57600
   #define SERIAL_SPEED 57600

/*
 * Alarmfunktion einschalten?
 * Dazu muss ein Lautsprecher an D13 und GND und eine weitere 'Eck-LED' an die 5te Reihe.
 * Dann gibt es einen neuen Modus, direkt nach der normalen Zeitanzeige. Die neue LED
 * blinkt nach dem Moduswechsel und der Alarm ist eingeschaltet. Drueckt man jetzt M+ oder H+ stellt man
 * die Alarmzeit ein, angedeutet durch die blinkende Alarm-LED. Druckt man 10 Sekunden
 * keine Taste, hoert das Blinken auf und die normale Zeit wird wieder angezeigt.
 * Bei erreichen des Alarms piept der Lautpsrecher auf D13. Zum Ausschalten muss der
 * Modus-Taster gedrueckt werden.
 * Default: ausgeschaltet
 */
// #define ENABLE_ALARM

/*
 * Die Status-LEDs koennen hier durch auskommentieren ausgeschaltet werden.
 * Default: eingeschaltet 
 */
#define ENABLE_DCF_LED
#define ENABLE_SQW_LED

/*
 * Wer einen LDR (an A3) installiert hat, kann diese Zeile auskommentieren und hat dann eine von
 * der Umgebungshelligkeit abhaengige Helligkeit.
 * Default: eingeschaltet
 */
#define ENABLE_LDR

/*
 * Dieser Schalter erhoeht die Helligkeit, indem er nicht beleuchtete Zeilen ueberspringt. (Tipp
 * von Johannes)
 * Default: ausgeschaltet
 */
#define SKIP_BLANK_LINES

/*
 * Dieser Schalter stellt die Anzeige auf den Kopf, falls man die Kabel beim Anoden-
 * multiplexer nicht kreuzen moechte oder es vergessen hat.
 * Default: ausgeschaltet
 */
//#define UPSIDE_DOWN

/*
 * Dieser Schalter stellt die Anzeige auf den Kopf und teilt das Display, falls man die 
 * Kabel beim Anodenmultiplexer nicht kreuzen moechte oder es vergessen hat aber dennoch
 * SERIAL-DATA oben in den Anoden-Multiplexer fuehrt.
 * Default: ausgeschaltet
 */
// #define SPLIT_SIDE_DOWN

#include <Wire.h> // Wire library fuer I2C
#include "DS1307.h"
#include "MyDCF77.h"
#include "ShiftRegister.h"
#include "Button.h"
#include "LDR.h"
#include "DCF77Helper.h"

/**
 * Die Real-Time-Clock
 */
DS1307 ds1307(0x68);
byte helperSeconds;
/**
 * Der serielle Ausgang zu den Multiplexern
 */
#define outputEnablePin   3
ShiftRegister shiftRegister(10, 12, 11);
byte linesToWrite = 10;
/**
 * Die Funkuhr.
 */
#define dcf77Led      8
#define dcf77Signal   9
MyDCF77 dcf77(dcf77Signal);
DCF77Helper dcf77Helper;
/**
 * Das Rechtecksignal der RTC fuer den Interrupt
 */
#define rtcSQWLed     4
#define rtcSQWSignal  2
/**
 * Der Lautsprecher
 */
#define SPEAKER  99
#define SPEAKER_FREQUENCY 200000
/**
 * Variablen fuer den Alarm.
 */
#ifdef ENABLE_ALARM
TimeStamp alarmTime(0, 7, 0, 0, 0, 0);
boolean isBeeping;
byte showAlarmTimeTimer;
boolean toneIsOn;
#endif

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
  #define LOVEH      6
  #define NFTM       7
  #define TEST       8
  #define MAX        9 //maximum of modes
  int mode = NORMAL;
/*#else
  #define NORMAL     0
  #define ALARM      1
  #define SECONDS    2
  #define BRIGHTNESS 3
  #define SCRAMBLE   4
  #define BLANK      5
  #define ALL        6
  #define LOVEH      7
  #define NFTM       8
  #define TEST       9
  #define MAX       10 //maximum of modes

  int mode = NORMAL;
#endif
*/

/**
 * Hier definiert man die Ab- und Anschaltzeiten fuer das Display. Die Abschaltung des
 * Displays verbessert den Empfang des DCF77-Empfaengers. Und hilft, falls die Uhr im 
 * Schlafzimmer haengt.
 * Sind alle Werte auf 0 wird das Display nie abgeschaltet. Nach einer Minute kann man das 
 * Display manuell wieder ein- / ausschalten.
 * Achtung! Wenn sich die Uhr nachmittags abschaltet ist sie in der falschen Tageshaelfte!
 */
// um 3 Uhr Display abschalten (Minuten, Stunden, -, -, -, -)
TimeStamp offTime(0, 0, 0, 0, 0, 0);
// um 4:30 Uhr Display wieder anschalten (Minuten, Stunden, -, -, -, -)
TimeStamp onTime(0, 0, 0, 0, 0, 0);
// Merker fuer den Modus vor der Abschaltung...
int lastMode = mode;

/*
 * modifications for rovivo library
 * 1. loveheart
 * 2. swiss national holyday
 */

//Hochzeitstag
byte HTag = 41;
byte HMonat = 5;
byte heartnr = 1; //LoveHeart
int intervallH = 2500;
int zaehlerH = 0;
bool topH = false;

//Nationalfeiertag
byte NTag = 41; // rovivo 41 = off
byte NMonat = 8; //Schweizerkreuz
int intervallN = 2500;
int zaehlerN = 0;
bool topN = false;
byte Nnr = 1;

//Test Rovivo
byte testnr = 1;

// Ueber die Wire-Library festgelegt:
// Arduino analog input 4 = I2C SDA
// Arduino analog input 5 = I2C SCL

// Die Matrix, eine Art Bildschirmspeicher
word matrix[16];
// load ROVIVO library with modifications
#include "rovivo.h"

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
  pinMode(dcf77Led, OUTPUT);
  digitalWrite(dcf77Led, LOW);

  // DS1307-Pins konfigurieren
  pinMode(rtcSQWSignal, INPUT);
  digitalWrite(rtcSQWSignal, HIGH);
  pinMode(rtcSQWLed, OUTPUT);
  digitalWrite(rtcSQWLed, LOW);
      
  // DCF77-LED drei Mal als 'Hello' blinken lassen
  for(int i=0; i<3; i++) {
    digitalWrite(dcf77Led, HIGH);
    tone(SPEAKER, SPEAKER_FREQUENCY); 
    delay(100);
    digitalWrite(dcf77Led, LOW);
    noTone(SPEAKER);
    delay(100);    
  }

  // Matrix loeschen
  clearScreenBuffer();

  // 1 Hz-SQW-Signal einschalten
  ds1307.readTime();  
  ds1307.writeTime();
  helperSeconds = ds1307.getSeconds();

  Serial.print("Time: ");
  Serial.print(ds1307.getHours());
  Serial.print(":");
  Serial.print(ds1307.getMinutes());
  Serial.print(":");
  Serial.println(ds1307.getSeconds());
  Serial.print("Date: ");
  Serial.print(ds1307.getDate());
  Serial.print(".");
  Serial.print(ds1307.getMonth());
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
          ds1307.readTime();
        }
#endif      
        if(helperSeconds == 0) {
          ds1307.readTime();
          helperSeconds = ds1307.getSeconds();
        }
        break;
      case SECONDS:
      case BLANK:
        ds1307.readTime();
        helperSeconds = ds1307.getSeconds();
        break;
      // andere Modi egal...
    }

    // Bildschirmpuffer beschreiben...
    switch (mode) {
      case NORMAL:
        clearScreenBuffer();
        setMinutes(ds1307.getHours(), ds1307.getMinutes());
        setCorners(ds1307.getMinutes());
        break;
#ifdef ENABLE_ALARM
      case ALARM:
        clearScreenBuffer();
        if(showAlarmTimeTimer == 0) {
          setMinutes(ds1307.getHours(), ds1307.getMinutes());
          setCorners(ds1307.getMinutes());
          matrix[4] |= 0b0000000000011111; // Alarm
        } else {        
          setMinutes(alarmTime.getHours(), alarmTime.getMinutes());
          setCorners(alarmTime.getMinutes());
          matrix[0] &= 0b0010001111111111; // ES IST weg
          if(showAlarmTimeTimer % 2 == 0) {
            matrix[4] |= 0b0000000000011111; // Alarm
          }
          showAlarmTimeTimer--;
        }
      break;
#endif      
      case SECONDS:
        clearScreenBuffer();
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= ziffern[ds1307.getSeconds() / 10][i] << 11;
          matrix[1 + i] |= ziffern[ds1307.getSeconds() % 10][i] << 5;
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
      case LOVEH:
        setLH();
        break;
      case NFTM:
        setNFT();
        break;
      case TEST:
        setTEST();
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
        ds1307.incMinutes();
        ds1307.setSeconds(0);
        ds1307.writeTime();
        ds1307.readTime();
        helperSeconds = 0;  
#ifdef DEBUG
        Serial.print("M is now ");
        Serial.println(ds1307.getMinutes());
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
        ds1307.incHours();
        ds1307.setSeconds(0);
        ds1307.writeTime();
        ds1307.readTime();
        helperSeconds = 0;
#ifdef DEBUG
        Serial.print("H is now ");
        Serial.println(ds1307.getHours());
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
 
 //Modus auf Herz umschalten wenn das Datum der Hochzeitstag ist
 if (ds1307.getDate() == HTag && ds1307.getMonth() == HMonat){
   if (zaehlerH > intervallH && topH == LOW)
    {topH = HIGH;}
   if (topH == LOW)
    {zaehlerH++;}
   if (topH == HIGH && zaehlerH > 0)
    {zaehlerH--;
      if (zaehlerH == 0)
        {topH = LOW;}
    }
    if (topH == HIGH) {mode = LOVEH;}
    if (topH == LOW){mode = NORMAL;}
 Serial.println(topH);
 Serial.println(zaehlerH);
 }

  //Modus auf Schweizerkreuz umschalten wenn das Datum der 1. August ist
 if (ds1307.getDate() == NTag && ds1307.getMonth() == NMonat){
   if (zaehlerN > intervallN && topN == LOW)
    {topN = HIGH;}
   if (topN == LOW)
    {zaehlerN++;}
   if (topN == HIGH && zaehlerN > 0)
    {zaehlerN--;
      if (zaehlerN == 0)
        {topN = LOW;}
    }
    if (topN == HIGH) {mode = NFTM;}
    if (topN == LOW){mode = NORMAL;}
 Serial.println(topN);
 Serial.println(zaehlerN);
 }

 
  // Display zeitgesteuert abschalten?
  if((offTime.getMinutesOfDay() != 0) && (onTime.getMinutesOfDay() != 0)) {
    if((mode != BLANK) && (offTime.getMinutesOfDay() == ds1307.getMinutesOfDay())) {
      mode = BLANK;
      digitalWrite(outputEnablePin, HIGH);
    }
    if((mode == BLANK) && (onTime.getMinutesOfDay() == ds1307.getMinutesOfDay())) {
      mode = lastMode;
      digitalWrite(outputEnablePin, LOW);
    }
  }

#ifdef ENABLE_ALARM  
  // Alarm?
  if((mode == ALARM) && (showAlarmTimeTimer == 0) && !isBeeping) {
    if(alarmTime.getMinutesOf12HoursDay() == ds1307.getMinutesOf12HoursDay()) {
      isBeeping = true;
    }
  }
  if(isBeeping) {
    if(ds1307.getSeconds() % 2 == 0) {
      tone(SPEAKER, SPEAKER_FREQUENCY);
      toneIsOn = true;
    } else {
      noTone(SPEAKER);
      toneIsOn = false;
    }
  }
#endif

  // Die Matrix auf die LEDs multiplexen
  if(mode != BLANK) {
#ifdef SPLIT_SIDE_DOWN
    writeScreenBufferToMatrixSplitSideDown();
#else
    writeScreenBufferToMatrix();
#endif
  }

  // Status-LEDs ausgeben
#ifdef ENABLE_DCF_LED
  digitalWrite(dcf77Led, dcf77.signal());
#endif
#ifdef ENABLE_SQW_LED
  digitalWrite(rtcSQWLed, digitalRead(rtcSQWSignal));
#endif

  // DCF77-Empfaenger abfragen
  if(dcf77.poll()) {
#ifdef DEBUG
    Serial.print("Captured: ");
    Serial.println(dcf77.asString());
    Serial.flush();
#endif
  
    ds1307.readTime();
    dcf77Helper.addSample(dcf77, ds1307);
    // stimmen die Abstaende im Array?
    if(dcf77Helper.samplesOk()) {
      ds1307.setSeconds(0);
      ds1307.setMinutes(dcf77.getMinutes());
      ds1307.setHours(dcf77.getHours());
      // wir setzen auch das Datum, dann kann man, wenn man moechte,
      // auf das Datum eingehen (spezielle Nachrichten an speziellen
      // Tagen). Allerdings ist das Datum bisher ungeprueft!
      ds1307.setDate(dcf77.getDate());
      ds1307.setDayOfWeek(dcf77.getDayOfWeek());
      ds1307.setMonth(dcf77.getMonth());
      // die DS1307 moechte das Jahr zweistellig
      int y = dcf77.getYear();
      while (y > 100) {
        y = y-100;
      }
      ds1307.setYear(y);

      ds1307.writeTime();
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

