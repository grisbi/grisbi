/* permet la configuration du logiciel */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "parametres.h"


#include "affichage.h"
#include "affichage_liste.h"
#include "banque.h"
#include "devises.h"
#include "equilibrage.h"
#include "exercice.h"
#include "import.h"
#include "traitement_variables.h"
#include "type_operations.h"
#include "utils.h"




GtkTreeStore *preference_tree_model;
GtkWidget * hpaned;
GtkNotebook * preference_frame;
gint preference_selected = -1;
GtkTreeSelection * selection;
GtkWidget * button_close, * button_help;
GtkWidget *tree_view;
GtkWidget * bouton_display_lock_active;

extern gint decalage_echeance; 

/**
 * Creates a simple TreeView and a TreeModel to handle preference
 * tabs.  Sets preference_tree_model to the newly created TreeModel.
 *
 * \return a GtkScrolledWindow
 */
GtkWidget * create_preferences_tree ( )
{
    GtkWidget *sw;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    /* Create model */
    preference_tree_model = gtk_tree_store_new (2, 
						G_TYPE_STRING, 
						G_TYPE_INT);

    /* Create container + TreeView */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);
    tree_view = gtk_tree_view_new();
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), 
			     GTK_TREE_MODEL (preference_tree_model));

    /* Make column */
    cell = gtk_cell_renderer_text_new ();
    column = 
	gtk_tree_view_column_new_with_attributes ("Categories",
						  cell,
						  "text", 0,
						  NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
				 GTK_TREE_VIEW_COLUMN (column));

    /* Handle select */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    g_signal_connect (selection, "changed", 
		      ((GCallback)selectionne_liste_preference), 
		      preference_tree_model);

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function ( selection, preference_selectable_func, 
					     NULL, NULL );

    /* Put the tree in the scroll */
    gtk_container_add (GTK_CONTAINER (sw), tree_view);

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect (tree_view, "realize",
		      ((GCallback)gtk_tree_view_expand_all), NULL);

    return sw;
}


gboolean preference_selectable_func (GtkTreeSelection *selection,
				     GtkTreeModel *model,
				     GtkTreePath *path,
				     gboolean path_currently_selected,
				     gpointer data)
{
    GtkTreeIter iter;
    GValue value = {0, };

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get_value ( model, &iter, 1, &value );

    if ( g_value_get_int(&value) == NOT_A_PAGE )
    {
	g_value_unset (&value);
	return FALSE;
    }

    g_value_unset (&value);
    return TRUE;
}


/**
 * Creates a new GtkDialog with a paned list of topics and a paned
 * notebook that allows to switch between all pages.  A click on the
 * list selects one specific page.
 *
 * \param page Initial page to select.
 */
