
void SDCard() {  // SDCard functions are here

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  tft.setCursor(10, 65);
  tft.print("SDCard: Max 32GB, FAT32.");
  tft.setCursor(10, 85);
  tft.print("Some SDCards cause issues.");

  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);
  redrawMainScreen = true;
  drawSDBtns();
  readSDBtns();
}

//##########################################################################################################################//

void drawSDBtns() {


  struct Button {
    int x;
    int y;
    const char* label;
  };

  Button buttons[] = {
    { 18, 132, "Test" },
    { 15, 153, "SDcard" },
    { 97, 130, "Upload" },
    { 97, 151, "SDcard" },
    { 182, 132, "Format" },
    { 178, 151, "LittleFS" },
    { 265, 132, "Memo" },
    { 265, 151, "to SD" },
    { 20, 188, "Show" },
    { 14, 210, "LittleFS" },
    { 100, 188, "Delete" },
    { 100, 210, "SDfiles" },
    { 183, 190, "Record" },
    { 183, 210, "Audio" },
    /*
    { 265, 190, "  "}, 
    { 270, 210, "  "}, 
    { 265, 245, "  "}, 
    { 263, 268, "  "}, 
    { 183, 245, " " }, 
    { 183, 268, "  "}, 
    { 100, 245, "  "}, 
    { 100, 268, "  "}
  */
  };


  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_SILVERBLUE);


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }
  drawButton(8, 236, 74, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setCursor(20, 254);
  etft.print("BACK");
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readSDBtns() {


  if (!pressed) return;
  int buttonID = getButtonID();

  if (row < 2 || row > 4 || column > 4)
    return;  // outside of area

  switch (buttonID) {

    case 21:
      readSDCard(0);
      rebuildMainScreen(1);
      break;
    case 22:
      copyFilesToLittleFS();
      rebuildMainScreen(1);
      break;
    case 23:
      formatLittleFSWithWarning();
      break;
    case 24:
      exportMemoInfo();
      rebuildMainScreen(1);
      break;
    case 31:
      listLittleFSFiles();
      tDoublePress(),
        rebuildMainScreen(1);
      break;
    case 32:
      deleteRecursive(SD, "/");
      preferences.putBool("fB", true);
      ESP.restart();
      break;
    case 33:
      wavRecord();
      rebuildMainScreen(1);
      break;
    case 34:
      break;
    case 41:
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
// Functionality to work with an external SD card

void readSDCard(bool close) {  // 0 = read and close,  1 = leave open

  tRel();

  int ctr = 0;
 
tft.fillScreen(TFT_BLACK);
tft.setCursor(0, 0);
tft.endWrite();

digitalWrite(TFT_CS, HIGH);
digitalWrite(TOUCH_CS, HIGH);
digitalWrite(SD_CS, LOW);  // use GPIO33 for CS SDcard and set it low

while (!SD.begin(SD_CS, spiSD, 2000000))  {

    ctr++;
    tft.print("Mounting SD card...\n");
    if (ctr == 10) {
      tft.setTextColor(TFT_RED);
      tft.print("-SD Card Mount failed!\n");
      tft.setTextColor(TFT_GREEN);
      delay(1000);
      return;
    }
    delay(100);
  }

  tft.print("\nSD Card Mounted\n");
  uint64_t cardSize = SD.cardSize() / (1048567);
  tft.printf("\nSD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);

  digitalWrite(TFT_CS, LOW);
  digitalWrite(SD_CS, INPUT_PULLUP);


  if (close)
    return;

  tft.print("\nTouch to continue");
  tDoublePress();

  drawSdJpeg("/armap.jpg", 0, 0);  // draw Jpeg for testing

  tDoublePress();

  SD.end();
  preferences.putBool("fB", true);
  ESP.restart();
}
//##########################################################################################################################//



void loadLists() {
  if (!LittleFS.begin(true)) {
    Serial_println("LittleFS Mount Failed");
    return;
  }

  loadAllCSVFiles();  // Load all CSV files


#ifdef PRINT_STORAGE_DEBUG_MESSAGES
  printLoadedData();  // Print loaded data
#endif
}

//##########################################################################################################################//

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {

  File root = fs.open(dirname);
  if (!root) {
    tft.println("\nFailed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    tft.println("\nNot a directory");
    return;
  }

  tft.print("FILES ON SDCard:\n");

  File file = root.openNextFile();
  while (file) {
    tft.printf("%s %u bytes\n", file.name(), file.size());
    file = root.openNextFile();
  }
}


//##########################################################################################################################//
// Draw a Jpeg for SDCard testing

// xpos, ypos is top left corner of plotted image
void drawSdJpeg(const char* filename, int xpos, int ypos) {

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile = SD.open(filename, FILE_READ);  // or, file handle reference for SD library

  if (!jpegFile) {
    Serial_print("ERROR: File \"");
    Serial_print(filename);
    Serial_println("\" not found!");
    return;
  }

  Serial_println("===========================");
  Serial_print("Drawing file: ");
  Serial_println(filename);
  Serial_println("===========================");

  // Use one of the following methods to initialise the decoder:
  bool decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
  //bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

  if (decoded) {

    // render the image onto the screen at given coordinates
    jpegRender(0, 0);
  } else {
    Serial_println("Jpeg file format not supported!");
  }
}


//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################

// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void jpegRender(int xpos, int ypos) {

  //jpegInfo(); // Print information from the JPEG file (could comment this line out)

  uint16_t* pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;



  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // Fetch data from the file, decode and display
  while (JpegDec.read()) {  // While there is more data in the file
    pImg = JpegDec.pImage;  // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w) {
      uint16_t* cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++) {
        p += mcu_w;
        for (int w = 0; w < win_w; w++) {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }


    // draw image MCU block only if it will fit on the screen
    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort();  // Image has run off bottom of screen so abort decoding
  }

  tft.setSwapBytes(swapBytes);
}

//##########################################################################################################################//

void listLittleFSFiles() {
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);

  tft.println("Files in LittleFS:");
  tft.println("----------------");


  while (file) {
    tft.print(file.name());
    tft.print(" - ");
    tft.print(file.size());
    tft.println(" bytes");
    file = root.openNextFile();
  }

  uint64_t totalBytes = LittleFS.totalBytes();
  uint64_t usedBytes = LittleFS.usedBytes();
  uint64_t remainingBytes = totalBytes - usedBytes;

  tft.println("----------------");
  tft.print("Total space: ");
  tft.print(totalBytes);
  tft.println(" bytes");
  tft.print("Used space: ");
  tft.print(usedBytes);
  tft.println(" bytes");
  tft.print("Remaining space: ");
  tft.print(remainingBytes);
  tft.println(" bytes");
}

//##########################################################################################################################//


void copyFilesToLittleFS() {


  pinMode(SD_CS, OUTPUT);  //  prepare for SDcard
  //digitalWrite(TFT_CS, HIGH);    // unselect display
  delay(20);
  digitalWrite(SD_CS, LOW);  // use ENCODER_PIN_B for CS SDcard
  delay(20);
  SD.begin(SD_CS, spiSD, 20000000);  // need to call twice WHY?
  delay(20);


  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);

  if (SD.begin(SD_CS, spiSD, 20000000)) {
    tft.print("\nSD Card Mounted\n");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  tft.printf("\nSD Card Size: %lluMB\n\n", cardSize);

  listDir(SD, "/", 0);


  tft.print("\nTouch to copy files to LittleFS.\n");
  tft.setTextColor(TFT_RED);
  tft.print("All existing files on LittleFS will be deleted!\n");
  tft.setTextColor(TFT_GREEN);
  tDoublePress();



  if (!LittleFS.begin()) {
    tft.println("LittleFS Mount Failed");
    delay(2000);
    LittleFS.format();
    preferences.putBool("fB", true);
    ESP.restart();
  } else
    tft.println("LittleFS mounted");
  LittleFS.format();
  SD.begin(SD_CS, spiSD, 20000000);

  File root = SD.open("/");
  File file = root.openNextFile();

  while (file) {
    if (!file.isDirectory()) {

      tft.fillRect(0, 300, 480, 19, TFT_BLACK);
      tft.setCursor(0, 300);
      tft.printf("Copying: %s", file.name());

      String filePath = "/" + String(file.name());
      File destFile = LittleFS.open(filePath, FILE_WRITE);
      if (!destFile) {
        Serial_printf("Failed to open %s for writing\n", file.name());
        file = root.openNextFile();
        continue;
      }

      while (file.available()) {
        destFile.write(file.read());
      }
      destFile.close();
    }
    file = root.openNextFile();
  }


  listLittleFSFiles();  // Display files
  tft.println("\n Done, touch to continue.");
  tDoublePress();
  SD.end();
  preferences.putBool("fB", true);
  ESP.restart();  // reset pin 16 for encoder
}

//##########################################################################################################################//
//Update structures from the CSV files on LittleFS
std::vector<String> splitCSVLine(const String& line) {
  std::vector<String> tokens;
  int lastIndex = 0, nextIndex;

  while ((nextIndex = line.indexOf(',', lastIndex)) != -1) {
    String token = line.substring(lastIndex, nextIndex);
    token.trim();  // Remove whitespace
    tokens.push_back(token);
    lastIndex = nextIndex + 1;
  }

  // Ensure we don’t add an empty token at the end
  String lastToken = line.substring(lastIndex);
  lastToken.trim();
  if (!lastToken.isEmpty()) {
    tokens.push_back(lastToken);
  }

#ifdef PRINT_STORAGE_DEBUG_MESSAGES
  Serial_printf("DEBUG: Parsed %d tokens from line: %s\n", tokens.size(), line.c_str());
#endif
  return tokens;
}

//##########################################################################################################################//
void loadCSVtoStruct(const char* filename) {
  Serial_print("Loading: ");
  Serial_println(filename);

  String filePath = "/" + String(filename);
  File file = LittleFS.open(filePath, "r");

  if (!file) {
    Serial_print("ERROR: Failed to open ");
    Serial_println(filename);
    return;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();  // Remove whitespace

    // Skip empty lines
    if (line.length() == 0) continue;

    // Parse CSV line
    std::vector<String> tokens = splitCSVLine(line);

    // Identify structure based on filename
    if (strcmp(filename, "BandInfo.csv") == 0) {
      if (tokens.size() == 5) {
#ifdef PRINT_STORAGE_DEBUG_MESSAGES
        Serial_println("INFO: Updating BandInfo structure...");
#endif
        bandList.push_back({
          strdup(tokens[0].c_str()),             // bandName
          tokens[1].toInt(),                     // startFreqKHz
          tokens[2].toInt(),                     // stopFreqKHz
          static_cast<bool>(tokens[3].toInt()),  // isAmateurRadioBand
          tokens[4].toInt()                      // bandNumber
        });
      } else {
#ifdef PRINT_STORAGE_DEBUG_MESSAGES
        Serial_print("WARNING: Skipped malformed line (expected 5 tokens): ");
        Serial_println(line);
#endif
      }
    } else if (strcmp(filename, "MemoInfo.csv") == 0) {
      if (tokens.size() == 5) {

#ifdef PRINT_STORAGE_DEBUG_MESSAGES
        Serial_println("INFO: Updating MemoInfo structure...");
#endif
        memoList.push_back({
          tokens[0].toInt(),          //memoNumber
          strdup(tokens[1].c_str()),  // memoName
          tokens[2].toInt(),          // memoFreq
          tokens[3].toInt(),          // memoModType
          tokens[4].toInt(),          // memoBandwidth
        });

      } else {

#ifdef PRINT_STORAGE_DEBUG_MESSAGES
        Serial_print("Line error, expected 5 tokens");
        Serial_println(line);
#endif
      }
    }

    else if (strcmp(filename, "SlowScanFrequencies.csv") == 0) {
      if (tokens.size() == 3) {
#ifdef PRINT_STORAGE_DEBUG_MESSAGES
        Serial_println("INFO: Updating SlowScanFrequencies structure...");
#endif
        slowScanList.push_back({
          tokens[0].toInt(),         // freq
          tokens[1].toInt(),         // modT
          strdup(tokens[2].c_str())  // desc
        });
      } else {
#ifdef PRINT_STORAGE_DEBUG_MESSAGES
        Serial_print("WARNING: Skipped malformed line (expected 3 tokens): ");
        Serial_println(line);
#endif
      }
    } else {
#ifdef PRINT_STORAGE_DEBUG_MESSAGES
      Serial_print("WARNING: Unknown file ");
      Serial_println(filename);
#endif
    }
  }

  file.close();
}
//##########################################################################################################################//

void loadAllCSVFiles() {
  Serial_println("Scanning LittleFS for CSV files...");
  File root = LittleFS.open("/");
  File file = root.openNextFile();

  while (file) {
    String filename = file.name();
    if (filename.endsWith(".csv")) {
      loadCSVtoStruct(filename.c_str());
    }
    file = root.openNextFile();
  }

  // Load original values if CSV files do not exist
  if (bandList.empty()) {
    Serial_println("INFO: Loading original BandInfo values...");
    bandList = originalBandList;
  }

  if (memoList.empty()) {
    Serial_println("INFO: Loading original MemoInfo values...");
    memoList = originalMemoList;
  }
  if (slowScanList.empty()) {
    Serial_println("INFO: Loading original SlowScanFrequencies values...");
    slowScanList = originalSlowScanList;
  }
}

//##########################################################################################################################//
void printLoadedData() { // debug helper


  Serial_printf("\n--- Band Info (%d entries) ---\n", bandList.size());
  for (const auto& band : bandList) {
    Serial_printf("%s, %ld, %ld, %d, %d\n",
                  band.bandName, band.startFreqKHz, band.stopFreqKHz, band.isAmateurRadioBand, band.bandNumber);
  }


  Serial_printf("\n--- Memo Info (%d entries) ---\n", memoList.size());
  for (const auto& memo : memoList) {
    Serial_printf("%d, %s, %ld, %d, %d\n",
                  memo.memoNumber, memo.memoName, memo.memoFreq, memo.memoModType, memo.memoBandwidth);
  }

  Serial_printf("\n--- SlowScan Frequencies (%d entries) ---\n", slowScanList.size());
  for (const auto& scan : slowScanList) {
    Serial_printf("%ld, %d, %s\n", scan.freq, scan.modT, scan.desc);
  }
}
//##########################################################################################################################//

// Copies MemoInfo.csv to SDCard for editing with CSV editor

void exportMemoInfo() {

  readSDCard(1);

  if (!LittleFS.begin(true)) {
    tft.println("\nAn error has occurred while mounting LittleFS");
  } else {
    tft.println("\nLittleFS mounted");
  }

  copyMemoInfoToSD();

  SD.end();
  pinMode(SD_CS, INPUT_PULLUP);  // free pin for encoder
  tDoublePress();
}
//##########################################################################################################################//

void copyMemoInfoToSD() {


  File srcFile = LittleFS.open("/MemoInfo.csv", FILE_READ);
  if (!srcFile) {
    tft.print("\nFailed to open MemoInfo.csv onLittleFS");
    return;
  }

  SD.remove("/MemoInfo.csv");  // Remove existing file

  File destFile = SD.open("/MemoInfo.csv", FILE_WRITE);
  if (!destFile) {
    tft.print("\nFailed to open dest. file");
    srcFile.close();
    return;
  }

  tft.print("\nCopying MemoInfo.csv to SDCard\n");

  while (srcFile.available()) {
    destFile.write(srcFile.read());
  }

  tft.print("\nDone!\n");

  srcFile.close();
  destFile.close();
}

//##########################################################################################################################//

void overwriteMemoInfoLine(int buttonID) {  // writes new data to LittleFS when a Save Memo action occurs


  buttonID -= 1;  // buttonID starts with 1, set to 0

  const char* filename = "/MemoInfo.csv";
  File file = LittleFS.open(filename, FILE_READ);

  if (!file) {
    Serial_println("Failed to open MemoInfo");
    return;
  }

  // Read all lines and store in a vector
  std::vector<String> lines;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    lines.push_back(line);
  }
  file.close();

  // Calculate the line index
  int lineIndex = buttonID;

  // Update the specific line with new values
  if (lineIndex < lines.size()) {
    MemoInfo& m = memoList[buttonID];


    lines[lineIndex] = String(m.memoNumber) + "," + "--" + "," + String(FREQ) + "," + String(modType) + ","
                       + String(bandWidth);  // insert current FREQ, modType and bandwidth. Overwrite memo name
  }


  else {
    Serial_println("buttonID is out of range");
    return;
  }

  // Write all lines back to the file
  file = LittleFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial_println("overwriteMemoInfoLine:Error open MemoInfo.csv ");
    return;
  }

  for (const String& line : lines) {
    file.println(line);
  }
  file.close();
  Serial_println("MemoInfo.csv updated");
}


//##########################################################################################################################//
// reload memoList from MemoInfo.csv

void reloadMemoList() {
  const char* filename = "/MemoInfo.csv";
  File file = LittleFS.open(filename, FILE_READ);

  if (!file) {
    Serial_println("Failed to open MemoInfo.csv for reading");
    return;
  }

  memoList.clear();  // Clear the existing list

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    std::vector<String> tokens = splitCSVLine(line);

    if (tokens.size() == 5) {
      MemoInfo memo;
      memo.memoNumber = tokens[0].toInt();
      memo.memoName = strdup(tokens[1].c_str());
      memo.memoFreq = tokens[2].toInt();
      memo.memoModType = tokens[3].toInt();
      memo.memoBandwidth = tokens[4].toInt();
      memoList.push_back(memo);

#ifdef PRINT_STORAGE_DEBUG_MESSAGES
      Serial_printf("Added Memo: Number=%d, Name=%s, Freq=%d, ModType=%d, Bandwidth=%d\n",
                    memo.memoNumber, memo.memoName, memo.memoFreq,
                    memo.memoModType, memo.memoBandwidth);
#endif

    } else {
      Serial_printf("reloadMemoList: Skipping token error %s\n", line.c_str());
    }
  }

  file.close();
  Serial_println("MemoInfo.csv loaded");
}

