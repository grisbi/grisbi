;NSIS Modern User Interface
;Multilingual Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  
;--------------------------------
;Include Env Vars modification function
;available at http://nsis.sourceforge.net/Environmental_Variables:_append,_prepend,_and_remove_entries

  ;!include "EnvVarUpdate.nsh" 
  
  
;-----------------------
;Defining useful parameters

!define PRODUCT "Grisbi" ;Name of the project
!define MAJOR "0"        ;Major version number
!define MINOR "6"        ;Minor version number
!define PATCH "0"        ;Patch version number
!define STAGE "RC 2"     ;Developement stage id (RC, beta, alpha)
!define SMALL_STAGE "rc2" ;Small dev stage id without spaces
!define EXE_PATH "win32-msvc\target\Win32\Release" ;Path to the exe file you want to pack
!define GNUWIN32_PATH "C:\Program Files (x86)\GnuWin32\bin" ;Path to the GnuWin32 root (must have libxml and openssl)
!define GTK_INSTALL_PATH "." ; Path to the GTK installer file to pack
!define GTK_INSTALL_FILE "gtk2-runtime-2.16.5-2009-08-06-ash.exe" ;Filename of the GTK installer

;Automatically defined parameters
!define BRANCH "${MAJOR}.${MINOR}.${PATCH}"
!define VER "${BRANCH} ${STAGE}"
!define SMALL_VER "${BRANCH}_${SMALL_STAGE}"
!define FULLNAME "${PRODUCT} ${VER}"
!define SHORTNAME "${PRODUCT}_${SMALL_VER}"

;--------------------------------
;General

  ;Name and file
  Name "${FULLNAME}"
  !define MUI_ICON "win32/grisbi.ico"
  !define MUI_UNICON "win32/grisbi.ico"
  !define MUI_COMPONENTSPAGE_NODESC "true"
  OutFile "${SHORTNAME}.exe"
  
  SetCompressor /SOLID lzma

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${FULLNAME}"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\${FULLNAME}" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKLM" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\${FULLNAME}" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

!ifndef NOINSTTYPES ; only if not defined
  InstType "Full (recommended)"
  InstType "Full (without GTK)"
  InstType "Grisbi only (no plugins)"
  ;InstType /NOCUSTOM
  ;InstType /COMPONENTSONLYONCUSTOM
!endif


;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" ;first language is the default language
  !insertmacro MUI_LANGUAGE "French" 
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"

;--------------------------------
;Reserve Files
  
  ;If you are using solid compression, files that are required before
  ;the actual installation should be stored first in the data block,
  ;because this will make your installer start faster.
  
  !insertmacro MUI_RESERVEFILE_LANGDLL
  
  
  !include "installer_strings.nsh"

;--------------------------------
;Installer Sections

Section $(ProgFiles)

  SectionIn 1 2 3 RO

  SetOutPath "$INSTDIR"
  
  File /r /x "plugin" /x "dtd" /x "*.dll" /x "Makefile.am" /x ".cvsignore" /x "CVS" /x "*.exp" /x "*.idb" /x "*.lib" /x "Release" "${EXE_PATH}\*.*"
  File "win32\grisbi.ico"
  
  ;Store installation folder
  WriteRegStr HKLM "Software\${FULLNAME}" "" $INSTDIR
  
  ; File extension keys
  WriteRegStr HKCR ".gsb" "" "gsb_account_file"
  WriteRegExpandStr HKCR "gsb_account_file\Shell\open\command" "" "$INSTDIR\grisbi.exe"
  WriteRegExpandStr HKCR "gsb_account_file\DefaultIcon" "" "$INSTDIR\grisbi.ico"
  
  ;Create uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "DisplayName" "${FULLNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section $(GTK)

  SectionIn 1
  
  SetOutPath "$INSTDIR"
  File /r "win32-msvc\target\gtk-dev\bin\*.dll" ;"${GTK_INSTALL_PATH}\${GTK_INSTALL_FILE}"
  
  SetOutPath "$INSTDIR\lib\gtk-2.0\2.10.0\engines"
  File "win32-msvc\target\gtk-dev\lib\gtk-2.0\2.10.0\engines\libwimp.dll"
  
  SetOutPath "$INSTDIR\share\locale"
  File /r "win32-msvc\target\gtk-dev\share\locale\cs"
  File /r "win32-msvc\target\gtk-dev\share\locale\da"
  File /r "win32-msvc\target\gtk-dev\share\locale\de"
  File /r "win32-msvc\target\gtk-dev\share\locale\el"
  File /r "win32-msvc\target\gtk-dev\share\locale\eo"
  File /r "win32-msvc\target\gtk-dev\share\locale\es"
  File /r "win32-msvc\target\gtk-dev\share\locale\fa"
  File /r "win32-msvc\target\gtk-dev\share\locale\fr"
  File /r "win32-msvc\target\gtk-dev\share\locale\he"
  File /r "win32-msvc\target\gtk-dev\share\locale\it"
  File /r "win32-msvc\target\gtk-dev\share\locale\nl"
  File /r "win32-msvc\target\gtk-dev\share\locale\pl"
  File /r "win32-msvc\target\gtk-dev\share\locale\pt_BR"
  File /r "win32-msvc\target\gtk-dev\share\locale\ro"
  File /r "win32-msvc\target\gtk-dev\share\locale\ru"
  File /r "win32-msvc\target\gtk-dev\share\locale\zh_CN"

