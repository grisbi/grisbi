/* ************************************************************************** */
/*  ce fichier contient les paramètres de l'affichage de la liste d'opé       */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2008 Benjamin Drieu (bdrieu@april.org)                            */
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
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "affichage_liste.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "navigation.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_str.h"
#include "structures.h"
#include "custom_list.h"
#include "fenetre_principale.h"
#include "gsb_data_form.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean display_mode_button_changed ( GtkWidget *button,
                        gint *line_ptr );
static gboolean gsb_transactions_list_display_change_max_items ( GtkWidget *entry,
                        gpointer null );
static void gsb_transactions_list_display_show_gives_balance ( void );
static gboolean gsb_transactions_list_display_sort_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gint *pointeur );
static gboolean gsb_transactions_list_display_update_auto_checkbutton ( GtkWidget *checkbutton,
                        GtkWidget *container );
static gboolean gsb_transactions_list_display_update_combofix ( void );
/*END_STATIC*/



gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];

/* line displayed when the list show 1 line */
gint display_one_line;

/* lines displayed when the list show 2 lines
 * this is a number 0-5 according the order of the combo-box in the configuration */
gint display_two_lines;

/* lines displayed when the list show 3 lines
 * this is a number 0-3 according the order of the combo-box in the configuration */
gint display_three_lines;



/*START_EXTERN*/
extern GSList *liste_labels_titres_colonnes_liste_ope;
extern gchar *tips_col_liste_operations[CUSTOM_MODEL_N_VISIBLES_COLUMN];
extern gchar *titres_colonnes_liste_operations[CUSTOM_MODEL_N_VISIBLES_COLUMN];
/*END_EXTERN*/




/**
 * create the page of configuration for the transaction list behavior
 *
 * \param
 *
 * \return a vbox containing the configuration page
 * */
GtkWidget *onglet_affichage_operations ( void )
{
    GtkWidget * vbox_pref, *label, *paddingbox;
    GtkWidget *hbox, *vbox_label, *vbox_buttons;
    gchar *display_mode_lines_text [] = {
    _("In one line visible, show the line: "),
    _("In two lines visibles, show the lines: "),
    _("In three lines visibles, show the lines: "),
    };
    gchar *line_1 [] = {
    "1", "2", "3", "4",
    NULL };
    gchar *line_2 [] = {
    "1-2", "1-3", "1-4", "2-3", "2-4", "3-4",
    NULL };
    gchar *line_3 [] = {
    "1-2-3", "1-2-4", "1-3-4", "2-3-4",
    NULL };
    gint i;

    vbox_pref = new_vbox_with_title_and_icon ( _("Transaction list behavior"),
                        "transaction-list.png" );

    /* heading and boxes for layout */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _( "Display modes" ) );

    hbox = gtk_hbox_new ( FALSE, 5);
    vbox_label = gtk_vbox_new ( TRUE, 5);
    vbox_buttons = gtk_vbox_new ( TRUE, 5);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    /* fill the table */
    for (i=0 ; i<3 ; i++)
    {
    gint j;
    GtkWidget *button;
    gchar **text_line = NULL;
    gint position = 0;

    /* set the line title */
    label = gtk_label_new ( display_mode_lines_text[i] );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX (vbox_label), label, FALSE, FALSE, 0);

    switch (i)
    {
        case 0:
        text_line = line_1;
        position = display_one_line;
        break;
        case 1:
        text_line = line_2;
        position = display_two_lines;
        break;
        case 2:
        text_line = line_3;
        position = display_three_lines;
        break;
    }

    button = gtk_combo_box_new_text ();
    g_signal_connect ( G_OBJECT (button), "changed",
            G_CALLBACK (display_mode_button_changed),
            GINT_TO_POINTER (i));
    gtk_box_pack_start ( GTK_BOX (vbox_buttons), button, FALSE, FALSE, 0);

    j=0;
    while (text_line[j])
    {
        gtk_combo_box_append_text (GTK_COMBO_BOX (button), text_line[j]);
        j++;
    }
    gtk_combo_box_set_active ( GTK_COMBO_BOX (button), position);
    }

    /* pack vboxes in hbox */
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox_label, FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox_buttons, FALSE, FALSE, 0 );

    /* do we show the content of the selected transaction in the form for
     * each selection ? */
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_automem_checkbutton_new (
                        _("Use simple click to select transactions"),
                        &conf.show_transaction_selected_in_form,
                        NULL, NULL ),
                        FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_automem_checkbutton_new (
                        _("Highlights the transaction that gives the balance today"),
                        &conf.show_transaction_gives_balance,
                        G_CALLBACK ( gsb_transactions_list_display_show_gives_balance ), NULL ),
                        FALSE, FALSE, 0 );

    /* Primary sorting option for the transactions */
    gsb_automem_radiobutton3_new_with_title ( vbox_pref,
                        _("Primary sorting option"),
                        _("Sort by value date (if fail, try with the date)"),
                        _("Sort by value date and then by date"),
                        NULL,
                        &conf.transactions_list_primary_sorting,
                        G_CALLBACK ( gsb_transactions_list_display_sort_changed ),
                        &conf.transactions_list_primary_sorting,
                        GTK_ORIENTATION_VERTICAL );

    /* Secondary sorting option for the transactions */
    gsb_automem_radiobutton3_new_with_title ( vbox_pref,
                        _("Secondary sorting option"),
                        _("Sort by transaction number"),
                        _("Sort by type of amount (credit debit)"),
                        _("Sort by payee name (if fail, by transaction number)"),
                        &conf.transactions_list_secondary_sorting,
                        G_CALLBACK ( gsb_transactions_list_display_sort_changed ),
                        &conf.transactions_list_secondary_sorting,
                        GTK_ORIENTATION_VERTICAL );

    /* Account distinction */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
                        _("Account differentiation"));

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_automem_checkbutton_new (
                        _("Remember display settings for each account separately"),
                        &etat.retient_affichage_par_compte,
                        NULL, NULL ),
                        FALSE, FALSE, 0 );

    if ( !gsb_data_account_get_accounts_amount () )
    {
        gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}


