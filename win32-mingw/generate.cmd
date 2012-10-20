@set PKG_CONFIG_PATH=C:\MinGW\lib\pkgconfig
::subst c: "g:\"
::cmake --build . --target clean
rmdir Cmakefiles /s /q
del cmakecache.txt
del Makefile
cmake -G "MinGW Makefiles"
::pause
