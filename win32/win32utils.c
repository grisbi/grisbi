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
/** 
 * Retrieve the absolute path of a CSIDL directory named
 *      
 *      *Warning* This function use SHGetFolderPath - [from MSDN library]
 *          This function is a superset of SHGetSpecialFolderPath, included 
 *          with earlier versions of the Shell. It is implemented in a redistri-
 *          butable DLL, SHFolder.dll, that also simulates many of the new Shell
 *          folders on older platforms such as Windows 95, Windows 98, and 
 *          Windows NT® 4.0. This DLL always calls the current platform's ver-
 *          sion of this function. If that fails, it will try to simulate the 
 *          appropriate behavior. Only some CSIDLs are supported
 *
 * \param folder_path  already allocated buffer to retrieve the result
 * \param csidl        Window ID of the directory (csidl)
 *
 * \return error status of the operations 
 *
 */
HRESULT win32_get_folder_path(gchar* folder_path,const int csidl)        /* {{{ */
{   
    HRESULT hr          = NO_ERROR;

    // Allocate a pointer to an Item ID list
    LPITEMIDLIST pidl;

    *folder_path = 0;

    hr = SHGetFolderPath(NULL, csidl, NULL, 0, folder_path);
    // Force the application of the CSIDL_FLAG_CREATE nativly supported by XP only 
    if (!hr && (( csidl & CSIDL_FLAG_CREATE) == CSIDL_FLAG_CREATE))
    {
        CreateDirectory(folder_path,NULL);
    }
    return hr;
} /* }}}  */

static gchar my_documents_path[MAX_PATH+1];
static gchar windows_path     [MAX_PATH+1]; 
static gchar grisbirc_path    [MAX_PATH+1];

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
gchar* win32_get_windows_folder_path()                  /* {{{ */
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
    SetLastError(win32_get_folder_path(grisbirc_path,CSIDL_APPDATA|CSIDL_FLAG_CREATE));
    g_strlcat(grisbirc_path,"\\Grisbi\\",MAX_PATH+1);
    CreateDirectory(grisbirc_path,NULL);
    return grisbirc_path;
} /* }}} */
// }}}1
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// End of WinUtils
// -------------------------------------------------------------------------
