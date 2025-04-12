#!/bin/bash

make clean
make
clear

# for file in tests/*; do
    # echo "-----------------------" $file "------------------------------------"
    ./vsopc -c "tests/testListExample.vsop"
    echo "-------------------------------------------------------------------------------------"
# done