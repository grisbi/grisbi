/*!
 * \file    win32libofx.c
 * \brief   Implementation of OFX support  for Windows
 *          
 * \author  Francois Terrot (Francois.terrot@grisbi.org)
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
#include <libofx/libofx.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef int (FNOFXPROCFILE)(int, char **);
typedef FNOFXPROCFILE* PFNOFXPROCFILE;

/**
 * On Windows load ofx_proc_file function from libofx.dll before calling it
 */
int ofx_proc_file(int argc, char *argv[])
{
    int             ret           = -1;
    PFNOFXPROCFILE  pfnOfxProcFile = NULL;
    HMODULE         hLibOfxDll    = LoadLibrary("libofx.dll");
    
    if (hLibOfxDll)
    {
        pfnOfxProcFile = (PFNOFXPROCFILE)GetProcAddress(hLibOfxDll,"ofx_proc_file");
    }
    
    ret = (pfnOfxProcFile) ? (*pfnOfxProcFile)(argc,argv) : -1; 
    
    CloseHandle(hLibOfxDll);
    hLibOfxDll = NULL;
    pfnOfxProcFile = NULL;

    return ret;
}


#ifdef __cplusplus
}
#endif


