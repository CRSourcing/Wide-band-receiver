//##########################################################################################################################//


//IMPORTANT: To compile, select the "ESP32 Dev Module" and the "No OTA (2MB APP, 2MB SPIFFS)" partition scheme. 
// Select compiler directives according to your hardware:

#define TV_TUNER_PRESENT  // If this option is commented out, a shortwave receiver version will compile. In this case max. FREQ = 50MHz.

#define AUDIO_SQUAREWAVE_PRESENT  // Audio squarewave present on GPIO39 for SSTV and RTTY decoding. Experimental.

#define FAST_TOUCH_HANDLER  // Invokes a faster touch handler that does not sample. Could cause spurious errors, but increases speed significantly. No problems so far.

#define SHOW_DEBUG_UTILITIES  // Will show Debug utilities panel. Contains helper functions and status messages.

#define NBFM_DEMODULATOR_PRESENT  // Uses an additional MC3361 as hardware NBFM demodulator. Better audio than the SI4732 flank demodulator
// Provides a frequency offset indicator and software AFC.

#define SI5351_GENERATES_CLOCKS  //If uncommented, the SI5351 will generate the LO frequency plus 2 clocks, 4MHz for the tuner and 32768Hz for the SI4732. 

#define TINYSA_PRESENT  // Syncs and controls a tinySA with receiving frequency, if connected via serial to the ESP32.

#define SW_ATTENUATOR_PRESENT  // uncommment only if a voltage controlled attenuator is present in the shortwave RF path.
//Generates gain control voltage on dac1(GPIO_NUM_25). 0V = max. gain, 3.3V = min gain.

//#define PRINT_STORAGE_DEBUG_MESSAGES // Creates serial debug messages about files on LittleFS and SDcard.
//##########################################################################################################################//

/*
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
The code is a bit rough since I am not a programmer. It does include some nice features though, such as morse decoder, slow scan waterfall, memory bank scanning, web tools, etc.
To build the hardware you should have experience with RF circuits and the appropiate toolset. At least a tinySA and NanoVNA are strongly recommended. 
At least basic Arduino programming knowledge helps a lot!


Optionally, the code can be compiled for a shortwave receiver only version. In this case you will have a receiver that covers 0-50MHz.
The shortwave receiver will have better filtering and smoother tuning than a SI4732 only receiver.
If you build the"shortwave only" version, please connect the ouput of the lowpass via 220nF to the RF input of the AD831 mixer.

For testing, the GUI can be started with only the ESP32 and the ILI9488 display connected, but responses will be sluggish and several of the functions will lock the GIU.
The receiver can be tested with only the SI4732 only connected (no SSB). Select "No Mixer" in the "Debug" panel. 
Connect the antenna directly to the AM input of the SI4732. "No Mixer" is not sticky and for testing only.



IMPORTANT: To compile, select the "ESP32 Dev Module" and the "No OTA (2MB APP, 2MB SPIFFS)" partition scheme. 
It should then compile without issues. Some warning messages from the libraries may show up. They are not relevant.


Hardware description:

1. RF gets amplified 15dB using a MAR-6 and a SBA4089 gain block. YG602020 or any other gain block that goes down to 100KHz with a high enough IP3 can also be used. 
A small signal relay switches the gain block output betwee the tuner input (75Ohms) and the 0-50MHz lowpass filter for SW mode.

The UR 1316 tuner output pins are connected to the IF relay, the I2C bus, +33V from a 5->33V bost converter and the AGC output of the ESP32. 
A transistor logic switches the tuner and the boost converter power off when frequency < 50MHz. If frequency stability of the tuner is more important than 
power consumption, omit this logic. It helps saving current and eliminates possible crosstalk fromt he tuner output when in shortwave mode. 
It means however that the tuner will change temperature and have frequency drift when it warms up. My tuner drifts about 2KHz.
The CD1316 tuner has a different pinout and does not need the boost converter.

The ACG voltage from the ESP32 drives tuner gain down when a  strong signal is received. To reduce intermodulation the AGC voltage reduces tuner amplifiction even when no signal is received.

2. The IF output of the tuner connects via a resistor to parallel resonant LC and the IF input relay. Output of the tuner needs to be reduced (by some 20dB).

The IF relay switches either the output of the tuner or the lowpass filter output to the input of the AD831. AD831 gets fed with CLK2 from the SI5351. 
If the tuner is in use, the SI5351 oscillates 21.4MHz above the IF spectrum (37- 38MHz). When the frequency reaches either end, the tuner PLL gets reprogrammed.
Tuners UR/UV1316 require a 5V -> 33V boost converter. A boost converter with NE555 produced audible interference, I therefore use a transistorized converter.
Tuners UR/UV1316 cover from 50 - 860 MHz. Sensistivity of the receiver is about 0.2-0.3 uV for 10dB SNR and mostly determined by the 1st gain block. 

3. For the "Shortwave only" version, please connect the ouput of the lowpass via 220nF to the RF input of the AD831 mixer. No RF input relay is needed


4. The IF side of the AD831 is terminated with 50 Ohms and enters a crystal filter which is made of 3 * 21.4 MHz (Aliexpress) filters in series. 
The housings of the 2nd and 3rd filter are always grounded, while the 1st is grounded via 22pf and a ground path through 2 diodes that can be activated in the software. 
These allows to change the IF bandwidth.
The filters flanks should not be too steep, otherwise NBFM flank demodulation will get distorted. Filters with 7.5KHz or 15KHz filter bandwidth (21M7, 21M15) are recommended.
If the compiler option "NBFM_DEMODULATOR_PRESENT" is enabled, the software will not use flank demodulation, but an optional MC3361 NBFM demodulator instead. 

My filter's center frequency is off around 3KHz (21.397MHz) for some reason, but this can be compensated in software. The filter is critical for not overloading the SI4732.
The IF can be configured in the software, anything up to 30MHz is possible, so you could also use filters for different frequencies. 
The higher the IF, the better image rejection will be.

5. A tinySA can be used as optional panorama adapter. 

6. The crystal filter output connects to a SI4732. There is quite a loss (10 - 15 db estimated) caused by the the crystal filter, since it does not use impendance transformation, 
but the preamp provides enough amplification to overcome it. 
The SI4732 is controlled by an ESP32 development board with 38 pins. (The 30 pin dev board does not provide enough GPIO's.) 
SI4732 is in the standard configuration , optionally with an external 32.768 KHz TCXO.The software for FM radio reception is quite basic.
One of the audio outputs go to a squelch transistor which is driven by one of the ESP32 GPIO's. It grounds the audio signal when triggered and  eliminates noise and the hard cracks when switching mode.
From the squelch transistor the signal goes to the volume potentiomenter and then to a LM386 audioamplifier which drives headphones and speaker. GPIO26 is connected via 220KOhms
to the input of the audio amplifier and provides a short touch sound. The other audio output (pin 16) goes via a transistor amplifier to the sampling input of the ESP32 GPIO36, see (11).


7. The ESP32 drives the SI4732 and SI5351 on the same I2C bus. Bus speed is 2MHz, but will get autmatically reduced if the ESP32 can't initiate the SI4732 
or when the TV tuner gets adressed.
A squelch potentiometer and a fine tune potentiometer are connected to the ESP32.
The fine tune potentiometer is also used to adjust colors in waterfall mode. The fine tune pot needs to be of good quality, since any noise causes frequency jumps.
The ESP32 also drives the 3.5" ILI9488 touch display in a standard configuration using the tft eSPI library.
This code will only work with ILI9488 480*320 pixel displays and is NOT adaptable for smaller displays.

8. 12V DC input input gets regulated through two linear regulators down to +5 and +9V. Power consumption is about 400-500ma. Both regulators require a small heatsink.

9. For the FFT analysis and the morse decoder, audio output pin 16 goes via 4.7K and 1 microfarad to the base of a npn transistor amplifier. Emitter via 100 Ohms to ground, base via 47K to collector and collector via 1K to +3.3V.
Collector then goes to pin 36 of the ESP which does the sampling. Gain is about 20 dB.

10. An additional audio amplifier with a square wave forming circuit consisting of a two transistor amplifier (and optionally a NE555) can be connected to GPIO39.
This allows the experimental SSTV and RTTY decoders to work.

11. If you use a tinySA, connect GPIO 1 of the ESP32 with the RX pin of the tinySA and GPIO 3 with the TX pin (close to the battery connector).
This will allow control of tinySA parameters from the receiver menu.
It will also allow calibrated measurement of the signalstrength in dBm and microvolts (using the seleced RBW of the tinySA). 
If you do this, please note that the tinySA needs to be switched off when loading firmware into the ESP32.
Connect the tinySA audio output via 10K and 0.1uF in series to the audio output of the SI4732.
You can then listen to the tinySA. Check the Youtube video to see how it works.


12. Additional SD card support has been implemented.  It uses the on board SD card slot of the ILI9488 display. Connect the pins in parallel to the existing SPI bus, except the CS pin.
The SD Card CS pin gets connected to ESP32 GPIO 33. Not all SD cards work, this seems to be a limitation of the SD card library. Try several, including older ones. 

This will allow to update the 3 main structures (memory, station scan frequencies and predefined bands) from SD card so no recompiling is needed when values get changed. 
These files can be found in the "data" folder. 

13. The receiver does require a formatted LittleFS file system on the ESP32. It will auto format during the 1st run. 
If the SD card option does not work, yuo can use the LittleFS uploader (https://github.com/earlephilhower/arduino-littlefs-upload) to upload the files to the file system.
Do not introduce additional commas, CR or else anyting into the lines of the CSV files, there is little error checking implemented.

14. An experimental option to record audio to SDcard has been implemented. It will record only when squelch is open and it's performance depends on the SDcard.

15. An additional NBFM demodulator has been developed since the SI4732 demodulates NBFM only via flank demodulation. 
An MC3361 converts 21.4MHz to 400KHz and uses a quadrature demodulator to generate audio and the discriminator voltage which can be displayed on the upper tuning meter.


16. I have not designed a PCB. The ESP board is directly attached to the backside of the display and so is the LM386 audio amp.  
The IF/RF part is build upon the copper sides of two seperate (unetched) PCB's with the SI4732, the NBFM demodulator, crystal filter and voltage regulators on one.
The LNA, TV tuner, 33V boost converter, AD831 and SI5351 are on the other board. All RF/IF modules are built in separate shielded boxes.
Audio uses a separate ground from the digital lines.

The I2C and SPI buses of the receiver and tinySA will cause some noise on some frequencies, 
but with a clean built and proper shielding techniques these noises will mostly get buried in the atmospheric noise of the antenna. Shielding and good grounding is important!

17. For better frequency stability a TCXO modification can be used: Add 25MHz TCXO to SI5351 module as described on the internet. 
Do not use a 32768Hz crystal with SI4732. Connect CLK0 via 1nF to pin 13 of SI4732. 
Remove 4MHz crystal from tuner. Connect CLK1 via attenuator (see schematic) to hot solder point of the crystal. Uncomment //define SI5351_GENERATES_CLOCKS


Calibration:

1. Set Master Volume to 50. Set AVC to 100.Set SMUTE to 0.

2. In Config -> SetIF SI4732: Set to 21400 if not already set. If peak frequency of crystal filter is is already known set to the peak frequency instead and skip this step.
3. Feed input of crystal filter with signal 21.4MHz at -60dBm. Change frequency of signal generator +- 5KHz in 1KHz steps and change simultaneously "SetIF SI4732" to the same frequency.
   Select the frequency with the strongest dBm reading and press encoder. Some cheap filters show two peaks, about 5KHz away. Instead of directly grounding  the cover, I grounded it via 22pf.
   This gave a single flat peak.

4. Feed antenna input with 10MHz (precise) and tune to 10MHz. In Config -> Calib SI5351: Adjust for strongest dBm or microvolt level.
   This will correct the offset of the SI5351. 
5. Change modes to USB and LSB and adjust the BFO's to zero Hz audible tone. Change mode to CW and set the BFO to roughly 600Hz. Press button Audio WF. It should draw a red line. If not, correct
   Master Volume.
6. Feed antenna input with 10MHz and a calibrated level. In More Config -> Gain Calibr. adjust displayed dBm value to the calibrated level. This will only calibrate the displayed level 
   for SW, not VHF/UHF.
7. For TV tuner: Set your signal generator to it's highest possible frequency between 50 and 870MHz. Set the radio to the same frequency. 
   In More Config -> Tuner +-ppm adjust for strongest signal, press encoder.
8. Additionally for SSB in VHF/UHF calibrate the TV tuner BFO's for zero Hz audible tone, but the tuner may have temparature drift.

9. "Tun. Attn" will reduce the tuner gain when no signal is received. It will also affect the point where the tuner agc kicks in.

--------------


TinySA Setup and Operation:
Hardware Connection
The TinySA RF input is connected via a buffer amplifier to the output of the first RF gain block.
Recommended buffer gain: 15–20 dB (example: MAR‑6 LNA).


Firmware & Serial Configuration
Firmware requirement: TinySA firmware version ≥ v1.4‑105 (2023‑07‑21).
Serial connection: Configure TinySA to use 115200 baud so it accepts commands from the ESP32.

Buttons in the receiver "TinySA Options" menu:
TinySA Mode:

Window Mode: Displays a 1 MHz segment.
Marker 1 shows the tuned frequency within the window.
Marker moves across the screen as frequency changes.
Tuning outside the window switches to the next window.

Center Mode:
Tuned frequency stays at the center.
Slower, since TinySA must rebuild the entire screen whenever frequency changes.


TinySA Config: Adjust Span and Bandwidth.

Listen: Option to enable “Listen” if TinySA audio output is connected to the receiver’s audio amp.

TinySA Sync:
Synd disabled: TinySA works independently from the receiver.
Sync enabled: Tunes a 1MHz segment around the receiver frequency. Markers follow the tuned frequency.
Shows strongest peak within the 1 MHz segment.
Receiver uses TinySA’s dBm value for S‑Meter readings (more precise than SI4732 RSSI).



Additional Buttons (appear after successful sync on the lower right side of the receiver screen)
R: Resets TinySA.
M1: On TinySA drag Marker 1 to a signal of interest -> receiver tunes to that signal. Tap again to exit.
M2: Tunes receiver to frequency of Marker 2 (strongest peak).
M2F: Receiver follows Marker 2 only when squelch is closed. Stays on a frequency once squelch opens, when squelch closes again moves to next strongest peak.
Cfg: Configure TinySA parameters.

-----------


TinySA setup Before Use:
Connection Settings
Config → More → Connection → Change to Serial.
Set serial speed to 115200.

Trace Menu
Trace 2 (Green): Enabled
Trace 3 (Red): Enabled
Trace 1 (Yellow): Disabled

Marker Menu
Assign Marker 1 to Trace 2, unselect “Tracking” and everything else that may be selected.
Assign Marker 2 to Trace 3, select “Tracking”.

Display Menu
Enable Waterfall.
Additional: Set Sweep Accuracy to Fast.


Level Menu
Ext Gain: Enter buffer amplifier gain (e.g., 15 dB).

Save Settings
Save configuration as Startup Preset.

Enable “Save Settings”.



 
Software hints:

There is little error checking implemented, so if you press combinations that make no sense you will get a result that makes no sense.

Options to select a frequency: 
1.Set Frequency manually via "Set Freq", 2. Load freq from Memo, 3. Select a Band, 4. Use Up- Down keys, or 5. tap the frequency display either
on the left or right side.

"Select Band": Loads a predefined band from struct BandInfo. Configurable, tuning either without limits (can leave the band) or loop mode (stays within band). 
Scan: Seek up, Seek down, or set a range. Scan is squelch driven. If band loop mode is enabled, Seek will stay within the band. 

Memory scan: Only possible within 1 page. Press Scan, then press the buttons to scan, then scan again. Close squelch enough to avoid stuttering.

AutoSquelch: If activated (default), either SNR OR RSSI will trigger the squelch. The squelch will always open  when SNR is above 0, no matter squelchpot position. 
The squelch will adapt (get less sensitive) if it opens multiple times quickly (stuttering).  
The squelchpot will still set the RSSI (signal strength) treshold. The little "SNR" indicator will lit white when SNRSquelch is in use.

Waterfall (slow scan): Displays either a frequency range "View Range", or a band "View Band" as a waterfall. Use the fine tune potentiometer to adjust sensitivity and colors.
Touch on a spot on the lower horizontal signal bar (while it says "SCANNING") and enter listen mode. 
Touch on the waterfall to go back to scan mode, or touch SET to set the frequency and leave.

Touch Tune: Shows += 500KHz around tuned frequency, use encoder to change frequency. Touch a spot on the signal area and it tunes in and you can listen to frequency.
If the touch is off, use encoder to fine tune. Touch "Cont." to leave frequency and continue.Touch back to go back to main menu without setting the frequency. 
Touch "Set" to go back to main menu and set frequency.

3D Waterfall displays a trapezoid that shows frequency time relationship and gives the illusion of a 3D waterfall.

1MHz steps: After pressing the encoder,step size will be changed to 1MHz. This helps to rapidly view 1MHz segments on the tinySA. Press encoder again to return to previous step size.


Station scan: Will scan through a list of (25) predefined stations. Can be configured to stop when audio or carrier is detected. 

Show Panorama: Will display a +-500KHz spectrum around the tuned frequency when the squelch is closed. Squelch should open when there is a signal at the tuned frequency. 
If a signal on the center frequency does not open the squelch, multiple erroneous peaks will be shown. Close squelch just a bit.

To change modulation, bandwith, filter bandwidth, or ACG, press the buttons, or tap the indicators ("icons") directly.

Display noise: At some frequencies periodic noise caused by the display updates can be present. On the main screen, press "More" and the display will be frozen. 
Tap the empty black area to go back to the main screen. Good grounding practices and strong decoupling of the different supply voltages will reduce display and I2C noise.

"Tuner Attn." selects the maximum gain of the TV tuner. Full tuner gain can easily overload the tuner. Reduce gain until the atmospheric noise is still dominant.  
If the max gain is set too high, intermoduation will appear. If the max gain is too low, the receiver will lose sensitivity. Good vaules for max gain are between 20 and 35. 
The tuner AGC will lower the gain automatically if a strong signal has been received.

Analog meters: If analog meters are enabled, in order to tune down/up,please press the frequency display left or right 
If a hardware NBFM demodulator is installed, the upper meter will show the frequency deviation of the signal.
Touch the upper meter to enable AFC in AM or NBFM. The AFC is not sticky and will auto disable everytime SSB or CW mode is selected.

*/


