#include "utils.h"
#include <stdlib.h>
#include <string.h>

// Fonction pour obtenir l'ID alphabétique d'un sommet
char *getId(int num) {
    static char buffer[10];
    buffer[0] = '\0';

    if (num <= 0) {
        return buffer;
    }

    // Conversion en base 26 (A-Z)
    int temp = num;
    int length = 0;

    // Calculer la longueur nécessaire
    while (temp > 0) {
        temp = (temp - 1) / 26;
        length++;
    }

    // Remplir le buffer de la fin vers le début
    buffer[length] = '\0';
    temp = num;

    for (int i = length - 1; i >= 0; i--) {
        int remainder = (temp - 1) % 26;
        buffer[i] = 'A' + remainder;
        temp = (temp - 1) / 26;
    }

    return buffer;
}

// Fonction pour calculer le minimum de deux entiers
int min(int a, int b) {
    return (a < b) ? a : b;
}
