/*
 * Arduino library for the ADS1220 24-bit ADC breakout board from ProtoCentral
 *
 * SPDX-License-Identifier: MIT
 *
 * Author: Ashwin Whitchurch, Protocentral Electronics
 * Contact: support@protocentral.com
 * Copyright (c) 2018-2026 ProtoCentral Electronics
 *
 * This software is licensed under the MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * For information on how to use, visit:
 * https://github.com/Protocentral/Protocentral_ADS1220
 */

#ifndef PROTOCENTRAL_ADS1220_H
#define PROTOCENTRAL_ADS1220_H

#include <Arduino.h>
#include <SPI.h>

// Library version
#define ADS1220_LIBRARY_VERSION   "2.0.0"

// SPI configuration
#define ADS1220_SPI_DUMMY_BYTE    0xFF
#define ADS1220_SPI_CLOCK_SPEED   2000000   // 2 MHz (ADS1220 supports up to ~10 MHz)

// ADS1220 SPI commands
#define ADS1220_CMD_RESET         0x06    // Reset the device
#define ADS1220_CMD_START         0x08    // Start/sync a conversion
#define ADS1220_CMD_POWERDOWN     0x02    // Enter power-down mode
#define ADS1220_CMD_RDATA         0x10    // Read data by command
#define ADS1220_CMD_RREG          0x20    // Read register(s)  (ORed with addr<<2 | n-1)
#define ADS1220_CMD_WREG          0x40    // Write register(s) (ORed with addr<<2 | n-1)

// Configuration register addresses
#define ADS1220_REG_CONFIG0       0x00
#define ADS1220_REG_CONFIG1       0x01
#define ADS1220_REG_CONFIG2       0x02
#define ADS1220_REG_CONFIG3       0x03

// Register field masks
#define ADS1220_CONFIG0_MUX_MASK    0xF0
#define ADS1220_CONFIG0_GAIN_MASK   0x0E
#define ADS1220_CONFIG0_PGA_BYPASS  0x01
#define ADS1220_CONFIG1_DR_MASK     0xE0
#define ADS1220_CONFIG1_MODE_MASK   0x18
#define ADS1220_CONFIG1_CM_BIT      0x04   // Conversion mode (1 = continuous)
#define ADS1220_CONFIG1_TS_BIT      0x02   // Temperature sensor mode
#define ADS1220_CONFIG1_BCS_BIT     0x01   // Burn-out current sources
#define ADS1220_CONFIG2_VREF_MASK   0xC0
#define ADS1220_CONFIG2_FIR_MASK    0x30
#define ADS1220_CONFIG2_PSW_BIT     0x08   // Low-side power switch
#define ADS1220_CONFIG2_IDAC_MASK   0x07
#define ADS1220_CONFIG3_I1MUX_MASK  0xE0
#define ADS1220_CONFIG3_I2MUX_MASK  0x1C
#define ADS1220_CONFIG3_DRDYM_BIT   0x02   // DRDY on DOUT/DRDY

// Default pin assignments (override via constructor)
static constexpr uint8_t ADS1220_DEFAULT_CS_PIN   = 7;   ///< Default chip-select pin
static constexpr uint8_t ADS1220_DEFAULT_DRDY_PIN = 2;   ///< Default data-ready pin

// Full-scale code of the 24-bit signed output (2^23)
static constexpr int32_t ADS1220_FULL_SCALE = 8388608L;

/**
 * @brief Output data rate. Listed rates are for Normal mode; Duty-cycle mode
 *        runs at 1/4 of these and Turbo mode runs at 2x (up to 2000 SPS).
 */
enum ADS1220_DataRate : uint8_t {
    ADS1220_DR_20SPS   = 0x00,
    ADS1220_DR_45SPS   = 0x20,
    ADS1220_DR_90SPS   = 0x40,
    ADS1220_DR_175SPS  = 0x60,
    ADS1220_DR_330SPS  = 0x80,
    ADS1220_DR_600SPS  = 0xA0,
    ADS1220_DR_1000SPS = 0xC0
};

/** @brief ADC operating mode (CONFIG1 MODE[1:0]). */
enum ADS1220_OpMode : uint8_t {
    ADS1220_MODE_NORMAL     = 0x00,   ///< Normal mode (256 kHz modulator clock)
    ADS1220_MODE_DUTYCYCLE  = 0x08,   ///< Duty-cycle mode (lowest power)
    ADS1220_MODE_TURBO      = 0x10    ///< Turbo mode (512 kHz, rates doubled)
};

