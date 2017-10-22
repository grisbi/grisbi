;Grisbi version 1.1.1 pour windows
;
;Version 0.1
;5 mai 2017

;-----------------------------------
; Include Modern UI
;-----------------------------------
  !include "MUI2.nsh"

;-----------------------------------
; Generalités
;-----------------------------------

; Définition de la version du produit
;------------------------------------
!define PRODUCT "Grisbi" 					; Name of the project
!define MAJOR "1"        					; Major version number
!define MINOR "1"        					; Minor version number
!define PATCH "1"        					; Patch version number
!define STAGE "mingw64"						; Pour le fichier d'installation

;	Automatically defined parameters
;---------------------------------
!define BRANCH "${MAJOR}.${MINOR}.${PATCH}"
!define VER "${BRANCH}-${STAGE}"
!define FULLNAME "${PRODUCT}-${VER}"

; Définition des répertoires
;---------------------------
!define MSYS2 																	"C:\msys64"
!define GRISBI_SRC_DIR 													"${MSYS2}\home\Pierre\grisbi"
!define GRISBI_SRC_BIN_DIR 											"${MSYS2}\mingw64\bin"
!define GRISBI_SRC_LIB_GDK_PIXBUF_DIR						"${MSYS2}\mingw64\lib\gdk-pixbuf-2.0"
!define GRISBI_SRC_SHARE_DOC_DIR 								"${MSYS2}\mingw64\share\doc\grisbi"
!define GRISBI_SRC_SHARE_GLIB_SCHEMAS_DIR				"${MSYS2}\mingw64\share\glib-2.0\schemas"
!define GRISBI_SRC_SHARE_GRISBI_CATEGORIES_DIR	"${MSYS2}\mingw64\share\grisbi\categories"
!define GRISBI_SRC_SHARE_GRISBI_UI_DIR 					"${MSYS2}\mingw64\share\grisbi\ui"
!define GRISBI_SRC_SHARE_LOCALE_DIR 						"${MSYS2}\mingw64\share\locale"
!define GRISBI_SRC_SHARE_MAN_MAN1_DIR 					"${MSYS2}\mingw64\share\man\man1"
!define GRISBI_SRC_SHARE_MIME-INFO_DIR 					"${MSYS2}\mingw64\share\mime-info"
!define GRISBI_SRC_SHARE_MIME_PACKAGES_DIR 			"${MSYS2}\mingw64\share\mime\packages"
!define GRISBI_SRC_SHARE_PIXMAPS_DIR 						"${MSYS2}\mingw64\share\pixmaps\grisbi"

; Définition du fichier de l'installeur
;--------------------------------------
Name Name "${FULLNAME}"																		; Nom du programme à installer
!define MUI_ICON "${GRISBI_SRC_DIR}\Win32\grisbi.ico"
!define MUI_UNICON "${GRISBI_SRC_DIR}\Win32\grisbi.ico"
!define MUI_COMPONENTSPAGE_NODESC "true"

OutFile "${FULLNAME}.exe"																	; Nom du programme d'installation

InstallDir "$PROGRAMFILES64\${FULLNAME}"									; Répertoire d'installation par défaut

InstallDirRegKey HKLM "Software\${FULLNAME}" ""

; Demande des droits administrateur (impératif si Grisbi est installé dans Program Files)
;----------------------------------
RequestExecutionLevel admin

;-----------------------------------
; Interface Settings
;-----------------------------------

  !define MUI_ABORTWARNING

;-----------------------------------
; Language Selection Dialog Settings
;-----------------------------------

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
  !define MUI_LANGDLL_REGISTRY_KEY "Software\${FULLNAME}"
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;-----------------------------------
; Pages du programme d'installation
;-----------------------------------

; Page d'accueil
;---------------
!define MUI_WELCOMEPAGE_TITLE "Bienvenue dans le programme d'installation de grisbi version 1.1.1"
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT "Ce programme va installer automatiquement Grisbi."
!insertmacro MUI_PAGE_WELCOME

; Pages d'installation en cours
;------------------------------
  !insertmacro MUI_PAGE_LICENSE "${GRISBI_SRC_DIR}\COPYING"
	!insertmacro MUI_PAGE_INSTFILES 

; Page de fin
;------------
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_TITLE "Fin de la procédure d'installation de Grisbi"
!define MUI_FINISHPAGE_TEXT "Grisbi-1.1.1-mingw64 est installé sur votre ordinateur."
!define MUI_FINISHPAGE_RUN "$INSTDIR\bin\grisbi.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Exécuter Grisbi-1.1.1-mingw64"
!insertmacro MUI_PAGE_FINISH

;-----------------------------------
; Langue de l'installeur
;-----------------------------------

	!insertmacro MUI_LANGUAGE "French" 			; first language is the default language
  !insertmacro MUI_LANGUAGE "English"
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

;	Reserve Files
;--------------

  ;If you are using solid compression, files that are required before
  ;the actual installation should be stored first in the data block,
  ;because this will make your installer start faster.

  !insertmacro MUI_RESERVEFILE_LANGDLL


  !include "installer_strings.nsh"

;-----------------------------------
; Installation de Grisbi
;-----------------------------------

