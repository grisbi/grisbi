@echo off
:: Change the next line to choose which gtk+ version you download
SET FILENAME=gtk+-bundle_2.16.5-20090731_win32
mkdir target
cd target
wget -nc -c http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.16/%FILENAME%.zip
:: Checking if already unzipped
:: If the readme file exists for this version, update the unzip
IF EXIST gtk-dev\%FILENAME%.README.txt (
	echo Found gtk-dev directory with the same gtk+ version.
	echo Updating the files...
	unzip -uo %FILENAME% -d gtk-dev
	echo Done updating the files in gtk-dev!
)
:: If the readme file for the current version does not exist, remove the dir, and unzip again
IF NOT EXIST gtk-dev\%FILENAME%.README.txt (
	echo Found gtk-dev directory with a different gtk+ version.
	echo Deleting gtk-dev for compatibility reasons...
	rmdir /S /Q gtk-dev
	echo Done deleting gtk-dev directory!
	echo Unzipping downloaded archive...
	unzip %FILENAME% -d gtk-dev
	echo Done unzipping archive!
)
SET FILENAME=
cd ..
cmake -G "Visual Studio 9 2008" .