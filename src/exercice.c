/* Ce fichier s'occupe de la gestion des exercices */
/* exercice.c */

/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http:// www.grisbi.org */

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
GtkWidget *onglet_exercices ( void )
{
  GtkWidget *hbox_pref, *vbox_pref;
  GtkWidget *separateur;
  GtkWidget *label;
  GtkWidget *frame;
  GSList *liste_tmp;
  GtkWidget *scrolled_window;
  GtkWidget *vbox;
  GtkWidget *vbox2;
  GtkWidget *bouton;
  GtkWidget *hbox;
  gchar *titres[]={_("Name")};


  vbox_pref = gtk_vbox_new ( FALSE,
			     5 );
  gtk_widget_show ( vbox_pref );

  hbox_pref = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
		       hbox_pref,
		       TRUE,
		       TRUE,
		       0);
  gtk_widget_show ( hbox_pref );



/* création de la 1ère colonne */

  vbox = gtk_vbox_new ( FALSE,
			    5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       vbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( vbox );


/* création de la clist des exercies */


  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       scrolled_window,
		       FALSE,
		       FALSE,
		       0);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize ( GTK_WIDGET ( scrolled_window ),
			 FALSE,
			 120 );
  gtk_widget_show ( scrolled_window );


  clist_exercices_parametres = gtk_clist_new_with_titles ( 1,
							 titres );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_exercices_parametres ) ,
				      0,
				      TRUE );
  gtk_clist_column_titles_passive ( GTK_CLIST ( clist_exercices_parametres ));
  gtk_signal_connect_object  ( GTK_OBJECT ( fenetre_preferences ),
			       "apply",
			       GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
			       GTK_OBJECT ( clist_exercices_parametres ));
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      clist_exercices_parametres );
  gtk_widget_show ( clist_exercices_parametres );



  /*   si pas de fichier ouvert, on grise */

  if ( !nb_comptes )
    gtk_widget_set_sensitive ( vbox,
			       FALSE );
  else
    {

      /* on crée la liste_struct_exercices_tmp qui est un copie de liste_struct_exercices originale */
      /* avec laquelle on travaillera dans les parametres */

      liste_struct_exercices_tmp = NULL;
      liste_tmp = liste_struct_exercices;

      while ( liste_tmp )
	{
	  struct struct_exercice *exercice;
	  struct struct_exercice *copie_exercice;

	  exercice = liste_tmp -> data;
	  copie_exercice = calloc ( 1,
				    sizeof ( struct struct_exercice ));

	  copie_exercice -> no_exercice = exercice -> no_exercice;
	  copie_exercice -> nom_exercice = g_strdup ( exercice -> nom_exercice );

	  if ( exercice -> date_debut )
	    copie_exercice -> date_debut = g_date_new_dmy ( g_date_day ( exercice -> date_debut ),
							    g_date_month ( exercice -> date_debut ),
							    g_date_year ( exercice -> date_debut ));

	  if ( exercice -> date_fin )
	    copie_exercice -> date_fin = g_date_new_dmy ( g_date_day ( exercice -> date_fin ),
							  g_date_month ( exercice -> date_fin ),
							  g_date_year ( exercice -> date_fin ));

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
	  struct struct_exercice *exercice;
	  gchar *ligne[1];
	  gint ligne_insert;

	  exercice = liste_tmp -> data;

	  ligne[0] = exercice -> nom_exercice;

	  ligne_insert = gtk_clist_append ( GTK_CLIST ( clist_exercices_parametres ),
					    ligne );

	  /* on associe à la ligne la struct de la exercice */

	  gtk_clist_set_row_data ( GTK_CLIST ( clist_exercices_parametres ),
				   ligne_insert,
				   exercice );

	  liste_tmp = liste_tmp -> next;
	}
    }

/* création de la hbox des boutons en dessous */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );


/* ajout du bouton ajouter */

  /* FIXME */
  bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
