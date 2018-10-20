/******************************************************************************/
/*                                                                            */
/*                                gsb_combo_box                               */
/*                                                                            */
/*     Copyright (C)    2000-2006 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2006 Benjamin Drieu (bdrieu@april.org)                       */
/*               2008-2018 Pierre Biava (grisbi@pierre.biava.name)            */
/*          http://www.grisbi.org                                             */
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
/******************************************************************************/

/**
 * \file gsb_combo_box.c
 * plays with special combo_box for grisbi
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_combo_box.h"
#include "gsb_data_payee.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "structures.h"
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
 * Cette fonction retourne un GtkListStore à partir d'une liste
 * TODO modifier le fonctionnement de cette liste
 *
 * \param le tableau de chaines à mettre dans le modèle
 *
 * \return un GtkListStore.
 **/
static GtkListStore *gsb_combo_box_list_store_new_from_list (GSList *list)
{
	GtkListStore *store = NULL;
	GSList *tmp_list;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);

	tmp_list = list;
	while (tmp_list)
	{
		GtkTreeIter iter;
		gchar *string;

		string = tmp_list->data;
		tmp_list = tmp_list->next;

		/* should not append */
		if (!tmp_list)
			break;

		gtk_list_store_append (GTK_LIST_STORE (store), &iter);
		gtk_list_store_set (store,
							&iter,
							0, string,
							1, GPOINTER_TO_INT (tmp_list->data),
							-1);

		tmp_list = tmp_list->next;
	}

	/* return */
	return store;
}

/**
 * Cette fonction initialise la colonne visible d'un GtkComboBox
 *
 * \param le combo à initialiser
 * \param le numéro de la colonne texte
 *
 * \return
 **/
static void gsb_combo_box_set_text_renderer (GtkComboBox *combo,
											 gint num_col)
{
    GtkCellRenderer *renderer;

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
									renderer,
                                    "text", num_col,
                                    NULL);
}

/* COMBO_BOX WITH COMPLETION */
/**
 * fill a parent_iter of the model given in param
 * with the string given in param
 *
 * \param store
 * \param parent_iter
 * \param string
 * \param list_number 	the number of the list
 *
 * \return TRUE
 **/
static gboolean gsb_combo_form_box_fill_iter_parent (GtkTreeStore *store,
											   GtkTreeIter *iter_parent,
                        					   const gchar *string,
                        					   gint list_number)
{
	gtk_tree_store_append (store, iter_parent, NULL);
	gtk_tree_store_set (store,
						iter_parent,
						COMBO_COL_VISIBLE_STRING, string,
						COMBO_COL_REAL_STRING, string,
						COMBO_COL_VISIBLE, TRUE,
						COMBO_COL_LIST_NUMBER, list_number,
						-1);

	return TRUE;
}

/**
 * fill a child_iter of the model given in param
 * with the string given in param
 *
 * \param store
 * \param parent_iter
 * \param string
 * \param list_number 	the number of the list
 *
 * \return TRUE
 **/
static gboolean gsb_combo_form_box_fill_iter_child (GtkTreeStore *store,
													GtkTreeIter *iter_parent,
													const gchar *string,
													const gchar *real_string,
													gint list_number)
{
	GtkTreeIter iter_child;

	gtk_tree_store_append (store, &iter_child, iter_parent);
	gtk_tree_store_set (store,
						&iter_child,
						COMBO_COL_VISIBLE_STRING, string,
						COMBO_COL_REAL_STRING, real_string,
						COMBO_COL_VISIBLE, TRUE,
						COMBO_COL_LIST_NUMBER, list_number,
						-1);

	return TRUE;
}

/**
 * fill the model of the combofix given in param
 * with the list given in param
 * carreful : the list is not cleared, so if needed, must do it before
 *
 * \param                combofix
 * \param list           a g_slist of strings
 * \param list_number 	the number of the list for a complex, 0 else
 *
 * \return TRUE ok, FALSE pb
 **/
