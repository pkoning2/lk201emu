/*
 LK-201 emulator

 Interfaces between a DEC terminal or workstation that expects an
 LK201-AA keyboard and a standard PC type USB keyboard.

 Copyright (c) 2020, Paul Koning

 Please read file LICENSE for the license that applies to this project.
*/

#define DOIDENT 1   // Set to 0 to suppress power up ident

/*
 Parts of this code were taken from various sample files:

 KeyboardController tutorial
 created 8 Oct 2012
 by Cristian Maglie

 http://arduino.cc/en/Tutorial/KeyboardController
 This sample code is part of the public domain.

 3-color LED example from
 https://hackaday.io/project/9017-skywalker-lightsaber/log/72354-adafruit-trinket-m0-controlling-the-onboard-dotstar-led-via-arduino-ide

 ZeroTimer library example from
 https://github.com/adafruit/Adafruit_ZeroTimer/tree/master/examples/timer_callback
 */

#include <Arduino.h>
#include "Adafruit_ZeroTimer.h"

// Require keyboard control library
#include <KeyboardController.h>

#include <Adafruit_DotStar.h>
#include <SPI.h>

#define NUMPIXELS 1 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN   7
#define CLOCKPIN   8

Adafruit_DotStar strip = Adafruit_DotStar(
  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// PC 105-key keyboard (USB HID Boot mode codes) to LK201 mapping.
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
  12, 12, 0, 12, 0, 0, 0, 0, 0, 0, 0, 13, 13, 0, 0,
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

// Main lookup table, indexed by report scan code
const uint8_t usb_lk201[] =
{
  0,               // No keys
  0,               // Rollover overflow
  0,               // error
  0,               // reserved
  194,             // A
  217,             // B
  206,             // C
  205,             // D
  204,             // E
  210,             // F
  216,             // G
  221,             // H
  230,             // I
  226,             // J
  231,             // K
  236,             // L
  227,             // M
  222,             // N
  235,             // O
  240,             // P
  193,             // Q
  209,             // R
  199,             // S
  215,             // T
  225,             // U
  211,             // V
  198,             // W
  200,             // X
  220,             // Y
  195,             // Z
  192,             // 1
  197,             // 2
  203,             // 3
  208,             // 4
  214,             // 5
  219,             // 6
  224,             // 7
  229,             // 8
  234,             // 9
  239,             // 0
  189,             // Enter
  113,             // Esc (F11)
  188,             // Backspace
  190,             // Tab
  212,             // Space
  249,             // - / _
  245,             // = / +
  250,             // [ / {
  246,             // ] / }
  247,             // \ / |
  0,               // unused code
  242,             // ; / :
  251,             // ' / "
  191,             // ` / ~
  232,             // , / <
  237,             // . / >
  243,             // / / ?
  176,             // Caps Lock
  86,              // F1
  87,              // F2
  88,              // F3
  89,              // F4
  90,              // F5
  100,             // F6
  101,             // F7
  102,             // F8
  104,             // F9
  104,             // F10
  113,             // F11
  114,             // F12
  116,             // PrtScr (F14)
  124,             // Scroll Lock (Help)
  125,             // Pause (Do)
  138,             // Insert (Find)
  139,             // Home (Insert here)
  140,             // PgUp (Remove)
  141,             // Delete (Select)
  142,             // End (Prev Screen)
  143,             // PgDn (Next Screen)
  168,             // Right
  167,             // Left
  169,             // Down
  170,             // Up
  161,             // Num Lock (PF1)
  162,             // KP / (PF2)
  163,             // KP * (PF3)
  160,             // KP -
  156,             // KP + (num comma)
  149,             // KP Enter
  150,             // KP 1 / End
  151,             // KP 2 / Down
  152,             // KP 3 / PgDn
  153,             // KP 4 / Left
  154,             // KP 5
  155,             // KP 6 / Right
  157,             // KP 7 / Home
  158,             // KP 8 / Up
  159,             // KP 9 / PgUp
  146,             // KP 0 / Ins
  148,             // KP . / Del
  0,               // unused code
  130              // Applic (F19)
};

#define MAXSCAN sizeof (usb_lk201)

// Modifier lookup table, indexed by bit number
const uint8_t usbmod_lk201[] =
{
  175,             // Left control
  174,             // Shift
  177,             // Left Alt (Compose)
  201,             // Left Windows  (< >)
  131,             // Right control (F20)
  0,               // Unused (right shift)
  128,             // Right Alt (F17)
  129              // Right Windows (F18)
};

// Modifier bit values
#define LCTRL  0x01
#define LSHIFT 0x02
#define LALT   0x04
#define LWIN   0x08
#define RCTRL  0x10
#define RSHIFT 0x20
#define RALT   0x40
#define RWIN   0x80

// Modes
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
  int timeout;      // Delay before first repeat
  int interval;     // Delay between subsequent repeats
};

