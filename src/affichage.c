/* Ce fichier s'occupe des différents paramètres d'affichage réglés
   dans les paramètres */
/* affichage.c */

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


GtkWidget * list_font_name_label, * list_font_size_label;
GtkWidget * general_font_name_label, * general_font_size_label;


/**
 * Updates transaction form according to state "etat"
 *
 * \param checkbox Widget that triggers this event.  Normally not
 * needed there.
 * \param data A pointer to some random data passed to this hook.  Not
 * used there.
 */
gboolean
update_transaction_form ( GtkWidget * checkbox,
			  gpointer data )
{
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
    }
}


GtkWidget *
onglet_display_transaction_form ( void )
{
  GtkWidget *hbox, *vbox_pref;
  GtkWidget *label, *paddingbox;
  GtkWidget *table, *bouton, *radiogroup;

  vbox_pref = new_vbox_with_title_and_icon ( _("Transaction form"),
					     "form.png" );

  /* What to do if RETURN is pressed into transaction form */
  radiogroup = new_radiogroup_with_title (vbox_pref,
					  _("Pressing RETURN in transaction form"),
					  _("selects next field"),
					  _("terminates transaction"),
					  &etat.entree, NULL);

  /* Displayed fields */
  paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					  COLON(_("Displayed fields")));
  
  table = gtk_table_new ( 0, 2, TRUE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ),
		       table,
		       FALSE, FALSE, 0 );

  gtk_table_attach ( GTK_TABLE ( table ),
		     new_checkbox_with_title (_("Transaction number"),
					      &etat.affiche_no_operation,
					      G_CALLBACK(update_transaction_form)),
		     0, 1, 0, 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );

  gtk_table_attach ( GTK_TABLE ( table ),
		     new_checkbox_with_title (_("Value date"),
					      &etat.affiche_date_bancaire,
					      G_CALLBACK(update_transaction_form)),
		     0, 1, 1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );

  gtk_table_attach ( GTK_TABLE ( table ),
		     new_checkbox_with_title (_("Financial year"),
					      &etat.utilise_exercice,
					      G_CALLBACK(update_transaction_form)),
		     0, 1, 2, 3,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );

  gtk_table_attach ( GTK_TABLE ( table ),
		     new_checkbox_with_title (_("Budgetary information"),
					      &etat.utilise_imputation_budgetaire,
					      G_CALLBACK(update_transaction_form)),
		     0, 1, 3, 4,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );

  gtk_table_attach ( GTK_TABLE ( table ),
		     new_checkbox_with_title (_("Voucher number"),
					      &etat.utilise_piece_comptable,
					      G_CALLBACK(update_transaction_form)),
		     1, 2, 0, 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );

  gtk_table_attach ( GTK_TABLE ( table ),
		     new_checkbox_with_title (_("Bank reference"),
					      &etat.utilise_info_banque_guichet,
					      G_CALLBACK(update_transaction_form)),
		     1, 2, 1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );

  gtk_table_attach ( GTK_TABLE ( table ),
		     new_checkbox_with_title (_("'Accept' and 'Cancel' buttons"),
					      &etat.affiche_boutons_valider_annuler,
					      G_CALLBACK(update_transaction_form)),
		     1, 2, 2, 3,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );

  /* How to display financial year */
  radiogroup = new_radiogroup_with_title (vbox_pref,
					  _("By default, use financial year"),
					  _("last selected financial year"),
					  _("according to transaction date"),
					  &etat.affichage_exercice_automatique, 
					  NULL);

  return vbox_pref;
}



/**
 * Creates the "Fonts & logo" tab.  This function creates some buttons
 * that are borrowed from applications like gedit.
 *
 * \returns A newly allocated vbox
 */
