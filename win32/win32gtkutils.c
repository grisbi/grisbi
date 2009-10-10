/*!
 * \file    win32gtkutils.c
 * \brief   Implementation of some usefull function to make the GTK API
 *          working with the WIN32 one.
 * \author  Francois Terrot (Francois.terrot@grisbi.org)
 *
 * \note About coding rules used for this file
 * 
 * As this file implement an interface between two worlds, the function prototype
 * used in the same place as GTK one will follow the "Grisbi" coding standard and 
 * GTK types.
 * 
 * But the functions bodies are WIN32 code, so the code of the functions will
 * follow the classical WIN32 coding standards (especially for naming rule and types).
 * All type adaption needed to be done by the function.
 *
 */
/* ************************************************************************** */
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
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "win32gtkutils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HKLM_GTK_20         "SOFTWARE\\GTK\\2.0"    /*!< GTK2 information location is the registry subkeys tree */
#define GTK_DLL_PATH_KEY    "DllPath"               /*!< "GTK Dlls Path" location registry entry name */

#define HKLM_UNINSTALL_GTK      "SOFTWARE\\Microsoft\\Windows\\WinGTK-2_is1"
#define GTK_INSTALL_PATH_KEY    "InstallLocation"
/*!
 * 
 * Under Windows, shared libraries (alse named DLLs) should be located in the  (uorking
 * directory or in one of the directories listed in the "PATH" environnment variable.
 * So to be able to run Grisbi using a common installation of GTK2, the GTK Dll path 
 * should be present in the "PATH".
 *
 * The best way to be sure the "GTK Dll Path" is present in the "PATH" variable is to
 * add it each time GRISBI is run without any check.
 *
 * To save some time during Dll search, the "Gtk Dll Path" is insert at the first position of the directory list. 
 *
 * \note 
 * This modification of the "PATH" value impact <b>only</b> the process conext of the current instance of Grisbi.
 * Neither the system or user global "PATH" value are modified and the modification is lost
 * when Grisbi is closed.
 * 
 * \note
 * The value of the "PATH" variable is a list of absolute directory separate by a semi column ';'
 * Environment variable may be used.
 *
 * \return  Global status of the operations.
 * \retval  TRUE    The GTK2 Dll Path is now present in the current PATH value
 * \retval  FALSE   Someting avoid the function to add "GTK Dlls Path" to the PATH value
 *                  use win32_get_last_error() to retrieve a more detailled error code.
 *                  - ERROR_EMPTY : GTK seems to not be present (no registry keys)
 *
 * \ref GetEnvironmentVariable()
 * \ref SetEnvironmentVariable() 
 * \ref win32_get_last_error()
 */
gboolean win32_make_sure_the_gtk2_dlls_path_is_in_PATH()
{ /* {{{ */

   LPCTSTR lpPathBuffer     = NULL;
   DWORD    nPathBufferSize = 0; 
    
   DWORD   dwStatus         = NO_ERROR;
   DWORD   dwType           = REG_SZ;
   PCHAR   pKeyValBuffer    = NULL   ;
   DWORD   dwKeyValSize     = 0;
   HKEY    hKey             = HKEY_LOCAL_MACHINE;

   // Retrieve the installation location of GTK2 from registry
   dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCSTR)(HKLM_UNINSTALL_GTK), 0, KEY_READ, &hKey);
   if (dwStatus == NO_ERROR)
   {
       // Get the size of the key to allocate the correct size for the temporary pBuffer
       dwStatus = RegQueryValueEx(hKey,(LPCSTR)(GTK_INSTALL_PATH_KEY),0,&dwType,(LPBYTE)pKeyValBuffer,&dwKeyValSize);
       if (dwStatus == NO_ERROR)
       {
           // allocate the buffer and don't forget the '\0'
           pKeyValBuffer = (char*)malloc(sizeof(char*)*(dwKeyValSize+1));
           memset(pKeyValBuffer,0,(sizeof(char*)*(dwKeyValSize+1)));

           // At last, ... read the value ...
           dwStatus = RegQueryValueEx(hKey,(LPCSTR)(GTK_INSTALL_PATH_KEY),0,&dwType,(LPBYTE)pKeyValBuffer,&dwKeyValSize);
           if ((dwStatus == NO_ERROR)&&(dwKeyValSize == 0))
           {
               dwStatus = ERROR_EMPTY;
           }
       }
       // Don't forget to close the handle
       RegCloseKey(hKey);
   }
   
   // There is nothing to add if dwStatus is an error value
   if (dwStatus == NO_ERROR)
   {
       // Make the buffer to retrieve the content of the path varialble to small
       // to retrieve the size used by the current PATH value
       nPathBufferSize = GetEnvironmentVariable((LPCTSTR)("PATH"),NULL,0);

       // As we use "brute force" the allocated buffer need to take account of the
       // GTK Dll path we will add to the PATH without forgetting the ';' and '\0'
       if ( (lpPathBuffer = (LPCTSTR)malloc( sizeof(TCHAR) * (nPathBufferSize + dwKeyValSize + 1 + 1))))
       {
           // Insert GTL Dll path first to win some time ...
           // In a other way add the current "PATH" value after the "Dll Path" one
           TCHAR* ptrEndOfPathString = (TCHAR*)lpPathBuffer+dwKeyValSize;

           strcpy((char*)lpPathBuffer,(char*)pKeyValBuffer);
           * (ptrEndOfPathString++) = ';';
           if (!GetEnvironmentVariable((LPCTSTR)("PATH"),ptrEndOfPathString,nPathBufferSize))
           {
               dwStatus = GetLastError();
           }

           // At last set the new value for PATH ...
           if ((dwStatus == NO_ERROR)&&(!SetEnvironmentVariable((LPCTSTR)("PATH"),lpPathBuffer)))
           {
               dwStatus = GetLastError();
           }
       }
       else
       {
           dwStatus = ERROR_OUTOFMEMORY;
       }
   }

   // Leave the place as clean as we found it ...
   if (lpPathBuffer)
   {
       free((void*)lpPathBuffer);
       lpPathBuffer = NULL;
   }
   if (pKeyValBuffer)
   {
       free(pKeyValBuffer);
       pKeyValBuffer = NULL;
   }

   // back to the GTK world...
   SetLastError(dwStatus);
   return (gboolean)(dwStatus == NO_ERROR);

} /* }}} win32_force_gtk2_dlls_to_be_in_path */

