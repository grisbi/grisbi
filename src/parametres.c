/* permet la configuration du logiciel */

/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
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



/* ************************************************************************************************************** */
void preferences ( GtkWidget *widget,
		   gint page_demandee )
{

  /* création de la fenêtre */

  fenetre_preferences = gnome_property_box_new ();
  gtk_window_set_title ( GTK_WINDOW ( fenetre_preferences ),
			 _("Grisbi setup") );
  gtk_window_set_transient_for ( GTK_WINDOW (fenetre_preferences),
				 GTK_WINDOW (window));
  gtk_window_set_modal ( GTK_WINDOW (fenetre_preferences),
			 TRUE);
  gnome_dialog_set_parent( GNOME_DIALOG ( fenetre_preferences ),
			   GTK_WINDOW ( window) );

  gtk_widget_hide ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> help_button );


/*   on crée le premier onglet : Général */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_general(),
				   gtk_label_new ( _("Main") ) );


/* création du 2ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_fichier(),
				   gtk_label_new ( _("Files")  ));

/* création du 3ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_echeances(),
				   gtk_label_new ( _("Scheduler") ) );


/* création du 4ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_affichage(),
				   gtk_label_new ( _("Display") ) );


/* création du 5ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_applet(),
				   gtk_label_new ( _("Applet") ) );



/* création du 6ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_devises(),
				   gtk_label_new ( _("Currencies") ) );


/* création du 7ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_banques(),
				   gtk_label_new ( _("Banks") ) );

/* création du 9ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_exercices(),
				   gtk_label_new ( _("Financial years") ) );

/* création du 9ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_types_operations(),
				   gtk_label_new ( _("Methods of payment") ) );

/* création du 10ème onglet */

  gnome_property_box_append_page ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				   onglet_affichage_liste(),
				   gtk_label_new ( _("Informations") ) );

/* connection du bouton appliquer */


  gtk_signal_connect ( GTK_OBJECT ( fenetre_preferences ),
		       "apply",
		       (GtkSignalFunc) changement_preferences,
		       NULL );

/* on se met sur la page demandée */

  gtk_notebook_set_page ( GTK_NOTEBOOK ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> notebook ),
			  page_demandee );
  gtk_widget_show ( fenetre_preferences );

  gnome_property_box_set_state ( GNOME_PROPERTY_BOX ( fenetre_preferences ),
				 FALSE );

}
/* ************************************************************************************************************** */






/* ************************************************************************************************************** */
/* page Général */
/* ************************************************************************************************************** */

GtkWidget *onglet_general ( void )
{
  GtkWidget *hbox_pref;
  GtkWidget *vbox_pref;
  GtkWidget *separateur;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *scrolled_window;


  vbox_pref = gtk_vbox_new ( FALSE,               /* fenetre de base verticale */
			     5 );
  gtk_widget_show ( vbox_pref );

  hbox_pref = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
		       hbox_pref,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox_pref );



/* création de la 1ère colonne */

  vbox = gtk_vbox_new ( FALSE,
			    5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       vbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( vbox );


/* action de la touche entree */

  bouton_entree_enregistre = gtk_radio_button_new_with_label ( NULL,
						       SPACIFY(_("ENTER means save the transaction")) );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_entree_enregistre,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_entree_enregistre );

  bouton_entree_enregistre_pas = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_entree_enregistre )),
								   SPACIFY(_("ENTER means switch beetween the form fields")) );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_entree_enregistre_pas,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_entree_enregistre_pas );

  if ( etat.entree == 1 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_entree_enregistre ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_entree_enregistre_pas ),
				   TRUE );
   gtk_signal_connect_object ( GTK_OBJECT ( bouton_entree_enregistre ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));

/* séparation */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


