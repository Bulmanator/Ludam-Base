#!/bin/sh

COMPILER_FLAGS="-fPIC -fno-strict-aliasing -Wall -Wno-unused-function -I $(dirname $0)/.."
LINKER_FLAGS=-lGL

if [[ "$1" == "release" ]];
then
    g++ -Werror -O2 -shared $COMPILER_FLAGS "$(dirname $0)/linux_glx.cpp" -o "renderer_glx.so" $LINKER_FLAGS
else
    g++ -O0 -g -ggdb -shared $COMPILER_FLAGS "$(dirname $0)/linux_glx.cpp" -o "renderer_glx.so" $LINKER_FLAGS
fi
