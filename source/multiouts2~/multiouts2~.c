/**
 * @file multiouts~.c
 * Objet MSP multiouts à nombre de sorties dynamique (refactorisé)
 *
 *  LP multiouts objet dynamiquement modifiable nombre de sorties
 *                janvier 2003 - GMEM
 *
 * Cette version de mars 2025 restructure le code en :
 *  - Remplaçant les 16 variables individuelles pour les gains par des tableaux dynamiques.
 *  - Unifiant le calcul des gains dans une fonction compute_gains().
 *  - Unifiant la fonction perform DSP pour traiter n’importe quel nombre de sorties.
 *
 *  Updated: 2025-03-07
 */

#include <math.h>
#include <stdlib.h>
#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

void* multiouts_class;

typedef struct _multiouts {
    t_pxobject x_obj;
    double     x_scal;      // Facteur de distance (0 à 1)
    double     x_teta;      // Direction (0 à 1, correspond à un pourcentage de 360°)
    long       x_sr;        // Taux d’échantillonnage
    int        x_nsamp;     // Nombre d’échantillons pour le ramp
    int        x_nouts;     // Nombre de sorties
    int        x_position;  // Position dans la rampe (compte à rebours)

    // Tableaux dynamiques pour le traitement :
    double* hp_prev;  // Valeurs de gain en cours (précédentes)
    double* hp_next;  // Valeurs de gain cibles (calculées)
    double* inc;      // Incréments par échantillon (pour le ramp)
} t_multiouts;

/* Prototypes des fonctions utilitaires */
double spat(double x, double d, int n);
double spat2(double x, double d);

void compute_gains(t_multiouts* x);

/* Méthodes DSP */
void  multiouts_dsp64(t_multiouts* x, t_object* dsp64, short* count, double samplerate,
                      long maxvectorsize, long flags);
void* multiouts_perform_generic(t_multiouts* x, t_object* dsp64, double** ins, long numins,
                                double** outs, long numouts, long sampleframes, long flags,
                                void* userparam);

/* Méthodes de messages */
void multiouts_ft1(t_multiouts* x, double f);  // Définit la direction
void multiouts_ft2(t_multiouts* x, double f);  // Définit la distance
void multiouts_in3(t_multiouts* x, long n);    // Définit le temps de la rampe en ms

void  multiouts_assist(t_multiouts* x, void* b, long m, long a, char* s);
void* multiouts_new(double nouts);
void  multiouts_free(t_multiouts* x);

/************************** Fonction principale **************************/

void ext_main(void* r)
{
    t_class* c;
    c = class_new("multiouts~", (method)multiouts_new, (method)multiouts_free, sizeof(t_multiouts),
                  0L, A_DEFFLOAT, A_DEFFLOAT, 0);

    class_addmethod(c, (method)multiouts_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)multiouts_ft1, "ft1", A_FLOAT, 0);
    class_addmethod(c, (method)multiouts_ft2, "ft2", A_FLOAT, 0);
    class_addmethod(c, (method)multiouts_in3, "in3", A_LONG, 0);
    class_addmethod(c, (method)multiouts_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    multiouts_class = c;
}

/************************** Assistance **************************/

void multiouts_assist(t_multiouts* x, void* b, long m, long a, char* s)
{
    if (m == 2) {
        sprintf(s, "(signal) Sortie");
    } else {
        switch (a) {
            case 0:
                sprintf(s, "(signal) Entrée");
                break;
            case 1:
                sprintf(s, "(float) Direction (0-1 : pourcentage de 360°), défaut 0");
                break;
            case 2:
                sprintf(s, "(float) Distance (0-1), défaut 1");
                break;
            case 3:
                sprintf(s, "(int) Temps de ramp (ms)");
                break;
        }
    }
}

/************************** Allocation et libération **************************/

void* multiouts_new(double nouts)
{
    t_multiouts* x = (t_multiouts*)object_alloc(multiouts_class);
    dsp_setup((t_pxobject*)x, 1);  // une entrée signal
    intin((t_pxobject*)x, 3);      // 3ème inlet pour le temps de ramp
    floatin((t_pxobject*)x, 2);    // 2ème inlet pour la distance
    floatin((t_pxobject*)x, 1);    // 1ère inlet pour la direction

    x->x_nouts = (int)nouts;
    if (x->x_nouts < 2) {
        x->x_nouts = 2;  // minimum 2 sorties
    }

    // Création d'un nombre de sorties égal à x_nouts
    for (int i = 0; i < x->x_nouts; i++) {
        outlet_new((t_pxobject*)x, "signal");
    }

    x->x_sr       = sys_getsr();
    x->x_nsamp    = (int)(x->x_sr * 0.05);  // 50 msec par défaut
    x->x_position = x->x_nsamp;

    x->x_scal = 1.;
    x->x_teta = 0.;

    /* Allocation des tableaux dynamiques */
    x->hp_prev = (double*)sysmem_newptr(x->x_nouts * sizeof(double));
    x->hp_next = (double*)sysmem_newptr(x->x_nouts * sizeof(double));
    x->inc     = (double*)sysmem_newptr(x->x_nouts * sizeof(double));

    /* Initialisation : la première sortie démarre à 1, le reste à 0 */
    x->hp_prev[0] = 1.0;
    for (int i = 1; i < x->x_nouts; i++) {
        x->hp_prev[i] = 0.;
    }

    /* Initialement, les gains cibles sont égaux aux gains précédents */
    for (int i = 0; i < x->x_nouts; i++) {
        x->hp_next[i] = x->hp_prev[i];
        x->inc[i]     = 0.;
    }

    return (x);
}

