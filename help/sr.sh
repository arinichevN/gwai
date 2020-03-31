#!/bin/bash
#
for (( n=0;n<1000;n++ ))
do
printf "<apr>g" | nc 127.0.0.1 49188
done
