/*
 LK-201 emulator

 Interfaces between a DEC terminal or workstation that expects an
 LK201-AA keyboard and a standard PS2 connector PC keyboard.

 Copyright (c) 2020-2021, Paul Koning

 Please read file LICENSE for the license that applies to this project.
*/

#define DOIDENT 1   // Set to 0 to suppress power up ident

/*
 Parts of this code were taken from various sample files:

 Polled PS-2 keyboard controller copyright 2009 by Bill Westfield.
 */

#include <Arduino.h>

#include "wiring_private.h"
#include "pins_arduino.h"

// Pin assignment

#define APIN 9              // PWM output, 2 kHz.
#define PS2CLK 12
#define PS2DAT 11
#define WAITLED LED_BUILTIN // pin for Wait LED


// PC 105-key keyboard to LK201 mapping.
//
// Entries in the main lookup table are keyed by the scan code.  The
// value is the LK201 keycode.  Entries in the "divs" table are keyed
// by LK201 keycode, and give the "keyboard division" number which
// selects the mode (up/down, autorepeat, down only) and, if
// applicable, autorepeat parameter buffer.
//
// For the most part these mappings are obvious.  The 6 editing keys
// (above the cursor keys) are mapped according to their location, not
// their labels, so "home" is coded as LK201 "insert here" (rather than
// using the "insert" key for this code).
//
// Similarly, num lock, scroll lock and pause become F14, F15 (Help)
// and F16 (Do).  And the numeric keypad mappings are by layout.  Note
// that there is no PF4 because the PC keypad has one key less than the
// LK201, so the - key is used for - (Cut) instead of PF4 as its
// placement might suggest.
//
// The left Windows key is the < > key.
//
// For convenience, Esc is mapped to F11 which is its conventional
// interpretation.  Both shifts are shift, but the right
// Alt/Win/App/Ctrl keys are F17 to F20 instead.

const uint8_t divs[] =
{
    10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    11, 11, 11, 11, 11, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 12, 12, 12, 0, 0, 0, 0, 0, 0, 0, 13, 13, 0, 0,
    14, 14, 14, 14, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9,
    0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    0, 0, 0, 7, 7, 8, 8, 0, 0, 0, 6, 6, 5, 5, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 3, 4, 4, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,
    1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1
};
// First LK201 code in divs is 86
#define DIVBASE 86

// LK201 division mode codes
#define D 0         // Down only
#define A 1         // Autorepeat
#define U 3         // Up/down

// Default mode settings for the divisions.  Note that divisions are
// numbered starting at 1.
// Defaults are: main, keypad, delete, cursor - autorepeat; return,
// tab, lock, compose: down only; shift, control, editing: down/up.
// Documentation doesn't say what we do for function keys: make those
// down only.
// Division 0 doesn't exist, we use it internally for special keys
// like ALL UP.  Set it to up/down so those are always sent.
const uint8_t defmodes[] =
{ U, A, A, A, D, D, U, A, A, U, D, D, D, D, D };

// Active modes, loaded from defmode at reset but can be modified by
// the host.
uint8_t modes[sizeof (defmodes)];

// Auto-repeat buffers
struct arparams
{
    int timeout;      // Delay before first repeat in ms
    int interval;     // Delay between subsequent repeats in ms
};

#define WPM2MS(x)  (1200 / (x))
#define RATE2DELAY(x) (1000 / (x))
#define ARBUF(tmo,rate) { tmo, RATE2DELAY (rate) }

const arparams defarbufs[4] =
{
    ARBUF (500, 30),
    ARBUF (300, 30),
    ARBUF (500, 40),
    ARBUF (300, 40)
};

// These are the active params, loaded at reset but changeable.
arparams arbufs[4];

// Auto-repeat buffer assignments for divisions.  Documentation
// doesn't state what the default assignment is for divisions that
// don't default to autorepeat; use zero for those.
const uint8_t defar[] =
{ 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 };

// Active auto-repeat buffer assignments, loaded from defar at reset
// but can be modified by the host.
uint8_t ar[sizeof (defar)];

// Current auto-repeat values
unsigned long int arTime;
const arparams *arBuf;
int nextArKey;

// Ident stuff
#define IDUNIT WPM2MS (30)   // 30 wpm
#define T 1
#define H (T * 3)
#define S (-T)
#define LS (-H)