/* affichage ou non d'un message d'alerte quand passage sous les soldes minis */

  bouton_solde_mini = gtk_radio_button_new_with_label ( NULL,
							SPACIFY(_("Warning message if the minimum balances are over minimum level defined")) );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_solde_mini,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_solde_mini );

   bouton_pas_solde_mini= gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_solde_mini )),
						     SPACIFY(_("No warnings")) );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_pas_solde_mini,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_pas_solde_mini );

  if ( etat.alerte_mini == 1 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (bouton_solde_mini  ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_pas_solde_mini ),
				   TRUE );

  gtk_signal_connect_object ( GTK_OBJECT ( bouton_solde_mini ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
     

/* séparation */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


/* affichage ou non d'un message d'alerte sur la permission du fichier de compte */

  bouton_affiche_permission = gtk_radio_button_new_with_label ( NULL,
							SPACIFY(_("Warning message if the file is readable by someone else")) );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_permission,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_permission );

  bouton_affiche_pas_permission = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_affiche_permission )),
							   SPACIFY(_("No warnings")) );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_pas_permission,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_pas_permission );

  if ( etat.alerte_permission )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_permission ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_pas_permission ),
				   TRUE );

  gtk_signal_connect_object ( GTK_OBJECT ( bouton_affiche_permission ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
     

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


/* séparateur vertical entre les deux parties */

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );



/* création de la 2ème colonne */

  vbox = gtk_vbox_new ( FALSE,
			    5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       vbox,
		       TRUE,
		       TRUE,
		       0);
  gtk_widget_show ( vbox );

  /* mise en place du titre du fichier */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("File name")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  entree_titre_fichier = gtk_entry_new ();
  if ( titre_fichier )
    gtk_entry_set_text ( GTK_ENTRY ( entree_titre_fichier ),
			 titre_fichier );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_titre_fichier ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_titre_fichier,
		       TRUE,
		       TRUE,
		       0);
  gtk_widget_show ( entree_titre_fichier );

  if ( !nb_comptes )
    gtk_widget_set_sensitive ( entree_titre_fichier,
			       FALSE );

  /* mise en place de l'adresse commune */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Common address")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  scrolled_window = gtk_scrolled_window_new ( NULL,
					      NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       scrolled_window,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( scrolled_window );

  entree_adresse_commune = gtk_text_new ( FALSE,
					  FALSE );
  gtk_text_set_editable ( GTK_TEXT ( entree_adresse_commune ),
			  TRUE );
  if ( adresse_commune )
    gtk_text_insert ( GTK_TEXT (entree_adresse_commune  ),
		      NULL,
		      NULL,
		      NULL,
		      adresse_commune,
		      -1 );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_adresse_commune ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      entree_adresse_commune );
  gtk_widget_show ( entree_adresse_commune );

  /* mise en place de la seconde adresse */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Secondary address")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  scrolled_window = gtk_scrolled_window_new ( NULL,
					      NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       scrolled_window,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( scrolled_window );

  entree_adresse_secondaire = gtk_text_new ( FALSE,
					  FALSE );
  gtk_text_set_editable ( GTK_TEXT ( entree_adresse_secondaire ),
			  TRUE );
  if ( adresse_secondaire )
    gtk_text_insert ( GTK_TEXT ( entree_adresse_secondaire ),
		      NULL,
		      NULL,
		      NULL,
		      adresse_secondaire,
		      -1 );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_adresse_secondaire ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      entree_adresse_secondaire );
  gtk_widget_show ( entree_adresse_secondaire );



  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  if ( !nb_comptes )
    {
      gtk_widget_set_sensitive ( entree_adresse_commune,
				 FALSE );
      gtk_widget_set_sensitive ( entree_adresse_secondaire,
				 FALSE );
    }



  return ( vbox_pref );

}
/* ************************************************************************************************************** */






/* ************************************************************************************************************** */
/* page Fichier */
/* ************************************************************************************************************** */

GtkWidget *onglet_fichier ( void )
{
  GtkWidget *box_pref1, *hbox_pref, *vbox_pref;
  GtkWidget *separateur;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;

  vbox_pref = gtk_vbox_new ( FALSE,               /* fenetre de base verticale */
			     5 );
  gtk_widget_show ( vbox_pref );

  hbox_pref = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
		       hbox_pref,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox_pref );



  box_pref1 = gtk_vbox_new ( FALSE,
			    5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       box_pref1,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( box_pref1 );


/* configuration du démarrage automatique du dernier fichier ou non */

  bouton_avec_demarrage = gtk_radio_button_new_with_label ( NULL,
							    SPACIFY(_("Automatic loading of the last file used")) );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       bouton_avec_demarrage,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_avec_demarrage );

  bouton_sans_demarrage = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_avec_demarrage )),
						     SPACIFY(_("Start without automatic loading")) );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       bouton_sans_demarrage,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_sans_demarrage );

  if ( etat.dernier_fichier_auto == 1 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_avec_demarrage ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_sans_demarrage ),
				   TRUE );

  gtk_signal_connect_object ( GTK_OBJECT ( bouton_avec_demarrage ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
     

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  /* configuration de la sauvegarde automatique */

  bouton_save_auto = gtk_radio_button_new_with_label ( NULL,
							    SPACIFY(_("Automatically save while closing")) );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       bouton_save_auto,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (bouton_save_auto  );

  bouton_save_non_auto = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_save_auto )),
							   SPACIFY(_("Ask each time for saving")) );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       bouton_save_non_auto,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_save_non_auto );

  if ( etat.sauvegarde_auto == 1 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_save_auto ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_save_non_auto ),
				   TRUE );
    
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_save_auto ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


