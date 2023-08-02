/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdlib.h>

/*START_INCLUDE*/
#include "etats_prefs.h"
#include "dialog.h"
#include "etats_config.h"
#include "etats_page_amount.h"
#include "etats_page_accounts.h"
#include "etats_page_budget.h"
#include "etats_page_category.h"
#include "etats_page_payee.h"
#include "etats_page_period.h"
#include "etats_page_text.h"
#include "etats_page_transfer.h"
#include "grisbi_app.h"
#include "gsb_currency.h"
#include "gsb_data_payment.h"
#include "gsb_data_report.h"
#include "gsb_form_widget.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
#ifdef OS_OSX
static const gchar *label_search_help = N_("Command-click\nto add to the selection");
#endif /* OS_OSX */

/* necessaire pour le drag and drop dans la liste des tris */
static GtkWidget *treeview_data_grouping;

/* sert a l'actualisation du combo box payee du formulaire */
static gboolean payee_last_state;

/* select last_page */
static gint last_page_number;

/* liste des plages de date possibles */
static const gchar *jours_semaine[] =
{
	N_("Monday"),
	N_("Tuesday"),
	N_("Wednesday"),
	N_("Thursday"),
	N_("Friday"),
	N_("Saturday"),
	N_("Sunday"),
	NULL,
};

static const gchar *data_separation_periodes[] =
{
	N_("Day"),
	N_("Week"),
	N_("Month"),
	N_("Year"),
	NULL,
};

/* données de classement des opérations */
static const gchar *etats_prefs_classement_operations[] =
{
	N_("date"),
	N_("value date"),
	N_("transaction number"),
	N_("payee"),
	N_("category"),
	N_("budgetary line"),
	N_("note"),
	N_("method of payment"),
	N_("cheque/transfer number"),
	N_("voucher"),
	N_("bank reference"),
	N_("reconciliation reference"),
	NULL,
};
/*END_STATIC*/

/* Private structure type */
typedef struct _EtatsPrefsPrivate EtatsPrefsPrivate;

struct _EtatsPrefsPrivate
{
	GtkWidget *		vbox_prefs;

	GtkWidget *		hpaned;
	GtkWidget *		treeview_left_panel;
	GtkWidget *		notebook_etats_prefs;

	gboolean		form_date_force_prev_year;

	/*ONGLET_MODE_PAIEMENT*/
	GtkWidget *		onglet_etat_mode_paiement;
	GtkWidget *		bouton_detaille_mode_paiement_etat;
	GtkWidget *		treeview_mode_paiement;
	GtkWidget *		togglebutton_select_all_mode_paiement;
	GtkWidget *		label_modes_search_help;
	GtkWidget *		vbox_mode_paiement_etat;

	/*ONGLET_DIVERS*/
	GtkWidget *		onglet_etat_divers;
	GtkWidget *		radiobutton_marked;
	GtkWidget *		vbox_marked_buttons;
	GtkWidget *		radiobutton_marked_No_R;
	GtkWidget *		bouton_pas_detailler_ventilation;
	GtkWidget *		radiobutton_marked_all;
	GtkWidget *		checkbutton_marked_P;
	GtkWidget *		checkbutton_marked_R;
	GtkWidget *		checkbutton_marked_T;

	/*ONGLET_DATA_GROUPING*/
	GtkWidget *		onglet_data_grouping;
	GtkWidget *		sw_data_grouping;
	GtkWidget *		button_data_grouping_down;
	GtkWidget *		button_data_grouping_up;

	GtkWidget *		bouton_group_by_account;
	GtkWidget *		bouton_group_by_payee;
	GtkWidget *		bouton_group_by_categ;
	GtkWidget *		bouton_group_by_ib;

	/*ONGLET_DATA_SEPARATION*/
	GtkWidget *		onglet_data_separation;
	GtkWidget *		button_split_by_income_expenses;
	GtkWidget *		button_split_by_fyears;
	GtkWidget *		button_split_by_period;
	GtkWidget *		bouton_split_by_type_period;
	GtkWidget *		bouton_debut_semaine;
	GtkWidget *		paddingbox_data_by_period;

	/*ONGLET_AFFICHAGE_GENERALITES*/
	GtkWidget *		affichage_etat_generalites;
	GtkWidget *		entree_nom_etat;
	GtkWidget *		check_button_compl_name;
	GtkWidget *		hbox_combo_compl_name;
	GtkWidget *		combo_box_compl_name_function;
	GtkWidget *		combo_box_compl_name_position;
	GtkWidget *		textview_compl_name;
	GtkWidget *		button_sel_value_date;
	GtkWidget *		bouton_ignore_archives;
	GtkWidget *		bouton_afficher_nb_opes;
	GtkWidget *		bouton_inclure_dans_tiers;

/*ONGLET_AFFICHAGE_TITLES*/
	GtkWidget *		affichage_etat_titles;
	GtkWidget *		bouton_afficher_noms_comptes;
	GtkWidget *		bouton_affiche_sous_total_compte;
	GtkWidget *		bouton_afficher_noms_tiers;
	GtkWidget *		bouton_affiche_sous_total_tiers;
	GtkWidget *		bouton_afficher_noms_categ;
	GtkWidget *		bouton_affiche_sous_total_categ;
	GtkWidget *		bouton_afficher_sous_categ;
	GtkWidget *		bouton_affiche_sous_total_sous_categ;
	GtkWidget *		bouton_afficher_pas_de_sous_categ;
	GtkWidget *		bouton_afficher_noms_ib;
	GtkWidget *		bouton_affiche_sous_total_ib;
	GtkWidget *		bouton_afficher_sous_ib;
	GtkWidget *		bouton_affiche_sous_total_sous_ib;
	GtkWidget *		bouton_afficher_pas_de_sous_ib;

	/*ONGLET_AFFICHAGE_OPERATIONS*/
	GtkWidget *		affichage_etat_operations;
	GtkWidget *		bouton_afficher_opes;
	GtkWidget *		bouton_afficher_no_ope;
	GtkWidget *		bouton_afficher_date_opes;
	GtkWidget *		bouton_afficher_value_date_opes;
	GtkWidget *		bouton_afficher_tiers_opes;
	GtkWidget *		bouton_afficher_categ_opes;
	GtkWidget *		bouton_afficher_sous_categ_opes;
	GtkWidget *		bouton_afficher_ib_opes;
	GtkWidget *		bouton_afficher_sous_ib_opes;
	GtkWidget *		bouton_afficher_notes_opes;
	GtkWidget *		bouton_afficher_type_ope;
	GtkWidget *		bouton_afficher_no_cheque;
	GtkWidget *		bouton_afficher_pc_opes;
	GtkWidget *		bouton_afficher_exo_opes;
	GtkWidget *		bouton_afficher_infobd_opes;
	GtkWidget *		bouton_afficher_no_rappr;
	GtkWidget *		bouton_afficher_titres_colonnes;
	GtkWidget *		bouton_titre_changement;
	GtkWidget *		bouton_titre_en_haut;
	GtkWidget *		bouton_choix_classement_ope_etat;
	GtkWidget *		bouton_rendre_ope_clickables;
	GtkWidget *		vbox_show_transactions;

	/*ONGLET_AFFICHAGE_DEVISES*/
	GtkWidget *		affichage_etat_devises;
	GtkWidget *		combobox_devise_general;
	GtkWidget *		combobox_devise_payee;
	GtkWidget *		combobox_devise_categ;
	GtkWidget *		combobox_devise_ib;
	GtkWidget *		combobox_devise_amount;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPrefs, etats_prefs, GTK_TYPE_DIALOG)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/*RIGHT_PANEL : ONGLET_MODE_PAIEMENT*/
/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkTreeModel *etats_prefs_onglet_mode_paiement_get_model (void)
{
	GtkListStore *list_store;
	GSList *liste_nom_types = NULL;
	GSList *list_tmp;

	list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);

	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store), 0, GTK_SORT_ASCENDING);

	/* create a list of unique names */
	list_tmp = gsb_data_payment_get_payments_list ();

	while (list_tmp)
	{
		GtkTreeIter iter;
		gchar *name;
		gint payment_number;

		payment_number = gsb_data_payment_get_number (list_tmp->data);
		name = my_strdup (gsb_data_payment_get_name (payment_number));

		if (!g_slist_find_custom (liste_nom_types,
								  name,
								  (GCompareFunc) utils_str_search_str_in_string_list))
		{
			liste_nom_types = g_slist_append (liste_nom_types, name);
			gtk_list_store_append (list_store, &iter);
			gtk_list_store_set (list_store, &iter, 0, name, 1, payment_number, -1);
		}
		else
			g_free (name);

		list_tmp = list_tmp->next;
	}

	/* on libère la mémoire utilisée par liste_nom_types */
	g_slist_free_full (liste_nom_types, (GDestroyNotify) g_free);

	return GTK_TREE_MODEL (list_store);
}

/**
 * Sélectionne les iters en fonction des données de la liste
 *
 * \param liste des lignes à sélectionner
 * \param nom du tree_view concerné
 *
 * \return
 **/
static void etats_prefs_onglet_mode_paiement_select_rows_from_list (GSList *liste,
																	GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GSList *tmp_list;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		do
		{
			gchar *tmp_str;

			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &tmp_str, -1);

			tmp_list = liste;
			while (tmp_list)
			{
				gchar *str;

				str = tmp_list->data;

				if (strcmp (str, tmp_str) == 0)
					gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);

				tmp_list = tmp_list->next;
			}
			g_free (tmp_str);
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}
}

/**
 * initialise le tree_view avec son modèle et son type de sélection
 *
 * \param
 *
 * \return
 **/
