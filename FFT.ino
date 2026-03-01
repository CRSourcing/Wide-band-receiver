//##########################################################################################################################//

// Functions for audio analysis.
// 2 audio spectrum Analyzers, mini waterfall and mini oscilloscope
// Also includes a CW decoder using the Goertzel algorithm
// Idea from here: https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_03.ino


const int startX = 135;
const int startY = 294;
static char oldminiWindowMode = 0;
static int offset = 0;
static bool mStat = false;
int ampl;
const int width = 84;
const int height = 24;


void audioSpectrum() {

  if (oldminiWindowMode != miniWindowMode) {  //  miniWindowMode selects btw off, low resolution, high resolution, mini osci. audio waterfall. Spectrums display , waterfall
    oldminiWindowMode = miniWindowMode;
    if (miniWindowMode >= 6)
      miniWindowMode = 1;

    preferences.putChar("spectr", miniWindowMode);        // save mode
    tft.fillRect(startX - 5, startY, 98, 26, TFT_BLACK);  // overwrite last window
  }


  if (audioMuted && !pressed) {

    if (!mStat) {
      mStat = true;
      tft.fillRect(startX, startY, 92, 26, TFT_BLACK);  // overwrite when muted
      tft.setCursor(startX + 15, startY + 8);
      tft.setTextColor(TFT_YELLOW);
      tft.printf("MUTED");
      tft.setTextColor(textColor);
    }
    return;
  }


  if (mStat) {
    tft.fillRect(startX, startY, 92, 25, TFT_BLACK);
    mStat = false;
  }



  if (miniWindowMode == 3) {                         // mini oscilloscope
    tft.fillRect(startX, startY, 86, 27, TFT_NAVY);  //background
    FFTSample(256, 0, true);                         //256 samples, 0 uS delaytime,  draw miniosci
  } else
    FFTSample(256, 0, false);  //256 samples, 300 uS delaytime for the other miniwindows



  if (miniWindowMode == 1) {  // low resolution spectrum analyzer, uses 16 channels


    for (byte band = 0; band <= 16; band++) {  // overwrite black, slow decay
      int xPos = startX + 5 * band;
      int yPos = DISP_HEIGHT - Rpeak[band];

      tft.fillRect(xPos, yPos, 3, 2, TFT_BLACK);

      if (Rpeak[band] >= 1)
        Rpeak[band] -= 1;
    }

    for (int i = 2; i < (SAMPLES / 2); i++) {  // only use lower half of bins
      if (RvReal[i] > 750) {                   // eliminate noise
        byte band = getBandVal(i);
        displayBand(band, (int)RvReal[i]);
      }
    }
  }

  if (miniWindowMode == 2) {  // high resolution, 85 channels

    for (int i = 2; i < SAMPLES / 3; i++) {
      // calculate height
      int height = DISP_HEIGHT - (int)(RvReal[i] / 5);
      height = (height < startY) ? startY : height;

      for (int j = 0; j < 5; j++) {
        tft.drawPixel(startX + i, height + offset + j, TFT_BLACK);
      }
      offset = (offset + 5) % 23;

      int ampl = DISP_HEIGHT - (int)(RvReal[i] / amplitude);
      ampl = (ampl < startY) ? startY : ampl;

      tft.drawFastVLine(startX + i, ampl, DISP_HEIGHT - ampl, TFT_SKYBLUE);
    }
  }


  if (miniWindowMode == 4) {  // audio waterfall
    const int gradient = 100;

    // shift
    for (int i = height - 1; i >= 0; i--) {
      for (int j = 0; j < width; j++) {
        wabuf[i + 1][j] = wabuf[i][j];
      }
    }

    // fill first row again
    for (int j = 0; j < width; j++) {
      wabuf[0][j] = (int)(RvReal[j + 2] / amplitude);  // eliminate zero and reduce bandwidth to 1.5 KHz
    }

    // draw pixels
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        tft.drawPixel(startX + j, startY + i, valueToWaterfallColor(gradient * wabuf[i][j]));
      }
    }
  }
  if (miniWindowMode == 5) {  //envelope

    const int width = 84;
    const int height = 24;

    for (int j = 0; j < width - 1; j++) {  // shift data one column left
      for (int i = 0; i < height; i++) {
        wabuf[i][j] = wabuf[i][j + 1];
      }
    }
    // fill the right
    for (int i = 0; i < height; i++) {
      wabuf[i][width - 1] = (int)(RvReal[i + 2] / amplitude);  // reduce bandwidth to 2 KHz
    }

    const int halfH = height / 2;
    int prev = halfH;
    float smoothFact = 0.25;  // Smoothen

    for (int j = 0; j < width; j++) {
      int env = halfH;
      bool valid = false;

      // calculate env
      for (int i = 0; i < height; i++) {
        int value = wabuf[i][j];
        if (value > 0) {
          valid = true;
          if (i < halfH && value > wabuf[env][j]) {  // For upper and lower parts
            env = i;
          }
          if (i > halfH && value > wabuf[env][j]) {
            env = i;
          }
        }
      }

      // smoothen
      env = (int)(smoothFact * prev + (1.0 - smoothFact) * env);
      prev = env;

      // draw
      if (valid) {
        // Draw NAVY
        tft.drawFastVLine(startX + j, startY, height, TFT_NAVY);
        // draw boundaries
        for (int i = 0; i <= env; i++) {
          int yUpper = startY + halfH - i / 2;
          int yLower = startY + halfH + i / 2;
          tft.drawPixel(startX + j, yUpper, TFT_WHITE);
          tft.drawPixel(startX + j, yLower, TFT_WHITE);
        }
      }
    }
  }

  if (pressed)
    tRel();
}
//##########################################################################################################################//

