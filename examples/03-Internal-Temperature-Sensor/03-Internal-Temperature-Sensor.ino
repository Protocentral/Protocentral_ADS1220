/*
 * 03-Internal-Temperature-Sensor for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * This example reads the ADS1220's on-chip temperature sensor and prints the
 * die temperature in degrees Celsius and Fahrenheit. No external connections
 * to the analog inputs are required.
 *
 * Features demonstrated:
 * - Internal temperature-sensor mode
 * - readTemperature() (enables the sensor, reads, and restores the mux/mode)
 *
 * Hardware Connections:
 * | ADS1220 Pin | Function       | Arduino Uno | ESP32 (VSPI) |
 * |-------------|:--------------:|:-----------:|:------------:|
 * | DRDY        | Data Ready     | D2          | D2           |
 * | MISO        | SPI Data Out   | D12         | D19          |
 * | MOSI        | SPI Data In    | D11         | D23          |
 * | SCLK        | SPI Clock      | D13         | D18          |
 * | CS          | Chip Select    | D7          | D7           |
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
    Serial.println(F("ADS1220 03-Internal-Temperature-Sensor"));
    Serial.println(F("======================================"));

    if (!ads1220.begin()) {
        Serial.println(F("ADS1220 not detected. Check wiring and power."));
        while (1) { delay(1000); }
    }

    // 20 SPS with the 50/60 Hz filter gives the quietest temperature reading.
    ads1220.setDataRate(ADS1220_DR_20SPS);
    ads1220.setFIRFilter(ADS1220_FIR_50_60);
    ads1220.setConversionMode(false);   // single-shot
}

void loop() {
    float tempC = ads1220.readTemperature();
    float tempF = tempC * 9.0f / 5.0f + 32.0f;

    Serial.print(F("Die temperature: "));
    Serial.print(tempC, 4);
    Serial.print(F(" C  ("));
    Serial.print(tempF, 3);
    Serial.println(F(" F)"));

    delay(1000);
}
