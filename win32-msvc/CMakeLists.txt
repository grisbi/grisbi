CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(POLICY CMP0015)
   # policies not known to CMake 2.6 ...
   cmake_policy(SET CMP0015 NEW)
endif(POLICY CMP0015)

PROJECT (Grisbi C)

# MS Visual Studio 10 Macros
# $(Platform)      The name of current project platform (for example, "Win32").
# $(Configuration) The name of the current project configuration (for example, "Debug").
# $(TargetDir)     The directory of the primary output file for the build (defined as drive + path); includes the trailing backslash '\'.
# see http://msdn.microsoft.com/en-us/library/c02as0cs%28v=VS.100%29.aspx for more macros

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY target\\$(Platform))
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY target\\$(Platform)\\$(Configuration)\\plugins)
SET(package_name grisbi)

ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ADD_DEFINITIONS(-D__PRETTY_FUNCTION__="")
ADD_DEFINITIONS(-D_WIN32)

# Define to 1 if translation of program messages to the user's native language is requested.
ADD_DEFINITIONS(-DENABLE_NLS=1)

# Define if you have the iconv() function.
ADD_DEFINITIONS(-DHAVE_ICONV=1)

# Define if you have <langinfo.h> and nl_langinfo(CODESET).
ADD_DEFINITIONS(-DHAVE_LANGINFO_CODESET=1)

# Define if your <locale.h> file defines LC_MESSAGES.
ADD_DEFINITIONS(-DHAVE_LC_MESSAGES=1)

# Define to 1 if you have the `intl' library (-lintl).
ADD_DEFINITIONS(-DHAVE_LIBINTL=1)

# Define to 1 if you have the <limits.h> header file.
ADD_DEFINITIONS(-DHAVE_LIMITS_H=1)

# Define to 1 if you have the <locale.h> header file.
ADD_DEFINITIONS(-DHAVE_LOCALE_H=1)

# Define to 1 if you have the <malloc.h> header file.
ADD_DEFINITIONS(-DHAVE_MALLOC_H=1)

# Define to 1 if you have the <memory.h> header file.
ADD_DEFINITIONS(-DHAVE_MEMORY_H=1)

# Define to 1 if you have the `mempcpy' function.
ADD_DEFINITIONS(-DHAVE_MEMPCPY=1)

# Define to 1 to have grisbi plugins management system activated
ADD_DEFINITIONS(-DHAVE_PLUGINS=1)

# Define to 1 if you have the `setlocale' function.
ADD_DEFINITIONS(-DHAVE_SETLOCALE=1)

# Define to 1 if you have the <string.h> header file.
ADD_DEFINITIONS(-DHAVE_STRING_H=1)

# Define to 1 to have OFX support disabled
#ADD_DEFINITIONS(-DNOOFX=1)
# Define to 1 if you want to have OFX support using libofx 0.7.0
ADD_DEFINITIONS(-DOFX_0_7=1)

# Define to 1 when you do not want to use SSL as crypt algorithm (NOT RECOMMANDED)
#ADD_DEFINITIONS(-DNOSSL=1)

INCLUDE_DIRECTORIES(
	target/$(Platform)/gtk-dev/include/atk-1.0
	target/$(Platform)/gtk-dev/include/cairo
	target/$(Platform)/gtk-dev/include/glib-2.0
	target/$(Platform)/gtk-dev/include/gdk-pixbuf-2.0
	target/$(Platform)/gtk-dev/include/gtk-2.0
	target/$(Platform)/gtk-dev/include/pango-1.0
	target/$(Platform)/gtk-dev/include
	target/$(Platform)/gtk-dev/lib/glib-2.0/include
	target/$(Platform)/gtk-dev/lib/gtk-2.0/include
	target/$(Platform)/plugins-dev/libofx/include
	target/$(Platform)/plugins-dev/libxml2/include
	target/$(Platform)/plugins-dev/iconv/include
	target/$(Platform)/plugins-dev/openssl/include
	../win32-msvc/src
	../win32-msvc
	../win32
	../src
	../src/plugins/gnucash
	../src/plugins/ofx
	../src/plugins/openssl
)

LINK_DIRECTORIES(
	target/$(Platform)/gtk-dev/lib
	target/$(Platform)/plugins-dev/libofx/lib
	target/$(Platform)/plugins-dev/libxml2/lib
	target/$(Platform)/plugins-dev/iconv/lib
	target/$(Platform)/plugins-dev/openssl/lib
	target/$(Platform)/plugins-dev/openssl/lib/VC
	$(TargetDir)
)