byte getBandVal(int i) {  // values for 16 channel spectrum analyzer


  if (i <= 2) return 0;
  if (i <= 4) return 1;
  if (i <= 6) return 2;
  if (i <= 8) return 3;
  if (i <= 10) return 4;
  if (i <= 12) return 5;
  if (i <= 14) return 6;
  if (i <= 17) return 7;
  if (i <= 21) return 8;
  if (i <= 26) return 9;
  if (i <= 31) return 10;
  if (i <= 36) return 11;
  if (i <= 42) return 12;
  if (i <= 48) return 13;
  if (i <= 55) return 14;
  if (i <= 62) return 15;
  if (i <= 70) return 16;
  return 16;
}

//##########################################################################################################################//

void displayBand(int band, int dsize) {

  int dmax = 23;
  dsize /= amplitude;
  if (dsize > dmax) dsize = dmax;
  for (int s = 0; s <= dsize; s++) {
    tft.drawPixel(startX + 5 * band, DISP_HEIGHT - s, TFT_YELLOW);
    tft.drawPixel(startX + 5 * band + 1, DISP_HEIGHT - s, TFT_YELLOW);
    tft.drawPixel(startX + 5 * band + 2, DISP_HEIGHT - s, TFT_YELLOW);
  }
  if (dsize > Rpeak[band])
    Rpeak[band] = dsize;
}

//##########################################################################################################################//
// CW decoder and helper functions


void tuneCWDecoder() {  // shows a small waterfall that helps tune to 558 Hz audio.


  tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);
  tft.setCursor(0, 150);
  tft.setTextColor(TFT_YELLOW);
  tft.print("Align signal with center bar.\n\nAudio frequency should be 558Hz.\n\nPress encoder to start decoding.\n\nPress again to leave.");
  tft.setTextColor(textColor);

  const int startX = 100;
  const int startY = 70;
  const int width = 84;
  const int height = 24;
  const int gradient = 100;

  tft.fillRect(startX + width, startY - 10, 3, 44, TFT_GREEN);  // center bar

  while (true) {

    if (clw || cclw) {  // fine tune
      FREQ += clw ? 25 : -25;
      displayFREQ(FREQ);
      setLO();
      clw = 0;
      cclw = 0;
    }


    FFTSample(256, 300, false);  //256 samples, 300 uS delaytime, do not draw miniosci

    // Waterfall  shift rows
    for (int i = height; i >= 0; i--) {
      for (int j = 0; j < width; j++) {
        wabuf[i + 1][j] = wabuf[i][j];
      }
    }

    // fill first row again
    for (int j = 0; j < width; j++) {
      wabuf[0][j] = (int)(RvReal[j + 2] / amplitude);
    }

    // draw pixels
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < 2 * width; j += 2) {
        tft.drawPixel(startX + j, startY + i, valueToWaterfallColor(gradient * wabuf[i][j / 2]));
        tft.drawPixel(startX + j, startY + i + 1, valueToWaterfallColor(gradient * wabuf[i][j / 2]));
      }
    }

    if (digitalRead(ENCODER_BUTTON) == LOW)
      break;
  }
  while (digitalRead(ENCODER_BUTTON) == LOW)
    ;
}

