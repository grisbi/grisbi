/* fichier qui s'occupe de la page d'accueil ( de démarrage lors de l'ouverture d'un fichier */
/*           accueil.c */

/*     Copyright (C) 2000-2002  Cédric Auger */
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


/* ************************************************************************************************************ */
GtkWidget *creation_onglet_accueil ( void )
{
  GtkWidget *fenetre_accueil;
  GtkWidget *base;
  GtkWidget *base_scroll;
  GtkWidget *base_box_scroll;
  struct passwd *utilisateur;
  gchar *nom_utilisateur;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *separateur;
  gchar tampon_date [50];
  time_t date;


/*   la première séparation : une hbox : à gauche, le logo, à droite le reste */

  fenetre_accueil = gtk_hbox_new ( FALSE,
				   15 );

  gtk_widget_show ( fenetre_accueil );


  /* création du logo */

  if ( chemin_logo )
    {
      logo_accueil =  gnome_pixmap_new_from_file ( chemin_logo );

      gtk_box_pack_start ( GTK_BOX ( fenetre_accueil ),
			   logo_accueil,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( logo_accueil );

      /* séparation gauche-droite */

      separateur = gtk_vseparator_new ();
      gtk_box_pack_start ( GTK_BOX ( fenetre_accueil ),
			   separateur,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( separateur );
  
    }
  else
    logo_accueil = NULL;

/* création de la partie droite */

  base = gtk_vbox_new ( FALSE,
			15 );
  gtk_box_pack_end ( GTK_BOX ( fenetre_accueil ),
		       base,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( base );


  /*   la partie tout en haut affiche la date, le nom et l'heure */

  hbox = gtk_hbox_new ( FALSE,
				  5 );
  gtk_box_pack_start ( GTK_BOX ( base ),
		       hbox,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( hbox );


  /* récupère le nom de l'utilisateur, si nul, met le login */

  utilisateur = getpwuid ( getuid () );

  if ( !strlen ( nom_utilisateur = g_strdelimit ( utilisateur->pw_gecos, ",", 0 ) ) )
    nom_utilisateur = utilisateur->pw_name;

  label = gtk_label_new ( g_strconcat ( _("Utilisateur : "), nom_utilisateur, NULL) );

  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       TRUE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );


  /* met la date à coté */

  time ( &date );
  strftime ( (gchar *) tampon_date,
	     (size_t) 50,
	     "%A %d %B %Y",
	     (const struct tm *) localtime ( &date ) );


/* met la première lettre en majuscule */

  tampon_date[0] = toupper ( tampon_date[0]);

  label = gtk_label_new ( tampon_date );
  gtk_misc_set_alignment ( GTK_MISC (label ),
			   1,
			   1);

  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       TRUE,
 		       TRUE,
		       5 );
  gtk_widget_show ( label );


  /*   crée le timer qui appelle la fonction change_temps toutes les secondes */

  strftime ( (gchar *) tampon_date,
	     (size_t) 50,
	     "%X",
	     (const struct tm *) localtime ( &date ) );

  label_temps = gtk_label_new ( tampon_date );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label_temps,
		       TRUE,
		       FALSE,
		       5 );
  gtk_widget_show ( label_temps );

  id_temps = gtk_timeout_add ( 1000,
			       (GtkFunction) change_temps,
			       GTK_WIDGET ( label_temps ));


  /*   en dessous, on met le titre du fichier s'il existe */

  if ( titre_fichier )
    {
      label_titre_fichier = gtk_label_new ( titre_fichier );
      gtk_box_pack_start ( GTK_BOX ( base ),
			   label_titre_fichier,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label_titre_fichier );
    }

  /* séparation haut-bas */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( base ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  /* on crée à ce niveau base_scroll qui est aussi une vbox mais qui peut scroller verticalement */

  base_scroll = gtk_scrolled_window_new ( NULL,
					  NULL);
  gtk_container_set_border_width ( GTK_CONTAINER ( base_scroll ),
				   5 );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( base_scroll ),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX ( base ),
		      base_scroll,
		      TRUE,
		      TRUE,
		      0);
  gtk_widget_show (base_scroll);
  

  /* on met la nouvelle vbox dans le fenetre scrollable */

  base_box_scroll = gtk_vbox_new ( FALSE,
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (base_scroll),
					  base_box_scroll);

  gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN (base_scroll)  -> child ),
				 GTK_SHADOW_NONE );
  gtk_container_set_border_width ( GTK_CONTAINER ( base_box_scroll ),
				   10 );
  gtk_widget_show (base_box_scroll);



  /* on crée la première frame dans laquelle on met les états des comptes */

  frame_etat_comptes_accueil = gtk_frame_new ( _(" Solde des comptes ") );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_etat_comptes_accueil ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       frame_etat_comptes_accueil,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame_etat_comptes_accueil );


  /* on met la liste des comptes et leur état dans la frame */

  update_liste_comptes_accueil ();


  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );



