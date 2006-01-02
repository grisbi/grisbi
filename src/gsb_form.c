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
#include "exercice.h"
#include "type_operations.h"
#include "operations_formulaire.h"
#include "echeancier_formulaire.h"
#include "devises.h"
#include "erreur.h"
#include "utils_devises.h"
#include "comptes_traitements.h"
#include "calendar.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "navigation.h"
#include "utils_dates.h"
#include "gtk_combofix.h"
#include "utils_editables.h"
#include "main.h"
#include "utils_comptes.h"
#include "structures.h"
#include "traitement_variables.h"
#include "fenetre_principale.h"
#include "include.h"
#include "gsb_form.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_form_activate_expander ( GtkWidget *expander,
				      gpointer null );
static gboolean gsb_form_allocate_size ( GtkWidget *table,
				  GtkAllocation *allocation,
				  gpointer null );
static gboolean gsb_form_clean ( gint account_number );
static GtkWidget *gsb_form_create_element_from_number ( gint element_number,
						 gint account_number );
static gboolean gsb_form_fill_scheduled_part ( void );
static gboolean gsb_form_fill_transaction ( gint transaction_number );
static gboolean gsb_form_frequency_button_changed ( GtkWidget *combo_box,
					     gpointer null );
static gint gsb_form_get_account_from_button ( void );
static gint gsb_form_get_account_number_from_origin ( gint origin );
static GtkWidget *gsb_form_get_element_widget_2 ( gint element_number,
					   gint account_number );
static gint gsb_form_get_origin ( void );
static gboolean gsb_form_init_entry_colors ( void );
static gboolean gsb_form_valid ( void );
static gboolean touches_champ_formulaire ( GtkWidget *widget,
				    GdkEventKey *ev,
				    gint *ptr_origin );
/*END_STATIC*/

/*START_EXTERN*/
extern gchar *last_date;
extern GSList *liste_struct_devises;
extern FILE * out;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *window;
/*END_EXTERN*/



/** label of the last statement */
GtkWidget *label_last_statement;

/** the expander */
static GtkWidget *form_expander;

/** the 2 styles, grey or normal for the entries of the
 * form, need to be in static later */
GtkStyle *style_entree_formulaire[2];

/** the 3 parts of the form :
 * for scheduled transactions
 * for transactions
 * the buttons valid/cancel */
static GtkWidget *form_scheduled_part;
static GtkWidget *form_transaction_part;
static GtkWidget *form_button_part;

/** when the automatic complete transaction is done
 * for a breakdown of transaction, we propose to recover too
 * the children with that button */
static GtkWidget *form_button_recover_breakdown;

/** need to set the 2 buttons valid/cancel here and cannot
 * just show/hide the form_button_part because of the breakdown button */
static GtkWidget *form_button_valid;
static GtkWidget *form_button_cancel;

/** contains the pointeurs to the widget in the transaction part form */
static GtkWidget *form_tab_transactions[MAX_HEIGHT][MAX_WIDTH];

/** contains the pointeurs to the widget in the scheduled part form */
static GtkWidget *form_tab_scheduled[SCHEDULED_FORM_MAX_WIDGETS];

/* FIXME : next values need to be static and changed */

/* vbox contenant le séparateur et les boutons valider/annuler */
/* affichée ou non suivant la conf */

GtkWidget *vbox_boutons_formulaire;


/* contient les adresses des widgets dans le formulaire en fonction */
/* de leur place */


GtkWidget *formulaire;


/**
 * create an empty form in an gtk_expander
 *
 * \param
 *
 * \return the form
 * */
