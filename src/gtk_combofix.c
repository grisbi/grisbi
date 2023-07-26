/******************************************************************************/
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2021 Pierre Biava (grisbi@pierre.biava.name)                 */
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
/******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <ctype.h>

/*START_INCLUDE*/
#include "gtk_combofix.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "structures.h"
#include "utils_buttons.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint block_expose_event;
static GtkTreeIter report_parent_iter;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

#define COMBOFIX_MIN_WIDTH		250

typedef struct _GtkComboFixPrivate  GtkComboFixPrivate;

struct _GtkComboFixPrivate
{
	GtkWidget *			entry;
    GtkWidget *			button;
    GtkWidget *			popup;
    GtkWidget *			tree_view;

	/* tree_view */
    GtkTreeStore *		store;
    GtkTreeModel *		model_filter;
    GtkTreeModel *		model_sort;
    GtkTreeSelection *	selection;

  	/* set properties */
    gboolean			case_sensitive;			/* TRUE for case sensitive (in that case, the first entry give the case) */
    gboolean			force;					/* TRUE if the entry content must belong to the list  */
    gboolean			mixed_sort;				/* TRUE mix the different list, FALSE separate them */
    gint				visible_items;			/* number of items */
	gint				minimum_key_length;		/* minimum_key_length of completion */
	gboolean			ignore_accents;			/* if case_sensitive is TRUE ignore accents in the completion */

	gint				type;					/* type : 0 : payee, 1 : category, 2 : budget */

    /* old entry */
    gchar *				old_entry;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtkComboFix, gtk_combofix, GTK_TYPE_BOX)

enum CombofixColumns
{
    COMBOFIX_COL_VISIBLE_STRING = 0,    /* string : what we see in the combofix */
    COMBOFIX_COL_REAL_STRING,           /* string : what we set in the entry when selecting something */
    COMBOFIX_COL_VISIBLE,               /* boolean : if that line has to be showed */
    COMBOFIX_COL_LIST_NUMBER,           /* int : the number of the list 0, 1 ou 2 (CREDIT DEBIT SPECIAL) */
    COMBOFIX_COL_SEPARATOR,             /* TRUE : if this is a separator */
    COMBOFIX_N_COLUMNS
};

enum CombofixKeyDirection
{
    COMBOFIX_UP = 0,
    COMBOFIX_PAGE_UP,
    COMBOFIX_DOWN,
    COMBOFIX_PAGE_DOWN
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void gtk_combofix_completion_no_match_selected (GtkEntryCompletion *completion,
													   GtkComboFixPrivate *priv)
{
	devel_debug (NULL);
	if (priv->force)
	{
		gint end_pos;

		end_pos = gtk_editable_get_position (GTK_EDITABLE (priv->entry));
		gtk_editable_delete_text (GTK_EDITABLE (priv->entry), end_pos-1, end_pos);

	}
}

/**
 * positionne le bouton "Change" du formulaire si le compte destinataire
 * du transfert a une devise différente du compte de départ.
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_completion_match_selected (GtkEntryCompletion *entry,
														GtkTreeModel *model,
														GtkTreeIter *iter,
														GtkComboFix *combofix)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);
	devel_debug (NULL);

	if (priv->type == METATREE_CATEGORY)
	{
		gchar *tmp_str;

		gtk_tree_model_get (GTK_TREE_MODEL (model), iter, 0, &tmp_str, -1);
		if (g_str_has_prefix (tmp_str, _("Transfer : ")))
		{
			GtkWidget *widget;

			widget = gsb_form_widget_get_widget (TRANSACTION_FORM_DEVISE);
			if (widget != NULL && gtk_widget_get_visible (widget))
			{
				const gchar *entry_account_name;
				gint entry_account_currency;
				gint entry_account_number;
				gint form_account_nb;
				gint form_account_currency;

				form_account_nb = gsb_form_get_account_number ();
				form_account_currency = gsb_data_account_get_currency (form_account_nb);

				/* data of selected account */
				entry_account_name = memchr (tmp_str, ':', strlen (tmp_str));
				entry_account_name = entry_account_name + 2;
				entry_account_number = gsb_data_account_get_no_account_by_name (entry_account_name);
				entry_account_currency = gsb_data_account_get_currency (entry_account_number);

				if (entry_account_currency == form_account_currency)
					gtk_widget_hide (gsb_form_widget_get_widget (TRANSACTION_FORM_CHANGE));
				else
					gtk_widget_show (gsb_form_widget_get_widget (TRANSACTION_FORM_CHANGE));
			}
		}
		g_free (tmp_str);
	}
	return FALSE;
}

/**
 * get the first item of completion and fill the entry with it
 * Works if the length of the text >= the length of the completion key
 *
 * \param entry
 *
 * \return
 **/
static void gtk_combofix_completion_choose_first_item (GtkWidget *entry)
{
	GtkEntryCompletion *completion;
	GtkTreeModel *model;
	GtkTreeIter iter;
	const gchar *key;
	gchar *string;

	completion = gtk_entry_get_completion (GTK_ENTRY (entry));
	model = gtk_entry_completion_get_model (completion);
	key = gtk_entry_get_text (GTK_ENTRY (entry));

	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &string, -1);
			if (string && g_str_has_prefix (string, key))
			{
				gtk_entry_set_text (GTK_ENTRY (entry), string);
				g_free (string);
				break;
			}
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}
}

/**
 * insert un item dans la completion
 *
 * \param
 * \param
 *
 * \return
 **/
static void gtk_combofix_completion_insert_new_item (GtkComboFix *combofix,
													 const gchar *text)
{
	GtkEntryCompletion *completion;
	GtkTreeModel *store;
	GtkTreeIter iter;
	GtkTreeIter new_iter;
    GtkComboFixPrivate *priv;

	priv = gtk_combofix_get_instance_private (combofix);

	completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
	store = gtk_entry_completion_get_model (completion);

	if (gtk_tree_model_get_iter_first (store, &iter))
	{
		do
		{
			gchar *tmp_str;
			gchar *str_to_free1;
			gchar *str_to_free2;

			gtk_tree_model_get (store, &iter, 0, &tmp_str, -1);
			if (!tmp_str)
				continue;

			str_to_free1 = g_utf8_casefold (text, -1);
			str_to_free2 = g_utf8_casefold (tmp_str, -1);
			if (g_utf8_collate (str_to_free1, str_to_free2) < 0)
			{
				gtk_list_store_insert_before (GTK_LIST_STORE (store), &new_iter, &iter);
				gtk_list_store_set (GTK_LIST_STORE (store), &new_iter, 0, text, -1);
				g_free (str_to_free1);
				g_free (str_to_free2);
				g_free (tmp_str);

				return;
			}
			g_free (str_to_free1);
			g_free (str_to_free2);
			g_free (tmp_str);
		}
		while (gtk_tree_model_iter_next (store, &iter));

		/* si on est là c'est qu'on est à la fin de la liste on ajoute le texte */
		gtk_list_store_append (GTK_LIST_STORE (store), &new_iter);
		gtk_list_store_set (GTK_LIST_STORE (store), &new_iter, 0, text, -1);
	}
	else
	{
		gtk_list_store_append (GTK_LIST_STORE (store), &new_iter);
		gtk_list_store_set (GTK_LIST_STORE (store), &new_iter, 0, text, -1);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean  gtk_combofix_completion_match_func (GtkEntryCompletion *completion,
													 const gchar *key,
													 GtkTreeIter *iter,
													 gpointer user_data)
{
	GtkTreeModel *model;
	gchar *new_key;
	gchar *new_text;
	const gchar *search;
	gchar *text;
	gchar *tmp_text;
	gssize nbre_bytes;

	model = gtk_entry_completion_get_model (completion);
	gtk_tree_model_get (model, iter, 0, &text, -1);

	if (!text)
		return FALSE;

	search = gtk_entry_get_text (GTK_ENTRY (gtk_entry_completion_get_entry (completion)));
	if (!search)
		return FALSE;

	new_key = utils_str_remove_accents (search);
	tmp_text = utils_str_remove_accents (text);
	nbre_bytes = strlen (new_key);
	new_text = g_strndup (tmp_text, nbre_bytes);
	g_free (tmp_text);

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
 * supprime le séparateur pour les états comme tiers
 *
 * \param model
 * \param iter_parent
 *
 * \return the position of parent_iter
 **/
static void gtk_combofix_remove_for_report (GtkTreeModel *model,
                                            GtkTreeIter *iter_parent)
{
    GtkTreeIter iter;
    gboolean separator;
    gboolean valid;

    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(model), &iter);
    while (valid)
    {
        gtk_tree_model_get (GTK_TREE_MODEL(model),
							&iter,
							COMBOFIX_COL_SEPARATOR, &separator,
							-1);
        if (separator)
        {
            break;
        }
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &iter);
    }

    gtk_tree_store_remove (GTK_TREE_STORE (model), iter_parent);
    gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
}

/**
 * vérifie si il existe un séparateur, l'ajoute si nécessaire
 *
 * \param model
 *
 * \return TRUE si un séparateur a été ajouté, FALSE sinon
 **/
static gboolean gtk_combofix_search_for_report (GtkTreeModel *model)
{
    GtkTreeIter iter;
    gchar *tmp_str;
    gboolean separator;
    gboolean valid;

    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(model), &iter);
    while (valid)
    {
        gtk_tree_model_get (GTK_TREE_MODEL(model),
							&iter,
							COMBOFIX_COL_SEPARATOR, &separator,
							-1);

        if (separator)
        {
            if (gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &iter))
				report_parent_iter = iter;

            return FALSE;
        }
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &iter);
    }

    gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (model),
                        &iter,
                        COMBOFIX_COL_LIST_NUMBER, 0,
                        COMBOFIX_COL_SEPARATOR, TRUE,
                        -1);

    tmp_str = g_strdup (_("Report"));
    gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (model),
                        &iter,
                        COMBOFIX_COL_VISIBLE_STRING, tmp_str,
                        COMBOFIX_COL_REAL_STRING, tmp_str,
                        COMBOFIX_COL_VISIBLE, TRUE,
                        COMBOFIX_COL_LIST_NUMBER, 1,
                        -1);
    g_free (tmp_str);
	report_parent_iter = iter;

    return TRUE;
}

