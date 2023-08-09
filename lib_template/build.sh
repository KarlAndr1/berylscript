#!/usr/bin/env bash

cc -c -Wall beryl_lib.c -I../src
cc -shared beryl_lib.o -oberyl_lib.so.beryl
