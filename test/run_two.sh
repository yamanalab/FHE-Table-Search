#!/bin/bash

PWD=`pwd`
TOPDIR=${PWD}/..
BINDIR=${TOPDIR}/build/demo

RESFILE=${PWD}/res.txt

if [ $# -ne 3 ]; then
    echo "./run.sh <value_x> <value_y> <expected value>"
    exit 1
fi

IS_EXIST_DEC=`ps auxww | grep ./dec | grep -v grep`
IS_EXIST_CS=`ps auxww | grep ./cs  | grep -v grep`

if [ -z "${IS_EXIST_DEC}" ]; then
    echo "Start decryptor"
    (cd ${BINDIR}/dec  && ./dec 2>&1 > /dev/null &)
fi

if [ -z "${IS_EXIST_CS}" ]; then
    echo "Start computation server"
    (cd ${BINDIR}/cs && ./cs 2>&1 > /dev/null &)
fi

echo -n "Two input: x=$1, y=$2, exp=$3 ... "
(cd ${BINDIR}/user && ./user ${1} ${2} 1> /dev/null 2>${RESFILE})

RESULT=`cat ${RESFILE}`

TESTRES="NG"
if [ ${RESULT} = $3 ]; then
    TESTRES="OK"
fi

echo "res=${RESULT} => ${TESTRES}"

