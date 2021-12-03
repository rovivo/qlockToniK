#define DEBUG
//#define DEBUGmatrix
//#define testline

#include <Wire.h>                           // Wire library fuer I2C
#include <DS3231.h>
#include <EEPROM.h>
#include <NeoPixelBrightnessBus.h>          // https://github.com/Makuna/NeoPixelBus
#include "Constants.h"
#include "Button.h"

#define LED_COUNT 114
#define PixelPin  2

#define LDR       A3

Button minusButton(5);
Button plusButton(6);
Button modeChangeButton(7);
// Ueber die Wire-Library festgelegt:
// Arduino pro mini analog input 4 = I2C SDA
// Arduino pro mini analog input 5 = I2C SCL  

uint16_t interval           = 5000;         // [ms] Interval in dem die Uhrzeit vom Internet akuallisiert wird
uint8_t regenbogeninterval  = 50;

DS3231 clock;
bool      h12Flag;
bool      pmFlag;
bool      century = false;

int16_t	  matrix[10]      = {0,0,0,0,0,0,0,0,0,0};
uint8_t   LdrVal          = 0;
uint8_t   Brightness      = 150;
uint8_t   BrSet           = 0;

RgbwColor farbe;
uint8_t   SwitchMode      = 3;
uint8_t   oldSecond       = 0;
uint16_t  regenbogenfarbe = 0;
uint32_t  previousMillis  = 0; 
uint8_t   ColorSw         = 10;
uint32_t  regenbogentimer;

// Modus
#define NORMAL        0
#define SECONDS       1
#define SetHr         2
#define SetMin        3
#define SetDay        4
#define SetMon        5
#define SetYear       6
#define SetBrigh      7
#define ALL           8 
#define MAX           9 //maximum of modes
int mode = NORMAL;
bool firstRound;

int lastMode = mode;

NeoPixelBrightnessBus<NeoGrbwFeature, NeoSk6812Method> strip(LED_COUNT, PixelPin);

void setup() {
  Serial.begin(9600);
  Serial.println("Initialazing...");

  #ifdef DEBUG
    Serial.println("debug-mode ON");
  #endif
  Serial.flush();

  // starte Wire-Library als I2C-Bus Master
  Wire.begin();

  strip.Begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.SetBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.ClearTo(black);     // Set all pixel colors to 'off'
  mergeMatrix(View_Startup);
  writeNeo(RGBwblue);
  strip.Show();             // Turn OFF all pixels ASAP

  Serial.print("Time: ");
  Serial.print(clock.getHour(h12Flag, pmFlag));
  Serial.print(":");
  Serial.print(clock.getMinute());
  Serial.print(":");
  Serial.println(clock.getSecond());
  Serial.print("Date: ");
  Serial.print(clock.getDate());
  Serial.print(".");
  Serial.print(clock.getMonth(century));
  Serial.print(".");
  Serial.println(clock.getYear());
  Serial.flush();

  for(int i=0; i<1000; i++) {
    LdrVal = analogRead(LDR);
  }
  // first round ever write vars to eeprom
  if (EEPROM.read(0) != 123) {
    EEPROM.update(1, 1);
    EEPROM.update(2, 1);
    EEPROM.update(3, 0);
    EEPROM.update(0, 123);
  }
  
  EEPROM.get(1, SwitchMode);
  EEPROM.get(2, ColorSw);
  EEPROM.get(3, BrSet);
  delay(1000);
}


