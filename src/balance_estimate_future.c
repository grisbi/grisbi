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

/* ./configure --with-balance-estimate */

#include "include.h"
#include <config.h>

#ifdef ENABLE_BALANCE_ESTIMATE

/*START_INCLUDE*/
#include "balance_estimate_future.h"
#include "./balance_estimate_tab.h"
#include "./balance_estimate_config.h"
#include "./balance_estimate_data.h"
#include "./balance_estimate_hist.h"
#include "./utils_dates.h"
#include "./gsb_data_account.h"
#include "./gsb_data_budget.h"
#include "./gsb_calendar_entry.h"
#include "./gsb_data_category.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_fyear.h"
#include "./gsb_real.h"
#include "./gsb_scheduler.h"
#include "./gsb_transactions_list_sort.h"
#include "./main.h"
#include "./mouse.h"
#include "./navigation.h"
#include "./include.h"
#include "./structures.h"
#include "./traitement_variables.h"
#include "./erreur.h"
#include "./utils.h"
/*END_INCLUDE*/


/*START_STATIC*/

/*END_STATIC*/

/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern gchar* bet_duration_array[];
extern GdkColor couleur_fond[0];
extern GdkColor couleur_bet_division;
extern GtkWidget *notebook_general;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_future_create_page ( void )
{
    GtkWidget *notebook;
    GtkWidget *page;
    //~ GtkWidget *widget = NULL;
    //~ GtkWidget *initial_date = NULL;
    //~ GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label;
    //~ GtkWidget *spin_button = NULL;
    //~ GtkWidget *previous = NULL;
    //~ GtkWidget *scrolled_window;
    //~ GtkWidget *tree_view;
    //~ GtkTreeStore *tree_model;
    //~ GtkTreeModel *sortable;
    //~ GtkCellRenderer *cell;
    //~ GtkTreeViewColumn *column;
    //~ gint iduration;

    devel_debug (NULL);
    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");
    page = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_set_name ( page, "bet_future_page" );

    /* create the title */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5) ;

    label = gtk_label_new ("Future data");
    gtk_container_add ( GTK_CONTAINER ( align ), label );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_future_title", label );


    gtk_widget_show_all ( page );

    return page;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_future_configure_form_to_future ( void )
{
    //~ gtk_widget_show (form_scheduled_part);

    return TRUE;
}
/**
 *
 *
 *
 *
 * */
gboolean bet_future_configure_form_to_transaction ( void )
{
    //~ gtk_widget_hide (form_scheduled_part);

    return TRUE;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
