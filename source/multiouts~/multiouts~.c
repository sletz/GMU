/**
 @file
 multiouts~ - LP multiouts objet dynamiquement modifiable nombre de sorties
 
 updated 12/26/17
 
 @ingroup	LP
 */
#include "ext.h"
#include "z_dsp.h"
#include "math.h"
#include "ext_obex.h"

/*******************************************************************/
/*  LP multiouts objet dynamiquement modifiable nombre de sorties  */
/*                janvier 2003 - GMEM                              */
/*******************************************************************/

void *multiouts_class;

typedef struct _multiouts
{
    t_pxobject x_obj;
	double x_scal;		// distance de la source entre le centre (0.) et les HP (1.)
	double x_teta;		// angle entre 0. et 1.****
	
    double x_hp1scp; 	// vol de chaque HP avec correction de distance (prev)
	double x_hp2scp;
	double x_hp3scp;
	double x_hp4scp;
	double x_hp5scp;
	double x_hp6scp;
	double x_hp7scp;
	double x_hp8scp;
	double x_hp9scp;
	double x_hp10scp;
	double x_hp11scp;
	double x_hp12scp;
	double x_hp13scp;
	double x_hp14scp;
	double x_hp15scp;
	double x_hp16scp;
    
	double x_hp1scn; 	// vol de chaque HP avec correction de distance (next)
	double x_hp2scn;
	double x_hp3scn;
	double x_hp4scn;
	double x_hp5scn;
	double x_hp6scn;
	double x_hp7scn;
	double x_hp8scn;
	double x_hp9scn;
	double x_hp10scn;
	double x_hp11scn;
	double x_hp12scn;
	double x_hp13scn;
	double x_hp14scn;
	double x_hp15scn;
	double x_hp16scn;
    
	double x_x1; 			// (hpi-next - hpi-prev) / nsamp = distance à parcourir
	double x_x2;
	double x_x3;
	double x_x4;
	double x_x5;
	double x_x6;
	double x_x7;
	double x_x8;
	double x_x9;
	double x_x10;
	double x_x11;
	double x_x12;
	double x_x13;
	double x_x14;
	double x_x15;
	double x_x16;
    
    long  x_sr;
    long  x_position;	/* position entre nsamp et < 0 */
    int  x_nsamp;		/* number of samples during each line sr/f */
    int  x_nouts;		/* number of outputs */
} t_multiouts;

double spat (double x, double d, int n);
double spat2 (double x, double d);
void multiouts_ft1(t_multiouts *x, double f);     // direction
void multiouts_ft2(t_multiouts *x, double f);     // distance
void multiouts_in3(t_multiouts *x, long f);       // line time

void multiouts_floatval2(t_multiouts *x) ;
void multiouts_floatval2(t_multiouts *x);
void multiouts_floatval3(t_multiouts *x);
void multiouts_floatval4(t_multiouts *x);
void multiouts_floatval5(t_multiouts *x);
void multiouts_floatval6(t_multiouts *x);
void multiouts_floatval7(t_multiouts *x);
void multiouts_floatval8(t_multiouts *x);
void multiouts_floatval9(t_multiouts *x);
void multiouts_floatval10(t_multiouts *x);
void multiouts_floatval11(t_multiouts *x);
void multiouts_floatval12(t_multiouts *x);
void multiouts_floatval13(t_multiouts *x);
void multiouts_floatval14(t_multiouts *x);
void multiouts_floatval15(t_multiouts *x);
void multiouts_floatval16(t_multiouts *x);

// void multiouts_dsp(t_multiouts *x, t_signal **sp, short *count);
void multiouts_dsp64(t_multiouts *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);

void multiouts_float(t_multiouts *x, double f);
void multiouts_int(t_multiouts *x, long n);
// void multiouts_clear(t_multiouts *x);
void multiouts_assist(t_multiouts *x, void *b, long m, long a, char *s);
void *multiouts_new(double val);



void ext_main(void *r)
{
	t_class *c ;
    c = class_new("multiouts~", (method)multiouts_new, (method)dsp_free, sizeof(t_multiouts), NULL, A_DEFFLOAT,A_DEFFLOAT, 0);
    
	// class_addmethod(c, (method)multiouts_dsp, "dsp", A_CANT, 0); 	// respond to the dsp message
	class_addmethod(c, (method)multiouts_dsp64, "dsp64", A_CANT, 0); 	// respond to the dsp message
    class_addmethod(c, (method)multiouts_ft1, "ft1", A_FLOAT, 0);
    class_addmethod(c, (method)multiouts_ft2, "ft2", A_FLOAT, 0);
	class_addmethod(c, (method)multiouts_in3, "in3", A_LONG, 0);
	// class_addmethod(c, (method)multiouts_clear, "clear", 0);
	class_addmethod(c, (method)multiouts_assist,"assist",A_CANT,0);
    
    class_dspinit(c);							// must call this function for MSP object classes
    
	class_register(CLASS_BOX, c);
	multiouts_class = c;
}