SET (grisbi_source
	../src/*.c
	../src/*.h
	../win32-msvc/src/getopt.c
	../win32-msvc/src/getopt.h
	../win32-msvc/src/getopt1.c
	../win32-msvc/src/realpath.c
	../win32-msvc/src/realpath.h
	../win32-msvc/src/resource.h
	../win32-msvc/src/grisbi.rc
	../win32-msvc/config.h
	../win32/win32gtkutils.c
	../win32/win32gtkutils.h
	../win32/win32utils.c
	../win32/win32utils.h
	# ../win32/grisbi.ico
)

ADD_EXECUTABLE(grisbi
	WIN32
	${grisbi_source}
)

TARGET_LINK_LIBRARIES( grisbi
	glib-2.0
	intl
	cairo
	gdk_pixbuf-2.0
	gdk-win32-2.0
	gio-2.0
	gmodule-2.0
	gobject-2.0
	gtk-win32-2.0
	pango-1.0
	pangocairo-1.0
	zdll
)

set_target_properties ( grisbi
	PROPERTIES ENABLE_EXPORTS 1
)

ADD_DEPENDENCIES (grisbi prepare_env prepare_mo)

ADD_LIBRARY ( ofx
	SHARED
	../src/plugins/ofx/ofx.c
	../src/plugins/ofx/ofx.h
)

ADD_CUSTOM_COMMAND ( TARGET ofx
	POST_BUILD
#	COMMAND XCOPY /D /Y $(Configuration)\\ofx.dll target\\$(Platform)\\$(Configuration)\\plugins\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\libofx\\bin\\* target\\$(Platform)\\$(Configuration)
	COMMAND IF NOT EXIST target\\$(Platform)\\$(Configuration)\\dtd MKDIR target\\$(Platform)\\$(Configuration)\\dtd
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\libofx\\bin\\dtd\\o*.d* target\\$(Platform)\\$(Configuration)\\dtd
)

ADD_LIBRARY ( gnucash
	SHARED
	../src/plugins/gnucash/gnucash.c
	../src/plugins/gnucash/gnucash.h
)

ADD_CUSTOM_COMMAND ( TARGET gnucash
	POST_BUILD
	COMMAND XCOPY /D /Y $(Configuration)\\gnucash.dll target\\$(Platform)\\$(Configuration)\\plugins\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\libxml2\\bin\\libxml2.dll target\\$(Platform)\\$(Configuration)\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\iconv\\bin\\iconv.dll target\\$(Platform)\\$(Configuration)\\
)

ADD_LIBRARY ( openssl
	SHARED
	../src/plugins/openssl/openssl.c
	../src/plugins/openssl/openssl.h
)

ADD_CUSTOM_COMMAND ( TARGET openssl
	POST_BUILD
#	COMMAND XCOPY /D /Y $(Configuration)\\openssl.dll target\\$(Platform)\\$(Configuration)\\plugins\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\openssl\\bin\\libeay32.dll target\\$(Platform)\\$(Configuration)
)


ADD_DEPENDENCIES (ofx grisbi)
TARGET_LINK_LIBRARIES (ofx
	glib-2.0
	intl
	grisbi
	libofx
)
ADD_DEPENDENCIES (gnucash grisbi)
TARGET_LINK_LIBRARIES (gnucash
	glib-2.0
	intl
	grisbi
	libxml2
	iconv
)
ADD_DEPENDENCIES (openssl grisbi)
TARGET_LINK_LIBRARIES (openssl
	glib-2.0
	intl
	grisbi
	gobject-2.0
	gtk-win32-2.0
	libeay32
	ssleay32
)

ADD_CUSTOM_TARGET (prepare_env)

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND IF NOT EXIST target\\$(Platform)\\$(Configuration) mkdir target\\$(Platform)\\$(Configuration)
)


# ADD_CUSTOM_COMMAND(TARGET prepare_env
	# POST_BUILD
	# COMMAND xcopy /Y /C /D target\\gtk-dev\\bin\\*.dll target\\Debug
	# COMMAND xcopy /Y /C /D target\\gtk-dev\\bin\\*.dll target\\Release
# )

# ADD_CUSTOM_COMMAND(TARGET prepare_env
	# POST_BUILD
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\etc target\\Debug\\etc\\
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\etc target\\Release\\etc\\
# )

# ADD_CUSTOM_COMMAND(TARGET prepare_env
	# POST_BUILD
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\lib\\gtk-2.0\\2.10.0 target\\Debug\\lib\\gtk-2.0\\2.10.0\\
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\lib\\gtk-2.0\\2.10.0 target\\Release\\lib\\gtk-2.0\\2.10.0\\
# )

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND xcopy /S /Y /D ..\\pixmaps\\*.png target\\$(Platform)\\$(Configuration)\\pixmaps\\
	# COMMAND xcopy /S /Y /D ..\\pixmaps\\*.png target\\Release\\pixmaps\\
)

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND xcopy /S /Y /D ..\\help target\\$(Platform)\\$(Configuration)\\help\\
	# COMMAND xcopy /S /Y /D ..\\help target\\Release\\help\\
)

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND xcopy /S /Y /D ..\\share\\categories target\\$(Platform)\\$(Configuration)\\help\\tips\\categories\\
	# COMMAND xcopy /S /Y /D ..\\share\\categories target\\Release\\help\\tips\\categories\\
)

#ADD_CUSTOM_COMMAND(TARGET prepare_env
#	POST_BUILD
#	COMMAND tools\\dos2unix.cmd ..\\help\\tips\\tips.txt target\\$(Platform)\\$(Configuration)\\help\\tips\\tips#.txt
	# COMMAND tools\\dos2unix.cmd ..\\help\\tips\\tips.txt target\\Release\\help\\tips\\tips.txt
#)

set(
  po_files
  ../po/cs.po
  ../po/da.po
  ../po/de.po
  ../po/el.po
  ../po/eo.po
  ../po/es.po
  ../po/fa.po
  ../po/fr.po
  ../po/he.po
  ../po/it.po
  ../po/lv.po
  ../po/nl.po
  ../po/pl.po
  ../po/pt_BR.po
  ../po/ro.po
  ../po/ru.po
  ../po/zh_CN.po
)

# set (
  # tips_po_files
  # ../help/tips/de.po
  # ../help/tips/es.po
  # ../help/tips/fr.po
# )

set (msgfmt target\\Win32\\gettext\\bin\\msgfmt.exe)

ADD_CUSTOM_TARGET(prepare_mo
	COMMAND IF NOT EXIST target\\lib mkdir target\\lib
	COMMAND IF NOT EXIST target\\lib\\locale mkdir target\\lib\\locale
)

foreach (current_po ${po_files})
  get_filename_component(
    lang
    ${current_po}
    NAME_WE
  )

  set(mo_file target\\lib\\locale\\${lang}\\LC_MESSAGES\\${package_name}.mo)

  add_custom_command(TARGET prepare_mo
    POST_BUILD
	COMMAND IF NOT EXIST target\\lib\\locale\\${lang} mkdir target\\lib\\locale\\${lang}
	COMMAND IF NOT EXIST target\\lib\\locale\\${lang}\\LC_MESSAGES mkdir target\\lib\\locale\\${lang}\\LC_MESSAGES
  )

  add_custom_command(TARGET prepare_mo
	POST_BUILD
    # OUTPUT ${mo_file}
    COMMAND ${msgfmt} -c ${current_po} -o ${mo_file}
    DEPENDS ${current_po}
  )
 endforeach (current_po)

 # foreach (current_po ${tips_po_files})
  # get_filename_component(
    # lang
    # ${current_po}
    # NAME_WE
  # )

  # set(mo_file target\\lib\\locale\\${lang}\\LC_MESSAGES\\${package_name}-tips.mo)

  # add_custom_command(TARGET prepare_mo
    # POST_BUILD
	# COMMAND IF NOT EXIST target\\lib\\locale\\${lang} mkdir target\\lib\\locale\\${lang}
	# COMMAND IF NOT EXIST target\\lib\\locale\\${lang}\\LC_MESSAGES mkdir target\\lib\\locale\\${lang}\\LC_MESSAGES
  # )

  # add_custom_command(TARGET prepare_mo
	# POST_BUILD
  ##  OUTPUT ${mo_file}
    # COMMAND ${msgfmt} -c ${current_po} -o ${mo_file}
    # DEPENDS ${current_po}
  # )
 # endforeach (current_po)

 add_custom_command(TARGET prepare_mo
	POST_BUILD
	COMMAND IF NOT EXIST target\\$(Platform)\\$(Configuration)\\lib mkdir target\\$(Platform)\\$(Configuration)\\lib
	COMMAND xcopy /S /Y /I /D target\\lib\\locale target\\$(Platform)\\$(Configuration)\\lib\\locale
	# COMMAND IF NOT EXIST target\\Release\\lib mkdir target\\Release\\lib
	# COMMAND xcopy /S /Y /I /D target\\lib\\locale target\\Release\\lib\\locale
)