#include <Arduino.h>
#include <driver/gptimer.h>
#include <driver/ledc.h>
#include <FS.h>
#include <font_Arial.h>
#include <HTTPClient.h>
#include <JPEGDecoder.h>
#include <LittleFS.h>
#include <PNGdec.h>
#include <Preferences.h>
#include <si5351.h>
#include <SPI.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <TFT_eSPI.h>
#include <unordered_map>
#include <vector>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>


#include "arduinoFFT.h"
#include "DacESP32.h"
#include "DSEG7_Classic_Mini_Regular_34.h"
#include "FS.h"
#include "patch_full.h"
#include "Rotary.h"
#include "SD.h"
#include "SI4735.h"
#include "Sprites.h"
#include "logSerial.h"

//##########################################################################################################################//

#ifdef TV_TUNER_PRESENT

#define LOW_BAND_LOWER_LIMIT 50000000  // TV tuner bands limits tested with tuner UR1316
#define LOW_BAND_UPPER_LIMIT 167000000
#define MID_BAND_UPPER_LIMIT 454000000
#define HIGH_BAND_UPPER_LIMIT 875000000      // sensitivity decreases rapidly > 850MHz
#define SHORTWAVE_MODE_UPPER_LIMIT 50000000  //above 50MHz tuner is in use
#define HIGHEST_ALLOWED_FREQUENCY HIGH_BAND_UPPER_LIMIT

#define TUNER_UR1316  // Tuner type 1

#endif

#ifndef TV_TUNER_PRESENT
#define HIGHEST_ALLOWED_FREQUENCY 50000000  // 50MHz
#define SHORTWAVE_MODE_UPPER_LIMIT 50000000
#endif

#define LOWEST_ALLOWED_FREQUENCY 100000  // could probably go lower, but noise gets dominant
//##########################################################################################################################//


//colors
#define TFT_BLACK 0x0000 /*   0,   0,   0 */
#define TFT_NAVY 0x000F  /*   0,   0, 128 */
#define TFT_DARKDARKGREEN 0x1b41
#define TFT_DARKGREEN 0x03E0
#define TFT_MIDGREEN 0x0584
#define TFT_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define TFT_DARKVIOLET 0x2028  // 4,1,8
#define TFT_MAROON 0x7800      /* 128,   0,   0 */
#define TFT_PURPLE 0x780F      /* 128,   0, 128 */
#define TFT_OLIVE 0x7BE0       /* 128, 128,   0 */
#define TFT_LIGHTGREY 0xD69A   /* 211, 211, 211 */
#define TFT_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define TFT_DARKDARKGREY 0x2945
#define TFT_SILVERBLUE 0x8D5F
#define TFT_BLUE 0x001F  /*   0,   0, 255 */
#define TFT_GREEN 0x07E0 /*   0, 255,   0 */
#define TFT_CYAN 0x07FF  /*   0, 255, 255 */
#define TFT_RED 0xF800
#define TFT_DARKRED 0x90C1 /* 255,   0,   0 */
#define TFT_MAGENTA 0xF81F /* 255,   0, 255 */
#define TFT_YELLOW 0xFFE0  /* 255, 255,   0 */
#define TFT_WHITE 0xFFFF   /* 255, 255, 255 */
#define TFT_ORANGE 0xFDA0  /* 255, 180,   0 */
#define TFT_DEEPORANGE 0xf401
#define TFT_GREENYELLOW 0xB7E0
#define TFT_PINK 0xFE19 /* 255, 192, 203 */
#define TFT_DARKBROWN 0x6a44
#define TFT_BROWN 0x9A60 /* 150,  75,   0 */
#define TFT_LIGHTBROWN 0x9347
#define TFT_GOLD 0xFEA0    /* 255, 215,   0 */
#define TFT_SILVER 0xC618  /* 192, 192, 192 */
#define TFT_SKYBLUE 0x867D /* 135, 206, 235 */
#define TFT_VIOLET 0x915C  /* 180,  46, 226 */
#define TFT_GREY 0x5AEB
#define TFT_FOREGROUND TFT_GOLD
#define TFT_GRID TFT_YELLOW

//plain button colors
#define TFT_BTNBDR TFT_NAVY
#define TFT_BTNCTR TFT_BLUE
#define TFT_MAINBTN_BDR TFT_NAVY
#define TFT_MAINBTN_CTR TFT_GREY

