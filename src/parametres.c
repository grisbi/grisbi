/* permet la configuration du logiciel */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003 Benjamin Drieu (bdrieu@april.org) */
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
#include "en_tete.h"

GtkTreeStore *preference_tree_model;
GtkWidget * hpaned;
GtkNotebook * preference_frame;
gint preference_selected = -1;
GtkTreeSelection * selection;
GtkWidget * button_close, * button_help;

/** FIXME move it */
gboolean set_text_from_area ( GtkTextBuffer *buffer, gpointer tamere );


/**
 * Creates a simple TreeView and a TreeModel to handle preference
 * tabs.  Sets preference_tree_model to the newly created TreeModel.
 *
 * \returns a GtkScrolledWindow
 */
GtkWidget * create_preferences_tree ( )
{
  GtkWidget *tree, *item, *sw;
  GList *items = NULL;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;

  /* Create model */
  preference_tree_model = gtk_tree_store_new (2, 
					      G_TYPE_STRING, 
					      G_TYPE_INT);

  /* Create container + TreeView */
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_AUTOMATIC);
  tree = gtk_tree_view_new();
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), 
			   GTK_TREE_MODEL (preference_tree_model));

  /* Make column */
  cell = gtk_cell_renderer_text_new ();
  column = 
    gtk_tree_view_column_new_with_attributes ("Categories",
					      cell,
					      "text", 0,
					      NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree),
			       GTK_TREE_VIEW_COLUMN (column));

  /* Handle select */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  g_signal_connect (selection, "changed", 
		    G_CALLBACK (selectionne_liste_preference), 
		    preference_tree_model);

  /* Put the tree in the scroll */
  gtk_container_add (GTK_CONTAINER (sw), tree);

  /* expand all rows after the treeview widget has been realized */
  g_signal_connect (tree, "realize",
		    G_CALLBACK (gtk_tree_view_expand_all), NULL);

  return sw;
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
  GtkWidget *tree, *right, *w, *vbox, *hbox, *hbox2, *separator,
    *button_ok, *button_cancel;
  GtkTreeIter iter, iter2;
  GtkTreePath * path;

  /* Create dialog */
  fenetre_preferences = gtk_dialog_new ();
  gtk_dialog_add_buttons (GTK_DIALOG(fenetre_preferences), 
			  GTK_STOCK_HELP,  GTK_RESPONSE_HELP,
			  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			  NULL);
  gtk_window_set_title ( GTK_WINDOW ( fenetre_preferences ),
			 _("Grisbi setup") );

  /* Create List & Tree for topics */
  tree = create_preferences_tree();  
  hpaned = gtk_hpaned_new();
  gtk_paned_add1(GTK_PANED(hpaned), tree);
  hbox = gtk_hbox_new ( FALSE, 0 );
  gtk_paned_add2(GTK_PANED(hpaned), hbox);

  gtk_box_pack_start ( GTK_BOX ( hbox ), gtk_label_new ( "    " ),
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
		      0, _("Transaction list"),
		      1, TRANSACTION_LIST_PAGE,
		      -1);
  gtk_notebook_append_page (preference_frame, onglet_affichage_liste(), NULL);
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
		      0, _("Fonts & logo"),
		      1, FONTS_AND_LOGO_PAGE,
		      -1);
  gtk_notebook_append_page (preference_frame, onglet_display_fonts(), NULL);

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
  gtk_notebook_append_page (preference_frame, 
			    onglet_types_operations(),
			    NULL);

  if ( page )
    {
      gtk_notebook_set_current_page ( preference_frame, page-1 );
    }

  gtk_widget_show_all ( hpaned );
  gtk_container_set_border_width ( GTK_CONTAINER(hpaned), 6 );
  gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(fenetre_preferences) -> vbox ), 
		       hpaned, TRUE, TRUE, 0);

  while ( 1 )
    {
      switch (gtk_dialog_run ( GTK_DIALOG ( fenetre_preferences ) ))
	{
	case GTK_RESPONSE_CLOSE:
	  gtk_widget_destroy ( GTK_WIDGET ( fenetre_preferences ));
	  return;
	case GTK_RESPONSE_HELP:
	  break;
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
  GtkWidget widget;

  if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
    return;

  gtk_tree_model_get_value (model, &iter, 1, &value);

/*   if (preference_selected) */
/*     { */
/*       gtk_widget_hide_all(preference_selected); */
/*       g_object_ref(preference_selected); /\* GRUIK *\/ */
/*       gtk_container_remove (GTK_CONTAINER (preference_frame), preference_selected); */
/*     } */

  preference_selected = g_value_get_int(&value);
  if (preference_selected != -1)
    {
      gtk_notebook_set_page (preference_frame, preference_selected);
    }

  g_value_unset (&value);

  return FALSE;
}


/* ************************************************************************************************************** */
/* activer le bouton appliquer */
/* ************************************************************************************************************** */
void activer_bouton_appliquer ( )
{
/*   gtk_widget_set_sensitive ( button_apply, TRUE ); */
}


/**
 * Creates the "Warning & Messages" tab.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_messages_and_warnings ( void )
{
  GtkWidget *hbox, *vbox_pref;
  GtkWidget *separateur;
  GtkWidget *paddingbox, *label;

  vbox_pref = new_vbox_with_title_and_icon ( _("Messages & warnings"),
					     "warnings.png" );

   /* Warnings */
  paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					  _("Warnings messages"));

  /* Display a warning message if minimum balances are under minimum level */
  bouton_solde_mini = new_checkbox_with_title ( _("Display a warning message if minimum balances are under minimum level"),
						&(etat.alerte_mini), NULL );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_solde_mini,
		       FALSE, FALSE, 0 );

  /* Display a warning message if account file is readable by someone else */
  bouton_affiche_permission = new_checkbox_with_title ( _("Display a warning message if account file is readable by someone else"),
						&(etat.alerte_permission), NULL );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_affiche_permission,
		       FALSE, FALSE, 0 );

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
  entree_jours = gtk_entry_new_with_max_length (10);
  gtk_widget_set_usize ( GTK_WIDGET ( entree_jours ), 60, FALSE );
  gtk_entry_set_text ( GTK_ENTRY ( entree_jours ),
		       g_strdup_printf ( "%d", decalage_echeance ));
  gtk_signal_connect_object ( GTK_OBJECT ( entree_jours ), "changed",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_end ( GTK_BOX ( hbox ), entree_jours,
		     FALSE, FALSE, 0 );

  gtk_widget_show_all ( vbox_pref );

  return ( vbox_pref );

}



/**
 * Creates the "Files" tab.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_fichier ( void )
{
  GtkWidget *box_pref1, *vbox_pref, *paddingbox;
  GtkWidget *separateur;
  GtkWidget *vbox;
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
    new_checkbox_with_title ( _("Warn if file is used by someone else"),
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
    gtk_spin_button_new ( GTK_ADJUSTMENT 
			  ( gtk_adjustment_new ( compression_fichier,
						 0, 9, 1, 5, 1 )),
			  1, 0 );
  gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON ( spin_button_compression_fichier ),
				TRUE );
  gtk_signal_connect_object ( GTK_OBJECT ( spin_button_compression_fichier ),
			      "changed",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
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
    gtk_spin_button_new ( GTK_ADJUSTMENT ( gtk_adjustment_new ( nb_max_derniers_fichiers_ouverts,
								0, 20, 1, 5, 1 )),
			  1, 0 );
  gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON ( spin_button_derniers_fichiers_ouverts ),
				TRUE );
  gtk_signal_connect_object ( GTK_OBJECT ( spin_button_derniers_fichiers_ouverts ),
			      "changed",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
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
						   NULL, changement_choix_backup);
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_demande_backup,
		       FALSE, FALSE, 0 );

  if ( nb_comptes )
    {
      if ( nom_fichier_backup )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ),
				       TRUE );
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ),
				       FALSE );

      /* Mise en forme de l'entrée du chemin de la backup */
      hbox = gtk_hbox_new ( FALSE, 5 );
      gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			   FALSE, FALSE, 0 );

      label = gtk_label_new ( COLON(_("Backup file")) );
      gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			   FALSE, FALSE, 0 );
      gtk_widget_show ( label );

      entree_chemin_backup = gnome_file_entry_new ( "backup_grisbi",
						    _("Grisbi backup") );
      gtk_widget_set_usize ( gnome_file_entry_gnome_entry ( GNOME_FILE_ENTRY ( entree_chemin_backup )),
			     300,
			     FALSE );
      gnome_file_entry_set_modal ( GNOME_FILE_ENTRY ( entree_chemin_backup ),
				   TRUE );

      if ( nom_fichier_backup )
	{
	  gtk_entry_set_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY (entree_chemin_backup ))),
			       nom_fichier_backup );
	  gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ),
				     TRUE );
	}
      else
	gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ),
				   FALSE );
      
      gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBO ( gnome_file_entry_gtk_entry ( entree_chemin_backup) ) -> entry ),
				  "changed",
				  activer_bouton_appliquer,
				  GTK_OBJECT (fenetre_preferences));
      gtk_box_pack_start ( GTK_BOX ( hbox ), entree_chemin_backup,
			   FALSE, FALSE, 0 );
      gtk_widget_show ( entree_chemin_backup );
    }
  else
    {
      gtk_widget_set_sensitive ( bouton_demande_backup, FALSE );
    }


  /* Compression level of backups */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Backup compression level")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( label );

  spin_button_compression_backup = 
    gtk_spin_button_new ( GTK_ADJUSTMENT ( gtk_adjustment_new ( compression_backup,
								0, 9, 1, 5, 1 )),
			  1, 0 );
  gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON ( spin_button_compression_backup ),
				TRUE );
  gtk_signal_connect_object ( GTK_OBJECT ( spin_button_compression_backup ),
			      "changed",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button_compression_backup,
		     FALSE, FALSE, 0 );
  gtk_widget_show ( spin_button_compression_backup );

  gtk_widget_show_all ( vbox_pref );
  return ( vbox_pref );

}



