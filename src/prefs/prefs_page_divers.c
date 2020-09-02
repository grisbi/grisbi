/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_divers.h"
#include "bet_finance_ui.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_account.h"
#include "gsb_automem.h"
#include "gsb_dirs.h"
#include "gsb_locale.h"
#include "gsb_regex.h"
#include "gsb_rgba.h"
#include "navigation.h"
#include "structures.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDiversPrivate   PrefsPageDiversPrivate;

struct _PrefsPageDiversPrivate
{
	GtkWidget *			vbox_divers;

	/* programs */
    GtkWidget *			grid_divers_programs;

	/* notebbok low resolution */
	GtkWidget *			notebook_others_options;

	/* pages pour notebook */
	GtkWidget *			divers_localization_paddingbox;
	GtkWidget *			divers_scheduler_paddingbox;
	GtkWidget *			title_divers_localization;
	GtkWidget *			title_divers_scheduler;

	/* scheduler */
	GtkWidget *         vbox_divers_scheduler;
	GtkWidget *         hbox_divers_scheduler_set_default_account;
	GtkWidget *			checkbutton_scheduler_set_default_account;
	GtkWidget *			checkbutton_scheduler_set_fixed_date;
	GtkWidget *			checkbutton_scheduler_set_fixed_day;
	GtkWidget *			hbox_divers_scheduler_nb_days_before_scheduled;
	GtkWidget *         hbox_divers_scheduler_set_fixed_date;
	GtkWidget *         hbox_divers_scheduler_set_fixed_day;
    GtkWidget *         spinbutton_nb_days_before_scheduled;
	GtkWidget *         spinbutton_scheduler_fixed_day;

	/* localization */
	GtkWidget *			box_choose_date_format;
	GtkWidget *			box_choose_numbers_format;
	GtkWidget *			combo_choose_language;
	GtkWidget *			combo_choose_decimal_point;
	GtkWidget *			combo_choose_thousands_separator;
	GtkWidget *			radiobutton_choose_date_1;
	GtkWidget *			radiobutton_choose_date_2;
	GtkWidget *			radiobutton_choose_date_3;
	GtkWidget *			radiobutton_choose_date_4;

};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDivers, prefs_page_divers, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
static void prefs_page_divers_choose_thousands_sep_changed (GtkComboBoxText *widget,
															gpointer null)
{
    GtkWidget *combo_box;
    GtkWidget *entry;
    gchar *str_capital;
    const gchar *text;

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

    /* reset capital */
    entry = bet_finance_ui_get_capital_entry ();
    str_capital = utils_real_get_string_with_currency (gsb_real_double_to_real (
                    etat.bet_capital),
                    etat.bet_currency,
                    FALSE);

    gtk_entry_set_text (GTK_ENTRY (entry), str_capital);
    g_free (str_capital);

    gsb_gui_navigation_update_localisation (1);
}

static void prefs_page_divers_choose_decimal_point_changed (GtkComboBoxText *widget,
															gpointer null)
{
    GtkWidget *combo_box;
    GtkWidget *entry;
    gchar *str_capital;
    const gchar *text;

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

    /* reset capital */
    entry = bet_finance_ui_get_capital_entry ();
    str_capital = utils_real_get_string_with_currency (gsb_real_double_to_real (
                    etat.bet_capital),
                    etat.bet_currency,
                    FALSE);

    gtk_entry_set_text (GTK_ENTRY (entry), str_capital);
    g_free (str_capital);

    gsb_gui_navigation_update_localisation (1);
}

