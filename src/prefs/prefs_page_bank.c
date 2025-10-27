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
#include "prefs_page_bank.h"
#include "dialog.h"
#include "gsb_bank.h"
#include "gsb_data_bank.h"
#include "gsb_data_account.h"
#include "gsb_file.h"
#include "navigation.h"
#include "widget_bank_details.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "widget_account_property.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageBankPrivate   PrefsPageBankPrivate;

struct _PrefsPageBankPrivate
{
	GtkWidget *			vbox_bank;

	GtkWidget * 		box_bank_details;
	GtkWidget *			button_bank_add;
    GtkWidget *         button_bank_remove;
	GtkWidget *			sw_bank;
    GtkWidget *			treeview_bank;

	GtkWidget *			w_bank_details;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageBank, prefs_page_bank, GTK_TYPE_BOX)

enum bank_list_col
{
   BANK_VIEW_NAME_COL = 0,
   BANK_TALKER_COL,
   BANK_NUMBER,
   BANK_ROW_COLOR,
   BANK_LIST_COL_NB
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * callback used to create a new bank
 *
 * \param button	the button "add" or the combobox of the list of banks
 * \param page
 *
 * \return FALSE
 **/
static void prefs_page_bank_add_clicked (GtkWidget *button,
										 PrefsPageBank *page)
{
	GtkWidget *combo_bank_list;
	GtkTreeSelection *selection;
	GtkListStore *store;
	GtkTreeIter iter;
    gint bank_number;
	PrefsPageBankPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bank_get_instance_private (page);

	devel_debug (NULL);

	/* create the new bank */
    bank_number = gsb_data_bank_new (_("New bank"));
    if (!bank_number)
    {
		dialogue_error_memory ();

		return;
    }

	/* play with the list if we are on the configuration */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_bank));
	store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_bank)));

	/* remove first the selection */
	gtk_tree_selection_unselect_all (selection);

	/* add the selection to the list */
	gtk_list_store_append (GTK_LIST_STORE (store), &iter);
	gtk_list_store_set (GTK_LIST_STORE (store),
						&iter,
						BANK_VIEW_NAME_COL, gsb_data_bank_get_name (bank_number),
						BANK_TALKER_COL, "",
						BANK_NUMBER, bank_number,
						-1);
	gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);

	/* remove the bank from the combobox list */
	combo_bank_list = widget_account_property_get_combo_bank_list();
	if (combo_bank_list)
		gsb_bank_update_combo_list_model (combo_bank_list);

	/* set sensitive button_bank_remove */
	gtk_widget_set_sensitive (priv->button_bank_remove, TRUE);

	/* select the new bank in the entry and give the focus */
	widget_bank_details_select_name_entry (bank_number, priv->w_bank_details);

	/* affiche le widget bank_details */
	if (gtk_widget_get_no_show_all (priv->w_bank_details))
	{
		gtk_widget_set_no_show_all (priv->w_bank_details, FALSE);
		gtk_widget_show_all (priv->w_bank_details);
	}

    gsb_file_set_modified (TRUE);
}

/**
 * Fills in bank details widgets with corresponding stuff.  Make them
 * active as well.
 *
 * \param selection
 * \param w_bank_details	widget containing the form
 *
 * \return FALSE
 **/
static void prefs_page_bank_list_change_selection (GtkTreeSelection *selection,
												   GtkWidget *w_bank_details)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
devel_debug (NULL);
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
		gint bank_number;

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, BANK_NUMBER, &bank_number, -1);
		widget_bank_details_update_form (bank_number, w_bank_details);
    }
    else
		widget_bank_details_update_form (-1, w_bank_details);
}

/**
 * delete a bank
 *
 * \param button
 * \param page
 *
 * \return FALSE
 **/
