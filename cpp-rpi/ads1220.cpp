#include "ads1220.h"

#include <unistd.h>
#include <iostream>

#include <spidev_lib++.h>
#include <wiringPi.h>

using namespace std;

Ads1220::Ads1220(int csPin, int drdyPin) : _csPin(csPin), _drdyPin(drdyPin) {
  pinMode(csPin, OUTPUT);
  pinMode(drdyPin, INPUT);

  string spidevPath = "/dev/spidev0.1";
  spi = new SPI(spidevPath.c_str());
  if (!spi->begin()) {
    cerr << "Could't init SPI on path " << spidevPath << endl;
    exit(1);
  }
  spi->setMode(1);
  spi->setSpeed(50000);
  spi->setBitPerWord(8);

  sendSpiCommand(SpiCommand::RESET, 0);
  usleep(_sleepBetweenCommandsInMicroSeconds);
  digitalWrite(csPin, LOW);

  writeRegister(REG0_ADDR, _configReg0);
  writeRegister(REG1_ADDR, _configReg1);
  writeRegister(REG2_ADDR, _configReg2);
  writeRegister(REG3_ADDR, _configReg3);

  setPgaBypass(true);
  setDataRate(DataRate::SPS_1000);
  setOperatingMode(OperatingMode::TURBO);
  setConversionMode(ConversionMode::CONTINUOUS);
  setVoltageReferenceSource(VoltageReferenceSource::ANALOG_SUPPLY_AVDD_AVSS);

  usleep(_sleepBetweenCommandsInMicroSeconds);
  cout << "Initialised SPI on " << spidevPath << " with config: "
      << int(readRegister(REG0_ADDR)) << ", " << int(readRegister(REG1_ADDR)) << ", " 
      << int(readRegister(REG2_ADDR)) << ", " << int(readRegister(REG3_ADDR)) << endl;

  digitalWrite(csPin, HIGH);

  usleep(300000);
}

Ads1220::Ads1220() : Ads1220(DEFAULT_CS_PIN, DEFAULT_DRDY_PIN) {}

Ads1220::~Ads1220() {
  delete spi;
}

uint32_t Ads1220::readValue(MultiplexerConfiguration configuration) {
  selectMultiplexerConfiguration(configuration);
  usleep(_sleepBetweenCommandsInMicroSeconds);

  if (_singleShotMode) {
    sendSpiCommand(SpiCommand::START, 0);
    usleep(_sleepBetweenCommandsInMicroSeconds);
  }

  while (digitalRead(_drdyPin) != LOW) {
    usleep(_sleepBetweenCommandsInMicroSeconds);
  }

  digitalWrite(_csPin, LOW);
  usleep(_sleepBetweenCommandsInMicroSeconds);

  spi->read(_rxBuffer + 0, 1);
  spi->read(_rxBuffer + 1, 1);
  spi->read(_rxBuffer + 2, 1);

  uint32_t value = (_rxBuffer[0] << 16) | (_rxBuffer[1] << 8) | _rxBuffer[2];

  digitalWrite(_csPin, HIGH);
  usleep(_sleepBetweenCommandsInMicroSeconds);

  return value;
}

uint32_t Ads1220::readUntilNewValue(MultiplexerConfiguration configuration, int32_t previousValue) {
  while (true) {
    uint32_t value = readValue(configuration) >> _shiftResultBy;
    int32_t diff = value - previousValue;
    if (value <= _maxResult && abs(diff) > _detectionThreshold) {
      return value;
    }
  }
}

void Ads1220::setPgaGain(PgaGain gain) {
  _configReg0 &= ~MASK_CONFIG_0_PGA_GAIN;
  _configReg0 |= int(gain) << 1;
  writeRegister(REG0_ADDR, _configReg0);
}

void Ads1220::setPgaBypass(bool bypass) {
  _configReg0 &= ~MASK_CONFIG_0_PGA_BYPASS;
  _configReg0 |= bypass;
  writeRegister(REG0_ADDR, _configReg0);
}

void Ads1220::setDataRate(DataRate rate) {
  _configReg1 &= ~MASK_CONFIG_1_DATA_RATE;
  _configReg1 |= int(rate) << 5;
  writeRegister(REG1_ADDR, _configReg1);
}

void Ads1220::setOperatingMode(OperatingMode mode) {
  _configReg1 &= ~MASK_CONFIG_1_OPERATING_MODE;
  _configReg1 |= int(mode) << 3;
  writeRegister(REG1_ADDR, _configReg1);
}

void Ads1220::setConversionMode(ConversionMode mode) {
  _configReg1 &= ~MASK_CONFIG_1_CONVERSION_MODE;
  _configReg1 |= int(mode) << 2;
  writeRegister(REG1_ADDR, _configReg1);
  _singleShotMode = mode == ConversionMode::SINGLE_SHOT;
}

void Ads1220::setVoltageReferenceSource(VoltageReferenceSource source) {
  _configReg2 &= ~MASK_CONFIG_2_VOLTAGE_REFERENCE;
  _configReg2 |= int(source) << 6;
  writeRegister(REG2_ADDR, _configReg2);
}

void Ads1220::sendSpiCommand(SpiCommand command, uint8_t payload) {
  digitalWrite(_csPin, LOW);
  usleep(_sleepBetweenCommandsInMicroSeconds);
  digitalWrite(_csPin, HIGH);
  usleep(_sleepBetweenCommandsInMicroSeconds);
  digitalWrite(_csPin, LOW);
  usleep(_sleepBetweenCommandsInMicroSeconds);

  _txBuffer[0] = int(command) | payload;
  spi->write(_txBuffer, 1);

  usleep(_sleepBetweenCommandsInMicroSeconds);
  digitalWrite(_csPin, HIGH);
}

void Ads1220::writeRegister(uint8_t addr, uint8_t data) {
  digitalWrite(_csPin, LOW);
  usleep(_sleepBetweenCommandsInMicroSeconds);

  // 0100rrnn - rr=register address; nn=number bytes to write
  _txBuffer[0] = int(SpiCommand::WREG) | (addr << 2);
  _txBuffer[1] = data;
  spi->write(_txBuffer, 2);

  usleep(_sleepBetweenCommandsInMicroSeconds);
  digitalWrite(_csPin, HIGH);
}

uint8_t Ads1220::readRegister(uint8_t addr) {
  digitalWrite(_csPin, LOW);
  usleep(_sleepBetweenCommandsInMicroSeconds);

  // 0010rrnn - rr=register address; nn=number bytes to read
  _txBuffer[0] = int(SpiCommand::RREG) | (addr << 2);
  spi->write(_txBuffer, 1);
  spi->read(_rxBuffer, 1);

  usleep(_sleepBetweenCommandsInMicroSeconds);
  digitalWrite(_csPin, HIGH);

  return _rxBuffer[0];
}

void Ads1220::selectMultiplexerConfiguration(MultiplexerConfiguration configuration) {
  _configReg0 &= ~MASK_CONFIG_0_MUX;
  _configReg0 |= int(configuration) << 4;
  writeRegister(REG0_ADDR, _configReg0);
}
