// wav recorder and player. Depends heavily on the quality of the SD card. Uses <SdFat.h> library for faster writing

// Settings
#define SAMPLE_RATE 8000  // 4KHz
#define SAMPLE_INTERVAL (1000000 / SAMPLE_RATE)
#define RECORD_TIME 9999
#define BUFFER_SIZE 4096  // buffer for writing onto SD, bigger = better, less "plops".

// WAV file header structure
typedef struct {
  char chunkID[4];         // "RIFF"
  uint32_t chunkSize;      // File size - 8
  char format[4];          // "WAVE"
  char subchunk1ID[4];     // "fmt "
  uint32_t subchunk1Size;  // 16 for PCM
  uint16_t audioFormat;    // 1 = PCM
  uint16_t numChannels;    // 1 = mono
  uint32_t sampleRate;
  uint32_t byteRate;       // sampleRate * numChannels * bitsPerSample/8
  uint16_t blockAlign;     // numChannels * bitsPerSample/8
  uint16_t bitsPerSample;  // 8
  char subchunk2ID[4];     // "data"
  uint32_t subchunk2Size;  // data size
} WavHeader_8bit;


//##########################################################################################################################//

//Forward decl
void createWavHeader8bit(WavHeader_8bit* header, uint32_t sampleCount);

//##########################################################################################################################//


void mountSDCard() {
  tft.endWrite();
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TFT_CS, LOW);  // GPIO33 for SD CS
  uint16_t ctr = 0;

  while (!sd.begin(SD_CS, SD_SCK_MHZ(25))) {  // <SdFat.h>
    ctr++;
    displayText(300, 300, 179, 15, "Mounting SDcard");

    if (ctr == 10) {
      Serial_print(" Mount failed!\n");
      displayText(300, 300, 179, 16, "Mount failed");
      return;
    }
    delay(100);
  }

  displayText(300, 300, 179, 16, "SD card mounted");
  delay(500);
  displayText(300, 300, 179, 16, " ");
}

//##########################################################################################################################//
void wavRecord() {
  tft.setTextColor(TFT_YELLOW);
  tft.fillRect(5, 65, 333, 225, TFT_BLACK);
  tft.setCursor(8, 70);
  tft.print("WAV recorder records only");
  tft.setCursor(8, 90);
  tft.print("when squelch is open.");
  tft.setCursor(8, 130);
  tft.print("Move encoder to stop rec.");
  tft.setCursor(8, 170);
  tft.print("While recording, only the ");
  tft.setCursor(8, 190);
  tft.print("squelch can be adjusted.");
  tft.setCursor(8, 220);
  tft.print("Creates consecutive .wav");
  tft.setCursor(8, 240);
  tft.print("files on SDCard.");
  tft.setCursor(8, 260);
  tft.print("8 Bit Mono, 8000 sampl/sec");

  char tempVol = si4735.getVolume();
  bool circle = false;
  bool mutestat = audioMuted;

  si4735.setHardwareAudioMute(false);
  si4735.setVolume(60);

  mountSDCard();

  // Find next available file name
  int fileIndex = 1;
  String fileName;
  do {
    fileName = "/rec" + String(fileIndex) + ".wav";
    fileIndex++;
  } while (sd.exists(fileName.c_str()));  // use sd.exists() with SdFat

  f = sd.open(fileName.c_str(), O_WRITE | O_CREAT | O_TRUNC);
  if (!f) {
    Serial_println("Faild to create file!");
    displayText(300, 300, 179, 15, "Failed to create file");
    return;
  }

  displayText(220, 300, 259, 15, "Recording:");
  tft.print(fileName);

  // Build WAV header
  WavHeader_8bit header;
  createWavHeader8bit(&header, RECORD_TIME * SAMPLE_RATE);
  f.write((uint8_t*)&header, sizeof(header));

  uint32_t samplesToRecord = RECORD_TIME * SAMPLE_RATE;
  uint8_t buffer[BUFFER_SIZE];
  uint32_t samplesRecorded = 0;
  int16_t offsetComp = 2048 - gpio36_Offset;
  uint16_t ctr = 0;


  while (!(clw + cclw)) {  //encoder moved

    ctr++;

    if (ctr == 2) {  // indroduces delay before the squelch closes
      ctr = 0;
      si4735.getCurrentReceivedSignalQuality(0);
      signalStrength = si4735.getCurrentRSSI();
      readSquelchPot(0);
      if (signalStrength > currentSquelch)
        si4735.setAudioMute(false);
      else
        si4735.setAudioMute(true);
    }


    if (signalStrength > currentSquelch) {

      if (!circle) {
        tft.fillCircle(5, 310, 5, TFT_RED);
        circle = true;
      }


      uint32_t nextSampleTime = micros() + SAMPLE_INTERVAL;

      for (int b = 0; b < BUFFER_SIZE && samplesRecorded < samplesToRecord; b++) {

        buffer[b] = (analogRead(AUDIO_INPUT_PIN) + offsetComp) >> 4;

        while ((int32_t)(micros() - nextSampleTime) < 0) {
        }

        samplesRecorded++;
        nextSampleTime += SAMPLE_INTERVAL;
      }

      f.write((uint8_t*)buffer, BUFFER_SIZE);
    }

    else {
      tft.fillCircle(5, 310, 5, TFT_BLACK);
      circle = false;
    }
  }

  samplesRecorded = samplesToRecord;
  clw = false;
  cclw = false;

  f.flush();
  f.close();
  si4735.setVolume(tempVol);
  mutestat = audioMuted;
  si4735.setAudioMute(mutestat);
  si4735.setHardwareAudioMute(mutestat);
  displayText(280, 300, 199, 15, "Finished");
  delay(500);
  tft.fillRect(220, 300, 259, 19, TFT_BLACK);
  tft.setTextColor(textColor);
  digitalWrite(TFT_CS, LOW);
  digitalWrite(SD_CS, INPUT_PULLUP);
  return;
}


