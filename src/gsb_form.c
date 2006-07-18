/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org) 	      */
/*			http://www.grisbi.org   			      */
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
 * \file gsb_form.c
 * all that you need for the form is here !!!
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_form.h"
#include "gsb_form_transaction.h"
#include "echeancier_formulaire.h"
#include "erreur.h"
#include "comptes_traitements.h"
#include "calendar.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "utils_dates.h"
#include "gsb_fyear.h"
#include "navigation.h"
#include "gsb_payment_method.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "utils_editables.h"
#include "main.h"
#include "structures.h"
#include "traitement_variables.h"
#include "fenetre_principale.h"
#include "gsb_form.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_form_activate_expander ( GtkWidget *expander,
				      gpointer null );
static gboolean gsb_form_allocate_size ( GtkWidget *table,
				  GtkAllocation *allocation,
				  gpointer null );
static void gsb_form_check_auto_separator ( GtkWidget *entry );
static GtkWidget *gsb_form_create_element_from_number ( gint element_number,
						 gint account_number );
static gboolean gsb_form_element_can_receive_focus ( gint element_number,
					      gint account_number );
static gboolean gsb_form_fill_scheduled_part ( void );
static gboolean gsb_form_frequency_button_changed ( GtkWidget *combo_box,
					     gpointer null );
static gint gsb_form_get_account_from_button ( void );
static gint gsb_form_get_element_expandable ( gint element_number );
static gint gsb_form_get_origin ( void );
static gboolean gsb_form_init_entry_colors ( void );
static gboolean gsb_form_key_press_event ( GtkWidget *widget,
				    GdkEventKey *ev,
				    gint *ptr_origin );
static void gsb_form_set_entry_is_empty ( GtkWidget *entry,
				   gboolean empty );
static void gsb_form_set_focus ( gint element_number );
/*END_STATIC*/

/*START_EXTERN*/
extern FILE * out;
extern GtkTreeSelection * selection;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *window;
/*END_EXTERN*/



/** label of the last statement */
GtkWidget *label_last_statement = NULL;

/** the expander */
static GtkWidget *form_expander = NULL;

/** the 2 styles, grey or normal for the entries of the
 * form, need to be in static later */
GtkStyle *style_entree_formulaire[2];

/** the 3 parts of the form :
 * for scheduled transactions
 * for transactions
 * the buttons valid/cancel */
static GtkWidget *form_scheduled_part;
static GtkWidget *form_transaction_part;
GtkWidget *form_button_part;

/** when the automatic complete transaction is done
 * for a breakdown of transaction, we propose to recover too
 * the children with that button */
static GtkWidget *form_button_recover_breakdown;

/** need to set the 2 buttons valid/cancel here and cannot
 * just show/hide the form_button_part because of the breakdown button */
static GtkWidget *form_button_valid;
static GtkWidget *form_button_cancel;

/** contains the pointeurs to the widget in the transaction part form */
GtkWidget *form_tab_transactions[MAX_HEIGHT][MAX_WIDTH];

/** contains the pointeurs to the widget in the scheduled part form */
static GtkWidget *form_tab_scheduled[SCHEDULED_FORM_MAX_WIDGETS];

/* FIXME : next values need to be static and changed */

/* contient les adresses des widgets dans le formulaire en fonction */
/* de leur place */
GtkWidget *formulaire;



/**
 * Create an empty form in an GtkExpander.
 *
 * \return Expander that contains form.
 * */
GtkWidget * gsb_form_new ( void )
{
    /* Create the expander */
    form_expander = gtk_expander_new ( "" );
    gtk_expander_set_expanded ( GTK_EXPANDER ( form_expander ),
				etat.formulaire_toujours_affiche );
    g_signal_connect ( G_OBJECT(form_expander),
		       "activate",
		       G_CALLBACK (gsb_form_activate_expander),
		       NULL );

    gsb_form_create_widgets ();

    return form_expander;
}



/**
 *  Do the grunt job of creating widgets in for the Grisbi form.
 *  Fills them in form_expander, a GtkExpander normally created by
 *  gsb_form_new().  It is reentrant, that is calling it over and over
 *  will reinitialize form content and delete previous content.
 */
void gsb_form_create_widgets ()
{
    GtkWidget * hbox, * label, * separator, * hbox_buttons;
    GtkWidget * child = gtk_bin_get_child ( GTK_BIN(form_expander) );

    gsb_form_hide ();
    if ( child && GTK_IS_WIDGET(child) )
    {
	gtk_container_remove ( GTK_CONTAINER(form_expander), child );
    }

    /* Expander has a composite label */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_expander_set_label_widget ( GTK_EXPANDER(form_expander), hbox );

    /* set the label transaction form */
    label = gtk_label_new ( "" );
    gtk_label_set_markup_with_mnemonic ( GTK_LABEL ( label ), 
					 g_strconcat ( "<span weight=\"bold\">", 
						       _("Transaction/Scheduled _form"),
						       "</span>", NULL ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE, FALSE, 0 );

    /* set the last statement label */
    label_last_statement = gtk_label_new ( "" );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       label_last_statement,
		       FALSE, FALSE, 0 );

    /* Create form inside the expander :
     * the form is 3 parts :
     * top : the values specific for the scheduled transactions
     * middle : the values for transactions and scheduled transactions
     * bottom : the buttons valid/cancel */

    formulaire = gtk_vbox_new ( FALSE,
				5 );
    gtk_container_add ( GTK_CONTAINER ( form_expander ),
			formulaire );

    /* the scheduled part is a table of SCHEDULED_WIDTH col x SCHEDULED_HEIGHT rows */
    form_scheduled_part = gtk_table_new ( SCHEDULED_HEIGHT, 
					  SCHEDULED_WIDTH,
					  FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE (form_scheduled_part),
				 6 );
    gtk_box_pack_start ( GTK_BOX (formulaire),
			 form_scheduled_part,
			 FALSE, FALSE,
			 0 );
    gsb_form_fill_scheduled_part ();

    /* add a separator between the scheduled and transaction part */
    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX (formulaire),
			 separator,
			 FALSE, FALSE,
			 0 );

    /* the transactions part is a variable table,
     * so set to 1x1 for now, it will change when we show it */
    form_transaction_part = gtk_table_new ( 1, 1, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE (form_transaction_part), 6 );
    g_signal_connect ( G_OBJECT (form_transaction_part),
		       "size-allocate",
		       G_CALLBACK (gsb_form_allocate_size),
		       NULL );
    gtk_box_pack_start ( GTK_BOX (formulaire),
			 form_transaction_part,
			 FALSE, FALSE,
			 0 );

    /* the buttons part is a hbox, with the recuperate child breakdown
     * on the left and valid/cancel on the right */
    form_button_part = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX (formulaire), form_button_part, FALSE, FALSE, 0 );

    /* add a separator between the transaction and button part */
    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX (form_button_part), separator, FALSE, FALSE, 0 );

    /* Hbox containing buttons */
    hbox_buttons = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX (form_button_part), hbox_buttons, FALSE, FALSE, 0 );

    /* create the check button to recover the children of breakdowns */
    form_button_recover_breakdown = gtk_check_button_new_with_label ( _("Recover the children"));
    gtk_box_pack_start ( GTK_BOX (hbox_buttons),
			 form_button_recover_breakdown,
			 FALSE, FALSE,
			 0 );

    /* create the valid/cancel buttons */
    form_button_valid = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_button_set_relief ( GTK_BUTTON (form_button_valid), GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (form_button_valid), "clicked",
		       GTK_SIGNAL_FUNC (gsb_form_finish_edition), NULL );
    gtk_box_pack_end ( GTK_BOX (hbox_buttons), form_button_valid, FALSE, FALSE, 0 );

    form_button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON (form_button_cancel), GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (form_button_cancel), "clicked",
		       GTK_SIGNAL_FUNC (gsb_form_escape_form), NULL );
    gtk_box_pack_end ( GTK_BOX (hbox_buttons), form_button_cancel, FALSE, FALSE, 0 );

    /* init the colors for the entries */
    gsb_form_init_entry_colors ();

    /* Kludge : otherwise, GtkExpander won't give us as many space
       as we need. */
    gtk_widget_set_size_request ( hbox, 2048, -1 );

    gtk_widget_show_all ( hbox );
    gtk_widget_show_all ( formulaire );
    gtk_widget_hide ( form_scheduled_part );

    if ( ! etat.affiche_boutons_valider_annuler )
    {
	gtk_widget_hide_all ( form_button_part );
    }

    gsb_form_show ( FALSE );
}



