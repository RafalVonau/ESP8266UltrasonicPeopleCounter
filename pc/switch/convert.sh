#!/bin/bash

ls *.png | xargs -n 1 python2 ./bmptoc.py
cat 1.cpp > ./switch.cpp
ls *.c | xargs -I % -n 1 cat % >> ./switch.cpp
cat 2.cpp >> ./switch.cpp
rm ./*.c