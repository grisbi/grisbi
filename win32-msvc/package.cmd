@echo off
set DIR=target\zip
mkdir %DIR%

xcopy /Y target\Release\*.dll %DIR%
xcopy /Y target\Release\*.exe %DIR%
xcopy /S /Y target\Release\etc %DIR%\etc\
xcopy /S /Y target\Release\lib %DIR%\lib\

xcopy /S /Y ..\help %DIR%\help\
call tools\dos2unix.cmd ..\help\tips\tips.txt %DIR%\help\tips\tips.txt

xcopy /S /Y ..\pixmaps\*.png %DIR%\pixmaps\

cd %DIR%
zip -r ..\grisbi *
pause
