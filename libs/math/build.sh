#!/usr/bin/env bash

cc -c -Wall beryl_lib.c -I../../src

cc -shared beryl_lib.o -o../../env/libs/math.beryldl -lm
cp ./math.beryl ../../env/libs/math.beryl
