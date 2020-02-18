/* ************************************************************************** */
/*  ce fichier contient les paramètres de l'affichage de la liste d'opé       */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2008 Benjamin Drieu (bdrieu@april.org)                            */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "affichage_liste.h"
#include "custom_list.h"
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_automem.h"
#include "gsb_combo_box.h"
#include "gsb_data_account.h"
#include "gsb_data_form.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "navigation.h"
#include "prefs/prefs_page_metatree.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean display_mode_button_changed ( GtkWidget *button,
                        gint *line_ptr );
static void gsb_transactions_list_display_show_gives_balance ( void );
static gboolean gsb_transactions_list_display_sort_changed ( GtkComboBox *widget,
                        gint *pointeur );
/*END_STATIC*/

/* line displayed when the list show 1 line */
gint display_one_line = 0;		/* fixes bug 1875 */

/* lines displayed when the list show 2 lines
 * this is a number 0-5 according the order of the combo-box in the configuration */
gint display_two_lines;

/* lines displayed when the list show 3 lines
 * this is a number 0-3 according the order of the combo-box in the configuration */
gint display_three_lines;

/*START_EXTERN*/
/*END_EXTERN*/

/**
 *
 *
 * \param
 *
 * \return
 * */
static gboolean fyear_combobox_sort_order_changed (GtkWidget *checkbutton,
												   gpointer data)
{
	GSettings *settings;

	settings = grisbi_settings_get_settings (SETTINGS_FORM);
	g_settings_set_int ( G_SETTINGS (settings),
                        "fyear-combobox-sort-order",
                        conf.fyear_combobox_sort_order);

	return FALSE;
}

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
    GtkWidget *button;
    const gchar *display_mode_lines_text [] = {
    _("In one line visible, show the line: "),
    _("In two lines visibles, show the lines: "),
    _("In three lines visibles, show the lines: "),
    };
    const gchar *line_1 [] = {"1", NULL};
    const gchar *line_2 [] = {"1-2", "1-3", "1-4", NULL};
    const gchar *line_3 [] = {"1-2-3", "1-2-4", "1-3-4", NULL};
    gchar *options_tri_primaire[] = {
		_("Sort by value date (if fail, try with the date)"),
		_("Sort by value date and then by date"),
		_("Forced sort by transaction date"),
		NULL
    };
    gchar *options_tri_secondaire[] = {
		_("Sort by transaction number"),
		_("Sort by type of amount (credit debit)"),
		_("Sort by payee name (if fail, by transaction number)"),
		_("Sort by date and then by transaction number"),
		NULL
    };
    gint i;
	gboolean is_loading;

    vbox_pref = new_vbox_with_title_and_icon ( _("Transaction list behavior"),
                        "gsb-transaction-list-32.png" );
	is_loading = grisbi_win_file_is_loading ();

    /* heading and boxes for layout */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _( "Display modes" ) );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    vbox_label = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_box_set_homogeneous ( GTK_BOX ( vbox_label ), TRUE );
    vbox_buttons = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_box_set_homogeneous ( GTK_BOX ( vbox_buttons ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    /* fill the table */
    for (i=0 ; i<3 ; i++)
    {
    gint j;
    const gchar **text_line = NULL;
    gint position = 0;

    /* set the line title */
    label = gtk_label_new ( display_mode_lines_text[i] );
    utils_labels_set_alignment ( GTK_LABEL (label), 0, 1);
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

    button = gtk_combo_box_text_new ();
    g_signal_connect ( G_OBJECT (button), "changed",
            G_CALLBACK (display_mode_button_changed),
            GINT_TO_POINTER (i));
    gtk_box_pack_start ( GTK_BOX (vbox_buttons), button, FALSE, FALSE, 0);

    j=0;
    while (text_line[j])
    {
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (button), text_line[j]);
        j++;
    }
    gtk_combo_box_set_active ( GTK_COMBO_BOX (button), position);
		if (i == 0 || !is_loading)
			gtk_widget_set_sensitive (button, FALSE);
    }

    /* pack vboxes in hbox */
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox_label, FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox_buttons, FALSE, FALSE, 0 );

    /* do we show the content of the selected transaction in the form for
     * each selection ? */
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_automem_checkbutton_gsettings_new (
                        _("Use simple click to select transactions"),
                        &conf.show_transaction_selected_in_form,
                        NULL, NULL ),
                        FALSE, FALSE, MARGIN_BOX );

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_automem_checkbutton_gsettings_new (
                        _("Highlights the transaction that gives the balance today"),
                        &conf.show_transaction_gives_balance,
                        G_CALLBACK ( gsb_transactions_list_display_show_gives_balance ), NULL ),
                        FALSE, FALSE, 0 );

    /* Primary sorting option for the transactions */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Primary sorting option") );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

	button = gsb_combo_box_gsettings_new_from_list (options_tri_primaire,
											   NULL,
											   TRUE,
											   conf.transactions_list_primary_sorting,
											   G_CALLBACK (gsb_transactions_list_display_sort_changed),
											   GINT_TO_POINTER (PRIMARY_SORT));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    /* Secondary sorting option for the transactions */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Secondary sorting option") );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

	button = gsb_combo_box_gsettings_new_from_list (options_tri_secondaire,
											   NULL,
											   TRUE,
											   conf.transactions_list_secondary_sorting,
											   G_CALLBACK (gsb_transactions_list_display_sort_changed),
											   GINT_TO_POINTER (SECONDARY_SORT));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    /* Account distinction */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
                        _("Account differentiation"));

    //~ gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        //~ gsb_automem_checkbutton_new (
                        //~ _("Remember display settings for each account separately"),
                        //~ &etat.retient_affichage_par_compte,
                        //~ NULL, NULL ),
                        //~ FALSE, FALSE, 0 );

	if (!is_loading)
			gtk_widget_set_sensitive (paddingbox, FALSE);

	return ( vbox_pref );
}


