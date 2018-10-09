#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

g++ -g \
SAM.Picker/*.cpp \
common/*.cpp \
-Wall -lsteam_api -L$SCRIPTPATH/bin \
-o $SCRIPTPATH/bin/samrewritten \
&& \
g++ -g \
SAM.Game/*.cpp \
common/*.cpp \
-lsteam_api -L$SCRIPTPATH/bin -o $SCRIPTPATH/bin/samgame
