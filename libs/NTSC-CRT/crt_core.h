/*****************************************************************************/
/*
 * NTSC/CRT - integer-only NTSC video signal encoding / decoding emulation
 *
 *   by EMMIR 2018-2023
 *
 *   YouTube: https://www.youtube.com/@EMMIR_KC/videos
 *   Discord: https://discord.com/invite/hdYctSmyQJ
 */
/*****************************************************************************/
#ifndef _CRT_CORE_H_
#define _CRT_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* crt_core.h
 *
 * The demodulator. This is also where you can define which system to emulate.
 * 
 */
    
/* library version */
#define CRT_MAJOR 2
#define CRT_MINOR 3
#define CRT_PATCH 0

    
#define CRT_SYSTEM_NTSC     0 /* standard NTSC */
#define CRT_SYSTEM_NES      1 /* decode 6 or 9-bit NES pixels */
#define CRT_SYSTEM_PV1K     2 /* Casio PV-1000 */
#define CRT_SYSTEM_SNES     3 /* SNES - uses RGB */
#define CRT_SYSTEM_TEMP     4 /* template implementation */
#define CRT_SYSTEM_NTSCVHS  5 /* standard NTSC VHS */
#define CRT_SYSTEM_NESRGB   6 /* encode RGB image with NES artifacts */

/* the system to be compiled */
#ifndef CRT_SYSTEM
#define CRT_SYSTEM CRT_SYSTEM_NES
#endif

#if (CRT_SYSTEM == CRT_SYSTEM_NES)
#include "crt_nes.h"
#elif (CRT_SYSTEM == CRT_SYSTEM_SNES)
#include "crt_snes.h"
#elif (CRT_SYSTEM == CRT_SYSTEM_NTSC)
#include "crt_ntsc.h"
#elif (CRT_SYSTEM == CRT_SYSTEM_PV1K)
#include "crt_pv1k.h"
#elif (CRT_SYSTEM == CRT_SYSTEM_TEMP)
#include "crt_template.h"
#elif (CRT_SYSTEM == CRT_SYSTEM_NTSCVHS)
#include "crt_ntscvhs.h"
#elif (CRT_SYSTEM == CRT_SYSTEM_NESRGB)
#include "crt_nesrgb.h"
#else
#error No system defined
#endif

/* NOTE: this library does not use the alpha channel at all */
#define CRT_PIX_FORMAT_RGB  0  /* 3 bytes per pixel [R,G,B,R,G,B,R,G,B...] */
#define CRT_PIX_FORMAT_BGR  1  /* 3 bytes per pixel [B,G,R,B,G,R,B,G,R...] */
#define CRT_PIX_FORMAT_ARGB 2  /* 4 bytes per pixel [A,R,G,B,A,R,G,B...]   */
#define CRT_PIX_FORMAT_RGBA 3  /* 4 bytes per pixel [R,G,B,A,R,G,B,A...]   */
#define CRT_PIX_FORMAT_ABGR 4  /* 4 bytes per pixel [A,B,G,R,A,B,G,R...]   */
#define CRT_PIX_FORMAT_BGRA 5  /* 4 bytes per pixel [B,G,R,A,B,G,R,A...]   */

/* do bloom emulation (side effect: makes screen have black borders) */
#define CRT_DO_BLOOM    0  /* does not work for NES */
#define CRT_DO_VSYNC    1  /* look for VSYNC */
#define CRT_DO_HSYNC    1  /* look for HSYNC */

struct CRT {
    signed char analog[CRT_INPUT_SIZE];
    signed char inp[CRT_INPUT_SIZE]; /* CRT input, can be noisy */

    int outw, outh; /* output width/height */
    int out_format; /* output pixel format (one of the CRT_PIX_FORMATs) */
    unsigned char *out; /* output image */

    int hue, brightness, contrast, saturation; /* common monitor settings */
    int black_point, white_point; /* user-adjustable */
    int scanlines; /* leave gaps between lines if necessary */
    int blend; /* blend new field onto previous image */
    unsigned v_fac; /* factor to stretch img vertically onto the output img */

    /* internal data */
    int ccf[CRT_CC_VPER][CRT_CC_SAMPLES]; /* faster color carrier convergence */
    int hsync, vsync; /* keep track of sync over frames */
    int rn; /* seed for the 'random' noise */
};

/* Initializes the library. Sets up filters.
 *   w   - width of the output image
 *   h   - height of the output image
 *   f   - format of the output image
 *   out - pointer to output image data
 */
extern void crt_init(struct CRT *v, int w, int h, int f, unsigned char *out);

/* Updates the output image parameters
 *   w   - width of the output image
 *   h   - height of the output image
 *   f   - format of the output image
 *   out - pointer to output image data
 */
extern void crt_resize(struct CRT *v, int w, int h, int f, unsigned char *out);

/* Resets the CRT settings back to their defaults */
extern void crt_reset(struct CRT *v);

/* Modulates RGB image into an analog NTSC signal
 *   s - struct containing settings to apply to this field
 */
extern void crt_modulate(struct CRT *v, struct NTSC_SETTINGS *s);
    
/* Demodulates the NTSC signal generated by crt_modulate()
 *   noise - the amount of noise added to the signal (0 - inf)
 */
extern void crt_demodulate(struct CRT *v, int noise);

/* Get the bytes per pixel for a certain CRT_PIX_FORMAT_
 * 
 *   format - the format to get the bytes per pixel for
 *   
 * returns 0 if the specified format does not exist
 */
extern int crt_bpp4fmt(int format);

/*****************************************************************************/
/*************************** FIXED POINT SIN/COS *****************************/
/*****************************************************************************/

#define T14_2PI           16384
#define T14_MASK          (T14_2PI - 1)
#define T14_PI            (T14_2PI / 2)

extern void crt_sincos14(int *s, int *c, int n);

#ifdef __cplusplus
}
#endif

#endif
