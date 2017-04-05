#!/bin/bash

if [ ! $# == 4 ]; then

echo "Usage : ./test1.sh [NodeNum] [EqualIndexVersion] [OrderIndex] [BlockSizeInBit]"
echo "The EqualIndexVersion: 0 means NO index, 1 means version 1 equal index, 2 means version 2 equal index."
echo "The OrderIndex: 0 means NO index, 1 means Order Index."
echo "The Block_size in bit is [BlockSizeInBit]."

exit

fi

NODENUM=$1
EQUALINDEXVERSION=$2
ORDERINDEX=$3
BLOCKSIZEINBIT=$4

./Test_insert ${NODENUM} 1 1000 ${EQUALINDEXVERSION} ${ORDERINDEX} ${BLOCKSIZEINBIT}
./Test_insert ${NODENUM} 1 2000 ${EQUALINDEXVERSION} ${ORDERINDEX} ${BLOCKSIZEINBIT}
./Test_insert ${NODENUM} 1 4000 ${EQUALINDEXVERSION} ${ORDERINDEX} ${BLOCKSIZEINBIT}
./Test_insert ${NODENUM} 1 8000 ${EQUALINDEXVERSION} ${ORDERINDEX} ${BLOCKSIZEINBIT}
./Test_insert ${NODENUM} 1 16000 ${EQUALINDEXVERSION} ${ORDERINDEX} ${BLOCKSIZEINBIT}