const int8_t errB[] =
{ LS, H, S, T, S, T, S, T, LS, 0 };

const int8_t errP[] =
{ LS, T, S, H, S, H, S, T, LS, 0 };

const int8_t errX[] =
{ LS, T, S, T, S, T, S, T, S, T, S, T, S, T, S, T, LS, 0 };

#if DOIDENT
// "de ni1d"
const int8_t ident[] =
{
    H, S, T, S, T, LS, T, LS, LS, S,
    H, S, T, LS, T, S, T, LS,
    T, S, H, S, H, S, H, S, H, LS,
    H, S, T, S, T, 0 
};
#endif

// Key click and bell parameters
bool clickOff = false;
bool ctrlClick = false;
int clickVol;
bool bellOff = false;
int bellVol;
#define DEFVOL 2    // that means 3rd highest -- the coding is inverted.
#define IDVOL  4    // medium for ID string

// Max would be half of the PWM top value, i.e., 255 (square wave) but
// that produces a distinctly different sounding tone than the
// narrower pulses do.  So set full scale a bit lower to avoid that.
#define FS 200

#define P 0.707
#define E(i, b, pp) ((i & (1 << b)) ? (pp): 1)

// Volume table (analog output value for a given volume).  Note that 0
// is highest.  We scale exponentially since audio perception is
// logarithmic.  The exponent (P) is determined experimentally, by
// looking for something that sounds right.  Sqrt(0.5) seems about
// right.
#define VOL(i) (FS * E(i, 2, P*P*P*P) * E(i, 1, P*P) * E(i, 0, P))
const int vol[8] =
{
  VOL (0), VOL (1), VOL (2), VOL (3),
  VOL (4), VOL (5), VOL (6), VOL (7)
};

// Audio generation control
#define CLICKTIME 5         // 5 ms tone produces a good click
#define BELLTIME  125       // 125 ms tone

void doClick (void)
{
    if (!clickOff)
    {
        analogWrite (APIN, vol[clickVol]);
        delay (CLICKTIME);
        analogWrite (APIN, 0);
    }
}

void doBell (void)
{
    if (!bellOff)
    {
        analogWrite (APIN, vol[bellVol]);
        delay (BELLTIME);
        analogWrite (APIN, 0);
    }
}

// Use the pin 0/1 serial port
#define Serial Serial1

/*
 * ps2kpolled.c
 *
 * Copyright 2009 by Bill Westfield
 *
 * Polled driver for PS2-style keyboards.
 * See http://www.beyondlogic.org/keyboard/keybrd.htm
 */

// Max time in microseconds to wait for the keyboard to say something.
#define MAXPOLL 1000UL

unsigned char ps2k_clk, ps2k_dat;

#define WAITLOW(pin) while (digitalRead(pin) != LOW) ;
#define WAITHIGH(pin) while (digitalRead(pin) != HIGH) ;

/*
 * ps2k_init
 * initialize our interface to a PS2 keyboard.
 *
 * The normal inactive state of the keyboard interface is (a) CLK is
 * output and driven LOW to block keyboard transmissions, (b) DATA is
 * open-collector input and pulled up.
 *
 * Keyboard action functions that need to change any of this are
 * expected to return things to their normal state on exit.
 */
void ps2k_init (unsigned char clock, unsigned char data)
{
    ps2k_clk = clock;
    ps2k_dat = data;
    pinMode(ps2k_clk, OUTPUT);
    digitalWrite(ps2k_clk, LOW);    /* Claim the clock */
    pinMode(ps2k_dat, INPUT_PULLUP);
    digitalWrite(ps2k_dat, HIGH);   /* Enable pullup */
}

/*
 * _ps2k_getcode
 * Read a single keycode from a PS2 keyboard.  This may be a leadin
 * to a multi-symbol key, or an acknowledgement, or a key-release
 * event; we don't care at all.
 *
 * This is only an internal subroutine; note that it does NOT release
 * the clock signal to enable the keyboard to send any data.
 *
 * Interrupts should be disabled on entry; on exit they are still
 * disabled.
 */