/**
 * show or hide the expander
 *
 * \param visible TRUE or FALSE
 * \param transactions_list TRUE if we are on transactions, FALSE if we are on scheduler
 *
 * return FALSE
 * */
gboolean gsb_form_set_expander_visible ( gboolean visible,
					 gboolean transactions_list )
{
    if ( visible )
    {
	gtk_widget_show (form_expander);

	if (transactions_list)
	    gtk_widget_show (label_last_statement);
	else
	    gtk_widget_hide (label_last_statement);
    }
    else
	gtk_widget_hide (form_expander);

    return FALSE;
}


/**
 * called when change the state of the expander
 *
 * \param expander
 *
 * \return FALSE
 * */
gboolean gsb_form_activate_expander ( GtkWidget *expander,
				      gpointer null )
{
    if ( gtk_expander_get_expanded (GTK_EXPANDER (expander)))
    {
	gsb_form_hide ();
	etat.formulaire_toujours_affiche = FALSE;
    }
    else
    {
	gsb_form_show (FALSE);
	etat.formulaire_toujours_affiche = TRUE;
    }
    return FALSE;
}


/**
 * show the form, detect automaticly what we need to show, even for transactions,
 * scheduled_transactions and the buttons valid/cancel
 *
 * \param show TRUE if we want to automatickly turn on the expander, 
 * 		FALSE if we don't want, so just let it
 *
 * \return FALSE
 * */
gboolean gsb_form_show ( gboolean show )
{
    gint origin;

    devel_debug ( "gsb_form_show" );

    origin = gsb_form_get_origin ();

    switch (origin)
    {
	case -2:
	    return FALSE;
	    break;

	case -1:
	    gtk_widget_show (form_scheduled_part);
	    break;

	default:
	    gtk_widget_hide (form_scheduled_part);
    }

    gsb_form_fill_transaction_part (GINT_TO_POINTER (origin));
    gtk_widget_show (form_transaction_part);

    if ( !gtk_expander_get_expanded (GTK_EXPANDER (form_expander))
	 &&
	 show )
	gtk_expander_set_expanded (GTK_EXPANDER (form_expander),
				   TRUE );

    /* FIXME :transform that to be local variable here */
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show (form_button_part);

    return FALSE;
}

/**
 * return the current account number according,
 * if we are on transactions list, return the current account,
 * if we are on scheduling list, return the selected account on button
 *
 * \param origin
 *
 * \return the account number or -2 if problem (-1 is reserved to get account from the button)
 * */
gint gsb_form_get_account_number ( void )
{
    gint account_number;
    gint origin;

    origin = gsb_form_get_origin ();

    switch (origin)
    {
	case -2:
	    return -2;
	    break;

	case -1:
	    account_number = gsb_form_get_account_from_button ();
	    break;
	    
	default:
	    account_number = origin;
    }
    return account_number;
}


/**
 * get the page where we are and return a number according that page :
 * -2 : we are neither on scheduled transactions, neither transactions list
 * -1 : we are on the scheduled transactions
 *  0 to x : the account number where we are
 *  that function is called at each begining to know where we are and what to do...
 *
 *  \param
 *
 *  \return the number where we are
 *  */
gint gsb_form_get_origin ( void )
{
    gint account_number;

    switch (gsb_gui_navigation_get_current_page())
    {
	case GSB_ACCOUNT_PAGE:
	    account_number = gsb_gui_navigation_get_current_account ();

	    if ( account_number != -1)
		return account_number;
	    break;

	case GSB_SCHEDULER_PAGE:
	    return -1;
	    break;
    }
    return -2;
}



/** 
 * hide the form, for now destroy it
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_hide ( void )
{
    gint row, column;

    devel_debug ("gsb_form_hide");

    for ( row=0 ; row < MAX_HEIGHT ; row++ )
	for ( column=0 ; column < MAX_WIDTH ; column++ )
	    if ( form_tab_transactions[row][column] )
	    {
		/* if there is something in the combofix we destroy, the popup will
		 * be showed because destroying the gtk_entry will erase it directly,
		 * so the simpliest way to avoid that is to erase now the entry, but with
		 * gtk_combofix_set_text [cedric] (didn't succeed with another thing...) */
		if ( GTK_IS_COMBOFIX ( form_tab_transactions[row][column] ) )
		    gtk_combofix_set_text ( GTK_COMBOFIX (form_tab_transactions[row][column]),
					    "" );

		gtk_widget_destroy (form_tab_transactions[row][column]);
		form_tab_transactions[row][column] = NULL;
	    }
    return FALSE;
}


/**
 * fill the scheduled part : that widgets are created at the begining
 * and normally never destroyed, they are showed only for
 * scheduled transactions
 *
 * \param 
 *
 * \return FALSE
 * */
gboolean gsb_form_fill_scheduled_part ( void )
{
    gint row, column;

    for ( row=0 ; row < SCHEDULED_HEIGHT ; row++ )
	for ( column=0 ; column < SCHEDULED_WIDTH ; column++ )
	{
	    gint element_number;
	    GtkWidget *widget = NULL;

	    element_number = row*SCHEDULED_WIDTH + column;

	    switch ( element_number )
	    {
		case SCHEDULED_FORM_ACCOUNT:
		    widget = gtk_combo_box_new ();
		    gsb_account_create_name_tree_model ( widget,
							 NULL,
							 FALSE );
		    gtk_combo_box_set_active ( GTK_COMBO_BOX (widget),
					       0 );
		    g_signal_connect_swapped ( G_OBJECT (widget),
					       "changed",
					       G_CALLBACK (gsb_form_fill_transaction_part),
					       GINT_TO_POINTER (-1));
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Choose the account"),
					   _("Choose the account") );
		    break;

		case SCHEDULED_FORM_AUTO:
		    widget = gtk_combo_box_new_text ();
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Manual"));
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Automatic"));
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Automatic/manual scheduled transaction"),
					   _("Automatic/manual scheduled transaction") );
		    break;

		case SCHEDULED_FORM_FREQUENCY_BUTTON:
		    widget = gtk_combo_box_new_text ();
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Once"));
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Weekly"));
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Monthly"));
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Yearly"));
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Custom"));
		    g_signal_connect ( G_OBJECT (widget),
				       "changed",
				       G_CALLBACK (gsb_form_frequency_button_changed),
				       NULL );
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Frequency"),
					   _("Frequency"));
		    break;

		case SCHEDULED_FORM_LIMIT_DATE:
		    widget = gtk_entry_new_with_max_length (11);
		    g_signal_connect ( G_OBJECT (widget),
				       "button-press-event",
				       GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
				       GINT_TO_POINTER ( SCHEDULER_FORM_FINAL_DATE ) );
		    g_signal_connect ( G_OBJECT (widget),
				       "focus-in-event",
				       GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
				       NULL );
		    g_signal_connect_after ( GTK_OBJECT (widget),
					     "focus-out-event",
					     GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
					     GINT_TO_POINTER ( SCHEDULER_FORM_FINAL_DATE ) );
		    break;

		case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		    widget = gtk_entry_new ();
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Custom frequency"),
					   _("Custom frequency") );

		    break;

		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		    widget = gtk_combo_box_new_text ();
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Days"));
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Months"));
		    gtk_combo_box_append_text ( GTK_COMBO_BOX (widget),
						_("Years"));
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Custom frequency"),
					   _("Custom frequency") );
		    break;
	    }

	    if (!widget)
		continue;

	    gtk_signal_connect ( GTK_OBJECT (widget),
				 "key-press-event",
				 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
				 GINT_TO_POINTER ( SCHEDULER_FORM_ACCOUNT ) );
	    form_tab_scheduled[element_number] = widget;

	    gtk_table_attach ( GTK_TABLE (form_scheduled_part),
			       widget,
			       column, column+1,
			       row, row+1,
			       GTK_EXPAND | GTK_FILL,
			       GTK_EXPAND | GTK_FILL,
			       0, 0);
	}
    return FALSE;
}


