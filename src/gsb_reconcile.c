/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2009-2013 Pierre Biava (grisbi@pierre.biava.name)     */
/*          https://www.grisbi.org/                                            */
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

/**
 * \file gsb_reconcile.c
 * functions used to reconcile an account
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <string.h>
#include <ctype.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_reconcile.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_real.h"
#include "gsb_reconcile_list.h"
#include "gsb_rgba.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "transaction_list_sort.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_reconcile_cancel ( GtkWidget *button,
				        gpointer null );
static gboolean gsb_reconcile_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data );
static gboolean gsb_reconcile_finish_reconciliation ( GtkWidget *button,
					    gpointer null );
static void gsb_reconcile_sensitive ( gboolean sensitive );
static gchar *gsb_reconcile_build_label ( int reconcile_number );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget * reconcile_panel;
/*END_EXTERN*/


/* all the widget used to do the reconciliation */
static GtkWidget *reconcile_number_entry;
static GtkWidget *reconcile_initial_balance_entry;
static GtkWidget *reconcile_final_balance_entry;
static GtkWidget *reconcile_new_date_entry;

static GtkWidget *reconcile_last_date_label;
static GtkWidget *reconcile_initial_balance_label;
static GtkWidget *reconcile_final_balance_label;
static GtkWidget *reconcile_marked_balance_label;
static GtkWidget *reconcile_variation_balance_label;
static GtkWidget *reconcile_ok_button;

GtkWidget *reconcile_sort_list_button;

/* previous values from the user, to be restored after the reconciliation */
static gint reconcile_save_rows_number;
static gint reconcile_save_show_marked;
static gint reconcile_save_account_display;

/* backup the number of the last transaction converted into planned transaction during the reconciliation */
static gint reconcile_save_last_scheduled_convert = 0;

/**
 * return the date max of transactions ready to reconcile
 *
 * \param account_number the number of the current account
 *
 * \return the date max of the  transactions
 **/
static const GDate *gsb_reconcile_get_pointed_transactions_max_date (gint account_number)
{
    GSList *list_tmp;
    const GDate *max_date = NULL;

    list_tmp = gsb_data_transaction_get_transactions_list ();

    while (list_tmp)
    {
        gint transaction_number;
        const GDate *transaction_date;

        transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);
        if (gsb_data_transaction_get_account_number (transaction_number) == account_number
			&&
			(gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_POINTEE
			 ||
			 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELEPOINTEE))
		{
            transaction_date = gsb_data_transaction_get_date (transaction_number);
            //~ printf("transaction_date: %s\n", gsb_format_gdate (transaction_date));
            if (max_date == NULL || g_date_compare (transaction_date, max_date) > 0)
			{
                max_date = transaction_date;
                //~ printf("max_date: %s\n", gsb_format_gdate (max_date));
            }
        }
        list_tmp = list_tmp -> next;
    }
    return max_date;
}

/**
 * Create a vbox that contains all controls used to display
 * information related to current reconciliation.
 *
 * \return A newly-allocated frame containing the reconcile widget
 */
