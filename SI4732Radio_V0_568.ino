//##########################################################################################################################//

//IMPORTANT: To compile, select the "ESP32 Dev Module" and the "No OTA (2MB APP, 2MB SPIFFS)" partition scheme.

// Comment/uncomment compiler directives according to your hardware:

#define TV_TUNER_PRESENT  // If this option is commented out, a shortwave receiver version will compile. In this case max. FREQ = 50MHz.

#define AUDIO_SQUAREWAVE_PRESENT  // Audio squarewave present on GPIO39 for SSTV and RTTY decoding. Experimental.

#define FAST_TOUCH_HANDLER  // Invokes a faster touch handler with reduced sampling. Could cause spurious errors if the touchscreen is worn out, but increases speed significantly. No problems so far.

#define SHOW_DEBUG_UTILITIES  // Will show Debug utilities panel. Contains helper functions and status messages.

#define NBFM_DEMODULATOR_PRESENT  // Uses an additional MC3361 as hardware NBFM demodulator. Better audio than the SI4732 flank demodulator
// Provides a "tuning" meter like in old FM Stereo receivers and software AFC.

//#define SI5351_GENERATES_CLOCKS  //If uncommented, the SI5351 will generate the LO frequency plus 2 clocks, 4MHz for the tuner and 32768Hz for the SI4732.

#define TINYSA_PRESENT  // Syncs and controls a tinySA with receiving frequency, if connected via serial to the ESP32.

//#define SW_ATTENUATOR_PRESENT  // uncommment only if a voltage controlled attenuator is present in the shortwave RF path.
//Generates gain control voltage on dac1(GPIO_NUM_25). 0V = max. gain, 3.3V = min gain.

//#define FLIP_IMAGE // Uncomment this if the image is upside down.
//#define TFT_INVERSION_ON// Uncomment if the image is inverted.


const char* ssid = "MMV2025";        // WIFI credentials needed for web tools and LittleFS uploader
const char* password = "Pekita#2020";// WIFI credentials needed for web tools and LittleFS uploader

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
A tinySA can be used as optional panorama adapter and will then get automatically configured and updated by the firmware. 
The code is a bit rough since I am not a programmer. It does include some nice features though, such as morse decoder, slow scan waterfall, memory bank scanning, web tools, etc.
To build the hardware you should have experience with RF circuits and the appropiate toolset. At least a tinySA and NanoVNA are strongly recommended. 
At least basic Arduino programming knowledge helps a lot!


Optionally, the code can be compiled for a shortwave receiver only version. In this case you will have a receiver that covers 0-50MHz.
The shortwave receiver will have better filtering and smoother tuning than a SI4732 only receiver.
If you build the"shortwave only" version, please connect the ouput of the lowpass via 220nF to the RF input of the AD831 mixer.

For testing, the GUI can be started with only the ESP32 and the ILI9488 display connected, but responses will be sluggish and several of the functions will not work.
The receiver can be tested with only the SI4732 connected (no SSB). Select "No Mixer" in the "Debug" panel. 
In this case connect the antenna directly to the AM input of the SI4732. "No Mixer" is not sticky and for testing only.


IMPORTANT: To compile, select the "ESP32 Dev Module" and the "No OTA (2MB APP, 2MB SPIFFS)" partition scheme. 
It should then compile without issues. Some warning messages from the libraries may show up. They are not relevant.


Hardware description:

1. RF gets amplified 10db using a SBA4089 gain block.  YG602020 or any other gain block that goes down to 100KHz with a high enough IP3 can also be used. 
If sensitivity is insufficient (around -110dBm for a cearly discernable AM signal), an addidional LNA can be placed in front oft the gain block. 
This will however decrease dynamic range and increase intermodulation products. 

A small signal relay switches the LNA output between the tuner input (75Ohms) and the 0-50MHz lowpass filter for SW mode.

The UR 1316 tuner output pins are connected to the IF relay, the I2C bus, +33V from a 5->33V bost converter and the AGC output of the ESP32. 
A transistor logic switches the tuner and the boost converter power off when frequency < 50MHz. If frequency stability of the tuner is more important than 
power consumption, omit this logic. It helps saving current and eliminates possible crosstalk fromt he tuner output when in shortwave mode. 
It means however that the tuner will warm up and cool down and have frequency drift. My tuner drifts about 2KHz.
The CD1316 tuner has a different pinout and does not need the boost converter.