void multiouts_assist(t_multiouts *x, void *b, long m, long a, char *s)
{
	if (m == 2)
		sprintf(s,"(signal) Output");
	else {
		switch (a) {
            case 0: sprintf(s,"(signal) Input"); break;
            case 1: sprintf(s,"(float) Direction (0-1) (percent of 360 clockwise) default 0 => out1"); break;
            case 2: sprintf(s,"(float) Distance (0-1) default 1"); break;
            case 3: sprintf(s,"(int) Line Time (ms)"); break;
		}
	}
}


void *multiouts_new(double val)
{
	t_multiouts *x = object_alloc(multiouts_class);
	dsp_setup((t_pxobject *)x,1);	// set up DSP for the instance and create signal inlets
	intin  ((t_pxobject *)x,3);
  	floatin((t_pxobject *)x,2);
   	floatin((t_pxobject *)x,1);
    
   	x->x_nouts = val;
  	// x->x_teta = 0.;
  	// x->x_scal = 1.;
    
 	outlet_new((t_pxobject *)x, "signal");
    outlet_new((t_pxobject *)x, "signal");
    
    if (val > 2)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 3)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 4)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 5)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 6)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 7)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 8)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 9)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 10)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 11)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 12)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 13)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 14)
    	outlet_new((t_pxobject *)x, "signal");
    if (val > 15)
    	outlet_new((t_pxobject *)x, "signal");
    x->x_sr = sys_getsr() ;
    
    x->x_x1 = 0. ;
	x->x_x2 = 0. ;
	x->x_x3 = 0. ;
	x->x_x4 = 0. ;
	x->x_x5 = 0. ;
	x->x_x6 = 0. ;
	x->x_x7 = 0. ;
	x->x_x8 = 0. ;
	x->x_x9 = 0. ;
	x->x_x10 = 0. ;
	x->x_x11 = 0. ;
	x->x_x12 = 0. ;
	x->x_x13 = 0. ;
	x->x_x14 = 0. ;
	x->x_x15 = 0. ;
	x->x_x16 = 0. ;
    
	x->x_hp1scp	= 1. ;
	x->x_hp2scp	= 0. ;
	x->x_hp3scp	= 0. ;
	x->x_hp4scp	= 0. ;
	x->x_hp5scp	= 0. ;
	x->x_hp6scp	= 0. ;
	x->x_hp7scp	= 0. ;
	x->x_hp8scp	= 0. ;
	x->x_hp9scp	= 0. ;
	x->x_hp10scp = 0. ;
	x->x_hp11scp = 0. ;
	x->x_hp12scp = 0. ;
	x->x_hp13scp = 0. ;
	x->x_hp14scp = 0. ;
	x->x_hp15scp = 0. ;
	x->x_hp16scp = 0. ;
    
	x->x_hp1scn	= 0. ;
	x->x_hp2scn	= 0. ;
	x->x_hp3scn	= 0. ;
	x->x_hp4scn	= 0. ;
	x->x_hp5scn	= 0. ;
	x->x_hp6scn	= 0. ;
	x->x_hp7scn	= 0. ;
	x->x_hp8scn	= 0. ;
	x->x_hp9scn	= 0. ;
	x->x_hp10scn = 0. ;
	x->x_hp11scn = 0. ;
	x->x_hp12scn = 0. ;
	x->x_hp13scn = 0. ;
	x->x_hp14scn = 0. ;
	x->x_hp15scn = 0. ;
	x->x_hp16scn = 0. ;
    
    x->x_nsamp 	= (t_int)(x->x_sr*0.05) ;       // 50 msec par defaut
	x->x_position =  x->x_nsamp ;
	x->x_scal = 1. ;
    x->x_teta = 0. ;
    
    return (x);
}

/*****************************************/
double spat2 (double x, double d)
{
	double x1;
	
	x1 = fmod(x, 1.0) ;
	x1 = (x1 < 0.5 ? 0.5 - x1 : x1 - 0.5) ; // x1 = abs(x - 0.5) ;
	x1 = (1 - (x1 * 2 * d)) ; 				// fct linéaire ;
	x1 = (x1 < 0 ? 0 : x1) ; 				//  max(0, (1 - (x1 * 2)) ;
	// return pow(x1 , 0.5) * ((d / 2) + 0.5) ;// (racine de x1) compensée pour la distance; ? ou
	return pow(x1 * ((d / 2) + 0.5) , 0.5)  ; //   racine de (x1 compensé pour la distance) ?
}

