#!/usr/bin/env python
print "test"

import spidev
import time
import sys
import RPi.GPIO as GPIO
from array import *


RREG =   0x20;
WREG  =  0x40;
START=0x08
STOP=0x0A
RDATAC=      0x10 

SDATAC	=	0x11	
RDATA	=	0x12


GPIO.setmode(GPIO.BOARD)


DRDY_PIN = 13;
START_PIN = 15;
CS_PIN = 18
RESET_PIN = 16


GPIO.setwarnings(False);

GPIO.setup(CS_PIN,GPIO.OUT)

GPIO.setup(DRDY_PIN,GPIO.IN)

spi = spidev.SpiDev()				# create spi object
spi.open(0,1)					# open spi port 0, device (CS)1
spi.max_speed_hz = (500000)
spi.mode = (1)

print "test"




count = 0

def Reg_Write(address,data):    # write Fibonacci series up to 
	print ("Writing %s to Register %s" %( hex(data),hex(address)))
	print(data);


	opcode1 = (address<<2) | 0x40

	GPIO.output(CS_PIN, False)
	time.sleep(0.002)               # sleep for 0.1 seconds
	GPIO.output(CS_PIN, True)
	time.sleep(0.002)               # sleep for 0.1 seconds

	GPIO.output(CS_PIN, False)
	time.sleep(0.002)               # sleep for 0.1 seconds
	resp = spi.xfer2([opcode1])        # transfer one byte
	resp = spi.xfer2([data])        # transfer one byte
	time.sleep(0.002)               # sleep for 0.1 seconds
	GPIO.output(CS_PIN, True)
	return;

def Reg_read(address):    # write Fibonacci series up to
	print ("Reading from Register %s" %(hex(address)))
        opcode1 = address | 0x20

        GPIO.output(CS_PIN, False)
        time.sleep(0.002)               # sleep for 0.1 seconds
        GPIO.output(CS_PIN, True)
        time.sleep(0.002)               # sleep for 0.1 seconds

        GPIO.output(CS_PIN, False)
        time.sleep(0.002)               # sleep for 0.1 seconds
        resp = spi.xfer2([opcode1])        # transfer one byte
        resp = spi.xfer2([0xff])        # transfer one byt
        time.sleep(0.002)               # sleep for 0.1 seconds
        GPIO.output(CS_PIN, True)

	#print("Data read is %s" %(hex(resp)))
	print(resp);
	return;

def Spi_command(command):    # write Fibonacci series up to
        GPIO.output(CS_PIN, False)
        time.sleep(0.002)               # sleep for 0.1 seconds
        GPIO.output(CS_PIN, True)
        time.sleep(0.002)               # sleep for 0.1 seconds

        GPIO.output(CS_PIN, False)
        time.sleep(0.002)               # sleep for 0.1 seconds
        resp = spi.xfer2([command])        # transfer one byte
        time.sleep(0.002)               # sleep for 0.1 seconds
        GPIO.output(CS_PIN, True)

        return;


def initialisation1():  
	print ("INITIALISATION");
	#GPIO.output(CS_PIN, False)

 

	Spi_command(START);				
	time.sleep(0.1);
	Spi_command(STOP);				
	time.sleep(0.1);

	Spi_command(SDATAC);				
	time.sleep(0.3);

	Reg_Write(0x00, 0x01); 		
	time.sleep(0.01); 
	Reg_Write(0x01, 0x04);	
	time.sleep(0.01); 
	Reg_Write(0x02, 0x10);		
	time.sleep(0.01); 
	Reg_Write(0x03, 0x00);	
	time.sleep(0.01); 
	
	Reg_read(0x00);
	time.sleep(0.1);
	Reg_read(0x04);
	time.sleep(0.1);
	Reg_read(0x08);
	time.sleep(0.1);
	Reg_read(0x0c);
	


        
	Spi_command(RDATAC);				
	time.sleep(0.3); 

	Spi_command(START);
	time.sleep(0.1);
	 

	GPIO.output(CS_PIN, True)
	return;

'''def Regread(arg1):
        # Add both the parameters and return them."
        total = arg1 + 67
        print "Inside the function : ", total
        return total;
'''
# Now you can call sum function
'''try:
        while True:
                total = sum( 10,20 );
                print "Outside the function : ", total
except KeyboardInterrupt:                       # Ctrl+C pressed, so...
        spi.close()                             # ... close the port before exit
'''
def toHex(dec):
	x = (dec % 16)
	digits = "0123456789ABCDEF"
	rest = dec / 16
	if (rest == 0):
		return digits[x]
	return toHex(rest) + digits[x]

def initialisation2():

	time.sleep(0.2)
	print "STARTED"

	#GPIO.output(CS_PIN, False)
	resp = spi.xfer2([0x11])        # transfer one byte
	time.sleep(0.3)	
	return;