void preferences ( gint page )
{
    GtkWidget *hbox, *tree;
    GtkTreeIter iter, iter2;

    /* Create dialog */
    fenetre_preferences = gtk_dialog_new_with_buttons (_("Grisbi setup"), 
						       GTK_WINDOW ( window ),
						       GTK_DIALOG_MODAL,
						       /* FIXME: untill we implement the help system, this is useless */
						       /* 			  GTK_STOCK_HELP,  GTK_RESPONSE_HELP, */
						       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						       NULL);

    /* Create List & Tree for topics */
    tree = create_preferences_tree();  
    hpaned = gtk_hpaned_new();
    gtk_paned_add1(GTK_PANED(hpaned), tree);
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_paned_add2(GTK_PANED(hpaned), hbox);

    gtk_box_pack_start ( GTK_BOX ( hbox ), gtk_label_new ( "  " ),
			 FALSE, FALSE, 0 ); /* FIXME: ugly! */

    /* Frame for preferences */
    preference_frame = GTK_NOTEBOOK ( gtk_notebook_new () );
    gtk_notebook_set_show_border ( preference_frame, FALSE );
    gtk_notebook_set_show_tabs  ( preference_frame, FALSE );
    gtk_notebook_set_scrollable ( preference_frame, TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), GTK_WIDGET(preference_frame),
			 TRUE, TRUE, 0 );

    /* File tab */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter,
			0, _("Files"),
			1, FILES_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_fichier(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter,
			0, _("Import"),
			1, IMPORT_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_importation(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter,
			0, _("Programs"),
			1, SOFTWARE_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_programmes(), NULL);

    /* Display subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter,
			0, _("Display"),
			1, NOT_A_PAGE,
			-1);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Fonts & logo"),
			1, FONTS_AND_LOGO_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_display_fonts(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Messages & warnings"),
			1, MESSAGES_AND_WARNINGS_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_messages_and_warnings(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Addresses & titles"),
			1, ADDRESSES_AND_TITLES_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_display_addresses(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Transaction form"),
			1, TRANSACTION_FORM_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_display_transaction_form(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Transactions list cells"),
			1, TRANSACTIONS_LIST_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_affichage_liste(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Transactions list lines"),
			1, TRANSACTIONS_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, 
			      GTK_WIDGET(onglet_affichage_operations()), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Reconciliation"),
			1, RECONCILIATION_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, tab_display_reconciliation(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Totals currencies"),
			1, TOTALS_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, tab_display_totals(), NULL);


    /* Resources subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter,
			0, _("Resources"),
			1, NOT_A_PAGE,
			-1);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Currencies"),
			1, CURRENCIES_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_devises(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Banks"),
			1, BANKS_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_banques(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Financial years"),
			1, FINANCIAL_YEARS_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_exercices(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
			&iter2,
			0, _("Payment methods"),
			1, METHODS_OF_PAYMENT_PAGE,
			-1);
    gtk_notebook_append_page (preference_frame, onglet_types_operations(), NULL);

    gtk_widget_show_all ( hpaned );
    gtk_container_set_border_width ( GTK_CONTAINER(hpaned), 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(fenetre_preferences) -> vbox ), 
			 hpaned, TRUE, TRUE, 0);

    /** TODO: honor page argument */
    /*   if ( page != NOT_A_PAGE ) */
    /*     { */
    /*       gtk_widget_show_all ( tree_view ); */
    /*       gtk_tree_model_foreach ( GTK_TREE_MODEL(preference_tree_model), */
    /* 			       (GtkTreeModelForeachFunc) select_row_from_page,  */
    /* 			       (gpointer) page ); */
    /*     } */


    while ( 1 )
    {
	switch (gtk_dialog_run ( GTK_DIALOG ( fenetre_preferences ) ))
	{
	    case GTK_RESPONSE_HELP:
		/* Hook some help function */
		break;
	    default:
		gtk_widget_destroy ( GTK_WIDGET ( fenetre_preferences ));
		return;
	}
    }
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* callback appelé quand on sélectionne un membre de la liste */
/* ************************************************************************************************************** */
gboolean selectionne_liste_preference ( GtkTreeSelection *selection,
					GtkTreeModel *model )
{
    GtkTreeIter iter;
    GValue value = {0, };

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    gtk_tree_model_get_value (model, &iter, 1, &value);

    preference_selected = g_value_get_int(&value);
    gtk_notebook_set_page (preference_frame, preference_selected);

    g_value_unset (&value);

    return FALSE;
}


/**
 * Creates the "Warning & Messages" tab.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_messages_and_warnings ( void )
{
    GtkWidget *hbox, *vbox_pref;
    GtkWidget *paddingbox, *label;

    vbox_pref = new_vbox_with_title_and_icon ( _("Messages & warnings"),
					       "warnings.png" );

    /* Warnings */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					    _("Warnings messages"));

    /* Display a warning message if minimum balances are under minimum level */
    bouton_solde_mini = new_checkbox_with_title ( _("Do not Warn if balances are under minimum levels"),
						  &(etat.display_message_minimum_alert), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_solde_mini, FALSE, FALSE, 0 );

    /* Display a warning message if account file is readable by someone else */
    /* On Windows, the chmod feature does not work: FAT does not have right access permission notions , 
     * on NTFS it to complicated to implement => the feature is removed from the Windows version :
     * for that the corresponding parameter check box is not displayed and the paramater is forced to not display msg ( 1!!! ) */
#ifndef _WIN32
    bouton_affiche_permission = new_checkbox_with_title ( _("Do not warn if account file is readable by someone else"),
							  &(etat.display_message_file_readable), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_affiche_permission, FALSE, FALSE, 0 );