/**
 * vérifie si l'état dont le nom est passé en paramètre existe
 *
 * \param model
 * \param report_name
 *
 * \return TRUE if exist or FALSE
 **/
static gboolean gtk_combofix_search_report (GtkTreeModel *model,
                                            const gchar *report_name)
{
    GtkTreeIter iter;

    if (!gtk_tree_model_iter_has_child (GTK_TREE_MODEL (model), &report_parent_iter))
        return FALSE;

    if (!gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter, &report_parent_iter))
        return FALSE;
    do
    {
        gchar *tmp_str = NULL;

        gtk_tree_model_get (GTK_TREE_MODEL(model),
							&iter,
							COMBOFIX_COL_VISIBLE_STRING, &tmp_str,
							-1);

        if (tmp_str)
        {
            if (g_utf8_collate (tmp_str, report_name) == 0)
            {
                g_free (tmp_str);
                return TRUE;
            }
            g_free (tmp_str);
        }
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

    return FALSE;
}

/**
 * vérifie si la chaine text existe déjà
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return TRUE si trouvé FALSE autrement
 **/
static gboolean gtk_combofix_search_for_text (GtkTreeModel *model,
											  GtkTreePath *path,
											  GtkTreeIter *iter,
											  gpointer *data)
{
    gchar *tmp_str;
    gboolean case_sensitive;
    gboolean separator;
    gint return_value;

    gtk_tree_model_get (GTK_TREE_MODEL(model),
						iter,
			            COMBOFIX_COL_REAL_STRING, &tmp_str,
                        COMBOFIX_COL_SEPARATOR, &separator,
			            -1);

    if (separator)
    {
        g_free (tmp_str);
        return FALSE;
    }

    case_sensitive = GPOINTER_TO_INT (data[2]);
    if (case_sensitive)
        return_value = !strcmp ((gchar *) data[0], tmp_str);
    else
        return_value = !g_utf8_collate (g_utf8_casefold ((gchar *) data[0], -1),
                                         g_utf8_casefold (tmp_str, -1));
    if (return_value)
        data[1] = GINT_TO_POINTER (1);

    return return_value;
}

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
static gboolean gtk_combofix_fill_iter_parent (GtkTreeStore *store,
											   GtkTreeIter *iter_parent,
                        					   const gchar *string,
                        					   gint list_number)
{
    gtk_tree_store_append (store, iter_parent, NULL);
    gtk_tree_store_set (store,
						iter_parent,
						COMBOFIX_COL_VISIBLE_STRING, string,
						COMBOFIX_COL_REAL_STRING, string,
						COMBOFIX_COL_VISIBLE, TRUE,
						COMBOFIX_COL_LIST_NUMBER, list_number,
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
static gboolean gtk_combofix_fill_iter_child (GtkTreeStore *store,
											  GtkTreeIter *iter_parent,
                        					  const gchar *string,
                        					  const gchar *real_string,
                        					  gint list_number)
{
    GtkTreeIter iter_child;

    gtk_tree_store_append (store, &iter_child, iter_parent);
    gtk_tree_store_set (store,
                        &iter_child,
                        COMBOFIX_COL_VISIBLE_STRING, string,
                        COMBOFIX_COL_REAL_STRING, real_string,
                        COMBOFIX_COL_VISIBLE, TRUE,
                        COMBOFIX_COL_LIST_NUMBER, list_number,
                        -1);

    return TRUE;
}

/**
 * fill the model of the combofix given in param
 * with the list given in param
 * carreful : the list is not cleared, so if needed, must do it before
 *
 * \param				combofix
 * \param list			a g_slist of strings
 * \param list_number	the number of the list for a complex, 0 else
 *
 * \return TRUE ok, FALSE pb
 **/
static gboolean gtk_combofix_fill_store (GtkComboFix *combofix,
                        				 GSList *list,
										 gint list_number)
{
    GSList *tmp_list;
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_store;
    GtkTreeIter iter_parent;
	gchar *free_str1;
    gchar *last_parent = NULL;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);
    if (!list)
	    return FALSE;

    /* normally the list cannot begin by a child, but we check here to
     * avoid a big crash */

    if (list->data && ((gchar *) (list->data))[0] == '\t')
    {
        gboolean FAILED = TRUE;

        g_print ("GtkComboFix error : the first entry in the list is a child, cannot fill the combofix\n");
        g_return_val_if_fail (FAILED, FALSE);
    }

	completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
	completion_store = gtk_entry_completion_get_model (completion);

	free_str1 = g_utf8_casefold (_("Report"), -1);
    tmp_list = list;

    while (tmp_list)
    {
        gchar *string;
        gchar* tmp_str;

        string = tmp_list->data;

        /* create the new iter where it's necessary and iter will focus on it */
        if (string)
        {
			GtkTreeIter new_iter;

            if (string[0] == '\t')
            {
				if (last_parent)
				{
					/* it's a child */
					tmp_str = g_strconcat (last_parent, " : ", string + 1, NULL);
					gtk_combofix_fill_iter_child (priv->store, &iter_parent, string + 1, tmp_str, list_number);

					/* append a row in the completion */
					gtk_list_store_append (GTK_LIST_STORE (completion_store), &new_iter);
					gtk_list_store_set (GTK_LIST_STORE (completion_store), &new_iter, 0, tmp_str, -1);
					g_free (tmp_str);
				}
				else
					alert_debug("Wrong format\n");
            }
            else
            {
                /* it's a parent */
                gtk_combofix_fill_iter_parent (priv->store, &iter_parent, string, list_number);
				/* append a row in the completion ignore reports for payees */
				if (priv->type == METATREE_PAYEE)
				{
					gchar *free_str2;

					free_str2 = g_utf8_casefold (string, -1);
					if (g_utf8_collate (free_str1, free_str2))
					{
						gtk_list_store_append (GTK_LIST_STORE (completion_store), &new_iter);
						gtk_list_store_set (GTK_LIST_STORE (completion_store), &new_iter, 0, string, -1);
					}
					g_free (free_str2);
				}
				else
				{
					/* on n'affiche pas la tête de categ/IB dans la completion si force Categ/IB est TRUE */
					/* et il existe des sous catégories. On traite les listes credit et débit */
					if (priv->type && priv->force && list_number < 2)
					{
						gint div_number;
						gint nbre_sub_division = 0;

						if (priv->type == METATREE_CATEGORY)
						{
							div_number = gsb_data_category_get_number_by_name (string, priv->force, priv->type);
							nbre_sub_division = gsb_data_category_get_sub_category_list_length (div_number);
						}
						else
						{
							div_number = gsb_data_budget_get_number_by_name (string, priv->force, priv->type);
							nbre_sub_division = gsb_data_budget_get_sub_budget_list_length (div_number);
						}

						if (nbre_sub_division == 0)
						{
							gtk_list_store_append (GTK_LIST_STORE (completion_store), &new_iter);
							gtk_list_store_set (GTK_LIST_STORE (completion_store), &new_iter, 0, string, -1);
						}
					}
					else
					{
						gtk_list_store_append (GTK_LIST_STORE (completion_store), &new_iter);
						gtk_list_store_set (GTK_LIST_STORE (completion_store), &new_iter, 0, string, -1);
					}
				}

                last_parent = string;
            }
        }

        tmp_list = tmp_list->next;
    }
	g_free (free_str1);

    priv->visible_items += g_slist_length (list);

    return TRUE;
}

/**
 * set all the rows of the list to be showed
 *
 * \param combofix
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_set_all_visible_rows (GtkComboFix *combofix)
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    gint path_ok;
    GtkComboFixPrivate *priv;

	if (!combofix)
		return FALSE;

	priv = gtk_combofix_get_instance_private (combofix);
    priv->visible_items = 0;
    model = GTK_TREE_MODEL (priv->store);
    path = gtk_tree_path_new_first ();
    path_ok = gtk_tree_model_get_iter (model, &iter, path);

    while (path_ok)
    {
        gint value;

        /* if mixed_sort is set, we don't show any separator line */
        if (priv->mixed_sort)
        {
            gint separator;

            gtk_tree_model_get (GTK_TREE_MODEL (model),
								&iter,
								COMBOFIX_COL_SEPARATOR, &separator,
								-1);

	    if (separator)
		    value = FALSE;
	    else
		    value = TRUE;
	}
	else
	    value = TRUE;

        gtk_tree_store_set (GTK_TREE_STORE (model),
							&iter,
							COMBOFIX_COL_VISIBLE, value,
							-1);

        priv->visible_items++;

        /* increment the path */
        if (gtk_tree_model_iter_has_child (model, &iter))
            gtk_tree_path_down (path);
        else
            gtk_tree_path_next (path);

        path_ok = gtk_tree_model_get_iter (model, &iter, path);

        /* if path_ok is FALSE, perhaps we are on the end of the children list... */
        if (!path_ok && gtk_tree_path_get_depth (path) > 1)
        {
            gtk_tree_path_up (path);
            gtk_tree_path_next (path);
            path_ok = gtk_tree_model_get_iter (model, &iter, path);
        }
    }
    gtk_tree_view_expand_all (GTK_TREE_VIEW (priv->tree_view));

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gint gtk_combofix_get_screen_height (GtkComboFix *combofix,
											gint y)
{
	GdkWindow *window;
	GdkDisplay *display;
	GdkMonitor *monitor;
	GdkRectangle rectangle;

	window = gtk_widget_get_window (GTK_WIDGET (combofix));
	display = gdk_window_get_display (window);
	monitor = gdk_display_get_monitor_at_point (display, 0, y);
	gdk_monitor_get_geometry (monitor, &rectangle);

	return rectangle.height;
}