GtkWidget *gsb_reconcile_create_box ( void )
{
    GtkWidget *frame, *label, *table, *vbox, *hbox, *button, *separator;
	GtkWidget *sw;

    frame = gtk_frame_new ( NULL );
	sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_NEVER);
	gtk_container_add (GTK_CONTAINER (sw), frame);
	g_object_set_data (G_OBJECT (sw), "reconcile_frame", frame);

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), MARGIN_BOX);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    /* the title of the frame */
    label = gtk_label_new ( NULL );
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    utils_labels_set_alignment ( GTK_LABEL (label), 0.0, 0.0 );
    gtk_frame_set_label_widget ( GTK_FRAME(frame), label);

    /* number of reconcile */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0);

    label = gtk_label_new ( _("Reconciliation reference: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0);

    reconcile_number_entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (reconcile_number_entry), 12);
	gtk_widget_set_hexpand (reconcile_number_entry, TRUE);
    gtk_widget_set_tooltip_text ( GTK_WIDGET (reconcile_number_entry),
								 _("If reconciliation reference ends in a digit, it is "
								   "automatically incremented at each reconciliation.\n"
								   "You can let it empty if you don't want to keep a trace of "
								   "the reconciliation.") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), reconcile_number_entry, FALSE, FALSE, 0);

    separator = gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0);

    /* under the reconcile number, we have a table */
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), MARGIN_BOX);
	gtk_grid_set_row_spacing (GTK_GRID (table), MARGIN_BOX);
    gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 0);

    separator = gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL );
    gtk_grid_attach (GTK_GRID (table), separator, 0, 1, 3, 1 );

    separator = gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL );
    gtk_grid_attach (GTK_GRID (table), separator, 0, 3, 3, 1 );

    separator = gtk_separator_new ( GTK_ORIENTATION_VERTICAL );
	gtk_widget_set_margin_start (separator, MARGIN_BOX);
	gtk_widget_set_margin_end (separator, MARGIN_BOX);
    gtk_grid_attach (GTK_GRID (table), separator, 1, 0, 1, 5 );

    label = gtk_label_new ( _("Date") );
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

    label = gtk_label_new ( _("Balance") );
    gtk_grid_attach (GTK_GRID (table), label, 2, 0, 1, 1);

    reconcile_last_date_label = gtk_label_new ( NULL );
    gtk_grid_attach (GTK_GRID (table), reconcile_last_date_label, 0, 2, 1, 1 );

    /* set the old balance, an entry the first time, will be unsensitive after */
    reconcile_initial_balance_entry = gtk_entry_new ( );
	gtk_entry_set_width_chars (GTK_ENTRY (reconcile_initial_balance_entry), 13);
	gtk_widget_set_hexpand (reconcile_initial_balance_entry, TRUE);
    g_signal_connect ( G_OBJECT ( reconcile_initial_balance_entry ),
                        "changed",
		                G_CALLBACK (gsb_reconcile_update_amounts),
		                NULL );
    g_signal_connect ( G_OBJECT ( reconcile_initial_balance_entry ),
			            "focus-out-event",
                        G_CALLBACK ( gsb_reconcile_entry_lose_focus ),
                        NULL );
    gtk_grid_attach (GTK_GRID (table), reconcile_initial_balance_entry, 2, 2, 1, 1 );

    /* make the new date entry */
    reconcile_new_date_entry = gsb_calendar_entry_new (TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (reconcile_new_date_entry), 13);
	gtk_widget_set_hexpand (reconcile_new_date_entry, TRUE);
    gtk_grid_attach (GTK_GRID (table), reconcile_new_date_entry, 0, 4, 1, 1 );

    /* set the new balance */
    reconcile_final_balance_entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (reconcile_final_balance_entry), 13);
	gtk_widget_set_hexpand (reconcile_final_balance_entry, TRUE);
    g_signal_connect ( G_OBJECT ( reconcile_final_balance_entry ),
                        "changed",
		                G_CALLBACK (gsb_reconcile_update_amounts),
		                NULL );
    g_signal_connect ( G_OBJECT ( reconcile_final_balance_entry ),
			            "focus-out-event",
                        G_CALLBACK ( gsb_reconcile_entry_lose_focus ),
                        NULL );
    gtk_grid_attach (GTK_GRID (table), reconcile_final_balance_entry, 2, 4, 1, 1 );

    /* 2nd table under that, with the balances labels */
    table = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (table), 5);
	gtk_grid_set_column_homogeneous (GTK_GRID (table),TRUE);
    gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 0);

    label = gtk_label_new ( _("Initial balance: ") );
    utils_labels_set_alignment ( GTK_LABEL ( label ), 0, 0.5 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

    reconcile_initial_balance_label = gtk_label_new ( NULL );
    utils_labels_set_alignment ( GTK_LABEL ( reconcile_initial_balance_label ), 1, 0.5 );
    gtk_widget_set_hexpand (reconcile_initial_balance_label, TRUE);
    gtk_widget_set_halign (reconcile_initial_balance_label, GTK_ALIGN_END);
    gtk_grid_attach (GTK_GRID (table), reconcile_initial_balance_label, 1, 0, 1, 1);

    label = gtk_label_new ( _("Final balance: ") );
    utils_labels_set_alignment ( GTK_LABEL ( label ), 0, 0.5 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

    reconcile_final_balance_label = gtk_label_new ( NULL );
    utils_labels_set_alignment ( GTK_LABEL ( reconcile_final_balance_label ), 1, 0.5 );
    gtk_widget_set_halign (reconcile_final_balance_label, GTK_ALIGN_END);
    gtk_grid_attach (GTK_GRID (table), reconcile_final_balance_label, 1, 1, 1, 1);

    label = gtk_label_new ( _("Checking: ") );
    utils_labels_set_alignment ( GTK_LABEL ( label ), 0, 0.5 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);

    reconcile_marked_balance_label = gtk_label_new ( NULL );
    utils_labels_set_alignment ( GTK_LABEL ( reconcile_marked_balance_label ), 1, 0.5 );
    gtk_widget_set_halign (reconcile_marked_balance_label, GTK_ALIGN_END);
    gtk_grid_attach (GTK_GRID (table), reconcile_marked_balance_label, 1, 2, 1, 1);

    separator = gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL );
    gtk_grid_attach (GTK_GRID (table), separator, 0, 3, 2, 1);

    label = gtk_label_new ( _("Variance: ") );
    utils_labels_set_alignment ( GTK_LABEL ( label ), 0, 0.5 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 4, 1, 1);

    reconcile_variation_balance_label = gtk_label_new ( NULL );
    utils_labels_set_alignment ( GTK_LABEL ( reconcile_variation_balance_label ), 1, 0.5 );
    gtk_grid_attach (GTK_GRID (table), reconcile_variation_balance_label, 1, 4, 1, 1);

    /* set the button to sort with the method of paymen */
    separator = gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0);

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_set_homogeneous ( GTK_BOX ( hbox ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0);

    reconcile_sort_list_button = gtk_check_button_new_with_label (_("Sort by method of payment"));
    gtk_button_set_relief ( GTK_BUTTON (reconcile_sort_list_button), GTK_RELIEF_NONE);
    g_signal_connect ( G_OBJECT (reconcile_sort_list_button), "clicked",
		       G_CALLBACK (gsb_reconcile_list_button_clicked), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), reconcile_sort_list_button, FALSE, FALSE, 0);

    /* make the buttons */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_set_homogeneous ( GTK_BOX ( hbox ), TRUE );
    gtk_box_pack_end ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0);

    button = utils_buttons_button_new_from_stock ("gtk-cancel", _("_Cancel"));
    gtk_button_set_relief ( GTK_BUTTON ( button), GTK_RELIEF_NONE);
    g_signal_connect ( G_OBJECT (button), "clicked",
		       G_CALLBACK (gsb_reconcile_cancel), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0);

    reconcile_ok_button = utils_buttons_button_new_from_stock ("gtk-ok", _("V_alidate"));
    gtk_button_set_relief ( GTK_BUTTON ( reconcile_ok_button), GTK_RELIEF_NONE);
    g_signal_connect ( G_OBJECT ( reconcile_ok_button ), "clicked",
		       G_CALLBACK (gsb_reconcile_finish_reconciliation), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), reconcile_ok_button, FALSE, FALSE, 0);

    separator = gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL );
    gtk_box_pack_end ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0);

    gtk_widget_show_all ( sw );

    return ( sw );
}


