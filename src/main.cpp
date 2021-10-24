#include <Homie.h>

#include <AirGradient.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "Ticker.h"

AirGradient ag = AirGradient();

SSD1306Wire display(0x3c, SDA, SCL);

HomieNode tempNode("temperature", "temperature", "sensor");
HomieNode humidityNode("humidity", "humidity", "sensor");
HomieNode pmNode("pm", "pm", "sensor");
HomieNode co2Node("co2", "co2", "sensor");

Ticker mTicker;

void showPM();
void showCO2();
void showTempHum();

const float TEMP_OFFSET = -4.0;

// DISPLAY
void showTextRectangle(String ln1, String ln2, boolean small)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  if (small)
  {
    display.setFont(ArialMT_Plain_16);
  }
  else
  {
    display.setFont(ArialMT_Plain_24);
  }
  display.drawString(32, 16, ln1);
  display.drawString(32, 36, ln2);
  display.display();
}

void showPM()
{
  int PM2 = ag.getPM2_Raw();
  showTextRectangle("PM2", String(PM2), false);
  pmNode.setProperty("pm").send(String(PM2));
  pmNode.setProperty("non_raw").send(String(ag.getPM2()));

  mTicker.once_ms_scheduled(3000, showCO2);
}

void showCO2()
{
  int CO2 = ag.getCO2_Raw();
  showTextRectangle("CO2", String(CO2), false);
  co2Node.setProperty("ppm").send(String(CO2));

  mTicker.once_ms_scheduled(3000, showTempHum);
}

void showTempHum()
{
  TMP_RH result = ag.periodicFetchData();
  float temp = result.t + TEMP_OFFSET;

  showTextRectangle(String(temp), String(result.rh) + "%", false);

  tempNode.setProperty("t").send(String(temp));
  humidityNode.setProperty("rh").send(String(result.rh));

  mTicker.once_ms_scheduled(3000, showPM);
}

void loopHandler()
{
}

void setupHandler()
{
}

void setup()
{
  Serial.begin(9600);

  display.init();
  display.flipScreenVertically();

  Homie_setFirmware("airgradient-homie", "1.0.0");
  Homie_setBrand("Airgradient");

  tempNode.advertise("t");
  humidityNode.advertise("rh");
  pmNode.advertise("pm");
  co2Node.advertise("ppm");

  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  Homie.setup();

  showTextRectangle("CO2_Init", String(ESP.getChipId(), HEX), true);
  ag.CO2_Init();

  showTextRectangle("TMP_RH_Init", String(ESP.getChipId(), HEX), true);
  ag.TMP_RH_Init(0x44);

  showTextRectangle("PMS_Init", String(ESP.getChipId(), HEX), true);
  ag.PMS_Init();

  showTextRectangle("Done", String(ESP.getChipId(), HEX), true);
  showPM();
}

void loop()
{
  Homie.loop();
}