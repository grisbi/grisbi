/* ************************************************************************** */
/* Ce fichier comprend toutes les opérations concernant le traitement	      */
/* des fichiers								      */
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


#include "include.h"

#include <time.h>



/*START_INCLUDE*/
#include "gsb_file.h"
#include "./menu.h"
#include "./utils.h"
#include "./fenetre_principale.h"
#include "./dialog.h"
#include "./utils_file_selection.h"
#include "./gsb_assistant_account.h"
#include "./gsb_category.h"
#include "./gsb_currency_config.h"
#include "./gsb_data_account.h"
#include "./gsb_data_archive_store.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file_load.h"
#include "./gsb_file_save.h"
#include "./gsb_file_util.h"
#include "./navigation.h"
#include "./gsb_real.h"
#include "./gsb_status.h"
#include "./gsb_transactions_list.h"
#include "./traitement_variables.h"
#include "./main.h"
#include "./accueil.h"
#include "./utils_files.h"
#include "./utils_str.h"
#include "./parametres.h"
#include "./affichage_liste.h"
#include "./import.h"
#include "./gsb_file_config.h"
#include "./utils_file_selection.h"
#include "./fenetre_principale.h"
#include "./include.h"
#include "./erreur.h"
#include "./structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_file_append_name_to_opened_list ( gchar * path_fichier );
static  gchar *gsb_file_dialog_ask_name ( void );
static  gint gsb_file_dialog_save ( void );
static gboolean gsb_file_save_backup ( void );
static gboolean gsb_file_save_file ( gint origine );
/*END_STATIC*/




gchar *nom_fichier_backup;

/**
 * keep the last path used in grisbi, save in the configuration at the end */
static gchar *last_path_used;



/*START_EXTERN*/
extern GtkWidget *main_hpaned ;
extern GtkWidget *main_vbox;
extern gint max;
extern GtkWidget * navigation_tree_view;
extern gsize nb_derniers_fichiers_ouverts ;
extern gint nb_max_derniers_fichiers_ouverts ;
extern gchar *nom_fichier_comptes;
extern GtkWidget *notebook_general;
extern GSList *scheduled_transactions_taken;
extern GSList *scheduled_transactions_to_take;
extern GtkTreeSelection * selection;
extern gchar **tab_noms_derniers_fichiers_ouverts ;
extern GtkWidget *table_etat ;
extern gchar *titre_fichier;
extern GtkWidget *tree_view_vbox;
extern GtkWidget *window;
extern GtkWidget *window_vbox_principale;
/*END_EXTERN*/



/**
 * Called by menu file -> new,
 * close the last file and open a new one
 * 
 * \param none
 * 
 * \return FALSE
 * */
gboolean gsb_file_new ( void )
{
    /* continue only if closing the file is ok */
    if ( !gsb_file_close () )
	return FALSE;

    /* WARNING : there is another way to create a new file : importing a qif/ofx/csv account */
    /* 	      so a change here need to be changed in the import file */
    /* 	      (see traitement_operations_importees) */

    init_variables ();

    /* create the first currency */
    if ( !gsb_currency_config_add_currency ( NULL, NULL ) )
	return FALSE;

    /* Create initial lists. */
    gsb_category_choose_default_category ();

    /* create the first account */
    gsb_assistant_account_run ();

    /* init the gui */
    gsb_file_new_gui ();

    /* 	set the grid */
    gsb_transactions_list_draw_grid (etat.affichage_grille);

    mise_a_jour_accueil ( TRUE );
    gsb_gui_navigation_set_selection ( GSB_HOME_PAGE, -1, NULL );

    modification_fichier ( TRUE );
    return FALSE;
}



/**
 * Initialize user interface part when a new accounts file is created.
 */