// button and sprite button sizes
#define TILE_WIDTH 75
#define TILE_HEIGHT 50
#define SPRITEBTN_WIDTH 80
#define SPRITEBTN_HEIGHT 50
#define M_SIZE 0.59  // meter size, does not affect the sprite frame size

//display size
#define DISP_WIDTH 480
#define DISP_HEIGHT 320

// modulation
#define AM 1
#define LSB 2
#define USB 3
#define NBFM 4
#define WBFM 5
#define SYNC 6  //AM Sync, doesn't work well
#define CW 7


#define FMSTARTFREQ 9950  // WBFM initial freq, use frequency of a nice radio station

#define DEFAULT_AM_STEP 5000
#define DEFAULT_SSB_STEP 1000
#define DEFAULT_CW_STEP 100
#define DEFAULT_SYNC_STEP 5000
#define DEFAULT_NBFM_STEP 12500
#define DEFAULT_WBFM_STEP 100000

//ESP32 GPIO's
#define IF_FILTER_BANDWIDTH_PIN 0  // switches filter bandwidth, GPIO 0 okay for this
#define RESET_PIN 12               // SI4735 reset
#define NBFM_MUTE_PIN 13           // Hardware NBFM demodulator mute pin, muted when another modType is in use
#define IF_INPUT_RELAY 14          // activates IF relay
#define ENCODER_PIN_B 16           // Encoder left and right pins
#define ENCODER_PIN_A 17           // Encoder left and right pins
#define ESP32_I2C_SDA 21           // I2C bus
#define ESP32_I2C_SCL 22           // I2C bus
#define TUNER_AGC_PIN 25           // tuner AGC pin


#ifndef NBFM_DEMODULATOR_PRESENT
#define tinySA_PIN 27  // GPIO for relay driver that selects input for the tinySA
#endif

#ifdef NBFM_DEMODULATOR_PRESENT
#define TUNING_VOLTAGE_READ_PIN 27  // Tuning voltage read pin
#endif

#define ENCODER_BUTTON 33   // encoder pressbutton pin
#define MUTEPIN 32          // goes via 3.3K to base of external mute transistor
#define FINETUNE_PIN 34     // finetune pot center pin
#define SQUELCH_POT 35      // squelch pot center pin
#define AUDIO_INPUT_PIN 36  // audio input for FFT analysis from SI4732
#define PULSE_PIN 39        // squarewave audio input




// Waterfall
#define WATERFALL_SCREEN_WIDTH 480
#define WATERFALL_SCREEN_HEIGHT 227  // more than that will lead to a memory allocation error
#define FRAMEBUFFER_HALF_WIDTH 120   // need to use two half size frame buffers, couldn't allocate a full size 240*231 buffer
#define FRAMEBUFFER_FULL_WIDTH 240
#define FRAMEBUFFER_HEIGHT 227  // was 231, but crashed with new Arduino IDE 2.3.5
#define AUDIO_FRAMEBUFFER_HEIGHT 40

// Fun
#define TIME_UNTIL_ANIMATIONS 60  // seconds until animations start (pacm, audio waterfall)

// 3D drawing
#define NUM_COLUMNS 333
#define COLUMN_HEIGHT NUM_COLUMNS / 4

//TFT
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_RESET 4

//LittleFS
#define FORMAT_LittleFS_IF_FAILED true  //format the LittleFS if not already formatted or error

//SD card
#define SCK 18    // definitions for SD card
#define MISO 19   //MISO=SDO
#define MOSI 23   //MOSI=SDI
#define SD_CS 33  // Connect CS to GPIO33 (also used for encoder press button), all other connections in parallel to SPI bus.
// Format SDcard as FAT32

//FFT
#define SAMPLES 256
#define FFT_SPEED_OVER_PRECISION
#define FFT_SQRT_APPROXIMATION


//tinySA
#define DEFAULT_SPAN 1000000  // cover 1MHz

//Splashscreen  will show file splash.jpeg at startup
#define SHOW_SPLASHSCREEN

//Connections with ILI 9488 3.5" display:
/// Touschscreen TDO -> 19
//TFT_SDI -> 23
//TFT_CLK ->18
//TFT_CS   15  // Chip select control pin
//TFT_DC    2  // Data Command control pin
//TFT_RST   4  // Reset pin (could connect to RST pin)
//TOUCH_CS 5     // Chip select pin (T_CS) of touch screen


// TFT display defines in user setup.h
/* 
#define TFT_MISO 19 //!!!!Connect ESP32 19 with touchscreen TDO. NOT with TFT_MISO. Connect other pins as below
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin 
#define TOUCH_CS 5     // Chip select pin (T_CS) of touch screen

*/
//##########################################################################################################################//
//Classes
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
TFT_eSPI tft = TFT_eSPI();
TFT_eSPI_ext etft = TFT_eSPI_ext(&tft);
Si5351 si5351(0x60);  //Si5351 I2C Address 0x60
SI4735 si4735;
Preferences preferences;  // EEPROM save data
PNG png;
File myfile;
SPIClass spiSD = SPIClass(VSPI);                // for SDcard
DacESP32 dac2(GPIO_NUM_26), dac1(GPIO_NUM_25);  // dac2 for sine wave oscillator, dac1 for tuner AGC or SW RF attenuator

float RvReal[SAMPLES * 2];
float RvImag[SAMPLES * 2];
ArduinoFFT<float> FFT = ArduinoFFT<float>(RvReal, RvImag, 512, 5850);  // Max freq = 5850/2 Hz, 256 usable bins
RingBuffer logBuffer; // debug logs
//##########################################################################################################################//

// global variables

/* GPTimer handle currently not in use

static gptimer_handle_t gptimer = NULL;
// Flag to check when timer fies
volatile bool timer_fired = false;

*/

const uint16_t size_content = sizeof ssb_patch_content;  // see ssb_patch_content in patch_full.h or patch_init.h
volatile bool clw = false;                               // encoder direction clockwise
volatile bool cclw = false;                              // counter clockwise

const char ver[] = "V.523";         // version

long I2C_BUSSPEED = 2100000;  // Adjust as needed. This is high, but seems to work fine. Gets automatically reduced when the tv tuner gets addressed
long STEP;                    //STEP size
long OLDSTEP;
long MIN_FREQ = 100000;  // lowest frequency
long SI5351calib = 0;    //  si5351.set_correction


long FREQ = 100000;            // receiver tuned frequency
long FREQ_OLD = FREQ - 1;      // start with != FREQ value
long LO_RX;                    // SI5351 frequency in Hz
int SI4735TUNED_FREQ = 21400;  // 1st IF in KHz Also mid frequency of crystal filter, SI4732 needs to be tuned to it. 
bool wideIFFilter = true;      // IF filter bandwidth false = narrow, true = wide;
bool afcEnable = false;
int NBFMOffset = 0;           // shift of  SI4735TUNED_FREQ in KHz to demodulate FM on the flank of the filter.
int afcVoltage = 0;           // voltage tap from the quadrature demodulator
int discriminatorZero = 100;  // middle of the tuning meter needle
int RFGainCorrection = -6;    //  corrects overall gain provided by the 1st gain block after splitter and filter
uint8_t SWAttn = 0; // Shortwave attenuator attenuation, 0 = 0dB, 255= 30dB attenuation
uint8_t SWMinAttn= 0; // Shortwave attenuator minimum attenuation, 0 = 0dB, 255= 30dB attenuation
bool attnOFF = true;  // SW Attenuator off
long lastAMFREQ = -1;  // AM frequency before switching to WBFM
long span = 1000000;   // tinySA default span, configured in TSA Presets 0 and 1
long potVal;           // fine tune potentiometer read value
long lim2 = 0;         // frequency limits from touchpad entries
long lim1 = 0;         // frequency limits from touchpad entries
long keyVal = 0;       // scan mode frequencies delivered from touchpad


bool TVTunerActive = false;     //will get set to true when tuner in use
bool lowBand = false;     // // TV tuner bands
bool midBand = false;
bool highBand = false;
uint8_t initialGain = 180;     // adjust tuner gain when no signal received, depending on gain of the LNA. Must be as low as possible to reduce cross modulation
uint8_t agcVal = initialGain;  // starting point of the AGC
uint32_t OLDPLLFREQ = -1;
uint32_t PLLFREQ = 0;            // PLL freq of the TV tuner
int16_t tunerOffsetPPM = 0;      // corrects tuner crystal offset
uint8_t tunerGain = 32;          // dB gain provided by (tuner - attenuator)
uint8_t tunerdBmcorrection = 0;  // correcta sMeter and dBm values when tuner AGC kicks in, since tuner gain decreases
#ifdef TV_TUNER_PRESENT
long MAX_FREQ = HIGHEST_ALLOWED_FREQUENCY;
#endif

#ifndef TV_TUNER_PRESENT
long MAX_FREQ = 50000000;
#endif

bool tinySA_RF_Mode = true;     // start tinySA in RF mode
bool tinySACenterMode = false;  // tinySA in RF mode synchronizes with receiver frequency
bool tinySAfound = false;
bool syncEnabled = false;     // uses Marker 1 for Smeter and dBm and uV indicators
bool serialDebugTSA = false;  // used to debug incoming TSA messages
bool showAGCGraph = false;
bool scanMode = false;
bool pressed = false;
bool pressSound = true;
bool redrawMainScreen = true;  // redraw  main window elements
bool encLockedtoSynth = true;  // locks encoder to SI5351
bool ssbLoaded = false;
bool SI4735WBFMTune = false;  // for WBFM, will set SI4732 Frequency
bool SI4732found = false;
bool altStyle = false;         // false = use sprites, true = use tiles
bool displayDebugInfo = true;  // display clock info and looptimer
bool showScanRange = true;     // show scan range after entered
bool LOAboveRF = true;         // true = LO above RF, false = LO below RF
bool WBFMactive = false;       // to switch back to AM/SSB after WBFM
bool loopBands = false;        // if true then FREQ will cycle when a band end is reached
bool use1MHzSteps = false;     // use 1MHz Steps after encoder pressed;
bool showFREQ = true;          // false supresses FREQ display
bool setFilterManually = false;
bool roundToStep = true;   // round  FREQ up or down to the next STEP when STEP or modulation gets changed
bool sevenSeg = false;     // use 7 segemnt font
int selected_band = -1;    // number of the selected band  -1 means no band selected
bool showMeters = false;   // shows analog meters on right side
bool resetSmeter = false;  // reset Smeter when frequency changes, to avoid decay delay
bool funEnabled = false;   // animations
bool enableAnimations = false;
const int Xsmtr = 10;  // Smeter position
const int Ysmtr = 58;  // Smeter position
int dBm = 0;           // dBm based onn RSSI
int TSAdBm = 0;        // dBm coming from tinySA readings


const int HorSpacing = 86;     // buttons horizontal distance
const int VerSpacing = 65;     // buttons vertical distance
const int vTouchSpacing = 72;  // adjust vertical touch spacing
bool loadHistory = false;      // right panel available

uint8_t bandWidth = 0;
uint8_t lastSSBBandwidth = 3;
uint8_t lastAMBandwidth = 0;
uint8_t AGCDIS = 0;  //This param selects whether the AGC is enabled or disabled (0 = AGC enabled; 1 = AGC disabled);
uint8_t AGCIDX = 0;  //AGC Index (0 = Minimum attenuation (max gain); 1 – 36 = Intermediate attenuation); if >greater than 36 - Maximum attenuation (min gain) ).
uint8_t SNR = 0;
int8_t signalStrength = 0;  // RSSI
float microVolts;
uint8_t modType = 1;                        //modulation type
uint16_t tx = 0, ty = 0, ttx = 0, tty = 0;  // touchscreen coordinates and temporary value holders
uint16_t row = 0, column = 0;               // buttons are grouped in rows and columns

