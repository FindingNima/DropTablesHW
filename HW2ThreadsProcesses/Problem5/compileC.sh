#!/bin/bash
gcc -c ${1}.c
gcc -o $1 ${1}.o -lpthread -lrt