/** @brief PGA gain setting (CONFIG0 GAIN[2:0]). */
enum ADS1220_Gain : uint8_t {
    ADS1220_GAIN_1   = 0x00,
    ADS1220_GAIN_2   = 0x02,
    ADS1220_GAIN_4   = 0x04,
    ADS1220_GAIN_8   = 0x06,
    ADS1220_GAIN_16  = 0x08,
    ADS1220_GAIN_32  = 0x0A,
    ADS1220_GAIN_64  = 0x0C,
    ADS1220_GAIN_128 = 0x0E
};

/** @brief Input multiplexer configuration (CONFIG0 MUX[3:0]). */
enum ADS1220_Mux : uint8_t {
    // Differential pairs (AINP, AINN)
    ADS1220_MUX_AIN0_AIN1 = 0x00,   ///< Default
    ADS1220_MUX_AIN0_AIN2 = 0x10,
    ADS1220_MUX_AIN0_AIN3 = 0x20,
    ADS1220_MUX_AIN1_AIN2 = 0x30,
    ADS1220_MUX_AIN1_AIN3 = 0x40,
    ADS1220_MUX_AIN2_AIN3 = 0x50,
    ADS1220_MUX_AIN1_AIN0 = 0x60,
    ADS1220_MUX_AIN3_AIN2 = 0x70,
    // Single-ended (AINx referenced to AVSS)
    ADS1220_MUX_AIN0_AVSS = 0x80,
    ADS1220_MUX_AIN1_AVSS = 0x90,
    ADS1220_MUX_AIN2_AVSS = 0xA0,
    ADS1220_MUX_AIN3_AVSS = 0xB0,
    // Diagnostics
    ADS1220_MUX_VREF_DIV4 = 0xC0,   ///< (VREFP - VREFN) / 4
    ADS1220_MUX_AVDD_DIV4 = 0xD0,   ///< (AVDD - AVSS) / 4
    ADS1220_MUX_SHORTED   = 0xE0    ///< Both inputs shorted to (AVDD+AVSS)/2 (offset)
};

/** @brief Voltage reference source (CONFIG2 VREF[1:0]). */
enum ADS1220_Reference : uint8_t {
    ADS1220_REF_INTERNAL_2V048   = 0x00,   ///< Internal 2.048 V reference
    ADS1220_REF_EXTERNAL_REFP0   = 0x40,   ///< External REFP0/REFN0 pins
    ADS1220_REF_EXTERNAL_AIN0    = 0x80,   ///< External AIN0/AIN3 (REFP1/REFN1)
    ADS1220_REF_ANALOG_SUPPLY    = 0xC0    ///< Analog supply (AVDD - AVSS)
};

/** @brief 50/60 Hz FIR notch filter (CONFIG2 FIR[1:0], 20 & 5 SPS only). */
enum ADS1220_FIRFilter : uint8_t {
    ADS1220_FIR_OFF    = 0x00,
    ADS1220_FIR_50_60  = 0x10,   ///< Simultaneous 50 & 60 Hz rejection
    ADS1220_FIR_50     = 0x20,   ///< 50 Hz rejection
    ADS1220_FIR_60     = 0x30    ///< 60 Hz rejection
};

/** @brief IDAC excitation current magnitude (CONFIG2 IDAC[2:0]). */
enum ADS1220_IDACCurrent : uint8_t {
    ADS1220_IDAC_OFF    = 0x00,
    ADS1220_IDAC_10UA   = 0x01,
    ADS1220_IDAC_50UA   = 0x02,
    ADS1220_IDAC_100UA  = 0x03,
    ADS1220_IDAC_250UA  = 0x04,
    ADS1220_IDAC_500UA  = 0x05,
    ADS1220_IDAC_1000UA = 0x06,
    ADS1220_IDAC_1500UA = 0x07
};