/**
 * set the position and the size of the popup
 *
 * \param combofix
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_set_popup_position (GtkComboFix *combofix)
{
	GtkWidget *form_transaction_part;
    gint x = 0;
    gint y = 0;
    gint height;
    gint row_spacing;
    gint num_row;
    GdkRectangle rectangle;
    GtkAllocation allocation;
    gint horizontal_separator;
	gint screen_height;
    GtkComboFixPrivate *priv;

    if (!combofix)
        return FALSE;

    priv = gtk_combofix_get_instance_private (combofix);

    /* get the position of the combofix */
    /* en fait il semble qu'on récupère toujours la position de "form_transaction_part" */
    gdk_window_get_origin (gtk_widget_get_window (priv->entry), &x, &y);

    /* get the allocation of combofix */
    gtk_widget_get_allocation (GTK_WIDGET (combofix), &allocation);

    /* on corrige le bug (?) de gdk_window_get_origin () */
	form_transaction_part = gsb_form_get_form_transaction_part ();
    row_spacing = gtk_grid_get_row_spacing (GTK_GRID (form_transaction_part));
    num_row = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (combofix), "num_row"));
    if (num_row)
        y += (num_row * (allocation.height + row_spacing));

    gtk_widget_style_get (GTK_WIDGET (priv->tree_view),
						  "horizontal-separator", &horizontal_separator,
						  NULL);

    if (gtk_widget_get_realized (priv->tree_view))
    {
        gtk_tree_view_get_cell_area (GTK_TREE_VIEW (priv->tree_view),
									 gtk_tree_path_new_first (),
									 NULL,
									 &rectangle);
        /* the 4 is found at home, a good number to avoid the scrollbar with 1 item */
        height = (priv->visible_items) * (rectangle.height + horizontal_separator) + 4;
    }
    else
    {
        height = (priv->visible_items) * (allocation.height + horizontal_separator) + 4;
    }

    /* if the popup is too small to contain all, we check to set it on the bottom or on the top
     * if the place on the top is more than 2 times bigger than the bottom, we set it on the top */

	screen_height = gtk_combofix_get_screen_height (combofix, y);

    if (((screen_height - y - allocation.height) < height)
     &&
     (((screen_height - y) * 2) <= y))
    {
        /* popup on the top */
        if (y > height)
            y = y - height;
        else
        {
            height = y;
            y = 0;
        }
    }
    else
    {
        /* popup on the bottom */
        y += allocation.height;

        if ((screen_height - y) < height)
            height = screen_height - y;
    }

    gtk_window_move (GTK_WINDOW (priv->popup), x, y);
    gtk_window_resize (GTK_WINDOW (priv->popup), allocation.width, height);

    return FALSE;
}

/**
 * called when the popup is exposed, used to set the selection
 * because don't work if set directly after the entry_set
 *
 * \param combofix
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_expose_entry (GtkComboFix *combofix)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

	if (block_expose_event)
	    return FALSE;

    block_expose_event = 1;

    gtk_editable_select_region (GTK_EDITABLE (priv->entry),
								gtk_editable_get_position (GTK_EDITABLE (priv->entry)),
								-1);
    return FALSE;
}

/**
 * called for a button press while the popup is showed
 * if the mouse is outside the popup, hide it
 *
 * \param popup
 * \param ev
 * \param combofix
 *
 * \return TRUE if we are on the popup, FALSE else
 **/
static gboolean gtk_combofix_button_press (GtkWidget *popup,
										   GdkEventButton *ev,
										   GtkComboFix *combofix)
{
    GtkAllocation allocation;

    gtk_widget_get_allocation (popup, &allocation);

    if ((ev->x_root > allocation.x)
		&& (ev->x_root < (allocation.x +  allocation. width))
		&& (ev->y_root > allocation.y)
		&& (ev->x_root < (allocation.y +allocation. height)))
        return TRUE;

    gtk_widget_hide (popup);

    return FALSE;
}

/**
 * called when the entry receive a focus out event
 * hide the popup and check the content of the entry if force is set
 *
 * \param entry
 * \param ev
 * \param combofix
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_focus_out (GtkWidget *entry,
										GdkEvent *ev,
										GtkComboFix *combofix)
{
	gtk_combofix_hide_popup (combofix);

    /* fix bug 2208 */
    //~ gtk_editable_select_region (GTK_EDITABLE (entry), 0, 0);

    return (FALSE);
}