void gsb_file_new_gui ( void )
{
    GtkWidget * tree_view_widget;

    /* dégrise les menus nécessaire */
    
    menus_sensitifs ( TRUE );

    /*     récupère l'organisation des colonnes  */
    recuperation_noms_colonnes_et_tips ();

    /* Create main widget. */
    gtk_box_pack_start ( GTK_BOX ( window_vbox_principale), create_main_widget(),
			 TRUE, TRUE, 0 );

    /* Create transaction list. */
    tree_view_widget = gsb_transactions_list_make_gui_list ();
    gtk_box_pack_start ( GTK_BOX ( tree_view_vbox ),
			 tree_view_widget,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( tree_view_widget );

    navigation_change_account ( GINT_TO_POINTER ( gsb_gui_navigation_get_current_account () ) );

    /* Display accounts in menus */
    gsb_menu_update_accounts_in_menus ();

    /* Affiche le nom du fichier de comptes dans le titre de la fenetre */
    gsb_file_update_window_title();

    gtk_notebook_set_page ( GTK_NOTEBOOK( notebook_general ), GSB_HOME_PAGE );

    gtk_widget_show ( notebook_general );
}


/**
 * called by file -> open
 * open a new file
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_file_open_menu ( void )
{
    GtkWidget *selection_fichier;
    GtkFileFilter * filter;

    selection_fichier = file_selection_new ( _("Open an accounts file"),
					     FILE_SELECTION_MUST_EXIST);
    gtk_window_set_position ( GTK_WINDOW ( selection_fichier ), GTK_WIN_POS_MOUSE);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (selection_fichier),
					 gsb_file_get_last_path ());

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Grisbi files (*.gsb)") );
    gtk_file_filter_add_pattern ( filter, "*.gsb" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( selection_fichier ), filter );
    gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( selection_fichier ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("All files") );
    gtk_file_filter_add_pattern ( filter, "*" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( selection_fichier ), filter );

    switch ( gtk_dialog_run ( GTK_DIALOG (selection_fichier)))
    {
	case GTK_RESPONSE_OK:
	    if ( gsb_file_close() )
	    {
		gtk_widget_hide ( selection_fichier );
		nom_fichier_comptes = file_selection_get_filename ( GTK_FILE_CHOOSER ( selection_fichier ) ) ;
		gsb_file_update_last_path (file_selection_get_last_directory ( GTK_FILE_CHOOSER ( selection_fichier),
									       TRUE ));
		gsb_file_open_file (nom_fichier_comptes);
	    }
	    break;
      default:
	  break;
    }
    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (selection_fichier), TRUE));
    gtk_widget_destroy ( selection_fichier );
    return FALSE;
}


/**
 * update the variable last_path_used with the path given in param
 *
 * \param last_path
 *
 * \return
 * */
void gsb_file_update_last_path ( const gchar *last_path )
{
    if (last_path
	&&
	strlen (last_path))
    {
	if (last_path_used)
	    g_free (last_path_used);
	last_path_used = my_strdup (last_path);
    }
}

/**
 * the last_path_used is local variable,
 * that return the content of that variable
 *
 * \param
 *
 * \return a const gchar, the last path used in grisbi
 * */
const gchar *gsb_file_get_last_path ( void )
{
    return last_path_used;
}


/**
 * called by file -> last files -> click on the name
 *
 * \param item
 * \param file_number_ptr the number of file in a pointer format
 *
 * \return FALSE
 * */
gboolean gsb_file_open_direct_menu ( GtkMenuItem *item,
				     gint *file_number_ptr )
{
    /* continue only if can close the current file */
    if ( !gsb_file_close() )
	return FALSE;

    nom_fichier_comptes = tab_noms_derniers_fichiers_ouverts[GPOINTER_TO_INT (file_number_ptr)];
    gsb_file_open_file (nom_fichier_comptes);

    return FALSE;
}



/**
 * open a new grisbi file, don't check anything about another opened file that must
 * have been done before
 * 
 * \para filename the name of the file
 * 
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_file_open_file ( gchar *filename )
{
    gint i;
    GSList *list_tmp;
    GtkWidget *main_widget;

    devel_debug ( g_strdup_printf ("gsb_file_open_file : %s",
				   filename ));

    if ( !filename
	 ||
	 !strlen (filename))
	return FALSE;

    gsb_status_wait ( TRUE );
    gsb_status_message ( _("Loading accounts") );

    /* try to load the file */
    if ( gsb_file_load_open_file (filename))
    {
	/* the file has been opened succesfully */

	/* we make a backup if necessary */
	if ( etat.sauvegarde_demarrage )
	{
	    gchar *backup_filename;
	    gchar **tab_char;

	    gsb_status_message ( _("Autosaving") );

	    backup_filename = my_strdup ( filename );

	    /* we get only the name of the file, not the path */

	    tab_char = g_strsplit ( backup_filename,
				    C_DIRECTORY_SEPARATOR,
				    0);
	    i=0;
	    while ( tab_char[i] )
		i++;

	    backup_filename = g_strconcat ( my_get_gsb_file_default_dir(),
				C_DIRECTORY_SEPARATOR,
#ifndef _WIN32
                                ".",
#endif
				tab_char [i-1],
				".bak",
				NULL );

	    g_strfreev ( tab_char );
	    gsb_file_save_save_file ( backup_filename,
				      etat.compress_backup,
				      FALSE );
	    g_free (backup_filename);
	}
    }
    else
    {
	/* Loading failed.  If the saving function at opening is set,
	 * we ask to load the last file */

	gsb_status_message ( _("Failed to load accounts") );

	if ( etat.sauvegarde_demarrage )
	{
	    gchar *backup_filename;
	    gchar **tab_char;

	    gsb_status_message ( _("Loading backup") );

	    /* create the name of the backup */
	    i=0;
	    tab_char = g_strsplit ( filename, C_DIRECTORY_SEPARATOR, 0);
	    while ( tab_char[i] )
		i++;
	    backup_filename = g_strconcat ( my_get_gsb_file_default_dir(),
					    C_DIRECTORY_SEPARATOR,
					    tab_char [i-1],
					    ".bak",
					    NULL );
	    g_strfreev ( tab_char );

	    /* try to load the backup */

	    if ( gsb_file_load_open_file ( backup_filename ) )
	    {
		/* the backup loaded succesfully */

		dialogue_error_hint ( _("Grisbi was unable to load file.  However, Grisbi loaded a backup file instead but all changes made since this backup were possibly lost."),
				      g_strdup_printf ( _("Error loading file '%s'"), filename) );
		g_free (backup_filename);
	    }
	    else
	    {
		/* the loading backup failed */

		dialogue_error_hint ( _("Grisbi was unable to load file.  Additionally, Grisbi was unable to load a backup file instead."),
				      g_strdup_printf ( _("Error loading file '%s'"), filename) );
		g_free (backup_filename);
		gsb_status_stop_wait ( TRUE );
		return FALSE;
	    }
	}
	else
	{
	    gsb_status_stop_wait ( TRUE );
	    return FALSE;
	}
    }

    /* ok, here the file or backup is loaded */
    gsb_status_message ( _("Checking schedulers"));

    /* the the name in the last opened files */
    gsb_file_append_name_to_opened_list ( filename );

    /* get the names of the columns */
    recuperation_noms_colonnes_et_tips ();

    /* we show and update the menus */
    menus_sensitifs ( TRUE );

    /* we make the main window */
    gsb_status_message ( _("Creating main window"));
    main_widget = create_main_widget();

    /* check the amounts of all the accounts */
    gsb_status_message ( _("Checking amounts"));
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint account_number;

	account_number = gsb_data_account_get_no_account ( list_tmp -> data );

	gsb_data_account_calculate_current_and_marked_balances (account_number);

	/* set the minimum balances to be shown or not
	 * if we are already under the minimum, we will show nothing */

	gsb_data_account_set_mini_balance_authorized_message ( account_number,
							       gsb_real_cmp ( gsb_data_account_get_current_balance (account_number),
									      gsb_data_account_get_mini_balance_authorized (account_number)) == -1 );
	gsb_data_account_set_mini_balance_wanted_message ( account_number,
							   gsb_real_cmp ( gsb_data_account_get_current_balance (account_number),
									  gsb_data_account_get_mini_balance_wanted (account_number)) == -1 );
	list_tmp = list_tmp -> next;
    }

    /* set the name of the file in the window title
     * and in the menu of the main window, so main_widget must
     * have been created */
    gsb_file_update_window_title();

    gsb_status_message ( _("Creating interface"));
    gsb_menu_update_view_menu (gsb_gui_navigation_get_current_account ());
    gsb_menu_update_accounts_in_menus ();

    /* append that window to the main window */
    gtk_box_pack_start ( GTK_BOX (window_vbox_principale), main_widget, TRUE, TRUE, 0 );
    gtk_widget_show ( main_widget );

    /* create the archives store data, ie the transaction wich will replace the archive in
     * the list of transactions */
    gsb_data_archive_store_create_list ();

    /* create and fill the gui transactions list */
    gtk_box_pack_start ( GTK_BOX ( tree_view_vbox ),
			 gsb_transactions_list_make_gui_list (),
			 TRUE,
			 TRUE,
			 0 );

    /* update the main page */
    mise_a_jour_accueil (TRUE);

    /* 	set the grid */
    gsb_transactions_list_draw_grid (etat.affichage_grille);

    modification_fichier ( FALSE );

    gsb_status_message ( _("Done") );
    gsb_status_stop_wait ( TRUE );

    /* go to the home page */
    gsb_gui_navigation_set_selection ( GSB_HOME_PAGE, -1, NULL );    

    /* set the focus to the selection tree at left */
    gtk_widget_grab_focus (navigation_tree_view);

    return TRUE;
}


