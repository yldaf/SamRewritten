#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

CPP_FILES=$(find src -type f -iname *.cpp -print)

g++ -std=c++17 -g \
`pkg-config --cflags gtk+-3.0` \
-rdynamic -export-dynamic -pthread -Wall \
$CPP_FILES \
-L$SCRIPTPATH/bin \
-o $SCRIPTPATH/bin/samrewritten \
`pkg-config --libs gtk+-3.0` \
-lpthread -lgmodule-2.0 -lsteam_api -lcurl -lyajl -ldl

echo "If there wasn't any compilation error, you can launch the manager with ./bin/launch.sh"