double spat (double x, double d, int n)
{
	double x1;
	
	x1 = fmod(x, 1.0) ;
	x1 = (x1 < 0.5 ? 0.5 - x1 : x1 - 0.5) ; // x1 = abs(x - 0.5) ;
	x1 = (1 - (x1 * n * d)) ; // fct linéaire ;
	x1 = (x1 < 0 ? 0 : x1) ; //  max(0, (1 - (x1 * 3)) ;
	return pow(x1 , 0.5) * ((d / 2) + 0.5) ; // (racine de x1) compensée pour la distance;
}


/***************************************/

void multiouts_ft1(t_multiouts *x, double teta)
{
	teta = (teta < 0 ? 0 : teta) ;
	x->x_teta = teta ;
    
    switch (x->x_nouts) {
		case 2: multiouts_floatval2 (x) ;
			break ;
		case 3: multiouts_floatval3 (x) ;
			break ;
		case 4: multiouts_floatval4 (x) ;
			break ;
		case 5: multiouts_floatval5 (x) ;
			break ;
		case 6: multiouts_floatval6 (x) ;
			break ;
		case 7: multiouts_floatval7 (x) ;
			break ;
		case 8: multiouts_floatval8 (x) ;
			break ;
		case 9: multiouts_floatval9 (x) ;
			break ;
		case 10: multiouts_floatval10 (x) ;
			break ;
		case 11: multiouts_floatval11 (x) ;
			break ;
		case 12: multiouts_floatval12 (x) ;
			break ;
		case 13: multiouts_floatval13 (x) ;
			break ;
		case 14: multiouts_floatval14 (x) ;
			break ;
		case 15: multiouts_floatval15 (x) ;
			break ;
		case 16: multiouts_floatval16 (x) ;
	}
	x->x_x1 = (x->x_hp1scn - x->x_hp1scp) / (x->x_nsamp + 1) ;
	x->x_x2 = (x->x_hp2scn - x->x_hp2scp) / (x->x_nsamp + 1) ;
	x->x_x3 = (x->x_hp3scn - x->x_hp3scp) / (x->x_nsamp + 1) ;
	x->x_x4 = (x->x_hp4scn - x->x_hp4scp) / (x->x_nsamp + 1) ;
	x->x_x5 = (x->x_hp5scn - x->x_hp5scp) / (x->x_nsamp + 1) ;
	x->x_x6 = (x->x_hp6scn - x->x_hp6scp) / (x->x_nsamp + 1) ;
	x->x_x7 = (x->x_hp7scn - x->x_hp7scp) / (x->x_nsamp + 1) ;
	x->x_x8 = (x->x_hp8scn - x->x_hp8scp) / (x->x_nsamp + 1) ;
	x->x_x9 = (x->x_hp9scn - x->x_hp9scp) / (x->x_nsamp + 1) ;
	x->x_x10 = (x->x_hp10scn - x->x_hp10scp) / (x->x_nsamp + 1) ;
	x->x_x11 = (x->x_hp11scn - x->x_hp11scp) / (x->x_nsamp + 1) ;
	x->x_x12 = (x->x_hp12scn - x->x_hp12scp) / (x->x_nsamp + 1) ;
	x->x_x13 = (x->x_hp13scn - x->x_hp13scp) / (x->x_nsamp + 1) ;
	x->x_x14 = (x->x_hp14scn - x->x_hp14scp) / (x->x_nsamp + 1) ;
	x->x_x15 = (x->x_hp15scn - x->x_hp15scp) / (x->x_nsamp + 1) ;
	x->x_x16 = (x->x_hp16scn - x->x_hp16scp) / (x->x_nsamp + 1) ;
	x->x_position =  x->x_nsamp ;
}

