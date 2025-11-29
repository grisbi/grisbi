/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                                */
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
#include "prefs_page_import.h"
#include "widget_import_asso.h"
#include "widget_import_files.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageImportPrivate   PrefsPageImportPrivate;

struct _PrefsPageImportPrivate
{
	GtkWidget *			vbox_import;

    GtkWidget *			notebook_import_pages;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageImport, prefs_page_import, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des import
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_import_setup_page (PrefsPageImport *page,
                                          GrisbiPrefs *win)
{
	GtkWidget *head_page;
	GtkWidget *vbox_import_files;
	GtkWidget *vbox_import_asso;
	GtkWidget *label;
	GrisbiWinRun *w_run;
	PrefsPageImportPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_import_get_instance_private (page);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Import"), "gsb-import-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_import), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_import), head_page, 0);

	vbox_import_files = GTK_WIDGET (widget_import_files_new (win));
	gtk_widget_set_margin_top (vbox_import_files, MARGIN_TOP);
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_import_pages), vbox_import_files, NULL);
	label = gtk_label_new (_("Files import"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (priv->notebook_import_pages), vbox_import_files, label);

	/* set manage associations */
	vbox_import_asso = GTK_WIDGET (widget_import_asso_new (win));
	gtk_widget_set_margin_top (vbox_import_asso, MARGIN_TOP);
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_import_pages), vbox_import_asso, NULL);
	label = gtk_label_new (_("Associations for import"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (priv->notebook_import_pages), vbox_import_asso, label);

	/* set page of notebook */
	if (w_run->prefs_import_tab)
		gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_import_pages), w_run->prefs_import_tab);

	/* set signal notebook_impert */
	g_signal_connect (G_OBJECT (priv->notebook_import_pages),
	                  "switch-page",
	                  (GCallback) utils_prefs_page_notebook_switch_page,
	                  &w_run->prefs_import_tab);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_import_init (PrefsPageImport *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void prefs_page_import_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_import_parent_class)->dispose (object);
}

static void prefs_page_import_class_init (PrefsPageImportClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_import_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_import.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImport, vbox_import);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImport, notebook_import_pages);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
PrefsPageImport *prefs_page_import_new (GrisbiPrefs *win)
{
	PrefsPageImport *page;

	page = g_object_new (PREFS_PAGE_IMPORT_TYPE, NULL);
	prefs_page_import_setup_page (page,win);

	return page;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *prefs_page_import_get_notebook (GtkWidget *page)
{
	PrefsPageImportPrivate *priv;

	priv = prefs_page_import_get_instance_private (PREFS_PAGE_IMPORT (page));

	return priv->notebook_import_pages;
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