/**
 * called when the frequency button is changed
 * show/hide the necessary widget according to its state
 *
 * \param combo_box
 *
 * \return FALSE 
 * */
gboolean gsb_form_frequency_button_changed ( GtkWidget *combo_box,
					     gpointer null )
{
    gchar *selected_item;

    selected_item = gtk_combo_box_get_active_text ( GTK_COMBO_BOX (combo_box));
    
    if ( !strcmp ( selected_item,
		   _("Once")))
    {
	gtk_widget_hide (form_tab_scheduled[SCHEDULED_FORM_LIMIT_DATE]);
	gtk_widget_hide (form_tab_scheduled[SCHEDULED_FORM_FREQUENCY_USER_ENTRY]);
	gtk_widget_hide (form_tab_scheduled[SCHEDULED_FORM_FREQUENCY_USER_BUTTON]);
    }
    else
    {
	gtk_widget_show (form_tab_scheduled[SCHEDULED_FORM_LIMIT_DATE]);
	if ( !strcmp ( selected_item,
		       _("Custom")))
	{
	    gtk_widget_show (form_tab_scheduled[SCHEDULED_FORM_FREQUENCY_USER_ENTRY]);
	    gtk_widget_show (form_tab_scheduled[SCHEDULED_FORM_FREQUENCY_USER_BUTTON]);
	}
	else
	{
	    gtk_widget_hide (form_tab_scheduled[SCHEDULED_FORM_FREQUENCY_USER_ENTRY]);
	    gtk_widget_hide (form_tab_scheduled[SCHEDULED_FORM_FREQUENCY_USER_BUTTON]);
	}
    }
    g_free (selected_item);
    return FALSE;
}


/**
 * get the account number from the scheduled button and return it
 *
 * \param
 *
 * \return the account number or -2 if problem
 * */
gint gsb_form_get_account_from_button ( void )
{
    gint account_number;
    GtkTreeIter iter;

    /* if no account button, go away... */
    if (!form_tab_scheduled[SCHEDULED_FORM_ACCOUNT])
	return -2;

    if ( !gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (form_tab_scheduled[SCHEDULED_FORM_ACCOUNT]),
					  &iter ))
	return -2;

    gtk_tree_model_get ( GTK_TREE_MODEL (gtk_combo_box_get_model (GTK_COMBO_BOX (form_tab_scheduled[SCHEDULED_FORM_ACCOUNT]))),
			 &iter,
			 1, &account_number,
			 -1 );
    return account_number;
}


/**
 * fill the form according to the account_number :
 *
 * \param ptr_account_number a pointer corresponding to an int (transformed with GINT_TO_POINTER), the number of account
 * or -1 to get it from the account button
 * if comes with -1, the form will be fill ONLY if the account button is visible
 * need to be a pointer because called directly from a g_signal_connect
 *
 * \return FALSE
 * */
gboolean gsb_form_fill_transaction_part ( gint *ptr_account_number )
{
    gint row, column;
    gint rows_number, columns_number;
    gint account_number;

    account_number = GPOINTER_TO_INT (ptr_account_number);

    devel_debug ( g_strdup_printf ("gsb_form_fill_transaction_part account_number : %d", account_number ));

    gsb_form_hide ();

    /* account_number can be -1 if come here from the accounts choice button,
     * and -2 if there were a problem with the origin */

    switch (account_number)
    {
	case -2:
	    return FALSE;
	    break;
	    
	case-1:
	    if ( GTK_WIDGET_VISIBLE (form_tab_scheduled[SCHEDULED_FORM_ACCOUNT]))
	    {
		account_number = gsb_form_get_account_from_button ();
		if (account_number == -2 )
		    return FALSE;
	    }
	    else
		return FALSE;
	    break;
    }

    rows_number = gsb_data_form_get_nb_rows (account_number);
    columns_number = gsb_data_form_get_nb_columns (account_number);

    gtk_table_resize ( GTK_TABLE (form_transaction_part),
		       rows_number,
		       columns_number );

    for ( row=0 ; row < rows_number ; row++ )
	for ( column=0 ; column < columns_number ; column++ )
	{
	    GtkWidget *widget;
	    gint element = gsb_data_form_get_value ( account_number, column, row );
	    widget = gsb_form_create_element_from_number ( element, account_number );
	    form_tab_transactions[row][column] = widget;

	    if ( !widget )
		continue;

	    gtk_table_attach ( GTK_TABLE (form_transaction_part),
			       widget,
			       column, column+1,
			       row, row+1,
			       gsb_form_get_element_expandable ( element ),
			       gsb_form_get_element_expandable ( element ),
			       0, 0);
	    gtk_widget_show (widget);
	}
    gsb_form_clean (account_number);

    return FALSE;
}


/**
 * create and return the widget according to its element number
 *
 * \param element_number the number corresponding to the widget wanted
 * \param account_number
 *
 * \return the widget
 * */