/**
 * Perform the "Save" feature in menu
 *
 * \return TRUE on success.  FALSE otherwise.
 */
gboolean gsb_file_save ( void )
{
    return gsb_file_save_file ( 0 );
}


/**
 * Perform the "Save as" feature in menu
 *
 * \return TRUE on success.  FALSE otherwise.
 */
gboolean gsb_file_save_as ( void )
{
    return gsb_file_save_file ( -2 );
}


/**
 * save the file
 *
 * \param origine 0 from gsb_file_save (menu), -1 from gsb_file_close, -2 from gsb_file_save_as
 *
 * \return TRUE if ok, FALSE if problem
 * */
gboolean gsb_file_save_file ( gint origine )
{
    gint etat_force, result;
    gchar *nouveau_nom_enregistrement;

    devel_debug (g_strdup_printf ( "gsb_file_save_file from %d", origine ));

    etat_force = 0;

    if ( ( ! etat.modification_fichier && origine != -2 ) ||
	 ! gsb_data_account_get_accounts_amount () )
    {
	notice_debug ( "nothing done in gsb_file_save_file" );
	return ( TRUE );
    }

    /* si le fichier de comptes n'a pas de nom ou si on enregistre sous un nouveau nom */
    /*     c'est ici */

    if ( !nom_fichier_comptes || origine == -2 )
	nouveau_nom_enregistrement = gsb_file_dialog_ask_name ();
    else
	nouveau_nom_enregistrement = nom_fichier_comptes;

    if ( !nouveau_nom_enregistrement )
	return FALSE;

    /*     on vérifie que le fichier n'est pas locké */

    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement
	 &&
	 origine != -2 )
    {
	dialogue_error_hint ( g_strdup_printf( _("Grisbi was unable to save this file because it is locked.  Please save it with another name or activate the \"%s\" option in preferences."),
					       _("Force saving of locked files" ) ),
			      g_strdup_printf( _("Can not save file \"%s\""), 
					       nom_fichier_comptes ) );
	return ( FALSE );
    }

    /*   on a maintenant un nom de fichier */
    /*     et on sait qu'on peut sauvegarder */

    gsb_status_message ( _("Saving file") );

    result = gsb_file_save_save_file ( nouveau_nom_enregistrement,
				       etat.compress_file,
				       FALSE );

    if ( result )
    {
	/* 	l'enregistrement s'est bien passé, */
	/* 	on délock le fichier (l'ancien ou le courant) */
	    
	gsb_file_util_modify_lock ( FALSE );

	nom_fichier_comptes = nouveau_nom_enregistrement;

	/* 	... et locke le nouveau */

	gsb_file_util_modify_lock ( TRUE );

	/* 	dans tout les cas, le fichier n'était plus ouvert à l'ouverture */

	etat.fichier_deja_ouvert = 0;
	modification_fichier ( FALSE );
	gsb_file_update_window_title ();
	gsb_file_append_name_to_opened_list ( nom_fichier_comptes );
    }

    /*     on enregistre la backup si nécessaire */

    gsb_file_save_backup();

    gsb_status_message ( _("Done") );

    return ( result );
}


