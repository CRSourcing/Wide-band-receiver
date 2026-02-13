//##########################################################################################################################//
// Internet tool collection
void connectWIFI() {

  int ctr = 0;
  tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);
  WiFi.begin(ssid, password);
  tft.setCursor(5, 20);
  tft.print("Connecting to WIFI\n");
  while (WiFi.status() != WL_CONNECTED) {
    ctr++;
    if (ctr == 20)
      ESP.restart();
    tft.print(".");
    delay(500);
  }
  tft.println("connected.");
}

//##########################################################################################################################//


void downloadFile(bool fill) {  // downloads a jpg or png file

  if (fill)
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

  if (!LittleFS.begin()) {
    tft.printf("\n\nLittleFS Mount Failed");
    LittleFS.format();
    ESP.restart();
  } else
    tft.printf("\n\nLittleFS mounted\n\nDownloading...");

  
File file;  

if (downloadSelector < 6) {
    file = LittleFS.open("/image.img", FILE_WRITE);
} else {
    file = LittleFS.open("/sked-b25.lst", FILE_WRITE); // EiBi station list must not end with .csv (no automatic load)

}

if (!file) {
    tft.print("Failed to open file for writing\n");
    return;
}

  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();  // Do not use HTTPS certificate

  switch (downloadSelector) {  // use the predefined URL's
    case 0:
      break;
    case 1:
      http.begin(client, host1);
      break;
    case 2:
      http.begin(client, host2);
      break;
    case 3:
      http.begin(client, host3);
      break;
    case 4:
      http.begin(client, host4);
      break;
    case 5:
      http.begin(client, host5);
      break;
      case 6:
      http.begin(client, eibi);
      break;  
  }

  int16_t httpCode = http.GET();

  Serial.printf("HTTP code %d\n", httpCode);
  if (httpCode == HTTP_CODE_OK) {
    http.writeToStream(&file);
    tft.println("\nDownload successful");
  } else {
    tft.println("Download failed");
  }

  file.close();
  http.end();  // Close connection
  delay(1000);
}


//##########################################################################################################################//

void displayPNG() {

  int rc = png.open("/image.img", myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rc == PNG_SUCCESS) {
    rc = png.decode(NULL, 0);
    png.close();
  } else {
    tft.printf("Error opening PNG file");
  }
}


//##########################################################################################################################//

void *myOpen(const char *filename, int32_t *size) {
  myfile = LittleFS.open(filename);
  if (!myfile) return NULL;
  *size = myfile.size();
  return &myfile;
}

//##########################################################################################################################//
void myClose(void *handle) {
  if (myfile) myfile.close();
}

//##########################################################################################################################//
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}

//##########################################################################################################################//
int32_t mySeek(PNGFILE *handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}

//##########################################################################################################################//
int PNGDraw(PNGDRAW *pDraw) {
  uint16_t usPixels[2500];
  const uint32_t leftShift = -110, downShift = 70;
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushRect(leftShift + xShift, pDraw->y - downShift + yShift, pDraw->iWidth, 1, usPixels);
  return 1;
}

//##########################################################################################################################//

void drawIBtns() {

  tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite row 3 from mainscreen
  draw12Buttons(TFT_BLUE, TFT_NAVY);         // draw new buttons
  struct Button {
    int x;
    int y;
    const char *label;
  };

  Button buttons[] = {
    { 20, 133, "EiBi" },
    { 20, 153, "List" },
    { 100, 190, "SW" },
    { 100, 210, "Fade" },
    { 190, 200, "Earth" },
    { 265, 248, "NOAA" },
    { 273, 200, "Sun" },
    { 270, 268, "Alerts" },
    { 190, 248, "SW" },
    { 190, 268, "Cond." },
    { 100, 248, "Sat" },
    { 100, 268, "Image" },
    { 20, 248, "Fore" },
    { 20, 268, "cast" }
  };

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_GREEN);

  etft.setTTFFont(Arial_14);

  for (int i = 0; i < 14; i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }
}

//##########################################################################################################################//

void readIBtns() {

  tDoublePress();

  preferences.putBool("fB", true);  // Does not go back to main menu, uses fast restart instead. Image displays eats up too much memory and does not reliably free it
                                          //, so better reboot after leaving
  int buttonID = getButtonID();

  if (buttonID >= 31)
    pressSound = false;  // avoid squeaking sound while rebooting

  switch (buttonID) {
    case 21:
    downloadSelector = 6; // EiBi station list
    connectWIFI();
    downloadFile(true);
    WiFi.disconnect();
    preferences.putBool("fB", true);  // set fastboot
    ESP.restart();
    break;
    case 31:  // propagation forecast
      break;
    case 32:  // SW fade map
      downloadSelector = 1;
      swappedJPEG = false;
      xShift = -40;
      JPEGWrapper(15);
      break;
    case 33:  // globe view
      downloadSelector = 2;
      swappedJPEG = true;
      xShift = +60;
      yShift = -10;
      JPEGWrapper(15);
      break;
    case 34:  // sun image
      downloadSelector = 3;
      swappedJPEG = true;
      yShift = -70;
      JPEGWrapper(15);
      break;
    case 41:  // propagation forecast
      reportSelector = 0;
      reportWrapper(30);
      break;
    case 42:
      downloadSelector = 4;  // Central America sat image
      swappedJPEG = true;
      xShift = -420;
      yShift = -620;
      JPEGWrapper(15);
      break;
    case 43:
      downloadSelector = 5;  // condition map
      xShift = 0;
      yShift = -15;
      PNGWrapper(30);
      break;
    case 44:  // NOAA spaceweathr alerts
      reportSelector = 1;
      reportWrapper(30);
      break;
  }
  tx = ty = pressed = 0;
  return;
  tRel();
}