// Bodmer analog meters
int xSh = 340;   // xshift meters
int ySh = 47;    // yshift 1st meter
int ySh2 = 126;  // yshift 2nd meter
float ltx = 0;   // Saved x coord of bottom of needle
float ltx2 = 0;
uint16_t osx = M_SIZE * 120, osy = M_SIZE * 120;    // Saved x & y coords
uint16_t osx2 = M_SIZE * 120, osy2 = M_SIZE * 120;  // Saved x & y coords
int old_analog = 0;                                 // Value last displayed
int old_analog2 = 0;                                // Value last displayed


// misc
const uint16_t textColor = TFT_ORANGE;
bool audioMuted = false;            // mute state
char miniWindowMode = 0;            // mini window mode: 1 = 16 channel, 2 = 85 channel, 3= minioscilloscope, 4 = waterfall, 5 = envelope
int amplitude = 150;                // amplitute divider for spectrum analyzer
int fTrigger = 0;                   // triggers functions in main loop
int currentSquelch = 0;             // squelch trigger level
uint8_t vol = 50;                   // global volume, should be around 50
bool disableFFT = false;            // stops the spectrum analyzer during time critical operations
bool SNRSquelch = true;             // use either RSSI or SNR to trigger squelch
bool noMixer = false;               // run with antenna directly connected to SI4732 for testing
bool showTouchCoordinates = false;  // debug
int loopDelay = 0;                  // stabilize loop at not less than 10ms

// Waterfall
const int wfSensitivity = 50;                 // is a divider, more means less sensitivity
uint16_t* framebuffer1;                       // First framebuffer for waterfall, each is 240 pixels wide
uint16_t* framebuffer2;                       // Second framebuffer for waterfall
uint16_t stretchedX[FRAMEBUFFER_FULL_WIDTH];  // array audio waterfall to strech 240 to 331 pixels
int currentLine = 0;                          // Current line being written to
uint16_t newLine[FRAMEBUFFER_FULL_WIDTH];     // transfer buffer
bool smoothColorGradient = false;             // smooth = blue to white, otherwise blue to red
bool showAudioWaterfall = false;
uint8_t wabuf[26][90] = { 0 };  // audio waterfall miniwindow buffer
bool audiowf = false;
bool showPanorama = false;  //Panorama screen while squelch is closed


// Slow scan
unsigned long stationDuration = 3;  // Duration per station in seconds
unsigned long lastSwitchTime = 0;   // Tracks when the station was last switched
int currentStationIndex = 0;        // Current station index
bool stopScan = false;
bool nextStation = false;
bool lastStation = false;
bool slowScan = false;
bool syncStatus;                 // to store TSA sync status
bool stopState = false;          // stop when a SNR is detected
bool audioPeakDetected = false;  // signal found when audio peak detected
int audioPeakVal = 0;
int audioTreshold = 45;

//FFT
int Rpeak[256] = { 0 };
float pk;  // peak frequency
long peakVol = 0;
int FFTGain = 100;  //  adjustable through config menue
int dcOffset;       // dc offset of pin 36, should be around 1.65V

// touch tune
bool touchTune = false;
bool drawTrapezoid = false;
const long tSpan = 1000000;  //  1 MHz default span of touchtune

#ifdef AUDIO_SQUAREWAVE_PRESENT
// Pulse counter
volatile uint16_t frq = 0;                 // audiofrequency pulse counter
volatile unsigned long lastPulseTime = 0;  // Timestamp of the last rising edge
volatile unsigned long pulseInterval = 0;  // Time between rising edges
volatile bool newPulse = false;
#endif

// Sprites
extern const uint16_t But1[], But2[], But3[], But4[], But5[], But6[], But7[], But8[];  // sprite buttons in sprite.h
const uint16_t* buttonImages[] = { But1, But2, But3, But4, But5, But6, But7, But8 };
uint16_t buttonSelected = 4;  // 4-11


// Web
uint8_t imageSelector = 0;
bool swappedJPEG = false;
const char* ssid = "MMV2025";
const char* password = "Pekita#2020";
int yShift = 0;
int xShift = 0;
int reportSelector = 0;

// URL's for png and jpeg images. Change as you like. Resolution is limited to 480 * 320 pixels
const char* host1 = "https://www.sws.bom.gov.au/Images/HF%20Systems/Global%20HF/Fadeout%20Charts/rtalf.jpeg";
const char* host2 = "https://cdn.star.nesdis.noaa.gov/GOES19/ABI/FD/GEOCOLOR/339x339.jpg";
//const char* host3 = "https://www.sws.bom.gov.au/Images/SOLROT/noscript/SOL_IMG_2.jpg";
const char* host3 = "https://www.sws.bom.gov.au/Images/SOLROT/noscript/SOL_IMG_9.jpg";
const char* host4 = "https://cdn.star.nesdis.noaa.gov/GOES19/ABI/SECTOR/CAM/GEOCOLOR/1000x1000.jpg";
const char* host5 = "https://services.swpc.noaa.gov/images/animations/d-rap/global/d-rap/latest.png";

const char* al = "https://services.swpc.noaa.gov/products/alerts.json";     // Spaceweather alerts from NOAA
const char* fr = "https://services.swpc.noaa.gov/text/3-day-forecast.txt";  // Forecast from NOAA

//##########################################################################################################################//
// structures

// Original lists to load when CSV files missing.If a CSV file on the LittleFS is found, values will be loaded from the
// LittleFS. The LittleFS can be deleted or be overwritten by copying files from an SDcard, so editing of the parameters is possible without recompiling.


struct MemoInfo {
  int memoNumber;
  const char* memoName;
  long memoFreq;
  int memoModType;
  int memoBandwidth;
};


std::vector<MemoInfo> originalMemoList = {  // Any entry here will be carried over to LittleFS and can be exported to SDCard
  { 0, "Example", 146000000, 1, 1 },
  { 1, "--", 0, 1 },
  { 2, "--", 0, 1 },
  { 3, "--", 0, 1, 1 },
  { 4, "--", 0, 1, 1 },
  { 5, "--", 0, 1, 1 },
  { 6, "--", 0, 1, 1 },
  { 7, "--", 0, 1, 1 },
  { 8, "--", 0, 1, 1 },
  { 9, "--", 0, 1, 1 },
  { 10, "--", 0, 1, 1 },
  { 11, "--", 0, 1, 1 },
  { 12, "--", 0, 1, 1 },
  { 13, "--", 0, 1, 1 },
  { 14, "--", 0, 1, 1 },
  { 15, "--", 0, 1, 1 },
  { 16, "--", 0, 1, 1 },
  { 17, "--", 0, 1, 1 },
  { 18, "--", 0, 1, 1 },
  { 19, "--", 0, 1, 1 },
  { 20, "--", 0, 1, 1 },
  { 21, "--", 0, 1, 1 },
  { 22, "--", 0, 1, 1 },
  { 23, "--", 0, 1, 1 },
  { 24, "--", 0, 1, 1 },
  { 25, "--", 0, 1, 1 },
  { 26, "--", 0, 1, 1 },
  { 27, "--", 0, 1, 1 },
  { 28, "--", 0, 1, 1 },
  { 29, "--", 0, 1, 1 },
  { 30, "--", 0, 1, 1 },
  { 31, "--", 0, 1, 1 },
  { 32, "--", 0, 1, 1 },
  { 33, "--", 0, 1, 1 },
  { 34, "--", 0, 1, 1 },
  { 35, "--", 0, 1, 1 },
  { 36, "--", 0, 1, 1 },
  { 37, "--", 0, 1, 1 },
  { 38, "--", 0, 1, 1 },
  { 39, "--", 0, 1, 1 },
  { 40, "--", 0, 1, 1 },
  { 41, "--", 0, 1, 1 },
  { 42, "--", 0, 1, 1 },
  { 43, "--", 0, 1, 1 },
  { 44, "--", 0, 1, 1 },
  { 45, "--", 0, 1, 1 },
  { 46, "--", 0, 1, 1 },
  { 47, "--", 0, 1, 1 },
  { 48, "--", 0, 1, 1 },
  { 49, "--", 0, 1, 1 },
  { 50, "--", 0, 1, 1 },
  { 51, "--", 0, 1, 1 },
  { 52, "--", 0, 1, 1 },
  { 53, "--", 0, 1, 1 },
  { 54, "--", 0, 1, 1 },
  { 55, "--", 0, 1, 1 },
  { 56, "--", 0, 1, 1 },
  { 57, "--", 0, 1, 1 },
  { 58, "--", 0, 1, 1 },
  { 59, "--", 0, 1, 1 },
  { 60, "--", 0, 1, 1 },
  { 61, "--", 0, 1, 1 },
  { 62, "--", 0, 1, 1 },
  { 63, "--", 0, 1, 1 },
  { 64, "--", 0, 1, 1 },
  { 65, "--", 0, 1, 1 },
  { 66, "--", 0, 1, 1 },
  { 67, "--", 0, 1, 1 },
  { 68, "--", 0, 1, 1 },
  { 69, "--", 0, 1, 1 },
  { 70, "--", 0, 1, 1 },
  { 71, "--", 0, 1, 1 },
  { 72, "--", 0, 1, 1 },
  { 73, "--", 0, 1, 1 },
  { 74, "--", 0, 1, 1 },
  { 75, "--", 0, 1, 1 },
  { 76, "--", 0, 1, 1 },
  { 77, "--", 0, 1, 1 },
  { 78, "--", 0, 1, 1 },
  { 79, "--", 0, 1, 1 },
  { 80, "--", 0, 1, 1 },
  { 81, "--", 0, 1, 1 },
  { 82, "--", 0, 1, 1 },
  { 83, "--", 0, 1, 1 },
  { 84, "--", 0, 1, 1 },
  { 85, "--", 0, 1, 1 },
  { 86, "--", 0, 1, 1 },
  { 87, "--", 0, 1, 1 },
  { 88, "--", 0, 1, 1 },
  { 89, "--", 0, 1, 1 },
  { 90, "--", 0, 1, 1 },
  { 91, "--", 0, 1, 1 },
  { 92, "--", 0, 1, 1 },
  { 93, "--", 0, 1, 1 },
  { 94, "--", 0, 1, 1 },
  { 95, "--", 0, 1, 1 },
  { 96, "--", 0, 1, 1 },
  { 97, "--", 0, 1, 1 },
  { 98, "--", 0, 1, 1 },
  { 99, "--", 0, 1, 1 },
  { 100, "--", 0, 1, 1 },
  { 101, "--", 0, 1, 1 },
  { 102, "--", 0, 1, 1 },
  { 103, "--", 0, 1, 1 },
  { 104, "--", 0, 1, 1 },
  { 105, "--", 0, 1, 1 },
  { 106, "--", 0, 1, 1 },
  { 107, "--", 0, 1, 1 },
  { 108, "--", 0, 1, 1 },
  { 109, "--", 0, 1, 1 },
  { 110, "--", 0, 1, 1 },
  { 111, "--", 0, 1, 1 },
  { 112, "--", 0, 1, 1 },
  { 113, "--", 0, 1, 1 },
  { 114, "--", 0, 1, 1 },
  { 115, "--", 0, 1, 1 },
  { 116, "--", 0, 1, 1 },
  { 117, "--", 0, 1, 1 },
  { 118, "--", 0, 1, 1 },
  { 119, "--", 0, 1, 1 },
  { 120, "--", 0, 1, 1 },
  { 121, "--", 0, 1, 1 },
  { 122, "--", 0, 1, 1 },
  { 123, "--", 0, 1, 1 },
  { 124, "--", 0, 1, 1 },
  { 125, "--", 0, 1, 1 },
  { 126, "--", 0, 1, 1 },
  { 127, "--", 0, 1, 1 }
};