//##########################################################################################################################//
void createWavHeader8bit(WavHeader_8bit* header, uint32_t sampleCount) {
  // RIFF chunk
  memcpy(header->chunkID, "RIFF", 4);
  header->chunkSize = sampleCount + 36;  // 8-bit samples
  memcpy(header->format, "WAVE", 4);

  // Format subchunk
  memcpy(header->subchunk1ID, "fmt ", 4);
  header->subchunk1Size = 16;
  header->audioFormat = 1;  // PCM
  header->numChannels = 1;  // Mono
  header->sampleRate = SAMPLE_RATE;
  header->byteRate = SAMPLE_RATE;  // sampleRate × channels × (8/8)
  header->blockAlign = 1;          // channels × (8/8)
  header->bitsPerSample = 8;

  // Data subchunk
  memcpy(header->subchunk2ID, "data", 4);
  header->subchunk2Size = sampleCount;  // samples × channels × (8/8)
}
//##########################################################################################################################//


// simple .wav player, has some pops and plops.


//##########################################################################################################################//


#define B_SIZE 2048

void playWavFile() {

  uint8_t bufferA[B_SIZE];
  uint8_t bufferB[B_SIZE];


  String selected = selectFileWithExtension(".wav");

  FsFile f = sd.open(selected.c_str(), O_READ);
  if (!f) {
    tft.print("Failed to open.");
    tft.println(selected);
    delay(1000);
    rebuildMainScreen(false);
    redrawIndicators();
    return;
  }

  WavHeader_8bit header;
  f.read((uint8_t*)&header, sizeof(header));

  if (header.audioFormat != 1 || header.numChannels != 1 || header.bitsPerSample != 8) {
    Serial.println("Unsupported WAV format");
    f.close();
    return;
  }

  si4735.setAudioMute(true);

  uint32_t sampleInterval = 1000000 / header.sampleRate;
  uint32_t nextSampleTime = micros();

  // fill both buffers
  int bytesA = f.read(bufferA, B_SIZE);
  int bytesB = f.read(bufferB, B_SIZE);

  bool useA = true;

  int pos = 0;
  int bytesAvailable = bytesA;

  while (bytesAvailable > 0) {
    // Output sample

    if (clw || cclw)
      break;


    uint8_t sample = useA ? bufferA[pos] : bufferB[pos];

    dac2.outputVoltage((uint8_t)sample);  // set the dac


    pos++;
    nextSampleTime += sampleInterval;

    // pingpong fill the buffers wh
    while (micros() < nextSampleTime) {
      if (useA && bytesB < B_SIZE) {
        int n = f.read(bufferB + bytesB, 64);
        if (n > 0) bytesB += n;
      } else if (!useA && bytesA < B_SIZE) {
        int n = f.read(bufferA + bytesA, 64);
        if (n > 0) bytesA += n;
      }
    }


    if (pos >= bytesAvailable) {
      pos = 0;
      if (useA) {
        bytesAvailable = bytesB;
        bytesB = 0;  // mark buffer B empty, will refill during wait
        tft.fillCircle(10, 310, 5, TFT_RED);

      } else {
        bytesAvailable = bytesA;
        bytesA = 0;  // mark buffer A as empty
        tft.fillCircle(10, 310, 5, TFT_GREEN);
      }
      useA = !useA;
      if (bytesAvailable <= 0) break;  // end of file
    }
  }

  f.close();
  Serial.println("Playback finished");
  si4735.setAudioMute(false);
  rebuildMainScreen(false);
}


//##########################################################################################################################//


void wavPlayer() {

  mountSDCard();
  playWavFile();
}

//##########################################################################################################################//

String selectFileWithExtension(const char* extension) {
  mountSDCard();

  // List all files having const char* extension
  FsFile dir = sd.open("/");
  if (!dir) {
    Serial.println("Failed to open root directory!");
    redrawIndicators();
    return "";
  }

  std::vector<String> files;
  FsFile file;
  while (file.openNext(&dir, O_RDONLY)) {
    if (!file.isDir()) {
      char name[64];
      file.getName(name, sizeof(name));
      String fname = String(name);
      if (fname.endsWith(extension)) {
        files.push_back(fname);
      }
    }
    file.close();
  }

  dir.close();

  if (files.empty()) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setCursor(10, 150);
    tft.printf("No %s files found!\n", extension);
    return "";
  }

  int selected = 0;
  int indx = 0;
  const int maxRows = 12;
  bool once = true;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 0);
  tft.printf("Select %s file and press encoder.", extension);

  while (true) {
    if (once || clw || cclw) {
      for (int i = 0; i < maxRows && (indx + i) < (int)files.size(); i++) {
        int y = 30 + i * 22;
        if ((indx + i) == selected) {
          tft.setTextColor(TFT_YELLOW);
        } else {
          tft.setTextColor(TFT_GREY);
        }
        tft.setCursor(10, y);
        tft.print(files[indx + i]);
      }
      once = false;
    }

    // up / down
    if (cclw) {
      if (selected > 0) {
        selected--;
        if (selected < indx) indx--;
      }
      cclw = 0;
    }
    if (clw) {
      if (selected < (int)files.size() - 1) {
        selected++;
        if (selected >= indx + maxRows) indx++;
      }
      clw = 0;
    }

    if (digitalRead(ENCODER_BUTTON) == LOW) {
      return files[selected];
    }

    delay(10);
  }
}


//##########################################################################################################################//