/* FIXME: deprecated */
GtkWidget *onglet_echeances ( void )
{
}



/* ************************************************************************************************************ */
void change_selection_verif ( GtkWidget *liste_comptes,
			       gint ligne,
			       gint colonne,
			       GdkEventButton *event,
			       gint * data )
{
  *data = ligne;
  gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_enlever ),
			     TRUE );
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void changement_utilisation_applet ( void )
{
  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affichage_applet ) ) )
    {
      gtk_widget_set_sensitive ( GTK_WIDGET ( frame_demarrage ),
				 TRUE );
      if ( fichier_verifier_selectionne != -1 )
	gtk_clist_select_row ( GTK_CLIST ( liste_comptes_verifies ),
			       fichier_verifier_selectionne,
			       0 );
    }
  else
    {
      gtk_widget_set_sensitive ( GTK_WIDGET ( frame_demarrage ),
				 FALSE );
      gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_verifies ) );
    }
}
/* **************************************************************************************************************************** */





/* **************************************************************************************************************************** */
void ajouter_verification ( GtkWidget *bouton_add,
			    GtkWidget *fenetre_preferences  )
{
  GtkWidget *dialogue_box,*label;
  GtkWidget *fenetre_choix_fichier;
  gint retour;
  gchar *nom_fichier;
  GSList *pointeur_liste;
  FILE *fichier;

  dialogue_box = gnome_dialog_new ( _("Looking for the Grisbi file"),
				GNOME_STOCK_BUTTON_OK,
				GNOME_STOCK_BUTTON_CANCEL,
				NULL );
  gnome_dialog_set_default ( GNOME_DIALOG ( dialogue_box ),
			     0 );
  gnome_dialog_set_parent ( GNOME_DIALOG ( dialogue_box ),
			    GTK_WINDOW ( fenetre_preferences ) );
  gtk_window_set_modal ( GTK_WINDOW ( dialogue_box ),
			 TRUE );
  gtk_signal_connect ( GTK_OBJECT ( dialogue_box ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
		        "destroy" );

  label = gtk_label_new ( COLON(_("Enter the accounts file")) );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue_box ) -> vbox ),
		       label,
		       TRUE,
		       FALSE,
		       5);
  gtk_widget_show ( label );


  fenetre_choix_fichier = gnome_file_entry_new ( "fichier_grisbi",
						 _("Grisbi file") );
  gnome_file_entry_set_default_path ( GNOME_FILE_ENTRY ( fenetre_choix_fichier ),
				      dernier_chemin_de_travail );
  gtk_widget_set_usize ( gnome_file_entry_gnome_entry ( GNOME_FILE_ENTRY ( fenetre_choix_fichier )),
			 300,
			 FALSE );
  
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue_box ) -> vbox ),
		       fenetre_choix_fichier,
		       TRUE,
		       FALSE,
		       5);
  gtk_widget_show ( fenetre_choix_fichier );


  retour = gnome_dialog_run ( GNOME_DIALOG ( dialogue_box ) );

  switch ( retour )
    {

    case 0 :
      /* vérification que le fichier n'est pas déjà dans la liste */

      nom_fichier = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY (fenetre_choix_fichier )))));
      
      if ( !strlen ( nom_fichier ))
	{
	  gnome_dialog_close  ( GNOME_DIALOG ( dialogue_box ) );
	  return;
	}
	  
      if ( ( pointeur_liste = fichier_a_verifier ))
	{
	  do
	    if ( !strcmp ( pointeur_liste -> data, nom_fichier ) )
	      {
		gnome_dialog_close  ( GNOME_DIALOG ( dialogue_box ) );
		dialogue ( SPACIFY(_("This accounts file has already been checked!")));
		return;
	      }
	  while ( ( pointeur_liste = pointeur_liste-> next ));
	}

      /* vérification que c'est bien un fichier grisbi */

      fichier = fopen ( nom_fichier,
			"r" );

      if ( fichier )
	{
	  /* vérifications que la 2ème ligne est bien <Grisbi> */

	  gchar buffer[9];

	  fscanf ( fichier,
		   "%*[^\n]\n" );
	  fscanf ( fichier,
		   "%8s",
		   buffer );

	  if ( strcmp ( buffer,
			"<Grisbi>" ))
	    {
	      gnome_dialog_close  ( GNOME_DIALOG ( dialogue_box ) );
	      dialogue ( _("This file is not a grisbi file") );
	      fclose ( fichier );
	      return;
	    }

	  fclose ( fichier );
	}
      else
	{
	  gnome_dialog_close  ( GNOME_DIALOG ( dialogue_box ) );
	  dialogue ( g_strconcat ( _("Error:\n"),
				   strerror ( errno ),
				   NULL ));
	  return;
	}


      /*   c'est bien un fichier grisbi, on l'ajoute à la liste */

      nom_fichier = g_strdup ( nom_fichier );
      fichier_a_verifier_tmp = g_slist_append ( fichier_a_verifier_tmp,
						nom_fichier );
      nb_fichier_verifier++;
      gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_verifies ),
			       gtk_clist_append ( GTK_CLIST ( liste_comptes_verifies ),&nom_fichier ),
			       nom_fichier );

      activer_bouton_appliquer ( );

    default:
      gnome_dialog_close  ( GNOME_DIALOG ( dialogue_box ) );
    }

}
/* **************************************************************************************************************************** */




/* **************************************************************************************************************************** */
void supprimer_verification ( GtkWidget *bouton_supp,
			      GtkWidget *fenetre_preferences )
{

  if ( fichier_verifier_selectionne == -1 )
    return;
  liste_suppression_fichier_a_verifier = g_slist_append ( liste_suppression_fichier_a_verifier,
							  gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_verifies ), fichier_verifier_selectionne ));
  
  gtk_clist_remove ( GTK_CLIST ( liste_comptes_verifies ), 
		     fichier_verifier_selectionne );
  nb_fichier_verifier--;
  if ( fichier_verifier_selectionne == nb_fichier_verifier )
    fichier_verifier_selectionne--;

  if ( fichier_verifier_selectionne != -1 )
    gtk_clist_select_row ( GTK_CLIST ( liste_comptes_verifies ), 
			   fichier_verifier_selectionne ,
			   0 );
  else
    gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_enlever ),
			       FALSE );
    
  activer_bouton_appliquer (  );

}
/* **************************************************************************************************************************** */