/** @brief IDAC1 routing destination (CONFIG3 I1MUX[2:0]). */
enum ADS1220_IDAC1Route : uint8_t {
    ADS1220_IDAC1_DISABLED = 0x00,
    ADS1220_IDAC1_AIN0     = 0x20,
    ADS1220_IDAC1_AIN1     = 0x40,
    ADS1220_IDAC1_AIN2     = 0x60,
    ADS1220_IDAC1_AIN3     = 0x80,
    ADS1220_IDAC1_REFP0    = 0xA0,
    ADS1220_IDAC1_REFN0    = 0xC0
};

/** @brief IDAC2 routing destination (CONFIG3 I2MUX[2:0]). */
enum ADS1220_IDAC2Route : uint8_t {
    ADS1220_IDAC2_DISABLED = 0x00,
    ADS1220_IDAC2_AIN0     = 0x04,
    ADS1220_IDAC2_AIN1     = 0x08,
    ADS1220_IDAC2_AIN2     = 0x0C,
    ADS1220_IDAC2_AIN3     = 0x10,
    ADS1220_IDAC2_REFP0    = 0x14,
    ADS1220_IDAC2_REFN0    = 0x18
};

/**
 * @brief Convenience configuration structure applied by begin(config).
 */
struct ADS1220_Config {
    ADS1220_DataRate  dataRate;       ///< Output data rate
    ADS1220_OpMode    opMode;         ///< Normal / duty-cycle / turbo
    ADS1220_Gain      gain;           ///< PGA gain
    ADS1220_Mux       mux;            ///< Input multiplexer
    ADS1220_Reference reference;      ///< Voltage reference source
    ADS1220_FIRFilter firFilter;      ///< 50/60 Hz filter
    bool              pgaBypass;      ///< true = bypass/disable PGA
    bool              continuousMode; ///< true = continuous, false = single-shot
};

/**
 * @brief Arduino driver for the Texas Instruments ADS1220 24-bit delta-sigma ADC.
 *
 * The ADS1220 is a low-noise, 24-bit, 4-channel (2 differential / 4 single-ended)
 * delta-sigma ADC with an integrated PGA (1-128), internal 2.048 V reference,
 * internal temperature sensor, two matched IDAC current sources for sensor
 * excitation (RTD/thermistor), a 50/60 Hz FIR filter and a low-side power switch.
 *
 * Features of this library:
 * - Typed enum / struct configuration API
 * - Blocking and non-blocking (async) reads
 * - Automatic raw-counts to voltage conversion using the active reference & gain
 * - Internal temperature-sensor readout in degrees Celsius
 * - Per-instance SPI bus selection and ESP32 custom-pin initialisation
 *
 * @author ProtoCentral Electronics
 * @version 2.0.0
 */
class Protocentral_ADS1220 {
public:
    // ---- Constructors -----------------------------------------------------

    /** @brief Construct using the default CS (7) and DRDY (2) pins on the default SPI bus. */
    Protocentral_ADS1220();

    /**
     * @brief Construct with custom control pins on the default SPI bus.
     * @param csPin   Chip-select pin
     * @param drdyPin Data-ready pin
     */
    Protocentral_ADS1220(uint8_t csPin, uint8_t drdyPin);

    /**
     * @brief Construct with custom control pins and a specific SPI bus instance.
     * @param csPin   Chip-select pin
     * @param drdyPin Data-ready pin
     * @param spi     Pointer to the SPI bus (e.g. &SPI, &SPI1)
     */
    Protocentral_ADS1220(uint8_t csPin, uint8_t drdyPin, SPIClass* spi);

    // ---- Initialisation ---------------------------------------------------

    /**
     * @brief Initialise the device with the library default configuration.
     * @param startSPI Call SPI.begin() internally (default true)
     * @return true on success
     */
    bool begin(bool startSPI = true);

    /**
     * @brief Initialise the device and apply a configuration structure.
     * @param config   Desired configuration
     * @param startSPI Call SPI.begin() internally (default true)
     * @return true on success
     */
    bool begin(const ADS1220_Config& config, bool startSPI = true);

    /**
     * @brief Initialise on an ESP32 with explicit SPI pins.
     * @param sck  SPI clock pin
     * @param miso SPI MISO pin
     * @param mosi SPI MOSI pin
     * @return true on success
     */
    bool begin(uint8_t sck, uint8_t miso, uint8_t mosi);

    /** @brief Release resources and end the SPI session. */
    void end();

    // ---- Configuration ----------------------------------------------------