// === Global Vectors to modify lists when read from LittleFS===



struct SlowScanFrequencies {
  long freq;
  int modT;
  const char* desc;
};


struct BandInfo {
  const char* bandName;
  long startFreqKHz;
  long stopFreqKHz;
  bool isAmateurRadioBand;
  int bandNumber;
};





// Lists for storing data either LittleFS or original values copied over
std::vector<BandInfo> bandList;
std::vector<MemoInfo> memoList;
std::vector<SlowScanFrequencies> slowScanList;




// Original values to load when CSV files missing


std::vector<BandInfo> originalBandList = {  // Any entry here will be carried over to LittleFS and possibly SDCard
  { "LW", 150, 280, false, 1 },
  { "MW", 520, 1710, false, 2 },
  { "160M", 1800, 2000, true, 3 },       // Amateur radio band,
  { "120M", 2300, 2495, false, 4 },      // Broadcast SW band,
  { "90M", 3200, 3400, false, 5 },       // Broadcast SW band,
  { "80M", 3500, 4000, true, 6 },        // Amateur radio band,
  { "75M", 3900, 4000, false, 7 },       // Broadcast SW band,
  { "60M", 5250, 5450, true, 8 },        // Amateur radio band,
  { "49M", 5900, 6200, false, 9 },       // Broadcast SW band,
  { "41M", 7200, 7600, false, 10 },      // Broadcast SW band,
  { "40M", 7000, 7300, true, 11 },       // Amateur radio band,
  { "31M", 9500, 9900, false, 12 },      // Broadcast SW band,
  { "30M", 10100, 10150, true, 13 },     // Amateur radio band,
  { "25M", 11600, 12100, false, 14 },    // Broadcast SW band,
  { "EXIT", 0, 0, false, 15 },           // Placeholder for exit
  { "NEXT", 0, 1, false, 16 },           // Placeholder for next
  { "22M", 13570, 13870, false, 17 },    // Broadcast SW band,
  { "20M", 14000, 14350, true, 18 },     // Amateur radio band,
  { "19M", 15100, 15800, false, 19 },    // Broadcast SW band,
  { "17M", 18068, 18168, true, 20 },     // Amateur radio band,
  { "16M", 17550, 17900, false, 21 },    // Broadcast SW band,
  { "15M", 21000, 21450, true, 22 },     // Amateur radio band,
  { "13M", 21450, 21850, false, 22 },    // Broadcast SW band,
  { "12M", 24890, 24990, true, 24 },     // Amateur radio band,
  { "11M", 25670, 26100, false, 24 },    // Broadcast SW band,
  { "10M", 28000, 29700, true, 26 },     // Amateur radio band,
  { "CB", 26965, 27600, false, 27 },     // CB band,
  { "4M", 50000, 54000, true, 28 },      // Amateur radio band,
  { "AIR", 118000, 128000, false, 30 },  // Air band,
  { "2M", 144000, 148000, true, 29 },    // Amateur radio band,
  { "EXIT", 0, 0, false, 15 },           // Placeholder for exit
  { "NEXT", 0, 1, false, 16 },           // Placeholder for next
  { "70CM", 430000, 440000, true, 31 },  // Amateur radio band,
  { "AIR2", 119500, 123000, false, 32 },
  { "WBFM", 88000, 108000, false, 33 },
  { "USR", 10000, 10100, false, 34 },  // User defined bands
  { "USR", 15000, 15100, false, 35 },
  { "USR", 20000, 20100, false, 36 },
  { "USR", 25000, 25100, false, 37 },
  { "USR", 30000, 30100, false, 38 },
  { "USR", 35000, 35100, false, 39 },
  { "USR", 40000, 40100, false, 40 },
  { "USR", 45000, 45100, false, 41 },
  { "USR", 50000, 50100, false, 42 },
  { "USR", 55000, 55100, false, 43 },
  { "USR", 60000, 60100, false, 44 },
  { "USR", 65000, 65100, false, 45 },
  { "EXIT", 0, 0, false, 15 }

};


const int numBands = bandList.size();


std::vector<SlowScanFrequencies> originalSlowScanList = {  // Change as you like. Any entry here will be copied over to LittleFS

  { 910000, 1, "AM Religious station" },
  { 5980000, 1, "Radio Marti" },
  { 6030000, 1, "Radio Marti" },
  { 6030500, 1, "Radio Marti" },
  { 7355000, 1, "Radio Marti" },
  { 9265000, 1, "WINB" },
  { 9885000, 1, "Radio Taiwan" },
  { 9955000, 1, "WWV Time Signal" },
  { 11775000, 1, "Voice of America ??" },
  { 11775000, 1, "Radio Taiwan International" },
  { 11815000, 1, "Radio France Internationale" },
  { 12050000, 1, "BBC World Service" },
  { 14074000, 1, "FT8 20m" },
  { 14230000, 1, "SSTV 20m" },
  { 15440000, 1, "Chinese Voice" },
  { 17650000, 1, "?" },
  { 17795000, 1, "? " },
  { 27025000, 1, "CB " },
  { 28074000, 1, "FT8 10m" },
  { 28125000, 1, "10m Beacon" },
  { 28680000, 1, "SSTV 10m" },
  { 1100000, 1, "EMPTY" },
  { 1200000, 1, "EMPTY" },
  { 1300000, 1, "EMPTY" },
  { 127675000, 1, "ATIS" }
};
const int numStations = 25;  //fixed size, do not change



struct FreqMode {  // frequency history buffer
  uint32_t frequency;
  char mode;
};

FreqMode buffer[10];
uint8_t bufferIndex = 0;
bool bufferFull = false;  //history buffer full





//##########################################################################################################################//

//CODE START


//##########################################################################################################################//
/*

// Timer interrupt handler currently not in use
static bool IRAM_ATTR timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_ctx) {

  Serial_printf("%ld\n", millis());
  return true;  // return  need to yield at the end of ISR
}

//##########################################################################################################################//

void startHWT() {  // initiate the HW timer. Currently not in use, left code for future
  gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1 * 1000 * 1000,  // 1MHz resolution (1 tick = 1us)
  };

  // Create timer
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

  // Set alarm callback
  gptimer_event_callbacks_t cbs = {
    .on_alarm = timer_isr,
  };
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

  // Enable timer
  ESP_ERROR_CHECK(gptimer_enable(gptimer));

  // Configure alarm
  gptimer_alarm_config_t alarm_config = {
    .alarm_count = 120000 * 1000,  // 2min in us (at 1MHz resolution)
    .reload_count = 0,
    .flags = {
      .auto_reload_on_alarm = true,
    },
  };
  ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

  // Start timer
  ESP_ERROR_CHECK(gptimer_start(gptimer));

  Serial_println("HW timer started");
}

*/
//##########################################################################################################################//

void IRAM_ATTR RotaryEncFreq() {  // rotary encoder interrupt handler
  int encoderStatus = encoder.process();
  if (encoderStatus) {
    if (encoderStatus == DIR_CW)
      clw = true;
    else
      cclw = true;
  }
}

//##########################################################################################################################//

#ifdef AUDIO_SQUAREWAVE_PRESENT

void IRAM_ATTR handlePulse() {
  newPulse = true;  // Pulse handler for SSTV / RTTY freq counter
}

#endif

//##########################################################################################################################//

void setup() {

  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL, I2C_BUSSPEED);
  preferences.begin("data", false);  // preferences namespace is data. Needs to be loaded before bootscreen
  Serial.begin(115200);
  //startHWT();  // not used in this version
  bootScreen();
  etft.TTFdestination(&tft);
  etft.setTTFFont(Arial_13);
  //uint16_t calData[5] = { 292 3333, 198, 3539, 7 };  //example for display 4.0
  uint16_t calData[5] = { // values for display 3.5
                          (uint16_t)preferences.getInt("cal0", 323),
                          (uint16_t)preferences.getInt("cal1", 3305),
                          (uint16_t)preferences.getInt("cal2", 481),
                          (uint16_t)preferences.getInt("cal3", 2670),
                          (uint16_t)preferences.getInt("cal4", 5)
  };

  //calData[5] = {324, 3335, 427, 2870, 5 }; 
  tft.setTouch(calData);
  loadLastSettings();  // load settings from flash
  drawFrame();
  spriteBorder();


  pinMode(IF_INPUT_RELAY, OUTPUT);  // pin for IF relay and to enable tuner voltage
  pinMode(TUNER_AGC_PIN, OUTPUT);
  pinMode(NBFM_MUTE_PIN, OUTPUT);

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(MUTEPIN, OUTPUT);
  pinMode(AUDIO_INPUT_PIN, INPUT);  // Audio FFT
  pinMode(PULSE_PIN, INPUT);        // Audio square wave
  // pinMode(tinySA_PIN, OUTPUT);  // tinySA pin
  pinMode(IF_FILTER_BANDWIDTH_PIN, OUTPUT);  // IF filter

#ifdef NBFM_DEMODULATOR_PRESENT
  pinMode(TUNING_VOLTAGE_READ_PIN, INPUT);  // NBFM demodulator AFC read
#endif


  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), RotaryEncFreq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), RotaryEncFreq, CHANGE);
  SI5351_Init();
  loadLists();                    // load structures, use from LittleFS if exists
  createMemoInfoCSVIfNotExist();  //Create memoInfo on LittleFS if not there
  radioInit();
  Serial_printf("\nTouchcal: %d %d %d %d %d\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
  Serial_println(" DSP Wide Band Receiver ready to rock!\n");

#ifdef TINYSA_PRESENT
  Serial.setTimeout(5);  // do not wait, if the TSA does not answer quickly
  tinySAInit();
#endif
}

//##########################################################################################################################//