void multiouts_floatval2(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	x->x_hp1scn = spat2( teta + 0.5, d) ;
	x->x_hp2scn = spat2( teta + 0.0, d) ;
}
void multiouts_floatval3(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval4(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval5(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval6(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval7(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval8(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval9(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval10(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 - (5 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp10scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval11(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 - (5 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 + (5 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp10scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp11scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}
void multiouts_floatval12(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 - (5 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 - (6 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 + (5 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp10scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp11scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp12scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}

void multiouts_floatval13(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 - (5 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 - (6 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 + (6 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 + (5 * delta), d, n) ;
	x->x_hp10scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp11scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp12scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp13scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}

void multiouts_floatval14(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 - (5 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 - (6 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 - (7 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 + (6 * delta), d, n) ;
	x->x_hp10scn = spat( teta + 0.5 + (5 * delta), d, n) ;
	x->x_hp11scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp12scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp13scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp14scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}


void multiouts_floatval15(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 - (5 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 - (6 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 - (7 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 + (7 * delta), d, n) ;
	x->x_hp10scn = spat( teta + 0.5 + (6 * delta), d, n) ;
	x->x_hp11scn = spat( teta + 0.5 + (5 * delta), d, n) ;
	x->x_hp12scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp13scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp14scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp15scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}

void multiouts_floatval16(t_multiouts *x)
{
	double teta = x->x_teta ;
	float d = x->x_scal ;
	int n = x->x_nouts ;
	float delta = 1./(float)n ;
	x->x_hp1scn = spat( teta + 0.5, d, n) ;
	x->x_hp2scn = spat( teta + 0.5 - (1 * delta), d, n) ;
	x->x_hp3scn = spat( teta + 0.5 - (2 * delta), d, n) ;
	x->x_hp4scn = spat( teta + 0.5 - (3 * delta), d, n) ;
	x->x_hp5scn = spat( teta + 0.5 - (4 * delta), d, n) ;
	x->x_hp6scn = spat( teta + 0.5 - (5 * delta), d, n) ;
	x->x_hp7scn = spat( teta + 0.5 - (6 * delta), d, n) ;
	x->x_hp8scn = spat( teta + 0.5 - (7 * delta), d, n) ;
	x->x_hp9scn = spat( teta + 0.5 - (8 * delta), d, n) ;
	x->x_hp10scn = spat( teta + 0.5 + (7 * delta), d, n) ;
	x->x_hp11scn = spat( teta + 0.5 + (6 * delta), d, n) ;
	x->x_hp12scn = spat( teta + 0.5 + (5 * delta), d, n) ;
	x->x_hp13scn = spat( teta + 0.5 + (4 * delta), d, n) ;
	x->x_hp14scn = spat( teta + 0.5 + (3 * delta), d, n) ;
	x->x_hp15scn = spat( teta + 0.5 + (2 * delta), d, n) ;
	x->x_hp16scn = spat( teta + 0.5 + (1 * delta), d, n) ;
}


/***************************************/
void multiouts_ft2(t_multiouts *x, double d)
{
	d = (d > 1. ? 1. : d) ;
	d = (d < 0. ? 0. : d) ;
	x->x_scal = d ;
    switch (x->x_nouts) {
		case 2: multiouts_floatval2 (x) ;
			break ;
		case 3: multiouts_floatval3 (x) ;
			break ;
		case 4: multiouts_floatval4 (x) ;
			break ;
		case 5: multiouts_floatval5 (x) ;
			break ;
		case 6: multiouts_floatval6 (x) ;
			break ;
		case 7: multiouts_floatval7 (x) ;
			break ;
		case 8: multiouts_floatval8 (x) ;
			break ;
		case 9: multiouts_floatval9 (x) ;
			break ;
		case 10: multiouts_floatval10 (x) ;
			break ;
		case 11: multiouts_floatval11 (x) ;
			break ;
		case 12: multiouts_floatval12 (x) ;
			break ;
		case 13: multiouts_floatval13 (x) ;
			break ;
		case 14: multiouts_floatval14 (x) ;
			break ;
		case 15: multiouts_floatval15 (x) ;
			break ;
		case 16: multiouts_floatval16 (x) ;
	}
	x->x_x1 = (x->x_hp1scn - x->x_hp1scp) / (x->x_nsamp + 1) ;
	x->x_x2 = (x->x_hp2scn - x->x_hp2scp) / (x->x_nsamp + 1) ;
	x->x_x3 = (x->x_hp3scn - x->x_hp3scp) / (x->x_nsamp + 1) ;
	x->x_x4 = (x->x_hp4scn - x->x_hp4scp) / (x->x_nsamp + 1) ;
	x->x_x5 = (x->x_hp5scn - x->x_hp5scp) / (x->x_nsamp + 1) ;
	x->x_x6 = (x->x_hp6scn - x->x_hp6scp) / (x->x_nsamp + 1) ;
	x->x_x7 = (x->x_hp7scn - x->x_hp7scp) / (x->x_nsamp + 1) ;
	x->x_x8 = (x->x_hp8scn - x->x_hp8scp) / (x->x_nsamp + 1) ;
	x->x_x9 = (x->x_hp9scn - x->x_hp9scp) / (x->x_nsamp + 1) ;
	x->x_x10 = (x->x_hp10scn - x->x_hp10scp) / (x->x_nsamp + 1) ;
	x->x_x11 = (x->x_hp11scn - x->x_hp11scp) / (x->x_nsamp + 1) ;
	x->x_x12 = (x->x_hp12scn - x->x_hp12scp) / (x->x_nsamp + 1) ;
	x->x_x13 = (x->x_hp13scn - x->x_hp13scp) / (x->x_nsamp + 1) ;
	x->x_x14 = (x->x_hp14scn - x->x_hp14scp) / (x->x_nsamp + 1) ;
	x->x_x15 = (x->x_hp15scn - x->x_hp15scp) / (x->x_nsamp + 1) ;
	x->x_x16 = (x->x_hp16scn - x->x_hp16scp) / (x->x_nsamp + 1) ;
	x->x_position =  x->x_nsamp ;
}

/***************************************/

void multiouts_in3(t_multiouts *x, long n)
{
    x->x_nsamp 	= (t_int)(x->x_sr*n/1000) ;
}

/***************************************************************/
/****************** PERFORMS METHODS ***************************/
/***************************************************************/

void multiouts_perform2(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double val1, val2;
	double valf1, valf2;
	double x1, x2;
	double  insig ;
	long position ;
    
    t_double	*in = ins[0];           // Input 1
	t_double	*out1 = outs[0];		// Output 1
	t_double	*out2 = outs[1];		// Output 2
    
  	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
    
	while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
   		 	val1 = valf1 ;
   		 	val2 = valf2 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
  			x1 = 0. ;
   			x2 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	
	x->x_position = position ;

}



void *multiouts_perform3(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    double val1, val2, val3;
	double valf1, valf2, valf3;
	float x1, x2, x3;
	double  insig ;
	long position ;
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
   		 	val1 = valf1 ;
   		 	val2 = valf2 ;
   		 	val3 = valf3 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform4(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4;
	float valf1, valf2, valf3, valf4;
	float x1, x2, x3, x4;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
    
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
    
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform5(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5;
	float valf1, valf2, valf3, valf4, valf5;
	float x1, x2, x3, x4, x5;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
    
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
    
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform6(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6;
	float valf1, valf2, valf3, valf4, valf5, valf6;
	float x1, x2, x3, x4, x5, x6;
	double insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
    
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
    
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	
	x->x_position = position ;
	
}

void *multiouts_perform7(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7;
	float x1, x2, x3, x4, x5, x6, x7;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[8];
    
	
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
    
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform8(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
    
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform9(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
    
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
    
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	x->x_x9 = x9 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform10(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9, val10;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9, valf10;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9, x10;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
	t_double	*out10= outs[9];
	
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
	val10  = x->x_hp10scp ;
	
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
	valf10  = x->x_hp10scn ;
	
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
	x10 = x->x_x10 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
     		val10 = val10 + x10;
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
			val10 = valf10 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   			x10 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	x->x_hp10scp = val10 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	x->x_x9 = x9 ;
	x->x_x10 = x10 ;
	
	x->x_position = position ;
	
}

void *multiouts_perform11(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9, val10, val11;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9, valf10, valf11;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9, x10, x11;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
	t_double	*out10= outs[9];
	t_double	*out11= outs[10];
    
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
	val10  = x->x_hp10scp ;
	val11  = x->x_hp11scp ;
    
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
	valf10  = x->x_hp10scn ;
	valf11  = x->x_hp11scn ;
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
	x10 = x->x_x10 ;
	x11 = x->x_x11 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
     		val10 = val10 + x10;
     		val11 = val11 + x11;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
			val10 = valf10 ;
			val11 = valf11 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   			x10 = 0. ;
   			x11 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	x->x_hp10scp = val10 ;
	x->x_hp11scp = val11 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	x->x_x9 = x9 ;
	x->x_x10 = x10 ;
	x->x_x11 = x11 ;
	
	x->x_position = position ;
	
    
}

void *multiouts_perform12(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9, val10, val11, val12;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9, valf10, valf11, valf12 ;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9, x10, x11, x12;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
	t_double	*out10= outs[9];
	t_double	*out11= outs[10];
	t_double	*out12= outs[11];
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
	val10  = x->x_hp10scp ;
	val11  = x->x_hp11scp ;
	val12  = x->x_hp12scp ;
	
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
	valf10  = x->x_hp10scn ;
	valf11  = x->x_hp11scn ;
	valf12  = x->x_hp12scn ;
	
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
	x10 = x->x_x10 ;
	x11 = x->x_x11 ;
	x12 = x->x_x12 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
     		val10 = val10 + x10;
     		val11 = val11 + x11;
     		val12 = val12 + x12;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
			val10 = valf10 ;
			val11 = valf11 ;
			val12 = valf12 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   			x10 = 0. ;
   			x11 = 0. ;
   			x12 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	x->x_hp10scp = val10 ;
	x->x_hp11scp = val11 ;
	x->x_hp12scp = val12 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	x->x_x9 = x9 ;
	x->x_x10 = x10 ;
	x->x_x11 = x11 ;
	x->x_x12 = x12 ;
	
	x->x_position = position ;
	
}

void *multiouts_perform13(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9, val10, val11, val12, val13;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9, valf10, valf11, valf12, valf13;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9, x10, x11, x12, x13;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
	t_double	*out10= outs[9];
	t_double	*out11= outs[10];
	t_double	*out12= outs[11];
	t_double	*out13= outs[12];
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
	val10  = x->x_hp10scp ;
	val11  = x->x_hp11scp ;
	val12  = x->x_hp12scp ;
	val13  = x->x_hp13scp ;
	
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
	valf10  = x->x_hp10scn ;
	valf11  = x->x_hp11scn ;
	valf12  = x->x_hp12scn ;
	valf13  = x->x_hp13scn ;
	
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
	x10 = x->x_x10 ;
	x11 = x->x_x11 ;
	x12 = x->x_x12 ;
	x13 = x->x_x13 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
     		val10 = val10 + x10;
     		val11 = val11 + x11;
     		val12 = val12 + x12;
     		val13 = val13 + x13;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
			val10 = valf10 ;
			val11 = valf11 ;
			val12 = valf12 ;
			val13 = valf13 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   			x10 = 0. ;
   			x11 = 0. ;
   			x12 = 0. ;
   			x13 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	x->x_hp10scp = val10 ;
	x->x_hp11scp = val11 ;
	x->x_hp12scp = val12 ;
	x->x_hp13scp = val13 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	x->x_x9 = x9 ;
	x->x_x10 = x10 ;
	x->x_x11 = x11 ;
	x->x_x12 = x12 ;
	x->x_x13 = x13 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform14(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9, val10, val11, val12, val13, val14;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9, valf10, valf11, valf12, valf13, valf14;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9, x10, x11, x12, x13, x14;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
	t_double	*out10= outs[9];
	t_double	*out11= outs[10];
	t_double	*out12= outs[11];
	t_double	*out13= outs[12];
	t_double	*out14= outs[13];
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
	val10  = x->x_hp10scp ;
	val11  = x->x_hp11scp ;
	val12  = x->x_hp12scp ;
	val13  = x->x_hp13scp ;
	val14  = x->x_hp14scp ;
    
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
	valf10  = x->x_hp10scn ;
	valf11  = x->x_hp11scn ;
	valf12  = x->x_hp12scn ;
	valf13  = x->x_hp13scn ;
	valf14  = x->x_hp14scn ;
	
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
	x10 = x->x_x10 ;
	x11 = x->x_x11 ;
	x12 = x->x_x12 ;
	x13 = x->x_x13 ;
	x14 = x->x_x14 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
     		val10 = val10 + x10;
     		val11 = val11 + x11;
     		val12 = val12 + x12;
     		val13 = val13 + x13;
     		val14 = val14 + x14;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
   			*out14++ = insig * val14;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
			val10 = valf10 ;
			val11 = valf11 ;
			val12 = valf12 ;
			val13 = valf13 ;
			val14 = valf14 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
   			*out14++ = insig * val14;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   			x10 = 0. ;
   			x11 = 0. ;
   			x12 = 0. ;
   			x13 = 0. ;
   			x14 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	x->x_hp10scp = val10 ;
	x->x_hp11scp = val11 ;
	x->x_hp12scp = val12 ;
	x->x_hp13scp = val13 ;
	x->x_hp14scp = val14 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	x->x_x9 = x9 ;
	x->x_x10 = x10 ;
	x->x_x11 = x11 ;
	x->x_x12 = x12 ;
	x->x_x13 = x13 ;
	x->x_x14 = x14 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform15(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9, val10, val11, val12, val13, val14, val15;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9, valf10, valf11, valf12, valf13, valf14, valf15;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9, x10, x11, x12, x13, x14, x15;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
	t_double	*out10= outs[9];
	t_double	*out11= outs[10];
	t_double	*out12= outs[11];
	t_double	*out13= outs[12];
	t_double	*out14= outs[13];
	t_double	*out15= outs[14];
    
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
	val10  = x->x_hp10scp ;
	val11  = x->x_hp11scp ;
	val12  = x->x_hp12scp ;
	val13  = x->x_hp13scp ;
	val14  = x->x_hp14scp ;
	val15  = x->x_hp15scp ;
	
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
	valf10  = x->x_hp10scn ;
	valf11  = x->x_hp11scn ;
	valf12  = x->x_hp12scn ;
	valf13  = x->x_hp13scn ;
	valf14  = x->x_hp14scn ;
	valf15  = x->x_hp15scn ;
	
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
	x10 = x->x_x10 ;
	x11 = x->x_x11 ;
	x12 = x->x_x12 ;
	x13 = x->x_x13 ;
	x14 = x->x_x14 ;
	x15 = x->x_x15 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
     		val10 = val10 + x10;
     		val11 = val11 + x11;
     		val12 = val12 + x12;
     		val13 = val13 + x13;
     		val14 = val14 + x14;
     		val15 = val15 + x15;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
   			*out14++ = insig * val14;
   			*out15++ = insig * val15;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
			val10 = valf10 ;
			val11 = valf11 ;
			val12 = valf12 ;
			val13 = valf13 ;
			val14 = valf14 ;
			val15 = valf15 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
   			*out14++ = insig * val14;
   			*out15++ = insig * val15;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   			x10 = 0. ;
   			x11 = 0. ;
   			x12 = 0. ;
   			x13 = 0. ;
   			x14 = 0. ;
   			x15 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	x->x_hp10scp = val10 ;
	x->x_hp11scp = val11 ;
	x->x_hp12scp = val12 ;
	x->x_hp13scp = val13 ;
	x->x_hp14scp = val14 ;
	x->x_hp15scp = val15 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ;
	x->x_x9 = x9 ;
	x->x_x10 = x10 ;
	x->x_x11 = x11 ;
	x->x_x12 = x12 ;
	x->x_x13 = x13 ;
	x->x_x14 = x14 ;
	x->x_x15 = x15 ;
	
	x->x_position = position ;
    
}

void *multiouts_perform16(t_multiouts *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	float val1, val2, val3, val4, val5, val6, val7, val8;
	float val9, val10, val11, val12, val13, val14, val15, val16;
	float valf1, valf2, valf3, valf4, valf5, valf6, valf7, valf8;
	float valf9, valf10, valf11, valf12, valf13, valf14, valf15, valf16;
	float x1, x2, x3, x4, x5, x6, x7, x8;
	float x9, x10, x11, x12, x13, x14, x15, x16;
	double  insig ;
	long position ;
    
    
  	t_double	*in = ins[0];           // Input 1
	t_double	*out1= outs[0];
	t_double	*out2= outs[1];
	t_double	*out3= outs[2];
	t_double	*out4= outs[3];
	t_double	*out5= outs[4];
	t_double	*out6= outs[5];
	t_double	*out7= outs[6];
	t_double	*out8= outs[7];
	t_double	*out9= outs[8];
	t_double	*out10= outs[9];
	t_double	*out11= outs[10];
	t_double	*out12= outs[11];
	t_double	*out13= outs[12];
	t_double	*out14= outs[13];
	t_double	*out15= outs[14];
	t_double	*out16= outs[15];
    
	position = x->x_position ;
    
	val1  = x->x_hp1scp ;
	val2  = x->x_hp2scp ;
	val3  = x->x_hp3scp ;
	val4  = x->x_hp4scp ;
	val5  = x->x_hp5scp ;
	val6  = x->x_hp6scp ;
	val7  = x->x_hp7scp ;
	val8  = x->x_hp8scp ;
	val9  = x->x_hp9scp ;
	val10  = x->x_hp10scp ;
	val11  = x->x_hp11scp ;
	val12  = x->x_hp12scp ;
	val13  = x->x_hp13scp ;
	val14  = x->x_hp14scp ;
	val15  = x->x_hp15scp ;
	val16  = x->x_hp16scp ;
	
	valf1  = x->x_hp1scn ;
	valf2  = x->x_hp2scn ;
	valf3  = x->x_hp3scn ;
	valf4  = x->x_hp4scn ;
	valf5  = x->x_hp5scn ;
	valf6  = x->x_hp6scn ;
	valf7  = x->x_hp7scn ;
	valf8  = x->x_hp8scn ;
	valf9  = x->x_hp9scn ;
	valf10  = x->x_hp10scn ;
	valf11  = x->x_hp11scn ;
	valf12  = x->x_hp12scn ;
	valf13  = x->x_hp13scn ;
	valf14  = x->x_hp14scn ;
	valf15  = x->x_hp15scn ;
	valf16  = x->x_hp16scn ;
    
	x1 = x->x_x1 ;
	x2 = x->x_x2 ;
	x3 = x->x_x3 ;
	x4 = x->x_x4 ;
	x5 = x->x_x5 ;
	x6 = x->x_x6 ;
	x7 = x->x_x7 ;
	x8 = x->x_x8 ;
	x9 = x->x_x9 ;
	x10 = x->x_x10 ;
	x11 = x->x_x11 ;
	x12 = x->x_x12 ;
	x13 = x->x_x13 ;
	x14 = x->x_x14 ;
	x15 = x->x_x15 ;
	x16 = x->x_x16 ;
    
    while (sampleframes--)
    {
    	if(position--)
    	{
     		insig = *in++ ;
     		val1 = val1 + x1;
     		val2 = val2 + x2;
     		val3 = val3 + x3;
     		val4 = val4 + x4;
     		val5 = val5 + x5;
     		val6 = val6 + x6;
     		val7 = val7 + x7;
     		val8 = val8 + x8;
     		val9 = val9 + x9;
     		val10 = val10 + x10;
     		val11 = val11 + x11;
     		val12 = val12 + x12;
     		val13 = val13 + x13;
     		val14 = val14 + x14;
     		val15 = val15 + x15;
     		val16 = val16 + x16;
            
       		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
   			*out14++ = insig * val14;
   			*out15++ = insig * val15;
   			*out16++ = insig * val16;
   		}
   		else  			// si position arrive à zéro
   		{
   		 	insig = *in++ ;
			val1 = valf1 ;
			val2 = valf2 ;
			val3 = valf3 ;
			val4 = valf4 ;
			val5 = valf5 ;
			val6 = valf6 ;
			val7 = valf7 ;
			val8 = valf8 ;
			val9 = valf9 ;
			val10 = valf10 ;
			val11 = valf11 ;
			val12 = valf12 ;
			val13 = valf13 ;
			val14 = valf14 ;
			val15 = valf15 ;
			val16 = valf16 ;
     		*out1++ = insig * val1;
   			*out2++ = insig * val2;
   			*out3++ = insig * val3;
   			*out4++ = insig * val4;
   			*out5++ = insig * val5;
   			*out6++ = insig * val6;
   			*out7++ = insig * val7;
   			*out8++ = insig * val8;
       		*out9++ = insig * val9;
   			*out10++ = insig * val10;
   			*out11++ = insig * val11;
   			*out12++ = insig * val12;
   			*out13++ = insig * val13;
   			*out14++ = insig * val14;
   			*out15++ = insig * val15;
   			*out16++ = insig * val16;
  			x1 = 0. ;
   			x2 = 0. ;
   			x3 = 0. ;
   			x4 = 0. ;
   			x5 = 0. ;
   			x6 = 0. ;
   			x7 = 0. ;
   			x8 = 0. ;
   			x9 = 0. ;
   			x10 = 0. ;
   			x11 = 0. ;
   			x12 = 0. ;
   			x13 = 0. ;
   			x14 = 0. ;
   			x15 = 0. ;
   			x16 = 0. ;
   		}
	}
	
	x->x_hp1scp = val1 ;
	x->x_hp2scp = val2 ;
	x->x_hp3scp = val3 ;
	x->x_hp4scp = val4 ;
	x->x_hp5scp = val5 ;
	x->x_hp6scp = val6 ;
	x->x_hp7scp = val7 ;
	x->x_hp8scp = val8 ;
	x->x_hp9scp = val9 ;
	x->x_hp10scp = val10 ;
	x->x_hp11scp = val11 ;
	x->x_hp12scp = val12 ;
	x->x_hp13scp = val13 ;
	x->x_hp14scp = val14 ;
	x->x_hp15scp = val15 ;
	x->x_hp16scp = val16 ;
	
	x->x_x1 = x1 ;
	x->x_x2 = x2 ;
	x->x_x3 = x3 ;
	x->x_x4 = x4 ;
	x->x_x5 = x5 ;
	x->x_x6 = x6 ;
	x->x_x7 = x7 ;
	x->x_x8 = x8 ; 
	x->x_x9 = x9 ;
	x->x_x10 = x10 ;
	x->x_x11 = x11 ;
	x->x_x12 = x12 ;
	x->x_x13 = x13 ;
	x->x_x14 = x14 ;
	x->x_x15 = x15 ;
	x->x_x16 = x16 ; 
	
	x->x_position = position ;
    
}



/**********************************************************/
/* 0 = entrée gauche, 1 = control de pan, ensuite sorties */
void multiouts_dsp64(t_multiouts *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	int n = x->x_nouts ;
	// post("x->x_nouts = %d" , n) ;
	switch (n)
	{
		case 2 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform2, 0, NULL);			break ;
		case 3 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform3, 0, NULL);			break ;
		case 4 :
    		object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform4, 0, NULL);			break ;
		case 5 :
    		object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform5, 0, NULL);			break ;
		case 6 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform6, 0, NULL);			break ;
		case 7 :
    		object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform7, 0, NULL);		break ;
		case 8 :
    		object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform8, 0, NULL);			break ;
		case 9 :
    		object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform9, 0, NULL);			break ;
		case 10 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform10, 0, NULL);		break ;
		case 11 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform11, 0, NULL);		break ;
		case 12 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform12, 0, NULL);		break ;
		case 13 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform13, 0, NULL);			break ;
		case 14 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform14, 0, NULL);		break ;
		case 15 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform15, 0, NULL);		break ;
		case 16 :
	    	object_method(dsp64, gensym("dsp_add64"), x, multiouts_perform16, 0, NULL);		break ;
            
	}
}