/**
 *
 *
 * */
gboolean gsb_transactions_list_display_sort_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gint *pointeur )
{
    gint page_number;
    gint account_nb;

    page_number = gsb_gui_navigation_get_current_page ( );

    if ( pointeur )
    {
        gint value = 0;

        value = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( checkbutton ), "pointer" ) );
        *pointeur = value;
        if ( etat.modification_fichier == 0 )
            modification_fichier ( TRUE );
    }

    switch ( page_number )
    {
	case GSB_ACCOUNT_PAGE:
        account_nb = gsb_gui_navigation_get_current_account ();
        if (account_nb != -1)
            gsb_transactions_list_update_tree_view (account_nb, TRUE);
	    break;
    }

    return FALSE;
}


/**
 * called when we change a button for the display mode
 *
 * \param button 	the combo box wich changed
 * \param line_ptr	a gint* wich is the line of the button (ie 1 line mode, 2 lines or 3 lines)
 *
 * \return FALSE
 * */
static gboolean display_mode_button_changed ( GtkWidget *button,
                        gint *line_ptr )
{
    gint line = GPOINTER_TO_INT (line_ptr);

    switch (line)
    {
	case 0:
	    /* 1 line visible mode */
	    display_one_line = gtk_combo_box_get_active (GTK_COMBO_BOX (button));
	    break;
	case 1:
	    /* 2 lines visibles mode */
	    display_two_lines = gtk_combo_box_get_active (GTK_COMBO_BOX (button));
	    break;
	case 2:
	    /* 3 lines visibles mode */
	    display_three_lines = gtk_combo_box_get_active (GTK_COMBO_BOX (button));
	    break;
    }

    /* update the visible account */
    gsb_transactions_list_update_tree_view (gsb_gui_navigation_get_current_account (), TRUE);

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return FALSE;
}