void loop() {

  encoderMoved();  // check whether encoder has moved

  if (FREQ != FREQ_OLD) {  // If FREQ changes, update Si5351A
    FREQCheck();           //check whether within FREQ range
    displayFREQ(FREQ);     // display new FREQ
    setLO();               // and tune it in
    resetSmeter = true;    // reset to zero
    autoloopBands();       // if autoloop is enabled frequency will stay within a selected band
    FREQ_OLD = FREQ;
  }
  getRSSIAndSNR();  // get RSSI (signalStrength) and SNR, valid for all functions in the main loop

  displaySTEP(false);  // check STEP, do not force update

  readSquelchPot(true);  // true = read and draw position circle
  setSquelch();
  fineTune();  // read frequency potentiometer

#ifdef TV_TUNER_PRESENT
  setTunerAGC();  // TV tuner AGC reduces tuner gain for better IP3, shows value as circle in the Tuner Gain tile
#endif

#ifdef SW_ATTENUATOR_PRESENT
  if (!TVTunerActive)
    setShortwaveAGC();  // TV tuner AGC reduces tuner gain for better IP3, shows value intile
#endif


  saveCurrentSettings();  // save settings after 1 minute if freq has not changed


  // functions that don't have to run in every loop cycle get called with fTrigger

  if (fTrigger % 3 == 0) {

    checkTouchCoordinates();  // this function takes long. Calling it 1 out of 3 loop cycles is suficient.

#ifdef NBFM_DEMODULATOR_PRESENT
    getDiscriminatorVoltage();  // display Tuning meter
#endif

#ifndef NBFM_DEMODULATOR_PRESENT
    if (dBm < 0 && (!scanMode) && showMeters)
      plotNeedle(signalStrength, 2);  // update the SMeter needle
#endif

    setMode();     //  set mode (tune or scan mode), must run after checkTouchCoordinates(). Needs a recent value of pressed,
    touchTuner();  // graphical frequency display, switches to selected FREQ when touched
    mainScreen();  // build main window
  }




  if (fTrigger % 8 == 0 && disableFFT == false) {  // spectrum eats a lot of processing resources


#ifdef SHOW_DEBUG_UTILITIES

    if (showAGCGraph) {
      if (!TVTunerActive)
        drawByteTrace(127 - signalStrength, 127);  // shortwave AGC trace
      else
        drawByteTrace(agcVal, 70);  // 70 = max attenuation, tuner AGC trace
    }
#endif

    PanoramaScanner();  // scans +-500 KHz around current frequency when squelch is closed
    audioSpectrum();
    if (enableAnimations && funEnabled)
      pacM(true);
    displaySmeterBar(2);  // // update the SMeter bar, so that it moves fluently btw reads
    if (showMeters) {
      int vol = peakVol / 1000;
      if (!audioMuted)
        plotNeedle2(vol, 3);
      else
        plotNeedle2(1, 0);
    }
  }

#ifdef TINYSA_PRESENT
  if (fTrigger % 15 == 0)
    synctinySA();  // tinySA synchronisation when in RF mode
#endif



  if (fTrigger % 25 == 0) {
    clockDisplay();  // clock in lower left corner
  }


  if (fTrigger % 50 == 0) {

    if (syncEnabled == false || tinySA_RF_Mode == false || tinySAfound == false)
      calculateAndDisplaySignalStrength();  // Smeter about 3x second, alternatively gets called from convertTodBm(),when tinySA provides signal level

    slowTaskHandler();  // run tasks with a long period
    fTrigger = 0;
    disableFFT = false;
  }


  fTrigger++;  // function trigger counter, triggers functions that should not run every loop cycle

  if (loopDelay)  // prevents the main loop from running faster than 10ms since several functions depend on the speed of it.
    delay(loopDelay);
}
//##########################################################################################################################//


void encoderMoved() {

  use1MhzEncoderStep();  // change FREQ in 1MHZ segments if encoder pressed

  if (encLockedtoSynth) {  // if encoder is used to change FREQ
    if (clw) {
      disableFFT = true;  // disable spectrum analyzer when moving encoder to make tuning more responsive
      enableAnimations = false;
      FREQ += STEP;
      clw = false;
    } else if (cclw) {
      disableFFT = true;
      enableAnimations = false;
      FREQ -= STEP;
      cclw = false;
    }
  }
}
//##########################################################################################################################//

void FREQCheck() {  // error check and eliminate rounding error when keypad is used for frequency input

  if (FREQ < MIN_FREQ)
    FREQ = MIN_FREQ;

  if (FREQ > MAX_FREQ)
    FREQ = MAX_FREQ;


  if (FREQ % 10 >= 8 || FREQ % 10 <= 2) {  // eliminate frequency rounding errors. Keypad entry uses floats, so rounding errors may come up
    long lowerEnd = (FREQ / 10) * 10;
    long upperEnd = lowerEnd + 10;

    if (FREQ % 10 >= 9) {
      FREQ = upperEnd;  // Round up
    } else {
      FREQ = lowerEnd;  // Round down
    }
  }
}

//##########################################################################################################################//

//Frequency display, updates only the section that changes, not the entire frequency display to reduce flicker when tuning.
void displayFREQ(long freq) {
  static uint32_t oldFreq = 0;
  const uint16_t xPos = 8;
  const uint16_t yPos = 10;

  if (showFREQ == false)
    return;


  long old_Mhz = oldFreq / 1000000;
  long old_Khz = (oldFreq - (old_Mhz * 1000000)) / 1000;
  long old_Hz = (oldFreq - (old_Mhz * 1000000)) - old_Khz * 1000;

  long new_Mhz = freq / 1000000;
  long new_Khz = (freq - (new_Mhz * 1000000)) / 1000;
  long new_Hz = (freq - (new_Mhz * 1000000)) - new_Khz * 1000;

  colorSelector();  // select display color


  if (!sevenSeg) {  // Arial font

    if (oldFreq != freq) {  // overwrite digits that have changed
      if (old_Mhz != new_Mhz)
        tft.fillRect(xPos, yPos, 155, 34, TFT_BLACK);

      if (old_Khz != new_Khz)
        tft.fillRect(xPos + 75, yPos, 87, 34, TFT_BLACK);

      if (old_Hz != new_Hz)
        tft.fillRect(xPos + 155, yPos, 67, 34, TFT_BLACK);
    }
    etft.setTTFFont(Arial_32);


    if (new_Mhz >= 10) {  // position cursor
      etft.setCursor(xPos, yPos);
    } else if (new_Mhz >= 1) {
      etft.setCursor(xPos + 20, yPos);
    } else {
      etft.setCursor(xPos + 50, yPos);
    }
    etft.printf("%3.0ld", new_Mhz);

    etft.setCursor(xPos + 75, yPos);
    etft.printf(".%03ld.", new_Khz);
    etft.setTTFFont(Arial_20);
    etft.printf("%03ld", new_Hz);
    etft.setTTFFont(Arial_32);
    etft.setCursor(xPos + 208, yPos);
    etft.print(" MHz");

  }

  else {

    // 7 segment font
    if (oldFreq != freq) {  // fill only the digits that have changed
      if (old_Mhz != new_Mhz)
        tft.fillRect(xPos, yPos - 3, 90, 37, TFT_BLACK);

      if (old_Khz != new_Khz)
        tft.fillRect(xPos + 95, yPos - 3, 93, 37, TFT_BLACK);

      if (old_Hz != new_Hz)
        tft.fillRect(xPos + 190, yPos - 3, 85, 37, TFT_BLACK);
    }

    tft.setTextSize(0);
    tft.setFreeFont(&DSEG7_Classic_Mini_Regular_34);
    if (new_Mhz >= 100) {
      tft.setCursor(xPos, yPos + 31);
    }

    if (new_Mhz >= 10 && new_Mhz < 100) {
      tft.setCursor(xPos + 20, yPos + 31);
    }
    if (new_Mhz >= 1 && new_Mhz < 10) {
      tft.setCursor(xPos + 45, yPos + 31);
    }
    tft.printf("%3.0ld.", new_Mhz);

    tft.setCursor(xPos + 101, yPos + 31);
    tft.printf("%03ld.%03ld", new_Khz, new_Hz);
    tft.setFreeFont(NULL);
    tft.setTextSize(2);
    tft.setTextColor(textColor);
  }

  oldFreq = freq;
}

//##########################################################################################################################//
void colorSelector() {  // set frequency display colors

#ifdef TV_TUNER_PRESENT
  if (FREQ < SHORTWAVE_MODE_UPPER_LIMIT) {
    etft.setTextColor(TFT_GREEN);  // shortwave normal color
    tft.setTextColor(TFT_GREEN);
  } else {
    etft.setTextColor(TFT_SKYBLUE);  //VHF/UHF normal color
    tft.setTextColor(TFT_SKYBLUE);
  }

#endif

#ifndef TV_TUNER_PRESENT
  etft.setTextColor(TFT_GREEN);  // shortwave normal color
  tft.setTextColor(TFT_GREEN);
#endif

  if (SI4735WBFMTune) {  //WBFM
    etft.setTextColor(TFT_MAGENTA);
    tft.setTextColor(TFT_MAGENTA);
    return;
  }


  if (noMixer) {
    etft.setTextColor(TFT_YELLOW);
    tft.setTextColor(TFT_YELLOW);
  }
}

//##########################################################################################################################//
//STEP display in upper right corner
void displaySTEP(bool update) {
  static uint32_t OLD_STEP;



  if (OLD_STEP == STEP && !update)
    return;
  else
    OLD_STEP = STEP;

  tft.setTextColor(textColor);

  if (use1MHzSteps == true)
    tft.setTextColor(TFT_RED);


  uint16_t xPos = 345;
  uint16_t yPos = 26;

  // Clear previous text
  tft.fillRect(xPos, yPos, 130, 20, TFT_BLACK);
  tft.setCursor(xPos, yPos);
  tft.setTextSize(2);

  if (modType == WBFM) {
    tft.print("Step:100KHz");
    return;
  }

  if (STEP < 1000)
    tft.printf("Step:%ldHz", STEP);
  if (STEP >= 1000 && STEP < 1000000)
    tft.printf("Step:%ldKHz", STEP / 1000);
  if (STEP >= 1000000)
    tft.printf("Step:%ldMHz", STEP / 1000000);
}

//##########################################################################################################################//

//STEP switching


void setSTEP() {

  if (modType == WBFM)  // fixed 100Khz
    return;

  int pos = 0;
  uint32_t AMStepSize[13] = { 250, 500, 1000, 2500, 5000, 6000, 8333, 9000, 10000, 100000, 1000000, 10000000, 0 };
  uint32_t SSBStepSize[13] = { 10, 25, 50, 100, 250, 500, 1000, 5000, 10000, 100000, 1000000, 10000000, 0 };
  uint32_t FMStepSize[13] = { 500, 1000, 2500, 5000, 6250, 10000, 12500, 25000, 50000, 100000, 1000000, 10000000, 0 };
  const char* AMLabels[] = { "250Hz", "500Hz", "1KHz", "2.5KH", "5KHz", "6KHz", "8.3KHz", "9KHz", "10KHz", "100KHz", "1MHz", "10MHz" };
  const char* SSBLabels[] = { "10Hz", "25Hz", "50Hz", "100Hz", "250Hz", "500Hz", "1KHz", "5KHz", "10KHz", "100KHz", "1MHz", "10MHz" };
  const char* FMLabels[] = { "500Hz", "1K", "2.5K", "5K", "6.25K", "10K", "12.5K", "25K", "50K", "100K", "1MHz", "10MHz" };

  if (pressed) {

    draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


    int positions[][2] = {
      { 19, 141 }, { 105, 141 }, { 185, 141 }, { 265, 141 }, { 19, 198 }, { 105, 198 }, { 185, 198 }, { 265, 198 }, { 19, 255 }, { 96, 255 }, { 185, 255 }, { 260, 255 }
    };



    etft.setTTFFont(Arial_14);
    etft.setTextColor(TFT_GREEN);

    for (int i = 0; i < 12; i++) {
      etft.setCursor(positions[i][0], positions[i][1]);
      if (modType == AM)
        etft.print(AMLabels[i]);
      if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
        etft.print(SSBLabels[i]);
      if (modType == NBFM)
        etft.print(FMLabels[i]);
    }

    etft.setTextColor(textColor);

    tDoublePress();  // wait for touch

    pressed = get_Touch();

    if (ty > 300 || ty < 120 || tx > 345)  // nothing there
      return;

    column = tx / HorSpacing;
    row = 1 + ((ty - 20) / vTouchSpacing);


    if (row < 2) {  // outside of area
      pressed = 0;
      tx = 0;
      ty = 0;
      return;
    }

    if (row == 2)
      pos = column;
    if (row == 3)
      pos = column + 4;
    if (row == 4)
      pos = column + 8;


    if (modType == AM)
      STEP = AMStepSize[pos];
    if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
      STEP = SSBStepSize[pos];
    if (modType == NBFM)
      STEP = FMStepSize[pos];
  }



  tRel();
  tx = ty = pressed = 0;
  displaySTEP(false);

  roundFreqToSTEP();

  redrawMainScreen = true;
  mainScreen();
}
//##########################################################################################################################//