static void prefs_page_divers_choose__number_format_init (PrefsPageDivers *page)
{
    gchar *mon_decimal_point;
    gchar *mon_thousands_sep;
	PrefsPageDiversPrivate *priv;

	priv = prefs_page_divers_get_instance_private (page);

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_choose_thousands_separator), "' '");
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_choose_thousands_separator), ".");
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_choose_thousands_separator), ",");
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_choose_thousands_separator), "''");

    mon_decimal_point = gsb_locale_get_mon_decimal_point ();
    if (strcmp (mon_decimal_point, ",") == 0)
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_decimal_point), 1);
    else
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_decimal_point), 0);
    g_free (mon_decimal_point);

    mon_thousands_sep = gsb_locale_get_mon_thousands_sep ();
    if (mon_thousands_sep == NULL)
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_thousands_separator), 3);
    else if (strcmp (mon_thousands_sep, ".") == 0)
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_thousands_separator), 1);
    else if (strcmp (mon_thousands_sep, ",") == 0)
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_thousands_separator), 2);
    else
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_thousands_separator), 0);

    if (mon_thousands_sep)
        g_free (mon_thousands_sep);

    g_object_set_data (G_OBJECT (priv->combo_choose_decimal_point), "separator", priv->combo_choose_thousands_separator);
    g_object_set_data (G_OBJECT (priv->combo_choose_thousands_separator), "separator", priv->combo_choose_decimal_point);

	g_signal_connect (G_OBJECT (priv->combo_choose_decimal_point),
					  "changed",
					  G_CALLBACK (prefs_page_divers_choose_decimal_point_changed),
					  NULL);
    g_signal_connect (G_OBJECT (priv->combo_choose_thousands_separator),
                        "changed",
                        G_CALLBACK (prefs_page_divers_choose_thousands_sep_changed),
                        NULL);
}

static gboolean prefs_page_divers_choose_date_format_toggle (GtkToggleButton *togglebutton,
															 GdkEventButton *event,
															 gpointer null)
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
 *
 * \return
 **/
static void  prefs_page_divers_choose_date_format_init (PrefsPageDivers *page)
{
    gchar *format_date;
	PrefsPageDiversPrivate *priv;

	priv = prefs_page_divers_get_instance_private (page);

    format_date = g_strdup ("%d/%m/%Y");
    g_object_set_data_full (G_OBJECT (priv->radiobutton_choose_date_1), "pointer", format_date, g_free);

    format_date = g_strdup ("%m/%d/%Y");
    g_object_set_data_full (G_OBJECT (priv->radiobutton_choose_date_2), "pointer", format_date, g_free);

    format_date = g_strdup ("%d.%m.%Y");
    g_object_set_data_full (G_OBJECT (priv->radiobutton_choose_date_3), "pointer", format_date, g_free);

    format_date = g_strdup ("%Y-%m-%d");
    g_object_set_data_full (G_OBJECT (priv->radiobutton_choose_date_4), "pointer", format_date, g_free);

    format_date = gsb_date_get_format_date ();
    if (format_date)
    {
        if (strcmp (format_date, "%m/%d/%Y") == 0)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_choose_date_2), TRUE);
        else if (strcmp (format_date, "%d.%m.%Y") == 0)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_choose_date_3), TRUE);
        else if (strcmp (format_date, "%Y-%m-%d") == 0)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_choose_date_4), TRUE);

        g_free (format_date);
    }

    g_signal_connect (G_OBJECT (priv->radiobutton_choose_date_1),
                        "button-release-event",
                        G_CALLBACK (prefs_page_divers_choose_date_format_toggle),
                        NULL);
    g_signal_connect (G_OBJECT (priv->radiobutton_choose_date_2),
                        "button-release-event",
                        G_CALLBACK (prefs_page_divers_choose_date_format_toggle),
                        NULL);
    g_signal_connect (G_OBJECT (priv->radiobutton_choose_date_3),
                        "button-release-event",
                        G_CALLBACK (prefs_page_divers_choose_date_format_toggle),
                        NULL);
    g_signal_connect (G_OBJECT (priv->radiobutton_choose_date_4),
                        "button-release-event",
                        G_CALLBACK (prefs_page_divers_choose_date_format_toggle),
                        NULL);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean prefs_page_divers_scheduler_warm_button_changed (GtkWidget *checkbutton,
																 PrefsPageDivers *page)
{
    gboolean *value;

    value = g_object_get_data (G_OBJECT (checkbutton), "pointer");
    if (value)
    {
		PrefsPageDiversPrivate *priv;

		*value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbutton));
		priv = prefs_page_divers_get_instance_private (page);
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)))
		{
			gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_nb_days_before_scheduled), FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->hbox_divers_scheduler_set_fixed_day), TRUE);
		}
		else
		{
			gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_nb_days_before_scheduled), TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->hbox_divers_scheduler_set_fixed_day), FALSE);
		}
	}

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
static gboolean prefs_page_divers_scheduler_set_fixed_day_changed (GtkWidget *checkbutton,
																   GtkWidget *widget)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)))
		gtk_widget_set_sensitive (GTK_WIDGET (widget), TRUE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (widget), FALSE);

    return FALSE;
}