GtkWidget *gsb_form_new ( void )
{
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *separator;

    /* Create the expander */
    form_expander = gtk_expander_new ( "" );
    gtk_expander_set_expanded ( GTK_EXPANDER ( form_expander ),
				etat.formulaire_toujours_affiche );
    g_signal_connect ( G_OBJECT(form_expander),
		       "activate",
		       G_CALLBACK (gsb_form_activate_expander),
		       NULL );

    /* Expander has a composite label */
    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_expander_set_label_widget ( GTK_EXPANDER(form_expander), hbox );

    /* Kludge : otherwise, GtkExpander won't give us as many space
       as we need. */
    gtk_widget_set_size_request ( hbox, 2048, -1 );

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
				 10 );
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
    form_transaction_part = gtk_table_new ( 1, 1,
					    FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE (form_transaction_part),
				 10 );
    g_signal_connect ( G_OBJECT (form_transaction_part),
		       "size-allocate",
		       G_CALLBACK (gsb_form_allocate_size),
		       NULL );
    gtk_box_pack_start ( GTK_BOX (formulaire),
			 form_transaction_part,
			 FALSE, FALSE,
			 0 );

    /* add a separator between the transaction and button part */
    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX (formulaire),
			 separator,
			 FALSE, FALSE,
			 0 );

    /* the buttons part is a hbox, with the recuperate child breakdown
     * on the left and valid/cancel on the right */
    form_button_part = gtk_hbox_new ( FALSE,
				      0 );
    gtk_box_pack_start ( GTK_BOX (formulaire),
			 form_button_part,
			 FALSE, FALSE,
			 0 );

    /* create the check button to recover the children of breakdowns */
    form_button_recover_breakdown = gtk_check_button_new_with_label ( _("Recover the children"));
    gtk_box_pack_start ( GTK_BOX (form_button_part),
			 form_button_recover_breakdown,
			 FALSE, FALSE,
			 0 );

    /* create the valid/cancel buttons */
    form_button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON (form_button_cancel),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (form_button_cancel),
		       "clicked",
		       GTK_SIGNAL_FUNC (gsb_form_hide),
		       NULL );
    gtk_box_pack_end ( GTK_BOX (form_button_part),
		       form_button_cancel,
		       FALSE, FALSE,
		       0 );

    form_button_valid = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_button_set_relief ( GTK_BUTTON (form_button_valid),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (form_button_valid),
		       "clicked",
		       GTK_SIGNAL_FUNC (gsb_form_finish_edition),
		       NULL );
    gtk_box_pack_end ( GTK_BOX (form_button_part),
		       form_button_valid,
		       FALSE, FALSE,
		       0 );


    /* init the colors for the entries */
    gsb_form_init_entry_colors ();

    /* first show all in the form_expander and second hide the form_expander
     * because at the begining we are on the main page
     * and i'm too lazy to write gtk_widget_show everywhere... */
    gtk_widget_show_all (form_expander);
    gtk_widget_hide (form_expander);
    
    return form_expander;
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
	gsb_form_hide ();
    else
	gsb_form_show ();

    return FALSE;
}


/**
 * show the form, detect automaticly what we need to show, even for transactions,
 * scheduled_transactions and the buttons valid/cancel
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_show ( void )
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
    
    /* FIXME :transform that to be local variable here */
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show (form_button_part);

    return FALSE;
}

/**
 * return the account number according to the origin,
 * if we are on transactions list, return the current account,
 * if we are on scheduling list, return the selected account on button
 *
 * \param origin
 *
 * \return the account number or -2 if problem (-1 is reserved to get account from the button)
 * */
