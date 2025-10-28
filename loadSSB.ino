

//=======================================================================================
void loadSSB() {



  if (!ssbLoaded) {

    if (!slowScan) {  //  do not display message bar in slowScan
      tft.fillRect(0, 294, 222, 25, TFT_BLACK);
      tft.setTextColor(TFT_RED);
      tft.setCursor(130, 300);
      tft.print("loadSSB");
      tft.setTextColor(textColor);
    }


    Serial_println("Loading SSB patch");
    delay(25);
    si4735.reset();
    si4735.queryLibraryId();
    si4735.patchPowerUp();
    delay(50);
    si4735.downloadPatch(ssb_patch_content, size_content);
    si4735.setSSBConfig(bandWidth, 0, 0, 1, 0, 1);
    delay(25);
    si4735.setSSBAudioBandwidth(bandWidth);
    if (bandWidth <= 2) {
      si4735.setSBBSidebandCutoffFilter(0);
    } else {
      si4735.setSBBSidebandCutoffFilter(1);
    }
    ssbLoaded = true;
    if (!slowScan)
      tft.fillRect(130, 294, 92, 25, TFT_BLACK);
    /*
setSSBConfig()
AUDIOBW	SSB Audio bandwidth; 0 = 1.2kHz (default); 1=2.2kHz; 2=3kHz; 3=4kHz; 4=500Hz; 5=1kHz.
SBCUTFLT	SSB side band cutoff filter for band passand low pass filter if 0, the band pass filter to cutoff both the unwanted side band and high frequency component > 2kHz of the wanted side band (default).
AVC_DIVIDER	set 0 for SSB mode; set 3 for SYNC mode.
AVCEN	SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
SMUTESEL	SSB Soft-mute Based on RSSI or SNR.
DSP_AFCDIS	DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.

*/
  
  }
}

//##########################################################################################################################//