/*   bouton = gnome_stock_button ( GNOME_STOCK_PIXMAP_ADD ); */
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC  ( ajout_exercice ),
		       clist_exercices_parametres );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton );

/* ajout du bouton annuler */

  /* FIXME */
  bouton_supprimer_exercice = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
/*   bouton_supprimer_exercice = gnome_stock_button ( GNOME_STOCK_PIXMAP_REMOVE ); */
  gtk_widget_set_sensitive ( bouton_supprimer_exercice,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_exercice ),
		       "clicked",
		       GTK_SIGNAL_FUNC  ( supprime_exercice ),
		       clist_exercices_parametres );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_supprimer_exercice,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton_supprimer_exercice );





/* séparation */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( separateur );

  /* création du bouton association automatique */

  bouton = gtk_button_new_with_label ( _("Automatic association") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( association_automatique ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton );



  /* création de la 2ème colonne */

  vbox = gtk_vbox_new ( FALSE,
			    5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_box_pack_start ( GTK_BOX ( hbox_pref ),
		       vbox,
		       TRUE,
		       TRUE,
		       0);
  gtk_widget_show ( vbox );


/* frame de droite-haut qui contient les caractéristiques de la exercice */

  frame = gtk_frame_new ( NULL );
  gtk_container_set_border_width ( GTK_CONTAINER ( frame ),
				   10 );
  gtk_widget_set_sensitive ( frame,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( frame );

  /* la sélection d'une exercice dégrise la frame */

  gtk_signal_connect ( GTK_OBJECT ( clist_exercices_parametres ),
		       "select-row",
		       GTK_SIGNAL_FUNC ( selection_ligne_exercice ),
		       frame );
  gtk_signal_connect ( GTK_OBJECT ( clist_exercices_parametres ),
		       "unselect-row",
		       GTK_SIGNAL_FUNC ( deselection_ligne_exercice ),
		       frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 0 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox2 ),
				   5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );


  /* mise en forme du nom de l'exercice */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Name")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  nom_exercice = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       nom_exercice,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( nom_exercice );


/* mise en forme des dates de début et fin de l'exercice */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Start")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  debut_exercice = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       debut_exercice,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( debut_exercice );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( separateur );

  label = gtk_label_new ( COLON(_("End")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  fin_exercice = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       fin_exercice,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( fin_exercice );

  /* mise en place de l'affichage dans le formulaire */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  affichage_exercice = gtk_check_button_new_with_label ( _("Displayed in the form") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       affichage_exercice,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( affichage_exercice );



/* séparation */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       separateur,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( separateur );

/* ajout des bouton appliquer et annuler */

  hbox_boutons_modif_exercice = gtk_hbox_new ( FALSE,
			0 );
  gtk_widget_set_sensitive ( hbox_boutons_modif_exercice,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox_boutons_modif_exercice,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox_boutons_modif_exercice );


/*   on met ici toutes les connections qui, pour chaque changement, rendent sensitif les boutons */

  gtk_signal_connect ( GTK_OBJECT ( nom_exercice ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modif_detail_exercice ),
		       NULL);
  gtk_signal_connect ( GTK_OBJECT ( debut_exercice ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modif_detail_exercice ),
		       NULL);
  gtk_signal_connect ( GTK_OBJECT ( fin_exercice ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modif_detail_exercice ),
		       NULL);
  gtk_signal_connect ( GTK_OBJECT ( affichage_exercice ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( modif_detail_exercice ),
		       NULL);

  /* FIXME */
  bouton = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
/*   bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL ); */
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( annuler_modif_exercice ),
		       clist_exercices_parametres );
  gtk_box_pack_end ( GTK_BOX ( hbox_boutons_modif_exercice ),
		       bouton,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton );

  /* FIXME */
  bouton = gtk_button_new_from_stock (GTK_STOCK_APPLY);
/*   bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_APPLY ); */
  gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( applique_modif_exercice  ),
			      GTK_OBJECT ( clist_exercices_parametres ) );
  gtk_box_pack_end ( GTK_BOX ( hbox_boutons_modif_exercice ),
		       bouton,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton );


/* frame de droite-bas qui contient le choix de l'affichage des exercices */

  frame = gtk_frame_new ( _("Display of the financial year") );
  gtk_container_set_border_width ( GTK_CONTAINER ( frame ),
				   10 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 0 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox2 ),
				   5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );


  bouton_affichage_auto_exercice = gtk_radio_button_new_with_label ( NULL,
								     _("Automatic display according to the date") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_affichage_auto_exercice,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( bouton_affichage_auto_exercice );

  bouton = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON (bouton_affichage_auto_exercice)),
					     _("Automatic display of last selected financial year") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( bouton );

  if ( etat.affichage_exercice_automatique )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affichage_auto_exercice ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton ),
				   TRUE );

  gtk_signal_connect_object ( GTK_OBJECT ( bouton_affichage_auto_exercice ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));


  return ( vbox_pref );
}
/* ************************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction ajout_exercice */
/* appelée par le bouton ajouter de l'onglet exercices des paramètres */
/* ***************************************************************************************************** */

