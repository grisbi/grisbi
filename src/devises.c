/* Fichier devises.c */
/* s'occupe de tout ce qui concerne les devises */


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



/* ***************************************************************************************************** */
/* Fonction creation_devises_de_base */
/* appelé lors de l'ouverture d'un nouveau fichier ou lors */
/* de l'ouverture de la version 0.2.5 */
/* met en mémoire les devises de base : l'euro et le franc */
/* ***************************************************************************************************** */

void creation_devises_de_base ( void )
{
  struct struct_devise *devise;


  liste_struct_devises = NULL;

  /* création de l'euro */

  devise = malloc ( sizeof ( struct struct_devise ));

  devise -> no_devise = 1;
  devise -> nom_devise= g_strdup ( _("Euro") );
  devise -> code_iso4217_devise = g_strdup ( _("EUR") );
  devise -> code_devise = g_strdup ( _("â‚¬") );
  devise -> passage_euro = 0;
  devise -> date_dernier_change = NULL;
  devise -> une_devise_1_egale_x_devise_2 = 0;
  devise -> no_devise_en_rapport = 0;
  devise -> change = 0;

  liste_struct_devises = g_slist_append ( liste_struct_devises,
					  devise );

  nb_devises = 1;
  no_derniere_devise = 1;

}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* Fonction creation_option_menu_devises */
/* retourne le menu d'un option menu avec les différentes devises définies */
/* si devise_cachee = -1, met toutes les devises sous forme de leur sigle */
/* sinon, ne met pas la devise correspondant à devise_cachee ( 0 = aucune ) */
/* liste_tmp est la liste utilisée : soit liste_struct_devises dans le cas général, */
/*                       soit liste_struct_devises_tmp dans le cas des paramètres */
/* **************************************************************************************************** */

GtkWidget *creation_option_menu_devises ( gint devise_cachee,
					  GSList *liste_tmp )
{
  GtkWidget *menu;
  GtkWidget *menu_item;


  menu = gtk_menu_new ();

  if ( devise_cachee > 0 )
    {

      /* le 1er item c'est aucune devise */

      menu_item = gtk_menu_item_new_with_label ( _("None") );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "adr_devise",
			    devise_nulle );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "no_devise",
			    GINT_TO_POINTER ( 0 ) );

      gtk_menu_append ( GTK_MENU ( menu ),
			menu_item );

      gtk_widget_show ( menu_item );
    }


  while ( liste_tmp )
    {
      struct struct_devise *devise;

      devise = liste_tmp -> data;

      if ( devise_cachee != devise -> no_devise )
	{
	  if ( devise_cachee == -1 )
	    menu_item = gtk_menu_item_new_with_label ( devise_name ( devise ) );
	  else
	    menu_item = gtk_menu_item_new_with_label ( g_strconcat ( devise -> nom_devise,
								     " ( ",
								     devise_name ( devise ),
								     " )",
								     NULL ));

	  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				"adr_devise",
				devise );
	  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				"no_devise",
				GINT_TO_POINTER ( devise -> no_devise ) );


	  gtk_menu_append ( GTK_MENU ( menu ),
			    menu_item );

	  gtk_widget_show ( menu_item );
	}
      liste_tmp = liste_tmp -> next;
    }

  gtk_widget_show ( menu );

  return ( menu );

}
/* **************************************************************************************************** */



/***********************************************************************************************************/
/* Fonction ajout_devise */
/* appelée pour créer une nouvelle devise */
/* le widget est soit un option menu si l'appel vient du chargement de la version 0.2.4, dans ce cas on utilise liste_struct_devises */
/* soit c'est la clist des paramètres, dans ce cas on utilise liste_struct_devises_tmp */
/***********************************************************************************************************/

