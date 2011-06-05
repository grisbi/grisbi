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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "affichage_liste.h"
#include "custom_list.h"
#include "fenetre_principale.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_form.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean display_mode_button_changed ( GtkWidget *button,
                        gint *line_ptr );
static gboolean gsb_transaction_list_config_button_set_active_from_string ( GtkWidget *tree_view,
                        gchar *string,
                        gboolean active );
static void gsb_transaction_list_config_button_set_normal_color ( GtkWidget *toggle_button,
                        gboolean normal );
static GtkWidget *gsb_transaction_list_config_create_buttons_table ( GtkWidget *tree_view );
static GtkWidget *gsb_transaction_list_config_create_tree_view ( GtkListStore *store );
static gboolean gsb_transaction_list_config_drag_begin ( GtkWidget *tree_view,
                        GdkDragContext *drag_context,
                        gpointer null );
static gboolean gsb_transaction_list_config_drag_end ( GtkWidget *tree_view,
                        GdkDragContext *drag_context,
                        gpointer null );
static gboolean gsb_transaction_list_config_fill_store ( GtkWidget *tree_view );
static gboolean gsb_transaction_list_config_realized ( GtkWidget *tree_view,
                        gpointer null );
static void gsb_transaction_list_config_toggle_element_button ( GtkWidget *toggle_button,
                        GtkWidget *tree_view );
static gboolean gsb_transaction_list_config_update_list_config ( GtkWidget *tree_view );
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

/* tableau qui contient les numéros des éléments affichés (voir gsb_transactions_list.h) */
gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];

/* line displayed when the list show 1 line */
gint display_one_line;

/* lines displayed when the list show 2 lines
 * this is a number 0-5 according the order of the combo-box in the configuration */
gint display_two_lines;

/* lines displayed when the list show 3 lines
 * this is a number 0-3 according the order of the combo-box in the configuration */
gint display_three_lines;

/* buttons to config the list */
static GtkWidget *list_config_buttons[18];

/** tmp for drag'n drop */
static gint start_drag_column;
static gint start_drag_row;


/*START_EXTERN*/
extern GdkColor couleur_grise;
extern GdkColor couleur_selection;
extern gchar *tips_col_liste_operations[CUSTOM_MODEL_VISIBLE_COLUMNS];
extern gchar *titres_colonnes_liste_operations[CUSTOM_MODEL_VISIBLE_COLUMNS];
extern gint transaction_col_width[CUSTOM_MODEL_VISIBLE_COLUMNS];
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
	    row[j] = gsb_transaction_list_get_titre_colonne_liste_ope ( tab_affichage_ope[i][j] - 1 );

	    /* on the first row, set for titles and tips, for others row, only for tips */
	    if ( i )
	    {
            if ( row[j] )
            {
                if ( tips_col_liste_operations[j] )
                    tips_col_liste_operations[j] = g_strconcat ( tips_col_liste_operations[j],
                                                    "- ",
                                                    row[j], " ",
                                                    NULL );
                else
                    tips_col_liste_operations[j] = g_strconcat ( " ", row[j], " ", NULL );

                if ( !titres_colonnes_liste_operations[j] )
                    titres_colonnes_liste_operations[j] = row[j];
                else
                    g_free ( row[j] );
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


/**
 * Affichage de l'onglet de sélection des données de la liste des colonnes
 *
 *
 * */
GtkWidget *onglet_affichage_liste ( void )
{
    GtkWidget *onglet;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *table;
    GtkWidget *paddingbox;
    GtkListStore* list_store;

	/* à la base, on met une vbox */
	onglet = new_vbox_with_title_and_icon ( _("Transactions list cells"), "transaction-list.png" );

    /* partie 1 visualisation de l'arrangement des données */
	paddingbox = new_paddingbox_with_title ( onglet, FALSE, _("Transactions list preview") );

    /*create the scolled window for tree_view */
    sw = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
                        GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_AUTOMATIC );
    gtk_widget_set_size_request ( sw, -1, 160 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), sw, TRUE, TRUE, 0 );

    /* create the list_store */
    list_store = gtk_list_store_new ( 2 * CUSTOM_MODEL_VISIBLE_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING,
                        G_TYPE_INT );

    /* create the tree_view */
    tree_view = gsb_transaction_list_config_create_tree_view ( list_store );
    g_object_unref ( G_OBJECT ( list_store ) );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw ), tree_view );

    /* partie 2 Source des données */
	paddingbox = new_paddingbox_with_title ( onglet, FALSE, _("Transactions list contents") );

	/* on crée maintenant une table de 3x6 boutons */
	table = gsb_transaction_list_config_create_buttons_table ( tree_view );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0 );

    gtk_widget_show_all ( onglet );

    return onglet;
}