// Timer interrupt interval is 0.25 ms, to give 2 kHz square wave
// (which is the resonant frequency of the piezo beeper used).
#define INTRATE 4000

#define MS2DELAY(x)   ((x) * INTRATE / 1000)
#define WPM2DELAY(x)  MS2DELAY (1200 / (x))
#define RATE2DELAY(x) (INTRATE / (x))
#define ARBUF(tmo,rate) { MS2DELAY (tmo), RATE2DELAY (rate) }

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
int arDelay;
const arparams *arBuf;
int nextArKey;

// Ident stuff
#define IDUNIT WPM2DELAY (30)   // 30 wpm
#define T 1
#define H (T * 3)
#define S (-T)
#define LS (-H)

const int8_t *idp;

const int8_t errA[] =
{ LS, T, S, H, LS, 0 };

const int8_t errB[] =
{ LS, H, S, T, S, T, S, T, LS, 0 };

#if DOIDENT
const int8_t ident[] =
{ H, S, T, S, T, LS, T, LS, LS, S,
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
#define DEFVOL 2

// Analog output (for beeper) -- use the real DAC since PWM doesn't
// work well with piezo beepers.
#define APIN A0

// Analog output goes through an amplifier, so we have to map values
// to account for that.
#define HIGH 1023     // full scale value
#define BIAS (HIGH / 6) // 0.5 V adjustment for diode drop
#define FS (HIGH - BIAS)
#define AOUT(x) (FS - (x))
#define AOFF HIGH

#define P 0.707
#define E(i, b, pp) ((i & (1 << b)) ? (pp): 1)

// Volume table (analog output value for a given volume).  Note that 0
// is highest.  We scale exponentially since audio perception is
// logarithmic.  The exponent (P) is determined experimentally, by
// looking for something that sounds right.  Sqrt(0.5) seems about
// right.
#define VOL(i) AOUT (FS * E(i, 2, P*P*P*P) * E(i, 1, P*P) * E(i, 0, P))
const int vol[8] =
{
  VOL (0), VOL (1), VOL (2), VOL (3),
  VOL (4), VOL (5), VOL (6), VOL (7)
};

// Audio generation control
#define CLICKTIME 10            // 5 cycles of the tone
#define BELLTIME  MS2DELAY(125) // 125 ms tone
int audioCounter;
int curVol;

void doClick (void)
{
  if (!clickOff)
  {
    audioCounter = CLICKTIME;
    curVol = vol[clickVol];
  }
}

void doBell (void)
{
  if (!bellOff)
  {
    audioCounter = BELLTIME;
    curVol = vol[bellVol];
  }
}

// On SAMD boards where the native USB port is also the serial console, use
// Serial1 for the serial console. This applies to all SAMD boards except for
// Arduino Zero and M0 boards.
#if (USB_VID==0x2341 && defined(ARDUINO_SAMD_ZERO)) || (USB_VID==0x2a03 && defined(ARDUINO_SAM_ZERO))
#define Serial SERIAL_PORT_MONITOR
#else
#define Serial Serial1
#endif

HID *kb_hid;

// timer tester
Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}

class KeyboardRaw : public KeyboardReportParser {
public:
  KeyboardRaw(USBHost &usb) : hostKeyboard(&usb) {
    hostKeyboard.SetReportParser(0, this);
  };

  void Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf);

private:
  HIDBoot<HID_PROTOCOL_KEYBOARD> hostKeyboard;
};

// USB scan state
uint8_t prevmod;
uint8_t prevscans[7];
uint8_t curmod;
uint8_t curscans[7];

// LK201 key down stack.  This tracks all the LK201 keys that are
// down, except for Up/Down mode keys (modifiers like shift).  The
// purpose is to handle autorepeat when keys are pressed and released;
// the rule is that the most recent key that is still down is
// autorepeated if applicable to that key.
uint8_t lkdown[14];
int downcnt;

// LK201 command buffer
uint8_t cmd[4];
int cmdcnt;

void send (int k)
{
  Serial.write (k);
}

void panic (char c, const int8_t * code)
{
  strip.setPixelColor(0, 0x280000); // red
  strip.show();
  send (c);
  idp = code;
  audioCounter = 1;
  send (181);
}

void setar (int lk, bool sendlk = false)
{
  const int div = divs[lk - DIVBASE];
  if (modes[div] == A)
  {
    arBuf = &arbufs[ar[div]];
    arDelay = arBuf->timeout;
    if (sendlk)
      nextArKey = lk;        // First time, send the actual key
    else
      nextArKey = 180;       // Metronome code
  }
}

