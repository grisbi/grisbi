/* ************************************************************************** */
/* quelques fonction donnant acces aux API windows                            */
/*                                                                            */
/*                                win32util.c                                 */
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

#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <shlobj.h>

#include "win32utils.h"

// -------------------------------------------------------------------------
// Windows(c) Usefull Functions                                  PART_1 {{{1
// -------------------------------------------------------------------------
/**
 * give access to the SetLastError windows function
 *
 * \param win_error error code to store
 */
void win32_set_last_error(guint win_error) /* {{{ */
{
    SetLastError((DWORD)(win_error));
} /* }}} */

/** 
 * give access without type warning to the GetLastError windows function
 *
 * \return the last error code stored using win32_set_last_error
 */
guint win32_get_last_error(void) /* {{{ */
{
    return (guint)(GetLastError());
} /* }}} */
/**
 * 
 * Retrieve String translation of an Windows error number
 * 
 *   ! The returned string MUST be unallocated after used
 *
 * \param win_error Windows error code
 * 
 * \return a string buffer containing the string
 */
gchar* win32_get_error_string(const guint win_error)                        /* {{{ */
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    (DWORD)(win_error),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), // Default language
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL);

    return (gchar*)(lpMsgBuf);
} /* }}} */

/**
 * Encapsulation of the windows LocalFree function to be used for buffer 
 *    allocaed by the LocalMalloc function
 * 
 * \param ptr ptr to free
 *
 */
void win32_free(void* ptr) /* {{{ */
{
    LocalFree(ptr);
} /* }}} */

// }}}1
// -------------------------------------------------------------------------
// Windows(c) Special Paths ...                                  {{{1 PART_2
// --------------------------------------------------------------------------
static gchar my_documents_path [MAX_PATH+1];
static gchar windows_path      [MAX_PATH+1]; 
static gchar grisbirc_path     [MAX_PATH+1];
static gchar grisbi_exe_path   [MAX_PATH+1];
static HMODULE            hSHFolder        = NULL;
static PFNSHGETFOLDERPATH pSHGetFolderPath = NULL;
/**
 * Retrieve the SHGetFolder function pointer from the good dll depending
 *   of the OS
 *   W2K/XP => Shell32
 *  older   => SHFolder.dll
 */
static PFNSHGETFOLDERPATH _win32_getfolderpath() /* {{{ */
{
    if (pSHGetFolderPath == NULL) 
    {
        // First try fom shell32.dll
        if (hSHFolder) { FreeLibrary(hSHFolder); }
        hSHFolder = LoadLibrary("shell32.dll");
        if (hSHFolder)
        {
            pSHGetFolderPath = (PFNSHGETFOLDERPATH)GetProcAddress(hSHFolder,SZ_SHGETFOLDERPATH);
        }
    }
    if (pSHGetFolderPath == NULL) 
    {
        // Next try fom shfolder.dll
        if (hSHFolder) { FreeLibrary(hSHFolder); }
        hSHFolder = LoadLibrary("ShFolder.dll");
        if (hSHFolder)
        {
            pSHGetFolderPath = (PFNSHGETFOLDERPATH)GetProcAddress(hSHFolder,SZ_SHGETFOLDERPATH);
        }

    }

    return pSHGetFolderPath;
} /* }}} */
/** 
 * Retrieve the absolute path of a CSIDL directory named
 *
 * \param folder_path  already allocated buffer to retrieve the result
 * \param csidl        Window ID of the directory (csidl)
 *
 * \return error status of the operations 
 *
 */

HRESULT win32_get_folder_path(gchar* folder_path,const int csidl)        /* {{{ */
{   

    HRESULT hr             = NO_ERROR;
    int      folder_csidl  = csidl & CSIDL_FOLDER_MASK;
    gboolean create_folder = csidl & CSIDL_FLAG_CREATE;
    gchar*   utf8filename  = NULL;
    
    PFNSHGETFOLDERPATH  pGetFolderPath = _win32_getfolderpath();

    *folder_path           = 0;

    if ((pGetFolderPath)&&(!(pGetFolderPath)(NULL,csidl,NULL,0,folder_path)))
    {
        hr = GetLastError();
    }
    else
    {
        folder_path = g_strconcat("C:\\",NULL);
        hr = 0;
    }
 /*   
    if (!g_utf8_validate(folder_path, -1, NULL))
    {
      utf8filename = g_filename_to_utf8(folder_path, -1, NULL, NULL, NULL);
      if (utf8filename == NULL) {
	//message_warning(_("Some characters in the filename are neither UTF-8 nor your local encoding.\nSome things will break."));
      }
    }
    if (utf8filename != NULL) g_strlcpy(folder_path,utf8filename,MAX_PATH);
*/
    return hr;
} /* }}}  */

/**
 * Retrive the "My Documents" absloute directory path
 *  
 *  Use GetLastError to retrieve error status
 *  ! See win32_get_folder_path header for warning information
 *  
 * \return my documents absolute path string address ended by "\\"
 *
 */
gchar* win32_get_my_documents_folder_path()            /* {{{ */
{
    SetLastError(win32_get_folder_path(my_documents_path,CSIDL_PERSONAL|CSIDL_FLAG_CREATE));
    g_strlcat(my_documents_path,"\\",MAX_PATH+1);
    return my_documents_path;
} /* }}}  */