GtkWidget * onglet_display_fonts ( void )
{
  GtkWidget *hbox, *vbox_pref;
  GtkWidget *label, *paddingbox;
  GtkWidget *table, *font_button, *logo_button;
  GtkWidget *hbox_font, *init_button;

  vbox_pref = new_vbox_with_title_and_icon ( _("Fonts & logo"),
					     "fonts.png" );


  /* Change Grisbi Logo */
  paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					   _("Grisbi logo") );

  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0 );

  logo_button = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( logo_button ),
			  GTK_RELIEF_NONE );
  gtk_container_add ( GTK_CONTAINER ( logo_button ),
		  logo_button = gtk_button_new_from_stock (GTK_STOCK_OK));
		      //gnome_stock_pixmap_widget ( logo_button,
						  //GNOME_STOCK_PIXMAP_FORWARD ));
  gtk_signal_connect ( GTK_OBJECT ( logo_button ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( modification_logo_accueil ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       logo_button,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( logo_button );

  label = gtk_label_new ( g_strconcat (" : ", 
				       _("Change homepage logo"), 
				       NULL ));
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( label );



  /* Change fonts */
  paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					   _("Fonts") );

  /* Create table */
  table = gtk_table_new ( 2, 2, FALSE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
  gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
		       FALSE, FALSE, 0 );

  /* Change general font */
  label = gtk_label_new (COLON(_("General font")));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 0, 1,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );

  /* Create font button */
  font_button = gtk_button_new ();
  hbox_font = gtk_hbox_new ( FALSE, 0 );
  general_font_name_label = gtk_label_new (general_font_name);
  gtk_widget_modify_font (general_font_name_label,
			  pango_font_description_from_string (fonte_general));
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       general_font_name_label,
		       TRUE, TRUE, 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       gtk_vseparator_new (),
		       FALSE, FALSE, 0 );
  general_font_size_label = gtk_label_new (general_font_size);
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       general_font_size_label,
		       FALSE, FALSE, 5 );
  gtk_widget_show_all ( hbox_font );
  gtk_container_add (GTK_CONTAINER ( font_button ), hbox_font);
  gtk_signal_connect ( GTK_OBJECT ( font_button ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( choix_fonte_general ),
		       NULL );
  gtk_table_attach ( GTK_TABLE ( table ),
		     font_button, 1, 2, 0, 1,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );
  update_font_button(general_font_name_label,
		     general_font_size_label,
		     fonte_general);

  /* Change list font */
  label = gtk_label_new (COLON(_("Transaction list font")));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 1, 2,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );

  /* Create font button */
  font_button = gtk_button_new ();
  hbox_font = gtk_hbox_new ( FALSE, 0 );
  list_font_name_label = gtk_label_new (list_font_name);
  gtk_widget_modify_font (list_font_name_label,
			  pango_font_description_from_string (fonte_liste));
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       list_font_name_label,
		       TRUE, TRUE, 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       gtk_vseparator_new (),
		       FALSE, FALSE, 0 );
  list_font_size_label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       list_font_size_label,
		       FALSE, FALSE, 5 );
  gtk_widget_show_all ( hbox_font );
  gtk_container_add (GTK_CONTAINER(font_button), hbox_font);
  gtk_signal_connect ( GTK_OBJECT ( font_button ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( choix_fonte ),
		       NULL );
  gtk_table_attach ( GTK_TABLE ( table ),
		     font_button, 1, 2, 1, 2,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );
  update_font_button(list_font_name_label,
		     list_font_size_label,
		     fonte_liste);

  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_end ( GTK_BOX ( vbox_pref ), hbox,
		     FALSE, FALSE, 0 );
  init_button = gtk_button_new_with_label ( SPACIFY(_("Restore defaults fonts")) );
  gtk_box_pack_end ( GTK_BOX ( hbox ), init_button,
		     FALSE, FALSE, 0 );
  g_signal_connect (init_button, "clicked", 
		    G_CALLBACK (init_fonts), NULL);

  return vbox_pref;
}


