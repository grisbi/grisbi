/*! 
 * \file  utils_file_selections.h
 * \brief  GtkFileSelection enhancement API interface
 */
/* ************************************************************************** */
/*                                                                            */
/*                  GtkFileSelection enhancement API                          */
/*                                                                            */
/*   Manages implicit conversion between UTF8 to sysop locale charset :       */
/*      All input string are given in UTF8 to the API which convert them to   */
/*      the good charset, (same for output from locale to UTF8)               */
/*   Add some more convenient ways to access the GtkFileSelection properties  */
/*   Authorised to hijack the standard GTK file_selection dialog to replace it*/
/*  by the default Windows one                                                */
/*                                                                            */
/*                        file_selection.h                                    */
/*                                                                            */
/*     Copyright (C)	2004- xxxx François Terrot (grisbi@terrot.net)	      */
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
#ifndef _H_UTILS_FILE_SELECTION_
#define _H_UTILS_FILE_SELECTION_ (1)


/*! File selection dialog behaviour properties */
#define FILE_SELECTION_DEFAULT          0x0000  /**< No particular behaviour (open file dialog)*/
#define FILE_SELECTION_IS_OPEN_DIALOG   0x0000  /**< The file selection is used for open operation   */
#define FILE_SELECTION_IS_SAVE_DIALOG   0x0001  /**< The file selection is used for a save operation */
#define FILE_SELECTION_NOOVERWRITECHECK 0x0002  /**< No existance check and overwrite confirmation done - used only for save operations */
#define FILE_SELECTION_MUST_EXIST       0x0002  /**< The selected file name must exists - only for open operation*/
#define FILE_SELECTION_MULTISELECTION   0x0004  /**< \todo allow the multi selection behaviour */

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
gchar* file_selection_get_filename(GtkFileChooser* filesel);
gchar* file_selection_get_last_directory(GtkFileChooser* filesel,gboolean ended) ;
GtkWidget* file_selection_new ( const gchar *title, const gint properties );
/*END_DECLARATION*/

#endif
