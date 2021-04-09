/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "etats_page_payee.h"
#include "dialog.h"
//~ #include "etats_config.h"
#include "etats_prefs.h"
#include "gsb_data_payee.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/
	/* return */

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
#ifdef OS_OSX
static const gchar *label_search_help = N_("Command-click\nto add to the selection");
#endif /* OS_OSX */

/* variables utilisées pour la gestion des tiers*/
static GtkTreePath *payee_selected = NULL;
/*END_STATIC*/

typedef struct _EtatsPagePayeePrivate   EtatsPagePayeePrivate;

struct _EtatsPagePayeePrivate
{
	GtkWidget *			vbox_etats_page_payee;

	GtkWidget *			button_detaille_payee;
	GtkWidget *			button_first_payee;
	GtkWidget *			button_last_payee;
	GtkWidget *			button_next_payee;
	GtkWidget *			button_previous_payee;
	GtkWidget *			entry_search_payee;
	GtkWidget *			label_search_help_payee;
	GtkWidget *			togglebutton_select_all_payee;
	GtkWidget *			treeview_payee;
	GtkWidget *			vbox_detaille_payee;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPagePayee, etats_page_payee, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Fonction de de recherche de tiers
 *
 * \param text à rechercher
 * \param tree_view pour la recherche
 *
 * \return
 **/
static void etats_page_payee_search_iter_from_entry (const gchar *text,
													 GtkTreeView *tree_view,
													 gint sens)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	gint index = 1;
	glong longueur;

	if (!text || strlen (text) == 0)
		return;

	model = gtk_tree_view_get_model (tree_view);
	path = gtk_tree_path_new_first ();

	if (!gtk_tree_model_get_iter (model, &iter, path))
		return;

	do
	{
		gchar *str;
		gchar *tmp_str;

		gtk_tree_model_get (model, &iter, 0, &str, -1);

		longueur = g_utf8_strlen (text, -1);
		tmp_str = g_strndup (str, longueur);

		if (strcmp (g_utf8_casefold (tmp_str, -1),  g_utf8_casefold (text, -1)) == 0)
		{
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, TRUE, 0.0, 0.0);
			break;
		}
		if (path)
		{
			gtk_tree_path_next (path);
			if (!gtk_tree_model_get_iter (model, &iter, path))
				index--;
		}
	}

	/*termine la boucle si la lettre n'existe pas */
	while (index);
}

/**
 * Fonction de CALLBACK pour la recherche de tiers
 *
 * \param editable
 * \param new_test
 * \param longueur ajoutée
 * \param position de l'ajout
 * \param tree_view pour la recherche
 *
 * \return
 **/
static void etats_page_payee_entry_insert_text (GtkEditable *editable,
												gchar *new_text,
												gint new_text_length,
												gpointer position,
												GtkWidget *tree_view)
{
	gchar *text;

	/* on bloque l'appel de la fonction */
	g_signal_handlers_block_by_func (G_OBJECT (editable),
									 G_CALLBACK (etats_page_payee_entry_insert_text),
									 tree_view);

	gtk_editable_insert_text (editable, new_text, new_text_length, position);

	/* on lance la recherche de la chaine */
	text = gtk_editable_get_chars (editable, 0, -1);
	etats_page_payee_search_iter_from_entry (text, GTK_TREE_VIEW (tree_view), GDK_RIGHTBUTTON);

	/* on débloque l'appel de la fonction */
	g_signal_handlers_unblock_by_func (G_OBJECT (editable),
									   G_CALLBACK (etats_page_payee_entry_insert_text),
									   tree_view);

	/* evite d'écrire en double dans l'entry */
	g_signal_stop_emission_by_name (editable, "insert_text");
}

/**
 * Fonction de CALLBACK pour la recherche de tiers
 *
 * \param editable
 * \param début du caractère supprimé
 * \param fin du caractère supprimé
 * \param tree_view pour la recherche
 *
 * \return
 **/
static void etats_page_payee_entry_delete_text (GtkEditable *editable,
												gint start_pos,
												gint end_pos,
												GtkWidget *tree_view)
{
	gchar *text;

	text = gtk_editable_get_chars (editable, 0, start_pos);
	etats_page_payee_search_iter_from_entry (text, GTK_TREE_VIEW (tree_view), GDK_LEFTBUTTON);
}