/**
 * called when the entry receive a focus in event
 *
 *
 * \param entry
 * \param ev
 * \param combofix
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_focus_in (GtkWidget *entry,
									   GdkEvent *ev,
									   GtkComboFix *combofix)
{
    const gchar *text;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    text = gtk_entry_get_text (GTK_ENTRY (priv->entry));

    if (priv->old_entry && strlen (priv->old_entry))
        g_free (priv->old_entry);

    if (text && strlen (text))
        priv->old_entry = g_strdup (text);
    else
        priv->old_entry = NULL;

    return (FALSE);
}

/**
 * the popup need to be modal to work fine, but the entry won't receive
 * some signal anymore... that function continue the signal to the entry
 *
 * \param popup
 * \param ev
 * \param combofix
 *
 * \return FALSE or TRUE according to the entry key press event return
 **/
static gboolean gtk_combofix_popup_key_press_event (GtkWidget *popup,
													GdkEventKey *ev,
													GtkComboFix *combofix)
{
    gboolean return_val;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    g_signal_emit_by_name (priv->entry,
						   "key-press-event",
						   ev,
						   &return_val);
    return return_val;
}

/**
 * get the selected item and fill the entry with it
 *
 * \param combofix
 *
 * \return TRUE if ok, FALSE if no selection
 **/
static gboolean gtk_combofix_choose_selection (GtkComboFix *combofix)
{
    GtkTreeIter iter;
    gchar *string;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    /* if there is no selection, go away */
    if (!gtk_tree_selection_get_selected (priv->selection, NULL, &iter))
	    return FALSE;

    gtk_tree_model_get (GTK_TREE_MODEL (priv->model_sort),
						&iter,
						COMBOFIX_COL_REAL_STRING, &string,
						-1);

    if (string && strlen (string))
        gtk_combofix_set_text (combofix, string);

    return TRUE;
}

/**
 * move the iter given in param of 1 step up or down and
 * go into the children if necessary
 *
 * \param model the tree model
 * \param iter a pointer to the iter to move
 * \param direction COMBOFIX_DOWN or COMBOFIX_UP
 *
 * \return TRUE ok, FALSE no change
 **/
static gboolean gtk_combofix_move_selection_one_step (GtkComboFix *combofix,
													  GtkTreeIter *iter,
													  gint direction)
{
    gint result = 0;
    GtkTreePath *path;
    GtkTreePath *saved_path;
    GtkTreeModel *model;
    gint separator;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);
    model = priv->model_sort;
    path = gtk_tree_model_get_path (model, iter);
    saved_path = gtk_tree_path_copy (path);

    switch (direction)
    {
		case COMBOFIX_DOWN:
			do
			{
				if (gtk_tree_model_iter_has_child (model, iter)
					&& gtk_tree_view_row_expanded (GTK_TREE_VIEW (priv->tree_view), path))
					gtk_tree_path_down (path);
				else
					gtk_tree_path_next (path);

				result = gtk_tree_model_get_iter (model, iter, path);

				/* if result is FALSE, perhaps we are on the end of the children list... */
				if (!result && gtk_tree_path_get_depth (path) > 1)
				{
					gtk_tree_path_up (path);
					gtk_tree_path_next (path);
					result = gtk_tree_model_get_iter (model, iter, path);
				}

				/* check if we are not on a separator */
				if (result)
					gtk_tree_model_get (model,
										iter,
										COMBOFIX_COL_SEPARATOR, &separator,
										-1);
				else
					separator = 0;
			}
			while (separator);
			break;

		case COMBOFIX_UP:
			do
			{
				result = gtk_tree_path_prev (path);

				if (result)
				{
					/* there is a prev path, but now, if we are on a parent, go to the last child,
					 * else, stay there */
					result = gtk_tree_model_get_iter (model, iter, path);

					if (result
						&& gtk_tree_model_iter_has_child (model, iter)
						&& gtk_tree_view_row_expanded (GTK_TREE_VIEW (priv->tree_view), path))
					{
						/* there is some children, go to the last one */
						gint i;

						gtk_tree_path_down (path);

						for (i = 0 ; i < gtk_tree_model_iter_n_children (model, iter) - 1 ; i++)
							gtk_tree_path_next (path);

						result = gtk_tree_model_get_iter (model, iter, path);
					}
				}
				else
				{
					/* there is no prev path, if we are not on the toplevel, go to the
					 * parent */

					if (gtk_tree_path_get_depth (path) > 1)
					{
						gtk_tree_path_up (path);
						result = gtk_tree_model_get_iter (model, iter, path);
					}
				}
				/* check if we are not on a separator */
				if (result)
					gtk_tree_model_get (model,
										iter,
										COMBOFIX_COL_SEPARATOR, &separator,
										-1);
				else
					separator = 0;
			}
			while (separator);
			break;
    }

    gtk_tree_path_free (path);

    /* if result is FALSE, iter was changed so set it to its initial value */
    if (!result)
        gtk_tree_model_get_iter (model, iter, saved_path);

    gtk_tree_path_free (saved_path);

    return result;
}

/**
 * return the number of visible rows showed on a page in the popup
 *
 * \param combofix
 *
 * \return the number of visible rows, 0 if problem
 **/
static gint gtk_combofix_get_rows_number_by_page (GtkComboFix *combofix)
{
    gint return_value;
    GtkAdjustment *adjustment;
    GtkComboFixPrivate *priv;

    if (!combofix)
        return 0;

    priv = gtk_combofix_get_instance_private (combofix);
    adjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (priv->tree_view));
    return_value = priv->visible_items
                        * gtk_adjustment_get_page_size (adjustment)
                        / gtk_adjustment_get_upper (adjustment);

    return return_value;
}

/**
 * called to move the selection in the tree_view
 * didn't succeed to give the focus to the tree_view so must do
 * this manual
 *
 * \param combofix
 * \param direction a combofix_key_direction
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_move_selection (GtkComboFix *combofix,
											 gint direction)
{
    GtkTreeIter sorted_iter;
    gint result = 0;
    GtkComboFixPrivate *priv;

    if (!combofix)
	    return FALSE;

    priv = gtk_combofix_get_instance_private (combofix);

    if (gtk_tree_selection_get_selected (priv->selection, NULL, &sorted_iter))
    {
        /* there is already a selection */
        gint i;

        switch (direction)
        {
            case COMBOFIX_DOWN:
				result = gtk_combofix_move_selection_one_step (combofix,
										&sorted_iter,
										COMBOFIX_DOWN);
				break;

            case COMBOFIX_UP:
				result = gtk_combofix_move_selection_one_step (combofix,
															   &sorted_iter,
															   COMBOFIX_UP);
				break;

            case COMBOFIX_PAGE_DOWN:
				for (i=0 ; i<gtk_combofix_get_rows_number_by_page (combofix) ; i++)
					result = result | gtk_combofix_move_selection_one_step (combofix,
																			&sorted_iter,
																			COMBOFIX_DOWN);
				break;

            case COMBOFIX_PAGE_UP:
				for (i=0 ; i<gtk_combofix_get_rows_number_by_page (combofix) ; i++)
					result = result | gtk_combofix_move_selection_one_step (combofix,
																			&sorted_iter,
																			COMBOFIX_UP);
				break;
        }
    }
    else
    {
        /* there is no current selection,
         * get the first selectable line */
        gint separator = 0;
        do
        {
            if (separator)
				result = gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->model_sort), &sorted_iter);
            else
				result = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->model_sort), &sorted_iter);
            if (result)
				gtk_tree_model_get (GTK_TREE_MODEL (priv->model_sort),
									&sorted_iter,
									COMBOFIX_COL_SEPARATOR, &separator,
									-1);
            else
				separator = 0;
        }
        while (separator);
	}

	if (result)
	{
        GtkTreePath *path;

        gtk_tree_selection_select_iter (priv->selection, &sorted_iter);
        path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->model_sort), &sorted_iter);
        if (path)
        {
            gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->tree_view), path, NULL, FALSE, 0, 0);
            gtk_tree_path_free (path);
        }
    }

    return FALSE;
}

/**
 * called for a key-press-event on the entry of the combofix
 *
 * \param entry
 * \param ev
 * \param combofix
 *
 * \return FALSE or TRUE, depends if need to block the signal
 **/