/* configuration de l'enregistrement forcé */

  bouton_force_enregistrement = gtk_radio_button_new_with_label ( NULL,
							    SPACIFY(_("Force saving")) );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       bouton_force_enregistrement,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_force_enregistrement );

  bouton_force_pas_enregistrement = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_force_enregistrement )),
							   SPACIFY(_("Save only if the file isn't open by another user")) );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       bouton_force_pas_enregistrement,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_force_pas_enregistrement );

  if ( etat.force_enregistrement )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_force_enregistrement ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_force_pas_enregistrement ),
				   TRUE );
    
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_force_enregistrement ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );



/* séparateur vertical entre les deux parties */

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


/* fenetre verticale à droite */

  vbox = gtk_vbox_new ( FALSE,
			    5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       vbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( vbox );


  /* mise en forme de la demande de backup */

  bouton_demande_backup = gtk_check_button_new_with_label ( _("Automatic backup") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_demande_backup,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_demande_backup );

  if ( nb_comptes )
    {
      if ( nom_fichier_backup )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ),
				       TRUE );
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ),
				       FALSE );

      gtk_signal_connect_object ( GTK_OBJECT ( bouton_demande_backup ),
				  "toggled",
				  GTK_SIGNAL_FUNC ( gnome_property_box_changed),
				  GTK_OBJECT (fenetre_preferences));
      gtk_signal_connect ( GTK_OBJECT ( bouton_demande_backup ),
			   "toggled",
			   GTK_SIGNAL_FUNC ( changement_choix_backup ),
			   NULL );

      /* mise en forme de l'entrée du chemin de la backup */

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
      
      gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBO ( GNOME_FILE_ENTRY ( entree_chemin_backup) -> gentry ) -> entry ),
				  "changed",
				  gnome_property_box_changed,
				  GTK_OBJECT (fenetre_preferences));
      gtk_box_pack_start ( GTK_BOX ( vbox ),
			   entree_chemin_backup,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( entree_chemin_backup );


    }
    else
      gtk_widget_set_sensitive ( bouton_demande_backup,
				 FALSE );


  /* mise en forme de la demande de sauvegarde à l'ouverture */

  bouton_save_demarrage = gtk_check_button_new_with_label ( _("Backup at each opening (~/.filename.bak)") );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_save_demarrage ),
			      "toggled",
			      GTK_SIGNAL_FUNC ( gnome_property_box_changed),
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_save_demarrage,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_save_demarrage );


  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_save_demarrage ),
				 etat.sauvegarde_demarrage );


  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  /* mise en forme de la demande du nombre de derniers fichiers à garder en mémoire */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Memorise last opened files")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  spin_button_derniers_fichiers_ouverts = gtk_spin_button_new ( GTK_ADJUSTMENT ( gtk_adjustment_new ( nb_max_derniers_fichiers_ouverts,
												      0,
												      20,
												      1,
												      5,
												      1 )),
								1,
								0 );
  gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON ( spin_button_derniers_fichiers_ouverts ),
				TRUE );
  gtk_signal_connect_object ( GTK_OBJECT ( spin_button_derniers_fichiers_ouverts ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       spin_button_derniers_fichiers_ouverts,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( spin_button_derniers_fichiers_ouverts );

  /* mise en forme de la compression de l'enregistrement */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("File compression level")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  spin_button_compression_fichier = gtk_spin_button_new ( GTK_ADJUSTMENT ( gtk_adjustment_new ( compression_fichier,
												0,
												9,
												1,
												5,
												1 )),
							  1,
							  0 );
  gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON ( spin_button_compression_fichier ),
				TRUE );
  gtk_signal_connect_object ( GTK_OBJECT ( spin_button_compression_fichier ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       spin_button_compression_fichier,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( spin_button_compression_fichier );

  /* mise en forme de la compression de la backup */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Backup compression level")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  spin_button_compression_backup = gtk_spin_button_new ( GTK_ADJUSTMENT ( gtk_adjustment_new ( compression_backup,
												0,
												9,
												1,
												5,
												1 )),
							  1,
							  0 );
  gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON ( spin_button_compression_backup ),
				TRUE );
  gtk_signal_connect_object ( GTK_OBJECT ( spin_button_compression_backup ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       spin_button_compression_backup,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( spin_button_compression_backup );


  return ( vbox_pref );

}
/* ************************************************************************************************************** */






/* ************************************************************************************************************** */
/* page Echéances */
/* ************************************************************************************************************** */

GtkWidget *onglet_echeances ( void )
{
  GtkWidget *box_pref1, *hbox_pref;
  GtkWidget *separateur;
  GtkWidget *label;
  GtkWidget *hvbox;



  hbox_pref = gtk_hbox_new ( FALSE,
			     5 );
  gtk_widget_show ( hbox_pref );



  box_pref1 = gtk_vbox_new ( FALSE,
			    5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       box_pref1,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( box_pref1 );



/*   configuration du nb de jours avant la prise d'une échéance  */

  label = gtk_label_new ( SPACIFY(COLON(_("Number of days before a warning message advertising a scheduled transaction"))) );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hvbox = gtk_hbox_new ( FALSE,
			      10 );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       hvbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hvbox );

  entree_jours = gtk_entry_new_with_max_length (10);
  gtk_widget_set_usize ( GTK_WIDGET ( entree_jours ),
			 60,
			 FALSE );
  gtk_entry_set_text ( GTK_ENTRY ( entree_jours ),
		       g_strdup_printf ( "%d",
					 decalage_echeance ));
  gtk_signal_connect_object ( GTK_OBJECT ( entree_jours ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));

  gtk_box_pack_end ( GTK_BOX ( hvbox ),
		       entree_jours,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_jours );

  separateur = gtk_hseparator_new ();

  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );



  return ( hbox_pref );

}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* page applet */
/* ************************************************************************************************************** */

