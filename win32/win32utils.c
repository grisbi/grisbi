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
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "win32utils.h"

// -------------------------------------------------------------------------
// Windows(c) Usefull Functions      
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
// Windows(c) Special Paths ...     
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
 * Retrieve the absolute path in utf8 of a CSIDL directory named
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
    //gboolean create_folder = csidl & CSIDL_FLAG_CREATE;
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
    
    if (!g_utf8_validate(folder_path, -1, NULL))
    {
      utf8filename = g_filename_to_utf8(folder_path, -1, NULL, NULL, NULL);
      if (utf8filename == NULL) {
        utf8filename = g_strconcat("C:\\",NULL);
      }
      if (utf8filename != NULL) g_strlcpy(folder_path,utf8filename,MAX_PATH);
    }
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
    // g_strlcat(my_documents_path,"\\",MAX_PATH+1);
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
    gchar* local_filename = NULL;

    SetLastError(win32_get_folder_path(grisbirc_path,CSIDL_APPDATA|CSIDL_FLAG_CREATE));
    g_strlcat(grisbirc_path,"\\Grisbi\\",MAX_PATH+1);

    // To create directory we need to go back from utf8 to locale
    local_filename = g_filename_from_utf8(grisbirc_path, -1, NULL, NULL, NULL);
    if (local_filename == NULL) g_strlcpy(local_filename,grisbirc_path,MAX_PATH);

    CreateDirectory(local_filename,NULL);

    return grisbirc_path;
} /* }}} */


/**
 * store full path with filename retrieved from argv[0] converted to utf-8 for any internal use
 */
void  win32_set_app_path(gchar* syslocale_app_dir) /* {{{ */
{
    gchar* uft8_app_dir = NULL;
    if (!syslocale_app_dir)
    {
        g_strlcpy(grisbi_exe_path,"",MAX_PATH);
    }
    else
    {
        uft8_app_dir = g_filename_to_utf8(syslocale_app_dir, -1, NULL, NULL, NULL);
        if (uft8_app_dir == NULL)
        {
            uft8_app_dir = g_strdup(syslocale_app_dir);
        }
        g_strlcpy(grisbi_exe_path,uft8_app_dir,MAX_PATH);
    }
    if (uft8_app_dir)
    {
        g_free(uft8_app_dir);
        uft8_app_dir = NULL;
    }
} /* }}} */
 
/**
 * Construct app subdir like help directory from application running dir
 */
gchar* win32_app_subdir_folder_path(gchar * app_subdir) /* {{{ */
{
    return g_strdelimit(g_strconcat(g_path_get_dirname ( grisbi_exe_path  ),"\\",app_subdir,NULL),
                        "\\",
                        '/');
} /* }}} */
// -------------------------------------------------------------------------
// Windows(c) Version ID and Technology       
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
	DWORD dwPlatormId;
	DWORD dwMajorVers;
	DWORD dwMinorVers;

    OSVERSIONINFO VersInfos;
    VersInfos.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VersInfos);

    dwPlatormId = VersInfos.dwPlatformId;
    dwMajorVers = VersInfos.dwMajorVersion;
    dwMinorVers = VersInfos.dwMinorVersion;

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

BOOL win32_shell_execute_open(const gchar* file) /* {{{ */
{
   return ((int)ShellExecute(NULL, "open", file, NULL, NULL, SW_SHOWNORMAL)>32);
} /* }}} */

/*
 * Start a new process based on CreateProcess
 */
