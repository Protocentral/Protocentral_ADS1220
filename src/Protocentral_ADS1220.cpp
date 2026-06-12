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

#include "Protocentral_ADS1220.h"

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Protocentral_ADS1220::Protocentral_ADS1220()
    : Protocentral_ADS1220(ADS1220_DEFAULT_CS_PIN, ADS1220_DEFAULT_DRDY_PIN, &SPI) {}

Protocentral_ADS1220::Protocentral_ADS1220(uint8_t csPin, uint8_t drdyPin)
    : Protocentral_ADS1220(csPin, drdyPin, &SPI) {}

Protocentral_ADS1220::Protocentral_ADS1220(uint8_t csPin, uint8_t drdyPin, SPIClass* spi)
    : _csPin(csPin),
      _drdyPin(drdyPin),
      _spi(spi),
      _spiSettings(ADS1220_SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE1),
      _vref(2.048f),
      _gain(1.0f),
      _continuousMode(false),
      _initialized(false) {
    _configReg[0] = 0x00;
    _configReg[1] = 0x04;   // DR=20SPS, Normal, continuous, TS off, BCS off
    _configReg[2] = 0x10;   // Internal Vref, 50/60Hz rejection, PSW open, IDAC off
    _configReg[3] = 0x00;
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

bool Protocentral_ADS1220::begin(bool startSPI) {
    pinMode(_csPin, OUTPUT);
    pinMode(_drdyPin, INPUT);
    digitalWrite(_csPin, HIGH);

    if (startSPI) {
        _spi->begin();
    }

    reset();
    delayMicroseconds(100);

    // After power-up/reset the device pulls DRDY low once it is ready.
    waitForDRDY(10);

    // Default configuration: AIN0/AIN1 differential, gain 1, PGA enabled,
    // 20 SPS, normal mode, continuous off, internal 2.048 V reference.
    _configReg[0] = 0x00;
    _configReg[1] = 0x00;   // single-shot by default for the new API
    _configReg[2] = 0x10;
    _configReg[3] = 0x00;

    writeRegister(ADS1220_REG_CONFIG0, _configReg[0]);
    writeRegister(ADS1220_REG_CONFIG1, _configReg[1]);
    writeRegister(ADS1220_REG_CONFIG2, _configReg[2]);
    writeRegister(ADS1220_REG_CONFIG3, _configReg[3]);

    _vref = 2.048f;
    _gain = 1.0f;
    _continuousMode = false;
    _initialized = true;

    return testConnection();
}

bool Protocentral_ADS1220::begin(const ADS1220_Config& config, bool startSPI) {
    if (!begin(startSPI)) {
        return false;
    }
    applyConfig(config);
    return true;
}

bool Protocentral_ADS1220::begin(uint8_t sck, uint8_t miso, uint8_t mosi) {
    pinMode(_csPin, OUTPUT);
    pinMode(_drdyPin, INPUT);
    digitalWrite(_csPin, HIGH);

#if defined(ARDUINO_ARCH_ESP32)
    _spi->begin(sck, miso, mosi, _csPin);
#else
    (void)sck; (void)miso; (void)mosi;
    _spi->begin();
#endif

    return begin(false);
}

void Protocentral_ADS1220::end() {
    if (_initialized) {
        stopConversion();
        _spi->end();
        _initialized = false;
    }
}

// ---------------------------------------------------------------------------
// Low-level SPI
// ---------------------------------------------------------------------------

void Protocentral_ADS1220::spiCommand(uint8_t command) {
    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);
    _spi->transfer(command);
    delayMicroseconds(1);
    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();
}

void Protocentral_ADS1220::writeRegister(uint8_t address, uint8_t value) {
    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);
    _spi->transfer(ADS1220_CMD_WREG | (address << 2));
    _spi->transfer(value);
    delayMicroseconds(1);
    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();

    if (address <= ADS1220_REG_CONFIG3) {
        _configReg[address] = value;
    }
}

