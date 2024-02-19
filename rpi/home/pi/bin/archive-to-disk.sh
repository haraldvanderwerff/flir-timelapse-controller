#!/bin/bash
#
# harald.vanderwerff@utwente.nl
# May 2020

SRC='/dev/shm'
DIR='/home/pi/data'

mkdir -p $DIR/quicklook
mkdir -p $DIR/metadata

touch $DIR/quicklook
touch $DIR/metadata

# clean up oldest files if there is less than 10Mb disk space
DF=`df -k / --output=avail | tail -1`
[ $DF -le 10000 ] && find $DIR -type f -printf '%T+ %p\n' | sort | head -4 | awk '{print $2}' | xargs rm -v

# copy new data from ramdrive to disk
cp $SRC/*.tif $DIR
cp $SRC/*.hdr $DIR
cp $SRC/*.csv $DIR/metadata/
cp $SRC/*.jpg $DIR/quicklook/

exit 0
