#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "project root path required"
    exit
fi
cd $1
PROJECT_ROOT=`pwd`
echo "We are in $PROJECT_ROOT"

PACKAGE=com.johnsoft.pcalldemo
ABI=arm64-v8a #armeabi-v7a
SO_NAME=libpcall.so
DEX_NAME=pcall.dex.jar
JAR_NAME=pcalla-1.0.0.jar

$PROJECT_ROOT/gradlew :pcalla:jar
dx --dex --output=$PROJECT_ROOT/pcalla/build/libs/$DEX_NAME $PROJECT_ROOT/pcalla/build/libs/$JAR_NAME

$PROJECT_ROOT/gradlew :pcall:externalNativeBuildDebug

adb push $PROJECT_ROOT/pcall/build/intermediates/cmake/debug/obj/$ABI/$SO_NAME /data/local/tmp
adb push $PROJECT_ROOT/pcalla/build/libs/$DEX_NAME /data/local/tmp
adb shell run-as $PACKAGE cp /data/local/tmp/$SO_NAME ./$SO_NAME
adb shell run-as $PACKAGE cp /data/local/tmp/$DEX_NAME ./$DEX_NAME
APP_DATA_PATH=`adb shell run-as $PACKAGE pwd`
adb shell am attach-agent $PACKAGE $APP_DATA_PATH/$SO_NAME