/**
 * save a backup of the file, using nom_fichier_backup
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_file_save_backup ( void )
{
    gboolean retour;

    if ( !nom_fichier_backup || !strlen(nom_fichier_backup) )
	return FALSE;

    gsb_status_message ( _("Saving backup") );

    retour = gsb_file_save_save_file( nom_fichier_backup,
				      etat.compress_backup,
				      FALSE );

    gsb_status_message ( _("Done") );

    return ( retour );
}



/**
 * propose to save the file if changed
 * if the current file is not changed, return GTK_RESPONSE_NO directly, without dialog
 *
 * \param
 *
 * \return GTK_RESPONSE_OK to save, GTK_RESPONSE_NO not to save, other to cancel
 * */
static gint gsb_file_dialog_save ( void )
{
    gchar * hint, * message = "", * time_elapsed;
    time_t now = time ( NULL );
    gint result;
    GtkWidget *dialog;
    gint difference = (gint) difftime ( now, etat.modification_fichier );

    /*     si le fichier n'est pas modifié on renvoie qu'on ne veut pas enregistrer */

    if ( !etat.modification_fichier )
	return GTK_RESPONSE_NO;
    
    if ( etat.sauvegarde_auto && 
	 ( !etat.fichier_deja_ouvert || etat.force_enregistrement ) &&
	 nom_fichier_comptes )
      return GTK_RESPONSE_OK;

    /*     si le fichier était déjà locké et que force enregistrement n'est pas mis, */
    /*     on prévient ici */

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_WARNING, 
				      GTK_BUTTONS_NONE,
				      " " );
    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement )
    {
	hint = _("Save locked files?");
	message = g_strdup_printf ( _("The document '%s' is locked but modified. If you want to save it, you must cancel and save it with another name or activate the \"%s\" option in setup."),
				    (nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")),
				    _("Force saving of locked files"));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 NULL );
	gtk_dialog_set_default_response ( GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL ); 
    }
    else
    {
	hint = g_strdup_printf (_("Save changes to document '%s' before closing?"),
				(nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_SAVE, GTK_RESPONSE_OK,
				 NULL );
	gtk_dialog_set_default_response ( GTK_DIALOG(dialog), GTK_RESPONSE_OK ); 
    }

    if ( difference >= 120 )
    {
	time_elapsed = g_strdup_printf ( _( "%d minutes and %d seconds" ),
					 difference / 60, difference % 60 );
    }
    else if ( difference >= 60 )
    {
	time_elapsed = g_strdup_printf ( _( "1 minute and %d seconds" ),
					 difference % 60 );
    }
    else
    {
	time_elapsed = g_strdup_printf ( _( "%d seconds" ), difference );
    }
    message = g_strconcat ( message, 
			    g_strdup_printf ( _("If you close without saving, all of your changes "
						"since %s will be discarded."),
					      time_elapsed ),
			    NULL );
    g_free ( time_elapsed );
    
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), 
			   make_hint ( hint, message ) );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );

    return result;
}