/**
 * callback called when changing the account from the button
 *
 * \param button
 *
 * \return FALSE
 * */
static gboolean prefs_page_divers_scheduler_change_account (GtkWidget *combo)
{
	etat.scheduler_default_account_number = gsb_account_get_combo_account_number (combo);

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
static void prefs_page_divers_choose_language_changed (GtkComboBox *combo,
													   PrefsPageDivers *page)
{
	GtkTreeIter iter;

	if (gtk_combo_box_get_active_iter (combo, &iter))
	{
		GSettings *settings;
		GtkTreeModel *model;
		gchar *string;
		gchar *tmp_str;
		gchar *hint;
		gint index;
		GrisbiAppConf *a_conf;

		settings = grisbi_settings_get_settings (SETTINGS_GENERAL);
		a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
		gtk_tree_model_get (model, &iter, 0, &string, 1, &index, -1);
		if (index == 0)
		{
			a_conf->language_chosen = NULL;
			g_settings_reset (G_SETTINGS (settings), "language-chosen");
		}
		else
		{
			a_conf->language_chosen = string;
			g_settings_set_string (G_SETTINGS (settings), "language-chosen", a_conf->language_chosen);
		}

		tmp_str = g_strdup (_("You will have to restart Grisbi for the new language to take effect."));
		hint = g_strdup_printf (_("Changes the language of Grisbi for \"%s\"!"), string);
        dialogue_warning_hint (tmp_str , hint);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gint prefs_page_divers_choose_language_list_new (GtkWidget *combo,
														GrisbiAppConf *a_conf)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	GSList *list = NULL;
	GSList *tmp_list;
	GDir *dir;
	const gchar *str_color;
	const gchar *dirname;
	const gchar *locale_dir;
	gint i = 0;
	gint activ_index = 0;

	locale_dir = gsb_dirs_get_locale_dir ();
	dir = g_dir_open (locale_dir, 0, NULL);
	if (!dir)
		return 0;

	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
	str_color = gsb_rgba_get_couleur_to_string ("text_gsetting_option_normal");

	while ((dirname = g_dir_read_name (dir)) != NULL)
    {
		gchar *filename;

		filename = g_build_filename (locale_dir,
									 dirname,
									 "LC_MESSAGES",
									 GETTEXT_PACKAGE ".mo",
									 NULL);
		if (g_file_test (filename, G_FILE_TEST_EXISTS))
		{
			list = g_slist_insert_sorted (list, g_strdup (dirname), (GCompareFunc) g_strcmp0);
		}

	  g_free (filename);
	}

	g_dir_close (dir);
	tmp_list = list;

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, _("System Language"), 1, i, 2, str_color, -1);

	i++;

	while (tmp_list)
	{
		if (g_strcmp0 (a_conf->language_chosen, tmp_list->data) == 0)
			activ_index = i;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, (gchar *) tmp_list->data, 1, i, 2, str_color, -1);

        i++;
		tmp_list = tmp_list->next;
    }

	gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
									renderer,
									"text", 0,
									"foreground", 2,
									NULL);

	g_free ((gpointer)str_color);

	return activ_index;
}