/* **************************************************************************************************************************** */
void changement_choix_backup ( GtkWidget *bouton,
			       gpointer pointeur )
{
  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup )))
    gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ), TRUE );
  else
    {
      gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ), FALSE );
      nom_fichier_backup = NULL;
    }
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void real_changement_preferences ( GtkWidget *fenetre_preferences,
				   gint page,
				   gpointer data )
{
  gint buffer;
  gchar *home_dir;
  gint i, j;
  GSList *liste_tmp;
  GtkWidget *menu;
  GtkTooltips *tooltip;

  home_dir = getenv ("HOME");


  switch ( page )
    {
    case 0 : 

      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_entree_enregistre ) ) )
	etat.entree = 1;
      else
	etat.entree = 0;

      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_solde_mini ) ) )
	etat.alerte_mini = 1;
      else
	etat.alerte_mini = 0;

      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_permission ) ) )
	etat.alerte_permission = 1;
      else
	etat.alerte_permission = 0;

      if ( titre_fichier )
	{
	  if ( strcmp ( titre_fichier,
			g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier )))))
	    {
	      modification_fichier ( TRUE );
	      titre_fichier = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier ))));

	      if ( strlen ( titre_fichier ))
		gtk_label_set_text ( GTK_LABEL ( label_titre_fichier ),
				      titre_fichier );
	      else
		{
		  titre_fichier = NULL;
		  gtk_widget_destroy ( label_titre_fichier );
		}
	    }
	}
      else
	{
	  if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier )))))
	    {
	      titre_fichier = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier ))));
	      label_titre_fichier = gtk_label_new ( titre_fichier );
	      gtk_box_pack_start ( GTK_BOX ( ((GtkBoxChild *)(GTK_BOX ( page_accueil )->children->data))->widget ),
				   label_titre_fichier,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_box_reorder_child ( GTK_BOX ( ((GtkBoxChild *)(GTK_BOX ( page_accueil )->children->data))->widget ),
				      label_titre_fichier,
				      1 );
	      gtk_widget_show ( label_titre_fichier );
	      modification_fichier ( TRUE );
	    }
	}


      if ( adresse_commune )
	{
	  if ( strcmp ( adresse_commune,
			g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_commune ),
							     0,
							     -1 ))))
	    {
	      modification_fichier ( TRUE );
	      adresse_commune = g_strdup ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_commune ),
										0,
										-1 )));

	      if ( strlen ( adresse_commune ))
		{
		  adresse_commune = g_strdelimit ( adresse_commune,
						   "{",
						   '(' );
		  adresse_commune = g_strdelimit ( adresse_commune,
						   "}",
						   ')' );
		}
	      else
		adresse_commune = NULL;
	    }
	}
      else
	{
	  if ( strlen ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_commune ),
							     0,
							     -1 ))))
	    {
	      adresse_commune = g_strdup ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_commune ),
										0,
										-1 )));
	      adresse_commune = g_strdelimit ( adresse_commune,
					       "{",
					       '(' );
	      adresse_commune = g_strdelimit ( adresse_commune,
					       "}",
					       ')' );
	      modification_fichier ( TRUE );
	    }
	}

      if ( adresse_secondaire )
	{
	  if ( strcmp ( adresse_secondaire,
			g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_secondaire ),
							     0,
							     -1 ))))
	    {
	      modification_fichier ( TRUE );
	      adresse_secondaire = g_strdup ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_secondaire ),
										0,
										-1 )));

	      if ( strlen ( adresse_secondaire ))
		{
		  adresse_secondaire = g_strdelimit ( adresse_secondaire,
						   "{",
						   '(' );
		  adresse_secondaire = g_strdelimit ( adresse_secondaire,
						   "}",
						   ')' );
		}
	      else
		adresse_secondaire = NULL;
	    }
	}
      else
	{
	  if ( strlen ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_secondaire ),
							     0,
							     -1 ))))
	    {
	      adresse_secondaire = g_strdup ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( entree_adresse_secondaire ),
										0,
										-1 )));
	      adresse_secondaire = g_strdelimit ( adresse_secondaire,
					       "{",
					       '(' );
	      adresse_secondaire = g_strdelimit ( adresse_secondaire,
					       "}",
					       ')' );
	      modification_fichier ( TRUE );
	    }
	}

      break;


    case 1 :

      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_avec_demarrage ) )  )
	etat.dernier_fichier_auto = 1;
      else
	etat.dernier_fichier_auto = 0;

      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_save_auto ) )  )
	etat.sauvegarde_auto = 1;
      else
	etat.sauvegarde_auto = 0;

      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_force_enregistrement ) )  )
	etat.force_enregistrement = 1;
      else
	etat.force_enregistrement = 0;

      if ( nb_comptes )
	{
	  /* 	  si on a touché au bouton save auto, le fichier devra être enregistré */

	  if ( ( !nom_fichier_backup && gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ) ))
	       ||
	       ( nom_fichier_backup && !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ) ))
	       ||
	       ( nom_fichier_backup && strcmp ( nom_fichier_backup,
						g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( GTK_COMBO ( gnome_file_entry_gtk_entry (entree_chemin_backup) ) -> entry  ) )))))
	    modification_fichier ( TRUE );

	  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ) )
	       &&
	       strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( GTK_COMBO ( gnome_file_entry_gtk_entry (entree_chemin_backup)) -> entry  ) ) )))
	    /*** BENJ FIXME : heu .... c'est vraiment ça ? ***/
	    nom_fichier_backup = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( GTK_COMBO (  gnome_file_entry_gtk_entry(entree_chemin_backup) ) -> entry  ) ) ));
	  else
	    nom_fichier_backup = NULL;
	}


      etat.sauvegarde_demarrage = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_save_demarrage ));


      /* récupère le nb max de noms de derniers fichiers à garder */

      efface_derniers_fichiers_ouverts ();

      nb_max_derniers_fichiers_ouverts = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button_derniers_fichiers_ouverts ));
      if ( nb_derniers_fichiers_ouverts > nb_max_derniers_fichiers_ouverts )
	nb_derniers_fichiers_ouverts = nb_max_derniers_fichiers_ouverts;

      affiche_derniers_fichiers_ouverts ();

      /* récupère la compression */

      compression_fichier = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button_compression_fichier ));
      compression_backup = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button_compression_backup ));

      /*       par-défaut, on compresse le fichier */

      xmlSetCompressMode ( compression_fichier );

      if ( compression_fichier )
	dialogue ( _("Warning: the compression disables the multi-users checking.") );

      break;


    case 2 :

      if ( ( buffer = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_jours ) ), FALSE ) ) != decalage_echeance )
	{
	  decalage_echeance = buffer;
	  update_liste_echeances_manuelles_accueil();
	  update_liste_echeances_auto_accueil();
	}

    case 3 :
      /* onglet affichage */

  
      /*       on place les comptes selon les préférences */

      g_slist_free ( ordre_comptes );
      ordre_comptes = NULL;

      for ( buffer = 0 ; buffer < nb_comptes ; buffer++ )
	ordre_comptes = g_slist_append ( ordre_comptes,
					gtk_clist_get_row_data ( GTK_CLIST ( liste_choix_ordre_comptes ),
								 buffer ) );

      /* on récupère les préférences du formulaire */

      etat.affiche_no_operation = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_operation ));
      etat.affiche_date_bancaire = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_bancaire ));
      etat.utilise_exercice = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_exercices ));
      etat.utilise_imputation_budgetaire = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_imputation_budgetaire ));
      etat.utilise_piece_comptable = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_piece_comptable ));
      etat.utilise_info_banque_guichet = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_info_banque_guichet ));
      etat.affiche_boutons_valider_annuler = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_boutons_valider_annuler ));

      /* on sensitive ou non/affiche ou non les widget du formulaire si un fichier est ouvert */

      if ( nb_comptes )
	{
	  if ( etat.affiche_no_operation )
	    gtk_widget_show ( widget_formulaire_operations[0] );
	  else
	    gtk_widget_hide ( widget_formulaire_operations[0] );


	  gtk_widget_set_sensitive ( widget_formulaire_operations[7],
				     etat.affiche_date_bancaire );

	  gtk_widget_set_sensitive ( widget_formulaire_operations[11],
				     etat.utilise_exercice );
	  gtk_widget_set_sensitive ( widget_formulaire_ventilation[6],
				     etat.utilise_exercice );
	  gtk_widget_set_sensitive ( widget_formulaire_echeancier[9],
				     etat.utilise_exercice );

	  gtk_widget_set_sensitive ( widget_formulaire_operations[12],
				     etat.utilise_imputation_budgetaire );
	  gtk_widget_set_sensitive ( page_imputations,
				     etat.utilise_imputation_budgetaire );
	  gtk_widget_set_sensitive ( widget_formulaire_ventilation[4],
				     etat.utilise_imputation_budgetaire );
	  gtk_widget_set_sensitive ( widget_formulaire_echeancier[10],
				     etat.utilise_imputation_budgetaire );

	  gtk_widget_set_sensitive ( widget_formulaire_operations[14],
				     etat.utilise_piece_comptable );
	  gtk_widget_set_sensitive ( widget_formulaire_ventilation[7],
				     etat.utilise_piece_comptable );
	  gtk_widget_set_sensitive ( widget_formulaire_echeancier[12],
				     etat.utilise_piece_comptable );

	  gtk_widget_set_sensitive ( widget_formulaire_operations[17],
				     etat.utilise_info_banque_guichet );
	  gtk_widget_set_sensitive ( widget_formulaire_echeancier[11],
				     etat.utilise_info_banque_guichet );

	  if ( etat.affiche_boutons_valider_annuler )
	    {
	      gtk_widget_show ( separateur_formulaire_operations );
	      gtk_widget_show ( hbox_valider_annuler_ope );
	      gtk_widget_show ( separateur_formulaire_echeancier );
	      gtk_widget_show ( hbox_valider_annuler_echeance );
	    }
	  else
	    {
	      gtk_widget_hide ( separateur_formulaire_operations );
	      gtk_widget_hide ( hbox_valider_annuler_ope );
	      gtk_widget_hide ( separateur_formulaire_echeancier );
	      gtk_widget_hide ( hbox_valider_annuler_echeance );

	      affiche_cache_le_formulaire ();
	      affiche_cache_le_formulaire ();
	    }


	  /* on récupère le no de la devise des totaux des tiers si un fichier est ouvert */

	  if ( no_devise_totaux_tiers != GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ) -> menu_item ),
										 "no_devise" )))
	    {
	      no_devise_totaux_tiers = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ) -> menu_item ),
									       "no_devise" ));
	  

	      /* si on est sur une des onglets d'affichage des tiers, categ ou imput, on le réaffiche */

	      switch ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general )))
		{
		case 4:
		  remplit_arbre_tiers ();
		  modif_categ = 1;
		  modif_imputation = 1;
		  break;

		case 5:
		  remplit_arbre_categ ();
		  modif_tiers = 1;
		  modif_imputation = 1;
		  break;

		case 6:
		  remplit_arbre_imputation ();
		  modif_categ = 1;
		  modif_tiers = 1;
		  break;
		}
	    }
	}

      /* récupération de l'affichage du nombre d'écritures */
      /* peut se faire même si aucun fichier n'est ouvert, mais dans ce cas, ne modifie */
      /* pas les titres de colonnes */
      

      if ( etat.affiche_nb_ecritures_listes != gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_ecritures )))
	{
	  etat.affiche_nb_ecritures_listes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_ecritures ));

	  /* change les titres des listes si un fichier est ouvert */

	  if ( nb_comptes )
	    {
	      if ( etat.affiche_nb_ecritures_listes )
		{
		  gtk_clist_set_column_title ( GTK_CLIST ( arbre_tiers ),
					       0,
					       _("Third parties list (transactions)") );
		  gtk_clist_set_column_title ( GTK_CLIST ( arbre_categ ),
					       0,
					       _("Categories list (transactions)") );
		  gtk_clist_set_column_title ( GTK_CLIST ( arbre_imputation ),
					       0,
					       _("Budgetary lines list (transactions)") );
		}
	      else
		{
		  gtk_clist_set_column_title ( GTK_CLIST ( arbre_tiers ),
					       0,
					       _("Third parties list") );
		  gtk_clist_set_column_title ( GTK_CLIST ( arbre_categ ),
					       0,
					       _("Categories list") );
		  gtk_clist_set_column_title ( GTK_CLIST ( arbre_imputation ),
					       0,
					       _("Budgetary lines list") );
		}

	      mise_a_jour_tiers ();
	      mise_a_jour_categ ();
	      mise_a_jour_imputation ();
	    }
	}


      /* récupération du tri par date ou date bancaire */

      etat.classement_par_date = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_classer_liste_par_date ));

      if ( nb_comptes )
	{
	  reaffiche_liste_comptes ();
	  update_liste_comptes_accueil();
	  modification_fichier ( TRUE );
	  demande_mise_a_jour_tous_comptes();
	  verification_mise_a_jour_liste();
	}
      break;

    case 4 :
      if ( liste_suppression_fichier_a_verifier )
	{
	  GSList *liste_suppression_fichier_a_verifier_variable;
	  
	  liste_suppression_fichier_a_verifier_variable = liste_suppression_fichier_a_verifier;

	  do
	    fichier_a_verifier = g_slist_remove ( fichier_a_verifier,
						  liste_suppression_fichier_a_verifier_variable->data );
	  while ( ( liste_suppression_fichier_a_verifier_variable = liste_suppression_fichier_a_verifier_variable->next ) );
	  
	  g_slist_free ( liste_suppression_fichier_a_verifier );
	  liste_suppression_fichier_a_verifier = NULL;

/* 	  si l'applet est démarrée, envoie le signal 10 ( SIGUSR1 ) pour qu'elle se mette à jour */

	  if ( verifie_affichage_applet() == 1 )
	    {
	      sauve_configuration ();
	      kill ( pid_applet, SIGUSR1 );
	    }
	}

      if ( fichier_a_verifier_tmp != NULL )
	{
	  GSList *fichier_a_verifier_tmp_variable;
	  
	  fichier_a_verifier_tmp_variable = fichier_a_verifier_tmp;

	  do
	    fichier_a_verifier = g_slist_append ( fichier_a_verifier,
					    fichier_a_verifier_tmp_variable  ->data );
	  while ( ( fichier_a_verifier_tmp_variable = fichier_a_verifier_tmp_variable ->next ) );
	  
	  g_slist_free ( fichier_a_verifier_tmp );
	  fichier_a_verifier_tmp = NULL;

/* 	  si l'applet est démarrée, envoie le signal 10 ( SIGUSR1 ) pour qu'elle se mette à jour */

	  if ( verifie_affichage_applet() == 1 )
	    {
	      sauve_configuration ();
	      kill ( pid_applet, SIGUSR1 );
	    }
	}

      /* FIXME: reintroduce if we add applet again */