static gboolean gsb_combo_form_box_fill_store (GtkTreeStore *store,
											   GSList *list,
											   gint list_number)
{
	GSList *tmp_list;

	GtkTreeIter iter_parent;
	gchar *last_parent = NULL;

	if (!list)
	    return FALSE;

	/* normally the list cannot begin by a child, but we check here to
	 * avoid a big crash */
	if (list->data && ((gchar *) (list->data))[0] == '\t')
	{
		gboolean FAILED = TRUE;

		g_print ("GsbComboFormBox error : the first entry in the list is a child, cannot fill the combofix\n");
		g_return_val_if_fail (FAILED, FALSE);
	}

	tmp_list = list;

	while (tmp_list)
	{
		gchar *string;
		gchar* real_string;

		string = tmp_list->data;

		/* create the new iter where it's necessary and iter will focus on it */
		if (string)
		{
			if (string[0] == '\t')
			{
				/* it's a child */
				real_string = g_strconcat (last_parent, " : ", string + 1, NULL);
				gsb_combo_form_box_fill_iter_child (GTK_TREE_STORE (store), &iter_parent, string + 1, real_string, list_number);

				g_free (real_string);
			}
			else
			{
				/* it's a parent */
				gsb_combo_form_box_fill_iter_parent (GTK_TREE_STORE (store), &iter_parent, string, list_number);
				last_parent = string;
			}
		}

		tmp_list = tmp_list->next;
	}

	return TRUE;
}

/**
 * Creates a tree model containing the completions
 *
 * \param			combofix tree_store
 * \param			content : 0 : payee, 1 : category, 2 : budget
 *
 * \return			a list_store of names for completion
 **/
