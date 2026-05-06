void ConfigScreen3() {

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


  redrawMainScreen = true;
  drawCf3Btns();
  readCf3Btns();
}

//##########################################################################################################################//

void drawCf3Btns() {


  struct Button {
    const int x;
    const int y;
    const char *label;
  };

  const Button buttons[] PROGMEM = {
    { 20, 132, "Debug" },
    { 20, 153, "Info" },
    { 100, 130, "Waterf." },
    { 100, 151, "Colors" },
    { 185, 132, "Have" },
    { 185, 151, "Fun" },
    { 265, 132, "CAT" },
    { 265, 151, "Cntl." },
    { 20, 188, "" },
    { 20, 210, "" },
    { 100, 188, "" },
    { 100, 210, "" },
    { 185, 190, "" },
    { 188, 210, "" },
    { 262, 190, "" },
    { 262, 210, "" },
    { 100, 245, "" },
    { 100, 265, "" },
    { 185, 245, "" },
    { 185, 265, "" },
    { 270, 245, "" },
    { 270, 265, "" }

  };

  etft.setTextColor(TFT_GREEN);

  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }

  drawButton(8, 236, 75, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 255);
  etft.print(F("BACK"));
  ;
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readCf3Btns() {



  if (!pressed) return;
  int buttonID = getButtonID();

  if (!buttonID)
    return;  // outside of area

  switch (buttonID) {
    case 21:
      displayDebugInfo = !preferences.getBool("dbgI", 0);
      preferences.putBool("dbgI", displayDebugInfo);
      tft.setCursor(10, 75);
      tft.printf("DisplayDebugInfo %s", displayDebugInfo ? "Yes" : "No");
      if (!displayDebugInfo)
        tft.fillRect(0, 296, 130, 24, TFT_BLACK);
      delay(1000);
      break;
    case 22:
      smoothColorGradient = !smoothColorGradient;
      preferences.putBool("smoothWF", smoothColorGradient);
      tft.setCursor(10, 75);
      tft.printf("Smooth waterf. colors: %s", smoothColorGradient ? "Yes" : "No");
      delay(1000);
      break;
    case 23:
      funEnabled = !funEnabled;
      tft.setCursor(10, 75);
      if (funEnabled)
        tft.print(F("Have fun!"));
      else
        tft.print(F("Fun disabled"));
      preferences.putBool("fun", funEnabled);
      delay(500);
      break;
    case 24:
        enableRigCtl = !enableRigCtl;
      preferences.putBool("cat", enableRigCtl);
      if (enableRigCtl) {
        displayCATIntro();
      tRel();
      }
    else {
    tft.setCursor(10,100);
    tft.print ("CAT is now off."); 
    delay(1500);
    }
      break;
    case 31:
      break;
    case 32:
      break;
    case 33:
      break;
    case 34:
      break;
    case 41:
      tRel();
      return;
    case 42:
      break;
    case 43:
      break;
    case 44:
      break;
    default:
      resetMainScreen();
      return;
  }

  redrawMainScreen = true;
  tRel();
}

//##########################################################################################################################//
// CAT ICom IC - R9000 emulator

uint8_t rigAddress = 0x2A;  //IC - R9000 address, will mirror different models

void rigCtl() {


  static uint8_t buffer[32];
  static int idx = 0;


  while (Serial.available()) {
    uint8_t c = Serial.read();
    buffer[idx++] = c;

    if (c == 0xFD) {  // End of CI-V frame
      processFrame(buffer, idx);
      idx = 0;
    }
  }
}
//##########################################################################################################################//
void processFrame(uint8_t *frame, int len) {

  if (frame[0] != 0xFE || frame[1] != 0xFE || (len < 6))  //invalid
    return;

  uint8_t cmd = frame[4];
  rigAddress = frame[2];  // mirror rig address sent by controller

  /*
  tft.fillRect(0, 296, 280, 24, TFT_BLACK);
  tft.setCursor(0, 300);

  for (int i = 0; i < len; i++) {
    if (frame[i] < 0x10) tft.print("0");  // leading zero
    tft.print(frame[i], HEX);
    tft.print(" ");
  }
*/



  if (cmd == 0x03 && len == 6) {
    // Controller reads Freq
    sendFrequency();
  }

  else if (cmd == 0x04) {
    // controller reads mode
    uint8_t reply[] = { 0xFE, 0xFE, 0xE0, rigAddress, 0x04, 0x00, 0xFD };

    if (modType == LSB)
      reply[5] = 0x00;
    else if (modType == USB)
      reply[5] = 0x01;
    else if (modType == AM)
      reply[5] = 0x02;
    else if (modType == CW)
      reply[5] = 0x03;
    else if (modType == NBFM)
      reply[5] = 0x05;
    else if (modType == WBFM)
      reply[5] = 0x06;
    Serial.write(reply, sizeof(reply));
  }


  else if (cmd == 0x05) {
    // Controller sets freq
    FREQ = decodeBCD(&frame[5], len - 6);
    sendOK();

  }



  else if (cmd == 0x06) {  // controller sets mode

    uint8_t mode = frame[5];

    if (mode == 0)
      modType = LSB;
    else if (mode == 1)
      modType = USB;
    else if (mode == 2)
      modType = AM;
    else if (mode == 3)
      modType = CW;
    else if (mode == 5)
      modType = NBFM;

    else if (mode == 6)
      modType = WBFM;

    sendOK();
    loadSi4735parameters();
    printModulation();

  }

  else if (cmd == 0x15) {

    uint8_t subCmd = frame[5];

    if (subCmd == 0x1) {  // squelch
      uint8_t reply[8] = { 0xFE, 0xFE, 0xE0, rigAddress, 0x15, 0x01, 0x00, 0xFD };

      if (!audioMuted) {
        reply[6] = 0x01;  //open
      }
      Serial.write(reply, sizeof(reply));
    }

    if (subCmd == 0x2) {  // signal strength
      sendSignalStrength(signalStrength);
    }

  }

  else if (cmd == 0x11)  // select attenuator level
    sendNG();

  else if (cmd == 0x14) {  // select gain, squelch
    uint8_t subCmd = frame[5];
    uint8_t gainLevel = bcdToUint8(&frame[6], 2);  // 2 bytes for up to 255, but only 32 levels are available

    //if (subCmd == 0x01)
    //  si4735.setVolume(2 * gainLevel);  //Volume does not work well
    
    
    if (subCmd == 0x02) { //gain control, not working well either, agc overwrites it
      gainLimit = 5 * gainLevel;
      SWMinAttn = 5 * gainLevel;
    }
    sendOK();
  }


else
  sendNG();


}

