// initialize HW, draw permanent buttons and  screen elements

void bootScreen() {
  int16_t si4735Addr = 0;
  tft.init();

#ifdef FLIP_IMAGE
 tft.setRotation(3);
#else
tft.setRotation(1);
#endif


  tft.setTextColor(TFT_FOREGROUND);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 30);
  tft.print("Getting SI4732 address...\n");

  do {
    si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
    if (I2C_BUSSPEED > 200000)
      I2C_BUSSPEED -= 100000;
    else {
      tft.setCursor(0, 100);
      tft.print("SI4732 NOT FOUND!");
      delay(2000);
      break;
    }
    Wire.end();
    Wire.begin(21, 22, I2C_BUSSPEED);
    tft.fillRect(0, 50, 480, 20, TFT_BLACK);
    tft.setCursor(0, 50);
    tft.printf("I2C bus frequency %ld", I2C_BUSSPEED);
    delay(100);
  }

  while (!si4735Addr);

  tft.fillScreen(TFT_BLACK);
  fastBoot = preferences.getBool("fB", 0);
 
 if (!fastBoot) {
 
#ifdef SHOW_SPLASHSCREEN
    LittleFS.begin(true);  // bool formatOnFail = true
    if (LittleFS.exists("/splash.jpg")){
    swappedJPEG = true;    // depends on file format
    Serial.print("Loading splash\n");
    drawJpeg("/splash.jpg", 0, 0);
    } 
   else 
     Serial.print("splash not found\n");
    LittleFS.end();
#endif
    tft.setTextSize(3);
    tft.setCursor(0, 30);

#ifdef TV_TUNER_PRESENT
    tft.println(" 0.1 - 860MHz\n\n AM FM SSB CW\n\n Wide Band Receiver\n");
#endif

#ifndef TV_TUNER_PRESENT
    tft.println(" 0.1-50MHz\n\nUniversal Receiver\n");
#endif

    tft.setTextSize(2);
    tft.setCursor(0, 180);
    if (si4735Addr == 0) {
      tft.setTextColor(TFT_RED);
      tft.print("Si4732 not found!!");
    } else {
      tft.setTextColor(TFT_GREEN);
      Serial_println("Si4732 found");
      tft.print("Si4732 found, Addr: ");
      tft.println(si4735Addr, HEX);
    }

    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(0, 260);
    tft.print("Touch screen to see settings,\nor to calibrate touchscreen");
    tft.setTextColor(textColor);
    tft.setCursor(0, 200);
    tft.print(ver);

    if (si4735Addr == 0x11) {
      si4735.setDeviceI2CAddress(0);
      SI4732found = true;
    } else if (si4735Addr == 0x63) {
      SI4732found = true;
      si4735.setDeviceI2CAddress(1);
    }
  }
   
 if (fastBoot) 
   preferences.putBool("fB", false);  // remove fastboot flag

}
//##########################################################################################################################//

void spriteBorder() {

  tft.pushImage(0, 0, 480, 1, (uint16_t *)border480);
  tft.pushImage(0, 1, 480, 1, (uint16_t *)border480);
  tft.pushImage(0, 292, 480, 1, (uint16_t *)border480);
  tft.pushImage(0, 293, 480, 1, (uint16_t *)border480);
  tft.pushImage(0, 0, 1, 292, (uint16_t *)border320);
  tft.pushImage(1, 0, 1, 292, (uint16_t *)border320);
  tft.pushImage(478, 0, 1, 292, (uint16_t *)border320);
  tft.pushImage(479, 0, 1, 292, (uint16_t *)border320);
}

//##########################################################################################################################//

void drawFrame() {

  tft.fillScreen(TFT_BLACK);
  spriteBorder();  // loade border lines
  //tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  tft.drawFastHLine(0, 46, DISP_WIDTH, TFT_GRID);
  tft.drawFastVLine(340, 46, 246, TFT_GRID);
  tft.fillCircle(162 + 13, 47, 3, TFT_GREEN);  //draw a circle to display finetune pot center
}


//##########################################################################################################################//