void loop() {  
  switch (mode) {
    case NORMAL:
      clearMatrix();
      generateClockMatrix(clock.getHour(h12Flag, pmFlag), clock.getMinute());
      writeNeo(ignoreColor);
      break;
    //--------------------------------------------------------------------------  
    case SECONDS:
      clearMatrix();
      for (int i = 0; i < 7; i++) {
        matrix[1 + i] |= ziffern[clock.getSecond() / 10][i] << 6;
        matrix[1 + i] |= ziffern[clock.getSecond() % 10][i] << 0;
      }
      matrix[9] = 0b0100000000000;
      writeNeo(RGBwwhite);
      break;
    //--------------------------------------------------------------------------  
    case SetHr:
      clearMatrix();
      if (firstRound) {
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= staben[0][i] << 6;
          matrix[1 + i] |= staben[1][i] << 0;
        }
        matrix[9] = 0b0110000000000;
        writeNeo(RGBwred);
        strip.Show();
        firstRound = false;
        delay(2000);
      }
      else {
      for (int i = 0; i < 7; i++) {
        matrix[1 + i] |= ziffern[clock.getHour(h12Flag, pmFlag) / 10][i] << 6;
        matrix[1 + i] |= ziffern[clock.getHour(h12Flag, pmFlag) % 10][i] << 0;
      }
      matrix[9] = 0b0110000000000;
      writeNeo(RGBwgreen);
      }
      break;
    //--------------------------------------------------------------------------  
    case SetMin:
      clearMatrix();
      if (firstRound) {
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= staben[2][i] << 6;
          matrix[1 + i] |= staben[3][i] << 0;
        }
        matrix[9] = 0b0111000000000;
        writeNeo(RGBwred);
        strip.Show();
        firstRound = false;
        delay(2000);
      }
      else {
      for (int i = 0; i < 7; i++) {
        matrix[1 + i] |= ziffern[clock.getMinute() / 10][i] << 6;
        matrix[1 + i] |= ziffern[clock.getMinute() % 10][i] << 0;
      }
      matrix[9] = 0b0111000000000;
      writeNeo(RGBwgreen);
      }
      break;
    //--------------------------------------------------------------------------  
    case SetDay:
      clearMatrix();
      if (firstRound) {
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= staben[4][i] << 6;
          matrix[1 + i] |= staben[4][i] << 0;
        }
        matrix[9] = 0b0111100000000;
        writeNeo(RGBwred);
        strip.Show();
        firstRound = false;
        delay(2000);
      }
      else {
      for (int i = 0; i < 7; i++) {
        matrix[1 + i] |= ziffern[clock.getDate() / 10][i] << 6;
        matrix[1 + i] |= ziffern[clock.getDate() % 10][i] << 0;
      }
      matrix[9] = 0b0111100000000;
      writeNeo(RGBwgreen);
      }
      break;
    //--------------------------------------------------------------------------  
    case SetMon:
      clearMatrix();
      if (firstRound) {
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= staben[2][i] << 6;
          matrix[1 + i] |= staben[2][i] << 0;
        }
        matrix[9] = 0b0111110000000;
        writeNeo(RGBwred);
        strip.Show();
        firstRound = false;
        delay(2000);
      }
      else {
      for (int i = 0; i < 7; i++) {
        matrix[1 + i] |= ziffern[clock.getMonth(century) / 10][i] << 6;
        matrix[1 + i] |= ziffern[clock.getMonth(century) % 10][i] << 0;
      }
      matrix[9] = 0b0111110000000;
      writeNeo(RGBwgreen);
      }
      break;
    //--------------------------------------------------------------------------  
    case SetYear:
      clearMatrix();
      if (firstRound) {
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= staben[5][i] << 6;
          matrix[1 + i] |= staben[5][i] << 0;
        }
        matrix[9] = 0b0111111000000;
        writeNeo(RGBwred);
        strip.Show();
        firstRound = false;
        delay(2000);
      }
      else {
      for (int i = 0; i < 7; i++) {
        matrix[1 + i] |= ziffern[clock.getYear() / 10][i] << 6;
        matrix[1 + i] |= ziffern[clock.getYear() % 10][i] << 0;
      }
      matrix[9] = 0b0111111000000;
      writeNeo(RGBwgreen);
      }
      break;
   //--------------------------------------------------------------------------  
   case SetBrigh: 
     clearMatrix();
     if (firstRound) {
        for (int i = 0; i < 7; i++) {
          matrix[1 + i] |= staben[6][i] << 6;
          matrix[1 + i] |= staben[1][i] << 0;
        }
        matrix[9] = 0b0111111100000;
        writeNeo(RGBwred);
        strip.Show();
        firstRound = false;
        delay(2000);
      }
      else {
        if (BrSet == 0) {
          for (int i = 0; i < 7; i++) {
            matrix[1 + i] |= staben[7][i] << 3;
          }
        }
        else {
          for (int i = 0; i < 7; i++) {
            matrix[1 + i] |= ziffern[BrSet][i] << 3;
          }
        }
        matrix[9] = 0b0111111100000;
        writeNeo(RGBwgreen);
      }
     break;
    //--------------------------------------------------------------------------  
    case ALL:
      clearMatrix();
      mergeMatrix(View_Full);
      writeNeo(RGBwred);
      break;
    //--------------------------------------------------------------------------          
  }


  // Taste Minus gedrückt
  if (minusButton.pressed()) {
    #ifdef DEBUG
        Serial.println("\nMinus pressed...");
        Serial.flush();    
    #endif      
    switch(mode) {
      case NORMAL:
        ColorSw++;
        if (ColorSw > 10) {ColorSw = 0;}
        EEPROM.update(2, ColorSw);
        break;  
      case SetHr:
        if (clock.getHour(h12Flag, pmFlag) <= 0) {clock.setHour(23);}
        else {clock.setHour(clock.getHour(h12Flag, pmFlag) - 1);}
        break;
      case SetMin:
        if (clock.getMinute() <= 0) {clock.setMinute(59);}
        else {clock.setMinute(clock.getMinute() - 1);}
        break;
      case SetDay:
        if (clock.getDate() <= 1) {clock.setDate(31);}
        else {clock.setDate(clock.getDate() - 1);}
        break;
      case SetMon:
        if (clock.getMonth(century) <= 1) {clock.setMonth(12);}
        else {clock.setMonth(clock.getMonth(century) - 1);}
        break;
      case SetYear:
        if (clock.getYear() <= 0) {clock.setYear(99);}
        else {clock.setYear(clock.getYear() - 1);}
        break;
      case SetBrigh:
        if (BrSet <= 0) {BrSet = 0;}
        else {BrSet--;}
        EEPROM.update(3, BrSet);
        break;
    }
  }
  
  // Taste Plus gedrueckt?
  if (plusButton.pressed()) {
    #ifdef DEBUG
        Serial.println("\nPlus pressed...");
    #endif      
    switch(mode) {
      case NORMAL:
        SwitchMode++;
        if (SwitchMode > 3) {SwitchMode = 1;}  
        EEPROM.update(1, SwitchMode);    
        break;
      case SetHr:
        if (clock.getHour(h12Flag, pmFlag) >= 23) {clock.setHour(0);}
        else {clock.setHour(clock.getHour(h12Flag, pmFlag) + 1);}
        break;
      case SetMin:
        if (clock.getMinute() >= 59) {clock.setMinute(0);}
        else {clock.setMinute(clock.getMinute() + 1);}
        break;
      case SetDay:
        if (clock.getDate() >= 31) {clock.setDate(1);}
        else {clock.setDate(clock.getDate() + 1);}
        break;
      case SetMon:
        if (clock.getMonth(century) >= 12) {clock.setMonth(1);}
        else {clock.setMonth(clock.getMonth(century) + 1);}
        break;
      case SetYear:
        if (clock.getYear() >= 99) {clock.setYear(0);}
        else {clock.setYear(clock.getYear() + 1);}
        break;
      case SetBrigh:
        if (BrSet >= 9) {BrSet = 9;}
        else {BrSet++;}
        Serial.println("hoo+");
        EEPROM.update(3, BrSet);
        break;
    }
  }

  // Taste Moduswechsel gedrueckt?
  if (modeChangeButton.pressed()) {
    mode++;
    firstRound = true;
    if(mode == MAX) {
      mode = 0;
    }
    lastMode = mode;
  }

  // LDR einlesen und Helligkeit anpassen
  LdrVal = analogRead(LDR);
  // ldr < 50 = dunkel, > 950 = maxhell
  if (LdrVal < 50) {
    LdrVal = 50;
  }
  if (LdrVal > 950) {
    LdrVal = 1024;  
  }
  uint8_t BrightnessRaw = map(LdrVal, 50, 1024, 16, 254);
  if (BrightnessRaw < Brightness){Brightness = Brightness -1;}
  if (BrightnessRaw > Brightness){Brightness = Brightness +1;} 

  if (BrSet == 0) {
    strip.SetBrightness(Brightness);  
  }
  else {
    strip.SetBrightness(SetBrValue[BrSet]);  
  }

  // das ganze an die Leds senden
  strip.Show();
}


