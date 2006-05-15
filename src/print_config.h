/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2006 Benjamin Drieu (bdrieu@april.org)		      */
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

#ifndef PRINT_CONFIG_H
#define PRINT_CONFIG_H

/* Header file for print_config.c */

gboolean print_config ( );
gboolean change_print_to_file ( GtkButton *button, gpointer data );

GtkWidget * print_config_general ( GtkWidget * );
GtkWidget * print_config_paper ( GtkWidget * );
GtkWidget * print_config_appearance ( GtkWidget * );

struct paper_config 
{
    gchar * name;
    gchar * dvips_name;
    gfloat width;
    gfloat height;
};


enum filetype {
  POSTSCRIPT_FILE = 0,
  LATEX_FILE,
  NUM_FILETYPES,
}; 

enum orientation {
  LANDSCAPE = 0,
  PORTRAIT,
  NUM_ORIENTATION,
}; 

struct print_config
{
  gboolean printer;
  gchar * printer_name;
  gchar * printer_filename;
  enum filetype filetype;
  struct paper_config paper_config;
  enum orientation orientation;
};

#endif /* PRINT_CONFIG_H */
