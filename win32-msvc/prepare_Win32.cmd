@echo off
:: Change the next lines to choose which gtk+ version you download.
:: Choose runtime version posterior to dev version
::    Get this file name from http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.18/
::    Specify the BUNDLE file
::    Don't include the extension
SET GTK_DEV_FILE_BASENAME=gtk+-bundle_2.22.0-20101016_win32

:: The rest of the script should do the rest

SET CURRENT_DIR=%CD%
IF NOT EXIST target MKDIR target
IF NOT EXIST target\Win32 MKDIR target\Win32
SET TARGET_DIR=%CURRENT_DIR%\target\Win32
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
SET LIBXML_FILE_BASENAME=libxml2-2.7.6.win32
cd %DOWNLOADS_DIR%
wget -nc -c ftp://ftp.zlatkovic.com/libxml/%LIBXML_FILE_BASENAME%.zip
cd %TARGET_DIR%
unzip -uo %DOWNLOADS_DIR%\%LIBXML_FILE_BASENAME%.zip -d plugins-dev
IF EXIST plugins-dev\libxml2 RMDIR /S /Q plugins-dev\libxml2
MOVE plugins-dev\%LIBXML_FILE_BASENAME% plugins-dev\libxml2
PAUSE

:: Download and unzip iconv dev and bin files
SET ICONV_FILE_BASENAME=iconv-1.9.2.win32
cd %DOWNLOADS_DIR%
wget -nc -c ftp://ftp.zlatkovic.com/libxml/%ICONV_FILE_BASENAME%.zip
cd %TARGET_DIR%
unzip -uo %DOWNLOADS_DIR%\%ICONV_FILE_BASENAME%.zip -d plugins-dev
IF EXIST plugins-dev\iconv RMDIR /S /Q plugins-dev\iconv
MOVE plugins-dev\%ICONV_FILE_BASENAME% plugins-dev\iconv
PAUSE

:: Download and install openssl, copy the required files in the right place
SET OPENSSL_FILE_BASENAME=Win32OpenSSL-1_0_0c
cd %DOWNLOADS_DIR%
wget -nc -c http://www.slproweb.com/download/%OPENSSL_FILE_BASENAME%.exe
IF NOT EXIST "C:\OpenSSL\readme.txt" (
	IF EXIST %OPENSSL_FILE_BASENAME%.exe (
		ECHO ***** ATTENTION: Do not modify the default installation path for OpenSSL otherwise you will have trouble compiling *****
		PAUSE
		START %OPENSSL_FILE_BASENAME%.exe
		EXIT /B 0) ELSE (
		ECHO OpenSSL package not downloaded. Please edit the script with correct filename and re-run.
		CD %CURRENT_DIR%
		EXIT /B 1) ) ELSE (
	ECHO OpenSSL already installed. Please check version.)
cd %TARGET_DIR%
if not exist plugins-dev\openssl mkdir plugins-dev\openssl
if not exist plugins-dev\openssl\lib mkdir plugins-dev\openssl\lib
xcopy /Y C:\OpenSSL\lib\libeay32.lib plugins-dev\openssl\lib\.
xcopy /Y C:\OpenSSL\lib\ssleay32.lib plugins-dev\openssl\lib\.
IF NOT EXIST plugins-dev\openssl\include mkdir plugins-dev\openssl\include
xcopy /YICD C:\OpenSSL\include\openssl\* plugins-dev\openssl\include\openssl
IF NOT EXIST plugins-dev\openssl\bin mkdir plugins-dev\openssl\bin
xcopy /YICD C:\OpenSSL\*.dll plugins-dev\openssl\bin

:: Download and unzip libofx dev and bin files
cd %DOWNLOADS_DIR%
wget --no-check-certificate -nc -c https://sourceforge.net/projects/grisbi/files/dependancies/0.6/libofx_Win32.zip/download
cd %TARGET_DIR%
unzip -uo %DOWNLOADS_DIR%\libofx_Win32.zip -d plugins-dev

:: Download and unzip gettext to get msgfmt.exe
cd %DOWNLOADS_DIR%
wget -nc -c http://sourceforge.net/projects/gnuwin32/files/gettext/0.14.4/gettext-0.14.4-bin.zip/download
cd %TARGET_DIR%
unzip -uo %DOWNLOADS_DIR%\gettext-0.14.4-bin.zip -d gettext

cd %DOWNLOADS_DIR%
wget -nc -c http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.22/%GTK_DEV_FILE_BASENAME%.zip
:: Checking if already unzipped
:: If the readme file exists for this version, update the unzip
cd %TARGET_DIR%
IF NOT EXIST gtk-dev (
	echo GTK+ development files not present.
	echo Unzipping the archive now...
	unzip %DOWNLOADS_DIR%\%GTK_DEV_FILE_BASENAME% -d gtk-dev
	echo Done unzipping archive!
)
IF EXIST gtk-dev\%GTK_DEV_FILE_BASENAME%.README.txt (
	echo Found gtk-dev directory with the same gtk+ version.
	echo Updating the files...
	unzip -uo %DOWNLOADS_DIR%\%GTK_DEV_FILE_BASENAME% -d gtk-dev
	echo Done updating the files in gtk-dev!
)
:: If the readme file for the current version does not exist, remove the dir, and unzip again
IF NOT EXIST gtk-dev\%GTK_DEV_FILE_BASENAME%.README.txt (
	echo Found gtk-dev directory with a different gtk+ version.
	echo Deleting gtk-dev for compatibility reasons...
	rmdir /S /Q gtk-dev
	echo Done deleting gtk-dev directory!
	echo Unzipping downloaded archive...
	unzip %DOWNLOADS_DIR%\%GTK_DEV_FILE_BASENAME% -d gtk-dev
	echo Done unzipping archive!
)
SET GTK_DEV_FILE_BASENAME=
cd %CURRENT_DIR%
call generate.cmd

PAUSE