void SI5351_Init() {

  tft.setTextSize(2);
  bool i2c_found;
  for (int i = 0; i < 10; i++) {
    i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    if (!i2c_found) {
      tft.setCursor(10 + 10 * i, 60);
      tft.print(".");
      delay(100);
    }
  }

  if (!i2c_found) {
    tft.print(" SI5351 not found!\n");
    Serial_print(" SI5351 not found!\n");
    return;
  }

  else
    Serial_print(" SI5351 found!\n");

  SI5351calib = preferences.getLong("calib", 0);
  si5351.set_correction(SI5351calib, SI5351_PLL_INPUT_XO);  // read calibration from preferences

  si5351.set_ms_source(SI5351_CLK2, SI5351_PLLA);
  si5351.output_enable(SI5351_CLK2, 1);


#ifdef SI5351_GENERATES_CLOCKS
#ifdef TV_TUNER_PRESENT
  si5351.set_ms_source(SI5351_CLK1, SI5351_PLLA);  // 4Mhz for TV tuner
  si5351.output_enable(SI5351_CLK1, 1);
  si5351.set_freq(4000000L * 100ULL, SI5351_CLK1);
  Serial_println("CLK1 enabled at 4MHz");
#endif
  si5351.set_ms_source(SI5351_CLK0, SI5351_PLLB);  // 32768Hz for SI4732
  si5351.output_enable(SI5351_CLK0, 1);
  si5351.set_freq(32768L * 100ULL, SI5351_CLK0);
  Serial_println("CLK0 enabled at 32768Hz");
#endif
}
//##########################################################################################################################//


void radioInit() {

  dac1.outputVoltage((uint8_t)(180));  // set to full VHF/UHF gain

#ifdef NBFM_DEMODULATOR_PRESENT
  afcVoltage = analogRead(TUNING_VOLTAGE_READ_PIN);
  Serial_printf("AFC DC offset %d\n", afcVoltage);
#endif

  /*
Parameters
uint8_t	CTSIEN sets Interrupt anabled or disabled (1 = anabled and 0 = disabled )
uint8_t	GPO2OEN sets GP02 Si473X pin enabled (1 = anabled and 0 = disabled )
uint8_t	PATCH Used for firmware patch updates. Use it always 0 here.
uint8_t	XOSCEN sets external Crystal enabled or disabled. 0 = Use external RCLK (crystal oscillator disabled); 1 = Use crystal oscillator
uint8_t	FUNC sets the receiver function have to be used [0 = FM Receive; 1 = AM (LW/MW/SW) and SSB (if SSB patch apllied)]
uint8_t	OPMODE set the kind of audio mode you want to use.
*/



#ifdef SI5351_GENERATES_CLOCKS
  si4735.setPowerUp(0, 0, 0, 0, 1, 5); 

#else
  si4735.setPowerUp(0, 0, 0, 1, 1, 5);
#endif

  si4735.radioPowerUp();
  si4735.setAM(520, 29900, SI4735TUNED_FREQ, 1);



  si4735.setAudioMuteMcuPin(MUTEPIN);
  digitalWrite(MUTEPIN, HIGH);
  dcOffset = analogRead(AUDIO_INPUT_PIN);  // need to measure the collector voltage of the tft amplifier to center mini oscilloscope
  Serial_printf("FFT DC Offset %d\n", dcOffset);


  if (modType == USB || modType == LSB)
    STEP = DEFAULT_SSB_STEP;
  else if (modType == CW)
    STEP = DEFAULT_CW_STEP;
  else if (modType == SYNC)
    STEP = DEFAULT_SYNC_STEP;
  else if (modType == AM)
    STEP = DEFAULT_AM_STEP;
  else if (modType == NBFM)
    STEP = DEFAULT_NBFM_STEP;




  vol = preferences.getChar("Vol", 50);  // get global volume
  si4735.setVolume(vol);
  si4735.setAMSoftMuteSnrThreshold(preferences.getChar("SMute", 0));
  si4735.setAvcAmMaxGain(preferences.getChar("AVC", 0));
  digitalWrite(MUTEPIN, LOW);
  digitalWrite(IF_FILTER_BANDWIDTH_PIN, HIGH);  // default use wide IF filter
  digitalWrite(NBFM_MUTE_PIN, LOW);             // LOW means the NBFM demodulator is muted

  loadSi4735parameters(); // finally load the parameters 
}

//##########################################################################################################################//

void tinySAInit() {


  //Serial_println("color 8 0x0009ff"); // trace 3 blue
  //delay(50);

#ifdef TINYSA_PRESENT
  loadRFMode();
  delay(50);
  Serial_println("marker 2 peak");
#endif
}

/*
Example for change trace 1 color to hex = #de0707:
'color 6 0xde0707'

read current colors:
'color'
Color indexes:
#define LCD_BG_COLOR             0
#define LCD_FG_COLOR             1
#define LCD_GRID_COLOR           2
#define LCD_MENU_COLOR           3
#define LCD_MENU_TEXT_COLOR      4
#define LCD_MENU_ACTIVE_COLOR    5
#define LCD_TRACE_1_COLOR        6
#define LCD_TRACE_2_COLOR        7
#define LCD_TRACE_3_COLOR        8
#define LCD_TRACE_4_COLOR        9
#define LCD_NORMAL_BAT_COLOR    10
#define LCD_LOW_BAT_COLOR       11
#define LCD_TRIGGER_COLOR       12
#define LCD_RISE_EDGE_COLOR     13
#define LCD_FALLEN_EDGE_COLOR   14
#define LCD_SWEEP_LINE_COLOR    15
#define LCD_BW_TEXT_COLOR       16
#define LCD_INPUT_TEXT_COLOR    17
#define LCD_INPUT_BG_COLOR      18
#define LCD_BRIGHT_COLOR_BLUE   19
#define LCD_BRIGHT_COLOR_RED    20
#define LCD_BRIGHT_COLOR_GREEN  21
#define LCD_DARK_GREY           22
#define LCD_LIGHT_GREY          23
#define LCD_HAM_COLOR           24
#define LCD_GRID_VALUE_COLOR    25
#define LCD_M_REFERENCE         26
#define LCD_M_DELTA             27
#define LCD_M_NOISE             28
#define LCD_M_DEFAULT           29

*/