//##########################################################################################################################//

/* MORSE DECODER
Modified from: https://github.com/G6EJD/ESP32-Morse-Decoder/blob/master/README.md
CW Decoder by Hjalmar Skovholm Hansen OZ1JHM  VER 1.01
 Feel free to change, copy or what ever you like but respect
 that license is http://www.gnu.org/copyleft/gpl.html
 Read more here http://en.wikipedia.org/wiki/Goertzel_algorithm 
 Adapted for the ESP32/ESP8266 by G6EJD  
*/

float magnitude = 0;
int magnitudelimit = 1000;      // adjusted volume, SI4735 volume needs to be set to 50
int magnitudelimit_low = 1000;  // adjusted volume,
int realstate = LOW;
int realstatebefore = LOW;
int filteredstate = LOW;
int filteredstatebefore = LOW;

float coeff;
float Q1 = 0;
float Q2 = 0;
float sine;
float cosine;
float sampling_freq = 25000;  // adjusted from 45000 to 25000
float target_freq = 558.0;    // adjust for your needs see above
int n = 64;                   // if you change here please change next line also
int testData[64];             // adjusted from 128  to 64
float bw;

// Noise Blanker time which shall be computed so this is initial
int nbtime = 6;  /// ms noise blanker

long starttimehigh;
long highduration;
long lasthighduration;
long hightimesavg;
long lowtimesavg;
long startttimelow;
long lowduration;
long laststarttime = 0;
#define num_chars 20
char displayBuffer[num_chars + 1] = { 0 };
char CodeBuffer[10];
int stop = LOW;
int wpm, oldwpm = -1;
bool showCodeBuffer = false;
bool bwInit = false;


