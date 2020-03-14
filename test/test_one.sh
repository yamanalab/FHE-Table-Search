#!/bin/bash

while read row; do
    s1=`echo ${row} | fold -s1 | head -n1`
    if [ ${s1} != '#' ]; then
	x=`echo ${row} | cut -d , -f 1`
	ex=`echo ${row} | cut -d , -f 2`

	./run_one.sh ${x} ${ex}
    fi
done < test_one.csv