void ajout_exercice ( GtkWidget *bouton,
		    GtkWidget *clist )
{
  struct struct_exercice *exercice;
  gchar *ligne[1];
  gint ligne_insert;

/* enlève la sélection de la liste ( ce qui nettoie les entrées ) */

  gtk_clist_unselect_all ( GTK_CLIST ( clist ));


/* crée une nouvelle exercice au nom de "nouvel exercice" en mettant tous les paramètres à 0 et le no à -1 */

  exercice = calloc ( 1,
		    sizeof ( struct struct_exercice ));

  exercice -> no_exercice = -1;
  exercice -> nom_exercice = g_strdup ( _("New financial year") );

  liste_struct_exercices_tmp = g_slist_append ( liste_struct_exercices_tmp,
					      exercice );

  ligne[0] = exercice -> nom_exercice;

  ligne_insert = gtk_clist_append ( GTK_CLIST ( clist ),
				    ligne );

  /* on associe à la ligne la struct de la exercice */

  gtk_clist_set_row_data ( GTK_CLIST ( clist ),
			   ligne_insert,
			   exercice );

/* on sélectionne le nouveau venu */

  gtk_clist_select_row ( GTK_CLIST ( clist ),
			 ligne_insert,
			 0 );

/* on sélectionne le "nouvel exercice" et lui donne le focus */

  gtk_widget_set_sensitive ( hbox_boutons_modif_exercice,
			     TRUE );
  gtk_entry_select_region ( GTK_ENTRY ( nom_exercice ),
			    0,
			    -1 );
  gtk_widget_grab_focus ( nom_exercice );


}
/* ***************************************************************************************************** */


/* **************************************************************************************************************************** */
/* Fonction supprime_exercice */
/* appelée lorsqu'on clicke sur le bouton annuler dans les paramètres */
/* **************************************************************************************************************************** */

void supprime_exercice ( GtkWidget *bouton,
		       GtkWidget *liste )
{
  struct struct_exercice *exercice;
  GtkWidget *dialogue;
  GtkWidget *label;
  gint resultat;

  exercice = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_exercice );

  dialogue = gnome_dialog_new ( _("Confirm the deletion of this financial year"),
				GNOME_STOCK_BUTTON_YES,
				GNOME_STOCK_BUTTON_NO,
				NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				 GTK_WINDOW ( fenetre_preferences ));

  label = gtk_label_new ( g_strconcat ( _("Do you really want to delete the financial year:\n"),
					exercice -> nom_exercice,
					" ?",
					NULL ) );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialogue ));

  if ( !resultat )
    {
      gtk_clist_remove ( GTK_CLIST ( liste ),
			 ligne_selection_exercice );
      liste_struct_exercices_tmp = g_slist_remove ( liste_struct_exercices_tmp,
						  exercice );
      free ( exercice );

      gnome_property_box_changed ( GNOME_PROPERTY_BOX ( fenetre_preferences));
    }

}
/* **************************************************************************************************************************** */