/* mise en place de la partie fin des comptes passif */

  frame_etat_fin_compte_passif = gtk_frame_new ( _(" Comptes passifs terminés ") );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_etat_fin_compte_passif ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       frame_etat_fin_compte_passif,
		       FALSE,
		       FALSE,
		       0 );

  separateur_passif_manu = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       separateur_passif_manu,
		       FALSE,
		       FALSE,
		       0 );


  mise_a_jour_fin_comptes_passifs ();

/*   mise en place de la partie des échéances manuelles ( non affiché ) */


  frame_etat_echeances_manuelles_accueil = gtk_frame_new ( _(" Echéances manuelles arrivées à terme ") );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_etat_echeances_manuelles_accueil ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       frame_etat_echeances_manuelles_accueil,
		       FALSE,
		       FALSE,
		       0 );

  separateur_manu_auto = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       separateur_manu_auto,
		       FALSE,
		       FALSE,
		       0 );


  /* mise en place de la partie des échéances auto  ( non affiché )*/

  frame_etat_echeances_auto_accueil = gtk_frame_new ( _(" Echéances automatiques saisies ") );

  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_etat_echeances_auto_accueil ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       frame_etat_echeances_auto_accueil,
		       FALSE,
		       FALSE,
		       0 );

  separateur_auto_mini = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       separateur_auto_mini,
		       FALSE,
		       FALSE,
		       0 );


/* partie des fin d'échéances */

  frame_etat_echeances_finies = gtk_frame_new ( _(" Echéances terminées : ") );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_etat_echeances_finies ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       frame_etat_echeances_finies,
		       FALSE,
		       FALSE,
		       0 );


  separateur_ech_finies_soldes_mini = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       separateur_ech_finies_soldes_mini,
		       FALSE,
		       FALSE,
		       0 );

/* partie des soldes minimaux autorisés */

  frame_etat_soldes_minimaux_autorises = gtk_frame_new ( _(" Soldes minimaux autorisés ") );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_etat_soldes_minimaux_autorises ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       frame_etat_soldes_minimaux_autorises,
		       FALSE,
		       FALSE,
		       0 );


  separateur_des_soldes_mini = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       separateur_des_soldes_mini,
		       FALSE,
		       FALSE,
		       0 );


/* partie des soldes minimaux voulus */

  frame_etat_soldes_minimaux_voulus = gtk_frame_new ( _(" Soldes minimaux voulus ") );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_etat_soldes_minimaux_voulus ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX ( base_box_scroll ),
		       frame_etat_soldes_minimaux_voulus,
		       FALSE,
		       FALSE,
		       0 );

  mise_a_jour_soldes_minimaux ();

  return ( fenetre_accueil );
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
void change_temps ( GtkWidget *label_temps )
{
  gchar tampon_date [50];
  time_t date;

  time ( &date );
  strftime ( (gchar *) tampon_date,
	     (size_t) 50,
	     "%X",
	     (const struct tm *) localtime ( &date ) );

  gtk_label_set_text ( GTK_LABEL (label_temps ),
		       tampon_date );

}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction appelée lorsqu'on clicke sur une échéance à saisir */
/* ************************************************************************************************************ */

void saisie_echeance_accueil ( GtkWidget *event_box,
			       GdkEventButton *event,
			       struct operation_echeance *echeance )
{
  GtkWidget *ancien_parent;
  struct operation_echeance *ancienne_selection_echeance;
  GtkWidget *dialog;
  gint resultat;


    
  /* on sélectionne l'échéance demandée */

