/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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

/**
 * \file gsb_assistant_file.c
 * we find here the complete assistant launched for creating a new grisbi file
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_assistant_file.h"
#include "grisbi_app.h"
#include "gsb_assistant.h"
#include "gsb_automem.h"
#include "gsb_bank.h"
#include "gsb_category.h"
#include "gsb_currency_config.h"
#include "gsb_currency.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_select_icon.h"
#include "import.h"
#include "parametres.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_assistant_file_change_title ( GtkWidget *title_entry,
						  GtkWidget *filename_entry );
static gboolean gsb_assistant_file_choose_filename ( GtkWidget *button,
						     GtkWidget *entry );
static GtkWidget *gsb_assistant_file_page_2 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_file_page_3 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_file_page_4 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_file_page_5 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_file_page_finish ( GtkWidget *assistant,
						   gboolean import );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

enum file_assistant_page
{
    FILE_ASSISTANT_INTRO= 0,
    FILE_ASSISTANT_PAGE_2,
    FILE_ASSISTANT_PAGE_3,
    FILE_ASSISTANT_PAGE_4,
    FILE_ASSISTANT_PAGE_5,
    FILE_ASSISTANT_PAGE_FINISH,
};

/* the box containing the currencies list
 * treeview and model are saved into the keys "treeview" and "model" */
static GtkWidget *currency_list_box;

/* the button to know what assistant to launch at the end */
static GtkWidget *button_create_account_next;

/* nom du fichier de compte créé PROVISOIRE */
static gchar *nom_fichier_comptes;