    void setDataRate(ADS1220_DataRate rate);
    void setOperatingMode(ADS1220_OpMode mode);
    void setGain(ADS1220_Gain gain);
    void enablePGA(bool enable);
    void setInputMux(ADS1220_Mux mux);
    void setReference(ADS1220_Reference ref);

    /**
     * @brief Tell the library the actual external reference voltage so that
     *        voltage conversions are correct when not using the internal reference.
     * @param volts External reference voltage in volts
     */
    void setReferenceVoltage(float volts);

    void setFIRFilter(ADS1220_FIRFilter filter);
    void setConversionMode(bool continuous);
    void enableTemperatureSensor(bool enable);
    void enableBurnoutSources(bool enable);
    void setIDACCurrent(ADS1220_IDACCurrent current);
    void setIDAC1Route(ADS1220_IDAC1Route route);
    void setIDAC2Route(ADS1220_IDAC2Route route);
    void enableLowSideSwitch(bool enable);
    void setDRDYOnDOUT(bool enable);

    // ---- Acquisition ------------------------------------------------------

    /** @brief Issue START/SYNC to begin (or restart) conversions. */
    void startConversion();

    /** @brief Put the device into power-down (also stops continuous conversions). */
    void stopConversion();

    /** @brief True while the DRDY pin indicates new data is available. */
    bool isDataReady() const;

    /**
     * @brief Blocking read of raw signed 24-bit counts (sign-extended to 32-bit).
     *
     * In single-shot mode this issues START and waits for DRDY. In continuous
     * mode it waits for the next DRDY edge.
     * @return Signed conversion result, or 0 on timeout
     */
    int32_t readRaw();

    /**
     * @brief Blocking read converted to volts using the active reference & gain.
     * @return Measured voltage in volts
     */
    float readVoltage();

    /**
     * @brief Non-blocking raw read; returns false if no new data yet.
     * @param result Receives the raw counts when data is ready
     */
    bool readRawAsync(int32_t& result);

    /**
     * @brief Non-blocking voltage read; returns false if no new data yet.
     * @param result Receives the voltage when data is ready
     */
    bool readVoltageAsync(float& result);

    /**
     * @brief Single-shot read of one single-ended channel (AINx vs AVSS).
     * @param channel 0-3
     * @return Signed raw counts
     */
    int32_t readSingleEnded(uint8_t channel);

    /**
     * @brief Read the internal temperature sensor in degrees Celsius.
     *
     * Temporarily enables temperature-sensor mode, takes a reading and restores
     * the previous mode.
     * @return Temperature in degrees Celsius
     */
    float readTemperature();

    // ---- Utility ----------------------------------------------------------

    /** @brief Reset the device to its power-on register defaults. */
    void reset();

    /**
     * @brief Basic communication check by writing and reading back a register.
     * @return true if the readback matches
     */
    bool testConnection();

    void writeRegister(uint8_t address, uint8_t value);
    uint8_t readRegister(uint8_t address);

    /** @brief Copy the four configuration registers into out[4]. */
    void getConfigRegisters(uint8_t out[4]);

    /** @brief Pretty-print the four configuration registers. */
    void printRegisters(Stream& stream = Serial);

    /** @brief Convert raw counts to volts using the active reference & gain. */
    float countsToVoltage(int32_t counts) const;

    /** @brief Convert volts to raw counts using the active reference & gain. */
    int32_t voltageToCounts(float voltage) const;

    /** @brief Library version string. */
    static const char* getLibraryVersion();

private:
    uint8_t  _csPin;
    uint8_t  _drdyPin;
    SPIClass* _spi;
    SPISettings _spiSettings;

    uint8_t  _configReg[4];   ///< Shadow copy of CONFIG0..3
    float    _vref;           ///< Active reference voltage (V)
    float    _gain;           ///< Active PGA gain as a number
    bool     _continuousMode;
    bool     _initialized;

    void spiCommand(uint8_t command);
    bool waitForDRDY(uint32_t timeout_ms);
    int32_t readConversion();          ///< Low-level 3-byte read + sign extend
    void applyConfig(const ADS1220_Config& config);
    static float gainToFloat(ADS1220_Gain gain);
    static float referenceToVoltage(ADS1220_Reference ref);
};

#endif // PROTOCENTRAL_ADS1220_H