/**
 * gère la sensibilité des boutons premier,précédent, suivant et dernier pour les tiers
 *
 * \param show left buttons 0 = insensitif 1 = sensitif -1 = sans changement
 * \param show right buttons 0 = insensitif 1 = sensitif -1 = sans changement
 *
 * \return
 **/
static void etats_page_payee_show_hide_prev_next_buttons (gint show_left,
														  gint show_right,
														  EtatsPagePayeePrivate *priv)
{
	if (show_left >= 0)
	{
		gtk_widget_set_sensitive (priv->button_first_payee, show_left);
		gtk_widget_set_sensitive (priv->button_previous_payee, show_left);
	}
	if (show_right >= 0)
	{
		gtk_widget_set_sensitive (priv->button_next_payee, show_right);
		gtk_widget_set_sensitive (priv->button_last_payee, show_right);
	}
}

/**
 * Gestion des flêches de déplacement des tiers sélextionnés
 *
 * \param button
 * \param event
 * \param priv
 *
 * \return
 **/
static gboolean etats_page_payee_select_prev_next_item (GtkWidget *button,
														GdkEventButton *event,
														EtatsPagePayeePrivate *priv)
{
	GtkTreePath *start_path;
	GtkTreePath *end_path;
	GtkTreePath *first_path;
	GtkTreePath *last_path;
	GtkTreePath *path = NULL;
	GtkTreeSelection *selection;
	GList *liste;
	gpointer ptr_sens;
	gint sens;
	gint nbre_selections;
	gboolean find = FALSE;
	gboolean return_value = FALSE;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payee));
	liste = gtk_tree_selection_get_selected_rows (selection, NULL);
	if (liste == NULL)
	{
		etats_page_payee_show_hide_prev_next_buttons (0, 0, priv);

		return FALSE;
	}

	ptr_sens =  g_object_get_data (G_OBJECT (button), "ptr_sens");
	sens = GPOINTER_TO_INT (ptr_sens);

	if (payee_selected == NULL)
		payee_selected = liste->data;

	nbre_selections = g_list_length (liste);
	first_path = g_list_nth_data (liste, 0);
	last_path = g_list_nth_data (liste, nbre_selections - 1);

	liste = g_list_find_custom (liste, payee_selected, (GCompareFunc) gtk_tree_path_compare);
	if (gtk_tree_view_get_visible_range (GTK_TREE_VIEW (priv->treeview_payee), &start_path, &end_path))
	{
		while (liste)
		{
			path = (GtkTreePath *) liste->data;

			if (gtk_tree_selection_path_is_selected (selection, path))
			{
				if (sens == GDK_LEFTBUTTON)
				{
					if (gtk_tree_path_compare (start_path, path) <= 0)
					{
						liste = liste->prev;
						continue;
					}
					else if (gtk_tree_path_compare (path, payee_selected) != 0)
					{
						gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->treeview_payee), path, NULL, FALSE, 0., 0.);
						payee_selected = path;
						find = TRUE;
						break;
					}
				}
				else
				{
					if (gtk_tree_path_compare (path, end_path) <= 0)
					{
						liste = liste->next;
						continue;
					}
					else if (gtk_tree_path_compare (path, payee_selected) != 0)
					{
						gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->treeview_payee), path, NULL, FALSE, 0., 0.);
						payee_selected = path;
						find = TRUE;
						break;
					}
				}
			}

			if (sens == GDK_LEFTBUTTON)
				liste = liste->prev;
			else
				liste = liste->next;
		}

		gtk_tree_path_free (start_path);
		gtk_tree_path_free (end_path);

		if (find)
			return_value = TRUE;
	}

	if (!find)
	{
		liste = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (liste == NULL)
			return FALSE;

		if (sens == GDK_LEFTBUTTON)
			liste = g_list_first (liste);
		else
			liste = g_list_last (liste);

		path = (GtkTreePath *) liste->data;
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->treeview_payee), path, NULL, FALSE, 0., 0.);
		payee_selected = path;

		return_value = TRUE;
	}

	if (gtk_tree_path_compare (path, first_path) == 0)
		etats_page_payee_show_hide_prev_next_buttons (0, -1, priv);
	else
		etats_page_payee_show_hide_prev_next_buttons (1, -1, priv);

	if (gtk_tree_path_compare (path, last_path) == 0)
		etats_page_payee_show_hide_prev_next_buttons (-1, 0, priv);
	else
		etats_page_payee_show_hide_prev_next_buttons (-1, 1, priv);

	return return_value;
}

/**
 * Gestion des flèches de déplacement des tiers sélextionnés
 *
 * \param button
 * \param event
 * \param priv
 *
 * \return
 **/
