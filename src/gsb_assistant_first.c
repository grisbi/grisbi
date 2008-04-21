/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_assistant_first.c
 * we find here the complete assistant launched at the first use of grisbi
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_first.h"
#include "./gsb_assistant.h"
#include "./gsb_automem.h"
#include "./gsb_category.h"
#include "./gsb_currency_config.h"
#include "./gsb_file.h"
#include "./parametres.h"
#include "./utils_files.h"
#include "./utils_str.h"
#include "./utils.h"
#include "./affichage.h"
#include "./structures.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gboolean gsb_assistant_first_backup_dir_chosen ( GtkWidget *button,
							gpointer null );
static  gboolean gsb_assistant_first_change_title ( GtkWidget *title_entry,
						   GtkWidget *filename_entry );
static  gboolean gsb_assistant_first_choose_filename ( GtkWidget *button,
						      GtkWidget *entry );
static  GtkWidget *gsb_assistant_first_page_2 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_first_page_3 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_first_page_4 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_first_page_5 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_first_page_6 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_first_page_finish ( GtkWidget *assistant );
static  gboolean gsb_assistant_first_toggle_backup ( GtkWidget *toggle_button,
						    gpointer null );
/*END_STATIC*/

/*START_EXTERN*/
extern gchar *adresse_commune ;
extern gchar *nom_fichier_comptes ;
extern GtkTreeSelection * selection ;
extern gchar *titre_fichier ;
extern GtkWidget *window ;
/*END_EXTERN*/

enum first_assistant_page
{
    FIRST_ASSISTANT_INTRO= 0,
    FIRST_ASSISTANT_PAGE_2,
    FIRST_ASSISTANT_PAGE_3,
    FIRST_ASSISTANT_PAGE_4,
    FIRST_ASSISTANT_PAGE_5,
    FIRST_ASSISTANT_PAGE_6,
    FIRST_ASSISTANT_PAGE_FINISH,
};

/* used to create the name of the backup if asked
 * the name created will be finame_backup.gsb */
static gboolean create_backup = FALSE;

/* the line to choose the backup directory, here because hidden if no backup asked */
static GtkWidget *hbox_backup_dir;