void ajout_devise ( GtkWidget *bouton,
		    GtkWidget *widget )
{
  GtkWidget *dialog;
  GtkWidget *label;
  gint resultat;
  GtkWidget *hbox;
  GtkWidget *entree_nom;
  GtkWidget *entree_code, *entree_iso_code;
  GtkWidget *check_bouton;
  GtkWidget*hbox_passage_euro;
  GtkWidget *entree_conversion_euro;
  struct struct_devise *devise;
  gchar *nom_devise;
  gchar *code_devise, *code_iso4217_devise;
  GtkWidget *label_nom_devise;

  dialog = gnome_dialog_new ( _("Add a currency"),
				GNOME_STOCK_BUTTON_OK,
				GNOME_STOCK_BUTTON_CANCEL,
				NULL );
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
		        "destroy" );
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "key-press-event",
		       GTK_SIGNAL_FUNC ( bloque_echap_choix_devise ),
		       NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			    0 );

  label = gtk_label_new ( COLON(_("New currency")) );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );


  label = gtk_label_new ( COLON(_("Currency name")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  entree_nom = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_nom ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_nom,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( entree_nom );


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Currency code")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  entree_code = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_code ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_code,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( entree_code );


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Currency ISO 4217 code")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  entree_iso_code = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_iso_code ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_iso_code,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( entree_iso_code );

  check_bouton = gtk_check_button_new_with_label ( _("This currency will switch to the euro") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       check_bouton,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( check_bouton );

  hbox_passage_euro = gtk_hbox_new ( FALSE,
				     5  );
  gtk_signal_connect ( GTK_OBJECT ( check_bouton ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( passe_a_l_euro ),
		       hbox_passage_euro );
  gtk_widget_set_sensitive ( hbox_passage_euro,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox_passage_euro,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox_passage_euro );
 
  label = gtk_label_new ( POSTSPACIFY(_("1 Euro equals")) );
  gtk_box_pack_start ( GTK_BOX ( hbox_passage_euro ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  entree_conversion_euro = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_conversion_euro ));
  gtk_box_pack_start ( GTK_BOX ( hbox_passage_euro ),
		       entree_conversion_euro,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( entree_conversion_euro );


  /* mise en place du label_nom_devise qui contiendra le nom de la devise dès que l'entrée nom_devise perd le focus */

  label_nom_devise = gtk_label_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox_passage_euro ),
		       label_nom_devise,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label_nom_devise );


  gtk_signal_connect ( GTK_OBJECT ( entree_nom ),
		       "focus-out-event",
		       GTK_SIGNAL_FUNC ( nom_nouvelle_devise_defini ),
		       label_nom_devise );

  gtk_widget_grab_focus ( entree_nom );
  gtk_editable_set_position ( GTK_EDITABLE ( entree_nom ),
			   0 );
 reprise_dialog:

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));


  switch ( resultat )
    {
    case 0 :

      nom_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom ))));
      code_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_code ))));
      code_iso4217_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code ))));
 
      if ( strlen ( nom_devise ) && strlen ( code_devise ) )
	{
	  GtkWidget *menu;
	  GtkWidget *item;


	  devise = malloc ( sizeof ( struct struct_devise ));
	  devise -> nom_devise = nom_devise;
	  devise -> code_devise = code_devise;
	  devise -> code_iso4217_devise = code_iso4217_devise;
	  devise -> passage_euro = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_bouton ));
	  devise -> no_devise_en_rapport = 0;
	  devise -> date_dernier_change = NULL;
	  devise -> une_devise_1_egale_x_devise_2 = 0;

	  if ( devise -> passage_euro )
	    devise -> change = g_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion_euro ))),
					  NULL );
	  else
	    devise -> change = 0;


	  /* 	  si le widget est une clist, c'est que l'appel vient du menu de configuration, */
	  /* on met la liste à jour et on ajoute la devise à liste_struct_devises_tmp */
	  /* 	    sinon, c'est que c'est le chargement de la version 0.2.5, on met l'option menu et liste_struct_devises à jour */
	  /* à retirer à partir de la 0.2.7 */

	  if ( GTK_IS_CLIST ( widget ))
	    {
	      gchar *ligne[3];
	      gint ligne_liste;

	      devise -> no_devise = ++no_derniere_devise_tmp;
	      liste_struct_devises_tmp = g_slist_append ( liste_struct_devises_tmp,
							  devise );
	      nb_devises_tmp++;

	      ligne[0] = devise -> nom_devise;
	      ligne[1] = devise -> code_iso4217_devise;
	      ligne[2] = devise -> code_devise;
	      
	      ligne_liste = gtk_clist_append ( GTK_CLIST ( widget ),
					       ligne );
	      gtk_clist_set_row_data  ( GTK_CLIST ( widget ),
					ligne_liste,
					devise );

	      /* dégrise appliquer dans paramètres */

	      activer_bouton_appliquer ( );

	    }
	  else
	    {
	      devise -> no_devise = ++no_derniere_devise;
	      liste_struct_devises = g_slist_append ( liste_struct_devises,
						      devise );
	      nb_devises++;

	      menu = gtk_option_menu_get_menu ( GTK_OPTION_MENU ( widget ));
	      
	      item = gtk_menu_item_new_with_label ( g_strconcat ( devise -> nom_devise,
								  " ( ",
								  devise -> code_devise,
								  " )",
								  NULL ));
	      gtk_object_set_data ( GTK_OBJECT ( item ),
				    "adr_devise",
				    devise );
	      gtk_object_set_data ( GTK_OBJECT ( item ),
				    "no_devise",
				    GINT_TO_POINTER ( devise -> no_devise ) );
	      gtk_menu_append ( GTK_MENU ( menu ),
				item );
	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
					 menu );

	      gtk_widget_show (item );
	    }

	  modification_fichier ( TRUE );
	}
      else
	{
	  dialogue ( _("All fields are not filled in!") );
	  goto reprise_dialog;
	}
      break;

    }
  gnome_dialog_close ( GNOME_DIALOG ( dialog ));
}
/***********************************************************************************************************/



/***********************************************************************************************************/
gint bloque_echap_choix_devise ( GtkWidget *dialog,
				 GdkEventKey *key,
				 gpointer null )
{

  /* empèche la touche echap de fermer la fenetre */

  if ( key -> keyval == 65307 )
    {
      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( dialog ),
				     "key-press-event" );
      return ( TRUE );
    }

  return ( FALSE );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction nom_nouvelle_devise_defini */
/* appelée lorsque l'utilisateur a défini le nouveau nom de la devise */
/* affiche le nom de la devise derrière l'entrée pour la conversion en euro */
/***********************************************************************************************************/