/**
 * Build the new label for the reconciliation, given the old one.
 * The expected format is NAME+NUMBER, so this function returns
 * a newly allocated string whose format is NAME+(NUMBER+1). It
 * preserves leading '0' for the NUMBER string.
 *
 * If this is the first label building (first reconciliation for
 * this account), then the function returns a standard string
 * of the account name (lower case) + '-1'.
 *
 * \param reconcile_number
 *
 * \return the new string label
 */
gchar *gsb_reconcile_build_label ( int reconcile_number )
{
    gchar *tmp;
    gchar *old_label;
    gchar *new_label;
    gchar format[6] = "%s%0d";
    int __reconcile_number;
    size_t __size;
    int __expand;

    /* old_label = NAME + NUMBER */
    old_label = g_strdup ( gsb_data_reconcile_get_name ( reconcile_number ) );

    /* return account NAME + '1' */
    if ( !old_label )
    {
        tmp = gsb_data_account_get_name ( gsb_gui_navigation_get_current_account () );
        new_label = g_strconcat ( tmp, "-1", NULL );
        tmp = new_label;
        while ( *tmp != '\0' )
        {
            if ( *tmp == ' ' )
                *tmp = '-';
            else
                *tmp = tolower ( *tmp );
            tmp ++;
        }
        return new_label;
    }

    /* we try to find some digits at the end of the name,
     * if found, get the biggest number until we find a non digit character */
    __expand = 1;
    tmp = old_label + ( strlen ( old_label ) - 1 ) * sizeof ( gchar );

    while ( tmp >= old_label )
    {
        if ( !isdigit ( tmp[0] ) )
            break;

        if ( *tmp != '9' )
            __expand = 0;
        tmp--;
    }

    tmp ++; /* step forward to the first digit */

    __reconcile_number = utils_str_atoi ( tmp ) + 1;

    /* if stage 99 -> 100 for example,
     * then we have to allocate one more byte */
    __size = strlen ( tmp ) + __expand;
    /* format string for the output (according NUMBER string length) */
    format[3] = 48 + __size;

    /* close the NAME string */
    *tmp = 0;
    /* NAME + NUMBER + '\0' */
    __size += strlen ( old_label ) * sizeof ( gchar ) + 1;
    new_label = g_malloc0 ( __size * sizeof ( gchar ) );
    sprintf ( new_label, format, old_label, __reconcile_number );

    /* replace ' ' by '0' in number */
    tmp = new_label + strlen ( old_label ) * sizeof ( gchar );
    while ( *tmp == ' ' )
        *tmp++ = '0';

    g_free ( old_label );

    return new_label;
}


