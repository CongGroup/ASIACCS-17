#!/bin/bash

if [ ! $# == 1 ]; then

echo "Usage : ./test1.sh [NodeNum]"
exit

fi

NODENUM=$1

CURTIME=`date +%s`
CURTIME=$(($CURTIME+2))

DURATION=100

KEYSIZE=30

RANDTIME=`date +%N`
RANDTIME=$(($RANDTIME+$$))


echo $CURTIME

for i in {1..1}
do

echo $NODENUM $CURTIME $DURATION $KEYSIZE ${RANDTIME}0$i

echo "./Benchmark $NODENUM $CURTIME $DURATION $KEYSIZE ${RANDTIME}0$i"
./Benchmark $NODENUM $CURTIME $DURATION $KEYSIZE ${RANDTIME}0$i

done