#endif
    /* Display a warning message if file is already opened */
    bouton_display_lock_active = new_checkbox_with_title ( _("Do not warn about an already opened file"),
							   &(etat.display_message_lock_active), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_display_lock_active, FALSE, FALSE, 0 );

    /* Display a warning message that QIF doesn't contain currencies */
    bouton_display_lock_active = new_checkbox_with_title ( _("Do not warn about about QIF not containing currencies"),
							   &(etat.display_message_qif_export_currency), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_display_lock_active, FALSE, FALSE, 0 );


    /* Number of days before a warning message advertising a scheduled
       transaction */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					    _("Scheduler warnings"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);
    label = gtk_label_new ( SPACIFY(COLON(_("Number of days before a warning message advertising a scheduled transaction"))) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0 );

    entree_jours = new_int_spin_button ( &(decalage_echeance),
					 /* Limit to one year */
					 0, 365, 1, 5, 1, 1, 0, NULL ); 
    gtk_box_pack_start ( GTK_BOX ( hbox ), entree_jours,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all ( vbox_pref );

    if ( !nb_comptes )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}


/** 
 * Changes backup file name upon every keystroke on entry.
 *
 * \param entry Widget that triggered handler
 * \param value Not used
 * \param length Not used
 * \param position Not used
 *
 * \returns FALSE
 */
gboolean change_backup_path ( GtkEntry *entry, gchar *value, gint length, gint * position )
{
    nom_fichier_backup = g_strdup ( gtk_entry_get_text ( GTK_ENTRY(entry) ));

    if ( nom_fichier_backup && !strlen(nom_fichier_backup) )
    {
	nom_fichier_backup = NULL;
    }

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return ( FALSE );
}



/**
 * Creates the "Files" tab.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_fichier ( void )
{
    GtkWidget *vbox_pref, *paddingbox;
    GtkWidget *hbox;
    GtkWidget *label;

    vbox_pref = new_vbox_with_title_and_icon ( _("Files"),
					       "files.png" );

    /* Account file handling */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					    _("Account files handling"));

    /* Automatically load last file on startup? */
    bouton_avec_demarrage =
	new_checkbox_with_title (_("Automatically load last file on startup"),
				 &(etat.dernier_fichier_auto), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_avec_demarrage, 
			 FALSE, FALSE, 0 );

    bouton_save_auto = new_checkbox_with_title (_("Automatically save on exit"),
						&(etat.sauvegarde_auto), NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_save_auto, 
			 FALSE, FALSE, 0 );

    /* Warn if file is used by someone else? */
    bouton_force_enregistrement = 
	new_checkbox_with_title ( _("Force saving of locked files"),
				  &(etat.force_enregistrement), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_force_enregistrement,
			 FALSE, FALSE, 0 );

    /* Compression level of files */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0 );
    label = gtk_label_new ( COLON(_("File compression level")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0 );
    spin_button_compression_fichier = 
	new_int_spin_button ( &(compression_fichier),
			      0.0, 9.0, 1.0, 5.0, 1.0, 1.0, 0, NULL );

    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button_compression_fichier,
			 FALSE, FALSE, 0 );

    /* Memorize last opened files in menu */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0 );
    label = gtk_label_new ( COLON(_("Memorise last opened files")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0 );
    spin_button_derniers_fichiers_ouverts = 
	new_int_spin_button ( &(nb_max_derniers_fichiers_ouverts),
			      0.0, 20.0, 1.0, 5.0, 1.0, 1.0, 0, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button_derniers_fichiers_ouverts,
			 FALSE, FALSE, 0 );

    /* Backups */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					    _("Backups"));

    /* Backup at each opening? */
    bouton_save_demarrage = 
	new_checkbox_with_title ( _("Make a backup copy after opening files"),
				  &(etat.sauvegarde_demarrage), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_save_demarrage,
			 FALSE, FALSE, 0 );

    /* Automatic backup ? */
    bouton_demande_backup = new_checkbox_with_title (_("Make a backup copy before saving files"),
						     NULL, (GCallback) changement_choix_backup);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_demande_backup,
			 FALSE, FALSE, 0 );

    if ( nb_comptes )
    {
	gboolean dummy = (nom_fichier_backup != NULL &&
			  strlen(nom_fichier_backup) > 0);
	/* Ugly dance ... */
	checkbox_set_value ( bouton_demande_backup, &dummy, FALSE );

	/* Mise en forme de l'entrée du chemin de la backup */
	hbox = gtk_hbox_new ( FALSE, 5 );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			     FALSE, FALSE, 0 );

	label = gtk_label_new ( COLON(_("Backup file")) );
	gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			     FALSE, FALSE, 0 );

	entree_chemin_backup = gtk_entry_new ();

	if ( nom_fichier_backup && strlen(nom_fichier_backup) )
	{
	    gtk_entry_set_text ( GTK_ENTRY ( entree_chemin_backup ),
				 nom_fichier_backup );
	    gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ),
				       TRUE );
	}
	else
	    gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ),
				       FALSE );

	g_signal_connect_after ( GTK_OBJECT ( entree_chemin_backup),
				 "insert-text",
				 (GCallback) change_backup_path,
				 NULL);
	g_signal_connect_after ( GTK_OBJECT ( entree_chemin_backup),
				 "delete-text",
				 (GCallback) change_backup_path,
				 NULL);
	gtk_box_pack_start ( GTK_BOX ( hbox ), entree_chemin_backup,
			     TRUE, TRUE, 0 );
    }
    else
    {
	gtk_widget_set_sensitive ( bouton_demande_backup, FALSE );
    }


    /* Compression level of backups */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Backup compression level")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0 );

    spin_button_compression_backup = new_int_spin_button ( &(compression_backup),
							   0, 9, 1, 5, 1, 1, 0, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button_compression_backup,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all ( vbox_pref );

    if ( !nb_comptes )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}





