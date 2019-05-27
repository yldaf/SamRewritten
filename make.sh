#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

#export LD_LIBRARY_PATH=$SCRIPTPATH/bin

g++ -std=c++17 -g \
`pkg-config --cflags gtk+-3.0` \
-rdynamic -export-dynamic -pthread -Wall \
SAM.Picker/*.cpp \
common/*.cpp \
-L$SCRIPTPATH/bin \
-o $SCRIPTPATH/bin/samrewritten \
`pkg-config --libs gtk+-3.0` \
-lpthread -lgmodule-2.0 -lsteam_api -lcurl -lyajl

echo "If there wasn't any compilation error, you can launch the manager with ./bin/launch.sh"