static unsigned char _ps2k_getcode(void)
{
    unsigned char keycode, i, parity;
    unsigned long start = micros ();
    unsigned char pin;
    
    // Wait for a Start bit, but not longer than a millisecond.  We do
    // this to avoid getting stuck in this function if the keyboard
    // has nothing to say.
    while (digitalRead (ps2k_clk) == HIGH)
    {
        if (micros () - start > MAXPOLL)
        {
            // time is up, return "no data"
            return 0;
        }
    }
    WAITHIGH(ps2k_clk);         /*  Discard start bit */

    keycode = parity = 0;
    for (i=0; i<10; i++)        /* Read 8 data bits, parity, stop */
    {
        WAITLOW(ps2k_clk);      /*   On each falling edge of clock */
        pin = digitalRead (ps2k_dat);
        if (i < 8)
        {
            keycode >>= 1;      /* LSB is sent first, shift in bits */
            if (pin == HIGH)
            {                   /*   from the MSB of the result */
                keycode |= 0x80;
            }
        }
        if (i < 9 && pin == HIGH)
        {
            parity ^= 1;        /* Accumulate parity */
        }
        WAITHIGH(ps2k_clk);     /* prepare for next clock transition */
    }
    if (parity != 1)
    {
        // Parity error.  Report that to caller with a return value
        // that isn't used as either a scan code or a status code.
        keycode = 0xfb;
        digitalWrite (PS2PE, HIGH);
    }
    return(keycode);
}

/*
 * ps2k_getkey
 * Leaves the keyboard with our host driving the clock signal low, which
 * forces the keyboard to buffer (and not send) additional data.
 */
int ps2k_getkey(void)
{
    int result;

    result = 0;

    noInterrupts ();
    pinMode(ps2k_clk, INPUT_PULLUP);
    digitalWrite(ps2k_clk, HIGH);  /* release ownership; let keybd send */

    result = _ps2k_getcode();      /* Get one byte of keycode */
    pinMode(ps2k_clk, OUTPUT);
    digitalWrite(ps2k_clk, LOW);   /* (stops keyboard from additional sends) */
    interrupts ();

    // To make sure the keyboard sees the clock low, hold it for 200
    // microseconds.  That ensures any transfer started right at the
    // instance we gave up waiting is aborted rather than continuing
    // if another poll happens immediately after exit.
    delayMicroseconds (200);

    return result;
}

/*
 * ps2k_sendbyte
 * Send a byte from the host to the keyboard.
 * This is ... complicated, because even though the host is sending the
 * data, the keyboard does the clocking.  I had all sorts of troubles
 * with terminating the write, apparently solved by the insertion of
 * the "delayMicroseconds()" call after the parity bit.
 *
 * Note that this routine does not process any response codes that the
 * keyboard might send.  They'll show up to ps2k_getkey and will need to
 * be ignorned (or not, as the case may be.)
 */
void ps2k_sendbyte(unsigned char code)
{
    unsigned char parity = 0;       /* Get first byte */
    unsigned char i;

    pinMode(ps2k_dat, OUTPUT);
    digitalWrite(ps2k_dat, LOW);      /* Say we want to send data */

    pinMode(ps2k_clk, INPUT_PULLUP);
    digitalWrite(ps2k_clk, HIGH);

    WAITLOW(ps2k_clk);              /* Send Start bit */

    for (i = 0; i < 8; i++)         /* Send 8 data bits, LSB first */
    {
        if (code & 0x1)
        {
            digitalWrite(ps2k_dat, HIGH);
            parity++;               /* Count one bits to calculate parity */
        }
        else
        {
            digitalWrite(ps2k_dat, LOW);
        }
        WAITHIGH(ps2k_clk);
        WAITLOW(ps2k_clk);
        code >>= 1;                 /* Shift to get next bit ready. */
    }

    if ((parity & 0x1))             /* Send proper parity bit */
    {
        digitalWrite(ps2k_dat, LOW);
    }
    else
    {
        digitalWrite(ps2k_dat, HIGH);
    }
    WAITHIGH(ps2k_clk);
    WAITLOW(ps2k_clk);

    pinMode(ps2k_dat, INPUT_PULLUP); /* Switch back to input / OC mode */
    digitalWrite(ps2k_dat, HIGH);   /* This sends the STOP bit */

    WAITHIGH(ps2k_clk);             /* Wait for end of stop bit time */
    WAITLOW(ps2k_clk);              /* Wait for ACK from keyboard */
    /* TBD: should we look at the ACK signal? */
    WAITHIGH(ps2k_clk);             /* Wait for end of ACK time */
    pinMode(ps2k_clk, OUTPUT);
    digitalWrite(ps2k_clk, LOW);      /*   stop additional transmitting */

    // To make sure the keyboard sees the clock low, hold it for 200
    // microseconds.  That ensures any transfer started just before
    // this point is aborted rather than continuing if another poll
    // happens immediately after exit.
    delayMicroseconds (200);
}

