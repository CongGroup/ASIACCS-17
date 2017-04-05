#!/bin/bash

if [ ! $# == 1 ]; then

echo "Usage : ./test2.sh [NodeNum]"
exit

fi

NODENUM=$1

CURTIME=`date +%s`
CURTIME=$(($CURTIME+2))

DURATION=100

KEYSIZE=16

RANDTIME=`date +%N`

echo $CURTIME
echo $RANDTIME

for i in {1..1}
do

echo $NODENUM $CURTIME $DURATION $KEYSIZE ${RANDTIME}

echo "./TestGet $NODENUM $CURTIME $DURATION $KEYSIZE ${RANDTIME}"
./TestGet $NODENUM $CURTIME $DURATION $KEYSIZE ${RANDTIME}

done