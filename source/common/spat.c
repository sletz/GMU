/************************************************************************
 *  Common spatialisation code
 ************************************************************************/

#include "spat.h"
#include <math.h>

// Fonction pour spatialiser sur n voix
float spat(float x, float d, int n)
{
    float x1;

    x1 = fmod(x, 1.0);
    x1 = (x1 < 0.5 ? 0.5 - x1 : x1 - 0.5);  // x1 = abs(x - 0.5) ;
    x1 = (1 - (x1 * n * d));                // fct lineaire ;
    x1 = (x1 < 0 ? 0 : x1);                 //  max(0, x1 );
    return pow(x1, 0.5) * ((d / 2) + 0.5);  // (racine de x1) compensee pour la distance;
}

// Fonction pour spatialiser sur 2 voix
float spat2(float x, float d)
{
    float x1;

    x1 = fmod(x, 1.0);
    x1 = (x1 < 0.5 ? 0.5 - x1 : x1 - 0.5);  // x1 = abs(x - 0.5) ;
    x1 = (1 - (x1 * 2 * d));                // fct lineaire ;
    x1 = (x1 < 0 ? 0 : x1);                 //  max(0, (1 - (x1 * 2)) ;
    //	return pow(x1 , 0.5) * ((d / 2) + 0.5) ;	// (racine de x1) compensee pour la distance;
    return pow(x1 * ((d / 2) + 0.5), 0.5);  // (racine de x1) compensee pour la distance
}