BOOL win32_create_process(gchar* application_path,gchar* arg_line,gchar* utf8_working_directory, gboolean detach,gboolean with_sdterr) /* {{{ */
{

    DWORD dw;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR arg[2*MAX_PATH];
	HANDLE hStdError;

    gchar* syslocale_working_directory = NULL; 
      
    if ((utf8_working_directory)&&(*utf8_working_directory))
    {
        syslocale_working_directory = g_strdelimit(g_locale_from_utf8(utf8_working_directory,-1,NULL,NULL,NULL),"/",'\\');
    }

    memset( &si, 0, sizeof(si) );
    si.cb = sizeof(si);

    memset( &pi, 0, sizeof(pi) );
    // STDERR
    if (with_sdterr)
    {
        TCHAR stderr_path[MAX_PATH];
        strcpy(stderr_path,application_path);
        strcat(stderr_path,".err");
        hStdError = CreateFile(stderr_path,     // file to create
                   GENERIC_WRITE,          // open for writing
                   0,                      // do not share
                   NULL,                   // default security
                   CREATE_ALWAYS,          // overwrite existing
                   FILE_ATTRIBUTE_NORMAL,  // normal file
                   NULL);                  // no attr. template

        if (hStdError == INVALID_HANDLE_VALUE) 
        { 
            printf("Could not open file (error %d)\n", (int)GetLastError());
        }
        else
        {
            si.hStdError = hStdError;
        }
    }

    sprintf(arg," \"%s\" ",arg_line);
    if(!CreateProcess(application_path,
        arg,
        NULL,
        NULL,
        FALSE,
        CREATE_DEFAULT_ERROR_MODE|DETACHED_PROCESS|NORMAL_PRIORITY_CLASS,
        NULL,
        syslocale_working_directory,
        &si,
        &pi))
    {
        TCHAR szBuf[255]; 
        LPVOID lpMsgBuf;
        dw = GetLastError(); 

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );

        wsprintf(szBuf, 
        "CreateProcess %s failed with error %d: %s", 
        application_path, dw, lpMsgBuf); 
        
 
        MessageBox(NULL,szBuf, "Error CreateProcess", MB_OK); 

        LocalFree(lpMsgBuf);
    }
    else
    {
        // Wait until child process exits.
        if (!detach) WaitForSingleObject( pi.hProcess, INFINITE );

        // Close process and thread handles. 
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );

    }

    // Restore the current environnement
    if ((si.hStdError) && (si.hStdError != INVALID_HANDLE_VALUE))
    {
        CloseHandle(si.hStdError);
        si.hStdError = NULL;
    }
   if (!syslocale_working_directory) 
   { 
       g_free(syslocale_working_directory);
       syslocale_working_directory = NULL;
   }
   return (gboolean)(dw==0);
} /* }}} */

/* 
 * The GetLongPathName API call is only available on Windows 98/ME and Windows 2000/XP. 
 * It is not available on Windows 95 & NT. This function will emulate it on all system.
 * The algorithm recursively strips off path components, then reassembles them with
 * their long name equivalents.
 * 
 *
 * \param lpszShortPath [in]  Pointer to a null-terminated path to be converted limited to MAX_PATH characters
 * \param lpszLongPath  [out] Pointer to the buffer to receive the long path. 
 * \param cchBuffer     [in]  Size of the buffer, in TCHARs. 
 * \caveats
 *    The provided buffer MUST have been ALLOCATED before calling the function.
 *   
 * @return 
 *    If the function succeeds, the return value is the length of the string copied to the lpszLongPath parameter,
 *    in TCHARs. This length does not include the terminating null character.
 *    If the function fails for any other reason, the return value is zero.
 *    To get extended error information, call GetLastError.
 *
 *    If the lpszLongPath buffer is too small to contain the path, the return
 *    value is the size of the buffer required to hold the path, including the
 *    terminating null character, in TCHARs. Therefore, if the return value is
 *    greater than cchBuffer, call the function again with a buffer is at least
 *    this large.
 * 
 * @note
 *   This function is inspired on the "GetLongPathName API Function Emulation" 
 *   provided by Randall Garacci on the CodeGuru site
 *   http://www.codeguru.com/Cpp/W-P/files/article.php/c4461/
 *   Here after the code:
 *   DWORD GetLongPathName(CString  strShortPath,
 *                          CString& strLongPath  )
 *    {
 *      int iFound = strShortPath.ReverseFind('\\');
 *      if (iFound > 1)
 *      {
 *        // recurse to peel off components
 *        //
 *        if (GetLongPathName(strShortPath.Left(iFound),
 *                            strLongPath) > 0)
 *        {
 *          strLongPath += '\\';
 *    
 *          if (strShortPath.Right(1) != "\\")
 *          {
 *            WIN32_FIND_DATA findData;
 *    
 *            // append the long component name to the path
 *            //
 *            if (INVALID_HANDLE_VALUE != ::FindFirstFile
 *               (strShortPath, &findData))
 *            {
 *              strLongPath += findData.cFileName;
 *            }
 *            else
 *            {
 *              // if FindFirstFile fails, return the error code
 *              //
 *              strLongPath.Empty();
 *              return 0;
 *            }
 *          }
 *        }
 *      }
 *      else
 *      {
 *        strLongPath = strShortPath;
 *      }
 *    
 *      return strLongPath.GetLength();
 *    }
 * 
 * @note (for reading purpose \ and replaced by / in the samples)
 *   dir1/dir2/file  "long dir1/long dir2/long filename"
 *   /dir1/dir2/file "/long dir1/long dir2/long filename"
 *   c:/dir1/dir2/file -> "c:/long dir1/long dir2/long file name"
 *   
 * @caveats   
 *   \\server\dir is not supported in this version of the implementation 
 *   duplicate directory separator are not supported by this implementation
 * 
 */
