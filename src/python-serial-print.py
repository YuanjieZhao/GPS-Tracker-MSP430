#!/usr/bin/python2.7
import serial # for serial port
import numpy as np # for arrays, numerical processing

#define the serial port. Pick one:
port = "/dev/ttyACM0"  #for Linux
#port = "COM5" #For Windows?
#port = "/dev/tty.uart-XXXX" #For Mac?

#start our program proper:
#open the serial port
try:
    # It seems that sometimes the port doesn't work unless 
    # you open it first with one speed, then change it to the correct value
    ser = serial.Serial(port,2400,timeout = 0.050) 
    ser.baudrate=4800
# with timeout=0, read returns immediately, even if no data
except:
    print ("Opening serial port",port,"failed")
    print ("Edit program to point to the correct port.")
    print ("Hit enter to exit")
    raw_input()
    quit()

ser.flushInput()

while(1): #loop forever
    data = ser.readline() # read until a end-of-line character
    if len(data) > 0: #was there a byte to read?
        print (data)