static GtkTreeModel *gsb_combo_form_box_completion_fill_model (GtkTreeModel *model,
															   gint content)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = gtk_list_store_new (1, G_TYPE_STRING);
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		do
		{
			GtkTreeIter new_iter;
			gchar *real_string;
			gboolean separator;

			gtk_tree_model_get (GTK_TREE_MODEL(model),
								&iter,
								COMBO_COL_REAL_STRING, &real_string,
								COMBO_COL_SEPARATOR, &separator,
								-1);
			if (separator)
			{
				g_free (real_string);
				continue;
			}
			else
			{
				//~ printf ("name = %s\n", real_string);
				gtk_list_store_append (store, &new_iter);
				gtk_list_store_set (store, &new_iter, 0, real_string, -1);

				g_free (real_string);
			}
			if (content)
			{
				GtkTreeIter child_iter;

				if (gtk_tree_model_iter_children (model, &child_iter, &iter))
				{
					do
					{

						gtk_tree_model_get (model,
											&child_iter,
											COMBO_COL_REAL_STRING, &real_string,
											COMBO_COL_SEPARATOR, &separator,
											-1);
						if (separator)
						{
							g_free (real_string);
							continue;
						}
						else
						{
							//~ printf ("name = %s\n", real_string);
							gtk_list_store_append (store, &new_iter);
							gtk_list_store_set (store, &new_iter, 0, real_string, -1);

							g_free (real_string);
						}
					}
					while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &child_iter));
				}
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}

	return GTK_TREE_MODEL (store);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean  gsb_combo_form_box_completion_match_func (GtkEntryCompletion *completion,
														   const gchar *key,
														   GtkTreeIter *iter,
														   gpointer user_data)
{
	GtkTreeModel *model;
	gchar *new_key;
	gchar *new_text;
	const gchar *search;
	gchar *text;
	gssize nbre_bytes;

	model = gtk_entry_completion_get_model (completion);
	gtk_tree_model_get (model, iter, 0, &text, -1);

	if (!text)
		return FALSE;

	search = gtk_entry_get_text (GTK_ENTRY (gtk_entry_completion_get_entry (completion)));
	if (!search)
		return FALSE;

	if (conf.completion_ignore_accents)
	{
		gchar *tmp_text;

		new_key = utils_str_remove_accents (search);
		tmp_text = utils_str_remove_accents (text);
		nbre_bytes = strlen (new_key);
		new_text = g_strndup (tmp_text, nbre_bytes);
		g_free (tmp_text);
	}
	else
	{
		nbre_bytes = strlen (search);
		new_key = g_strndup (search, nbre_bytes);
		new_text = g_strndup (text, nbre_bytes);
	}

	if (g_strcmp0 (new_text, new_key) == 0)
	{
		g_free (new_key);
		g_free (new_text);
		return TRUE;
	}
	else
	{
		g_free (new_key);
		g_free (new_text);
		return FALSE;
	}

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void gsb_combo_form_box_changed (GtkComboBox *widget,
										GtkWidget *entry)
{
	if (gsb_form_widget_check_empty (entry))
	{
		gtk_widget_grab_focus (entry);
		gsb_form_button_press_event (entry, NULL, NULL);
		gtk_editable_set_position (GTK_EDITABLE (entry), -1);
		gsb_form_widget_set_empty (entry, FALSE);
	}
}

/**
 * the default function to sort the combofix,
 * if mixed is set, all the list will be sorted by alphabetic order,
 * else, for a complex combofix, each list will be sorted by itself
 *
 * \param model_sort
 * \param iter_1
 * \param iter_2
 * \param combofix
 *
 * \return -1 if iter_1 before iter_2 ...
 **/
static gint gsb_combo_form_box_sort_func (GtkTreeModel *model_sort,
										  GtkTreeIter *iter_1,
										  GtkTreeIter *iter_2,
										  GtkComboBox *combo)
{
    gint list_number_1;
    gint list_number_2;
    gchar *string_1;
    gchar *string_2;
    gchar *cmp_string_1;
    gchar *cmp_string_2;
    gint return_value = 0;
    gboolean separator_1;
    gboolean separator_2;

    if (iter_1)
        gtk_tree_model_get (GTK_TREE_MODEL (model_sort),
                        iter_1,
                        COMBO_COL_LIST_NUMBER, &list_number_1,
                        COMBO_COL_VISIBLE_STRING, &string_1,
                        COMBO_COL_SEPARATOR, &separator_1,
                        -1);
    else
        return -1;

    if (iter_2)
        gtk_tree_model_get (GTK_TREE_MODEL (model_sort),
                        iter_2,
                        COMBO_COL_LIST_NUMBER, &list_number_2,
                        COMBO_COL_VISIBLE_STRING, &string_2,
                        COMBO_COL_SEPARATOR, &separator_2,
                        -1);
    else
        return 1;

    if (etat.combofix_mixed_sort == FALSE)
        return_value = list_number_1 - list_number_2;

    if (return_value == 0)
    {
        if (separator_1)
            return 1;
        if (separator_2)
            return -1;

        if (string_1 == NULL)
            return -1;
        if (string_2 == NULL)
            return 1;

        cmp_string_1 = g_utf8_collate_key (string_1, -1);
        cmp_string_2 = g_utf8_collate_key (string_2, -1);
        return_value = strcmp (cmp_string_1, cmp_string_2);

        g_free (cmp_string_1);
        g_free (cmp_string_2);
    }
    g_free (string_1);
    g_free (string_2);

    return return_value;
}

/**
 * check if the given row is or not a separator,
 * used in interne in gtk
 *
 * \param model
 * \param iter
 * \param combofix
 *
 * \return TRUE if it's a separator, FALSE else
 **/
static gboolean gsb_combo_form_box_separator_func (GtkTreeModel *model,
												   GtkTreeIter *iter,
												   gpointer data)
{
    gboolean value;

	gtk_tree_model_get (GTK_TREE_MODEL (model),
						iter,
						COMBO_COL_SEPARATOR, &value,
						-1);
    if (value)
	    return TRUE;

	return FALSE;
}

/**
 * renvoie le model initial du gtk_combo_box
 *
 * \param
 *
 * \return
 **/
static GtkTreeModel *gsb_combo_form_box_get_store (GtkComboBox *combo)
{
	GtkTreeModel *model;
	GtkTreeModel *model_filter;
	GtkTreeModel *model_sort;

	model_sort = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
	model_filter = gtk_tree_model_sort_get_model (GTK_TREE_MODEL_SORT (model_sort));
	model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (model_filter));

	return model;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * Cette fonction retourne un GtkListStore à partir d'un tableau de chaine
 *
 * \param le tableau de chaines à mettre dans le modèle
 *
 * \return un GtkListStore.
 **/
GtkListStore *gsb_combo_box_list_store_new_from_array (gchar **array)
{
    GtkListStore *store = NULL;
    gint i = 0;

    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);

    while (array[i])
    {
        GtkTreeIter iter;
        gchar *string;

        string = gettext (array[i]);
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, string, 1, i, -1);

        i++;
    }

    /* return */
    return store;
}