/**
 * start the reconciliation, called by a click on the
 * reconcile button
 *
 * \param button the button we click to come here
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_run_reconciliation ( GtkWidget *button,
                        gpointer null )
{
	GtkWidget *reconcile_frame;
    GDate *date;
    gint account_number;
    gint reconcile_number;
    gchar *label;
    gchar *string;
    gchar* tmpstr;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
    account_number = gsb_gui_navigation_get_current_account ();
    reconcile_number = gsb_data_reconcile_get_account_last_number (account_number);

    label = gsb_reconcile_build_label ( reconcile_number );
    gtk_entry_set_text ( GTK_ENTRY ( reconcile_number_entry ), label );
    g_free ( label );

    /* reset records in run structure if user has changed of account */
    if (run.reconcile_account_number != account_number)
    {
        g_free (run.reconcile_final_balance);
        if (run.reconcile_new_date)
            g_date_free (run.reconcile_new_date);
        run.reconcile_final_balance = NULL;
        run.reconcile_new_date = NULL;
        run.reconcile_account_number = -1;
    }

    /* set last input date/amount if available */
    if (run.reconcile_new_date)
    {
        date = run.reconcile_new_date;
    }
    else
    {
        /* increase the last date of 1 month */
        date = gsb_date_copy (gsb_data_reconcile_get_final_date (reconcile_number));
        if (date)
        {
            GDate *today;

			string = gsb_format_gdate ( date );
            gtk_label_set_text ( GTK_LABEL ( reconcile_last_date_label ), string);
            gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_last_date_label ), FALSE );
			gtk_widget_set_name (reconcile_last_date_label, "reconcile_old_entry_insensitive");

            g_free (string);
            g_date_add_months ( date, 1 );

            /* if w_etat->reconcile_end_date or the new date is after today, set today */
            today = gdate_today();
            if ( w_etat->reconcile_end_date || g_date_compare ( date, today) > 0 )
            {
                g_date_free (date);
                date = gdate_today();
            }
            else
                g_date_free (today);

            /* it's not the first reconciliation, set the old balance and unsensitive the old balance entry */
            tmpstr = utils_real_get_string (gsb_data_reconcile_get_final_balance (reconcile_number));
            gtk_entry_set_text ( GTK_ENTRY ( reconcile_initial_balance_entry ), tmpstr);
            g_free ( tmpstr );
			gtk_widget_set_name (reconcile_initial_balance_entry, "reconcile_old_entry_insensitive");
            gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_initial_balance_entry ), FALSE );
        }
        else
        {
            gtk_label_set_text ( GTK_LABEL ( reconcile_last_date_label ), _("None") );

            date = gdate_today();

            /* it's the first reconciliation, set the initial balance and make sensitive the old balance to change
             * it if necessary */
            tmpstr = utils_real_get_string ( gsb_data_account_get_init_balance (account_number, -1));
            gtk_entry_set_text ( GTK_ENTRY ( reconcile_initial_balance_entry ), tmpstr);
            g_free ( tmpstr );
			gtk_widget_set_name (reconcile_initial_balance_entry, "reconcile_old_entry");
            gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_initial_balance_entry ), TRUE );
        }
    }

    string = gsb_format_gdate (date);
    gtk_entry_set_text ( GTK_ENTRY ( reconcile_new_date_entry ), string );
    g_free (string);
    g_date_free (date);

    /* set last input amount if available and if the account is the good one */
    gtk_entry_set_text ( GTK_ENTRY ( reconcile_final_balance_entry ),
            (run.reconcile_final_balance) ? run.reconcile_final_balance : "");
    g_free(run.reconcile_final_balance);

    /* set the title */
	reconcile_frame = g_object_get_data (G_OBJECT (reconcile_panel), "reconcile_frame");
    tmpstr = g_markup_printf_escaped ( _(" <b>%s reconciliation</b> "),
					     gsb_data_account_get_name (account_number));
    gtk_label_set_markup ( GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (reconcile_frame))), tmpstr );
    g_free ( tmpstr );

    /* we go to the reconciliation mode */
    run.equilibrage = 1;

    /* set all the balances for reconciliation */
    gsb_reconcile_update_amounts (NULL, NULL);

    /* set the transactions list to reconciliation mode */
    /* only change the current account */
    reconcile_save_account_display = w_etat->retient_affichage_par_compte;
    w_etat->retient_affichage_par_compte = 1;

    /* hide the marked R transactions */
    reconcile_save_show_marked = gsb_data_account_get_r (account_number);
    if (reconcile_save_show_marked)
    {
        gsb_data_account_set_r (account_number, FALSE );
        gsb_transactions_list_mise_a_jour_affichage_r (FALSE);
    }

    /* 1 line on the transaction list */
    reconcile_save_rows_number = gsb_data_account_get_nb_rows (account_number);
    if (reconcile_save_rows_number != 1)
        gsb_transactions_list_set_visible_rows_number ( 1 );

    /* sort by method of payment if in conf */
    if (gsb_data_account_get_reconcile_sort_type (account_number))
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (reconcile_sort_list_button),
				       TRUE );

    gtk_widget_show_all ( reconcile_panel );

    transaction_list_show_toggle_mark (TRUE);

    /* unsensitive all that could change the account number */
    gsb_reconcile_sensitive (FALSE);

    gtk_widget_grab_focus ( GTK_WIDGET ( reconcile_number_entry ) );
    return FALSE;
}



