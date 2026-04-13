
#!/bin/bash

option=${1:-"build"}

if [ "$option" = "init" ]; then
    make clean
fi

bear -- make

# Build unique test binary for test 1
bear --append -- clang -std=c99 -finstrument-functions -g -gdwarf-4 avl.c -o avl_t1

