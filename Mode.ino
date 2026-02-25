//##########################################################################################################################//
void setMode() {  // gets called from loop, selects btw. normal or scan mode, activates freq up and freq down. Calls selectModulation() too.

  static uint16_t dly = 0;
  static bool stop = false;
  const int btLeftBorder = 350;



  if((tx > btLeftBorder && pressed && !scanMode) || (!scanMode && pressed && ty < 65)){
    // UP or DOWN pressed or frequency display left or right pressed
    disableFFT = true;
    freq_UP_DOWN();
  }


#ifdef NBFM_DEMODULATOR_PRESENT 

 if ( (tx > btLeftBorder) && (ty > 45) && (ty < 115) & pressed && !scanMode && showMeters) // set AFC on/off and display small green AFC indicator in upper meter
     getAFCstatus();  

#endif


  if (!scanMode && lim1 && lim2) {  // reset scan parameters
    lim1 = 0;
    lim2 = 0;
    showScanRange = false;
    stop = true;
  }

  if (scanMode) {


    if (keyVal && showScanRange && lim1 && lim2) { // scan with frequency limits
      tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite row 3 to make room for scan parameters
      printScanRange();                          // print scan parameters once (not in every cycle)
      showScanRange = false;
    }

    getRSSIAndSNR();


    if ((signalStrength > currentSquelch) || (SNRSquelch && SNR) || clw || cclw) {  // Scan stops when signal or encoder moved
      stop = true;
      dly = 0;
    }


    if (signalStrength < currentSquelch && keyVal == 2)  // Scan continues when signal drops
      dly++;

    if (dly == 100) {  // insert some seconds of delay before continuing to scan
      stop = false;
      dly = 0;
    }

    if (pressed)
      stop = false;

    if (lim1 && lim2) {
      if (FREQ >= lim1)  // keep frequency within boundaries
        FREQ = lim2;
      if (FREQ < lim2)
        FREQ = lim1;
    }

#ifdef FAST_TOUCH_HANDLER
    delay(10);
#endif

    scanUpDown(stop);
  }

  if ((ty > 215 && ty < 295) && !scanMode && tx > btLeftBorder) {  // Mode button pressed
    tRel();
    selectModulation();
  }
}

//##########################################################################################################################//

void selectModulation() {



  int lastmodType = modType;
  tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite row 3 from mainscreen
  tft.fillRect(345, 48, 130, 242, TFT_BLACK);
  draw12Buttons(TFT_BLUE, TFT_NAVY);  // draw new buttons
  tft.setTextColor(TFT_GREEN);



#ifdef AUDIO_SQUAREWAVE_PRESENT
  tft.setCursor(21, 132);
  tft.print("SSTV");
  tft.setCursor(21, 152);
  tft.print("Sc/Ma");
  tft.setCursor(105, 132);
  tft.print("RTTY");
  tft.setCursor(105, 152);
  tft.print("45.45");
#endif


  tft.setCursor(21, 198);
  tft.print("AM");
  tft.setCursor(105, 198);
  tft.print("LSB");
  tft.setCursor(188, 198);
  tft.print("USB");
  tft.setCursor(280, 198);
  tft.print(" ");
  tft.setCursor(20, 254);
  tft.print("WBFM");
  tft.setCursor(275, 198);
  tft.print("NBFM");
  tft.setCursor(188, 255);
  tft.print("CW");
  tft.setCursor(280, 245);
  tft.print("CW");
  tft.setCursor(275, 265);
  tft.print("DECO");
  tft.setCursor(105, 255);
  tft.print("SYNC");



  if (funEnabled) {
    pressed = false;
    int ctr = 0;
    while (!get_Touch()) {
      ctr++;
      delay(100);
      if (ctr > 20)
        pacM(false);
    }
  } else
    tPress();

  int buttonID = getButtonID();
   if (!buttonID)
    return;  // outside of area

  switch (buttonID) {

    case 21:
#ifdef AUDIO_SQUAREWAVE_PRESENT
      SSTVINIT();
      rebuildMainScreen(1);
#endif
      break;
    case 22:
#ifdef AUDIO_SQUAREWAVE_PRESENT
      RTTYINIT();
      rebuildMainScreen(1);
#endif
      break;
    case 31:
      modType = AM;
      break;
    case 32:
      modType = LSB;
      break;
    case 33:
      modType = USB;
      break;
    case 34:
      modType = NBFM;
      break;
    case 41:
      lastAMFREQ = FREQ;  // save last non WBFM FREQ
      modType = WBFM;
      WBFMactive = true;
      break;
    case 42:
      modType = SYNC;
      break;
    case 43:
      modType = CW;
      break;
    case 44:
      modType = CW;  // CW decoder
      loadSi4735parameters();
      CWDecoder();
      rebuildMainScreen(1);
      loadSi4735parameters();
      modType = USB;  // need to change so that the decoder gets reloaded when selected CW again
      STEP = DEFAULT_SSB_STEP;
      si4735.setSSBAudioBandwidth(3);  // 4 KHz
      break;
    default:
      break;
  }


  if (modType != lastmodType) {
    Serial_printf("modType %d loaded\n", modType);
    loadSi4735parameters();
  }
  resetMainScreen();
  mainScreen();
}



