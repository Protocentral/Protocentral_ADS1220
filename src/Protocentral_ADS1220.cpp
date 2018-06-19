//////////////////////////////////////////////////////////////////////////////////////////
//
//    Arduino library for the ADS1220 24-bit ADC breakout board
//
//    Author: Ashwin Whitchurch
//    Copyright (c) 2018 ProtoCentral
//
//    Based on original code from Texas Instruments
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   For information on how to use, visit https://github.com/Protocentral/Protocentral_ADS1220/
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "Protocentral_ADS1220.h"
#include <SPI.h>

//#define BOARD_SENSYTHING ST_1_3

Protocentral_ADS1220::Protocentral_ADS1220() 								// Constructors
{

}

void Protocentral_ADS1220::writeRegister(uint8_t address, uint8_t value)
{
    digitalWrite(m_cs_pin,LOW);
    delay(5);
    SPI.transfer(WREG|(address<<2));
    SPI.transfer(value);
    delay(5);
    digitalWrite(m_cs_pin,HIGH);
}

uint8_t Protocentral_ADS1220::readRegister(uint8_t address)
{
    uint8_t data;

    digitalWrite(m_cs_pin,LOW);
    delay(5);
    SPI.transfer(RREG|(address<<2));
    data = SPI.transfer(SPI_MASTER_DUMMY);
    delay(5);
    digitalWrite(m_cs_pin,HIGH);

    return data;
}

void Protocentral_ADS1220::begin(uint8_t cs_pin, uint8_t drdy_pin)
{
    m_drdy_pin=drdy_pin;
    m_cs_pin=cs_pin;

    pinMode(m_cs_pin, OUTPUT);
    pinMode(m_drdy_pin, INPUT);

#if defined(BOARD_SENSYTHING)
    SPI.begin(18, 35, 23, 19);
#else
    SPI.begin();
#endif
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE1);

    delay(100);
    ads1220_Reset();
    delay(100);

    digitalWrite(m_cs_pin,LOW);

    m_config_reg0 = 0x00;   //Default settings: AINP=AIN0, AINN=AIN1, Gain 1, PGA enabled
    m_config_reg1 = 0x04;   //Default settings: DR=20 SPS, Mode=Normal, Conv mode=continuous, Temp Sensor disabled, Current Source off
    m_config_reg2 = 0x10;   //Default settings: Vref internal, 50/60Hz rejection, power open, IDAC off
    m_config_reg3 = 0x00;   //Default settings: IDAC1 disabled, IDAC2 disabled, DRDY pin only

    writeRegister( CONFIG_REG0_ADDRESS , m_config_reg0);
    writeRegister( CONFIG_REG1_ADDRESS , m_config_reg1);
    writeRegister( CONFIG_REG2_ADDRESS , m_config_reg2);
    writeRegister( CONFIG_REG3_ADDRESS , m_config_reg3);

    delay(100);

    Config_Reg0 = readRegister(CONFIG_REG0_ADDRESS);
    Config_Reg1 = readRegister(CONFIG_REG1_ADDRESS);
    Config_Reg2 = readRegister(CONFIG_REG2_ADDRESS);
    Config_Reg3 = readRegister(CONFIG_REG3_ADDRESS);

    Serial.println("Config_Reg : ");
    Serial.println(Config_Reg0,HEX);
    Serial.println(Config_Reg1,HEX);
    Serial.println(Config_Reg2,HEX);
    Serial.println(Config_Reg3,HEX);
    Serial.println(" ");

    digitalWrite(m_cs_pin,HIGH);

    delay(100);

    //Start_Conv();
    delay(100);
}

void Protocentral_ADS1220::SPI_Command(unsigned char data_in)
{
    digitalWrite(m_cs_pin, LOW);
    delay(2);
    digitalWrite(m_cs_pin, HIGH);
    delay(2);
    digitalWrite(m_cs_pin, LOW);
    delay(2);
    SPI.transfer(data_in);
    delay(2);
    digitalWrite(m_cs_pin, HIGH);
}

void Protocentral_ADS1220::ads1220_Reset()
{
    SPI_Command(RESET);
}

void Protocentral_ADS1220::Start_Conv()
{
    SPI_Command(START);
}

void Protocentral_ADS1220::PGA_ON(void)
{
    m_config_reg0 &= ~_BV(0);
    writeRegister(CONFIG_REG0_ADDRESS,m_config_reg0);
}

void Protocentral_ADS1220::PGA_OFF(void)
{
    m_config_reg0 |= _BV(0);
    writeRegister(CONFIG_REG0_ADDRESS,m_config_reg0);
}

void Protocentral_ADS1220::set_conv_mode_continuous(void)
{
    m_config_reg1 |= _BV(2);
    writeRegister(CONFIG_REG1_ADDRESS,m_config_reg1);
}

void Protocentral_ADS1220::set_conv_mode_single_shot(void)
{
    m_config_reg1 &= ~_BV(2);
    writeRegister(CONFIG_REG1_ADDRESS,m_config_reg1);
}

