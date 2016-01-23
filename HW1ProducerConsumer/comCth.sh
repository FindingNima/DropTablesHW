#!/bin/bash

gcc -c ${1}.c -lpthread
gcc -o $1 ${1}.o -lpthread
rm *.o
