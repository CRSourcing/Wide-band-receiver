// wav recorder and player. experimental!


// Settings
#define SAMPLE_RATE 10000
#define SAMPLE_INTERVAL (1000000 / SAMPLE_RATE)
#define RECORD_TIME 9999  // seconds
#define ADC_PIN 36        // Analog sample pin
#define BUFFER_SIZE 2048  // buffer for writing onto SD

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
int16_t readAudioSample();
//##########################################################################################################################//


void mountSDCard() {
  // Initialize SD card
  int ctr = 0;
  tft.endWrite();
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TFT_CS, LOW);  // use GPIO33 for CS SDcard

  while (!SD.begin(SD_CS, SPI, 2000000, "/sd", 10, false)) {
    ctr++;
    displayText(300, 300, 179, 15, "Mounting SDcard");

    Serial_print("Mounting SD card...\n");
    if (ctr == 10) {
      Serial_print(" Mount failed!\n");
      displayText(300, 300, 179, 16, "Mount failed");
      return;
    }
    delay(100);
  }
  Serial_println("\nSD Card Mounted");
  displayText(300, 300, 179, 16, "SD card mounted");
  uint64_t cardSize = SD.cardSize() / (1048567);
  Serial_printf("\nSD Card Size: %lluMB\n", cardSize);
  delay(300);
  Serial_println("SD Card initialized.");
  displayText(300, 300, 179, 16, "initialized");
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
  tft.print("Move encoder to stop record");
  tft.setCursor(8, 170);
  tft.print("While recording, only the ");
  tft.setCursor(8, 190);
  tft.print("squelch can be adjusted.");
  tft.setCursor(8, 220);
  tft.print("Creates consecutive .wav");
  tft.setCursor(8, 240);
  tft.print("files on SDCard.");


  const uint16_t beepSamples = SAMPLE_RATE * 0.1;  // confirmation beep
  uint16_t beepPeriod = SAMPLE_RATE / 1000;
  char tempVol = si4735.getVolume();
  bool circle = false;
  bool mutestat = audioMuted;
  bool beepOnce = false;
  si4735.setHardwareAudioMute(false);
  si4735.setVolume(60);  // this will eventually depend on the amplification of the FFT transistor

  mountSDCard();

  // Find next available file name
  int fileIndex = 1;
  String fileName;
  do {
    fileName = "/rec" + String(fileIndex) + ".wav";
    fileIndex++;
  } while (SD.exists(fileName.c_str()));

  // Create WAV file
  File file = SD.open(fileName.c_str(), FILE_WRITE);
  if (!file) {
    Serial_println("Failed to create file!");
    displayText(300, 300, 179, 15, "Failed to create file");
    return;
  }

  displayText(220, 300, 259, 15, "Recording:");
  tft.print(fileName);

  // Build the WAV header

  WavHeader_8bit header;
  createWavHeader8bit(&header, RECORD_TIME * SAMPLE_RATE);


  file.write((uint8_t*)&header, sizeof(header));

  uint32_t samplesToRecord = RECORD_TIME * SAMPLE_RATE;

  //int16_t buffer[BUFFER_SIZE];
  int8_t buffer[BUFFER_SIZE];
  uint32_t samplesRecorded = 0;
  
  int16_t offsetComp = 2048 - dcOffset;

  while (samplesRecorded < samplesToRecord) {


    si4735.getCurrentReceivedSignalQuality(0);
    signalStrength = si4735.getCurrentRSSI();
    readSquelchPot(0);

    if (signalStrength > currentSquelch) {
      si4735.setAudioMute(false);
      beepOnce = false;

      if (!circle) {
        tft.fillCircle(5, 310, 5, TFT_RED);  // indicator
        circle = true;
      }

      uint32_t nextSampleTime = micros() + SAMPLE_INTERVAL;

      
      for (int b = 0; b < BUFFER_SIZE && samplesRecorded < samplesToRecord; b++) {
        
        buffer[b] = (analogRead(ADC_PIN) + offsetComp) >> 4;  // 8 bit recording, need to compensate the FFT transistorcollector voltage offset from midpoint (2048)

        while ((int32_t)(micros() - nextSampleTime) < 0) {  // delay needed to match SAMPLE_INTERVAL
           __asm__ volatile ("nop");
        }

        samplesRecorded++;
        nextSampleTime += SAMPLE_INTERVAL;
      }

      file.write((uint8_t*)buffer, BUFFER_SIZE * sizeof(int8_t));
    }

    else {  // short "roger beep"

      delay(100);
      uint8_t sst = constrain((signalStrength / 3), 0, 60);
      beepPeriod = (SAMPLE_RATE / 2000) + sst / 2;  // change frequency depending on signalstrength

      si4735.getCurrentReceivedSignalQuality(0);
      signalStrength = si4735.getCurrentRSSI();
      readSquelchPot(0);


      if (!beepOnce && signalStrength < currentSquelch) {
        beepOnce = true;
        si4735.setAudioMute(true);
        for (uint16_t i = 0; i < beepSamples; i++) {
          int8_t beepSample = (i % beepPeriod < beepPeriod / 2) ? 127 : -128;  // Square wave
          file.write((uint8_t*)&beepSample, sizeof(int8_t));
          samplesRecorded++;
          if (samplesRecorded >= samplesToRecord)
            break;
        }
      }


      if (circle) {
        tft.fillCircle(5, 310, 5, TFT_BLACK);
        circle = false;
      }
    }


    if (clw || cclw) {  // encoder moved, finish recording
      samplesRecorded = samplesToRecord;
      clw = 0;
      cclw = 0;
      file.flush();
      file.close();
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
  }
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


// simple .wav player, has a pulsating sound. For testing only.

#define PLAYER_BUFFER_SIZE 2000
uint8_t audio_buffer[PLAYER_BUFFER_SIZE];
size_t buffer_pos = PLAYER_BUFFER_SIZE;
size_t bytes_available = 0;
const int PWM_RESOLUTION = 8;  // 8-bit  wav
const int PWM_CHANNEL = 0;
bool playing = false;

File wav_file;
//##########################################################################################################################//

void setupAudioPWM() {  // use PWM instead of DAC, seems faster
  // Configure PWM timer
  ledc_timer_config_t timer_cfg = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT,  // 8-bit  WAV format
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 20000,  // PWM freq
    .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&timer_cfg);

  // Configure PWM channel
  ledc_channel_config_t ch_cfg = {
    .gpio_num = 26,  // same as DAC2
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,  // Start at 0% duty
    .hpoint = 0
  };
  ledc_channel_config(&ch_cfg);
}