The ACG voltage from the ESP32 drives tuner gain down when a  strong signal is received. 
To reduce intermodulation, the AGC voltage reduces tuner amplifiction even when no signal is received.


2. The IF output of the tuner connects via a resistor to parallel resonant LC and the IF input relay. Both together reduce the tuner output by some 20dB.

The IF relay switches either the output of the tuner or the lowpass filter output to the input of the AD831. The lowpass filter for shortwave reception must
have at least 80dB attenuation in the FM radio band, otherwise mix products caused by the 3rd harmonic of the LO will be audible. 
A 9th order Chebychev filter is adequate.

AD831 gets fed with CLK2 from the SI5351. 
The SI5351 oscillates 21.4MHz above the desired frequency (LO above RF). When the tuner is in use, SI5351 oscillates 21.4 MHz above the tuner's IF spectrum (using IF from 37- 38MHz). 
The tuner PLL gets programmed in 1MHz steps and the SI5351 covers the range inbetween.
Tuners UR/UV1316 require a 5V -> 33V boost converter. A boost converter with NE555 produced audible interference, I therefore use a transistorized converter.
Tuners UR/UV1316 cover from 50 - 860 MHz. Sensistivity of the receiver is about 0.2-0.3 uV for 10dB SNR and mostly determined by the 1st gain block. 

3. For the "Shortwave only" version, please connect the ouput of the lowpass via 220nF to the RF input of the AD831 mixer. No RF input relay is needed


4. The IF output of the AD831 enters a crystal filter which is made of 3 * 21.4 MHz (Aliexpress) filters in series. 
The housings of the 2nd and 3rd filter are always grounded, while the 1st is grounded via 22pf and a ground path through 2 diodes that can be activated in the software. 
These allows to change the IF bandwidth.
The filters flanks should not be too steep, otherwise NBFM flank demodulation will get distorted. Filters with 7.5KHz or 15KHz filter bandwidth (21M7, 21M15) are recommended.
If the compiler directive "NBFM_DEMODULATOR_PRESENT" is enabled, the software will not use flank demodulation, but an optional MC3361 NBFM demodulator instead. 

My filter's center frequency is off around 3KHz (21.397MHz) for some reason, but this can be compensated in software. The filter is critical for not overloading the SI4732.
The IF can be configured in the software, anything up to 30MHz is possible, so you could also use filters for different frequencies. 
The higher the IF, the better image rejection will be.

5. A tinySA can be used as optional panorama adapter. 