/**
 * ask the name of the file to save it
 *
 * \param
 *
 * \return a newly allocated string containing the new name
 * */
static gchar *gsb_file_dialog_ask_name ( void )
{
    gchar *new_name;
    GtkWidget *dialog;
    gint result;

    dialog = file_selection_new ( _("Name the accounts file"),
				  FILE_SELECTION_IS_SAVE_DIALOG);
    gtk_window_set_modal ( GTK_WINDOW ( dialog ),
			   TRUE );
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
					 gsb_file_get_last_path ());

    if ( ! nom_fichier_comptes )
	gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( dialog ),
					    g_strconcat ( titre_fichier, ".gsb", NULL ) );
    else
	gtk_file_chooser_select_filename ( GTK_FILE_CHOOSER (dialog),
					   nom_fichier_comptes );

    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( result )
    {
	case GTK_RESPONSE_OK :
	    new_name = file_selection_get_filename ( GTK_FILE_CHOOSER ( dialog ));
	    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE));
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	    return NULL;
    }

    if ( ! g_strrstr ( new_name, "." ) )
    {
	new_name = g_strconcat ( new_name, ".gsb", NULL );
    }

    return new_name;
}



/**
 * close the file
 * if no file loaded or no change, directly return TRUE
 *
 * \param
 *
 * \return FALSE if problem, TRUE if ok
 * */
