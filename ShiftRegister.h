/**
 * ShiftRegister.h
 * Mit der ShiftRegister-Klasse wird das Ausgeben der Bitmuster auf die 
 * 74HC595-Shift-Register gekapselt.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.2
 * @created  24.2.2011
 * @updated  13.2.2012
 *
 * Versionshistorie:
 * V 1.1:  - Schnelle DigitalWrite-Methoden eingefuehrt.
 *           Ueber die Definition von SHIFTREGISTER_TURBO
 *           kann es eingeschaltet werden.
 * V 1.2:  - Kompatibilitaet zu Arduino-IDE 1.0 hergestellt.
 */
#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define SHIFTREGISTER_TURBO

class ShiftRegister {
public:
  ShiftRegister(int dataPin, int clockPin, int latchPin);

  void shiftOut(word data);

  void prepareShiftregisterWrite();
  void finishShiftregisterWrite();

private:
  // slow version
  int _dataPin;
  int _clockPin;
  int _latchPin;

  // fast version
  uint8_t _dataBit;
  uint8_t _dataPort;
  volatile uint8_t *_dataOut;    

  uint8_t _clockBit;
  uint8_t _clockPort;
  volatile uint8_t *_clockOut;    

  uint8_t _latchBit;
  uint8_t _latchPort;
  volatile uint8_t *_latchOut;    

  void fastDigitalWriteToData(uint8_t val);
  void fastDigitalWriteToClock(uint8_t val);
  void fastDigitalWriteToLatch(uint8_t val);
};

#endif

