/* ce fichier contient les paramètres de l'affichage de la liste d'opé */


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

#define __(X) X

gchar *labels_boutons [] = { __("Date"),
			     __("Date de valeur"),
			     __("Tiers"),
			     __("R/P"),
			     __("Débit"),
			     __("Crédit"),
			     __("Solde"),
			     __("Montant (devise compte)"),
			     __("Moyen de paiement"),
			     __("N° rapprochement"),
			     __("Exercice"),
			     __("Catégorie"),
			     __("IB"),
			     __("Pièce comptable"),
			     __("Notes"),
			     __("Infos banque/guichet"),
			     __("N° opération"),
			     NULL };


/* ************************************************************************************************************** */
GtkWidget *onglet_affichage_liste ( void )
{
  GtkWidget *onglet;
  gchar *titres [] = { _("Col1"),
		       _("Col2"),
		       _("Col3"),
		       _("Col4"),
		       _("Col5"),
		       _("Col6"),
		       _("Col7") };
  gint i, j;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *hbox;

  ligne_depart_drag = 0;
  col_depart_drag = 0;

  /*   à la base, on met une vbox */

  onglet = gtk_vbox_new ( FALSE,
			  5 );
  gtk_widget_show ( onglet );


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Affichage des données de l'opération dans la liste :" ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("     Opérations simplifiées : affiche la 1ère ligne." ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("     Opérations semi-complètes : affiche les 1ère, 2ème et 4ème lignes." ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("     Opérations complètes : affiche toutes les lignes." ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );



  /* mise en place de la clist_affichage_liste */
  /*   on lui met des titres redimensionnables */
  /*   elle fait 7 colonnes et 4 lignes */

  clist_affichage_liste = gtk_clist_new_with_titles ( 7,
				      titres );
  gtk_clist_column_titles_passive ( GTK_CLIST ( clist_affichage_liste ));


  gtk_signal_connect ( GTK_OBJECT ( clist_affichage_liste ),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( pression_bouton_classement_liste ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( clist_affichage_liste ),
		       "button_release_event",
		       GTK_SIGNAL_FUNC ( lache_bouton_classement_liste ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( clist_affichage_liste ),
			      "resize-column",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_signal_connect ( GTK_OBJECT ( clist_affichage_liste ),
		       "size-allocate",
		       GTK_SIGNAL_FUNC ( changement_taille_liste_affichage ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       clist_affichage_liste,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( clist_affichage_liste );



  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Informations affichées dans la liste :" ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  /* on crée maintenant une table de 3x6 boutons */

  table = gtk_table_new ( 3,
			  6,
			  FALSE );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       table,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( table );

  for ( i=0 ; i<3 ; i++ )
    for ( j=0 ; j<6 ; j++ )
      {
	if ( labels_boutons[j+ i*6] )
	  {
	    boutons_affichage_liste[j + i*6] = gtk_toggle_button_new_with_label ( labels_boutons[j + i*6] );
	    gtk_signal_connect ( GTK_OBJECT ( boutons_affichage_liste[j + i*6] ),
				 "toggled",
				 GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
				 GINT_TO_POINTER ( j + i*6 ));
	    gtk_table_attach_defaults ( GTK_TABLE ( table ),
					boutons_affichage_liste[j + i*6],
					j, j+1,
					i, i+1 );
	    gtk_widget_show ( boutons_affichage_liste[j + i*6] );
	  }
      }


  /* on permet maintenant de choisir soi même la taille des colonnes */

  bouton_choix_perso_colonnes = gtk_check_button_new_with_label ( _("Grisbi ajuste la taille des colonnes selon le tableau ci-dessus" ));
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_choix_perso_colonnes ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       bouton_choix_perso_colonnes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_perso_colonnes );



  /* on recopie le tab_affichage_ope */

  for ( i = 0 ; i<4 ; i++ )
    for ( j = 0 ; j<7 ; j++ )
      tab_affichage_ope_tmp[i][j] = tab_affichage_ope[i][j];

  /* on remplit le tableau */

  remplissage_tab_affichage_ope ( clist_affichage_liste );

  /* on met le bouton des tailles de colonne */

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_choix_perso_colonnes ),
				 etat.largeur_auto_colonnes );

  /* à remplacer par un affichage dynamique */

  gtk_clist_set_column_width ( GTK_CLIST ( clist_affichage_liste ),
				     0,
				     gtk_clist_optimal_column_width ( GTK_CLIST ( clist_affichage_liste ), 0 ));
  gtk_clist_set_column_width ( GTK_CLIST ( clist_affichage_liste ),
				     1,
				     gtk_clist_optimal_column_width ( GTK_CLIST ( clist_affichage_liste ), 1 ));
  gtk_clist_set_column_width ( GTK_CLIST ( clist_affichage_liste ),
				     2,
				     gtk_clist_optimal_column_width ( GTK_CLIST ( clist_affichage_liste ), 2 ));
  gtk_clist_set_column_width ( GTK_CLIST ( clist_affichage_liste ),
				     3,
				     gtk_clist_optimal_column_width ( GTK_CLIST ( clist_affichage_liste ), 3 ));
  gtk_clist_set_column_width ( GTK_CLIST ( clist_affichage_liste ),
				     40,
				     gtk_clist_optimal_column_width ( GTK_CLIST ( clist_affichage_liste ), 4 ));
  gtk_clist_set_column_width ( GTK_CLIST ( clist_affichage_liste ),
				     5,
				     gtk_clist_optimal_column_width ( GTK_CLIST ( clist_affichage_liste ), 5 ));
  gtk_clist_set_column_width ( GTK_CLIST ( clist_affichage_liste ),
				     6,
				     gtk_clist_optimal_column_width ( GTK_CLIST ( clist_affichage_liste ), 6 ));


  return ( onglet );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
gboolean pression_bouton_classement_liste ( GtkWidget *clist,
					    GdkEventButton *ev )
{
  GdkCursor *cursor;
  GdkPixmap *source, *mask;
  GdkColor fg = { 0, 65535, 0, 0 }; /* Red. */
  GdkColor bg = { 0, 0, 0, 65535 }; /* Blue. */

static unsigned char cursor1_bits[] = {
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0x00, 0x00, 0x3f, 0xfc, 0x3f, 0xfc, 0x00, 0x00, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01};
static unsigned char cursor1mask_bits[] = {
   0x80, 0x01, 0x8e, 0x71, 0x86, 0x61, 0x8a, 0x51, 0x90, 0x09, 0xa0, 0x05,
   0x40, 0x02, 0x3f, 0xfc, 0x3f, 0xfc, 0x40, 0x02, 0xa0, 0x05, 0x90, 0x09,
   0x8a, 0x51, 0x86, 0x61, 0x8e, 0x71, 0x80, 0x01};

  /*   si la souris se trouve dans les titres, on se barre simplement */

  if ( ev -> window != GTK_CLIST ( clist ) -> clist_window )
    return ( FALSE );


  /* on crée le nouveau curseur */



  source = gdk_bitmap_create_from_data (NULL,
					cursor1_bits,
					16,
					16);
  mask = gdk_bitmap_create_from_data (NULL,
				      cursor1mask_bits,
				      16,
				      16);

  cursor = gdk_cursor_new_from_pixmap (source,
				       mask,
				       &fg,
				       &bg,
				       8,
				       8);
  gdk_pixmap_unref (source);
  gdk_pixmap_unref (mask);




  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( clist ),
				 "button_press_event");

  /* récupère et sauve les coordonnées de la liste au départ */

  gtk_clist_get_selection_info ( GTK_CLIST ( clist ),
				 ev -> x,
				 ev -> y,
				 &ligne_depart_drag,
				 &col_depart_drag );

  /* on grab la souris */

  gdk_pointer_grab ( GTK_CLIST ( clist) -> clist_window,
		     FALSE,
		     GDK_BUTTON_RELEASE_MASK,
		     GTK_CLIST ( clist ) -> clist_window,
		     cursor,
		     GDK_CURRENT_TIME );

  return ( TRUE );
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
gboolean lache_bouton_classement_liste ( GtkWidget *clist,
					 GdkEventButton *ev )
{
  gint ligne_arrivee_drag;
  gint col_arrivee_drag;
  gchar *texte_depart[1];
  gchar *texte_arrivee[1];
  gchar *buffer;
  gint buffer_int;

  /*   si la souris se trouve dans les titres, on se barre simplement */

  if ( ev -> window != GTK_CLIST ( clist ) -> clist_window )
    return ( FALSE );

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( clist ),
				 "button_release_event");

  /* récupère et sauve les coordonnées de la liste au départ */

  gtk_clist_get_selection_info ( GTK_CLIST ( clist ),
				 ev -> x,
				 ev -> y - 5,
				 &ligne_arrivee_drag,
				 &col_arrivee_drag );

  /* on dégrab la souris */

  gdk_pointer_ungrab ( GDK_CURRENT_TIME );

  /* si la cellule de départ est la même que celle de l'arrivée, on se barre */
  
  if ( ligne_depart_drag == ligne_arrivee_drag
       &&
       col_depart_drag == col_arrivee_drag )
    return ( TRUE );


  /* on échange les 2 textes */

  gtk_clist_get_text ( GTK_CLIST ( clist ),
		       ligne_depart_drag,
		       col_depart_drag,
		       texte_depart );

  gtk_clist_get_text ( GTK_CLIST ( clist ),
		       ligne_arrivee_drag,
		       col_arrivee_drag,
		       texte_arrivee );

  buffer = g_strdup ( texte_arrivee[0] );

  gtk_clist_set_text ( GTK_CLIST ( clist ),
		       ligne_arrivee_drag,
		       col_arrivee_drag,
		       g_strdup ( texte_depart[0] ));

  gtk_clist_set_text ( GTK_CLIST ( clist ),
		       ligne_depart_drag,
		       col_depart_drag,
		       buffer );

  /* on échange les 2 nombres */

  buffer_int = tab_affichage_ope_tmp[ligne_depart_drag][col_depart_drag];
  tab_affichage_ope_tmp[ligne_depart_drag][col_depart_drag] = tab_affichage_ope_tmp[ligne_arrivee_drag][col_arrivee_drag];
  tab_affichage_ope_tmp[ligne_arrivee_drag][col_arrivee_drag] = buffer_int;

  /* on dégrise le appliquer de la fenetre de préférences */

  gnome_property_box_changed ( GNOME_PROPERTY_BOX ( fenetre_preferences ));

  return ( TRUE );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void remplissage_tab_affichage_ope ( GtkWidget *clist )
{
  gchar *ligne [7];
  gint i;

  gtk_clist_freeze ( GTK_CLIST ( clist ));
  gtk_clist_clear ( GTK_CLIST ( clist ));

  for ( i=0 ; i<4 ; i++ )
    {
      gint j;

      for ( j=0 ; j<7 ; j++ )
	{
	  switch ( tab_affichage_ope_tmp[i][j] )
	    {
	      case 0:
		ligne[j] = "";
		break;

	      case 1:
		ligne[j] = "Date";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[0] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (0) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[0] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[0] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (0) );
		break;

	      case 2:
		ligne[j] = "Date de valeur";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[1] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (1) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[1] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[1] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (1) );
		break;

	      case 3:
		ligne[j] = "Tiers";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[2] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (2) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[2] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[2] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (2) );
		break;

	      case 4:
		ligne[j] = "P/R";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[3] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (3) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[3] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[3] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (3) );
		break;

	      case 5:
		ligne[j] = "Débit";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[4] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (4) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[4] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[4] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (4) );
		break;

	      case 6:
		ligne[j] = "Crédit";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[5] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (5) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[5] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[5] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (5) );
		break;

	      case 7:
		ligne[j] = "Solde";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[6] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (6) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[6] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[6] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (6) );
		break;

	      case 8:
		ligne[j] = "Montant(devise compte)";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[7] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (7) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[7] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[7] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (7) );
		break;

	      case 9:
		ligne[j] = "Moyen de paiement";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[8] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (8) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[8] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[8] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (8) );
		break;

	      case 10:
		ligne[j] = "N° de rapprochement";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[9] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (9) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[9] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[9] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (9) );
		break;

	      case 11:
		ligne[j] = "Exercice";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[10] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (10) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[10] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[10] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (10) );
		break;

	      case 12:
		ligne[j] = "Catégories";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[11] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (11) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[11] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[11] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (11) );
		break;

	      case 13:
		ligne[j] = "Imputations budgétaires";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[12] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (12) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[12] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[12] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (12) );
		break;

	      case 14:
		ligne[j] = "Pièce comptable";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[13] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (13) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[13] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[13] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (13) );
		break;

	      case 15:
		ligne[j] = "Notes";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[14] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (14) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[14] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[14] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (14) );
		break;

	      case 16:
		ligne[j] = "Info banque/guichet";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[15] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (15) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[15] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[15] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (15) );
		break;

	      case 17:
		ligne[j] = "N° d'opération";
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[16] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (16) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[16] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[16] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (16) );
		break;

	      case 18:
		ligne[j] = "N° chèque/virement";
		break;
	    }
	}

      gtk_clist_append ( GTK_CLIST ( clist ),
			 ligne );
    }

  gtk_clist_thaw ( GTK_CLIST ( clist ));

}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* Fonction appelée lorsqu'on click sur un bouton de l'affichage de la liste */
/* retire ou met le texte correspondant dans le 1er emplacement libre de */
/* la liste */
/* ************************************************************************************************************** */

