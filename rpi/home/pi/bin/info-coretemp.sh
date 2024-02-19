#!/bin/bash
#
# harald.vanderwerff@utwente.nl, May 2020

T=`vcgencmd measure_temp | egrep -o '[0-9]*\.[0-9]*'`
echo "Coretemp: $T C"