/*       if ( ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affichage_applet ) ) ) && (!verifie_affichage_applet() ) ) */
/* 	system ( g_strconcat ( APPLET_BIN_DIR, "&", NULL ) ); */

      if ( ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affichage_applet ) ) ) && (verifie_affichage_applet()  ) )
	  kill ( pid_applet, SIGTERM );

      /* 	  reaffiche_liste_equilibrage(); */

      break;

    case 5 :

      if ( liste_struct_devises )
	g_slist_free ( liste_struct_devises );

      liste_struct_devises = liste_struct_devises_tmp;
      no_derniere_devise = no_derniere_devise_tmp;
      nb_devises = nb_devises_tmp;


      /* on modifie la liste des devises de l'option menu du formulaire */

      gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ) -> menu );
      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ),
				 creation_option_menu_devises ( -1,
								liste_struct_devises ));
      gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_echeancier[4] ) -> menu );
      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[4] ),
				 creation_option_menu_devises ( -1,
								liste_struct_devises ));

      /* on modifie la liste des devises de l'option menu du detail des comptes */

      gtk_widget_destroy ( GTK_OPTION_MENU ( detail_devise_compte ) -> menu );
      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_devise_compte ),
				 creation_option_menu_devises ( 0,
								liste_struct_devises ));
      gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte  ) -> menu ),
				  "selection-done",
				  GTK_SIGNAL_FUNC ( modif_detail_compte ),
				  GTK_OBJECT ( hbox_boutons_modif ) );
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
      gtk_option_menu_set_history ( GTK_OPTION_MENU (  detail_devise_compte),
				    g_slist_position ( liste_struct_devises,
						       g_slist_find_custom ( liste_struct_devises,
									     GINT_TO_POINTER ( DEVISE ),
									     ( GCompareFunc ) recherche_devise_par_no )));


      /* on recrée la liste_struct_devises_tmp qui est un copie de liste_struct_devises originale */
      /* avec laquelle on travaillera dans les parametres */

      liste_struct_devises_tmp = NULL;
      liste_tmp = liste_struct_devises;

      while ( liste_tmp )
	{
	  struct struct_devise *devise;
	  struct struct_devise *copie_devise;

	  devise = liste_tmp -> data;
	  copie_devise = malloc ( sizeof ( struct struct_devise ));

	  copie_devise -> no_devise = devise -> no_devise;
	  copie_devise -> nom_devise = g_strdup ( devise -> nom_devise );
	  copie_devise -> code_devise = g_strdup ( devise -> code_devise );
	  copie_devise -> code_iso4217_devise = g_strdup ( devise -> code_iso4217_devise );
	  copie_devise -> passage_euro = devise -> passage_euro;
	  copie_devise -> une_devise_1_egale_x_devise_2 = devise -> une_devise_1_egale_x_devise_2;
	  copie_devise -> no_devise_en_rapport = devise -> no_devise_en_rapport;
	  copie_devise -> change = devise -> change;

	  if ( devise -> date_dernier_change )
	    copie_devise -> date_dernier_change = g_date_new_dmy ( devise -> date_dernier_change -> day,
								   devise -> date_dernier_change -> month,
								   devise -> date_dernier_change -> year );
	  else
	    copie_devise -> date_dernier_change = NULL;

	  liste_struct_devises_tmp = g_slist_append ( liste_struct_devises_tmp,
						      copie_devise );
	  liste_tmp = liste_tmp -> next;
	}

      no_derniere_devise_tmp = no_derniere_devise;
      nb_devises_tmp = nb_devises;


      /* on réassocie les lignes de la liste avec les devises temporaires */

      liste_tmp = liste_struct_devises_tmp;

      while ( liste_tmp )
	{
	  gtk_clist_set_row_data ( GTK_CLIST ( clist_devises_parametres ),
				   g_slist_position ( liste_struct_devises_tmp,
						      liste_tmp ),
				   liste_tmp -> data );

	  liste_tmp = liste_tmp -> next;
	}

      /* on recrée le bouton devise de l'onglet affichage */

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ),
				 creation_option_menu_devises ( -1,
								liste_struct_devises ) );
      gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ) -> menu ),
				  "selection-done",
				  activer_bouton_appliquer,
				  GTK_OBJECT (fenetre_preferences));
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ),
				    g_slist_position ( liste_struct_devises,
						       g_slist_find_custom ( liste_struct_devises,
									     GINT_TO_POINTER ( no_devise_totaux_tiers ),
									     ( GCompareFunc ) recherche_devise_par_no )));

      /* on recrée les boutons de devises dans la conf de l'état */

      if ( onglet_config_etat )
	{
	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
				     creation_option_menu_devises ( 0,
								    liste_struct_devises ));
	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
				     creation_option_menu_devises ( 0,
								    liste_struct_devises ));
	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
				     creation_option_menu_devises ( 0,
								    liste_struct_devises ));
	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_montant_etat ),
				     creation_option_menu_devises ( 0,
								    liste_struct_devises ));

	  selectionne_devise_categ_etat_courant ();
	  selectionne_devise_ib_etat_courant ();
	  selectionne_devise_tiers_etat_courant ();
	}

      mise_a_jour_tiers ();
      mise_a_jour_categ ();
      mise_a_jour_imputation ();
      update_liste_comptes_accueil();
      update_liste_echeances_manuelles_accueil ();
      update_liste_echeances_auto_accueil ();
      modification_fichier ( TRUE );

      break;

    case 6 :

      if ( liste_struct_banques )
	g_slist_free ( liste_struct_banques );

      liste_struct_banques = liste_struct_banques_tmp;
      no_derniere_banque = no_derniere_banque_tmp;
      nb_banques = nb_banques_tmp;

      gtk_widget_destroy ( GTK_OPTION_MENU ( detail_option_menu_banque ) -> menu );
      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_option_menu_banque ),
				 creation_menu_banques () );
      gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_option_menu_banque  ) -> menu ),
				  "selection-done",
				  GTK_SIGNAL_FUNC ( modif_detail_compte ),
				  GTK_OBJECT ( hbox_boutons_modif ) );

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( detail_option_menu_banque ),
				    g_slist_position ( liste_struct_banques,
						       g_slist_find_custom ( liste_struct_banques,
									     GINT_TO_POINTER ( BANQUE ),
									     ( GCompareFunc ) recherche_banque_par_no )) + 1 );

      /* on recrée la liste tmp des banques */


      liste_struct_banques_tmp = NULL;
      liste_tmp = liste_struct_banques;

      while ( liste_tmp )
	{
	  struct struct_banque *banque;
	  struct struct_banque *copie_banque;

	  banque = liste_tmp -> data;
	  copie_banque = malloc ( sizeof ( struct struct_banque ));

	  copie_banque -> no_banque = banque -> no_banque;
	  copie_banque -> nom_banque = g_strdup ( banque -> nom_banque );
	  copie_banque -> code_banque = g_strdup ( banque -> code_banque );
	  copie_banque -> adr_banque = g_strdup ( banque -> adr_banque );
	  copie_banque -> tel_banque = g_strdup ( banque -> tel_banque );
	  copie_banque -> email_banque = g_strdup ( banque -> email_banque );
	  copie_banque -> web_banque = g_strdup ( banque -> web_banque );
	  copie_banque -> remarque_banque = g_strdup ( banque -> remarque_banque );
	  copie_banque -> nom_correspondant = g_strdup ( banque -> nom_correspondant );
	  copie_banque -> tel_correspondant = g_strdup ( banque -> tel_correspondant );
	  copie_banque -> email_correspondant = g_strdup ( banque -> email_correspondant );
	  copie_banque -> fax_correspondant = g_strdup ( banque -> fax_correspondant );

	  liste_struct_banques_tmp = g_slist_append ( liste_struct_banques_tmp,
						      copie_banque );
	  liste_tmp = liste_tmp -> next;
	}

      no_derniere_banque_tmp = no_derniere_banque;
      nb_banques_tmp = nb_banques;


      /* remplissage de la liste avec les banques temporaires */

      liste_tmp = liste_struct_banques_tmp;

      while ( liste_tmp )
	{
	  /* on associe à la ligne la struct de la banque */

	  gtk_clist_set_row_data ( GTK_CLIST ( clist_banques_parametres ),
				   g_slist_position ( liste_struct_banques_tmp,
						      liste_tmp ),
				   liste_tmp -> data );

	  liste_tmp = liste_tmp -> next;
	}


      modification_fichier ( TRUE );
      break;

      /* onglet exercices */

    case 7 :

      etat.affichage_exercice_automatique = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affichage_auto_exercice ));

      if ( nb_comptes )
	{
	  if ( liste_struct_exercices )
	    g_slist_free ( liste_struct_exercices );

	  liste_struct_exercices = liste_struct_exercices_tmp;
	  no_derniere_exercice = no_derniere_exercice_tmp;
	  nb_exercices = nb_exercices_tmp;

	  /* on remplace l'option menu des exercices des formulaire */

	  gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_operations[11] ) -> menu );
	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[11] ),
				     creation_menu_exercices (0) );

	  gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_ventilation[5] ) -> menu );
	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[5] ),
				     creation_menu_exercices (0) );

	  gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_echeancier[9] ) -> menu );
	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[9] ),
				     creation_menu_exercices (1) );

	  /* on recrée la liste tmp des exercices */

	  liste_struct_exercices_tmp = NULL;
	  liste_tmp = liste_struct_exercices;

	  while ( liste_tmp )
	    {
	      struct struct_exercice *exercice;
	      struct struct_exercice *copie_exercice;

	      exercice = liste_tmp -> data;
	      copie_exercice = malloc ( sizeof ( struct struct_exercice ));

	      copie_exercice -> no_exercice = exercice -> no_exercice;
	      copie_exercice -> nom_exercice = g_strdup ( exercice -> nom_exercice );

	      if ( exercice -> date_debut )
		copie_exercice -> date_debut = g_date_new_dmy ( g_date_day ( exercice -> date_debut ),
								g_date_month ( exercice -> date_debut ),
								g_date_year ( exercice -> date_debut ));
	      else
		copie_exercice -> date_debut = NULL;

	      if ( exercice -> date_fin )
		copie_exercice -> date_fin = g_date_new_dmy ( g_date_day ( exercice -> date_fin ),
							      g_date_month ( exercice -> date_fin ),
							      g_date_year ( exercice -> date_fin ));
	      else
		copie_exercice -> date_fin = NULL;

	      copie_exercice -> affiche_dans_formulaire = exercice -> affiche_dans_formulaire;

	      liste_struct_exercices_tmp = g_slist_append ( liste_struct_exercices_tmp,
							    copie_exercice );
	      liste_tmp = liste_tmp -> next;
	    }

	  no_derniere_exercice_tmp = no_derniere_exercice;
	  nb_exercices_tmp = nb_exercices;


	  /* remplissage de la liste avec les exercices temporaires */

	  liste_tmp = liste_struct_exercices_tmp;

	  while ( liste_tmp )
	    {
	      /* on associe à la ligne la struct de la exercice */

	      gtk_clist_set_row_data ( GTK_CLIST ( clist_exercices_parametres ),
				       g_slist_position ( liste_struct_exercices_tmp,
							  liste_tmp ),
				       liste_tmp -> data );

	      liste_tmp = liste_tmp -> next;
	    }


	  /* on remplit à nouveau la liste des exos de la conf de l'état */

	  remplissage_liste_exo_etats ();

	  /* on resélectionne dans cette liste ce qui était sélectionné */

	  selectionne_liste_exo_etat_courant ();


	  modification_fichier ( TRUE );
	}
      break;

      /* onglet types */

    case 8:

      /* récupère le check button afficher tous les types */

      etat.affiche_tous_les_types = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_choix_affichage_formulaire ));

      /* on remplace les anciennes valeurs par les valeurs temporaires */

      for ( i=0 ; i<nb_comptes ; i++ )
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	  g_slist_free ( TYPES_OPES );
	  TYPES_OPES = liste_tmp_types[i];
	  TYPE_DEFAUT_DEBIT = type_defaut_debit[i];
	  TYPE_DEFAUT_CREDIT = type_defaut_credit[i];

	  TRI = tri_tmp[i];
	  g_slist_free ( LISTE_TRI );
	  LISTE_TRI = liste_tri_tmp[i];
	  NEUTRES_INCLUS = neutres_inclus_tmp[i];
	}

      /* on recrée les valeurs temporaires */

      liste_tmp_types = malloc ( nb_comptes * sizeof (gpointer));
      type_defaut_debit = malloc ( nb_comptes * sizeof (gint));
      type_defaut_credit = malloc ( nb_comptes * sizeof (gint));
      liste_tri_tmp = malloc ( nb_comptes * sizeof (gpointer));
      tri_tmp = malloc ( nb_comptes * sizeof (gint));
      neutres_inclus_tmp = malloc ( nb_comptes * sizeof (gint));

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

      for ( i=0 ; i<nb_comptes ; i++ )
	{
	  GSList *liste_tmp;
	  GSList *liste_types_tmp;

	  liste_tmp = TYPES_OPES;
	  liste_types_tmp = NULL;

	  while ( liste_tmp )
	    {
	      struct struct_type_ope *type_ope;
	      struct struct_type_ope *type_ope_tmp;

	      type_ope = liste_tmp -> data;
	      type_ope_tmp = malloc ( sizeof ( struct struct_type_ope ));

	      type_ope_tmp -> no_type = type_ope -> no_type;
	      type_ope_tmp -> nom_type = g_strdup ( type_ope -> nom_type );
	      type_ope_tmp -> signe_type = type_ope -> signe_type;
	      type_ope_tmp -> affiche_entree = type_ope -> affiche_entree;
	      type_ope_tmp -> numerotation_auto = type_ope -> numerotation_auto;
	      type_ope_tmp -> no_en_cours = type_ope -> no_en_cours;
	      type_ope_tmp -> no_compte = type_ope -> no_compte;

	      liste_types_tmp = g_slist_append ( liste_types_tmp,
						 type_ope_tmp );

	      liste_tmp = liste_tmp -> next;
	    }

	  liste_tmp_types[i] = liste_types_tmp;

	  type_defaut_debit[i] = TYPE_DEFAUT_DEBIT;
	  type_defaut_credit[i] = TYPE_DEFAUT_CREDIT;

	  /* on s'occupe des tris */

	  tri_tmp[i] = TRI;
	  neutres_inclus_tmp[i] = NEUTRES_INCLUS;

	  liste_tmp = LISTE_TRI;
	  liste_types_tmp = NULL;

	  while ( liste_tmp )
	    {
	      liste_types_tmp = g_slist_append ( liste_types_tmp,
						 liste_tmp -> data );

	      liste_tmp = liste_tmp -> next;
	    }

	  liste_tri_tmp[i] = liste_types_tmp;

	  p_tab_nom_de_compte_variable++;
	}

      /* on réaffiche si nécessaire l'option menu du formulaire */

      if ( (menu = creation_menu_types ( 1, compte_courant , 0 )))
	{
	  gint pos_type;

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
				     menu );

	  pos_type = cherche_no_menu_type ( TYPE_DEFAUT_DEBIT );

	  if ( pos_type != -1 )
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					  pos_type );
	  else
	    {
	      struct struct_type_ope *type;

	      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					    0 );
	      TYPE_DEFAUT_DEBIT = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
									  "no_type" ));

	      /* on affiche l'entrée des chèques si nécessaire */

	      type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
					   "adr_type" );

	      if ( type -> affiche_entree )
		gtk_widget_show ( widget_formulaire_operations[10] );
	    }

	  gtk_widget_show ( widget_formulaire_operations[9] );
	}
      else
	{
	  gtk_widget_hide ( widget_formulaire_operations[9] );
	  gtk_widget_hide ( widget_formulaire_operations[10] );
	}


      demande_mise_a_jour_tous_comptes ();
      verification_mise_a_jour_liste ();
      modification_fichier ( TRUE );

      break;


      /* onglet Affichage_liste */

    case 9:

      /* récupère l'automatisme de la taille des colonnes */

      etat.largeur_auto_colonnes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_choix_perso_colonnes ));
      etat.retient_affichage_par_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_caracteristiques_lignes_par_compte ));

      if ( nb_comptes )
	{
	  /* on transfère juste tab_affichage_ope_tmp sur tab_affichage_ope */

	  for ( i = 0 ; i<4 ; i++ )
	    for ( j = 0 ; j<7 ; j++ )
	      tab_affichage_ope[i][j] = tab_affichage_ope_tmp[i][j];

	  /* si on decide d'avoir le même affichage pour tous les comptes, on harmonise ici avec le compte courant */

	  if ( !etat.retient_affichage_par_compte )
	    {
	      gint affichage_r_courant;
	      gint nb_lignes_ope_courant;

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	      affichage_r_courant = AFFICHAGE_R;
	      nb_lignes_ope_courant = NB_LIGNES_OPE;

	      for ( i=0 ; i<nb_comptes ; i++ )
		{
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

		  AFFICHAGE_R = affichage_r_courant;
		  NB_LIGNES_OPE = nb_lignes_ope_courant;
		}
	    }

	  /* on change le resizable des colonnes */

	  for ( i=0 ; i<nb_comptes ; i++ )
	    {
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	      for ( j=0 ; j<7 ; j++ )
		gtk_clist_set_column_resizeable ( GTK_CLIST ( CLIST_OPERATIONS ),
						  j,
						  !etat.largeur_auto_colonnes );
	      gtk_signal_connect ( GTK_OBJECT (CLIST_OPERATIONS),
				   "resize_column",
				   GTK_SIGNAL_FUNC (changement_taille_colonne),
				   NULL );
	    }

	  /* récupère les rapports de colonnes entre eux */

	  for ( i=0 ; i<7 ; i++ )
	    rapport_largeur_colonnes[i] = GTK_CLIST ( clist_affichage_liste ) -> column[i].width * 100 / clist_affichage_liste -> allocation.width;



	  /* récupère les largeurs de colonnes sur la liste des opés */

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	  for ( i=0 ; i<7 ; i++ )
	    taille_largeur_colonnes[i] = GTK_CLIST ( CLIST_OPERATIONS ) -> column[i].width;

	  changement_taille_liste_ope ( CLIST_OPERATIONS,
					NULL,
					GINT_TO_POINTER (compte_courant) );
      
	  /* récupère les nouveaux titres de colonne et tips */
      
	  recuperation_noms_colonnes_et_tips();

	  /* on met les nouveaux noms de colonnes et tips aux listes des opérations */

	  tooltip = gtk_tooltips_new ();

	  for ( i=0 ; i<nb_comptes ; i++ )
	    {
	      gint j;

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	      for ( j=0 ; j<7 ; j++ )
		{
		  gtk_clist_set_column_title ( GTK_CLIST ( CLIST_OPERATIONS ),
					       j,
					       titres_colonnes_liste_operations[j] );
	      
		  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltip ),
					 GTK_CLIST (CLIST_OPERATIONS)->column[j].button,
					 tips_col_liste_operations[j],
					 tips_col_liste_operations[j] );
		}
	    }

	  /* récupère les lignes à afficher en fonction de l'affichage */

	  ligne_affichage_une_ligne = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_une_ligne ) -> menu_item ),
									      "no_ligne" ));

	  lignes_affichage_deux_lignes = NULL;
	  lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							  gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_1 ) -> menu_item ),
										"no_ligne" ));
	  lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							  gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_2 ) -> menu_item ),
										"no_ligne" ));

	  lignes_affichage_trois_lignes = NULL;
	  lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							  gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_1 ) -> menu_item ),
										"no_ligne" ));
	  lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							  gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_2 ) -> menu_item ),
										"no_ligne" ));
	  lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							  gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_3 ) -> menu_item ),
										"no_ligne" ));


	  demande_mise_a_jour_tous_comptes ();
	  verification_mise_a_jour_liste ();
	  modification_fichier ( TRUE );
	}
 
      break;

    }

  sauve_configuration ();

}
/* **************************************************************************************************************************** */