def Read_Data():
	buff = [0,0,0,0,0,0,0,0,0];
	#buff=array('b',[0,0,0,0,0,0,0,0,0])
	#buff = [];
	GPIO.output(CS_PIN, False)
	#time.sleep(0.001)               # sleep for 0.1 seconds
	#for i in range(0,9):
	buff = spi.xfer2([0xff,0xff,0xff])
	'''buff2 = spi.xfer2([0xff])
	buff3 = spi.xfer2([0xff])'''
	#buff[i] = buffer[0]
	#print("%x" %buffer[0])	
	'''channel = 1
	r = spi.xfer2([1, (8+channel)<<4, 0])
	#r = spi.xfer2([0xff],[0xff],[0xff],[0xff],[0xff],[0xff],[0xff],[0xff],[0xff])
	r = spi.xfer2([0xff])
	#adcout = ((r[0]&3) << 8)
	#print [(x) for x in buff]'''

	#time.sleep(0.002)               # sleep for 0.1 seconds
	GPIO.output(CS_PIN, True)
	
	print("0x %x %x %x\t"%(buff[0],buff[1],buff[2]))
	value = buff[0]<<16 | buff[1]<<8 | buff[2]
	print(value)
	volt = (float(value) * .000244140625)
        print  volt,"mV"

	#print("0x %x %x %x\t"%(buff[0],buff[1],buff[2]))
	'''print int(buff[0])
	print int(buff[1])'''

	'''print int(buff2[0])
	print int(buff3[0])'''


	return buff;
	#return r

def Read_adcvoltage():
	result = Read_Data()
	return result
	'''val = ((result[5]&0xff) << 16) | ((result[6]&0xff) << 8) | (result[7]&0xff)
	if val > 0x7FFFFF:
		return (val - 0xFFFFFF)*6/8388608.0
	else:
		return ((result[5] << 16) | (result[6] << 8) | (result[7]))*6/8388608.0'''

	
voltage = 0

def ADCRead():
	#global voltage
	while(GPIO.input(DRDY_PIN) == True):
		time.sleep(0.001)		

	if (GPIO.input(DRDY_PIN) == False):
		#if 1:
		data = Read_adcvoltage();
		#val = ((data[5]&0xff) << 16) | ((data[6]&0xff) << 8) | (data[7]&0xff)
		val = ((data[6]&0xff) << 16) + ((data[7]&0xff) << 8) + (data[8]&0xff)
		#val = (data&0xff0000) + (data&0xff00) + (data&0xff)
		val = int(val)

		if( (val&(1<<(24-1))) != 0 ):
			value = val - 0x1000000			
		else:
			value = val
			
		voltage = (float(value)*100/8388607)
	
		#print("DRDY_PIN : %s\nvalue: %s"%((GPIO.input(DRDY_PIN)),value))
		#print "drdy false"
		#print("\t0x %x %x %x\tval: %x, %d\t2'scomp value: %s\tVoltage in mV: %s"%(data[6],data[7],data[8],val,val,value,voltage))
 		#print("Channel 2 value : %s"%(value))
		#time.sleep(.5)               # sleep for 0.1 seconds
		return voltage	
	'''#else:
		#print("DRDY_PIN : %s"%(GPIO.input(DRDY_PIN)))
		#print("DRDY_PIN : %s\nvalue: %s"%((GPIO.input(DRDY_PIN)),value))
		print "drdy true"
		return voltage'''
		#return 0

			
'''except KeyboardInterrupt:                       # Ctrl+C pressed, so...
        spi.close()                             # ... close the port before exit
        print("STOPPED")'''




def generator():
  global values,scale2
  #adc = ADS1x15()
  # Read channel 0 and 1 in single-ended mode (1 bit = 3mV)
  print "start"
  #rint("DRDY_PIN : %s"%(GPIO.input(DRDY_PIN)))

  for i in range(1200):
    #values.append(adc.readADCSingleEnded(0)*0.003)
    #print("DRDY_PIN : %s"%(GPIO.input(DRDY_PIN))
    values.append(ADCRead())
  root.after(int(scale2['to'])-scale2.get(),generator)

def plotter():
  global values,scale1,scale2
  NumberSamples=min(len(values),scale1.get())
  CurrentXAxis=pylab.arange(len(values)-NumberSamples,len(values),1)
  line1[0].set_data(CurrentXAxis,pylab.array(values[-NumberSamples:]))
  ax.axis([CurrentXAxis.min(),CurrentXAxis.max(),range_min,range_max])
  drawing.draw()
  root.after(25,plotter)


def _quit():
    root.quit()     # stops mainloop
    root.destroy()  # this is necessary on Windows to prevent
                    # Fatal Python Error: PyEval_RestoreThread: NULL tstate


initialisation1()
initialisation2()
Read_Data()

while True:

	Read_Data()
	time.sleep(0.5)
