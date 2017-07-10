/**
 * SetCornersCCW.h
 * Klasse fuer das Setzten der Eck-Leds. Da man sich haeufig beim Verkabeln
 * vertut und dann die Software anpassen muss, sind die Eck-Leds hier ausgelagert.
 * Dann hat man es bei Software-Updates einfacher.
 *
 * Hier: CCW - Counter Clock Wise (gegen den Uhrzeigersinn)
 *
 * @mc       Arduino/RBBB
 * @autor    Christian Aschoff / caschoff _AT_ mac _DOT_ com
 * @version  1.0
 * @created  5.6.2012
 *
 */
#ifndef SETCORNERSCCW_H
#define SETCORNERSCCW_H

/**
 * Setzt die vier Punkte in den Ecken, je nach minutes % 5 (Rest).
 * Hat man sich in der Verkabelung vertan, muss man die Reihenfolge 
 * der Zahlen in den eckigen Klammern tauschen (aber es bleibt bei
 * den Zahlen 0-3, die die Reihe der Matrix angeben, an die man die
 * Anoden angeloetet hat).
 */
void setCorners(int minutes) {
  switch (minutes % 5) {
  case 0:
    break;
  case 1:
    matrix[1] |= 0b0000000000011111; // 1
    break;
  case 2:
    matrix[1] |= 0b0000000000011111; // 1
    matrix[0] |= 0b0000000000011111; // 2
    break;
  case 3:
    matrix[1] |= 0b0000000000011111; // 1
    matrix[0] |= 0b0000000000011111; // 2
    matrix[2] |= 0b0000000000011111; // 3
    break;
  case 4:
    matrix[1] |= 0b0000000000011111; // 1
    matrix[0] |= 0b0000000000011111; // 2
    matrix[2] |= 0b0000000000011111; // 3
    matrix[3] |= 0b0000000000011111; // 4
    break;
  }
}

#endif