/**
 * Convert the Windows lang code into the compliant gettext LOCALE value.
 * 
 * \note This function has be inspired by win_gaim.c:wgaim_lcid_to_posix() 
 *  from the Gaim (http://gaim.sf.net/) project
 *  
 *  \param lcid Window code from a language
 *
 *  \return the corresponding LOCALE string 
 *  \retval NULL if the lcid value is unknown
 *
 *  \caveats : the returned string must be unallocated when no more needed
 *  
 */
#define CASE_LCID_RETURN_LANG(lcid,lang)   case lcid : return g_strdup(lang) 
static gchar* win32_lcid_to_lang(LCID lcid) 
{ /* {{{ */
    switch(lcid) {
        CASE_LCID_RETURN_LANG(1026,"bgr"); /* bulgarian */
        CASE_LCID_RETURN_LANG(1027,"cat"); /* catalan */
        CASE_LCID_RETURN_LANG(1050,"hrv"); /* croatian - croatia */
        CASE_LCID_RETURN_LANG(1029,"csy"); /* czech */
        CASE_LCID_RETURN_LANG(1030,"dan"); /* danish */
        CASE_LCID_RETURN_LANG(1043,"nld"); /* dutch - netherlands */
        CASE_LCID_RETURN_LANG(1033,"enu"); /* english - us */
        CASE_LCID_RETURN_LANG(1035,"fin"); /* finnish */
        CASE_LCID_RETURN_LANG(1036,"fra"); /* french - france */
        CASE_LCID_RETURN_LANG(1031,"deu"); /* german - germany */
        CASE_LCID_RETURN_LANG(1032,"ell"); /* greek */
        CASE_LCID_RETURN_LANG(1037,"heb"); /* hebrew */
        CASE_LCID_RETURN_LANG(1038,"hun"); /* hungarian */
        CASE_LCID_RETURN_LANG(1040,"ita"); /* italian - italy */
        CASE_LCID_RETURN_LANG(1041,"jpn"); /* japanese */
        CASE_LCID_RETURN_LANG(1042,"kor"); /* korean */
        CASE_LCID_RETURN_LANG(1063,"lth"); /* lithuanian */
        CASE_LCID_RETURN_LANG(1071,"mki"); /* macedonian */
        CASE_LCID_RETURN_LANG(1045,"plk"); /* polish */
        CASE_LCID_RETURN_LANG(2070,"ptg"); /* portuguese - portugal */
        CASE_LCID_RETURN_LANG(1046,"ptb"); /* portuguese - brazil */
        CASE_LCID_RETURN_LANG(1048,"rom"); /* romanian - romania */
        CASE_LCID_RETURN_LANG(1049,"rus"); /* russian - russia */
        CASE_LCID_RETURN_LANG(2074,"srl"); /* serbian - latin - serbia*/
        CASE_LCID_RETURN_LANG(3098,"srb"); /* serbian - cyrillic - serbia*/
        CASE_LCID_RETURN_LANG(2052,"chs"); /* chinese - simplified - Poeple's Republic of China*/
        CASE_LCID_RETURN_LANG(1051,"sky"); /* slovak */
        CASE_LCID_RETURN_LANG(1060,"slv"); /* slovenian */
        CASE_LCID_RETURN_LANG(1034,"esp"); /* spanish */
        CASE_LCID_RETURN_LANG(1052,"sqi"); /* albanian */
        CASE_LCID_RETURN_LANG(1053,"sve"); /* swedish - sweden*/
        CASE_LCID_RETURN_LANG(1054,"tha"); /* thai */
        CASE_LCID_RETURN_LANG(1028,"cht"); /* chinese - taiwan (traditional) */
        CASE_LCID_RETURN_LANG(1055,"trk"); /* turkish */
        CASE_LCID_RETURN_LANG(1058,"ukr"); /* ukrainian */
        default:
                return (gchar*)NULL;
        }
} /* }}} */