/**
 * create the tree_view for the transaction list configuration
 * set the model given in param
 * set the columns and all the connections
 *
 * \param model the model to set in the tree_view
 *
 * \return the tree_view
 * */
GtkWidget *gsb_transaction_list_config_create_tree_view ( GtkListStore *store )
{
    GtkWidget *tree_view;
    gint column;
    GtkTargetEntry target_entry[] = { { "text", GTK_TARGET_SAME_WIDGET, 0 } };

    if ( !store )
        return NULL;

    /* create the tree_view */
    tree_view = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ) );

    gtk_tree_view_set_grid_lines ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_VIEW_GRID_LINES_BOTH );
    
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) ),
                        GTK_SELECTION_NONE );

    /* set the columns */
    for ( column=0 ; column < CUSTOM_MODEL_VISIBLE_COLUMNS ; column++ )
    {
        GtkTreeViewColumn *tree_view_column;

        tree_view_column = gtk_tree_view_column_new_with_attributes ( titres_colonnes_liste_operations[column],
                                            gtk_cell_renderer_text_new (),
                                            "text", 2*column,
                                            NULL );
        gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( tree_view_column ) );
        gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( tree_view_column ),
                        GTK_TREE_VIEW_COLUMN_FIXED );
        gtk_tree_view_column_set_resizable ( GTK_TREE_VIEW_COLUMN ( tree_view_column ), TRUE );
    }

    /* we will fill the size of the columns when the window will be realized */
    g_signal_connect ( G_OBJECT ( tree_view ),
                        "realize",
                        G_CALLBACK ( gsb_transaction_list_config_realized ),
                        NULL );

    /* enable the drag'n drop, we need to use low-level api because
     * gtk_tree_view api can only move the entire row, not only a cell
     * (at least, didn't find how...) */
    gtk_drag_source_set ( tree_view,
                        GDK_BUTTON1_MASK,
                        target_entry, 1,
                        GDK_ACTION_MOVE );
    g_signal_connect ( G_OBJECT (tree_view),
                        "drag-begin",
                        G_CALLBACK ( gsb_transaction_list_config_drag_begin ),
                        NULL );

    gtk_drag_dest_set ( tree_view,
                        GTK_DEST_DEFAULT_ALL,
                        target_entry, 1,
                        GDK_ACTION_MOVE );
    g_signal_connect ( G_OBJECT (tree_view),
                        "drag-end",
                        G_CALLBACK ( gsb_transaction_list_config_drag_end ),
                        NULL );

    return tree_view;
}


/**
 * called once the window is realized,
 * fill the form and size the columns according to the configuration
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_transaction_list_config_realized ( GtkWidget *tree_view,
                        gpointer null )
{
    gint column;

    if ( !assert_account_loaded ( ) )
      return FALSE;

    /* fill and update the transaction list and buttons */
    gsb_transaction_list_config_update_list_config ( tree_view );

    for ( column = 0 ; column < CUSTOM_MODEL_VISIBLE_COLUMNS ; column++ )
    {
        gint width;

        width = ( transaction_col_width [column] * ( tree_view -> allocation.width ) ) / 100;
        gtk_tree_view_column_set_fixed_width (
                        gtk_tree_view_get_column ( GTK_TREE_VIEW ( tree_view ), column ),
                        width );
    }

    gdk_window_set_cursor ( tree_view -> window, gdk_cursor_new ( GDK_HAND2 ) );

    return FALSE;
}