/**
 *
 *
 * */
static gboolean gsb_transactions_list_display_sort_changed ( GtkComboBox *widget,
                        gint *pointeur )
{
    GSettings *settings;
    gint page_number;
    gint account_nb;
    gint value = 0;
    gint sort_type = 0;

    settings = grisbi_settings_get_settings ( SETTINGS_GENERAL );

    page_number = gsb_gui_navigation_get_current_page ( );
    value = gtk_combo_box_get_active ( widget );
    sort_type = GPOINTER_TO_INT ( pointeur );

    switch ( sort_type )
    {
        case PRIMARY_SORT:
            conf.transactions_list_primary_sorting = value;
            switch ( value )
            {
                case 0:
                    g_settings_reset (G_SETTINGS (settings), "transactions-list-primary-sorting");
                    break;
                case 1:
                    g_settings_set_string (G_SETTINGS (settings),
										   "transactions-list-primary-sorting",
                                           "Sort by value date and then by date" );
                    break;
                case 2:
                    g_settings_set_string ( G_SETTINGS ( settings ),
										   "transactions-list-primary-sorting",
                                           "Forced sort by date");
			}
            break;
        case SECONDARY_SORT:
            conf.transactions_list_secondary_sorting = value;
            switch ( value )
            {
                case 0:
                    g_settings_reset ( G_SETTINGS ( settings ), "transactions-list-secondary-sorting" );
                    break;
                case 1:
                    g_settings_set_string ( G_SETTINGS ( settings ), "transactions-list-secondary-sorting",
                                           "Sort by type of amount" );
                    break;
                case 2:
                    g_settings_set_string ( G_SETTINGS ( settings ), "transactions-list-secondary-sorting",
                                           "Sort by payee name" );
                    break;
                case 3:
                    g_settings_set_string ( G_SETTINGS ( settings ), "transactions-list-secondary-sorting",
                                           "Sort by date and then by transaction number" );
            }
            break;
    }
    gsb_file_set_modified ( TRUE );

    switch ( page_number )
    {
        case GSB_ACCOUNT_PAGE:
            account_nb = gsb_gui_navigation_get_current_account ();
            if ( account_nb != -1 )
                gsb_transactions_list_update_tree_view ( account_nb, TRUE );
            break;
    }

    return FALSE;
}


/**
 * called when we change a button for the display mode
 *
 * \param button 	the combo box which changed
 * \param line_ptr	a gint* which is the line of the button (ie 1 line mode, 2 lines or 3 lines)
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

    gsb_file_set_modified ( TRUE );

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

/* ************************************************************************************************************** */
/* renvoie le widget contenu dans l'onglet divers du gsb_form_get_form_widget ()/liste des paramètres */
/* ************************************************************************************************************** */
GtkWidget *onglet_diverse_form_and_lists ( void )
{
    GtkWidget *vbox_pref;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *paddingbox;
	gboolean is_loading;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	is_loading = grisbi_win_file_is_loading ();

    vbox_pref = new_vbox_with_title_and_icon ( _("Form behavior"), "gsb-form-32.png" );

    /* What to do if RETURN is pressed into transaction form */
	paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Pressing RETURN in transaction form"));
	button = gsb_automem_radiobutton_gsettings_new (_("selects next field"),
											   _("terminates transaction"),
											   &conf.form_enter_key,
											   NULL,
											   NULL);
	gtk_box_pack_start (GTK_BOX (paddingbox), button, FALSE, FALSE, 0);

	/* Interpretation of future dates */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Interpretation of future dates"));

	button = gsb_automem_checkbutton_new (_("Replace the year of future dates with the previous year"),
										  &w_etat->form_date_force_prev_year,
										  NULL,
										  NULL);
	gtk_box_pack_start (GTK_BOX (paddingbox), button, FALSE, FALSE, 0);

	label = gtk_label_new (_("This option allows all future dates to be considered as dates "
							 "from the previous calendar year"));
	gtk_label_set_xalign (GTK_LABEL (label), GSB_LEFT);
	gtk_box_pack_start (GTK_BOX (paddingbox), label, FALSE, FALSE, 0);

	if (!is_loading)
		gtk_widget_set_sensitive (paddingbox, FALSE);

	/* How to display financial year */
	paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Automatic financial year is set"));
    button = gsb_automem_radiobutton_gsettings_new (_("according to transaction date"),
											   _("according to transaction value date"),
											   &conf.affichage_exercice_automatique,
											   NULL,
											   NULL);
	gtk_box_pack_start (GTK_BOX (paddingbox), button, FALSE, FALSE, 0);

    /* How to sort the financial year */
	paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Sorting the exercises in the button of the form"));
	button = gsb_automem_checkbutton_gsettings_new (_("Sorting descending of the exercises"),
											   &conf.fyear_combobox_sort_order,
											   G_CALLBACK (fyear_combobox_sort_order_changed),
											   "fyear-combobox-sort-order");
	gtk_box_pack_start (GTK_BOX (paddingbox), button, FALSE, FALSE, 0);

    /* automatic amount separatior fields */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					    _("Automatic amount separator"));

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 gsb_automem_checkbutton_new (_("Automagically add separator in amounts fields if unspecified"),
						      &etat.automatic_separator,
						      NULL, NULL),
			 FALSE, FALSE, 0 );

	if (!is_loading)
		gtk_widget_set_sensitive (paddingbox, FALSE);

    return vbox_pref;
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
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