gint gsb_form_get_account_number_from_origin ( gint origin )
{
    gint account_number;

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
				       GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
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

	    widget = gsb_form_create_element_from_number ( gsb_data_form_get_value ( account_number,
										     column,
										     row ),
							   account_number );
	    form_tab_transactions[row][column] = widget;

	    if ( !widget )
		continue;

	    gtk_table_attach ( GTK_TABLE (form_transaction_part),
			       widget,
			       column, column+1,
			       row, row+1,
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
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
    GtkWidget *menu;

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
	    widget = gtk_option_menu_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the financial year"),
				   _("Choose the financial year") );
	    menu = gtk_menu_new ();
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
				       creation_menu_exercices (0) );
	    break;

	case TRANSACTION_FORM_PARTY:
	    widget = gtk_combofix_new_complex ( gsb_data_payee_get_name_and_report_list(),
						FALSE,
						TRUE,
						TRUE,
						30 );
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    widget = gtk_combofix_new_complex ( gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE),
						FALSE,
						TRUE,
						TRUE,
						30 );
	    break;

	case TRANSACTION_FORM_FREE:
	    break;

	case TRANSACTION_FORM_BUDGET:
	    widget = gtk_combofix_new_complex ( gsb_data_budget_get_name_list (TRUE, TRUE),
						FALSE,
						TRUE,
						TRUE,
						0 );

	    break;

	case TRANSACTION_FORM_TYPE:
	    widget = gtk_option_menu_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the method of payment"),
				   _("Choose the method of payment") );

	    /* set the debien menu by default */

	    if ( ( menu = creation_menu_types ( 1,
						account_number,
						0 )))
		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
					   menu );
	    break;

	case TRANSACTION_FORM_DEVISE:
	    widget = gtk_option_menu_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose currency"),
				   _("Choose currency") );
	    menu = creation_option_menu_devises ( -1,
						  liste_struct_devises );
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
				       menu );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
					  g_slist_index ( liste_struct_devises,
							  devise_par_no ( gsb_data_account_get_currency (account_number))));
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
	    widget = gtk_option_menu_new ();
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
				 GTK_SIGNAL_FUNC ( entree_prend_focus ),
				 GINT_TO_POINTER ( element_number ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "focus-out-event",
				 GTK_SIGNAL_FUNC ( entree_perd_focus ),
				 GINT_TO_POINTER ( element_number ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "button-press-event",
				 GTK_SIGNAL_FUNC ( clique_champ_formulaire ),
				 GINT_TO_POINTER ( element_number ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "key-press-event",
				 GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
				 GINT_TO_POINTER ( element_number ));
	}
	else
	{
	    if ( GTK_IS_COMBOFIX ( widget ))
	    {
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget ) -> entry ),
				     "focus-in-event",
				     GTK_SIGNAL_FUNC ( entree_prend_focus ),
				     GINT_TO_POINTER ( element_number ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "focus-out-event",
				     GTK_SIGNAL_FUNC ( entree_perd_focus ),
				     GINT_TO_POINTER ( element_number ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "button-press-event",
				     GTK_SIGNAL_FUNC ( clique_champ_formulaire ),
				     GINT_TO_POINTER ( element_number ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "key-press-event",
				     GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
				     GINT_TO_POINTER ( element_number ));
	    }
	    else
		/* neither an entry, neither a combofix */
		gtk_signal_connect ( GTK_OBJECT ( widget ),
				     "key-press-event",
				     GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
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
		gtk_widget_set_style ( widget,
				       style_entree_formulaire[ENGRIS] );
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     _("Limit date") );
		break;

	    case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		gtk_widget_set_style ( widget,
				       style_entree_formulaire[ENGRIS] );
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

	    widget =  gsb_form_get_element_widget_2 ( value,
						      account_number );

	    /* better to protect here if widget != NULL (bad experience...) */
	    if (widget)
		switch (value)
		{
		    case TRANSACTION_FORM_DATE:

			gtk_widget_set_sensitive ( widget,
						   TRUE );
			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Date") );
			break;

		    case TRANSACTION_FORM_VALUE_DATE:

			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Value date") );
			break;

		    case TRANSACTION_FORM_EXERCICE:

			gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
						   FALSE );
			break;

		    case TRANSACTION_FORM_PARTY:

			gtk_widget_set_sensitive ( widget,
						   TRUE );
			gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					       style_entree_formulaire[ENGRIS] );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Payee") );
			break;

		    case TRANSACTION_FORM_DEBIT:

			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Debit") );
			gtk_widget_set_sensitive ( widget,
						   TRUE );
			break;

		    case TRANSACTION_FORM_CREDIT:

			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Credit") );
			gtk_widget_set_sensitive ( widget,
						   TRUE );
			break;

		    case TRANSACTION_FORM_CATEGORY:

			gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					       style_entree_formulaire[ENGRIS] );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Categories : Sub-categories") );
			gtk_widget_set_sensitive ( widget,
						   TRUE );
			break;

		    case TRANSACTION_FORM_FREE:
			break;

		    case TRANSACTION_FORM_BUDGET:

			gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					       style_entree_formulaire[ENGRIS] );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Budgetary line") );
			break;

		    case TRANSACTION_FORM_NOTES:

			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Notes") );
			break;

		    case TRANSACTION_FORM_TYPE:

			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						      cherche_no_menu_type ( gsb_data_account_get_default_debit (gsb_gui_navigation_get_current_account ()) ) );
			gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
						   FALSE );
			break;

		    case TRANSACTION_FORM_CONTRA:

			gtk_widget_hide ( widget );

			break;

		    case TRANSACTION_FORM_CHEQUE:

			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Cheque/Transfer number") );
			break;

		    case TRANSACTION_FORM_DEVISE:

			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						      g_slist_index ( liste_struct_devises,
								      devise_par_no ( gsb_data_account_get_currency (gsb_gui_navigation_get_current_account ()) )));
			gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
						   FALSE );
			break;

		    case TRANSACTION_FORM_CHANGE:

			gtk_widget_hide ( widget );

			break;

		    case TRANSACTION_FORM_BANK:

			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     _("Bank references") );
			break;

		    case TRANSACTION_FORM_VOUCHER:

			gtk_widget_set_style ( widget,
					       style_entree_formulaire[ENGRIS] );
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
 * FIXME : transitionnal package, the purpose is to have element_number and account_number
 * in the param
 * xxx */