/**
 * this function is called to launch the file opening assistant
 *
 * \param first_opening TRUE if this is the first opening of Grisbi,
 * 			and that assistant is continuing from the general configuration assistant
 * \param import	TRUE if we come from import (possible when we do file->import without anything
 * 			opened, because we need to set up the variables and currencies)
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_file_run ( gboolean first_opening,
					 gboolean import )
{
    GtkResponseType return_value;
    GtkWidget *assistant;
    gchar *currency_name, *currency_iso_code, *currency_nickname;
    gint currency_floating;
    GtkTreeIter iter;
    gboolean launch_account_assistant;
    const gchar *text_1, *text_2;

    /* create the assistant */
    if (first_opening)
    {
	text_1 = _("Welcome to Grisbi!");
	text_2 = _("General configuration of Grisbi is done.\n\n"
		   "Next assistant will now help you to create and configure a new account file.\n"
		   "Don't forget that you can change everything later in the 'Preferences' dialog.\n\n"
		   "Once the file created, you will be able to create a new account "
		   "or to import some previous datas");
    }
    else
    {
	if (import)
	{
	    text_1 = _("New file Assistant to import");
	    text_2 = _("Before importing files, you need to create a new file and fill "
		       "some default values as created a new currency.\n"
		       "This assistant will help you to create a new account file.\n\n"
		       "Every configuration step from this assistant can be edited "
		       "later in the 'Preferences' dialog.\n"
		       "Once you have created your account file, you will be able to "
		       "create a new account from scratch or to import previous data, i.e. "
		       "from your previous accounting software or your bank website.");
	}
	else
	{
	    text_1 = _("New file Assistant");
	    text_2 = _("This assistant will help you to create a new account file.\n\n"
		       "Every configuration step from this assistant can be edited "
		       "later in the 'Preferences' dialog.\n"
		       "Once you have created your account file, you will be able to "
		       "create a new account from scratch or to import previous data, i.e. "
		       "from your previous accounting software or your bank website.");
	}
    }

    assistant = gsb_assistant_new (text_1, text_2,
				   "gsb-new-file-32.png", NULL);

    gsb_assistant_add_page ( assistant,
			     gsb_assistant_file_page_2 (assistant),
			     FILE_ASSISTANT_PAGE_2,
			     FILE_ASSISTANT_INTRO,
			     FILE_ASSISTANT_PAGE_3,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_file_page_3 (assistant),
			     FILE_ASSISTANT_PAGE_3,
			     FILE_ASSISTANT_PAGE_2,
			     FILE_ASSISTANT_PAGE_4,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_file_page_4 (assistant),
			     FILE_ASSISTANT_PAGE_4,
			     FILE_ASSISTANT_PAGE_3,
			     FILE_ASSISTANT_PAGE_5,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_file_page_5 (assistant),
			     FILE_ASSISTANT_PAGE_5,
			     FILE_ASSISTANT_PAGE_4,
			     FILE_ASSISTANT_PAGE_FINISH,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_file_page_finish (assistant, import),
			     FILE_ASSISTANT_PAGE_FINISH,
			     FILE_ASSISTANT_PAGE_5,
			     0,
			     NULL );

    /* now we launch the assistant */
    return_value = gsb_assistant_run (assistant);

    if (return_value == GTK_RESPONSE_CANCEL)
    {
	/* the user stopped the assistant, we delete all the configured data */
	init_variables ();
	gtk_widget_destroy (assistant);
	return return_value;
    }

    /* the assistant is finished, we save the values not saved before */

    /* get the currency */
    if (gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (GTK_TREE_VIEW (g_object_get_data (G_OBJECT (currency_list_box),
													 "treeview"))),
					  NULL,
					  &iter ))
    {
	/* there is a selection, normaly, always the case */
	gtk_tree_model_get ( GTK_TREE_MODEL (g_object_get_data(G_OBJECT (currency_list_box),
							       "model")),
			     &iter,
			     CURRENCY_NAME_COLUMN, &currency_name,
			     CURRENCY_ISO_CODE_COLUMN, &currency_iso_code,
			     CURRENCY_NICKNAME_COLUMN, &currency_nickname,
			     CURRENCY_FLOATING_COLUMN, &currency_floating,
			     -1 );
	gsb_currency_config_create_currency ( currency_name, currency_nickname,
					      currency_iso_code, currency_floating);

	/* update the currency list for combobox */
	gsb_currency_update_combobox_currency_list ();
    }

    /* create the list of categories
     * the choice is in the assistant widget under the key "choice_value" */
    gsb_category_assistant_create_categories (assistant);

    /* get the next assistant to launch */
    launch_account_assistant = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                        (button_create_account_next));
    gtk_widget_destroy (assistant);

    /* initialise le logo accueil */
    gsb_select_icon_set_logo_pixbuf (
                        gsb_select_icon_get_default_logo_pixbuf ( ) );

	/* set the new filename */
	grisbi_win_set_filename (NULL, nom_fichier_comptes);
	if (nom_fichier_comptes)
		g_free (nom_fichier_comptes);

    /* and now, launch the next assistant */
    if (launch_account_assistant)
	gsb_file_new_finish ();
    else
	gsb_import_assistant_importer_fichier ();

    return return_value;
}



/**
 * create the page 3 of the first assistant
 * we create here the title of the file, name of the file and adress
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_file_page_2 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *scrolled_window;
    GtkWidget *paddingbox;
    GtkWidget *textview;
    GtkWidget *button;
    GtkWidget *table;
    GtkWidget *filename_entry;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    page = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 15 );
    gtk_container_set_border_width ( GTK_CONTAINER (page), BOX_BORDER_WIDTH );

    vbox = new_vbox_with_title_and_icon ( _("General configuration"),
					  "gsb-payees-32.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

	/* table 2x3 for layout */
    table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), 6);
	gtk_grid_set_column_spacing (GTK_GRID (table), 6);

	/* Account file title */
	paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Titles and filenames") );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0);

	/* label account name */
	label = gtk_label_new ( _("Name of accounting entity: ") );
	utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

	/* need to declare filename_entry first for the next callback,
	 * if no filename, set the title.gsb as default name */
	nom_fichier_comptes = g_strdup (grisbi_win_get_filename (NULL));
	if (!nom_fichier_comptes)
	nom_fichier_comptes = g_strconcat ( gsb_dirs_get_default_dir (),
			G_DIR_SEPARATOR_S, _("My accounts"), ".gsb", NULL );
	filename_entry = gsb_automem_entry_new (&nom_fichier_comptes, NULL, NULL);

    entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY(entry), w_etat->accounting_entity);
	g_signal_connect (G_OBJECT(entry),
					  "changed",
					  G_CALLBACK (gsb_assistant_file_change_title),
					  filename_entry);
	g_object_set_data ( G_OBJECT (entry), "last_title", my_strdup (w_etat->accounting_entity));
	gtk_grid_attach (GTK_GRID (table), entry, 1, 0, 2, 1);

	/* filename */
	label = gtk_label_new ( _("Filename: ") );
	utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

	gtk_grid_attach (GTK_GRID (table), filename_entry, 1, 1, 1, 1);

	button = gtk_button_new_with_label ("...");
	gtk_button_set_relief ( GTK_BUTTON (button), GTK_RELIEF_NONE );
	g_signal_connect ( G_OBJECT (button), "clicked",
			G_CALLBACK (gsb_assistant_file_choose_filename), filename_entry );

	gtk_grid_attach (GTK_GRID (table), button, 2, 1, 1, 1);

    /* will we crypt the file ? */
