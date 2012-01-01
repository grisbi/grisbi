CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
PROJECT (Grisbi C)
#pour que cmake 2.8 marche
if (POLICY CMP0015)
		cmake_policy(SET CMP0015 OLD)
endif (POLICY CMP0015)

if (WIN32)
	set (PlateformName "WIN32")
	ADD_DEFINITIONS(-D_WIN32)
endif(WIN32)

if (MINGW)
	set (TargetDir .)
	ADD_DEFINITIONS(-D_MINGW)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H)
	set(STATIC_SCRIPT_BINDINGS TRUE)
#	SET(BUILD_SHARED_LIBS ON)
#    set(CMAKE_VERBOSE_MAKEFILE ON) 
    set(CMAKE_BUILD_TYPE Debug)   
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   -mms-bitfields")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mms-bitfields")
    set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         -mms-bitfields")
    set(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS}       -mms-bitfields")
endif (MINGW)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/modules_cmakes)

INCLUDE (CheckIncludeFiles)
#on verifie que le programme existe
INCLUDE (${PROJECT_SOURCE_DIR}/modules_cmake/FindMsgfmt.cmake)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/target/${PlateformName})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/target/${PlateformName}/plugins)
SET(PACKAGE_NAME grisbi)
set (grisbi_MAJOR 0)
set (grisbi_MINOR 9)
set (grisbi_PATCH 5)
set (grisbi_version "${grisbi_MAJOR}.${grisbi_MINOR}.${grisbi_PATCH} pour windows")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH)
MARK_AS_ADVANCED(grisbi_version)

configure_file (config.h.cmake config.h)
SET (MINGW_DIR C:/MinGW)
SET (MYSYS_DIR ${MINGW_DIR}/msys/1.0)
INCLUDE_DIRECTORIES(
	${MINGW_DIR}/include
	${MINGW_DIR}/include/atk-1.0
	${MINGW_DIR}/include/cairo
	${MINGW_DIR}/include/gtk-2.0
	${MINGW_DIR}/lib/gtk-2.0/include
	${MINGW_DIR}/include/glib-2.0
	${MINGW_DIR}/lib/glib-2.0/include
	${MINGW_DIR}/include/pango-1.0
	${MINGW_DIR}/include/gdk-pixbuf-2.0
	${MYSYS_DIR}/local/include/
	${MYSYS_DIR}/local/include/libxml2/include
	${MYSYS_DIR}/local/include/libgoffice-0.8/
	${MINGW_DIR}/include/libgsf-1
	${PROJECT_SOURCE_DIR}/../win32
	${PROJECT_SOURCE_DIR}/../win32-msvc/src
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/../src
	${PROJECT_SOURCE_DIR}/../src/plugins/gnucash
	${PROJECT_SOURCE_DIR}/../src/plugins/ofx
	${PROJECT_SOURCE_DIR}/../src/plugins/openssl
	${PROJECT_BINARY_DIR}
)
LINK_DIRECTORIES(
#	${PROJECT_SOURCE_DIR}/target/${PlateformName}/plugins-dev/openssl/lib
#	${PROJECT_SOURCE_DIR}/target/${PlateformName}/plugins-dev/openssl/lib/VC
	${MYSYS_DIR}/local/lib
	${MYSYS_DIR}/local/bin
	${MINGW}/bin
	${PROJECT_SOURCE_DIR}
	${PROJECT_BINARY_DIR}
)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ADD_DEFINITIONS(-D__PRETTY_FUNCTION__="")

#version de dev
ADD_DEFINITIONS(-D_IS_DEVELOPMENT_VERSION=1)

# Define to 1 if translation of program messages to the user's native language is requested.
option (HELLO_NLS_ENABLED "Native Language Support" ON)

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
ADD_DEFINITIONS(-DHAVE_OFX=1)

# Define to 1 when you do not want to use SSL as crypt algorithm (NOT RECOMMANDED)
#ADD_DEFINITIONS(-DNOSSL=1)
ADD_DEFINITIONS(-DHAVE_SSL=1)

# Define to 1 to get goffice support
ADD_DEFINITIONS(-DHAVE_GOFFICE=1)

#Define to 1 to enable static plugins
ADD_DEFINITIONS(-DENABLE_STATIC=1)

