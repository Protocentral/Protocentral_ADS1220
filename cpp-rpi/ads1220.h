#pragma once

#include <cstdint>

class SPI;

class Ads1220 {
public:
  Ads1220(int csPin, int drdyPin);
  Ads1220();
  ~Ads1220();

  enum MultiplexerConfiguration {
    AIN0_SINGLE_ENDED = 0b1000,
    AIN1_SINGLE_ENDED = 0b1001,
    AIN2_SINGLE_ENDED = 0b1010,
    AIN3_SINGLE_ENDED = 0b1011,
    // Check the datasheet (section 8.6.1.1) for more configurations
  };

  enum PgaGain {
    GAIN_1    = 0b000, // default
    GAIN_2    = 0b001,
    GAIN_4    = 0b010,
    GAIN_8    = 0b011,
    GAIN_16   = 0b100,
    GAIN_32   = 0b101,
    GAIN_64   = 0b110,
    GAIN_128  = 0b111,
  };

  enum DataRate {
    // SPS values are for the normal operating mode
    SPS_20    = 0b000, // default
    SPS_45    = 0b001,
    SPS_90    = 0b010,
    SPS_175   = 0b011,
    SPS_330   = 0b100,
    SPS_600   = 0b101,
    SPS_1000  = 0b110,
  };

  enum OperatingMode {
    NORMAL      = 0b00, // default
    DUTY_CYCLE  = 0b01,
    TURBO       = 0b10,
  };

  enum ConversionMode {
    SINGLE_SHOT = 0b0, // default
    CONTINUOUS  = 0b1,
  };

  enum VoltageReferenceSource {
    INTERNAL_2048_mV = 0b00, // default
    EXTERNAL_REFP0_REFN0 = 0b01,
    EXTERNAL_AIN0_REFP1_AIN3_REFN1 = 0b10,
    ANALOG_SUPPLY_AVDD_AVSS = 0b11,
  };

  uint32_t readValue(MultiplexerConfiguration configuration);
  uint32_t readUntilNewValue(MultiplexerConfiguration configuration, int32_t previousValue);

  void setPgaGain(PgaGain gain);
  void setPgaBypass(bool bypass); // default: false (enabled)
  void setDataRate(DataRate rate);
  void setOperatingMode(OperatingMode mode);
  void setConversionMode(ConversionMode mode);
  void setVoltageReferenceSource(VoltageReferenceSource source);

  void setShiftResultBy(int value) { _shiftResultBy = value; }
  void setMaxResult(int value) { _maxResult = value; }
  void setDetectionThreshold(int value) { _detectionThreshold = value; }
  void setSleepBetweenCommandsInMicroSeconds(int value) { _sleepBetweenCommandsInMicroSeconds = value; }

private:
  SPI*      spi = nullptr;

  int       _csPin;
  int       _drdyPin;

  bool      _singleShotMode = true;

  uint8_t   _configReg0 = 0;
  uint8_t   _configReg1 = 0;
  uint8_t   _configReg2 = 0;
  uint8_t   _configReg3 = 0;

  uint8_t   _txBuffer[2];
  uint8_t   _rxBuffer[3];

  int       _shiftResultBy = 10;
  int       _maxResult = 8192;
  int       _detectionThreshold = 3;

  int       _sleepBetweenCommandsInMicroSeconds = 500;

  enum SpiCommand {
    RESET     = 0b00000110,
    START     = 0b00001000,
    POWERDOWN = 0b00000010,
    RDATA     = 0b00010000,
    RREG      = 0b00100000,
    WREG      = 0b01000000,
  };

  void sendSpiCommand(SpiCommand command, uint8_t payload);
  void writeRegister(uint8_t addr, uint8_t data);
  uint8_t readRegister(uint8_t addr);

  void selectMultiplexerConfiguration(MultiplexerConfiguration configuration);

  static constexpr const int DEFAULT_CS_PIN = 18;
  static constexpr const int DEFAULT_DRDY_PIN = 13;

  static constexpr const uint8_t REG0_ADDR = 0x00;
  static constexpr const uint8_t REG1_ADDR = 0x01;
  static constexpr const uint8_t REG2_ADDR = 0x02;
  static constexpr const uint8_t REG3_ADDR = 0x03;

  static constexpr const uint8_t MASK_CONFIG_0_MUX        = 0b11110000;
  static constexpr const uint8_t MASK_CONFIG_0_PGA_GAIN   = 0b00001110;
  static constexpr const uint8_t MASK_CONFIG_0_PGA_BYPASS = 0b00000001;

  static constexpr const uint8_t MASK_CONFIG_1_DATA_RATE        = 0b11100000;
  static constexpr const uint8_t MASK_CONFIG_1_OPERATING_MODE   = 0b00011000;
  static constexpr const uint8_t MASK_CONFIG_1_CONVERSION_MODE  = 0b00000100;
  static constexpr const uint8_t MASK_CONFIG_1_TEMPERATURE_SENSOR_MODE = 0b00000010;
  static constexpr const uint8_t MASK_CONFIG_1_BURNOUT_CURRENT_SOURCES = 0b00000001;

  static constexpr const uint8_t MASK_CONFIG_2_VOLTAGE_REFERENCE  = 0b11000000;
  static constexpr const uint8_t MASK_CONFIG_2_FIR_FILTER         = 0b00110000;
  static constexpr const uint8_t MASK_CONFIG_2_LOW_SIDE_SWITCH    = 0b00001000;
  static constexpr const uint8_t MASK_CONFIG_2_IDAC_CURRENT       = 0b00000111;

  static constexpr const uint8_t MASK_CONFIG_3_IDAC1_ROUTING      = 0b11100000;
  static constexpr const uint8_t MASK_CONFIG_3_IDAC2_ROUTING      = 0b00011100;
  static constexpr const uint8_t MASK_CONFIG_3_DRDY_MODE          = 0b00000010;
};