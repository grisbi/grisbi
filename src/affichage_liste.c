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
/*END_STATIC*/

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
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