/**
 * finish the reconciliation,
 * called by a click on the finish button
 *
 * \param button
 * \param null
 *
 * \return FALSE
 */
gboolean gsb_reconcile_finish_reconciliation ( GtkWidget *button,
					    gpointer null )
{
	GtkWidget *label_last_statement;
    GSList *list_tmp_transactions;
    GDate *date;
    const GDate *initial_date;
	const GDate *transactions_max_date;
    gint account_number;
    gint reconcile_number;
    GsbReal real;
	gchar *tmp_str;
	gchar *tmp_date;

    account_number = gsb_gui_navigation_get_current_account ();

    if ( gsb_real_sub ( gsb_real_add ( utils_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( reconcile_initial_balance_entry ))),
				       gsb_data_account_calculate_waiting_marked_balance (account_number)),
			utils_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( reconcile_final_balance_entry )))).mantissa != 0 )
    {
	dialogue_warning_hint ( _("There is a variance in balances, check that both final balance and initial balance minus marked transactions are equal."),
				_("Reconciliation can't be completed.") );
	return FALSE;
    }

    /* get and check the reconcile name */
    reconcile_number = gsb_data_reconcile_get_number_by_name (gtk_entry_get_text ( GTK_ENTRY ( reconcile_number_entry )));
    if (reconcile_number)
    {
	dialogue_warning_hint ( _("There is already a reconciliation with that "
                        "name, you must use another name or let it free.\nIf the "
                        "reconcile name is ending by a number,\n"
                        "it will be automatically incremented."),
                        _("Reconciliation can't be completed.") );
	return FALSE;
    }


    /* get and save the date */
    date = gsb_calendar_entry_get_date (reconcile_new_date_entry);
    if (!date)
    {
    tmp_str = g_strdup_printf ( _("Invalid date: '%s'"),
						  gtk_entry_get_text ( GTK_ENTRY ( reconcile_new_date_entry )));
	dialogue_warning_hint ( tmp_str,
				_("Reconciliation can't be completed.") );
	g_free ( tmp_str );
	return FALSE;
    }

    /* teste si la date de fin inclue toutes les transactions pointées */
    transactions_max_date = gsb_reconcile_get_pointed_transactions_max_date (account_number);
    if (g_date_compare (transactions_max_date, date) > 0)
    {
		if (!dialogue_yes_no (_("There are pointed transactions that occur later than "
								"the reconciliation end date.\n"
							    "If you continue these transactions will be ignored.\n\n"
								"Do you want to continue?"),
							  _("Warning: the reconciliation may be incomplete!"),
							  GTK_RESPONSE_NO))

        return FALSE;
    }

