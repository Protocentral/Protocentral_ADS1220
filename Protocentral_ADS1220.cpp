#include <Arduino.h>
#include "Protocentral_ADS1220.h"
#include <SPI.h>


 
void Protocentral_ADS1220::writeRegister(uint8_t address, uint8_t value)
{
  digitalWrite(ADS1220_CS_PIN,LOW);
  delay(5);
  SPI.transfer(WREG|(address<<2));      	
  SPI.transfer(value); 
  delay(5);
  digitalWrite(ADS1220_CS_PIN,HIGH);
}  

uint8_t Protocentral_ADS1220::readRegister(uint8_t address)
{
  uint8_t data;

  digitalWrite(ADS1220_CS_PIN,LOW);
  delay(5);
  SPI.transfer(RREG|(address<<2));      	
  data = SPI.transfer(SPI_MASTER_DUMMY); 
  delay(5);
  digitalWrite(ADS1220_CS_PIN,HIGH);

  return data;
}  


void Protocentral_ADS1220::begin()
{
  static char data;

  Serial.begin(9600);	        	//115200 57600
  SPI.begin();                           // wake up the SPI bus.
  //SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
	
  delay(100);
  SPI_Reset();                                            
  delay(100);                                                    

  digitalWrite(ADS1220_CS_PIN,LOW);
  /*SPI.transfer(WREG);           //WREG command (43h, 08h, 04h, 10h, and 00h)
  SPI.transfer(0x01);      	
  SPI.transfer(0x04);     
  SPI.transfer(0x10);    
  SPI.transfer(0x00);   
  */
  Config_Reg0 = 0x01;
  Config_Reg1 = 0x04;
  Config_Reg2 = 0x10;
  Config_Reg3 = 0x00;
  

  writeRegister( CONFIG_REG0_ADDRESS , Config_Reg0);
  writeRegister( CONFIG_REG1_ADDRESS , Config_Reg1);
  writeRegister( CONFIG_REG2_ADDRESS , Config_Reg2);
  writeRegister( CONFIG_REG3_ADDRESS , Config_Reg3);
  delay(100);
  /*
  SPI.transfer(RREG);           //RREG
  data = SPI.transfer(SPI_MASTER_DUMMY);
  //Serial.println(data);
  data = SPI.transfer(SPI_MASTER_DUMMY); 
  //Serial.println(data);
  data = SPI.transfer(SPI_MASTER_DUMMY); 
  //Serial.println(data);
  data = SPI.transfer(SPI_MASTER_DUMMY); 
  //Serial.println(data);
  */

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
  digitalWrite(ADS1220_CS_PIN,HIGH); //release chip, signal end transfer

  SPI_Start();
  delay(100);

}

void Protocentral_ADS1220::SPI_Command(unsigned char data_in)
{
  digitalWrite(ADS1220_CS_PIN, LOW);
  delay(2);
  digitalWrite(ADS1220_CS_PIN, HIGH);
  delay(2);
  digitalWrite(ADS1220_CS_PIN, LOW);
  delay(2);
  SPI.transfer(data_in);
  delay(2);
  digitalWrite(ADS1220_CS_PIN, HIGH);
}

void Protocentral_ADS1220::SPI_Reset()
{
  SPI_Command(RESET);		                    			
}

void Protocentral_ADS1220::SPI_Start()
{
  SPI_Command(START);
}


Protocentral_ADS1220::Protocentral_ADS1220() 								// Constructors 
{
  Serial.begin(9600);	        	//115200 57600
  Serial.println("ads1220 class declared");
  NewDataAvailable = false;
}

void Protocentral_ADS1220::PGA_ON(void)
{	 
  Config_Reg0 &= ~_BV(0);
  writeRegister(CONFIG_REG0_ADDRESS,Config_Reg0);	
}

void Protocentral_ADS1220::PGA_OFF(void)
{	 
  Config_Reg0 |= _BV(0);
  writeRegister(CONFIG_REG0_ADDRESS,Config_Reg0);	
}