static void prefs_page_bank_remove_clicked (GtkWidget *button,
											PrefsPageBank *page)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *list_tmp;
	gchar* tmp_str;
    gint bank_number;
	gint current_account = -1;
	gint nbre_bank;
	gboolean account_is_display = FALSE;
    gboolean bank_is_used = FALSE;
    gboolean result;
	PrefsPageBankPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bank_get_instance_private (page);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_bank));
	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
		return;

	current_account = gsb_gui_navigation_get_current_account ();

    /* get the bank number */
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, BANK_NUMBER, &bank_number, -1);

    /* check if the bank is used in grisbi */
    list_tmp = gsb_data_account_get_list_accounts ();
    while (list_tmp)
    {
		gint i;

		i = gsb_data_account_get_no_account (list_tmp->data);

		if (gsb_data_account_get_bank (i) == bank_number)
		{
			bank_is_used = TRUE;
			if (current_account == i)
				account_is_display = TRUE;
		}
		list_tmp = list_tmp->next;
    }

    if (bank_is_used)
    {
		tmp_str = g_strdup_printf (_("Bank \"%s\" is used by one or several accounts.\nDo you really want to remove it?"),
								   gsb_data_bank_get_name (bank_number));
		result = dialogue_yes_no (tmp_str, _("Confirmation of bank removal"), GTK_RESPONSE_NO);
		g_free (tmp_str);
    }
    else
    {
        tmp_str = g_strdup_printf (_("Are you sure you want to remove bank \"%s\"?\n"),
								   gsb_data_bank_get_name (bank_number));
		result = dialogue_yes_no (tmp_str, _("Confirmation of bank removal"), GTK_RESPONSE_NO);
       g_free (tmp_str);
    }

    if (result)
    {
		GtkWidget *combobox;

		/* remove the bank */
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		gsb_data_bank_remove (bank_number);

		/* remove the bank from the combobox list */
		combobox = widget_account_property_get_combo_bank_list ();
		gsb_bank_update_combo_list_model (combobox);

		nbre_bank = gsb_data_bank_max_number ();

		/* set combobox at None if the current account is displayed or nbre_bank = 0 */
		if (account_is_display || nbre_bank == 0)
			gsb_bank_combo_list_set_bank (combobox, 0);

		/* set unsensitive button_bank_remove */
		if (nbre_bank == 0)
		{
			gtk_widget_set_sensitive (priv->button_bank_remove, FALSE);

			/* cache le widget bank_details */
			gtk_widget_set_no_show_all (priv->w_bank_details, TRUE);
			gtk_widget_hide (priv->w_bank_details);
		}
		else
			gtk_widget_set_sensitive (priv->button_bank_remove, TRUE);

		gsb_file_set_modified (TRUE);
	}
}

/**
 * Init the tree_view for the bank list
 * set the model given in param
 * set the columns and all the connections
 *
 * \param page
 *
 * \return
 **/
static void prefs_page_bank_setup_tree_view (PrefsPageBank *page)
{
	GtkListStore *store = NULL;
	GtkTreeSelection *selection;
	GSList *tmp_list;
    const gchar *titles[] = {N_("Bank"), N_("Contact name")};
    gfloat alignment[] = {COLUMN_LEFT, COLUMN_LEFT};
    gint i;
    gint nbre_bank = 0;
    gint sw_height = 0;
	PrefsPageBankPrivate *priv;

	priv = prefs_page_bank_get_instance_private (page);

    /* set the store */
    store = gtk_list_store_new (BANK_LIST_COL_NB,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_INT,
								GDK_TYPE_RGBA);

    /* Create tree tree_view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_bank), GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));

    /* connect the selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_bank));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);

    /* set the columns */
    for (i = 0 ; i < BANK_NUMBER; i++)
    {
		GtkTreeViewColumn *column;
		GtkCellRenderer *cell;

		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", alignment[i], NULL);

		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		gtk_tree_view_column_set_alignment (column, alignment[i]);

		gtk_tree_view_column_pack_start (column, cell, TRUE);
		gtk_tree_view_column_set_title (column, gettext (titles[i]));
		gtk_tree_view_column_set_attributes (column,
											 cell,
											 "text", i,
											 "cell-background-rgba", BANK_ROW_COLOR,
											 NULL);
		gtk_tree_view_column_set_expand (column, TRUE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(priv->treeview_bank), column);
    }

	/* fill the model */
	tmp_list = gsb_data_bank_get_bank_list ();
    nbre_bank = g_slist_length (tmp_list);
	while (tmp_list)
	{
	    gint bank_number;
	    GtkTreeIter iter;

	    bank_number = gsb_data_bank_get_no_bank (tmp_list->data);

	    gtk_list_store_append (GTK_LIST_STORE (store), &iter);
	    gtk_list_store_set (GTK_LIST_STORE (store),
							&iter,
							BANK_VIEW_NAME_COL, gsb_data_bank_get_name (bank_number),
							BANK_TALKER_COL, gsb_data_bank_get_correspondent_name (bank_number),
							BANK_NUMBER, bank_number,
							-1);
	    tmp_list = tmp_list->next;
	}

    /* set the height of sw */
	if (nbre_bank <= 3)
		sw_height = 110;
	else if (nbre_bank > 5)
		sw_height = 200;
	else
		sw_height = (33 * nbre_bank) + 10;
	gtk_widget_set_size_request (priv->sw_bank, -1, sw_height);

	/* set colors */
	utils_set_list_store_background_color (priv->treeview_bank, BANK_ROW_COLOR);

	/* set sensitive button_bank_remove */
	if (nbre_bank)
		gtk_widget_set_sensitive (priv->button_bank_remove, TRUE);
	else
		gtk_widget_set_sensitive (priv->button_bank_remove, FALSE);
}