6. The crystal filter output connects to a SI4732. There is quite a loss (10 - 15 db estimated) caused by the the crystal filter, since it does not use impendance transformation, 
but the preamp provides enough amplification to overcome it. 
The SI4732 is controlled by an ESP32 development board with 38 pins. (The 30 pin dev board does not provide enough GPIO's.) 
SI4732 is in the standard configuration , optionally with an external 32.768 KHz TCXO.
One of the audio outputs go to a squelch transistor which is driven by one of the ESP32 GPIO's. It grounds the audio signal when triggered and eliminates noise and the hard cracks when switching mode.
From the squelch transistor the signal goes to the volume potentiomenter and then to a LM386 audioamplifier which drives headphones and speaker. GPIO26 is connected via a resistor
to the input of the audio amplifier and provides a short touch sound. 220K provides a smooth touch sound, 22K a strong one.
The other audio output (pin 16) goes via a transistor amplifier to the sampling input of the ESP32 GPIO36.


7. The ESP32 drives the SI4732 and SI5351 on the same I2C bus. Bus speed is 2MHz, but will get autmatically reduced if the ESP32 can't initiate the SI4732 
or when the TV tuner gets adressed.
A squelch potentiometer and a fine tune potentiometer are connected to the ESP32.
The fine tune potentiometer is also used to adjust colors in waterfall mode. The fine tune pot needs to be of good quality, since any noise causes frequency jumps.
The ESP32 also drives the 3.5" ILI9488 touch display in a standard configuration using the tft eSPI library.
This code will only work with ILI9488 480*320 pixel displays and is NOT adaptable for smaller displays.

8. 12V DC input input gets regulated through two linear regulators down to +5 and +9V. Power consumption is about 400-500ma. Both regulators require a small heatsink.

9. For the FFT analysis and the morse decoder, audio output pin 16 goes via 4.7K and 1 microfarad to the base of a npn transistor amplifier. Emitter via 100 Ohms to ground, base via 47K to collector and collector via 1K to +3.3V.
Collector then goes to pin 36 of the ESP which does the sampling. Gain should be about 20 dB and 1V pp audio at the output works fine.

10. An additional audio amplifier with a square wave forming circuit consisting of a two transistor amplifier (and optionally a NE555) can be connected to GPIO39.
This allows the experimental SSTV and RTTY decoders to work.

11. If you use a tinySA, connect GPIO 1 of the ESP32 with the RX pin of the tinySA and GPIO 3 with the TX pin (close to the battery connector).
This will allow control of tinySA parameters from the receiver menu.
It will also allow calibrated measurement of the signalstrength in dBm and microvolts (using the seleced RBW of the tinySA). 
If you do this, please note that the tinySA needs to be switched off when loading firmware into the ESP32.
Connect the tinySA audio output via 10K and 0.1uF in series to the audio output of the SI4732.
You can then listen to the tinySA. Check the Youtube video to see how it works.


12. Additional SD card support has been implemented.  It uses the on board SD card slot of the ILI9488 display. Connect the SD card pins in parallel to the existing SPI bus, except the CS pin.
The SD Card CS pin gets connected to ESP32 GPIO 33. Not all SD cards work, this seems to be a limitation of the SD card library. Try several, including older ones. 

13. The receiver does require a formatted LittleFS file system on the ESP32 flash. It will auto format during the 1st run. Several files are needed on LittleFS.
These files are located in the \data folder of this sketch. Upload them either with the WIFI uploader or from SD card. csv files can be edited and contain station mames and frequencies 
Do not introduce additional commas, CR or else anyting into the lines of the CSV files, there is little error checking implemented.

14. An experimental option to record audio to SDcard has been implemented. It will record only when squelch is open and it's performance depends on the SDcard.

15. An additional NBFM demodulator has been developed since the SI4732 demodulates NBFM only via flank demodulation. 
An MC3361 converts 21.4MHz to 400KHz and uses a quadrature demodulator to generate audio and the discriminator voltage which can be displayed on the upper tuning meter.
The Q of the quadrature LC is critical, you may need to experiment with different values.

16. I have not designed a PCB. The ESP board is directly attached to the backside of the display and so is the LM386 audio amp.  
The IF/RF part is build upon the copper sides of two seperate (unetched) PCB's with the SI4732, the NBFM demodulator, crystal filter and voltage regulators on one.
The LNA, TV tuner, 33V boost converter, AD831 and SI5351 are on the other board. All RF/IF modules are built in separate shielded boxes.
Audio uses a separate ground from the digital lines.

The I2C and SPI buses of the receiver and tinySA will cause some noise on some frequencies, 
but with a clean built and proper shielding techniques these noises will mostly get buried in the atmospheric noise. Shielding and good grounding is important!

17. For better frequency stability a TCXO can be used: Replace crystal of the SI5351 module with a 25MHz TCXO as described on the internet. 
In this case do not use a 32768Hz crystal with SI4732. Connect CLK0 via 1nF to pin 13 of SI4732. 
Carefully remove the 4MHz crystal from tuner. Connect CLK1 via attenuator (see schematic) to the "hot" point of the crystal. Uncomment //define SI5351_GENERATES_CLOCKS

18. The TV tuner provides good attenuation outside it's filter bandwidth. By design it does not provide attenuation within it's channel bandwith (+-4MHz), 
so strong signals within the bandwidth can cause intermodulation in the first tuner stage.
 "Tun. Attn" will reduce the tuner gain. It will also affect the point where the tuner agc kicks in. 
If you hear crossmodulation, use the "Tuner Attn" button to adjust the AGC until crossmodulation disappears.

19. Using a wide band antenna (discone) can cause intermodulation in the SBA4089. The most obvious effect is an increased noise floor. Try a FM bandstop filter.  






Initial steps:

1. Touch the screen while the receiver is booting up for the first time. You should see a temporary screen. Press the encoder and 
follow the instructions to calibrate the touch screen. After that, touch the screen with a stylus and move the stylus around. 
A white circle should appear exactly under the stylus or very close to it.
If so, press the encoder to save, or move the encoder to recalibrate. Proper calibration is essential for the touch functions. Some touchscreens are better than others.

2. Set Master Volume to 50. Set AVC to 100.Set SMUTE to 0.

3. In Config -> SetIF SI4732: Set to 21400 if not already set. If peak frequency of crystal filter is is already known set to the peak frequency instead and skip this step.
4. Feed input of crystal filter with signal 21.4MHz at -60dBm. Change frequency of signal generator +- 5KHz in 1KHz steps and change simultaneously "SetIF SI4732" to the same frequency.
   Select the frequency with the strongest dBm reading and press encoder. Some cheap filters show two peaks, about 5KHz away. 
   Instead of directly grounding the filter housings, I grounded it via 22pf.
   This gave a single flat peak.

5. Feed antenna input with 10MHz (precise) and tune to 10MHz. In Config -> Calib SI5351: Adjust for strongest dBm or microvolt level.
   This will correct the offset of the SI5351. 
6. Change modes to USB and LSB and adjust the BFO's to zero Hz audible tone. Change mode to CW and set the BFO to roughly 600Hz. Press button Audio WF. It should draw a red line. If not, correct
   Master Volume.
7. Feed antenna input with 10MHz and a calibrated level. In More Config -> Gain Calibr. adjust displayed dBm value to the calibrated level. This will only calibrate the displayed level 
   for SW, not for VHF/UHF.
8. For TV tuner: Set your signal generator to it's highest possible frequency between 50 and 870MHz. Set the radio to the same frequency. 
   In More Config -> Tuner +-ppm adjust for strongest signal, press encoder to save. This setting is unfortunately somewhat temperature dependent.
9. Additionally for SSB in VHF/UHF calibrate the TV tuner BFO's for zero Hz audible tone.

10. Tap on "More -> Storage". Use either SD card or the WIFI uploader to transfer the files in the \data folder to the LittleFS.

11. Tap on More -> Web Tools -> EiBi List to download the latest shortwave station list if interested. This should be repeated every 6 months or so.

--------------


TinySA Setup and Operation:
Hardware Connections:

The TinySA RF input is connected via a buffer amplifier to the output of the first RF gain block.
Recommended buffer gain: 15–20 dB (example: MAR‑6 LNA).
Connect GPIO 1 of the ESP32 with the RX pin of the tinySA and GPIO 3 with the TX pin (located on the tinySA board close to the battery connector).
I have used a "Double Pole Double Throw" switch as power switch for both receiver and tinySA. 
One pole switches the receiver on and the other pole is connected in parallel to the on/off switch of the tinySA and switches the tinySA.  
TinySA needs it's battery permanently connected to keep it's setup data, so don't remove the battery. 
TinySA battery gets charged via 1N4007 and 10 Ohms (1/2 Watts) in serial from +5V only when the receiver is on. 
Optionally connect the tinySA audio output via 0.1uF and 10K to the hot end of the volume pot.


Firmware & Serial Configuration
Firmware requirement: TinySA firmware version ≥ v1.4‑105 (2023‑07‑21).
Serial connection: Activate and configure TinySA to use 115200 baud so it accepts commands from the ESP32.

//------------------------------------


TinySA firmware setup required:
Connection Settings:
Config → More → Connection → Change to Serial.
Set serial speed to 115200.

Trace Menu
Trace 2 (Green): Enabled
Trace 3 (Red): Enabled
Trace 1 (Yellow): Disabled

Marker Menu:
Assign Marker 1 to Trace 2, unselect “Tracking” and everything else that may be selected.
Assign Marker 2 to Trace 3, select “Tracking”.

Display Menu:
Enable Waterfall.
Optional: Set Sweep Accuracy to Fast.


Level Menu:
Ext Gain: Enter buffer amplifier gain (e.g. 15 dB), so that it will substracted from the gain calibrations.

Save Settings:
Save configuration as Startup Preset.
Enable “Save Settings”.

//-----------------------------------------

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

Sync enabled: Tunes a 1MHz segment around the receiver frequency. Marker 1 follows the tuned frequency.
Marker 2 shows strongest peak within the 1 MHz segment.
Receiver uses TinySA’s dBm value for S‑Meter readings (more precise than SI4732 RSSI).



Additional Buttons (appear after a successful sync on the bottom right side of the receiver screen):
R: Resets TinySA.
M1: On TinySA drag Marker 1 to a signal of interest -> receiver tunes to that signal. Tap again to exit this mode.
M2: Tunes receiver to frequency of Marker 2 (strongest peak).
M2F: Receiver follows Marker 2 only when squelch is closed. Stays on a frequency once squelch opens, when squelch closes again moves to next strongest peak.
Cfg: Configure TinySA parameters.

-----------


 
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
The squelchpot will still set the RSSI (signal strength) treshold. The little "SNR" indicator will lit white when AutoSquelch is in use.

Slow Waterfall: Displays either a frequency range "View Range", or a band "View Band" as a waterfall. Use the fine tune potentiometer to adjust sensitivity and colors.
Touch on a spot on the lower horizontal signal bar (while it says "SCANNING") and enter listen mode. 
Touch on the waterfall to go back to scan mode, or touch SET to set the frequency and leave. 
"Show 1MHz" will show a 1 MHz waterfall display. The fine tune potentiometer is not in use in this mode. Touch on a peak to listen to it.  


Touch Tune: Shows += 500KHz around tuned frequency, use encoder to change frequency. Touch a spot on the signal area and it tunes in and you can listen to frequency.
If the frequency is somewhat off, use encoder to fine tune. Touch "Cont." to leave frequency and continue.Touch back to go back to main menu without setting the frequency. 
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

// Libraries and headers:

// Built-in 
#include <Arduino.h>          // Core Arduino framework
#include <driver/gptimer.h>   // ESP-IDF driver (part of ESP32 core)
#include <driver/ledc.h>      // ESP-IDF driver (part of ESP32 core)
#include <FS.h>               // File system abstraction (ESP32 core)
#include <HTTPClient.h>       // HTTP client (ESP32 core)
#include <LittleFS.h>         // LittleFS support (ESP32 core)
#include <Preferences.h>      // NVS storage (ESP32 core)
#include <SPI.h>              // SPI bus (Arduino core)
#include <stdint.h>           // Standard C header
#include <stdio.h>            // Standard C header
#include <string.h>           // Standard C header
#include <unordered_map>      // Standard C++ STL
#include <vector>             // Standard C++ STL
#include <WiFi.h>             // Wi-Fi (ESP32 core)
#include <WiFiClient.h>       // TCP client (ESP32 core)
#include <WiFiClientSecure.h> // Secure TCP client (ESP32 core)
#include <Wire.h>             // I²C bus (Arduino core)
#include <WebServer.h>        // Web server (ESP32 core)
#include "SD.h"               // SD card support (ESP32 core)

// Needs to be installed separately (via Arduino Library Manager or GitHub)
#include <TFT_eSPI.h>         // Library Manager: "TFT_eSPI" by Bodmer
#include <TFT_eSPI_ext.h>     // GitHub only: https://github.com/FrankBoesing/TFT_eSPI_ext
#include <font_Arial.h>       // Comes with TFT_eSPI_ext repo
#include <JPEGDecoder.h>      // Library Manager: "JPEGDecoder" by Bodmer
#include <PNGdec.h>           // Library Manager: "PNGdec" by Larry Bank
#include <si5351.h>           // Library Manager: "Si5351Arduino" by Jason Milldrum
#include "arduinoFFT.h"       // Library Manager: "arduinoFFT" by kosme
#include "DacESP32.h"         // Library Manager: "DacESP32" (ESP32 DAC audio library)
#include "Rotary.h"           // Library Manager: "Rotary Encoder" by Brian Low
#include "SI4735.h"           // Library Manager: "SI4735" by Ricardo Caratti


// Project-specific (local headers included with project)
#include "patch_full.h"       // Project-specific SSB patch
#include "Sprites.h"          // Project-specific bitmaps and sprites
#include "nxfont24.h"         // Project-specific nixie font
#include "logSerial.h"        // Project-specific logging
#include "Config.h"           // Project-specific macros and globals

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

void startHWT() {  // initiate the HW timer. Currently not in use, left code for future use
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
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);  // encoder needed for touch calibration
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), RotaryEncFreq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), RotaryEncFreq, CHANGE);

  bootScreen();
  etft.TTFdestination(&tft);  // Additional fonts
  etft.setTTFFont(Arial_13);


  uint16_t calData[5] = { // values for display 3.5
                          (uint16_t)preferences.getInt("cal0", 323),
                          (uint16_t)preferences.getInt("cal1", 3305),
                          (uint16_t)preferences.getInt("cal2", 481),
                          (uint16_t)preferences.getInt("cal3", 2670),
                          (uint16_t)preferences.getInt("cal4", 5)
  };

  //uint16_t calData[5] = {324, 3335, 427, 2870, 3 }; // example for display 3.5"
  //uint16_t calData[5] = {290, 3588, 301, 3510, 1 }; // example for display 3.5" flipped
  // uint16_t calData[5] = { 292 3333, 198, 3539, 7 };  //example for display 4.0
  tft.setTouch(calData);
  loadLastSettings();  // load settings from flash
  drawFrame();
  spriteBorder();
  pinMode(IF_INPUT_RELAY, OUTPUT);  // pin for IF relay and to enable tuner voltage
  pinMode(TUNER_AGC_PIN, OUTPUT);
  pinMode(NBFM_MUTE_PIN, OUTPUT);
  pinMode(MUTEPIN, OUTPUT);
  pinMode(AUDIO_INPUT_PIN, INPUT);  // Audio FFT
  pinMode(PULSE_PIN, INPUT);        // Audio square wave
  // pinMode(tinySA_PIN, OUTPUT);  // tinySA pin
  pinMode(IF_FILTER_BANDWIDTH_PIN, OUTPUT);  // IF filter