/**
 * Création de la page de gestion des divers
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_divers_setup_divers_page (PrefsPageDivers *page)
{
	GtkWidget *head_page;
	GtkWidget *entry_divers_programs;
    GtkWidget *vbox_button;
    GtkWidget *combo;
	gint combo_index;
	gboolean is_loading;
	GrisbiAppConf *a_conf;
	PrefsPageDiversPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_divers_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Divers"), "gsb-generalities-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_divers), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_divers), head_page, 0);

    /* set the variables for programs */
	entry_divers_programs = gsb_automem_entry_new (&a_conf->browser_command, NULL, NULL);
	gtk_grid_attach (GTK_GRID (priv->grid_divers_programs), entry_divers_programs, 1, 0, 1, 1);
	if (conf.low_resolution_screen)
	{
		GtkWidget *label;

		label = gtk_label_new ((gpointer) _("Scheduler"));
		priv->notebook_others_options = gtk_notebook_new ();
		gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_others_options),
								  priv->divers_scheduler_paddingbox,
								  label);

		label = gtk_label_new ((gpointer) _("Localization"));
		gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_others_options),
								  priv->divers_localization_paddingbox,
								  label);
		gtk_box_pack_start (GTK_BOX (priv->vbox_divers), priv->notebook_others_options, FALSE, FALSE, 0);
		gtk_widget_show_all (priv->notebook_others_options);
	}
	else
	{
		gtk_label_set_text (GTK_LABEL (priv->title_divers_scheduler), (gpointer) _("Scheduler"));
		gtk_label_set_text (GTK_LABEL (priv->title_divers_localization), (gpointer) _("Localization"));

		gtk_box_pack_start (GTK_BOX (priv->vbox_divers), priv->divers_scheduler_paddingbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (priv->vbox_divers), priv->divers_localization_paddingbox, FALSE, FALSE, 0);
	}

	/* set the scheduled variables */
	vbox_button = gsb_automem_radiobutton_gsettings_new (_("Warn/Execute the scheduled transactions arriving at expiration date"),
													_("Warn/Execute the scheduled transactions of the month"),
													&conf.execute_scheduled_of_month,
													(GCallback) prefs_page_divers_scheduler_warm_button_changed,
													page);

	gtk_box_pack_start (GTK_BOX (vbox_button), priv->hbox_divers_scheduler_nb_days_before_scheduled, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (vbox_button), priv->hbox_divers_scheduler_nb_days_before_scheduled, 1);
	gtk_box_pack_start (GTK_BOX (priv->vbox_divers_scheduler), vbox_button, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_divers_scheduler), vbox_button, 0);

	/* initialise le bouton nombre de jours avant alerte execution */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_nb_days_before_scheduled),
							   conf.nb_days_before_scheduled);

	/* Init checkbutton set fixed day */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_scheduler_set_fixed_day),
								  conf.scheduler_set_fixed_day);

	/* Init spinbutton_scheduler_fixed_day */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_scheduler_fixed_day),
							   conf.scheduler_fixed_day);

	/* Init checkbutton set fixed date */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_scheduler_set_fixed_date),
								  etat.scheduler_set_fixed_date);
	gtk_widget_set_sensitive (priv->checkbutton_scheduler_set_fixed_date, is_loading);

	/* Adding and init widgets select defaut compte */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_scheduler_set_default_account),
								  etat.scheduler_set_default_account);

	if (is_loading)
		combo = gsb_account_create_combo_list (G_CALLBACK (prefs_page_divers_scheduler_change_account), NULL, FALSE);
	else
		combo = utils_prefs_create_combo_list_indisponible ();

	gtk_widget_set_size_request (combo, FORM_COURT_WIDTH, -1);
	gtk_box_pack_start (GTK_BOX (priv->hbox_divers_scheduler_set_default_account), combo, FALSE, FALSE, 0);
	g_object_set_data (G_OBJECT (priv->checkbutton_scheduler_set_default_account),
                       "widget", combo);

	if (etat.scheduler_set_default_account)
		gsb_account_set_combo_account_number (combo, etat.scheduler_default_account_number);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);

	gtk_widget_show (combo);

	/* sensitive widgets */
	if (conf.execute_scheduled_of_month)
	{
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_nb_days_before_scheduled), FALSE);
		gtk_widget_set_sensitive (priv->hbox_divers_scheduler_set_fixed_day, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->hbox_divers_scheduler_set_fixed_day, FALSE);
	}

	if (!conf.scheduler_set_fixed_day)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_scheduler_fixed_day), FALSE);

	/* Connect signal */
    /* callback for spinbutton_nb_days_before_scheduled */
    g_signal_connect (priv->spinbutton_nb_days_before_scheduled,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &conf.nb_days_before_scheduled);

    /* callback for checkbutton_scheduler_set_fixed_day */
    g_signal_connect (priv->checkbutton_scheduler_set_fixed_day,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.scheduler_set_fixed_day);

	g_signal_connect_after (G_OBJECT (priv->checkbutton_scheduler_set_fixed_day),
							"toggled",
							G_CALLBACK (prefs_page_divers_scheduler_set_fixed_day_changed),
							priv->spinbutton_scheduler_fixed_day);

    /* callback for spinbutton_scheduler_fixed_day */
    g_signal_connect (priv->spinbutton_scheduler_fixed_day,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &conf.scheduler_fixed_day);

    /* callback for checkbutton_scheduler_set_fixed_date */
		g_signal_connect (priv->checkbutton_scheduler_set_fixed_date,
						  "toggled",
						  G_CALLBACK (utils_prefs_page_checkbutton_changed),
						  &etat.scheduler_set_fixed_date);

    /* callback for checkbutton_scheduler_set_default_account */
    g_signal_connect (priv->checkbutton_scheduler_set_default_account,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.scheduler_set_default_account);

	/* set the localization parameters */
	/* set the language */
	combo_index = prefs_page_divers_choose_language_list_new (priv->combo_choose_language, a_conf);

	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_language), combo_index);
	g_signal_connect (G_OBJECT (priv->combo_choose_language),
					  "changed",
					  G_CALLBACK (prefs_page_divers_choose_language_changed),
					  NULL);

	/* set the others parameters */
	prefs_page_divers_choose_date_format_init (page);
	prefs_page_divers_choose__number_format_init (page);

	if (is_loading == FALSE)
	{
		gtk_widget_set_sensitive (priv->hbox_divers_scheduler_set_default_account, FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->box_choose_date_format), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->box_choose_numbers_format), FALSE);
	}
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_divers_init (PrefsPageDivers *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_divers_setup_divers_page (page);
}

static void prefs_page_divers_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_divers_parent_class)->dispose (object);
}

static void prefs_page_divers_class_init (PrefsPageDiversClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_divers_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_divers.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, vbox_divers);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, grid_divers_programs);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, divers_localization_paddingbox);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, divers_scheduler_paddingbox);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, title_divers_localization);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, title_divers_scheduler);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, vbox_divers_scheduler);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_set_default_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, checkbutton_scheduler_set_default_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_set_fixed_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, checkbutton_scheduler_set_fixed_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_set_fixed_day);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, checkbutton_scheduler_set_fixed_day);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, spinbutton_scheduler_fixed_day);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_nb_days_before_scheduled);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, spinbutton_nb_days_before_scheduled);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, box_choose_date_format);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, combo_choose_language);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, box_choose_numbers_format);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, combo_choose_decimal_point);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, combo_choose_thousands_separator);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, radiobutton_choose_date_1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, radiobutton_choose_date_2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, radiobutton_choose_date_3);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, radiobutton_choose_date_4);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageDivers *prefs_page_divers_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_DIVERS_TYPE, NULL);
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

