/************************************************************************
 *
 *			                >>>>>>> BUFGRANUL2~ <<<<<<<
 *
 *						  multi-buffer enveloppe externe,
 *                          multi-buffer son externe.
 *              continuite des grains lors d'un changement de buffer.
 *                         controle en float et/ou en audio
 * 					selection de buffer son par entree signal
 *                           ----------------------
 *                              GMEM 2002-2004
 *         Laurent Pottier / Loic Kessous / Charles Bascou / Leopold Frey
 *
 * -----------------------------------------------------------------------
 *
 *
 * N.B.
 *      * Pour compatibilite maximum Mac/PC :
 *        pas d'accents dans les commentaires svp
 *
 *      * Faites des commentaires !
 *
 ************************************************************************/

#include "bufGranul2~.h"
#include "spat.h"

void*     bufGranul_class;
t_symbol* ps_buffer;

static int x_sinc_table_offset[16] = {15360, 14336, 13312, 12288, 11264, 10240, 9216, 8192,
                                      7168,  6144,  5120,  4096,  3072,  2048,  1024, 0};

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

void ext_main(void* r)
{
    t_class* c;

    c = class_new("bufgranul~", (method)bufGranul_new, (method)bufGranul_free, sizeof(t_bufGranul),
                  (method)NULL, A_GIMME, 0L);

    class_dspinit(c);

    class_addmethod(c, (method)bufGranul_set, "set", A_GIMME, 0);  // definition du buffer~ son
    class_addmethod(c, (method)bufGranul_setenv, "setenv", A_GIMME,
                    0);  // definition du buffer~ enveloppe
    class_addmethod(c, (method)bufGranul_envbuffer, "envbuffer", A_LONG, 0);  // # env buffer actif

    class_addmethod(c, (method)bufGranul_loop, "loop", A_GIMME, 0);  // infos du mode loop
    class_addmethod(c, (method)bufGranul_nvoices, "nvoices", A_LONG,
                    0);  // nombre de voix (polyphonie)
    class_addmethod(c, (method)bufGranul_bchan_offset, "bchan_offset", A_LONG,
                    0);                                         // nombre de voix (polyphonie)
    class_addmethod(c, (method)bufGranul_tellme, "tellme", 0);  // demande d'info
    class_addmethod(c, (method)bufGranul_bang, "bang",
                    0);  // routine pour un bang, declenchement d'un grain
    class_addmethod(c, (method)bufGranul_float, "float",
                    0);  // affectation des valeurs par des flottants

    class_addmethod(c, (method)bufGranul_assist, "assist", A_CANT, 0);  // assistance in out
    class_addmethod(c, (method)bufGranul_dsp64, "dsp64", A_CANT, 0);    // signal processing
    class_addmethod(c, (method)bufGranul_sinterp, "sinterp", A_LONG,
                    0);  // interpollation dans la lecture du buffer pour �viter les clics
    class_addmethod(c, (method)bufGranul_microtiming, "microtiming", A_LONG,
                    0);  // infos du mode loop
    class_addmethod(c, (method)bufGranul_clear, "clear",
                    0);  // panique ! effacement des grains en cours
    class_addmethod(c, (method)bufGranul_clear, "panic",
                    0);  // panique ! effacement des grains en cours

    class_addmethod(c, (method)bufGranul_killall, "killall",
                    0);  // small vecsize fadeout and then panic
    class_addmethod(c, (method)bufGranul_kill, "kill", A_LONG, 0);

    class_addmethod(c, (method)bufGranul_polymode, "polymode", A_LONG, 0);

    class_addmethod(c, (method)bufGranul_grain, "grain", A_GIMME, 0);

#ifdef PERF_DEBUG
    class_addmethod(c, (method)bufGranul_poll, "poll", A_LONG, 0);
    class_addmethod(c, (method)bufGranul_info, "info", A_LONG, 0);
#endif

    ps_buffer = gensym("buffer~");

    class_register(CLASS_BOX, c);

    bufGranul_class = c;

    post("2017 BufGranul~ 64bit Charles Bascou GMEM Marseille F");
    post("build %s %s", __DATE__, __TIME__);
}

// Reception d'un bang, declenche un grain
void bufGranul_bang(t_bufGranul* x)
{
    x->x_askfor = 1;
}

int bufGranul_poly_assign_voice(t_bufGranul* x)
{
    int    freevoice = -1, curpoly = 0;
    int    i;
    double actual_ind = 0., remain_ind = 0.;
    int    zombi_actual, zombi_remain, zombi_robin;

    // check actual polyphony and check age and remain of each voices
    for (i = 0; i < NVOICES; i++) {
        if (x->x_voiceOn[i] == 1)  // active
        {
            curpoly++;
            zombi_robin = i;
            if (actual_ind < x->x_ind[i]) {
                actual_ind   = x->x_ind[i];
                zombi_actual = i;
            }

            if (remain_ind < x->x_remain_ind[i]) {
                remain_ind   = x->x_remain_ind[i];
                zombi_remain = i;
            }

        } else if (x->x_voiceOn[i] == 2)  // waiting
        {
            // don't touch waiting voice
        } else if (x->x_voiceOn[i] == 0) {  // not in killing mode
            freevoice = i;
        }
    }

    x->x_nvoices_active = curpoly;

    if (freevoice == -1) {  // no hardvoice available
        return 0;
    }

    if (curpoly < x->x_nvoices) {  // ok poly is not exceeded
        return freevoice;
    }

    // post("zombi_robin %d zombi_actual %d zombi_remain %d",zombi_robin,zombi_actual,zombi_remain);

    // 0 : wait 1 : round_robin 2 : max_time_lasted 3 : max_time_remaining
    switch (x->x_poly_mode) {
        case 0:
            return 0;

        case 1:
            bufGranul_kill(x, zombi_robin);
            return freevoice;

        case 2:  // kill grain which is the oldest
            bufGranul_kill(x, zombi_actual);
            return freevoice;

        case 3:  // kill grain which has the longest tail
            bufGranul_kill(x, zombi_remain);
            return freevoice;
    }
}

void bufGranul_poly_check_and_kill(t_bufGranul* x)
{
    int    curpoly = 0;
    int    i;
    double actual_ind = 0., remain_ind = 0.;
    int    zombi_actual, zombi_remain, zombi_robin;

    // check actual polyphony and check age and remain of each voices
    for (i = 0; i < NVOICES; i++) {
        if (x->x_voiceOn[i] == 1)  // active
        {
            curpoly++;
            zombi_robin = i;
            if (actual_ind < x->x_ind[i]) {
                actual_ind   = x->x_ind[i];
                zombi_actual = i;
            }

            if (remain_ind < x->x_remain_ind[i]) {
                actual_ind   = x->x_remain_ind[i];
                zombi_remain = i;
            }

        } else if (x->x_voiceOn[i] == 2)  // waiting
        {
            // don't touch waiting voice
        }
    }

    if (curpoly >= x->x_nvoices)  // poly is full so we have to kill one voice
    {
        // 0 : wait 1 : round_robin 2 : max_time_lasted 3 : max_time_remaining
        switch (x->x_poly_mode) {
            case 0:  // special for wait mode kill oldest
                bufGranul_kill(x, zombi_robin);
                return;

            case 1:
                bufGranul_kill(x, zombi_robin);
                return;

            case 2:  // kill grain which is the oldest
                bufGranul_kill(x, zombi_actual);
                return;

            case 3:  // kill grain which has the longest tail
                bufGranul_kill(x, zombi_remain);
                return;
        }
    }
}

