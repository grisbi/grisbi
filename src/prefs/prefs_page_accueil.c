/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
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

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_accueil.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_locale.h"
#include "gsb_data_partial_balance.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageAccueilPrivate   PrefsPageAccueilPrivate;

struct _PrefsPageAccueilPrivate
{
	GtkWidget *			vbox_accueil;

	GtkWidget *			box_lang_fr;
    GtkWidget *			checkbutton_balances_with_scheduled;
    GtkWidget *			hbox_paddingbox_lang_fr;
    GtkWidget *         hbox_paddingbox_partial_balance;
	GtkWidget *			vbox_loading_partial_balance;
	GtkWidget *			button_partial_balance_add;
	GtkWidget *			button_partial_balance_edit;
	GtkWidget *			button_partial_balance_remove;
    GtkWidget *         treeview_partial_balance;
    GtkWidget *			checkbutton_partial_balance;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageAccueil, prefs_page_accueil, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * force le recalcul des soldes et la mise à jour de la page d'accueil
 *
 * \param   none
 *
 * \return  FALSE
 * */
static gboolean prefs_page_accueil_checkbutton_balances_with_scheduled_toggle (GtkToggleButton *button,
																			   GrisbiAppConf *a_conf)
{
    GSList *list_tmp;

	list_tmp = gsb_data_account_get_list_accounts ();
    while (list_tmp)
    {
        gint account_number;

        account_number = gsb_data_account_get_no_account (list_tmp->data);
        gsb_data_account_set_balances_are_dirty (account_number);

        list_tmp = list_tmp->next;
    }
	gsb_gui_navigation_update_home_page ();
	utils_prefs_gsb_file_set_modified ();

    return FALSE;
}

/**
 * callback function for a_conf->group_partial_balance_under_account variable
 *
 * \param button        object clicked
 * \param user_data
 *
 * \return              FALSE
 * */
static gboolean prefs_page_accueil_checkbutton_partial_balance_toggle (GtkToggleButton *button,
																	   GrisbiAppConf *a_conf)
{
    gsb_gui_navigation_update_home_page ();
	utils_prefs_gsb_file_set_modified ();

    return FALSE;
}

/**
 * callback function for a_conf->pluriel_final variable
 *
 * \param button        object clicked
 * \param user_data
 *
 * \return              FALSE
 * */
static gboolean prefs_page_accueil_checkbutton_pluriel_final_toggle (GtkToggleButton *button,
																	 GrisbiAppConf *a_conf)
{
    gsb_gui_navigation_update_home_page ();
	utils_prefs_gsb_file_set_modified ();

    return FALSE;
}

/**
 * Fonction appellée quand on sélectionne un solde partiel
 *
 * \param
 * \param
 * \param
 * \param
 * \param page
 *
 * \return TRUE
 **/
static	gboolean prefs_page_accueil_partial_balance_select_func (GtkTreeSelection *selection,
																 GtkTreeModel *model,
																 GtkTreePath *path,
																 gboolean path_currently_selected,
																 PrefsPageAccueil *page)
{
	PrefsPageAccueilPrivate *priv;

	priv = prefs_page_accueil_get_instance_private (page);

    gtk_widget_set_sensitive (priv->button_partial_balance_edit, TRUE);
    gtk_widget_set_sensitive (priv->button_partial_balance_remove, TRUE);

    return TRUE;
}

/**
 * initialisation de la page de gestion de l'onglet accueil
 *
 * \param page
 *
 * \return
 **/
static void prefs_page_accueil_setup_accueil_page (PrefsPageAccueil *page)
{
	GtkWidget *head_page;
	const gchar *langue;
	gboolean is_loading;
	GrisbiAppConf *a_conf;
	PrefsPageAccueilPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_accueil_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Configuration of the main page"), "gsb-title-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_accueil), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_accueil), head_page, 0);

    /* set the box for french langage*/
	langue = gsb_locale_get_language ();
	if (langue)
	{
		gchar *tmp_str = g_ascii_strup (langue, -1);
		if (g_strstr_len (tmp_str, -1, "FR"))
		{
			GtkWidget *vbox_button;

			vbox_button = gsb_automem_radiobutton_gsettings_new ("Soldes finals",
																 "Soldes finaux",
																 &a_conf->pluriel_final,
																 G_CALLBACK (prefs_page_accueil_checkbutton_pluriel_final_toggle),
																 a_conf);
			gtk_box_pack_start (GTK_BOX (priv->box_lang_fr), vbox_button, FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX (priv->vbox_accueil), priv->hbox_paddingbox_lang_fr, FALSE, FALSE, 0);
			gtk_box_reorder_child (GTK_BOX (priv->vbox_accueil), priv->hbox_paddingbox_lang_fr, 1);
		}
		g_free (tmp_str);
	}

	/* set a_conf->balances_with_scheduled */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_balances_with_scheduled),
								  a_conf->balances_with_scheduled);

	/* set a_conf->group_partial_balance_under_accounts */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_partial_balance),
								  a_conf->group_partial_balance_under_accounts);

	/* set data for buttons */
	g_object_set_data (G_OBJECT (priv->vbox_accueil), "add_button", priv->button_partial_balance_add);
    g_object_set_data (G_OBJECT (priv->vbox_accueil), "edit_button", priv->button_partial_balance_edit);
    g_object_set_data (G_OBJECT (priv->vbox_accueil), "remove_button", priv->button_partial_balance_remove);

	/* Connect signal checkbutton_balances_with_scheduled */
    g_signal_connect (priv->checkbutton_balances_with_scheduled,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->balances_with_scheduled);

	/* Connect signal checkbutton_partial_balance */
    g_signal_connect (priv->checkbutton_partial_balance,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->group_partial_balance_under_accounts);

	/* set gsettings */
	g_signal_connect_after (priv->checkbutton_balances_with_scheduled,
							"toggled",
							G_CALLBACK (prefs_page_accueil_checkbutton_balances_with_scheduled_toggle),
							a_conf);
    g_signal_connect_after (priv->checkbutton_partial_balance,
							"toggled",
							G_CALLBACK (prefs_page_accueil_checkbutton_partial_balance_toggle),
							a_conf);

	if (is_loading)
	{
		GtkWidget *treeview;
	    GtkListStore *list_store;
		GtkTreeViewColumn *column;
		GtkCellRenderer *cell;
		GtkTreeSelection *selection;
		GtkTreeDragDestIface * dst_iface;
		GtkTreeDragSourceIface * src_iface;
		static GtkTargetEntry row_targets[] = {{(gchar*)"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0}};

		/* create the model */
		list_store = gsb_partial_balance_create_model ();

		/* populate the model if necessary */
		if (g_slist_length (gsb_data_partial_balance_get_list ()) > 0)
			gsb_partial_balance_fill_model (list_store);

		/* init the treeview */
		treeview = priv->treeview_partial_balance;
		gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (list_store));
		gtk_widget_set_name (treeview, "colorized_tree_view");
		g_object_set_data (G_OBJECT (priv->vbox_accueil), "treeview", treeview);

		/* Enable drag & drop */
		gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (treeview),
												GDK_BUTTON1_MASK,
												row_targets,
												1,
												GDK_ACTION_MOVE);
		gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (treeview),
											  row_targets,
											  1,
											  GDK_ACTION_MOVE);
		gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
		gtk_tree_selection_set_select_function (selection,
												(GtkTreeSelectionFunc) prefs_page_accueil_partial_balance_select_func,
												page,
												NULL);

		/* Nom du solde partiel */
		cell = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Name"),
														   cell,
														   "text", 0,
														   "cell-background-rgba", PARTIAL_BACKGROUND_COLOR,
														   NULL);
		gtk_tree_view_column_set_alignment (column, 0.5);
		gtk_tree_view_column_set_sort_column_id (column, 0);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

		/* Liste des comptes */
		cell = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Accounts list"),
														   cell,
														   "text", 1,
														   "cell-background-rgba", PARTIAL_BACKGROUND_COLOR,
														   NULL);
		gtk_tree_view_column_set_alignment (column, 0.5);
		gtk_tree_view_column_set_sort_column_id (column, 1);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

		/* Colorize */
		cell = gtk_cell_renderer_toggle_new ();
		g_signal_connect (cell,
							"toggled",
							G_CALLBACK (gsb_partial_balance_colorise_toggled),
							treeview);
		gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE(cell), FALSE);
		g_object_set (cell, "xalign", 0.5, NULL);

		column = gtk_tree_view_column_new_with_attributes (_("Colorize"),
														   cell,
														   "active", 5,
														   "cell-background-rgba", PARTIAL_BACKGROUND_COLOR,
														   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		/* Type de compte */
		cell = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Account kind"),
														   cell,
														   "text", 2,
														   "cell-background-rgba", PARTIAL_BACKGROUND_COLOR,
														   NULL);
		gtk_tree_view_column_set_alignment (column, 0.5);
		gtk_tree_view_column_set_sort_column_id (column, 2);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

		/* Devise */
		cell = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Currency"),
														   cell,
														   "text", 3,
														   "cell-background-rgba", PARTIAL_BACKGROUND_COLOR,
														   NULL);
		gtk_tree_view_column_set_alignment (column, 0.5);
		gtk_tree_view_column_set_sort_column_id (column, 3);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

		dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (list_store);
		if (dst_iface)
			dst_iface -> drag_data_received = &gsb_data_partial_balance_drag_data_received;

		src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (list_store);
		if (src_iface)
		{
			gtk_selection_add_target (treeview, GDK_SELECTION_PRIMARY, GDK_SELECTION_TYPE_ATOM, 1);
			src_iface -> drag_data_get = &gsb_data_partial_balance_drag_data_get;
		}

		 utils_set_list_store_background_color (treeview, PARTIAL_BACKGROUND_COLOR);
		g_object_unref (list_store);
	}
	else
	{
		gtk_widget_set_sensitive (priv->vbox_loading_partial_balance, FALSE);
	}
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_accueil_init (PrefsPageAccueil *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_accueil_setup_accueil_page (page);
}

static void prefs_page_accueil_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_accueil_parent_class)->dispose (object);
}

static void prefs_page_accueil_class_init (PrefsPageAccueilClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_accueil_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_accueil.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, vbox_accueil);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, box_lang_fr);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, hbox_paddingbox_lang_fr);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, checkbutton_balances_with_scheduled);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, hbox_paddingbox_partial_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, treeview_partial_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, vbox_loading_partial_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, button_partial_balance_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, button_partial_balance_edit);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, button_partial_balance_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageAccueil, checkbutton_partial_balance);

	/* set callback functions */
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), gsb_partial_balance_add);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), gsb_partial_balance_edit);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), gsb_partial_balance_remove);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageAccueil *prefs_page_accueil_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_ACCUEIL_TYPE, NULL);
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