gboolean gsb_file_close ( void )
{
    gint result;

    devel_debug ( "gsb_file_close" );


    if ( !assert_account_loaded () )
	return ( TRUE );

    /* ask for saving */
    result = gsb_file_dialog_save();

    switch ( result )
    {
	case GTK_RESPONSE_OK:

	    /* try to save */
	    if ( !gsb_file_save_file (-1) )
		return ( FALSE );

	case GTK_RESPONSE_NO :
	     /* remove the lock */
	    if ( !etat.fichier_deja_ouvert
		 &&
		 gsb_data_account_get_accounts_amount ()
		 &&
		 nom_fichier_comptes )
		gsb_file_util_modify_lock ( FALSE );

	    /* libère les opérations de tous les comptes */
	    g_slist_free ( gsb_data_transaction_get_transactions_list ());
	    g_slist_free ( gsb_data_transaction_get_complete_transactions_list ());
	    g_slist_free ( gsb_data_account_get_list_accounts () );

	    /* libère les échéances */
	    g_slist_free ( gsb_data_scheduled_get_scheduled_list () );
	    g_slist_free ( scheduled_transactions_to_take );
	    g_slist_free ( scheduled_transactions_taken );
	    gtk_widget_destroy ( main_vbox );

	    init_variables ();

	    gsb_file_update_window_title();

	    menus_sensitifs ( FALSE );

	    main_hpaned = NULL;
	    table_etat = NULL;

	    return ( TRUE );

	default :
	    return FALSE;
    }
}


/**
 * set/update the name of the file on the title of the window
 *
 * \param
 *
 * \return
 * */
void gsb_file_update_window_title ( void )
{
    gchar **parametres = NULL;
    gchar *titre = NULL;
    gint i=0;

    devel_debug ( "gsb_file_update_window_title" );

    if ( titre_fichier && strlen(titre_fichier) )
      titre = titre_fichier;
    else if ( nom_fichier_comptes )
    {
	parametres = g_strsplit ( nom_fichier_comptes, C_DIRECTORY_SEPARATOR, 0);
	while ( parametres[i] )
	  i++;
	titre = my_strdup(parametres [i-1]);
	g_strfreev ( parametres );
    }
    else
    {
      titre = g_strconcat ( "<", _("unnamed"), ">", NULL );
    }

    titre = g_strconcat ( titre, " - ", _("Grisbi"), NULL );
    gtk_window_set_title ( GTK_WINDOW ( window ), titre );
}