void changement_preferences ( GtkWidget *fenetre_preferences,
			      gint page,
			      gpointer data )
{
  int i;

  for (i=0; i <= 9; i++)
    {
      real_changement_preferences ( fenetre_preferences, i, data );
    }
}

/* **************************************************************************************************************************** */
/* fonction qui ferme la fenêtre préférences */
/* **************************************************************************************************************************** */
void fermeture_preferences ( GtkWidget *button,
			     gint page,
			     gpointer data )
{
  GtkTreeIter iter;
  
  gtk_tree_model_get_iter_first(preference_tree_model, &iter);
  
  do
    {
      GValue value = {0, };
      gtk_tree_model_get_value (preference_tree_model, &iter, 1, &value);
      gtk_widget_destroy(g_value_get_pointer(&value));
    }
  while (gtk_tree_model_iter_next (preference_tree_model, &iter));

  preference_selected = NULL;
  gtk_widget_destroy (fenetre_preferences);
}
/* **************************************************************************************************************************** */




/* **************************************************************************************************************************** */
/* fonction qui vérifie la présence de l'applet et renvoie 1 ou 0 */
/* **************************************************************************************************************************** */

gint verifie_affichage_applet ( void )
{
  return ( ( pid_applet = gnome_config_get_int ( "/Grisbi_applet/PID/PID" )) != 0 );
}
/* **************************************************************************************************************************** */



