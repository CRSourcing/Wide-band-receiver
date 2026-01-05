
// slowly scan through struct SlowScanFrequencies
void SlowStationScan() {

  if (!slowScan)
    return;

  audioMuted = false;
  
  if (! useNixieDial)
  tft.fillRect(330, 4, 145, 20, TFT_BLACK); // overwrite TSA signal strength
  si4735.setAudioMute(false);  // function needs audio signal
  si4735.setHardwareAudioMute(false);
  audioMuted = false;
  modType = AM;
  loadSi4735parameters();
  DrawSmeterScale();
  etft.setTTFFont(Arial_14);
  drawSlowScanButtons();
  currentStationIndex = 0;

  lastSwitchTime = millis();

  while (true) {

    // Show selected stations
    setStation(slowScanList[currentStationIndex].freq);  

    if (!slowScan)  // in case "Leave" was pressed
      break;

    unsigned long currentTime = millis();

    if (!readSlowScanButtons()) {  // function returns 0 when Exit pressed
      return;                      // Exit
    }
    // Switch to the next station after duration. switch immediately if EXIT or NEXT are found in Bandinfo
    if ((currentTime - lastSwitchTime >= (stationDuration * 1000) && !stopScan)) {  // 2 seconds is a good value
      lastSwitchTime = currentTime;                                                 // Reset the timer
      currentStationIndex++;                                                        // Move to the next station
      audioPeakVal = 0;
    }

    audioScan();

    if (nextStation == true) {
      currentStationIndex++;
      nextStation = false;
      stopScan = true;
    }

    if (lastStation == true && currentStationIndex) {
      currentStationIndex--;
      lastStation = false;
      stopScan = true;
    }


    if (currentStationIndex >= numStations) {
      currentStationIndex = 0;  // Loop back to the first station
    }

    if (stopState && SNR)  {
      stopScan = true;
    }

    statusIndicator();

    drawCircles();
  }
}

//##########################################################################################################################//

// Function to set radio to a station
void setStation(long freq) {
  
  static int ctr = 0;
  static char oldModType = -1;
  ctr++;


  if (FREQ_OLD == freq)
    return;

  tft.fillRect(120, 180, 167, 55, TFT_BLACK);  // overwrite station and audio message


  FREQ = freq;
  FREQCheck();        //check whether within FREQ range
  displayFREQ(FREQ);  // display new FREQ
  modType = slowScanList[currentStationIndex].modT; 

  etft.fillRect(5, 100, 330, 20, TFT_BLACK);   // overwrite last station name
  etft.setTextColor(TFT_SKYBLUE);
  etft.setTTFFont(Arial_14);


    if (modType == 2 || modType == 3 || modType == 6 || modType == 7) 
         etft.setTextColor(TFT_RED); // SSB mode
          
  etft.setCursor(10, 100);
  etft.printf("%d: ", currentStationIndex + 1);
  etft.print(slowScanList[currentStationIndex].desc);
       
   if (oldModType != modType) 
     loadSi4735parameters(); 

if ((modType == 2 || modType == 3 || modType == 6 || modType == 7) && (oldModType != modType))
  delay(1000); // Additional delay after loading SSB patch
  oldModType = modType;
  setLO();       // and tune it in
  getRSSIAndSNR();

 calculateAndDisplaySignalStrength(); // display RSSI/SNR
 displaySmeterBar(10); // fast smeter. no delay

  FREQ_OLD = FREQ;
  SNR = 0;
}


//##########################################################################################################################//

void drawSlowScanButtons() {


  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  tft.fillRect(3, 235, 336, 57, TFT_BLACK);   // clear lower row
  tft.fillRect(90, 180, 247, 57, TFT_BLACK);  // clear mid row

  etft.setTextColor(TFT_GREEN);
  etft.setTTFFont(Arial_14);

  etft.setCursor(21, 140);
  etft.print("Pause");

  etft.setCursor(110, 140);
  etft.print("Last");

  etft.setCursor(195, 140);
  etft.print("Next");

  etft.setCursor(275, 140);
  etft.print("Exit");

  etft.setCursor(20, 190);
  etft.print("Signal");

  etft.setCursor(20, 208);
  etft.print("Stop");
}

