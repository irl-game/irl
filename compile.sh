#!/bin/bash

for bin in server client test; do
    cd $bin
    echo Entering directory \`$bin\'
    coddle debug
    echo Leaving directory \`$bin\'
    cd ..
done