// Panning suivant nombres de hps par voix(min:1  max:16)
// void pannerV(t_bufGranul *x, int voice)
void panner(double* out, int n, double teta, double d)
{
    // int n = x->x_nouts;
    double delta = 1. / (double)n;
    // double d = ((x->Vdist[voice] < 0 ? 0 : x->Vdist[voice]) > 1 ? 1 : x->Vdist[voice]);
    // double teta = (x->Vpan[voice] < 0 ? 0 : x->Vpan[voice]) ;

    switch (n) {
        case 1:
            out[0] = 1.;
        case 2:
            out[0] = spat2(teta + 0.5, d);
            out[1] = spat2(teta + 0.0, d);
            break;

        case 3:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 + (1 * delta), d, n);
            break;

        case 4:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 - (2 * delta), d, n);
            out[3] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 5:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 - (2 * delta), d, n);
            out[3] = spat(teta + 0.5 + (2 * delta), d, n);
            out[4] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 6:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 - (2 * delta), d, n);
            out[3] = spat(teta + 0.5 - (3 * delta), d, n);
            out[4] = spat(teta + 0.5 + (2 * delta), d, n);
            out[5] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 7:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 - (2 * delta), d, n);
            out[3] = spat(teta + 0.5 - (3 * delta), d, n);
            out[4] = spat(teta + 0.5 + (3 * delta), d, n);
            out[5] = spat(teta + 0.5 + (2 * delta), d, n);
            out[6] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 8:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 - (2 * delta), d, n);
            out[3] = spat(teta + 0.5 - (3 * delta), d, n);
            out[4] = spat(teta + 0.5 - (4 * delta), d, n);
            out[5] = spat(teta + 0.5 + (3 * delta), d, n);
            out[6] = spat(teta + 0.5 + (2 * delta), d, n);
            out[7] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 9:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 - (2 * delta), d, n);
            out[3] = spat(teta + 0.5 - (3 * delta), d, n);
            out[4] = spat(teta + 0.5 - (4 * delta), d, n);
            out[5] = spat(teta + 0.5 + (4 * delta), d, n);
            out[6] = spat(teta + 0.5 + (3 * delta), d, n);
            out[7] = spat(teta + 0.5 + (2 * delta), d, n);
            out[8] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 10:
            out[0] = spat(teta + 0.5, d, n);
            out[1] = spat(teta + 0.5 - (1 * delta), d, n);
            out[2] = spat(teta + 0.5 - (2 * delta), d, n);
            out[3] = spat(teta + 0.5 - (3 * delta), d, n);
            out[4] = spat(teta + 0.5 - (4 * delta), d, n);
            out[5] = spat(teta + 0.5 - (5 * delta), d, n);
            out[6] = spat(teta + 0.5 + (4 * delta), d, n);
            out[7] = spat(teta + 0.5 + (3 * delta), d, n);
            out[8] = spat(teta + 0.5 + (2 * delta), d, n);
            out[9] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 11:
            out[0]  = spat(teta + 0.5, d, n);
            out[1]  = spat(teta + 0.5 - (1 * delta), d, n);
            out[2]  = spat(teta + 0.5 - (2 * delta), d, n);
            out[3]  = spat(teta + 0.5 - (3 * delta), d, n);
            out[4]  = spat(teta + 0.5 - (4 * delta), d, n);
            out[5]  = spat(teta + 0.5 - (5 * delta), d, n);
            out[6]  = spat(teta + 0.5 + (5 * delta), d, n);
            out[7]  = spat(teta + 0.5 + (4 * delta), d, n);
            out[8]  = spat(teta + 0.5 + (3 * delta), d, n);
            out[9]  = spat(teta + 0.5 + (2 * delta), d, n);
            out[10] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
        case 12:
            out[0]  = spat(teta + 0.5, d, n);
            out[1]  = spat(teta + 0.5 - (1 * delta), d, n);
            out[2]  = spat(teta + 0.5 - (2 * delta), d, n);
            out[3]  = spat(teta + 0.5 - (3 * delta), d, n);
            out[4]  = spat(teta + 0.5 - (4 * delta), d, n);
            out[5]  = spat(teta + 0.5 - (5 * delta), d, n);
            out[6]  = spat(teta + 0.5 - (6 * delta), d, n);
            out[7]  = spat(teta + 0.5 + (5 * delta), d, n);
            out[8]  = spat(teta + 0.5 + (4 * delta), d, n);
            out[9]  = spat(teta + 0.5 + (3 * delta), d, n);
            out[10] = spat(teta + 0.5 + (2 * delta), d, n);
            out[11] = spat(teta + 0.5 + (1 * delta), d, n);
            break;

        case 13:
            out[0]  = spat(teta + 0.5, d, n);
            out[1]  = spat(teta + 0.5 - (1 * delta), d, n);
            out[2]  = spat(teta + 0.5 - (2 * delta), d, n);
            out[3]  = spat(teta + 0.5 - (3 * delta), d, n);
            out[4]  = spat(teta + 0.5 - (4 * delta), d, n);
            out[5]  = spat(teta + 0.5 - (5 * delta), d, n);
            out[6]  = spat(teta + 0.5 - (6 * delta), d, n);
            out[7]  = spat(teta + 0.5 + (6 * delta), d, n);
            out[8]  = spat(teta + 0.5 + (5 * delta), d, n);
            out[9]  = spat(teta + 0.5 + (4 * delta), d, n);
            out[10] = spat(teta + 0.5 + (3 * delta), d, n);
            out[11] = spat(teta + 0.5 + (2 * delta), d, n);
            out[12] = spat(teta + 0.5 + (1 * delta), d, n);
            break;

        case 14:
            out[0]  = spat(teta + 0.5, d, n);
            out[1]  = spat(teta + 0.5 - (1 * delta), d, n);
            out[2]  = spat(teta + 0.5 - (2 * delta), d, n);
            out[3]  = spat(teta + 0.5 - (3 * delta), d, n);
            out[4]  = spat(teta + 0.5 - (4 * delta), d, n);
            out[5]  = spat(teta + 0.5 - (5 * delta), d, n);
            out[6]  = spat(teta + 0.5 - (6 * delta), d, n);
            out[7]  = spat(teta + 0.5 - (7 * delta), d, n);
            out[8]  = spat(teta + 0.5 + (6 * delta), d, n);
            out[9]  = spat(teta + 0.5 + (5 * delta), d, n);
            out[10] = spat(teta + 0.5 + (4 * delta), d, n);
            out[11] = spat(teta + 0.5 + (3 * delta), d, n);
            out[12] = spat(teta + 0.5 + (2 * delta), d, n);
            out[13] = spat(teta + 0.5 + (1 * delta), d, n);
            break;

        case 15:
            out[0]  = spat(teta + 0.5, d, n);
            out[1]  = spat(teta + 0.5 - (1 * delta), d, n);
            out[2]  = spat(teta + 0.5 - (2 * delta), d, n);
            out[3]  = spat(teta + 0.5 - (3 * delta), d, n);
            out[4]  = spat(teta + 0.5 - (4 * delta), d, n);
            out[5]  = spat(teta + 0.5 - (5 * delta), d, n);
            out[6]  = spat(teta + 0.5 - (6 * delta), d, n);
            out[7]  = spat(teta + 0.5 - (7 * delta), d, n);
            out[8]  = spat(teta + 0.5 + (7 * delta), d, n);
            out[9]  = spat(teta + 0.5 + (6 * delta), d, n);
            out[10] = spat(teta + 0.5 + (5 * delta), d, n);
            out[11] = spat(teta + 0.5 + (4 * delta), d, n);
            out[12] = spat(teta + 0.5 + (3 * delta), d, n);
            out[13] = spat(teta + 0.5 + (2 * delta), d, n);
            out[14] = spat(teta + 0.5 + (1 * delta), d, n);
            break;

        case 16:
            out[0]  = spat(teta + 0.5, d, n);
            out[1]  = spat(teta + 0.5 - (1 * delta), d, n);
            out[2]  = spat(teta + 0.5 - (2 * delta), d, n);
            out[3]  = spat(teta + 0.5 - (3 * delta), d, n);
            out[4]  = spat(teta + 0.5 - (4 * delta), d, n);
            out[5]  = spat(teta + 0.5 - (5 * delta), d, n);
            out[6]  = spat(teta + 0.5 - (6 * delta), d, n);
            out[7]  = spat(teta + 0.5 - (7 * delta), d, n);
            out[8]  = spat(teta + 0.5 - (8 * delta), d, n);
            out[9]  = spat(teta + 0.5 + (7 * delta), d, n);
            out[10] = spat(teta + 0.5 + (6 * delta), d, n);
            out[11] = spat(teta + 0.5 + (5 * delta), d, n);
            out[12] = spat(teta + 0.5 + (4 * delta), d, n);
            out[13] = spat(teta + 0.5 + (3 * delta), d, n);
            out[14] = spat(teta + 0.5 + (2 * delta), d, n);
            out[15] = spat(teta + 0.5 + (1 * delta), d, n);
            break;
    }
}
