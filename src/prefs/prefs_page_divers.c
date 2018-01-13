/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
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
#include "dialog.h"
#include "grisbi_settings.h"
#include "gsb_account.h"
#include "gsb_automem.h"
#include "gsb_dirs.h"
#include "parametres.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDiversPrivate   PrefsPageDiversPrivate;

struct _PrefsPageDiversPrivate
{
	GtkWidget *			vbox_divers;

    GtkWidget *			grid_divers_programs;

	GtkWidget *         vbox_divers_scheduler;
	GtkWidget *         hbox_divers_scheduler_set_default_account;
	GtkWidget *			checkbutton_scheduler_set_default_account;
	GtkWidget *			eventbox_scheduler_set_default_account;
	GtkWidget *         hbox_divers_scheduler_set_fixed_date;
	GtkWidget *			checkbutton_scheduler_set_fixed_date;
	GtkWidget *			eventbox_scheduler_set_fixed_date;
	GtkWidget *         gtkcombobox_scheduler_fixed_date;
	GtkWidget *         hbox_divers_scheduler_set_fixed_day;
	GtkWidget *			checkbutton_scheduler_set_fixed_day;
	GtkWidget *			eventbox_scheduler_set_fixed_day;
	GtkWidget *         spinbutton_scheduler_fixed_day;
	GtkWidget *			hbox_divers_scheduler_nb_days_before_scheduled;
    GtkWidget *         spinbutton_nb_days_before_scheduled;
	GtkWidget *			box_divers_localisation;
	GtkWidget *			combo_choose_language;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDivers, prefs_page_divers, GTK_TYPE_BOX)

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
static gboolean prefs_page_divers_scheduler_set_fixed_date_changed (GtkWidget *checkbutton,
																	GtkWidget *widget)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (widget), TRUE);
	}
	else
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);
		etat.scheduler_default_fixed_date = 0;
		gtk_widget_set_sensitive (GTK_WIDGET (widget), FALSE);
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
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_divers_scheduler_default_fixed_day_changed (GtkComboBox *widget,
																   gpointer     user_data)
{
	etat.scheduler_default_fixed_date = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
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

		settings = grisbi_settings_get_settings (SETTINGS_GENERAL);
		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
		gtk_tree_model_get (model, &iter, 0, &string, 1, &index, -1);
		if (index == 0)
		{
			conf.language_chosen = NULL;
			g_settings_reset (G_SETTINGS (settings), "language-chosen");
		}
		else
		{
			conf.language_chosen = string;
			g_settings_set_string (G_SETTINGS (settings), "language-chosen", conf.language_chosen);
		}

		tmp_str = g_strdup (_("You will have to restart Grisbi for the new langage to take effect."));
		hint = g_strdup_printf ( _("Changes the language of Grisbi for \"%s\"!"), string );
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
static gint prefs_page_divers_choose_language_list_new (GtkWidget *combo)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	GSList *list = NULL;
	GSList *tmp_list;
	GDir *dir;
	const gchar *dirname;
	const gchar *locale_dir;
	gint i = 0;
	gint activ_index = 0;

	locale_dir = gsb_dirs_get_locale_dir ();
	dir = g_dir_open (locale_dir, 0, NULL);
	if (!dir)
		return 0;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);

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
	gtk_list_store_set (store, &iter, 0, _("System Language"), 1, i, -1);
	i++;

	while (tmp_list)
	{
		if (g_strcmp0 (conf.language_chosen, tmp_list->data) == 0)
			activ_index = i;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, (gchar *) tmp_list->data, 1, i, -1);

        i++;
		tmp_list = tmp_list->next;
    }

	gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
									renderer,
									"text", 0,
									NULL);

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
	GtkWidget *date_widget;
	GtkWidget *number_widget;
	gint combo_index;
	gboolean is_loading;
	PrefsPageDiversPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_divers_get_instance_private (page);
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Divers"), "gsb-generalities-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_divers), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_divers), head_page, 0);

    /* set the variables for programs */
	entry_divers_programs = gsb_automem_entry_new ( &conf.browser_command, NULL, NULL);
	gtk_grid_attach (GTK_GRID (priv->grid_divers_programs), entry_divers_programs, 1, 0, 1, 1);

	/* set the scheduled variables */
	vbox_button = utils_prefs_automem_radiobutton_blue_new (_("Warn/Execute the scheduled transactions arriving at expiration date"),
															_("Warn/Execute the scheduled transactions of the month"),
															&conf.execute_scheduled_of_month,
															(GCallback) prefs_page_divers_scheduler_warm_button_changed,
															page);

	gtk_box_pack_start (GTK_BOX (vbox_button), priv->hbox_divers_scheduler_nb_days_before_scheduled, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (vbox_button), priv->hbox_divers_scheduler_nb_days_before_scheduled, 1);
	gtk_box_pack_start (GTK_BOX (priv->vbox_divers_scheduler), vbox_button, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_divers_scheduler), vbox_button, 0);

	/* initialise le bouton nombre de jours avant alerte execution */
	if (conf.execute_scheduled_of_month)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_nb_days_before_scheduled), FALSE);

	/* Adding set fixed date */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_scheduler_set_fixed_date),
								  etat.scheduler_set_fixed_date);
	if (!etat.scheduler_set_fixed_date)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->gtkcombobox_scheduler_fixed_date), FALSE);

    g_signal_connect (priv->eventbox_scheduler_set_fixed_date,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_scheduler_set_fixed_date);

    g_signal_connect (priv->checkbutton_scheduler_set_fixed_date,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.scheduler_set_fixed_date);

	g_signal_connect_after (G_OBJECT (priv->checkbutton_scheduler_set_fixed_date),
							"toggled",
							G_CALLBACK (prefs_page_divers_scheduler_set_fixed_date_changed),
							priv->gtkcombobox_scheduler_fixed_date);

	/* Adding set fixed day */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_scheduler_set_fixed_day),
								  conf.scheduler_set_fixed_day);
	if (!conf.scheduler_set_fixed_day)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_scheduler_fixed_day), FALSE);

    g_signal_connect (priv->eventbox_scheduler_set_fixed_day,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_scheduler_set_fixed_day);

    g_signal_connect (priv->checkbutton_scheduler_set_fixed_day,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.scheduler_set_fixed_day);

	g_signal_connect_after (G_OBJECT (priv->checkbutton_scheduler_set_fixed_day),
							"toggled",
							G_CALLBACK (prefs_page_divers_scheduler_set_fixed_day_changed),
							priv->spinbutton_scheduler_fixed_day);

	if (conf.execute_scheduled_of_month)
	{
		gtk_widget_set_sensitive (priv->hbox_divers_scheduler_set_fixed_day, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->hbox_divers_scheduler_set_fixed_day, FALSE);
	}

	/* Adding select defaut compte */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_scheduler_set_default_account),
								  etat.scheduler_set_default_account);

    combo = gsb_account_create_combo_list (G_CALLBACK (prefs_page_divers_scheduler_change_account), NULL, FALSE);
	gtk_box_pack_start (GTK_BOX (priv->hbox_divers_scheduler_set_default_account), combo, FALSE, FALSE, 0);
	g_object_set_data (G_OBJECT (priv->checkbutton_scheduler_set_default_account),
                       "widget", combo);

	if (etat.scheduler_set_default_account)
		gsb_account_set_combo_account_number (combo, etat.scheduler_default_account_number);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);

	gtk_widget_show (combo);

	/* Connect signal */
    g_signal_connect (priv->eventbox_scheduler_set_default_account,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_scheduler_set_default_account);

    g_signal_connect (priv->checkbutton_scheduler_set_default_account,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.scheduler_set_default_account);

	/* set spinbutton_nb_days_before_scheduled value */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_nb_days_before_scheduled),
							   conf.nb_days_before_scheduled);

    /* callback for spinbutton_nb_days_before_scheduled */
    g_signal_connect (priv->spinbutton_nb_days_before_scheduled,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &conf.nb_days_before_scheduled);

	/* gtkcombobox_scheduler_fixed_date */
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->gtkcombobox_scheduler_fixed_date), etat.scheduler_default_fixed_date);

    /* callback for gtkcombobox_scheduler_fixed_date */
    g_signal_connect (priv->gtkcombobox_scheduler_fixed_date,
					  "changed",
					  G_CALLBACK (prefs_page_divers_scheduler_default_fixed_day_changed),
					  NULL);

	/* set spinbutton_scheduler_fixed_day */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_scheduler_fixed_day),
							   conf.scheduler_fixed_day);

    /* callback for spinbutton_scheduler_fixed_day */
    g_signal_connect (priv->spinbutton_scheduler_fixed_day,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &conf.scheduler_fixed_day);

	/* set the localization parameters */
	/* set the language */
	combo_index = prefs_page_divers_choose_language_list_new (priv->combo_choose_language);

	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_choose_language), combo_index);
	g_signal_connect (G_OBJECT (priv->combo_choose_language),
					  "changed",
					  G_CALLBACK (prefs_page_divers_choose_language_changed),
					  NULL);

	/* set the others parameters */
	date_widget = gsb_config_date_format_chosen (priv->box_divers_localisation, GTK_ORIENTATION_HORIZONTAL);
    number_widget = gsb_config_number_format_chosen (priv->box_divers_localisation, GTK_ORIENTATION_VERTICAL);

	if (is_loading == FALSE)
	{
		gtk_widget_set_sensitive (priv->hbox_divers_scheduler_set_default_account, FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (date_widget), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (number_widget), FALSE);
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

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, vbox_divers_scheduler);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_set_default_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, checkbutton_scheduler_set_default_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, eventbox_scheduler_set_default_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_set_fixed_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, checkbutton_scheduler_set_fixed_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, eventbox_scheduler_set_fixed_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, gtkcombobox_scheduler_fixed_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_set_fixed_day);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, checkbutton_scheduler_set_fixed_day);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, eventbox_scheduler_set_fixed_day);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, spinbutton_scheduler_fixed_day);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, hbox_divers_scheduler_nb_days_before_scheduled);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, spinbutton_nb_days_before_scheduled);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, box_divers_localisation);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, combo_choose_language);
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

