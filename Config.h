//Macros and global variables


#ifndef CONFIG_H
#define CONFIG_H
#endif




#ifdef TV_TUNER_PRESENT

#define LOW_BAND_LOWER_LIMIT 50000000  // TV tuner bands limits tested with tuner UR1316
#define LOW_BAND_UPPER_LIMIT 167000000
#define MID_BAND_UPPER_LIMIT 454000000
#define HIGH_BAND_UPPER_LIMIT 875000000      // sensitivity decreases rapidly > 850MHz
#define SHORTWAVE_MODE_UPPER_LIMIT 50000000  //above 50MHz tuner is in use
#define HIGHEST_ALLOWED_FREQUENCY HIGH_BAND_UPPER_LIMIT

#define TUNER_UR1316  // Tuner type 1

#else
#define HIGHEST_ALLOWED_FREQUENCY 50000000  // 50MHz
#define SHORTWAVE_MODE_UPPER_LIMIT 50000000
#endif

#define LOWEST_ALLOWED_FREQUENCY 100000  // could probably go lower, but noise gets dominant
//##########################################################################################################################//





//Colors
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
#define TFT_LIGHTBULB 0xFFDC

//plain button colors
#define TFT_BTNBDR TFT_NAVY
#define TFT_BTNCTR TFT_BLUE
#define TFT_MAINBTN_BDR TFT_NAVY
#define TFT_MAINBTN_CTR TFT_GREY

// retro dial colors
#define TFT_DIAL_BACKGROUND  TFT_BLACK
#define TFT_DIAL_TICKS TFT_LIGHTBULB

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


#define FMSTARTFREQ 10000  // WBFM initial freq, use frequency of a nice radio station

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
#define WATERFALL_SCREEN_HEIGHT 220  // more than that will lead to a memory allocation error
#define FRAMEBUFFER_HALF_WIDTH 120   // need to use two half size frame buffers, couldn't allocate a full size 240*231 buffer
#define FRAMEBUFFER_FULL_WIDTH 240
#define FRAMEBUFFER_HEIGHT 220  // was 231, but crashed with new Arduino IDE 2.3.5
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
TFT_eSPI_ext etft = TFT_eSPI_ext(&tft); // Install from https://github.com/FrankBoesing/TFT_eSPI_ext
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

const char ver[] = "V.564";         // version

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
uint8_t SWAttn = 0;     // Shortwave attenuator attenuation, 0 = 0dB, 255= 30dB attenuation
uint8_t SWMinAttn= 0;  // Shortwave attenuator minimum attenuation, 0 = 0dB, 255= 30dB attenuation
bool attnOFF = true;   // SW Attenuator off
long lastAMFREQ = -1;  // AM frequency before switching to WBFM
long span = 1000000;   // tinySA default span, configured in TSA Presets 0 and 1
long potVal;           // fine tune potentiometer read value
long lim2 = 0;         // frequency limits from touchpad entries
long lim1 = 0;         // frequency limits from touchpad entries
long keyVal = 0;       // scan mode frequencies delivered from touchpad


bool TVTunerActive = false;     //will get set to true when tuner in use
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
int selected_band = -1;    // number of the selected band  -1 means no band selected
bool showMeters = false;   // shows analog meters on right side
bool resetSmeter = false;  // reset Smeter when frequency changes, to avoid decay delay
bool funEnabled = false;   // animations
bool enableAnimations = false;
bool useNixieDial = false;
bool fastBoot = false;     // quick boot when enabled


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
uint8_t modType = 1;                        //modulation type, 
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
char miniWindowMode = 3;            // mini window mode: 1 = 16 channel, 2 = 85 channel, 3= minioscilloscope, 4 = waterfall, 5 = envelope
int amplitude = 150;                // amplitute divider for spectrum analyzer
int fTrigger = 0;                   // triggers functions in main loop
int currentSquelch = 0;             // squelch trigger level
uint8_t vol = 50;                   // global volume, should be around 50
bool disableFFT = false;            // stops the spectrum analyzer during time critical operations
bool SNRSquelch = true;             // use either RSSI or SNR to trigger squelch
bool noMixer = false;               // run with antenna directly connected to SI4732 for testing
bool vfo1Active = true;
bool showTouchCoordinates = false;  // debug
int loopDelay = 0;                  // stabilize loop at not less than 10ms

// Waterfall
const int wfSensitivity = 50;                 // is a divider, more means less sensitivity
uint16_t* framebuffer1;                       // First framebuffer for waterfall, also used in panoramascan
uint16_t* framebuffer2;                       // Second framebuffer for waterfall
uint16_t stretchedX[FRAMEBUFFER_FULL_WIDTH] = { 0 };  // array audio waterfall to strech 240 to 331 pixels
int currentLine = 0;                          // Current line being written to
uint16_t newLine[DISP_WIDTH] = { 0 };             // line transfer buffer
bool smoothColorGradient = false;             // smooth = blue to white, otherwise blue to red
bool showAudioWaterfall = false;
uint8_t wabuf[26][90] = { 0 };  // audio waterfall miniwindow buffer
bool audiowf = false;
bool showPanorama = false;  //Panorama screen while squelch is closed
bool show1MhzSegment = 0; // 0 shows +-500KHz around FREQ, 1 starts with full MHz 

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

// Buttons
extern const uint16_t But1[], But2[], But3[], But4[], But5[], But6[], But7[], But8[];  // sprite buttons in sprite.h
const uint16_t* buttonImages[] = { But1, But2, But3, But4, But5, But6, But7, But8 };
uint16_t buttonSelected = 4;  // 4-11


// Web
uint8_t imageSelector = 0;
bool swappedJPEG = false;
const char* ssid = "YourSSID";
const char* password = "YourPW";
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