/**
 * check if the line given in param, according to the number of visible lines
 * if visibles_lines is 4, that function returns always TRUE
 *
 * \param line_in_transaction	the line in the transaction we want to check
 * \param visibles_lines	the number of visibles lines (1, 2, 3 or 4)
 *
 * \return TRUE : the line should be showed, FALSE : the line must be hidden
 * */
gboolean display_mode_check_line ( gint line_in_transaction,
                        gint visibles_lines )
{
    switch (visibles_lines)
    {
	case 1:
	    /* 1 line visible mode */
	    if (line_in_transaction == display_one_line)
		return TRUE;
	    break;
	case 2:
	    /* 2 lines visibles mode */
	    switch (display_two_lines)
	    {
		case 0:
		    /* show lines 1-2 */
		    if (line_in_transaction == 0 || line_in_transaction == 1)
			return TRUE;
		    break;
		case 1:
		    /* show lines 1-3 */
		    if (line_in_transaction == 0 || line_in_transaction == 2)
			return TRUE;
		    break;
		case 2:
		    /* show lines 1-4 */
		    if (line_in_transaction == 0 || line_in_transaction == 3)
			return TRUE;
		    break;
		case 3:
		    /* show lines 2-3 */
		    if (line_in_transaction == 1 || line_in_transaction == 2)
			return TRUE;
		    break;
		case 4:
		    /* show lines 2-4 */
		    if (line_in_transaction == 1 || line_in_transaction == 3)
			return TRUE;
		    break;
		case 5:
		    /* show lines 3-4 */
		    if (line_in_transaction == 2 || line_in_transaction == 3)
			return TRUE;
		    break;
	    }
	    break;
	case 3:
	    /* 3 lines visibles mode */
	    switch (display_three_lines)
	    {
		case 0:
		    /* show lines 1-2-3 */
		    if (line_in_transaction == 0 || line_in_transaction == 1 || line_in_transaction == 2)
			return TRUE;
		    break;
		case 1:
		    /* show lines 1-2-4 */
		    if (line_in_transaction == 0 || line_in_transaction == 1 || line_in_transaction == 3)
			return TRUE;
		    break;
		case 2:
		    /* show lines 1-3-4 */
		    if (line_in_transaction == 0 || line_in_transaction == 2 || line_in_transaction == 3)
			return TRUE;
		    break;
		case 3:
		    /* show lines 2-3-4 */
		    if (line_in_transaction == 1 || line_in_transaction == 2 || line_in_transaction == 3)
			return TRUE;
		    break;
	    }
	    break;
	default:
	    /* here all the lines should be visible (visibles_lines = 4) */
	    return TRUE;
    }
    return FALSE;
}


/**
 * fill the titres_colonnes_liste_operations variable and the associated tips
 * so get the first row element name and set it for title
 * fill the tips with all the elements of the column
 *
 * \param
 *
 * \return
 * */