gchar* win32_long_name(gchar* short_name) /* {{{ */
{
    gchar* syslocale_short_name = g_strdelimit(g_strdup(short_name),"/",'\\');
    gchar* syslocale_long_name  = NULL;

    
    // optimisation : only short names containing '~' have a chance to be different in 'long name'
    if (strchr(syslocale_short_name,'~'))
    {
        gint    index               = 0;
        gchar** syslocale_subitems  = g_strsplit(syslocale_short_name,"\\",0);
        gchar*  item                = syslocale_subitems[0];

        if ((strlen(item) == 2)&&(item[1] == ':'))
        {
            syslocale_long_name = g_strdup(item);
            index++;
        }

        for(;syslocale_subitems[index] != NULL; index++)
        {
            item = syslocale_subitems[index];
            //
            // search and append the long component name to the path
            // 
            if (*item) 
            {
                gchar* fullpath = g_strjoin("\\", syslocale_long_name, item, NULL);
                WIN32_FIND_DATA findData;

                if (INVALID_HANDLE_VALUE != FindFirstFile(fullpath,&findData))
                {
                    gchar* previous_long_name = syslocale_long_name;
                    syslocale_long_name = g_strjoin("\\",previous_long_name,findData.cFileName,NULL);
                    g_free(previous_long_name);
                }
                else // if FindFirstFile fails, stop here and return shortname
                {
                    if (syslocale_long_name)
                    {
                        g_free(syslocale_long_name);
                        syslocale_long_name = NULL;
                    }
                }
            }
        }

        if (syslocale_subitems)
        {
            g_strfreev(syslocale_subitems);
            syslocale_subitems = NULL;
        }
    }
    
    // if long name is not not convert it to utf8
    // else use short name ...
    if (!syslocale_long_name)
    {
        syslocale_long_name = g_strdup(short_name);
    }
    
    if (syslocale_short_name)
    {
        g_free(syslocale_short_name);
        syslocale_short_name = NULL;
    }

    
    return syslocale_long_name;
} /* }}} */
/*
 * Return the absolute path of the provided path
 */
gchar* win32_full_path(gchar* syslocale_path) /* {{{ */
{
    TCHAR lpsz_fullpath[(MAX_PATH*2)+1];
    _fullpath(lpsz_fullpath,(char*)syslocale_path,(size_t)(MAX_PATH*2));
    return g_strdup((gchar*)lpsz_fullpath);
    
} /* }}} */

#define _WIN32_CHAR_IS_DIR_SEPARATOR(c) ((c=='\\')||(c== '/'))
/** \deprecated */
#ifdef _USE_DEPRECTED_
DWORD win32_get_long_path_name(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD ccBuffer) /* {{{ */
{
    int iFound = -1;
        
    iFound = strlen(lpszShortPath);

    // UNC path form is not supported, return immediatly
    if ( (iFound >= 2) && (_WIN32_CHAR_IS_DIR_SEPARATOR(lpszShortPath[0])) && (_WIN32_CHAR_IS_DIR_SEPARATOR(lpszShortPath[1]) ))
    {
        return -1;
    }
    
    
    //// remove duplicate (or more) '\' from path by going back to the first of the serie
    //while ((iFound > 0) && (lpszShortPath[iFound-1] == '\')) { iFound--; }
    // 
    /*
     * Reverse find '\\' in szShortPath 
     */
    for ( iFound = strlen(lpszShortPath)-1; (iFound >= 0) && (! _WIN32_CHAR_IS_DIR_SEPARATOR(lpszShortPath[iFound])) ; --iFound);
    // Remove duplicated directory separator

    /*
     * Last item to convert detection:
     * 01234567890
     * c:/dir1/    iFound = 2;path[1] = ':'
     * /dir1/...   iFound = 0;path[1]!= ':'
     * dir1/       iFound =-1;path[1]!= ':';path[2] != '\' ** FIXME Not supported **
     */
    /*
     * If we point to a '\'  at the third position of the string, check if we are in the <disk>:\ case
     */
    if ((iFound == 2) && (lpszShortPath[1]==':')) { iFound = -1 ; }
    
    // There is still part in the string which can be converted
    if (iFound >  1) 
    {
        TCHAR szLeftOfShortName[MAX_PATH+1];

        strcpy((TCHAR*)szLeftOfShortName,(TCHAR*)lpszShortPath);
        szLeftOfShortName[iFound] = 0;
        
        if (win32_get_long_path_name(szLeftOfShortName,lpszLongPath,ccBuffer) > 0)
        {

            if ( lpszShortPath[strlen(lpszShortPath)-1] != '\\' )
            {
                // search and append the long component name to the path
                WIN32_FIND_DATA findData;
                if (INVALID_HANDLE_VALUE != FindFirstFile(lpszShortPath,&findData))
                {
                    strcat((TCHAR*)lpszLongPath,"\\");
                    strcat((TCHAR*)lpszLongPath,findData.cFileName);
                }
                else // if FindFirstFile fails, return the error code
                {
                    *lpszLongPath = 0;
                    return 0;
                }
            }
        }
    }
    // nothing to convert ( c:\ \ )
    else
    {
        strcpy((TCHAR*)lpszLongPath,(TCHAR*)lpszShortPath);
    }
    return strlen((TCHAR*)lpszLongPath);
} /* }}} */
#endif
/**
 * Get Environment variable value using Windows method.
 *
 * \param env_var Environment varaible name
 * \return newly allocated buffer containing the variable value 
 *      the buffer as to be freed using g_free when no more needed.
 * 
 * \retval NULL an error occured. PLease win32_get_last_error to have more information
 */
