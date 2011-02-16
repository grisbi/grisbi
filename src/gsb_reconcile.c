/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009-2010 Pierre Biava (grisbi@pierre.biava.name)     */
/* 			http://www.grisbi.org				                              */
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
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_reconcile.h"
#include "dialog.h"
#include "utils_dates.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "navigation.h"
#include "fenetre_principale.h"
#include "gsb_real.h"
#include "gsb_reconcile_list.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "utils_editables.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "transaction_list.h"
#include "transaction_list_sort.h"
#include "structures.h"
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
/*END_STATIC*/

/*START_EXTERN*/
extern GdkColor calendar_entry_color;
extern GtkWidget *label_last_statement;
extern gint mise_a_jour_liste_comptes_accueil;
extern GtkWidget * navigation_tree_view;
extern GtkWidget * reconcile_panel;
extern GtkUIManager * ui_manager;
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
 * Create a vbox that contains all controls used to display
 * information related to current reconciliation.
 *
 * \return A newly-allocated frame containing the reconcile widget
 */
GtkWidget *gsb_reconcile_create_box ( void )
{
    GtkWidget *frame, *label, *table, *vbox, *hbox, *button, *separator;

    frame = gtk_frame_new ( NULL );
    vbox = gtk_vbox_new ( FALSE, 3 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 3 );
    gtk_container_add ( GTK_CONTAINER ( frame ), vbox );

    /* the title of the frame */ 
    label = gtk_label_new ( NULL );
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment ( GTK_MISC (label), 0.0, 0.0 );
    gtk_frame_set_label_widget ( GTK_FRAME(frame), label);

    /* number of reconcile */ 
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0);

    label = gtk_label_new ( _("Reconciliation reference: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0);

    reconcile_number_entry = gtk_entry_new ();
    gtk_widget_set_tooltip_text ( GTK_WIDGET (reconcile_number_entry),
                            _("If reconciliation reference ends in a digit, it is "
                            "automatically incremented at each reconciliation.\n"
                            "You can let it empty if you don't want to keep a trace of "
                            "the reconciliation.") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), reconcile_number_entry, TRUE, TRUE, 0);

    separator = gtk_hseparator_new();
    gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0);

    /* under the reconcile number, we have a table */
    table = gtk_table_new ( 3, 5, FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 3 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 0);

    separator = gtk_hseparator_new();
    gtk_table_attach_defaults ( GTK_TABLE ( table ), separator, 0, 3, 1, 2 );

    separator = gtk_hseparator_new();
    gtk_table_attach_defaults ( GTK_TABLE ( table ), separator, 0, 3, 3, 4 );

    separator = gtk_vseparator_new ();
    gtk_table_attach_defaults ( GTK_TABLE ( table ), separator, 1, 2, 0, 5 );

    label = gtk_label_new ( _("Date") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, 0, 1);

    label = gtk_label_new ( _("Balance") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 2, 3, 0, 1);

    reconcile_last_date_label = gtk_label_new ( NULL );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_last_date_label,
				0, 1, 2, 3 );

    /* set the old balance,
     * an entry the first time, will be unsensitive after */
    reconcile_initial_balance_entry = gtk_entry_new ( );
    gtk_widget_set_size_request ( reconcile_initial_balance_entry, 50, -1 );
    g_signal_connect ( G_OBJECT ( reconcile_initial_balance_entry ),
                        "changed",
		                G_CALLBACK (gsb_reconcile_update_amounts),
		                NULL );
    g_signal_connect ( G_OBJECT ( reconcile_initial_balance_entry ),
			            "focus-out-event",
                        G_CALLBACK ( gsb_reconcile_entry_lose_focus ),
                        NULL );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_initial_balance_entry,
				2, 3, 2, 3 );

    /* make the new date entry */
    reconcile_new_date_entry = gsb_calendar_entry_new (TRUE);
    gtk_widget_set_size_request ( reconcile_new_date_entry, 50, -1 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_new_date_entry,
				0, 1, 4, 5 );

    /* set the new balance */
    reconcile_final_balance_entry = gtk_entry_new ();
    gtk_widget_set_size_request ( reconcile_final_balance_entry, 50, -1 );
    g_signal_connect ( G_OBJECT ( reconcile_final_balance_entry ),
                        "changed",
		                G_CALLBACK (gsb_reconcile_update_amounts),
		                NULL );
    g_signal_connect ( G_OBJECT ( reconcile_final_balance_entry ),
			            "focus-out-event",
                        G_CALLBACK ( gsb_reconcile_entry_lose_focus ),
                        NULL );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_final_balance_entry,
				2, 3, 4, 5 );

    separator = gtk_hseparator_new();
    gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0);


    /* 2nd table under that, with the balances labels */ 
    table = gtk_table_new ( 5, 2, FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 0);

    label = gtk_label_new ( _("Initial balance: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, 0, 1);

    reconcile_initial_balance_label = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC ( reconcile_initial_balance_label ), 1, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_initial_balance_label, 1, 2, 0, 1);

    label = gtk_label_new ( _("Final balance: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, 1, 2);

    reconcile_final_balance_label = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC ( reconcile_final_balance_label ), 1, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_final_balance_label, 1, 2, 1, 2);

    label = gtk_label_new ( _("Checking: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, 2, 3);

    reconcile_marked_balance_label = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC ( reconcile_marked_balance_label ), 1, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_marked_balance_label, 1, 2, 2, 3);

    separator = gtk_hseparator_new();
    gtk_table_attach_defaults ( GTK_TABLE ( table ), separator, 0, 2, 3, 4);

    label = gtk_label_new ( _("Variance: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, 4, 5);

    reconcile_variation_balance_label = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC ( reconcile_variation_balance_label ), 1, 0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), reconcile_variation_balance_label, 1, 2, 4, 5);

    /* set the button to sort with the method of paymen */
    separator = gtk_hseparator_new();
    gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0);

    hbox = gtk_hbox_new ( TRUE, 3 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0);

    reconcile_sort_list_button = gtk_check_button_new_with_label (_("Sort by method of payment"));
    gtk_button_set_relief ( GTK_BUTTON (reconcile_sort_list_button), GTK_RELIEF_NONE);
    g_signal_connect ( G_OBJECT (reconcile_sort_list_button), "clicked",
		       G_CALLBACK (gsb_reconcile_list_button_clicked), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), reconcile_sort_list_button, FALSE, FALSE, 0);

    /* make the buttons */
    hbox = gtk_hbox_new ( TRUE, 3 );
    gtk_box_pack_end ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0);

    button = gtk_button_new_from_stock ( GTK_STOCK_CANCEL );
    gtk_button_set_relief ( GTK_BUTTON ( button), GTK_RELIEF_NONE);
    g_signal_connect ( G_OBJECT (button), "clicked",
		       G_CALLBACK (gsb_reconcile_cancel), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0);

    reconcile_ok_button = gtk_button_new_from_stock ( GTK_STOCK_OK );
    gtk_button_set_relief ( GTK_BUTTON ( reconcile_ok_button), GTK_RELIEF_NONE);
    g_signal_connect ( G_OBJECT ( reconcile_ok_button ), "clicked",
		       G_CALLBACK (gsb_reconcile_finish_reconciliation), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), reconcile_ok_button, FALSE, FALSE, 0);

    separator = gtk_hseparator_new();
    gtk_box_pack_end ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0);

    gtk_widget_show_all ( frame ); 

    return ( frame );
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
    GDate *date;
    gint account_number;
    gint reconcile_number;
    gchar *last_name;
    gchar *string;
	gchar* tmpstr;

    account_number = gsb_gui_navigation_get_current_account ();

    reconcile_number = gsb_data_reconcile_get_account_last_number (account_number);

    /* get the last reconcile number and try to increase the number in the name */

    last_name = my_strdup (gsb_data_reconcile_get_name (reconcile_number));

    if (last_name)
    {
	gchar *tmp_pointer;
	gchar *end_pointer;

	/* we try to find some digits at the end of the name,
	 * if found, get the biggest number untill we find a non digit character */
	end_pointer = last_name + (strlen ( last_name ) - 1) * sizeof (gchar);
	tmp_pointer = end_pointer;

	while ( isdigit ( tmp_pointer[0] ) && tmp_pointer >= last_name )
	    tmp_pointer--;

	if ( tmp_pointer != end_pointer )
	{
	    /* ok we have some digits at the end of the name */
	    gchar *zero_string;
	    gint digit_size;
	    gint new_digit_size;
	    gchar *new_string;
	    gchar *digit_string;
		gchar* oldstr;

	    /* tmp_pointer is on the first non digit from the end of the last_name,
	     * so go to the first digit */
	    tmp_pointer++;

	    digit_string = my_strdup ( tmp_pointer );
	    tmp_pointer[0] = 0;

	    /* increase the number */
	    digit_size = strlen ( digit_string );
	    oldstr =  digit_string;
	    digit_string = utils_str_itoa ( utils_str_atoi ( digit_string ) + 1 );
	    g_free ( oldstr );
	    new_digit_size = strlen ( digit_string );

	    /* if new_digit_size is < of digit_size, it's because some 0 diseappear
	     * while the atoi and utils_str_itoa, so we set again that 0
	     * ie if we had 0007, we want 0008 and no 8 */
	    if ( new_digit_size < digit_size )
	    {
		gint i;

		zero_string = g_malloc ((digit_size-new_digit_size+1)*sizeof (gchar));

		for ( i=0 ; i<digit_size-new_digit_size ; i++ )
		    zero_string[i]='0';

		zero_string[digit_size-new_digit_size] = 0;
	    }
	    else
		zero_string = g_strdup ("");

	    /* create the new string */
	    new_string = g_strconcat ( last_name,
				       zero_string,
				       digit_string,
				       NULL );
	    g_free (last_name);
	    g_free (zero_string);
	    g_free (digit_string);
	    last_name = new_string;
	}
	gtk_entry_set_text ( GTK_ENTRY ( reconcile_number_entry ),
			     last_name );
	g_free (last_name);
    }
    else
    {
        tmpstr = utils_str_itoa ( gsb_data_reconcile_max_number ( ) + 1 );
        gtk_entry_set_text ( GTK_ENTRY ( reconcile_number_entry ), tmpstr );
    }

    /* reset records in etat if user has changed of account */
    if (etat.reconcile_account_number != account_number)
    {
        if (etat.reconcile_final_balance) g_free (etat.reconcile_final_balance);
        if (etat.reconcile_new_date) g_date_free (etat.reconcile_new_date);
        etat.reconcile_final_balance = NULL;
        etat.reconcile_new_date = NULL;
        etat.reconcile_account_number = -1;
    }

    /* set last input date/amount if available */
    if (etat.reconcile_new_date)
    {
        date = etat.reconcile_new_date;
    }
    else
    {
        /* increase the last date of 1 month */
        date = gsb_date_copy (gsb_data_reconcile_get_final_date (reconcile_number));
        if (date)
        {
            GDate *today;
            gchar *string ;

            string = gsb_format_gdate ( date );
            gtk_label_set_text ( GTK_LABEL ( reconcile_last_date_label ),
                    string);
            gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_last_date_label ),
                    FALSE );
            g_free (string);
            g_date_add_months ( date, 1 );

            /* if etat.reconcile_end_date or the new date is after today, set today */
            today = gdate_today();
            if ( etat.reconcile_end_date || g_date_compare ( date, today) > 0 )
            {
                g_date_free (date);
                date = gdate_today();
            }
            else
                g_date_free (today);

            /* it's not the first reconciliation, set the old balance and unsensitive the old balance entry */
            tmpstr = gsb_real_get_string (gsb_data_reconcile_get_final_balance (reconcile_number));
            gtk_entry_set_text ( GTK_ENTRY ( reconcile_initial_balance_entry ), tmpstr);
            g_free ( tmpstr );
            gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_initial_balance_entry ),
                    FALSE );
        }
        else
        {
            gtk_label_set_text ( GTK_LABEL ( reconcile_last_date_label ), _("None") );

            date = gdate_today();

            /* it's the first reconciliation, set the initial balance and make sensitive the old balance to change
             * it if necessary */
            tmpstr = gsb_real_get_string ( gsb_data_account_get_init_balance (account_number, -1));
            gtk_entry_set_text ( GTK_ENTRY ( reconcile_initial_balance_entry ), tmpstr);
            g_free ( tmpstr );
            gtk_widget_set_sensitive ( GTK_WIDGET ( reconcile_initial_balance_entry ), TRUE );
        }
    }

    string = gsb_format_gdate (date);
    gtk_entry_set_text ( GTK_ENTRY ( reconcile_new_date_entry ),
			 string );
    g_free (string);
    g_date_free (date);

    /* set last input amount if available and if the account is the good one */
    gtk_entry_set_text ( GTK_ENTRY ( reconcile_final_balance_entry ), 
            (etat.reconcile_final_balance) ? etat.reconcile_final_balance : "");
    if (etat.reconcile_final_balance)
        g_free(etat.reconcile_final_balance);

    /* set the title */
    tmpstr = g_markup_printf_escaped ( _(" <b>%s reconciliation</b> "),
					     gsb_data_account_get_name (account_number));
    gtk_label_set_markup ( GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (reconcile_panel))),
			   tmpstr );
    g_free ( tmpstr );

    /* we go to the reconciliation mode */
    etat.equilibrage = 1;

    /* set all the balances for reconciliation */
    gsb_reconcile_update_amounts (NULL, NULL);

    /* set the transactions list to reconciliation mode */
    /* only change the current account */
    reconcile_save_account_display = etat.retient_affichage_par_compte;
    etat.retient_affichage_par_compte = 1;

    /* hide the marked R transactions */
    reconcile_save_show_marked = gsb_data_account_get_r (account_number);
    if (reconcile_save_show_marked)
    {
        gsb_data_account_set_r (account_number, FALSE );
        mise_a_jour_affichage_r (FALSE);
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
    GSList *list_tmp_transactions;
    GDate *date;
    gint account_number;
    gint reconcile_number;
    gsb_real real;
	gchar* tmpstr;

    account_number = gsb_gui_navigation_get_current_account ();

    if ( gsb_real_sub ( gsb_real_add ( gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( reconcile_initial_balance_entry ))),
				       gsb_data_account_calculate_waiting_marked_balance (account_number)),
			gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( reconcile_final_balance_entry )))).mantissa != 0 )
    {
	dialogue_warning_hint ( _("There is a variance in balances, check that both final balance and initial balance minus marked transactions are equal."),
				_("Reconciliation can't be completed.") );
	return FALSE;
    }

    /* get and check the reconcile name */
    reconcile_number = gsb_data_reconcile_get_number_by_name (gtk_entry_get_text ( GTK_ENTRY ( reconcile_number_entry )));
    if (reconcile_number)
    {
	dialogue_warning_hint ( _("There is already a reconcile with that name, you must use another name or let it free.\nIf the reconcile name is ending by a number,\nit will be automatically incremented."),
				_("Reconciliation can't be completed.") );
	return FALSE;
    }


    /* get and save the date */
    date = gsb_calendar_entry_get_date (reconcile_new_date_entry);
    if (!date)
    {
	gchar* tmpstr = g_strdup_printf ( _("Invalid date: '%s'"),
						  gtk_entry_get_text ( GTK_ENTRY ( reconcile_new_date_entry )));
	dialogue_warning_hint ( tmpstr,
				_("Reconciliation can't be completed.") );
	g_free ( tmpstr );
	return FALSE;
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

    tmpstr = g_strdup_printf ( _("Last statement: %s"), gsb_format_gdate (date));
    gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			 tmpstr);
    g_free ( tmpstr );

    /* create the new reconcile structure */
    reconcile_number = gsb_data_reconcile_new (gtk_entry_get_text (GTK_ENTRY (reconcile_number_entry)));
    gsb_data_reconcile_set_account ( reconcile_number, account_number );

    /* set the variables of the reconcile */
    gsb_data_reconcile_set_final_date ( reconcile_number, date );
    g_date_free (date);

    date = gsb_parse_date_string (gtk_label_get_text (GTK_LABEL (reconcile_last_date_label)));
    gsb_data_reconcile_set_init_date ( reconcile_number, date );
    g_free (date);

    real = gsb_real_get_from_string ( gtk_entry_get_text (
                        GTK_ENTRY ( reconcile_initial_balance_entry ) ) );
    gsb_data_reconcile_set_init_balance ( reconcile_number, real );

    real = gsb_real_get_from_string ( gtk_entry_get_text (
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
	       gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_TELERAPPROCHEE ))
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

    mise_a_jour_liste_comptes_accueil = 1;

    /* go back to the normal transactions list */
    gsb_reconcile_cancel (NULL, NULL);

    /* reset records in etat: to do after gsb_reconcile_cancel */
    if (etat.reconcile_final_balance) g_free (etat.reconcile_final_balance);
    if (etat.reconcile_new_date) g_date_free (etat.reconcile_new_date);
    etat.reconcile_final_balance = NULL;
    etat.reconcile_new_date = NULL;
    etat.reconcile_account_number = -1;

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    if ( reconcile_save_last_scheduled_convert )
    {
        gsb_gui_navigation_set_selection ( GSB_SCHEDULER_PAGE, 0, NULL );
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
    gtk_widget_set_sensitive ( navigation_tree_view, sensitive );
    gsb_gui_sensitive_headings (sensitive);
    /* add by pbiava 02/11/2009 */
    gtk_widget_set_sensitive ( gtk_ui_manager_get_widget ( ui_manager, 
                              "/menubar/ViewMenu/ShowReconciled/" ),
			       sensitive );
    gtk_widget_set_sensitive ( gtk_ui_manager_get_widget ( ui_manager, 
                              "/menubar/ViewMenu/ShowArchived/" ),
			       sensitive );
    gtk_widget_set_sensitive ( gtk_ui_manager_get_widget (ui_manager, 
							  "/menubar/ViewMenu/ShowClosed/" ),
			       sensitive );
    gtk_widget_set_sensitive ( gtk_ui_manager_get_widget (ui_manager, 
							  "/menubar/EditMenu/ConvertToScheduled/" ),
			       sensitive );
    gtk_widget_set_sensitive ( gtk_ui_manager_get_widget (ui_manager, 
							  "/menubar/EditMenu/NewAccount/" ),
			       sensitive );
    gtk_widget_set_sensitive ( gtk_ui_manager_get_widget (ui_manager, 
							  "/menubar/EditMenu/RemoveAccount/" ),
			       sensitive );
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
    etat.equilibrage = 0;

    /* save the final balance/new date for the next time the user will try to reconcile */
    etat.reconcile_account_number = gsb_gui_navigation_get_current_account ();
    etat.reconcile_final_balance = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( reconcile_final_balance_entry ) ) );
    etat.reconcile_new_date = gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY ( reconcile_new_date_entry ) ) );

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
    etat.retient_affichage_par_compte = reconcile_save_account_display;

    gsb_transactions_list_set_visible_rows_number ( reconcile_save_rows_number );

    if (reconcile_save_show_marked)
    {
        gsb_data_account_set_r (gsb_gui_navigation_get_current_account (), TRUE );
        mise_a_jour_affichage_r (TRUE);
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
    gsb_real amount;
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
            gtk_widget_modify_base ( entry, GTK_STATE_NORMAL, NULL );
        }
        else
        {
            /* the entry is not valid, make it red */
            gtk_widget_modify_base ( entry, GTK_STATE_NORMAL,
                            &calendar_entry_color );
            return FALSE;
        }
    }

    currency_number = gsb_data_account_get_currency ( account_number );
    amount = gsb_utils_edit_calculate_entry ( reconcile_final_balance_entry );
    final_balance = gsb_real_get_string_with_currency ( amount, currency_number, FALSE );
    gtk_label_set_text ( GTK_LABEL ( reconcile_final_balance_label ), final_balance );

    /* set the marked balance amount,
     * this is what we mark as P while reconciling, so it's the total marked balance 
     * - the initial marked balance */
    tmp_string = gsb_real_get_string_with_currency (
                        gsb_data_account_calculate_waiting_marked_balance ( account_number ),
                        currency_number,
                        FALSE );
    gtk_label_set_text ( GTK_LABEL ( reconcile_marked_balance_label ), tmp_string );
    g_free (tmp_string);

    /* calculate the variation balance and show it */
    amount = gsb_real_sub ( gsb_real_add (
                        gsb_real_get_from_string ( initial_balance ),
					    gsb_data_account_calculate_waiting_marked_balance ( account_number ) ),
			            gsb_real_get_from_string ( final_balance ) );

    tmpstr = gsb_real_get_string_with_currency ( amount, currency_number, FALSE );
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