#ifdef NBFM_DEMODULATOR_PRESENT
  pinMode(TUNING_VOLTAGE_READ_PIN, INPUT);  // NBFM demodulator AFC read pin
#endif

  SI5351_Init();
  loadLists();                    // load structures, use from LittleFS if exists
  createMemoInfoCSVIfNotExist();  //Create memoInfo on LittleFS if not there
  radioInit();                    // prepare SI4732 and AGC

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

  if (FREQ != FREQ_OLD) {
    FREQCheck();         //check whether within FREQ range
    displayFREQ(FREQ);   // display new FREQ
    setLO();             // and tune it in
    resetSmeter = true;  // reset to zero
    autoloopBands();     // if autoloop is enabled frequency will stay within a selected band
    FREQ_OLD = FREQ;
    //Serial.print("freq changed\n");
  }


  getRSSIAndSNR();  // get RSSI (signalStrength) and SNR, valid for all functions in the main loop

  displaySTEP(false);  // check STEP

  readSquelchPot(true);  // true = read and draw position circle
  setSquelch();
  fineTune();  // read frequency potentiometer

#ifdef TV_TUNER_PRESENT
  setTunerAGC();  // TV tuner AGC reduces tuner gain for better IP3, shows value in the Tuner Gain tile
#endif

#ifdef SW_ATTENUATOR_PRESENT
  if (!TVTunerActive)
    setShortwaveAGC();  // shortwave AGC