gboolean nom_nouvelle_devise_defini ( GtkWidget *entree,
				  GdkEventFocus *ev,
				  GtkWidget *label )
{
  gchar *nom_devise;

  if ( strlen ( nom_devise = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
    gtk_label_set_text ( GTK_LABEL ( label ),
			 nom_devise );

  return FALSE;
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* Fonction retrait_devise */
/***********************************************************************************************************/

void retrait_devise ( GtkWidget *bouton,
		      GtkWidget *liste )
{
  gint devise_trouvee;
  gint i;
  struct struct_devise *devise;

  if ( ligne_selection_devise == -1 )
    return;


  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );

  /*   recherche dans tous les comptes, les opés et les échéances si la devise n'est pas utilisée */
  /* si elle l'est, empêche sa suppression */

  devise_trouvee = 0;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      if ( DEVISE == devise -> no_devise )
	{
	  devise_trouvee = 1;
	  i = nb_comptes;
	}
      else
	{
	  GSList *liste_tmp;

	  liste_tmp = LISTE_OPERATIONS;

	  while ( liste_tmp )
	    {
	      if ( ((struct structure_operation *)(liste_tmp -> data )) -> devise == devise -> no_devise )
		{
		  devise_trouvee = 1;
		  i = nb_comptes;
		  liste_tmp = NULL;
		}
	      else
		liste_tmp = liste_tmp -> next;
	    }

	  if ( !devise )
	    {
	      liste_tmp = gsliste_echeances;

	      while ( liste_tmp )
		{
		  if ( ((struct operation_echeance *)(liste_tmp -> data )) -> devise == devise -> no_devise )
		    {
		      devise_trouvee = 1;
		      i = nb_comptes;
		      liste_tmp = NULL;
		    }
		  else
		    liste_tmp = liste_tmp -> next;
		}
	    }
	}
      p_tab_nom_de_compte_variable++;
    }


  /* si c'est l'euro qu'on veut supprimer, on interdit */

  if ( !strcmp ( devise -> nom_devise,
		 _("Euro") ))
    devise_trouvee = 1;



  if ( devise_trouvee )
    {
      dialogue ( g_strdup_printf ( _("The %s currency is used in the current account.\nYou can't delete it."),
				   devise -> nom_devise ) ) ;
      return;
    }


  gtk_clist_remove ( GTK_CLIST ( liste ),
		     ligne_selection_devise );

  liste_struct_devises_tmp = g_slist_remove ( liste_struct_devises_tmp,
					      devise );
  nb_devises_tmp--;

  activer_bouton_appliquer ( );

}
/***********************************************************************************************************/




/* ************************************************************************************************************ */
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom )
{

  return ( g_strcasecmp ( g_strstrip ( devise -> nom_devise ),
			  nom ) );

}
/* ************************************************************************************************************ */


/***********************************************************************************************************/
/* Fonction recherche_devise_par_no */
/* appelée par un g_slist_find_custom */
/***********************************************************************************************************/

gint recherche_devise_par_no ( struct struct_devise *devise,
			       gint *no_devise )
{

  return ( devise -> no_devise != GPOINTER_TO_INT ( no_devise ));

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction selection_devise */
/* permet de choisir une devise pour un compte */
/* utilisée quand la devise d'un compte est supprimée, pour le réaffecter */
/* et pour l'importation d'un fichier qif */
/***********************************************************************************************************/

gint selection_devise ( gchar *nom_du_compte )
{
  GtkWidget *dialogue;
  GtkWidget *label;
  GtkWidget *option_menu;
  GtkWidget *bouton;
  GtkWidget *hbox;
  gint resultat;


  dialogue = gnome_dialog_new ( _("Select a currency"),
				GNOME_STOCK_BUTTON_OK,
				NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				 GTK_WINDOW ( window ) );
  gtk_signal_connect ( GTK_OBJECT ( dialogue ),
		       "delete_event",
		       GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( dialogue ),
		       "key-press-event",
		       GTK_SIGNAL_FUNC ( bloque_echap_choix_devise ),
		       NULL );

  label = gtk_label_new ( g_strdup_printf ( _("Please choose a currency for account \"%s\":\n"),
					    nom_du_compte ) );
  
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       label,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  /* met la liste des devises sans "Aucunes" */

  option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ) );

  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       option_menu,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( option_menu );

  bouton = gtk_button_new_with_label ( _("Add a currency") );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( ajout_devise ),
		       option_menu );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  

  gnome_dialog_run ( GNOME_DIALOG ( dialogue ));

  resultat = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
						     "no_devise" ) );
  gnome_dialog_close ( GNOME_DIALOG ( dialogue ));

  return ( resultat );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
void passe_a_l_euro ( GtkWidget *toggle_bouton,
		      GtkWidget *hbox )
{

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( toggle_bouton )))
    gtk_widget_set_sensitive ( hbox,
			       TRUE );
  else
    gtk_widget_set_sensitive ( hbox,
			       FALSE );



}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction demande_taux_de_change : */
/* affiche une fenetre permettant d'entrer le taux de change entre la devise du compte et la devise demandée */
/* renvoie ce taux de change */
/* le taux renvoyé est <0 si une_devise_compte_egale_x_devise_ope = 1, > 0 sinon */
/***********************************************************************************************************/

