/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cedric Auger (cedric@grisbi.org)	          */
/*			2003-2009 Benjamin Drieu (bdrieu@april.org)	                      */
/*      2009 Thomas Peel (thomas.peel@live.fr)                                */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                 */
/* 			https://www.grisbi.org				                              */
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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */


/**
 * \file gsb_bank.c
 * do the bank configuration
 * do the bank combobox
 */


#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_bank.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "widgets/widget_bank_details.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_bank_edit_bank (gint bank_number,
									GtkWidget *combobox);

static GtkTreeModel *bank_list_model = NULL;
/*END_STATIC*/

/* the model containing the list of the banks for the combobox */
enum BankComboColumns
{
   BANK_NAME_COL = 0,
   BANK_NUMBER_COL
};

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * called when change the bank in the list
 * here just check if we want to create a new bank
 *
 * \param combo_box
 * \param null
 *
 * \return FALSE
 **/
static void gsb_bank_combo_list_changed (GtkWidget *combobox,
										 gpointer null)
{
    gint bank_number;

    if (!combobox)
	    return;

    bank_number = gsb_bank_combo_list_get_bank_number (combobox);

    /* check if not new bank, ie -1 */
    if (bank_number != -1)
    {
        gsb_data_account_set_bank (gsb_gui_navigation_get_current_account (), bank_number);

		/* Mark file as modified */
        gsb_file_set_modified (TRUE);

		return;
    }

    /* asked to add a new bank */
    gsb_bank_edit_bank (bank_number, combobox);
}

/**
 * create a window with a form containing all the parameters of the bank
 * and allow to modify the values
 * the form is the same in the property of banks, but in a window
 *
 * \param bank_number	the number of bank
 * \param combobox 		NULL or a pointer to a specific combobox to save the selection while editing the bank
 *
 * \return FALSE
 **/
static gboolean gsb_bank_edit_bank (gint bank_number,
									GtkWidget *combobox)
{
	GtkWidget *bank_details;
	GtkWidget *button_cancel;
	GtkWidget *button_apply;
    GtkWidget *dialog;
    GtkWidget *scrolled_window;
    GtkWidget *vbox;
    gint result;

	devel_debug_int (bank_number);
    dialog = gtk_dialog_new_with_buttons (_("Edit bank"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										  NULL, NULL,
										  NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_CANCEL);
	gtk_widget_set_can_default (button_cancel, TRUE);

	button_apply = gtk_button_new_with_label (_("Apply"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_apply, GTK_RESPONSE_APPLY);
	gtk_widget_set_can_default (button_apply, TRUE);

	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (dialog_get_content_area (dialog)), scrolled_window);
    gtk_widget_set_size_request (scrolled_window, 600, 400);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BOX_BORDER_WIDTH);
    gtk_container_add (GTK_CONTAINER (scrolled_window), vbox);

	/* set bank details */
	bank_details = GTK_WIDGET (widget_bank_details_new (NULL, combobox));
	gtk_box_pack_start (GTK_BOX (vbox), bank_details, FALSE, FALSE, 0);

	/* affiche le widget bank_details */
	if (gtk_widget_get_no_show_all (bank_details))
		gtk_widget_set_no_show_all (bank_details, FALSE);

    gtk_widget_show_all (dialog);

    if (bank_number == -1)	/* c'est une nouvelle banque */
    {
		widget_bank_details_select_name_entry (bank_number, bank_details);
    }
    else
        widget_bank_details_update_form	 (bank_number, bank_details);

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    if (result == GTK_RESPONSE_APPLY)
    {
        if (bank_number == -1)
            bank_number = gsb_data_bank_new (_("New bank"));

        /* on bloque la fonction de callback */
        g_signal_handlers_block_by_func (G_OBJECT (combobox),
                                         G_CALLBACK (gsb_bank_combo_list_changed),
                                         NULL);

		widget_bank_details_update_bank_data (bank_number, bank_details);
		gsb_bank_update_combo_list_model (combobox);
        gsb_bank_combo_list_set_bank (combobox, bank_number);

        /* Mark file as modified */
        gsb_file_set_modified (TRUE);

        /* on débloque la fonction de callback */
        g_signal_handlers_unblock_by_func (G_OBJECT (combobox),
                                           G_CALLBACK (gsb_bank_combo_list_changed),
                                           NULL);
    }
    else
    {
        if (bank_number == -1)
        {
            gint account_number;

            account_number = gsb_gui_navigation_get_current_account ();
            gsb_bank_combo_list_set_bank (combobox,
                        gsb_data_account_get_bank (account_number));
        }
    }

    gtk_widget_destroy (dialog);

    return FALSE;
}