// declenchement grain par liste
// delay(ms) begin detune amp length pan dist buffer envbuffer
void bufGranul_grain(t_bufGranul* x, t_symbol* s, short ac, t_atom* av)
{
    int j, p;
    int nvoices = x->x_nvoices;
    int xn      = 0;

    if (ac < 9) {
        post(
            "bufgranul~ : grain args are <delay(ms)> <begin> <detune> <amp> <length> <pan> <dist> "
            "<buffer> <envbuffer>");
        return;
    }

    double delay  = atom2float(av, 0);
    double begin  = atom2float(av, 1);
    double detune = atom2float(av, 2);
    double amp    = atom2float(av, 3);
    double length = atom2float(av, 4);
    double pan    = MOD(atom2float(av, 5), 1.);
    double dist   = atom2float(av, 6);

    int buffer    = (int)atom2float(av, 7);
    int envbuffer = (int)atom2float(av, 8);

    double srms = x->x_sr * 0.001;  // double for precision

    begin = begin * srms;
    delay = delay * srms;
    p     = nvoices;

    p = bufGranul_poly_assign_voice(x);  // get free voice according to polymode
    // post("p : %d",p);

    if (p) {
        x->x_voiceOn[p] = (delay > 0.) ? 2 : 1;               // wait mode or active mode
        x->x_sind[p] = x->Vbeg[p] = begin;                    // index dans buffer
        x->Vtranspos[p]           = detune;                   // valeur de pitch
        x->Vamp[p]                = amp;                      // amplitude
        x->Vbuf[p]                = buffer_check(x, buffer);  // numero du buffer son
        x->Venv[p] = bufferenv_check(x, envbuffer);           // enveloppe active pour ce grain

        if (length < 0) {
            x->Vlength[p] = -length * srms;
            x->envinc[p]  = -1. * (float)(x->x_env_frames[x->Venv[p]] - 1.) / x->Vlength[p];
            x->envind[p]  = x->x_env_frames[x->Venv[p]] - 1;
        } else if (length == 0.)  // whole buffer
        {
            x->Vlength[p] = x->x_buf_frames[x->Vbuf[p]];
            x->envinc[p]  = (float)(x->x_env_frames[x->Venv[p]] - 1.) / x->Vlength[p];
            x->envind[p]  = 0.;
        } else {
            x->Vlength[p] = length * srms;
            x->envinc[p]  = (float)(x->x_env_frames[x->Venv[p]] - 1.) / x->Vlength[p];
            x->envind[p]  = 0.;
        }

        x->Vpan[p]  = MAX(pan, 0);        // pan
        x->Vdist[p] = SAT(dist, 0., 1.);  // distance
        panner(x->Vhp[p], x->x_nouts, x->Vpan[p], x->Vdist[p]);

        x->x_ind[p]        = 0;
        x->x_remain_ind[p] = (long)x->Vlength[p];
        x->x_delay[p]      = delay;  // delay de declenchement dans le vecteur

        x->Vloop[p]      = x->x_loop;
        x->Vloopstart[p] = x->x_loopstart;
        x->Vloopend[p]   = x->x_loopend;
    }
}

// effacement de tous les grains en cours (panique)
void bufGranul_clear(t_bufGranul* x)
{
    int i;
    for (i = 0; i < NVOICES; i++) {
        x->x_voiceOn[i] = 0;
    }
}

// mark as kill all grains
void bufGranul_killall(t_bufGranul* x)
{
    int i;
    for (i = 0; i < NVOICES; i++) {
        x->x_voiceOn[i] = (x->x_voiceOn[i] > 0) ? 3 : 0;
    }
}

// kill specific grain
void bufGranul_kill(t_bufGranul* x, long k)
{
    if (k < NVOICES) {
        if (x->x_voiceOn[k] > 0) {
            x->x_voiceOn[k] = 3;
        }
    }
}

// adress specific voice
void bufGranul_setvoice(t_bufGranul* x, long k)
{
    if (k < NVOICES) {
        if (x->x_voiceOn[k]) {
            x->x_voiceOn[k] = 3;
        }
    }
}

void bufGranul_polymode(t_bufGranul* x, long k)
{
    k              = SAT(k, 0, 3);
    x->x_poly_mode = k;
}

// voir en sortie le nombre de voix, le buffer son actif etc...
#ifdef PERF_DEBUG
void bufGranul_poll(t_bufGranul* x, long n)
{
    x->ask_poll = (n > 0 ? 1 : 0);
}

#endif

float atom2float(t_atom* av, int ind)
{
    switch (av[ind].a_type) {
        case A_FLOAT:
            return av[ind].a_w.w_float;
            break;
        case A_LONG:
            return av[ind].a_w.w_long;
            break;
        default:
            return 0.;
    }
}

t_symbol* atom2symbol(t_atom* av, int ind)
{
    switch (av[ind].a_type) {
        case A_SYM:
            return av[ind].a_w.w_sym;
            break;
        default:
            return 0;
    }
}

// Fonction de verification de la validit� du buffer demand�
int buffer_check(t_bufGranul* x, int num)
{
    int index;
    index = (num < 0 || num >= NSBUF) ? 0 : num;
    return index;
}

// Fonction de verification de la validit� du buffer demand�
int bufferenv_check(t_bufGranul* x, int num)
{
    return (x->x_env_buf[num] == 0) ? 0 : num;
}

// Reception des valeurs sur les entrees non-signal
void bufGranul_float(t_bufGranul* x, double f)
{
    if (x->x_obj.z_in == 0) {
        post("a float in first inlet don't do anything");
    } else if (x->x_obj.z_in == 1) {
        x->x_begin = (f > 0.) ? f : 0.;
    } else if (x->x_obj.z_in == 2) {
        x->x_transpos = f;
    } else if (x->x_obj.z_in == 3) {
        x->x_amp = (f > 0.) ? f : 0.;
    } else if (x->x_obj.z_in == 4) {
        if (f < 0) {
            x->x_length  = (-f > MIN_LENGTH) ? -f : MIN_LENGTH;
            x->x_env_dir = -1;
        } else {
            x->x_length  = (f > MIN_LENGTH) ? f : MIN_LENGTH;
            x->x_env_dir = 1;
        }
    } else if (x->x_obj.z_in == 5) {
        x->x_pan = MOD(f, 1.);
    } else if (x->x_obj.z_in == 6) {
        x->x_dist = f;
    } else if (x->x_obj.z_in == 7) {
        x->x_active_buf = buffer_check(x, (int)f);
    }
}

// limitation de la polyphonie
void bufGranul_nvoices(t_bufGranul* x, long n)
{
    n            = (n > 0) ? n : 1;
    x->x_nvoices = (n < NMAXPOLY) ? n : NMAXPOLY;
}

void bufGranul_bchan_offset(t_bufGranul* x, long n)
{
    n                 = (n > 0) ? n : 0;
    x->x_bchan_offset = n;
}

// interpollation dans la lecture du buffer (evite clic, crack et autre pop plop)
void bufGranul_sinterp(t_bufGranul* x, long n)
{
    x->x_sinterp = SAT(n, 0, 2);
}

void bufGranul_microtiming(t_bufGranul* x, long flag)
{
    x->x_microtiming = SAT(flag, 0, 1);
}

// mode loop begin end ....
void bufGranul_loop(t_bufGranul* x, t_symbol* s, short ac, t_atom* av)
{
    int j;

    if (ac < 1) {
        post("bufgranul~ : loop bad args... loop <mode> [<begin> <end>]");
        return;
    }
    if (ac == 1 && av[0].a_type == A_LONG) {
        x->x_loop = (av[0].a_w.w_long != 0);
        return;
    }
    if (ac == 3 && av[0].a_type == A_LONG) {
        if (av[0].a_w.w_long != 0) {
            // loopstart
            switch (av[1].a_type) {
                case A_LONG:
                    x->x_loopstart = MAX(av[1].a_w.w_long, 0);
                    break;

                case A_FLOAT:
                    x->x_loopstart = MAX(av[1].a_w.w_float, 0);
                    break;

                default:
                    post("bugranul~ : loop bad arg");
                    x->x_loop = 0;
                    return;
            }
            // loopend
            switch (av[2].a_type) {
                case A_LONG:
                    x->x_loopend = MAX(av[2].a_w.w_long, x->x_loopstart + MIN_LOOP_LENGTH);
                    break;

                case A_FLOAT:
                    x->x_loopend = MAX(av[2].a_w.w_float, x->x_loopstart + MIN_LOOP_LENGTH);
                    break;

                default:
                    post("bugranul~ : loop bad arg");
                    x->x_loop = 0;
                    return;
            }

            x->x_loop = 2;
            // post("start %f end %f",x->x_loopstart,x->x_loopend);
        } else {
            x->x_loop = 0;
        }
    } else {
        x->x_loop = 0;
        post("bufgranul~ : loop bad args... loop <mode> [<begin> <end>]");
    }
}

