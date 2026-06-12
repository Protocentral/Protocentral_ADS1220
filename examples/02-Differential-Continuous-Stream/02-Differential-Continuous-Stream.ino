/*
 * 02-Differential-Continuous-Stream for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * This example streams the differential voltage between AIN0 (AINP) and
 * AIN1 (AINN) in continuous-conversion mode. A DRDY interrupt signals when a
 * new sample is ready and the data is read with a non-blocking async call.
 *
 * Features demonstrated:
 * - Continuous conversion mode
 * - DRDY hardware interrupt for sample-accurate timing
 * - Non-blocking readVoltageAsync()
 *
 * Hardware Connections:
 * | ADS1220 Pin | Function       | Arduino Uno | ESP32 (VSPI) |
 * |-------------|:--------------:|:-----------:|:------------:|
 * | DRDY        | Data Ready     | D2 (INT)    | D2           |
 * | MISO        | SPI Data Out   | D12         | D19          |
 * | MOSI        | SPI Data In    | D11         | D23          |
 * | SCLK        | SPI Clock      | D13         | D18          |
 * | CS          | Chip Select    | D7          | D7           |
 * | AIN0        | Positive Input | Signal +    | Signal +     |
 * | AIN1        | Negative Input | Signal -    | Signal -     |
 *
 * License: MIT License
 */

#include "Protocentral_ADS1220.h"

#define ADS1220_CS_PIN    7
#define ADS1220_DRDY_PIN  2

Protocentral_ADS1220 ads1220(ADS1220_CS_PIN, ADS1220_DRDY_PIN);

volatile bool drdyFlag = false;

void drdyISR() {
    drdyFlag = true;
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("ADS1220 02-Differential-Continuous-Stream"));
    Serial.println(F("========================================="));

    if (!ads1220.begin()) {
        Serial.println(F("ADS1220 not detected. Check wiring and power."));
        while (1) { delay(1000); }
    }

    ads1220.setInputMux(ADS1220_MUX_AIN0_AIN1);   // differential AIN0 - AIN1
    ads1220.setGain(ADS1220_GAIN_1);
    ads1220.setDataRate(ADS1220_DR_600SPS);
    ads1220.setReference(ADS1220_REF_INTERNAL_2V048);
    ads1220.setConversionMode(true);              // continuous

    attachInterrupt(digitalPinToInterrupt(ADS1220_DRDY_PIN), drdyISR, FALLING);
    ads1220.startConversion();

    Serial.println(F("Streaming AIN0-AIN1 (mV)..."));
}

void loop() {
    if (drdyFlag) {
        drdyFlag = false;

        float voltage;
        if (ads1220.readVoltageAsync(voltage)) {
            Serial.println(voltage * 1000.0f, 4);
        }
    }
}
