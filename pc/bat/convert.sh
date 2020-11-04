#!/bin/bash

ls *.png | xargs -n 1 python2 ./bmptoc.py
cat 1.cpp > ./bat.cpp
ls *.c | xargs -I % -n 1 cat % >> ./bat.cpp
cat 2.cpp >> ./bat.cpp
rm ./*.c