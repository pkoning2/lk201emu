# Digital Equipment LK201 keyboard emulators

The `kicad` subdirectory contains two subdirectories, `lk201` for the USB emulator and `lk201ps2` for the PS2 version.  In both you will find the design files for the emulator circuit board, which you can inspect or modify using the open source KiCAD package.

## Creating the circuit board

There are three ways to get a circuit board.
1. From the shared designs section of the OSHPark PCB fabricator website.  You will there both the [USB board](https://oshpark.com/shared_projects/zDjWDDDL "LK201 emulator") and the [PS2 board](https://oshpark.com/shared_projects/FJsOFwzi "LK201-PS2 emulator").
2. From any PCB fabricator that accepts KiCAD board files (such as OSHPark).
3. From any PCB fabricator that doesn't, by generating the standard *Gerber* files.

If you want to use a fabricator that needs Gerber files, open the `lk201.kicad_pcb` (USB) or `lk201ps2.kicad_pcb` (PS2) file using KiCAD (specifically the *Pcbnew* application) and use the File->Plot menu option to generate them.  You'll want to review the detailed instructions from the fabricator to confirm settings, file name rules, etc.

## Schematic

The board schematic is in `lk201.sch` (USB) or `lk201ps2.sch`.  You won't need this to create the board, but you can review this if you are interested, or if you want to modify the design.

## Assembly

The design is a simple circuit board with all through-hole components.  If you have experience building small electronic kits it should not create any problems.  If you do not, you may want to seek out some help since this is not a "Heathkit" style kit with detailed instructions or tutorials.

## USB board

### Parts list

Here are all the parts in the design, with schematic reference (also on the board silk screen labeling), value or device type, description, along with manufacturer name and Digikey part number.  The Digikey numbers are just by way of illustration.  Capacitors and resistors are generic, you can replace them by similar devices from other suppliers so long as they fit in the available space.  The capacitors  are small ceramic devices with 0.1 inch lead spacing; the resistors are standard 1/4 watt parts.  The transistor is a general purpose small NPN transistor, many other TO92 package transistors should work so long as the pinout matches.  Beware of the 2N2222, there are two variants with different pinouts!

If you don't want the key click and beep (bell) features, omit LS1, Q1, R1 and R2.
| Ref | Value | Description | Manufacturer | Digikey part # |
| --- | ----- | ----------- | ------------ | -------------- |
| C1 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C2 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C3 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C4 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C5 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C6 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C7 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| J1 | RJ22 | RJ connector, 4P4C (4 positions 4 connected), RJ9/RJ10/RJ22 | Assmann WSW | AE10381-ND |
| LS1 | PS1420 | Crystal speaker/transducer | TDK Corporation | 445-2526-1-ND |
| Q1 | 2N3904 | 0.2A Ic, 40V Vce, Small Signal NPN Transistor, TO-92 | ON Semiconductor | 2N3904FS-ND |
| R1 | 1k | Resistor, 1/4 W | Stackpole | CF14JT1K00CT-ND |
| R2 | 3.3k | Resistor, 1/4 W | Stackpole | CF14JT3K30TR-ND |
| U1 | L78L05 | Positive 100mA 30V Linear Regulator, Fixed Output 5V, TO-92 | ST Microelectronics | 497-13127-1-ND |
| U2 | Trinket M0 | Adafruit Trinket M0 board (SAMD21) | Adafruit | 1528-2361-ND |
| U3 | ICL3232CPZ | Dual RS232 driver/receiver, 5V supply | Renesas | ICL3232CPZ-ND |

If you want to use Digikey as your supplier, the saved BOM is available [there](https://www.digikey.com/short/zrhrcz "Digikey BOM").

### Assembly suggestions

I found that this order of installing the components works well:
1. U3 (16 pin DIP)
2. Resistors and capacitors
3. U1 (voltage regular) and Q1 (transistor)
4. LS1 (piezo beeper)
5. U2 (Trinket M0).  The board comes with a set of pins.  Cut off two 5-pin sections and solder these to the board, then slip the Trinket onto the pins and solder it.
6. J1 (RJ connector)

Note the correct orientation of U1, U2, U3, and Q1.

## PS2 board

### Parts list

Here are all the parts in the design, with schematic reference (also on the board silk screen labeling), value or device type, description, along with manufacturer name and Digikey part number.  The Digikey numbers are just by way of illustration.  Capacitors and resistor are generic, you can replace them by similar devices from other suppliers so long as they fit in the available space.  The capacitors  are small ceramic devices with 0.1 inch lead spacing; the resistors are standard 1/4 watt parts.  

If you don't want the key click and beep (bell) features, omit LS1 and R1.
| Ref | Value | Description | Manufacturer | Digikey part # |
| --- | ----- | ----------- | ------------ | -------------- |
| C1 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C2 | 0.01uF | Unpolarized capacitor | Kemet | 399-4150-1-ND |
| C3 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C4 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C5 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| C6 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND |
| J1 | RJ22 | RJ connector, 4P4C (4 positions 4 connected), RJ9/RJ10/RJ22 | Assmann WSW | AE10381-ND |
| J2 | Mini-DIN-6 | 6-pin Mini-DIN connector | Adam Tech | 2057-MDJ-006-FS-ND |
| LS1 | PS1420 | Crystal speaker/transducer | TDK Corporation | 445-2526-1-ND |
| R1 | 3.3k | Resistor, 1/4 W | Stackpole | CF14JT3K30TR-ND |
| U1 | L78L05 | Positive 100mA 30V Linear Regulator, Fixed Output 5V, TO-92 | ST Microelectronics | 497-13127-1-ND |
| U1 | Itsy Bitsy (5V) | Adafruit Itsy Bitsy board, 5V version | Adafruit | 1528-2501-ND |
| U2 | ICL3232CPZ | Dual RS232 driver/receiver, 5V supply | Renesas | ICL3232CPZ-ND |

Warning: there are two versions of the Itsy Bitsy, as there are with many of the AVR based Arduino microcontrollers.  One has 3.3 volt I/O, the other has 5 volt I/O.  The PS2 design requires the 5 volt version.  Do not use the 3.3 volt version, it will be destroyed when you plug in the DEC side serial cable since that supplies 12 volts to the board and the 3.3 volt Arduino has a regulator that can only handle 6 volt max power input.  And even if the regulator could handle it, driving a PS2 keyboard with 3.3 volt logic levels will not work.

If you want to use Digikey as your supplier, the saved BOM is available [there](https://www.digikey.com/short/qbc941dj "Digikey PS2 BOM").

### Assembly suggestions

I found that this order of installing the components works well:
1. U2 (16 pin DIP)
2. Resistor and capacitors
4. LS1 (piezo beeper)
5. U1 (Arduino).  The board comes with a set of pins.  Cut off two 14-pin sections and a 5-pin section and solder these to the board, then slip the Itsy Bitsy onto the pins and solder it.
6. J1 (RJ) and J2 (mini-DIN) connectors.

Note the correct orientation of U1 and U2.

## Software load

After the board is built you will need to load the software.  See the software.md file for instructions.