// LK201 command buffer
uint8_t cmd[4];
int cmdcnt;

void send (int k)
{
  Serial.write (k);
}

void panic (char c, const int8_t * code)
{
    send (c);
    send_code (code);
    send (181);
}

void key (int lk, bool down)
{
    const int div = divs[lk - DIVBASE];

    if (down || modes[div] == U)
    {
        send (lk);
    }
    if (down && lk != 174 && (lk != 175 || ctrlClick))
    {
        doClick ();
    }
}

#define BREAK     0x01
#define MODIFIER  0x02
#define MODIFIER2  0x04

const uint8_t ps2_xlat[] =
{
    0,
    103,        // F9
    0,
    90,         // F5
    88,         // F3
    86,         // F1
    87,         // F2
    114,        // F12
    0,
    104,        // F10
    102,        // F8
    100,        // F6
    89,         // F4
    190,        // Tab
    191,        // `
    0,
    0,
    177,        // Left Alt (Compose)
    174,        // Left Shift
    0,
    175,        // Left Ctrl
    193,        // q
    192,        // 1
    0,
    0,
    0,
    195,        // z
    199,        // s
    194,        // a
    198,        // w
    197,        // 2
    0,
    0,
    206,        // c
    200,        // x
    205,        // d
    204,        // e
    208,        // 4
    203,        // 3
    0,
    0,
    212,        // Spacebar
    211,        // v
    210,        // f
    215,        // t
    209,        // r
    214,        // 5
    0,
    0,
    222,        // n
    217,        // b
    221,        // h
    216,        // g
    220,        // y
    219,        // 6
    0,
    0,
    0,
    227,        // m
    226,        // j
    225,        // u
    224,        // 7
    229,        // 8
    0,
    0,
    232,        // ,
    231,        // k
    230,        // i
    235,        // o
    239,        // 0
    234,        // 9
    0,
    0,
    237,        // .
    243,        // /
    236,        // l
    242,        // ;
    240,        // p
    249,        // -
    0,
    0,
    0,
    251,        // '
    0,
    250,        // [
    245,        // =
    0,
    0,
    176,        // Caps Lock
    174,        // Right Shift
    189,        // Enter
    246,        // ]
    0,
    247,        // backslash
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    188,        // Backspace
    0,
    0,
    150,        // Keypad 1 / End
    0,
    153,        // Keypad 4 / Left
    157,        // Keypad 7 / Home
    0,
    0,
    0,
    146,        // Keypad 0 / Ins
    148,        // Keypad . / Del
    151,        // Keypad 2 / Down
    154,        // Keypad 5
    155,        // Keypad 6 / Right
    158,        // Keypad 8 / Up
    113,        // Esc
    161,        // Num Lock (PF1)
    113,        // F11
    156,        // Keypad + (num comma)
    152,        // Keypad 3 / PgDn
    160,        // Keypad -
    163,        // Keypad * (PF3)
    159,        // Keypad 9 / PgUp
    124,        // Scroll Lock (Help)
    0,
    0,
    0,
    0,
    101,        // F7
};

// Key state is tracked by a double-linked list which gives keys
// currently down in time order.  There is a tail pointer (lastkey)
// but no head pointer, we have no need for that.  Each pointer is a
// key code, or 0 for first/last.  Keys currently up have next and
// prev both set to zero.  To tell the difference between key up and
// only this key down, look in lastkey; if an entry has 0 pointers but
// lastkey points to it, that key is down; otherwise it is up.
struct KeyState 
{
    uint8_t next;
    uint8_t prev;
};

KeyState vt220_keystate[256 - DIVBASE];
uint8_t lastkey;