  ancienne_selection_echeance = echeance_selectionnnee;
  echeance_selectionnnee = echeance;

  ancien_parent = formulaire_echeancier -> parent;

  /* crée la boite de dialogue */

  dialog = gnome_dialog_new ( _("Saisie d'une échéance"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_widget_set_usize ( GTK_WIDGET ( dialog ),
			 600,
			 FALSE );
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
		        "destroy" );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			     0 );


  /* met le formulaire dans la boite de dialogue */

  gtk_widget_unrealize ( frame_formulaire_echeancier );
  gtk_widget_reparent ( formulaire_echeancier,
			GNOME_DIALOG ( dialog ) -> vbox );


/* applique aux entry l'entrée à ok */

  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( widget_formulaire_echeancier [0] ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( GTK_COMBOFIX ( widget_formulaire_echeancier [1] ) -> entry ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( widget_formulaire_echeancier [2] ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( widget_formulaire_echeancier [3] ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( GTK_COMBOFIX ( widget_formulaire_echeancier [6] ) -> entry ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( widget_formulaire_echeancier [8] ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( GTK_COMBOFIX ( widget_formulaire_echeancier [10] ) -> entry ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( widget_formulaire_echeancier [11] ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( widget_formulaire_echeancier [12] ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( widget_formulaire_echeancier [14] ));


  /* enlève les boutons valider/annuler si necessaire */

  if ( etat.affiche_boutons_valider_annuler )
    {
      gtk_widget_hide (separateur_formulaire_echeancier);
      gtk_widget_hide (hbox_valider_annuler_echeance);
    }

  /* remplit le formulaire */

  click_sur_saisir_echeance();


  etat.formulaire_echeance_dans_fenetre = 1;

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( !resultat )
    fin_edition_echeance ();

  gtk_widget_reparent ( formulaire_echeancier,
			ancien_parent );

  gnome_dialog_close ( GNOME_DIALOG ( dialog ));

  etat.formulaire_echeance_dans_fenetre = 0;

  /* remet les boutons valider/annuler si necessaire */

  if ( etat.affiche_boutons_valider_annuler )
    {
      gtk_widget_show (separateur_formulaire_echeancier);
      gtk_widget_show (hbox_valider_annuler_echeance);
    }


  formulaire_echeancier_a_zero();

  echeance_selectionnnee = ancienne_selection_echeance;


  if ( !etat.formulaire_echeancier_toujours_affiche )
    gtk_widget_hide ( frame_formulaire_echeancier );

}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
void met_en_prelight ( GtkWidget *event_box,
			GdkEventMotion *event,
			gpointer pointeur )
{
  gtk_widget_set_state ( GTK_WIDGET ( GTK_BIN (event_box)->child ),
			 GTK_STATE_PRELIGHT );
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
void met_en_normal ( GtkWidget *event_box,
		     GdkEventMotion *event,
		     gpointer pointeur )
{
  gtk_widget_set_state ( GTK_WIDGET ( GTK_BIN (event_box)->child ),
			 GTK_STATE_NORMAL );
}
/* ************************************************************************************************************ */




/* ************************************************************************************************************ */
/* Fonction update_liste_comptes_accueil */
/* affiche la liste des comptes et leur solde courant dans la frame qui leur */
/* est réservée dans l'accueil */
/* ************************************************************************************************************ */


void update_liste_comptes_accueil ( void )
{
  GtkStyle *style_label_nom_compte;
  GdkColor couleur_bleue, couleur_jaune;
  GtkWidget *event_box;
  GtkWidget *label;
  GSList *ordre_comptes_variable;
  GtkWidget *table;
  gint i;

  if ( !nb_comptes )
    return;

/* création du style normal -> bleu */
/* pointeur dessus -> jaune-rouge */

  label = gtk_label_new ("");

  style_label = gtk_style_copy ( gtk_widget_get_style (label));
  style_label_nom_compte = gtk_style_copy ( gtk_widget_get_style (label));
  couleur_bleue.red = 500;
  couleur_bleue.green = 500;
  couleur_bleue.blue = 65535;
  couleur_jaune.red =40000;
  couleur_jaune.green =40000;
  couleur_jaune.blue = 0;

  style_label->fg[GTK_STATE_PRELIGHT] = couleur_jaune;
  style_label->fg[GTK_STATE_NORMAL] = couleur_bleue;

  style_label_nom_compte ->fg[GTK_STATE_PRELIGHT] = couleur_jaune;



  if ( GTK_BIN ( frame_etat_comptes_accueil ) -> child )
    gtk_widget_destroy ( GTK_BIN ( frame_etat_comptes_accueil ) -> child );


  /*  on met dans une table les comptes avec leur solde */

  table = gtk_table_new ( nb_comptes + 2,
			  5,
			  FALSE );
  gtk_container_add ( GTK_CONTAINER ( frame_etat_comptes_accueil ),
		      table );
  gtk_widget_show ( table );


/* met une ligne vide en haut ( le label a déjà été créé pour récupérer le style */

  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      label,
			      0, 1,
			      0, 1 );
  gtk_widget_show ( label );
 
/* affichage des comptes et de leur solde */

  ordre_comptes_variable = ordre_comptes;
  i = 1;

  do
    {
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );

      if ( !COMPTE_CLOTURE )
	{
	  /*       met un label à gauche, il a déjà été créé pour récupérer le style */

	  label = gtk_label_new ( "" );
	  gtk_table_attach_defaults ( GTK_TABLE ( table ),
				      label,
				      0, 1,
				      i, i+1 );
	  gtk_widget_show ( label );
    


	  event_box = gtk_event_box_new ();
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "enter_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_prelight ),
			       NULL );
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "leave_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_normal ),
			       NULL );
	  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
				      "button_press_event",
				      GTK_SIGNAL_FUNC ( changement_compte ),
				      GINT_TO_POINTER ( ordre_comptes_variable->data ) );
	  gtk_table_attach ( GTK_TABLE ( table ),
			     event_box,
			     1, 2,
			     i, i+1,
			     GTK_FILL |GTK_SHRINK,
			     GTK_FILL| GTK_SHRINK,
			     20, 0 );
	  gtk_widget_show ( event_box );

	  label = gtk_label_new ( g_strconcat ( (gchar *) NOM_DU_COMPTE, " : ", NULL ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5);
	  gtk_widget_set_style ( label,
				 style_label_nom_compte );

	  gtk_container_add ( GTK_CONTAINER ( event_box ),
			      label );
	  gtk_widget_show ( label );


	  event_box = gtk_event_box_new ();
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "enter_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_prelight ),
			       NULL );
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "leave_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_normal ),
			       NULL );
	  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
				      "button_press_event",
				      GTK_SIGNAL_FUNC ( changement_compte ),
				      GINT_TO_POINTER ( ordre_comptes_variable->data ));
	  gtk_table_attach ( GTK_TABLE ( table ),
			     event_box,
			     2, 3,
			     i, i+1,
			     GTK_FILL| GTK_SHRINK,
			     GTK_FILL| GTK_SHRINK,
			     0, 0 );
	  gtk_widget_show ( event_box );


	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    SOLDE_COURANT ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   1,
				   0.5);
	  gtk_widget_set_style ( label,
				 style_label );
	  gtk_container_add ( GTK_CONTAINER ( event_box ),
			      label );
	  gtk_widget_show ( label );


       /* mise en place du nom de la devise */

	  label = gtk_label_new ( ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
										  GINT_TO_POINTER ( DEVISE ),
										  (GCompareFunc) recherche_devise_par_no )-> data )) -> code_devise);
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5);
	  gtk_table_attach ( GTK_TABLE ( table ),
			     label,
			     3, 4,
			     i, i+1,
			     GTK_FILL | GTK_SHRINK,
			     GTK_FILL | GTK_SHRINK,
			     5, 0 );
	  gtk_widget_show ( label );



	  label = gtk_label_new ("");
	  gtk_table_attach_defaults ( GTK_TABLE ( table ),
				      label,
				      4, 5,
				      i, i+1 );
	  gtk_widget_show ( label );

	}
      i++;
    }
  while ( (  ordre_comptes_variable = ordre_comptes_variable->next ) );


  /* met un label vide en bas */

  label = gtk_label_new ( "" );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      label,
			      0, 1,
			      i, i+1 );
  gtk_widget_show ( label );
    

}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
void update_liste_echeances_manuelles_accueil ( void )
{
  verification_echeances_a_terme ();

  if ( echeances_a_saisir )
    {
      GtkWidget *vbox;
      GtkWidget *label;
      GSList *pointeur_liste;
      GtkWidget *event_box;
      GtkWidget *hbox;

      /* s'il y avait déjà un fils dans la frame, le détruit */
      
      if ( GTK_BIN ( frame_etat_echeances_manuelles_accueil ) -> child )
	gtk_widget_destroy ( GTK_BIN ( frame_etat_echeances_manuelles_accueil ) -> child );

      /*       on affiche la seconde frame dans laquelle on place les échéances à saisir */

      gtk_widget_show ( frame_etat_echeances_manuelles_accueil );
      gtk_widget_show ( separateur_manu_auto );
 
      /* on y place la liste des échéances */

      vbox = gtk_vbox_new ( FALSE,
			    5 );
      gtk_container_add ( GTK_CONTAINER ( frame_etat_echeances_manuelles_accueil ),
			  vbox );
      gtk_widget_show ( vbox );

      /* on met une ligne vide pour faire joli */

      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX (vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );


      pointeur_liste = echeances_a_saisir;

      while ( pointeur_liste )
	{
	  hbox = gtk_hbox_new ( TRUE,
				5 );
	  gtk_box_pack_start ( GTK_BOX ( vbox ),
			       hbox,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show (  hbox );

	  /* 	  bouton à gauche */

	  event_box = gtk_event_box_new ();
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "enter_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_prelight ),
			       NULL );
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "leave_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_normal ),
			       NULL );
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "button_press_event",
			       (GtkSignalFunc) saisie_echeance_accueil,
			       ECHEANCE_COURANTE );
	  gtk_box_pack_start ( GTK_BOX ( hbox ),
			       event_box,
			       TRUE,
			       TRUE,
			       5 );
	  gtk_widget_show ( event_box  );


	  if ( ECHEANCE_COURANTE->tiers )
	    label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d : %s",
						      ECHEANCE_COURANTE->jour,
						      ECHEANCE_COURANTE->mois,
						      ECHEANCE_COURANTE->annee,
						      ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
												     GINT_TO_POINTER ( ECHEANCE_COURANTE->tiers ),
												     (GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers ) );
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d : %s",
						      ECHEANCE_COURANTE->jour,
						      ECHEANCE_COURANTE->mois,
						      ECHEANCE_COURANTE->annee,
						      _("Aucun tiers défini") ));
	  
	  gtk_widget_set_style ( label,
				 style_label );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5);
	  gtk_container_add ( GTK_CONTAINER ( event_box ),
			      label );
	  gtk_widget_show ( label  );
	  
	  /* 	  label à droite */

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ECHEANCE_COURANTE->compte;

	  if ( ECHEANCE_COURANTE -> montant >= 0 )
	    label = gtk_label_new ( g_strdup_printf (_(" Crédit de %4.2f %s sur %s"),
						     ECHEANCE_COURANTE->montant,
						     ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
												     GINT_TO_POINTER ( ECHEANCE_COURANTE -> devise ),
												     (GCompareFunc) recherche_devise_par_no )->data))-> code_devise,
						     NOM_DU_COMPTE ));
	  else
	    label = gtk_label_new ( g_strdup_printf (_(" Débit de %4.2f %s sur %s"),
						     -ECHEANCE_COURANTE->montant,
						     ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
												     GINT_TO_POINTER ( ECHEANCE_COURANTE -> devise ),
												     (GCompareFunc) recherche_devise_par_no )->data))-> code_devise,
						     NOM_DU_COMPTE ));


	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5);
	  gtk_box_pack_start ( GTK_BOX ( hbox ),
			       label,
			       FALSE,
			       TRUE,
			       0 );
	  gtk_widget_show (  label );

	  pointeur_liste = pointeur_liste -> next;
	}

      /* on met une ligne vide pour faire joli */

      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX (vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

    }
  else
    {
      gtk_widget_hide ( frame_etat_echeances_manuelles_accueil );
      gtk_widget_hide ( separateur_manu_auto );

    }
}
/* ************************************************************************************************************ */