void mergeMatrix(int in[]) {
  int pix = 0;
	for(int row=0; row<10; row++) 
  {
    if( row == 0 or row == 9) {pix = 13;} else {pix = 11;}
    for(int col=0; col<pix; col++) 
    {
      if(bitRead(in[row], col) == 1)
      {
        bitWrite(matrix[row], col, 1);
      }
    }
	}
}


void generateClockMatrix(unsigned short h, unsigned short m) {
	if(h >= 12)
		h -= 12;
	
	clearMatrix();
	// Standard-Text (immer sichtbar)
	mergeMatrix(View_EsIst);
	//// Einzelne Minuten (1-4)
	switch(m%5){
		case 1:mergeMatrix(View_Min_1);break;
		case 2:mergeMatrix(View_Min_2);break;
		case 3:mergeMatrix(View_Min_3);break;
		case 4:mergeMatrix(View_Min_4);break;
	}
	// Minuten: 5er-Schritte
	int h_offset = 0;
	switch(m/5){
		case  0:mergeMatrix(View_Uhr);break; // ??:00
		case  1:mergeMatrix(View_Min05);break; // ??:05
		case  2:mergeMatrix(View_Min10);break; // ??:15
		case  3:mergeMatrix(View_Min15);break; // ??:15
		case  4:mergeMatrix(View_Min20);break; // ??:20
		case  5:mergeMatrix(View_Min25);h_offset=1;break; // ??:25
		case  6:mergeMatrix(View_Min30);h_offset=1;break; // ??:30
		case  7:mergeMatrix(View_Min35);h_offset=1;break; // ??:35
		case  8:mergeMatrix(View_Min40);h_offset=1;break; // ??:40
		case  9:mergeMatrix(View_Min45);h_offset=1;break; // ??:45
		case 10:mergeMatrix(View_Min50);h_offset=1;break; // ??:50
		case 11:mergeMatrix(View_Min55);h_offset=1;break; // ??:55
	}
	// Stunden
	switch(h + h_offset){
		case  1:mergeMatrix(View_Hour01);break;
		case  2:mergeMatrix(View_Hour02);break;
		case  3:mergeMatrix(View_Hour03);break;
		case  4:mergeMatrix(View_Hour04);break;
		case  5:mergeMatrix(View_Hour05);break;
		case  6:mergeMatrix(View_Hour06);break;
		case  7:mergeMatrix(View_Hour07);break;
		case  8:mergeMatrix(View_Hour08);break;
		case  9:mergeMatrix(View_Hour09);break;
		case 10:mergeMatrix(View_Hour10);break;
		case 11:mergeMatrix(View_Hour11);break;
		default:mergeMatrix(View_Hour12);break;
	}
	// Stunden-Korrektur 'Eins' -> 'Ein'
	if(h%12 == 1 and m/5 == 0) {bitWrite(matrix[5], 7, 0);}
}