/**
 * called when we begin a drag,
 * find what cell was under the cursor and change it
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_transaction_list_config_drag_begin ( GtkWidget *tree_view,
                        GdkDragContext *drag_context,
                        gpointer null )
{
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    GdkWindow *drawable;
    GdkRectangle rectangle;
    GdkPixbuf *pixbuf_cursor;

    /* get the cell coord */
    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
                        &x,
                        &y,
                        FALSE );
    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
				    x,
				    y,
				    &path,
				    &tree_column,
				    NULL, NULL );

    if ( !path
	 ||
	 !tree_column )
        return FALSE;

    start_drag_column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view ) ), tree_column );
    start_drag_row = utils_str_atoi ( gtk_tree_path_to_string ( path ) );

    /* draw the new cursor */
    drawable = gtk_tree_view_get_bin_window (GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_view_get_cell_area ( GTK_TREE_VIEW ( tree_view ),
                        path,
                        tree_column,
                        &rectangle );

    pixbuf_cursor = gdk_pixbuf_get_from_drawable ( NULL,
                        GDK_DRAWABLE ( drawable ),
                        gdk_colormap_get_system (),
                        rectangle.x,
                        rectangle.y,
                        0,
                        0,
                        rectangle.width,
                        rectangle.height );
    gtk_drag_source_set_icon_pixbuf ( tree_view, pixbuf_cursor );

    g_object_unref ( pixbuf_cursor );

    if ( start_drag_row == 0 )
        gtk_tree_view_column_set_title  ( tree_column, "" );

    return FALSE;
}
					

/**
 * called when we end a drag,
 * find what cell was under the cursor and do the split between the 2 cells
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_transaction_list_config_drag_end ( GtkWidget *tree_view,
                        GdkDragContext *drag_context,
                        gpointer null )
{
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gchar *string;
    gint x, y;
    gint end_drag_row;
    gint end_drag_column;
    gint element;
    gint old_element;
    gint current_account;

    /* get the cell position */
    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view ) ),
                        &x,
                        &y,
                        FALSE );
    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
                        x,
                        y,
                        &path,
                        &tree_column,
                        NULL,
                        NULL );

    if ( !path || !tree_column )
        return FALSE;

    end_drag_column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view ) ), tree_column );
    end_drag_row = utils_str_atoi ( gtk_tree_path_to_string ( path ) );

    /* if we are on the same cell, go away */
    if ( start_drag_row == end_drag_row
	 &&
	 start_drag_column == end_drag_column )
        return ( FALSE );

    element = tab_affichage_ope[start_drag_row][start_drag_column];
    current_account = gsb_gui_navigation_get_current_account ( );

    /* save the old position et désensitive le bouton correspondant */
    old_element = tab_affichage_ope[end_drag_row][end_drag_column];
    if ( old_element )
    {
        string = gsb_transaction_list_get_titre_colonne_liste_ope ( old_element - 1 );
        gsb_transaction_list_config_button_set_active_from_string ( tree_view, string, FALSE );

        g_free ( string );
    }

    /* positionne le nouvel élément */
    tab_affichage_ope[end_drag_row][end_drag_column] = element;

    /* the element was already showed, we need to erase the last cell first */
    tab_affichage_ope[start_drag_row][start_drag_column] = 0;
    transaction_list_update_cell ( start_drag_column, start_drag_row );

    /* modifie le titre de la colonne si nécessaire */
    if ( end_drag_row == 0 )
    {
        string = gsb_transaction_list_get_titre_colonne_liste_ope ( element - 1 );
        gtk_tree_view_column_set_title  ( tree_column, string );

        g_free ( string );
    }

    /* fill the list */
    gsb_transaction_list_config_update_list_config ( tree_view );

    /* met à jour la liste des opérations */
    transaction_list_update_element ( element );
    recuperation_noms_colonnes_et_tips ( );
    update_titres_tree_view ( );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return (FALSE);
}


