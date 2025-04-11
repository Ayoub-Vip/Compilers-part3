#!/bin/bash

make clean
make
clear

# for file in tests/*; do
    # echo "-----------------------" $file "------------------------------------"
    ./vsopc -c "tests/fact.vsop"
    echo "-------------------------------------------------------------------------------------"
# done