// Impression de l'etat de l'objet
void bufGranul_tellme(t_bufGranul* x)
{
    int i;
    post("  ");
    post("_______BufGranul~'s State_______");

    post("::global settings::");

    post("outputs : %ld", x->x_nouts);
    post("voices : %ld", x->x_nvoices);
    post("________________________________");

    post("::sound buffers::");

    post("-- Active sound buffer : %ld --", x->x_active_buf + 1);
    post("-- Number of sound buffers : %ld --", x->x_nbuffer);
    for (i = 0; i < x->x_nbuffer; i++) {
        if (x->x_buf_sym[i] && x->x_buf_sym[i]->s_name) {
            post("   sound buffer~ %ld name : %s", i + 1, x->x_buf_sym[i]->s_name);
        }
        if (x->x_buf_filename[i] && x->x_buf_filename[i]->s_name) {
            post("   sound buffer~ %ld filename : %s", i + 1, x->x_buf_filename[i]->s_name);
        }
    }
    post("________________________________");

    post("::envelope buffers::");

    post("-- Active envelope buffer : %ld --", x->x_active_env + 1);
    post("-- Number of envelope buffers : %ld --", x->x_nenvbuffer);
    for (i = 0; i < x->x_nenvbuffer; i++) {
        if (x->x_env_sym[i] && x->x_env_sym[i]->s_name) {
            post("   envelope buffer~ %ld name : %s", i + 1, x->x_env_sym[i]->s_name);
        }
    }
    post("________________________________");

    bufGranul_info(x, -1);
}

// Informations en sortie de l'objet
#ifdef PERF_DEBUG
void bufGranul_info(t_bufGranul* x, long n)
{
    // Si n=	0 infos sur buffer son
    //			1 infos sur buffer enveloppe
    //		   -1 infos sur buffers...
    int i;

    if (x->ask_poll) {
        if (n == 0 || n == -1) {
            for (i = 0; i < x->x_nbuffer; i++) {
                _SETSYM(x->info_list, gensym("buffer"));
                // if(x->x_buf[i])
                //{
                _SETLONG(x->info_list + 1, i);

                if (x->x_buf_sym[i] && x->x_buf_sym[i]->s_name) {
                    _SETSYM(x->info_list + 2, x->x_buf_sym[i]);
                } else {
                    _SETSYM(x->info_list + 2, gensym("unknown name"));
                }

                if (x->x_buf_filename[i] && x->x_buf_filename[i]->s_name) {
                    _SETSYM(x->info_list + 3, x->x_buf_filename[i]);
                } else {
                    _SETSYM(x->info_list + 3, gensym("unknown filename"));
                }

                if (x->x_buf_frames[i]) {
                    _SETFLOAT(x->info_list + 4, x->x_buf_frames[i] / 44.1);
                } else {
                    _SETFLOAT(x->info_list + 4, 0.);
                }

                outlet_list(x->info, 0l, 5, x->info_list);
            }
            //}
        }

        if (n == 1 || n == -1) {
            _SETSYM(x->info_list, gensym("active_env"));
            _SETFLOAT(x->info_list + 1, x->x_active_env + 1);
            outlet_list(x->info, 0l, 2, x->info_list);

            for (i = 0; i < x->x_nenvbuffer; i++) {
                if (x->x_env_sym[i]) {
                    _SETSYM(x->info_list, gensym("envbuffer"));
                    _SETLONG(x->info_list + 1, i);
                    _SETSYM(x->info_list + 2, x->x_env_sym[i]);
                    outlet_list(x->info, 0l, 3, x->info_list);
                }
            }
        }
    }
}

#endif

// NNNNNNN routine de creation de l'objet NNNNNNNNNNNNNNNNNNNNNNNNNNNN//

void* bufGranul_new(t_symbol* s, short ac, t_atom* av)
{
    t_buffer*    b1;
    int          i, j, symcount = 0, longcount = 0;
    double       f, w;
    t_bufGranul* x = (t_bufGranul*)object_alloc((t_class*)bufGranul_class);
    // dsp_setup((t_pxobject *)x,1);

    // creation des entres supplementaires---//

    dsp_setup((t_pxobject*)x, 8);

    x->x_nouts        = 2;  // pour que nb outs = 2 si pas specifie
    x->x_bchan_offset = 0;  // buffer channel offset if unspecified

    x->x_poly_mode = 0;

    if (ac < 2) {
        post("syntax error : bufgranul~ <snd_buf> <env_buf> <out_channels> <buffer_chans_offset");
        return 0;
    } else {
        //////////////arguments/////////////////
        // note : 1er symbol buffer son , 2eme symbol buffer env, 3eme nombres de sorties, buffer
        // channel offset

        for (i = 0; i < NSBUF; i++) {
            x->x_buf[i]          = 0;
            x->x_buf_filename[i] = gensym("empty_buffer");
            x->x_buf_sym[i]      = gensym("empty_buffer");
            x->x_buf_nchan[i]    = -1;
            x->x_buf_samples[i]  = 0;
            x->x_buf_frames[i]   = -1;
        }

        for (i = 0; i < NEBUF; i++) {
            x->x_env_buf[i]     = 0;
            x->x_env_sym[i]     = gensym("empty_buffer");
            x->x_env_samples[i] = 0;
            x->x_env_frames[i]  = -1;
        }

        for (j = 0; j < ac; j++) {
            switch (av[j].a_type) {
                case A_LONG:
                    if (longcount) {
                        x->x_bchan_offset = av[j].a_w.w_long;
                    } else {
                        x->x_nouts = av[j].a_w.w_long;
                    }

                    longcount++;
                    break;

                case A_SYM:
                    if (symcount) {
                        //	post("argument %ld is a symbol : name %s, assigned to env buffer~
                        // name",(long)j,av[j].a_w.w_sym->s_name);

                        x->x_env_sym[0] = av[j].a_w.w_sym;

                    } else {
                        //	post("argument %ld is a symbol : name %s, assigned to buffer~
                        // name",(long)j,av[j].a_w.w_sym->s_name);

                        x->x_buf_sym[0] = av[j].a_w.w_sym;
                        symcount++;
                    }
                    break;

                case A_FLOAT:
                    post("argument %ld is a float : %lf, not assigned", (long)j, av[j].a_w.w_float);
                    break;
            }
        }

///////////////////////////////
// info outlet
#ifdef PERF_DEBUG
        x->info = outlet_new((t_pxobject*)x, 0L);
#endif

        // limit n outs to 16
        x->x_nouts = SAT(x->x_nouts, 1, 16);

        // creation des sortie signal
        for (int hp = 0; hp < x->x_nouts; hp++) {
            outlet_new((t_pxobject*)x, "signal");
        }

        // allocations des tableaux
        if (!bufGranul_alloc(x)) {
            post("error bufGranul~ : not enough memory to instanciate");
        }

        // initialisation des autres parametres
        x->x_nvoices  = 128;
        x->x_askfor   = 0;
        x->x_begin    = 0;
        x->x_transpos = 1.0;
        x->x_amp      = 1.0;
        x->x_length   = 100;

        x->x_sinterp     = 1;
        x->x_loop        = 0;
        x->x_microtiming = 0;

        x->x_pan            = 1 / (x->x_nouts * 2);
        x->x_dist           = 1;
        x->x_nvoices_active = 0;
        x->x_env_dir        = 1;

        for (i = 0; i < NVOICES; i++) {
            x->x_ind[i]     = -1;
            x->x_env[i]     = 0;
            x->x_voiceOn[i] = 0;
            x->Vbuf[i]      = 0;
        }

        // initialisation des parametres d'activite (modifie par la suite dans setenv et set)
        x->x_nbuffer    = 1;
        x->x_active_buf = 0;
        x->x_nenvbuffer = 1;
        x->x_active_env = 0;

        for (i = 1; i < NSBUF; i++) {
            x->x_buf[i]             = 0;
            x->x_buf_sym[i]         = 0;
            x->x_env_buf[i]         = 0;
            x->x_env_sym[i]         = 0;
            x->x_buf_valid_index[i] = 0;
        }

        // generation de la table de coeff d'interpolation
        x->x_linear_interp_table =
            (t_linear_interp*)sysmem_newptr(TABLE_SIZE * sizeof(struct linear_interp));

        if (!x->x_linear_interp_table) {
            post("bufGranul~ : not enough memory to instanciate");
            return (0);
        }
        for (i = 0; i < TABLE_SIZE; i++) {
            f = (double)(i) / TABLE_SIZE;  // va de 0 a 1

            x->x_linear_interp_table[i].a = 1 - f;
            x->x_linear_interp_table[i].b = f;
        }

        // fill sinc table + blackman ( 8 zero crossings for 17 points interpol )

        // generation de la table de coeff d'interpolation
        x->x_sinc_interp_table = (double*)sysmem_newptr(SINC_TABLE_SIZE * sizeof(double));
        x->x_blackman_table    = (double*)sysmem_newptr(SINC_TABLE_SIZE * sizeof(double));
        x->x_sinc_norm_table   = (double*)sysmem_newptr(SINC_TABLE_SIZE * sizeof(double));

        int    sinc_table_size_2 = SINC_TABLE_SIZE / 2.;
        double sinc_sum          = 0.;
        for (i = 0; i < SINC_TABLE_SIZE; i++) {
            f = (double)(i - sinc_table_size_2) / (double)sinc_table_size_2;  // va de -1 a 1
            f = M_PI * f * 8;                                                 // for 8 zerocorssings

            // BLACKMAN
            w = .42 - .5 * cos((double)(2 * M_PI * i) / SINC_TABLE_SIZE) +
                .08 * cos((double)(4 * M_PI * i) / SINC_TABLE_SIZE);

            x->x_blackman_table[i] = w;

            x->x_sinc_interp_table[i] = ((f != 0.) ? (sin(f) / f) : 1.);
            sinc_sum += x->x_sinc_interp_table[i];

            // post("black %lf\n",w);//x->x_blackman_table[i]);
        }

        // printf("sinc sum %lf\n",sinc_sum);

        int    m;
        double val;
        double highpass_fact;

        for (i = 0; i < SINC_TABLE_SIZE; i++) {
            highpass_fact = i;
            highpass_fact /= (double)SINC_TABLE_SIZE;
            val = 0.;
            //
            for (m = -7; m < 9; m++) {
                val += x->x_sinc_interp_table[(
                    long)(8192 + highpass_fact * (8192 - (x_sinc_table_offset[m + 7])))];
            }
            x->x_sinc_norm_table[i] = (double)1. / val;
            // printf("i %d hpfact %f norm %f\n",i,highpass_fact,x->x_sinc_norm_table[i]);
        }
        x->x_fadein       = (double*)sysmem_newptr(MAX_VECTORSIZE * sizeof(double));
        x->x_kill_fadeout = (double*)sysmem_newptr(MAX_VECTORSIZE * sizeof(double));
        x->x_unity_gain   = (double*)sysmem_newptr(MAX_VECTORSIZE * sizeof(double));

        for (i = 0; i < MAX_VECTORSIZE; i++) {
            x->x_unity_gain[i] = 1.;
        }

        return (x);
    }
}
// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN//