/* ************************************************************************************************************ */
void update_liste_echeances_auto_accueil ( void )
{

  if ( echeances_saisies )
    {
      GtkWidget *vbox;
      GtkWidget *label;
      GSList *pointeur_liste;
      GtkWidget *hbox;
      struct structure_operation *operation;

      /* s'il y avait déjà un fils dans la frame, le détruit */
      
      if ( GTK_BIN ( frame_etat_echeances_auto_accueil ) -> child )
	gtk_widget_destroy ( GTK_BIN ( frame_etat_echeances_auto_accueil ) -> child );

      /*       on affiche la seconde frame dans laquelle on place les échéances à saisir */

      gtk_widget_show ( frame_etat_echeances_auto_accueil );
 

      /* on y place la liste des échéances */

      vbox = gtk_vbox_new ( FALSE,
			    5 );
      gtk_container_add ( GTK_CONTAINER ( frame_etat_echeances_auto_accueil ),
			  vbox);
      gtk_widget_show ( vbox);

      /* on met une ligne vide pour faire joli */

      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX ( vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );


      pointeur_liste = echeances_saisies;

      while ( pointeur_liste )
	{
	  operation = pointeur_liste -> data;

	  hbox = gtk_hbox_new ( TRUE,
						     5 );
	  gtk_box_pack_start ( GTK_BOX ( vbox ),
			       hbox,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show (  hbox );

	  /* 	  label à gauche */
	  
	  if ( operation ->tiers )
	    label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d : %s",
						      operation ->jour,
						      operation ->mois,
						      operation ->annee,
						      ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
												     GINT_TO_POINTER ( operation->tiers ),
												     (GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers ) );
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d : %s",
						      operation ->jour,
						      operation ->mois,
						      operation ->annee,
						      _("Aucun tiers défini") ));
	  

	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5);
	  gtk_box_pack_start ( GTK_BOX ( hbox ),
			       label,
			       TRUE,
			       TRUE,
			       5 );
	  gtk_widget_show ( label  );

	  /* 	  label à droite */

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation->no_compte;

	  if ( operation -> montant >= 0 )
	    label = gtk_label_new ( g_strdup_printf (_(" Crédit de %4.2f %s sur %s"),
						     operation->montant,
						     ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
												     GINT_TO_POINTER ( operation -> devise ),
												     (GCompareFunc) recherche_devise_par_no )->data))-> code_devise,
						     NOM_DU_COMPTE ));
	  else
	    label = gtk_label_new ( g_strdup_printf (_(" Débit de %4.2f %s sur %s"),
						     -operation->montant,
						     ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
												     GINT_TO_POINTER (  operation -> devise ),
												     (GCompareFunc) recherche_devise_par_no )->data))-> code_devise,
						     NOM_DU_COMPTE ));

	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5);
	  gtk_box_pack_start ( GTK_BOX ( hbox ),
			       label,
			       FALSE,
			       TRUE,
			       0 );
	  gtk_widget_show (  label );

	  pointeur_liste = pointeur_liste -> next;
	}

      /* on met une ligne vide pour faire joli */

      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX ( vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      /*       comme des opés ont été saisies, on met à jour les listes */

      mise_a_jour_tiers ();
      mise_a_jour_categ ();
      mise_a_jour_imputation ();
    }
  else
    {
      gtk_widget_hide ( frame_etat_echeances_auto_accueil );
      gtk_widget_hide ( separateur_auto_mini );
    }
}
/* ************************************************************************************************************ */