GtkWidget *gsb_form_get_element_widget ( gint element_number )
{
    return gsb_form_get_element_widget_2 ( element_number,
					   gsb_gui_navigation_get_current_account());
}

/**
 * return the pointer to the widget corresponding to the given element
 *
 * \param element_number
 * \param account_number
 *
 * \return the widget or NULL
 * */
GtkWidget *gsb_form_get_element_widget_2 ( gint element_number,
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


/******************************************************************************/
/* Fonction appelée quand une entry prend le focus */
/* si elle contient encore des éléments grisés, on les enlève */
/******************************************************************************/
gboolean entree_prend_focus ( GtkWidget *entree,
			     GdkEventFocus *ev,
			     gint *ptr_origin )
{
    /*     l'entrée qui arrive peut être un combofix */
    /* si le style est le gris, on efface le contenu de l'entrée, sinon on fait rien */

    if ( GTK_IS_ENTRY ( entree ))
    {
	if ( gtk_widget_get_style ( entree ) == style_entree_formulaire[ENGRIS] )
	{
	    gtk_entry_set_text ( GTK_ENTRY ( entree ), "" );
	    gtk_widget_set_style ( entree, style_entree_formulaire[ENCLAIR] );
	}
    }
    else
    {
	if ( gtk_widget_get_style ( GTK_COMBOFIX (entree) -> entry ) == style_entree_formulaire[ENGRIS] )
	{
	    gtk_combofix_set_text ( GTK_COMBOFIX ( entree ), "" );
	    gtk_widget_set_style ( GTK_COMBOFIX (entree) -> entry, style_entree_formulaire[ENCLAIR] );
	}
    }


    return FALSE;
}
/******************************************************************************/



/******************************************************************************/
/* Fonction appelée quand une entry perd le focus */
/* si elle ne contient rien, on remet la fonction en gris */
/******************************************************************************/
gboolean entree_perd_focus ( GtkWidget *entree,
			     GdkEventFocus *ev,
			     gint *ptr_origin )
{
    gchar *texte;

    /*     on retire la sélection */

    gtk_editable_select_region ( GTK_EDITABLE ( entree ),
				 0,
				 0 );

    /*     on efface la popup du combofix si elle est affichée(arrive parfois pas trouvé pourquoi) */

    if ( GPOINTER_TO_INT ( ptr_origin ) == TRANSACTION_FORM_PARTY
	 ||
	 GPOINTER_TO_INT ( ptr_origin ) == TRANSACTION_FORM_CATEGORY
	 ||
	 GPOINTER_TO_INT ( ptr_origin ) == TRANSACTION_FORM_BUDGET )
    {
	GtkWidget *combofix;

	combofix = gsb_form_get_element_widget ( GPOINTER_TO_INT ( ptr_origin ));

	gtk_grab_remove ( GTK_COMBOFIX ( combofix ) -> popup );
	gdk_pointer_ungrab ( GDK_CURRENT_TIME );
	gtk_widget_hide ( GTK_COMBOFIX ( combofix ) ->popup );
    }

    texte = NULL;

    switch ( GPOINTER_TO_INT ( ptr_origin ) )
    {
	/* on sort de la date, soit c'est vide, soit on la vérifie,
	   la complète si nécessaire et met à jour l'exercice */
	case TRANSACTION_FORM_DATE :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		modifie_date ( entree );

		/* si c'est une modif d'opé, on ne change pas l'exercice */
		/* 		si on n'utilise pas l'exo, la fonction ne fera rien */

		if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire ),
					    "transaction_number_in_form" ))
		    affiche_exercice_par_date( gsb_form_get_element_widget (TRANSACTION_FORM_DATE),
					       gsb_form_get_element_widget (TRANSACTION_FORM_EXERCICE) );
	    }
	    else
		texte = _("Date");
	    break;

	    /* on sort du tiers : soit vide soit complète le reste de l'opé */

	case TRANSACTION_FORM_PARTY :

	    if ( !completion_operation_par_tiers ( entree ))
		texte = _("Payee");
	    break;

	    /* on sort du débit : soit vide, soit change le menu des types s'il ne correspond pas */

	case TRANSACTION_FORM_DEBIT :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		/* on commence par virer ce qu'il y avait dans les crédits */

		if ( gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT) ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT) ),
					 "" );
		    entree_perd_focus ( gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT),
					NULL,
					GINT_TO_POINTER ( TRANSACTION_FORM_CREDIT ) );
		}

		/* si c'est un menu de crédit, on y met le menu de débit */

		if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_TYPE ))
		{
		    if ( ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) )
			   &&
			   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ) -> menu ),
								   "signe_menu" ))
			   ==
			   2 )
			 ||
			 !GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ))
		    {
			/* on crée le nouveau menu et on met le défaut */

			GtkWidget *menu;

			if ( ( menu = creation_menu_types ( 1, gsb_gui_navigation_get_current_account (), 0  )))
			{
			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ),
						       menu );
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( gsb_data_account_get_default_debit (gsb_gui_navigation_get_current_account ()) ) );
			    gtk_widget_show ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) );
			}
			else
			{
			    gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) );
			    gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) );
			}

			/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
			/* s'il est affiché */

			if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA )
			     &&
			     GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) )
			     &&
			     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ) -> menu ),
								     "signe_menu" ))
			     ==
			     1 )
			{
			    GtkWidget *menu;

			    menu = creation_menu_types ( 2,
							 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ),
												 "account_transfer" )),
							 2  );

			    if ( menu )
				gtk_option_menu_set_menu ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ),
							   menu );
			    else
				gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ));
			}
		    }	
		    else
		    {
			/* on n'a pas recréé de menu, donc soit c'est déjà un menu de débit, soit tous les types */
			/* sont affichés, soit le widget n'est pas visible */
			/* on met donc le défaut, sauf si il y a qque chose dans les categ ou que le widget n'est pas visible */

			if ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) )
			     &&
			     ( !gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY )
			       ||
			       gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) ) == style_entree_formulaire[ENGRIS] ))
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( gsb_data_account_get_default_debit (gsb_gui_navigation_get_current_account ()) ) );
		    }
		}
	    }
	    else
		texte = _("Debit");
	    break;

	    /* on sort du crédit : soit vide, soit change le menu des types
	       s'il n'y a aucun tiers ( <=> nouveau tiers ) */

	case TRANSACTION_FORM_CREDIT :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		/* on commence par virer ce qu'il y avait dans les débits */

		if ( gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_DEBIT) ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_DEBIT) ),
					 "" );
		    entree_perd_focus ( gsb_form_get_element_widget (TRANSACTION_FORM_DEBIT),
					NULL,
					GINT_TO_POINTER ( TRANSACTION_FORM_DEBIT ));
		}

		/* si c'est un menu de crédit, on y met le menu de débit,
		   sauf si tous les types sont affichés */

		if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_TYPE ))
		{
		    if ( ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) )
			   &&
			   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ) -> menu ),
								   "signe_menu" )) == 1 ) ||
			 !GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ))
		    {
			/* on crée le nouveau menu et on met le défaut */

			GtkWidget *menu;

			if ( ( menu = creation_menu_types ( 2, gsb_gui_navigation_get_current_account (), 0  ) ) )
			{
			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ),
						       menu );
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( gsb_data_account_get_default_credit (gsb_gui_navigation_get_current_account ()) ) );
			    gtk_widget_show ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) );
			}
			else
			{
			    gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) );
			    gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) );
			}

			/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
			/* s'il est affiché */

			if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA )
			     &&
			     GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) )
			     &&
			     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ) -> menu ),
								     "signe_menu" )) == 2 )
			{
			    GtkWidget *menu;

			    menu = creation_menu_types ( 1,
							 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ),
												 "account_transfer" )),
							 2  );

			    if ( menu )
				gtk_option_menu_set_menu ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ),
							   menu );
			    else
				gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ));
			}
		    }
		    else
		    {
			/* on n'a pas recréé de menu, donc soit c'est déjà un menu de débit, soit tous les types */
			/* sont affichés, soit le widget n'est pas visible */
			/* on met donc le défaut, sauf si il y a qque chose dans les categ ou que le widget n'est pas visible */


			if ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) )
			     &&
			     ( !gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY )
			       ||
			       gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) ) == style_entree_formulaire[ENGRIS] ))
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( gsb_data_account_get_default_credit (gsb_gui_navigation_get_current_account ()) ) );
		    }
		}
	    }
	    else
		texte = _("Credit");
	    break;

	case TRANSACTION_FORM_VALUE_DATE :
	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		modifie_date ( entree );
	    else
		texte = _("Value date");
	    break;

	    /* si c'est un virement affiche le bouton des types de l'autre compte */

	case TRANSACTION_FORM_CATEGORY :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		if ( strcmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) ))),
			      _("Breakdown of transaction") ))
		{
		    gchar **tab_char;

		    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_EXERCICE ))
			gtk_widget_set_sensitive ( gsb_form_get_element_widget (TRANSACTION_FORM_EXERCICE),
						   TRUE );
		    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_BUDGET ))
			gtk_widget_set_sensitive ( gsb_form_get_element_widget (TRANSACTION_FORM_BUDGET),
						   TRUE );

		    /* vérification que ce n'est pas un virement */

		    tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) )),
						":",
						2 );

		    tab_char[0] = g_strstrip ( tab_char[0] );

		    if ( tab_char[1] )
			tab_char[1] = g_strstrip ( tab_char[1] );


		    if ( strlen ( tab_char[0] ) )
		    {
			if ( !strcmp ( tab_char[0],
				       _("Transfer") )
			     && tab_char[1]
			     && strlen ( tab_char[1] ) )
			{
			    /* c'est un virement : on recherche le compte associé et on affiche les types de paiement */

			    if ( strcmp ( tab_char[1],
					  _("Deleted account") ) )
			    {
				/* recherche le no de compte du virement */

				gint account_transfer;
				GSList *list_tmp;

				account_transfer = -1;
				list_tmp = gsb_data_account_get_list_accounts ();

				while ( list_tmp )
				{
				    gint i;

				    i = gsb_data_account_get_no_account ( list_tmp -> data );

				    if ( !g_strcasecmp ( gsb_data_account_get_name (i),
							 tab_char[1] ) )
					account_transfer = i;

				    list_tmp = list_tmp -> next;
				}

				/* si on a touvé un compte de virement, que celui ci n'est pas le compte */
				/* courant et que son menu des types n'est pas encore affiché, on crée le menu */

				if ( account_transfer != -1
				     &&
				     account_transfer != gsb_gui_navigation_get_current_account ()
				     &&
				     gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA ))
				{
				    /* si le menu affiché est déjà celui du compte de virement, on n'y touche pas */

				    if ( !GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) )
					 ||
					 ( recupere_no_compte ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) )
					   !=
					   account_transfer ))
				    {
					/* vérifie quel est le montant entré, affiche les types opposés de l'autre compte */

					GtkWidget *menu;

					if ( gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT) ) == style_entree_formulaire[ENCLAIR] )
					    /* il y a un montant dans le crédit */
					    menu = creation_menu_types ( 1, account_transfer, 2  );
					else
					    /* il y a un montant dans le débit ou défaut */
					    menu = creation_menu_types ( 2, account_transfer, 2  );

					/* si un menu à été créé, on l'affiche */

					if ( menu )
					{
					    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ),
								       menu );
					    gtk_widget_show ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) );
					}

					/* on associe le no de compte de virement au formulaire pour le retrouver */
					/* rapidement s'il y a un chgt débit/crédit */

					gtk_object_set_data ( GTK_OBJECT ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ),
							      "account_transfer",
							      GINT_TO_POINTER ( account_transfer ));
				    }
				}
				else
				    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA ))
					gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) );
			    }
			    else
				if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA ))
				    gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) );
			}
			else
			    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA ))
				gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) );
		    }
		    else
			if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA ))
			    gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) );

		    g_strfreev ( tab_char );
		}
	    }
	    else
		texte = _("Categories : Sub-categories");

	    break;

	case TRANSACTION_FORM_CHEQUE :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Cheque/Transfer number");
	    break;

	case TRANSACTION_FORM_BUDGET :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Budgetary line");
	    break;

	case TRANSACTION_FORM_VOUCHER :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Voucher");

	    break;

	case TRANSACTION_FORM_NOTES :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Notes");
	    break;

	case TRANSACTION_FORM_BANK :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Bank references");
	    break;
	default :
	    break;
    }

    /* si l'entrée était vide, on remet le défaut */
    /* si l'origine était un combofix, il faut remettre le texte */
    /* avec le gtk_combofix (sinon risque de complétion), donc utiliser l'origine */

    if ( texte )
    {
	switch ( GPOINTER_TO_INT ( ptr_origin ))
	{
	    case TRANSACTION_FORM_PARTY :
	    case TRANSACTION_FORM_CATEGORY :
	    case TRANSACTION_FORM_BUDGET :
		gtk_combofix_set_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (GPOINTER_TO_INT ( ptr_origin )) ),
					texte );
		break;

	    default:

		gtk_entry_set_text ( GTK_ENTRY ( entree ), texte );
		break;
	}
	gtk_widget_set_style ( entree, style_entree_formulaire[ENGRIS] );
    }
    return FALSE;
}
/******************************************************************************/