/**
 * Creates the "Titles & Addresses" tab.  This tab is mainly composed
 * of text entries and editables.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_display_addresses ( void )
{
  GtkWidget *hbox, *vbox_pref, *separateur, *scrolled_window, *label, *frame;
  GtkWidget *vbox2, *fleche, *hbox2, *bouton, *onglet, *paddingbox;
  GSList *liste_tmp;

  vbox_pref = new_vbox_with_title_and_icon ( _("Addresses & titles"),
					     "addresses.png" );

  /* Account file title */
  paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					   _("Titles") );
  hbox = gtk_hbox_new ( FALSE, 6 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);

  label = gtk_label_new ( COLON(_("Accounts file title")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0);

  entree_titre_fichier = new_text_entry (&titre_fichier,
					 G_CALLBACK(update_homepage_title));
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_titre_fichier,
		       TRUE, TRUE, 0);

  /* Addresses */
  paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					   _("Addresses") );

  /* Common address */
  label = gtk_label_new ( COLON(_("Common address")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), label,
		       TRUE, TRUE, 0);
  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
		       FALSE, FALSE, 0);
  entree_adresse_commune = gtk_text_new ( FALSE, FALSE );
  gtk_text_set_editable ( GTK_TEXT ( entree_adresse_commune ), TRUE );
  if ( adresse_commune )
    {
      gtk_text_insert ( GTK_TEXT (entree_adresse_commune  ),
			NULL,
			NULL,
			NULL,
			adresse_commune,
			-1 );
    }
  gtk_signal_connect_object ( GTK_OBJECT ( entree_adresse_commune ),
			      "changed",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      entree_adresse_commune );

  /* Secondary address */
  /** \note This is not implemented yet */
  label = gtk_label_new ( COLON(_("Secondary address")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), label,
		       TRUE, TRUE, 0);
  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
		       FALSE, FALSE, 0);
  entree_adresse_secondaire = gtk_text_new ( FALSE, FALSE );
  gtk_text_set_editable ( GTK_TEXT ( entree_adresse_secondaire ), TRUE );
  if ( adresse_secondaire )
    gtk_text_insert ( GTK_TEXT ( entree_adresse_secondaire ),
		      NULL,
		      NULL,
		      NULL,
		      adresse_secondaire,
		      -1 );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_adresse_secondaire ),
			      "changed",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      entree_adresse_secondaire );

  if ( !nb_comptes )
    {
      gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

  return ( vbox_pref );
}



