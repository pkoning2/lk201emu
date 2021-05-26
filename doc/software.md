# Digital Equipment LK201 keyboard emulators

The `lk201` subdirectory contains the software for the USB emulator circuit board, and the `lk201ps2` subdirectory contains the software for the PS2 emulator board.  These are two separate files, although the LK201 command parsing and serial link I/O code are the same in both files.  PC keyboard scan and decode are of course entirely different.  The audio output (beep and key click) are also different because of different hardware capabilities of the two Arduino devices used.

## Loading the software

### USB emulator board

An Arduino board such as the Adafruit Trinket M0 used in this project comes from the factory with a boot loader which is used to load the main program.  By default, the Trinket will connect as a device to the USB port of your computer.  But this emulator runs that same USB port in host (not device) mode, so after the initial programming it no longer talks directly to your PC or Mac by default.  The procedure given here also works fine for the initial load.

The solution is in the boot loader built into the Trinket:
1. Unplug the RJ22 serial cable (the connection to the DEC system).
2. Connect the Trinket to your computer.  If this is not the first time programming it, the Trinket will get power but will not make a connection because both sides are in USB host mode.
3. *Double click* the reset button.  The timing is slightly tricky; if done properly the small red LED next to the USB connector will blink (by slowly increasing and decreasing its intensity).  At this point, a `TRINKETBOOT` device will appear (USB attached FAT file system).
4. To use the prebuilt binary `lk201.uf2`, copy (drag and drop) that into the TRINKETBOOT device.  
5. Alternatively, use the Arduino SDK to compile and load the program source.

Either programming operation (3 or 4) will write the binary into the flash, then reset the Trinket (starting the software).  As a result the TRINKETBOOT device will suddenly disappear, which will probably result in a warning message from your computer's operating system about improper removal of the device. This is normal, the message can be ignored.

### PS2 emulator board

The PS2 emulator board is loaded through its micro-USB connector using the Arduino SDK.  

Unplug the RJ22 serial cable (the connection to the DEC system), then plug in a USB cable to your SDK machine.  Double click the reset button to enter the boot loader (the red LED on the Arduino board will blink), then use the SDK download operation.

## Dependencies

### USB emulator board

The `lk201.ino` source file is largely self contained.  To build it you will need the device libraries for the Adafruit Trinket M0 and the Adafruit ZeroTimer and DotStar libraries, all of which the SDK can download for you through the Library Manager.  You will also need the USB host library for SAMD based Arduino devices, available [on Github](https://github.com/gdsports/USB_Host_Library_SAMD "SAMD USB host library").

If you want to rebuild the `lk201.uf2` file, you will need the uf2conv.py utility available [on Github](https://github.com/microsoft.com "UF2 file format specification") in `utils/uf2conv.py`.  Proceed as follows:
1. Compile the program.
2. Use the `Export compiled binary` menu option to generate the `.bin` file.
3. Use `uv2conv.py` to convert that to a `.uf2` file.

### PS2 emulator board

The `lks201ps2.ino` source file is self-contained.  To build it you just need the device libraries for the Adafruit Trinket (classic AVR chip, not M0).

## Options

There is only one build option: change `#define DOIDENT 1` to `#define DOIDENT 0` to eliminate the identification message at device powerup.
