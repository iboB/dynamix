@echo off
if not exist gen mkdir gen
cd gen
if not exist vs14x64_cc mkdir vs14x64_cc
cd vs14x64_cc
cmake ../../.. -DDYNAMIX_BUILD_PERF=0 -DDYNAMIX_BUILD_EXAMPLES=0 -DDYNAMIX_BUILD_TUTORIALS=0 -DDYNAMIX_BUILD_SCRATCH=0 -DDYNAMIX_CUSTOM_CONFIG_FILE="\"%cd%\..\..\..\test\custom_config\custom_config.hpp\"" -G "Visual Studio 14 2015 Win64"
cd ..\..