//--------assitance information inlet et out1et----------------------//
void bufGranul_assist(t_bufGranul* x, void* b, long m, long a, char* s)
{
    switch (m) {
        case 1:  // inlet
            switch (a) {
                case 0:
                    sprintf(s, " bang,set, signal zero X-ing, et cetera)");
                    break;
                case 1:
                    sprintf(s, " begin (float & signal) ");
                    break;
                case 2:
                    sprintf(s, " transpos (float & signal) ");
                    break;
                case 3:
                    sprintf(s, " amp (float & signal) ");
                    break;
                case 4:
                    sprintf(s, " length (float & signal) ");
                    break;
                case 5:
                    sprintf(s, " pan (float & signal) ");
                    break;
                case 6:
                    sprintf(s, " dist (float & signal) ");
                    break;
                case 7:
                    sprintf(s, " sound buffer num (float & signal) ");
                    break;
            }
            break;
        case 2:  // outlet
            switch (a) {
                case 0:
                    sprintf(s, "(signal) Output 1");
                    break;
                case 1:
                    if (x->x_nouts > 1) {
                        sprintf(s, "(signal) Output 2");
                    } else {
                        sprintf(s, "info out (voices...)");
                    }
                    break;
                case 2:
                    if (x->x_nouts > 2) {
                        sprintf(s, "(signal) Output 3");
                    } else {
                        sprintf(s, "info out (voices...)");
                    }
                    break;
                case 3:
                    sprintf(s, "(signal) Output 4");
                    break;
                case 4:
                    if (x->x_nouts > 4) {
                        sprintf(s, "(signal) Output 5");
                    } else {
                        sprintf(s, "info out (voices...)");
                    }
                    break;
                case 5:
                    sprintf(s, "(signal) Output 6");
                    break;
                case 6:
                    if (x->x_nouts > 6) {
                        sprintf(s, "(signal) Output 7");
                    } else {
                        sprintf(s, "info out (voices...)");
                    }
                    break;
                case 7:
                    sprintf(s, "(signal) Output 8");
                    break;
                case 8:
                    sprintf(s, "info out (voices...)");
                    break;
            }
            break;
    }
}

//-----------------------------------------------------------------------------------//

// BUFFERS SON
// fonction d'assignation du buffer son #n
void bufGranul_set(t_bufGranul* x, t_symbol* msg, short ac, t_atom* av)
{
    // Changement du buffer son
    t_buffer* b;
    int       tmp;
    int       num;
    t_symbol* s;
    // 	post("bufGranul_set %s",s->s_name);

    if (ac == 2) {
        num = (int)atom2float(av, 0);
        if (num < 0 || num >= NSBUF) {
            post("bufGranul~ : <#buffer> out of range");
            return;
        }

        if (!(s = atom2symbol(av, 1))) {
            return;
        }

    } else if (ac == 1) {
        if (!(s = atom2symbol(av, 0))) {
            return;
        }
        num = 0;
    } else {
        post("bufGranul~ : error set <#buffer> <buffer name> ");
        return;
    }

    x->x_buf_sym[num] = s;
    if (x->x_nbuffer < num + 1) {
        x->x_nbuffer = num + 1;
    }

    if ((b = (t_buffer*)(s->s_thing)) && ob_sym(b) == ps_buffer && b && b->b_valid) {
        x->x_buf_filename[num] = b->b_filename;
        x->x_buf_nchan[num]    = b->b_nchans;
        x->x_buf_samples[num]  = b->b_samples;
        x->x_buf_frames[num]   = b->b_frames;  //*/
        x->x_buf[num]          = b;
        // On change de buffer actif
        // x->x_active_buf = tmp;

        bufGranul_info(x, 0);

    } else {
        error("bufGranul~: no buffer~ %s", s->s_name);
        x->x_buf[num] = 0;
    }
}