void multiouts_free(t_multiouts* x)
{
    dsp_free((t_pxobject*)x);
    if (x->hp_prev) {
        sysmem_freeptr(x->hp_prev);
    }
    if (x->hp_next) {
        sysmem_freeptr(x->hp_next);
    }
    if (x->inc) {
        sysmem_freeptr(x->inc);
    }
}

/************************** Fonctions utilitaires **************************/

// Calcul d'un gain en fonction de x, d et du nombre de sorties n
double spat(double x, double d, int n)
{
    double x1 = fmod(x, 1.0);
    x1        = (x1 < 0.5 ? 0.5 - x1 : x1 - 0.5);
    x1        = (1 - (x1 * n * d));
    x1        = (x1 < 0 ? 0 : x1);
    return pow(x1, 0.5) * ((d / 2) + 0.5);
}

// Variante pour 2 sorties
double spat2(double x, double d)
{
    double x1 = fmod(x, 1.0);
    x1        = (x1 < 0.5 ? 0.5 - x1 : x1 - 0.5);
    x1        = (1 - (x1 * 2 * d));
    x1        = (x1 < 0 ? 0 : x1);
    return pow(x1 * ((d / 2) + 0.5), 0.5);
}

// Calcul générique des gains cibles et des incréments de ramp
void compute_gains(t_multiouts* x)
{
    int n = x->x_nouts;
    if (n == 2) {
        x->hp_next[0] = spat2(x->x_teta + 0.5, x->x_scal);
        x->hp_next[1] = spat2(x->x_teta + 0.0, x->x_scal);
    } else if (n >= 3) {
        double delta       = 1.0 / (double)n;
        int    left_count  = n / 2;                 // nombre de sorties à gauche du centre
        int    right_count = (n - 1) - left_count;  // nombre de sorties à droite

        x->hp_next[0] = spat(x->x_teta + 0.5, x->x_scal, n);
        for (int i = 1; i <= left_count; i++) {
            x->hp_next[i] = spat(x->x_teta + 0.5 - i * delta, x->x_scal, n);
        }
        for (int j = 1; j <= right_count; j++) {
            /* Les sorties de droite sont assignées en ordre décroissant */
            x->hp_next[left_count + j] =
                spat(x->x_teta + 0.5 + (right_count - j + 1) * delta, x->x_scal, n);
        }
    }

    /* Calcul des incréments de ramp pour chaque sortie */
    for (int i = 0; i < x->x_nouts; i++) {
        x->inc[i] = (x->hp_next[i] - x->hp_prev[i]) / (x->x_nsamp + 1);
    }
    x->x_position = x->x_nsamp;
}

/************************** Messages de réglage **************************/

// Mise à jour de la direction
void multiouts_ft1(t_multiouts* x, double teta)
{
    teta      = (teta < 0 ? 0 : teta);
    x->x_teta = teta;
    compute_gains(x);
}

// Mise à jour de la distance
void multiouts_ft2(t_multiouts* x, double d)
{
    d         = (d > 1. ? 1. : d);
    d         = (d < 0. ? 0. : d);
    x->x_scal = d;
    compute_gains(x);
}

// Réglage du temps de ramp en millisecondes
void multiouts_in3(t_multiouts* x, long n)
{
    x->x_nsamp = (int)(x->x_sr * n / 1000);
}

/************************** DSP **************************/

void multiouts_dsp64(t_multiouts* x, t_object* dsp64, short* count, double samplerate,
                     long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform_generic, 0, NULL);
}

// Fonction perform générique traitant un nombre arbitraire de sorties
void* multiouts_perform_generic(t_multiouts* x, t_object* dsp64, double** ins, long numins,
                                double** outs, long numouts, long sampleframes, long flags,
                                void* userparam)
{
    t_double* in       = ins[0];
    int       n        = x->x_nouts;
    long      position = x->x_position;

    for (long j = 0; j < sampleframes; j++) {
        double sample = in[j];
        if (position-- > 0) {
            for (int i = 0; i < n; i++) {
                x->hp_prev[i] += x->inc[i];
                outs[i][j] = sample * x->hp_prev[i];
            }
        } else {
            for (int i = 0; i < n; i++) {
                x->hp_prev[i] = x->hp_next[i];
                outs[i][j]    = sample * x->hp_prev[i];
            }
        }
    }

    x->x_position = position;
    return userparam;
}