static void etats_prefs_onglet_mode_paiement_init_tree_view (EtatsPrefs *prefs)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (prefs);

	/* on récupère le model par appel à function */
	model = etats_prefs_onglet_mode_paiement_get_model ();

	gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (priv->treeview_mode_paiement), TRUE);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_mode_paiement));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_mode_paiement), GTK_TREE_MODEL (model));
	g_object_unref (G_OBJECT (model));

	/* set the color of selected row */
	gtk_widget_set_name (priv->treeview_mode_paiement, "tree_view");

	/* set the column */
	cell = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_mode_paiement), GTK_TREE_VIEW_COLUMN (column));
	gtk_tree_view_column_set_resizable (column, TRUE);
}

/**
 * Création de l'onglet moyens de paiement
 *
 * \param
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_mode_paiement_create_page (EtatsPrefs *prefs,
																gint page)
{
	GtkWidget *vbox;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Payment methods"), "gsb-payment-32.png");

	gtk_box_pack_start (GTK_BOX (priv->onglet_etat_mode_paiement), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->onglet_etat_mode_paiement), vbox, 0);

	gtk_widget_set_sensitive (priv->vbox_mode_paiement_etat, FALSE);

	/* on adapte le label pour Mac_OSX */
#ifdef OS_OSX
	gtk_label_set_text (GTK_LABEL (priv->label_modes_search_help), _(label_search_help));
	gtk_label_set_justify (GTK_LABEL (priv->label_modes_search_help), GTK_JUSTIFY_CENTER);
#endif /* OS_OSX */

	/* on crée la liste des moyens de paiement */
	etats_prefs_onglet_mode_paiement_init_tree_view (prefs);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
	g_object_set_data (G_OBJECT (priv->bouton_detaille_mode_paiement_etat), "etats_prefs", prefs);
	g_signal_connect (G_OBJECT (priv->bouton_detaille_mode_paiement_etat),
					  "toggled",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
					  GINT_TO_POINTER (page));

	/* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
	g_signal_connect (G_OBJECT (priv->bouton_detaille_mode_paiement_etat),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->vbox_mode_paiement_etat);

	/* on met la connection pour (dé)sélectionner tous les tiers */
	g_signal_connect (G_OBJECT (priv->togglebutton_select_all_mode_paiement),
					  "toggled",
					  G_CALLBACK (utils_togglebutton_select_unselect_all_rows),
					  priv->treeview_mode_paiement);

	return priv->onglet_etat_mode_paiement;
}

/*RIGHT_PANEL : ONGLET_DIVERS*/
/**
 * If applicable, update report navigation tree style to reflect which
 * pages have been changed.
 *
 * \param page_number Page that contained an interface element just
 *					  changed that triggered this event.
 *
 * \return	  FALSE
 **/
static gboolean etats_prefs_onglet_divers_update_style_left_panel (GtkWidget *button,
																   gint *page_number)
{
	gint active;
	gint index;
	EtatsPrefs *prefs;
	EtatsPrefsPrivate *priv;

	prefs = g_object_get_data (G_OBJECT (button), "etats_prefs");
	priv = etats_prefs_get_instance_private (prefs);

	index = utils_radiobutton_get_active_index (priv->radiobutton_marked_all);
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_pas_detailler_ventilation));

	if (GTK_IS_RADIO_BUTTON (button))
	{
		if (active == 0)
			etats_prefs_left_panel_tree_view_update_style (button, page_number);
	}
	else
	{
		if (index == 0)
			etats_prefs_left_panel_tree_view_update_style (button, page_number);
	}

	return TRUE;
}

/**
 * Création de l'onglet Divers
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_divers_create_page (EtatsPrefs *prefs,
														 gint page)
{
	GtkWidget *vbox;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Miscellaneous"), "gsb-generalities-32.png");

	gtk_box_pack_start (GTK_BOX (priv->onglet_etat_divers), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->onglet_etat_divers), vbox, 0);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
	g_object_set_data (G_OBJECT (priv->radiobutton_marked), "etats_prefs", prefs);
	g_signal_connect (G_OBJECT (priv->radiobutton_marked),
					  "toggled",
					  G_CALLBACK (etats_prefs_onglet_divers_update_style_left_panel),
					  GINT_TO_POINTER (page));

	/* on met la connection pour rendre sensitif la vbox_marked_buttons */
	g_signal_connect (G_OBJECT (priv->radiobutton_marked),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->vbox_marked_buttons);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
	g_object_set_data (G_OBJECT (priv->radiobutton_marked_No_R), "etats_prefs", prefs);
	g_signal_connect (G_OBJECT (priv->radiobutton_marked_No_R),
					  "toggled",
					  G_CALLBACK (etats_prefs_onglet_divers_update_style_left_panel),
					  GINT_TO_POINTER (page));

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
	g_object_set_data (G_OBJECT (priv->bouton_pas_detailler_ventilation), "etats_prefs", prefs);
	g_signal_connect (G_OBJECT (priv->bouton_pas_detailler_ventilation),
					  "toggled",
					  G_CALLBACK (etats_prefs_onglet_divers_update_style_left_panel),
					  GINT_TO_POINTER (page));

	return priv->onglet_etat_divers;
}

/*RIGHT_PANEL : ONGLET_DATA_GROUPING*/
/**
 * return la liste des données par ordre de tri
 *
 * \param report_number
 *
 * \return a GSList
 * */
static GSList *etats_config_onglet_data_grouping_get_list (gint report_number)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSList *tmp_list = NULL;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_data_grouping));

	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		do
		{
			gint type_data;

			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 2, &type_data, -1);
			tmp_list = g_slist_append (tmp_list, GINT_TO_POINTER (type_data));

			/* on ajoute les sous catégories ici */
			if (type_data == 1)
			{
				tmp_list = g_slist_append (tmp_list, GINT_TO_POINTER (2));
			}
			/* et les sous imputations ici */
			else if (type_data == 3)
			{
				tmp_list = g_slist_append (tmp_list, GINT_TO_POINTER (4));
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

		return tmp_list;
	}

	return NULL;
}

/**
 * retourne une chaine de caractère formatée en fonction du type de donnée
 * et de la position dans la liste
 *
 * \param type_data	 type de donnée : 1 Categ, 3 IB, 5 Account, 6 Payee.
 * \param pos		   numéro de ligne dans le modèle
 *
 * \return NULL or a formatted string
 */
static gchar *etats_prefs_onglet_data_grouping_get_string (gint type_data,
														   gint pos)
{
	gchar *string = NULL;
	gchar *text = NULL;

	switch (type_data)
	{
		case 1:
		text = g_strdup (_("Category"));
		break;

		case 3:
		text = g_strdup (_("Budgetary line"));
		break;

		case 5:
		text = g_strdup (_("Account"));
		break;

		case 6:
		text = g_strdup (_("Payee"));
		break;
	}

	if (!text)
		return NULL;

	if (pos <= 0)
		string = text;
	else if (pos == 1)
	{
		string = g_strconcat ("\t", text, NULL);
		g_free (text);
	}
	else if (pos == 2)
	{
		string = g_strconcat ("\t\t", text, NULL);
		g_free (text);
	}
	else if (pos == 3)
	{
		string = g_strconcat ("\t\t\t", text, NULL);
		g_free (text);
	}

	return string;
}

/**
 * Update le modèle avec les données sauvegardées
 *
 * \param report_number
 *
 * \return TRUE
 */
static gboolean etats_prefs_onglet_data_grouping_update_model (GtkWidget *tree_view,
															   gint report_number)
{
	GtkTreeModel *model;
	GSList *tmp_list;
	gint i = 0;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	/* on reset le model */
	gtk_list_store_clear (GTK_LIST_STORE (model));

	/* on remplit la liste des données */
	tmp_list = gsb_data_report_get_sorting_type_list (report_number);

	while (tmp_list)
	{
		GtkTreeIter iter;
		gchar *string = NULL;
		gint type_data;

		type_data = GPOINTER_TO_INT (tmp_list->data);

		string = etats_prefs_onglet_data_grouping_get_string (type_data, i);
		if (!string)
		{
			tmp_list = tmp_list->next;
			continue;
		}

		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, string, 1, i, 2, type_data, -1);

		g_free (string);

		i++;
		tmp_list = tmp_list->next;
	}

	return TRUE;
}

/**
 * déplace un item suite à un drag and drop dans le tree_view
 *
 * \param src_pos		   position avant
 * \param src_type_data		type de donnée
 * \param dst_pos		   position après drag and drop
 *
 * \return
 * */
static void etats_prefs_onglet_data_grouping_move_in_list (gint src_pos,
														   gint src_type_data,
														   gint dst_pos)
{
	GtkTreeModel *model;
	GSList *tmp_list;
	gint report_number;
	gint i = 0;

	report_number = gsb_gui_navigation_get_current_report ();
	tmp_list = g_slist_copy (gsb_data_report_get_sorting_type_list (report_number));

	/* on supprime les sous categ et les sous IB */
	tmp_list = g_slist_remove (tmp_list, GINT_TO_POINTER (4));
	tmp_list = g_slist_remove (tmp_list, GINT_TO_POINTER (2));

	/* on supprime la donnée à la position initiale */
	tmp_list = g_slist_remove (tmp_list, GINT_TO_POINTER (src_type_data));

	/* on insère la donnée à la position finale */
	tmp_list = g_slist_insert (tmp_list, GINT_TO_POINTER (src_type_data), dst_pos);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_data_grouping));

	/* on reset le model */
	gtk_list_store_clear (GTK_LIST_STORE (model));

	while (tmp_list)
	{
		GtkTreeIter iter;
		gchar *string = NULL;
		gint type_data;
		gpointer ptr;

		ptr = tmp_list->data;
		type_data = GPOINTER_TO_INT (ptr);

		string = etats_prefs_onglet_data_grouping_get_string (type_data, i);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, string, 1, i, 2, type_data, -1);

		g_free (string);

		i++;
		tmp_list = tmp_list->next;
	}
}