void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise ,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *entree;
  GtkWidget *menu;
  GtkWidget *item;
  GtkWidget *hbox;
  gint resultat;
  GtkWidget *entree_frais;
  

  dialog = gnome_dialog_new ( _("Entry of the exchange rate"),
			      GNOME_STOCK_BUTTON_OK,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gtk_signal_connect ( GTK_OBJECT ( dialog),
		       "delete_event",
		       GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
		       NULL );




  label = gtk_label_new ( COLON(_("Please enter the exchange rate")) );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       20 );
  gtk_widget_show ( label );

  /* création de la ligne du change */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox );

  label = gtk_label_new ( POSTSPACIFY(_("A")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  option_menu_devise_1= gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       option_menu_devise_1,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( option_menu_devise_1 );

  label = gtk_label_new ( SPACIFY(_("equals")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  entree = gtk_entry_new ();
  gtk_widget_set_usize ( entree,
			 100,
			 FALSE );
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( entree );

  option_menu_devise_2 = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       option_menu_devise_2,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( option_menu_devise_2 );

      /* création du menu de la 1ère devise ( le menu comporte la devise courante et celle associée ) */

  menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ( devise -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise_compte );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_1 ),
			     menu );
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_1 )-> menu ),
		       "selection_done",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       NULL );
  gtk_widget_show ( menu );
	  


      /* création du menu de la 2ème devise ( le menu comporte la devise courante et celle associée ) */

  menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ( devise -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise_compte );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_2 ),
			     menu );
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_2 ) -> menu ),
		       "selection_done",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       GINT_TO_POINTER ( 1 ));
  gtk_widget_show ( menu );
 




  /* création de la ligne des frais de change */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Exchange fees")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );


  entree_frais = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_frais ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_frais,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( entree_frais );

  label = gtk_label_new ( devise_compte -> nom_devise );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );


      /* choix des 1ère et 2ème devise */

  if ( taux_change
       ||
       frais_change )
    {

      if ( une_devise_compte_egale_x_devise_ope )
	{
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					1 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					0 );
	}
      else
	{
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					0 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					1 );
	}
      gtk_entry_set_text ( GTK_ENTRY ( entree ),
			   g_strdup_printf ( "%f",
					     taux_change ));
      gtk_entry_set_text ( GTK_ENTRY ( entree_frais ),
			   g_strdup_printf ( "%4.2f",
					     fabs ( frais_change )));

    }
  else
    {
      /* vérifie s'il y a déjà une association entre la devise du compte et la devise de l'opération */

      if ( devise_compte -> no_devise_en_rapport == devise -> no_devise )
	{
	  /* il y a une association de la devise du compte vers la devise de l'opération */

	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					devise_compte -> une_devise_1_egale_x_devise_2 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					!( devise_compte -> une_devise_1_egale_x_devise_2 ));

	  /*       si un change est déjà entré, on l'affiche */
	  
	  if ( devise_compte -> date_dernier_change )
	    gtk_entry_set_text ( GTK_ENTRY ( entree ),
				 g_strdup_printf ( "%f",
						   devise_compte -> change ));
	}
      else
	if ( devise -> no_devise_en_rapport == devise_compte -> no_devise )
	  {
	    /* il y a une association de la devise de l'opération vers la devise du compte */

	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					  !(devise -> une_devise_1_egale_x_devise_2 ));
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					  devise -> une_devise_1_egale_x_devise_2 );

	    /*       si un change est déjà entré, on l'affiche */
	  
	    if ( devise -> date_dernier_change )
	      gtk_entry_set_text ( GTK_ENTRY ( entree ),
				   g_strdup_printf ( "%f",
						     devise -> change ));
	  }
      else
	{
	  /* il n'y a aucun rapport établi entre les 2 devises */

	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					1 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					0 );
	}
    }



/* on lance la fenetre */

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( !resultat )
    {
      struct struct_devise *devise_tmp;

      taux_de_change[0] = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )),
				  NULL );
      taux_de_change[1] = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_frais )),
				  NULL );

      devise_tmp = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_1 ) -> menu_item ),
					 "adr_devise" );


      if ( devise_tmp -> no_devise != devise -> no_devise )
	taux_de_change[0] = -taux_de_change[0];

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
    }
  else
    {
      taux_de_change[0] = 0;
      taux_de_change[1] = 0;
    }
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Fonction devise_selectionnee */
/* appelée lorsqu'on selectionne une des 2 devises de la fenetre de demande du taux de change */
/* change automatiquement le 2ème option_menu */
/* entrée : origine = 0 : c'est le 1er option menu qui a été changé */
/*              origine = 1 sinon */
/***********************************************************************************************************/

gboolean devise_selectionnee ( GtkWidget *menu_shell,
			       gint origine )
{
  gint position;

  if ( origine )
    {
      /* le 2ème option menu a été changé */

      position = g_list_index ( GTK_MENU_SHELL ( menu_shell ) -> children,
				GTK_OPTION_MENU ( option_menu_devise_2 ) -> menu_item );

      gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
				    1 - position );
    }
  else
    {
      /* le 1er option menu a été changé */

      position = g_list_index ( GTK_MENU_SHELL ( menu_shell ) -> children,
				GTK_OPTION_MENU ( option_menu_devise_1 ) -> menu_item );

      gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
				    1 - position );
    }

  return FALSE;
}
/***********************************************************************************************************/





/**
 * Creates the currency list and associated form to configure them.
 *
 * \returns A newly created vbox
 */