/**
 * create a text only combo_box with an index
 * column 0 will contain the text
 * column 1 will have the index, in the order of the strings
 *
 * this function takes an array of string and attribute a number beginning by the first element of the array
 * to link some text with a special number, use gsb_combo_box_new_with_index_from_list
 *
 * \param string a pointer to an array of strings terminated by NULL, string will be appended in that order
 * \param func an optional function to call when change the current item (gboolean func (GtkWidget *combox, gpointer data)
 * \param data the data to send to the func
 *
 * \return a combo box widget
 **/
GtkWidget *gsb_combo_box_new_with_index (gchar **string,
										 GCallback func,
										 gpointer data)
{
    GtkWidget *combo;
    GtkTreeModel *model;

    combo = gtk_combo_box_new ();

    model = GTK_TREE_MODEL (gsb_combo_box_list_store_new_from_array (string));
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model);

    gsb_combo_box_set_text_renderer (GTK_COMBO_BOX (combo), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
    if (func)
        g_signal_connect (G_OBJECT (combo),
						  "changed",
						  G_CALLBACK (func),
						  data);

    return combo;
}

/**
 * create a text only combo_box with an index
 * column 0 will contain the text
 * column 1 will have the index
 *
 * the function takes a g_slist of text and number to attribute
 * 	this list must be as : text_1->number_1->text_2->number_2->...
 *
 * \param list	a g_slist containing succession of text and number to associate to the text
 * \param func an optional function to call when change the current item (gboolean func (GtkWidget *combox, gpointer data)
 * \param data the data to send to the func
 *
 * \return a combo box widget
 **/
GtkWidget *gsb_combo_box_new_with_index_from_list (GSList *list,
												   GCallback func,
												   gpointer data)
{
    GtkWidget *combo;
    GtkTreeModel *model;

    combo = gtk_combo_box_new ();

    model = GTK_TREE_MODEL (gsb_combo_box_list_store_new_from_list (list));
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model);

    gsb_combo_box_set_text_renderer (GTK_COMBO_BOX (combo), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

    if (func)
        g_signal_connect (G_OBJECT (combo),
						  "changed",
						  G_CALLBACK (func),
						  data);

    return combo;
}

/**
 * get the index of the current item in the combo_box given in param
 * the combo_box must have been created with gsb_combo_box_new_with_index
 *
 * \param combo_box
 *
 * \return a number, the index, or -1 if nothing selected
 **/
gint gsb_combo_box_get_index (GtkWidget *combo_box)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint index;

    if (!combo_box)
        return -1;

    if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo_box), &iter))
        return -1;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));
    gtk_tree_model_get (model, &iter, 1, &index, -1);

    return index;
}