static gboolean gtk_combofix_key_press_event (GtkWidget *entry,
											  GdkEventKey *ev,
											  GtkComboFix *combofix)
{
	GrisbiAppConf *a_conf;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

    switch (ev->keyval)
    {
		case GDK_KEY_ISO_Left_Tab:
		case GDK_KEY_Tab :
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			/* we get the current selection */
			if (gtk_widget_get_visible (priv->popup)
				&& strlen (gtk_entry_get_text (GTK_ENTRY (priv->entry))) == 0)
			{
			   if (!gtk_combofix_choose_selection (combofix))
				{
					/* here we did entry key, but no selection... so
					 * keep the current completion */
					gtk_combofix_hide_popup (combofix);
					gtk_editable_select_region (GTK_EDITABLE (priv->entry), 0, 0);
				}
			}
			else if ((gint) strlen (gtk_entry_get_text (GTK_ENTRY (priv->entry))) >=
					 a_conf->completion_minimum_key_length)
			{
				gtk_combofix_completion_choose_first_item (entry);
			}
			/* le traitement de ENTER est fait dans le formulaire */
			return FALSE;
			break;

		case GDK_KEY_Escape:
			if (gtk_widget_get_visible (priv->popup))
			{
				gtk_combofix_hide_popup (combofix);
				gtk_combofix_set_text (combofix, priv->old_entry);
				gtk_editable_select_region (GTK_EDITABLE (priv->entry), 0, 0);
				return TRUE;
			}
			break;

		case GDK_KEY_Down :
		case GDK_KEY_KP_Down :
			/* show the popup if necessary */
			if (!gtk_widget_get_visible (priv->popup))
				gtk_combofix_show_popup (combofix);

			gtk_combofix_move_selection (combofix, COMBOFIX_DOWN);
			gtk_combofix_choose_selection (combofix);
			return TRUE;
			break;

		case GDK_KEY_Up :
		case GDK_KEY_KP_Up :
			/* move the selection up in the combofix only if the popup is showed,
			 * else let the program works with the upper key */
			if (gtk_widget_get_visible (priv->popup))
			{
				gtk_combofix_move_selection (combofix, COMBOFIX_UP);
				gtk_combofix_choose_selection (combofix);
				return TRUE;
			}
			break;

		case GDK_KEY_Page_Up :
		case GDK_KEY_KP_Page_Up :
			/* show the popup if necessary */
			if (!gtk_widget_get_visible (priv->popup))
				gtk_combofix_show_popup (combofix);

			gtk_combofix_move_selection (combofix, COMBOFIX_PAGE_UP);
			gtk_combofix_choose_selection (combofix);
			return TRUE;
			break;

		case GDK_KEY_Page_Down :
		case GDK_KEY_KP_Page_Down :
			/* show the popup if necessary */
			if (!gtk_widget_get_visible (priv->popup))
				gtk_combofix_show_popup (combofix);

			gtk_combofix_move_selection (combofix, COMBOFIX_PAGE_DOWN);
			gtk_combofix_choose_selection (combofix);
			return TRUE;
			break;
    }
    return FALSE;
}

/**
 * Called when a button press event is triggered on the tree view.
 * Select an entry if clicked.
 *
 * \param tree_view	GtkTreeView that triggered event.  It should be the tree view
			attached to a gtk combofix.
 * \param ev		Triggered event.
 * \param combofix	The GtkComboFix that contains tree view.
 *
 * \return TRUE to block the signal, FALSE else
 **/
static gboolean gtk_combofix_button_press_event (GtkWidget *tree_view,
												 GdkEventButton *ev,
												 GtkComboFix *combofix)
{
    if (ev->type ==  GDK_BUTTON_PRESS)
    {
        gtk_combofix_choose_selection (combofix);
        gtk_combofix_hide_popup (combofix);
        return TRUE;
    }

    return FALSE;
}

/**
 * this function is very important, called when the popup
 * gets the release event. without that, when click outside,
 * move the mouse on the combofix will select some string with clicking anything
 *
 * to avoid that we need to propagate the signal release-event to the entry of the combofix
 *
 * \param popup
 * \param ev
 * \param combofix
 *
 * \return the returned value of the release event signal propagated
 **/
static gboolean gtk_combofix_button_release_event (GtkWidget *popup,
												   GdkEventKey *ev,
												   GtkComboFix *combofix)
{
    gboolean return_val;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    g_signal_emit_by_name (priv->entry,
						   "button-release-event",
						   ev,
						   &return_val);
    return return_val;
}

/**
 * called to select the text in the tree_view
 *
 * \param combofix
 * \param item name of the item
 *
 * \return FALSE
 **/