void roundFreqToSTEP() {  // round  FREQ up or down to the next STEP when STEP or modulation gets changed

  if (roundToStep == false)
    return;


  if (modType == AM) {             // Not SSB orNBFM
    if ((FREQ / 1000) % 10 > 5) {  // round 6 - 9 up
      FREQ /= STEP;
      FREQ *= STEP;
      FREQ += STEP;
    }

    if ((FREQ / 1000) % 10 < 5)  // round 1 - 4 down
    {
      FREQ /= STEP;
      FREQ *= STEP;
    }
  }
}

//##########################################################################################################################//



void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color1, uint32_t color2) {  // draws buttons as plain rectangles or sprites

  if (altStyle) {  //draw plain buttons

    tft.fillRectVGradient(x, y + 4, w, h / 2, color1, color2);
    tft.fillRectVGradient(x, y + (h / 2) + 4, w, h / 2, color2, color1);
    if (w < 300)  // silver frame around buttons
      tft.drawRect(x, y + 4, w, h, TFT_LIGHTGREY);
  }

  else {  //draw sprites, use height as selection criteria

    if (h == 50)

      tft.pushImage(x, y + 4, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[buttonSelected]);  // draw selected button sprite


    if (h == 78)

      tft.pushImage(x, y, 130, 80, (uint16_t*)Bigbutton);  // draw big green neon sprite for big buttons
  }
}
//##########################################################################################################################//

void draw12Buttons(uint16_t color1, uint16_t color2) {  // draws 3 rows with 4 buttons each


  /*
Button positions:
1	8	121
2	91	121
3	174	121
4	257	121
5	8	178
6	91	178
7	174	178
8	257	178
9	8	235
10	91	235
11	174	235
12	257	235

*/


  int hStart = 8;
  int ySpace = 57;   //57
  int yStart = 121;  //121
  tft.fillRect(2, 48, 337, 8, TFT_BLACK);
  tft.fillRect(2, 288, 337, 4, TFT_BLACK);  // overwrite remains of previous sprites
  for (int j = 0; j < 4; j++) {
    drawButton(hStart + j * 83, yStart + 2 * ySpace, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    drawButton(hStart + j * 83, yStart + ySpace, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    drawButton(hStart + j * 83, yStart, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
  }
}
//##########################################################################################################################//
void draw16Buttons() {  // draws 4 rows with 4 buttons each


  /*
Button positions:
1	6	58
2	89	58
3	172	58
4	255	58
5	6	116
6	89	116
7	172	116
8	255	116
9	6	174
10	89	174
11	172	174
12	255	174
13	6	232
14	89	232
15	172	232
16	255	232
*/

  uint16_t yb = 58;
  int h = 6;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      drawButton(h + j * 83, (i + 1) * yb, 78, 50, TFT_DARKGREY, TFT_BTNCTR);
    }
  }
}

//##########################################################################################################################//


void calculateAndDisplaySignalStrength() {  // gets called from main loop, or optionally from the tinySA handler
  // needed for squelch even when tinySA is  signal strength source

  if (signalStrength < 8 && !TVTunerActive) {
    //  Shortwave modes start with signalStrength 6 (mixer noise). Reduce
    signalStrength -= 5;
    SNR = 0;
  }


  //Serial_printf("SignalStrength:%d SNR:%d\n", signalStrength, SNR);

  if (!syncEnabled || !tinySA_RF_Mode || !TSAdBm) {  // calculate dBm from RSSI when no data from tinySA
    // Convert dBµV to dBm      substract gain from LNA
    dBm = signalStrength - 107 - RFGainCorrection;
  }


  if (syncEnabled && TSAdBm) {
    tft.setTextColor(TFT_GREEN);
    dBm = TSAdBm;  //take dBm from TSA marker 1
  }

  if (TVTunerActive && !TSAdBm && modType != WBFM) {
    dBm -= tunerGain - ((180 - initialGain) / 4);  // this roughly corrects if the tuner gain gets already reduced without siggnal to reduce crossmodulation
    dBm += tunerdBmcorrection;


    //DAC level vs tuner attenuation: 255 = 0dB, 159  = - 10dB, 145 = -20dB, 129 = -30dB, 105 = -40dB attenuation. Attenuation kicking in at around 180
  }

  double power_watts = pow(10.0, dBm / 10.0) * 1e-3;
  double voltage_rms = sqrt(power_watts * 50.0);  // 50Ohms
  microVolts = voltage_rms * 1e6;


  tft.fillRect(330, 4, 145, 22, TFT_BLACK);
  if (TSAdBm) {
    tft.setCursor(345, 8);

    if (microVolts < 10000)
      tft.printf("%5.1fuV", microVolts);
    else
      tft.printf("%6.0fuV", microVolts);
  }


  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);

  tft.fillRect(288, 62, 49, 28, TFT_BLACK);

  tft.setCursor(295, 64);
  tft.printf("%ddBm", dBm);
  tft.setCursor(300, 83);

  if (SNRSquelch)
    tft.setTextColor(TFT_WHITE);

  tft.printf("SNR:%d", SNR);
  tft.setTextSize(2);
}


//##########################################################################################################################//

void displaySmeterBar(int updateRate) {

  const int maxX = 300;
  int newSpoint = 0;

  int spoint = (int)calculateSpoint(microVolts);

  static int oldSpoint = spoint;

  if (spoint > maxX)  // limit
    spoint = maxX;


  if (spoint < oldSpoint) {  // When signal is decreasing
                             // Calculate new endpoint (not below current spoint)

    int diff = oldSpoint - spoint;

    if (diff < 10 * updateRate)
      newSpoint = oldSpoint - updateRate;

    else
      newSpoint = oldSpoint - 5 * updateRate;


    if (newSpoint < spoint)
      newSpoint = spoint;


    tft.fillRect(newSpoint, Ysmtr + 13, maxX - newSpoint, 18, TFT_BLACK);

    oldSpoint = newSpoint;  // Update the position
  }
  if (resetSmeter) {  // when frequency changes, do not decay slowly
    oldSpoint = 0;
    resetSmeter = false;
  }


  if (spoint > oldSpoint) {

    int diff = spoint - oldSpoint;

    if (diff > 10 * updateRate)
      oldSpoint += 10 * updateRate;  // start fast if the difference is big

    else
      oldSpoint += updateRate;  // slow down when getting close


    int tik = 0;
    uint8_t red, green, blue;


    while (tik < oldSpoint) {  // now start to draw the bars
      if (tik > 180) {
        // Fade smoothly from green (0,63,0) to white (31,63,31)
        float progress = (float)(tik - 180) / (oldSpoint - 180);  // 0.0 to 1.0
        red = (int)(31 * progress);                               // Red fades in (0→31)
        green = 63;                                               // Green  max (63)
        blue = (int)(31 * progress);                              // Blue fades in (0→31)
      } else if (tik <= 90) {
        // red → orange → yellow
        red = 31;
        green = (63 * tik) / 90;
        blue = 0;
      } else if (tik <= 180) {
        //yellow → green
        red = 31 - (31 * (tik - 90)) / 90;
        green = 63;
        blue = 0;
      }

      // Pack into RGB565

      uint16_t rgb565;
      if (tik % 3 == 0)
        rgb565 = 0;
      else
        rgb565 = (red << 11) | (green << 5) | blue;

      tft.fillRect(Xsmtr + 3 + tik, Ysmtr + 23 - tik / 25, 1, 4 + tik / 25, rgb565);
      tik++;
    }

  }  // endif (spoint > oldSpoint)
}

//##########################################################################################################################//

float calculateSpoint(float vRef) {
  struct Point {
    float v;
    float s;
  };

  const Point points[] = {
    { 0.0, 1 },
    { 0.1, 10 },
    { 0.2, 20 },
    { 0.4, 40 },
    { 0.8, 60 },
    { 1.6, 80 },
    { 3.2, 120 },
    { 6.4, 140 },
    { 12.8, 160 },
    { 25.6, 180 },
    { 51.2, 200 },
    { 160, 230 },
    { 500, 260 },
    { 501, 280 }  // anything > 500
  };

  const int numPoints = sizeof(points) / sizeof(points[0]);

  if (vRef <= points[0].v) return points[0].s;

  for (int i = 1; i < numPoints; i++) {
    if (vRef <= points[i].v) {
      // Linear interpolation between this point and previous
      float t = (vRef - points[i - 1].v) / (points[i].v - points[i - 1].v);
      return points[i - 1].s + t * (points[i].s - points[i - 1].s);
    }
  }

  return points[numPoints - 1].s;
}

//##########################################################################################################################//
void DrawSmeterScale() {

  String IStr;
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);

  for (int i = 0; i < 10; i++) {
    IStr = String(i);
    tft.setCursor((Xsmtr + 5 + (i * 16) * 1.3), Ysmtr + 5);
    tft.print(i);
  }

  for (int i = 1; i < 5; i++) {
    IStr = String(i * 10);
    tft.setCursor((Xsmtr + 165 + (i * 15) * 1.3), Ysmtr + 5);
    if ((i == 2) or (i == 4) or (i == 6)) {
      tft.print(" +");
      tft.print(i * 10);
    }
  }

  tft.setTextSize(2);
}
//##########################################################################################################################//

void slowTaskHandler() {

  static unsigned int ctr = 0;
  static unsigned long tmr = 0;
  const unsigned int tInt = 1000;  // 1000 ms
  static bool done = false;
  if (displayDebugInfo)
    loopTimer();


  if (millis() - tmr >= tInt) {
    tmr = millis();
    ctr++;
  }
  //Serial.printf("RSSI:%ddBuV Frequency:%ld\n", signalStrength, FREQ);

  if (!enableAnimations && done) {  // enableAnimations was set to false by encoder or keypress
    ctr = 0;
    done = false;
    if (funEnabled)
      redrawMainScreen = true;
    tRel();
    tx = ty = pressed = 0;
  }

  if (ctr > TIME_UNTIL_ANIMATIONS) {  // logic to start  pacm after TIME_UNTIL_ANIMATIONS
    enableAnimations = true;
    done = true;
    if (showAudioWaterfall)  // logic to start audio eaterfall after TIME_UNTIL_ANIMATIONS
      audioSpectrum256();
  }
}
//##########################################################################################################################//
void loopTimer() {  // displays loop time and stops if from falling below 10ms

  long now;
  static long before;
  static long oldCycleTime = -1;
  long cycleTime = 0;

  now = micros();  // measure loop time
  cycleTime = now - before;
  before = now;

  if (oldCycleTime == cycleTime)
    return;
  else
    oldCycleTime = cycleTime;

  etft.setTTFFont(Arial_9);
  etft.fillRect(0, 308, 130, 12, TFT_BLACK);
  etft.setCursor(0, 308);
  etft.setTextColor(TFT_GREEN);
  cycleTime /= 50000l;  // convert microseconds to seconds and divide by 50 since it gets called only 1 out of 50 loops
  etft.printf("LOOP:%ldms", cycleTime);


  if (cycleTime < 10)  // stabilize the loop at minimum 10ms , otherwise functions that depend on the loop time may run too fast
    loopDelay++;

  if (cycleTime > 10)
    loopDelay--;

  if (loopDelay < 0)
    loopDelay = 0;

  etft.setTextColor(textColor);
}