/**
 * Read the registry and return the corresponding value as a gchar* (g_strdup already done)
 *
 * @param a_hRoot
 * @param a_szKey
 * @param a_szValue
 * @param a_dwType
 *
 */
gchar* win32_strdup_registry(HKEY a_hRoot, TCHAR* a_szKey, TCHAR* a_szValue, DWORD a_dwType)
{
    gchar* gzLocale = NULL;
    // Retrieve value set by grisbi_set_language
    DWORD   dwStatus         = NO_ERROR;
    DWORD   dwType           = a_dwType;
    PCHAR   pKeyValBuffer    = NULL;
    DWORD   dwKeyValSize     = 0;
    HKEY    hKey             = a_hRoot;
    
    // ---------------------------------------------------------------------
    // Grisbi language has been configured through the specific application
    // ---------------------------------------------------------------------
    // Retrieve the application location from registry
    dwStatus = RegOpenKeyEx(a_hRoot, (LPCSTR)a_szKey, 0, KEY_READ, &hKey);
    if (dwStatus == NO_ERROR)
    {
        // Get the size of the key to allocate the correct size for the temporary pBuffer
        dwStatus = RegQueryValueEx(hKey, (LPCSTR)a_szValue,0,&dwType,(LPBYTE)pKeyValBuffer,&dwKeyValSize);
        if (dwStatus == NO_ERROR)
        {
            // allocate the buffer and don't forget the '\0'
            pKeyValBuffer = (char*)malloc(sizeof(char*)*(dwKeyValSize+1));
            memset(pKeyValBuffer,0,(sizeof(char*)*(dwKeyValSize+1)));

            // At least, ... read the value ...
            dwStatus = RegQueryValueEx(hKey,(LPCSTR)a_szValue,0,&dwType,(LPBYTE)pKeyValBuffer,&dwKeyValSize);
            if ((dwStatus == NO_ERROR)&&(dwKeyValSize == 0))
            {
                dwStatus = ERROR_EMPTY;
            }
        }
        // Don't forget to close the handle
        RegCloseKey(hKey);
    }
    
    if ((dwStatus == NO_ERROR)&&(pKeyValBuffer)&&(*pKeyValBuffer))
    {
        gzLocale = g_strdup((gchar*)pKeyValBuffer);
    }

    if (pKeyValBuffer)
    {
        free(pKeyValBuffer);
        pKeyValBuffer = NULL;
    }

    SetLastError(dwStatus);
    return (gzLocale);
} 
/**
 * 
 * Determine and set Grisbi locale as follows (in order of priority):
 * - Value of HKCU\SOFTWARE\GRISBI\LANG
 * - Value of %LANG% (LANG con content a LOCALE definition or a LCID value)
 * - Default user value
 * 
 * @return Does the function change the application context LANG value
 * @retval TRUE a value have set to LANG
 * @retval FALSE LANG has not been modified for any reason.
 * 
*/
gboolean win32_set_locale() 
{ /* {{{ */
    gchar* gzLocale = NULL;
    BOOL    bFound           = FALSE;
    
    // ---------------------------------------------------------------------
    // Grisbi language has been configured through the specific application
    // ---------------------------------------------------------------------
    // 1 - look into HKCU/SOFTWARE/GRISBI/LANG
    gzLocale = win32_strdup_registry(HKEY_CURRENT_USER,"SOFTWARE\\GRISBI","LANG",REG_SZ);
    bFound   = (BOOL)(gzLocale && (*gzLocale));
    
    // ---------------------------------------------------------------------
    // Try to find language setting using the LANG environment variable
    // ---------------------------------------------------------------------
    if(!bFound)
    {
		g_unsetenv("LANG");
        gzLocale = g_getenv("LANG");
        bFound   = (BOOL)(gzLocale && (*gzLocale));
        if (bFound)
        {
            // LANG can contain the Windows LCID instead of the LC_LOCALE value, in this case we need to convert it
            // LANG can the LCID given in decimal
            // LANG can the LCID given is hexadecimal
            // When LANG is the LCID its size is 3 or 4 and start with a digit
            // In other case we suppose LANG to be a valid LC_LOCALE value
            int len = strlen(gzLocale);
            if ( (len==3||len==4)&&(g_ascii_isdigit(*gzLocale))) 
            {
                    gzLocale = win32_lcid_to_lang(atoi(gzLocale));
                    bFound   = (BOOL)(gzLocale && (*gzLocale));
            }
        }
    
    }
    // ---------------------------------------------------------------------
    // Last change: retrieve current Windows value
    // ---------------------------------------------------------------------
    if ( (!bFound) || (!gzLocale) || ( (gzLocale) && (!(*gzLocale)) ) )
    {
        gzLocale = win32_lcid_to_lang(GetUserDefaultLCID());
    }

    if ((gzLocale)&&(*gzLocale))
    {
        bFound = TRUE;
        g_setenv("LANG",gzLocale,TRUE);
        g_free(gzLocale);
        gzLocale = NULL;
    }
    return bFound;
} /* }}} */


#ifdef __cplusplus
}
#endif

