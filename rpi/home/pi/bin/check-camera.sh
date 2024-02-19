#!/bin/bash
#
# harald.vanderwerff@utwente.nl
#
# This script is executed when user 'pi' logs on
#

function camera_up {
  _VAR=$(/home/pi/bin/check-camera.exe 2>/dev/null | head -1 | cut -d' ' -f 1)
  if [ "$_VAR" = "Image" ]; then
    true
  else
    false
  fi
}

# wait for the camera to come up
until camera_up; do
  sleep 5
done

exit 0