/**
 * Create a box with a nice bold title and content slightly indented.
 * All content is packed vertically in a GtkVBox.  The paddingbox is
 * also packed in its parent.
 *
 * \param parent Parent widget to pack paddingbox in
 * \param fill Give all available space to padding box or not
 * \param title Title to display on top of the paddingbox
 */
GtkWidget *
new_paddingbox_with_title (GtkWidget * parent, gboolean fill, gchar * title)
{
  GtkWidget * hbox, *paddingbox, *label;

  label = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1 );
  gtk_label_set_markup ( GTK_LABEL ( label ), 
			 g_strconcat ("<span weight=\"bold\">",
				      title,
				      "</span>",
				      NULL ) );
  if ( parent )
    {
      gtk_box_pack_start ( GTK_BOX ( parent ), label,
			   FALSE, FALSE, 0);
    }

  hbox = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( parent ), hbox,
		       fill, fill, 0);

  /* Some padding.  ugly but the HiG advises it this way ;-) */
  label = gtk_label_new ( "    " );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0 );

  /* Then make the vbox itself */
  paddingbox = gtk_vbox_new ( FALSE, 6 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), paddingbox,
		       TRUE, TRUE, 0);

  /* Put a label at the end to feed a new line */
  label = gtk_label_new ( "    " );
  gtk_box_pack_end ( GTK_BOX ( paddingbox ), label,
		     FALSE, FALSE, 0 );

  gtk_widget_show_all ( hbox );

  return paddingbox;
}


/**
 * Function that makes a nice title with an optional icon.  It is
 * mainly used to automate preference tabs with titles.
 * 
 * \param title Title that will be displayed in window
 * \param filename (relative or absolute) to an image in a file format
 * recognized by gtk_image_new_from_file().  Use NULL if you don't
 * want an image to be displayed
 * 
 * \returns A pointer to a vbox widget that will contain all created
 * widgets and user defined widgets
 */
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
					  gchar * image_filename)
{
  GtkWidget *vbox_pref, *separator, *hbox, *label, *image;

  vbox_pref = gtk_vbox_new ( FALSE, 5 );
  gtk_widget_show ( vbox_pref );

  /* Title hbox */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_pref ), hbox,
		       FALSE, FALSE, 0);
  gtk_widget_show ( hbox );

  /* Icon */
  if ( image_filename )
    {
      image = gtk_image_new_from_file (g_strconcat(PIXMAPS_DIR,
						   "/",
						   image_filename,
						   NULL));
      gtk_box_pack_start ( GTK_BOX ( hbox ), image,
			   FALSE, FALSE, 0);
      gtk_widget_show ( image );
    }

  /* Nice huge title */
  label = gtk_label_new ( title );
  gtk_label_set_markup ( GTK_LABEL(label), 
			 g_strconcat ("<span size=\"x-large\" weight=\"bold\">",
				      g_markup_escape_text (title,
							    strlen(title)),
				      "</span>",
				      NULL ) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  /* Separator */
  separator = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
		       separator,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( separator );

  return vbox_pref;
}


/**
 * Create a GtkCheckButton with a callback associated.  Initial value
 * of this checkbox is set to the value of *data.  This checkbox calls
 * set_boolean upon toggle, which in turn modifies *data.  If a hook
 * is possibly executed as well.
 *
 * \param label The label for this checkbutton
 * \param data A pointer to a boolean integer
 * \param hook A GCallBack to execute if not null
 * \return A newly allocated GtkVBox
 */
GtkWidget *
new_checkbox_with_title ( gchar * label, guint * data, GtkSignalFunc * hook)
{
  GtkWidget * checkbox;

  checkbox = gtk_check_button_new_with_label ( label );
  checkbox_set_value ( checkbox, data, TRUE );

  gtk_object_set_data ( GTK_OBJECT ( checkbox ), "set-boolean", 
			g_signal_connect ( GTK_OBJECT (checkbox), "toggled",
					   GTK_SIGNAL_FUNC (set_boolean), data ));
 
  if (hook)
    {
      gtk_object_set_data ( GTK_OBJECT ( checkbox ), "hook", 
			    g_signal_connect ( GTK_OBJECT (checkbox), "toggled",
					       GTK_SIGNAL_FUNC (hook), data ));
    }

  return checkbox;
}