GtkWidget *onglet_devises ( void )
{
  GtkWidget *hbox_pref, *vbox_pref, *separateur, *label, *frame, *paddingbox;
  GtkWidget *scrolled_window, *vbox;
  GSList *liste_tmp;
  gchar *titres_devise [3] = { _("Currency"),
			       _("Code ISO"),
			       _("Code") };
  GtkWidget *bouton;
  GtkWidget *hbox;

  vbox_pref = new_vbox_with_title_and_icon ( _("Currencies"),
					     "pixmaps/currencies.png" );

  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( hbox ), 10 );
  gtk_box_pack_start ( GTK_BOX ( vbox_pref ), hbox,
		       FALSE, FALSE, 0);
  gtk_widget_show ( hbox );

  /* Currency list */
  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
		       FALSE, FALSE, 0);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize ( GTK_WIDGET ( scrolled_window ),
			 FALSE, 120 );
  gtk_widget_show ( scrolled_window );

  clist_devises_parametres = gtk_clist_new_with_titles ( 3, titres_devise );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_devises_parametres ) ,
				      0, TRUE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_devises_parametres ) ,
				      1, TRUE );
  gtk_clist_column_titles_passive ( GTK_CLIST ( clist_devises_parametres ));
  gtk_clist_set_column_justification ( GTK_CLIST ( clist_devises_parametres ),
				       1, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( clist_devises_parametres ),
				       2, GTK_JUSTIFY_RIGHT);
  /* FIXME ? */
  gtk_signal_connect_object  ( GTK_OBJECT ( fenetre_preferences ),
			       "apply",
			       GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
			       GTK_OBJECT ( clist_devises_parametres ));
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      clist_devises_parametres );
  gtk_widget_show ( clist_devises_parametres );


  /*   s'il n'y a pas de fichier ouvert, on grise */
  if ( !nb_comptes )
    gtk_widget_set_sensitive ( vbox, FALSE );
  else
    {
      /* on crée la liste_struct_devises_tmp qui est un copie de liste_struct_devises originale */
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


      /* remplissage de la liste avec les devises temporaires */

      liste_tmp = liste_struct_devises_tmp;

      while ( liste_tmp )
	{
	  struct struct_devise *devise;
	  gchar *ligne[3];
	  gint ligne_insert;

	  devise = liste_tmp -> data;

	  ligne[0] = devise -> nom_devise;
	  ligne[1] = devise -> code_iso4217_devise;
	  ligne[2] = devise -> code_devise;

	  ligne_insert = gtk_clist_append ( GTK_CLIST ( clist_devises_parametres ),
					    ligne );

	  /* on associe à la ligne la struct de la devise */

	  gtk_clist_set_row_data ( GTK_CLIST ( clist_devises_parametres ),
				   ligne_insert,
				   devise );

	  liste_tmp = liste_tmp -> next;
	}

    }

  /* Create Add/Remove buttons */
  vbox = gtk_vbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( vbox );

  /* Button "Add" */
  bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC  ( ajout_devise ),
		       clist_devises_parametres );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
		       FALSE, FALSE, 5 );
  gtk_widget_show ( bouton );

  /* Button "Remove" */
  bouton_supprimer_devise = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_devise ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_supprimer_devise, FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_devise ),
		       "clicked",
		       GTK_SIGNAL_FUNC  ( retrait_devise ),
		       clist_devises_parametres );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_devise,
		       FALSE, FALSE, 5 );
  gtk_widget_show ( bouton_supprimer_devise );


  /* Input form for currencies */
  paddingbox = paddingbox_new_with_title (vbox_pref, 
					  _("Currency properties"));

  /* Selecting a currency activates this form */
  gtk_signal_connect ( GTK_OBJECT ( clist_devises_parametres ),
		       "select-row",
		       GTK_SIGNAL_FUNC ( selection_ligne_devise ),
		       frame );
  gtk_signal_connect ( GTK_OBJECT ( clist_devises_parametres ),
		       "unselect-row",
		       GTK_SIGNAL_FUNC ( deselection_ligne_devise ),
		       frame );

  /* Create currency name entry */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);
  label = gtk_label_new ( COLON(_("Name")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );
  entree_nom_devise_parametres = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_nom_devise_parametres ),
		       "changed",
		       GTK_SIGNAL_FUNC ( changement_nom_entree_devise ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( entree_nom_devise_parametres ),
		       "changed",
		       activer_bouton_appliquer,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_nom_devise_parametres,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_nom_devise_parametres );

  /* Create code entry */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);
  label = gtk_label_new ( COLON(_("Code")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );
  entree_code_devise_parametres = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_code_devise_parametres ),
		       "changed",
		       GTK_SIGNAL_FUNC ( changement_code_entree_devise ),
		       NULL );
  gtk_signal_connect  ( GTK_OBJECT ( entree_code_devise_parametres ),
			"changed",
			activer_bouton_appliquer,
			NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_code_devise_parametres,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_code_devise_parametres );

  /* Create code entry */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);
  label = gtk_label_new ( COLON(_("ISO code")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );
  entree_iso_code_devise_parametres = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_iso_code_devise_parametres ),
		       "changed",
		       GTK_SIGNAL_FUNC ( changement_iso_code_entree_devise ),
		       NULL );
  gtk_signal_connect  ( GTK_OBJECT ( entree_iso_code_devise_parametres ),
			"changed",
			activer_bouton_appliquer,
			NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_iso_code_devise_parametres,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_iso_code_devise_parametres );

  /* Will switch to Euro? */
  check_button_euro = gtk_check_button_new_with_label ( _("Will switch to Euro") );
  gtk_signal_connect ( GTK_OBJECT ( check_button_euro ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( change_passera_euro ),
		       clist_devises_parametres );
  gtk_signal_connect ( GTK_OBJECT ( check_button_euro ),
		       "toggled",
		       activer_bouton_appliquer,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), check_button_euro,
		       FALSE, FALSE, 0);
  gtk_widget_show ( check_button_euro );


  /* Création de la ligne devise associée */
  hbox_devise_associee = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox_devise_associee,
		       FALSE, FALSE, 0);
  gtk_widget_show ( hbox_devise_associee );

  label = gtk_label_new ( COLON(_("Associated currency")) );
  gtk_box_pack_start ( GTK_BOX ( hbox_devise_associee ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );

  option_menu_devises = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_devise_associee ), option_menu_devises,
		       FALSE, FALSE, 0);
  gtk_widget_show ( option_menu_devises );


  /* Création de la ligne de change entre les devises */
  label_date_dernier_change = gtk_label_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), label_date_dernier_change,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label_date_dernier_change );

  /* Création de la ligne du change */
  hbox_ligne_change = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox_ligne_change,
		       FALSE, FALSE, 0);
  gtk_widget_show ( hbox_ligne_change );

  label = gtk_label_new ( POSTSPACIFY(_("One")) );
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );

  devise_1 = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), devise_1,
		       FALSE, FALSE, 0);
  gtk_widget_show ( devise_1 );

  label = gtk_label_new ( SPACIFY(_("costs")) );
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );

  entree_conversion = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_conversion ),
		       "changed",
		       activer_bouton_appliquer,
		       NULL );
  gtk_widget_set_usize ( entree_conversion,
			 100,
			 FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), entree_conversion,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_conversion );

  devise_2 = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), devise_2,
		       FALSE, FALSE, 0);
  gtk_widget_show ( devise_2 );

  return ( vbox_pref );

}
/* ************************************************************************************************************** */