/**
 * callback when treeview_data_grouping receive a drag and drop signal
 *
 * \param drag_dest
 * \param dest_path
 * \param selection_data
 *
 * \return FALSE
 **/
static gboolean etats_prefs_onglet_data_grouping_drag_data_received (GtkTreeDragDest *drag_dest,
																	 GtkTreePath *dest_path,
																	 GtkSelectionData *selection_data)
{
	if (dest_path && selection_data)
	{
		GtkTreeModel *model;
		GtkTreeIter src_iter;
		GtkTreeIter dest_iter;
		GtkTreePath *src_path;
		gint src_pos = 0;
		gint dest_pos = 0;
		gint src_type_data = 0;
		gint dest_type_data = 0;

		/* On récupère le model et le path d'origine */
		gtk_tree_get_row_drag_data (selection_data, &model, &src_path);

		/* On récupère les données des 2 lignes à modifier */
		if (gtk_tree_model_get_iter (model, &src_iter, src_path))
			gtk_tree_model_get (model, &src_iter, 1, &src_pos, 2, &src_type_data, -1);

		if (gtk_tree_model_get_iter (model, &dest_iter, dest_path))
			gtk_tree_model_get (model, &dest_iter, 1, &dest_pos, 2, &dest_type_data, -1);
		else
			return FALSE;

		/* on met à jour la liste des types pour le tri de données */
		etats_prefs_onglet_data_grouping_move_in_list (src_pos, src_type_data, dest_pos);

		return TRUE;
	}

	/* return */
	return FALSE;
}

/**
 * Fill the drag & drop structure with the path of selected column.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_source		Not used.
 * \param path				Original path for the gtk selection.
 * \param selection_data	A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 **/
static gboolean etats_prefs_onglet_data_grouping_drag_data_get (GtkTreeDragSource *drag_source,
																GtkTreePath *dest_path,
																GtkSelectionData *selection_data)
{
	if (dest_path && selection_data)
	{
		GtkTreeModel *model;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_data_grouping));
		gtk_tree_set_row_drag_data (selection_data, GTK_TREE_MODEL (model), dest_path);
	}

	return FALSE;
}

/**
 * Checks the validity of the change of position
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_dest		 Not used.
 * \param path			  Original path for the gtk selection.
 * \param selection_data	A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 **/
static gboolean etats_prefs_onglet_data_grouping_drop_possible (GtkTreeDragDest *drag_dest,
																GtkTreePath *dest_path,
																GtkSelectionData *selection_data)
{
	GtkTreePath *orig_path;
	GtkTreeModel *model;
	gint src_pos = 0;
	gint dst_pos = 0;
	GtkTreeIter iter;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

	if (gtk_tree_model_get_iter (model, &iter, orig_path))
		gtk_tree_model_get (model, &iter, 1, &src_pos, -1);

	if (gtk_tree_model_get_iter (model, &iter, dest_path))
		gtk_tree_model_get (model, &iter, 1, &dst_pos, -1);

	if (dst_pos < 0 || dst_pos > 3)
		return FALSE;

	if (src_pos != dst_pos)
		return TRUE;
	else
		return FALSE;
}

/**
 * fonction de callback de changement de sélection
 *
 * \param selection
 * \param NULL
 *
 * \return
 **/
static void etats_prefs_onglet_data_grouping_selection_changed (GtkTreeSelection *selection,
																EtatsPrefs *prefs)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gint pos;
		EtatsPrefsPrivate *priv;

		priv = etats_prefs_get_instance_private (prefs);

		gtk_tree_model_get (model, &iter, 1, &pos, -1);
		switch (pos)
		{
			case 0:
				desensitive_widget (NULL, priv->button_data_grouping_up);
				sensitive_widget (NULL,priv->button_data_grouping_down);
				break;
			case 3:
				sensitive_widget (NULL,priv->button_data_grouping_up);
				desensitive_widget (NULL,priv->button_data_grouping_down);
				break;
			default:
				sensitive_widget (NULL,priv->button_data_grouping_up);
				sensitive_widget (NULL,priv->button_data_grouping_down);
				break;
		}
	}
}

/**
 * crée un nouveau tree_view initialisé avec model.
 * le modèle comporte 3 colonnes : G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT
 * le tree_view n'affiche que la colonne texte.
 *
 * \return the tree_wiew
 **/
static gboolean etats_prefs_onglet_data_grouping_init_tree_view (EtatsPrefs *prefs)
{
	GtkWidget *tree_view;
	GtkWidget *window;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkTreeDragDestIface *dst_iface;
	GtkTreeDragSourceIface *src_iface;
	static GtkTargetEntry row_targets[] = {{(gchar*)"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0}};
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (prefs);

	window = GTK_WIDGET (grisbi_app_get_active_window (NULL));

	/* colonnes du list_store :
	 *  1 : chaine affichée
	 *  2 : numéro de ligne dans le modèle
	 *  3 : type de donnée : 1 Categ, 3 IB, 5 Account, 6 Payee.
	 */
	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);

	/* set tree_view */
	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_widget_set_name (tree_view, "tree_view");
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(tree_view), FALSE);
	gtk_widget_set_events (tree_view, GDK_SCROLL_MASK);
	gtk_container_add (GTK_CONTAINER(priv->sw_data_grouping), tree_view);

	/* set the column */
	cell = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
	gtk_tree_view_column_set_resizable (column, TRUE);

	/* Enable drag & drop */
	gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (tree_view),
											GDK_BUTTON1_MASK,
											row_targets,
											1,
											GDK_ACTION_MOVE);
	gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (tree_view), row_targets, 1, GDK_ACTION_MOVE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (tree_view), TRUE);

	dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (store);
	if (dst_iface)
	{
		dst_iface->drag_data_received = &etats_prefs_onglet_data_grouping_drag_data_received;
		dst_iface->row_drop_possible = &etats_prefs_onglet_data_grouping_drop_possible;
	}

	src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (store);
	if (src_iface)
	{
		gtk_selection_add_target (window, GDK_SELECTION_PRIMARY, GDK_SELECTION_TYPE_ATOM, 1);
		src_iface->drag_data_get = &etats_prefs_onglet_data_grouping_drag_data_get;
	}

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	g_signal_connect (G_OBJECT (selection),
					  "changed",
					  G_CALLBACK (etats_prefs_onglet_data_grouping_selection_changed),
					  prefs);

	g_object_unref (G_OBJECT (store));

	treeview_data_grouping = tree_view;

	return TRUE;
}

/**
 * callback when a button receive a clicked signal
 *
 * \param the button
 * \param a pointer for the direction of movement
 *
 * \return
 **/
static void etats_prefs_onglet_data_grouping_button_clicked (GtkWidget *button,
															 gpointer data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter orig_iter;

	/* On récupère le model et le path d'origine */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_data_grouping));

	if (gtk_tree_selection_get_selected (selection, &model, &orig_iter))
	{
		GtkTreeIter dest_iter;
		GtkTreePath *path;
		gchar *string = NULL;
		gint orig_pos = 0;
		gint dest_pos;
		gint orig_type_data;
		gint dest_type_data;
		gint sens;

		sens = GPOINTER_TO_INT (data);

		path = gtk_tree_model_get_path (model, &orig_iter);

		/* On récupère les données des 2 lignes à modifier */
		gtk_tree_model_get (model, &orig_iter, 1, &orig_pos, 2, &orig_type_data, -1);

		if (sens == GTK_DIR_UP)
			gtk_tree_path_prev (path);
		else
			gtk_tree_path_next (path);

		if (gtk_tree_model_get_iter (model, &dest_iter, path))
			gtk_tree_model_get (model, &dest_iter, 1, &dest_pos, 2, &dest_type_data, -1);
		else
			return;
		/* on met à jour la ligne de destination */
		string = etats_prefs_onglet_data_grouping_get_string (orig_type_data, dest_pos);
		gtk_list_store_set (GTK_LIST_STORE (model), &dest_iter, 0, string, 2, orig_type_data, -1);

		g_free (string);

		/* on met à jour la ligne d'origine */
		string = etats_prefs_onglet_data_grouping_get_string (dest_type_data, orig_pos);
		gtk_list_store_set (GTK_LIST_STORE (model), &orig_iter, 0, string, 2, dest_type_data, -1);

		/* on garde la sélection sur le même iter */
		gtk_tree_selection_select_path (selection, path);

		g_free (string);
	}
}

/**
 * Création de l'onglet groupement des donnés
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_data_grouping_create_page (EtatsPrefs *prefs,
 																gint page)
{
	GtkWidget *vbox;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Data grouping"), "gsb-organization-32.png");

	gtk_box_pack_start (GTK_BOX (priv->onglet_data_grouping), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->onglet_data_grouping), vbox, 0);

	etats_prefs_onglet_data_grouping_init_tree_view (prefs);

	/* on met la connection pour modifier l'ordre des données dans le tree_view data_grouping */
	g_signal_connect (G_OBJECT (priv->button_data_grouping_up),
					  "clicked",
					  G_CALLBACK (etats_prefs_onglet_data_grouping_button_clicked),
					  GINT_TO_POINTER (GTK_DIR_UP));

	g_signal_connect (G_OBJECT (priv->button_data_grouping_down),
					  "clicked",
					  G_CALLBACK (etats_prefs_onglet_data_grouping_button_clicked),
					  GINT_TO_POINTER (GTK_DIR_DOWN));

	return priv->onglet_data_grouping;
}