/**
 * fill the list and set the correct buttons as active/passive
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_transaction_list_config_update_list_config ( GtkWidget *tree_view )
{
    GtkTreeModel *store;
    gint i;

    /* fill the store */
    gsb_transaction_list_config_fill_store ( tree_view );
    store = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    /* active/unactive the buttons */
    for ( i = 0 ; i < 18 ; i++ )
	{
        GtkTreeIter iter;
        gint current_number;

        current_number = GPOINTER_TO_INT ( g_object_get_data (
                            G_OBJECT ( list_config_buttons[i] ), "element_number" ) );

        if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( store ), &iter ) )
        {
            do
            {
                gint num_1 = 0;
                gint num_2 = 0;
                gint num_3 = 0;
                gint num_4 = 0;
                gint num_5 = 0;
                gint num_6 = 0;
                gint num_7 = 0;

                gtk_tree_model_get ( GTK_TREE_MODEL ( store ), &iter,
                                        1, &num_1,
                                        3, &num_2,
                                        5, &num_3,
                                        7, &num_4,
                                        9, &num_5,
                                        11, &num_6,
                                        13, &num_7,
                                        -1 );

                if ( current_number == num_1 ||
                 current_number == num_2 ||
                 current_number == num_3 ||
                 current_number == num_4 ||
                 current_number == num_5 ||
                 current_number == num_6 )
                {
                    g_signal_handlers_block_by_func ( G_OBJECT ( list_config_buttons[i] ),
                                        G_CALLBACK ( gsb_transaction_list_config_toggle_element_button ),
                                        tree_view );
                    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( list_config_buttons[i] ), TRUE );
                    gsb_transaction_list_config_button_set_normal_color ( list_config_buttons[i], FALSE );

                    g_signal_handlers_unblock_by_func ( G_OBJECT ( list_config_buttons[i] ),
                                        G_CALLBACK ( gsb_transaction_list_config_toggle_element_button ),
                                        tree_view );
                }
                else
                    gsb_transaction_list_config_button_set_normal_color ( list_config_buttons[i], TRUE );
            }
            while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( store ), &iter ) );
        }
    }

    return FALSE;
}


/**
 * fill the configuration store according to the organization for transaction list
 *
 *
 * \return FALSE
 * */
gboolean gsb_transaction_list_config_fill_store ( GtkWidget *tree_view )
{
    GtkListStore *store;
    gchar *row[CUSTOM_MODEL_VISIBLE_COLUMNS];
    gint i;
    gint j;

    store = GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) ) );
						
    gtk_list_store_clear ( store );

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
        GtkTreeIter iter;

        gtk_list_store_append ( GTK_LIST_STORE ( store ), &iter );

        for ( j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
        {
            row[j] = gsb_transaction_list_get_titre_colonne_liste_ope ( tab_affichage_ope[i][j] - 1 );
            /* on met le nom dans les lignes paires et le numéro de l'élément dans las lignes impaires */
            gtk_list_store_set ( GTK_LIST_STORE ( store ), &iter,
                        2*j, row[j],
                        2*j+1, tab_affichage_ope[i][j],
                        -1 );

            if ( row[j] )
                g_free ( row[j] );
        }
    }
    return FALSE;
}


/**
 * create the table of buttons and set the signals
 *
 * \param
 *
 * \return the new table of buttons
 * */
GtkWidget *gsb_transaction_list_config_create_buttons_table ( GtkWidget *tree_view )
{
    GtkWidget *table;
    gint current_number = 0;
    gint row, column;

    /* the table is 3x6 buttons */
    table = gtk_table_new ( 3, 6, FALSE );

    for ( row=0 ; row < 3 ; row++ )
	for ( column = 0 ; column < 6 ; column++ )
	{
	    gchar *string;
	    gchar *changed_string;

	    string = gsb_transaction_list_get_titre_colonne_liste_ope ( current_number );

	    if ( string )
	    {
            /* the max string in the button is 10 characters */
            changed_string = limit_string ( string, 10 );

            list_config_buttons[current_number] = gtk_toggle_button_new_with_label ( changed_string );
            g_object_set_data ( G_OBJECT ( list_config_buttons[current_number] ),
                        "element_number",
                        GINT_TO_POINTER ( current_number + 1 ) );
            g_signal_connect ( G_OBJECT ( list_config_buttons[current_number] ),
                        "toggled",
                        G_CALLBACK ( gsb_transaction_list_config_toggle_element_button ),
                        tree_view );
            gtk_table_attach_defaults ( GTK_TABLE ( table ),
                        list_config_buttons[current_number],
                        column, column+1,
                        row, row+1 );

            /* set the tooltip with the real name */
            gtk_widget_set_tooltip_text ( GTK_WIDGET ( list_config_buttons[current_number] ), string );

            g_free ( string );
            g_free ( changed_string );
	    }

	    current_number++;
	}

    return table;
}


