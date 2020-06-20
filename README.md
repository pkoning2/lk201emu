# Digital Equipment LK201 keyboard emulator

This project allows a standard USB PC keyboard to be used as replacement for a DEC LK201 keyboard.  It consists of a small circuit built around an Adafruit Trinket M0 Arduino processor, and software for that to implement a USB host driver for the USB keyboard and the LK201 communication protocol. 

Subdirectory `lk201` is the Arduino software.  You can build it using the standard Arduino SDK, or use the prebuilt binary.

Subdirectory `kicad` is the circuit board design.  You can view it using the open source KiCAD package.

Subdirectory `doc` contains several documentation files in MD format.  File `emulator.md` describes the features of the emulator; `kicad` describes the board design and assembly information, and `software` describes the software and the load procedure.  There are also two JPG files that show the circuit board.

File `LICENSE` is the open source license under which this emulator is released.