/* ************************************************************************************************************ */
/* Fonction mise_a_jour_soldes_minimaux */
/* vérifie les soldes de tous les comptes, affiche un message d'alerte si nécessaire */
/* et ajoute dans l'accueil les comptes sous les soldes minimaux */
/* ************************************************************************************************************ */

void mise_a_jour_soldes_minimaux ( void )
{
  GtkWidget *vbox_1;
  GtkWidget *vbox_2;
  GtkWidget *label;
  gint i;
  GtkWidget *hbox;

  /* s'il y avait déjà un fils dans la frame, le détruit */
      
  if ( GTK_BIN ( frame_etat_soldes_minimaux_autorises ) -> child )
    gtk_widget_destroy ( GTK_BIN ( frame_etat_soldes_minimaux_autorises ) -> child );
  if ( GTK_BIN ( frame_etat_soldes_minimaux_voulus ) -> child )
    gtk_widget_destroy ( GTK_BIN ( frame_etat_soldes_minimaux_voulus ) -> child );

  gtk_widget_hide ( separateur_des_soldes_mini );
  gtk_widget_hide ( frame_etat_soldes_minimaux_autorises );
  gtk_widget_hide ( frame_etat_soldes_minimaux_voulus );

  vbox_1 = NULL;
  vbox_2 = NULL;


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {
      if ( SOLDE_COURANT < SOLDE_MINI && TYPE_DE_COMPTE != 2 )
	{
	  if ( vbox_1 )
	    {
	      label = gtk_label_new ( NOM_DU_COMPTE );
	      gtk_box_pack_start ( GTK_BOX ( vbox_1 ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );
	    }
	  else
	    {
	      vbox_1 = gtk_vbox_new ( TRUE,
				      5 );
	      gtk_container_add ( GTK_CONTAINER ( frame_etat_soldes_minimaux_autorises ),
				  vbox_1 );
	      gtk_container_set_border_width ( GTK_CONTAINER ( vbox_1 ),
					       5 );
	      gtk_widget_show ( vbox_1 );
	      gtk_widget_show ( frame_etat_soldes_minimaux_autorises );
	      gtk_widget_show ( separateur_auto_mini );

	      label = gtk_label_new ("");
	      gtk_box_pack_start ( GTK_BOX ( vbox_1 ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );

	      hbox = gtk_hbox_new ( FALSE,
				    0 );
	      gtk_box_pack_start ( GTK_BOX ( vbox_1 ),
				   hbox,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( hbox );

	      label = gtk_label_new ( _("Les comptes suivants sont sous le seuil minimal autorisé :") );
	      gtk_box_pack_start ( GTK_BOX ( hbox ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );

	      label = gtk_label_new ( NOM_DU_COMPTE );
	      gtk_box_pack_start ( GTK_BOX ( vbox_1 ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );
	    }

	  if ( !MESSAGE_SOUS_MINI )
	    {
	      if ( SOLDE_COURANT < SOLDE_MINI_VOULU )
		{
		  if ( !patience_en_cours )
		    dialogue ( g_strconcat ( _("Attention, le compte "),
					     NOM_DU_COMPTE,
					     _(" est passé sous les seuils minimaux autorisés et voulus !"),
					     NULL ));
		  MESSAGE_SOUS_MINI_VOULU = 1;
		}
	      else
		if ( !patience_en_cours )
		  dialogue ( g_strconcat ( _("Attention, le compte "),
					   NOM_DU_COMPTE,
					   _(" est passé sous le seuil minimal autorisé !"),
					   NULL ));
	      MESSAGE_SOUS_MINI = 1;
	    }
	}

      if ( SOLDE_COURANT < SOLDE_MINI_VOULU && TYPE_DE_COMPTE != 2 )
	{
	  if ( vbox_2 )
	    {
	      label = gtk_label_new ( NOM_DU_COMPTE );
	      gtk_box_pack_start ( GTK_BOX ( vbox_2 ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );
	    }
	  else
	    {
	      vbox_2 = gtk_vbox_new ( TRUE,
				      5 );
	      gtk_container_add ( GTK_CONTAINER ( frame_etat_soldes_minimaux_voulus ),
				  vbox_2 );
	      gtk_container_set_border_width ( GTK_CONTAINER ( vbox_2 ),
					       5 );
	      gtk_widget_show ( vbox_2 );
	      gtk_widget_show ( frame_etat_soldes_minimaux_voulus );


	      label = gtk_label_new ("");
	      gtk_box_pack_start ( GTK_BOX ( vbox_2 ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );

	      if ( vbox_1 )
		gtk_widget_show ( separateur_des_soldes_mini );

	      hbox = gtk_hbox_new ( FALSE,
				    0 );
	      gtk_box_pack_start ( GTK_BOX ( vbox_2 ),
				   hbox,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( hbox );

	      label = gtk_label_new ( _("Les comptes suivants sont sous le seuil minimal voulu :") );
	      gtk_box_pack_start ( GTK_BOX ( hbox ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );

	      label = gtk_label_new ( NOM_DU_COMPTE );
	      gtk_box_pack_start ( GTK_BOX ( vbox_2 ),
				   label,
				   FALSE,
				   FALSE,
				   0 );
	      gtk_widget_show ( label );
	    }

	  if ( !MESSAGE_SOUS_MINI_VOULU )
	    {
	      if ( SOLDE_COURANT < SOLDE_MINI )
		{
		  if ( !patience_en_cours )
		    dialogue ( g_strconcat ( _("Attention, le compte "),
					     NOM_DU_COMPTE,
					     _(" est passé sous les seuils minimaux autorisés et voulus !"),
					     NULL ));
		  MESSAGE_SOUS_MINI = 1;
		}
	      else
		if ( !patience_en_cours )
		  dialogue ( g_strconcat ( _("Attention, le compte "),
					   NOM_DU_COMPTE,
					   _(" est passé sous le seuil minimal voulu !"),
					   NULL ));
	      MESSAGE_SOUS_MINI_VOULU = 1;
	    }
	}

      if ( SOLDE_COURANT > SOLDE_MINI )
	MESSAGE_SOUS_MINI = 0;
      if ( SOLDE_COURANT > SOLDE_MINI_VOULU )
	MESSAGE_SOUS_MINI_VOULU = 0;


      p_tab_nom_de_compte_variable++;
    }


  if ( vbox_1 )
    {
      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX ( vbox_1 ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );
    }

  if ( vbox_2 )
    {
      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX ( vbox_2 ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );
    }

}
/* ************************************************************************************************************ */





/* ************************************************************************************************************ */
void mise_a_jour_fin_comptes_passifs ( void )
{
  gint i;
  GtkWidget *vbox;
  GtkWidget *label;
  GSList *liste_tmp;
  GSList *pointeur;


  if ( GTK_BIN ( frame_etat_fin_compte_passif ) -> child )
    {
      gtk_widget_destroy ( GTK_BIN ( frame_etat_fin_compte_passif ) -> child );
      gtk_widget_hide (  frame_etat_fin_compte_passif );
      gtk_widget_hide ( separateur_manu_auto );
    }


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
  liste_tmp = NULL;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {
      if ( TYPE_DE_COMPTE == 2
	   &&
	   SOLDE_COURANT >= 0 )
	liste_tmp = g_slist_append ( liste_tmp,
				     NOM_DU_COMPTE );

      p_tab_nom_de_compte_variable++;
    }


  if ( g_slist_length ( liste_tmp ) )
    {
      vbox = gtk_vbox_new ( FALSE,
			    0 );
      gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				       5 );
      gtk_container_add ( GTK_CONTAINER ( frame_etat_fin_compte_passif ),
			  vbox );
      gtk_widget_show ( vbox );

      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX ( vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      if ( g_slist_length ( liste_tmp ) > 1 )
	label = gtk_label_new (_("Les comptes de passif suivants sont arrivés à terme :"));
      else
	label = gtk_label_new (_("Le compte de passif suivant est arrivé à terme :"));
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_box_pack_start ( GTK_BOX ( vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );


      pointeur = liste_tmp;

      while ( pointeur )
	{
	  label = gtk_label_new (pointeur -> data );
	  gtk_box_pack_start ( GTK_BOX ( vbox ),
			       label,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show ( label );

	  pointeur = pointeur -> next;
	}

      label = gtk_label_new ("");
      gtk_box_pack_start ( GTK_BOX ( vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      gtk_widget_show ( frame_etat_fin_compte_passif );
      gtk_widget_show ( separateur_passif_manu );
    }

}
/* ************************************************************************************************************ */