#ifdef HAVE_SSL
    {
        button = gsb_automem_checkbutton_new ( _("Encrypt Grisbi file"),
                                               &(etat.crypt_file), G_CALLBACK (gsb_gui_encryption_toggled), NULL);
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), button,
                             FALSE, FALSE, 0 );

        if ( etat.crypt_file )
            run.new_crypted_file = TRUE;
    }
#else
    {
        run.new_crypted_file = FALSE;
    }
#endif

    /* date format */
    paddingbox = gsb_config_date_format_chosen ( vbox, GTK_ORIENTATION_HORIZONTAL );

    /* decimal and thousands separator */
    paddingbox = gsb_config_number_format_chosen ( vbox, GTK_ORIENTATION_HORIZONTAL );

    /* Address */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Your address") );

    /* Common address */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
			 FALSE, FALSE, 0);
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window),
					  GTK_SHADOW_IN );
    textview = gsb_automem_textview_new ( &w_etat->adr_common, NULL, NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			textview );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the page 4 of the first assistant
 * we create here the first currency for the software
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_file_page_3 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;

    page = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER (page), BOX_BORDER_WIDTH );

    vbox = new_vbox_with_title_and_icon ( _("Select base currency"),
					  "gsb-currencies-32.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    /* set up the menu */
    currency_list_box = gsb_currency_config_create_box_popup (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 currency_list_box,
			 TRUE, TRUE, 0 );

    /* Select default currency. */
    gtk_tree_model_foreach ( GTK_TREE_MODEL(g_object_get_data ( G_OBJECT(currency_list_box), "model" )),
			     (GtkTreeModelForeachFunc) gsb_currency_config_select_default,
			     g_object_get_data ( G_OBJECT(currency_list_box), "treeview" ) );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the page 5 of the first assistant
 * selection of the list of categories
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_file_page_4 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *button_list;

    page = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER (page), BOX_BORDER_WIDTH );

    vbox = new_vbox_with_title_and_icon ( _("Select the list of categories you will use"),
					  "gsb-categories-32.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    button_list = gsb_category_assistant_create_choice_page (assistant);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_list,
			 TRUE, TRUE, 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the page 6 of the first assistant
 * Creation of the banks
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_file_page_5 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *bank_page;

    page = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER (page), BOX_BORDER_WIDTH );

    /* the configuration page is very good, keep it */
    bank_page = gsb_bank_create_page (TRUE);
    gtk_box_pack_start ( GTK_BOX (page),
			 bank_page,
			 TRUE, TRUE, 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the last page of the first assistant
 *
 * \param assistant 	the GtkWidget assistant
 * \param import	TRUE if we come to this assistant for importing some files
 * 			so by default, set the radiobutton on import files
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_file_page_finish ( GtkWidget *assistant,
						   gboolean import )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button;

    page = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER (page), BOX_BORDER_WIDTH );

    vbox = new_vbox_with_title_and_icon ( _("Configuration finished!"),
					  "reconciliation-32.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("Default values are now configured for your account file. "
			     "Next assistant will help you creating a new account.\n\n"
			     "Remember that all of the values can be changed in the 'Preferences' dialog.\n\n"
			     "Select a way to populate your account file and press the 'Close' button."));
    utils_labels_set_alignement ( GTK_LABEL (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    button_create_account_next = gtk_radio_button_new_with_label ( NULL,
								   _("Create a new account from scratch"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_create_account_next,
			 FALSE, FALSE, 0 );

    button = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (GTK_RADIO_BUTTON (button_create_account_next)),
					       _("Import data from online bank services or from accounting software"));
    if (import)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				      TRUE );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}