void CWDecoder() {

  target_freq = 558.0;  // target frequency
  bw = sampling_freq / n;
  int k;
  float omega;
  k = (int)(0.5 + ((n * target_freq) / sampling_freq));
  omega = (2.0 * PI * k) / n;
  sine = sin(omega);
  cosine = cos(omega);
  coeff = 2.0 * cosine;

  tuneCWDecoder();

  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  tft.drawString("BW = " + String(bw, 0) + "Hz", 190, 80);

  FREQ_OLD -= 1;
  displayFREQ(FREQ);


  while (true) {

    if (digitalRead(ENCODER_BUTTON) == LOW)  // leave
      return;

    if (clw || cclw) {  // fine tune

      if (clw)
        FREQ += 25;
      if (cclw)
        FREQ -= 25;
      displayFREQ(FREQ);
      setLO();
      clw = 0;
      cclw = 0;
    }

    //////////////////////////////////// The basic goertzel calculation //////////////////////////////////////


    // Read analog input and do the Goertzel
    for (byte index = 0; index < n; index++) {
      testData[index] = analogRead(AUDIO_INPUT_PIN);
      float Q0;
      Q0 = coeff * Q1 - Q2 + (float)testData[index];
      Q2 = Q1;
      Q1 = Q0;
    }

    float magnitudeSquared = (Q1 * Q1) + (Q2 * Q2) - Q1 * Q2 * coeff;  // we do only need the real part //
    magnitude = sqrt(magnitudeSquared);
    Q2 = 0;
    Q1 = 0;

    if (magnitude > magnitudelimit_low) { magnitudelimit = (magnitudelimit + ((magnitude - magnitudelimit) / 6)); }  /// moving average filter
    if (magnitudelimit < magnitudelimit_low) magnitudelimit = magnitudelimit_low;


    // Now check the magnitude //
    if (magnitude > magnitudelimit * 0.6)  // just to have some space up
      realstate = HIGH;
    else
      realstate = LOW;

    // Clean up the state with a noise blanker //

    if (realstate != realstatebefore) { laststarttime = millis(); }
    if ((millis() - laststarttime) > nbtime) {
      if (realstate != filteredstate) {
        filteredstate = realstate;
      }
    }

    if (filteredstate != filteredstatebefore) {
      if (filteredstate == HIGH) {
        starttimehigh = millis();
        lowduration = (millis() - startttimelow);
      }

      if (filteredstate == LOW) {
        startttimelow = millis();
        highduration = (millis() - starttimehigh);
        if (highduration < (2 * hightimesavg) || hightimesavg == 0) {
          hightimesavg = (highduration + hightimesavg + hightimesavg) / 3;  // now we know avg dit time ( rolling 3 avg)
        }
        if (highduration > (5 * hightimesavg)) {
          hightimesavg = highduration + hightimesavg;  // if speed decrease fast ..
        }
      }
    }

    // Now check the baud rate based on dit or dah duration either 1, 3 or 7 pauses
    if (filteredstate != filteredstatebefore) {
      stop = LOW;
      if (filteredstate == LOW) {                                                        // we did end on a HIGH
        if (highduration < (hightimesavg * 2) && highduration > (hightimesavg * 0.6)) {  /// 0.6 filter out false dits
          strcat(CodeBuffer, ".");
          showCodeBuffer = true;
        }

        if (highduration > (hightimesavg * 2) && highduration < (hightimesavg * 6)) {
          strcat(CodeBuffer, "-");
          showCodeBuffer = true;
          wpm = (wpm + (1200 / ((highduration) / 3))) / 2;  //// the most precise we can do ;o)
        }
      }

      if (filteredstate == HIGH) {  //// we did end a LOW
        float lacktime = 1;
        if (wpm > 25) lacktime = 1.0;  ///  when high speeds we have to have a little more pause before new letter or new word
        if (wpm > 30) lacktime = 1.2;
        if (wpm > 35) lacktime = 1.5;
        if (lowduration > (hightimesavg * (2 * lacktime)) && lowduration < hightimesavg * (5 * lacktime)) {  // letter space
          CodeToChar();
          CodeBuffer[0] = '\0';
          showCodeBuffer = true;
          //AddCharacter('/');
          //Serial_print("/");
        }
        if (lowduration >= hightimesavg * (5 * lacktime)) {  // word space
          CodeToChar();
          CodeBuffer[0] = '\0';
          DisplayCharacter(' ');
          Serial_print(" ");
          showCodeBuffer = true;
        }
      }
    }
    if ((millis() - startttimelow) > (highduration * 6) && stop == LOW) {
      CodeToChar();
      CodeBuffer[0] = '\0';
      stop = HIGH;
      showCodeBuffer = true;
    }
    // the end of main loop clean up//
    realstatebefore = realstate;
    lasthighduration = highduration;
    filteredstatebefore = filteredstate;


    if (oldwpm != wpm) {
      tft.fillRect(0, 60, 110, 30, TFT_BLACK);
      tft.drawString("WPM = " + String(wpm), 50, 80);
      oldwpm = wpm;
    }

    if (showCodeBuffer) {
      tft.fillRect(260, 60, 219, 20, TFT_BLACK);
      tft.setTextColor(TFT_YELLOW);
      tft.setTextSize(3);
      CodeBuffer[7] = '\0';  // no more than 6 symbols
      tft.drawString(CodeBuffer, 370, 80);
      tft.setTextSize(2);
      tft.setTextColor(textColor);
      showCodeBuffer = false;
    }
  }
}

//##########################################################################################################################//