/**
 * check for the separator line in the combobox (ie the name is null)
 *
 * \param model
 * \param iter
 * \param null
 *
 * \return TRUE if separator, FALSE if not
 **/
static gboolean gsb_bank_combo_list_check_separator (GtkTreeModel *model,
											   GtkTreeIter *iter,
											   gpointer null)
{
    gchar *value;

    gtk_tree_model_get (GTK_TREE_MODEL (model), iter, BANK_NAME_COL, &value, -1);

    if (value)
	{
		g_free (value);
		return FALSE;
	}
	else
		return TRUE;
}

/**
 * create the model for the combobox of banks
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
static void gsb_bank_create_combobox_model (void)
{
    bank_list_model = GTK_TREE_MODEL (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT));
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * create a combo_box with the name of all the banks and an 'add' line at the end
 *
 * \param bank the index we want to place the combobox
 *
 * \return GtkWidget * the combo_box
 **/
GtkWidget *gsb_bank_create_combobox (gint index)
{
    GtkWidget *combo_box;
    GtkCellRenderer *renderer;

    /* create the model if not done */
    if (!bank_list_model)
        gsb_bank_update_combo_list_model (NULL);

    combo_box = gtk_combo_box_new_with_model (bank_list_model);

    /* show the text column */
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box),
									renderer,
									"text", BANK_NAME_COL,
									NULL);

    /* create the separator */
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo_box),
										  (GtkTreeViewRowSeparatorFunc) gsb_bank_combo_list_check_separator,
										  NULL,
										  NULL);

    /* set the index */
    gsb_bank_combo_list_set_bank (combo_box, index);

    /* the signal just check if we select new bank, to show the dialog to add a new bank */
    g_signal_connect (G_OBJECT (combo_box),
					  "changed",
					  G_CALLBACK (gsb_bank_combo_list_changed),
					  NULL);

    return combo_box;
}

/**
 * create a combo with the name of all the banks and an 'add' line at the end from glade ui
 *
 * \param bank the index we want to place the combobox
 *
 * \return
 **/
void gsb_bank_new_combobox_from_ui (GtkWidget *combo,
									gint index)
{
    GtkCellRenderer *renderer;

    /* create the model if not done */
    if (!bank_list_model)
        gsb_bank_update_combo_list_model (NULL);

    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), bank_list_model);

    /* show the text column */
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
									renderer,
									"text", BANK_NAME_COL,
									NULL);

    /* create the separator */
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo),
										  (GtkTreeViewRowSeparatorFunc) gsb_bank_combo_list_check_separator,
										  NULL,
										  NULL);

    /* set the index */
    gsb_bank_combo_list_set_bank (combo, index);

    /* the signal just check if we select new bank, to show the dialog to add a new bank */
    g_signal_connect (G_OBJECT (combo),
					  "changed",
					  G_CALLBACK (gsb_bank_combo_list_changed),
					  NULL);
}

/**
 * return the number of the curently showed on the combobox
 *
 * \param combobox
 *
 * \return the bank number, 0 for none, -1 for 'new bank', -2 'Inconnu'
 **/
gint gsb_bank_combo_list_get_bank_number (GtkWidget *combobox)
{
    GtkTreeIter iter;
    gint bank_number;

	if (!combobox)
		return -2;

    if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
		return -2;

    gtk_tree_model_get (GTK_TREE_MODEL (bank_list_model), &iter, BANK_NUMBER_COL, &bank_number, -1);

	return bank_number;
}