//##########################################################################################################################//

void formatLittleFSWithWarning() {

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_RED);
  tft.print("\n\nLittleFS is the ESP32 file system.\n");
  tft.print("\nPress encoder to format LittleFS.\n");
  tft.print("All files on LittleFS will be deleted!\n");
  tft.print("Touch to leave without formatting");
  pressed = false;
  while (true) {
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      LittleFS.format();
      break;
    }
    pressed = get_Touch();
    if (pressed)
      break;
  }
  LittleFS.end();
  preferences.putBool("fB", true);
  ESP.restart();
}

//##########################################################################################################################//

void createMemoInfoCSVIfNotExist() {
  const char* filename = "/MemoInfo.csv";
  if (LittleFS.exists(filename)) {

#ifdef PRINT_STORAGE_DEBUG_MESSAGES
    Serial_println("createMemoInfoCSVIfNotExist: MemoInfo.csv already found on LittleFS");
#endif
    return;
  }

  File mfile = LittleFS.open("/MemoInfo.csv", FILE_WRITE);
  if (!mfile) {
    Serial_println("Failed to open MemoInfo.csv for writing");
    return;
  }

  for (const auto& memo : memoList) {
    long freq = memo.memoFreq;
    String line = String(memo.memoNumber) + "," + memo.memoName + "," + String(freq) + "," + String(memo.memoModType) + "," + String(memo.memoBandwidth) + "\r\n";
    mfile.print(line);
  }

  mfile.close();
  Serial_println("MemoInfo.csv created");
}



