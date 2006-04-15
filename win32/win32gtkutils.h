/*!
 * \file    win32gtkutils.h
 * \brief   Declaration of some usefull functions to make the GTK API
 *          working with the WIN32 one.
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
#ifndef _H_WIN32GTKUTILS_EACF3E3A_E8AA_463d_8051_09838372ADAB_
#define _H_WIN32GTKUTILS_EACF3E3A_E8AA_463d_8051_09838372ADAB_ (1)

#include <windows.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Make sure any existing GTK Dlls Path is present in the PATH variable */
extern gchar*   win32_strdup_registry(HKEY a_hRoot, TCHAR* a_szKey, TCHAR* a_szValue, DWORD a_dwType);
extern gboolean win32_make_sure_the_gtk2_dlls_path_is_in_PATH();
extern gboolean win32_set_locale();

#ifdef __cplusplus
}
#endif


#endif //!_H_WIN32GTKUTILS_EACF3E3A_E8AA_463d_8051_09838372ADAB_