//##########################################################################################################################//

void sendFrequency() {
  uint8_t reply[11] = { 0xFE, 0xFE, 0xE0, rigAddress, 0x03 };  // E0 is the controllers address
  encodeBCD(FREQ, &reply[5]);                                  // writes 5 bytes
  reply[10] = 0xFD;                                            // terminator
  Serial.write(reply, sizeof(reply));

  /*  
for (int i = 0; i < 11; i++) {
  if (reply[i] < 0x10) tft.print("0"); // leading zero
  tft.print(reply[i], HEX);
  tft.print(" ");
}
tft.println();
tft.setTextColor(TFT_GREEN);
*/
}
//##########################################################################################################################//
void encodeBCD(uint32_t freq, uint8_t *out) {
  // 5 bytes, little endian BCD
  for (int i = 0; i < 5; i++) {
    out[i] = ((freq / (uint32_t)pow(10, i * 2)) % 100);
    out[i] = ((out[i] / 10) << 4) | (out[i] % 10);
  }
}
//##########################################################################################################################//
uint32_t decodeBCD(uint8_t *in, int n) {
  uint32_t freq = 0;
  for (int i = 0; i < n; i++) {
    uint8_t b = in[i];
    int val = ((b >> 4) & 0xF) * 10 + (b & 0xF);
    freq += val * pow(10, i * 2);
  }
  return freq;
}

//##########################################################################################################################//

void sendOK() {
  uint8_t reply[] = { 0xFE, 0xFE, 0xE0, rigAddress, 0xFB, 0xFD };
  Serial.write(reply, sizeof(reply));
}

//##########################################################################################################################//

// rejected
void sendNG() {
  uint8_t reply[] = { 0xFE, 0xFE, 0xE0, rigAddress, 0xFA, 0xFD };
  Serial.write(reply, sizeof(reply));
}

//##########################################################################################################################//

void sendSignalStrength(int8_t signalStrength) {

  signalStrength *= 1.5f; // approximation
  if (signalStrength > 99) signalStrength = 99;

  uint8_t bcd = ((signalStrength / 10) << 4) | (signalStrength % 10);
  uint8_t reply[8] = { 0xFE, 0xFE, 0xE0, rigAddress, 0x15, 0x02, bcd, 0xFD };
  Serial.write(reply, sizeof(reply));
}

//##########################################################################################################################//

// bcd to uint8
uint8_t bcdToUint8(uint8_t *bcd, int len) {
  uint16_t value = 0;
  for (int i = 0; i < len; i++) {
    uint8_t byte = bcd[i];
    uint8_t tens = (byte >> 4) & 0x0F;
    uint8_t ones = byte & 0x0F;
    value += (tens * 10 + ones) * pow(100, i);
  }
  return (uint8_t)value;  // clamp to 0–255
}

//##########################################################################################################################//

 void displayCATIntro() {

tft.fillScreen(TFT_BLACK);
tft.setCursor (0,0);
tft.println("Receiver uses ICOM CI-V protocol to respond to USB serial commands.\n");
tft.println("In fldigi or WSJT-X select ICom IC - R9000 as receiver\n");
tft.println("Com parameters: 115200, 8, 1, NO RTS/CTS");
tft.println("Linux: Use stty -F /dev/ttyUSB0 -crtscts to disable CTS/RTS\n");
tft.println("For grig use: grig -m 3023 -r /dev/ttyUSB0 -s 115200 -d 3\n\n");
tft.println("Touch to enable CAT.");
tPress();
rebuildMainScreen(false);

 }