/* **************************************************************************************************************************** */
/* Fonction selection_ligne_exercice */
/* appelée lorsqu'on sélectionne une exercice dans la liste */
/* **************************************************************************************************************************** */

void selection_ligne_exercice ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
			      GtkWidget *frame )
{
  struct struct_exercice *exercice;

  ligne_selection_exercice = ligne;

  exercice = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne );

/* remplit tous les champs */

  gtk_entry_set_text ( GTK_ENTRY ( nom_exercice ),
		       exercice -> nom_exercice );

  if ( exercice -> date_debut )
    gtk_entry_set_text ( GTK_ENTRY ( debut_exercice ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( exercice -> date_debut ),
					   g_date_month ( exercice -> date_debut ),
					   g_date_year ( exercice -> date_debut )));

  if ( exercice -> date_fin )
    gtk_entry_set_text ( GTK_ENTRY ( fin_exercice ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( exercice -> date_fin ),
					   g_date_month ( exercice -> date_fin ),
					   g_date_year ( exercice -> date_fin )));
					   

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( affichage_exercice ),
				 exercice -> affiche_dans_formulaire );

  gtk_widget_set_sensitive ( frame,
			     TRUE );
  gtk_widget_set_sensitive ( hbox_boutons_modif_exercice,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_supprimer_exercice,
			     TRUE );



}
/* **************************************************************************************************************************** */




/* **************************************************************************************************************************** */
/* Fonction deselection_ligne_exercice */
/* appelée lorsqu'on désélectionne une exercice dans la liste */
/* **************************************************************************************************************************** */

void deselection_ligne_exercice ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
			      GtkWidget *frame )
{
  struct struct_exercice *exercice;

  if ( GTK_WIDGET_SENSITIVE ( hbox_boutons_modif_exercice ) )
    {
      GtkWidget *dialogue;
      GtkWidget *label;
      gint resultat;

      exercice = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
					ligne );

      dialogue = gnome_dialog_new ( _("Confirmation of changes"),
				    GNOME_STOCK_BUTTON_YES,
				    GNOME_STOCK_BUTTON_NO,
				    NULL );
      gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				     GTK_WINDOW ( fenetre_preferences ));

      label = gtk_label_new ( g_strdup_printf ( _("The financial year %s had been modified.\n\nDo you want to save the changes?"),
						exercice -> nom_exercice ) );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialogue ));

      /*       si on veut les enregistrer , ok */
      /* si on ne veut pas et que c'est un nouvel exercice -> on l'efface */

      if ( !resultat )
	applique_modif_exercice ( liste );
      else
	{
	  if ( exercice -> no_exercice == -1 )
	    {
	      gtk_clist_remove ( GTK_CLIST ( liste ),
				 ligne );
	      liste_struct_exercices_tmp = g_slist_remove ( liste_struct_exercices_tmp,
							    exercice );
	      free ( exercice );
	    }
	}
    }

/* efface toutes les entrées */

  gtk_entry_set_text ( GTK_ENTRY ( nom_exercice ),
		       "" );
  gtk_entry_set_text ( GTK_ENTRY ( debut_exercice ),
		       "" );
  gtk_entry_set_text ( GTK_ENTRY ( fin_exercice ),
		       "" );

  gtk_widget_set_sensitive ( hbox_boutons_modif_exercice,
			     FALSE );
  gtk_widget_set_sensitive ( frame,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_supprimer_exercice,
			     FALSE );
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void modif_detail_exercice ( void )
{

  gtk_widget_set_sensitive ( hbox_boutons_modif_exercice,
			     TRUE );

}
/* **************************************************************************************************************************** */




/* ***************************************************************************************************** */
/* Fonction applique_modif_exercice */
/* appelée par le bouton appliquer qui devient sensitif lorsqu'on modifie la exercice des paramètres */
/* ***************************************************************************************************** */

