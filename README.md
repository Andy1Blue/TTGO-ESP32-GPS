# TTGO-ESP32-GPS

## Devices

* TTGO ESP32
* GY-NEO6MV2 (GPS Module)
## How to install (Arduino IDE)

* Add external library: `https://dl.espressif.com/dl/package_esp32_index.json`
* Install `esp32 by Espressif Systems` from Boards Manager
* Set Board: `TTGO LoRa32-OLED-V1`
* Install `TFT_eSPI` from Library Manager
* Install external library from http://arduiniana.org/libraries/tinygpsplus/ (ZIP file)
* Edit file `../Arduino/libraries/TFT_eSPI/User_Setup_Select.h` and uncomment

```c
#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
```

* Choose properly port
* Now you can open example project from File->Examples->TFT_eSPI->160x128
