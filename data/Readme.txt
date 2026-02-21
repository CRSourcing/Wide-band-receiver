Please upload the files in this folder to the ESP32 LittleFS. Either use "WIFI Sync", or or use arduino ide with the LittleFS Uploader (https://github.com/earlephilhower/arduino-littlefs-upload).

Edit the CSV files as you please (for example frequencies and station names), but do not change the format.

"memory.csv" is a PicoRX compatible stationlist and can be edited with a csv editor. Ony frequency and mode will be read, the other parameters are PicoRx specific. It will be shown under the "Load List" button.

"MemoInfo.csv" is a receiver specific format and the list will be shown under the "Load Memo" button. MemoInfo.csv can be copied from the receiver to SD card, then edited and uploaded again.
