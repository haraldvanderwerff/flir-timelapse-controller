#!/bin/bash
#
# harald.vanderwerff@utwente.nl
#
# This script is executed when user 'pi' logs on
#

function network_up {
  _VAR=$(cat /sys/class/net/eth0/operstate)
  if [ $_VAR = "up" ]; then
    true
  else
    false
  fi
}

# wait for the network to come up
until network_up; do
  sleep 5
done

exit 0
