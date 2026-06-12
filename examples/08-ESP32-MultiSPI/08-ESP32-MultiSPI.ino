/*
 * 08-ESP32-MultiSPI for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * Demonstrates initializing the ADS1220 on an ESP32 using a specific SPI bus
 * (HSPI/VSPI) and custom SCK/MISO/MOSI pins via begin(sck, miso, mosi). On
 * non-ESP32 boards it falls back to the default SPI bus so the sketch still
 * compiles everywhere in CI.
 *
 * Features demonstrated:
 * - Per-instance SPIClass selection (SPIClass* constructor)
 * - ESP32 custom-pin begin(sck, miso, mosi)
 * - Continuous streaming with a polled DRDY (isDataReady)
 *
 * Hardware Connections (ESP32, VSPI default pins shown):
 * | ADS1220 Pin | Function     | ESP32 Pin |
 * |-------------|:------------:|:---------:|
 * | DRDY        | Data Ready   | D4        |
 * | MISO        | SPI Data Out | D19       |
 * | MOSI        | SPI Data In  | D23       |
 * | SCLK        | SPI Clock    | D18       |
 * | CS          | Chip Select  | D5        |
 *
 * License: MIT License
 */

#include "Protocentral_ADS1220.h"

#if defined(ARDUINO_ARCH_ESP32)
  #define ADS1220_CS_PIN    5
  #define ADS1220_DRDY_PIN  4
  #define ADS1220_SCK_PIN   18
  #define ADS1220_MISO_PIN  19
  #define ADS1220_MOSI_PIN  23
  // Use the VSPI bus explicitly.
  SPIClass vspi(VSPI);
  Protocentral_ADS1220 ads1220(ADS1220_CS_PIN, ADS1220_DRDY_PIN, &vspi);
#else
  #define ADS1220_CS_PIN    7
  #define ADS1220_DRDY_PIN  2
  Protocentral_ADS1220 ads1220(ADS1220_CS_PIN, ADS1220_DRDY_PIN);
#endif

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("ADS1220 08-ESP32-MultiSPI"));
    Serial.println(F("========================="));

    bool ok;
#if defined(ARDUINO_ARCH_ESP32)
    ok = ads1220.begin(ADS1220_SCK_PIN, ADS1220_MISO_PIN, ADS1220_MOSI_PIN);
#else
    ok = ads1220.begin();
#endif

    if (!ok) {
        Serial.println(F("ADS1220 not detected. Check wiring and power."));
        while (1) { delay(1000); }
    }

    ads1220.setInputMux(ADS1220_MUX_AIN0_AIN1);
    ads1220.setGain(ADS1220_GAIN_1);
    ads1220.setDataRate(ADS1220_DR_330SPS);
    ads1220.setReference(ADS1220_REF_INTERNAL_2V048);
    ads1220.setConversionMode(true);   // continuous

    ads1220.startConversion();
    Serial.println(F("Streaming AIN0-AIN1 (mV)..."));
}

void loop() {
    if (ads1220.isDataReady()) {
        int32_t raw;
        if (ads1220.readRawAsync(raw)) {
            Serial.println(ads1220.countsToVoltage(raw) * 1000.0f, 4);
        }
    }
}