/**
 * set the index of the item in the combo_box given in param,
 * ie makes visible the text corresponding of the index
 * the combo_box must have been created with gsb_combo_box_new_with_index
 *
 * \param combo_box
 * \param index the index of the text we want to set visible
 *
 * \return TRUE ok, FALSE problem
 **/
gint gsb_combo_box_set_index (GtkWidget *combo_box,
							  gint index)
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    /* if no account button, go away... */
    if (!combo_box)
		return FALSE;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
		do
		{
			gint number;

			gtk_tree_model_get (model, &iter, 1, &number, -1);
			if (number == index)
			{
				gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo_box), &iter);
				return TRUE;
			}
		}
		while (gtk_tree_model_iter_next (model, &iter));
    }
    return FALSE;
}

/**
 * get the text of the current item in the combo_box given in param
 * the combo_box must have been created with gsb_combo_box_new_with_index
 *
 * \param combo_box
 *
 * \return a newly allocated string or NULL if nothing selected
 **/
gchar *gsb_combo_box_get_active_text (GtkComboBox *combo_box)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *string;

    if (!combo_box)
        return NULL;

    if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo_box), &iter))
        return NULL;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));
    gtk_tree_model_get (model, &iter, 0, &string, -1);

    return string;
}

/* COMBO_BOX WITH COMPLETION */
/**
 * initialise un gtk_combo_box with an entry and a model for payees
 *
 * \param list		a g_slist containing payees
 * \param func an optional function to call when change the current item (gboolean func (GtkWidget *combox, gpointer data)
 * \param data the data to send to the func
 *
 * \return a combo box widget
 * */
GtkWidget *gsb_combo_form_box_new (GSList *list,
								   GCallback func,
								   gpointer data)
{
	GtkWidget *combo;
	GtkWidget *entry;
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_model;
	GtkTreeIter iter;
	GtkTreeModel *model_filter;
	GtkTreeModel *model_sort;
	GtkTreeStore *store;
	GSList *tmp_list;
	gint length;
	gint list_number = 0;

	devel_debug (NULL);

	/* set model */
	store = gtk_tree_store_new (COMBO_N_COLUMNS,
								G_TYPE_STRING,		/* string : what we see in the combofix */
								G_TYPE_STRING,		/* string : what we set in the entry when selecting something */
								G_TYPE_BOOLEAN,		/* boolean : if that line has to be showed */
								G_TYPE_INT,			/* int : the number of the list 0, 1 ou 2 (CREDIT DEBIT SPECIAL) */
								G_TYPE_BOOLEAN		/* TRUE : if this is a separator */
								);

	tmp_list = list;
	length = g_slist_length (list);

	while (tmp_list)
	{
		gsb_combo_form_box_fill_store (GTK_TREE_STORE (store), tmp_list->data, list_number);

		/* set the separator */
		if (list_number < (length-1))
		{
			gtk_tree_store_append (GTK_TREE_STORE (store), &iter, NULL);
			gtk_tree_store_set (GTK_TREE_STORE (store),
								&iter,
								COMBO_COL_LIST_NUMBER, list_number,
								COMBO_COL_VISIBLE, TRUE,
								COMBO_COL_SEPARATOR, TRUE,
								-1);
		}
		list_number++;

		tmp_list = tmp_list->next;
	}

	/* set model and properties for combobox */
	combo = gtk_combo_box_new_with_entry ();
	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo),
										  (GtkTreeViewRowSeparatorFunc) gsb_combo_form_box_separator_func,
										  NULL,
										  NULL);

	/* we set the store in a filter to show only what is selected */
    model_filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (store), NULL);
    gtk_tree_model_filter_set_visible_column (GTK_TREE_MODEL_FILTER (model_filter),
											  COMBO_COL_VISIBLE);

    /* we set the filter in a sorting model */
    model_sort = gtk_tree_model_sort_new_with_model (model_filter);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model_sort),
										  COMBO_COL_VISIBLE_STRING,
										  GTK_SORT_ASCENDING);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (model_sort),
									 COMBO_COL_VISIBLE_STRING,
									 (GtkTreeIterCompareFunc) gsb_combo_form_box_sort_func,
									 combo,
									 NULL);

	gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model_sort);
	gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (combo), 0);

	/* set completion */
	completion = gtk_entry_completion_new ();
	gtk_entry_completion_set_inline_selection (completion, TRUE);
	if (etat.combofix_case_sensitive)
		gtk_entry_completion_set_match_func (completion,
											 (GtkEntryCompletionMatchFunc) gsb_combo_form_box_completion_match_func,
											 NULL,
											 NULL);

	gtk_entry_completion_set_minimum_key_length (completion, conf.combo_minimum_key_length);

	entry = gtk_bin_get_child (GTK_BIN (combo));
	gtk_entry_set_completion (GTK_ENTRY (entry), completion);
	g_object_unref (completion);

	/* Create a tree model and use it as the completion model */
	completion_model = gsb_combo_form_box_completion_fill_model (GTK_TREE_MODEL (store), length-1);
	gtk_entry_completion_set_model (completion, completion_model);
	g_object_unref (completion_model);

	/* Use model column 0 as the text column */
	gtk_entry_completion_set_text_column (completion, 0);

	g_signal_connect (G_OBJECT (combo),
					  "changed",
					  G_CALLBACK (gsb_combo_form_box_changed),
					  entry);

	if (func)
		g_signal_connect (G_OBJECT (combo),
						  "changed",
						  G_CALLBACK (func),
						  data);

	return combo;
}

