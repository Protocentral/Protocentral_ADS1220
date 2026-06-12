/*
 * 07-Register-Dump-Diagnostics for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * A bring-up / troubleshooting sketch. It verifies SPI communication, applies a
 * configuration using the ADS1220_Config structure, dumps all four
 * configuration registers, and runs the internal diagnostic multiplexer
 * settings (shorted inputs for offset, AVDD/4 and VREF/4 monitors).
 *
 * Features demonstrated:
 * - testConnection() communication check
 * - begin(const ADS1220_Config&) one-shot configuration
 * - printRegisters() / getConfigRegisters() / readRegister()
 * - Internal diagnostic MUX settings
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

ADS1220_Config config = {
    ADS1220_DR_175SPS,            // dataRate
    ADS1220_MODE_NORMAL,          // opMode
    ADS1220_GAIN_2,               // gain
    ADS1220_MUX_AIN0_AIN1,        // mux
    ADS1220_REF_INTERNAL_2V048,   // reference
    ADS1220_FIR_50_60,            // firFilter
    false,                        // pgaBypass (false = PGA enabled)
    false                         // continuousMode (false = single-shot)
};

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("ADS1220 07-Register-Dump-Diagnostics"));
    Serial.println(F("===================================="));

    if (!ads1220.begin(config)) {
        Serial.println(F("ADS1220 init/communication FAILED."));
        Serial.println(F("Check CS/DRDY/SPI wiring and power."));
        while (1) { delay(1000); }
    }
    Serial.println(F("Communication OK."));
    Serial.print(F("Library version: "));
    Serial.println(ads1220.getLibraryVersion());
    Serial.print(F("testConnection(): "));
    Serial.println(ads1220.testConnection() ? F("PASS") : F("FAIL"));
    Serial.println();

    ads1220.printRegisters();
    Serial.println();

    // Offset reading: inputs internally shorted.
    ads1220.setInputMux(ADS1220_MUX_SHORTED);
    Serial.print(F("Offset (shorted inputs), raw counts: "));
    Serial.println(ads1220.readRaw());

    // (AVDD - AVSS) / 4 supply monitor.
    ads1220.setInputMux(ADS1220_MUX_AVDD_DIV4);
    int32_t avddRaw = ads1220.readRaw();
    float avdd = ads1220.countsToVoltage(avddRaw) * 4.0f;
    Serial.print(F("AVDD monitor: "));
    Serial.print(avdd, 4);
    Serial.println(F(" V"));

    // (VREFP - VREFN) / 4 reference monitor.
    ads1220.setInputMux(ADS1220_MUX_VREF_DIV4);
    int32_t vrefRaw = ads1220.readRaw();
    float vref = ads1220.countsToVoltage(vrefRaw) * 4.0f;
    Serial.print(F("VREF monitor: "));
    Serial.print(vref, 4);
    Serial.println(F(" V"));
}

void loop() {
    // One-shot diagnostics; nothing to do in the loop.
    delay(1000);
}