GtkWidget *onglet_applet ( void )
{
  GtkWidget *box_pref1, *hbox_pref, *vbox_pref;
  GtkWidget *separateur;
  GSList *liste_tmp;
  GtkWidget *scrolled_window;
  GtkWidget *vbox_fleches, *fleche;


  vbox_pref = gtk_vbox_new ( FALSE,
			     5 );
  gtk_widget_show ( vbox_pref );

  hbox_pref = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
		       hbox_pref,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox_pref );



  box_pref1 = gtk_vbox_new ( FALSE,
			    5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       box_pref1,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( box_pref1 );


/* affichage ou non de l'applet */

  bouton_affichage_applet = gtk_check_button_new_with_label ( _("Use the applet for checking scheduled transactions") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affichage_applet ),
				  verifie_affichage_applet () );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_affichage_applet ),
			      "toggled",
			      GTK_SIGNAL_FUNC ( gnome_property_box_changed),
			      GTK_OBJECT (fenetre_preferences));
  gtk_signal_connect ( GTK_OBJECT ( bouton_affichage_applet ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( changement_utilisation_applet ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       bouton_affichage_applet,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affichage_applet );

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

/* affiche la liste des comptes à vérifier */

  frame_demarrage = gtk_frame_new ( SPACIFY(_("List of the account files that will be checked")) );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_demarrage ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_widget_set_sensitive ( GTK_WIDGET ( frame_demarrage ),
			     verifie_affichage_applet () );
  gtk_box_pack_start ( GTK_BOX ( box_pref1 ),
		       frame_demarrage,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame_demarrage );
  
/* création de la vbox ds la frame */

  hbox_pref = gtk_vbox_new ( FALSE,
			     5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( hbox_pref ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame_demarrage ),
		      hbox_pref );
  gtk_widget_show ( hbox_pref );

/* création de la liste des comptes vérifiés */

  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       scrolled_window,
		       FALSE,
		       FALSE,
		       0);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize ( GTK_WIDGET ( scrolled_window ),
			 200,
			 120 );
  gtk_widget_show ( scrolled_window );

  liste_comptes_verifies = gtk_clist_new ( 1 );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_comptes_verifies ) ,
				      0,
				      TRUE );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  liste_comptes_verifies );
  fichier_verifier_selectionne = -1;
  gtk_signal_connect ( GTK_OBJECT ( liste_comptes_verifies ),
		       "select_row",
		       (GtkSignalFunc ) change_selection_verif,
		        &fichier_verifier_selectionne);
  gtk_widget_show ( liste_comptes_verifies );

  nb_fichier_verifier = 0;
  fichier_a_verifier_tmp = NULL;

  if ( ( liste_tmp = fichier_a_verifier ))
    do
      {
	if ( strlen ( liste_tmp->data ))
	  {
	    gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_verifies ),
				     gtk_clist_append ( GTK_CLIST ( liste_comptes_verifies ), (gchar **) &liste_tmp->data ),
				     liste_tmp->data );
	    nb_fichier_verifier++;
	  }
      }
    while ( ( liste_tmp = liste_tmp -> next ));