/* teste la validité de la date de fin */
    reconcile_number = gsb_data_reconcile_get_account_last_number ( account_number );
    if ( reconcile_number )
    {
        initial_date = gsb_data_reconcile_get_final_date ( reconcile_number );
        if ( g_date_compare ( initial_date, date ) >= 0 )
        {
			tmp_date = gsb_format_gdate ( date );
            tmp_str = g_strdup_printf ( _("Invalid date: '%s'"), tmp_date );
			g_free(tmp_date);
            dialogue_warning_hint ( tmp_str, _("Reconciliation can't be completed.") );
            g_free ( tmp_str );

            return FALSE;
        }
    }
    else
    {
        tmp_str = g_strdup ( _("You can set the initial date of the reconciliation in the preferences.") );
        dialogue_warning ( tmp_str );
        g_free ( tmp_str );
    }

    if (!strlen (gtk_entry_get_text ( GTK_ENTRY ( reconcile_number_entry ))))
    {
	dialogue_warning_hint ( _("You need to set a name to the reconciliation ; at least, set a number,\nit will be automatically incremented later"),
				_("Reconciliation can't be completed.") );
	return FALSE;
    }

    /* restore the good sort of the list */
    if (transaction_list_sort_get_reconcile_sort ())
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (reconcile_sort_list_button),
				       FALSE );
	gsb_reconcile_list_button_clicked (reconcile_sort_list_button, NULL);
    }

	label_last_statement = grisbi_win_get_label_last_statement ();
	tmp_date = gsb_format_gdate (date);
    tmp_str = g_strdup_printf ( _("Last statement: %s"), tmp_date);
	g_free(tmp_date);
    gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			 tmp_str);
    g_free ( tmp_str );

    /* create the new reconcile structure */
    reconcile_number = gsb_data_reconcile_new (gtk_entry_get_text (GTK_ENTRY (reconcile_number_entry)));
    gsb_data_reconcile_set_account ( reconcile_number, account_number );

    /* set the variables of the reconcile */
    gsb_data_reconcile_set_final_date ( reconcile_number, date );
    g_date_free (date);

    date = gsb_parse_date_string (gtk_label_get_text (GTK_LABEL (reconcile_last_date_label)));
    gsb_data_reconcile_set_init_date ( reconcile_number, date );
    g_free (date);

    real = utils_real_get_from_string ( gtk_entry_get_text (
                        GTK_ENTRY ( reconcile_initial_balance_entry ) ) );
    gsb_data_reconcile_set_init_balance ( reconcile_number, real );

    real = utils_real_get_from_string ( gtk_entry_get_text (
                        GTK_ENTRY ( reconcile_final_balance_entry ) ) );
    gsb_data_reconcile_set_final_balance ( reconcile_number,
					   real );

    /* modify the reconciled transactions */
    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number
	     &&
	     ( gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_POINTEE
	       ||
	       gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_TELEPOINTEE ))
	{
	    gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
							  OPERATION_RAPPROCHEE );
	    gsb_data_transaction_set_reconcile_number ( transaction_number_tmp,
							reconcile_number );
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }

    /* update the P and T to R in the list */
    transaction_list_update_element (ELEMENT_MARK);

    /* update the reconcile number if necessary */
    transaction_list_update_element ( ELEMENT_RECONCILE_NB );

    run.mise_a_jour_liste_comptes_accueil = TRUE;

    /* go back to the normal transactions list */
    gsb_reconcile_cancel (NULL, NULL);

    /* reset records in run: to do after gsb_reconcile_cancel */
    g_free (run.reconcile_final_balance);
    if (run.reconcile_new_date)
        g_date_free (run.reconcile_new_date);
    run.reconcile_final_balance = NULL;
    run.reconcile_new_date = NULL;
    run.reconcile_account_number = -1;

    gsb_file_set_modified ( TRUE );

    if ( reconcile_save_last_scheduled_convert )
    {
        gsb_gui_navigation_set_selection ( GSB_SCHEDULER_PAGE, 0, 0);
        gsb_scheduler_list_select ( reconcile_save_last_scheduled_convert );
        gsb_scheduler_list_edit_transaction ( reconcile_save_last_scheduled_convert );
        reconcile_save_last_scheduled_convert = 0;
    }

    return FALSE;
}