static gboolean etats_page_payee_select_first_last_item (GtkWidget *button,
														 GdkEventButton *event,
														 EtatsPagePayeePrivate *priv)
{
	GtkTreeSelection *selection;
	GtkTreePath *first_path;
	GtkTreePath *last_path;
	GList *liste;
	gpointer ptr_sens;
	gint sens;
	gint nbre_selections;

	ptr_sens =  g_object_get_data (G_OBJECT (button), "ptr_sens");
	sens = GPOINTER_TO_INT (ptr_sens);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payee));

	liste = gtk_tree_selection_get_selected_rows (selection, NULL);
	if (liste == NULL)
	{
		etats_page_payee_show_hide_prev_next_buttons (0, 0, priv);

		return FALSE;
	}
	nbre_selections = g_list_length (liste);

	first_path = g_list_nth_data (liste, 0);
	last_path = g_list_nth_data (liste, nbre_selections - 1);

	if (sens == GDK_LEFTBUTTON)
	{
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->treeview_payee), first_path, NULL, FALSE, 0., 0.);
		payee_selected = first_path;
		etats_page_payee_show_hide_prev_next_buttons (0, 1, priv);
	}
	else
	{
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->treeview_payee), last_path, NULL, FALSE, 0., 0.);
		payee_selected = last_path;
		etats_page_payee_show_hide_prev_next_buttons (1, 0, priv);
	}

	return TRUE;
}

/**
 * fonction de callback de changement de sélection
 *
 * \param selection
 * \param NULL
 *
 * \return
 **/
static void etats_page_payee_selection_changed (GtkTreeSelection *selection,
												EtatsPagePayeePrivate *priv)
{
	GList *liste;

	/* on récupère la liste des libnes sélectionnées */
	liste = gtk_tree_selection_get_selected_rows (selection, NULL);

	/* on change la sensibilité des boutons de navigation si nécessaire */
	if (g_list_length (liste) > 1)
		etats_page_payee_show_hide_prev_next_buttons (1, 1, priv);
	else
		etats_page_payee_show_hide_prev_next_buttons (0, 0, priv);
}

/**
 * positionne le tree_view sur le premier tiers sélectionné
 *
 * \param tree_view
 * \param event
 * \param user_data = priv
 *
 * \return
 **/
static gboolean etats_page_payee_show_first_row_selected (GtkWidget *tree_view,
														  GdkEventVisibility  *event,
														  EtatsPagePayeePrivate *priv)
{
	GtkTreeSelection *selection;
	GtkTreePath *start_path;
	GtkTreePath *end_path;
	GtkTreePath *payee_selected;
	GList *liste;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	liste = gtk_tree_selection_get_selected_rows (selection, NULL);
	if (liste)
		payee_selected = (GtkTreePath *) liste->data;
	else
	{
		/* on ajoute un callback pour gérer le changement de sélection */
		g_signal_connect (G_OBJECT (selection),
						  "changed",
						  G_CALLBACK (etats_page_payee_selection_changed),
						  priv);

		return FALSE;
	}

	if (gtk_tree_view_get_visible_range (GTK_TREE_VIEW (tree_view), &start_path, &end_path))
	{
		if (payee_selected && gtk_tree_path_compare (payee_selected, end_path) == 1)
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), payee_selected, NULL, FALSE, 0., 0.);

		gtk_tree_path_free (start_path);
		gtk_tree_path_free (end_path);
	}
	else if (payee_selected)
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), payee_selected, NULL, FALSE, 0., 0.);

	/* on ajoute un callback pour gérer le changement de sélection */
	g_signal_connect (G_OBJECT (selection),
					  "changed",
					  G_CALLBACK (etats_page_payee_selection_changed),
					  priv);

	return TRUE;
}

/**
 * récupère l'index l'iter selectionné
 *
 * \param nom du tree_view
 *
 * \return numéro de la ligne sélectionnée
 **/
static GSList *etats_page_payee_get_rows_selected (GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GSList *tmp_list = NULL;
	GList *rows_list;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	rows_list = gtk_tree_selection_get_selected_rows (selection, &model);
	while (rows_list)
	{
		GtkTreePath *path;
		gint index;

		path = rows_list->data;

		gtk_tree_model_get_iter (model, &iter, path) ;
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);
		tmp_list = g_slist_append (tmp_list, GINT_TO_POINTER (index));

		rows_list = rows_list->next;
	}
	g_list_free_full (rows_list, (GDestroyNotify) gtk_tree_path_free);

	return tmp_list;
}