/*RIGHT_PANEL : ONGLET_DATA_SEPARATION*/
/**
 * fonction de callback appellée quand on change le type de période
 *
 * \param combo			le GtkComboBox qui change
 * \param widget		le widget qu'on rend sensible ou pas.
 *
 * \return
 */
static void etats_prefs_onglet_data_separation_combo_changed (GtkComboBox *combo,
															  EtatsPrefsPrivate *priv)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->button_split_by_period)))
	{
		if (gtk_combo_box_get_active (GTK_COMBO_BOX (combo)) == 1)
			gtk_widget_set_sensitive (priv->bouton_debut_semaine, TRUE);
		else
			gtk_widget_set_sensitive (priv->bouton_debut_semaine, FALSE);
	}
}

/**
 * Création de l'onglet séparation des donnés
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_data_separation_create_page (EtatsPrefs *prefs,
																  gint page)
{
	GtkWidget *vbox;
	GtkWidget *radio_button_utilise_exo;
	GtkTreeModel *model;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Data separation"), "gsb-organization-32.png");

	gtk_box_pack_start (GTK_BOX (priv->onglet_data_separation), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->onglet_data_separation), vbox, 0);

	/* on met la connexion pour la séparation par exercice avec le bouton radio_button_utilise_exo */
	radio_button_utilise_exo = etats_page_period_get_radio_button_utilise_exo (GTK_WIDGET (prefs));
	g_signal_connect (G_OBJECT (radio_button_utilise_exo),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->button_split_by_fyears);

	/* on met la connexion pour rendre sensible la boite avec le bouton bouton_split_by_type_period */
	g_signal_connect (G_OBJECT (priv->button_split_by_period),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->paddingbox_data_by_period);

	/* on crée le bouton avec les pérodes pour la séparation de l'état */
	model = GTK_TREE_MODEL (utils_list_store_create_from_string_array (data_separation_periodes));
	gtk_combo_box_set_model (GTK_COMBO_BOX (priv->bouton_split_by_type_period), model);
	utils_gtk_combo_box_set_text_renderer (GTK_COMBO_BOX (priv->bouton_split_by_type_period), 0);

	model = GTK_TREE_MODEL (utils_list_store_create_from_string_array (jours_semaine));
	gtk_combo_box_set_model (GTK_COMBO_BOX (priv->bouton_debut_semaine), model);
	utils_gtk_combo_box_set_text_renderer (GTK_COMBO_BOX (priv->bouton_debut_semaine), 0);

	/* on connecte le signal "changed" au bouton bouton_split_by_type_period
	 * pour rendre insensible le choix du jour de la semaine pour les choix
	 * autres que la semaine. On le met ici pour que l'initialisation se fasse
	 * proprement */
	g_signal_connect (G_OBJECT (priv->bouton_split_by_type_period),
					  "changed",
					  G_CALLBACK (etats_prefs_onglet_data_separation_combo_changed),
					  priv);

	return priv->onglet_data_separation;
}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_GENERALITES*/
/**
 * pré visualise le titre avec son complément
 *
 * \param	report_number
 * \param	priv
 *
 * \return
 **/
static void etats_prefs_display_name_with_complement (gint report_number,
													  EtatsPrefsPrivate *priv)
{
	gchar **tab;
	gchar *report_name = NULL;
	gchar *compl_str;
	gchar *tmp_str = NULL;
	gint function;
	gint position;

	report_name = gsb_data_report_get_report_name (report_number);
	function = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->combo_box_compl_name_function));
	position = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->combo_box_compl_name_position));
	tab = gsb_date_get_date_time_now_local ();

	if (function == 1)
	{
		if (position == 2)
		{
			compl_str = g_strconcat (tab[0], " ", _("at"), " ", tab[1], NULL);
		}
		else
		{
			compl_str = g_strconcat (tab[0], " ", tab[1], NULL);
		}
	}
	else
	{
		compl_str = g_strdup (tab[0]);
	}

	switch (position)
	{
		case 1:
			tmp_str = g_strconcat (report_name, " - ", compl_str, NULL);
			break;
		case 2:
			tmp_str = g_strconcat (report_name, "\n(", _("Edited"), " ", compl_str, ")", NULL);
			break;
		default:
			tmp_str = g_strconcat (compl_str, " - ", report_name, NULL);
	}
	g_free (compl_str);
	g_strfreev (tab);

	if (tmp_str)
	{
		GtkTextBuffer *buffer;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview_compl_name));
		gtk_text_buffer_set_text (buffer, tmp_str, -1);
		g_free (tmp_str);
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean etats_prefs_check_button_compl_name_toggled (GtkWidget *check_button,
															 EtatsPrefsPrivate *priv)
{
	gboolean activ;

	activ = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button));
	gtk_widget_set_sensitive (priv->hbox_combo_compl_name, activ);

	if (activ)
		etats_prefs_display_name_with_complement (gsb_gui_navigation_get_current_report (), priv);
	else
	{
		GtkTextBuffer *buffer;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview_compl_name));
		gtk_text_buffer_set_text (buffer, "", -1);
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
static void etats_prefs_combo_box_compl_data_changed (GtkComboBox *combo,
													   EtatsPrefsPrivate *priv)
{
	gint report_number;

	report_number = gsb_gui_navigation_get_current_report ();
	etats_prefs_display_name_with_complement (report_number, priv);
}

/**
 * Création de l'onglet affichage des generalites
 *
 * \param
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_generalites_create_page (EtatsPrefs *prefs,
																		gint page)
{
	GtkWidget *vbox;

	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Generalities"), "gsb-generalities-32.png");

	gtk_box_pack_start (GTK_BOX (priv->affichage_etat_generalites), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->affichage_etat_generalites), vbox, 0);

	/* on met une frama autour de entry_search_payee (pour gtk4) */
	gtk_entry_set_has_frame (GTK_ENTRY (priv->entree_nom_etat), TRUE);

	return priv->affichage_etat_generalites;
}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_TITLES*/
/**
 * Création de l'onglet affichage de diverses données
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_titles_create_page (EtatsPrefs *prefs,
																   gint page)
{
	GtkWidget *vbox;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Titles"), "gsb-title-32.png");

	gtk_box_pack_start (GTK_BOX (priv->affichage_etat_titles), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->affichage_etat_titles), vbox, 0);

	/* on met les connexions pour sensibiliser désensibiliser les données associées */
	g_signal_connect (G_OBJECT (priv->bouton_group_by_account),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_affiche_sous_total_compte);

	/* affichage possible des tiers */
	g_signal_connect (G_OBJECT (priv->bouton_group_by_payee),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_noms_tiers);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_payee),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_affiche_sous_total_tiers);

	/* affichage possible des categories */
	g_signal_connect (G_OBJECT (priv->bouton_group_by_categ),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_noms_categ);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_categ),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_affiche_sous_total_categ);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_categ),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_sous_categ);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_categ),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_affiche_sous_total_sous_categ);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_categ),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_pas_de_sous_categ);

	/* affichage possible des ib */
	g_signal_connect (G_OBJECT (priv->bouton_group_by_ib),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_noms_ib);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_ib),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_affiche_sous_total_ib);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_ib),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_sous_ib);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_ib),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_affiche_sous_total_sous_ib);

	g_signal_connect (G_OBJECT (priv->bouton_group_by_ib),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_pas_de_sous_ib);

	return priv->affichage_etat_titles;

}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_OPERATIONS*/
/**
 * Création de l'onglet affichage de opérations
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_operations_create_page (EtatsPrefs *prefs,
																	   gint page)
{
	GtkWidget *vbox;
	GtkTreeModel *model;

	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Transactions display"), "gsb-transdisplay-32.png");

	gtk_box_pack_start (GTK_BOX (priv->affichage_etat_operations), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->affichage_etat_operations), vbox, 0);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
	g_object_set_data (G_OBJECT (priv->bouton_afficher_opes), "etats_prefs", prefs);
	g_signal_connect (G_OBJECT (priv->bouton_afficher_opes),
					  "toggled",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
					  GINT_TO_POINTER (page));

	/* on met la connection pour rendre sensitif la vbox_show_transactions */
	g_signal_connect (G_OBJECT (priv->bouton_afficher_opes),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->vbox_show_transactions);

	/* on crée le bouton avec les types de classement des opérations */
	model = GTK_TREE_MODEL (utils_list_store_create_from_string_array (etats_prefs_classement_operations));
	gtk_combo_box_set_model (GTK_COMBO_BOX (priv->bouton_choix_classement_ope_etat), model);
	utils_gtk_combo_box_set_text_renderer (GTK_COMBO_BOX (priv->bouton_choix_classement_ope_etat), 0);

	/* on met les connexions */
	g_signal_connect (G_OBJECT (priv->bouton_afficher_categ_opes),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_sous_categ_opes);

	g_signal_connect (G_OBJECT (priv->bouton_afficher_ib_opes),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_afficher_sous_ib_opes);

	g_signal_connect (G_OBJECT (priv->bouton_afficher_titres_colonnes),
					  "toggled",
 					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_titre_changement);

	g_signal_connect (G_OBJECT (priv->bouton_afficher_titres_colonnes),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->bouton_titre_en_haut);

	return priv->affichage_etat_operations;

}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_DEVISES*/
/**
 * ajoute les combobox pour les devises
 *
 * \param
 *
 * \return
 */
static void etats_prefs_onglet_affichage_devises_make_combobox (EtatsPrefsPrivate *priv)
{
	gsb_currency_make_combobox_from_ui (priv->combobox_devise_general, FALSE);

	gsb_currency_make_combobox_from_ui (priv->combobox_devise_payee, FALSE);

	gsb_currency_make_combobox_from_ui (priv->combobox_devise_categ, FALSE);

	gsb_currency_make_combobox_from_ui (priv->combobox_devise_ib, FALSE);

	gsb_currency_make_combobox_from_ui (priv->combobox_devise_amount, FALSE);
}