//##########################################################################################################################//

void drawBigBtns() {

  tft.fillRect(345, 55, 130, 236, TFT_BLACK);

  if (!showMeters) {
    drawButton(345, 50, 130, 78, TFT_BLUE, TFT_NAVY);   // UP button
    drawButton(345, 129, 130, 78, TFT_BLUE, TFT_NAVY);  // DOWN button
    drawButton(345, 209, 130, 78, TFT_BLUE, TFT_NAVY);  // MODE button
  }

  else {

    tft.pushImage(345, 213, 130, 76, (uint16_t *)meterFrame);

    analogMeter(ySh);   // Draw upper analogue meter
    analogMeter(ySh2);  // Draw lower analogue meter
    plotNeedle2(0, 0);
  }


    tft.setTextFont(4); 
    tft.setTextSize(1); 
  if (!scanMode) {
    if (!showMeters) {
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(375, 158);
      tft.print("DOWN");
      tft.setCursor(390, 80);
      tft.print("UP");
    }

    else
      tft.setTextColor(textColor);
    tft.setCursor(375, 240);
    tft.print("MODE");
 
    
  }



  else {
    drawButton(345, 50, 130, 78, TFT_BLUE, TFT_NAVY);
    drawButton(345, 129, 130, 78, TFT_BLUE, TFT_NAVY);
    drawButton(345, 209, 130, 78, TFT_BLUE, TFT_NAVY);
    tft.setTextColor(TFT_SKYBLUE);
    tft.setCursor(370, 145);
    tft.print("SET");
    tft.setCursor(370, 175);
    tft.print("RANGE");
    tft.setCursor(375, 63);
    tft.print("SEEK");
    tft.setCursor(375, 93);
    tft.print("UP");
    tft.setCursor(375, 222);
    tft.print("SEEK");
    tft.setCursor(375, 252);
    tft.print("DOWN");
    tft.setTextSize(2);
    tft.setTextColor(textColor);
  }
  tft.setTextFont(1);  
  tft.setTextSize(2);
}


//##########################################################################################################################//


