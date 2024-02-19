#!/bin/bash
# harald.vanderwerff@utwente.nl

SRC='/dev/shm'
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

# store new data on USB stick
cp $SRC/*.tif $DIR/
cp $SRC/*.hdr $DIR/
cp $SRC/*.csv $DIR/metadata/
cp $SRC/*.jpg $DIR/quicklook/
cp -u /home/pi/log/boot.log $DIR/log/

umount $DIR

exit 0