GtkWidget *onglet_affichage ( void )
{
  GtkWidget *hbox, *vbox;
  GtkWidget *separateur;
  GtkWidget *label;
  GtkWidget *frame;
  GSList *liste_tmp;
  GtkWidget *vbox2;
  GtkWidget *fleche;
  GtkWidget *hbox2;
  GtkWidget *bouton;
  GtkWidget *onglet;

  onglet = gtk_hbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


  /* fenetre de base verticale */

  vbox = gtk_vbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );


  /*   ajout du choix de la fonte pour les listes */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  if ( fonte_liste )
    gnome_font_picker_set_font_name ( GNOME_FONT_PICKER ( bouton ),
				      fonte_liste );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  label = gtk_label_new ( SPACIFY(_(": Change lists font /")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  /* on remet un font_picker sans nom de fonte pour le raz */

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );

  gtk_container_remove ( GTK_CONTAINER ( bouton ),
			 GTK_BIN ( bouton ) -> child );
  label = gtk_label_new ( _("Init") );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      label );
  gtk_widget_show ( label );
  gtk_widget_show ( bouton );


  /*   ajout du choix de la fonte générale */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  if ( fonte_general )
    gnome_font_picker_set_font_name ( GNOME_FONT_PICKER ( bouton ),
				      fonte_general );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte_general ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  label = gtk_label_new ( SPACIFY(_(": Change general font /")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  /* on remet un font_picker sans nom de fonte pour le raz */

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte_general ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );

  gtk_container_remove ( GTK_CONTAINER ( bouton ),
			 GTK_BIN ( bouton ) -> child );
  label = gtk_label_new ( _("Init") );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      label );
  gtk_widget_show ( label );
  gtk_widget_show ( bouton );

  /* ajout de la modification du logo de l'accueil  */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		  bouton = gtk_button_new_from_stock (GTK_STOCK_OK));
		      //gnome_stock_pixmap_widget ( bouton,
						  //GNOME_STOCK_PIXMAP_FORWARD ));
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( modification_logo_accueil ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  label = gtk_label_new ( g_strconcat (" : ", 
				       _("Change homepage logo"), 
				       NULL ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );




  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );



  /* mise en place de l'ordre d'affichage des comptes */

  hbox2 = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox2,
		       TRUE,
		       TRUE,
		       0);
  
  frame = gtk_frame_new ( SPACIFY(_("Accounts display order")) );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_container_set_border_width ( GTK_CONTAINER ( frame ),
				   10 );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       frame,
		       TRUE,
		       TRUE,
		       0);


  if ( !ordre_comptes )
    {
      label = gtk_label_new ( _("No account defined") );
      gtk_container_add ( GTK_CONTAINER ( frame ),
			  label );
   }
  else
    {
      gpointer ** p_tab_nom_de_compte_variable;                    /* évite d'écraser la variable globale */
      gchar *nom_compte[1];
      GtkWidget *scroll_liste_choix_ordre_comptes;


      hbox = gtk_hbox_new ( FALSE,
				 5 );
      gtk_container_set_border_width ( GTK_CONTAINER ( hbox ),
				       10 );
      gtk_container_add ( GTK_CONTAINER ( frame ),
			  hbox );
      gtk_widget_show ( hbox );


/* mise en place de la list qui contient les noms et l'ordre des comptes */

      scroll_liste_choix_ordre_comptes = gtk_scrolled_window_new ( NULL, NULL );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   scroll_liste_choix_ordre_comptes,
			   FALSE,
			   FALSE,
			   0);
      gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_liste_choix_ordre_comptes ),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
      gtk_widget_set_usize ( GTK_WIDGET ( scroll_liste_choix_ordre_comptes ),
			     150,
			     120 );

      liste_choix_ordre_comptes = gtk_clist_new ( 1 );
      gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_choix_ordre_comptes ) ,
					 0,
					 TRUE );
      gtk_clist_set_reorderable ( GTK_CLIST ( liste_choix_ordre_comptes ),
				  TRUE );
      gtk_clist_set_use_drag_icons ( GTK_CLIST ( liste_choix_ordre_comptes ),
				     TRUE );
      gtk_container_add ( GTK_CONTAINER ( scroll_liste_choix_ordre_comptes ),
			  liste_choix_ordre_comptes );

      liste_tmp = ordre_comptes;

      do
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + (gint) liste_tmp->data;
	  nom_compte[0] = g_strdup ( NOM_DU_COMPTE );
	  gtk_clist_set_row_data ( GTK_CLIST ( liste_choix_ordre_comptes ),
				   gtk_clist_append ( GTK_CLIST ( liste_choix_ordre_comptes) , nom_compte ),
				   liste_tmp->data );
	}
      while ( ( liste_tmp = liste_tmp -> next ));

      gtk_signal_connect_object ( GTK_OBJECT ( liste_choix_ordre_comptes ),
				  "row_move",
				  activer_bouton_appliquer,
				  GTK_OBJECT (fenetre_preferences));
 
      /* on place ici les flèches sur le côté de la liste */

      vbox2 = gtk_vbutton_box_new ();

      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   vbox2,
			   FALSE,
			   FALSE,
			   0);

      fleche = gtk_button_new_from_stock (GTK_STOCK_GO_UP);


