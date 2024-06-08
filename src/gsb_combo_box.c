/******************************************************************************/
/*                                                                            */
/*                                gsb_combo_box                               */
/*                                                                            */
/*     Copyright (C)    2000-2006 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2006 Benjamin Drieu (bdrieu@april.org)                       */
/*               2008-2018 Pierre Biava (grisbi@pierre.biava.name)            */
/*          https://www.grisbi.org/                                            */
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


#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_combo_box.h"
#include "gsb_data_payee.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_rgba.h"
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
 * Cette fonction retourne un GtkListStore à partir d'un tableau de chaine
 *
 * \param le tableau de chaines à mettre dans le modèle
 *
 * \return un GtkListStore.
 **/
static GtkListStore *gsb_combo_box_list_store_new_from_array (const gchar **array)
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

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
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
GtkWidget *gsb_combo_box_new_with_index (const gchar **string,
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

GtkWidget *gsb_combo_box_gsettings_new_from_list (gchar **string,
											 const gchar *first_str,
											 gboolean set_activ,
											 gint value,
											 GCallback func,
											 gpointer data)
{
	GtkWidget *combo;
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	const gchar *str_color;
	gint i = 0;
	gint j = 0;

	combo = gtk_combo_box_new ();
	str_color = gsb_rgba_get_couleur_to_string ("text_gsetting_option_normal");
	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);

	if (first_str && strlen (first_str))
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, first_str, 1, i, 2, str_color, -1);

		i++;
	}

    while (string[j])
    {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, string[j], 1, i, 2, str_color, -1);

        i++;
		j++;
    }
	gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
									renderer,
									"text", 0,
									"foreground", 2,
									NULL);

	if (set_activ)
		gtk_combo_box_set_active (GTK_COMBO_BOX (combo), value);

	if (func)
        g_signal_connect (G_OBJECT (combo),
						  "changed",
						  G_CALLBACK (func),
						  data);

	return combo;
}

/**
 * new combo_box from glade widget
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
void gsb_combo_box_new_with_index_from_ui (GtkWidget *combo,
										   const gchar **string,
										   GCallback func,
										   gpointer data)
{
    GtkTreeModel *model;

    model = GTK_TREE_MODEL (gsb_combo_box_list_store_new_from_array (string));
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model);

    gsb_combo_box_set_text_renderer (GTK_COMBO_BOX (combo), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
    if (func)
        g_signal_connect (G_OBJECT (combo),
						  "changed",
						  G_CALLBACK (func),
						  data);
}

/**
 * create a text only combo_box with an index from glade widget
 * column 0 will contain the text
 * column 1 will have the index
 *
 * the function takes a g_slist of text and number to attribute
 * 	this list must be as : text_1->number_1->text_2->number_2->...
 *
 * \param combo from glade ui
 * \param list	a g_slist containing succession of text and number to associate to the text
 * \param func an optional function to call when change the current item (gboolean func (GtkWidget *combox, gpointer data)
 * \param data the data to send to the func
 *
 * \return
 **/
void gsb_combo_box_new_with_index_from_list_from_ui (GtkWidget *combo,
													 GSList *list,
													 GCallback func,
													 gpointer data)
{
    GtkTreeModel *model;

    model = GTK_TREE_MODEL (gsb_combo_box_list_store_new_from_list (list));
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model);

    gsb_combo_box_set_text_renderer (GTK_COMBO_BOX (combo), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

    if (func)
        g_signal_connect (G_OBJECT (combo),
						  "changed",
						  G_CALLBACK (func),
						  data);
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