void recuperation_noms_colonnes_et_tips ( void )
{
    gint i, j;
    gchar *row[CUSTOM_MODEL_VISIBLE_COLUMNS];

    /* unset the titles and tips */
    for ( j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
    {
	titres_colonnes_liste_operations[j] = NULL;
	tips_col_liste_operations[j] = NULL;
    }


    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	for ( j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
	{
	    /* 	    xxx changer ça pour faire une fonction comme gsb_form_widget_get_name */
	    row[j] = _(g_slist_nth_data ( liste_labels_titres_colonnes_liste_ope,
					tab_affichage_ope[i][j] - 1 ));

	    /* on the first row, set for titles and tips, for others row, only for tips */
	    if ( i )
	    {
		if ( row[j] )
		{
		    if ( !titres_colonnes_liste_operations[j] )
			titres_colonnes_liste_operations[j] = row[j];

		    if ( tips_col_liste_operations[j] )
			tips_col_liste_operations[j] = g_strconcat ( tips_col_liste_operations[j],
								     "- ",
								     row[j], " ",
								     NULL );
		    else
			tips_col_liste_operations[j] = g_strconcat ( " ", row[j], " ", NULL );
		}
	    }
	    else
	    {
		if ( row[j] )
		{
		    titres_colonnes_liste_operations[j] = row[j];
		    tips_col_liste_operations[j] = g_strconcat ( " ", row[j], " ", NULL );
		}
	    }
	}
}




/* ************************************************************************************************************** */
/* renvoie le widget contenu dans l'onglet divers du gsb_form_get_form_widget ()/liste des paramètres */
/* ************************************************************************************************************** */
GtkWidget *onglet_diverse_form_and_lists ( void )
{
    GtkWidget *vbox_pref, *paddingbox, *radiogroup;

    vbox_pref = new_vbox_with_title_and_icon ( _("Form behavior"),
					       "form.png" );

    /* What to do if RETURN is pressed into transaction form */
    radiogroup = gsb_automem_radiobutton_new_with_title (vbox_pref,
							 _("Pressing RETURN in transaction form"),
							 _("selects next field"),
							 _("terminates transaction"),
							 &conf.entree, NULL, NULL);

    /* How to display financial year */
    radiogroup = gsb_automem_radiobutton_new_with_title (vbox_pref,
							 _("Automatic financial year is set"),
							 _("according to transaction date"),
							 _("according to transaction value date"),
							 &etat.affichage_exercice_automatique, 
							 NULL, NULL);

    /* automatic amount separatior fields */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					    _("Automatic amount separator"));

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 gsb_automem_checkbutton_new (_("Automagically add separator in amounts fields if unspecified"),
						      &etat.automatic_separator,
						      NULL, NULL),
			 FALSE, FALSE, 0 );

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}



/* ************************************************************************************************************** */
/* renvoie le widget contenu dans l'onglet divers du gsb_form_get_form_widget ()/liste des paramètres */
/* ************************************************************************************************************** */
GtkWidget *onglet_form_completion ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *hbox, *label, *entry;
    GtkWidget *button;
	gchar* tmpstr;

    vbox_pref = new_vbox_with_title_and_icon ( _("Form completion"), "form.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
                        gsb_automem_checkbutton_new (
                        _("Automatic filling transactions from payee"),
                        &conf.automatic_completion_payee,
                        G_CALLBACK ( gsb_transactions_list_display_update_auto_checkbutton ),
                        vbox_pref ),
                        FALSE, FALSE, 0 );

    button = gsb_automem_checkbutton_new (
                        _("Automatically recover the children of the associated transaction"),
                        &conf.automatic_recover_splits,
                        NULL,
                        NULL );
    g_object_set_data ( G_OBJECT ( vbox_pref ), "button_1", button );
    gtk_widget_set_sensitive ( button, conf.automatic_completion_payee );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), button, FALSE, FALSE, 0 );

    button = gsb_automem_checkbutton_new (
                        _("Limit the filling with payees belonging to the current account"),
                        &conf.limit_completion_to_current_account,
                        NULL, NULL);
    g_object_set_data ( G_OBJECT ( vbox_pref ), "button_2", button );
    gtk_widget_set_sensitive ( button, conf.automatic_completion_payee );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), button, FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX (vbox_pref),
                        gsb_automem_checkbutton_new (_("Mix credit/debit categories"),
                        &etat.combofix_mixed_sort,
                        G_CALLBACK ( gsb_transactions_list_display_update_combofix), NULL),
                        FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX (vbox_pref),
                        gsb_automem_checkbutton_new (_("Case sensitive completion"),
                        &etat.combofix_case_sensitive,
                        G_CALLBACK ( gsb_transactions_list_display_update_combofix), NULL),
                        FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX (vbox_pref),
                        gsb_automem_checkbutton_new (_("Don't allow new payee creation"),
                        &etat.combofix_force_payee,
                        G_CALLBACK ( gsb_transactions_list_display_update_combofix), NULL),
                        FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX (vbox_pref),
                        gsb_automem_checkbutton_new (_("Don't allow new category/budget creation"),
                        &etat.combofix_force_category,
                        G_CALLBACK ( gsb_transactions_list_display_update_combofix), NULL),
                        FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX (vbox_pref), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new (
                        _("Maximum items showed in drop down lists (0 for no limit): ") );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    entry = gtk_entry_new ();
    gtk_widget_set_size_request ( entry, 30, -1 );
    tmpstr = utils_str_itoa (etat.combofix_max_item);
    gtk_entry_set_text ( GTK_ENTRY (entry), tmpstr);
    g_free ( tmpstr );
    g_signal_connect ( G_OBJECT (entry),
                        "changed",
                        G_CALLBACK (gsb_transactions_list_display_change_max_items),
                        NULL );
    gtk_box_pack_start ( GTK_BOX (hbox), entry, FALSE, FALSE, 0 );
    
    if ( !gsb_data_account_get_accounts_amount () )
    {
        gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;

}