gchar* win32_get_environment_variable(gchar* env_var) /* {{{ */
{
    int    nVarBufferSize = GetEnvironmentVariable((LPCTSTR)(char*)(env_var),NULL,0);
    
    TCHAR* lpVarBuffer    = (TCHAR*)malloc( sizeof(TCHAR) * (nVarBufferSize + 1));
    
    gchar* ret_buffer     = NULL; 
    
    if (lpVarBuffer)
    {
        GetEnvironmentVariable((LPCTSTR)(char*)(env_var),lpVarBuffer,nVarBufferSize);
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
    }
    
    // Copy buffer to a buffer managed by GTK
    ret_buffer = g_locale_to_utf8(g_strdelimit(g_strdup((gchar*)lpVarBuffer),"\\",'/'),-1,NULL,NULL,NULL);
    
    free(lpVarBuffer);
    lpVarBuffer = NULL;
    
    return ret_buffer;
} /* }}} */
/**
 * Set Environment variable value.
 *
 * \param env_var   variable name
 * \param value_buffer new value buffer
 *
 * \return same as SetEnvironmentVariable.
 */
gboolean win32_set_environment_variable(gchar* env_var, gchar* value_buffer) /* {{{ */
{
    gchar*   syslocale_buffer = g_locale_from_utf8(value_buffer,-1,NULL,NULL,NULL);
    gboolean bret             = SetEnvironmentVariable((LPCTSTR)(char*)env_var,syslocale_buffer);
    
    g_free(syslocale_buffer);
    return bret;
} /* }}} */

/**
 * Force the application path to be in PATH.
 *
 * This is used for dll load. This way we are sure that
 * local application is the coorecto one especially when running
 * from file association.
 *
 * \param syslocale_app_dir path to add
 * \param at 0 head, -1 end
 *
 */
void win32_add_to_path(gchar* utf8_app_dir,gint at) /* {{{ */
{
    gchar* new_path = NULL;
    gchar* utf8path = win32_get_environment_variable("PATH");
    
    switch (at)
    {
        default:
        case 0:
            new_path = g_strjoin(";",utf8_app_dir,utf8path);
            break;
        case -1:
            new_path = g_strjoin(";",utf8path,utf8_app_dir);
            break;
    }
    MessageBox(NULL,new_path,"",MB_OK);
    
    win32_set_environment_variable("PATH",g_strdelimit(new_path,"/",'\\'));

    g_free(utf8path); utf8path = NULL;
    g_free(new_path); new_path = NULL;

} /* }}} */
void win32_set_current_directory(gchar* utf8_dir) /* {{{ */
{
    gchar* syslocale_dir = g_strdelimit(g_locale_from_utf8(utf8_dir,-1,NULL,NULL,NULL),"/",'\\');
    SetCurrentDirectory(syslocale_dir);
    g_free(syslocale_dir);
} /* }}}  */

void win32_parse_gtkrc (const gchar * basename){
	gchar * out;
	gchar * gtkrc_file = g_strdelimit(g_strconcat(g_path_get_dirname ( grisbi_exe_path  ),"\\",basename,NULL),
                        "\\",
                        '/');
	out = g_locale_to_utf8(gtkrc_file, -1, NULL, NULL, NULL);
	g_free ( gtkrc_file);
	gtkrc_file = g_filename_from_utf8(out, -1, NULL, NULL, NULL);
	g_free (out);
	gtk_rc_parse(gtkrc_file);
	g_free ( gtkrc_file );
}


// -------------------------------------------------------------------------
// End of WinUtils
// -------------------------------------------------------------------------