/**
 * append a new name to the list of recently opened file
 *
 * \param path_fichier
 *
 * \return
 * */
void gsb_file_append_name_to_opened_list ( gchar * path_fichier )
{
    gint i, position;
    gchar * dernier, * real_name;

    devel_debug ( g_strdup_printf ("gsb_file_append_name_to_opened_list : %s", path_fichier ));

    if ( !nb_max_derniers_fichiers_ouverts ||
	 ! path_fichier)
	return;

    if ( nb_derniers_fichiers_ouverts < 0 )
	nb_derniers_fichiers_ouverts = 0;

    if ( !g_path_is_absolute ( nom_fichier_comptes ) )
    {	
	real_name = realpath ( nom_fichier_comptes, NULL );
	if ( ! real_name )
	{
	    notice_debug ( "could not resolve relative file name" );
	    return;
	}
	devel_debug ( real_name );
    }
    else
    {
	real_name = g_strdup ( path_fichier );
    }

    /* on commence par vérifier si ce fichier n'est pas dans les nb_derniers_fichiers_ouverts noms */
    position = 0;

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
	if ( !strcmp ( real_name,
		       tab_noms_derniers_fichiers_ouverts[i] ))
	{
	    /* 	si ce fichier est déjà le dernier ouvert, on laisse tomber */

	    if ( !i )
		return;

	    position = i;
	}

    efface_derniers_fichiers_ouverts();

    if ( position )
    {
	/*       le fichier a été trouvé, on fait juste une rotation */

	for ( i=position ; i>0 ; i-- )
	    tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];
	if ( real_name )
	    tab_noms_derniers_fichiers_ouverts[0] = my_strdup ( real_name );
	else
	    tab_noms_derniers_fichiers_ouverts[0] = my_strdup ( "<no file>" );

	affiche_derniers_fichiers_ouverts();

	return;
    }

    /*   le fichier est nouveau, on décale tout d'un cran et on met le nouveau à 0 */

    /*   si on est déjà au max, c'est juste un décalage avec perte du dernier */
    /* on garde le ptit dernier dans le cas contraire */

    if ( nb_derniers_fichiers_ouverts )
	dernier = tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1];
    else
	dernier = NULL;

    for ( i= nb_derniers_fichiers_ouverts - 1 ; i>0 ; i-- )
	tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];

    if ( nb_derniers_fichiers_ouverts < nb_max_derniers_fichiers_ouverts )
    {
	tab_noms_derniers_fichiers_ouverts = realloc ( tab_noms_derniers_fichiers_ouverts,
						       ( ++nb_derniers_fichiers_ouverts ) * sizeof ( gpointer ));
	tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1] = dernier;
    }

    tab_noms_derniers_fichiers_ouverts[0] = my_strdup ( real_name );

    affiche_derniers_fichiers_ouverts();

    g_free ( real_name );
}


/**
 * remove the filename from the list of recently opened file
 *
 * \param filename
 *
 * \return
 * */
void gsb_file_remove_name_from_opened_list ( gchar *filename )
{
    gint i, j;

    efface_derniers_fichiers_ouverts();

    for ( i = 0 ; i < nb_derniers_fichiers_ouverts ; i++ )
    {
	if ( ! strcmp (filename, tab_noms_derniers_fichiers_ouverts[i]) )
	{
	    nb_derniers_fichiers_ouverts--;

	    for ( j = i; ( j + 1 ) < nb_derniers_fichiers_ouverts; j++ )
	    {
		tab_noms_derniers_fichiers_ouverts[j] = tab_noms_derniers_fichiers_ouverts[j+1];
		
	    }
	}
    }
    affiche_derniers_fichiers_ouverts();
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