/**
 * called when we change a parameter of the combofix configuration
 * update the combofix in the form if they exists
 * as we don't know what was changed, update all the parameter (not a problem
 * because very fast)
 * at this level, the etat.___ variable has already been changed
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_display_update_combofix ( void )
{
    GtkWidget *combofix;

    combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
    if (combofix)
    {
	gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix),
				      etat.combofix_force_payee );
	gtk_combofix_set_max_items ( GTK_COMBOFIX (combofix),
				     etat.combofix_max_item );
	gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (combofix),
					  etat.combofix_case_sensitive );
    }

    combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_CATEGORY );
    if (combofix)
    {
	gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix),
				      etat.combofix_force_category );
	gtk_combofix_set_max_items ( GTK_COMBOFIX (combofix),
				     etat.combofix_max_item );
	gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (combofix),
					  etat.combofix_case_sensitive );
	gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (combofix),
				      etat.combofix_mixed_sort );
    }

    combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_BUDGET );
    if (combofix)
    {
	gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix),
				      etat.combofix_force_category );
	gtk_combofix_set_max_items ( GTK_COMBOFIX (combofix),
				     etat.combofix_max_item );
	gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (combofix),
					  etat.combofix_case_sensitive );
	gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (combofix),
				      etat.combofix_mixed_sort );
    }
    return FALSE;
}

/**
 * called when change in the max items field
 * change the variable and update the combofix
 *
 * \param entry
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_display_change_max_items ( GtkWidget *entry,
                        gpointer null )
{
    etat.combofix_max_item = utils_str_atoi ( gtk_entry_get_text (GTK_ENTRY (entry)));
    gsb_transactions_list_display_update_combofix ();

    return FALSE;
}


/**
 * Appellée lorsqu'on coche la case afficher le solde à aujourd'hui
 *
 *
 * */
void gsb_transactions_list_display_show_gives_balance ( void )
{
    gint account_number;

    account_number = gsb_gui_navigation_get_current_account ( );
    devel_debug_int ( account_number);
    if ( account_number != -1 )
        gsb_transactions_list_update_tree_view ( account_number, TRUE );
}


/**
 * Appellée lorsqu'on coche la case 
 * "Automatic filling transactions from payee"
 *
 * \param 
 * \param 
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_display_update_auto_checkbutton ( GtkWidget *checkbutton,
                        GtkWidget *container )
{
    GtkWidget *button;

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( checkbutton ) ) )
    {
        button = g_object_get_data ( G_OBJECT ( container ), "button_1" );
        gtk_widget_set_sensitive ( button, TRUE );
        button = g_object_get_data ( G_OBJECT ( container ), "button_2" );
        gtk_widget_set_sensitive ( button, TRUE );
    }
    else
    {
        button = g_object_get_data ( G_OBJECT ( container ), "button_1" );
        gtk_widget_set_sensitive ( button, FALSE );
        button = g_object_get_data ( G_OBJECT ( container ), "button_2" );
        gtk_widget_set_sensitive ( button, FALSE );
    }
 
    return FALSE;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