/**
 * make sensitive or unsensitive all that could change the account
 * while we are reconciling
 *
 * \param sensitive TRUE or FALSE
 *
 * \return
 * */
void gsb_reconcile_sensitive ( gboolean sensitive )
{
    gtk_widget_set_sensitive ( gsb_gui_navigation_get_tree_view ( ), sensitive );
    grisbi_win_headings_sensitive_headings (sensitive);
}


/**
 * come here to cancel the reconciliation
 * come here too at the end of the finish reconcile to set the default values
 *
 * \param button not used
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_cancel ( GtkWidget *button,
				        gpointer null )
{
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
    run.equilibrage = 0;

    /* save the final balance/new date for the next time the user will try to reconcile */
    run.reconcile_account_number = gsb_gui_navigation_get_current_account ();
    run.reconcile_final_balance = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( reconcile_final_balance_entry ) ) );
    run.reconcile_new_date = gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY ( reconcile_new_date_entry ) ) );

    /* set the normal color to the date entry */
    gsb_calendar_entry_set_color ( reconcile_new_date_entry, TRUE);

    /* restore the good sort of the list */
    if (transaction_list_sort_get_reconcile_sort ())
    {
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (reconcile_sort_list_button),
                           FALSE );
        gsb_reconcile_list_button_clicked (reconcile_sort_list_button, NULL);
    }

    transaction_list_show_toggle_mark (FALSE);

    /* restore the saved data */
    w_etat->retient_affichage_par_compte = reconcile_save_account_display;

    gsb_transactions_list_set_visible_rows_number ( reconcile_save_rows_number );

    if (reconcile_save_show_marked)
    {
        gsb_data_account_set_r (gsb_gui_navigation_get_current_account (), TRUE );
        gsb_transactions_list_mise_a_jour_affichage_r (TRUE);
    }

    /* Don't display uneeded widget for now. */
    gtk_widget_hide ( reconcile_panel );
    gsb_reconcile_sensitive (TRUE);

    return FALSE;
}


