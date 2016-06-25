@echo off
rmdir /s /q html
..\tools\c2md.rb -i ..\tutorial\basic.cpp -t 01_basic.dox.template
..\tools\c2md.rb -i ..\tutorial\messages.cpp ..\tutorial\mutation.cpp ..\tutorial\mutation_rules.cpp ..\tutorial\combinators.cpp -t 04_tutorials.dox.template
..\tools\c2md.rb -i ..\tutorial\allocators.cpp ..\tutorial\serialization.cpp -t 07_advanced.dox.template
doxygen
