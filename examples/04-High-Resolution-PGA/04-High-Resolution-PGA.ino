/*
 * 04-High-Resolution-PGA for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * This example demonstrates the integrated programmable gain amplifier (PGA).
 * It sweeps the gain from 1x to 128x on a differential AIN0-AIN1 input so you
 * can see how a higher gain resolves very small (microvolt-level) signals at
 * the cost of a smaller full-scale range.
 *
 * Full-scale differential input (internal 2.048 V reference):
 *   Gain 1   -> +/- 2.048 V       Gain 16  -> +/- 128 mV
 *   Gain 2   -> +/- 1.024 V       Gain 32  -> +/- 64 mV
 *   Gain 4   -> +/- 512 mV        Gain 64  -> +/- 32 mV
 *   Gain 8   -> +/- 256 mV        Gain 128 -> +/- 16 mV
 *
 * Features demonstrated:
 * - PGA gain selection across the full 1x..128x range
 * - 20 SPS + 50/60 Hz filter for lowest noise
 * - Microvolt-resolution differential measurement
 *
 * Hardware Connections:
 * | ADS1220 Pin | Function       | Arduino Uno | ESP32 (VSPI) |
 * |-------------|:--------------:|:-----------:|:------------:|
 * | DRDY        | Data Ready     | D2          | D2           |
 * | MISO        | SPI Data Out   | D12         | D19          |
 * | MOSI        | SPI Data In    | D11         | D23          |
 * | SCLK        | SPI Clock      | D13         | D18          |
 * | CS          | Chip Select    | D7          | D7           |
 * | AIN0        | Positive Input | Signal +    | Signal +     |
 * | AIN1        | Negative Input | Signal -    | Signal -     |
 *
 * Tip: short AIN0 to AIN1 to observe the input-referred noise at each gain.
 *
 * License: MIT License
 */

#include "Protocentral_ADS1220.h"

#define ADS1220_CS_PIN    7
#define ADS1220_DRDY_PIN  2

Protocentral_ADS1220 ads1220(ADS1220_CS_PIN, ADS1220_DRDY_PIN);

const ADS1220_Gain gains[] = {
    ADS1220_GAIN_1, ADS1220_GAIN_2, ADS1220_GAIN_4, ADS1220_GAIN_8,
    ADS1220_GAIN_16, ADS1220_GAIN_32, ADS1220_GAIN_64, ADS1220_GAIN_128
};
const uint8_t gainLabels[] = {1, 2, 4, 8, 16, 32, 64, 128};

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("ADS1220 04-High-Resolution-PGA"));
    Serial.println(F("=============================="));

    if (!ads1220.begin()) {
        Serial.println(F("ADS1220 not detected. Check wiring and power."));
        while (1) { delay(1000); }
    }

    ads1220.setInputMux(ADS1220_MUX_AIN0_AIN1);
    ads1220.setReference(ADS1220_REF_INTERNAL_2V048);
    ads1220.setDataRate(ADS1220_DR_20SPS);
    ads1220.setFIRFilter(ADS1220_FIR_50_60);
    ads1220.setConversionMode(false);   // single-shot

    Serial.println(F("Gain\tRaw counts\tVoltage (uV)"));
}

void loop() {
    for (uint8_t i = 0; i < 8; i++) {
        ads1220.setGain(gains[i]);
        delay(5);   // let the PGA settle before the conversion

        int32_t raw = ads1220.readRaw();
        float microvolts = ads1220.countsToVoltage(raw) * 1.0e6f;

        Serial.print(gainLabels[i]);
        Serial.print(F("x\t"));
        Serial.print(raw);
        Serial.print(F("\t\t"));
        Serial.println(microvolts, 2);
    }
    Serial.println();
    delay(1000);
}