/******************************************************************************/
gboolean clique_champ_formulaire ( GtkWidget *entree,
				   GdkEventButton *ev,
				   gint *ptr_origin )
{
    GtkWidget *popup_cal;
    struct struct_type_ope *type;

    /* on rend sensitif tout ce qui ne l'était pas sur le formulaire */

    degrise_formulaire_operations ();

    /* si l'entrée de la date est grise, on met la date courante seulement
       si la date réelle est grise aussi. Dans le cas contraire,
       c'est elle qui prend le focus */

    if ( gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE) ) == style_entree_formulaire[ENGRIS] )
    {
	if ( !gsb_data_form_check_for_value ( TRANSACTION_FORM_VALUE_DATE )
	     ||
	     gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_VALUE_DATE) ) == style_entree_formulaire[ENGRIS] )
	    {
		if ( !last_date )
		    last_date = gsb_today();

		gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE) ),
				     last_date );
		gtk_widget_set_style ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE),
				       style_entree_formulaire[ENCLAIR] );
	    }
    }

    /*     we set the exercice */

    if ( gsb_form_get_element_widget(TRANSACTION_FORM_EXERCICE) )
	affiche_exercice_par_date ( gsb_form_get_element_widget(TRANSACTION_FORM_DATE),
				    gsb_form_get_element_widget(TRANSACTION_FORM_EXERCICE));



    /*   si le type par défaut est un chèque, on met le nouveau numéro */

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_TYPE )
	 &&
	 GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ))
    {
	type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ) -> menu_item ),
				     "adr_type" );

	if ( type -> numerotation_auto )
	{
	    entree_prend_focus ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE), NULL, NULL );

	    if ( !strlen ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) ))))
		gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) ),
				     automatic_numbering_get_new_number ( type ) );
	}
    }

    /* si ev est null ( cad que ça ne vient pas d'un click mais appelé par ex
       à la fin de gsb_form_finish_edition ), on se barre */

    if ( !ev )
	return FALSE;

    /* énumération suivant l'entrée où on clique */

    switch ( GPOINTER_TO_INT ( ptr_origin ) )
    {
	case TRANSACTION_FORM_DATE :
	case TRANSACTION_FORM_VALUE_DATE :

	    /* si double click, on popup le calendrier */
	    if ( ev -> type == GDK_2BUTTON_PRESS )
	    {
		popup_cal = gsb_calendar_new ( entree );
		return TRUE;
	    }
	    break;

	default :

	    break;
    }
    return FALSE;
}
/******************************************************************************/


