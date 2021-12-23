#!/bin/bash

SCRIPT_PATH=$(dirname "$(realpath "$0")")
export LD_LIBRARY_PATH=${SCRIPT_PATH}

# Workaround waiting for a fix https://github.com/lloyd/yajl/issues/222
export LC_NUMERIC=en_US.UTF-8

cd "${SCRIPT_PATH}"/..
rm -f /tmp/sam-massif.out
valgrind --tool=massif --massif-out-file=/tmp/sam-massif.out ${SCRIPT_PATH}/samrewritten $@
massif-visualizer /tmp/sam-massif.out
