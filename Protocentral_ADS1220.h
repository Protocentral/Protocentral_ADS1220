//#ifndef ads1292r_h
//#define ads1292r_h

#include "Arduino.h"

#include "SPI.h" // necessary library

#define SPI_MASTER_DUMMY   0xFF
#define RESET 0x06                                                    //Send the RESET command (06h) to make sure the ADS1220 is properly reset after power-up
#define START 0x08                                         //Send the START/SYNC command (08h) to start converting in continuous conversion mode
//#define WREG  0x43
#define WREG  0x40
//#define RREG  0x23
#define RREG  0x20
#define CONFIG_REG0_ADDRESS 0x00
#define CONFIG_REG1_ADDRESS 0x01
#define CONFIG_REG2_ADDRESS 0x02
#define CONFIG_REG3_ADDRESS 0x03


#define REG_CONFIG_DR_20SPS 0x00
#define REG_CONFIG_DR_45SPS 0x20
#define REG_CONFIG_DR_90SPS 0x40
#define REG_CONFIG_DR_175SPS 0x60
#define REG_CONFIG_DR_330SPS 0x80
#define REG_CONFIG_DR_600SPS 0xA0
#define REG_CONFIG_DR_1000SPS 0xC0
#define REG_CONFIG_DR_MASK 0xE0

#define DR_20SPS 20
#define DR_45SPS 45
#define DR_90SPS 90
#define DR_175SPS 175
#define DR_330SPS 330
#define DR_600SPS 600
#define DR_1000SPS 1000

#define REG_CONFIG_PGA_GAIN_1 0x00
#define REG_CONFIG_PGA_GAIN_2 0x02
#define REG_CONFIG_PGA_GAIN_4 0x04
#define REG_CONFIG_PGA_GAIN_8 0x06
#define REG_CONFIG_PGA_GAIN_16 0x08
#define REG_CONFIG_PGA_GAIN_32 0x0A
#define REG_CONFIG_PGA_GAIN_64 0x0C
#define REG_CONFIG_PGA_GAIN_128 0x0E
#define REG_CONFIG_PGA_GAIN_MASK 0x0E

#define PGA_GAIN_1 1
#define PGA_GAIN_2 2
#define PGA_GAIN_4 4
#define PGA_GAIN_8 8
#define PGA_GAIN_16 16
#define PGA_GAIN_32 32
#define PGA_GAIN_64 64
#define PGA_GAIN_128 128



//#define PGA_ON (Config_Reg0 & ~_BV(0))
//#define PGA_OFF (Config_Reg0 | _BV(0))

#define ADS1220_CS_PIN 7
#define ADS1220_DRDY_PIN 6
#define _BV(bit) (1<<(bit))  

class Protocentral_ADS1220
{
  private:
    uint8_t Config_Reg0;
    uint8_t Config_Reg1;
    uint8_t Config_Reg2;
    uint8_t Config_Reg3;

  public:
    uint8_t NewDataAvailable;

    Protocentral_ADS1220();
    void begin(void);
    static void SPI_Start(void);
    static void SPI_Reset(void);

    static void SPI_Command(unsigned char data_in);
    void writeRegister(uint8_t address, uint8_t value);
    uint8_t readRegister(uint8_t address);
    uint8_t * Read_Data(void);
    uint8_t * get_config_reg(void);

    void PGA_OFF(void);
    void PGA_ON(void);
    void Continuous_conversion_mode_ON(void);
    void Single_shot_mode_ON(void);
    void set_data_rate(int datarate);
    void set_pga_gain(int pgagain);
};