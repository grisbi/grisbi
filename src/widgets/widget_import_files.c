/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                               */
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
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_import_files.h"
#include "gsb_automem.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetImportFilesPrivate   WidgetImportFilesPrivate;

struct _WidgetImportFilesPrivate
{
	GtkWidget *			vbox_import_files;

    GtkWidget *         spinbutton_import_files_nb_days;
    GtkWidget *			checkbutton_fusion_import_transactions;
    GtkWidget *			checkbutton_associate_categorie_for_payee;
    GtkWidget *			checkbutton_extract_number_for_check;
    GtkWidget *			checkbutton_copy_payee_in_note;
    GtkWidget *			checkbutton_csv_force_date_valeur_with_date;
    GtkWidget *			checkbutton_qif_use_field_extract_method_payment;
	GtkWidget *			checkbutton_qif_no_import_categories;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetImportFiles, widget_import_files, GTK_TYPE_BOX)

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
static gboolean widget_import_files_spinbutton_import_files_nb_days_changed (GtkWidget *spinbutton,
																				 gpointer null)
{
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

    w_etat->import_files_nb_days = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

    gsb_file_set_modified (TRUE);

    return ( FALSE );
}

/**
 * Création de la page de gestion des import_files
 *
 * \param prefs
 *
 * \return
 */
static void widget_import_files_setup_import_files_page (WidgetImportFiles *page)
{
	GrisbiWinEtat *w_etat;
	WidgetImportFilesPrivate *priv;

	devel_debug (NULL);

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	priv = widget_import_files_get_instance_private (page);

	/* set the variables for import of files*/
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_import_files_nb_days),
							   w_etat->import_files_nb_days);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_fusion_import_transactions),
								  w_etat->fusion_import_transactions);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_associate_categorie_for_payee),
								  w_etat->associate_categorie_for_payee);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_extract_number_for_check),
								  w_etat->extract_number_for_check);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_copy_payee_in_note),
								  w_etat->copy_payee_in_note);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_force_date_valeur_with_date),
								  w_etat->csv_force_date_valeur_with_date);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_qif_no_import_categories),
								  w_etat->qif_no_import_categories);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_qif_use_field_extract_method_payment),
								  w_etat->qif_use_field_extract_method_payment);

	/* set the choice of date for the financial year */
	gsb_automem_radiobutton_new_with_title (priv->vbox_import_files,
											_("Set the financial year"),
											_("According to the date"),
											_("According to the value date (if fail, try with the date)"),
											&w_etat->get_fyear_by_value_date,
											NULL,
											NULL);

    /* Connect signal spinbutton_import_files_nb_days */
    g_signal_connect ( G_OBJECT (priv->spinbutton_import_files_nb_days),
					  "value-changed",
					  G_CALLBACK (widget_import_files_spinbutton_import_files_nb_days_changed),
					  NULL );

    /* Connect signal checkbutton_fusion_import_transactions */
    g_signal_connect (priv->checkbutton_fusion_import_transactions,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->fusion_import_transactions);
    g_signal_connect_after (priv->checkbutton_fusion_import_transactions,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    /* Connect signal checkbutton_associate_categorie_for_payee */
    g_signal_connect (priv->checkbutton_associate_categorie_for_payee,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->associate_categorie_for_payee);
    g_signal_connect_after (priv->checkbutton_associate_categorie_for_payee,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    /* Connect signal checkbutton_extract_number_for_check */
    g_signal_connect (priv->checkbutton_extract_number_for_check,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->extract_number_for_check);
    g_signal_connect_after (priv->checkbutton_extract_number_for_check,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    /* Connect signal checkbutton_copy_payee_in_note */
    g_signal_connect (priv->checkbutton_copy_payee_in_note,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->copy_payee_in_note);
    g_signal_connect_after (priv->checkbutton_copy_payee_in_note,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    /* Connect signal checkbutton_csv_force_date_valeur_with_date */
    g_signal_connect (priv->checkbutton_csv_force_date_valeur_with_date,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->csv_force_date_valeur_with_date);
    g_signal_connect_after (priv->checkbutton_csv_force_date_valeur_with_date,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    /* Connect signal checkbutton_qif_no_import_categories */
    g_signal_connect (priv->checkbutton_qif_no_import_categories,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->qif_no_import_categories);
    g_signal_connect_after (priv->checkbutton_qif_no_import_categories,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    /* Connect signal checkbutton_qif_use_field_extract_method_payment */
    g_signal_connect (priv->checkbutton_qif_use_field_extract_method_payment,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->qif_use_field_extract_method_payment);
    g_signal_connect_after (priv->checkbutton_qif_use_field_extract_method_payment,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_import_files_init (WidgetImportFiles *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	widget_import_files_setup_import_files_page (page);
}

static void widget_import_files_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_import_files_parent_class)->dispose (object);
}

static void widget_import_files_class_init (WidgetImportFilesClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_import_files_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_import_files.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, vbox_import_files);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, spinbutton_import_files_nb_days);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, checkbutton_fusion_import_transactions);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, checkbutton_associate_categorie_for_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, checkbutton_extract_number_for_check);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, checkbutton_copy_payee_in_note);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, checkbutton_csv_force_date_valeur_with_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, checkbutton_qif_no_import_categories);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportFiles, checkbutton_qif_use_field_extract_method_payment);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
WidgetImportFiles * widget_import_files_new (GrisbiPrefs *win)
{
  return g_object_new (WIDGET_IMPORT_FILES_TYPE, NULL);
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

