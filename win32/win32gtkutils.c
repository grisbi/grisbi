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

#include "win32gtkutils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HKLM_GTK_20         "SOFTWARE\\GTK\\2.0"    /*!< GTK2 information location is the registry subkeys tree */
#define GTK_DLL_PATH_KEY    "DllPath"               /*!< "GTK Dlls Path" location registry entry name */

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
   dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCSTR)(HKLM_GTK_20), 0, KEY_READ, &hKey);
   if (dwStatus == NO_ERROR)
   {
       // Get the size of the key to allocate the correct size for the temporary pBuffer
       dwStatus = RegQueryValueEx(hKey,(LPCSTR)(GTK_DLL_PATH_KEY),0,&dwType,(LPBYTE)pKeyValBuffer,&dwKeyValSize);
       if (dwStatus == NO_ERROR)
       {
           // allocate the buffer and don't forget the '\0'
           pKeyValBuffer = (char*)malloc(sizeof(char*)*(dwKeyValSize+1));
           memset(pKeyValBuffer,0,(sizeof(char*)*(dwKeyValSize+1)));

           // At last, ... read the value ...
           dwStatus = RegQueryValueEx(hKey,(LPCSTR)(GTK_DLL_PATH_KEY),0,&dwType,(LPBYTE)pKeyValBuffer,&dwKeyValSize);
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

#ifdef __cplusplus
}
#endif