/******************************************************************************/
gboolean touches_champ_formulaire ( GtkWidget *widget,
				    GdkEventKey *ev,
				    gint *ptr_origin )
{
    gint origine;
    gint element_suivant;
    GtkWidget *popup_cal;

    origine = GPOINTER_TO_INT ( ptr_origin );

    /* si etat.entree = 1, la touche entrée finit l'opération ( fonction par défaut ) */
    /* sinon elle fait comme tab */

    if ( !etat.entree && ( ev -> keyval == GDK_Return || ev -> keyval == GDK_KP_Enter ))
	ev->keyval = GDK_Tab ;

    switch ( ev -> keyval )
    {
	case GDK_Escape :		/* échap */

	    echap_formulaire();
	    break;


	    /* 	    touche haut */

	case GDK_Up:

	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     2 );
	    widget_grab_focus_formulaire ( element_suivant );
	    return TRUE;
	    break;

	    /* 	    touche bas */

	case GDK_Down:

	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     3 );
	    widget_grab_focus_formulaire ( element_suivant );
	    return TRUE;
	    break;


	    /* 	    tabulation inversée (+SHIFT) */

	case GDK_ISO_Left_Tab:

	    /* 	    on passe au widget précédent */

	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     0 );
	    widget_grab_focus_formulaire ( element_suivant );

	    return TRUE;
	    break;

	    /* tabulation */

	case GDK_Tab :

	    /* une tabulation passe au widget affiché suivant */
	    /* et retourne à la date ou enregistre l'opé s'il est à la fin */

	    /* 	    si on est sur le débit et qu'on a entré qque chose dedans, on efface le crédit */

	    if ( origine == TRANSACTION_FORM_DEBIT
		 &&
		 strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
	    {
		gtk_widget_set_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT),
				       style_entree_formulaire[ENGRIS] );
		gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CREDIT) ),
				     _("Credit") );
	    }
	    
	    /* 	    si on est sur le crédit et qu'on a entré qque chose dedans, on efface le débit */

	    if ( origine == TRANSACTION_FORM_CREDIT
		 &&
		 strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
	    {
		gtk_widget_set_style ( gsb_form_get_element_widget (TRANSACTION_FORM_DEBIT),
				       style_entree_formulaire[ENGRIS] );
		gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_DEBIT) ),
				     _("Debit") );
	    }


	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     1 );

	    if ( element_suivant == -2 )
		gsb_form_finish_edition();
	    else
		widget_grab_focus_formulaire ( element_suivant );

	    return TRUE;
	    break;

	    /* touches entrée */

	case GDK_KP_Enter :
	case GDK_Return :

	    /* si la touche CTRL est elle aussi active et si on est sur un champ
	     * de date, c'est que l'on souhaite ouvrir un calendrier */

	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK
		 &&
		 ( origine == TRANSACTION_FORM_DATE
		   ||
		   origine == TRANSACTION_FORM_VALUE_DATE ))
	    {
		popup_cal = gsb_calendar_new ( GTK_WIDGET ( GTK_ENTRY ( gsb_form_get_element_widget (origine) ) ) );
		gtk_widget_grab_focus ( GTK_WIDGET ( popup_cal ) );
		return TRUE;
	    }

	    gsb_form_finish_edition();

	    return TRUE;
	    break;

	    /* touches + */

	case GDK_KP_Add:
	case GDK_plus:

	    /* si on est dans une entree de date, on augmente d'un jour
	       ou d'une semaine(si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

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


	    /* touches - */

	case GDK_KP_Subtract:
	case GDK_minus:

	    /* si on est dans une entree de date, on diminue d'un jour
	       ou d'une semaine(si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

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



	    /* touche PgUp */

	case GDK_Page_Up :
	case GDK_KP_Page_Up :

	    /* si on est dans une entree de date, on augmente d'un mois 
	       ou d'un an (si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

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


	    /* touche PgDown */

	case GDK_Page_Down :
	case GDK_KP_Page_Down :

	    /* si on est dans une entree de date, on diminue d'un mois 
	       ou d'un an (si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

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
/******************************************************************************/



/**
 * check the values in the form and valid them
 *
 * \param
 *
 * \return TRUE if the form is ok, FALSE else
 * */
gboolean gsb_form_valid ( void )
{
    gint origin;

    origin = gsb_form_get_origin ();


    return TRUE;
}


/**
 * get the values in the form and fill the given transaction
 *
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_form_fill_transaction ( gint transaction_number )
{
    gint origin;

    origin = gsb_form_get_origin ();



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
	    return (N_("Categories"));
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

    account_number = gsb_form_get_account_number_from_origin (gsb_form_get_origin ());
    if ( account_number == -2 )
	return FALSE;

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column < gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    GtkWidget *widget;

	    widget = gsb_form_get_element_widget_2 ( gsb_data_form_get_value ( account_number,
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
	gint width_percent;

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