/*   pour l'instant, aucune suppression de ces fichiers */

  liste_suppression_fichier_a_verifier = NULL;

  /* création des boutons ajouter et enlever sur le dessous de la liste */

  vbox_fleches = gtk_hbox_new ( FALSE,
				5 );

  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       vbox_fleches,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( vbox_fleches );

  fleche = gnome_stock_button ( GNOME_STOCK_PIXMAP_ADD );
  gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			 GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( fleche ),
		       "clicked",
		       (GtkSignalFunc ) ajouter_verification,
		       fenetre_preferences);
  gtk_box_pack_start ( GTK_BOX ( vbox_fleches ),
		       fleche,
		       TRUE,
		       FALSE,
		       5 );
  gtk_widget_show ( fleche );

  bouton_enlever = gnome_stock_button ( GNOME_STOCK_PIXMAP_REMOVE );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_enlever ),
			 GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_enlever ),
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_enlever ),
		       "clicked",
		       (GtkSignalFunc ) supprimer_verification,
		       fenetre_preferences );
  gtk_box_pack_start ( GTK_BOX ( vbox_fleches ),
		       bouton_enlever,
		       TRUE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton_enlever );


  return ( vbox_pref );
}
/* ************************************************************************************************************** */




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

      nom_fichier = g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY (fenetre_choix_fichier )))));
      
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

      gnome_property_box_changed ( GNOME_PROPERTY_BOX (fenetre_preferences ) );

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
    
  gnome_property_box_changed ( GNOME_PROPERTY_BOX ( fenetre_preferences ) );

}
/* **************************************************************************************************************************** */




/* **************************************************************************************************************************** */
void changement_choix_backup ( GtkWidget *bouton,
			       gpointer pointeur )
{
  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ) ) )
    gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ),
			       TRUE );
  else
    gtk_widget_set_sensitive ( GTK_WIDGET ( entree_chemin_backup ),
			       FALSE );
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void changement_preferences ( GtkWidget *fenetre_preferences,
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
			g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier )))))
	    {
	      modification_fichier ( TRUE );
	      titre_fichier = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier ))));

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
	  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier )))))
	    {
	      titre_fichier = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_titre_fichier ))));
	      label_titre_fichier = gtk_label_new ( titre_fichier );
	      gtk_box_pack_start ( GTK_BOX ( ((GtkBoxChild *)(GTK_BOX ( page_accueil )->children->next->next->data))->widget ),
				   label_titre_fichier,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_box_reorder_child ( GTK_BOX ( ((GtkBoxChild *)(GTK_BOX ( page_accueil )->children->next->next->data))->widget ),
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
						g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( GTK_COMBO ( GNOME_FILE_ENTRY (entree_chemin_backup) -> gentry) -> entry  ) )))))
	    modification_fichier ( TRUE );

	  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_demande_backup ) )
	       &&
	       strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( GTK_COMBO ( GNOME_FILE_ENTRY (entree_chemin_backup) -> gentry) -> entry  ) ) )))
	    nom_fichier_backup = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( GTK_COMBO ( GNOME_FILE_ENTRY (entree_chemin_backup) -> gentry) -> entry  ) ) ));
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

      if ( ( buffer = g_strtod ( gtk_entry_get_text ( GTK_ENTRY ( entree_jours ) ), FALSE ) ) != decalage_echeance )
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

      if ( ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affichage_applet ) ) ) && (!verifie_affichage_applet() ) )
	system ( g_strconcat ( APPLET_BIN_DIR, "&", NULL ) );

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
				  gnome_property_box_changed,
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

    default :

    }

  sauve_configuration ();

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

