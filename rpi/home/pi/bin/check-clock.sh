#!/bin/bash
#
# harald.vanderwerff@utwente.nl, May 2020
#

_VAR=$(timedatectl | grep NTP | awk -F':' '{print $2}')
if [ "$_VAR" = "yes" ]; then
  sudo hwclock -w
fi

exit 0
