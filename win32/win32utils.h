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
extern HRESULT win32_get_app_data_folder_path     (gchar*, int);
extern gchar*  win32_get_my_documents_folder_path (void);
extern gchar*  win32_get_windows_folder_path      (void);
extern gchar*  win32_get_grisbirc_folder_path     (void);

#endif//!WINUTILS_H_C61461B7_ACF2_4011_888A_030AD5F25F8F