/* **************************************************************************************************************************** */
/* Fonction selection_ligne_devise */
/* appelée lorsqu'on sélectionne une devise dans la liste */
/* **************************************************************************************************************************** */

gboolean selection_ligne_devise ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev,
				  GtkWidget *frame )
{
  struct struct_devise *devise;

  ligne_selection_devise = ligne;
  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );


  /* met le nom et le code de la devise */

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				     changement_nom_entree_devise,
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_devise_parametres ),
		       devise -> nom_devise );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				       gnome_property_box_changed,
				       fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				       changement_nom_entree_devise,
				       NULL );

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				     gnome_property_box_changed,
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				     changement_code_entree_devise,
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_code_devise_parametres ),
		       devise -> code_devise );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				       gnome_property_box_changed,
				       fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				       changement_code_entree_devise,
				       NULL );


  /* crée le menu des devises en enlevant la devise courante */

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devises ),
			     creation_option_menu_devises ( devise -> no_devise,
							    liste_struct_devises_tmp ));
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu ),
		       "selection-done",
		       GTK_SIGNAL_FUNC ( changement_devise_associee ),
		       liste );
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises) -> menu ),
		       "selection-done",
		       activer_bouton_appliquer,
		       NULL );
  /* FIXME ? */
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( check_button_euro ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( check_button_euro ),
				 devise -> passage_euro );

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( check_button_euro ),
				       GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				       fenetre_preferences );
 
  change_passera_euro ( check_button_euro,
			liste );

  gtk_widget_set_sensitive ( frame,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_supprimer_devise,
			     TRUE );

  if ( !strcmp ( devise -> nom_devise,
		 _("Euro") ) )
    {
      gtk_widget_set_sensitive ( check_button_euro,
				 FALSE );
      gtk_widget_set_sensitive ( entree_nom_devise_parametres,
				 FALSE );
    }
  else
    gtk_widget_set_sensitive ( entree_nom_devise_parametres,
			       TRUE );

  return FALSE;
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
/* Fonction deselection_ligne_devise */
/* appelée lorsqu'on désélectionne une devise dans la liste */
/* **************************************************************************************************************************** */

gboolean deselection_ligne_devise ( GtkWidget *liste,
				    gint ligne,
				    gint colonne,
				    GdkEventButton *ev,
				    GtkWidget *frame )
{
  struct struct_devise *devise;

  ligne_selection_devise = -1;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne );

  /* retire le nom et le code de la devise */
  
  /* FIXME ? */
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				     gnome_property_box_changed,
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				     changement_nom_entree_devise,
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_devise_parametres ),
		       "" );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				       gnome_property_box_changed,
				       fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				       changement_nom_entree_devise,
				       NULL );

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				     gnome_property_box_changed,
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				     changement_code_entree_devise,
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_code_devise_parametres ),
		       "" );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				       gnome_property_box_changed,
				       fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				       changement_code_entree_devise,
				       NULL );


  if ( ( devise -> passage_euro = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button_euro ) )) )
    {
      /* c'est une devise qui passera à l'euro */

      devise -> une_devise_1_egale_x_devise_2 = 0;
      devise -> no_devise_en_rapport = 1;
      devise -> change = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion )),
				    NULL );
      devise -> date_dernier_change = NULL;

    }
  else
    {
      /*       si le change a changé, c'est qu'il y a une mise à jours */

      if ( g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion )),
		      NULL )
	   !=
	   devise -> change )
	{
	  devise -> date_dernier_change = g_date_new ();
	  g_date_set_time ( devise -> date_dernier_change,
			    time (NULL));
	}


      /* qu'il y ait un changement dans le change ou pas, on récupère toutes les autres valeurs */

      devise -> no_devise_en_rapport =  ((struct struct_devise *)(gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
											"adr_devise" ))) -> no_devise;

      if ( devise -> no_devise_en_rapport
	   &&
	   ( ((struct struct_devise *)(gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( devise_1 ) -> menu_item ),
							     "adr_devise" ))) -> no_devise_en_rapport )
	   ==
	   devise -> no_devise_en_rapport )
	devise -> une_devise_1_egale_x_devise_2 = 1;
      else
	devise -> une_devise_1_egale_x_devise_2 = 0;
      
      devise -> change = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion )),
				    NULL );
    }
	    


  gtk_widget_set_sensitive ( check_button_euro,
			     TRUE );
  gtk_widget_set_sensitive ( frame,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_supprimer_devise,
			     FALSE );

  return FALSE;
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
/* Fonction change_passera_euro */
/* appelée quand appuie sur le bouton Passera à l'euro */
/* **************************************************************************************************************************** */

