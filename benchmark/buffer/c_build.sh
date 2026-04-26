#!/bin/bash

option=${1:-"build"}

if [ "$option" = "init" ]; then
    make clean
fi

bear -- make