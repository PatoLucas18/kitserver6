@echo off
echo Setting kitserver compile environment
@call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\vcvars32.bat"
echo Environment set
nmake
@echo off

copy "C:\Users\Pato\Documents\GitHub\kitserver6\src\output\stadium.dll" "C:\Games\Pro Evolution Soccer 6\Kitserver\stadium.dll"
pause