@echo off
if not exist gen mkdir gen
cd gen
if not exist vs15x64 mkdir vs15x64
cd vs15x64
cmake ../../.. -DDYNAMIX_BUILD_PERF=1 -G "Visual Studio 15 2017 Win64"
cd ..\..