/**
 * retourne la liste des tiers
 *
 * \param
 *
 * \return model
 * */
static GtkTreeModel *etats_page_payee_get_liste_tiers (void)
{
	GtkListStore *list_store;
	GSList *list_tmp;

	list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store), 0, GTK_SORT_ASCENDING);

	/* on remplit la liste des tiers */
	list_tmp = gsb_data_payee_get_payees_list ();

	while (list_tmp)
	{
		GtkTreeIter iter;
		gchar *name;
		gint payee_number;

		payee_number = gsb_data_payee_get_no_payee (list_tmp->data);

		name = my_strdup (gsb_data_payee_get_name (payee_number, FALSE));

		gtk_list_store_append (list_store, &iter);
		gtk_list_store_set (list_store, &iter, 0, name, 1, payee_number, -1);

		if (name)
			g_free (name);

		list_tmp = list_tmp->next;
	}

	return GTK_TREE_MODEL (list_store);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void etats_page_payee_setup_treeview (EtatsPagePayee *page)
{
	GtkCellRenderer *cell;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	EtatsPagePayeePrivate *priv;

	priv = etats_page_payee_get_instance_private (page);

	/* set le model */
	model = etats_page_payee_get_liste_tiers ();
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_payee), model);
	g_object_unref (G_OBJECT (model));

	/* set the color of selected row */
	gtk_widget_set_name (priv->treeview_payee, "tree_view");

	/* set the column */
	cell = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_payee), GTK_TREE_VIEW_COLUMN (column));

	/* set selection */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payee));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	/* on ajoute un callback pour aller au premier item sélectionné */
	g_signal_connect (G_OBJECT (priv->treeview_payee),
					  "visibility-notify-event",
					  G_CALLBACK (etats_page_payee_show_first_row_selected),
					  priv);
}

/**
 * Création de la page de gestion des payee
 *
 * \param
 *
 * \return
 **/
