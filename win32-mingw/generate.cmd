@set PKG_CONFIG_PATH=%CD%\target\Win32\gtk-dev\lib\pkgconfig
@set PATH=%CD%\target\Win32\gtk-dev\bin;%PATH%
::subst c: "g:\"
::cmake --build . --target clean
rmdir Cmakefiles /s /q
del cmakecache.txt
del Makefile
cmake -G "MinGW Makefiles"
::pause