/**
 * called when change into the file title,
 * we change the filename too, adding .gsb and the path,
 * only if the user didn't change it before
 *
 * \param title_entry
 * \param filename_entry
 *
 * \return FALSE
 * */
static void gsb_assistant_file_change_title ( GtkWidget *title_entry,
						  GtkWidget *filename_entry )
{
    gchar *new_filename;
    gchar *last_filename;
    gchar *last_title;
	const gchar *text;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

	text = gtk_entry_get_text (GTK_ENTRY (title_entry));
	if (text && strlen (text))
		w_etat->accounting_entity = g_strdup (text);

    /* set Grisbi title */
    grisbi_win_set_window_title (-1);

    /* first get the last content of the title to see if the filename
     * was automatically created, and in that case, we continue the automatic mode,
     * else we don't touch the filename entry */
    last_title = g_object_get_data ( G_OBJECT (title_entry),
				     "last_title");
    last_filename = g_strconcat ( gsb_dirs_get_default_dir (),
				  G_DIR_SEPARATOR_S,
				  last_title,
				  ".gsb",
				  NULL );
    /* set the new -last title- */
    g_free (last_title);
    g_object_set_data ( G_OBJECT (title_entry),
			"last_title", g_strdup (gtk_entry_get_text (GTK_ENTRY (title_entry))));

    if ( strcmp ( last_filename,
		  gtk_entry_get_text (GTK_ENTRY (filename_entry))))
    {
	/* there is a difference between the last title and the filename,
	 * so juste free the memory and do nothing */
	g_free (last_filename);
	return;
    }

    /* ok, the filename is an automatic creation,
     * so we change it */
    g_free (last_filename);

    if ( strlen ( gtk_entry_get_text (GTK_ENTRY (title_entry) ) ) )
    {
	new_filename = g_strconcat ( gsb_dirs_get_default_dir (),
				     G_DIR_SEPARATOR_S,
				     gtk_entry_get_text (GTK_ENTRY (title_entry)),
				     ".gsb",
				     NULL );
    }
    else
    {
	new_filename = g_strconcat ( gsb_dirs_get_default_dir (),
				     G_DIR_SEPARATOR_S,
				     _("My accounts"),
				     ".gsb",
				     NULL );
    }

    gtk_entry_set_text ( GTK_ENTRY (filename_entry),
			 new_filename );
    g_free (new_filename);

    return;
}

/**
 * called when click on the button to create a new filename
 * show a filechooser to create the name
 *
 * \param button
 * \param entry the entry containing the filename
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_file_choose_filename ( GtkWidget *button,
						     GtkWidget *entry )
{
    GtkWidget *dialog;
    gchar *tmpstr;

    dialog = gtk_file_chooser_dialog_new ( _("Create filename"),
					   GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
					   GTK_FILE_CHOOSER_ACTION_SAVE,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-ok", GTK_RESPONSE_ACCEPT,
					   NULL);

    tmpstr = g_path_get_basename (gtk_entry_get_text (GTK_ENTRY (entry)));
    gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( dialog ), tmpstr );
    g_free (tmpstr);

    tmpstr = g_path_get_dirname (gtk_entry_get_text (GTK_ENTRY (entry)));
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), tmpstr );
    g_free (tmpstr);

    gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( dialog ), TRUE);
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
	gchar *filename;

	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_entry_set_text ( GTK_ENTRY (entry),
			     filename );
	g_free (filename);
    }
    gtk_widget_destroy (dialog);
    return FALSE;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