file(GLOB grisbi_source
    ${PROJECT_SOURCE_DIR}/../src/*.c
	${PROJECT_SOURCE_DIR}/../win32/*.c
	${PROJECT_SOURCE_DIR}/../win32-msvc/src/*.c
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
	goffice-0.8-8
	ofx
	gnucash
	openssl
)

# set_target_properties ( grisbi
	# PROPERTIES ENABLE_EXPORTS 1
# )

ADD_LIBRARY ( ofx
	STATIC
	../src/plugins/ofx/ofx.c
	../src/plugins/ofx/ofx.h
)

ADD_CUSTOM_COMMAND ( TARGET ofx
	POST_BUILD
	#COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/${TargetDir}/ofx.dll ${PROJECT_BINARY_DIR}//target/${PlateformName}/plugins
	COMMAND ${CMAKE_COMMAND} -E copy ${MYSYS_DIR}/local/bin/libofx-5.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
	COMMAND ${CMAKE_COMMAND} -E copy ${MYSYS_DIR}/local/bin/libosp-5.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${MYSYS_DIR}/local/libofx/dtd ${PROJECT_BINARY_DIR}/target/${PlateformName}/dtd
)

ADD_LIBRARY ( gnucash
	STATIC
	../src/plugins/gnucash/gnucash.c
	../src/plugins/gnucash/gnucash.h
)

ADD_CUSTOM_COMMAND ( TARGET gnucash
	POST_BUILD
	#COMMAND ${CMAKE_COMMAND} -E copy  ${PROJECT_SOURCE_DIR}/${TargetDir}/gnucash.dll target\\${PlateformName}\\${OutDir}\\plugins
	COMMAND ${CMAKE_COMMAND} -E copy ${MYSYS_DIR}/local/bin/libxml2-2.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
	COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libiconv-2.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
)

ADD_LIBRARY ( openssl
	STATIC
	../src/plugins/openssl/openssl.c
	../src/plugins/openssl/openssl.h
)

ADD_CUSTOM_COMMAND ( TARGET openssl
	POST_BUILD
	#COMMAND ${CMAKE_COMMAND} -E copy  ${PROJECT_SOURCE_DIR}/target/openssl.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}/plugins
	COMMAND ${CMAKE_COMMAND} -E copy  ${MYSYS_DIR}/local/bin/libeay32.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
	COMMAND ${CMAKE_COMMAND} -E copy  ${MYSYS_DIR}/local/bin/ssleay32.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
)

# ADD_DEPENDENCIES (ofx grisbi)
#SET_TARGET_PROPERTIES(ofx PROPERTIES LINK_FLAGS "-no-undefined --enable-runtime-pseudo-reloc")
TARGET_LINK_LIBRARIES (ofx
	glib-2.0
	intl
	libofx-5
#    grisbi
)

# ADD_DEPENDENCIES (gnucash grisbi)
TARGET_LINK_LIBRARIES (gnucash
	glib-2.0
	intl
#	grisbi
	libxml2-2
	libiconv
)
# ADD_DEPENDENCIES (openssl grisbi)
# SET_TARGET_PROPERTIES(openssl PROPERTIES LINK_FLAGS -enable-auto-import)
TARGET_LINK_LIBRARIES (openssl
	glib-2.0
	intl
#	grisbi
	gobject-2.0
	gtk-win32-2.0
	libeay32
	ssleay32
)

ADD_DEPENDENCIES (grisbi prepare_env prepare_mo)

ADD_CUSTOM_TARGET (prepare_env COMMENT "preparation")

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../pixmaps ${PROJECT_BINARY_DIR}/target/${PlateformName}/pixmaps
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../help ${PROJECT_BINARY_DIR}/target/${PlateformName}/help
	COMMAND ${CMAKE_COMMAND} -E copy ${MYSYS_DIR}/local/bin/libgoffice-0.8-8.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/freetype6.dll ${PROJECT_BINARY_DIR}/target/${PlateformName} 
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/intl.dll ${PROJECT_BINARY_DIR}/target/${PlateformName}
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libatk-1.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}     
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libcairo-2.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}      
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libexpat-1.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}    
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libfontconfig-1.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgcc_s_dw2-1.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}   
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgdk-win32-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgdk_pixbuf-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgio-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}   
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libglib-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}    
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgmodule-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgobject-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgsf-1-114.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}    
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgthread-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libgtk-win32-2.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}   
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libiconv-2.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libpango-1.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}   
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libpangocairo-1.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libpangoft2-1.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libpangowin32-1.0-0.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libpng14-14.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}  
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/libstdc++-6.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName} 
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/pcre3.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}   
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/pthreadgc2.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName}    
    COMMAND ${CMAKE_COMMAND} -E copy ${MINGW_DIR}/bin/zlib1.dll  ${PROJECT_BINARY_DIR}/target/${PlateformName} 
)

if (HELLO_NLS_ENABLED)
	#on verifie que les headers sont la
	set (I18N_HEADERS locale.h libintl.h)
	foreach (HDR ${I18N_HEADERS})
		CHECK_INCLUDE_FILES (${HDR} HDR_RET)
		if (NOT ${HDR_RET})
			message (FATAL_ERROR "manque header.")
			ADD_DEFINITIONS(-DENABLE_NLS=0)
		endif (NOT ${HDR_RET})
	endforeach (HDR ${Hello_I18N_HEADERS})
	add_custom_target (prepare_mo COMMENT "Building i18n messages.")
	file(GLOB po_files ${PROJECT_SOURCE_DIR}/../po/*.po)
	add_custom_command (TARGET prepare_mo COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/target/lib/locale)

	foreach (current_po ${po_files})
		get_filename_component(
			lang
			${current_po}
			NAME_WE
		)
		set(mo_file ${PROJECT_BINARY_DIR}/target/${PlateformName}/lib/locale/${lang}/LC_MESSAGES/${PACKAGE_NAME}.mo)
		add_custom_command(TARGET prepare_mo
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/target/${PlateformName}/lib/locale/${lang}/LC_MESSAGES
			COMMAND ${MSGFMT_EXECUTABLE} -c ${current_po} -o ${mo_file}
			DEPENDS ${current_po}
		)
	 endforeach (current_po)
	ADD_DEFINITIONS(-DENABLE_NLS=1)
else(HELLO_NLS_ENABLED)
	ADD_DEFINITIONS(-DENABLE_NLS=0)
	message (STATUS "Native language support disabled.")
endif(HELLO_NLS_ENABLED)