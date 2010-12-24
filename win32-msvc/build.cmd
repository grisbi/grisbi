@echo off
rem "C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
CALL "%VS100COMNTOOLS%\vsvars32.bat"
msbuild Grisbi.sln /property:Configuration=Debug /maxcpucount
msbuild Grisbi.sln /property:Configuration=Release /maxcpucount