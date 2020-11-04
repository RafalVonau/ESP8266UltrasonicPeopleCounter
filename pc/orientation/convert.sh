#!/bin/bash

ls *.png | xargs -n 1 python2 ./bmptoc.py
cat 1.cpp > ./orientation.cpp
ls *.c | xargs -I % -n 1 cat % >> ./orientation.cpp
cat 2.cpp >> ./orientation.cpp
rm ./*.c