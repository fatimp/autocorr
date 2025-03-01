#!/bin/sh

lua generate.lua
mv autocorr_* ../src
mv params.c ../src
mv generated.h ../src