/**
 * Update the widget's property accordingly.  If update is set, update
 * widget as well.
 * 
 * \param checkbox The checkbox to update
 * \param data A pointer to a boolean which contains the new value to
 * fill in checkbox's properties.  This boolean will be modified by
 * checkbox's handlers as well.
 * \param uptdate Whether to update checkbox's appearance as well.
 */
void checkbox_set_value ( GtkWidget * checkbox, guint * data, gboolean update )
{
  if (data)
    {
      gulong hook_uid = gtk_object_get_data ( GTK_OBJECT(checkbox), "hook" );

      if ( hook_uid >= 0 )
	g_signal_handler_block ( checkbox, hook_uid);
      g_signal_handler_block ( checkbox,
			       gtk_object_get_data ( GTK_OBJECT(checkbox), 
						     "set-boolean" ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( checkbox ),
				     *data );      
      if ( hook_uid >= 0 )
	g_signal_handler_unblock ( checkbox, hook_uid );
      g_signal_handler_unblock ( checkbox,
				 gtk_object_get_data ( GTK_OBJECT(checkbox), 
						       "set-boolean" ));
    }

  if (update)
    gtk_object_set_data ( GTK_OBJECT ( checkbox ), "pointer", data);

  return checkbox;
}


/**
 * Set a boolean integer to the value of a checkbox.  Normally called
 * via a GTK "toggled" signal handler.
 * 
 * \param checkbox a pointer to a checkbox widget.
 * \param data a pointer to an integer that is to be modified.
 */
gboolean
set_boolean ( GtkWidget * checkbox, guint * dummy)
{
  gboolean *data;

  data = gtk_object_get_data ( GTK_OBJECT ( checkbox ), "pointer");
  if (data)
    *data = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox));

  return FALSE;
}


/**
 * Set a string to the value of an GtkEntry.
 *
 * \param entry The reference GtkEntry
 * \param value Handler parameter.  Not used.
 * \param length Handler parameter.  Not used.
 * \param position Handler parameter.  Not used.
 */
gboolean set_text (GtkEntry *entry, gchar *value, 
		   gint length, gint * position)
{
  gchar ** data;

  data = gtk_object_get_data ( GTK_OBJECT ( entry ), "pointer");
  if (data)
    *data = g_strdup ((gchar*) gtk_entry_get_text ( GTK_ENTRY (entry) ));

  return FALSE;
}



/**
 * Creates a new GtkEntry with a pointer to a string that will be
 * modified according to the entry's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * entry's contents are modified.
 */
GtkWidget * new_text_entry ( gchar ** value, GCallback * hook )
{
  GtkWidget * entry;

  entry = gtk_entry_new ();

  if (value && *value)
    gtk_entry_set_text ( GTK_ENTRY(entry), *value );

  gtk_object_set_data ( GTK_OBJECT ( entry ), "pointer", value);

  gtk_object_set_data ( GTK_OBJECT ( entry ), "insert-text", 
			g_signal_connect_after (GTK_OBJECT(entry), "insert-text",
						G_CALLBACK(set_text), NULL));
  gtk_object_set_data ( GTK_OBJECT ( entry ), "delete-text", 
			g_signal_connect_after (GTK_OBJECT(entry), "delete-text",
						G_CALLBACK(set_text), NULL));
  if ( hook )
    {
      gtk_object_set_data ( GTK_OBJECT ( entry ), "insert-hook", 
			    g_signal_connect_after (GTK_OBJECT(entry), 
						    "insert-text",
						    G_CALLBACK(hook), NULL));
      gtk_object_set_data ( GTK_OBJECT ( entry ), "delete-hook", 
			    g_signal_connect_after (GTK_OBJECT(entry), 
						    "delete-text",
						    G_CALLBACK(hook), NULL));
    }

  return entry;
}


/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * change the content of an integer passed as an argument.
 *
 * \param parent A widget to pack all created radio buttons in
 * \param title The title for this group
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param data A pointer to an integer that will be set to 0 or 1
 *        according to buttons toggles.
 * \param hook An optional hook to run at each toggle
 *
 * \return A newly created paddingbox
 */
GtkWidget *
new_radiogroup_with_title (GtkWidget * parent,
			   gchar * title, gchar * choice1, gchar * choice2,
			   guint * data, GCallback * hook)
{
  GtkWidget * button1, *button2, *paddingbox;

  paddingbox = new_paddingbox_with_title (parent, FALSE, COLON(title));
  
  button1 = gtk_radio_button_new_with_label ( NULL, choice1 );
  gtk_box_pack_start (GTK_BOX(paddingbox), button1, FALSE, FALSE, 0 );
  button2 = gtk_radio_button_new_with_label ( gtk_radio_button_group (button1), 
					      choice2 );
  gtk_box_pack_start (GTK_BOX(paddingbox), button2, FALSE, FALSE, 0 );

  if (data)
    {
      if (*data)
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button2 ), TRUE );
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button1 ), TRUE );
    }

  gtk_object_set_data ( GTK_OBJECT ( button2 ), "pointer", data);
  g_signal_connect ( GTK_OBJECT ( button2 ), "toggled",
		     set_boolean, NULL );

  if (hook)
    {
      g_signal_connect ( GTK_OBJECT ( button2 ), "toggled",
			 GTK_SIGNAL_FUNC ( hook ), data );
    }

  return paddingbox;
}



/**
 * Sets a GDate according to a date widget
 *
 * \param entry A GtkEntry that triggered this handler
 * \param value Handler parameter.  Not used.
 * \param length Handler parameter.  Not used.
 * \param position Handler parameter.  Not used.
 */
gboolean
set_date (GtkEntry *entry, gchar *value, gint length, gint * position)
{
  GDate ** data, temp_date;

  data = gtk_object_get_data ( GTK_OBJECT ( entry ), "pointer");

  g_date_set_parse ( &temp_date, gtk_entry_get_text (GTK_ENTRY(entry)) );
  if ( g_date_valid (&temp_date) && data)
    {
      if (!*data)
	*data = g_date_new ();
      g_date_set_parse ( *data, gtk_entry_get_text (GTK_ENTRY(entry)) );
    }

  return FALSE;
}


/**
 * Creates a new GtkHBox with an entry to type in a date.
 *
 * \param value A pointer to a GDate that will be modified at every
 * change.
 * \param An optional hook to run.
 */
GtkWidget * new_date_entry ( gchar ** value, GCallback * hook )
{
  GtkWidget *hbox, *entry, *date_entry;

  hbox = gtk_hbox_new ( FALSE, 6 );
  
  entry = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX(hbox), entry,
		       TRUE, TRUE, 0 );
  
  gtk_object_set_data ( GTK_OBJECT ( entry ), "pointer", value);

  if ( hook )
    {
      gtk_object_set_data ( GTK_OBJECT ( entry ), "insert-hook", 
			    g_signal_connect_after (GTK_OBJECT(entry), 
						    "insert-text",
						    G_CALLBACK(hook), NULL));
      gtk_object_set_data ( GTK_OBJECT ( entry ), "delete-hook", 
			    g_signal_connect_after (GTK_OBJECT(entry), 
						    "delete-text",
						    G_CALLBACK(hook), NULL));
    }
  gtk_object_set_data ( GTK_OBJECT ( entry ), "insert-text", 
			g_signal_connect_after (GTK_OBJECT(entry), "insert-text",
						G_CALLBACK(set_date), NULL));
  gtk_object_set_data ( GTK_OBJECT ( entry ), "delete-text", 
			g_signal_connect_after (GTK_OBJECT(entry), "delete-text",
						G_CALLBACK(set_date), NULL));

  date_entry = gtk_button_new_with_label ("...");
  gtk_box_pack_start ( GTK_BOX(hbox), date_entry,
		       FALSE, FALSE, 0 );
  gtk_object_set_data ( GTK_OBJECT ( date_entry ),
			"entry", entry);

  g_signal_connect ( GTK_OBJECT ( date_entry ), "clicked",
		     G_CALLBACK ( popup_calendar ), NULL );

  return hbox;
}



/**
 * Change the date that is handler by a date entry.
 *
 * \param hbox The date entry widget.
 * \param value The new date to modify.
 * \param Update GtkEntry value as well.
 */
void date_set_value ( GtkWidget * hbox, GDate ** value, gboolean update )
{
  gtk_object_set_data ( GTK_OBJECT ( get_entry_from_date_entry(hbox) ),
			"pointer", value );

  if ( update )
    {
      g_signal_handler_block ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "insert-hook"));
      g_signal_handler_block ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "insert-text"));
      g_signal_handler_block ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "delete-hook"));
      g_signal_handler_block ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "delete-text"));

      if ( value && *value )
	{
	  gtk_entry_set_text ( get_entry_from_date_entry(hbox),
			       g_strdup_printf ( "%02d/%02d/%04d",
						 g_date_day (*value),
						 g_date_month (*value),
						 g_date_year (*value)));
	}

      g_signal_handler_unblock ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "insert-hook"));
      g_signal_handler_unblock ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "insert-text"));
      g_signal_handler_unblock ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "delete-hook"));
      g_signal_handler_unblock ( get_entry_from_date_entry(hbox),
			       gtk_object_get_data (nom_exercice, "delete-text"));
    }
}