/**
 * Création de l'onglet affichage des devises
 *
 * \param
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_devises_create_page (EtatsPrefs *prefs,
																	gint page)
{
	GtkWidget *vbox;

	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (prefs);

	vbox = new_vbox_with_title_and_icon (_("Totals currencies"), "gsb-currencies-32.png");

	gtk_box_pack_start (GTK_BOX (priv->affichage_etat_devises), vbox, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->affichage_etat_devises), vbox, 0);

	etats_prefs_onglet_affichage_devises_make_combobox (priv);

	return priv->affichage_etat_devises;

}

/*LEFT_PANEL*/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean etats_prefs_left_panel_tree_view_selection_changed (GtkTreeSelection *selection,
																	EtatsPrefs *prefs)
{
	GtkWidget *notebook;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint selected;
	EtatsPrefsPrivate *priv;

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		return (FALSE);
	}

	/* on recuper le notebook */
	priv = etats_prefs_get_instance_private (prefs);
	notebook = priv->notebook_etats_prefs;

	gtk_tree_model_get (model, &iter, 1, &selected, -1);

	gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), selected);
	if (selected == PAYEE_PAGE_TYPE)
	{
		/* affiche le premier tiers concerné */
		etats_page_payee_show_first_row_selected (GTK_WIDGET (prefs));
	}

	return FALSE;
}

/**
 * selectionne la dernière page sélectionnée
 *
 * \param
 *
 * \return
 **/
gboolean etats_prefs_left_panel_tree_view_select_last_page (GtkWidget *prefs)
{
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (ETATS_PREFS (prefs));

	utils_prefs_left_panel_tree_view_select_page (priv->treeview_left_panel, priv->notebook_etats_prefs, last_page_number);

	return FALSE;
}

static void etats_prefs_left_panel_notebook_change_page (GtkNotebook *notebook,
														 gpointer npage,
														 gint page,
														 gpointer user_data)
{
	last_page_number = page;
}

/**
 * remplit le model pour la configuration des états
 *
 * \param
 * \param
 *
 * \return
 **/
static void etats_prefs_left_panel_populate_tree_model (GtkTreeStore *tree_model,
														EtatsPrefs *prefs)
{
	GtkWidget *notebook;
	GtkWidget *widget = NULL;
	gint page = 0;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (prefs);
	notebook = priv->notebook_etats_prefs;

	/* append group page */
	utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Data selection"), -1);

	/* append page Dates */
	widget = GTK_WIDGET (etats_page_period_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Dates"), DATE_PAGE_TYPE);
	page++;

	/* append page Transferts */
	widget = GTK_WIDGET (etats_page_transfer_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Transfers"), TRANSFER_PAGE_TYPE);
	page++;

	/* append page Accounts */
	widget = GTK_WIDGET (etats_page_accounts_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Accounts"), ACCOUNT_PAGE_TYPE);
	page++;

	/* append page Payee */
	widget = GTK_WIDGET (etats_page_payee_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Payee"), PAYEE_PAGE_TYPE);
	page++;

	/* append page Categories */
	widget = GTK_WIDGET (etats_page_category_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Categories"), CATEGORY_PAGE_TYPE);
	page++;

	/* append page Budgetary lines */
	widget = GTK_WIDGET (etats_page_budget_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Budgetary lines"), BUDGET_PAGE_TYPE);
	page++;

	/* append page Text */
	widget = GTK_WIDGET (etats_page_text_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Texts"), TEXT_PAGE_TYPE);
	page++;

	/* append page Amounts */
	widget = GTK_WIDGET (etats_page_amount_new (GTK_WIDGET (prefs)));
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Amounts"), AMOUNT_PAGE_TYPE);
	page++;

	/* append page Payment methods */
	widget = etats_prefs_onglet_mode_paiement_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Payment methods"), PAYEMENT_PAGE_TYPE);
	page++;

	/* append page Misc. */
	widget = etats_prefs_onglet_divers_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Miscellaneous"), MISC_PAGE_TYPE);
	page++;

	/* remplissage de l'onglet d'organisation */
	utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Data organization"), -1);

	/* Data grouping */
	widget = etats_prefs_onglet_data_grouping_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Data grouping"), DATA_GROUPING_TYPE);
	page++;

	/* Data separation */
	widget = etats_prefs_onglet_data_separation_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Data separation"), DATA_SEPARATION_TYPE);
	page++;

	/* remplissage de l'onglet d'affichage */
	utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Data display"), -1);

	/* append page Generalities */
	widget = etats_prefs_onglet_affichage_generalites_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Generalities"), AFFICHAGE_GENERALITES_TYPE);
	page++;

	/* append page titles */
	widget = etats_prefs_onglet_affichage_titles_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Titles"), AFFICHAGE_TITLES_TYPE);
	page++;

	/* append page Transactions */
	widget = etats_prefs_onglet_affichage_operations_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Transactions"), AFFICHAGE_OPERATIONS_TYPE);
	page++;

	/* append page Currencies */
	widget = etats_prefs_onglet_affichage_devises_create_page (prefs, page);
	utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Currencies"), AFFICHAGE_DEVISES_TYPE);
}

/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 * \param
 *
 *\return tree_view or NULL;
 **/
static GtkWidget *etats_prefs_left_panel_init_tree_view (EtatsPrefs *prefs)
{
	GtkTreeStore *model = NULL;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkTreeSelection *selection;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (prefs);

	/* Création du model */
	model = gtk_tree_store_new (LEFT_PANEL_TREE_NUM_COLUMNS,
								G_TYPE_STRING,		/* LEFT_PANEL_TREE_TEXT_COLUMN */
								G_TYPE_INT,			/* LEFT_PANEL_TREE_PAGE_COLUMN */
								G_TYPE_INT,			/* LEFT_PANEL_TREE_BOLD_COLUMN */
								G_TYPE_INT);		/* LEFT_PANEL_TREE_ITALIC_COLUMN */

	/* Create container + TreeView */
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (priv->treeview_left_panel), FALSE);
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_left_panel), GTK_TREE_MODEL (model));
	g_object_unref (G_OBJECT (model));

	/* set the color of selected row */
	gtk_widget_set_name (priv->treeview_left_panel, "tree_view");

	/* make column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Categories",
														cell,
														"text", LEFT_PANEL_TREE_TEXT_COLUMN,
														"weight", LEFT_PANEL_TREE_BOLD_COLUMN,
														"style", LEFT_PANEL_TREE_ITALIC_COLUMN,
														NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_left_panel), GTK_TREE_VIEW_COLUMN (column));

	/* Handle select */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_left_panel));
	g_signal_connect (selection,
					  "changed",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_selection_changed),
					  prefs);

	/* Choose which entries will be selectable */
	gtk_tree_selection_set_select_function (selection,
											utils_prefs_left_panel_tree_view_selectable_func,
											NULL,
											NULL);

	/* expand all rows after the treeview widget has been realized */
	g_signal_connect (priv->treeview_left_panel,
					  "realize",
					  G_CALLBACK (utils_tree_view_set_expand_all_and_select_path_realize),
					  (gpointer) "0:0");

	/* remplissage du paned gauche */
	etats_prefs_left_panel_populate_tree_model (model, prefs);

	return priv->treeview_left_panel;
}

/**
 * Création de la page des preferences de l'état
 *
 * \param prefs
 * \param grisbi win
 *
 * \return
 **/
static void etats_prefs_setup_page (EtatsPrefs *prefs,
									 GtkWidget *win)
{
	GrisbiWinEtat *w_etat;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (prefs);

	/* memorise la variable w_etat->form_date_force_prev_year */
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	if (w_etat->form_date_force_prev_year)
	{
		priv->form_date_force_prev_year = w_etat->form_date_force_prev_year;
		w_etat->form_date_force_prev_year = FALSE;
	}

	/* initialisation du gtk_tree_view. */
	etats_prefs_left_panel_init_tree_view (prefs);

	/* on met la connexion pour mémoriser la dernière page utilisée */
	g_signal_connect_after (priv->notebook_etats_prefs,
							"switch-page",
							G_CALLBACK (etats_prefs_left_panel_notebook_change_page),
							NULL);

	gtk_widget_show_all (priv->vbox_prefs);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 * Initialise EtatsPrefs
 *
 * \param prefs
 *
 * \return
 **/
static void etats_prefs_init (EtatsPrefs *prefs)
{
	gtk_widget_init_template (GTK_WIDGET (prefs));
}

/**
 *  called when destroy EtatsPrefs
 *
 * \param object
 *
 * \return
 **/
static void etats_prefs_dispose (GObject *object)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	/* restauration de la variable w_etat->form_date_force_prev_year */
	priv = etats_prefs_get_instance_private (ETATS_PREFS (object));
	if (priv->form_date_force_prev_year)
	{
		GrisbiWinEtat *w_etat;

		w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
		w_etat->form_date_force_prev_year = TRUE;
	}

	/* libération de l'objet prefs */
	G_OBJECT_CLASS (etats_prefs_parent_class)->dispose (object);
}


/**
 * finalise EtatsPrefs
 *
 * \param object
 *
 * \return
 **/
static void etats_prefs_finalize (GObject *object)
{
	devel_debug (NULL);

	/* libération de l'objet prefs */
	G_OBJECT_CLASS (etats_prefs_parent_class)->finalize (object);
}


/**
 * Initialise EtatsPrefsClass
 *
 * \param
 *
 * \return
 **/
static void etats_prefs_class_init (EtatsPrefsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = etats_prefs_dispose;
	object_class->finalize = etats_prefs_finalize;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/etats_prefs.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, vbox_prefs);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, hpaned);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, treeview_left_panel);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, notebook_etats_prefs);

	/*ONGLET_MODE_PAIEMENT*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, onglet_etat_mode_paiement);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_detaille_mode_paiement_etat);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, treeview_mode_paiement);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, togglebutton_select_all_mode_paiement);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, label_modes_search_help);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, vbox_mode_paiement_etat);

	/*ONGLET_DIVERS*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, onglet_etat_divers);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, radiobutton_marked);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, vbox_marked_buttons);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, radiobutton_marked_No_R);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_pas_detailler_ventilation);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, radiobutton_marked_all);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, checkbutton_marked_P);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, checkbutton_marked_R);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, checkbutton_marked_T);

	/*ONGLET_DATA_GROUPING*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, onglet_data_grouping);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, sw_data_grouping);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, button_data_grouping_down);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, button_data_grouping_up);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_group_by_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_group_by_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_group_by_categ);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_group_by_ib);

	/*ONGLET_DATA_SEPARATION*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, onglet_data_separation);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, button_split_by_income_expenses);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, button_split_by_fyears);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, button_split_by_period);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_split_by_type_period);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_debut_semaine);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, paddingbox_data_by_period);

	/*ONGLET_AFFICHAGE_GENERALITES*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, affichage_etat_generalites);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, entree_nom_etat);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, check_button_compl_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, hbox_combo_compl_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, combo_box_compl_name_function);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, combo_box_compl_name_position);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, textview_compl_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, button_sel_value_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_ignore_archives);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_nb_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_inclure_dans_tiers);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, affichage_etat_titles);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, affichage_etat_operations);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, affichage_etat_devises);

	/*ONGLET_AFFICHAGE_TITLES*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_noms_comptes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_affiche_sous_total_compte);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_noms_tiers);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_affiche_sous_total_tiers);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_noms_categ);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_affiche_sous_total_categ);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_sous_categ);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_affiche_sous_total_sous_categ);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_pas_de_sous_categ);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_noms_ib);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_affiche_sous_total_ib);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_sous_ib);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_affiche_sous_total_sous_ib);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_pas_de_sous_ib);

	/*ONGLET_AFFICHAGE_OPERATIONS*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, affichage_etat_operations);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_no_ope);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_date_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_value_date_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_tiers_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_categ_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_sous_categ_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_ib_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_sous_ib_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_notes_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_type_ope);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_no_cheque);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_pc_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_exo_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_infobd_opes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_no_rappr);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_afficher_titres_colonnes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_titre_changement);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_titre_en_haut);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_choix_classement_ope_etat);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, bouton_rendre_ope_clickables);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, vbox_show_transactions);

	/*ONGLET_AFFICHAGE_DEVISES*/
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, combobox_devise_general);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, combobox_devise_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, combobox_devise_categ);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, combobox_devise_ib);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPrefs, combobox_devise_amount);
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
EtatsPrefs *etats_prefs_new (GtkWidget *win)
{
	EtatsPrefs *prefs = NULL;

	prefs = g_object_new (ETATS_PREFS_TYPE, "transient-for", win, NULL);
	etats_prefs_setup_page (prefs, win);

	return prefs;
}

