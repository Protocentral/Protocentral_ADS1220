/*
 * 01-Basic-SingleEnded-Read for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * This example sequentially reads all four single-ended inputs (AIN0..AIN3,
 * each referenced to AVSS) in single-shot mode and prints the voltage on each.
 *
 * Features demonstrated:
 * - Device initialization with default configuration
 * - Single-ended channel readout with readSingleEnded()
 * - Automatic counts-to-voltage conversion (readVoltage / countsToVoltage)
 *
 * Hardware Connections:
 * | ADS1220 Pin | Function       | Arduino Uno | ESP32 (VSPI) |
 * |-------------|:--------------:|:-----------:|:------------:|
 * | DRDY        | Data Ready     | D2          | D2           |
 * | MISO        | SPI Data Out   | D12         | D19          |
 * | MOSI        | SPI Data In    | D11         | D23          |
 * | SCLK        | SPI Clock      | D13         | D18          |
 * | CS          | Chip Select    | D7          | D7           |
 * | DVDD        | Digital VDD    | +5V         | +3.3V        |
 * | DGND        | Digital Ground | GND         | GND          |
 * | AVDD        | Analog VDD     | +5V         | +3.3V        |
 * | AGND        | Analog Ground  | GND         | GND          |
 * | AIN0..AIN3  | Analog Inputs  | Signals     | Signals      |
 *
 * License: MIT License
 */

#include "Protocentral_ADS1220.h"

#define ADS1220_CS_PIN    7
#define ADS1220_DRDY_PIN  2

Protocentral_ADS1220 ads1220(ADS1220_CS_PIN, ADS1220_DRDY_PIN);

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("ADS1220 01-Basic-SingleEnded-Read"));
    Serial.println(F("================================="));

    if (!ads1220.begin()) {
        Serial.println(F("ADS1220 not detected. Check wiring and power."));
        while (1) { delay(1000); }
    }

    Serial.print(F("Library version: "));
    Serial.println(ads1220.getLibraryVersion());

    ads1220.setDataRate(ADS1220_DR_330SPS);
    ads1220.setGain(ADS1220_GAIN_1);
    ads1220.setReference(ADS1220_REF_INTERNAL_2V048);
    ads1220.setConversionMode(false);   // single-shot

    Serial.println(F("Reading AIN0..AIN3 (single-ended, mV)\n"));
}

void loop() {
    for (uint8_t ch = 0; ch < 4; ch++) {
        int32_t raw = ads1220.readSingleEnded(ch);
        float mv = ads1220.countsToVoltage(raw) * 1000.0f;

        Serial.print(F("AIN"));
        Serial.print(ch);
        Serial.print(F(": "));
        Serial.print(mv, 3);
        Serial.print(F(" mV\t"));
    }
    Serial.println();
    delay(500);
}