uint8_t Protocentral_ADS1220::readRegister(uint8_t address) {
    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);
    _spi->transfer(ADS1220_CMD_RREG | (address << 2));
    uint8_t value = _spi->transfer(ADS1220_SPI_DUMMY_BYTE);
    delayMicroseconds(1);
    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();

    return value;
}

// ---------------------------------------------------------------------------
// Configuration helpers
// ---------------------------------------------------------------------------

void Protocentral_ADS1220::setDataRate(ADS1220_DataRate rate) {
    uint8_t reg = (_configReg[1] & ~ADS1220_CONFIG1_DR_MASK) | rate;
    writeRegister(ADS1220_REG_CONFIG1, reg);
}

void Protocentral_ADS1220::setOperatingMode(ADS1220_OpMode mode) {
    uint8_t reg = (_configReg[1] & ~ADS1220_CONFIG1_MODE_MASK) | mode;
    writeRegister(ADS1220_REG_CONFIG1, reg);
}

void Protocentral_ADS1220::setGain(ADS1220_Gain gain) {
    uint8_t reg = (_configReg[0] & ~ADS1220_CONFIG0_GAIN_MASK) | gain;
    writeRegister(ADS1220_REG_CONFIG0, reg);
    _gain = gainToFloat(gain);
}

void Protocentral_ADS1220::enablePGA(bool enable) {
    uint8_t reg = _configReg[0];
    if (enable) {
        reg &= ~ADS1220_CONFIG0_PGA_BYPASS;   // PGA_BYPASS = 0 -> PGA enabled
    } else {
        reg |= ADS1220_CONFIG0_PGA_BYPASS;
    }
    writeRegister(ADS1220_REG_CONFIG0, reg);
}

void Protocentral_ADS1220::setInputMux(ADS1220_Mux mux) {
    uint8_t reg = (_configReg[0] & ~ADS1220_CONFIG0_MUX_MASK) | mux;
    writeRegister(ADS1220_REG_CONFIG0, reg);
}

void Protocentral_ADS1220::setReference(ADS1220_Reference ref) {
    uint8_t reg = (_configReg[2] & ~ADS1220_CONFIG2_VREF_MASK) | ref;
    writeRegister(ADS1220_REG_CONFIG2, reg);
    _vref = referenceToVoltage(ref);
}

void Protocentral_ADS1220::setReferenceVoltage(float volts) {
    _vref = volts;
}

void Protocentral_ADS1220::setFIRFilter(ADS1220_FIRFilter filter) {
    uint8_t reg = (_configReg[2] & ~ADS1220_CONFIG2_FIR_MASK) | filter;
    writeRegister(ADS1220_REG_CONFIG2, reg);
}

void Protocentral_ADS1220::setConversionMode(bool continuous) {
    uint8_t reg = _configReg[1];
    if (continuous) {
        reg |= ADS1220_CONFIG1_CM_BIT;
    } else {
        reg &= ~ADS1220_CONFIG1_CM_BIT;
    }
    writeRegister(ADS1220_REG_CONFIG1, reg);
    _continuousMode = continuous;
}

void Protocentral_ADS1220::enableTemperatureSensor(bool enable) {
    uint8_t reg = _configReg[1];
    if (enable) {
        reg |= ADS1220_CONFIG1_TS_BIT;
    } else {
        reg &= ~ADS1220_CONFIG1_TS_BIT;
    }
    writeRegister(ADS1220_REG_CONFIG1, reg);
}

void Protocentral_ADS1220::enableBurnoutSources(bool enable) {
    uint8_t reg = _configReg[1];
    if (enable) {
        reg |= ADS1220_CONFIG1_BCS_BIT;
    } else {
        reg &= ~ADS1220_CONFIG1_BCS_BIT;
    }
    writeRegister(ADS1220_REG_CONFIG1, reg);
}

void Protocentral_ADS1220::setIDACCurrent(ADS1220_IDACCurrent current) {
    uint8_t reg = (_configReg[2] & ~ADS1220_CONFIG2_IDAC_MASK) | current;
    writeRegister(ADS1220_REG_CONFIG2, reg);
}