/*FONCTIONS UTILITAIRES COMMUNES*/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GtkWidget *etats_prefs_get_page_by_number (GtkWidget *etats_prefs,
										   gint num_page)
{
	GtkWidget *page = NULL;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));
	page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook_etats_prefs), num_page);

	return page;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GtkWidget *etats_prefs_get_widget_by_name (const gchar *name,
										   GtkWidget *etats_prefs)
{
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	if (strcmp (name, "notebook_etats_prefs") == 0)
		return priv->notebook_etats_prefs;
	else if (strcmp (name, "treeview_left_panel") == 0)
		return priv->treeview_left_panel;
	else
		return NULL;
}

/**
 * If applicable, update report navigation tree style to reflect which
 * pages have been changed.
 *
 * \param page_number	Page that contained an interface element just
 *						changed that triggered this event.
 *
 * \return	  FALSE
 **/
gboolean etats_prefs_left_panel_tree_view_update_style (GtkWidget *button,
														gint *page_number)
{
	gint iter_page_number;

	iter_page_number = GPOINTER_TO_INT (page_number);
devel_debug_int(iter_page_number);
	if (iter_page_number)
	{
		EtatsPrefs *prefs;
		GtkTreeModel *model;
		GtkTreeIter parent_iter;
		gint active;
		gboolean italic = 0;
		EtatsPrefsPrivate *priv;


		prefs = g_object_get_data (G_OBJECT (button), "etats_prefs");
		priv = etats_prefs_get_instance_private (ETATS_PREFS (prefs));

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_left_panel));
		active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
		italic = active;

		if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
			return FALSE;

		do
		{
			GtkTreeIter iter;

			if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter, &parent_iter))
			{
				do
				{
					gint page;

					gtk_tree_model_get (GTK_TREE_MODEL (model),
										&iter,
										LEFT_PANEL_TREE_PAGE_COLUMN,
										&page,
										-1);

					if (page == iter_page_number)
						gtk_tree_store_set (GTK_TREE_STORE (model),
											&iter,
											LEFT_PANEL_TREE_ITALIC_COLUMN,
											italic,
											-1);
				}
				while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));

		return TRUE;
	}

	return FALSE;
}

/**
 * récupère l'index l'iter selectionné
 *
 * \param nom du tree_view
 *
 * \return numéro de la ligne sélectionnée
 **/
gint etats_prefs_tree_view_get_single_row_selected (GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gint index;

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);

		return index;
	}

	return -1;
}

/**
 * Sélectionne les iters en fonction des données de la liste
 *
 * \param liste des lignes à sélectionner
 * \param nom du tree_view concerné
 * \param numéro de la colonne contenant la donnée testée
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void etats_prefs_tree_view_select_rows_from_list (GSList *liste,
												  GtkWidget *tree_view,
												  gint column)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GSList *tmp_list;

	if (!liste)
		return;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		do
		{
			gint tmp_number;

			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, column, &tmp_number, -1);

			tmp_list = liste;
			while (tmp_list)
			{
				gint result;

				result = GPOINTER_TO_INT (tmp_list->data);

				if (result == tmp_number)
					gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);

				tmp_list = tmp_list->next;
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void etats_prefs_tree_view_select_single_row (GtkWidget *tree_view,
											  gint numero)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		do
		{
			gint index;

			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);

			if (numero == index)
			{
				gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);
				break;
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}
}

GSList *etats_prefs_tree_view_get_list_rows_selected (GtkWidget *tree_view)
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

/*ONGLET_MODE_PAIEMENT*/
/**
 * Initialise les informations de l'onglet modes de paiement
 *
 * \param etats_prefs
 * \param report_number
 *
 * \return
 **/
void etats_prefs_initialise_onglet_mode_paiement (GtkWidget *etats_prefs,
												  gint report_number)
{
	gint active;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	active = gsb_data_report_get_method_of_payment_used (report_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_mode_paiement_etat), active);

	if (active)
	{
		etats_prefs_onglet_mode_paiement_select_rows_from_list (gsb_data_report_get_method_of_payment_list
																(report_number),
																priv->treeview_mode_paiement);

		if (g_slist_length (gsb_data_report_get_method_of_payment_list (report_number)))
			utils_togglebutton_set_label_position_unselect (priv->togglebutton_select_all_mode_paiement,
															NULL,
															priv->treeview_mode_paiement);
	}
}

/**
 * Récupère les informations de l'onglet mode de paiement
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 **/
void etats_prefs_recupere_info_onglet_mode_paiement (GtkWidget *etats_prefs,
													 gint report_number)
{
	gint active;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_mode_paiement_etat));
	gsb_data_report_set_method_of_payment_used (report_number, active);
	if (active)
	{
		gsb_data_report_free_method_of_payment_list (report_number);

		if (utils_tree_view_all_rows_are_selected (GTK_TREE_VIEW (priv->treeview_mode_paiement)))
		{
			gchar *text;
			gchar *hint;

			hint = g_strdup (_("Performance issue."));
			text = g_strdup (_("All methods of payment have been selected.  Grisbi will run "
							   "faster without the \"Detail methods of payment used\" option activated."));

			dialogue_hint (text, hint);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->togglebutton_select_all_mode_paiement),
										  FALSE);
			gsb_data_report_set_method_of_payment_used (report_number, 0);

			g_free (text);
			g_free (hint);
		}
		else
			gsb_data_report_set_method_of_payment_list (report_number,
														etats_prefs_onglet_mode_paiement_get_list_rows_selected
														(priv->treeview_mode_paiement));

	}
}

/**
 * récupère la liste des libellés des item sélectionnés
 *
 * \param nom du tree_view
 *
 * \return numéro de la ligne sélectionnée
 **/
GSList *etats_prefs_onglet_mode_paiement_get_list_rows_selected (GtkWidget *tree_view)
{
	;
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
		gchar *tmp_str;

		path = rows_list->data;

		gtk_tree_model_get_iter (model, &iter, path) ;
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &tmp_str, -1);

		tmp_list = g_slist_append (tmp_list, tmp_str);

		gtk_tree_path_free (path);
		rows_list = rows_list->next;
	}
	g_list_free (rows_list);

	return tmp_list;
}

/*ONGLET_DIVERS*/
/**
 * Initialise les informations de l'onglet divers
 *
 * \param report_number
 *
 * \return
 */