static void etats_page_payee_setup_page (EtatsPagePayee *page,
										 GtkWidget *etats_prefs)
{
	GtkWidget *head_page;
	EtatsPagePayeePrivate *priv;

	devel_debug (NULL);
	priv = etats_page_payee_get_instance_private (page);

	/* set head page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Payees"), "gsb-payees-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_payee), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_payee), head_page, 0);

	/* on adapte le label pour Mac_OSX */
#ifdef OS_OSX
	gtk_label_set_text (GTK_LABEL (priv->label_search_help_payee), label_search_help);
	gtk_label_set_justify (GTK_LABEL (priv->label_search_help_payee), GTK_JUSTIFY_CENTER);
#endif /* OS_OSX */

	/* init tree_view */
	etats_page_payee_setup_treeview (page);

	/* on rend insensible les bouton premier et précédent car on est positionné sur le 1er item sélectionné */
	etats_page_payee_show_hide_prev_next_buttons (0, -1, priv);

	/* on rend les boutons premier, précédent, suivant et dernier actifs */
	g_object_set_data (G_OBJECT (priv->button_first_payee), "ptr_sens", GINT_TO_POINTER (GDK_LEFTBUTTON));
	g_signal_connect (G_OBJECT (priv->button_first_payee),
					  "button-press-event",
					  G_CALLBACK (etats_page_payee_select_first_last_item),
					  priv);

	g_object_set_data (G_OBJECT (priv->button_previous_payee), "ptr_sens", GINT_TO_POINTER (GDK_LEFTBUTTON));
	g_signal_connect (G_OBJECT (priv->button_previous_payee),
					  "button-press-event",
					  G_CALLBACK (etats_page_payee_select_prev_next_item),
					  priv);

	g_object_set_data (G_OBJECT (priv->button_next_payee), "ptr_sens", GINT_TO_POINTER (GDK_RIGHTBUTTON));
	g_signal_connect (G_OBJECT (priv->button_next_payee),
					  "button-press-event",
					  G_CALLBACK (etats_page_payee_select_prev_next_item),
					  priv);

	g_object_set_data (G_OBJECT (priv->button_last_payee), "ptr_sens",GINT_TO_POINTER (GDK_RIGHTBUTTON));
	g_signal_connect (G_OBJECT (priv->button_last_payee),
					  "button-press-event",
					  G_CALLBACK (etats_page_payee_select_first_last_item),
					  priv);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
	g_signal_connect (G_OBJECT (priv->button_detaille_payee),
					  "toggled",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
					  GINT_TO_POINTER (PAYEE_PAGE_TYPE));

	/* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
	g_signal_connect (G_OBJECT (priv->button_detaille_payee),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->vbox_detaille_payee);

	/* on met la connection pour (dé)sélectionner tous les tiers */
	g_signal_connect (G_OBJECT (priv->togglebutton_select_all_payee),
					  "toggled",
					  G_CALLBACK (utils_togglebutton_select_unselect_all_rows),
					  priv->treeview_payee);

	/* on met les fonctions d'insertion et de suppression de texte */
	g_signal_connect (G_OBJECT (priv->entry_search_payee),
					  "insert-text",
					  G_CALLBACK (etats_page_payee_entry_insert_text),
					  priv->treeview_payee);
	g_signal_connect (G_OBJECT (priv->entry_search_payee),
					  "delete-text",
					  G_CALLBACK (etats_page_payee_entry_delete_text),
					  priv->treeview_payee);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void etats_page_payee_init (EtatsPagePayee *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void etats_page_payee_dispose (GObject *object)
{
	G_OBJECT_CLASS (etats_page_payee_parent_class)->dispose (object);
}

static void etats_page_payee_class_init (EtatsPagePayeeClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = etats_page_payee_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/etats/etats_page_payee.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, vbox_etats_page_payee);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, button_detaille_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, button_first_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, button_last_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, button_next_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, button_previous_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, entry_search_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, label_search_help_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, togglebutton_select_all_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, treeview_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePayee, vbox_detaille_payee);
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
EtatsPagePayee *etats_page_payee_new (GtkWidget *etats_prefs)
{
	EtatsPagePayee *page;

	page = g_object_new (ETATS_PAGE_PAYEE_TYPE, NULL);
	etats_page_payee_setup_page (page, etats_prefs);

	return page;
}

/**
 *
 *
 * \param parent
 * \param report_number
 *
 * \return
 **/
void etats_page_payee_initialise_onglet (GtkWidget *etats_prefs,
										 gint report_number)
{
	gint active;
	EtatsPagePayee *page;
	EtatsPagePayeePrivate *priv;

	page = ETATS_PAGE_PAYEE (etats_prefs_get_page_by_number (etats_prefs, PAYEE_PAGE_TYPE));
	priv = etats_page_payee_get_instance_private (page);

	active = gsb_data_report_get_payee_detail_used (report_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_detaille_payee), active);
	gtk_widget_set_sensitive (priv->vbox_detaille_payee, active);

	if (active)
	{
		new_etats_prefs_tree_view_select_rows_from_list (gsb_data_report_get_payee_numbers_list (report_number),
														 priv->treeview_payee,
														 1);

		if (g_slist_length (gsb_data_report_get_payee_numbers_list (report_number)))
			utils_togglebutton_set_label_position_unselect (priv->togglebutton_select_all_payee,
															NULL,
															priv->treeview_payee);
	}
}

/**
 *
 *
 * \param parent
 * \param report_number
 *
 * \return
 **/
void etats_page_payee_get_info (GtkWidget *etats_prefs,
								gint report_number)
{
	gint active;
	EtatsPagePayee *page;
	EtatsPagePayeePrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_PAYEE (etats_prefs_get_page_by_number (etats_prefs, PAYEE_PAGE_TYPE));
	priv = etats_page_payee_get_instance_private (page);

	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->button_detaille_payee));
	gsb_data_report_set_payee_detail_used (report_number, active);

	if (active)
	{
		gsb_data_report_free_payee_numbers_list (report_number);

		/* set payee_selected = NULL */
		payee_selected = NULL;

		if (utils_tree_view_all_rows_are_selected (GTK_TREE_VIEW (priv->treeview_payee)))
		{
			gchar *text;
			gchar *hint;

			hint = g_strdup (_("Performance issue."));
			text = g_strdup (_("All payees have been selected.  Grisbi will run "
							   "faster without the \"Detail payees used\" option activated."));

			dialogue_hint (text, hint);
			etats_prefs_button_toggle_set_actif ("togglebutton_select_all_tiers", FALSE);
			gsb_data_report_set_payee_detail_used (report_number, FALSE);

			g_free (text);
			g_free (hint);
		}
		else
			gsb_data_report_set_payee_numbers_list (report_number,
													etats_page_payee_get_rows_selected (priv->treeview_payee));
	}
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