/**
 * Retrieve the "Windows" absolute directory path
 *   
 *  Use GetLastError to retrieve error status
 *  ! See win32_get_folder_path header for warning information
 * 
 * \return a pointer the the result status
 *
 */
gchar* win32_get_windows_folder_path(void)                  /* {{{ */
{
   SetLastError(GetWindowsDirectory(windows_path,MAX_PATH));
    return windows_path;
} /* }}}  */

/**
 * Retrive the absloute directory path of the gribi configuration file 
 *
 *  The configuration file is located in "Application Data"\Grisbi
 *  If the configuration file does not exist, it will be created by the
 *  function.
 *    
 *  Use GetLastError to retrieve error status
 *  ! See win32_get_folder_path header for warning information
 *
 * /return : absolute path string address ended by "\\"
 *
 */
gchar* win32_get_grisbirc_folder_path()  /* {{{ */
{
    /* special cases : APP_DATA & WIN95/NT4) */
    //win_version current_version = win32_get_windows_version();    
    //if ((current_version == WIN95)||(current_version == WINNT4))
    //{
   //     g_strlcpy(grisbirc_path,win32_get_windows_folder_path(),MAX_PATH+1);
    //    g_strlcat (grisbirc_path,"\\",MAX_PATH+1);        
    //} 
    //else
    //{
        gchar* local_filename = NULL;
        SetLastError(win32_get_folder_path(grisbirc_path,CSIDL_APPDATA|CSIDL_FLAG_CREATE));
        g_strlcat(grisbirc_path,"\\Grisbi\\",MAX_PATH+1);
 //       local_filename = g_filename_from_utf8(grisbirc_path, -1, NULL, NULL, NULL);
 //       if (local_filename == NULL) g_strlcpy(local_filename,grisbirc_path,MAX_PATH);
        CreateDirectory(grisbirc_path,NULL);
    //}
    return grisbirc_path;
} /* }}} */


/**
 * store full path with filename retrieved from argv[0]
 */
void  win32_set_app_path(gchar* app_dir)
{
    
    g_strlcpy(grisbi_exe_path,app_dir,MAX_PATH);
}
 
/**
 * Construct app subdir like help directory from application running dir
 */
gchar* win32_app_subdir_folder_path(gchar * app_subdir)
{
    return g_strdelimit(g_strconcat(g_path_get_dirname ( grisbi_exe_path  ),"\\",app_subdir,NULL),
                        "\\",
                        '/');
}
// }}}1
// -------------------------------------------------------------------------
// Windows(c) Version ID and Technology                          PART_3 {{{1
//      Version ID is 95/98/NT/2K/...
//      Technology is 3.1/9x/NT/...
// -------------------------------------------------------------------------
/**
 * Return the current Windows(c) Version ID 
 *
 * \return The Windows(c) Version ID in win_version
 */
win_version    win32_get_windows_version(void)                        /* {{{ */
{
    win_version current_version = WIN_UNKNOWN;

    OSVERSIONINFO VersInfos;
    VersInfos.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VersInfos);

    DWORD dwPlatormId = VersInfos.dwPlatformId;
    DWORD dwMajorVers = VersInfos.dwMajorVersion;
    DWORD dwMinorVers = VersInfos.dwMinorVersion;

    switch (dwPlatormId)
    {
        case VER_PLATFORM_WIN32s: // Win 3.1 
            current_version = WIN31;
            break;
        case VER_PLATFORM_WIN32_WINDOWS: // Win 9x
            if (dwMinorVers == 0)
            {
                current_version = WIN95;
            }
            else
            {
                current_version = WIN98;
            }
            break;
        case VER_PLATFORM_WIN32_NT:
            if (dwMajorVers == 3 )
            {
                current_version = WINNT3;
            }
            else if (dwMajorVers == 4 )
            {
                current_version = WINNT4;
            }
            else if (dwMajorVers == 5 )
            {
                current_version = WIN2K;
            }
            break;
        default:
            current_version = WIN_UNKNOWN;
            break;
    }
    return current_version;
    
} /* }}} GetWindowsVersion */
/**
 *  Return the current Windows(c) Technology (9x/NT/...) from
 *      a win_version Windows(c) Version ID.
 * \param version version to determine the technology
 * \return The Windows(c) Technology in win_technology
 */
win_technology win32_get_windows_technology(win_version version) /* {{{ */
{
    if ((version & WIN_UNSUPPORTED ) == WIN_UNSUPPORTED)
    {
        return WIN_UNSUPPORTED;
    }
    else if ((version & WIN_9X) == WIN_9X)
    {
        return WIN_9X;
    }
    else if ((version & WIN_NT) == WIN_NT)
    {
        return WIN_NT;
    }
    else
    {
        return WIN_UNSUPPORTED;
    }
} /* }}} win32_get_windows_technology */
/* }}} */

BOOL win32_shell_execute_open(gchar* file)
{
   return ((int)ShellExecute(NULL, "open", file, NULL, NULL, SW_SHOWNORMAL)>32);
}


// -------------------------------------------------------------------------
// End of WinUtils
// -------------------------------------------------------------------------



