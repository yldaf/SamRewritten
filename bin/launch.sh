#!/bin/bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

export LD_LIBRARY_PATH=$SCRIPTPATH
echo "Library path is" $LD_LIBRARY_PATH
$SCRIPTPATH/samrewritten