gboolean change_passera_euro ( GtkWidget *bouton,
			   GtkWidget *liste )
{
  struct struct_devise *devise;
  GtkWidget *menu;
  GtkWidget *item;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton ) ))
    {
      gtk_widget_hide ( hbox_devise_associee );
      gtk_widget_hide ( label_date_dernier_change );
      gtk_widget_show ( hbox_ligne_change );

      /* on met l'euro dans le 1er menu */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( _("Euro") );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_1 ),
				 menu );
      gtk_widget_show ( menu );


      /*       on met la devise dans le 2ème menu */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( devise -> nom_devise );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_2 ),
				 menu );
      gtk_widget_show ( menu );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_conversion ),
					 GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					 fenetre_preferences );

      gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			   g_strdup_printf ( "%f",
					     devise -> change ));
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_conversion ),
					   GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					   fenetre_preferences );
      /* on rend le tout sensitif */

      gtk_widget_set_sensitive ( hbox_ligne_change,
				 TRUE );
      gtk_widget_set_sensitive ( entree_conversion,
				 TRUE );
      gtk_widget_set_sensitive ( devise_2,
				 FALSE );
      gtk_widget_set_sensitive ( devise_1,
				 FALSE );

    }
  else
    {
/*       magouille car l'option menu ne contient pas le nom de la devise courante */

      if ( devise -> no_devise > devise -> no_devise_en_rapport )
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devises ),
				      g_slist_position ( liste_struct_devises_tmp,
							 g_slist_find_custom ( liste_struct_devises_tmp,
									       GINT_TO_POINTER ( devise -> no_devise_en_rapport ),
									       ( GCompareFunc ) recherche_devise_par_no )) + 1);
      else
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devises ),
				      g_slist_position ( liste_struct_devises_tmp,
							 g_slist_find_custom ( liste_struct_devises_tmp,
									       GINT_TO_POINTER ( devise -> no_devise_en_rapport ),
									       ( GCompareFunc ) recherche_devise_par_no ))  );

      gtk_widget_show ( hbox_devise_associee );

      if ( devise -> date_dernier_change )
	{
	  gchar date[11];

	  g_date_strftime ( date,
			    11,
			    "%d/%m/%Y",
			    devise -> date_dernier_change );

	  gtk_label_set_text ( GTK_LABEL ( label_date_dernier_change ),
			       date );
	  /* mise en place du change courant */

	  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_conversion ),
					     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					     fenetre_preferences );
	  gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			       g_strdup_printf ( "%f",
						 devise -> change ));
	  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_conversion ),
					       GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					       fenetre_preferences );

	}
      else
	{
	  gtk_label_set_text ( GTK_LABEL ( label_date_dernier_change ),
			       _("No exchange rate defined")  );
	  /* mise en place du change courant */
	  
	  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_conversion ),
					     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					     fenetre_preferences );
	  gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			       "" );
	  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_conversion ),
					       GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					       fenetre_preferences );
	}

      gtk_widget_show ( label_date_dernier_change );

      /* on rend le tout sensitif */

      gtk_widget_set_sensitive ( hbox_ligne_change,
				 TRUE );
      gtk_widget_set_sensitive ( entree_conversion,
				 TRUE );
      gtk_widget_set_sensitive ( devise_2,
				  TRUE);
      gtk_widget_set_sensitive ( devise_1,
				 TRUE );

      changement_devise_associee ( GTK_OPTION_MENU ( option_menu_devises ) -> menu,
				   liste );
      
      gtk_widget_show ( hbox_ligne_change );

    }

  return FALSE;
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
/* Fonction changement_devise_associee */
/* appelée lorsqu'on change la devise comparée */
/* **************************************************************************************************************************** */

