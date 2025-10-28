# Wide-band-receiver
An all mode wide band receiver 0.1-860MHz. Using Arduino IDE for firmware, SI4732+AD831 for shortwave reception and additionally an I2C TV tuner for VHF/UHF reception.

This firmware covers a wide band receiver project. It is an experimental receiver and should not be considered as a flawless building description with guaranteed success. 
It does require RF experience and tools.

It uses a NOS "tin can" I2C TV tuner and covers 0.1 - 860MHz.
The driver module "TVTuner.ino" is designed for Philips PAL I2C tuners and has been tested on UR1316-3 and CD1316-3 tuners. UV1316-3 tuner should also work.
Other Philips I2C tuners may have a different pinout and different IF and AGC specs, but many use the same data format and should work.

It may be possible to adapt it for other brands by adjusting the data packet that gets sent to the tuner. 
Philips tuners work fine with +3.3V on the I2C bus. Other brands may need level shifters.

It uses ESP32, ILI9488 with an RESISTIVE touch display (the red pcb), SI4732, SI5351 and AD831 as main components. No other displays are supported.
It uses potentiometers for volume, squelch and fine tune.
A tinySA can be used as optional panorama adapter and will then get automatically configured and updated by the software. 
It does include some nice features, such as morse decoder, slow scan waterfall, memory bank scanning, web tools, etc.
To build the hardware you should have experience with RF circuits and the appropiate toolset. At least a tinySA and NanoVNA are strongly recommended. 

See the main file SI4732Radio_V0_524.ino for more details.
![Alt text](1.jpg)