void Protocentral_ADS1220::setIDAC1Route(ADS1220_IDAC1Route route) {
    uint8_t reg = (_configReg[3] & ~ADS1220_CONFIG3_I1MUX_MASK) | route;
    writeRegister(ADS1220_REG_CONFIG3, reg);
}

void Protocentral_ADS1220::setIDAC2Route(ADS1220_IDAC2Route route) {
    uint8_t reg = (_configReg[3] & ~ADS1220_CONFIG3_I2MUX_MASK) | route;
    writeRegister(ADS1220_REG_CONFIG3, reg);
}

void Protocentral_ADS1220::enableLowSideSwitch(bool enable) {
    uint8_t reg = _configReg[2];
    if (enable) {
        reg |= ADS1220_CONFIG2_PSW_BIT;
    } else {
        reg &= ~ADS1220_CONFIG2_PSW_BIT;
    }
    writeRegister(ADS1220_REG_CONFIG2, reg);
}

void Protocentral_ADS1220::setDRDYOnDOUT(bool enable) {
    uint8_t reg = _configReg[3];
    if (enable) {
        reg |= ADS1220_CONFIG3_DRDYM_BIT;
    } else {
        reg &= ~ADS1220_CONFIG3_DRDYM_BIT;
    }
    writeRegister(ADS1220_REG_CONFIG3, reg);
}

void Protocentral_ADS1220::applyConfig(const ADS1220_Config& config) {
    setInputMux(config.mux);
    setGain(config.gain);
    enablePGA(!config.pgaBypass);
    setDataRate(config.dataRate);
    setOperatingMode(config.opMode);
    setConversionMode(config.continuousMode);
    setReference(config.reference);
    setFIRFilter(config.firFilter);
}

// ---------------------------------------------------------------------------
// Acquisition
// ---------------------------------------------------------------------------

void Protocentral_ADS1220::startConversion() {
    spiCommand(ADS1220_CMD_START);
}

void Protocentral_ADS1220::stopConversion() {
    spiCommand(ADS1220_CMD_POWERDOWN);
}

bool Protocentral_ADS1220::isDataReady() const {
    return digitalRead(_drdyPin) == LOW;
}

bool Protocentral_ADS1220::waitForDRDY(uint32_t timeout_ms) {
    uint32_t start = millis();
    while (digitalRead(_drdyPin) == HIGH) {
        if ((millis() - start) >= timeout_ms) {
            return false;
        }
    }
    return true;
}

int32_t Protocentral_ADS1220::readConversion() {
    uint8_t buffer[3];

    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);
    buffer[0] = _spi->transfer(ADS1220_SPI_DUMMY_BYTE);
    buffer[1] = _spi->transfer(ADS1220_SPI_DUMMY_BYTE);
    buffer[2] = _spi->transfer(ADS1220_SPI_DUMMY_BYTE);
    delayMicroseconds(1);
    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();

    int32_t result = ((int32_t)buffer[0] << 16) |
                     ((int32_t)buffer[1] << 8) |
                      (int32_t)buffer[2];

    // Sign-extend the 24-bit two's-complement value to 32 bits.
    if (result & 0x00800000) {
        result |= 0xFF000000;
    }
    return result;
}

int32_t Protocentral_ADS1220::readRaw() {
    if (!_continuousMode) {
        startConversion();
    }
    if (!waitForDRDY(1000)) {
        return 0;
    }
    return readConversion();
}

float Protocentral_ADS1220::readVoltage() {
    return countsToVoltage(readRaw());
}

bool Protocentral_ADS1220::readRawAsync(int32_t& result) {
    if (!isDataReady()) {
        return false;
    }
    result = readConversion();
    return true;
}

bool Protocentral_ADS1220::readVoltageAsync(float& result) {
    int32_t raw;
    if (!readRawAsync(raw)) {
        return false;
    }
    result = countsToVoltage(raw);
    return true;
}

