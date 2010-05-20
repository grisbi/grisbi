@echo off
:: Change the next lines to choose which gtk+ version you download.
:: Choose runtime version posterior to dev version
::    Get this file name from http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.18/
::    Specify the BUNDLE file
::    Don't include the extension
SET GTK_DEV_FILE=gtk+-bundle_2.18.7-20100213_win32
::    Get this file name from http://sourceforge.net/projects/gtk-win/files
::    Include the .exe extension
REM SET GTK_BIN_FILE=gtk2-runtime-2.16.5-2009-08-06-ash.exe

:: The rest of the script should do the rest

SET CURRENT_DIR=%CD%
IF NOT EXIST target MKDIR target
SET TARGET_DIR=%CURRENT_DIR%\target
IF NOT EXIST downloads MKDIR downloads
SET DOWNLOADS_DIR=%CURRENT_DIR%\downloads

echo Downloads directory : %DOWNLOADS_DIR%
echo Target directory : %TARGET_DIR%

REM PAUSE

:: Download and install the GTK runtime
REM cd %DOWNLOADS_DIR%
REM wget -nc -c "http://sourceforge.net/projects/gtk-win/files/GTK+ Runtime Environment/GTK+ 2.16/%GTK_BIN_FILE%/download"
REM .\%GTK_BIN_FILE%

:: Download and unzip libxml2 dev and bin files
cd %DOWNLOADS_DIR%
wget -nc -c ftp://ftp.zlatkovic.com/libxml/libxml2-2.7.6.win32.zip
cd %TARGET_DIR%
unzip -uo %DOWNLOADS_DIR%\libxml2-2.7.6.win32.zip -d plugins-dev
IF EXIST plugins-dev\libxml2 RMDIR /S /Q plugins-dev\libxml2
MOVE plugins-dev\libxml2-2.7.6.win32 plugins-dev\libxml2
PAUSE

:: Download and unzip iconv dev and bin files
cd %DOWNLOADS_DIR%
wget -nc -c ftp://ftp.zlatkovic.com/libxml/iconv-1.9.2.win32.zip
cd %TARGET_DIR%
unzip -uo %DOWNLOADS_DIR%\iconv-1.9.2.win32.zip -d plugins-dev
IF EXIST plugins-dev\iconv RMDIR /S /Q plugins-dev\iconv
MOVE plugins-dev\iconv-1.9.2.win32 plugins-dev\iconv
PAUSE

:: Download and install openssl, copy the required files in the right place
cd %DOWNLOADS_DIR%
wget -nc -c http://www.slproweb.com/download/Win32OpenSSL-0_9_8n.exe
ECHO ***** ATTENTION: Do not modify the default installation path for OpenSSL otherwise you will have trouble compiling *****
.\Win32OpenSSL-0_9_8n.exe
cd %TARGET_DIR%
if not exist plugins-dev\openssl mkdir plugins-dev\openssl
if not exist plugins-dev\openssl\lib mkdir plugins-dev\openssl\lib
xcopy /Y C:\OpenSSL\lib\libeay32.lib plugins-dev\openssl\lib\.
xcopy /Y C:\OpenSSL\lib\ssleay32.lib plugins-dev\openssl\lib\.
IF NOT EXIST plugins-dev\openssl\include mkdir plugins-dev\openssl\include
xcopy /YICD C:\OpenSSL\include\openssl\* plugins-dev\openssl\include\openssl
xcopy /YICD C:\OpenSSL\*.dll plugins-dev\openssl\bin

:: Download and unzip libofx dev and bin files
cd %DOWNLOADS_DIR%
wget -nc -c https://sourceforge.net/projects/grisbi/files/dependancies/0.6/libofx.zip/download
cd %TARGET_DIR%
unzip -uo %DOWNLOADS_DIR%\libofx.zip -d plugins-dev

cd %DOWNLOADS_DIR%
wget -nc -c http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.18/%GTK_DEV_FILE%.zip
:: Checking if already unzipped
:: If the readme file exists for this version, update the unzip
cd %TARGET_DIR%
IF NOT EXIST gtk-dev (
	echo GTK+ development files not present.
	echo Unzipping the archive now...
	unzip %DOWNLOADS_DIR%\%GTK_DEV_FILE% -d gtk-dev
	echo Done unzipping archive!
)
IF EXIST gtk-dev\%GTK_DEV_FILE%.README.txt (
	echo Found gtk-dev directory with the same gtk+ version.
	echo Updating the files...
	unzip -uo %DOWNLOADS_DIR%\%GTK_DEV_FILE% -d gtk-dev
	echo Done updating the files in gtk-dev!
)
:: If the readme file for the current version does not exist, remove the dir, and unzip again
IF NOT EXIST gtk-dev\%GTK_DEV_FILE%.README.txt (
	echo Found gtk-dev directory with a different gtk+ version.
	echo Deleting gtk-dev for compatibility reasons...
	rmdir /S /Q gtk-dev
	echo Done deleting gtk-dev directory!
	echo Unzipping downloaded archive...
	unzip %DOWNLOADS_DIR%\%GTK_DEV_FILE% -d gtk-dev
	echo Done unzipping archive!
)
SET GTK_DEV_FILE=
cd %CURRENT_DIR%
call generate.cmd

PAUSE