/* ************************************************************************** */
/*                                                                            */
/*                  GtkFileSelection enhancement API                          */
/*                                                                            */
/*   Manages implicit conversion between UTF8 to sysop locale charset :       */
/*      All input string are given in UTF8 to the API which convert them to   */
/*      the good charset, (same for output from locale to UTF8)               */
/*   Add some more convenient ways to access the GtkFileSelection properties  */
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
#define _H_UTILS_FILE_SELECTION_ "$Id: utils_file_selection.h,v 1.1 2004/08/22 20:16:01 teilginn Exp $"

#include <gtk/gtk.h>

void   file_selection_set_entry(GtkFileSelection* filesel,const gchar* utf8string);
gchar* file_selection_get_entry(GtkFileSelection* filesel);
void   file_selection_set_filename(GtkFileSelection* filsel,const gchar* utf8filename);
gchar* file_selection_get_filename(GtkFileSelection* filesel);
gchar** file_selection_get_selections(GtkFileSelection* filesel);
gchar* file_selection_get_last_directory(GtkFileSelection* filesel,gboolean ended);

#endif
