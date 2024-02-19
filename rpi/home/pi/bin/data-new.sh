#!/bin/bash
#
# harald.vanderwerff@utwente.nl
# May 2020

# keep a log of stages reached
STAGE='1000000'

# check if network is up
timeout 30 /home/pi/bin/check-network.sh > /dev/null 2>&1
[[ $? -eq 0 ]] && STAGE=$((STAGE+200000)) #|| exit 1

# check if camera is up
timeout 30 /home/pi/bin/check-camera.sh > /dev/null 2>&1
[[ $? -eq 0 ]] && STAGE=$((STAGE+30000)) #|| exit 1

# call acquisition script
/home/pi/bin/take-snapshot.py > /dev/null 2>&1
[[ $? -eq 0 ]] && STAGE=$((STAGE+4000)) #|| exit 1

# call quicklook function
/home/pi/bin/create-quicklook.sh > /dev/null 2>&1
[[ $? -eq 0 ]] && STAGE=$((STAGE+500))

# archive results to disk
/home/pi/bin/archive-to-disk.sh > /dev/null 2>&1
[[ $? -eq 0 ]] && STAGE=$((STAGE+60))

# archive results to stick
/home/pi/bin/archive-to-stick.sh > /dev/null 2>&1
[[ $? -eq 0 ]] && STAGE=$((STAGE+7)) #|| exit 1

# cleanup after ourselves
rm -r /dev/shm/*

echo $STAGE > /home/pi/data/log/acquisition.log

exit 0