static gboolean gtk_combofix_select_item (GtkComboFix *combofix,
										  const gchar *item)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path;
    gchar *ptr;
    gchar *tmp_item = NULL;
    gint result = 0;
    GtkComboFixPrivate *priv;

    if (!combofix)
	    return FALSE;
    if (!item || strlen (item) == 0)
        return FALSE;

    priv = gtk_combofix_get_instance_private (combofix);
    if ((ptr = g_utf8_strchr (item, -1, ':')))
        tmp_item = g_strndup (item, (ptr - item) -1);
    else
        tmp_item = g_strdup (item);

    model = GTK_TREE_MODEL (priv->model_sort);
    result = gtk_tree_model_get_iter_first (model, &iter);

    while (result)
    {
        gchar *tmp_str;

        gtk_tree_model_get (model, &iter, COMBOFIX_COL_REAL_STRING, &tmp_str, -1);

        if (tmp_str && tmp_item)
        {
            gchar *tmp_str_casefold, *tmp_item_casefold;
            int collate;

            tmp_str_casefold = g_utf8_casefold (tmp_str, -1);
            tmp_item_casefold = g_utf8_casefold (tmp_item, -1);
            collate = g_utf8_collate (tmp_str_casefold , tmp_item_casefold);
            g_free (tmp_item_casefold);
            g_free (tmp_str_casefold);

            if (collate  == 0)
                break;
        }

        result = gtk_tree_model_iter_next (model, &iter);
    }

    g_free (tmp_item);

    if (result == 0)
	{
        if (gtk_tree_model_get_iter_first (model, &iter))
		{
			gtk_tree_selection_select_iter (priv->selection, &iter);
			path = gtk_tree_model_get_path (model, &iter);
			if (path)
			{
				gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->tree_view), path, NULL, FALSE, 0, 0);
				gtk_tree_path_free (path);
			}
		}
	}

    return FALSE;
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
static gint gtk_combofix_default_sort_func (GtkTreeModel *model_sort,
											GtkTreeIter *iter_1,
											GtkTreeIter *iter_2,
											GtkComboFix *combofix)
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
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);
    if (iter_1)
        gtk_tree_model_get (GTK_TREE_MODEL (model_sort),
							iter_1,
							COMBOFIX_COL_LIST_NUMBER, &list_number_1,
							COMBOFIX_COL_VISIBLE_STRING, &string_1,
							COMBOFIX_COL_SEPARATOR, &separator_1,
							-1);
    else
        return -1;

    if (iter_2)
        gtk_tree_model_get (GTK_TREE_MODEL (model_sort),
							iter_2,
							COMBOFIX_COL_LIST_NUMBER, &list_number_2,
							COMBOFIX_COL_VISIBLE_STRING, &string_2,
							COMBOFIX_COL_SEPARATOR, &separator_2,
							-1);
    else
        return 1;

    if (priv->mixed_sort == FALSE)
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
static gboolean gtk_combofix_separator_func (GtkTreeModel *model,
											 GtkTreeIter *iter,
											 GtkComboFix *combofix)
{
    gboolean value;

    gtk_tree_model_get (GTK_TREE_MODEL (model), iter, COMBOFIX_COL_SEPARATOR, &value, -1);

    if (value)
	    return TRUE;
    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void gtk_combofix_create_button (GtkComboFix *combofix)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

	priv->button = utils_buttons_button_new_from_image ("gsb-arrow-down-16.png");
    gtk_button_set_relief (GTK_BUTTON (priv->button), GTK_RELIEF_NONE);
    g_signal_connect_swapped (G_OBJECT (priv->button),
							  "clicked",
							  G_CALLBACK (gtk_combofix_show_popup),
							  combofix);

	gtk_widget_show_all (priv->button);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void gtk_combofix_create_entry (GtkComboFix *combofix)
{
	GtkEntryCompletion *completion;
	GtkListStore *completion_store;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;
	GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_etat = grisbi_win_get_w_etat ();

	/* create entry */
    priv->entry = gtk_entry_new ();

	/* set completion */
	completion = gtk_entry_completion_new ();
	gtk_entry_completion_set_inline_selection (completion, TRUE);
	if (w_etat->combofix_case_sensitive)
		gtk_entry_completion_set_match_func (completion,
											 (GtkEntryCompletionMatchFunc) gtk_combofix_completion_match_func,
											 NULL,
											 NULL);
	gtk_entry_completion_set_minimum_key_length (completion, a_conf->completion_minimum_key_length);
	gtk_entry_completion_set_popup_single_match (completion, TRUE);
	gtk_entry_completion_set_text_column (completion, 0);

	/* set store */
	completion_store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (completion_store));

	/* set entry completion */
	gtk_entry_set_completion (GTK_ENTRY (priv->entry), completion);

	/* set entry signals */
    g_signal_connect (G_OBJECT (priv->entry),
                      "key-press-event",
                      G_CALLBACK (gtk_combofix_key_press_event),
                      combofix);
    g_signal_connect_swapped (G_OBJECT (priv->entry),
                        	  "draw",
                        	  G_CALLBACK (gtk_combofix_expose_entry),
                        	  combofix);
    g_signal_connect_after (G_OBJECT (priv->entry),
                        	"focus-in-event",
                        	G_CALLBACK (gtk_combofix_focus_in),
                        	combofix);
    g_signal_connect_after (G_OBJECT (priv->entry),
                        	"focus-out-event",
                        	G_CALLBACK (gtk_combofix_focus_out),
                        	combofix);

 	/* set completion signal */
   g_signal_connect (G_OBJECT (completion),
                      "match-selected",
                      G_CALLBACK (gtk_combofix_completion_match_selected),
                      combofix);

    gtk_widget_set_hexpand (priv->entry, TRUE);
    gtk_widget_show (priv->entry);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void gtk_combofix_create_popup (GtkComboFix *combofix)
{
    GtkWidget *frame;
    GtkCellRenderer *cell_renderer;
    GtkTreeViewColumn *tree_view_column;
    GtkWidget *scrolled_window;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    priv->popup = gtk_window_new (GTK_WINDOW_POPUP);
    g_object_ref (G_OBJECT (priv->popup));
    g_signal_connect (G_OBJECT (priv->popup),
                      "key-press-event",
                      G_CALLBACK (gtk_combofix_popup_key_press_event),
                      combofix);

    g_signal_connect (G_OBJECT (priv->popup),
                      "button-press-event",
                      G_CALLBACK (gtk_combofix_button_press),
                      combofix);
    g_signal_connect (G_OBJECT (priv->popup),
                      "button-release-event",
                      G_CALLBACK (gtk_combofix_button_release_event),
                      combofix);

    frame = gtk_frame_new (NULL);
    gtk_container_add (GTK_CONTAINER (priv->popup), frame);
    gtk_widget_show (frame);

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_window),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (frame), scrolled_window);
    gtk_widget_show (scrolled_window);

    /* Create the tree_store */
    priv->store = gtk_tree_store_new (COMBOFIX_N_COLUMNS,
									  G_TYPE_STRING,
									  G_TYPE_STRING,
									  G_TYPE_BOOLEAN,
									  G_TYPE_INT,
									  G_TYPE_BOOLEAN);

    /* we set the store in a filter to show only what is selected */
    priv->model_filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (priv->store), NULL);
    gtk_tree_model_filter_set_visible_column (GTK_TREE_MODEL_FILTER (priv->model_filter),
											  COMBOFIX_COL_VISIBLE);

    /* we set the filter in a sorting model */
    priv->model_sort = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (priv->model_filter));
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (priv->model_sort),
										  COMBOFIX_COL_VISIBLE_STRING,
										  GTK_SORT_ASCENDING);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (priv->model_sort),
									 COMBOFIX_COL_VISIBLE_STRING,
									 (GtkTreeIterCompareFunc) gtk_combofix_default_sort_func,
									 combofix,
									 NULL);

    /* make the column */
    cell_renderer = gtk_cell_renderer_text_new ();
    tree_view_column = gtk_tree_view_column_new_with_attributes ("",
																 cell_renderer,
																 "text", COMBOFIX_COL_VISIBLE_STRING,
																 NULL);
    gtk_tree_view_column_set_sizing (tree_view_column, GTK_TREE_VIEW_COLUMN_FIXED);

    /* set the sorting model in the tree view */
    priv->tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (priv->model_sort));

    priv->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree_view));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (priv->selection), GTK_SELECTION_SINGLE);
    gtk_tree_view_set_hover_selection (GTK_TREE_VIEW (priv->tree_view), TRUE);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree_view), FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree_view), tree_view_column);
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (priv->tree_view), TRUE);
    gtk_tree_view_set_row_separator_func (GTK_TREE_VIEW (priv->tree_view),
										  (GtkTreeViewRowSeparatorFunc) gtk_combofix_separator_func,
										  combofix,
										  NULL);

    g_signal_connect (G_OBJECT (priv->tree_view),
					  "button-press-event",
					  G_CALLBACK (gtk_combofix_button_press_event),
					  combofix);
    gtk_container_add (GTK_CONTAINER (scrolled_window), priv->tree_view);

    gtk_widget_show (priv->tree_view);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation de l'objet                            */
/******************************************************************************/
/**
 * called when create a new combofix
 *
 * \param combofix
 *
 * \return
 **/
static void gtk_combofix_init (GtkComboFix *combofix)
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    /* set the fields of the combofix */
    priv->force = FALSE;
    priv->case_sensitive = FALSE;
    priv->visible_items = 0;
	priv->ignore_accents = TRUE;		/* reproduit le fonctionnement de la completion de gtk */
	priv->minimum_key_length = 1;		/* la recherche commence au premier caractère */

    /* the combofix is a vbox */
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (combofix), vbox);
    gtk_widget_show (vbox);

    /* a hbox which contains the entry and the button */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);
    gtk_widget_show (hbox);

    /* set the entry with completion */
	gtk_combofix_create_entry (combofix);
    gtk_box_pack_start (GTK_BOX (hbox), priv->entry, TRUE, TRUE, 0);

    /* set the button */
	gtk_combofix_create_button (combofix);
    gtk_box_pack_start (GTK_BOX (hbox), priv->button, FALSE, FALSE, 0);

    /* set the popup but don't show it */
	gtk_combofix_create_popup (combofix);
}

/**
 * called when destroy a combofix
 *
 * \param combofix
 *
 * \return
 **/
static void gtk_combofix_dispose (GObject *combofix)
{
    G_OBJECT_CLASS (gtk_combofix_parent_class)->dispose (combofix);
}

/**
* called when destroy combofix
 *
 * \param combofix
 *
 * \return
 **/
static void gtk_combofix_finalize (GObject *combofix)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (GTK_COMBOFIX (combofix));

    if (priv->old_entry && strlen (priv->old_entry))
        g_free (priv->old_entry);

    /* Unref/free the model first, to workaround gtk/gail bug #694711 */
    gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree_view), NULL);
    g_object_unref (priv->model_sort);
    g_object_unref (priv->model_filter);
    g_object_unref (priv->store);

    gtk_widget_destroy (priv->popup);
    g_object_unref (priv->popup);

    G_OBJECT_CLASS (gtk_combofix_parent_class)->finalize (combofix);
}

/**
 * called when create a new combofix
 *
 * \param combofix
 *
 * \return
 **/
