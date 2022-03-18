#!/bin/bash

echo "building..."

g++ "enable.cpp" -o "enable.cgi" -lcgicc

sudo cp "enable.cgi" /usr/lib/cgi-bin/

sudo chmod +s "/usr/lib/cgi-bin/enable.cgi"

echo "done"