//##########################################################################################################################//

void loadSi4735parameters() { //for SSB this needs to consider whether in single or double conversion mode
                              //we are using high side injection (LO above RF) --> in single conversion mode the sidebands get inverted.
                              // double conversion mode means that the sidebands get inverted twice which brings them back to normal     

  si4735.setVolume(0);  // avoid volume peaks, especially when loading SSB patch

  if (modType != WBFM && WBFMactive == true) {  // restore last AM or SSB frequency and the step when coming from WBFM
    FREQ = lastAMFREQ;
    WBFMactive = false;
  }



  if (modType == AM) {
    digitalWrite(NBFM_MUTE_PIN, LOW);  //set pin LOW to mute the NBFM demodulator
    SI4735WBFMTune = false;
    bandWidth = lastAMBandwidth;
    STEP = DEFAULT_AM_STEP;
    roundFreqToSTEP();
    ssbLoaded = false;
    si4735.setAM(520, 29900, SI4735TUNED_FREQ, 1);
    si4735.setBandwidth(bandWidth, 1);

    //AMCHFLT	the choices are: 0 = 6 kHz Bandwidth 1 = 4 kHz Bandwidth 2 = 3 kHz Bandwidth 3 = 2 kHz Bandwidth 4 = 1 kHz Bandwidth 5 = 1.8 kHz Bandwidth 6 = 2.5 kHz Bandwidth, gradual roll off 7–15 = Reserved (Do not use).
    //AMPLFLT	Enables the AM Power Line Noise Rejection Filter.
  }

  if (modType == LSB || modType == USB || modType == SYNC) {
    digitalWrite(NBFM_MUTE_PIN, LOW);  // set pin LOW to mute the NBFM demodulator
    SI4735WBFMTune = false;
    STEP = DEFAULT_SSB_STEP;
    roundFreqToSTEP();
    bandWidth = lastSSBBandwidth;
    loadSSB();
  }


  int offset = 0; // load corresponding BFO offset



  if (modType == USB && singleConversionMode )
    offset = preferences.getInt("B1", 0);
  if (modType == USB && !singleConversionMode )
    offset = preferences.getInt("B2", 0);

  if (modType == LSB && singleConversionMode )
    offset = preferences.getInt("B3", 0);
  if (modType == LSB && !singleConversionMode )
    offset = preferences.getInt("B4", 0);


  if (modType == USB) {
    if (singleConversionMode ) {  // sidebands get inverted when in singleConversionMode 
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 1);  // Set LSB mode
      si4735.setSSBBfo(offset);
    } else {
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);  // Set USB mode
      si4735.setSSBBfo(offset);
    }
  }



  if (modType == LSB) {
    if (singleConversionMode ) { // sidebands get inverted when in singleConversionMode 
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);  
      si4735.setSSBBfo(offset);
    } else {
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 1);
      si4735.setSSBBfo(offset);  // Normal LSB
    }
  }


  if (modType == SYNC) {
    STEP = DEFAULT_SYNC_STEP;
    roundFreqToSTEP();
    si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);  // SYNC uses USB mode
    si4735.setSSBConfig(bandWidth, 1, 3, 1, 0, 0);      // SYNC config mode
    si4735.setSSBBfo(preferences.getInt("SYNCBBfoOffset", 0));
    loadSSB();
    ssbLoaded = false;  // Reload SSB when changing to USB/LSB to disable AFC
  }


  if (modType == CW) {
    digitalWrite(NBFM_MUTE_PIN, LOW);  //set pin LOW to mute the NBFM demodulator
    loadSSB();
    STEP = DEFAULT_CW_STEP;
    bandWidth = 5;  // 1KHz
    SI4735WBFMTune = false;
    si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);
    si4735.setSSBBfo(preferences.getInt("CWBfoOffset", 500));
    si4735.setSSBAudioBandwidth(5);  // 1KHz---
    si4735.setSBBSidebandCutoffFilter(0);
    ssbLoaded = false;  // need to reload SSB bandwidth
  }





  if (modType == NBFM) {
    SI4735WBFMTune = false;
    bandWidth = lastAMBandwidth;
    STEP = DEFAULT_NBFM_STEP;
    roundFreqToSTEP();
    ssbLoaded = false;
#ifndef NBFM_DEMODULATOR_PRESENT
    si4735.setAM(520, 29900, SI4735TUNED_FREQ + preferences.getInt("NBFMOffset", 0), 1);  // load the offset from preferences
#endif

#ifdef NBFM_DEMODULATOR_PRESENT
    si4735.setAM(520, 29900, SI4735TUNED_FREQ, 1);  // initiate the AM subsystem for squelch and RSSI
    digitalWrite(NBFM_MUTE_PIN, HIGH);              // set pin HIGH to unmute the NBFM demodulator
#endif
  }


  if (modType == WBFM) {
    digitalWrite(NBFM_MUTE_PIN, LOW);  //set pin LOW to mute the NBFM demodulator
    SI4735WBFMTune = true;
    ssbLoaded = false;
    loopBands = false;  // deactivate band limits
    STEP = DEFAULT_WBFM_STEP;
    FREQ = FMSTARTFREQ * 10000;
    FREQ_OLD = FREQ;             // avoid initial  si4735.frequencyUp()
    si4735.setup(RESET_PIN, 0);  // WBFM Mode
    delay(10);
    si4735.setFM(6400, 10800, FMSTARTFREQ, DEFAULT_WBFM_STEP / 10000);
    si4735.setFmStereoOff();

#ifdef NBFM_DEMODULATOR_PRESENT
    if (afcEnable) {
      afcEnable = false;
     // diable AFC in WBFM
    }
#endif
    displayFREQ(FREQ);

#ifdef TINYSA_PRESENT
    setLO();  // initialize VFO so that spectrum analyzer is tuned
#endif

    return;
  }

  if (modType == LSB || modType == USB || modType == SYNC || modType == CW) {

#ifdef NBFM_DEMODULATOR_PRESENT
    if (afcEnable) {
      afcEnable = false;
      // diable AFC in SSB modes
    }
#endif
    si4735.setSsbAgcOverrite(1, AGCIDX);                                // disable AGC to eliminateSSB humming noise
    si4735.setAMSoftMuteSnrThreshold(preferences.getChar("SMute", 0));  //needs to be reloaded
    si4735.setAvcAmMaxGain(preferences.getChar("AVC", 0));              //needs to be reloaded
  }