/**
 * this function is called to launch the first opening assistant
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_first_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Welcome to Grisbi !"),
				    _("This is the first time you open Grisbi, this assistant will help you to configure it.\n"
				      "All that you do here can be changed later in the configuration page.\n"
				      "Many other options are available in the configuration window.\n"
				      "Enjoy..." ),
				    "grisbi-logo.png",
				    NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_2 (assistant),
			     FIRST_ASSISTANT_PAGE_2,
			     FIRST_ASSISTANT_INTRO,
			     FIRST_ASSISTANT_PAGE_3,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_3 (assistant),
			     FIRST_ASSISTANT_PAGE_3,
			     FIRST_ASSISTANT_PAGE_2,
			     FIRST_ASSISTANT_PAGE_4,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_4 (assistant),
			     FIRST_ASSISTANT_PAGE_4,
			     FIRST_ASSISTANT_PAGE_3,
			     FIRST_ASSISTANT_PAGE_5,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_5 (assistant),
			     FIRST_ASSISTANT_PAGE_5,
			     FIRST_ASSISTANT_PAGE_4,
			     FIRST_ASSISTANT_PAGE_6,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_6 (assistant),
			     FIRST_ASSISTANT_PAGE_6,
			     FIRST_ASSISTANT_PAGE_5,
			     FIRST_ASSISTANT_PAGE_FINISH,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_finish (assistant),
			     FIRST_ASSISTANT_PAGE_FINISH,
			     FIRST_ASSISTANT_PAGE_6,
			     0,
			     NULL );

    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);
    return return_value;
}



/**
 * create the page 2 of the first assistant
 * this is the page for general configuration (web browser, automatic opening and saving... )
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_2 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *paddingbox;
    GtkWidget *table;
    GtkSizeGroup *size_group;
    gchar *text;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    vbox = new_vbox_with_title_and_icon ( _("General configuration of Grisbi"),
					  "money.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* configure the browser */
    paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Web browser"));

    table = gtk_table_new ( 0, 3, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    label = gtk_label_new ( _("Web browser"));
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entry = gsb_automem_entry_new ( &etat.browser_command, NULL, NULL );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    text = g_strconcat ( "<i>", _("You may use %s to expand URL.\n"
				  "I.e: \"firefox -remote %s\""), "</i>", NULL );
    label = gtk_label_new ( text );
    gtk_label_set_use_markup ( GTK_LABEL(label), TRUE );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 1, 2, 2, 3,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    g_free ( text );

    /* Account file handling */
    paddingbox = new_paddingbox_with_title (vbox, FALSE,
					    _("Account files handling"));

    /* Automatically load last file on startup? */
    button = gsb_automem_checkbutton_new (_("Automatically load last file on startup"),
					  &(etat.dernier_fichier_auto), NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, 
			 FALSE, FALSE, 0 );

    /* automatically save file at closing */
    button = gsb_automem_checkbutton_new (_("Automatically save on exit"),
					  &(etat.sauvegarde_auto), NULL, NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, 
			 FALSE, FALSE, 0 );

    /* crypt the grisbi file */
    button = gsb_automem_checkbutton_new ( _("Encrypt Grisbi file"),
					   &(etat.crypt_file), G_CALLBACK (gsb_gui_encryption_toggled), NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button,
			 FALSE, FALSE, 0 );

    /* Automatic backup ? */
    button = gsb_automem_checkbutton_new (_("Make a backup copy before saving files"),
					  &create_backup, G_CALLBACK (gsb_assistant_first_toggle_backup), NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the page 3 of the first assistant
 * we create here the title of the file, name of the file and adress
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_3 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *scrolled_window;
    GtkWidget *paddingbox;
    GtkWidget *hbox;
    GtkWidget *textview;
    GtkWidget *button;
    GtkWidget *filename_entry;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = new_vbox_with_title_and_icon ( _("General configuration of the new account"),
					  "new-payee.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    /* Account file title */
    paddingbox = new_paddingbox_with_title ( vbox, TRUE,
					     _("Titles and filenames") );
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( COLON(_("Accounts file title")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    /* need to declare filename_entry first for the next callback,
     * if no filename, set the title.gsb as default name */
    if (!nom_fichier_comptes)
	nom_fichier_comptes = g_strconcat ( my_get_gsb_file_default_dir (),
					    G_DIR_SEPARATOR_S,
					    titre_fichier,
					    ".gsb",
					    NULL );
    filename_entry = gsb_automem_entry_new (&nom_fichier_comptes,
					    NULL, NULL);

    entry = gsb_automem_entry_new (&titre_fichier,
				   ((GCallback)gsb_assistant_first_change_title), filename_entry);
    g_object_set_data ( G_OBJECT (entry),
			"last_title", my_strdup (titre_fichier));
    gtk_box_pack_start ( GTK_BOX ( hbox ), entry,
			 TRUE, TRUE, 0);
    
    /* filename */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( COLON(_("Filename")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    gtk_box_pack_start ( GTK_BOX ( hbox ), filename_entry,
			 TRUE, TRUE, 0);
    
    button = gtk_button_new_with_label ("...");
    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (gsb_assistant_first_choose_filename),
		       filename_entry );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button,
			 FALSE, FALSE, 0);

    /* backup directory */
    hbox_backup_dir = gtk_hbox_new ( FALSE, 6 );
    /* unsensitive by default */
    gtk_widget_set_sensitive ( hbox_backup_dir,
			       FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox_backup_dir,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( COLON(_("Backup directory")) );
    gtk_box_pack_start ( GTK_BOX ( hbox_backup_dir ), label,
			 FALSE, FALSE, 0);

    button = gtk_file_chooser_button_new (_("Select/Create backup directory"),
					  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER );
    g_signal_connect ( G_OBJECT (button),
		       "current-folder-changed",
		       G_CALLBACK (gsb_assistant_first_backup_dir_chosen),
		       NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox_backup_dir ), button,
			 TRUE, TRUE, 0);

    /* Address */
    paddingbox = new_paddingbox_with_title ( vbox, TRUE,
					     COLON(_("Common address")));

    /* Common address */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
			 FALSE, FALSE, 0);
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    textview = gsb_automem_textview_new ( &adresse_commune, NULL, NULL );
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
static GtkWidget *gsb_assistant_first_page_4 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *list;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = new_vbox_with_title_and_icon ( _("Select base currency for your file"),
					  "flags.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    /* set up the menu */
    list = gsb_currency_config_create_box_popup (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 list,
			 TRUE, TRUE, 0 );

    /* Select default currency. */
    gtk_tree_model_foreach ( GTK_TREE_MODEL(g_object_get_data ( G_OBJECT(list), "model" )), 
			     (GtkTreeModelForeachFunc) gsb_currency_config_select_default, 
			     g_object_get_data ( G_OBJECT(list), "treeview" ) );

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
static GtkWidget *gsb_assistant_first_page_5 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *button_list;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = new_vbox_with_title_and_icon ( _("Select the list of categories you will use"),
					  "categories.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* create the buttons,
     * the change will be saved in the assistant widget with the key "choice_value" (see gsb_category.h for the choices) */
    button_list = gsb_category_assistant_create_choice_page (assistant);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_list,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the page 6 of the first assistant
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_6 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("This is the page 2... blah blah blah encore"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the last page of the first assistant
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_finish ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("This is the last page... congratulation, grisbi is configured\n"
			     "do you want to use the assistant to create a new account ?\n"
			     "[button] oh yes ! i want to use the assistant\n"
			     "[button] no, i'm a rough guy, i will find by myself !\n"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * called when toggle the button create backup copy
 * show/hide the file chooser button of the directory of backup
 *
 * \param toggle_button
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_first_toggle_backup ( GtkWidget *toggle_button,
						    gpointer null )
{

    gtk_widget_set_sensitive ( hbox_backup_dir,
			       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button)));
    return FALSE;
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
static gboolean gsb_assistant_first_change_title ( GtkWidget *title_entry,
						   GtkWidget *filename_entry )
{
    gchar *new_filename;
    gchar *last_filename;
    gchar *last_title;

    update_homepage_title (GTK_ENTRY (title_entry), NULL, 0, 0);

    /* first get the last content of the title to see if the filename
     * was automaticaly created, and in that case, we continue the automatic mode,
     * else we don't touch the filename entry */
    last_title = g_object_get_data ( G_OBJECT (title_entry),
				     "last_title");
    last_filename = g_strconcat ( my_get_gsb_file_default_dir (), 
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
	return FALSE;
    }

    /* ok, the filename is an automatic creation,
     * so we change it */
    g_free (last_filename);

    new_filename = g_strconcat ( my_get_gsb_file_default_dir (), 
				 G_DIR_SEPARATOR_S,
				 gtk_entry_get_text (GTK_ENTRY (title_entry)),
				 ".gsb",
				 NULL );
    gtk_entry_set_text ( GTK_ENTRY (filename_entry),
			 new_filename );
    g_free (new_filename);
    return FALSE;
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
static gboolean gsb_assistant_first_choose_filename ( GtkWidget *button,
						      GtkWidget *entry )
{
    GtkWidget *dialog;
    gchar *tmpstr;

    dialog = gtk_file_chooser_dialog_new ("Create filename",
					  GTK_WINDOW (window),
					  GTK_FILE_CHOOSER_ACTION_SAVE,
					  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					  GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					  NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    /* set the folder */
    tmpstr = g_path_get_dirname (gtk_entry_get_text (GTK_ENTRY (entry)));
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
					 tmpstr);
    g_free (tmpstr);

    /* set the new name */
    tmpstr = g_path_get_basename (gtk_entry_get_text (GTK_ENTRY (entry)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
				       tmpstr );
    g_free (tmpstr);

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

/**
 * called when choose a new directory for the backup
 *
 * \param button the GtkFileChooserButton
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_first_backup_dir_chosen ( GtkWidget *button,
							gpointer null )
{
    gchar *path;

    path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (button));
    gsb_file_set_backup_path (path);
    if (path)
	g_free (path);

    return FALSE;
}