//##########################################################################################################################//

bool readSlowScanButtons() {


  pressed = get_Touch();

  displayHoldStatus();

  if (pressed || clw || cclw) {
    int buttonID = getButtonID();

  if (cclw){
     buttonID = 22;
     cclw = false;
  }      

  if (clw){
     buttonID = 23;
     clw = false;
  }      



    switch (buttonID) {
      case 21:
        stopScan = !stopScan;
        if (!stopScan) {
          audioPeakDetected = false;
          SNR = 0;
          delay(200);
        }
        tRel();
        return true;

      case 22:
        lastStation = true;
        audioPeakVal = 0;
        audioPeakDetected = false;
        SNR = 0;
        stopScan = true;
        delay(200);
        return true;

      case 23:
        nextStation = true;
        audioPeakVal = 0;
        audioPeakDetected = false;
        SNR = 0;
        stopScan = true;
        delay(200);
        return true;

      case 24:
        nextStation = false;
        lastStation = false;
        stopScan = false;
        slowScan = false;
        currentStationIndex = -1;
        etft.setTextColor(TFT_GREEN);
        for (int i = 0; i < 255; i++)  // reset to 0 to avoid artefacts when starting mini spectrum 16
          Rpeak[i] = 0;
        tx = ty = pressed = 0;
        slowScan = false;
        return false;

      case 31:
        stopState = !stopState;
        etft.setTextColor(TFT_GREEN);
        etft.setTTFFont(Arial_14);
        etft.fillRect(20, 208, 58, 18, TFT_BLACK);
        etft.setCursor(20, 208);
        if (stopState) etft.print("Contin.");
        else etft.print("Stop");
        tRel();
        return true;

      default:
        break;
    }

    buttonID = 0;

  }  // end if pressed

  return true;
}

//##########################################################################################################################//

void displayHoldStatus() {
  static bool oldstatus;


  if (oldstatus == stopScan)
    return;

  etft.setTextColor(TFT_GREEN);
  etft.setTTFFont(Arial_14);

  etft.fillRect(18, 138, 62, 20, TFT_BLACK);

  if (stopScan == false) {
    etft.setCursor(21, 140);
    etft.print("Pause");
  }

  else {
    etft.setTextColor(TFT_RED);
    etft.setCursor(18, 140);
    etft.print("Restart");
    etft.setTextColor(TFT_GREEN);
  }

  oldstatus = stopScan;
}


//##########################################################################################################################//

void statusIndicator() {

getRSSIAndSNR();


  if (SNR) {
    tft.setTextColor(TFT_ORANGE);
    tft.setCursor(130, 185);
    tft.print("Signal found");
    tft.setTextColor(TFT_GREEN);
  }


  if (audioPeakVal > audioTreshold) {
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(130, 210);
    tft.print("Audio found");
    tft.setTextColor(TFT_GREEN);
    audioPeakDetected = true;
  }

  else audioPeakDetected = false;
}

//##########################################################################################################################//**

void drawCircles() {

  const int dist = (480 / numStations);
  const int xPos = 10 + dist * currentStationIndex;
  const int yPos = 306;

  static int lastXpos = 0;


  if (xPos > 477) return;

  tft.fillCircle(lastXpos, yPos, 2, TFT_BLACK);

  if (!audioPeakDetected && !SNR)
    tft.fillCircle(xPos, yPos, 4, TFT_GREY);


  if (audioPeakDetected)
    tft.fillCircle(xPos, yPos, 4, TFT_YELLOW);


  if (SNR)
    tft.fillCircle(xPos, yPos, 4, TFT_ORANGE);

  if (audioPeakDetected && SNR)
    tft.fillCircle(xPos, yPos, 4, TFT_GREEN);

  tft.fillCircle(xPos, yPos, 2, TFT_RED);

  lastXpos = xPos;
}

//##########################################################################################################################//
