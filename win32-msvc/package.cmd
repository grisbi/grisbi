@echo off
set DIR=target\zip
mkdir %DIR%

xcopy /Y Release\*.exe %DIR%

xcopy /S /Y ..\help %DIR%\help\
call tools\dos2unix.cmd ..\help\tips\tips.txt %DIR%\help\tips\tips.txt

xcopy /S /Y ..\pixmaps\*.png %DIR%\pixmaps\

cd %DIR%
zip -r ..\grisbi *
pause
