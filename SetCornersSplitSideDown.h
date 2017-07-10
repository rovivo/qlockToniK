/**
 * SetCornersSplitSideDown.h
 * Klasse fuer das Setzten der Eck-Leds. Da man sich haeufig beim Verkabeln
 * vertut und dann die Software anpassen muss, sind die Eck-Leds hier ausgelagert.
 * Dann hat man es bei Software-Updates einfacher.
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.0
 * @created  21.3.2012
 *
 */
#ifndef SETCORNERSSPLITSIDEDOWN_H
#define SETCORNERSSPLITSIDEDOWN_H

/**
 * Setzt die vier Punkte in den Ecken, je nach minutes % 5 (Rest)
 */
void setCorners(int minutes) {
  switch (minutes % 5) {
  case 0:
    break;
  case 1:
    matrix[0] |= 0b0000000000011111;
    break;
  case 2:
    matrix[0] |= 0b0000000000011111;
    matrix[1] |= 0b0000000000011111;
    break;
  case 3:
    matrix[0] |= 0b0000000000011111;
    matrix[1] |= 0b0000000000011111;
    matrix[2] |= 0b0000000000011111;
    break;
  case 4:
    matrix[0] |= 0b0000000000011111;
    matrix[1] |= 0b0000000000011111;
    matrix[2] |= 0b0000000000011111;
    matrix[3] |= 0b0000000000011111;
    break;
  }
}

#endif
