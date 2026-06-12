/*
 * 05-RTD-PT100-3Wire for the ADS1220 24-bit ADC Library
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * Measures a 3-wire PT100 RTD using the ADS1220's two matched IDAC current
 * sources and a ratiometric external reference. This is the classic precision
 * temperature application the ADS1220 was designed for: the same excitation
 * current that biases the RTD also develops the reference voltage across a
 * precision reference resistor, so the measurement is independent of the
 * absolute IDAC value, and the two IDACs cancel the resistance of the lead
 * wires.
 *
 * Reference design (TI ADS1220 datasheet, 3-wire RTD):
 *   - IDAC1 drives AIN1 (RTD force lead), IDAC2 drives AIN2 (compensation lead)
 *   - The RTD differential voltage is measured across AIN1 (P) and AIN2 (N)
 *   - Both IDAC currents return through R_REF, which sets the external VREF
 *     on the REFP0/REFN0 pins (ratiometric)
 *
 * Ratiometric resistance (both IDACs flow through R_REF):
 *   R_rtd = (code * 2 * R_REF) / (2^23 * gain)
 *
 * Adjust R_REF and the IDAC current to match your hardware.
 *
 * Hardware Connections:
 * | ADS1220 Pin | Function          | Arduino Uno | ESP32 (VSPI) |
 * |-------------|:-----------------:|:-----------:|:------------:|
 * | DRDY        | Data Ready        | D2          | D2           |
 * | MISO        | SPI Data Out      | D12         | D19          |
 * | MOSI        | SPI Data In       | D11         | D23          |
 * | SCLK        | SPI Clock         | D13         | D18          |
 * | CS          | Chip Select       | D7          | D7           |
 * | AIN1/AIN2   | RTD + compensation| PT100 leads | PT100 leads  |
 * | REFP0/REFN0 | External VREF     | across R_REF| across R_REF |
 *
 * License: MIT License
 */

#include "Protocentral_ADS1220.h"
#include <math.h>

#define ADS1220_CS_PIN    7
#define ADS1220_DRDY_PIN  2

// Precision reference resistor in ohms (set to match your board)
#define R_REF      1620.0f
// PGA gain used below (keep these two in sync)
#define PGA_GAIN   8.0f
// PT100 nominal resistance at 0 C
#define RTD_R0     100.0f

Protocentral_ADS1220 ads1220(ADS1220_CS_PIN, ADS1220_DRDY_PIN);

// Callendar-Van Dusen coefficients for a standard PT100 (T >= 0 C)
static const float CVD_A = 3.9083e-3f;
static const float CVD_B = -5.775e-7f;

float rtdResistanceToCelsius(float r) {
    // Solve R = R0 (1 + A*T + B*T^2) for T (valid for T >= 0 C)
    float disc = CVD_A * CVD_A - 4.0f * CVD_B * (1.0f - r / RTD_R0);
    return (-CVD_A + sqrtf(disc)) / (2.0f * CVD_B);
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("ADS1220 05-RTD-PT100-3Wire"));
    Serial.println(F("=========================="));

    if (!ads1220.begin()) {
        Serial.println(F("ADS1220 not detected. Check wiring and power."));
        while (1) { delay(1000); }
    }

    // Measure the RTD differentially across AIN1 (P) and AIN2 (N).
    ads1220.setInputMux(ADS1220_MUX_AIN1_AIN2);
    ads1220.setGain(ADS1220_GAIN_8);
    ads1220.enablePGA(true);

    // Ratiometric: use the external reference developed across R_REF.
    ads1220.setReference(ADS1220_REF_EXTERNAL_REFP0);

    // Excitation: 500 uA out of each IDAC, IDAC1 -> AIN1, IDAC2 -> AIN2.
    ads1220.setIDACCurrent(ADS1220_IDAC_500UA);
    ads1220.setIDAC1Route(ADS1220_IDAC1_AIN1);
    ads1220.setIDAC2Route(ADS1220_IDAC2_AIN2);

    ads1220.setDataRate(ADS1220_DR_20SPS);
    ads1220.setFIRFilter(ADS1220_FIR_50_60);
    ads1220.setConversionMode(false);   // single-shot

    delay(10);   // allow the IDACs and RTD to settle
    Serial.println(F("Resistance (ohm)\tTemperature (C)"));
}

void loop() {
    int32_t code = ads1220.readRaw();

    // Ratiometric resistance with both IDAC currents through R_REF.
    float rRtd = ((float)code * 2.0f * R_REF) / (8388608.0f * PGA_GAIN);
    float tempC = rtdResistanceToCelsius(rRtd);

    Serial.print(rRtd, 3);
    Serial.print(F("\t\t\t"));
    Serial.println(tempC, 3);

    delay(500);
}