//      fleche = gnome_stock_button ( GNOME_STOCK_BUTTON_UP );
      gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			     GTK_RELIEF_NONE );
      gtk_signal_connect ( GTK_OBJECT ( fleche ),
			   "clicked",
			   (GtkSignalFunc ) deplacement_haut,
			   NULL );
      gtk_container_add ( GTK_CONTAINER ( vbox2 ),
			  fleche  );

      fleche = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
      //fleche = gnome_stock_button ( GNOME_STOCK_BUTTON_DOWN );
      gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			     GTK_RELIEF_NONE );
      gtk_signal_connect ( GTK_OBJECT ( fleche ),
			   "clicked",
			   (GtkSignalFunc ) deplacement_bas,
			    NULL);
      gtk_container_add ( GTK_CONTAINER ( vbox2 ),
			  fleche  );

      /* connecte maintenant les sélections */


      gtk_signal_connect_object ( GTK_OBJECT ( liste_choix_ordre_comptes ),
				  "select-row",
				  GTK_SIGNAL_FUNC ( selection_choix_ordre_comptes ),
				  GTK_OBJECT ( vbox2 ));
      gtk_signal_connect_object ( GTK_OBJECT ( liste_choix_ordre_comptes ),
				  "unselect-row",
				  GTK_SIGNAL_FUNC ( deselection_choix_ordre_comptes ),
				  GTK_OBJECT ( vbox2 ));

      /*       selectionne le premier compte */

      gtk_clist_select_row ( GTK_CLIST ( liste_choix_ordre_comptes ),
			     0,
			     0);
    }

  gtk_widget_show_all ( vbox );


  /*   place la partie de droite : comptabilité */

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  vbox = gtk_vbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  frame = gtk_frame_new ( _("Transactions list and form") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );

  bouton_afficher_no_operation = gtk_check_button_new_with_label ( _("Display the number of the edited transaction.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_operation ),
				 etat.affiche_no_operation );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_no_operation ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_no_operation,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_no_operation );

  /* GDC : bouton pour choisir d'afficher ou non la date réelle (colonne et champ) */

  bouton_afficher_date_bancaire = gtk_check_button_new_with_label ( _("Display the value date of the transactions.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_bancaire ),
				 etat.affiche_date_bancaire );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_date_bancaire ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_date_bancaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_date_bancaire );

  /* FinGDC */

  bouton_utiliser_exercices = gtk_check_button_new_with_label ( _("Use financial years.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_exercices ),
				 etat.utilise_exercice );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_exercices ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_exercices,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_exercices );

  bouton_utiliser_imputation_budgetaire = gtk_check_button_new_with_label ( _("Use budgetary informations.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_imputation_budgetaire ),
				 etat.utilise_imputation_budgetaire );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_imputation_budgetaire ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_imputation_budgetaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_imputation_budgetaire );

  bouton_utiliser_piece_comptable = gtk_check_button_new_with_label ( _("Use the voucher number field.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_piece_comptable ),
				 etat.utilise_piece_comptable );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_piece_comptable ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_piece_comptable,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_piece_comptable );

  bouton_utiliser_info_banque_guichet = gtk_check_button_new_with_label ( _("Use the bank references field.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_info_banque_guichet ),
				 etat.utilise_info_banque_guichet );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_info_banque_guichet ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_info_banque_guichet,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_info_banque_guichet );

  bouton_afficher_boutons_valider_annuler = gtk_check_button_new_with_label ( _("Display 'Accept' and 'Cancel' transaction buttons.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_boutons_valider_annuler ),
				 etat.affiche_boutons_valider_annuler );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_boutons_valider_annuler ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_boutons_valider_annuler,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_boutons_valider_annuler );



  /* mise en place de la frame sur l'affichage des listes */

  frame = gtk_frame_new ( _("Third parties, categories and budgetary info lists.") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );

  /*   mise en place du choix de la devise pour les totaux des tiers, catég et ib */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Currency")) );
  gtk_box_pack_start ( GTK_BOX (hbox  ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_choix_devise_totaux_tiers = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ),
			     creation_option_menu_devises ( -1,
							    liste_struct_devises ) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_choix_devise_totaux_tiers,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_devise_totaux_tiers );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( no_devise_totaux_tiers ),
									 ( GCompareFunc ) recherche_devise_par_no )));
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ) -> menu ),
			      "selection-done",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  if ( !nb_comptes )
    gtk_widget_set_sensitive ( hbox,
			       FALSE );


  /* affichage du nb d'écritures */

  bouton_afficher_nb_ecritures = gtk_check_button_new_with_label ( _("Display the number of transactions.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_ecritures ),
				 etat.affiche_nb_ecritures_listes );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_nb_ecritures ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_nb_ecritures,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_nb_ecritures );


  /*   place la partie de droite : classement des listes */

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  vbox = gtk_vbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  frame = gtk_frame_new ( _("Sort the transaction list") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );

  bouton_classer_liste_par_date = gtk_radio_button_new_with_label ( NULL,
								    _("by date") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_classer_liste_par_date,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_classer_liste_par_date );

  bouton_classer_liste_par_date_bancaire = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_classer_liste_par_date)),
									     _("by value date") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_classer_liste_par_date_bancaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_classer_liste_par_date_bancaire );

  if ( etat.classement_par_date )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_classer_liste_par_date ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_classer_liste_par_date_bancaire ),
				   TRUE );

  gtk_signal_connect_object ( GTK_OBJECT ( bouton_classer_liste_par_date ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_classer_liste_par_date_bancaire ),
			      "toggled",
			      activer_bouton_appliquer,
			      GTK_OBJECT (fenetre_preferences));

  return ( onglet );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void selection_choix_ordre_comptes ( GtkWidget *box )
{
  gtk_widget_set_sensitive ( box,
			     TRUE );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deselection_choix_ordre_comptes ( GtkWidget *box )
{
  gtk_widget_set_sensitive ( box,
			     FALSE );
}
/* ************************************************************************************************************** */



/* **************************************************************************************************************************** */
void deplacement_haut ( void )
{
  if ( !GPOINTER_TO_INT (GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ))
    return;

  gtk_clist_swap_rows ( GTK_CLIST ( liste_choix_ordre_comptes ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) - 1 );

}
/* **************************************************************************************************************************** */





/* **************************************************************************************************************************** */
void deplacement_bas ( void )
{
  if ( GPOINTER_TO_INT (GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) == ( nb_comptes - 1 ) )
    return;

  gtk_clist_swap_rows ( GTK_CLIST ( liste_choix_ordre_comptes ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) + 1 );
}
/* **************************************************************************************************************************** */



/**
 * Update two labels according to font name, which is parsed to
 * separate both name and size
 *
 * \param name_label a widget which is to receive the name part of the
 * font name
 * \param size_label a widget which is to receive the size part of the
 * font name
 * \param fontname a font name in the form "name, size" or "name,
 * attr, size"
 */
void update_font_button(GtkWidget * name_label,
			GtkWidget * size_label,
			gchar * fontname)
{
  gchar * font_name, *font_size, *tmp;

   if (!fontname)
    fontname = "Sans 10";

  gtk_widget_modify_font (name_label,
			  pango_font_description_from_string(fontname));

  font_name = g_strdup ( fontname );
  tmp = font_name + strlen(font_name) - 1;
  while (isdigit(*tmp) ||
	 (*tmp) == '.')
    tmp --;
  font_size = tmp+1;
	
  while (*tmp == ' ' ||
	 *tmp == ',')
    {
      *tmp=0;
      tmp--;
    }

  gtk_label_set_text (GTK_LABEL(name_label), font_name);
  gtk_label_set_text (GTK_LABEL(size_label), font_size);
}


/**
 * Reset fonts to their initial value, that is "Sans 10".  Update
 * fonts buttons as well.
 *
 * \param button Button that was clicked
 * \param user_data Not used but required by signal
 */
gboolean init_fonts ( GtkWidget * button,
		      gpointer user_data)
{
  fonte_liste = "Sans 10";
  fonte_general = "Sans 10";
  update_font_button (list_font_name_label, 
		      list_font_size_label, 
		      fonte_liste);
  update_font_button (general_font_name_label, 
		      general_font_size_label, 
		      fonte_general);
}



/* **************************************************************************************************************************** */
void choix_fonte ( GtkWidget *bouton,
		   gchar *fonte,
		   gpointer null )
{
  GdkFont *font;
  gchar * fontname;
  GtkWidget * dialog;
  gint i, size;

  dialog = gtk_font_selection_dialog_new (COLON(_("Transaction list font")));
  if (fonte_liste)
    gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG(dialog), 
					     fonte_liste);
  gtk_window_set_modal ( GTK_WINDOW ( dialog ), 
			 TRUE );

  switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
      case GTK_RESPONSE_OK:
	fontname = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG(dialog));
	gtk_widget_destroy (dialog);  
	update_font_button (list_font_name_label, list_font_size_label, 
			    fontname);
	break;
      default:
	gtk_widget_destroy (dialog);
	return;
    }

  fonte_liste = fontname;
  
  if ( nb_comptes )
    {
      font = gdk_font_load ( fonte_liste );

      /* Use font */
      /* FIXME FONTS ... this does not work !!! */
      gtk_style_set_font (style_couleur [0], font);
      gtk_style_set_font (style_couleur [1], font);
      gtk_style_set_font (style_rouge_couleur [0], font);
      gtk_style_set_font (style_rouge_couleur [1], font);

      /* Find font size */
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  gint size = pango_font_description_get_size
	    (pango_font_description_from_string(fontname));
	  gtk_clist_set_row_height ( GTK_CLIST ( CLIST_OPERATIONS ),
				     (size/PANGO_SCALE) + 2 );
	  gtk_clist_set_row_height ( GTK_CLIST ( liste_echeances ),
				     (size/PANGO_SCALE) + 2 );
	  p_tab_nom_de_compte_variable++;
	}

    }
}
/* **************************************************************************************************************************** */