/**
 * called when toggle a button of the form configuration, append or remove
 * the value from the tree view
 *
 * \param toggle_button the button we click
 *
 * \return FALSE
 * */
void gsb_transaction_list_config_toggle_element_button ( GtkWidget *toggle_button,
                        GtkWidget *tree_view )
{
    gint element;

    /* get the element number */
    element = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( toggle_button ), "element_number" ) );

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( toggle_button ) ) )
    {
        gint row, column;
        gboolean place_trouvee = FALSE;

        /* on change la couleur du bouton */
        gsb_transaction_list_config_button_set_normal_color ( toggle_button, FALSE );
                        
        /* button is on, append the element */
        for ( row = 3 ; row >= 0 ; row-- )
        {
            for ( column = 6 ; column >= 0 ; column-- )
            {
                gint element;

                element = tab_affichage_ope[row][column];
                if ( element == 0 )
                {
                    place_trouvee = TRUE;
                    break;
                }
            }
            if ( place_trouvee )
                break;
        }

        if ( place_trouvee )
        {
            /* on sauvegarde la position du nouvel élément */
            tab_affichage_ope[row][column] = element;

            /* met à jour la liste des opérations */
            transaction_list_update_element ( element );
        }
    }
    else
    {
        GtkTreeModel *store;
        GtkTreeIter iter;

        /* on change la couleur du bouton */
        gsb_transaction_list_config_button_set_normal_color ( toggle_button, TRUE );

        /* on supprime la donnée dans la liste */
        store = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
        if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( store ), &iter ) )
        {
            gint i = 0, j;

            do
            {
                for ( j = 0; j < CUSTOM_MODEL_VISIBLE_COLUMNS; j++ )
                {
                    gint num = 0;

                    gtk_tree_model_get ( GTK_TREE_MODEL ( store ), &iter, 2*j+1, &num, -1 );
                    if ( element == num )
                    {
                        tab_affichage_ope[i][j] = 0;
                        /* met à jour la liste des opérations */
                        transaction_list_update_cell (j, i);
                    }
                }
                i++;
            }
            while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( store ), &iter ) );
        }
    }

    /* fill the list */
    gsb_transaction_list_config_update_list_config ( tree_view );

    /* on finit de mettre à jour la liste des opérations */
    recuperation_noms_colonnes_et_tips ( );
    update_titres_tree_view ( );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/**
 * set the correct buttons as active/passive
 *
 * \param search string
 *
 * \return FALSE
 * */
gboolean gsb_transaction_list_config_button_set_active_from_string ( GtkWidget *tree_view,
                        gchar *string,
                        gboolean active )
{
    gint i;

    for ( i = 0 ; i < 18 ; i++ )
	{
        gchar *tmp_str;

        tmp_str = gtk_widget_get_tooltip_text ( list_config_buttons[i] );

        if ( string && g_utf8_collate ( string, tmp_str ) == 0 )
        {
            g_signal_handlers_block_by_func ( G_OBJECT ( list_config_buttons[i] ),
                                G_CALLBACK ( gsb_transaction_list_config_toggle_element_button ),
                                tree_view );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( list_config_buttons[i] ), active );
            gsb_transaction_list_config_button_set_normal_color ( list_config_buttons[i], !active );

            g_signal_handlers_unblock_by_func ( G_OBJECT ( list_config_buttons[i] ),
                                G_CALLBACK ( gsb_transaction_list_config_toggle_element_button ),
                                tree_view );

            g_free ( tmp_str );

            return TRUE;
        }
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void gsb_transaction_list_config_button_set_normal_color ( GtkWidget *toggle_button,
                        gboolean normal )
{
    if ( normal )
    {
        gtk_widget_modify_bg ( toggle_button, GTK_STATE_NORMAL, &couleur_grise );
        gtk_widget_modify_bg ( toggle_button, GTK_STATE_PRELIGHT, &couleur_grise );
    }
    else
    {
        gtk_widget_modify_bg ( toggle_button, GTK_STATE_NORMAL, &couleur_selection );
        gtk_widget_modify_bg ( toggle_button, GTK_STATE_ACTIVE, &couleur_selection );
        gtk_widget_modify_bg ( toggle_button, GTK_STATE_PRELIGHT, &couleur_selection );
    }
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