Section "binaires" bin

  SetOutPath "$INSTDIR\bin"
	
  ;fichiers à installer dans bin
  File /a /r "${GRISBI_SRC_BIN_DIR}\grisbi.exe"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libcairo-2.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libcroco-0.6-3.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgdk-3-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgdk_pixbuf-2.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgio-2.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libglib-2.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgobject-2.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgtk-3-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libintl-8.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libpango-1.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libpangocairo-1.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libxml2-2.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\zlib1.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgcc_s_seh-1.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libfontconfig-1.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libfreetype-6.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libpixman-1-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libpng16-16.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libcairo-gobject-2.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libepoxy-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgmodule-2.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libwinpthread-1.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libpcre-1.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libffi-6.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libatk-1.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libpangowin32-1.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libiconv-2.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libpangoft2-1.0-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\liblzma-5.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libexpat-1.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libbz2-1.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libharfbuzz-0.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libgraphite2.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\librsvg-2-2.dll"
	File /a /r "${GRISBI_SRC_BIN_DIR}\libstdc++-6.dll"
	File /a /r "${GRISBI_SRC_DIR}\Win32\grisbi.ico"
	
	
	;Store installation folder
  WriteRegStr HKCU "Software\${FULLNAME}" "" $INSTDIR

  ; File extension keys
  WriteRegStr HKCR ".gsb" "" "gsb_account_file"
  WriteRegExpandStr HKCR "gsb_account_file\Shell\open\command" "" "$INSTDIR\grisbi.exe %1"
  WriteRegExpandStr HKCR "gsb_account_file\DefaultIcon" "" "$INSTDIR\grisbi.ico"

  ;Create uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "DisplayName" "${FULLNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${FULLNAME}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "lib" lib

	;fichiers à installer dans lib\gdk-pixbuf-2.0
	SetOutPath "$INSTDIR\lib\gdk-pixbuf-2.0"
	File /a /r "${GRISBI_SRC_LIB_GDK_PIXBUF_DIR}\*.*"

SectionEnd


Section "Share" share

	;fichiers à installer dans share\doc\grisbi
	SetOutPath "$INSTDIR\share\doc\grisbi"
	File /a /r "${GRISBI_SRC_SHARE_DOC_DIR}\*.*"

	;fichiers à installer dans share\glib-2\schemas
	SetOutPath "$INSTDIR\share\glib-2.0\schemas"
	File /a /r "${GRISBI_SRC_SHARE_GLIB_SCHEMAS_DIR}\*.*"

	;fichiers à installer dans share\grisbi\categories
	SetOutPath "$INSTDIR\share\grisbi\categories"
	File /a /r "${GRISBI_SRC_SHARE_GRISBI_CATEGORIES_DIR}\*.*"

	;fichiers à installer dans share\grisbi\ui
	SetOutPath "$INSTDIR\share\grisbi\ui"
	File /a /r "${GRISBI_SRC_SHARE_GRISBI_UI_DIR}\*.*"
	
	;fichiers à installer dans share\locale
	SetOutPath "$INSTDIR\share\locale"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\cs"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\da"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\de"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\el"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\eo"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\es"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\fa"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\fr"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\he"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\it"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\lv"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\nl"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\pl"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\pt_BR"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\ro"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\ru"
  File /r "${GRISBI_SRC_SHARE_LOCALE_DIR}\zh_CN"

	;fichiers à installer dans share\man\man1
	SetOutPath "$INSTDIR\share\man\man1"
	File /a /r "${GRISBI_SRC_SHARE_MAN_MAN1_DIR}\grisbi.1"

	;fichiers à installer dans share\mime-info
	SetOutPath "$INSTDIR\share\mime-info"
	File /a /r "${GRISBI_SRC_SHARE_MIME-INFO_DIR}\grisbi.keys"
	File /a /r "${GRISBI_SRC_SHARE_MIME-INFO_DIR}\grisbi.mime"

	;fichiers à installer dans share\mime\packages
	SetOutPath "$INSTDIR\share\mime\packages"
	File /a /r "${GRISBI_SRC_SHARE_MIME_PACKAGES_DIR}\grisbi.xml"

	;fichiers à installer dans share\pixmaps\grisbi
	SetOutPath "$INSTDIR\share\pixmaps\grisbi"
	File /a /r "${GRISBI_SRC_SHARE_PIXMAPS_DIR}\*.*"
	
SectionEnd

Section $(Shortcut)

  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\Grisbi"
  CreateShortCut "$SMPROGRAMS\Grisbi\Uninstall ${FULLNAME}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Grisbi\${FULLNAME}.lnk" "$INSTDIR\bin\grisbi.exe" "" "$INSTDIR\bin\grisbi.ico" 0
  CreateShortCut "$DESKTOP\Grisbi.lnk" "$INSTDIR\bin\grisbi.exe" "" "$INSTDIR\bin\grisbi.ico" 0

SectionEnd

;Installer Functions
;-----------------------------------

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;-----------------------------------
; Désinstallation de Grisbi
;-----------------------------------

;Uninstaller Section
;-------------------

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r "$INSTDIR\bin"
  RMDir /r "$INSTDIR\lib"
	RMDir /r "$INSTDIR\share"
	RMDir /r /REBOOTOK "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\${FULLNAME}"
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

;Uninstaller Functions
;---------------------

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE

FunctionEnd
