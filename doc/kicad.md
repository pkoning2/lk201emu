# Digital Equipment LK201 keyboard emulator

The `kicad` subdirectory contains the design files for the emulator circuit board, which you can inspect or modify using the open source KiCAD package.

## Creating the circuit board

There are three ways to get a circuit board.
1. From the [shared designs](https://oshpark.com/shared_projects/zDjWDDDL "LK201 emulator") section of the OSHPark PCB fabricator website.
2. From any PCB fabricator that accepts KiCAD board files (such as OSHPark).
3. From any PCB fabricator that doesn't, by generating the standard *Gerber* files.

If you want to use a fabricator that needs Gerber files, open the `lk201.kicad_pcb` file using KiCAD (specifically the *Pcbnew* application) and use the File->Plot menu option to generate them.  You'll want to review the detailed instructions from the fabricator to confirm settings, file name rules, etc.

## Schematic

The board schematic is in `lk201.sch`.  You won't need this to create the board, but you can review this if you are interested, or if you want to modify the design.

## Assembly

The design is a simple circuit board with all through-hole components.  If you have experience building small electronic kits it should not create any problems.  If you do not, you may want to seek out some help since this is not a "Heathkit" style kit with detailed instructions or tutorials.

## Parts list

Here are all the parts in the design, with schematic reference (also on the board silk screen labeling), value or device type, description, along with manufacturer name and Digikey part number.  The Digikey numbers are just by way of illustration.  Capacitors and resistors are generic, you can replace them by similar devices from other suppliers so long as they fit in the available space.  The capacitors  are small ceramic devices with 0.1 in ch lead spacing; the resistors are standard 1/4 watt parts.  The transistor is a general purpose small NPN transistor, many other TO92 package transistors should work so long as the pinout matches.  Beware of the 2N2222, there are two variants with different pinouts!

If you don't want the key click and beep (bell) features, omit LS1, Q1, R1 and R2.
| Ref | Value | Description | Manufacturer | Digikey part # |
| --- | ----- | ----------- | ------------ | -------------- |
| C1 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND​ |
| C2 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND​ |
| C3 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND​ |
| C4 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND​ |
| C5 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND​ |
| C6 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND​ |
| C7 | 0.1uF | Unpolarized capacitor | Kemet | 399-14010-1-ND​ |
| J1 | RJ22 | RJ connector, 4P4C (4 positions 4 connected), RJ9/RJ10/RJ22 | Assmann WSW | ​AE10381-ND |
| LS1 | PS1420 | Crystal speaker/transducer | TDK Corporation | ​445-2526-1-ND |
| Q1 | 2N3904 | 0.2A Ic, 40V Vce, Small Signal NPN Transistor, TO-92 | ON Semiconductor | 2N3904FS-ND |
| R1 | 1k | Resistor, 1/4 W | Stackpole | CF14JT1K00CT-ND |
| R2 | 3.3k | Resistor, 1/4 W | Stackpole | CF14JT3K30TR-ND |
| U1 | L78L05 | Positive 100mA 30V Linear Regulator, Fixed Output 5V, TO-92 | ST Microelectronics | 497-13127-1-ND |
| U2 | Trinket M0 | Adafruit Trinket M0 board (SAMD21) | Adafruit | 1528-2361-ND |
| U3 | ICL3232CPZ | Dual RS232 driver/receiver, 5V supply, 120kb/s, 0C-70C | Renesas | ICL3232CPZ-ND​ |

## Assembly suggestions

I found that this order of installing the components works well:
1. U3 (16 pin DIP)
2. Resistors and capacitors
3. U1 (voltage regular) and Q1 (transistor)
4. LS1 (piezo beeper)
5. U2 (Trinket M0).  The board comes with a set of pins.  Cut off two 5-pin sections and solder these to the board, then slip the Trinket onto the pins and solder it.
6. J1 (RJ connector)

Note the correct orientation of U1, U2, U3, and Q1.

## Software load

After the board is built you will need to load the software.  See the software.md file for instructions.