// This function is invoked on each key stroke.  It resets autorepeat
// to act on the most recent key that has auto-repeat mode set, if
// any.
void update_ar (bool down)
{
    int lk = lastkey;
    int div;
    
    while (lk)
    {
        // Look for autorepeat key
        div = divs[lk - DIVBASE];
        if (modes[div] == A)
        {
            // We found the newest key that autorepeats
            break;
        }
        lk = vt220_keystate[lk - DIVBASE].prev;
    }
    if (lk == 0)
    {
        // Nothing autorepeats right now
        arTime = 0;
    }
    else
    {
        arBuf = &arbufs[ar[div]];
        arTime = millis () + arBuf->timeout;
        if (down && lk == lastkey)
        {
            // Key down and that key is the one autorepeating, so it
            // was just sent; we'll want metronome next.
            nextArKey = 180;
        }
        else
        {
            // Key up, or the autorepeating key isn't the one that was
            // just pressed, so send the autorepeating key next.
            nextArKey = lk;
        }
    }
}

// Convert PS2 scan code to LK201 key number.  Taken from ps2.c by
// Peter Bosch (peterbjornx).
int ps2_dataproc (unsigned int s)
{
    static uint8_t mods = 0;
    int c;

    s &= 0xff;
    //send (s);
    
    if (s == 0xF0) {
        mods |= BREAK;
        return 0;
    } else if (s == 0xE0) {
        mods |= MODIFIER;
        return 0;
    } else if (s == 0xE1) {
        mods |= MODIFIER2;
        return 0;
    }
    
    c = 0;
    if (mods & MODIFIER)
    {
        // Note: modifier E0 12 is prefixed onto Print Screen (always)
        // and onto a bunch of other keys like Windows and Right Ctrl
        // if NumLock is in effect (if that key has been pressed an
        // odd number of times).  So that code is ignored.
        switch (s)
        {
        case 0x11: c = 128; break;  // Right Alt (F17)
        case 0x14: c = 131; break;  // Right control (F20)
        case 0x1f: c = 201; break;  // Left Windows (< >)
        case 0x27: c = 129; break;  // Right Windows (F18)
        case 0x2f: c = 130; break;  // Applic (F19)
        case 0x4a: c = 162; break;  // Keypad / (PF2)
        case 0x5a: c = 149; break;  // Keypad Enter
        case 0x69: c = 142; break;  // End (Prev Screen)
        case 0x6b: c = 167; break;  // Left
        case 0x6c: c = 139; break;  // Home (Insert Here)
        case 0x70: c = 138; break;  // Insert (Find)
        case 0x71: c = 141; break;  // Delete (Select)
        case 0x72: c = 169; break;  // Down
        case 0x74: c = 168; break;  // Right
        case 0x75: c = 170; break;  // Up
        case 0x7a: c = 143; break;  // PgDn (Next Screen)
        case 0x7c: c = 116; break;  // PrtScr (F14)
        case 0x7d: c = 140; break;  // PgUp (Remove)
        case 0x7e: c = 125; break;  // Pause (Do) when Ctrl is active
        default:  break;
        }
    }
    else if (mods & MODIFIER2)
    {
        switch (s)
        {
            // The Pause key is a truly crazy case. It sends its "key
            // up" sequence immediately after the "down" sequence.
            // Second, it sends a sequence of two codes after the e1
            // modifier (but the modifier is only sent once).
            // Finally, if Control is down, it sends a completely
            // different sequence (e0 e7).  To handle the case of the
            // two-code sequence, when we see the first of the two we
            // ignore that but leave the modifiers set, so the second
            // code is also handled in this section of the code.
        case 0x14: return 0;        // Pause, first code
        case 0x77: c = 125; break;  // Pause (Do)
        default: break;
        }
    } else if (s < sizeof (ps2_xlat))
    {
        c = ps2_xlat[s];
    }

    if ((mods & BREAK) && (c != 0))
    {
        c |= 0x400;
    }
    mods = 0;
    
    return c;
}

void kserror (void)
{
    // Clear the key state data and send "all up"
    memset (vt220_keystate, 0, sizeof (vt220_keystate));
    lastkey = 0;
    arTime = 0;
    send (179);
    //send_code (errB);
}