void Protocentral_ADS1220::Continuous_conversion_mode_ON(void)
{
  Config_Reg1 |= _BV(2);
  writeRegister(CONFIG_REG1_ADDRESS,Config_Reg1);
}

void Protocentral_ADS1220::Single_shot_mode_ON(void)
{
  Config_Reg1 &= ~_BV(2);
  writeRegister(CONFIG_REG1_ADDRESS,Config_Reg1);
}


void Protocentral_ADS1220::set_data_rate(int datarate)
{
  Config_Reg1 &= ~REG_CONFIG_DR_MASK;
  
  switch(datarate)
  {
    case(DR_20SPS):
      Config_Reg1 |= REG_CONFIG_DR_20SPS; 
      break;
    case(DR_45SPS):
      Config_Reg1 |= REG_CONFIG_DR_45SPS; 
      break;
    case(DR_90SPS):
      Config_Reg1 |= REG_CONFIG_DR_90SPS; 
      break;
    case(DR_175SPS):
      Config_Reg1 |= REG_CONFIG_DR_175SPS; 
      break;
    case(DR_330SPS):
      Config_Reg1 |= REG_CONFIG_DR_330SPS; 
      break;
    case(DR_600SPS):
      Config_Reg1 |= REG_CONFIG_DR_600SPS; 
      break;
    case(DR_1000SPS):
      Config_Reg1 |= REG_CONFIG_DR_1000SPS; 
      break;
  }

  writeRegister(CONFIG_REG1_ADDRESS,Config_Reg1);
}


void Protocentral_ADS1220::set_pga_gain(int pgagain)
{
  Config_Reg0 &= ~REG_CONFIG_PGA_GAIN_MASK;

  switch(pgagain)
  {
    case(PGA_GAIN_1):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_1 ; 
      break;
    case(PGA_GAIN_2):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_2; 
      break;
    case(PGA_GAIN_4):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_4; 
      break;
    case(PGA_GAIN_8):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_8; 
      break;
    case(PGA_GAIN_16):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_16; 
      break;
    case(PGA_GAIN_32):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_32; 
      break;
    case(PGA_GAIN_64):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_64; 
      break;
    case(PGA_GAIN_128):
      Config_Reg0 |= REG_CONFIG_PGA_GAIN_128; 
      break;
  }
  
  writeRegister(CONFIG_REG0_ADDRESS,Config_Reg0);
}

uint8_t * Protocentral_ADS1220::get_config_reg()
{
  static uint8_t config_Buff[4];

  Config_Reg0 = readRegister(CONFIG_REG0_ADDRESS);
  Config_Reg1 = readRegister(CONFIG_REG1_ADDRESS);
  Config_Reg2 = readRegister(CONFIG_REG2_ADDRESS);
  Config_Reg3 = readRegister(CONFIG_REG3_ADDRESS);

  config_Buff[0] = Config_Reg0 ; 
  config_Buff[1] = Config_Reg1 ;
  config_Buff[2] = Config_Reg2 ;
  config_Buff[3] = Config_Reg3 ;

  return config_Buff;
}


uint8_t * Protocentral_ADS1220::Read_Data()
{
  static byte SPI_Buff[3];

  if((digitalRead(ADS1220_DRDY_PIN)) == LOW)             //        Wait for DRDY to transition low
  {
  	digitalWrite(ADS1220_CS_PIN,LOW);                         //Take CS low
  	delayMicroseconds(100);
  	for (int i = 0; i < 3; i++)
  	{ 
  	  SPI_Buff[i] = SPI.transfer(SPI_MASTER_DUMMY);
  	}
  	delayMicroseconds(100);
  	digitalWrite(ADS1220_CS_PIN,HIGH);                  //  Clear CS to high
  	NewDataAvailable = true;
  }
  	
  return SPI_Buff;
}