/**
 * update the labels according to the value in the account structure and the entries
 * so all the calculs must have been done before (for marked transactions)
 *
 * \param entry not used
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_update_amounts ( GtkWidget *entry,
					    gpointer null )
{
    GsbReal amount;
    gint account_number;
    gint currency_number;
    const gchar *initial_balance;
    const gchar *final_balance;
    gchar *tmp_string;
	gchar* tmpstr;
    gboolean valide = FALSE;

    /* first get the current account number */
    account_number = gsb_gui_navigation_get_current_account ();

    /* fill the labels corresponding to the balances */
    initial_balance = gtk_entry_get_text ( GTK_ENTRY (reconcile_initial_balance_entry) );
    gtk_label_set_text ( GTK_LABEL ( reconcile_initial_balance_label ), initial_balance );

    if ( entry )
    {
        valide = gsb_form_widget_get_valide_amout_entry (
                        gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
        if ( valide )
        {
            /* the entry is valid, make it normal */
        gtk_widget_set_name (entry, "form_entry");
        }
        else
        {
            /* the entry is not valid, make it red */
        gtk_widget_set_name (entry, "form_entry_error");
            return FALSE;
        }
    }

    currency_number = gsb_data_account_get_currency ( account_number );
    amount = utils_real_get_calculate_entry ( reconcile_final_balance_entry );
    final_balance = utils_real_get_string_with_currency ( amount, currency_number, FALSE );
    gtk_label_set_text ( GTK_LABEL ( reconcile_final_balance_label ), final_balance );

    /* set the marked balance amount,
     * this is what we mark as P while reconciling, so it's the total marked balance
     * - the initial marked balance */
    tmp_string = utils_real_get_string_with_currency (
                        gsb_data_account_calculate_waiting_marked_balance ( account_number ),
                        currency_number,
                        FALSE );
    gtk_label_set_text ( GTK_LABEL ( reconcile_marked_balance_label ), tmp_string );
    g_free (tmp_string);

    /* calculate the variation balance and show it */
    amount = gsb_real_sub ( gsb_real_add (
                        utils_real_get_from_string ( initial_balance ),
					    gsb_data_account_calculate_waiting_marked_balance ( account_number ) ),
			            utils_real_get_from_string ( final_balance ) );

    tmpstr = utils_real_get_string_with_currency ( amount, currency_number, FALSE );
    gtk_label_set_text ( GTK_LABEL ( reconcile_variation_balance_label ), tmpstr);
    g_free ( tmpstr );

    if ( amount.mantissa )
	    gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_ok_button ), FALSE );
    else
	    gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_ok_button ), TRUE );
    return FALSE;
}


gboolean gsb_reconcile_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data )
{
    gsb_form_check_auto_separator ( entry );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean gsb_reconcile_set_last_scheduled_transaction ( gint scheduled_transaction )
{
    reconcile_save_last_scheduled_convert = scheduled_transaction;

    return FALSE;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
