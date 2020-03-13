#!/bin/sh

#需要预先安装libcurl库，安装方法见README.md

GCC=gcc
MAIN_SOURCE=asrmain

$GCC --version && \
rm -rf $MAIN_SOURCE build && \
mkdir -p build && cd build && \
$GCC -g  -I../thirdparty/include -I..  -std=gnu99 ../token.c ../common.c ../$MAIN_SOURCE.c -L../thirdparty/lib -lcurl -o $MAIN_SOURCE && \
mv $MAIN_SOURCE .. && \
echo "build success....."
cd ..
cp $MAIN_SOURCE ../../../bin/
echo "cp bin success....."
