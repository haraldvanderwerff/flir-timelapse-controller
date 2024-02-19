#!/usr/bin/python3
#
# harald.vanderwerff@utwente.nl, May 2020

import serial
import time

device = serial.Serial('/dev/ttyS0',9600,timeout=2)
time.sleep(2)
device.flush()

try:
	v = device.readline()
	v = v[0:len(v)-2].decode("utf-8")
except:
	v = 'NA'

print("Voltage: ",v)
	
exit()
