/* ************************************************************************** */
/* quelques fonction donnant acces aux API windows                            */
/*                                                                            */
/*                                win32util.h                                 */
/*                                                                            */
/*     Copyright (C) 2004-     Francois Terrot (francois.terrot@grisbi.org)   */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#ifndef WINUTILS_H_C61461B7_ACF2_4011_888A_030AD5F25F8F
#define WINUTILS_H_C61461B7_ACF2_4011_888A_030AD5F25F8F

#include <windows.h>
#include <glib.h>

// -------------------------------------------------------------------------
// Windows(c) Usefull Functions                                       PART_1 
// -------------------------------------------------------------------------
extern gchar* win32_get_error_string(const guint win_error);
extern void   win32_set_last_error(guint);
extern guint  win32_get_last_error();

// -------------------------------------------------------------------------
// Windows(c) Special Paths ...                                       PART_2
// --------------------------------------------------------------------------
//! \def defined CSIDL_FLAG_CREATE if needed
#ifndef CSIDL_FLAG_CREATE
#define CSIDL_FLAG_CREATE 0x8000
#endif

typedef HRESULT (__stdcall * PFNSHGETFOLDERPATHA)(HWND, int, HANDLE, DWORD, LPSTR);  // "SHGetFolderPathA"
typedef HRESULT (__stdcall * PFNSHGETFOLDERPATHW)(HWND, int, HANDLE, DWORD, LPWSTR); // "SHGetFolderPathW"

#ifdef UNICODE
#define SHGetFolderPath     SHGetFolderPathW
#define SZ_SHGETFOLDERPATH  "SHGetFolderPathW"
#define PFNSHGETFOLDERPATH  PFNSHGETFOLDERPATHW
#else
#define SHGetFolderPath     SHGetFolderPathA
#define SZ_SHGETFOLDERPATH  "SHGetFolderPathA"
#define PFNSHGETFOLDERPATH  PFNSHGETFOLDERPATHA
#endif

extern HRESULT win32_get_app_data_folder_path     (gchar*, int);
extern gchar*  win32_get_my_documents_folder_path (void);
extern gchar*  win32_get_windows_folder_path      (void);
extern gchar*  win32_get_grisbirc_folder_path     (void);
extern void    win32_set_app_path                 (gchar*);
extern gchar*  win32_app_subdir_folder_path       (gchar*);
extern gchar*  win32_get_tmp_dir                  ();
extern void    win32_parse_gtkrc                  (const gchar* basename);

#define PIXMAPS_DIR win32_app_subdir_folder_path("pixmaps")  /* pixmaps */
#define HELP_PATH   win32_app_subdir_folder_path("help")     /* help */
#define LOCALEDIR   win32_app_subdir_folder_path("lib/locale")   /* locale */
#define LOGO_PATH   win32_app_subdir_folder_path("pixmaps/grisbi-logo.png")
#define DATA_PATH   win32_app_subdir_folder_path("share")
#define PLUGINS_DIR win32_app_subdir_folder_path("plugins")

// -------------------------------------------------------------------------
// Windows(c) Version ID and Technology                               PART_3 
//      Version ID is 95/98/NT/2K/...
//      Technology is 3.1/9x/NT/...
// -------------------------------------------------------------------------
// even values are supported values
// odd are not supported values
typedef enum
{
    WIN_UNKNOWN = 0x00,
    WIN31       = 0x01,
    // Win 9x
    WIN95  = 0x10,
    WIN98  = 0x12,
    WINME  = 0x14,
    // Win NTx
    WINNT3 = 0x21,
    WINNT4 = 0x22,
    WIN2K  = 0x24,
    WINXP  = 0x25,
    // Future ??
    WINNEXT  = 0xFF,
} win_version;

typedef enum
{   
    WIN_UNSUPPORTED = 0x01,
    WIN_9X          = 0x10,
    WIN_NT          = 0x20,
} win_technology;

extern win_version    win32_get_windows_version(void);
extern win_technology win32_get_windows_technology(win_version);

BOOL win32_shell_execute_open(const gchar* file);
BOOL win32_create_process(gchar* application_path,gchar* arg_line,gchar* utf8_working_dir,gboolean detach,gboolean with_sdterr);
// GetLongPathName
// -------------------------------------------------------------------------
typedef HRESULT (__stdcall * PFNSHGETGETLONGPATHNAMEA)(LPCTSTR, LPTSTR, DWORD);  // "GetLongPathNameA"
typedef HRESULT (__stdcall * PFNSHGETGETLONGPATHNAMEW)(LPCTSTR, LPTSTR, DWORD); // "GetLongPathNameW"

#ifndef _MSC_VER
#ifdef UNICODE
#define GetLongPathName     GetLongPathNameA
#define SZ_GETLONGPATHNAME  "GetLongPathNameA"
#define PFNGETLONGPATHNAME  PFNSHGETGETLONGPATHNAMEA
#else//UNICODE
#define GetLongPathName     GetLongPathNameW
#define SZ_GETLONGPATHNAME  "GetLongPathNameW"
#define PFNGETLONGPATHNAME  PFNSHGETGETLONGPATHNAMEW
#endif//UNICODE
#endif//_MSC_VER

//DWORD win32_get_long_path_name(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD ccBuffer);
gchar* win32_long_name(gchar*);
gchar* win32_full_path(gchar*);
void win32_set_current_directory(gchar* utf8_dir);
#endif//!WINUTILS_H_C61461B7_ACF2_4011_888A_030AD5F25F8F