void Protocentral_ADS1220::set_data_rate(int datarate)
{
    m_config_reg1 &= ~REG_CONFIG1_DR_MASK;
    m_config_reg1 |= datarate;
    writeRegister(CONFIG_REG1_ADDRESS,m_config_reg1);
}

void Protocentral_ADS1220::select_mux_channels(int channels_conf)
{
    m_config_reg0 &= ~REG_CONFIG0_MUX_MASK;
    m_config_reg0 |= channels_conf;
    writeRegister(CONFIG_REG0_ADDRESS,m_config_reg0);
}

void Protocentral_ADS1220::set_pga_gain(int pgagain)
{
    m_config_reg0 &= ~REG_CONFIG0_PGA_GAIN_MASK;
    m_config_reg0 |= pgagain ;
    writeRegister(CONFIG_REG0_ADDRESS,m_config_reg0);
}

uint8_t * Protocentral_ADS1220::get_config_reg()
{
    static uint8_t config_Buff[4];

    m_config_reg0 = readRegister(CONFIG_REG0_ADDRESS);
    m_config_reg1 = readRegister(CONFIG_REG1_ADDRESS);
    m_config_reg2 = readRegister(CONFIG_REG2_ADDRESS);
    m_config_reg3 = readRegister(CONFIG_REG3_ADDRESS);

    config_Buff[0] = m_config_reg0 ;
    config_Buff[1] = m_config_reg1 ;
    config_Buff[2] = m_config_reg2 ;
    config_Buff[3] = m_config_reg3 ;

    return config_Buff;
}

int32_t Protocentral_ADS1220::Read_WaitForData()
{
    static byte SPI_Buff[3];
    int32_t mResult32=0;
    long int bit24;

    if((digitalRead(m_drdy_pin)) == LOW)             //        Wait for DRDY to transition low
    {
        digitalWrite(m_cs_pin,LOW);                         //Take CS low
        delayMicroseconds(100);
        for (int i = 0; i < 3; i++)
        {
          SPI_Buff[i] = SPI.transfer(SPI_MASTER_DUMMY);
        }
        delayMicroseconds(100);
        digitalWrite(m_cs_pin,HIGH);                  //  Clear CS to high

        bit24 = SPI_Buff[0];
        bit24 = (bit24 << 8) | SPI_Buff[1];
        bit24 = (bit24 << 8) | SPI_Buff[2];                                 // Converting 3 bytes to a 24 bit int

        bit24= ( bit24 << 8 );
        mResult32 = ( bit24 >> 8 );                      // Converting 24 bit two's complement to 32 bit two's complement
    }
    return mResult32;
}

int32_t Protocentral_ADS1220::Read_SingleShot_WaitForData(void)
{
    static byte SPI_Buff[3];
    int32_t mResult32=0;
    long int bit24;

    Start_Conv();

    if((digitalRead(m_drdy_pin)) == LOW)             //        Wait for DRDY to transition low
    {
        digitalWrite(m_cs_pin,LOW);                         //Take CS low
        delayMicroseconds(100);
        for (int i = 0; i < 3; i++)
        {
          SPI_Buff[i] = SPI.transfer(SPI_MASTER_DUMMY);
        }
        delayMicroseconds(100);
        digitalWrite(m_cs_pin,HIGH);                  //  Clear CS to high

        bit24 = SPI_Buff[0];
        bit24 = (bit24 << 8) | SPI_Buff[1];
        bit24 = (bit24 << 8) | SPI_Buff[2];                                 // Converting 3 bytes to a 24 bit int

        bit24= ( bit24 << 8 );
        mResult32 = ( bit24 >> 8 );                      // Converting 24 bit two's complement to 32 bit two's complement
    }
    return mResult32;
}

int32_t Protocentral_ADS1220::Read_SingleShot_SingleEnded_WaitForData(uint8_t channel_no)
{
    static byte SPI_Buff[3];
    int32_t mResult32=0;
    long int bit24;

    select_mux_channels(channel_no);
    delay(100);

    Start_Conv();
    delay(100);

    if((digitalRead(m_drdy_pin)) == LOW)             //        Wait for DRDY to transition low
    {
        digitalWrite(m_cs_pin,LOW);                         //Take CS low
        delayMicroseconds(100);
        for (int i = 0; i < 3; i++)
        {
          SPI_Buff[i] = SPI.transfer(SPI_MASTER_DUMMY);
        }
        delayMicroseconds(100);
        digitalWrite(m_cs_pin,HIGH);                  //  Clear CS to high

        bit24 = SPI_Buff[0];
        bit24 = (bit24 << 8) | SPI_Buff[1];
        bit24 = (bit24 << 8) | SPI_Buff[2];                                 // Converting 3 bytes to a 24 bit int

        bit24= ( bit24 << 8 );
        mResult32 = ( bit24 >> 8 );                      // Converting 24 bit two's complement to 32 bit two's complement
    }
    return mResult32;
}