GtkWidget *gsb_form_create_element_from_number ( gint element_number,
						 gint account_number )
{
    GtkWidget *widget;

    if ( !element_number )
	return NULL;

    widget = NULL;

    switch ( element_number )
    {
	case TRANSACTION_FORM_DATE:
	case TRANSACTION_FORM_DEBIT:
	case TRANSACTION_FORM_CREDIT:
	case TRANSACTION_FORM_VALUE_DATE:
	case TRANSACTION_FORM_NOTES:
	case TRANSACTION_FORM_CHEQUE:
	case TRANSACTION_FORM_VOUCHER:
	case TRANSACTION_FORM_BANK:

	    widget = gtk_entry_new();
	    break;

	case TRANSACTION_FORM_EXERCICE:
	    widget = gsb_fyear_make_combobox ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the financial year"),
				   _("Choose the financial year") );
	    break;

	case TRANSACTION_FORM_PARTY:
	    widget = gtk_combofix_new_complex ( gsb_data_payee_get_name_and_report_list());
	    gtk_combofix_set_force_text ( GTK_COMBOFIX (widget),
					  etat.combofix_force_payee );
	    gtk_combofix_set_max_items ( GTK_COMBOFIX (widget),
					 etat.combofix_max_item );
	    gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (widget),
					      etat.combofix_case_sensitive );
	    gtk_combofix_set_enter_function ( GTK_COMBOFIX (widget),
					      etat.combofix_enter_select_completion );
	    /* we never mix the payee because the only case of the complex combofix is
	     * for the report and there is no sense to mix report with the payee */
	    gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (widget),
					  FALSE );
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    widget = gtk_combofix_new_complex ( gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE));
	    gtk_combofix_set_force_text ( GTK_COMBOFIX (widget),
					  etat.combofix_force_category );
	    gtk_combofix_set_max_items ( GTK_COMBOFIX (widget),
					 etat.combofix_max_item );
	    gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (widget),
					      etat.combofix_case_sensitive );
	    gtk_combofix_set_enter_function ( GTK_COMBOFIX (widget),
					      etat.combofix_enter_select_completion );
	    gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (widget),
					  etat.combofix_mixed_sort );
	    break;

	case TRANSACTION_FORM_FREE:
	    break;

	case TRANSACTION_FORM_BUDGET:
	    widget = gtk_combofix_new_complex ( gsb_data_budget_get_name_list (TRUE, TRUE));
	    gtk_combofix_set_force_text ( GTK_COMBOFIX (widget),
					  etat.combofix_force_category );
	    gtk_combofix_set_max_items ( GTK_COMBOFIX (widget),
					 etat.combofix_max_item );
	    gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (widget),
					      etat.combofix_case_sensitive );
	    gtk_combofix_set_enter_function ( GTK_COMBOFIX (widget),
					      etat.combofix_enter_select_completion );
	    gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (widget),
					  etat.combofix_mixed_sort );
	    break;

	case TRANSACTION_FORM_TYPE:
	    widget = gtk_combo_box_new ();
	    g_signal_connect ( G_OBJECT (widget),
			       "changed",
			       G_CALLBACK (gsb_payment_method_changed_callback),
			       NULL );
	    gsb_payment_method_create_combo_list ( widget,
						   GSB_PAYMENT_DEBIT,
						   account_number );
	    gtk_combo_box_set_active ( GTK_COMBO_BOX (widget),
				       0 );
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the method of payment"),
				   _("Choose the method of payment") );
	    break;

	case TRANSACTION_FORM_DEVISE:
	    widget = gsb_currency_make_combobox (FALSE);
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose currency"),
				   _("Choose currency") );
	    gsb_currency_set_combobox_history ( widget,
						gsb_data_account_get_currency (account_number));
	    break;

	case TRANSACTION_FORM_CHANGE:
	    widget = gtk_button_new_with_label ( _("Change") );
	    gtk_button_set_relief ( GTK_BUTTON ( widget ),
				    GTK_RELIEF_NONE );
	    gtk_signal_connect ( GTK_OBJECT (  widget ),
				 "clicked",
				 GTK_SIGNAL_FUNC ( click_sur_bouton_voir_change ),
				 NULL );
	    break;

	case TRANSACTION_FORM_CONTRA:
	    /* no menu at begining, appened when choose the contra-account */
	    widget = gtk_combo_box_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Contra-transaction method of payment"),
				   _("Contra-transaction method of payment") );
	    break;

	case TRANSACTION_FORM_OP_NB:
	case TRANSACTION_FORM_MODE:
	    widget = gtk_label_new ( "" );
	    break;
    }

    if ( widget )
    {
	if ( GTK_IS_ENTRY ( widget ))
	{
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "focus-in-event",
				 GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
				 GINT_TO_POINTER ( element_number ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "focus-out-event",
				 GTK_SIGNAL_FUNC ( gsb_form_entry_lose_focus ),
				 GINT_TO_POINTER ( element_number ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "button-press-event",
				 GTK_SIGNAL_FUNC ( gsb_form_button_press_event ),
				 GINT_TO_POINTER ( element_number ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "key-press-event",
				 GTK_SIGNAL_FUNC ( gsb_form_key_press_event ),
				 GINT_TO_POINTER ( element_number ));
	}
	else
	{
	    if ( GTK_IS_COMBOFIX ( widget ))
	    {
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget ) -> entry ),
				     "focus-in-event",
				     GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
				     GINT_TO_POINTER ( element_number ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "focus-out-event",
				     GTK_SIGNAL_FUNC ( gsb_form_entry_lose_focus ),
				     GINT_TO_POINTER ( element_number ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "button-press-event",
				     GTK_SIGNAL_FUNC ( gsb_form_button_press_event ),
				     GINT_TO_POINTER ( element_number ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "key-press-event",
				     GTK_SIGNAL_FUNC ( gsb_form_key_press_event ),
				     GINT_TO_POINTER ( element_number ));
	    }
	    else
		/* neither an entry, neither a combofix */
		gtk_signal_connect ( GTK_OBJECT ( widget ),
				     "key-press-event",
				     GTK_SIGNAL_FUNC ( gsb_form_key_press_event ),
				     GINT_TO_POINTER ( element_number ));
	}
    }
    return widget;
}


/**
 * clean the form according to the account_number
 * and set the default values
 *
 * \param account number
 *
 * \return FALSE
 * */
gboolean gsb_form_clean ( gint account_number )
{
    gint row, column;
    GtkWidget *widget;

    devel_debug ( g_strdup_printf ( "gsb_form_clean account %d",
				    account_number ));

    /* clean the scheduled widget */

    for ( column = 0 ; column < SCHEDULED_FORM_MAX_WIDGETS ; column++ )
    {
	widget = form_tab_scheduled[column];

	switch (column)
	{
	    case SCHEDULED_FORM_AUTO:
	    case SCHEDULED_FORM_FREQUENCY_BUTTON:
	    case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		gtk_combo_box_set_active ( GTK_COMBO_BOX (widget),
					   0 );
		break;

	    case SCHEDULED_FORM_LIMIT_DATE:
		gsb_form_set_entry_is_empty ( widget,
					      TRUE );
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     _("Limit date") );
		break;

	    case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		gsb_form_set_entry_is_empty ( widget,
					      TRUE );
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     _("User frequency") );
		break;
	}
    }

    /* clean the transactions widget */

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column <  gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gint value;

	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );

	    widget =  gsb_form_get_element_widget ( value,
						      account_number );

	    /* better to protect here if widget != NULL (bad experience...) */
	    if (widget)
		switch (value)
		{
		    case TRANSACTION_FORM_DATE:

			gtk_widget_set_sensitive ( widget,
						   TRUE );
			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Date") );
			break;

		    case TRANSACTION_FORM_VALUE_DATE:

			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Value date") );
			break;

		    case TRANSACTION_FORM_EXERCICE:

			/* editing a transaction can show some fyear wich shouldn't be showed,
			 * so hide them here */
			gsb_fyear_update_fyear_list ();

			/* set the combo_box on 'Automatic' */
			gsb_fyear_set_combobox_history ( widget,
							 0 );

			gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
						   FALSE );
			break;

		    case TRANSACTION_FORM_PARTY:

			gtk_widget_set_sensitive ( widget,
						   TRUE );
			gsb_form_set_entry_is_empty ( GTK_COMBOFIX ( widget ) -> entry,
						      TRUE );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Payee") );
			break;

		    case TRANSACTION_FORM_DEBIT:

			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Debit") );
			gtk_widget_set_sensitive ( widget,
						   TRUE );
			break;

		    case TRANSACTION_FORM_CREDIT:

			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Credit") );
			gtk_widget_set_sensitive ( widget,
						   TRUE );
			break;

		    case TRANSACTION_FORM_CATEGORY:

			gsb_form_set_entry_is_empty ( GTK_COMBOFIX ( widget ) -> entry,
						      TRUE );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Categories : Sub-categories") );
			gtk_widget_set_sensitive ( widget,
						   TRUE );
			break;

		    case TRANSACTION_FORM_FREE:
			break;

		    case TRANSACTION_FORM_BUDGET:

			gsb_form_set_entry_is_empty ( GTK_COMBOFIX ( widget ) -> entry,
						      TRUE );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Budgetary line") );
			break;

		    case TRANSACTION_FORM_NOTES:

			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Notes") );
			break;

		    case TRANSACTION_FORM_TYPE:
			gtk_combo_box_set_active ( GTK_COMBO_BOX ( widget ),
						   gsb_payment_method_get_payment_location ( widget,
											     gsb_data_account_get_default_debit (account_number)));
			gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
						   FALSE );
			break;

		    case TRANSACTION_FORM_CONTRA:

			gtk_widget_hide ( widget );

			break;

		    case TRANSACTION_FORM_CHEQUE:

			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Cheque/Transfer number") );
			break;

		    case TRANSACTION_FORM_DEVISE:

			gsb_currency_set_combobox_history ( widget,
							    gsb_data_account_get_currency (account_number));
			gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
						   FALSE );
			break;

		    case TRANSACTION_FORM_CHANGE:

			gtk_widget_hide ( widget );

			break;

		    case TRANSACTION_FORM_BANK:

			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Bank references") );
			break;

		    case TRANSACTION_FORM_VOUCHER:

			gsb_form_set_entry_is_empty ( widget,
						      TRUE );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Voucher") );
			break;

		    case TRANSACTION_FORM_OP_NB:

			gtk_label_set_text ( GTK_LABEL ( widget ),
					     "" );
			break;

		    case TRANSACTION_FORM_MODE:

			gtk_label_set_text ( GTK_LABEL ( widget ),
					     "" );
			break;
		}
	}

    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "transaction_number_in_form",
			  NULL );
    return FALSE;
}