int32_t Protocentral_ADS1220::readSingleEnded(uint8_t channel) {
    static const ADS1220_Mux seMux[4] = {
        ADS1220_MUX_AIN0_AVSS, ADS1220_MUX_AIN1_AVSS,
        ADS1220_MUX_AIN2_AVSS, ADS1220_MUX_AIN3_AVSS
    };
    if (channel > 3) {
        channel = 0;
    }
    setInputMux(seMux[channel]);
    return readRaw();
}

float Protocentral_ADS1220::readTemperature() {
    uint8_t savedReg1 = _configReg[1];

    enableTemperatureSensor(true);
    // Allow the modulator to settle after the mode change.
    startConversion();
    waitForDRDY(1000);
    int32_t raw = readConversion();

    // The temperature result is the 14 most-significant bits, 0.03125 C / LSB.
    int32_t tempCode = raw >> 10;
    if (tempCode & 0x2000) {            // sign bit of a 14-bit value
        tempCode |= 0xFFFFC000;        // sign-extend to 32 bits
    }
    float celsius = tempCode * 0.03125f;

    // Restore the previous CONFIG1 (temperature-sensor bit cleared).
    writeRegister(ADS1220_REG_CONFIG1, savedReg1);

    return celsius;
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

void Protocentral_ADS1220::reset() {
    spiCommand(ADS1220_CMD_RESET);
    delayMicroseconds(100);
}

bool Protocentral_ADS1220::testConnection() {
    // Write a recognisable pattern to CONFIG3 (reserved low bit stays 0) and
    // read it back. Restore the shadowed value afterwards.
    uint8_t saved = _configReg[3];
    writeRegister(ADS1220_REG_CONFIG3, 0x02);
    uint8_t readback = readRegister(ADS1220_REG_CONFIG3);
    writeRegister(ADS1220_REG_CONFIG3, saved);
    return (readback == 0x02);
}

void Protocentral_ADS1220::getConfigRegisters(uint8_t out[4]) {
    for (uint8_t i = 0; i < 4; i++) {
        out[i] = readRegister(i);
        _configReg[i] = out[i];
    }
}

void Protocentral_ADS1220::printRegisters(Stream& stream) {
    uint8_t regs[4];
    getConfigRegisters(regs);
    stream.println(F("ADS1220 configuration registers:"));
    for (uint8_t i = 0; i < 4; i++) {
        stream.print(F("  CONFIG"));
        stream.print(i);
        stream.print(F(" (0x0"));
        stream.print(i);
        stream.print(F("): 0x"));
        if (regs[i] < 0x10) {
            stream.print('0');
        }
        stream.println(regs[i], HEX);
    }
}

float Protocentral_ADS1220::countsToVoltage(int32_t counts) const {
    return ((float)counts / (float)ADS1220_FULL_SCALE) * (_vref / _gain);
}

int32_t Protocentral_ADS1220::voltageToCounts(float voltage) const {
    return (int32_t)((voltage * _gain / _vref) * (float)ADS1220_FULL_SCALE);
}

const char* Protocentral_ADS1220::getLibraryVersion() {
    return ADS1220_LIBRARY_VERSION;
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

float Protocentral_ADS1220::gainToFloat(ADS1220_Gain gain) {
    switch (gain) {
        case ADS1220_GAIN_1:   return 1.0f;
        case ADS1220_GAIN_2:   return 2.0f;
        case ADS1220_GAIN_4:   return 4.0f;
        case ADS1220_GAIN_8:   return 8.0f;
        case ADS1220_GAIN_16:  return 16.0f;
        case ADS1220_GAIN_32:  return 32.0f;
        case ADS1220_GAIN_64:  return 64.0f;
        case ADS1220_GAIN_128: return 128.0f;
        default:               return 1.0f;
    }
}

float Protocentral_ADS1220::referenceToVoltage(ADS1220_Reference ref) {
    // Only the internal reference has a known, fixed value. For external and
    // analog-supply references the caller should use setReferenceVoltage().
    switch (ref) {
        case ADS1220_REF_INTERNAL_2V048: return 2.048f;
        default:                         return 2.048f;
    }
}
