# Digital Equipment LK201 keyboard emulator

The `lk201` subdirectory contains the software for the emulator circuit board.

## Loading the software

An Arduino board such as the Adafruit Trinket M0 used in this project comes from the factory with a boot loader which is used to load the main program.  By default, the Trinket will connect as a device to the USB port of your computer.  But this emulator runs that same USB port in host (not device) mode, so after the initial programming it no longer talks directly to your PC or Mac by default.  The procedure given here also works fine for the initial load.

The solution is in the boot loader built into the Trinket:
1. Connect the Trinket to your computer.  If this is not the first time programming it, the Trinket will get power but will not make a connection because both sides are in USB host mode.
2. *Double click* the reset button.  The timing is slightly tricky; if done properly the small red LED next to the USB connector will blink.  At this point, a `TRINKETBOOT` device will appear (USB attached FAT file system).
3. To use the prebuilt binary `lk201.uf2`, copy (drag and drop) that into the TRINKETBOOT device.  
4. Alternatively, use the Arduino SDK to compile and load the program source.

Either programming operation (3 or 4) will write the binary into the flash, then reset the Trinket (starting the software).  As a result the TRINKETBOOT device will suddenly disappear, which will probably result in a warning message from your computer's operating system about improper removal of the device. This is normal, the message can be ignored.

## Dependencies

The `lk201.ino` source file is largely self contained.  To build it you will need the device libraries for the Adafruit Trinket M0 and the Adafruit ZeroTimer library, both of which the SDK can download for you.  You will also need the USB host library for SAMD based Arduino devices, available [on Github](https://github.com/gdsports/USB_Host_Library_SAMD "SAMD USB host library").

## Options

There is only one build option: change `#define DOIDENT 1` to `#define DOIDENT 0` to eliminate the identification message at device powerup.
