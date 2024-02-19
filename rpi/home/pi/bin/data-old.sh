#!/bin/bash
#
# harald.vanderwerff@utwente.nl
# May 2020

SRC='/home/pi/data'
DEV='/dev/sda1'
DIR='/mnt'

[ -e $DEV ] && mount $DIR || exit 1

[ $? -ne 0 ] && exit 1

mkdir -p $DIR/quicklook
mkdir -p $DIR/metadata
mkdir -p $DIR/log

# clean up oldest files if there is less than 100Mb disk space
DF=`df -k $DIR --output=avail | tail -1`
[ $DF -le 100000 ] && find $DIR -type f -printf '%T+ %p\n' | sort | head -4 | awk '{print $2}' | xargs rm -v

# copy data to USB stick
cp -ru $SRC/metadata  $DIR/
cp -ru $SRC/quicklook $DIR/
cp -ru $SRC/log       $DIR/
cp -ru $SRC/*.tif $DIR/
cp -ru $SRC/*.hdr $DIR/

umount $DIR

exit 0
