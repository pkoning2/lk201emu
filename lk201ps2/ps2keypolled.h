/*
 * ps2keypolled.h
 * Copyright 2009 by Bill Westfield
 *
 * defines for polled interface to PS2-like keyboards.
 */

#ifdef __cplusplus
extern "C"{
#endif
    extern unsigned char ps2k_getcode(void);
    extern int ps2k_getkey(void);
    extern void ps2k_sendbyte(unsigned char);
    extern unsigned char ps2k_translate(int);
    extern void ps2k_init (unsigned char clock, unsigned char data);
#ifdef __cplusplus
} // extern "C"
#endif

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif


/*
 * Value to add to "extended" keycodes.  Can be 0x80 to fit in a singlpe
 * byte, which ALMOST never conflicts with anything.  Or it can be 100.
 */
#define PS2K_EXTEND 0x100

#define PS2K_NOKEY 0xFF

extern unsigned char ps2k_clk, ps2k_dat;