int bufGranul_bufferinfos(t_bufGranul* x)
{
    int       cpt;
    t_buffer* b;
    long      loopstart, loopend, looplength;
    float     srms = x->x_sr * 0.001;
    int       loop = x->x_loop;

    // retourne 0 : sortie nulle
    // retourne 1 : calcul des grains

    // Test si enabled
    if (x->x_obj.z_disabled) {
        return -1;
    }

    // init des index � 0
    memset(x->x_buf_valid_index, 0, NSBUF * sizeof(int));

    // Test sur la validit� et l'activite des buffers & initialisations
    for (cpt = 0; cpt < x->x_nbuffer; cpt++) {
        if (x->x_buf_sym[cpt])  // si un nom de buffer a �t� donn� -> collecter les infos
        {
            if ((b = (t_buffer*)(x->x_buf_sym[cpt]->s_thing)) && ob_sym(b) == ps_buffer &&
                b->b_valid && !b->b_inuse)  // attention inuse !!!
            {
                x->x_buf[cpt]             = b;
                x->x_buf_valid_index[cpt] = cpt;
                x->x_buf_filename[cpt]    = b->b_filename;
                x->x_buf_nchan[cpt]       = b->b_nchans;
                x->x_buf_samples[cpt]     = b->b_samples;
                x->x_buf_frames[cpt]      = b->b_frames;
                x->x_buf_sronsr[cpt]      = (float)b->b_sr / x->x_sr;

                if (loop) {
                    if (loop == 1) {
                        loopstart  = 0;
                        loopend    = b->b_frames;
                        looplength = loopend - loopstart;
                    }
                    if (loop == 2) {
                        loopstart  = SAT(x->x_loopstart * srms, 0, b->b_frames);
                        loopend    = SAT(x->x_loopend * srms, 0, b->b_frames);
                        looplength = loopend - loopstart;
                    }

                    x->x_buf_loopstart[cpt]  = loopstart;
                    x->x_buf_loopend[cpt]    = loopend;
                    x->x_buf_looplength[cpt] = looplength;
                }

            } else {
                // post("invalid %d",cpt);
                //  le buffer 0 est celui utilis� si celui choisi est pas valide ... si il n'y est
                //  pas -> pas dsp !!!
                if (cpt == 0) {
                    return 0;
                }
                x->x_buf[cpt] = 0;
            }
        }
    }

    for (cpt = 0; cpt < x->x_nenvbuffer; cpt++) {
        if (x->x_env_sym[cpt])  // si un nom de buffer a �t� donn� -> collecter les infos
        {
            if ((b = (t_buffer*)(x->x_env_sym[cpt]->s_thing)) && ob_sym(b) == ps_buffer &&
                b->b_valid && !b->b_inuse)  // attention inuse !!!
            {
                x->x_env_buf[cpt]     = b;
                x->x_env_samples[cpt] = b->b_samples;
                x->x_env_frames[cpt]  = b->b_frames;

            } else {
                // post("invalid %d",cpt);
                //  le buffer 0 est celui utilis� si celui choisi est pas valide ... si il n'y est
                //  pas -> pas dsp !!!
                if (cpt == 0) {
                    return 0;
                }
                x->x_env_buf[cpt] = 0;
            }
        }
    }

    return 1;
}

// BUFFERS ENVELOPPE

void bufGranul_setenv(t_bufGranul* x, t_symbol* msg, short ac, t_atom* av)
{
    // Changement du buffer enveloppe
    t_buffer* b;
    int       num;
    int       tmp;
    t_symbol* s;
    // 	post("bufGranul_setenv %s",s->s_name);

    if (ac == 2) {
        num = (int)atom2float(av, 0);
        if (num < 0 || num >= NEBUF) {
            post("bufGranul~ : <#buffer> out of range");
            return;
        }

        if (!(s = atom2symbol(av, 1))) {
            return;
        }

    } else if (ac == 1) {
        if (!(s = atom2symbol(av, 0))) {
            return;
        }
        num = 0;

    } else {
        post("bufGranul~ : error setenv <#buffer> <buffer name> ");
        return;
    }

    x->x_env_sym[num] = s;
    if (x->x_nenvbuffer < num + 1) {
        x->x_nenvbuffer = num + 1;
    }

    if ((b = (t_buffer*)(s->s_thing)) && ob_sym(b) == ps_buffer && b && b->b_valid) {
        x->x_env_buf[num]     = b;
        x->x_env_sym[num]     = s;
        x->x_env_samples[num] = b->b_samples;
        x->x_env_frames[num]  = b->b_frames;
        bufGranul_info(x, 1);
    } else {
        error("bufGranul~: no buffer~ %s", s->s_name);
        x->x_env_buf[num] = 0;
    }
}

// affectation du num�ro de buffer enveloppe actif
void bufGranul_envbuffer(t_bufGranul* x, long n)
{
    x->x_active_env = (n < 0) ? 0 : ((n >= NEBUF) ? NEBUF - 1 : n);
}

// DSP

void bufGranul_dsp64(t_bufGranul* x, t_object* dsp64, short* count, double samplerate,
                     long maxvectorsize, long flags)
{
    int i;
    x->x_sr = sys_getsr();

    // signal connected to inlets ?
    x->x_in2con = count[1] > 0;
    x->x_in3con = count[2] > 0;
    x->x_in4con = count[3] > 0;
    x->x_in5con = count[4] > 0;
    x->x_in6con = count[5] > 0;
    x->x_in7con = count[6] > 0;
    x->x_in8con = count[7] > 0;

    // generate fadein and fadeout

    for (i = 0; i < maxvectorsize; i++) {
        x->x_fadein[i]       = (double)i / maxvectorsize;
        x->x_kill_fadeout[i] = (double)(maxvectorsize - i) / maxvectorsize;
    }

    object_method(dsp64, gensym("dsp_add64"), x, bufGranul_perform, 0, NULL);
}

void bufGranul_free(t_bufGranul* x)
{
    bufGranul_desalloc(x);

    if (x->x_linear_interp_table) {
        sysmem_freeptr(x->x_linear_interp_table);
    }

    dsp_free((t_pxobject*)x);
}

// routines allocations