/**
 * change the list of an existing combo_form_box
 *
 * \param combo_box
 * \param list the new list
 *
 * \return TRUE if ok, FALSE if problem
 **/
gboolean gsb_combo_form_box_set_list (GtkComboBox *combo_box,
									  GSList *list)
{
	GSList *tmp_list;
	gint list_number = 0;
	gint length;
	GtkTreeIter iter;
	GtkTreeModel *store;

	g_return_val_if_fail (combo_box, FALSE);
	g_return_val_if_fail (GTK_IS_COMBO_BOX (combo_box), FALSE);
	g_return_val_if_fail (list, FALSE);

	store = gsb_combo_form_box_get_store (combo_box);
	gtk_tree_store_clear (GTK_TREE_STORE (store));

	tmp_list = list;
	length = g_slist_length (list);

	while (tmp_list)
	{
		gsb_combo_form_box_fill_store (GTK_TREE_STORE (store), tmp_list->data, list_number);

		/* set the separator */
		if (list_number < (length-1))
		{
			gtk_tree_store_append (GTK_TREE_STORE (store), &iter, NULL);
			gtk_tree_store_set (GTK_TREE_STORE (store),
								&iter,
								COMBO_COL_LIST_NUMBER, list_number,
								COMBO_COL_SEPARATOR, TRUE,
								-1);
		}
		list_number++;

		tmp_list = tmp_list->next;
	}

	return TRUE;
}

/**
 * retourne le widget entry du combobox
 *
 * \param
 *
 * \return
 **/
GtkWidget *gsb_combo_form_box_get_entry (GtkComboBox *combo_box)
{
	GtkWidget *entry;
	gboolean has_entry;

	has_entry = gtk_combo_box_get_has_entry (combo_box);
	if (has_entry)
		entry = gtk_bin_get_child (GTK_BIN (combo_box));
	else
		entry = NULL;

	return entry;
}

/**
 * ajoute une ligne dans la liste du combobox
 *
 * \param	combo
 * \param	texte à ajouter
 *
 * \return
 **/