/**
 * return the pointer to the widget corresponding to the given element
 *
 * \param element_number
 * \param account_number
 *
 * \return the widget or NULL
 * */
GtkWidget *gsb_form_get_element_widget ( gint element_number,
					 gint account_number )
{
    gint row;
    gint column;

    if ( !element_number )
	return NULL;

    if ( gsb_data_form_look_for_value ( account_number,
					element_number,
					&row,
					&column ))
	return form_tab_transactions[row][column];
    
    return NULL;
}



/**
 * Determine element is expandable or not in a GtkTable.
 *
 * \param TODO
 * 
 * \return
 */
gint gsb_form_get_element_expandable ( gint element_number )
{
    switch ( element_number )
    {
	case TRANSACTION_FORM_DEVISE:
	case TRANSACTION_FORM_TYPE:
	case TRANSACTION_FORM_OP_NB:
	case TRANSACTION_FORM_MODE:
	    printf ("%s is shrinkable\n", gsb_form_get_element_name(element_number)); 
	    return GTK_SHRINK;

	default:
	    return GTK_EXPAND | GTK_FILL;

    }
}


/**
 * called when an entry get the focus, if the entry is free,
 * set it normal and erase the help content
 *
 * \param entry
 * \param ev
 *
 * \return FALSE
 * */
gboolean gsb_form_entry_get_focus ( GtkWidget *entry )
{
    /* the entry can be a combofix or a real entry */
    if (GTK_IS_COMBOFIX ( entry ))
    {
	if ( gsb_form_check_entry_is_empty (GTK_COMBOFIX (entry) -> entry))
	{
	    gtk_combofix_set_text ( GTK_COMBOFIX (entry), "" );
	    gsb_form_set_entry_is_empty ( GTK_COMBOFIX (entry) -> entry,
					  FALSE );
	}
    }
    else
    {
	if ( gsb_form_check_entry_is_empty (entry) )
	{
	    gtk_entry_set_text ( GTK_ENTRY (entry), "" );
	    gsb_form_set_entry_is_empty ( entry,
					  FALSE );
	}
    }
    return FALSE;
}


/**
 * called when an entry lose the focus
 *
 * \param entry
 * \param ev
 * \param ptr_origin a pointer gint wich is the number of the element
 *
 * \return FALSE
 * */
gboolean gsb_form_entry_lose_focus ( GtkWidget *entry,
				     GdkEventFocus *ev,
				     gint *ptr_origin )
{
    gchar *string;
    gint element_number;
    GtkWidget *widget;
    gint account_number;

    /* remove the selection */

    gtk_editable_select_region ( GTK_EDITABLE ( entry ),
				 0,
				 0 );
    element_number = GPOINTER_TO_INT (ptr_origin);
    account_number = gsb_form_get_account_number ();

    /* sometimes the combofix popus stays showed, so remove here */

    if ( element_number == TRANSACTION_FORM_PARTY
	 ||
	 element_number == TRANSACTION_FORM_CATEGORY
	 ||
	 element_number == TRANSACTION_FORM_BUDGET )
    {
	widget = gsb_form_get_element_widget (element_number,
					      account_number );

	gtk_grab_remove ( GTK_COMBOFIX ( widget ) -> popup );
	gdk_pointer_ungrab ( GDK_CURRENT_TIME );
	gtk_widget_hide ( GTK_COMBOFIX ( widget ) ->popup );
    }

    /* string will be filled only if the field is empty */
    string = NULL;

    switch ( element_number )
    {
	case TRANSACTION_FORM_DATE :

	    /* check and finish the date */
	    gsb_date_check_and_complete_entry ( entry );
	    break;

	case TRANSACTION_FORM_PARTY :

	    /* we complete the transaction */
	    if ( !gsb_form_transaction_complete_form_by_payee (gtk_entry_get_text (GTK_ENTRY (entry))))
		string = gsb_form_get_element_name (TRANSACTION_FORM_PARTY);
	    break;

	case TRANSACTION_FORM_DEBIT :

	    /* we change the payment method to adapt it for the debit */
	    if ( strlen ( gtk_entry_get_text ( GTK_ENTRY (entry))))
	    {
		/* empty the credit */
		widget = gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT,
						      account_number);
		if (!gsb_form_check_entry_is_empty (widget))
		{
		    gtk_entry_set_text ( GTK_ENTRY (widget),
					 gsb_form_get_element_name (TRANSACTION_FORM_CREDIT));
		    gsb_form_set_entry_is_empty ( widget,
						  TRUE );
		}
		widget = gsb_form_get_element_widget ( TRANSACTION_FORM_TYPE,
						       account_number );

		/* change the signe of the method of payment and the contra */
		if ( gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_CREDIT)
		{
		    gsb_payment_method_create_combo_list ( widget,
							   GSB_PAYMENT_DEBIT,
							   account_number );
		    /* if there is no payment method, the last function hide it, but we have
		     * to hide the cheque element too */
		    if ( !GTK_WIDGET_VISIBLE (widget))
			gtk_widget_hide (gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
								      account_number));

		    widget = gsb_form_get_element_widget ( TRANSACTION_FORM_CONTRA,
							     account_number );
		    if ( GTK_WIDGET_VISIBLE (widget))
			gsb_payment_method_create_combo_list ( gsb_form_get_element_widget ( TRANSACTION_FORM_CONTRA,
											     account_number ),
							       GSB_PAYMENT_CREDIT,
							       account_number );
		}
		gsb_form_check_auto_separator (entry);
	    }
	    else
		string = gsb_form_get_element_name (TRANSACTION_FORM_DEBIT);
	    break;

	case TRANSACTION_FORM_CREDIT :

	    /* we change the payment method to adapt it for the debit */
	    if ( strlen ( gtk_entry_get_text ( GTK_ENTRY (entry))))
	    {
		/* empty the credit */
		widget = gsb_form_get_element_widget (TRANSACTION_FORM_DEBIT,
							account_number );
		if (!gsb_form_check_entry_is_empty (widget))
		{
		    gtk_entry_set_text ( GTK_ENTRY (widget),
					 gsb_form_get_element_name (TRANSACTION_FORM_DEBIT));
		    gsb_form_set_entry_is_empty ( widget,
						  TRUE );
		}
		widget = gsb_form_get_element_widget ( TRANSACTION_FORM_TYPE,
						       account_number );

		/* change the signe of the method of payment and the contra */
		if ( gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_DEBIT)
		{
		    gsb_payment_method_create_combo_list ( widget,
							   GSB_PAYMENT_CREDIT,
							   account_number );
		    /* if there is no payment method, the last function hide it, but we have
		     * to hide the cheque element too */
		    if ( !GTK_WIDGET_VISIBLE (widget))
			gtk_widget_hide (gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
								      account_number));

		    widget = gsb_form_get_element_widget ( TRANSACTION_FORM_CONTRA,
							     account_number );
		    if ( GTK_WIDGET_VISIBLE (widget))
			gsb_payment_method_create_combo_list ( widget,
							       GSB_PAYMENT_DEBIT,
							       account_number );
		}
		gsb_form_check_auto_separator (entry);
	    }
	    else
		string = gsb_form_get_element_name (TRANSACTION_FORM_CREDIT);
	    break;

	case TRANSACTION_FORM_VALUE_DATE :
	    if ( !gsb_date_check_and_complete_entry (entry))
		string = gsb_form_get_element_name (TRANSACTION_FORM_VALUE_DATE);
	    break;

	case TRANSACTION_FORM_CATEGORY :

	    if ( strlen ( gtk_entry_get_text ( GTK_ENTRY (entry))))
	    {
		if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA ))
		{
		    /* if it's a transfer, set the contra_method of payment menu */
		    gint contra_account_number;

		    if ( (contra_account_number = gsb_form_check_for_transfer ( gtk_entry_get_text ( GTK_ENTRY (entry)))) >= 0
			 &&
			 contra_account_number != account_number )
		    {
			if ( gsb_form_check_entry_is_empty (gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT,
											 account_number)))
			    /* there is something in debit */
			    gsb_payment_method_create_combo_list ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA,
												  account_number),
								   GSB_PAYMENT_CREDIT,
								   contra_account_number );
			else
			    /* there is something in credit */
			    gsb_payment_method_create_combo_list ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA,
												  account_number),
								   GSB_PAYMENT_DEBIT,
								   contra_account_number );
		    }
		    else
			gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA,
									 account_number ));
		}
	    }
	    else
		string = gsb_form_get_element_name (TRANSACTION_FORM_CATEGORY);

	    break;

	case TRANSACTION_FORM_CHEQUE :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entry )))))
		string = gsb_form_get_element_name (TRANSACTION_FORM_CHEQUE);
	    break;

	case TRANSACTION_FORM_BUDGET :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entry )))))
		string = gsb_form_get_element_name (TRANSACTION_FORM_BUDGET);
	    break;

	case TRANSACTION_FORM_VOUCHER :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entry )))))
		string = gsb_form_get_element_name (TRANSACTION_FORM_VOUCHER);

	    break;

	case TRANSACTION_FORM_NOTES :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entry )))))
		string = gsb_form_get_element_name (TRANSACTION_FORM_NOTES);
	    break;

	case TRANSACTION_FORM_BANK :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entry )))))
		string = gsb_form_get_element_name (TRANSACTION_FORM_BANK);
	    break;
	default :
	    break;
    }

    /* if string is not NULL, the entry is empty so set the empty field to TRUE */

    if ( string )
    {
	switch ( element_number)
	{
	    case TRANSACTION_FORM_PARTY :
	    case TRANSACTION_FORM_CATEGORY :
	    case TRANSACTION_FORM_BUDGET :
		/* need to work with the combofix to avoid some signals if we work
		 * directly on the entry */
		gtk_combofix_set_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (element_number,
										    account_number) ),
					string );
		break;

	    default:
		gtk_entry_set_text ( GTK_ENTRY ( entry ), string );
		break;
	}
	gsb_form_set_entry_is_empty ( entry,
				      TRUE );
    }
    return FALSE;
}


