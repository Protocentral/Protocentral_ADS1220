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
#include <limits>

//#define BOARD_SENSYTHING ST_1_3


#ifdef _BV
#undef _BV
#endif

#define _BV(bit) (1<<(bit))


Protocentral_ADS1220::Protocentral_ADS1220() 								// Constructors
{

}

void Protocentral_ADS1220::writeRegister(uint8_t address, uint8_t value)
{
    digitalWrite(m_cs_pin,LOW);
    delayMicroseconds(1);
    SPI.transfer(WREG|(address<<2));
    SPI.transfer(value);
    delayMicroseconds(1);
    digitalWrite(m_cs_pin,HIGH);
}

uint8_t Protocentral_ADS1220::readRegister(uint8_t address)
{
    uint8_t data;

    digitalWrite(m_cs_pin,LOW);
    delayMicroseconds(1);
    SPI.transfer(RREG|(address<<2));
    data = SPI.transfer(SPI_MASTER_DUMMY);
    delayMicroseconds(1);
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
    SPI.setClockDivider(SPI_CLOCK_DIV2);
#endif
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE1);

    ads1220_Reset();
    delayMicroseconds(50);
    
    // The device pulls nDRDY low after it is initialized
    if(!WaitForData(1)){
        // TODO: Handle the timeout
    }

    m_config_reg0 = 0x00;   //Default settings: AINP=AIN0, AINN=AIN1, Gain 1, PGA enabled
    m_config_reg1 = 0x04;   //Default settings: DR=20 SPS, Mode=Normal, Conv mode=continuous, Temp Sensor disabled, Current Source off
    m_config_reg2 = 0x10;   //Default settings: Vref internal, 50/60Hz rejection, power open, IDAC off
    m_config_reg3 = 0x00;   //Default settings: IDAC1 disabled, IDAC2 disabled, DRDY pin only

    writeRegister( CONFIG_REG0_ADDRESS , m_config_reg0);
    writeRegister( CONFIG_REG1_ADDRESS , m_config_reg1);
    writeRegister( CONFIG_REG2_ADDRESS , m_config_reg2);
    writeRegister( CONFIG_REG3_ADDRESS , m_config_reg3);
}

void Protocentral_ADS1220::PrintRegisterValues(){
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
}

void Protocentral_ADS1220::SPI_Command(unsigned char data_in)
{
    digitalWrite(m_cs_pin, LOW);
    delayMicroseconds(1);
    SPI.transfer(data_in);
    delayMicroseconds(1);
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

bool Protocentral_ADS1220::WaitForData(unsigned int timeout_ms){
    while(digitalRead(m_drdy_pin)){
        if(timeout_ms){
            delay(1);
            --timeout_ms;
        } else {
            return false;
        }
    }
    return true;
}

uint8_t * Protocentral_ADS1220::Read_Data(void){
    digitalWrite(m_cs_pin, LOW);                         //Take CS low
    delayMicroseconds(1);
    for (int i = 0; i < 3; i++)
    {
        DataReg[i] = SPI.transfer(0);
    }
    delayMicroseconds(1);
    digitalWrite(m_cs_pin, HIGH);                  //  Clear CS to high

    return DataReg;
}

int32_t Protocentral_ADS1220::DataToInt(){
    int32_t result = 0;
    result = DataReg[0];
    result = (result << 8) | DataReg[1];
    result = (result << 8) | DataReg[2];

    if (DataReg[0] & (1<<7)) {
        result |= 0xFF000000;
    }

    return result;
}

int32_t Protocentral_ADS1220::Read_WaitForData()
{
    if(!WaitForData(60)){
        return std::numeric_limits<int32_t>::min();
    }
    Read_Data();
    return DataToInt();
}

int32_t Protocentral_ADS1220::Read_SingleShot_WaitForData(void)
{
    Start_Conv();
    return Read_WaitForData();
}

int32_t Protocentral_ADS1220::Read_SingleShot_SingleEnded_WaitForData(uint8_t channel_no)
{
    select_mux_channels(channel_no);
    return Read_SingleShot_WaitForData();
}

#define VREF_MASK ((1 << 6) & (1<<7))
#define VREF_INT (0 << 6)
#define VREF_EXT (1 << 6)

void Protocentral_ADS1220::internal_reference(){
    m_config_reg2 &= ~VREF_MASK;
    m_config_reg2 |= VREF_INT;
    writeRegister(CONFIG_REG2_ADDRESS,m_config_reg2);
}

void Protocentral_ADS1220::external_reference(){
    m_config_reg2 &= ~VREF_MASK;
    m_config_reg2 |= VREF_EXT;
    writeRegister(CONFIG_REG2_ADDRESS,m_config_reg2);
}