//##########################################################################################################################//

void displayJpeg() {
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);

  drawJpeg("/image.img", 0, 0);  // All image files are downloaded under filename image.img
}

//##########################################################################################################################//

void drawJpeg(const char *filename, int xpos, int ypos) {
  File jpegFile = LittleFS.open(filename, "r");  // Declare and openfile
  if (!jpegFile) {
    Serial_println("Failed to open jpeg file\n");
    return;
  }
  JpegDec.decodeFsFile(filename);
  renderJPEG(xpos, ypos);
  jpegFile.close();
}
//##########################################################################################################################//


void renderJPEG(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;
  uint32_t min_w = min((uint32_t)mcu_w, max_x % mcu_w);
  uint32_t min_h = min((uint32_t)mcu_h, max_y % mcu_h);
  bool isSwapped = swappedJPEG;

  while (isSwapped ? JpegDec.readSwappedBytes() : JpegDec.read()) {
    pImg = JpegDec.pImage;
    uint16_t mcu_x = JpegDec.MCUx;
    uint16_t mcu_y = JpegDec.MCUy;
    uint16_t mcu_pix_x = mcu_x * mcu_w;
    uint16_t mcu_pix_y = mcu_y * mcu_h;
    uint16_t mcu_w_curr = (mcu_pix_x + mcu_w <= max_x) ? mcu_w : min_w;
    uint16_t mcu_h_curr = (mcu_pix_y + mcu_h <= max_y) ? mcu_h : min_h;
    {
      tft.pushImage(mcu_pix_x + xpos + xShift, mcu_pix_y + ypos + yShift, mcu_w_curr, mcu_h_curr, pImg);
    }
  }
}

//##########################################################################################################################//

void JPEGWrapper(int cycle) {  // JPG wrapper, cycle = reload time in minutes

  while (true) {
    connectWIFI();
    downloadFile(false);
    WiFi.disconnect();
    displayJpeg();
    for (long i = 0; i < cycle * 600; i++) {
      get_Touch();
      if (pressed)
        ESP.restart();
      delay(100);
    }
  }
}
//##########################################################################################################################//

void PNGWrapper(int cycle) {

  while (true) {
    connectWIFI();
    downloadFile(false);
    WiFi.disconnect();
    displayPNG();
    for (long i = 0; i < cycle * 600; i++) {
      get_Touch();
      if (pressed)
        ESP.restart();
      delay(100);
    }
  }
}
//##########################################################################################################################//

void reportWrapper(int cycle) {

  encLockedtoSynth = false;
  static long offset = 0, oldOffset = 0;

  while (true) {
    getReport();
    displayReport(offset);
    for (long i = 0; i < cycle * 600; i++) {
      if (clw)  // change page
        offset += 400;
      if (cclw)
        offset -= 400;
      if (offset < 0)
        offset = 0;

      clw = 0;
      cclw = 0;

      if (oldOffset != offset) {
        displayReport(offset);
        oldOffset = offset;
      }
      get_Touch();
      if (pressed)
        ESP.restart();
      delay(100);
    }
  }
}

//##########################################################################################################################//


void getReport(void) {  //writes JSON or text page to file. Takes looooong....

  connectWIFI();
  tft.println("Preparing report...please be patient");
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();  //do not use https certificate

  if (reportSelector == 0) {  //NOAA forecast uses text file
    client.connect(fr, 443);
    http.begin(client, fr);
  }

  if (reportSelector == 1) {
    client.connect(al, 443);  // NOAA alert uses JSON format
    http.begin(client, al);
  }


  uint16_t httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK)
    tft.println("Connection error");
  if (!LittleFS.begin()) {
    tft.printf("\n\nLittleFS Mount Failed");
    LittleFS.format();
    ESP.restart();
  } else
    tft.printf("\n\nLittleFS mounted\n\nDownloading...");
  File file = LittleFS.open("/fc.txt", "w");
  http.writeToStream(&file);
  http.end();
  WiFi.disconnect();
}


//##########################################################################################################################//

void displayReport(size_t offset) {

  char txtbuf[1500] = { 0 };  // buffer for webpage text extractor

  int bufferSize = 1000;
  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    tft.printf("LittleFS Mount Failed\n");
    return;
  }

  // Open file
  File file = LittleFS.open("/fc.txt", FILE_READ);
  if (!file) {
    tft.printf("Failed to open file:fc.txt");
    return;
  }

  // Check if offset is inside file size
  size_t fileSize = file.size();
  if (offset >= fileSize) {
    tft.printf("End");
    offset = fileSize;
  }

  // goto offset position
  file.seek(offset, SeekSet);

  //read into the buffer
  size_t bytesRead = file.readBytes(txtbuf, bufferSize);
  txtbuf[bytesRead] = '\0';
  file.close();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 0);

  uint16_t ccounter = 0;
  uint16_t cursorX;
  uint8_t cb;

  while (txtbuf[ccounter]) {
    cb = txtbuf[ccounter];
    cursorX = tft.getCursorX();

    if (cursorX > 479) {
      tft.setCursor(0, tft.getCursorY() + 23);  // new line
    }

    if (cb == '{')
      tft.setCursor(0, tft.getCursorY() + 46);  // new JSON


    // Suppress control characters
    if (!(cb == 195 || cb == '{' || cb == '}' || cb == '\\' || cb == '[')) {
      tft.write(cb);
    }

    if (reportSelector == 1) {
      if (cb == '\\') {  // remove \r\n sequence in alerts
        ccounter += 3;
        tft.write(' ');
      }
    }
    ccounter++;
  }
}

//##########################################################################################################################//
