#ifdef TINYSA_PRESENT

void tinySAScreen() { 

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


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

  Button buttons[] = {
    { 17, 132, "TinySA" }, { 20, 153, "Mode" }, { 102, 132, "TinySA" }, { 100, 153, "Config" }, { 185, 132, "TinySA" }, { 185, 153, "Sync" }, 
    { 265, 132, " " }, { 265, 151, " " }, { 20, 188, "Sweep" }, { 20, 210, "0-30" }, { 100, 188, "Sweep" }, { 100, 210, "0-200" }, 
    { 185, 188, "Sweep" }, { 178, 210, "118-128" }, { 265, 190, "Swp" }, { 270, 210, "FM" }, 
    { 265, 245, "  " }, { 263, 268, "  " }, { 183, 245, "  " }, { 183, 268, "  " }, { 100, 255, "Listen" }, { 100, 268, "  " }
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
  etft.print("BACK");
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
      tft.setCursor(10, 85);
      if (tinySACenterMode) {
        tft.print("Center Mode");
        Serial.println("color 6 0x000000");
        delay(50);
        Serial.println("color 7 0x0009ff");
      } else {
        tft.printf("Window mode");
        Serial.println("color 7 0x00ff04");
        delay(500);
      }
      break; 
    case 22:
     tinySAConfigScreen();
      tRel(); 
    break;
    case 23:

     if (! useNixieDial)
      tft.fillRect(330, 8, 135, 15, TFT_BLACK);  // overwrite last microvolt indication
      tft.fillRect(232, 294, 247, 25, TFT_BLACK);
      tft.fillCircle(470, 20, 2, TFT_BLACK); // overwrite communication indicator
      syncEnabled = !syncEnabled;
      tft.setCursor(10, 90);
      tft.printf("Sync with TinySA %s\n", syncEnabled ? "Enabled" : "Disabled");
      delay(500);
      preferences.putBool("useTSADBm", syncEnabled);
      if (syncEnabled)
         resetTSA();
      tRel(); 
    break;
    
    case 24: 
  
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