void applique_modif_exercice ( GtkWidget *liste )
{
  struct struct_exercice *exercice;

  exercice = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_exercice );

  if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( nom_exercice )))))
    {
      dialogue ( _("The financial year must have a name.") );
      return;
    }

  if ( exercice -> no_exercice == -1 )
    {
      exercice -> no_exercice = ++no_derniere_exercice_tmp;
      nb_exercices_tmp++;
    }

  exercice -> nom_exercice = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( nom_exercice ))));

  if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( debut_exercice )))))
    exercice -> date_debut = NULL;
  else
    {
      if ( modifie_date ( debut_exercice ))
	{
	  gint jour, mois, an;

	  sscanf ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( debut_exercice ))),
		   "%d/%d/%d",
		   &jour,
		   &mois,
		   &an );

	  exercice->date_debut = g_date_new_dmy ( jour,
						      mois,
						      an );
	}
      else
	return;
    }

  if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( fin_exercice )))))
    exercice -> date_fin = NULL;
  else
    {
      if ( modifie_date ( fin_exercice ))
	{
	  gint jour, mois, an;

	  sscanf ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( fin_exercice ))),
		   "%d/%d/%d",
		   &jour,
		   &mois,
		   &an );

	  exercice->date_fin = g_date_new_dmy ( jour,
						mois,
						an );
	}
      else
	return;
    }

  exercice->affiche_dans_formulaire = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( affichage_exercice ));

  gtk_widget_set_sensitive ( hbox_boutons_modif_exercice,
			     FALSE );

/* met le nom dans la liste */

  gtk_clist_set_text ( GTK_CLIST ( liste ),
		       ligne_selection_exercice,
		       0,
		       exercice -> nom_exercice );

  gnome_property_box_changed ( GNOME_PROPERTY_BOX ( fenetre_preferences));
}
/* ***************************************************************************************************** */



/* **************************************************************************************************************************** */
/* Fonction annuler_modif_exercice */
/* appelée lorsqu'on clicke sur le bouton annuler dans les paramètres */
/* **************************************************************************************************************************** */

void annuler_modif_exercice ( GtkWidget *bouton,
			      GtkWidget *liste )
{
  struct struct_exercice *exercice;

  exercice = gtk_clist_get_row_data ( GTK_CLIST ( clist_exercices_parametres ),
				    ligne_selection_exercice );

/* si c'était une nouvelle exercice, on la supprime */

  if ( exercice -> no_exercice == -1 )
    {
      gtk_clist_unselect_all ( GTK_CLIST ( clist_exercices_parametres ));
      return;
      gtk_clist_remove ( GTK_CLIST ( clist_exercices_parametres ),
			 ligne_selection_exercice );
      liste_struct_exercices_tmp = g_slist_remove ( liste_struct_exercices_tmp,
						    exercice );
      free ( exercice );
      return;
    }

/* sinon, on remplit tous les champs avec l'ancienne exercice */

  gtk_entry_set_text ( GTK_ENTRY ( nom_exercice ),
		       exercice -> nom_exercice );

  if ( exercice -> date_debut )
    gtk_entry_set_text ( GTK_ENTRY ( debut_exercice ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( exercice -> date_debut ),
					   g_date_month ( exercice -> date_debut ),
					   g_date_year ( exercice -> date_debut )));

  if ( exercice -> date_fin )
    gtk_entry_set_text ( GTK_ENTRY ( fin_exercice ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( exercice -> date_fin ),
					   g_date_month ( exercice -> date_fin ),
					   g_date_year ( exercice -> date_fin )));
					   

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( affichage_exercice ),
				 exercice -> affiche_dans_formulaire );

  gtk_widget_set_sensitive ( hbox_boutons_modif_exercice,
			     FALSE );
}
/* **************************************************************************************************************************** */





