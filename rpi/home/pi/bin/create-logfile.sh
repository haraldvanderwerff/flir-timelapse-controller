#!/bin/bash
#
# harald.vanderwerff@utwente.nl, May 2020

DIR='/mnt'
DEV='/dev/sda1'
FILE='/home/pi/data/log/system.log'

# make new log file if not existent yet
mkdir -p /home/pi/data/log
if [ ! -e $FILE ]; then
  echo -e "tgps,trtc,tsys,sync,volt,Tcore,log" > $FILE
fi

# get time variables first
_GPS=`timeout --preserve-status 5 /usr/bin/gpspipe -w -n 5 | grep TPV | tail +1`
_VAR=$(timedatectl)

# process the rest
_SYS=`echo "$_VAR" | head -2 | tail -1 | awk -F ":" '{print $2":"$3":"$4}'`
_RTC=`echo "$_VAR" | head -3 | tail -1 | awk -F ":" '{print $2":"$3":"$4" UTC"}'`
_GPS=`date +%a" "%Y-%m-%d" "%H:%M:%S" UTC" -d $(echo $_GPS | sed -r 's/.*"time":"([^"]*)".*/\1/')`
_SYN=$(timedatectl | grep NTP | awk -F':' '{print $2}')
_VOL=`/home/pi/bin/info-voltage.py | awk -F":" '{print $2}'`
_CPU=`/home/pi/bin/info-coretemp.sh | awk -F":" '{print $2}'`
_LOG=`cat /home/pi/data/log/acquisition.log`

echo -e $_GPS","$_RTC","$_SYS","$_SYN","$_VOL","$_CPU","$_LOG >> $FILE

[ -e $DEV ] && mount $DIR || exit 1

[ $? -ne 0 ] && exit 1

# prepare directory if needed
mkdir -p $DIR/log

# store logfile on the stick
cp -u $FILE $DIR/log/

umount $DIR

exit 0
