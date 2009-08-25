@echo off
rem "C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
CALL "%VS90COMNTOOLS%\vsvars32.bat"
msbuild Project.sln /property:Configuration=Debug /maxcpucount
msbuild Project.sln /property:Configuration=Release /maxcpucount