/* ************************************************************************************************************ */
gint recherche_exercice_par_nom ( struct struct_exercice *exercice,
				gchar *nom )
{

  return ( g_strcasecmp ( g_strstrip ( exercice -> nom_exercice ),
			  nom ) );

}
/* ************************************************************************************************************ */



/***********************************************************************************************************/
/* Fonction recherche_exercice_par_no */
/* appelée par un g_slist_find_custom */
/***********************************************************************************************************/

gint recherche_exercice_par_no ( struct struct_exercice *exercice,
				 gint *no_exercice )
{

  return ( exercice -> no_exercice != GPOINTER_TO_INT ( no_exercice ));

}
/***********************************************************************************************************/



/* ************************************************************************************************************ */
/* Fonction creation_menu_exercices */
/* crée un menu qui contient les noms des exercices associés à leur no et adr */
/* et le renvoie */
/* origine = 0 si ça vient des opérations */
/* origine = 1 si ça vient de l'échéancier ; dans ce cas on rajoute automatique */
/* ************************************************************************************************************ */

GtkWidget *creation_menu_exercices ( gint origine )
{
  GtkWidget *menu;
  GtkWidget *menu_item;
  GSList *pointeur;

  menu = gtk_menu_new ();
  gtk_widget_show ( menu );


  /* si ça vient de l'échéancier, le 1er est automatique */
  /* on lui associe -2 */

  if ( origine )
    {
      menu_item = gtk_menu_item_new_with_label ( _("Automatic") );
      gtk_menu_append ( GTK_MENU ( menu ),
			menu_item );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "adr_exercice",
			    GINT_TO_POINTER (-2));
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "no_exercice",
			    GINT_TO_POINTER (-2));
      gtk_widget_show ( menu_item );
    }

  /* le premier nom est Aucun */
  /* on lui associe 0 */

  menu_item = gtk_menu_item_new_with_label ( _("None") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"adr_exercice",
			NULL );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_exercice",
			NULL );
  gtk_widget_show ( menu_item );


  /* le second est non affiché */
  /* on lui associe -1 */

  menu_item = gtk_menu_item_new_with_label ( _("Not displayed") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"adr_exercice",
			GINT_TO_POINTER (-1));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_exercice",
			GINT_TO_POINTER (-1));
  gtk_widget_show ( menu_item );


  pointeur = liste_struct_exercices;

  while ( pointeur )
    {
      struct struct_exercice *exercice;

      exercice = pointeur -> data;

      if ( exercice->affiche_dans_formulaire )
	{
	  menu_item = gtk_menu_item_new_with_label ( exercice -> nom_exercice );
	  gtk_menu_append ( GTK_MENU ( menu ),
			    menu_item );
	  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				"adr_exercice",
				exercice );
	  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				"no_exercice",
				GINT_TO_POINTER ( exercice -> no_exercice ));
	  gtk_widget_show ( menu_item );
	}

      pointeur = pointeur -> next;
    }

  return ( menu );
}
/* ************************************************************************************************************ */


  
/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_exercice */
/*   argument : le numéro de l'exercice demandé */
/* renvoie la place demandée dans l'option menu du formulaire */
/* pour mettre l'history */
/* ************************************************************************************************************** */

gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu )
{
  GList *liste_tmp;
  gint trouve;
  gint non_affiche;
  gint i;

  liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( option_menu ) -> menu ) -> children;
  i= 0;
  non_affiche = 0;

  while ( liste_tmp )
    {

      trouve = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
						       "no_exercice" ));

      /*       si trouve = no demandé, c'est bon, on se barre */

      if ( trouve == no_demande )
	return ( i );

      /*  si on est sur la position du non affiché, on le sauve */

      if ( trouve == -1 )
	non_affiche = i;

      i++;
      liste_tmp = liste_tmp -> next;
    }

  /*   l'exo n'est pas affiché, on retourne la position de non affiché */

  return ( non_affiche );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* fonction affiche_exercice_par_date */
/* met automatiquement l'option menu de l'exercice par rapport */
/* à la date dans le formulaire */
/* ************************************************************************************************************** */

