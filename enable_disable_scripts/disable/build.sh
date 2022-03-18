#!/bin/bash

echo "building..."

g++ "disable.cpp" -o "disable.cgi" -lcgicc

sudo cp "disable.cgi" /usr/lib/cgi-bin/

sudo chmod +s "/usr/lib/cgi-bin/disable.cgi"

echo "done"