/* **************************************************************************************************************************** */
void choix_fonte_general ( GtkWidget *bouton,
			   gchar *fonte,
			   gpointer null )
{
  GdkFont *font;
  gchar * fontname;
  GtkWidget * dialog;
  GtkStyle * style_general;
  gint i;

  dialog = gtk_font_selection_dialog_new (COLON(_("General font")));
  if (fonte_general)
    gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG(dialog), 
					     fonte_general);
  gtk_window_set_modal ( GTK_WINDOW ( dialog ), 
			 TRUE );

  switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
    case GTK_RESPONSE_OK:
      fontname = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG(dialog));
      gtk_widget_destroy (dialog);  
      update_font_button (general_font_name_label, general_font_size_label, 
			  fontname);

      gtk_widget_destroy (dialog);
      break;
    default:
      gtk_widget_destroy (dialog);
      return;
    }

  fonte_general = fontname;

  style_general = gtk_widget_get_style ( window );
  gtk_style_set_font (style_general, gdk_font_load ( fonte_general ));
  style_general = gtk_widget_get_default_style ();
  gtk_style_set_font (style_general, gdk_font_load ( fonte_general ));

  if ( nb_comptes )
    {
/*       gtk_notebook_set_page ( GTK_NOTEBOOK ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> notebook ), */
/* 			      0 ); */
/*       gtk_notebook_set_page ( GTK_NOTEBOOK ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> notebook ), */
/* 			      3 ); */

/*       gtk_widget_destroy ( notebook_general ); */
/*       gtk_widget_destroy ( GNOME_APP ( window ) -> menubar ); */
/*       GNOME_APP ( window ) -> menubar = NULL; */
/*       gnome_app_create_menus ( GNOME_APP ( window ), */
/* 			       menu_principal ); */

/*       ouverture_confirmee (); */
    }
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void modification_logo_accueil ( void )
{
  GtkWidget *dialog;
  GtkWidget *choix;
  gint resultat;
  GtkWidget *bouton;

  dialog = gnome_dialog_new ( _("Select a new logo"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( fenetre_preferences ));

  choix = gnome_pixmap_entry_new ( "path_logo_grisbi",
				   _("Select a new logo"),
				   TRUE );
  gtk_widget_set_usize ( choix,
			 300,
			 300 );

  if ( chemin_logo )
    gtk_entry_set_text ( GTK_ENTRY ( gnome_pixmap_entry_gtk_entry ( GNOME_PIXMAP_ENTRY ( choix ))),
			 chemin_logo );
  gnome_pixmap_entry_set_preview ( GNOME_PIXMAP_ENTRY ( choix ),
				   TRUE );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       choix,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( choix );

  bouton = gtk_button_new_with_label ( _("Reset to default logo") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( remise_a_zero_logo ),
		       choix );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       bouton,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( bouton );



  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    {
      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }
  if ( nb_comptes )
    {
      /* on vire l'ancien logo et la séparation */
      
      if (chemin_logo)
	{
	  gtk_widget_destroy ( ((GtkBoxChild *)(GTK_BOX ( page_accueil ) -> children -> data )) -> widget );
	}  

      /* on change le logo */

      chemin_logo = gnome_pixmap_entry_get_filename ( GNOME_PIXMAP_ENTRY ( choix ));

      if ( !(chemin_logo
	     &&
	     strlen ( g_strstrip ( chemin_logo ))) )
	{
	  chemin_logo = NULL;
	  if ( logo_accueil
	       &&
	       GTK_IS_WIDGET ( logo_accueil ))
	    gtk_widget_hide ( logo_accueil );
	}
      else
	{
	  GtkWidget *separateur;

	  logo_accueil =  gnome_pixmap_new_from_file ( chemin_logo );

	  gtk_box_pack_start ( GTK_BOX ( page_accueil ),
			       logo_accueil,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show ( logo_accueil );

	  /* séparation gauche-droite */

	  separateur = gtk_vseparator_new ();
	  gtk_box_pack_start ( GTK_BOX ( page_accueil ),
			       separateur,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show ( separateur );

	  gtk_box_reorder_child ( GTK_BOX ( page_accueil ),
				  logo_accueil,
				  0 );
	  gtk_box_reorder_child ( GTK_BOX ( page_accueil ),
				  separateur,
				  1 );
  
	}
    }

  gnome_dialog_close ( GNOME_DIALOG ( dialog ));

  /* on sauvegarde le chemin */

  gnome_config_set_string ( "/Grisbi/Affichage/Chemin_du_logo",
			    chemin_logo );
  gnome_config_sync();
  modification_fichier ( TRUE );

}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
void remise_a_zero_logo ( GtkWidget *bouton,
			  GtkWidget *pixmap_entry )
{
  chemin_logo = CHEMIN_LOGO;

  gtk_entry_set_text ( GTK_ENTRY ( gnome_pixmap_entry_gtk_entry ( GNOME_PIXMAP_ENTRY ( pixmap_entry ))),
		       chemin_logo );

  gnome_pixmap_entry_set_preview ( GNOME_PIXMAP_ENTRY ( pixmap_entry ),
				   TRUE );
}
/* **************************************************************************************************************************** */



/* FIXME: document it ! */
gboolean
update_homepage_title (GtkEntry *entry, gchar *value, 
		       gint length, gint * position)
{
  gtk_label_set_text ( label_titre_fichier, 
		       gtk_entry_get_text (GTK_ENTRY (entry)) );

  return FALSE;
}
