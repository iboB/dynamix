@echo off
rem ~ generates a "lite" vs14 project with the scratch executable only
rem ~ used to iterate on library features faster without having to rebuild the
rem ~ whole smorgasboard of test/tutorial/example executables
if not exist gen mkdir gen
cd gen
if not exist vs14x64_lite mkdir vs14x64_lite
cd vs14x64_lite
cmake ../../.. -DDYNAMIX_BUILD_PERF=0 -DDYNAMIX_BUILD_UNIT_TESTS=0 -DDYNAMIX_BUILD_EXAMPLES=0 -DDYNAMIX_BUILD_TUTORIALS=0 -G "Visual Studio 14 2015 Win64"
cd ..\..
