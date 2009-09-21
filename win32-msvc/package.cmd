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

for %%f in (CS DA DE EL EO ES FA FR HE IT NL PL PT_BR RO RU ZH_CN) do (
	mkdir target\zip\lib\locale\%%f\LC_MESSAGES
	target\gtk-dev\bin\msgfmt.exe ..\po\%%f.po --output-file=target\zip\lib\locale\%%f\LC_MESSAGES\grisbi.mo
)

cd %DIR%
zip -r ..\grisbi *
pause