int bufGranul_alloc(t_bufGranul* x)
{
    if (!(x->x_ind = (long*)sysmem_newptr(NVOICES * sizeof(long)))) {
        return 0;
    }
    if (!(x->x_remain_ind = (long*)sysmem_newptr(NVOICES * sizeof(long)))) {
        return 0;
    }
    if (!(x->x_sind = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }

    if (!(x->Vbeg = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->Vtranspos = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->Vamp = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->Vlength = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->Vpan = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->Vdist = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }

    if (!(x->Vloop = (long*)sysmem_newptr(NVOICES * sizeof(long)))) {
        return 0;
    }
    if (!(x->Vloopstart = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->Vloopend = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }

    if (!(x->Vbuf = (long*)sysmem_newptr(NVOICES * sizeof(long)))) {
        return 0;
    }

    if (!(x->envinc = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->envind = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->x_env = (double*)sysmem_newptr(NVOICES * sizeof(double)))) {
        return 0;
    }
    if (!(x->Venv = (long*)sysmem_newptr(NVOICES * sizeof(long)))) {
        return 0;
    }

    if (!(x->x_delay = (int*)sysmem_newptr(NVOICES * sizeof(int)))) {
        return 0;
    }
    if (!(x->x_voiceOn = (int*)sysmem_newptr(NVOICES * sizeof(int)))) {
        return 0;
    }

    if (!(x->x_hp = (double*)sysmem_newptr(x->x_nouts * sizeof(double)))) {
        return 0;
    }

    if (!(x->Vhp = (double**)sysmem_newptr(NVOICES * sizeof(double*)))) {
        return 0;
    }

    for (int i = 0; i < NVOICES; i++) {
        if (!(x->Vhp[i] = (double*)sysmem_newptr(x->x_nouts * sizeof(double)))) {
            return 0;
        }
    }

    return 1;
}

int bufGranul_desalloc(t_bufGranul* x)
{
    sysmem_freeptr(x->x_ind);
    sysmem_freeptr(x->x_remain_ind);
    sysmem_freeptr(x->x_sind);

    sysmem_freeptr(x->Vbeg);
    sysmem_freeptr(x->Vtranspos);
    sysmem_freeptr(x->Vamp);
    sysmem_freeptr(x->Vlength);
    sysmem_freeptr(x->Vpan);
    sysmem_freeptr(x->Vdist);

    sysmem_freeptr(x->Vloop);
    sysmem_freeptr(x->Vloopstart);
    sysmem_freeptr(x->Vloopend);

    sysmem_freeptr(x->Vbuf);

    sysmem_freeptr(x->envinc);
    sysmem_freeptr(x->envind);
    sysmem_freeptr(x->x_env);
    sysmem_freeptr(x->Venv);

    sysmem_freeptr(x->x_delay);
    sysmem_freeptr(x->x_voiceOn);

    sysmem_freeptr(x->x_hp);

    for (int i = 0; i < NVOICES; i++) {
        sysmem_freeptr(x->Vhp[i]);
    }

    sysmem_freeptr(x->Vhp);

    return 1;
}

void bufGranul_perform(t_bufGranul* x, t_object* dsp64, double** ins, long numins, double** outs,
                       long numouts, long sampleframes, long flags, void* userparam)
{
    t_double* in         = ins[0];
    t_double* t_begin    = ins[1];
    t_double* t_transpos = ins[2];
    t_double* t_amp      = ins[3];
    t_double* t_length   = ins[4];
    t_double* t_pan      = ins[5];
    t_double* t_dist     = ins[6];
    t_double* t_sndbuf   = ins[7];

    int n = sampleframes;

    int N  = n;
    int xn = 0, p, hp;

    double srms = x->x_sr * 0.001;  // double for precision
    float  val;
    double sigin, prevsigin, savein;  // values of actual and previous input sample

    double beginind  = x->x_begin * srms;  // double for precision (this number can be very large)
    double lengthind = x->x_length * srms;

    // temporary buffer index variable
    int t_active_buf;  // buffer actif pour la création si pas entrée bufnum signal (
                       // x->x_active_buf peut ne plus être valide )
    int    t_bufnum;
    int    t_bufchan;
    int    t_bchan_offset = x->x_bchan_offset;
    int    t_bchan_offset_mod;
    int    t_active_env;
    float* t_bufsamples;
    float* t_envsamples;

    int  i, j;
    long k, l, m, k_base;
    int  nvoices = x->x_nvoices;

    int initTest;

    // variables grains
    int t_bufframes;

    double finc;
    double temp_sind;
    double source_findex, target_findex;
    int    dsp_i_begin;
    int    dsp_i_end;

    long loop_length;
    long loop_start, loop_end;

    int nech_process, nech_process_sat;

    // trick interp long index
    long base_lindex;
    long lindex;
    long lincr;

    double sinc_indexf;
    double fract_index;
    double hp_norm;

    long base_lindex_env;
    long lindex_env;
    long lincr_env;

    int buffer_index;
    int buffer_index_plus_1;
    int interp_table_index;

    t_linear_interp* interp_table      = x->x_linear_interp_table;
    double*          sinc_interp_table = x->x_sinc_interp_table;
    int              sinc_phasei       = SINC_TABLE_SIZE / 16;

    double* fade_table;

    double microtiming_coeff;

    // debug var
#ifdef PERF_DEBUG
    int n_voices_active;
#endif

    sigin = x->x_sigin;

#ifdef PERF_DEBUG
    if (x->ask_poll) {
        _SETSYM(x->info_list, gensym("voices"));
        _SETLONG(x->info_list + 1, x->x_nvoices_active);
        outlet_list(x->info, 0l, 2, x->info_list);
    }
#endif

    initTest = bufGranul_bufferinfos(x);
    if (initTest == 0) {
        goto zero;
    } else {
        t_active_buf = buffer_check(x, x->x_active_buf);
        t_active_env = bufferenv_check(x, x->x_active_env);
        // %%%%%%%%%%     creation de grains   %%%%%%%%%%%%%%%%%

        p = nvoices;

        // Dans le cas d'un grain declenche par un bang
        savein = in[0];
        if (x->x_askfor) {
            sigin       = -1.;
            in[0]       = 1.;
            x->x_askfor = 0;
        }

        // Meme chose avec declenchement par zero-crossing sur tout le vecteur in, dans la limite
        // fixee par nvoices
        while (n-- && p) {
            //-----signal zero-crossing ascendant declenche un grain----//
            prevsigin = sigin;
            sigin     = in[xn];
            if (prevsigin <= 0 && sigin > 0)  // creation d'un grain
            {
                microtiming_coeff = x->x_microtiming * prevsigin / (sigin - prevsigin);

                p = bufGranul_poly_assign_voice(x);  // get free voice according to polymode

                if (p) {
                    x->x_voiceOn[p] = 1;
                    x->x_sind[p] = x->Vbeg[p] = microtiming_coeff +
                                                (x->x_in2con) * t_begin[xn] * srms +
                                                (1 - x->x_in2con) * beginind;  // index dans buffer
                    x->Vtranspos[p] = (x->x_in3con) * t_transpos[xn] +
                                      (1 - x->x_in3con) * x->x_transpos;  // valeur de pitch
                    x->Vamp[p] =
                        (x->x_in4con) * t_amp[xn] + (1 - x->x_in4con) * x->x_amp;  // amplitude
                    x->Vbuf[p] = (x->x_in8con) ? buffer_check(x, (int)t_sndbuf[xn])
                                               : t_active_buf;  // numero du buffer son
                    x->Venv[p] = t_active_env;                  // enveloppe active pour ce grain

                    if (x->x_in5con) {
                        if (t_length[xn] < 0) {
                            x->Vlength[p] = -t_length[xn] * srms;
                            x->envinc[p] =
                                -1. * (float)(x->x_env_frames[t_active_env] - 2) / x->Vlength[p];
                            x->envind[p] = x->x_env_frames[t_active_env] - 1 +
                                           (1 + microtiming_coeff) * x->envinc[p];
                        } else if (t_length[xn] == 0.)  // whole buffer
                        {
                            x->Vlength[p] = x->x_buf_frames[x->Vbuf[p]];
                            x->envinc[p] =
                                (float)(x->x_env_frames[t_active_env] - 2) / x->Vlength[p];
                            x->envind[p] = (1 + microtiming_coeff) * x->envinc[p];
                        } else {
                            x->Vlength[p] = t_length[xn] * srms;
                            x->envinc[p] =
                                (float)(x->x_env_frames[t_active_env] - 2) / x->Vlength[p];
                            x->envind[p] = (1 + microtiming_coeff) * x->envinc[p];
                        }
                    } else {
                        if (lengthind == 0.) {
                            x->Vlength[p] = x->x_buf_frames[x->Vbuf[p]];
                        } else {
                            x->Vlength[p] = lengthind;
                        }
                        x->envinc[p] = x->x_env_dir * x->x_env_frames[t_active_env] / x->Vlength[p];
                        if (x->x_env_dir < 0) {
                            x->envind[p] = x->x_env_frames[t_active_env] - 1;
                        } else {
                            x->envind[p] = 0.;
                        }
                    }

                    x->Vpan[p] = (x->x_in6con) * t_pan[xn] + (1 - x->x_in6con) * x->x_pan;  // pan
                    x->Vpan[p] = MOD(x->Vpan[p], 1);
                    x->Vdist[p] =
                        (x->x_in7con) * t_dist[xn] + (1 - x->x_in7con) * x->x_dist;  // distance

                    x->Vloop[p]      = x->x_loop;
                    x->Vloopstart[p] = x->x_loopstart;
                    x->Vloopend[p]   = x->x_loopend;

                    panner(x->Vhp[p], x->x_nouts, MAX(x->Vpan[p], 0), SAT(x->Vdist[p], 0., 1.));

                    x->x_ind[p]        = 0;
                    x->x_remain_ind[p] = (long)x->Vlength[p];
                    x->x_delay[p]      = xn;  // delay de declenchement dans le vecteur

                } else {
                    goto perform;
                }
            }
            xn++;  // boucle on incremente le pointeur dans le vecteur in et le delay de
                   // declenchement
        }

        // %%%%%%%%%%     fin creation     %%%%%%%%%%%%%%%%%

    perform:

        in[0] = savein;
        //&&&&&&&&&&&&&  Boucle DSP  &&&&&&&&&&&&&
        n = N;
        j = 0;

        for (hp = 0; hp < x->x_nouts; hp++) {
            memset(outs[hp], 0, N * sizeof(double));
        }

        for (i = 0; i < NVOICES; i++) {
            // si la voix est en cours
            if (x->x_voiceOn[i] > 0)  //&& x->x_ind[i] < x->Vlength[i] )
            {
                // si delay + grand que taille vecteur on passe à voix suivante
                if (x->x_delay[i] >= N) {
                    if (x->x_voiceOn[i] == 3) {  // kill mode -> free voice
                        x->x_voiceOn[i] = 0;
                    } else {
                        x->x_voiceOn[i] = 2;  // wait mode
                    }
                    x->x_delay[i] -= N;
                    goto next;
                }

                if (x->x_voiceOn[i] ==
                    2)  // waiting voice now ready for playing -> kill voices if poly exceeded
                {
                    bufGranul_poly_check_and_kill(x);
                    x->x_voiceOn[i] = 1;
                }
                if (x->x_voiceOn[i] == 3) {  // kill voice
                    fade_table = x->x_kill_fadeout +
                                 x->x_delay[i];  // decalage du pointeur de fade out par le delai
                }
                // else if(x->x_ind[i]==0) // fadein
                //     fade_table=x->x_fadein;
                else {
                    fade_table = x->x_unity_gain;
                }

                // nb d'ech a caluler pour ce vecteur
                nech_process = MIN((N - x->x_delay[i]), x->x_remain_ind[i]);

                // Selon le buffer dans lequel on doit prendre le son x->Vbuf[i] (voir creation des
                // grains) Il se peut que le buffer associé au grain ne soit plus valide ->
                // verification -> si non on prend buffer #0
                t_bufnum           = x->x_buf_valid_index[x->Vbuf[i]];
                t_bufframes        = x->x_buf_frames[t_bufnum];
                t_bufsamples       = x->x_buf_samples[t_bufnum];
                t_bufchan          = x->x_buf_nchan[t_bufnum];
                t_bchan_offset_mod = t_bchan_offset % t_bufchan;

                t_envsamples = x->x_env_samples[x->Venv[i]];

                // pas d'increment en float
                finc = (x->Vtranspos[i] * x->x_buf_sronsr[t_bufnum]);

                // snd index source
                source_findex = x->x_sind[i];
                // snd index cible en fin de boucle
                target_findex = source_findex + nech_process * finc;

                // nb ech a calculer different selon boucle ou pas
                if (!x->Vloop[i]) {
                    // si index source hors des bornes -> kill grain
                    if (source_findex > t_bufframes || source_findex < 0) {
                        x->x_voiceOn[i] = 0;
                        goto next;
                    }

                    // nb SATURE d'ech a calculer pour ce vecteur (0 < ... < frames) ou (startloop <
                    // ... < endloop)
                    if (target_findex > t_bufframes) {
                        // on met la voix off
                        x->x_voiceOn[i] = 0;
                        // jusqu'a quel index on calcule
                        nech_process_sat = (long)(nech_process * (t_bufframes - source_findex) /
                                                  (target_findex - source_findex));

                        dsp_i_begin = x->x_delay[i];
                        dsp_i_end   = x->x_delay[i] + nech_process_sat;

                        temp_sind   = x->x_sind[i];
                        base_lindex = (long)x->x_sind[i];
                        lindex      = interp_index_scale(x->x_sind[i] - (double)base_lindex);
                        lincr       = interp_index_scale(finc);

                    } else if (target_findex < 0) {
                        // on met la voix off
                        x->x_voiceOn[i] = 0;
                        // jusqu'a quel index on calcule   ATTENTION arrondi depassement
                        nech_process_sat = (long)(nech_process * (source_findex) /
                                                  (source_findex - target_findex));

                        dsp_i_begin = x->x_delay[i];
                        dsp_i_end   = x->x_delay[i] + nech_process_sat;

                        temp_sind   = x->x_sind[i];
                        base_lindex = (long)x->x_sind[i];
                        lindex      = interp_index_scale(x->x_sind[i] - (double)base_lindex);
                        lincr       = interp_index_scale(finc);

                    } else {
                        nech_process_sat = nech_process;

                        dsp_i_begin = x->x_delay[i];
                        dsp_i_end   = x->x_delay[i] + nech_process_sat;

                        temp_sind   = x->x_sind[i];
                        base_lindex = (long)x->x_sind[i];
                        lindex      = interp_index_scale(x->x_sind[i] - (double)base_lindex);
                        lincr       = interp_index_scale(finc);

                        fract_index = x->x_sind[i] - (double)base_lindex;
                    }
                } else  // if loop
                {
                    /////////// LOOOP //////////
                    nech_process_sat = nech_process;

                    // target_findex = MOD(x->x_sind[i],x->x_buf_looplength[t_bufnum]);

                    dsp_i_begin = x->x_delay[i];
                    dsp_i_end   = x->x_delay[i] + nech_process_sat;

                    if (x->Vloop[i] == 1) {
                        loop_start  = 0;
                        loop_end    = t_bufframes;
                        loop_length = loop_end - loop_start;
                    }
                    if (x->Vloop[i] == 2) {
                        loop_start  = SAT(x->Vloopstart[i] * srms, 0, t_bufframes - 1);
                        loop_end    = SAT(x->Vloopend[i] * srms, loop_start + 1, t_bufframes);
                        loop_length = loop_end - loop_start;
                    }

                    // calcul du vrai float index en ech de lecture
                    temp_sind = x->x_sind[i];
                    temp_sind -= loop_start;
                    temp_sind = loop_start + MOD(temp_sind, loop_length);

                    // calcul de l'index de lecture buffer en long
                    base_lindex = (long)temp_sind;
                    lindex      = interp_index_scale(temp_sind - (double)base_lindex);
                    lincr       = interp_index_scale(finc);
                }

                // Enveloppe long index & incr calcul
                base_lindex_env = (long)x->envind[i];
                lindex_env      = interp_index_scale(x->envind[i] - (double)base_lindex_env);
                lincr_env       = interp_index_scale(x->envinc[i]);

                x->envind[i] += nech_process_sat * x->envinc[i];

                //***********************************
                // CALCUL EFFECTIF DES ECHANTILLONS
                //***********************************

                if (x->Vloop[i]) {
                    if (x->x_sinterp == 1) {  // loop + interp
                        for (j = dsp_i_begin; j < dsp_i_end; j++) {
                            // Lecture de l'enveloppe
                            buffer_index       = base_lindex_env + interp_get_int(lindex_env);
                            interp_table_index = interp_get_table_index(lindex_env);
                            x->x_env[i] =
                                interp_table[interp_table_index].a * t_envsamples[buffer_index] +
                                interp_table[interp_table_index].b * t_envsamples[buffer_index + 1];

                            lindex_env += lincr_env;

                            // x->envind[i] += x->envinc[i]; // 2 REMOVE

                            // Lecture de la forme d'onde
                            buffer_index = base_lindex + interp_get_int(lindex);
                            buffer_index -= loop_start;
                            buffer_index = loop_start + MODI(buffer_index, loop_length);

                            buffer_index_plus_1 = buffer_index + 1;
                            // buffer_index_plus_1 =
                            // MODI(buffer_index_plus_1,x->x_buf_looplength[t_bufnum]);

                            interp_table_index = interp_get_table_index(lindex);
                            val =
                                interp_table[interp_table_index].a *
                                    t_bufsamples[(buffer_index * t_bufchan) + t_bchan_offset_mod] +
                                interp_table[interp_table_index].b *
                                    t_bufsamples[(buffer_index_plus_1 * t_bufchan) +
                                                 t_bchan_offset_mod];

                            lindex += lincr;

                            // calcul de la valeur env[i]*son[i]*amp[i]
                            val = x->x_env[i] * val * x->Vamp[i] * (*fade_table++);

                            // calcul du pan en fonction de Vhp1[i] et Vhp2[i]
                            for (hp = 0; hp < x->x_nouts; hp++) {
                                outs[hp][j] += x->Vhp[i][hp] * val;
                            }
                        }
                    } else if (x->x_sinterp ==
                               2)  // LOOP + interp SINC ///****************************************
                                   // ///
                    {
                        double highpass_fact = (finc > 1.) ? 1 / finc : 1.;
                        long   norm_index    = 0;

                        for (j = dsp_i_begin; j < dsp_i_end; j++) {
                            // Lecture de l'enveloppe
                            buffer_index       = base_lindex_env + interp_get_int(lindex_env);
                            interp_table_index = interp_get_table_index(lindex_env);
                            x->x_env[i] =
                                interp_table[interp_table_index].a * t_envsamples[buffer_index] +
                                interp_table[interp_table_index].b * t_envsamples[buffer_index + 1];

                            lindex_env += lincr_env;

                            val                = 0.;
                            long temp_iind     = (long)temp_sind;
                            fract_index        = temp_sind - (double)temp_iind;
                            long fract_index_i = (double)(fract_index * (double)sinc_phasei);

                            for (m = -7; m < 9; m++) {
                                k = temp_iind + m;
                                k -= loop_start;
                                k = MODI(k, loop_length);
                                k += loop_start;

                                val += x->x_blackman_table[x_sinc_table_offset[m + 7] +
                                                           fract_index_i] *
                                       x->x_sinc_interp_table[(
                                           long)(8192 + highpass_fact *
                                                            (8192 - (x_sinc_table_offset[m + 7] +
                                                                     fract_index_i)))] *
                                       t_bufsamples[(k * t_bufchan) + t_bchan_offset_mod];
                                //   printf( " index %f
                                //   ",x->x_sinc_interp_table[x_sinc_table_offset[m+7]]);
                            }

                            temp_sind += finc;

                            norm_index = (long)(highpass_fact * SINC_TABLE_SIZE) - 1;
                            // printf("nindex %d\n",norm_index);

                            hp_norm = x->x_sinc_norm_table[norm_index];

                            // calcul de la valeur env[i]*son[i]*amp[i]
                            val = x->x_env[i] * val * x->Vamp[i] * hp_norm * (*fade_table++);

                            // calcul du pan en fonction de Vhp1[i] et Vhp2[i]
                            for (hp = 0; hp < x->x_nouts; hp++) {
                                outs[hp][j] += x->Vhp[i][hp] * val;
                            }
                        }
                    } else {  // loop + non interp
                        for (j = dsp_i_begin; j < dsp_i_end; j++) {
                            // Lecture de l'enveloppe
                            buffer_index = base_lindex_env + interp_get_int(lindex_env);
                            x->x_env[i]  = t_envsamples[buffer_index];

                            lindex_env += lincr_env;

                            // x->envind[i] += x->envinc[i]; // 2 REMOVE

                            // Lecture de la forme d'onde
                            buffer_index = base_lindex + interp_get_int(lindex);
                            buffer_index -= x->x_buf_loopstart[t_bufnum];
                            buffer_index =
                                loop_start + MODI(buffer_index, x->x_buf_looplength[t_bufnum]);

                            val = t_bufsamples[(buffer_index * t_bufchan) + t_bchan_offset_mod];

                            lindex += lincr;

                            // calcul de la valeur env[i]*son[i]*amp[i]
                            val = x->x_env[i] * val * x->Vamp[i] * (*fade_table++);

                            // calcul du pan en fonction de Vhp1[i] et Vhp2[i]
                            for (hp = 0; hp < x->x_nouts; hp++) {
                                outs[hp][j] += x->Vhp[i][hp] * val;
                            }
                        }
                    }
                }

                else                          // if non loop
                    if (x->x_sinterp == 1) {  // non loop + interp
                        for (j = dsp_i_begin; j < dsp_i_end; j++) {
                            // Lecture de l'enveloppe
                            buffer_index       = base_lindex_env + interp_get_int(lindex_env);
                            interp_table_index = interp_get_table_index(lindex_env);
                            x->x_env[i] =
                                interp_table[interp_table_index].a * t_envsamples[buffer_index] +
                                interp_table[interp_table_index].b * t_envsamples[buffer_index + 1];

                            lindex_env += lincr_env;

                            // x->envind[i] += x->envinc[i]; // 2 REMOVE

                            // Lecture de la forme d'onde
                            buffer_index       = base_lindex + interp_get_int(lindex);
                            interp_table_index = interp_get_table_index(lindex);
                            val =
                                interp_table[interp_table_index].a *
                                    t_bufsamples[(buffer_index * t_bufchan) + t_bchan_offset_mod] +
                                interp_table[interp_table_index].b *
                                    t_bufsamples[((buffer_index + 1) * t_bufchan) +
                                                 t_bchan_offset_mod];

                            lindex += lincr;

                            // calcul de la valeur env[i]*son[i]*amp[i]
                            val = x->x_env[i] * val * x->Vamp[i] * (*fade_table++);

                            // calcul du pan en fonction de Vhp1[i] et Vhp2[i]
                            for (hp = 0; hp < x->x_nouts; hp++) {
                                outs[hp][j] += x->Vhp[i][hp] * val;
                            }
                        }
                    } else if (x->x_sinterp ==
                               2) {  // non loop + interp SINC
                                     // ///**************************************** ///
                        for (j = dsp_i_begin; j < dsp_i_end; j++) {
                            // Lecture de l'enveloppe
                            buffer_index       = base_lindex_env + interp_get_int(lindex_env);
                            interp_table_index = interp_get_table_index(lindex_env);
                            x->x_env[i] =
                                interp_table[interp_table_index].a * t_envsamples[buffer_index] +
                                interp_table[interp_table_index].b * t_envsamples[buffer_index + 1];

                            lindex_env += lincr_env;

                            val                  = 0.;
                            long temp_iind       = (long)temp_sind;
                            fract_index          = temp_sind - (double)temp_iind;
                            long   fract_index_i = (double)(fract_index * (double)sinc_phasei);
                            double highpass_fact = (finc > 1.) ? 1 / finc : 1.;
                            long   norm_index    = 0;

                            for (m = -7; m < 9; m++) {
                                k = temp_iind + m;
                                k = SAT(k, 0, t_bufframes);
                                val += x->x_blackman_table[x_sinc_table_offset[m + 7] +
                                                           fract_index_i] *
                                       x->x_sinc_interp_table[(
                                           long)(8192 + highpass_fact *
                                                            (8192 - (x_sinc_table_offset[m + 7] +
                                                                     fract_index_i)))] *
                                       t_bufsamples[(k * t_bufchan) + t_bchan_offset_mod];
                                //   printf( " index %f
                                //   ",x->x_sinc_interp_table[x_sinc_table_offset[m+7]]);
                            }

                            temp_sind += finc;

                            norm_index = (long)(highpass_fact * SINC_TABLE_SIZE) - 1;
                            // printf("nindex %d\n",norm_index);

                            hp_norm = x->x_sinc_norm_table[norm_index];

                            // calcul de la valeur env[i]*son[i]*amp[i]
                            val = x->x_env[i] * val * x->Vamp[i] * hp_norm * (*fade_table++);

                            // calcul du pan en fonction de Vhp1[i] et Vhp2[i]
                            for (hp = 0; hp < x->x_nouts; hp++) {
                                outs[hp][j] += x->Vhp[i][hp] * val;
                            }
                        }
                    } else {  // non loop + non interp
                        for (j = dsp_i_begin; j < dsp_i_end; j++) {
                            // Lecture de l'enveloppe
                            buffer_index = base_lindex_env + interp_get_int(lindex_env);
                            x->x_env[i]  = t_envsamples[buffer_index];

                            lindex_env += lincr_env;

                            // x->envind[i] += x->envinc[i]; // 2 REMOVE

                            // Lecture de la forme d'onde
                            buffer_index = base_lindex + interp_get_int(lindex);
                            val = t_bufsamples[(buffer_index * t_bufchan) + t_bchan_offset_mod];

                            lindex += lincr;

                            // calcul de la valeur env[i]*son[i]*amp[i]
                            val = x->x_env[i] * val * x->Vamp[i] * (*fade_table++);

                            // calcul du pan en fonction de Vhp1[i] et Vhp2[i]
                            for (hp = 0; hp < x->x_nouts; hp++) {
                                outs[hp][j] += x->Vhp[i][hp] * val;
                            }
                        }
                    }

                // post("%d %d dsp_i_begin",dsp_i_begin,dsp_i_end);

                //********************************
                //  MAJ de l'état des grains

                x->x_sind[i] = target_findex;

                x->x_ind[i] += nech_process_sat;

                if (((x->x_remain_ind[i] -= nech_process_sat) <= 0) || x->x_voiceOn[i] == 3) {
                    x->x_voiceOn[i] = 0;
                    // post("remain kill");
                }

                // decremente delai
                x->x_delay[i] = MAX(x->x_delay[i] - N, 0);

            } else {
                // sinon = si la voix est libre//
                x->x_voiceOn[i] = 0;
            }

        next:
            j = 0;  // DUMMY

        }  // fin for

        x->x_sigin = sigin;

        return;
    }

zero:
    // Pas de declenchement de grains
    for (hp = 0; hp < x->x_nouts; hp++) {
        memset(outs[hp], 0, N * sizeof(double));
    }

out:
    return;
}

// THE END, that's all hulk!!!!!!