//##########################################################################################################################//

//##########################################################################################################################//

void playWavFile() {  // plays the .wav. Has periodic drouputs. For testing only


  setupAudioPWM();

  wav_file = SD.open("/rec1.wav");
  if (!wav_file) {
    displayText(300, 300, 179, 15, "Failed to open .wav");
    Serial_print("Failed to open rec1.wav");
    return;
  }


  // Read WAV header
  WavHeader_8bit header;
  wav_file.read((uint8_t*)&header, sizeof(header));

  // Verify format
  if (header.audioFormat != 1 || header.numChannels != 1 || header.bitsPerSample != 8) {
    Serial_println("Unsupported WAV format");
    wav_file.close();
    return;
  }
  si4735.setAudioMute(true);
  Serial_printf("Playing rec1.wav (%ldHz, %d-bit)\n", header.sampleRate, header.bitsPerSample);

  playing = true;
  uint32_t sampleInterval = 1000000 / header.sampleRate;
  uint32_t nextSampleTime = micros();




while (playing) {
  // Output sample
  uint8_t sample = audio_buffer[buffer_pos++];
   ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, sample);  // use PWM instead of DAC
   ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

  // Wait for next sample time
  nextSampleTime += sampleInterval;
  while (micros() < nextSampleTime) {  }

  // Refill buffer if needed
  if (buffer_pos >= bytes_available) {
    bytes_available = wav_file.read(audio_buffer, PLAYER_BUFFER_SIZE);
    buffer_pos = 0;
    if (bytes_available == 0 || clw || cclw) {
      clw = false;
      cclw = false;
      break;
    }
  }
}

  wav_file.close();
  digitalWrite(SD_CS, INPUT_PULLUP);
  Serial_println("Playback finished");
  si4735.setAudioMute(false);
}


//##########################################################################################################################//

void wavPlayer() {  

  mountSDCard();
  playWavFile();
}