void etats_prefs_initialise_onglet_divers (GtkWidget *etats_prefs,
										   gint report_number)
{
	gint index;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	index = gsb_data_report_get_show_m (report_number);
	utils_radiobutton_set_active_index (priv->radiobutton_marked_all, index);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_marked_P),
								  gsb_data_report_get_show_p (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_marked_R),
								  gsb_data_report_get_show_r (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_marked_T),
								  gsb_data_report_get_show_t (report_number));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_pas_detailler_ventilation),
								  gsb_data_report_get_not_detail_split (report_number));
}

/**
 * Récupère les informations de l'onglet divers
 *
 * \param
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void etats_prefs_recupere_info_onglet_divers (GtkWidget *etats_prefs,
											  gint report_number)
{
	gint index;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	index = utils_radiobutton_get_active_index (priv->radiobutton_marked_all);
	gsb_data_report_set_show_m (report_number, index);

	gsb_data_report_set_show_p (report_number,
								gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_marked_P)));
	gsb_data_report_set_show_r (report_number,
								gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_marked_R)));
	gsb_data_report_set_show_t (report_number,
								gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_marked_T)));

	gsb_data_report_set_not_detail_split (report_number,
											gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																		  (priv->bouton_pas_detailler_ventilation)));
}

/*ONGLET_DATA_GROUPING*/
/**
 * Initialise les informations de l'onglet groupement des données
 *
 * \param report_number
 *
 * \return
 */
void etats_prefs_initialise_onglet_data_grouping (GtkWidget *etats_prefs,
												  gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_group_by_account),
								  gsb_data_report_get_account_group_reports (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_group_by_payee),
								  gsb_data_report_get_payee_used (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_group_by_categ),
								  gsb_data_report_get_category_used (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_group_by_ib),
								  gsb_data_report_get_budget_used (report_number));

	etats_prefs_onglet_data_grouping_update_model (treeview_data_grouping, report_number);
}

/**
 * Récupère les informations de l'onglet groupement des données
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void etats_prefs_recupere_info_onglet_data_grouping (GtkWidget *etats_prefs,
													 gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	gsb_data_report_set_account_group_reports (report_number,
											   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																			 (priv->bouton_group_by_account)));
	gsb_data_report_set_payee_used (report_number,
									gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																  (priv->bouton_group_by_payee)));

	gsb_data_report_set_category_used (report_number,
									   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																	 (priv->bouton_group_by_categ)));
	gsb_data_report_set_budget_used (report_number,
									 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																   (priv->bouton_group_by_ib)));

	/* on récupère les données servant au tri des données pour l'affichage */
	gsb_data_report_free_sorting_type_list (report_number);
	gsb_data_report_set_sorting_type_list (report_number,
										   etats_config_onglet_data_grouping_get_list
										   (report_number));
}

/*ONGLET_DATA_SEPARATION*/
/**
 * Initialise les informations de l'onglet separation des données
 *
 * \param report_number
 *
 * \return
 */
void etats_prefs_initialise_onglet_data_separation (GtkWidget *etats_prefs,
													gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));


	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_split_by_income_expenses),
								  gsb_data_report_get_split_credit_debit (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_split_by_fyears),
								  gsb_data_report_get_financial_year_split (report_number));

	/* on initialise le combo bouton_split_by_type_period */
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->bouton_split_by_type_period),
							  gsb_data_report_get_period_split_type (report_number));

	/* on initialise le combo bouton_debut_semaine */
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->bouton_debut_semaine),
							  gsb_data_report_get_period_split_day (report_number) - G_DATE_MONDAY);

	if (gsb_data_report_get_period_split (report_number))
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_split_by_period), TRUE);

		if (gtk_combo_box_get_active (GTK_COMBO_BOX (priv->bouton_split_by_type_period)) == TRUE)
			gtk_widget_set_sensitive (priv->bouton_debut_semaine, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->paddingbox_data_by_period, FALSE);
	}
}

/**
 * Récupère les informations de l'onglet separation des données
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void etats_prefs_recupere_info_onglet_data_separation (GtkWidget *etats_prefs,
															  gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	gsb_data_report_set_split_credit_debit (report_number,
											gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																		  (priv->button_split_by_income_expenses)));
	gsb_data_report_set_financial_year_split (report_number,
											  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																			(priv->button_split_by_fyears)));
	gsb_data_report_set_period_split (report_number,
									  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																	(priv->button_split_by_period)));

	/* récupère des index des GtkComboBox */
	gsb_data_report_set_period_split_type (report_number,
										   gtk_combo_box_get_active (GTK_COMBO_BOX
																	 (priv->bouton_split_by_type_period)));
	gsb_data_report_set_period_split_day (report_number,
										  gtk_combo_box_get_active (GTK_COMBO_BOX
																	(priv->bouton_debut_semaine)));
}

/*ONGLET_AFFICHAGE_GENERALITES*/
/**
 * Initialise les informations de l'onglet généraités
 *
 * \param report_number
 *
 * \return
 **/
void etats_prefs_initialise_onglet_affichage_generalites (GtkWidget *etats_prefs,
														  gint report_number)
{
	gchar *report_name = NULL;
	gboolean activ;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	report_name = gsb_data_report_get_report_name (report_number);
	gtk_entry_set_text (GTK_ENTRY (priv->entree_nom_etat), report_name);

	/* on initialise le complément du nom si actif */
	activ = gsb_data_report_get_compl_name_used (report_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->check_button_compl_name), activ);
	if (report_name && activ)
	{
		utils_buttons_sensitive_by_checkbutton (priv->check_button_compl_name, priv->hbox_combo_compl_name);
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_box_compl_name_function),
								  gsb_data_report_get_compl_name_function (report_number));
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_box_compl_name_position),
								  gsb_data_report_get_compl_name_position (report_number));
		etats_prefs_display_name_with_complement (report_number, priv);
	}

	/* Connect signal */
	g_signal_connect (priv->check_button_compl_name,
					  "toggled",
					  G_CALLBACK (etats_prefs_check_button_compl_name_toggled),
					  priv);

	g_signal_connect (priv->combo_box_compl_name_function,
					  "changed",
					  G_CALLBACK (etats_prefs_combo_box_compl_data_changed),
					  priv);

	g_signal_connect (priv->combo_box_compl_name_position,
					  "changed",
					  G_CALLBACK (etats_prefs_combo_box_compl_data_changed),
					  priv);

	/* on initialise le type de date à sélectionner */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_sel_value_date),
								  gsb_data_report_get_date_select_value (report_number));

	/* on initialise les autres données */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_ignore_archives),
								  gsb_data_report_get_ignore_archives (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_nb_opes),
								  gsb_data_report_get_show_report_transaction_amount (report_number));

	/* mémorisation de l'état avant initialisation */
	payee_last_state = gsb_data_report_get_append_in_payee (report_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_inclure_dans_tiers), payee_last_state);
}

/**
 * Récupère les informations de l'onglet généralités
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void etats_prefs_recupere_info_onglet_affichage_generalites (GtkWidget *etats_prefs,
															 gint report_number)
{
	const gchar *text;
	gboolean payee_new_state = FALSE;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	/* on récupère le nom de l'état */
	text = gtk_entry_get_text (GTK_ENTRY (priv->entree_nom_etat));

	if (strlen (text) && strcmp (text, gsb_data_report_get_report_name (report_number)))
	{
		gsb_data_report_set_report_name (report_number, text);
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->check_button_compl_name)))
	{

		gsb_data_report_set_compl_name_used (report_number, TRUE);
		gsb_data_report_set_compl_name_function (report_number,
												 gtk_combo_box_get_active (GTK_COMBO_BOX
																		   (priv->combo_box_compl_name_function)));
		gsb_data_report_set_compl_name_position (report_number,
												 gtk_combo_box_get_active (GTK_COMBO_BOX
																		   (priv->combo_box_compl_name_position)));
	}
	else
	{
		gsb_data_report_set_compl_name_used (report_number, FALSE);
		gsb_data_report_set_compl_name_function (report_number, 0);
		gsb_data_report_set_compl_name_function (report_number, 0);
	}

	/* on récupère les autres informations */
	gsb_data_report_set_date_select_value (report_number,
											gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																		  (priv->button_sel_value_date)));
	gsb_data_report_set_ignore_archives (report_number,
										 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																	   (priv->bouton_ignore_archives)));
	gsb_data_report_set_show_report_transaction_amount (report_number,
														gtk_toggle_button_get_active
														(GTK_TOGGLE_BUTTON (priv->bouton_afficher_nb_opes)));

	/* recupere le statut du rapport pour maj combo payee dans le formulaire */
	payee_new_state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_inclure_dans_tiers));
	gsb_data_report_set_append_in_payee (report_number, payee_new_state);
	if (payee_last_state != payee_new_state)
		gsb_form_widget_update_payee_combofix (report_number, payee_new_state);


}

/*ONGLET_AFFICHAGE_TITLES*/
/**
 * Initialise les informations de l'onglet titres
 *
 * \param
 * \param report_number
 *
 * \return
 */
