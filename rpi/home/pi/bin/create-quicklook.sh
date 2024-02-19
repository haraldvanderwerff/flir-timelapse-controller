#!/bin/bash
#
# harald.vanderwerff@utwente.nl
# May 2020
#
# This scripts calls a python script for data acquisition
# and creates a quicklook image of the results.

# Convert 32-bit greyscale float .tiff
# to an 8-bit histogram equalized .jpg

DIR='/dev/shm/'
TIF=$(ls $DIR/*.tif | tail -1)
PNM=$DIR/$(basename $TIF .tif).pnm
JPG=$DIR/$(basename $TIF .tif).jpg
#
tifftopnm $TIF >> $PNM 
convert -equalize $PNM $JPG
#
rm $PNM

exit 0
