/**
 * SetHours_DE.h
 * Klasse fuer das Setzten der Stunden in Deutsch.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.0
 * @created  18.3.2012
 *
 */
#ifndef SETHOURS_DE_H
#define SETHOURS_DE_H

/**
 * Setzt die Stunden, je nach hours. 'glatt' bedeutet,
 * es ist genau diese Stunde und wir muessen 'UHR'
 * dazuschreiben und EIN statt EINS, falls es 1 ist.
 * (Zumindest im Deutschen)
 */
void setHours(int hours, boolean glatt) {
  while (hours > 12) {
    hours -= 12;
  }

  if (glatt) {
    UHR;
  }

  switch (hours) {
  case 0:
  case 12:
  case 24:
    H_ZWOELF;
    break;
  case 1:
  case 13:
    if (glatt) {
      H_EIN;
    } 
    else {
      H_EINS;
    }
    break;
  case 2:
  case 14:
    H_ZWEI;
    break;
  case 3:
  case 15:
    H_DREI;
    break;
  case 4:
  case 16:
    H_VIER;
    break;
  case 5:
  case 17:
    H_FUENF;
    break;
  case 6:
  case 18:
    H_SECHS;
    break;
  case 7:
  case 19:
    H_SIEBEN;
    break;
  case 8:
  case 20:
    H_ACHT;
    break;
  case 9:
  case 21:
    H_NEUN;
    break;
  case 10:
  case 22:
    H_ZEHN;
    break;
  case 11:
  case 23:
    H_ELF;
    //H_ELF2;
    //H_ELF3;
    break;
  }
}

#endif