/**
 * called when leave an amount entry, il the automatic separator
 * is on, set the separator in the entry if there is none
 *
 * \param entry
 *
 * \return
 * */
void gsb_form_check_auto_separator ( GtkWidget *entry )
{
    gint account_number;
    gchar *string;
    gint floating_point;
    gchar *tmp = NULL;
    gint i;

    if (!etat.automatic_separator
	||
	!entry )
	return;
    
    /* we need a g_strdup to permit to do the g_free later
     * because if strlen < floating point we need to
     * malloc another string */
    string = g_strdup (gtk_entry_get_text (GTK_ENTRY(entry)));

    account_number = gsb_form_get_account_number ();
    floating_point = gsb_data_currency_get_floating_point (gsb_data_account_get_currency (account_number));

    if ( index (string, '.')
	 ||
	 index (string, ','))
    {
	g_free (string);
	return;
    }

    /* if string is < the floating_point, increase it to have
     * 1 character more (to set the 0 before the .) */
    if (strlen(string) <= floating_point)
    {
	gchar *concat_tmp;

	tmp = g_malloc (floating_point - strlen(string) + 2);
	for (i=0 ; i<(floating_point - strlen(string) + 1) ; i++)
	    tmp[i] = '0';
	tmp[floating_point - strlen(string) + 1] = 0;
	concat_tmp = g_strconcat ( tmp,
				   string,
				   NULL );
	g_free (tmp);
	g_free (string);
	string = concat_tmp;
    }

    tmp = g_malloc ((strlen(string)+2) * sizeof (gchar));

    memcpy ( tmp, string, strlen(string) - floating_point);

    i = strlen(string) - floating_point;
    tmp[i] = '.';
    i++;
    memcpy ( tmp + i,
	     string + i - 1,
	     floating_point );
    i = i + floating_point;
    tmp[i] = 0;
    gtk_entry_set_text (GTK_ENTRY (entry),
			tmp );
    g_free (tmp);
    g_free (string);
}


/**
 * check if the string in the param is like "Transfer : account"
 * if yes, return the number of the account
 * if deleted account, return -2
 * if no return -1
 *
 * \param entry_string
 *
 * \return the number of the transfer_account
 * 		-1 if transfer to a non existant account
 * 		-2 if deleted account
 * 		-3 if not a transfer
 * */
gint gsb_form_check_for_transfer ( const gchar *entry_string )
{
    const gchar *account_name;

    if ( !entry_string
	 ||
	 strncmp ( entry_string,
		   _("Transfer : "),
		   strlen (_("Transfer : "))))
	 return -3;

    account_name = memchr ( entry_string,
			    ':',
			    strlen (entry_string));
    /* after the : there is a space before the account name */
    account_name = account_name + 2;

    if ( strcmp ( account_name,
		  _("Deleted account")))
	return gsb_data_account_get_no_account_by_name (account_name);

    else
	return -2;
}


/**
 * called when we press the button in an entry field in
 * the form
 *
 * \param entry wich receive the signal
 * \param ev can be NULL
 * \param ptr_origin a pointer to int on the element_number
 *
 * \return FALSE
 * */
gboolean gsb_form_button_press_event ( GtkWidget *entry,
				       GdkEventButton *ev,
				       gint *ptr_origin )
{
    gint element_number;
    gint account_number;

    element_number = GPOINTER_TO_INT (ptr_origin);
    account_number = gsb_form_get_account_number ();

    /* set the form sensitive */
    gsb_form_change_sensitive_buttons (TRUE);

    /* set the current date into the date entry except if there is already something into the value date */
    if ( gsb_form_check_entry_is_empty ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE,
								      account_number)))
    {
	if ( !gsb_data_form_check_for_value ( TRANSACTION_FORM_VALUE_DATE )
	     ||
	     gsb_form_check_entry_is_empty ( gsb_form_get_element_widget (TRANSACTION_FORM_VALUE_DATE,
									  account_number)))
	    {
		gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE,
									      account_number)),
				     gsb_date_today ());
		gsb_form_set_entry_is_empty ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE,
									   account_number),
					      FALSE );
	    }
    }

    /* set the number of cheque for the method of payment if necessary */
    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_TYPE )
	 &&
	 GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE,
							   account_number)))
    {
	gint payment_number;

	payment_number = gsb_payment_method_get_selected_number (gsb_form_get_element_widget (TRANSACTION_FORM_TYPE,
											      account_number));
	if ( gsb_payment_method_get_automatic_number ( payment_number,
						       account_number ))
	{
	    GtkWidget *widget;

	    widget = gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
						  account_number);

	    gsb_form_entry_get_focus (widget);

	    if ( !strlen (gtk_entry_get_text ( GTK_ENTRY (widget))))
		gtk_entry_set_text ( GTK_ENTRY (widget),
				     gsb_payment_method_automatic_numbering_get_new_number (payment_number,
											    account_number ));
	}
    }

    /* if ev is NULL, go away here
     * (means come from a function as gsb_form_finish_edition...) */
    if ( !ev )
	return FALSE;

    /* check for the calendar */
    switch ( element_number )
    {
	case TRANSACTION_FORM_DATE :
	case TRANSACTION_FORM_VALUE_DATE :
	    if ( ev -> type == GDK_2BUTTON_PRESS )
	    {
		gsb_calendar_new ( entry );
		return TRUE;
	    }
	    break;
    }
    return FALSE;
}