static void gtk_combofix_class_init (GtkComboFixClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = gtk_combofix_dispose;
    object_class->finalize = gtk_combofix_finalize;

	gtk_widget_class_set_css_name (GTK_WIDGET_CLASS (klass), "combofix");
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * create a combofix, with several list set one after the others
 * by default, force is not set, no max items
 * and case unsensitive
 *
 * \param list 		a g_slist of name (\t at the beginning makes it as a child)
 * \param type		0 : payee, 1 : category, 2 : budget
 *
 * \return the new widget
 **/
GtkWidget *gtk_combofix_new (GSList *list,
							 gint type)
{
    GtkComboFix *combofix;
    GtkComboFixPrivate *priv;

	combofix = g_object_new (GTK_TYPE_COMBOFIX, NULL);
    priv = gtk_combofix_get_instance_private (combofix);
	priv->type = type;

    gtk_combofix_set_list (combofix, list);

	gtk_widget_set_size_request (GTK_WIDGET (combofix), COMBOFIX_MIN_WIDTH, -1);

    return (GTK_WIDGET (combofix));
}

/**
 * create a new gtk_conbofix with properties
 *
 * \param list 		a g_slist of name (\t at the beginning makes it as a child)
 * \param force 	TRUE and the text must be in the list
 * \param sort 		TRUE and the list will be sorted automatickly
 * \param max_items	the minimum of characters to show the popup
 * \param type 		0 : payee, 1 : category, 2 : budget
 *
 * \return a gtkcombofix
 **/
GtkWidget *gtk_combofix_new_with_properties (GSList *list,
											 gboolean force_text,
											 gboolean case_sensitive,
											 gboolean mixed_sort,
											 gint type)
{
    GtkComboFix *combofix;
    GtkComboFixPrivate *priv;

	combofix = g_object_new (GTK_TYPE_COMBOFIX, NULL);
    priv = gtk_combofix_get_instance_private (combofix);

	priv->force = force_text;
    priv->case_sensitive = case_sensitive;
    priv->mixed_sort = mixed_sort;
	priv->type = type;

	gtk_combofix_set_list (combofix, list);
	gtk_widget_set_size_request (GTK_WIDGET (combofix), COMBOFIX_MIN_WIDTH, -1);

	/* fix_bug 2154 */
	if (priv->type == METATREE_PAYEE && priv->force)
	{
		GtkEntryCompletion *completion;

		completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
	   g_signal_connect (G_OBJECT (completion),
						 "no-matches",
						 G_CALLBACK (gtk_combofix_completion_no_match_selected),
						 priv);
	}

	return (GTK_WIDGET (combofix));
}

/**
 * get the entry in the combofix
 *
 * \param combofix
 *
 * \return a widget
 **/
GtkWidget *gtk_combofix_get_entry (GtkComboFix *combofix)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    g_return_val_if_fail (combofix, NULL);
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), NULL);

	return priv->entry;
}

/**
 * get the text in the combofix
 *
 * \param combofix
 *
 * \return a const gchar
 **/
const gchar *gtk_combofix_get_text (GtkComboFix *combofix)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    g_return_val_if_fail (combofix , NULL);
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), NULL);

    return (gtk_entry_get_text (GTK_ENTRY (priv->entry)));
}

/**
 * set the text in the combofix without showing the popup or
 * doing any check
 *
 * \param combofix
 * \param text
 *
 * \return
 **/
void gtk_combofix_set_text (GtkComboFix *combofix,
							const gchar *text)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

	/* bloque l'appel à gtk_combofix_expose_entry () pendant cette fonction */
	block_expose_event = 1;

    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    if (text && strlen (text) > 0)
        gtk_entry_set_text (GTK_ENTRY (priv->entry), text);
    else
        gtk_entry_set_text (GTK_ENTRY (priv->entry), "");

}

/**
 * set the properties of combofix
 *
 * \param 				combofix
 * \param				type of the combofix : 0 : payee, 1 : category, 2 : budget
 *
 * \return
 **/
void gtk_combofix_set_properties (GtkWidget *combofix)
{
	gint old_case_sensitive = 0;
	GrisbiWinEtat *w_etat;
	GtkComboFixPrivate *priv;

    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

	priv = gtk_combofix_get_instance_private (GTK_COMBOFIX (combofix));
	w_etat = grisbi_win_get_w_etat ();
	old_case_sensitive = priv->case_sensitive;

	if (priv->type)
	{
		priv->force = w_etat->combofix_force_category;
		priv->mixed_sort = w_etat->combofix_mixed_sort;
	}
	else
	{
		priv->force = w_etat->combofix_force_payee;
		priv->mixed_sort = FALSE;
	}
    priv->case_sensitive = w_etat->combofix_case_sensitive;
	if (old_case_sensitive - w_etat->combofix_case_sensitive)
	{
		GtkEntryCompletion *completion;

		completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
		if (old_case_sensitive)
		{
			gtk_entry_completion_set_match_func (completion, NULL, NULL, NULL);
		}
		else
		{
			gtk_entry_completion_set_match_func (completion,
												 (GtkEntryCompletionMatchFunc) gtk_combofix_completion_match_func,
												 NULL,
												 NULL);
		}
	}
}

/**
 * set the flag to force/unforce the text in the entry
 * if force is set, the value in the entry must belong to the list
 *
 * \param combofix
 * \param value
 *
 * \return
 **/
void gtk_combofix_set_force_text (GtkComboFix *combofix,
								  gboolean value)
{
    GtkComboFixPrivate *priv;

    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    priv = gtk_combofix_get_instance_private (combofix);
    priv->force = value;
}

/**
 * set if the completion is case sensitive or not
 *
 * \param combofix
 * \param case_sensitive TRUE or FALSE
 *
 * \return
 **/
void gtk_combofix_set_case_sensitive (GtkComboFix *combofix,
									  gboolean case_sensitive)
{
	gint old_case_sensitive = 0;
    GtkComboFixPrivate *priv;

    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    priv = gtk_combofix_get_instance_private (combofix);
	old_case_sensitive = priv->case_sensitive;
    priv->case_sensitive = case_sensitive;
	if (old_case_sensitive - case_sensitive)
	{
		GtkEntryCompletion *completion;

		completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
		if (old_case_sensitive)
		{
			gtk_entry_completion_set_match_func (completion, NULL, NULL, NULL);
		}
		else
		{
			gtk_entry_completion_set_match_func (completion,
												 (GtkEntryCompletionMatchFunc) gtk_combofix_completion_match_func,
												 NULL,
												 NULL);
		}
	}
}

/**
 * hide the popup
 *
 * \param combofix
 *
 * \return FALSE
 **/
gboolean gtk_combofix_hide_popup (GtkComboFix *combofix)
{
    GtkComboFixPrivate *priv;

    g_return_val_if_fail (combofix != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), FALSE);

    priv = gtk_combofix_get_instance_private (combofix);
    if (gtk_widget_get_visible (priv->popup))
    {
        gtk_grab_remove (priv->popup);
        gtk_widget_hide (priv->popup);
    }
    return FALSE;
}

/**
 * show the popup with all the content, not according to the entry
 *
 * \param combofix
 *
 * return FALSE
 **/
gboolean gtk_combofix_show_popup (GtkComboFix *combofix)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);
    if (gtk_widget_get_visible (priv->popup))
        return FALSE;

    gtk_combofix_set_all_visible_rows (combofix);
    gtk_combofix_set_popup_position (combofix);
    gtk_widget_show (priv->popup);
    gtk_combofix_select_item (combofix, gtk_combofix_get_text (combofix));
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gtk_window_set_modal (GTK_WINDOW (priv->popup), TRUE);
    return FALSE;
}

/**
 * set for the complex combofix if the different list have to
 * be mixed or separate
 *
 * \param combofix
 * \param mixed_sort TRUE or FALSE
 *
 * \return
 **/
void gtk_combofix_set_mixed_sort (GtkComboFix *combofix,
								  gboolean mixed_sort)
{
    GtkComboFixPrivate *priv;

    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    priv = gtk_combofix_get_instance_private (combofix);
    priv->mixed_sort = mixed_sort;
}

/**
 * change the list of an existing combofix
 *
 * \param combofix
 * \param list the new list
 *
 * \return TRUE if ok, FALSE if problem
 **/
gboolean gtk_combofix_set_list (GtkComboFix *combofix,
								GSList *list)
{
    GtkComboFixPrivate *priv;

    GSList *tmp_list;
    gint list_number = 0;
    gint length;
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_store;
    GtkTreeIter iter;

    g_return_val_if_fail (combofix, FALSE);
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), FALSE);
    g_return_val_if_fail (list, FALSE);

    priv = gtk_combofix_get_instance_private (combofix);
	if (!priv->store || !GTK_IS_TREE_STORE (priv->store))
		return  FALSE;
	else
	    gtk_tree_store_clear (priv->store);

	completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
	completion_store = gtk_entry_completion_get_model (completion);
	if (GTK_LIST_STORE (completion_store))
		gtk_list_store_clear (GTK_LIST_STORE (completion_store));

    tmp_list = list;
    length = g_slist_length (list);

    while (tmp_list)
    {
        gtk_combofix_fill_store (combofix, tmp_list->data, list_number);

        /* set the separator */
        if (list_number < (length-1))
        {
            gtk_tree_store_append (priv->store, &iter, NULL);
            gtk_tree_store_set (priv->store,
								&iter,
								COMBOFIX_COL_LIST_NUMBER, list_number,
								COMBOFIX_COL_SEPARATOR, TRUE,
								-1);
        }
        list_number++;

        tmp_list = tmp_list->next;
    }

    return TRUE;
}