void loadLastSettings() {


  pinMode(ENCODER_BUTTON, INPUT_PULLUP);  // needed during while(true)




  FREQ = preferences.getLong("lastFreq", 0);          // load last Freq
  bandWidth = preferences.getInt("lastBw", 0);        // last bandwidth
  modType = preferences.getChar("lastMod", 1);        //last modulation type
  altStyle = preferences.getBool("lastStyle", 0);     //plain or sprite style
  pressSound = preferences.getBool("pressSound", 0);  // short beep when pressed
  miniWindowMode = preferences.getChar("spectr", 3);  // audio spectrum analyzer mode
#ifdef TINYSA_PRESENT
  syncEnabled = preferences.getBool("useTSADBm", 0);  // use tinySA for DBm
#endif
  SNRSquelch = preferences.getBool("SNRSquelch", 0);         // SNR OR RSSI can open squelch
  buttonSelected = preferences.getInt("sprite", 1);          // load sprite for buttons
  loopBands = preferences.getBool("uBL", 0);                 // use band limits or not
  smoothColorGradient = preferences.getBool("smoothWF", 0);  // smooth waterfall colors
  SI4735TUNED_FREQ = preferences.getLong("IF", 21397);       // IF in KHz
  showAudioWaterfall = preferences.getBool("audiowf", 0);    // Audio waterfall after 2 minutes of inactivity
  showPanorama = preferences.getBool("showPan", 0);          // show +- 500 KHz when squelch closed
  NBFMOffset = preferences.getInt("NBFMOffset", 0);          // Offset to demodulate NBFM on flank
  showMeters = preferences.getBool("sM", 0);                 // meters cause a lot of traffic on SPI (and noise)
  tunerOffsetPPM = preferences.getInt("tunerOffsetPPM", 0);  // tuner crystal frequency correction
  RFGainCorrection = preferences.getInt("rfgc", 6);          // adjust gain for preamp/filter
  FFTGain = preferences.getInt("FFTGain", 100);              // initial gain for FFT analysis
  discriminatorZero = preferences.getInt("dZero", 100);
  initialGain = preferences.getUChar("agcS", 200);  // tuner agc start value.
  funEnabled = preferences.getBool("fun", 0);
  useNixieDial = preferences.getBool("dial", 0); 

  Serial_printf("\n%-35s %s\n", "Settings loaded:", "");
  Serial_printf("%-35s %ld Khz\n", "FREQ:", FREQ / 1000);
  Serial_printf("%-35s %d Khz\n", "IF:", SI4735TUNED_FREQ);
  Serial_printf("%-35s %d\n", "Mod Type:", modType);
  Serial_printf("%-35s %s\n", "Alt Style:", altStyle ? "Enabled" : "Disabled");
  Serial_printf("%-35s %d\n", "Mini Window Mode:", miniWindowMode);
  Serial_printf("%-35s %s\n", "SNR Squelch:", SNRSquelch ? "Enabled" : "Disabled");
  Serial_printf("%-35s %d\n", "Sprite Style:", buttonSelected);
  Serial_printf("%-35s %s\n", "Loop bands when tuning:", loopBands ? "Yes" : "No");
  Serial_printf("%-35s %s\n", "Show analog meters:", showMeters ? "Yes" : "No");
  Serial_printf("%-35s %s\n", "Smooth Waterfall Colors:", smoothColorGradient ? "Yes" : "No");
  Serial_printf("%-35s %s\n", "Audio Waterfall after inactivity:", showAudioWaterfall ? "Yes" : "No");
  Serial_printf("%-35s %d\n", "Touch tune mode:", preferences.getChar("tGr", 0));
  Serial_printf("%-35s %d\n", "Master Volume:", preferences.getChar("Vol", 50));
  Serial_printf("%-35s %d\n", "NBFM Offset:", NBFMOffset);
  Serial_printf("%-35s %d\n", "RF Gain Correction:", RFGainCorrection);
  Serial_printf("%-35s %d ppm\n", "TV Tuner Offset:", tunerOffsetPPM);
  Serial_printf("%-35s %s\n", "Show panorama when squelch closed:", showPanorama ? "Yes" : "No");
  Serial_printf("%-35s %d\n", "Tuner AGC start value", initialGain);
  Serial_printf("%-35s %s\n", "Retro dial:", useNixieDial ? "Yes" : "No");
  tRel();

if (! fastBoot) { // provide option to calibrate touchscreen
 
  for (int i = 0; i < 2000; i++) {
    delay(1);
    get_Touch();

    if (pressed) {
      tft.setTextSize(1);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Settings loaded:\n");
      tft.printf("FREQ: %ld Khz\n", FREQ / 1000);
      tft.printf("IF: %d Khz\n", SI4735TUNED_FREQ);
      tft.printf("Mod Type: %d\n", modType);
      tft.printf("Alt Style: %s\n", altStyle ? "Enabled" : "Disabled");
      tft.printf("Mini Window Mode: %d\n", miniWindowMode);
      tft.printf("SNR Squelch: %s\n", SNRSquelch ? "Enabled" : "Disabled");
      tft.printf("Sprite Style: %d\n", buttonSelected);
      tft.printf("Loop bands when tuning: %s\n", loopBands ? "Yes" : "No");
      tft.printf("Show analog meters: %s\n", showMeters ? "Yes" : "No");
      tft.printf("Smooth Waterfall Colors: %s\n", smoothColorGradient ? "Yes" : "No");
      tft.printf("Audio Waterfall after inactivity: %s\n", showAudioWaterfall ? "Yes" : "No");
      tft.printf("Touch tune mode: %d\n", preferences.getChar("tGr", 0));
      tft.printf("Master Volume: %d\n", preferences.getChar("Vol", 50));
      tft.printf("NBFM Offset: %d\n", NBFMOffset);
      tft.printf("RF Gain Correction: %d\n", RFGainCorrection);
      tft.printf("TV Tuner Offset: %dppm\n", tunerOffsetPPM);
      tft.printf("Show panorama when squelch closed %s\n", showPanorama ? "Yes" : "No");
      tft.printf("%-35s %s\n", "Retro dial:", useNixieDial ? "Yes" : "No");
      tft.printf("%-35s %d\n", "Tuner AGC start value", initialGain);
      tft.setTextSize(2);
      tft.setTextColor(TFT_RED);
      tft.print("\nPress encoder for touch calibration,\nor touch the screen to continue.");
      tft.setTextColor(textColor);

      tRel();

      while (1) {
        if (digitalRead(ENCODER_BUTTON) == LOW) {
          touchCal();
          return;
        }

        else if (get_Touch()) {
          pressed = false;
          return;
        }
      }
    }
  }
}


}

//##########################################################################################################################//