# Digital Equipment LK201 keyboard emulator

The emulator allows a standard PC-style USB keyboard to be used as a replacement for a DEC LK201 keyboard.  This is useful if you don't have one, or if all the ones you have stopped working, as happened to me.

The emulator circuit has two connectors: a modular jack which accepts the LK201 keyboard cable, and a USB connector.  The USB connector on the Trinket board is a micro-B connector; your keyboard presumably needs an A jack.  There are various ways to convert from one to the other; [Adafruit](https://www.adafruit.com/product/1099 "Micro B to A cable") sells one.

## Keyboard mapping

For the most part, the keys on the PC keyboard correspond directly to the same keys on the LK201.  This applies to all the letters and digits as well as most punctuation.  Similarly, F1 through F12 are the same in both, as are the cursor keys and most of the numeric keypad keys.  Both shift keys are shift, but only the left control key is control.

Some other keys are different on the two keyboards.  Sometimes similar key labels exist but the positions are different; in that case I have chosen to map the keys by their positions.

| PC keyboard key | LK201 key |
| --- | --- |
| Left Windows | < > |
| Left Alt | Compose |
| Esc  | F11 |
| Insert | Find |
| Home | Insert Here |
| PgUp | Remove |
| Delete | Select |
| End | Prev Screen |
| PgDn | Next Screen  |
| PrtScr | F14 |
| Scroll Lock | Help (F15) |
| Pause | Do (F16) |
| Right Alt | F17 | 
| Right Windows | F18 |
| App Menu | F19 |
| Right Control | F20 |
| Num Lock | PF1 | 
| Num keypad / | PF2 |
| Num keypad * | PF3 |

Note that the PF4 and F13 keys are not available, I ran out of places to put them.

## Lights

Standard PC keyboards have only three LEDs, while the LK201 has four.  The mappings are:

| PC keyboard LED | LK201 keyboard LED |
| --- | --- |
| Scroll lock | Hold screen |
| Caps lock | Lock |
| Num lock | Compose |

The WAIT light is not on the keyboard but is provided by the LED on the center of the Trinket board (the multi-color one), in green.

## Trinket middle LED color coding

When operating, the emulator uses the middle LED for the LK201 WAIT light, in green.  Other colors are used during initialization, to reflect the phases of the USB discovery.
* Purple: settling.
* Blue: waiting for device.  This is the code you will see when no keyboard is plugged in.
* Yellow: attached, reset device.
* Orange: wait for device reset complete.
* Cyan (light blue): configuring device.  If the board stays in this state, the USB device is not compatible with this software.

When the USB initialization completes, there is a brief bright green flash.

