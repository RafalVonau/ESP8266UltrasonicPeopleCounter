#!/bin/bash

ls *.png | xargs -n 1 python2 ./bmptoc.py
ls *.c | xargs -I % -n 1 cat % >> ./counter.cpp
rm ./*.c