/**
 * show the current bank in the combobox acording to the bank number
 *
 * \param combobox
 * \param bank_number 0 for none, -1 for new bank
 * \param number_for_func the number to give to the default_func when something changed
 *
 * \return FALSE
 **/
gboolean gsb_bank_combo_list_set_bank (GtkWidget *combobox,
								 gint bank_number)
{
    GtkTreeIter iter;

    if (!combobox)
        return FALSE;

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (bank_list_model), &iter))
        return FALSE;

    /* on bloque la fonction de callback */
    g_signal_handlers_block_by_func (G_OBJECT (combobox), gsb_bank_combo_list_changed, NULL);

    do
    {
        gint current_bank;

        gtk_tree_model_get (GTK_TREE_MODEL (bank_list_model),
							&iter,
							BANK_NUMBER_COL, &current_bank,
							-1);

        if (bank_number == current_bank)
        {
            /* bank found */
            gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combobox), &iter);
            break;
        }
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (bank_list_model), &iter));

    /* on débloque la fonction de callback */
    g_signal_handlers_unblock_by_func (G_OBJECT (combobox), gsb_bank_combo_list_changed, NULL);

    return FALSE;
}

/**
 * update the model for the combobox of the banks
 * add an 'add' button at the end
 *
 * \param combobox a facultative option, if not NULL, the current selection will be saved and restored at the end of the function
 *
 * \return TRUE if ok, FALSE if problem
 **/
gboolean gsb_bank_update_combo_list_model (GtkWidget *combobox)
{
    GtkTreeIter iter;
    GSList *list_tmp;
    gint save_bank_number = -2;

    /* save the selection */
    if (combobox)
    {
        save_bank_number = gsb_bank_combo_list_get_bank_number (combobox);
    }

    /* if no bank model, create it */
    if (!bank_list_model)
        gsb_bank_create_combobox_model ();

    gtk_list_store_clear (GTK_LIST_STORE (bank_list_model));

    /* first : none, whith 0 at number (no bank at 0) */
    gtk_list_store_append (GTK_LIST_STORE (bank_list_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (bank_list_model),
                        &iter,
                        BANK_NAME_COL, _("None"),
                        BANK_NUMBER_COL, 0,
                        -1);

    /* set the names */
    list_tmp = gsb_data_bank_get_bank_list ();

    while (list_tmp)
    {
        gint bank_number;

        bank_number = gsb_data_bank_get_no_bank (list_tmp->data);

        gtk_list_store_append (GTK_LIST_STORE (bank_list_model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (bank_list_model),
                        &iter,
                        BANK_NAME_COL, gsb_data_bank_get_name (bank_number),
                        BANK_NUMBER_COL, bank_number,
                        -1);
        list_tmp = list_tmp->next;
    }

    /* separator line : the name of bank is null */
    gtk_list_store_append (GTK_LIST_STORE (bank_list_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (bank_list_model),
                        &iter,
                        BANK_NAME_COL, NULL,
                        -1);

    /* item to add a bank : the number is -1 */
    gtk_list_store_append (GTK_LIST_STORE (bank_list_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (bank_list_model),
                        &iter,
                        BANK_NAME_COL, _("Add new bank"),
                        BANK_NUMBER_COL, -1,
                        -1);

    /* restore the selection */
    if (combobox)
        gsb_bank_combo_list_set_bank (combobox, save_bank_number);

    return TRUE;
}

/**
 * callback called when click on the edit-bank in the property of account
 * or when create a new account
 * show a window with the parameters of the bank in the combobox
 *
 * \param button the button which was clicked
 * \param combobox the combobox selected a bank
 *
 * \return FALSE
 **/
gboolean gsb_bank_edit_from_button (GtkWidget *button,
									GtkWidget *combobox)
{
    gint bank_number;

    bank_number = gsb_bank_combo_list_get_bank_number (combobox);

	/* if bank_number = 0, it's none ; -1 : it's new bank */
    if (bank_number <= 0)
        return FALSE;

    gsb_bank_edit_bank (bank_number, combobox);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_bank_free_combo_list_model (void)
{
	if (bank_list_model)
		gtk_list_store_clear (GTK_LIST_STORE (bank_list_model));
	bank_list_model = NULL;
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
