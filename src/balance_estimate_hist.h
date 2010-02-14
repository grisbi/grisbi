/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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

#ifndef _BALANCE_ESTIMATE_HIST_H
#define _BALANCE_ESTIMATE_HIST_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

enum bet_historical_data_columns {
    SPP_HISTORICAL_SELECT_COLUMN,
    SPP_HISTORICAL_DESC_COLUMN,
    SPP_HISTORICAL_PERIOD_COLUMN,
    SPP_HISTORICAL_BALANCE_COLUMN,
    SPP_HISTORICAL_AVERAGE_COLUMN,
    SPP_HISTORICAL_RETAINED_COLUMN,
    SPP_HISTORICAL_RETAINED_AMOUNT, /* retenaid column without currency */
    SPP_HISTORICAL_BALANCE_COLOR,
    SPP_HISTORICAL_NUM_COLUMNS
};

/* START_DECLARATION */
gboolean bet_historical_affiche_div ( GHashTable  *list_div,
                        GtkWidget *tree_view );
void bet_historical_create_page ( GtkWidget *notebook );
void bet_historical_origin_data_clicked ( GtkWidget *togglebutton, gpointer data );
gint bet_historical_get_fyear_from_combobox ( GtkWidget *combo_box );
void bet_historical_populate_data ( void );
void bet_historical_refresh_data ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max );
void bet_historical_fyear_clicked ( GtkWidget *combo, gpointer data );
/* END_DECLARATION */

#endif /*_BALANCE_ESTIMATE_HIST_H*/