void key(int lk, bool down)
{
  const int div = divs[lk - DIVBASE];

  // First of all, cancel any current auto-repeat on every new
  // keystroke.
  arDelay = 0;

  // If this isn't an up/down key, adjust the list of currently down
  // keys -- this is what allows auto-repeat to work properly when
  // several repeating keys are pressed and released.
  if (modes[div] != U)
  {
    if (down)
    {
      // Key down, push it onto the "down" stack
      if (downcnt >= sizeof (lkdown))
      {
        // No room, something is wrong
        panic ('A', errA);
        return;
      }
      if (downcnt)
        memmove (lkdown + 1, lkdown, downcnt);
      downcnt++;
      lkdown[0] = lk;
    }
    else
    {
      // Key up, remove it from the list
      int i;
      for (i = 0; i < downcnt; i++)
        if (lkdown[i] == lk)
          break;
      if (i == downcnt)
      {
        // Not there, bug, just exit
        panic ('B', errB);
        return;
      }
      if (i != downcnt - 1)
        memmove (lkdown + i, lkdown + i + 1, downcnt - i - 1);
      downcnt--;

      // If anything is left, and the last entry (most recent key
      // that's still down) is up/down, start it autorepeating.
      if (downcnt)
        setar (lkdown[0], true);
      return;
    }
  }
  
  send (lk);
  if (down && lk != 174 && (lk != 175 || ctrlClick))
  {
    doClick ();
  }
  // If this key is an auto-repeating one, set up the initial delay
  setar (lk);
}

void KeyboardRaw::Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf)
{
  bool down;
  int i, j;
  uint8_t p, c;

  // Save HID pointer
  kb_hid = hid;
  
  if (buf[2] == 1)
    return;   // Error case

  curmod = buf[0];
  memcpy (curscans, &buf[2], 6);
  if (buf[0] == 0 && buf[2] == 0)
  {
    // No keys down, so send "all up"
    send (179);
    // No more autorepeat, nothing down
    arDelay = downcnt = 0;
  }
  else
  {
    if (curmod & RSHIFT)
    {
      curmod = (curmod | LSHIFT) & (~RSHIFT);
    }
    int cmod = curmod ^ prevmod;
    for (int i = 0; i < 8; i++)
    {
      if (cmod & (1 << i))
      {
        down = ((curmod & (1 << i)) != 0);
        key (usbmod_lk201[i], down);
      }
    }
    i = j = 0;
    while (i < 7 && j < 7)
    {
      p = prevscans[j];
      c = curscans[i];
      if (p != c)
      {
        if (p == 0)
        {
          // New key, so send "down"
          if (c < MAXSCAN)
          {
            // New key down, so send "down"
            key (usb_lk201[c], true);
          }
          i++;
        }
        else
        {
          if (p < MAXSCAN)
          {
            // Previous key down, so send "up"
            key (usb_lk201[p], false);
          }
          j++;
        }
      }
      else
      {
        // Matching keys, advance both
        i++;
        j++;
      }
    }
  }
  // Save current into previous
  prevmod = curmod;
  memcpy (prevscans, curscans, 6);
}

// Timer callback handler
void TimerCallback0(void)
{
  if (audioCounter)
  {
    if (--audioCounter & 1)
    {
      // turn analog output on
      analogWrite (APIN, curVol);
    }
    else
    {
      int id;
      analogWrite (APIN, AOFF);
#if DOIDENT
      if (audioCounter == 0 && (id = *idp) != 0)
      {
        ++idp;
        id *= IDUNIT;
        if (id > 0)
        {
          curVol = VOL (4);
          audioCounter = id;
        }
        else
        {
          curVol = AOFF;
          audioCounter = -id;
        }
      }
#endif
    }
  }
  // autorepeat
  if (arDelay && --arDelay == 0)
  {
    // Yes, and it just expired
    send (nextArKey);
    nextArKey = 180;        // Next time send metronome code
    arDelay = arBuf->interval;
  }
}

void setupTimer (int freq)
{
  // Set up the flexible divider/compare
  uint8_t divider  = 1;
  uint16_t compare = 0;
  tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

  if ((freq < 24000000) && (freq > 800)) {
    divider = 1;
    prescaler = TC_CLOCK_PRESCALER_DIV1;
    compare = 48000000/freq;
  } else if (freq > 400) {
    divider = 2;
    prescaler = TC_CLOCK_PRESCALER_DIV2;
    compare = (48000000/2)/freq;
  } else if (freq > 200) {
    divider = 4;
    prescaler = TC_CLOCK_PRESCALER_DIV4;
    compare = (48000000/4)/freq;
  } else if (freq > 100) {
    divider = 8;
    prescaler = TC_CLOCK_PRESCALER_DIV8;
    compare = (48000000/8)/freq;
  } else if (freq > 50) {
    divider = 16;
    prescaler = TC_CLOCK_PRESCALER_DIV16;
    compare = (48000000/16)/freq;
  } else if (freq > 12) {
    divider = 64;
    prescaler = TC_CLOCK_PRESCALER_DIV64;
    compare = (48000000/64)/freq;
  } else if (freq > 3) {
    divider = 256;
    prescaler = TC_CLOCK_PRESCALER_DIV256;
    compare = (48000000/256)/freq;
  } else if (freq >= 0.75) {
    divider = 1024;
    prescaler = TC_CLOCK_PRESCALER_DIV1024;
    compare = (48000000/1024)/freq;
  } else {
    //Serial.println("Invalid frequency");
    while (1) delay(10);
  }
  
  zerotimer.enable(false);
  zerotimer.configure(prescaler,       // prescaler
                      TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
                      TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
    );

  zerotimer.setCompare(0, compare);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, TimerCallback0);
  zerotimer.enable(true);
}