/**
 * sensitive or unsensitive all the buttons on the form
 *
 * \param sensitive TRUE or FALSE
 *
 * \return FALSE
 * */
gboolean gsb_form_change_sensitive_buttons ( gboolean sensitive )
{
    gint account_number;
    
    account_number = gsb_form_get_account_number ();

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_TYPE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE,
									     account_number)),
				   sensitive );

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_DEVISE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_DEVISE,
									     account_number)),
				   sensitive );

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_EXERCICE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_EXERCICE,
									     account_number)),
				   sensitive );
    return FALSE;
}


/**
 * called when press a key on an element of the form
 *
 * \param widget wich receive the signal
 * \param ev
 * \param ptr_origin a pointer number of the element
 * 
 * \return FALSE
 * */
gboolean gsb_form_key_press_event ( GtkWidget *widget,
				    GdkEventKey *ev,
				    gint *ptr_origin )
{
    gint element_number;
    gint account_number;
    gint element_suivant;

    element_number = GPOINTER_TO_INT (ptr_origin);
    account_number = gsb_form_get_account_number ();

    /* if etat.entree = 1, entry finish the transaction, else does as tab */
    if ( !etat.entree
	 &&
	 ( ev -> keyval == GDK_Return 
	   ||
	   ev -> keyval == GDK_KP_Enter ))
	ev->keyval = GDK_Tab ;

    switch ( ev -> keyval )
    {
	case GDK_Escape :
	    gsb_form_escape_form ();
	    break;

	case GDK_Up:
	    /* xxx marche pas pour les combofix */
	    element_suivant = gsb_form_get_next_element ( account_number,
							  element_number,
							  GSB_UP );
	    gsb_form_set_focus ( element_suivant );
	    return TRUE;
	    break;

	case GDK_Down:
	    /* xxx marche pas pour les combofix */
	    element_suivant = gsb_form_get_next_element ( account_number,
							  element_number,
							  GSB_DOWN );
	    gsb_form_set_focus ( element_suivant );
	    return TRUE;
	    break;

	case GDK_ISO_Left_Tab:
	    element_suivant = gsb_form_get_next_element ( account_number,
							  element_number,
							  GSB_LEFT );
	    gsb_form_set_focus ( element_suivant );
	    return TRUE;
	    break;

	case GDK_Tab :
	    element_suivant = gsb_form_get_next_element ( account_number,
							  element_number,
							  GSB_RIGHT );

	    if ( element_suivant == -2 )
		gsb_form_finish_edition();
	    else
		gsb_form_set_focus ( element_suivant );
	    return TRUE;
	    break;

	case GDK_KP_Enter :
	case GDK_Return :

	    /* CONTROL ENTER opens the calendar */
	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK
		 &&
		 ( element_number == TRANSACTION_FORM_DATE
		   ||
		   element_number == TRANSACTION_FORM_VALUE_DATE ))
	    {
		GtkWidget *popup_cal;
		popup_cal = gsb_calendar_new ( gsb_form_get_element_widget (element_number,
									      account_number ));
		gtk_widget_grab_focus (popup_cal);
		return TRUE;
	    }

	    gsb_form_finish_edition();
	    return TRUE;
	    break;

	case GDK_KP_Add:
	case GDK_plus:
	case GDK_equal:		/* This should make all our US users happy */

	    /* increase the date of 1 day/week, or the check of 1 */
	    switch ( element_number )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK ||
			 ev -> keyval != GDK_KP_Add )
			inc_dec_date ( widget, ONE_DAY );
		    else
			inc_dec_date ( widget, ONE_WEEK );
		    return TRUE;
		    break;

		case TRANSACTION_FORM_CHEQUE:

		    increment_decrement_champ ( widget,
						1 );
		    return TRUE;
		    break;
	    }
	    break;

	case GDK_KP_Subtract:
	case GDK_minus:

	    /* decrease the date of 1 day/week, or the check of 1 */
	    switch ( element_number )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK ||
			 ev -> keyval != GDK_KP_Subtract  )
			inc_dec_date ( widget, -ONE_DAY );
		    else
			inc_dec_date ( widget, -ONE_WEEK );
		    return TRUE;
		    break;

		case TRANSACTION_FORM_CHEQUE:
		    
		    increment_decrement_champ ( widget,
						-1 );
		    return TRUE;
		    break;
	    }
	    break;

	case GDK_Page_Up :
	case GDK_KP_Page_Up :

	    /* increase the date of 1 month/year */
	    switch ( element_number )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK )
			inc_dec_date ( widget,
				       ONE_MONTH );
		    else
			inc_dec_date ( widget,
				       ONE_YEAR );

		    return TRUE;
		    break;
	    }
	    break;

	case GDK_Page_Down :
	case GDK_KP_Page_Down :

	    /* decrease the date of 1 month/year */
	    switch ( element_number )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK )
			inc_dec_date ( widget,
				       -ONE_MONTH );
		    else
			inc_dec_date ( widget,
				       -ONE_YEAR );

		    return TRUE;
		    break;
	    }
	    break;
    }
    return FALSE;
}



/**
 * set the focus on the given element
 *
 * \param element_number
 *
 * \return
 * */
void gsb_form_set_focus ( gint element_number )
{
    GtkWidget *widget;

    devel_debug ( g_strdup_printf ( "gsb_form_set_focus %d",
				    element_number ));

    widget = gsb_form_get_element_widget ( element_number,
					   gsb_form_get_account_number ());

    if ( !widget )
	return;
    
    if ( GTK_IS_COMBOFIX ( widget ))
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget ) -> entry );
    else
	gtk_widget_grab_focus ( widget );

    return;
}


/**
 * return the number of the next valid element in the direction given in param
 *
 * \param account_number
 * \param element_number
 * \param direction GSB_LEFT, GSB_RIGHT, GSB_UP, GSB_DOWN
 *
 * \return 	the next element number
 * 		-1 if problem or not change
 * 		-2 if end of the form and need to finish it
 * */
gint gsb_form_get_next_element ( gint account_number,
				 gint element_number,
				 gint direction )
{
    gint row;
    gint column;
    gint return_value_number = 0;
    gint form_column_number;
    gint form_row_number;

    if ( !gsb_data_form_look_for_value ( account_number,
					 element_number,
					 &row,
					 &column ))
	return -1;

    form_column_number = gsb_data_form_get_nb_columns (account_number);
    form_row_number = gsb_data_form_get_nb_rows (account_number);

    while ( !gsb_form_element_can_receive_focus ( return_value_number,
						  account_number )) 
    {
	switch ( direction )
	{
	    case GSB_LEFT:
		if ( !column && !row )
		{
		    /* we are at the upper left, go on the bottom right */
		    column = form_column_number;
		    row = form_row_number -1; 
		}

		if ( --column == -1 )
		{
		    column = form_column_number - 1;
		    row--;
		}
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    case GSB_RIGHT:
		if ( column == (form_column_number - 1)
		     &&
		     row == (form_row_number - 1))
		{
		    /* we are on the bottom right, we finish the edition or
		     * go to the upper left */
		    if ( !etat.entree )
		    {
			return_value_number = -2;
			continue;
		    }
		    column = -1;
		    row = 0; 
		}

		if ( ++column == form_column_number )
		{
		    column = 0;
		    row++;
		}
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    case GSB_UP:
		if ( !row )
		{
		    return_value_number = -1;
		    continue;
		}

		row--;
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    case GSB_DOWN:

		if ( row == (form_row_number - 1))
		{
		    return_value_number = -1;
		    continue;
		}
		row++;
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    default:
		return_value_number = -1;
	}
    }
    return return_value_number;
}


