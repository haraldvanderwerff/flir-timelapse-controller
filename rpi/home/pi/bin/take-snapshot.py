#!/usr/bin/python
#
# harald.vanderwerff@utwente.nl, 2020
#
import sys
import os
import logging
import numpy
import time
import csv

from libtiff import TIFF
from aravis import Camera
from datetime import datetime

def disk_free():
	stats = os.statvfs("/")
	free = stats.f_bavail * stats.f_frsize
	free = float(free) / 1024. / 1024.
	return free

if __name__ == "__main__":

	# Define the timestamp for the recording
	stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
	stamp = "FLIR01-A655sc-"+stamp
	if len(sys.argv) > 1:
		prefix = sys.argv[1]
	else:
		prefix = os.path.expanduser('/dev/shm/'+stamp)
	
	# Open the camera
	try:
		camera = Camera ("FLIR Systems-PT1000ST-FLR1-")
	except:
		try:
			camera = Camera ()
		except:
			exit ()
	
        print
        print "Camera vendor : ", camera.get_vendor_name ()
	print "Camera model  : ", camera.get_model_name ()
        print
	
	#payload = camera.get_payload ()
	device = camera.get_device ()
	
	# maximum width and height
	camera.set_feature("Width", 640) 
        camera.set_feature("Height", 480)  
	# 3 Hz = hardware minimum, let's not stress the Pi.
        camera.set_frame_rate (3)          
	# default setting, I have no idea what it means
        camera.set_exposure_time(1000)     
        # we want single frames
        camera.set_feature("AcquisitionMode", "SingleFrame")
	#camera.set_feature("AcquisitionMode", "MultiFrame")
        # set temperature output
        camera.set_feature("IRFormat","TemperatureLinear100mK")
	# Set focus distance (2k=1000m; 5k=20m; 10k=2m; 20k=0.75m...)
	#camera.set_feature("FocusPos","2000")
	# Focussing for FLIR02 aka Gorge
	#camera.set_feature("FocusPos","3850") 
	# Focussing for FLIR01 aka Hell
	camera.set_feature("FocusPos","6900")
	time.sleep(1)
		
	# Gather some environmental stats
	stats = {}
	stats['FileName']               = prefix+'.tiff'
	stats['CameraVendor']           = camera.get_vendor_name ()
	stats['CameraModel']            = camera.get_model_name ()
	stats['AtmosphericTemperature'] = camera.get_feature ("AtmosphericTemperature")
	stats['EstimatedTransmission']  = camera.get_feature ("EstimatedTransmission")
	stats['ExtOpticsTemperature']   = camera.get_feature ("ExtOpticsTemperature")
	stats['ExtOpticsTransmission']  = camera.get_feature ("ExtOpticsTransmission")
	stats['FocusDistance']          = camera.get_feature ("FocusDistance")
	stats['ObjectDistance']         = camera.get_feature ("ObjectDistance")
	stats['ObjectEmissivity']       = camera.get_feature ("ObjectEmissivity")
	stats['ReflectedTemperature']   = camera.get_feature ("ReflectedTemperature")
	stats['RelativeHumidity']       = camera.get_feature ("RelativeHumidity")
	camera.set_feature ("TSensSelector","0")
	stats['TSensShutter']           = camera.get_feature ("TSens")
	camera.set_feature ("TSensSelector","1")
	stats['TSensLens']              = camera.get_feature ("TSens")
	camera.set_feature ("TSensSelector","2")
	stats['TSensFront']             = camera.get_feature ("TSens")
	stats['IRFormat']               = camera.get_feature ("IRFormat")
	stats['AcquisitionMode']        = camera.get_feature ("AcquisitionMode")
	stats['FocusDistance']          = camera.get_feature ("FocusDistance")
	stats['FrameRate']              = camera.get_frame_rate ()  
	stats['ExposureTime']           = camera.get_exposure_time ()  
		
	# Do a NUC before taking a shot
	device.execute_command("NUCAction")
	time.sleep(0.5)
	
	# Make an array for multiple images
	bands=33
	arr = numpy.zeros([480,640,bands],numpy.float32)
	
	# Acquire multiple frames
	for band in range(bands): 
		camera.start_acquisition()
		arr[:,:,band] = camera.pop_frame()
		camera.stop_acquisition()
	
	# get the median of frames
	arr[arr == 0] = numpy.nan
	frame=numpy.nanmean(arr,axis=2).astype('uint16')

        # acquire a single frame
        #frame = camera.pop_frame()
	
        # Save frame to tiff file
        tiff = TIFF.open(prefix+'.tif', mode='w')
        tiff.write_image(frame)
        tiff.close()
	
	# Save stats to csv file
	writer = csv.writer(open(prefix+'.csv','w'),delimiter=',')
	for key,value in stats.items():
		writer.writerow([key,value])

	f = open(prefix+'.hdr','w')
	f.write("ENVI\n")
	f.write("description = {FLIR Image}\n")
	f.write("samples = 640\n")
	f.write("lines   = 480\n")
	f.write("bands   = 1\n")
	f.write("header offset = 0\n")
	f.write("file type = TIFF\n")
	f.write("data type = 12\n")
	f.write("interleave = bsq\n")
	f.write("sensor type = Unknown\n")
	f.write("byte order = 0\n")
	f.write("wavelength units = Unknown\n")
	f.write("band names = {"+stamp+"}\n")
	f.close()