void kbpoll (void)
{
    bool down;
    int i, j, scan;
    uint8_t p, c;
    KeyState *ks;
    
    scan = ps2k_getkey ();

    if (scan == 0)
    {
        return;
    }
    
    if (scan == 0xaa)
    {
        // self test OK -- TODO
        return;
    }
    else if (scan == 0xfa)
    {
        // ack -- TODO
        return;
    }
    else if (scan == 0xfb)
    {
        // Parity error
        send_code (errP);
        return;
    }
    else if (scan >= 0xfc && scan <= 0xff)
    {
        // Some sort of error, warn
        send_code (errX);
        return;
    }

    scan = ps2_dataproc (scan);
    if (scan == 0)
        return;
    
    ks = &vt220_keystate[(scan & 0xff) - DIVBASE];
    if (scan & 0x400)
    {
        // Key up
        down = false;
        scan &= 0xff;
        if (lastkey == scan)
        {
            // Newest key changed to "up".  Make its predecessor "last".
            lastkey = ks->prev;
            if (lastkey)
            {
                // Still some keys left.  Send key code if up/down.
                vt220_keystate[lastkey - DIVBASE].next = 0;
                key (scan, false);
            }
            else
            {
                // Nothing down now, send "all up" if this key is an
                // up/down key.
                if (modes[divs[scan - DIVBASE]] == U)
                {
                    send (179);
                }
            }
        }
        else
        {
            // Not most recent key
            if (ks->prev)
            {
                vt220_keystate[ks->prev - DIVBASE].next = ks->next;
            }
            if (ks->next)
            {
                vt220_keystate[ks->next - DIVBASE].prev = ks->prev;
            }
            else
            {
                // State error
                kserror ();
                return;
            }
            // Send key code if up/down mode.
            key (scan, false);
        }
        // In any case, mark this key as not down
        ks->prev = ks->next = 0;
    }
    else
    {
        // Key down
        if (scan == lastkey || ks->next != 0)
        {
            // Duplicate "down", that's the keyboard doing autorepeat.
            // Ignore it.
            return;
        }
        down = true;
        // New down, make this key the most recent key
        if (lastkey)
        {
            vt220_keystate[lastkey - DIVBASE].next = scan;
        }
        ks->prev = lastkey;
        ks->next = 0;
        lastkey = scan;
        key (scan, true);
    }
    // In every case other than the keyboard autorepeating, we have a
    // keyboard state change so we update our autorepeat state.
    update_ar (down);
}

// Ident sender
void send_code (const int8_t *idp)
{
    long int t;
    
    while (*idp != 0)
    {
        t = *idp++ * IDUNIT;
        if (t > 0)
        {
            // Send tone
            analogWrite (APIN, vol[IDVOL]);
            delay (t);
            analogWrite (APIN, 0);
        }
        else
        {
            delay (-t);
        }
    }
}

uint8_t curleds;

#define WAIT   1                // Wait LED (keyboard locked in RSTS)
#define COMPOS 2                // Compose mode
#define CAPS   4                // Caps/shift lock
#define HOLD   8                // Hold Screen

#define PS2_COMPOS 2            // Num Lock
#define PS2_CAPS   4            // Caps Lock
#define PS2_HOLD   1            // Scroll lock

void changeLeds (uint8_t mask, bool on)
{
    uint8_t ps2Leds;
  
    mask &= 0x0f;
    if (on)
        curleds |= mask;
    else
        curleds &= ~mask;
  
    // Map from LK-201 LEDs to PS2 ones.
    ps2Leds = 0;
    if (curleds & COMPOS) ps2Leds |= PS2_COMPOS;
    if (curleds & CAPS)   ps2Leds |= PS2_CAPS;
    if (curleds & HOLD)   ps2Leds |= PS2_HOLD;
  
    ps2k_sendbyte (0xed);
    // Get the command ack (yes, before sending the data byte that goes
    // with the command!)
    ps2k_getkey ();
    ps2k_sendbyte (ps2Leds);
    if (curleds & WAIT)
    {
        digitalWrite (WAITLED, HIGH);
    }
    else
    {
        digitalWrite (WAITLED, LOW);
    }
    // Get the second ack
    ps2k_getkey ();
}

void resetkb ()
{
    int scan;
    
    // Keyboard reset code
    memcpy (modes, defmodes, sizeof (modes));
    memcpy (ar, defar, sizeof (ar));
    memcpy (arbufs, defarbufs, sizeof (arbufs));

    // Reset current state
    memset (vt220_keystate, 0, sizeof (vt220_keystate));
    lastkey = 0;
    cmdcnt = 0;

    clickOff = ctrlClick = bellOff = false;
    clickVol = bellVol = DEFVOL;
    arTime = 0;

    // drain any pending data from keyboard
    do
    {
        scan = ps2k_getkey ();
    } while (scan);
    
    // Flash the LEDs
    changeLeds (0x0f, true);
    delay (100);
    changeLeds (0x0f, false);

    // Send reply
    send (1);
    send (0);
    send (0);
    send (0);
}

