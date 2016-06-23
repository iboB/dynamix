@echo off
if not exist gen mkdir gen
cd gen
if not exist vs14x64 mkdir vs14x64
cd vs14x64
cmake ../../.. -DDYNAMIX_BUILD_PERF=1 -G "Visual Studio 14 2015 Win64"
cd ..\..
