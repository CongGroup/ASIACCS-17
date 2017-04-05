#!/bin/bash

if [ ! $# == 7 ]; then

echo "Usage : ./Test_Throughput.sh [NodeNum] [LOOP] [OPTION] [TIME] [MaxBoundary] [SEEDS] [BLOCK_SIZE_INBITS]"
echo "[OPTION] => 0 means EqualV1, 1 means EqualV2, 2 means OrderQuery"
echo `date +%s --date "20 seconds"`
exit

fi


NODENUM=$1
LOOP=$2
OPTION=$3
CURTIME=$4
MaxBoundary=$5
SEEDS=$6
BLOCK_SIZE_INBITS=$7


DURATION=30

RANDTIME=`date +%N`
RANDTIME=${RANDTIME:0:5}$SEEDS


get_char()
{
        SAVEDSTTY=`stty -g`
        stty -echo
        stty raw
        dd if=/dev/tty bs=1 count=1 2> /dev/null
        stty -raw
        stty echo
        stty $SAVEDSTTY
}

rm -f Output_throughput

for i in $(seq 1 ${LOOP})
do

echo "./Test_Throughput $NODENUM $CURTIME $DURATION $OPTION ${RANDTIME}$i ${MaxBoundary} ${BLOCK_SIZE_INBITS}"
./Test_Throughput $NODENUM $CURTIME $DURATION $OPTION ${RANDTIME}$i ${MaxBoundary} ${BLOCK_SIZE_INBITS} >> Output_throughput &

done

echo "Press any key to continue..."
char=`get_char`

awk 'BEGIN{total=0}{total+=$1}END{print total}' Output_throughput



