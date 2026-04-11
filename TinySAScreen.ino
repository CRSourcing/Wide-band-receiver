#ifdef TINYSA_PRESENT

void tinySAScreen() {

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  if (timeSet) {
    timeOld = 0;
    tft.fillRect(340, 6, 116, 16, TFT_BLACK);  // overwrite last time
  }

  redrawMainScreen = true;
  drawTSAButtons();
  readTSAButtons();
}

//##########################################################################################################################//

void drawTSAButtons() {

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_ORANGE);

  struct Button {
    int x;
    int y;
    const char* label;
  };

  const Button buttons[] PROGMEM = {
    { 17, 132, "TinySA" }, { 20, 153, "Mode" }, { 102, 132, "Set" }, { 100, 153, "Param." }, { 185, 132, "Sync" }, { 182, 153, "Markers" }, { 275, 132, "Un" }, { 268, 151, "Sync" }, { 20, 188, "Swp" }, { 20, 210, "0-30" }, { 100, 188, "Swp" }, { 100, 210, "0-200" }, { 185, 188, "Swp" }, { 178, 210, "118-128" }, { 265, 190, "Swp" }, { 270, 210, "FM" }, { 265, 245, "  " }, { 263, 268, "  " }, { 183, 255, " Reset " }, { 183, 268, "  " }, { 100, 255, "Listen" }, { 100, 268, "  " }
  };


  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_YELLOW);


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }


  for (int i = 6; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }



  drawButton(8, 236, 75, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 255);
  etft.print(F("BACK"));
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readTSAButtons() {


  tRel();

  int buttonID = getButtonID();

  if (row < 2 || row > 4 || column > 4)
    return;  // outside of area

  switch (buttonID) {

    case 21:

      tinySACenterMode = !tinySACenterMode;
      tft.setCursor(10, 75);
      if (tinySACenterMode) {
        tft.print(F("Center Mode"));
        tft.setCursor(10, 95);
        tft.print(F("Freq stays in center"));
        Serial.println("color 7 0x00ffff");  // white
        delay(50);
      } else {
        tft.printf("Window mode");
        tft.setCursor(10, 95);
        tft.print(F("Freq moves through window"));
        Serial.println("color 7 0x00ff04");  // yellow
        delay(50);
      }
      delay(1000);
      break;
    case 22:
      tinySAConfigScreen();
      tRel();
      break;
    case 23:
      if (!useNixieDial)
        tft.fillRect(330, 6, 135, 17, TFT_BLACK);  // overwrite last microvolt indication
      tft.fillRect(230, 294, 250, 26, TFT_BLACK);  // overwrite area for sync buttons
      syncEnabled = !syncEnabled;
      if (!syncEnabled)
        TSAdBmValue = 0;
      tft.setCursor(10, 75);
      tft.printf("Sync Markers: %s\n", syncEnabled ? "ON" : "OFF");
      delay(1000);
      preferences.putBool("tsasync", syncEnabled);
      if (syncEnabled) {
        resetTSA();
        sprite2Init = false;  // no room for rolling RSSI history
        spr2.deleteSprite();  //free ram
      }

      tRel();
      break;

    case 24:
      centerTSA = false;
      syncEnabled = false;
      tft.setCursor(10, 67);
      tft.print(F("TSA unsynced"));
      tft.setCursor(10, 87);
      tft.print(F("for this session"));
      tft.setCursor(10, 107);
      delay(1000);
      break;
    case 31:
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep center 15M");
      delay(100);
      Serial.println("sweep span 30M");
      break;
    case 32:
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep center 100M");
      delay(100);
      Serial.println("sweep span 200M");
      break;
    case 33:
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep start 118M");
      delay(100);
      Serial.println("sweep stop 128M");
      break;
    case 34:
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep center 98M");
      delay(100);
      Serial.println("sweep span 20M");
      break;
    case 41:
      return;
    case 42:
      listenToTinySA();
      break;
    case 43:
      resetTSA();
      break;
    case 44:
      break;
    default:
      redrawMainScreen = true;
      tx = ty = pressed = 0;
      return;
  }

  redrawMainScreen = true;
  tRel();
}

#endif

//##########################################################################################################################//