void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice )
{
  GDate *date_courante;
  gint jour, mois, an;
  GSList *liste_tmp;
  gint trouve;
  
  if ( !etat.affichage_exercice_automatique )
    return;


  sscanf ( gtk_entry_get_text ( GTK_ENTRY ( entree_date )),
	   "%d/%d/%d",
	   &jour,
	   &mois,
	   &an );

  if ( !g_date_valid_dmy ( jour,
			   mois,
			   an ))
    return;

  date_courante = g_date_new_dmy ( jour,
				   mois,
				   an );

  /* on fait le tour des exercices pour trouver le premier qui inclut la date */

  liste_tmp = liste_struct_exercices;
  trouve = 0;
;
  while ( liste_tmp )
    {
      struct struct_exercice *exercice;

      exercice = liste_tmp -> data;

      if ( exercice->date_debut
	   &&
	   exercice->date_fin
	   &&
	   exercice->affiche_dans_formulaire
	   &&
	   (g_date_compare ( date_courante,
			     exercice->date_debut ) >= 0 )
	   &&
	   (g_date_compare ( date_courante,
			     exercice->date_fin ) <= 0 ))
	{
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_exercice ),
					cherche_no_menu_exercice ( exercice->no_exercice,
								   option_menu_exercice ));
	  trouve = 1;
	}

      liste_tmp = liste_tmp -> next;
    }

  /*       si rien trouvé, met à aucun */

  if ( !trouve )
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_exercice ),
				  0 );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* fonction association_automatique */
/* recherche les opés sans exercice, et les associe si possible avec un des exercice créé */
/* ************************************************************************************************************** */

void association_automatique ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  gint resultat;
  gint i;

  dialog = gnome_dialog_new ( _("Automatic association for the financial year"),
			      GNOME_STOCK_BUTTON_APPLY,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( fenetre_preferences ));

  label = gtk_label_new ( _("This function assigns each transaction without a financial year to the\none related to its transaction date.  If no financial year matches,\nthe transaction will not be changed.") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    return;
  
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GSList *pointeur_tmp;

      pointeur_tmp = LISTE_OPERATIONS;

      while ( pointeur_tmp )
	{
	  struct structure_operation *operation;

	  operation = pointeur_tmp -> data;

	  if ( !operation -> no_exercice )
	    {
	      GSList *pointeur_exo;

	      pointeur_exo = liste_struct_exercices;

	      while ( pointeur_exo )
		{
		  struct struct_exercice *exo;

		  exo = pointeur_exo -> data;

		  if ( g_date_compare ( exo -> date_debut,
					operation -> date ) <= 0
		       &&
		       g_date_compare ( exo -> date_fin,
					operation -> date ) >= 0 )
		    operation -> no_exercice = exo -> no_exercice;

		  pointeur_exo = pointeur_exo -> next;
		}
	    }
	  pointeur_tmp = pointeur_tmp -> next;
	}
      p_tab_nom_de_compte_variable++;
    }

  demande_mise_a_jour_tous_comptes ();
  verification_mise_a_jour_liste();

  modification_fichier ( TRUE );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* fonction recherche_exo_correspondant */
/* renvoie l'exercice correspondant la date donnée en argument */
/* si aucun ne correspond, on renvoie 0 */
/* ************************************************************************************************************** */

gint recherche_exo_correspondant ( GDate *date )
{
  GSList *liste_tmp;

  liste_tmp = liste_struct_exercices;

  while ( liste_tmp )
    {
      struct struct_exercice *exo;

      exo = liste_tmp -> data;

      if ( g_date_compare ( exo -> date_debut,
			    date ) <= 0
	   &&
	   g_date_compare ( exo -> date_fin,
			    date ) >= 0 )
	return ( exo -> no_exercice );

      liste_tmp = liste_tmp -> next;
    }

/*   on n'a pas trouvé l'exo, on retourne 0 */

  return ( 0 );

}
/* ************************************************************************************************************** */