void gsb_combo_form_box_append_text (GtkComboBox *combo_box,
									 const gchar *text)
{
	GtkWidget *entry;
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_model;
	GtkTreeModel *model;
	GtkTreeIter new_iter;
	gchar **tab_char;

	devel_debug (text);
	if (!combo_box || !GTK_IS_COMBO_BOX (combo_box))
		return;

	model = gsb_combo_form_box_get_store (combo_box);

	tab_char = g_strsplit (text, " : ", 2);
    if (tab_char[0])
    {
        GtkTreeIter iter_parent;

        gsb_combo_form_box_fill_iter_parent (GTK_TREE_STORE (model), &iter_parent, text, 0);

        if (tab_char[1])
        {
            gchar* tmpstr;

            tmpstr = g_strconcat ("\t", text, NULL);
            gsb_combo_form_box_fill_iter_child (GTK_TREE_STORE (model), &iter_parent, tab_char[1], text, 0);

            g_free (tmpstr);
        }
    }
    g_strfreev (tab_char);

	/* update completion */
	entry = gtk_bin_get_child (GTK_BIN (combo_box));
	completion = gtk_entry_get_completion (GTK_ENTRY (entry));
	completion_model = gtk_entry_completion_get_model (completion);
	gtk_list_store_append (GTK_LIST_STORE (completion_model), &new_iter);
	gtk_list_store_set (GTK_LIST_STORE (completion_model), &new_iter, 0, text, -1);

}

/**
 * supprime une ligne dans la liste du combobox
 *
 * \param	combo
 * \param	texte à ajouter
 *
 * \return
 **/
void gsb_combo_form_box_remove_text (GtkComboBox *combo_box,
									 const gchar *text)
{
	GtkWidget *entry;
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_model;
    GtkTreeIter iter;
	GtkTreeModel *store;
    gboolean valid;

	store = gsb_combo_form_box_get_store (combo_box);
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);

    while (valid)
    {
        gchar *tmp_str;
        gboolean separator;

        gtk_tree_model_get (GTK_TREE_MODEL (store),
							&iter,
							COMBO_COL_REAL_STRING, &tmp_str,
							COMBO_COL_SEPARATOR, &separator,
							-1);

        if (separator)
        {
            valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);
            continue;
        }
        if (etat.combofix_case_sensitive && !strcmp (text, tmp_str))
        {
            g_free (tmp_str);
            break;
        }
        else if (!g_utf8_collate (g_utf8_casefold (text, -1), g_utf8_casefold (tmp_str, -1)))
        {
            g_free (tmp_str);
            break;
        }

        g_free (tmp_str);

        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);
    }

	if (valid)
		gtk_tree_store_remove (GTK_TREE_STORE (store), &iter);

	/* update completion */
	entry = gtk_bin_get_child (GTK_BIN (combo_box));
	completion = gtk_entry_get_completion (GTK_ENTRY (entry));
	completion_model = gtk_entry_completion_get_model (completion);
    valid = gtk_tree_model_get_iter_first (completion_model, &iter);

    while (valid)
    {
        gchar *tmp_str;

        gtk_tree_model_get (completion_model, &iter, 0, &tmp_str, -1);

        if (etat.combofix_case_sensitive && !strcmp (text, tmp_str))
        {
            g_free (tmp_str);
            break;
        }
        else if (!g_utf8_collate (g_utf8_casefold (text, -1), g_utf8_casefold (tmp_str, -1)))
        {
            g_free (tmp_str);
            break;
        }

        g_free (tmp_str);

        valid = gtk_tree_model_iter_next (completion_model, &iter);
    }

    if (valid)
        gtk_list_store_remove (GTK_LIST_STORE (completion_model), &iter);
}

void gsb_combo_form_box_block_unblock_by_func (GtkWidget *combo_box,
											   gboolean block)
{
	GtkWidget *entry;

	if (!GTK_IS_COMBO_BOX (combo_box))
		return;

	entry = gtk_bin_get_child (GTK_BIN (combo_box));
	if (block)
		g_signal_handlers_block_by_func(combo_box, gsb_combo_form_box_changed, entry);
	else
		g_signal_handlers_unblock_by_func(combo_box, gsb_combo_form_box_changed, entry);
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