#ifdef NBFM_DEMODULATOR_PRESENT
  if (modType != NBFM) {
    si4735.setVolume(vol);  // NBFM demodulator requires SI4732 volume 0
  }
#endif


#ifndef NBFM_DEMODULATOR_PRESENT
  si4735.setVolume(vol);  // volume was zero, rise again
#endif
}

//##########################################################################################################################//

void tuneWBFMSI4735() {  // WBFM tunes SI4735 directly
  if (FREQ > FREQ_OLD)
    si4735.frequencyUp();
  if (FREQ < FREQ_OLD)
    si4735.frequencyDown();

  FREQ = 10000 * si4735.getCurrentFrequency(); // update FREQ
}


//##########################################################################################################################//
void printModulation() {


  if (altStyle)
    tft.fillRoundRect(11, 95, 55, 22, 10, TFT_BLUE);

  else 
    tft.pushImage(11, 92, 55, 26, (uint16_t *)Oval55);

  tft.setCursor(20, 98);
  tft.setTextColor(TFT_GREEN);
  switch (modType) {

    case WBFM:
      tft.print("WFM");
      break;
    case AM:
      tft.print("AM");
      break;
    case NBFM:
      tft.print("NFM");
      break;
    case LSB:
      tft.print("LSB");
      break;
    case USB:
      tft.print("USB");
      break;
    case SYNC:
      tft.print("SYN");
      break;
    case CW:
      tft.print("CW");
      break;
  }
  tft.setTextColor(textColor);
}

//###############################################################################################//

void use1MhzEncoderStep() {
  // Toggle encoder step size between 1 MHz and STEPSIZE
  if (digitalRead(ENCODER_BUTTON) == LOW) {
    use1MHzSteps = !use1MHzSteps;   // flip the state

    if (use1MHzSteps) {
      OLDSTEP = STEP;
      STEP = 1000000l;
    } else {
      STEP = OLDSTEP;
    }

    displaySTEP(false);

    while (digitalRead(ENCODER_BUTTON) == LOW) { }
    delay(100);
  }
}

