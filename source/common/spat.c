/************************************************************************
 *  Common spatialisation code
 ************************************************************************/

#include "spat.h"
#include <math.h>

/************************************************************************
 *  Common spatialisation code (Optimized)
 ************************************************************************/

#include <math.h>
#include "spat.h"

// Fonction pour spatialiser sur n voix
double spat(double x, double d, int n)
{
    double x1 = fabs(fmod(x, 1.0) - 0.5);   // Équivalent à abs(x - 0.5)
    x1        = fmax(0, 1 - (x1 * n * d));  // Applique la linéarisation et contraint à 0 min
    return pow(x1, 0.5) * ((d / 2) + 0.5);  // Compensation pour la distance
}

// Fonction pour spatialiser sur 2 voix
double spat2(double x, double d)
{
    double x1 = fabs(fmod(x, 1.0) - 0.5);
    x1        = fmax(0, 1 - (x1 * 2 * d));  // Fonction linéaire
    return pow(x1 * ((d / 2) + 0.5), 0.5);
}

// Fonction de panning pour n sorties
void panner(double* out, int n, double teta, double d)
{
    double delta = 1.0 / (double)n;

    if (n == 1) {
        out[0] = 1.0;
        return;
    }

    if (n == 2) {
        out[0] = spat2(teta + 0.5, d);
        out[1] = spat2(teta, d);
        return;
    }

    int left_count  = n / 2;                 // Nombre de sorties à gauche du centre
    int right_count = (n - 1) - left_count;  // Nombre à droite

    out[0] = spat(teta + 0.5, d, n);

    // Remplissage des canaux gauche et droite
    for (int i = 1; i <= left_count; i++) {
        out[i] = spat(teta + 0.5 - i * delta, d, n);
    }
    for (int j = 1; j <= right_count; j++) {
        out[left_count + j] = spat(teta + 0.5 + j * delta, d, n);
    }
}