void setup()
{
    Serial.begin (4800);

    // Set up the WAIT LED pin
    pinMode (WAITLED, OUTPUT);
    digitalWrite (WAITLED, LOW);
  
    // Initialize PS2 interface
    ps2k_init (PS2CLK, PS2DAT);

    // Modify the timer1 settings to produce a 2 kHz PWM waveform,
    // which is the resonant frequency of the piezo transducer used.
    // Clock prescale 8
	cbi(TCCR1B, CS10);
    // 9 bit mode
	cbi(TCCR1A, WGM10);
	sbi(TCCR1A, WGM11);
    
    // Set up tone output
    pinMode (APIN, OUTPUT);
    analogWrite (APIN, 0);

#if DOIDENT
    // Send our identification
    send_code (ident);
#endif
    delay (20);
    resetkb ();
}

void serialEvent()
{
    while (Serial.available())
    {
        // get the new byte:
        uint8_t inChar = (uint8_t)Serial.read();
        if (inChar == 0xfd)
        {
            // Power up reset is recognized immediately
            resetkb ();
            return;
        }
    
        if (cmdcnt < sizeof (cmd))
        {
            cmd[cmdcnt++] = inChar;
        }
        // If not end of parameters, keep collecting
        if ((inChar & 0x80) == 0)
            continue;

        const uint8_t op = cmd[0];
    
        if ((op & 1) == 0)
        {
            // Mode or auto-repeat parameters
            const int div = (op >> 3) & 0x0f;
            const int modnum = (op >> 1) & 0x03;
      
            if (div == 0x0f)
            {
                // Auto-repeat params.  "modnum" is the buffer number
                if (cmdcnt > 2)
                {
                    arbufs[modnum].timeout = (cmd[1] & 0x7f) * 5;
                    int rate = cmd[2] & 0x7f;
                    if (rate < 12)
                        rate = 12;
                    arbufs[modnum].interval = RATE2DELAY (rate);
                    update_ar (false);
                }
            }
            else
            {
                // Set mode
                if (div > 0 && div < sizeof (modes) && modnum != 2)
                {
                    modes[div] = modnum;
                    // Do AR buffer change if requested
                    if (cmdcnt > 1)
                    {
                        ar[div] = cmd[1] & 0x03;
                        update_ar (false);
                    }
                }
                // Ack the mode change
                send (186);
            }
        }
        else
        {
            switch (op)
            {
            case 0xab:
                // Request keyboard ID
                send (0);
                send (0);
                break;
            case 0x11:
                // LEDs off
                changeLeds (cmd[1], false);
                break;
            case 0x13:
                // LEDs on
                changeLeds (cmd[1], true);
                break;
            case 0x99:
                // Disable key click
                clickOff = true;
                break;
            case 0x1b:
                // Enable key click, set volume
                clickOff = false;
                clickVol = cmd[1] & 7;
                break;
            case 0xb9:
                // Disable click for CTRL
                ctrlClick = false;
                break;
            case 0xbb:
                // Enable click for CTRL
                ctrlClick = true;
                break;
            case 0x9f:
                // sound click
                doClick ();
                break;
            case 0xA1:
                // Disable bell
                bellOff = true;
                break;
            case 0x23:
                // Enable bell, set volume
                bellOff = false;
                bellVol = cmd[1] & 7;
                break;
            case 0xa7:
                // Sound bell
                doBell ();
                break;
            case 0xc1:
                // Inhibit autorepeat, current key only
                arTime = 0;
                break;
            }
        }
    
        // Reset command buffer
        cmdcnt = 0;
    }
}

void loop()
{
    // Check for input from host
    serialEvent();

    // Check for keyboard data
    kbpoll ();

    // Check for autorepeat to be done
    if (arTime && millis () >= arTime)
    {
        // Yes, and it just expired
        send (nextArKey);
        nextArKey = 180;        // Next time send metronome code
        doClick ();
        arTime = millis () + arBuf->interval;
    }
}
