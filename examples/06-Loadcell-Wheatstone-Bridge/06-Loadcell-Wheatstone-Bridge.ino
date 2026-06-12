/*
 * 06-Loadcell-Wheatstone-Bridge for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * Reads a resistive Wheatstone-bridge sensor (e.g. a strain-gauge load cell)
 * ratiometrically. The bridge excitation voltage is also used as the ADC's
 * external reference, so the reading is immune to drift in the excitation
 * supply. A high PGA gain resolves the small bridge output (typically a few
 * mV/V).
 *
 * Ratiometric output in mV/V:
 *   mV_per_V = (code / 2^23) * (1000 / gain)
 *
 * The example performs a simple tare (zero offset) at start-up and prints both
 * the bridge output (mV/V) and a scaled weight. Set CALIBRATION_FACTOR from a
 * known reference mass for real units.
 *
 * Hardware Connections:
 * | ADS1220 Pin | Function          | Arduino Uno | ESP32 (VSPI) |
 * |-------------|:-----------------:|:-----------:|:------------:|
 * | DRDY        | Data Ready        | D2          | D2           |
 * | MISO        | SPI Data Out      | D12         | D19          |
 * | MOSI        | SPI Data In       | D11         | D23          |
 * | SCLK        | SPI Clock         | D13         | D18          |
 * | CS          | Chip Select       | D7          | D7           |
 * | AIN1/AIN2   | Bridge S+ / S-    | Load cell   | Load cell    |
 * | REFP0/REFN0 | Bridge EXC+/EXC-  | Excitation  | Excitation   |
 *
 * License: MIT License
 */

#include "Protocentral_ADS1220.h"

#define ADS1220_CS_PIN    7
#define ADS1220_DRDY_PIN  2

#define PGA_GAIN            128.0f
// counts per (mV/V); set from a known calibration mass for engineering units
#define CALIBRATION_FACTOR  1.0f

Protocentral_ADS1220 ads1220(ADS1220_CS_PIN, ADS1220_DRDY_PIN);

float zeroOffset_mVperV = 0.0f;

float readBridge_mVperV() {
    int32_t code = ads1220.readRaw();
    return ((float)code / 8388608.0f) * (1000.0f / PGA_GAIN);
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("ADS1220 06-Loadcell-Wheatstone-Bridge"));
    Serial.println(F("====================================="));

    if (!ads1220.begin()) {
        Serial.println(F("ADS1220 not detected. Check wiring and power."));
        while (1) { delay(1000); }
    }

    // Bridge sense lines on AIN1 (P) / AIN2 (N).
    ads1220.setInputMux(ADS1220_MUX_AIN1_AIN2);
    ads1220.setGain(ADS1220_GAIN_128);
    ads1220.enablePGA(true);

    // Ratiometric: the bridge excitation is the external reference.
    ads1220.setReference(ADS1220_REF_EXTERNAL_REFP0);

    ads1220.setDataRate(ADS1220_DR_20SPS);
    ads1220.setFIRFilter(ADS1220_FIR_50_60);
    ads1220.setConversionMode(false);   // single-shot

    // Tare: average a few readings with no load applied.
    Serial.println(F("Taring (keep the cell unloaded)..."));
    delay(500);
    float sum = 0.0f;
    for (uint8_t i = 0; i < 16; i++) {
        sum += readBridge_mVperV();
    }
    zeroOffset_mVperV = sum / 16.0f;

    Serial.print(F("Zero offset: "));
    Serial.print(zeroOffset_mVperV, 5);
    Serial.println(F(" mV/V\n"));
    Serial.println(F("Output (mV/V)\tWeight (scaled)"));
}

void loop() {
    float mVperV = readBridge_mVperV() - zeroOffset_mVperV;
    float weight = mVperV * CALIBRATION_FACTOR;

    Serial.print(mVperV, 5);
    Serial.print(F("\t\t"));
    Serial.println(weight, 3);

    delay(250);
}
