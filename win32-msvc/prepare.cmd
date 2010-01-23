@echo off
:: Change the next lines to choose which gtk+ version you download.
:: Choose runtime version posterior to dev version
::    Get this file name from http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.16/
::    Specify the BUNDLE file
::    Don't include the extension
SET GTK_DEV_FILE=gtk+-bundle_2.16.5-20090731_win32
::    Get this file name from http://sourceforge.net/projects/gtk-win/files
::    Include the .exe extension
SET GTK_BIN_FILE=gtk2-runtime-2.16.5-2009-08-06-ash.exe

:: The rest of the script should do the rest

SET CURRENT_DIR=%CD%
IF NOT EXIST target MKDIR target
IF NOT EXIST downloads MKDIR downloads

:: Download and install the GTK runtime
cd %CURRENT_DIR%\downloads
wget -nc -c "http://sourceforge.net/projects/gtk-win/files/GTK+ Runtime Environment/GTK+ 2.16/%GTK_BIN_FILE%/download"
.\%GTK_BIN_FILE%

:: Download and unzip libxml2 dev and bin files
cd %CURRENT_DIR%\downloads
wget -nc -c ftp://ftp.zlatkovic.com/libxml/libxml2-2.7.6.win32.zip
cd %CURRENT_DIR%\target
unzip -uo %CURRENT_DIR%\downloads\libxml2-2.7.6.win32.zip -d plugins-dev
mv plugins-dev\libxml2-2.7.6.win32 plugins-dev\libxml2

:: Download and install openssl, copy the required files in the right place
cd %CURRENT_DIR%\downloads
wget -nc -c http://www.slproweb.com/download/Win32OpenSSL-0_9_8l.exe
ECHO ***** ATTENTION: Do not modify the default installation path for OpenSSL otherwise you will have trouble compiling *****
.\Win32OpenSSL-0_9_8l.exe
cd %CURRENT_DIR%\target
if not exist plugins-dev\openssl mkdir plugins-dev\openssl
if not exist plugins-dev\openssl\lib mkdir plugins-dev\openssl\lib
xcopy /Y C:\OpenSSL\lib\libeay32.lib plugins-dev\openssl\lib\.
xcopy /Y C:\OpenSSL\lib\ssleay32.lib plugins-dev\openssl\lib\.
IF NOT EXIST plugins-dev\openssl\include mkdir plugins-dev\openssl\include
xcopy /YICD C:\OpenSSL\include\openssl\* plugins-dev\openssl\include\openssl
xcopy /YICD C:\OpenSSL\*.dll plugins-dev\openssl\bin

:: Download and unzip libofx dev and bin files
cd %CURRENT_DIR%\downloads
wget -nc -c https://sourceforge.net/projects/grisbi/files/dependancies/0.6/libofx.zip/download
cd %CURRENT_DIR%\target
unzip -uo %CURRENT_DIR%\downloads\libofx.zip -d plugins-dev

cd %CURRENT_DIR%\downloads
wget -nc -c http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.16/%GTK_DEV_FILE%.zip
:: Checking if already unzipped
:: If the readme file exists for this version, update the unzip
cd %CURRENT_DIR%\target
IF EXIST gtk-dev\%GTK_DEV_FILE%.README.txt (
	echo Found gtk-dev directory with the same gtk+ version.
	echo Updating the files...
	unzip -uo %GTK_DEV_FILE% -d gtk-dev
	echo Done updating the files in gtk-dev!
)
:: If the readme file for the current version does not exist, remove the dir, and unzip again
IF NOT EXIST gtk-dev\%GTK_DEV_FILE%.README.txt (
	echo Found gtk-dev directory with a different gtk+ version.
	echo Deleting gtk-dev for compatibility reasons...
	rmdir /S /Q gtk-dev
	echo Done deleting gtk-dev directory!
	echo Unzipping downloaded archive...
	unzip %GTK_DEV_FILE% -d gtk-dev
	echo Done unzipping archive!
)
SET GTK_DEV_FILE=
cd %CURRENT_DIR%
call generate.cmd

PAUSE