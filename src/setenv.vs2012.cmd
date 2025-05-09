@echo off
echo Setting kitserver compile environment
@call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\vcvars32.bat"
echo Environment set
nmake
@echo off
pause