SectionEnd

SectionGroup "Plugins"
Section $(OFXplug)

  SectionIn 1 2

  SetOutPath "$INSTDIR\plugins"
  File "${EXE_PATH}\plugins\ofx.dll"
  
  SetOutPath "$INSTDIR"
  File /r "${EXE_PATH}\dtd"
  File "${EXE_PATH}\osp151.dll"
  File "${EXE_PATH}\libofx.dll"

SectionEnd

Section $(GnuCashPlugin)

  SectionIn 1 2

  SetOutPath "$INSTDIR\plugins"
  File "${EXE_PATH}\plugins\gnucash.dll"
  
  SetOutPath "$INSTDIR"
  File "${GNUWIN32_PATH}\libxml2.dll"
  File "${GNUWIN32_PATH}\iconv.dll"

SectionEnd

Section $(OpenSSLplug)

  SectionIn 1 2

  SetOutPath "$INSTDIR\plugins"
  File "${EXE_PATH}\plugins\openssl.dll"
  
  SetOutPath "$INSTDIR"
  File "${GNUWIN32_PATH}\libeay32.dll"

SectionEnd
SectionGroupEnd

Section $(Shortcut)

  SectionIn 1 2 3

  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\Grisbi"
  CreateShortCut "$SMPROGRAMS\Grisbi\Uninstall ${FULLNAME}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Grisbi\${FULLNAME}.lnk" "$INSTDIR\grisbi.exe" "" "$INSTDIR\grisbi.ico" 0
  CreateShortCut "$DESKTOP\Grisbi.lnk" "$INSTDIR\grisbi.exe" "" "$INSTDIR\grisbi.ico" 0
  
SectionEnd

;--------------------------------
;Installer Functions

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;--------------------------------
;Descriptions

  ;USE A LANGUAGE STRING IF YOU WANT YOUR DESCRIPTIONS TO BE LANGAUGE SPECIFIC

  ;Assign descriptions to sections
  ;!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  ;  !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} "A test section."
  ;!insertmacro MUI_FUNCTION_DESCRIPTION_END

 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  RMDir /r /REBOOTOK $INSTDIR
  Delete "$INSTDIR\Uninstall.exe"

  DeleteRegKey /ifempty HKLM "Software\${FULLNAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}"
  
  ; removing file extension keys
  DeleteRegKey HKCR ".gsb"
  DeleteRegKey HKCR "gsb_account_file"
  
  ; Remove shortcuts, if any
  SetShellVarContext all
  Delete "$SMPROGRAMS\Grisbi\Uninstall ${FULLNAME}.lnk"
  Delete "$SMPROGRAMS\Grisbi\${FULLNAME}.lnk"
  Delete "$DESKTOP\Grisbi.lnk"
  RMDir "$SMPROGRAMS\Grisbi"

SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd
