@echo off
if not exist gen mkdir gen
cd gen
if not exist vs14x64_static mkdir vs14x64_static
cd vs14x64_static
cmake ../../.. -DDYNAMIX_SHARED_LIB=0 -DDYNAMIX_BUILD_PERF=1 -G "Visual Studio 14 2015 Win64"
cd ..\..