void DisplayCharacter(char newchar) {
  for (int i = 0; i < num_chars; i++) {
    displayBuffer[i] = displayBuffer[i + 1];
  }

  static int ctr = 0;

  // dark grey
  uint16_t targetR = 8;
  uint16_t targetG = 16;
  uint16_t targetB = 8;

  if (isprint(newchar)) {
    displayBuffer[num_chars] = newchar;
    tft.setTextSize(3);
    tft.fillRect(4, 112, 470, 38, TFT_DARKDARKGREY);
    for (int i = 0; i <= num_chars; i++) {
      tft.setCursor(10 + 20 * i, 120);


      // starting color
      uint16_t r = (TFT_WHITE >> 11) & 0x1F;
      uint16_t g = (TFT_WHITE >> 5) & 0x3F;
      uint16_t b = TFT_WHITE & 0x1F;

      // calculate fading
      r = r + ((targetR - r) * (num_chars - i) / num_chars);
      g = g + ((targetG - g) * (num_chars - i) / num_chars);
      b = b + ((targetB - b) * (num_chars - i) / num_chars);

      // combine again
      uint16_t fadedColor = (r << 11) | (g << 5) | b;
      tft.setTextColor(fadedColor);
      tft.setTextSize(3);
      tft.print(displayBuffer[i]);  // older letters fade out
    }

    static int y = 0;
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(ctr, 180 + y);
    tft.print(newchar);  // print 5 lines, start from 0 when filled
    ctr += 14;

    if (ctr > 460) {  //new line
      y += 22;
      ctr = 0;
    }

    if (y == 110) {  // reset when start 6th line
      y = 0;
      ctr = 0;
      tft.fillRect(0, 178, 479, 110, TFT_BLACK);
    }
  }
}

//##########################################################################################################################//

// Initialize the Morse code lookup table
std::unordered_map<std::string, char> morseToCharMap = {
  { ".-", 'a' }, { "-...", 'b' }, { "-.-.", 'c' }, { "-..", 'd' }, { ".", 'e' }, { "..-.", 'f' }, { "--.", 'g' }, { "....", 'h' }, { "..", 'i' }, { ".---", 'j' }, { "-.-", 'k' }, { ".-..", 'l' }, { "--", 'm' }, { "-.", 'n' }, { "---", 'o' }, { ".--.", 'p' }, { "--.-", 'q' }, { ".-.", 'r' }, { "...", 's' }, { "-", 't' }, { "..-", 'u' }, { "...-", 'v' }, { ".--", 'w' }, { "-..-", 'x' }, { "-.--", 'y' }, { "--..", 'z' }, { ".----", '1' }, { "..---", '2' }, { "...--", '3' }, { "....-", '4' }, { ".....", '5' }, { "-....", '6' }, { "--...", '7' }, { "---..", '8' }, { "----.", '9' }, { "-----", '0' }, { "..--..", '?' }, { ".-.-.-", '.' }, { "--..--", ',' }, { "-.-.--", '!' }, { ".--.-.", '@' }, { "---...", ':' }, { "-....-", '-' }, { "-..-.", '/' }, { "-.--.", '(' }, { "-.--.-", ')' }, { ".-...", '&' }, { "...-..-", '$' }, { "...-.-", '>' }, { ".-.-.", '<' }
};

void CodeToChar() {
  std::string codeStr(CodeBuffer);  // Convert CodeBuffer to a string

  auto it = morseToCharMap.find(codeStr);  // lookup
  if (it != morseToCharMap.end()) {
    char decode_char = it->second;  // Get the corresponding character
    DisplayCharacter(decode_char);
    // Serial_print(decode_char);
  }
}

//##########################################################################################################################//

const float stretch = 1.39;  // stretch width to 333 pixels
#define FRAMEBUFFER_STRETCHED_WIDTH (int)(FRAMEBUFFER_FULL_WIDTH * stretch)


