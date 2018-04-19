/*************************************************************
Protocentral_ADS1220.ino
this example gives differential voltage across the AN0 And AN1 in mV
Hooking-up with the Arduino
|ads1220 pin label| Pin Function         |Arduino Connection|
|-----------------|:--------------------:|-----------------:|
| DRDY            | Data ready Output pin|  D6              |             
| MISO            | Slave Out            |  D12             |
| MOSI            | Slave In             |  D11             |
| SCLK            | Serial Clock         |  D13             |
| CS              | Chip Select          |  D7              |
| DVDD            | Digital VDD          |  +5V             |
| DGND            | Digital Gnd          |  Gnd             |
| AN0-AN3         | Analog Input         |  Analog Input    |
| AVDD            | Analog VDD           |  -               |
| AGND            | Analog Gnd           |  -               |
*************************************************************/

#include "Protocentral_ADS1220.h"
#include <SPI.h>

#define PGA 1                 // Programmable Gain = 1
#define VREF 2.048            // Internal reference of 2.048V
#define VFSR VREF/PGA             
#define FSR (((long int)1<<23)-1)  

volatile byte MSB;
volatile byte data;
volatile byte LSB;
//volatile char SPI_RX_Buff[3];
volatile byte *SPI_RX_Buff_Ptr;

Protocentral_ADS1220 ADS1220;

void setup()
{
  pinMode(ADS1220_CS_PIN, OUTPUT);
  pinMode(ADS1220_DRDY_PIN, INPUT);
  
  ADS1220.begin();
}
 
void loop()
{
  long int bit32;
  long int bit24;
  byte *config_reg;
  
  //if((digitalRead(ADS1220_DRDY_PIN)) == LOW)             //        Wait for DRDY to transition low
  { 
    SPI_RX_Buff_Ptr = ADS1220.Read_Data();
  }

  if(ADS1220.NewDataAvailable = true)
  {
  ADS1220.NewDataAvailable = false;

  MSB = SPI_RX_Buff_Ptr[0];    
  data = SPI_RX_Buff_Ptr[1];
  LSB = SPI_RX_Buff_Ptr[2];

  bit24 = MSB;
  bit24 = (bit24 << 8) | data;
  bit24 = (bit24 << 8) | LSB;                                 // Converting 3 bytes to a 24 bit int
    
  /*if (MSB & 0x80)
    bit32 = (bit24 | 0xFF000000);             // Converting 24 bit two's complement to 32 bit two's complement
  else    
    bit32 = bit24;
  */
  
  bit24= ( bit24 << 8 );
  bit32 = ( bit24 >> 8 );                      // Converting 24 bit two's complement to 32 bit two's complement
  
  float Vout = (float)((bit32*VFSR*1000)/FSR);     //In  mV
  
  delay(300);
  
  Serial.print("Vout in mV : ");  
  Serial.print(Vout);
  Serial.print("  32bit HEX : ");
  Serial.println(bit32,HEX);
  
 /* 
  config_reg = ADS1220.get_config_reg();
 
  Serial.print("Config Reg : ");
  Serial.print(config_reg[0],HEX);
  Serial.print(" ");  
  Serial.print(config_reg[1],HEX);
  Serial.print(" ");
  Serial.print(config_reg[2],HEX);
  Serial.print(" ");
  Serial.println(config_reg[3],HEX);
  
  //ADS1220.set_data_rate(DR_1000SPS);
  //ADS1220.set_data_rate(DR_330SPS);
  //ADS1220.set_pga_gain(PGA_GAIN_32);
 
  Serial.print(MSB,HEX);
  Serial.print(data,HEX);
  Serial.print(LSB,HEX);
  
  Serial.print(vout);
  Serial.print("    ");
  Serial.print(bit32,HEX);
  */
  }
}
 
 
