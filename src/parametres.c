/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*            2008-2020 Pierre Biava (grisbi@pierre.biava.name)               */
/*          https://www.grisbi.org/                                           */
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
 * \file parametres.c
 * we find here the configuration dialog
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "parametres.h"
#include "accueil.h"
#include "bet_data.h"
#include "bet_finance_ui.h"
#include "categories_onglet.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_bank.h"
#include "gsb_data_account.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_locale.h"
#include "gsb_real.h"
#include "gsb_regex.h"
#include "gsb_rgba.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "import.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_localisation_decimal_point_changed (GtkComboBoxText *widget,
													gpointer user_data)
{
	GtkWidget *combo_box;
	GtkWidget *entry;
	gchar *str_capital;
	const gchar *text;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();
	text = gtk_combo_box_text_get_active_text (widget);
	combo_box = g_object_get_data (G_OBJECT (widget), "separator");

	if (g_strcmp0 (text, ",") == 0)
	{
		gsb_locale_set_mon_decimal_point (",");

		if (g_strcmp0 (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_box)), ",") == 0)
		{
			gsb_locale_set_mon_thousands_sep (" ");
			gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 0);
		}
	}
	else
	{
		gsb_locale_set_mon_decimal_point (".");
		if (g_strcmp0 (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_box)), ".") == 0)
		{
			gsb_locale_set_mon_thousands_sep (",");
			gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 2);
		}
	}

	/* on sort si on est dans l'assistant de sélection d'un nouveau fichier */
	if (GPOINTER_TO_INT (user_data) == GTK_ORIENTATION_HORIZONTAL)
		return;

	/* reset capital */
	entry = bet_finance_ui_get_capital_entry ();
	str_capital = utils_real_get_string_with_currency (gsb_real_double_to_real (w_etat->bet_capital),
													   w_etat->bet_currency,
													   FALSE);

	gtk_entry_set_text (GTK_ENTRY (entry), str_capital);
	g_free (str_capital);

	gsb_gui_navigation_update_localisation (1);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_localisation_format_date_toggle (GtkToggleButton *togglebutton,
													 GdkEventButton *event,
													 gpointer user_data)
{
	const gchar *format_date;
	GrisbiWinRun *w_run;

	w_run = grisbi_win_get_w_run ();

	format_date = g_object_get_data (G_OBJECT (togglebutton), "pointer");
	gsb_date_set_format_date (format_date);
	gsb_regex_init_variables ();

	if (grisbi_win_file_is_loading () && !w_run->new_account_file)
		gsb_gui_navigation_update_localisation (0);

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_localisation_thousands_sep_changed (GtkComboBoxText *widget,
													gpointer user_data)
{
	GtkWidget *combo_box;
	GtkWidget *entry;
	gchar *str_capital;
	const gchar *text;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();
	text = gtk_combo_box_text_get_active_text (widget);
	combo_box = g_object_get_data (G_OBJECT (widget), "separator");

	if (g_strcmp0 (text, "' '") == 0)
	{
		gsb_locale_set_mon_thousands_sep (" ");
	}
	else if (g_strcmp0 (text, ".") == 0)
	{

		gsb_locale_set_mon_thousands_sep (".");
		if (g_strcmp0 (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_box)), ".") == 0)
		{
			gsb_locale_set_mon_decimal_point (",");
			gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 1);
		}
	}
	else if (g_strcmp0 (text, ",") == 0)
	{

		gsb_locale_set_mon_thousands_sep (",");
		if (g_strcmp0 (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_box)), ",") == 0)
		{
			gsb_locale_set_mon_decimal_point (".");
			gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 0);
		}
	}
	else
		gsb_locale_set_mon_thousands_sep (NULL);

	/* on sort si on est dans l'assistant de sélection d'un nouveau fichier */
	if (GPOINTER_TO_INT (user_data) == GTK_ORIENTATION_HORIZONTAL)
		return;

	/* reset capital */
	entry = bet_finance_ui_get_capital_entry ();
	str_capital = utils_real_get_string_with_currency (gsb_real_double_to_real (w_etat->bet_capital),
													   w_etat->bet_currency,
													   FALSE);
	gtk_entry_set_text (GTK_ENTRY (entry), str_capital);
	g_free (str_capital);

	gsb_gui_navigation_update_localisation (1);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GtkWidget *gsb_config_date_format_chosen (GtkWidget *parent,
										  gint sens)
{
	GtkWidget *button_1;
	GtkWidget *button_2;
	GtkWidget *button_3;
	GtkWidget *button_4;
	GtkWidget *hbox;
	GtkWidget *paddingbox;
	gchar *format_date;

	button_1 = gtk_radio_button_new_with_label (NULL, "dd/mm/yyyy");
	format_date = g_strdup ("%d/%m/%Y");
	g_object_set_data_full (G_OBJECT (button_1), "pointer", format_date, g_free);

	button_2 = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON
																			(button_1)),
												"mm/dd/yyyy");
	format_date = g_strdup ("%m/%d/%Y");
	g_object_set_data_full (G_OBJECT (button_2), "pointer", format_date, g_free);

	button_3 = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON
																			(button_1)),
												"dd.mm.yyyy");
	format_date = g_strdup ("%d.%m.%Y");
	g_object_set_data_full (G_OBJECT (button_3), "pointer", format_date, g_free);

	button_4 = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON
																			(button_1)),
												"yyyy-mm-dd.");
	format_date = g_strdup ("%Y-%m-%d");
	g_object_set_data_full (G_OBJECT (button_4), "pointer", format_date, g_free);

	if (sens == GTK_ORIENTATION_VERTICAL)
	{
		paddingbox = new_paddingbox_with_title (parent, FALSE, _("Choose the date format"));
		gtk_box_pack_start (GTK_BOX (paddingbox), button_1, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (paddingbox), button_2, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (paddingbox), button_3, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (paddingbox), button_4, FALSE, FALSE, 0);
	}
	else
	{
		paddingbox = new_paddingbox_with_title (parent, FALSE, _("Choose the date format"));
		hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_PADDING_BOX);
		gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), button_1, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), button_2, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), button_3, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), button_4, FALSE, FALSE, 0);
	}

	format_date = gsb_date_get_format_date ();
	if (format_date)
	{
		if (strcmp (format_date, "%m/%d/%Y") == 0)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_2), TRUE);
		else if (strcmp (format_date, "%d.%m.%Y") == 0)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_3), TRUE);
		else if (strcmp (format_date, "%Y-%m-%d") == 0)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_4), TRUE);

		g_free (format_date);
	}

	g_signal_connect (G_OBJECT (button_1),
					  "button-release-event",
					  G_CALLBACK (gsb_localisation_format_date_toggle),
					  GINT_TO_POINTER (sens));
	g_signal_connect (G_OBJECT (button_2),
					  "button-release-event",
					  G_CALLBACK (gsb_localisation_format_date_toggle),
					  GINT_TO_POINTER (sens));
	g_signal_connect (G_OBJECT (button_3),
					  "button-release-event",
					  G_CALLBACK (gsb_localisation_format_date_toggle),
					  GINT_TO_POINTER (sens));
	g_signal_connect (G_OBJECT (button_4),
					  "button-release-event",
					  G_CALLBACK (gsb_localisation_format_date_toggle),
					  GINT_TO_POINTER (sens));
	return paddingbox;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GtkWidget *gsb_config_number_format_chosen (GtkWidget *parent,
											gint sens)
{
	GtkWidget *dec_hbox;
	GtkWidget *dec_sep;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *paddingbox;
	GtkWidget *thou_hbox;
	GtkWidget *thou_sep;
	GtkSizeGroup *size_group;
	gchar *mon_decimal_point;
	gchar *mon_thousands_sep;

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	dec_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	label = gtk_label_new (_("Decimal point: "));
	utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
	gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group), label);
	gtk_box_pack_start (GTK_BOX (dec_hbox), label, FALSE, FALSE, 0);

	dec_sep = gtk_combo_box_text_new_with_entry ();
	gtk_editable_set_editable (GTK_EDITABLE (gtk_bin_get_child (GTK_BIN (dec_sep))), FALSE);
	gtk_entry_set_width_chars (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (dec_sep))), 5);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dec_sep), ".");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dec_sep), ",");
	gtk_box_pack_start (GTK_BOX (dec_hbox), dec_sep, FALSE, FALSE, 0);

	thou_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	label = gtk_label_new (_("Thousands separator: "));
	utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
	gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group), label);
	gtk_box_pack_start (GTK_BOX (thou_hbox), label, FALSE, FALSE, 0);

	thou_sep = gtk_combo_box_text_new_with_entry ();
	gtk_editable_set_editable (GTK_EDITABLE (gtk_bin_get_child (GTK_BIN (thou_sep))), FALSE);
	gtk_entry_set_width_chars (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (thou_sep))), 5);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (thou_sep), "' '");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (thou_sep), ".");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (thou_sep), ",");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (thou_sep), "''");

	gtk_box_pack_start (GTK_BOX (thou_hbox), thou_sep, FALSE, FALSE, 0);

	if (sens == GTK_ORIENTATION_VERTICAL)
	{
		paddingbox = new_paddingbox_with_title (parent,
												FALSE,
												_("Choose the decimal and thousands separator"));
		gtk_box_pack_start (GTK_BOX (paddingbox), dec_hbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (paddingbox), thou_hbox, FALSE, FALSE, 0);
	}
	else
	{
		paddingbox = new_paddingbox_with_title (parent, FALSE, _("Decimal and thousands separator"));
		hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
		gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), dec_hbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), thou_hbox, FALSE, FALSE, 0);
	}

	mon_decimal_point = gsb_locale_get_mon_decimal_point ();
	if (strcmp (mon_decimal_point, ",") == 0)
		gtk_combo_box_set_active (GTK_COMBO_BOX (dec_sep), 1);
	else
		gtk_combo_box_set_active (GTK_COMBO_BOX (dec_sep), 0);
	g_free (mon_decimal_point);

	mon_thousands_sep = gsb_locale_get_mon_thousands_sep ();
	if (mon_thousands_sep == NULL)
		gtk_combo_box_set_active (GTK_COMBO_BOX (thou_sep), 3);
	else if (strcmp (mon_thousands_sep, ".") == 0)
		gtk_combo_box_set_active (GTK_COMBO_BOX (thou_sep), 1);
	else if (strcmp (mon_thousands_sep, ",") == 0)
		gtk_combo_box_set_active (GTK_COMBO_BOX (thou_sep), 2);
	else
		gtk_combo_box_set_active (GTK_COMBO_BOX (thou_sep), 0);

	if (mon_thousands_sep)
		g_free (mon_thousands_sep);

	g_object_set_data (G_OBJECT (dec_sep), "separator", thou_sep);
	g_object_set_data (G_OBJECT (thou_sep), "separator", dec_sep);

	g_signal_connect (G_OBJECT (dec_sep),
					  "changed",
					  G_CALLBACK (gsb_localisation_decimal_point_changed),
					  GINT_TO_POINTER (sens));
	g_signal_connect (G_OBJECT (thou_sep),
					  "changed",
					  G_CALLBACK (gsb_localisation_thousands_sep_changed),
					  GINT_TO_POINTER (sens));

	return paddingbox;
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
