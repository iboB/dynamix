@echo off
if not exist gen mkdir gen
cd gen
if not exist vs12x64 mkdir vs12x64
cd vs12x64
cmake ../../.. -DDYNAMIX_BUILD_PERF=1 -G "Visual Studio 12 2013 Win64"
cd ..\..