/**
 * append a new line in a combofix
 *
 * \param combofix text
 *
 * \return
 **/
void gtk_combofix_append_text (GtkComboFix *combofix,
							   const gchar *text)
{
	GtkTreeIter iter_parent;
    gint empty;
    gpointer pointeurs[3] = { (gpointer) text, NULL, NULL };
    GtkComboFixPrivate *priv;

	if (!combofix || !GTK_IS_COMBOFIX (combofix))
		return;

    priv = gtk_combofix_get_instance_private (combofix);

	/* On sort pour les catégories/IB car la mise à jour est globale */
	if (priv-> type)
		return;

	pointeurs[2] = GINT_TO_POINTER (priv->case_sensitive);

    empty = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (priv->entry), "empty"));
	if (empty || priv->force)
		return;

	if (priv->old_entry && strcmp (text, priv->old_entry) == 0)
        return;

    gtk_tree_model_foreach (GTK_TREE_MODEL (priv->store),
							(GtkTreeModelForeachFunc) gtk_combofix_search_for_text,
							pointeurs);

	if (pointeurs[1] && GINT_TO_POINTER (pointeurs[1]))
		return;

	gtk_combofix_fill_iter_parent (priv->store, &iter_parent, text, 0);

    if (priv->old_entry && strlen (priv->old_entry))
        g_free (priv->old_entry);
    priv->old_entry = g_strdup (text);

	/* update completion */
	gtk_combofix_completion_insert_new_item (combofix, text);
}

/**
 * append a report as payee in a combofix
 *
 * \param combofix
 * \param report_name    the name of report
 *
 * \return
 **/
void gtk_combofix_append_report (GtkComboFix *combofix,
								 const gchar *report_name)
{
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_model;
	GtkTreeIter new_iter;
    gchar *tmp_str;
    gchar *tmp_str2;
    GtkComboFixPrivate *priv;

    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    if (!report_name || strlen (report_name) == 0)
        return;

    priv = gtk_combofix_get_instance_private (combofix);

	/* initialisation iter à invalid probablement inutile */
	report_parent_iter.stamp = 0;
    /* on cherche la partie etats on l'ajoute si nécessaire */
    if (gtk_combofix_search_for_report (GTK_TREE_MODEL (priv->store)))
        priv->visible_items++;

    if (!report_parent_iter.stamp)
        return;

    /* on sort si l'état demandé existe déjà */
    if (gtk_combofix_search_report (GTK_TREE_MODEL (priv->store), report_name))
        return;

    /* sinon on l'ajoute dans la liste des tiers */
    tmp_str = g_strdup (_("Report"));
    tmp_str2 = g_strconcat (tmp_str, " : ", report_name, NULL);
    gtk_combofix_fill_iter_child (priv->store, &report_parent_iter, report_name, tmp_str2, 1);
    priv->visible_items++;
	g_free (tmp_str);

	/* update completion */
	completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
	completion_model = gtk_entry_completion_get_model (completion);
	gtk_list_store_append (GTK_LIST_STORE (completion_model), &new_iter);
	gtk_list_store_set (GTK_LIST_STORE (completion_model), &new_iter, 0, tmp_str2, -1);
    g_free (tmp_str2);
}

/**
 * remove a line in a combofix
 *
 * \param combofix text
 *
 * \return
 **/
void gtk_combofix_remove_text (GtkComboFix *combofix,
							   const gchar *text)
{
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_model;
    GtkTreeIter iter;
    gboolean case_sensitive;
    gboolean valid;
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

	case_sensitive = priv->case_sensitive;
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->store), &iter);

    while (valid)
    {
        gchar *tmp_str;
        gboolean separator;

        gtk_tree_model_get (GTK_TREE_MODEL (priv->store),
							&iter,
							COMBOFIX_COL_REAL_STRING, &tmp_str,
							COMBOFIX_COL_SEPARATOR, &separator,
							-1);

        if (separator)
        {
            valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->store), &iter);
            continue;
        }
        if (case_sensitive && !strcmp (text, tmp_str))
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

        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->store), &iter);
    }

	if (valid)
		gtk_tree_store_remove (priv->store, &iter);

	/* update completion */
	completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
	completion_model = gtk_entry_completion_get_model (completion);
    valid = gtk_tree_model_get_iter_first (completion_model, &iter);

    while (valid)
    {
        gchar *tmp_str;

        gtk_tree_model_get (completion_model, &iter, 0, &tmp_str, -1);

        if (case_sensitive && !strcmp (text, tmp_str))
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


/**
 * remove a report in a payee combofix
 *
 * \param combofix
 * \param report_number
 *
 * \return
 **/
void gtk_combofix_remove_report (GtkComboFix *combofix,
								 const gchar *report_name)
{
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_model;
    GtkTreeIter iter;
    gchar *tmp_str;
    gchar *tmp_str2;
    gboolean valid;
	GrisbiWinEtat *w_etat;
    GtkComboFixPrivate *priv;

    /* on récupère le nom de l'état */
    tmp_str = g_strdup (_("Report"));
    tmp_str2 = g_strconcat (tmp_str, " : ", report_name, NULL);
    g_free (tmp_str);

    priv = gtk_combofix_get_instance_private (combofix);
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->store), &iter);

    while (valid)
    {
        gboolean separator;

        gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, COMBOFIX_COL_SEPARATOR, &separator, -1);
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->store), &iter);

        if (separator)
        {
            break;
        }
    }

    if (valid)
    {
        if (gtk_tree_model_iter_has_child (GTK_TREE_MODEL (priv->store), &iter))
        {
            gint children;
            GtkTreeIter child;

            children = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (priv->store), &iter);
            valid = FALSE;

            if (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->store), &child, &iter))
            {
                do
                {
                    gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &child,
										COMBOFIX_COL_REAL_STRING, &tmp_str,
										-1);

                    if (strcmp (tmp_str, tmp_str2) == 0)
                    {
                        g_free (tmp_str);
                        valid = TRUE;
                        children --;
                        break;
                    }

                    g_free (tmp_str);
                }
                while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->store), &child));

                if (valid)
                    gtk_tree_store_remove (priv->store, &child);
            }
            if (children == 0)
            {
                gtk_combofix_remove_for_report (GTK_TREE_MODEL (priv->store), &iter);
            }
        }
        else
            gtk_combofix_remove_for_report (GTK_TREE_MODEL (priv->store), &iter);
    }

	/* update completion */
	completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
	completion_model = gtk_entry_completion_get_model (completion);
	valid = gtk_tree_model_get_iter_first (completion_model, &iter);
	w_etat = grisbi_win_get_w_etat ();

    while (valid)
    {
        gtk_tree_model_get (completion_model, &iter, 0, &tmp_str, -1);

        if (w_etat->combofix_case_sensitive && !strcmp (tmp_str2, tmp_str))
        {
            g_free (tmp_str);
            break;
        }
        else if (!g_utf8_collate (g_utf8_casefold (tmp_str2, -1), g_utf8_casefold (tmp_str, -1)))
        {
            g_free (tmp_str);
            break;
        }

        g_free (tmp_str);

        valid = gtk_tree_model_iter_next (completion_model, &iter);
    }

    if (valid)
        gtk_list_store_remove (GTK_LIST_STORE (completion_model), &iter);
    g_free (tmp_str2);
}

/**
 *
 *
 * \param combofix text
 *
 * \return
 **/
void gtk_combofix_set_selection_callback (GtkComboFix *combofix,
										  GCallback func,
										  gpointer data)
{
    GtkComboFixPrivate *priv;

    priv = gtk_combofix_get_instance_private (combofix);

    if (func)
	    g_signal_connect (G_OBJECT (priv->selection),
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
