#!/bin/bash

set -e

VIDEOS=$(ls ~/video-subset1/*.y4m)
RATES="1 3 5 7 11 16 25 37 55 81 122 181 270"

for video in ${VIDEOS}; do
    vname=$(basename ${video})
    vname=${vname/%_*/}
    for rate in ${RATES}; do
        export OD_EC_ACCT_SUFFIX="${vname}-${rate}"
        examples/encoder_example -v ${rate} ${video} -o /dev/null
        exit 1
    done
done