void writeNeo(RgbwColor color) {
  byte pixE = 0;
	byte px   = 0;
  byte res  = 0;
  for(byte row=0; row<10; row++)
  {
  if( row == 0 or row == 9) {pixE = 13;} else {pixE = 11;}
      for(byte col=0; col<pixE; col++)
        {
        if((row%2) == 0) {res = col;} 
        else {res = pixE - col - 1;} 
        if(bitRead(matrix[row], res) == 1 ) {

          if (color != ignoreColor) {
            strip.SetPixelColor(px, RgbwColor(color));
          }
          else {
          // Farbe berechnen +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            switch (SwitchMode) {
              
              case 1: // sekuendlich angepasste farben
                if (clock.getSecond() != oldSecond) {
                  oldSecond = clock.getSecond();
                  regenbogenfarbe = (regenbogenfarbe + 1);
                  if (regenbogenfarbe == 360) {
                    regenbogenfarbe = 0;
                    }
                  }
                strip.SetPixelColor(px, HslColor((float)regenbogenfarbe / 360.0f, 1.0f, 0.25f));
                break;
              
              case 2: // Regenbogenfarben
                if (millis() - regenbogentimer  > regenbogeninterval) {
                  regenbogentimer = millis();
                  regenbogenfarbe = (regenbogenfarbe + 1);
                  if (regenbogenfarbe == 360) {
                    regenbogenfarbe = 0;
                  }
                }
                strip.SetPixelColor(px, HslColor(((float)regenbogenfarbe - (float)px) / 360.0f, 1.0f, 0.25f));
                break;

              case 3: // Fixe Farben
                if (ColorSw > 9) {
                  strip.SetPixelColor(px, RgbwColor(FarbVal[ColorSw]));
                  }
                  else {
                  strip.SetPixelColor(px, RgbwColor(HslColor(RgbColor(FarbVal[ColorSw]))));
                  }
                break;
            }                
          }
          // Farbe berechnen ende ------------------------------------------------------------------------

          #ifdef DEBUGmatrix
            if(px == 13 or px == 24 or px == 35 or px == 46 or px == 57 or px == 68 or px == 79 or px == 90) {Serial.print("   ");}
            Serial.print(" X ");
          #endif
        }
        else
        {
          #ifdef DEBUGmatrix
            if(px == 13 or px == 24 or px == 35 or px == 46 or px == 57 or px == 68 or px == 79 or px == 90) {Serial.print("   ");}
            Serial.print(" . ");
          #endif
          strip.SetPixelColor(px, black);
        }
			px++;
      }
		#ifdef DEBUGmatrix
			Serial.println();
		#endif
  }
  #ifdef DEBUGmatrix
  Serial.println();
  #endif
}


void clearMatrix() {
  for(int row=0; row<10; row++)
  {
	  matrix[row] = View_Blank[row];
  }
}