void etats_prefs_initialise_onglet_affichage_titres (GtkWidget *etats_prefs,
													  gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	/* données des comptes */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_comptes),
								  gsb_data_report_get_account_show_name (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_compte),
													  gsb_data_report_get_account_show_amount (report_number));
	sens_desensitive_pointeur (priv->bouton_group_by_account, priv->bouton_affiche_sous_total_compte);

	/* données des tiers */
	sens_desensitive_pointeur (priv->bouton_group_by_payee, priv->bouton_afficher_noms_tiers);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_tiers),
								  gsb_data_report_get_payee_show_name (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_payee, priv->bouton_affiche_sous_total_tiers);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_tiers),
								  gsb_data_report_get_payee_show_payee_amount (report_number));

	/* données des catégories */
	sens_desensitive_pointeur (priv->bouton_group_by_categ, priv->bouton_afficher_noms_categ);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_categ),
								  gsb_data_report_get_category_show_name (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_categ, priv->bouton_affiche_sous_total_categ);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_categ),
								  gsb_data_report_get_category_show_category_amount (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_categ, priv->bouton_afficher_sous_categ);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_categ),
								  gsb_data_report_get_category_show_sub_category (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_categ, priv->bouton_affiche_sous_total_sous_categ);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_sous_categ),
								  gsb_data_report_get_category_show_sub_category_amount (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_categ, priv->bouton_afficher_pas_de_sous_categ);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_pas_de_sous_categ),
								  gsb_data_report_get_category_show_without_category (report_number));

	/* données des IB */
	sens_desensitive_pointeur (priv->bouton_group_by_ib, priv->bouton_afficher_noms_ib);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_ib),
								  gsb_data_report_get_budget_show_name (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_ib, priv->bouton_affiche_sous_total_ib);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_ib),
								  gsb_data_report_get_budget_show_budget_amount (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_ib, priv->bouton_afficher_sous_ib);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_ib),
								  gsb_data_report_get_budget_show_sub_budget (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_ib, priv->bouton_affiche_sous_total_sous_ib);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_sous_ib),
								  gsb_data_report_get_budget_show_sub_budget_amount (report_number));

	sens_desensitive_pointeur (priv->bouton_group_by_ib, priv->bouton_afficher_pas_de_sous_ib);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_pas_de_sous_ib),
								  gsb_data_report_get_budget_show_without_budget (report_number));
}

/**
 * Récupère les informations de l'onglet titres
 *
 * \param
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void etats_prefs_recupere_info_onglet_affichage_titres (GtkWidget *etats_prefs,
														 gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	/* données des comptes */
	gsb_data_report_set_account_show_name (report_number,
										   gtk_toggle_button_get_active
										   (GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_comptes)));
	gsb_data_report_set_account_show_amount (report_number,
											 gtk_toggle_button_get_active
											 (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_compte)));

	/* données des tiers */
	gsb_data_report_set_payee_show_name (report_number,
										 gtk_toggle_button_get_active
										 (GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_tiers)));
	gsb_data_report_set_payee_show_payee_amount (report_number,
												 gtk_toggle_button_get_active
												 (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_tiers)));

	/* données des catégories */
	gsb_data_report_set_category_show_name (report_number,
											gtk_toggle_button_get_active
											(GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_categ)));
	gsb_data_report_set_category_show_category_amount (report_number,
													   gtk_toggle_button_get_active
													   (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_categ)));
	gsb_data_report_set_category_show_sub_category (report_number,
													gtk_toggle_button_get_active
													(GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_categ)));
	gsb_data_report_set_category_show_sub_category_amount (report_number,
														   gtk_toggle_button_get_active
														   (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_sous_categ)));
	gsb_data_report_set_category_show_without_category (report_number,
														gtk_toggle_button_get_active
														(GTK_TOGGLE_BUTTON (priv->bouton_afficher_pas_de_sous_categ)));

	/* données des IB */
	gsb_data_report_set_budget_show_name (report_number,
										  gtk_toggle_button_get_active
										  (GTK_TOGGLE_BUTTON (priv->bouton_afficher_noms_ib)));
	gsb_data_report_set_budget_show_budget_amount (report_number,
												   gtk_toggle_button_get_active
												   (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_ib)));
	gsb_data_report_set_budget_show_sub_budget (report_number,
												gtk_toggle_button_get_active
												(GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_ib)));
	gsb_data_report_set_budget_show_sub_budget_amount (report_number,
													   gtk_toggle_button_get_active
													   (GTK_TOGGLE_BUTTON (priv->bouton_affiche_sous_total_ib)));
	gsb_data_report_set_budget_show_without_budget (report_number,
													gtk_toggle_button_get_active
													(GTK_TOGGLE_BUTTON (priv->bouton_afficher_pas_de_sous_ib)));
}

/*ONGLET_AFFICHAGE_OPERATIONS*/
/**
 * Initialise les informations de l'onglet opérations
 *
 * \param
 * \param report_number
 *
 * \return
 */
void etats_prefs_initialise_onglet_affichage_operations (GtkWidget *etats_prefs,
														 gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	/* on affiche ou pas le choix des données des opérations */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_opes),
								  gsb_data_report_get_show_report_transactions (report_number));

	/* données des opérations à afficher */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_no_ope),
								  gsb_data_report_get_show_report_transaction_number (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_date_opes),
								  gsb_data_report_get_show_report_date (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_value_date_opes),
								  gsb_data_report_get_show_report_value_date (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_tiers_opes),
								  gsb_data_report_get_show_report_payee (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_categ_opes),
								  gsb_data_report_get_show_report_category (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_categ_opes),
								  gsb_data_report_get_show_report_sub_category (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_ib_opes),
								  gsb_data_report_get_show_report_budget (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_ib_opes),
						gsb_data_report_get_show_report_sub_budget (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_notes_opes),
								  gsb_data_report_get_show_report_note (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_type_ope),
								  gsb_data_report_get_show_report_method_of_payment (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_no_cheque),
								  gsb_data_report_get_show_report_method_of_payment_content (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_pc_opes),
								  gsb_data_report_get_show_report_voucher (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_exo_opes),
								  gsb_data_report_get_show_report_financial_year (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_infobd_opes),
								  gsb_data_report_get_show_report_bank_references (report_number));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_no_rappr),
								  gsb_data_report_get_show_report_marked (report_number));

	/* affichage des titres des colonnes */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_titres_colonnes),
								  gsb_data_report_get_column_title_show (report_number));

	if (!gsb_data_report_get_column_title_type (report_number))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_titre_en_haut), TRUE);

	/* sélectionner le type de classement des opérations */
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->bouton_choix_classement_ope_etat),
							  gsb_data_report_get_sorting_report (report_number));

	/* rendre les opérations cliquables */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_rendre_ope_clickables),
								  gsb_data_report_get_report_can_click (report_number));
}


/**
 * Récupère les informations de l'onglet opérations
 *
 * \param
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void etats_prefs_recupere_info_onglet_affichage_operations (GtkWidget *etats_prefs,
															gint report_number)
{
	gboolean affich_opes = FALSE;
	gboolean detail_ope;
	gboolean is_actif = FALSE;
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	affich_opes = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_opes));
	gsb_data_report_set_show_report_transactions (report_number, affich_opes);

	/* données des opérations */
	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_no_ope));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_transaction_number (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_date_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_date (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_value_date_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_value_date (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_tiers_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_payee (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_categ_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_category (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_categ_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_sub_category (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_ib_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_budget (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_sous_ib_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_sub_budget (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_notes_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_note (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_type_ope));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_method_of_payment (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_no_cheque));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_method_of_payment_content (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_pc_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_voucher (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_exo_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_financial_year (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_infobd_opes));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_bank_references (report_number, detail_ope);

	detail_ope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_afficher_no_rappr));
	if (detail_ope && !is_actif)
		is_actif = TRUE;
	gsb_data_report_set_show_report_marked (report_number, detail_ope);

	if (affich_opes && !is_actif)
		gsb_data_report_set_show_report_transactions (report_number, FALSE);

	/* titres des colonnes */
	gsb_data_report_set_column_title_show (report_number,
										   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																		 (priv->bouton_afficher_titres_colonnes)));

	gsb_data_report_set_column_title_type (report_number,
										   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																		 (priv->bouton_titre_changement)));

	/* type de classement des opérations */
	gsb_data_report_set_sorting_report (report_number,
										gtk_combo_box_get_active (GTK_COMBO_BOX (priv->bouton_choix_classement_ope_etat)));

	/* opérations cliquables */
	gsb_data_report_set_report_can_click (report_number,
										  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																		(priv->bouton_rendre_ope_clickables)));
}

/*ONGLET_AFFICHAGE_DEVISES*/
/**
 * Initialise les informations de l'onglet devises
 *
 * \param
 * \param report_number
 *
 * \return
 */
void etats_prefs_initialise_onglet_affichage_devises (GtkWidget *etats_prefs,
													  gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	gsb_currency_set_combobox_history (priv->combobox_devise_general,
									   gsb_data_report_get_currency_general (report_number));

	gsb_currency_set_combobox_history (priv->combobox_devise_payee,
									   gsb_data_report_get_payee_currency (report_number));

	gsb_currency_set_combobox_history (priv->combobox_devise_categ,
									   gsb_data_report_get_category_currency (report_number));

	gsb_currency_set_combobox_history (priv->combobox_devise_ib,
									   gsb_data_report_get_budget_currency (report_number));

	gsb_currency_set_combobox_history (priv->combobox_devise_amount,
									   gsb_data_report_get_amount_comparison_currency (report_number));
}

/**
 * Récupère les informations de l'onglet devises
 *
 * \param
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void etats_prefs_recupere_info_onglet_affichage_devises (GtkWidget *etats_prefs,
														  gint report_number)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);
	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));

	gsb_data_report_set_currency_general (report_number,
										  gsb_currency_get_currency_from_combobox (priv->combobox_devise_general));

	gsb_data_report_set_payee_currency (report_number,
										gsb_currency_get_currency_from_combobox (priv->combobox_devise_payee));

	gsb_data_report_set_category_currency (report_number,
										   gsb_currency_get_currency_from_combobox (priv->combobox_devise_categ));

	gsb_data_report_set_budget_currency (report_number,
										 gsb_currency_get_currency_from_combobox (priv->combobox_devise_ib));

	gsb_data_report_set_amount_comparison_currency (report_number,
													gsb_currency_get_currency_from_combobox (priv->combobox_devise_amount));
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