#endif

  saveCurrentSettings();  // save settings after 1 minute if freq has not changed

  // functions that don't have to run in every loop cycle get called with fTrigger

  if (fTrigger % 3 == 0) {

    checkTouchCoordinates();  // this function takes long. Calling it 1 out of 3 loop cycles is suficient.

#ifdef NBFM_DEMODULATOR_PRESENT
    getDiscriminatorVoltage();  // display Tuning meter

#else
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



    loadPanoramaScan(false);  //1 scans +-500 KHz around current frequency when squelch is closed (AM/FM only)
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
    synctinySA();  // tinySA synchronisation
#endif



  if (fTrigger % 25 == 0) {
    clockDisplay();  // clock in lower left corner
  }


  if (fTrigger % 50 == 0) {

    if (syncEnabled == false || tinySAfound == false)
      calculateAndDisplaySignalStrength();  // Smeter about 3x second, alternatively gets called from convertTodBm(),when tinySA provides signal level

    slowTaskHandler();  // run tasks with a long period
    fTrigger = 0;
    disableFFT = false;
  }



  fTrigger++;  // function trigger counter, triggers functions that should not run every loop cycle


  if (loopDelay)  // prevents the main loop from running faster than 10ms since several functions would then run too fast
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

  if (useNixieDial) {
    drawDial(freq);  // use retro dial instead
    return;
  }


  long old_Mhz = oldFreq / 1000000;
  long old_Khz = (oldFreq - (old_Mhz * 1000000)) / 1000;
  long old_Hz = (oldFreq - (old_Mhz * 1000000)) - old_Khz * 1000;

  long new_Mhz = freq / 1000000;
  long new_Khz = (freq - (new_Mhz * 1000000)) / 1000;
  long new_Hz = (freq - (new_Mhz * 1000000)) - new_Khz * 1000;

  colorSelector();  // select display color



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
  etft.setTextColor(TFT_GREEN);
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


  if (useNixieDial)  // no room to display step
    return;

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
  tft.fillRect(xPos - 10, yPos, 140, 20, TFT_BLACK);
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


static const uint32_t AMStepSize[] PROGMEM = { 250, 500, 1000, 2500, 5000, 6000, 8333, 9000, 10000, 100000, 1000000, 10000000 };
static const uint32_t SSBStepSize[] PROGMEM = { 10, 25, 50, 100, 250, 500, 1000, 5000, 10000, 100000, 1000000, 10000000 };
static const uint32_t FMStepSize[] PROGMEM = { 500, 1000, 2500, 5000, 6250, 10000, 12500, 25000, 50000, 100000, 1000000, 10000000 };

static const char* const AMLabels[] PROGMEM = { "250Hz", "500Hz", "1KHz", "2.5K", "5KHz", "6KHz", "8.3KHz", "9KHz", "10KHz", "100KHz", "1MHz", "10MHz" };
static const char* const SSBLabels[] PROGMEM = { "10Hz", "25Hz", "50Hz", "100Hz", "250Hz", "500Hz", "1KHz", "5KHz", "10KHz", "100KHz", "1MHz", "10MHz" };
static const char* const FMLabels[] PROGMEM = { "500Hz", "1K", "2.5K", "5K", "6.25K", "10K", "12.5K", "25K", "50K", "100K", "1MHz", "10MHz" };


struct StepTable {
  const uint32_t* sizes;
  const char* const* labels;
};

static const StepTable stepTables[] = {
  { AMStepSize, AMLabels },    // AM
  { SSBStepSize, SSBLabels },  // SSB/CW/SYNC
  { FMStepSize, FMLabels }     // NBFM
};

void setSTEP() {
  if (modType == WBFM) return;  // fixed 100kHz

  if (pressed) {
    draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

    const int positions[12][2] = {
      { 19, 141 }, { 105, 141 }, { 185, 141 }, { 265, 141 }, { 19, 198 }, { 105, 198 }, { 185, 198 }, { 265, 198 }, { 19, 255 }, { 96, 255 }, { 185, 255 }, { 260, 255 }
    };

    etft.setTTFFont(Arial_14);
    etft.setTextColor(TFT_GREEN);

    const char* const* labels = nullptr;
    const uint32_t* sizes = nullptr;

    if (modType == AM) {
      labels = stepTables[0].labels;
      sizes = stepTables[0].sizes;
    } else if (modType == LSB || modType == USB || modType == SYNC || modType == CW) {
      labels = stepTables[1].labels;
      sizes = stepTables[1].sizes;
    } else if (modType == NBFM) {
      labels = stepTables[2].labels;
      sizes = stepTables[2].sizes;
    }

    for (int i = 0; i < 12; i++) {
      etft.setCursor(positions[i][0], positions[i][1]);
      etft.print(labels[i]);
    }

    etft.setTextColor(textColor);

    tDoublePress();
    pressed = get_Touch();

    if (ty > 300 || ty < 120 || tx > 345) return;

    int column = tx / HorSpacing;
    int row = 1 + ((ty - 20) / vTouchSpacing);
    if (row < 2) {

      pressed = false;
      tx = 0;
      ty = 0;
      return;
    }

    int pos = (row == 2) ? column : (row == 3) ? column + 4
                                               : column + 8;
    STEP = sizes[pos];
  }

  tRel();
  displaySTEP(false);
  roundFreqToSTEP();
  resetMainScreen();
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



void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color1, uint32_t color2) {  // draws buttons as plain rectangles or push bitmaps


  if (buttonSelected < 0 || buttonSelected > 8)  // if value loaded from flash outside range (older versions use different values)
    buttonSelected = 4;

  if (altStyle) {  //draw plain buttons

    tft.fillRectVGradient(x, y + 4, w, h / 2, color1, color2);
    tft.fillRectVGradient(x, y + (h / 2) + 4, w, h / 2, color2, color1);
    if (w < 300)  // silver frame around buttons
      tft.drawRect(x, y + 4, w, h, TFT_LIGHTGREY);
  }

  else {  //draw sprites, use height as selection criteria

    if (h == 50)

      tft.pushImage(x, y + 4, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[buttonSelected]);  // draw selected button bitmap


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

  if (!syncEnabled || !TSAdBm) {  // calculate dBm from RSSI when no data from tinySA
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

  if (!useNixieDial) {
    tft.fillRect(330, 4, 145, 22, TFT_BLACK);
    tft.setCursor(345, 8);
  }

  if (showMeters && useNixieDial && !scanMode) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_GREEN);
    tft.fillRect(390, 115, 70, 8, TFT_BLACK);
    tft.setCursor(390, 115);

    // tft.fillRect(410, 4, 65, 8, TFT_BLACK);
    // tft.setCursor(410, 4);
  }



  if (TSAdBm) {  // only values from the tinySA are precise, RSSI from SI4732 depends on AGC


    if (microVolts < 10000)
      tft.printf("%5.1fuV", microVolts);
    else
      tft.printf("%6.0fuV", microVolts);
  }


  tft.fillRect(288, 62, 49, 28, TFT_BLACK);

  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);

  tft.setCursor(295, 64);
  tft.printf("%ddBm", dBm);

  tft.setCursor(300, 80);
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


    tft.fillRect(newSpoint, Ysmtr + 13, maxX - newSpoint, 15, TFT_BLACK);

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

  static uint8_t oldPot = 0;
  const int div = 295;
  uint32_t val = 0;
  uint8_t pot;


  for (int l = 0; l < 10; l++)
    val += analogRead(SQUELCH_POT);

  pot = val / div;  // oversample and  set pot to a value from 0 to 138

  if (pot <= oldPot - 2 || pot >= oldPot + 2 || redrawMainScreen) {  // eliminate A/D convrter noise, update only when pot turned

    currentSquelch = pot - 5;  // this sets currentSquelch to a value from  - 5 to 132 (leaving some margin).
    //A negative value is required to open squelch if RSSI == 0. A value > 127 is needed to always close



    disableFFT = true;  //temporarily disable spectrum analyzer to keep loop fast, so that circle does not jump


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


  if (currentSquelch > 127) {  // squelch fully closed == mute

    si4735.setAudioMute(true);
    si4735.setHardwareAudioMute(true);
    audioMuted = true;
    return;
  }




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
      dcOffset = analogRead(AUDIO_INPUT_PIN);
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

    if (audioMuted || signalStrength < 20)  // afcVoltage incorrect when signal too low
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


void getAFCstatus() {

#ifdef NBFM_DEMODULATOR_PRESENT
  if ((modType == AM || modType == NBFM) && showMeters && !scanMode) {  //
    afcEnable = !afcEnable;
    showAFCStatus();
    tRel();
  }
#endif

  showAFCStatus();
}


//##########################################################################################################################//



void showAFCStatus() {

  if (scanMode)
    return;


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

  Serial.printf("AFC %d\n", afcEnable);
}
//##########################################################################################################################//


void setAFC(int afcVoltage) {  // only for NBFM hardware, generates AFC voltage and uses the SI5351 oscillator correction to tune to frequency
                               // range about +-5KHz


  const int AFCStep = 5000;
  static long shift = 0;

  if (!scanMode) {
    tft.setTextColor(TFT_GREEN);  // need to reprint sice this gets overwritten when the meter gets redrawn
    tft.setTextSize(1);
    tft.setCursor(350, 65);
    tft.print("AFC");
    tft.setTextColor(textColor);
    tft.setTextSize(2);
  }

  bool aboveFreq = (afcVoltage > 5 && afcVoltage < 70 && signalStrength > 20);
  bool belowFreq = (afcVoltage < -5 && afcVoltage > -70 && signalStrength > 20);

  if (aboveFreq) {
    // shift direction depends on single/double conversion
    shift += (TVTunerActive ? -AFCStep : +AFCStep);

    if (showMeters) {
      tft.fillCircle(460, 115, 4, TFT_BLACK);
      tft.fillCircle(365, 115, 4, TFT_RED);
    }
  }

  if (belowFreq) {
    shift += (TVTunerActive ? +AFCStep : -AFCStep);

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

void hideBigBtns() { // hide UP/DOWN/MODE or the meters
  tft.fillRect(345, 48, 130, 242, TFT_BLACK);
}


//##########################################################################################################################//

void setShortwaveAGC() {  // this will drive the SW attenuator and act as an AGC. SWMinAttn determines the minimum attenuation.

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

  if (SWAttn < SWMinAttn)
    SWAttn = SWMinAttn;

  if (attnOFF)
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


void setRFAttenuatorMinAttn() {  //Min. attenuation of the shortwave RF attenuator

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

    if (SWMinAttn < 5) {
      attnOFF = true;
    } else {
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


    dac1.outputVoltage((uint8_t)SWMinAttn);  // set the dac
    clw = false;
    cclw = false;
  }

  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;

  delay(200);
  encLockedtoSynth = true;
}