/**
 * Création de la page de gestion des banques
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_bank_setup_page (PrefsPageBank *page)
{
	GtkWidget *head_page;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	PrefsPageBankPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bank_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Banks"), "gsb-banks-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_bank), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_bank), head_page, 0);

	/* set the list of banks */
	prefs_page_bank_setup_tree_view (page);

	/* set détails of bank */
	priv->w_bank_details = GTK_WIDGET (widget_bank_details_new (page, NULL));
	gtk_box_pack_start (GTK_BOX (priv->box_bank_details), priv->w_bank_details, FALSE, FALSE, 0);

	/* set selection signal to update first bank */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_bank));
	g_signal_connect (G_OBJECT (selection),
					  "changed",
					  G_CALLBACK (prefs_page_bank_list_change_selection),
					  priv->w_bank_details);

	/* select the first item */
	path = gtk_tree_path_new_from_indices (0, -1);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	/* set signals */
    g_signal_connect (G_OBJECT (priv->button_bank_add),
                      "clicked",
                      G_CALLBACK  (prefs_page_bank_add_clicked),
                      page);
    g_signal_connect (G_OBJECT (priv->button_bank_remove),
                      "clicked",
                      G_CALLBACK (prefs_page_bank_remove_clicked),
                      page);

	if (g_slist_length (gsb_data_bank_get_bank_list ()))
	{
		if (gtk_widget_get_no_show_all (priv->w_bank_details))
		{
			/* affiche le widget bank_details */
			gtk_widget_set_no_show_all (priv->w_bank_details, FALSE);
			gtk_widget_show_all (priv->w_bank_details);
		}
	}
	else
	{
		if (! gtk_widget_get_no_show_all (priv->w_bank_details))
		{
			/* cache le widget bank_details */
			gtk_widget_set_no_show_all (priv->w_bank_details, TRUE);
			gtk_widget_hide (priv->w_bank_details);
		}
	}
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_bank_init (PrefsPageBank *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_bank_setup_page (page);
}

static void prefs_page_bank_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_bank_parent_class)->dispose (object);
}

static void prefs_page_bank_class_init (PrefsPageBankClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_bank_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_bank.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBank, vbox_bank);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBank, box_bank_details);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBank, button_bank_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBank, button_bank_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBank, sw_bank);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBank, treeview_bank);
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
PrefsPageBank *prefs_page_bank_new (GrisbiPrefs *win)
{
	PrefsPageBank *page;

	page = g_object_new (PREFS_PAGE_BANK_TYPE, NULL);

	if (win == NULL) /* appel par assitant_file */
	{
		PrefsPageBankPrivate *priv;

		devel_debug (NULL);
		priv = prefs_page_bank_get_instance_private (page);
		gtk_widget_set_sensitive (priv->button_bank_remove, FALSE);
	}

	return page;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *prefs_page_bank_get_button_remove (PrefsPageBank *page)
{
	PrefsPageBankPrivate *priv;

	priv = prefs_page_bank_get_instance_private (page);

	return priv->button_bank_remove;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *prefs_page_bank_get_treeview (PrefsPageBank *page)
{
	PrefsPageBankPrivate *priv;

	priv = prefs_page_bank_get_instance_private (page);

	return priv->treeview_bank;
}

/**
 * Update bank name in list, called when the name of the bank changes, or
 * the name of the talker
 * update too the model list of the combobox
 *
 * \param entry 			Widget that triggered signal
 * \param combobox 			NULL or a combobox we want to keep the state while editing a bank
 *
 * \return FALSE
 **/
void prefs_page_bank_update_selected_line (GtkEntry *entry,
										   PrefsPageBank *page)
{
    GtkTreeIter iter;
	gint name_type = 0;
	gint column_number;
	PrefsPageBankPrivate *priv;

	priv = prefs_page_bank_get_instance_private (page);
	name_type= GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "name-type"));
	if (name_type)
		column_number = BANK_TALKER_COL;
	else
		column_number = BANK_VIEW_NAME_COL;

    /* first we update the list if we are in configuration */
    if (priv->treeview_bank && GTK_IS_TREE_VIEW (priv->treeview_bank))
    {
        GtkTreeSelection *selection;
        GtkTreeModel *model;

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_bank));

        if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
            gtk_list_store_set (GTK_LIST_STORE (model),
								&iter,
								column_number, gtk_entry_get_text (entry),
								-1);
    }

	gsb_bank_update_combo_list_model (widget_account_property_get_combo_bank_list());
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