void toggled_bouton_affichage_liste ( GtkWidget *bouton,
				      gint *no_bouton )
{
  /* on travaille en fait sur tab_affichage_ope_tmp et un appel */
  /* à remplissage_tab_affichage_ope va actualiser la liste */

  gint i, j;

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton )))
    {
      /* on vient d'enfoncer le bouton */

      /* on met ce no+1 dans la 1ère case de libre en commençant en haut à gauche */
      /* si ce no est 9-1(moyen de paiement), on y met aussi 18 (no chèque) */


      for ( i = 0 ; i<4 ; i++ )
	for ( j = 0 ; j<7 ; j++ )
	  if ( !tab_affichage_ope_tmp[i][j] )
	    {
	      tab_affichage_ope_tmp[i][j] = GPOINTER_TO_INT ( no_bouton ) + 1;

	      if ( GPOINTER_TO_INT ( no_bouton ) == 8 )
		no_bouton = GINT_TO_POINTER ( 17 );
	      else
		{
		  i=4;
		  j=7;
		}
	    }
    }
  else
    {
      /*       on vient de désenfoncer le bouton */

      /* recherche le no de bouton dans le tableau et met 0 à la place */
      /*       s'il s'agit du moyen de paiement(9), vire aussi le no de chèque(18) */

      for ( i = 0 ; i<4 ; i++ )
	for ( j = 0 ; j<7 ; j++ )
	  if ( tab_affichage_ope_tmp[i][j] == ( GPOINTER_TO_INT ( no_bouton ) + 1 )
	       ||
	       ( GPOINTER_TO_INT ( no_bouton ) == 8
		 &&
		 tab_affichage_ope_tmp[i][j] == 18 ))
	    tab_affichage_ope_tmp[i][j] = 0;
    }

  remplissage_tab_affichage_ope ( clist_affichage_liste );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void changement_taille_liste_affichage ( GtkWidget *clist,
					 GtkAllocation *allocation )
{
  gint i;
  gint largeur;


  largeur = allocation->width;
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( clist ),
				     GTK_SIGNAL_FUNC ( changement_taille_liste_affichage ),
				     NULL );
  for ( i=0 ; i<6 ; i++ )
    gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				 i,
				 rapport_largeur_colonnes[i] * largeur / 100 );
}
/* ************************************************************************************************************** */
