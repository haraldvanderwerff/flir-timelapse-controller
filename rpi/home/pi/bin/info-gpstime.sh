#!/bin/bash
#
# harald.vanderwerff@utwente.nl
#

_GPSPIPE=`timeout --preserve-status 5 /usr/bin/gpspipe -w -n 5 | grep TPV | tail +1`
_GPSMODE=`echo $_GPSPIPE | sed -r 's/.*"mode":([^"]*),.*/\1/'`
_GPSDATE=`echo $_GPSPIPE | sed -r 's/.*"time":"([^"]*)".*/\1/'`

echo "GPS time: "$_GPSDATE