/**
 * check if the given element can receive focus
 *
 * \param element_number
 * 
 * \return TRUE : can receive focus, or FALSE
 * */
gboolean gsb_form_element_can_receive_focus ( gint element_number,
					      gint account_number )
{
    GtkWidget *widget;

    /* if element_number is -1 or -2, the iteration while must
     * stop, so return TRUE */
    if ( element_number == -1
	 ||
	 element_number == -2 )
	return TRUE;

    widget = gsb_form_get_element_widget ( element_number,
					     account_number );

    if ( !widget )
	return FALSE;
    
    if ( !GTK_WIDGET_VISIBLE (widget))
	return FALSE;

    if ( !GTK_WIDGET_SENSITIVE (widget))
	return FALSE;

    if ( !(GTK_IS_COMBOFIX (widget)
	   ||
	   GTK_IS_ENTRY (widget)
	   ||
	   GTK_IS_BUTTON (widget)
	   ||
	   GTK_IS_COMBO_BOX (widget)))
	return FALSE;

    return TRUE;
}



/**
 * called when cancel a form
 *
 * \param 
 *
 * \return FALSE
 * */
gboolean gsb_form_escape_form ( void )
{
    if ( etat.formulaire_toujours_affiche )
	gsb_form_clean (gsb_form_get_account_number ());
    else
    {
	gsb_form_hide ();
	gtk_expander_set_expanded ( GTK_EXPANDER (form_expander),
				    FALSE );
    }
    gtk_widget_grab_focus (gsb_transactions_list_get_tree_view());
    return FALSE;
}




/**
 * get an element number and return its name
 *
 * \param element_number
 *
 * \return the name of the element or NULL if problem
 * */
gchar *gsb_form_get_element_name ( gint element_number )
{
    switch ( element_number )
    {
	case -1:
	    /* that value shouldn't be there, it shows that a gsb_data_form_... returns
	     * an error value */
	    warning_debug ( "gsb_form_get_element_name : a value in the form is -1 wich should not happen.\nA gsb_data_form_... function must have returned an error value..." );
	    return NULL;
	    break;

	case TRANSACTION_FORM_DATE:
	    return (N_("Date"));
	    break;

	case TRANSACTION_FORM_DEBIT:
	    return (N_("Debit"));
	    break;

	case TRANSACTION_FORM_CREDIT:
	    return (N_("Credit"));
	    break;

	case TRANSACTION_FORM_VALUE_DATE:
	    return (N_("Value date"));
	    break;

	case TRANSACTION_FORM_EXERCICE:
	    return (N_("Financial year"));
	    break;

	case TRANSACTION_FORM_PARTY:
	    return (N_("Payee"));
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    return (N_("Category line"));
	    break;

	case TRANSACTION_FORM_FREE:
	    return (N_("Free"));
	    break;

	case TRANSACTION_FORM_BUDGET:
	    return (N_("Budgetary line"));
	    break;

	case TRANSACTION_FORM_NOTES:
	    return (N_("Notes"));
	    break;

	case TRANSACTION_FORM_TYPE:
	    return (N_("Method of payment"));
	    break;

	case TRANSACTION_FORM_CHEQUE:
	    return (N_("Cheque/Transfer number"));
	    break;

	case TRANSACTION_FORM_DEVISE:
	    return (N_("Currency"));
	    break;

	case TRANSACTION_FORM_CHANGE:
	    return (N_("Change"));
	    break;

	case TRANSACTION_FORM_VOUCHER:
	    return (N_("Voucher"));
	    break;

	case TRANSACTION_FORM_BANK:
	    return (N_("Bank references"));
	    break;

	case TRANSACTION_FORM_CONTRA:
	    return (N_("Contra-transaction method of payment"));
	    break;

	case TRANSACTION_FORM_OP_NB:
	    return (N_("Transaction number"));
	    break;

	case TRANSACTION_FORM_MODE:
	    return (N_("Automatic/Manual"));
	    break;
    }
    return NULL;
}

/**
 * init the colors for the differents states of the entries
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_init_entry_colors ( void )
{
    GdkColor normal_color;
    GdkColor grey_color;

    normal_color.red = COULEUR_NOIRE_RED;
    normal_color.green = COULEUR_NOIRE_GREEN;
    normal_color.blue = COULEUR_NOIRE_BLUE;
    normal_color.pixel = 0;

    grey_color.red = COULEUR_GRISE_RED;
    grey_color.green = COULEUR_GRISE_GREEN;
    grey_color.blue = COULEUR_GRISE_BLUE;
    grey_color.pixel = 0;

    style_entree_formulaire[ENCLAIR] = gtk_style_new();
    style_entree_formulaire[ENCLAIR] -> text[GTK_STATE_NORMAL] = normal_color;

    style_entree_formulaire[ENGRIS] = gtk_style_new();
    style_entree_formulaire[ENGRIS] -> text[GTK_STATE_NORMAL] = grey_color;

    g_object_ref ( style_entree_formulaire[ENCLAIR] );
    g_object_ref ( style_entree_formulaire[ENGRIS] );

    return FALSE;
}

/**
 * set the size of the columns in the form, according to the user conf
 * and the size of the window
 *
 * \param table the table wich receive the 'size-allocate' signal
 * \param allocation
 *
 * \return FALSE
 * */
gboolean gsb_form_allocate_size ( GtkWidget *table,
				  GtkAllocation *allocation,
				  gpointer null )
{
    gint row, column;
    gint account_number;

    if (!gtk_expander_get_expanded (GTK_EXPANDER (form_expander)))
	return FALSE;

    account_number = gsb_form_get_account_number ();
    if ( account_number == -2 )
	return FALSE;

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column < gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    GtkWidget *widget;

	    widget = gsb_form_get_element_widget ( gsb_data_form_get_value ( account_number,
									       column,
									       row ),
						     account_number );

	    if ( widget )
		gtk_widget_set_usize ( widget,
				       gsb_data_form_get_width_column (account_number,
								       column ) * allocation -> width / 100,
				       FALSE );
	}

    for ( column = 0 ; column < 6 ; column++ )
    {
	gint width_percent = 0;

	switch (column)
	{
	    case SCHEDULED_FORM_ACCOUNT:
		width_percent = 30;
		break;

	    case SCHEDULED_FORM_AUTO:
		width_percent = 16;
		break;

	    case SCHEDULED_FORM_FREQUENCY_BUTTON:
		width_percent = 16;
		break;

	    case SCHEDULED_FORM_LIMIT_DATE:
		width_percent = 12;
		break;

	    case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		width_percent = 7;
		break;

	    case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		width_percent = 12;
		break;
	}
	gtk_widget_set_usize ( form_tab_scheduled[column],
			       width_percent * allocation -> width / 100,
			       FALSE );
    }
    return FALSE;
}

/**
 * check if the entry given in param is empty (ie grey) or not
 *
 * \param entry must be an entry
 *
 * \return TRUE : entry is free, FALSE : not free
 * */
gboolean gsb_form_check_entry_is_empty ( GtkWidget *entry )
{
    if (!entry)
	return FALSE;

    return (gtk_widget_get_style ( entry ) == style_entree_formulaire[ENGRIS] );
}


/**
 * set the style to the entry as empty or not
 *
 * \param entry must be an entry
 * \param empty TRUE or FALSE
 *
 * \return 
 * */
void gsb_form_set_entry_is_empty ( GtkWidget *entry,
				   gboolean empty )
{
    if (!entry)
	return;
    
    if (empty)
	gtk_widget_set_style ( entry,
			       style_entree_formulaire[ENGRIS] );
    else
	gtk_widget_set_style (  entry,
				style_entree_formulaire[ENCLAIR] );
}