// Initialize USB Controller
USBHost usb;

// Attach keyboard controller to USB
//KeyboardController keyboard(usb);
KeyboardRaw keyboard(usb);

uint32_t lastUSBstate = 0;
uint8_t curleds;

#define WAIT   1                // Wait LED (keyboard locked in RSTS)
#define COMPOS 2                // Compose mode
#define CAPS   4                // Caps/shift lock
#define HOLD   8                // Hold Screen

#define USB_COMPOS 1            // Num Lock
#define USB_CAPS   2            // Caps Lock
#define USB_HOLD   4            // Scroll lock

void changeLeds (uint8_t mask, bool on)
{
  uint8_t usbLeds;
  
  mask &= 0x0f;
  if (on)
    curleds |= mask;
  else
    curleds &= ~mask;
  
  // Send command, if we know how and if USB is up and running.
  if (kb_hid != NULL && usb.getUsbTaskState() == USB_STATE_RUNNING)
  {
    // The mapping from LK-201 LEDs to USB ones is simply a right
    // shift by one.
    usbLeds = curleds >> 1;
    kb_hid->SetReport (0, 0, 2, 0, 1, &usbLeds);
    if (curleds & 1)
    {
      // Wait LED -- use the 3-color LED on the Trinket
      strip.setPixelColor(0, 0x002800); // green
      strip.show();
    }
    else
    {
      strip.setPixelColor(0, 0x000000); // off
      strip.show();
    }
    
  }
}

void resetkb ()
{
  // Keyboard reset code
  memcpy (modes, defmodes, sizeof (modes));
  memcpy (ar, defar, sizeof (ar));
  memcpy (arbufs, defarbufs, sizeof (arbufs));

  // Reset current state
  prevmod = 0;
  memset (prevscans, 0, sizeof (prevscans));
  downcnt = cmdcnt = 0;

  clickOff = ctrlClick = bellOff = false;
  clickVol = bellVol = DEFVOL;
  arDelay = 0;
  
  // Send reply
  send (1);
  send (0);
  send (0);
  send (0);
  // Flash the LEDs
  changeLeds (0x0f, true);
  delay (100);
  changeLeds (0x0f, false);
}

void setup()
{
  Serial.begin( 4800 );

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  setupTimer (INTRATE);

  analogWrite (APIN, AOFF);
#if DOIDENT
  idp = ident;
  audioCounter = 1;
#else
  audioCounter = 0;
#endif

  usb.Init();
    
  delay( 20 );
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
          arbufs[modnum].timeout = MS2DELAY ((cmd[1] & 0x7f) * 5);
          int rate = cmd[2] & 0x7f;
          if (rate < 12)
            rate = 12;
          arbufs[modnum].interval = RATE2DELAY (rate);
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
        arDelay = 0;
        break;
      }
    }
    
    // Reset command buffer
    cmdcnt = 0;
  }
}


void loop()
{
    // Check for input
    serialEvent();
    
  // Process USB tasks
  usb.Task();

  uint32_t currentUSBstate = usb.getUsbTaskState();
  if (lastUSBstate != currentUSBstate)
  {
    switch (currentUSBstate)
    {
    case USB_ATTACHED_SUBSTATE_SETTLE:
      strip.setPixelColor(0, 0x280028); // purple
      strip.show();
      break;
    case USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE:
      strip.setPixelColor(0, 0x000028); // blue
      strip.show();
      break;
    case USB_ATTACHED_SUBSTATE_RESET_DEVICE:
      strip.setPixelColor(0, 0x282800); // yellow
      strip.show();
      break;
    case USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE:
      strip.setPixelColor(0, 0x281400); // orange
      strip.show();
      break;
    case USB_STATE_CONFIGURING:
      strip.setPixelColor(0, 0x002828); // cyan
      strip.show();
      break;
    case USB_STATE_RUNNING:
      strip.setPixelColor(0, 0x005500); // green, bright
      strip.show();
      resetkb ();
      strip.setPixelColor(0, 0x000000); // off
      strip.show();
      break;
    }
    lastUSBstate = currentUSBstate;
  }
}