/* **************************************************************************************************************************** */
void changement_choix_backup ( GtkWidget *bouton,
			       gpointer pointeur )
{
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup )))
    {
	gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ), TRUE );
	if (! nom_fichier_backup || !strlen(nom_fichier_backup) )
	{
	    gtk_entry_set_text ( GTK_ENTRY(entree_chemin_backup), _("backup.gsb") );
	    change_backup_path ( GTK_ENTRY(entree_chemin_backup), NULL, 0, 0 );
	}
    }
    else
    {
	gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ), FALSE );
	nom_fichier_backup = NULL;
    }
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
/* fonction qui ferme la fenêtre préférences */
/* **************************************************************************************************************************** */
void fermeture_preferences ( GtkWidget *button,
			     gint page,
			     gpointer data )
{
    GtkTreeIter iter;

    gtk_tree_model_get_iter_first(GTK_TREE_MODEL (preference_tree_model), &iter);

    do
    {
	GValue value = {0, };
	gtk_tree_model_get_value (GTK_TREE_MODEL (preference_tree_model), 
				  &iter, 1, &value);
	gtk_widget_destroy(g_value_get_pointer(&value));
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (preference_tree_model), &iter));

    preference_selected = 0;
    gtk_widget_destroy (fenetre_preferences);
}
/* **************************************************************************************************************************** */

/* *******************************************************************************/
/* page de configuration des logiciels externes */
/* *******************************************************************************/
GtkWidget *onglet_programmes (void)
{
    GtkWidget *vbox_pref, *label, *entree, *paddingbox, *table;
    GtkSizeGroup *size_group;

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    vbox_pref = new_vbox_with_title_and_icon ( _("Programs"), "exec.png" ); 

    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Print support"));
    
    table = gtk_table_new ( 0, 2, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    label = gtk_label_new ( _("LaTeX command"));
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entree = new_text_entry ( &etat.latex_command, NULL );
    gtk_table_attach ( GTK_TABLE(table), entree, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    label = gtk_label_new ( _("dvips command"));
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entree = new_text_entry ( &etat.dvips_command, NULL );
    gtk_table_attach ( GTK_TABLE(table), entree, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );


    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Misc"));

    table = gtk_table_new ( 0, 2, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    label = gtk_label_new ( _("Web browser"));
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entree = new_text_entry ( &etat.browser_command, NULL );
    gtk_table_attach ( GTK_TABLE(table), entree, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    gtk_size_group_set_mode ( size_group, GTK_SIZE_GROUP_HORIZONTAL );
    gtk_widget_show_all ( vbox_pref );

    if ( !nb_comptes )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}
/* *******************************************************************************/