//##########################################################################################################################//

void deleteRecursive(fs::FS& fs, const char* path) {

  mountSDCard();

  File root = fs.open(path);
  if (!root) {
    Serial_println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial_println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String filePath = String(path) + "/" + file.name();
    if (file.isDirectory()) {
      deleteRecursive(fs, filePath.c_str());
      fs.rmdir(filePath.c_str());
      Serial_printf("Removed folder: %s\n", filePath.c_str());
    } else {
      fs.remove(filePath.c_str());
      Serial_printf("Deleted file: %s\n", filePath.c_str());
    }
    file = root.openNextFile();
  }
  root.close();
}



//##########################################################################################################################//

void displayLogsFromBuffer(uint16_t x, uint16_t y) { // displays the Serial_ log buffer
    // Read max 400 char
    String logs = logBuffer.read().substring(0, 400);
    //logBuffer.clear();  // Empty the buffer after reading

    uint16_t currentY = y;
    uint8_t charCount = 0;
    

    tft.setTextSize(1);
 
    tft.fillRect(345, 48, 130, 242, TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setCursor(x, 53);
    tft.print ("Events:");
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(x, currentY);
     
    for (unsigned int i = 0; i < logs.length(); i++) {
        // Handle newlines
        if (logs[i] == '\n') {
            currentY += tft.fontHeight();
            tft.setCursor(x, currentY);
            charCount = 0;
            continue;
        }
        
        // Wrap every 20 characters
        if (charCount >= 20) {
            currentY += tft.fontHeight();
            tft.setCursor(x, currentY);
            charCount = 0;
        }
        
       if(currentY >= 290)
        return; 

        tft.print(logs[i]);
        charCount++;
    }

    tft.setTextColor(textColor);
    tft.setTextSize(2);
}