gboolean changement_devise_associee ( GtkWidget *menu_devises,
				  GtkWidget *liste )
{
  struct struct_devise *devise;
  struct struct_devise *devise_associee;
  GtkWidget *menu;
  GtkWidget *item;


  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );

  devise_associee = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
					  "adr_devise" );
  if ( devise_associee -> no_devise )
    {
      gtk_widget_set_sensitive ( hbox_ligne_change,
				 TRUE );

      /* création du menu de la 1ère devise ( le menu comporte la devise courante et celle associée ) */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( devise -> nom_devise );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    devise );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      item = gtk_menu_item_new_with_label ( NOM_DEVISE_ASSOCIEE );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
						  "adr_devise" ));
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );


      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_1 ),
				 menu );
      gtk_signal_connect ( GTK_OBJECT ( menu ),
			   "selection-done",
			   activer_bouton_appliquer,
			   NULL );

      gtk_widget_show ( menu );
	  


      /* création du menu de la 2ème devise ( le menu comporte la devise courante et celle associée ) */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( devise -> nom_devise );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    devise );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      item = gtk_menu_item_new_with_label ( NOM_DEVISE_ASSOCIEE );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
						  "adr_devise" ));
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );


      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_2 ),
				 menu );
      gtk_signal_connect ( GTK_OBJECT ( menu ),
			   "selection-done",
			   activer_bouton_appliquer,
			   NULL );

      gtk_widget_show ( menu );
 

      /* choix des 1ère et 2ème devise */

      gtk_option_menu_set_history ( GTK_OPTION_MENU ( devise_1 ),
				    !( devise -> une_devise_1_egale_x_devise_2 ));
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( devise_2 ),
				    devise -> une_devise_1_egale_x_devise_2 );
    }
  else
    {
      menu = gtk_menu_new ();
     gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_1 ),
				menu );

     menu = gtk_menu_new ();
     gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_2 ),
				menu );

     /* FIXME ? */
     gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_conversion ),
					GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					fenetre_preferences );
      gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			   "" );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_conversion ),
					   GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
					   fenetre_preferences );

      gtk_widget_set_sensitive ( hbox_ligne_change,
				 FALSE );
    }

  return FALSE;
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
gboolean changement_nom_entree_devise ( void )
{
  struct struct_devise *devise;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( clist_devises_parametres ),
				    ligne_selection_devise );

  devise -> nom_devise = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_devise_parametres ))));

  gtk_clist_set_text ( GTK_CLIST ( clist_devises_parametres ),
		       ligne_selection_devise,
		       0,
		       devise -> nom_devise );
  return FALSE;
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
gboolean changement_code_entree_devise ( void )
{
  struct struct_devise *devise;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( clist_devises_parametres ),
				    ligne_selection_devise );

  devise -> code_devise = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_code_devise_parametres ))));

  gtk_clist_set_text ( GTK_CLIST ( clist_devises_parametres ),
		       ligne_selection_devise,
		       1,
		       devise -> code_devise );
  return FALSE;
}
/* **************************************************************************************************************************** */



gboolean changement_iso_code_entree_devise ( void )
{
  struct struct_devise *devise;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( clist_devises_parametres ),
				    ligne_selection_devise );

  devise -> code_iso4217_devise = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code_devise_parametres ))));

  gtk_clist_set_text ( GTK_CLIST ( clist_devises_parametres ),
		       ligne_selection_devise,
		       2,
		       devise -> code_iso4217_devise );
  return FALSE;
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
/* cette fonction prend en argument un montant, la devise de renvoi (en général la devise du compte) */
/*      et la devise du montant donné en argument */
/* elle renvoie le montant de l'opération dans la devise de renvoi */
/* **************************************************************************************************************************** */

gdouble calcule_montant_devise_renvoi ( gdouble montant_init,
					gint no_devise_renvoi,
					gint no_devise_montant,
					gint une_devise_compte_egale_x_devise_ope,
					gdouble taux_change,
					gdouble frais_change )
{
  gdouble montant;

  /* tout d'abord, si les 2 devises sont les mêmes, on renvoie le montant directement */

  if ( no_devise_renvoi == no_devise_montant )
    return ( montant_init );

  /*   il faut faire une transformation du montant */
  /* on utilise les variables globales devise_compte et devise_operation pour */
  /* gagner du temps */

  /* récupère la devise du compte si nécessaire */

  if ( !devise_compte
       ||
       devise_compte -> no_devise != no_devise_renvoi )
    devise_compte = g_slist_find_custom ( liste_struct_devises,
					  GINT_TO_POINTER ( no_devise_renvoi ),
					  ( GCompareFunc ) recherche_devise_par_no) -> data;

  /* récupère la devise de l'opération si nécessaire */

  if ( !devise_operation
       ||
       devise_operation -> no_devise != no_devise_montant )
    devise_operation = g_slist_find_custom ( liste_struct_devises,
					     GINT_TO_POINTER ( no_devise_montant ),
					     ( GCompareFunc ) recherche_devise_par_no) -> data;

  /* on a maintenant les 2 devises, on peut faire les calculs */

  if ( devise_compte -> passage_euro
       &&
       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
    montant = montant_init * devise_compte -> change;
  else
    if ( devise_operation -> passage_euro
	 &&
	 !strcmp ( devise_compte -> nom_devise, _("Euro") ))
      montant = montant_init / devise_operation -> change;
    else
      if ( une_devise_compte_egale_x_devise_ope )
	montant = montant_init / taux_change - frais_change;
      else
	montant = montant_init * taux_change - frais_change;

  montant = ( rint (montant * 100 )) / 100;

  return ( montant);
}
/* **************************************************************************************************************************** */



gchar * devise_name ( struct struct_devise * devise )
{
  if (devise -> code_devise)
    return devise -> code_devise;

  return devise -> code_iso4217_devise;
}
