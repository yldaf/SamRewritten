#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

#export LD_LIBRARY_PATH=$SCRIPTPATH/bin

g++ -g `pkg-config --cflags gtk+-3.0` -rdynamic -export-dynamic -pthread -Wall SAM.Picker/*.cpp common/*.cpp -L$SCRIPTPATH/bin -o $SCRIPTPATH/bin/samrewritten `pkg-config --libs gtk+-3.0` -lpthread -lgmodule-2.0 -lsteam_api -lcurl \
&& \
#g++ -g SAM.Game/*.cpp common/functions.cpp common/lodepng.cpp -lsteam_api -L$SCRIPTPATH/bin -o $SCRIPTPATH/bin/samgame \
#&& \
./bin/launch.sh