void audioSpectrum256() {  // 512 bin audio watefrall, displays 240 channels after user inactivity

  if (audioMuted || showPanorama)  // showPanorama and audioSpectrum256() are mutually exclusive
    return;

  const int startX = 3;
  const int yOffset = 241;
  const int ampl = 80;
  int oldX = startX;
  int dmax = 30;
  int ctr = 0;


  tft.fillRect(2, 52, 338, 30, TFT_BLACK);   // overwrite area for spectrum
  tft.fillRect(2, 82, 338, 40, TFT_BLACK);   // overwrite area for waterfall
  tft.fillRect(0, 294, 230, 25, TFT_BLACK);  // Overwrite infobar

  if (!useNixieDial)
    tft.fillRect(330, 8, 135, 15, TFT_BLACK);  // Overwrite microvolts
  tft.fillCircle(470, 20, 2, TFT_BLACK);       // overwrite serial communication indicator

  tft.drawFastHLine(2, 81, 338, TFT_SKYBLUE);  // draw separator bar

  tft.setSwapBytes(true);

  framebuffer1 = (uint16_t*)heap_caps_malloc(FRAMEBUFFER_STRETCHED_WIDTH * AUDIO_FRAMEBUFFER_HEIGHT * sizeof(uint16_t), MALLOC_CAP_DMA);
  if (framebuffer1 == NULL)
    buffErr();

  memset(framebuffer1, 0, FRAMEBUFFER_STRETCHED_WIDTH * AUDIO_FRAMEBUFFER_HEIGHT * sizeof(uint16_t));  //black background

  for (int strX = 0; strX < FRAMEBUFFER_FULL_WIDTH; strX++) {
    stretchedX[strX] = round(strX * stretch);  // Precompute stretchedX;
  }


  while (digitalRead(ENCODER_BUTTON) == HIGH && !clw && !cclw && !get_Touch()) {

    tft.fillRect(135, startY, 86, 27, TFT_NAVY);  //background mini osci
    ctr++;

    if (ctr == 4) {  // 1 out of 4 loops
      ctr = 0;
      printMajorPeak();
      si4735.getCurrentReceivedSignalQuality(0);
      signalStrength = si4735.getCurrentRSSI();
      dBm = signalStrength - 107 + RFGainCorrection;


      if (TVTunerActive)  // at 1mV start tuner AGC
        dBm -= tunerGain;

      if (dBm < 0 && (!scanMode) && showMeters)
        plotNeedle(signalStrength, 3);  // update the SMeter needle
    }


    FFTSample(512, 0, true);

    for (int i = 0; i < FRAMEBUFFER_FULL_WIDTH; i++) {  //  process 0 - 2.75 KHz in 240 bins

      int yP = DISP_HEIGHT - 1 - Rpeak[i];
      int strX = stretchedX[i] + startX;

      if (Rpeak[i]) {
        int yPos = yP - yOffset;
        tft.fillRect(strX, yPos, 1, 2, TFT_BLACK);  // decay, draw 2-pixel-high black line

        if (strX > oldX + 1)
          tft.fillRect(oldX + 1, yPos, 1, 2, TFT_BLACK);  // gap decay
        Rpeak[i] -= 2;
      }

      int dsize = RvReal[i] / ampl;
      if (dsize > dmax) dsize = dmax;

      uint16_t clr = valueToWaterfallColor(RvReal[i]);


      if (i > 1 && clr > 25)  // discard first 2 bins and set background noise treshold
        newLine[strX] = clr;
      else
        newLine[strX] = 0;

      if (dsize > Rpeak[i])
        Rpeak[i] = dsize;


      if (i > 1 && dsize > 1) {
        tft.fillRectVGradient(strX, DISP_HEIGHT - dsize - yOffset, 1, dsize, TFT_SILVERBLUE, TFT_BLUE);
        //tft.drawFastVLine(strX, DISP_HEIGHT - 1 - dsize - yOffset, dsize, clr); // spectrum

        if (strX > oldX + 1)
          tft.fillRectVGradient(oldX + 1, DISP_HEIGHT - dsize - yOffset, 1, dsize, TFT_SILVERBLUE, clr);  // stretch gap fill
                                                                                                          //tft.drawFastVLine(oldX  + 1, DISP_HEIGHT - 1 - dsize - yOffset, dsize, clr);
      }

      oldX = strX;
    }

    addLineToFramebuffer1(newLine);
  }
  free(framebuffer1);
  tft.fillRect(135, startY, 86, 27, TFT_BLACK);  // overwrite last miniwindow
  tft.fillRect(260, 50, 70, 10, TFT_BLACK);      // overwrite last frequency peak
  for (int i = 0; i < 255; i++)                  // reset to 0 to avoid artefacts when starting mini spectrum 16
    Rpeak[i] = 0;

  tft.setSwapBytes(false);  // needed to swap when pushing
  redrawIndicators();
  readMainBtns();
}