/**
 * Pop up a window with a calendar that allows a date selection.  This
 * calendar runs "date_selectionnee" as a callback if a date is
 * selected.
 *
 * \param button Normally a GtkButton that triggered the handler.
 * This parameter will be used as a base to set popup's position.
 * This widget must also have a parameter (data) of name "entry"
 * which contains a pointer to a GtkEntry used to set initial value of
 * calendar.
 * \param data Handler parameter.  Not used.
 */
gboolean popup_calendar ( GtkWidget * button, gpointer data )
{
  GtkWidget *popup, *entree, *popup_boxv, *calendrier, *bouton, *frame;
  gint x, y, cal_jour, cal_mois, cal_annee;
  GtkRequisition taille_entree;

  /* Find associated gtkentry */
  entree = gtk_object_get_data ( GTK_OBJECT(button), "entry" );

  /* Find popup position */
  gdk_window_get_origin ( GTK_BUTTON (button) -> event_window, &x, &y );
  gtk_widget_size_request ( GTK_WIDGET (button), &taille_entree );
  y = y + taille_entree.height;

  /* Create popup */
  popup = gtk_window_new ( GTK_WINDOW_POPUP );
  gtk_window_set_modal ( GTK_WINDOW (popup), TRUE);
  gtk_widget_set_uposition ( GTK_WIDGET ( popup ), x, y );

  /* Create popup widgets */
  frame = gtk_frame_new ( NULL );
  gtk_container_add ( GTK_CONTAINER (popup), frame);
  popup_boxv = gtk_vbox_new ( FALSE, 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( popup_boxv ), 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ), popup_boxv);

  /* Set initial date according to entry */
  if ( !( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree ))))
	  &&
	  sscanf ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )),
		   "%02d/%02d/%04d", &cal_jour, &cal_mois, &cal_annee)))
    sscanf ( date_jour(), "%02d/%02d/%04d", &cal_jour, &cal_mois, &cal_annee);
      
  /* Creates calendar */
  calendrier = gtk_calendar_new();
  gtk_calendar_select_month ( GTK_CALENDAR ( calendrier ), cal_mois-1, cal_annee);
  gtk_calendar_select_day  ( GTK_CALENDAR ( calendrier ), cal_jour);
  gtk_calendar_display_options ( GTK_CALENDAR ( calendrier ),
				 GTK_CALENDAR_SHOW_HEADING |
				 GTK_CALENDAR_SHOW_DAY_NAMES |
				 GTK_CALENDAR_WEEK_START_MONDAY );

  /* Create handlers */
  gtk_signal_connect ( GTK_OBJECT ( calendrier), "day-selected-double-click",
		       GTK_SIGNAL_FUNC ( date_selectionnee ), entree );
  gtk_signal_connect_object ( GTK_OBJECT ( calendrier), "day-selected-double-click",
			      GTK_SIGNAL_FUNC ( close_calendar_popup ), popup );
  gtk_signal_connect ( GTK_OBJECT ( popup ), "key-press-event",
		       GTK_SIGNAL_FUNC ( touche_calendrier ), NULL );
  gtk_box_pack_start ( GTK_BOX ( popup_boxv ), calendrier,
		       TRUE, TRUE, 0 );

  /* Add the "cancel" button */
  bouton = gtk_button_new_with_label ( _("Cancel") );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton ), "clicked",
			      GTK_SIGNAL_FUNC ( close_calendar_popup ),
			      GTK_WIDGET ( popup ) );
  gtk_box_pack_start ( GTK_BOX ( popup_boxv ), bouton,
		       TRUE, TRUE, 0 );

  /* Show everything */
  gtk_widget_show_all ( popup );

  /* Grab pointer */
  gdk_pointer_grab ( popup -> window, TRUE,
		     GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		     GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
		     GDK_POINTER_MOTION_MASK,
		     NULL, NULL, GDK_CURRENT_TIME );
  
  return FALSE;
}


/**
 * Closes the popup specified as an argument.  As a quick but
 * disgusting hack, we also grab focus on "fenetre_preferences", the
 * dialog that contains all tabs, in order to preserve it.
 *
 * \param popup The popup to close.
 */
void close_calendar_popup ( GtkWidget *popup )
{
  gtk_widget_destroy ( popup );
  gtk_grab_remove ( fenetre_preferences );
  gtk_grab_add ( fenetre_preferences );
}


/**
 * Convenience function that returns the first widget child of a
 * GtkBox.  This is specially usefull for date "widgets" that contains
 * one GtkEntry and one GtkButton, both packed in a single GtkHBox.
 *
 * \param hbox A GtkBox that contains at least one child.
 */
GtkWidget * get_entry_from_date_entry (GtkWidget * hbox)
{
  return ((GtkBoxChild *) GTK_BOX(hbox)->children->data)->widget;
}


/** 
 * TODO: document
 */
void entry_set_value ( GtkWidget * entry, gchar ** value )
{
  /* Block everything */
  if (gtk_object_get_data ( entry, "insert-hook" ))
    g_signal_handler_block ( GTK_OBJECT(entry),
			     gtk_object_get_data ( entry, "insert-hook" ));
  if (gtk_object_get_data ( entry, "insert-text" ))
    g_signal_handler_block ( GTK_OBJECT(entry),
			     gtk_object_get_data ( entry, "insert-text" ));
  if (gtk_object_get_data ( entry, "delete-hook" ))
    g_signal_handler_block ( GTK_OBJECT(entry),
			     gtk_object_get_data ( entry, "delete-hook" ));
  if (gtk_object_get_data ( entry, "delete-text" ))
    g_signal_handler_block ( GTK_OBJECT(entry),
			     gtk_object_get_data ( entry, "delete-text" ));

  /* Fill in value */
  if (value && *value)
    gtk_entry_set_text ( GTK_ENTRY ( entry ), *value );
  else
    gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );

  gtk_object_set_data ( entry, "pointer", value );

  /* Unblock everything */
  if (gtk_object_get_data ( entry, "insert-hook" ))
    g_signal_handler_unblock ( GTK_OBJECT(entry),
			       gtk_object_get_data ( entry, "insert-hook" ));
  if (gtk_object_get_data ( entry, "insert-text" ))
    g_signal_handler_unblock ( GTK_OBJECT(entry),
			       gtk_object_get_data ( entry, "insert-text" ));
  if (gtk_object_get_data ( entry, "delete-hook" ))
    g_signal_handler_unblock ( GTK_OBJECT(entry),
			       gtk_object_get_data ( entry, "delete-hook" ));
  if (gtk_object_get_data ( entry, "delete-text" ))
    g_signal_handler_unblock ( GTK_OBJECT(entry),
			       gtk_object_get_data ( entry, "delete-text" ));
}


/**
 * Creates a new GtkTextView with a pointer to a string that will be
 * modified according to the text view's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * entry's contents are modified.
 */
GtkWidget * new_text_area ( gchar ** value, GCallback * hook )
{
  GtkWidget * text_view;
  GtkTextBuffer *buffer;

  text_view = gtk_text_view_new ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (text_view), 3);
  gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (text_view), 3);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (text_view), 3);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (text_view), 3);
  gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD );

  if (value && *value)
    gtk_text_buffer_set_text (buffer, *value, -1);
 
  g_object_set_data ( G_OBJECT ( buffer ), "pointer", value);

  g_object_set_data ( G_OBJECT ( buffer ), "change-text",
			g_signal_connect (G_OBJECT(buffer),
					  "changed",
					  G_CALLBACK(set_text_from_area),
					  NULL));
  if ( hook )
    g_object_set_data ( G_OBJECT ( buffer ), "change-hook",
			  g_signal_connect (G_OBJECT(buffer),
					    "changed",
					    G_CALLBACK(hook),
					    NULL));
  return text_view;
}


/** 
 * TODO: document
 */
void text_area_set_value ( GtkWidget * text_view, gchar ** value )
{
  GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  /* Block everything */
  if (g_object_get_data ( G_OBJECT(buffer), "change-hook" ))
    g_signal_handler_block ( G_OBJECT(buffer),
			     g_object_get_data ( G_OBJECT(buffer), "change-hook" ));
  if (g_object_get_data ( G_OBJECT(buffer), "change-text" ))
    g_signal_handler_block ( G_OBJECT(buffer),
			     g_object_get_data ( G_OBJECT(buffer), "change-text" ));

  /* Fill in value */
  if (value && *value)
    gtk_text_buffer_set_text (buffer, *value, -1 );
  else
    gtk_text_buffer_set_text (buffer, "", -1 );

  if ( value )
    g_object_set_data ( buffer, "pointer", value );

  /* Unblock everything */
  if (g_object_get_data ( buffer, "change-hook" ))
    g_signal_handler_unblock ( G_OBJECT(buffer),
			       g_object_get_data ( buffer, "change-hook" ));
  if (g_object_get_data ( buffer, "change-text" ))
    g_signal_handler_unblock ( G_OBJECT(buffer),
			       g_object_get_data ( buffer, "change-text" ));
}


/**
 * Set a string to the value of an GtkTextView
 *
 * \param entry The reference GtkEntry
 * \param value Handler parameter.  Not used.
 * \param length Handler parameter.  Not used.
 * \param position Handler parameter.  Not used.
 */
gboolean set_text_from_area ( GtkTextBuffer *buffer, gpointer tamere )
{
  GtkTextIter start, end;
  gchar ** data;

  data = g_object_get_data ( G_OBJECT ( buffer ), "pointer");

  gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
  gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

  if (data)
    *data = g_strdup ( gtk_text_buffer_get_text (buffer, &start, &end, 0) );

  return FALSE;
}
