#!/bin/bash
#
for (( n=0;n<1000;n++ ))
do
#printf "<?;24;1>g" | nc 127.0.0.1 49188
printf "<?;179;5>g" | nc 127.0.0.1 49188
done