//##########################################################################################################################//
void FFTSample(int sampleCount, int dly, bool drawOsci) {

  peakVol = 0;
  const int centerLineH = 308;  // centerline position
  const int amplPreset = 2;     // amplification preset. This depends on the amplification of the FFT amplifier trannsistor and may need to get adjusted 
  int32_t sum = 0;

  for (int i = 0; i < sampleCount; i++) {  // sampling loop
    sum = 0;

    if (!dly) {
      // Use oversampling instead of delay, 0-3KHz
      for (int j = 0; j < 4; j++) {
        sum += (analogRead(AUDIO_INPUT_PIN) - gpio36_Offset);
      }
    } else {
      sum = analogRead(AUDIO_INPUT_PIN) - gpio36_Offset;  // remove the dc offset caused by the transistor collector voltage at around 1.65V
      delayMicroseconds(dly);
    }


    if (drawOsci) {

      int am = (amplPreset * sum) / (255 - FFTGain ) ;  // calculate and limit amplitude
      
      am = constrain(am, -12, 12);
      if (i < 86)
        tft.drawPixel(135 + i, centerLineH + am, TFT_WHITE);  // draw trace
    }
    // Adjust and store the averaged value

    sum /= sampleCount / FFTGain;
    
    RvReal[i] = (float) sum;
    RvImag[i] = 0;
  }
  FFT.windowing(RvReal, sampleCount, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(RvReal, RvImag, sampleCount, FFT_FORWARD);
  FFT.complexToMagnitude(RvReal, RvImag, sampleCount);

  for (int i = 2; i < SAMPLES / 2; i++) {
    peakVol += (int)RvReal[i];
  }
  peakVol = constrain (peakVol, 0, 100000);


}
//##########################################################################################################################//

void addLineToFramebuffer1(uint16_t* newLine) {
  // Shift all existing lines down by one
  for (int y = AUDIO_FRAMEBUFFER_HEIGHT - 1; y > 0; y--) {
    memcpy(&framebuffer1[y * FRAMEBUFFER_STRETCHED_WIDTH],
           &framebuffer1[(y - 1) * FRAMEBUFFER_STRETCHED_WIDTH],
           FRAMEBUFFER_STRETCHED_WIDTH * sizeof(uint16_t));
  }

  // Copy the new line into the top row
  memcpy(&framebuffer1[0], newLine, FRAMEBUFFER_STRETCHED_WIDTH * sizeof(uint16_t));

  // Push the entire framebuffer to the screen
  updateDisp();
}

//##########################################################################################################################//
void updateDisp() {
  const int upper = 84;
  const int lower = 124;
  const int startX = 3;
  const int width = FRAMEBUFFER_STRETCHED_WIDTH;
  const int height = lower - upper;

  // Push  entire framebuffer
  tft.pushImage(startX, upper, width, height, framebuffer1);
}


//##########################################################################################################################//
void audioFreqAnalyzer() {
  int y = 15;
  const float stretchFactor = 1.92;


  for (int i = 1; i < DISP_WIDTH; i++)  // clean line buffer
    newLine[i] = 0;

  tft.fillScreen(TFT_BLACK);
  audioScale();
  si4735.setAudioMute(false);
  si4735.setHardwareAudioMute(false);

  tft.fillRect(0, 305, 240, 15, TFT_BLACK);
  tft.setCursor(0, 305);
  tft.printf("Frequency: %ld", FREQ);

  // Need byte swapping for pushImage
  tft.setSwapBytes(true);

  while (digitalRead(ENCODER_BUTTON) == HIGH && !get_Touch()) {
    FFTSample(512, 0, 0);

    if (clw || cclw) {
      if (clw) FREQ += 100;
      if (cclw) FREQ -= 100;

      tft.fillRect(0, 305, 480, 15, TFT_BLACK);
      tft.setCursor(0, 305);
      tft.printf("Frequency: %ld", FREQ);
      setLO();
      clw = false;
      cclw = false;
    }

    // Fill linebuffer
    for (int i = 1; i < DISP_WIDTH; i++) {
      int fftIndex = (int)(i / stretchFactor);
      if (fftIndex >= SAMPLES) fftIndex = SAMPLES - 1;
      newLine[i] = valueToWaterfallColor((int)(RvReal[fftIndex]));
    }

    // Push line
    tft.pushImage(0, y, DISP_WIDTH, 1, newLine);

    y++;

    drawRandomObjects();
    if (y == 300) {
      y = 15;
      tft.fillScreen(TFT_BLACK);
      audioScale();
    }
  }

  tft.setSwapBytes(false);
  rebuildMainScreen(0);
}

//##########################################################################################################################//


void audioScale() {

  tft.setTextSize(1);
  tft.drawFastHLine(0, 0, WATERFALL_SCREEN_WIDTH, TFT_WHITE);
  tft.drawFastVLine(WATERFALL_SCREEN_WIDTH - 1, 0, 5, TFT_WHITE);
  for (float i = 0; i < WATERFALL_SCREEN_WIDTH; i += WATERFALL_SCREEN_WIDTH / 10) {
    tft.drawFastVLine(i, 0, 5, TFT_WHITE);
    tft.setCursor(i, 8);
    if (i < WATERFALL_SCREEN_WIDTH)
      tft.printf("%.2f", i / 160);
  }
  tft.setTextSize(2);
}
//##########################################################################################################################//


void printMajorPeak() {


  tft.fillRect(260, 50, 70, 10, TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(260, 50);
  tft.setTextSize(1);

  float cpy[512] = { 0 };
  for (int i = 2; i < 512; i++)  // copy and disregard first 2 elements to remove DC spike
    cpy[i] = RvReal[i];
  pk = FFT.majorPeak(cpy, 512, 5930);  //2.75KHz
  tft.printf("Peak:%4.0fHz", pk);      // display peak frequency
  tft.setTextSize(2);
}



//##########################################################################################################################//


void audioScan() {  //  240 channels spectrum in slow scan mode

  const int startX = 3;
  const int yOffset = 30;
  const int ampl = 40;
  const float stretch = 1.39;
  int oldX = startX;
  int dmax = 55;


  for (int strX = 0; strX < FRAMEBUFFER_FULL_WIDTH; strX++) {
    stretchedX[strX] = round(strX * stretch);  // Precalculate stretchedX;
  }

  FFTSample(512, 0, false);

  for (int i = 0; i < FRAMEBUFFER_FULL_WIDTH; i++) {  //  process 0 - 2.75 KHz in 240 bins

    int yP = DISP_HEIGHT - 1 - Rpeak[i];
    int strX = stretchedX[i] + startX;

    if (Rpeak[i]) {
      int yPos = yP - yOffset;
      tft.fillRect(strX, yPos, 1, 2, TFT_BLACK);  // decay, draw 2-pixel-high black line

      if (strX > oldX + 1)
        tft.fillRect(oldX + 1, yPos, 1, 2, TFT_BLACK);  // gap decay
      Rpeak[i] -= 2;
    }

    int dsize = RvReal[i] / ampl;

    if (i > 1 && dsize > audioPeakVal)
      audioPeakVal = dsize;

    if (dsize > dmax) dsize = dmax;



    uint16_t clr = valueToWaterfallColor(RvReal[i]);


    if (i > 1 && clr > 25)  // discard first 2 bins and set background noise treshold
      newLine[i] = clr;
    else
      newLine[i] = 0;

    if (dsize > Rpeak[i])
      Rpeak[i] = dsize;


    if (i > 1 && dsize > 1) {
      tft.fillRectVGradient(strX, DISP_HEIGHT - dsize - yOffset, 1, dsize, TFT_SILVERBLUE, TFT_BLUE);
      //tft.drawFastVLine(strX, DISP_HEIGHT - 1 - dsize - yOffset, dsize, clr); // spectrum

      if (strX > oldX + 1)
        tft.fillRectVGradient(oldX + 1, DISP_HEIGHT - dsize - yOffset, 1, dsize, TFT_SILVERBLUE, clr);  // stretch gap fill
      //tft.drawFastVLine(oldX  + 1, DISP_HEIGHT - 1 - dsize - yOffset, dsize, clr);
    }
    oldX = strX;
  }
}