//##########################################################################################################################//
void readSquelchPot(bool draw) {  // reads value of squelch potentiometer and if draw is true displays a circle showing squelch adjustment

  static int oldPot = 0;
  const int div = 31;
  int val;
  int pot;


  for (int l = 0; l < 35; l++)
    val = analogRead(SQUELCH_POT);
  pot = val / div;  // oversample and  set range

  if (pot <= oldPot - 2 || pot >= oldPot + 2 || redrawMainScreen) {  // eliminate A/D convrter noise, update only when pot turned

    disableFFT = true;  //temporarily disable spectrum analyzer to  read ADC without interruption

    currentSquelch = pot - 10;  // this sets currentSquelch to a value from roughly -10 to 127. A negative value is required to open squelch if RSSI == 0

    if (!draw)  // do not draw squelch position circle (when squelch used in memo functions)
      return;

    tft.fillRect(oldPot * 2 + 8, 53, 14, 10, TFT_BLACK);
    tft.drawFastHLine(0, 57, 340, TFT_GRID);


    if (signalStrength > currentSquelch)
      tft.fillCircle(pot * 2 + 12, 57, 4, TFT_GREEN);
    else
      tft.fillCircle(pot * 2 + 12, 57, 4, TFT_MAGENTA);
    oldPot = pot;
  }
}

//##########################################################################################################################//
void setSquelch() {

  uint32_t open, duration;
  static uint32_t lastopen = 0;
  static int16_t stutter = 0;


  if (!SNRSquelch) {  // use RSSI only as open/close criteria

    if ((signalStrength > currentSquelch) && audioMuted) {
      audioMuted = false;
      si4735.setAudioMute(false);
      si4735.setHardwareAudioMute(false);
    } else if ((signalStrength < currentSquelch) && !audioMuted) {
      audioMuted = true;
      si4735.setAudioMute(true);  // set both software and hardware mute for better noise supression
      dcOffset = analogRead(AUDIO_INPUT_PIN);
      si4735.setHardwareAudioMute(true);
    }
  }

  else {  //SNRSquelch uses either SNR or RSSI to open the squelch

    if (signalStrength < 8 && !TVTunerActive)  // eliminate a "hanging" SNR due to mixer noise when in SW mode
      SNR = 0;


    if (stutter < 0)
      stutter = 0;

    if ((SNR || (signalStrength > currentSquelch + stutter)) && audioMuted) {
      audioMuted = false;
      si4735.setAudioMute(false);
      si4735.setHardwareAudioMute(false);
      open = millis();
      duration = open - lastopen;

      if (duration < 200)  // squelch is stuttering, make it less sensible
        stutter += 4;
      if (duration > 1000)  // constant signal, reset squelch
        stutter = 0;

      // Serial_printf("stutter %d open - lastopen%ld\n", stutter, duration);
      lastopen = open;


    }

    else if ((!SNR && (signalStrength < currentSquelch)) && !audioMuted) {
      si4735.setAudioMute(true);
      si4735.setHardwareAudioMute(true);
      audioMuted = true;
    }
  }
}
//##########################################################################################################################//

void displayText(int x, int y, int length, int height, const char* text) {  // helper to display text
  tft.fillRect(x, y, length, height, TFT_BLACK);
  tft.setCursor(x, y);
  tft.print(text);
}

//##########################################################################################################################//


void indicatorTouch() {  // Change step, modulation, bandwidth, AGC by touching the indicator areas

  int upper_y = 65;
  int lower_y = 115;

  if (ty < 35 || !pressed)  // outside of area
    return;

  // Toggle audio waterfall
  if (tx < 340 && ty > 35 && ty < 55) {
    showAudioWaterfall = !showAudioWaterfall;

    if (showAudioWaterfall) {
      tRel();
      audioSpectrum256();
    }
    pressed = 0;
    tx = 0;
    ty = 0;
  }

  if (ty > upper_y && ty < lower_y) {


    // Toggle AM, LSB, USB
    if (tx > 3 && tx < 45) {
      modType++;

      if (modType > 4) {
        modType = 1;
      }
      loadSi4735parameters();
      printModulation();
      printBandWidth();
      tRel();
      pressed = 0;
      tx = 0;
      ty = 0;
    }


    // Bandwidth down
    if (tx > 65 && tx < 108) {
      setBandwidth(-1);
      tRel();
    }

    // Bandwidth up
    if (tx > 108 && tx < 155) {
      setBandwidth(1);
      tRel();
    }

    if (tx > 173 && tx < 215) {
      setIFBandwidth();
      tRel();
    }

    // AGC/ATT down
    if (tx > 235 && tx < 260)
      setAGCMode(-1);

    // AGC/ATT up
    if (tx > 300 && tx < 325)
      setAGCMode(1);
  }
}


//##########################################################################################################################//

void getRSSIAndSNR() {

  si4735.getCurrentReceivedSignalQuality(0);
  SNR = si4735.getCurrentSNR();

  si4735.getCurrentReceivedSignalQuality(0);
  signalStrength = si4735.getCurrentRSSI();
}

//##########################################################################################################################//

void sineTone(uint32_t frequency, uint32_t duration) {
  // this needs an empty file named build_opt.h in the sketch directory, otherwise it slows the main loop down!
  dac2.enable();
  dac2.outputCW(frequency);
  delay(duration);
  dac2.disable();
}

//##########################################################################################################################//
#ifdef NBFM_DEMODULATOR_PRESENT

void getDiscriminatorVoltage() {


  if (showMeters) {

    if (modType == WBFM && !scanMode) {  // plot RSSI instead
      plotNeedle(signalStrength, 2);
      return;
    }


    afcVoltage = (analogRead(TUNING_VOLTAGE_READ_PIN) / 5) - discriminatorZero;  // use discriminatorZero to adjust afc meter to zero when tuned

    if (afcEnable)
      setAFC(afcVoltage);

    if (audioMuted)
      afcVoltage = 0;


    //Serial_printf(" AFC voltage %d\n", afcVoltage);

    if (TVTunerActive)
      plotNeedle(50 - afcVoltage, 2);  // valid for crystal freq > 21.4 MHz, otherwise swap - and +
    else
      plotNeedle(50 + afcVoltage, 2);  //
  }

  else
    return;
}
//##########################################################################################################################//
void showAFCStatus() {


  if (!afcEnable) {
    si5351.set_correction(SI5351calib, SI5351_PLL_INPUT_XO);  // reset correction
    tft.setTextColor(TFT_BLACK);
    tft.fillCircle(460, 115, 4, TFT_BLACK);
    tft.fillCircle(365, 115, 4, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(350, 65);
    tft.print("AFC");
    tft.setTextColor(textColor);
    tft.setTextSize(2);

  }

  else {

    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(1);
    tft.setCursor(350, 65);
    tft.print("AFC");
    tft.setTextColor(textColor);
    tft.setTextSize(2);
  }
}
//##########################################################################################################################//

void setAFC(int afcVoltage) {  // only for NBFM hardware

  const int AFCStep = 5000;
  static long shift = 0;


  tft.setTextColor(TFT_GREEN);  // need to reprint sice this gets overwritten when the meter gets redrawn
  tft.setTextSize(1);
  tft.setCursor(350, 65);
  tft.print("AFC");
  tft.setTextColor(textColor);
  tft.setTextSize(2);



  if (afcVoltage > 5 && afcVoltage < 70 && signalStrength > 20) {  //signal above FREQ
    shift -= AFCStep;

    if (showMeters) {
      tft.fillCircle(460, 115, 4, TFT_BLACK);
      tft.fillCircle(365, 115, 4, TFT_RED);
    }
  }

  if (afcVoltage < -5 && afcVoltage > -70 && signalStrength > 20) {  // signal below FREQ
    shift += AFCStep;

    if (showMeters) {
      tft.fillCircle(365, 115, 4, TFT_BLACK);
      tft.fillCircle(460, 115, 4, TFT_RED);
    }
  }


  if (shift < -100000)  // low limit
    shift = -100000;


  if (shift > 100000)  //high limit
    shift = 100000;


  if ((signalStrength <= 20 || audioMuted) && showMeters) {
    shift = 0;
    tft.fillCircle(460, 115, 4, TFT_BLACK);
    tft.fillCircle(365, 115, 4, TFT_BLACK);
  }


  if (afcVoltage > -5 && afcVoltage < 5) {
    tft.fillCircle(460, 115, 4, TFT_BLACK);
    tft.fillCircle(365, 115, 4, TFT_BLACK);
  }

  //Serial_printf("%ld %d  %d\n", shift, afcVoltage, signalStrength);


  si5351.set_correction(SI5351calib + shift, SI5351_PLL_INPUT_XO);  // use SI5351correction instead of adjusting FREQ to avoid running away
}

#endif

//##########################################################################################################################//

void hideBigBtns() {
  tft.fillRect(345, 48, 130, 242, TFT_BLACK);
}


//##########################################################################################################################//

void setShortwaveAGC() { // this will drive the SW attenuator and act as an AGC. SWMinAttn determines the minimum attenuation.

  static unsigned char oldRSSI = 0;
 
  int newRSSI = 8 * signalStrength - 200;
  newRSSI = constrain(newRSSI, 0, 255);

  if (newRSSI > oldRSSI) {  // increase attenuation gradually until a balance is found
    SWAttn++;
    oldRSSI++;
  }

  if (newRSSI < oldRSSI) {  // decrease attenuation gradually until a balance is found
    SWAttn--;
    oldRSSI--;
  }

if(SWAttn < SWMinAttn)
   SWAttn = SWMinAttn;

if(attnOFF)
   SWAttn = 0;


    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.fillRect(145, 140, 19, 9, TFT_BLACK);
    tft.setCursor(145, 140);
    tft.printf("%d", signalStrength);

    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(1);
    tft.fillRect(145, 157, 19, 9, TFT_BLACK);
    tft.setCursor(145, 158);
    tft.printf("%d", SWAttn);
    tft.setTextSize(2);
    tft.setTextColor(textColor);


  if (oldRSSI != newRSSI)
    dac1.outputVoltage((uint8_t)SWAttn);  // set the dac
}

//##########################################################################################################################//


void setRFAttenuatorMinAttn() {  //Min. attenuation of the RF attenuator
                                
    uint8_t oldSWMinAttn = 0;

    encLockedtoSynth = false;

    tft.setTextColor(textColor);
    tft.fillRect(10, 60, 328, 64, TFT_BLACK);
    tft.setCursor(10, 63);
    tft.print("Set minimum attenuation ");
    tft.setCursor(10, 83);
    tft.print("Press encoder to leave.");
    tft.setCursor(10, 104);
    tft.printf("Min Attenuator level:%d", SWMinAttn); 


    while (digitalRead(ENCODER_BUTTON) == HIGH) {

    delay(20);
      if (clw)
        SWMinAttn += 5;
      if (cclw)
       SWMinAttn -= 5;

  if (SWMinAttn <5) {
       attnOFF = true;
     }
     else {
       attnOFF = false;
     }


      if (oldSWMinAttn != SWMinAttn) {
        tft.fillRect(10, 104, 323, 20, TFT_BLACK);
        tft.setCursor(10, 104);
        
        if (attnOFF)
         tft.print("Attenuator OFF!           ");  
        else
          tft.printf("Min Attenuator level:%d", SWMinAttn);
        oldSWMinAttn = SWMinAttn;
      }

          
      dac1.outputVoltage((uint8_t) SWMinAttn);  // set the dac
      clw = false;
      cclw = false;
    }

    while (digitalRead(ENCODER_BUTTON) == LOW)
      ;
    
    delay(200);
    encLockedtoSynth = true;

  }
