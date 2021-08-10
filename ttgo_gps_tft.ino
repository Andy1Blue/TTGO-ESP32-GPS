/*
  Some part of code was used from: https://gist.github.com/janhajk/e54128867e1c695b6151e57f658234bc
*/

#include <TFT_eSPI.h>
#include <SPI.h>
#include <TinyGPS++.h> // http://arduiniana.org/libraries/tinygpsplus/
#include <HardwareSerial.h>
#include "EEPROM.h"
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

#define EEPROM_SIZE 128
#define TFT_GREY 0x5AEB
#define RX_PIN 37
#define TX_PIN 38

struct GpsDataState_t
{
  double originLat = 0;
  double originLon = 0;
  double originAlt = 0;
  double distMax = 0;
  double dist = 0;
  double altMax = -999999;
  double altMin = 999999;
  double spdMax = 0;
  double prevDist = 0;
};
GpsDataState_t gpsState = {};

#define TASK_SERIAL_RATE 1000 // ms
uint32_t nextSerialTaskTs = 0;
uint32_t nextOledTaskTs = 0;

void setup()
{
  tft.init();
  tft.setRotation(1);

  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  while (!EEPROM.begin(EEPROM_SIZE))
  {
    true;
  }

  long readValue;
  EEPROM_readAnything(0, readValue);
  gpsState.originLat = (double)readValue / 1000000;

  EEPROM_readAnything(4, readValue);
  gpsState.originLon = (double)readValue / 1000000;

  EEPROM_readAnything(8, readValue);
  gpsState.originAlt = (double)readValue / 1000000;
}

template <class T>
int EEPROM_writeAnything(int ee, const T &value)
{
  const byte *p = (const byte *)(const void *)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T>
int EEPROM_readAnything(int ee, T &value)
{
  byte *p = (byte *)(void *)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

void loop()
{
  static int p0 = 0;

  gpsState.originLat = gps.location.lat();
  gpsState.originLon = gps.location.lng();
  gpsState.originAlt = gps.altitude.meters();

  long writeValue;
  writeValue = gpsState.originLat * 1000000;
  EEPROM_writeAnything(0, writeValue);
  writeValue = gpsState.originLon * 1000000;
  EEPROM_writeAnything(4, writeValue);
  writeValue = gpsState.originAlt * 1000000;
  EEPROM_writeAnything(8, writeValue);
  EEPROM.commit();

  gpsState.distMax = 0;
  gpsState.altMax = -999999;
  gpsState.spdMax = 0;
  gpsState.altMin = 999999;

  while (SerialGPS.available() > 0)
  {
    gps.encode(SerialGPS.read());
  }

  if (gps.satellites.value() > 4)
  {
    gpsState.dist = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), gpsState.originLat, gpsState.originLon);

    if (gpsState.dist > gpsState.distMax && abs(gpsState.prevDist - gpsState.dist) < 50)
    {
      gpsState.distMax = gpsState.dist;
    }
    gpsState.prevDist = gpsState.dist;

    if (gps.altitude.meters() > gpsState.altMax)
    {
      gpsState.altMax = gps.altitude.meters();
    }

    if (gps.speed.kmph() > gpsState.spdMax)
    {
      gpsState.spdMax = gps.speed.kmph();
    }

    if (gps.altitude.meters() < gpsState.altMin)
    {
      gpsState.altMin = gps.altitude.meters();
    }
  }

  if (nextSerialTaskTs < millis())
  {
    nextSerialTaskTs = millis() + TASK_SERIAL_RATE;

    // Print in console
    Serial.print("YEAR= ");
    Serial.println(gps.date.year());

    Serial.print("LAT=");
    Serial.println(gps.location.lat(), 6);

    Serial.print("LONG=");
    Serial.println(gps.location.lng(), 6);

    Serial.print("ALT=");
    Serial.println(gps.altitude.meters());

    Serial.print("Sats=");
    Serial.println(gps.satellites.value());

    Serial.print("DST: ");
    Serial.println(gpsState.dist, 1);

    // Print in TFT
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(1, 1, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.print("Year: ");
    tft.println(gps.date.year());

    tft.print("Lat: ");
    tft.println(gps.location.lat());

    tft.print("Lng: ");
    tft.println(gps.location.lng());

    tft.print("Alt: ");
    tft.println(gps.altitude.meters());

    tft.print("Sats: ");
    tft.println(gps.satellites.value());

    tft.print("Dst: ");
    tft.println(gpsState.dist);

    tft.print("nextSerialTaskTs: ");
    tft.println(nextSerialTaskTs);
  }
}
