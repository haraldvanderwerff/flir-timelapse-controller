#!/bin/bash
#
# harald.vanderwerff@utwente.nl
# May 2020

clear
echo "FLIR image acquisition"
echo "----------------------"
echo

# copy any old data to stick for about 5 minutes
echo " -> Archiving old data"
timeout 300 /home/pi/bin/data-old.sh &

echo " -> Wait for 5 minutes..."
sleep 300

# acquire new data and copy to disk & stick
echo " -> Acquiring new data..."
timeout 240 /home/pi/bin/data-new.sh > /dev/null 2>&1

# write logfile and copy it to stick
echo " -> Writing a logfile..."
/home/pi/bin/create-logfile.sh > /dev/null 2>&1

# synchronize clock if the GPS has a lock
echo " -> Updating hardware clock..."
/home/pi/bin/check-clock.sh > /dev/null 2>&1

# shutdown after business
echo " -> Shutting down..."
sudo shutdown -h 1 &

exit 0
