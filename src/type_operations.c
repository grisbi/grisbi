/* Ce fichier s'occupe de la gestion des types d'opérations */
/* type_operations.c */

/*     Copyright (C) 2000-2002  Cédric Auger */
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
/* renvoie la page qui sera dans les paramètres */
/* ************************************************************************************************************** */

GtkWidget *onglet_types_operations ( void )
{
  GtkWidget *onglet;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *scrolled_window;
  gchar *titres[2] = { _("Comptes"),
		       _("Défaut") };
  gint i;
  GtkWidget *vbox;
  GtkWidget *hbox2;
  GtkWidget *menu;
  GtkWidget *item;
  GtkWidget *label;
  GtkWidget *bouton;
  GtkWidget *vbox2;
  GtkWidget *separateur;


/*   on fait une copie de tous les types, et les met dans une liste temporaire */
/* la même chose pour les tris */

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

  /*   onglet : une hbox */

  onglet = gtk_hbox_new ( FALSE,
			  5 );
  gtk_widget_show ( onglet );


  /* frame contenant les types d'opérations */

  frame = gtk_frame_new ( _("Types d'opérations") );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( hbox ),
				   5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      hbox );
  gtk_widget_show ( hbox );

  /*   mise en place de la partie de gauche : l'arbre sur les boutons ajouter-retirer */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );


  /* mise en place de l'arbre */

  scrolled_window = gtk_scrolled_window_new ( NULL,
					      NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );

  arbre_types_operations = gtk_ctree_new_with_titles ( 2,
				      0,
				      titres );
  gtk_clist_column_titles_passive ( GTK_CLIST ( arbre_types_operations ));
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( arbre_types_operations ),
				     0,
				     TRUE );
  gtk_ctree_set_line_style ( GTK_CTREE ( arbre_types_operations ),
			     GTK_CTREE_LINES_DOTTED );
  gtk_ctree_set_expander_style ( GTK_CTREE ( arbre_types_operations ),
				 GTK_CTREE_EXPANDER_CIRCULAR );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      arbre_types_operations );
  gtk_widget_show ( arbre_types_operations );


  /* remplissage de l'arbre : on fait le tour de tous les comptes */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GtkCTreeNode *node_compte;
      GtkCTreeNode *node_debit;
      GtkCTreeNode *node_credit;
      gchar *ligne[2];
      GSList *liste_tmp;


      ligne[0] = NOM_DU_COMPTE;
      ligne[1] = NULL;

      node_compte = gtk_ctree_insert_node ( GTK_CTREE ( arbre_types_operations ),
					    NULL,
					    NULL,
					    ligne,
					    0,
					    NULL, NULL,
					    NULL, NULL,
					    FALSE,
					    FALSE );

      gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_types_operations ),
				    node_compte,
				    GINT_TO_POINTER ( i ) );

      /*       pour chaque compte on ajout débit et crédit */

      ligne[0] = _("Débit");
      ligne[1] = NULL;

      node_debit = gtk_ctree_insert_node ( GTK_CTREE ( arbre_types_operations ),
					   node_compte,
					   NULL,
					   ligne,
					   0,
					   NULL, NULL,
					   NULL, NULL,
					   FALSE,
					   FALSE );

      ligne[0] = _("Crédit");
      ligne[1] = NULL;

      node_credit = gtk_ctree_insert_node ( GTK_CTREE ( arbre_types_operations ),
					   node_compte,
					   NULL,
					   ligne,
					   0,
					   NULL, NULL,
					   NULL, NULL,
					   FALSE,
					   FALSE );


      /* fait le tour des types d'opé du compte */

      liste_tmp = liste_tmp_types[i];

      while ( liste_tmp )
	{
	  struct struct_type_ope *type_ope;
	  GtkCTreeNode *node_parent;
	  GtkCTreeNode *node_type;

	  type_ope = liste_tmp->data;

	  ligne[0] = type_ope -> nom_type;

	  if ( type_ope -> no_type == TYPE_DEFAUT_DEBIT
	       ||
	       type_ope -> no_type == TYPE_DEFAUT_CREDIT )
	    ligne[1] = "x";
	  else
	    ligne[1] = NULL;

	  if ( !type_ope -> signe_type )
	    node_parent = node_compte;
	  else
	    if ( type_ope -> signe_type == 1 )
	      node_parent = node_debit;
	    else
	      node_parent = node_credit;

	  node_type = gtk_ctree_insert_node ( GTK_CTREE ( arbre_types_operations ),
					      node_parent,
					      NULL,
					      ligne,
					      0,
					      NULL, NULL,
					      NULL, NULL,
					      FALSE,
					      FALSE );

	  /* on associe au type l'adr de sa structure */

	  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_types_operations ),
					node_type,
					type_ope );

	  liste_tmp = liste_tmp -> next;
	}

      p_tab_nom_de_compte_variable++;
    }


  /* mise en place des boutons ajouter et supprimer */

  hbox2 = gtk_hbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox2,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox2 );

  bouton_ajouter_type = gnome_stock_button ( GNOME_STOCK_PIXMAP_ADD );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_type ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_ajouter_type,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_type ),
		       "clicked",
		       (GtkSignalFunc ) ajouter_type_operation,
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_ajouter_type ),
			      "clicked",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       bouton_ajouter_type,
		       TRUE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton_ajouter_type );

  bouton_retirer_type = gnome_stock_button ( GNOME_STOCK_PIXMAP_REMOVE );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_retirer_type ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_retirer_type,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_retirer_type ),
		       "clicked",
		       (GtkSignalFunc ) supprimer_type_operation,
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_retirer_type ),
			      "clicked",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       bouton_retirer_type,
		       TRUE,
		       FALSE,
		       5 );
  gtk_widget_show ( bouton_retirer_type );


  /* mise en place du détail des types */

  vbox2 = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox2,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox2 );


  /* mise en place de la vbox qui sera sensitive ou non */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_widget_set_sensitive ( vbox,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  /* mise en place de la ligne de modification du nom */

  hbox2 = gtk_hbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox2,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( hbox2 );

 
  label = gtk_label_new ( _("Nom : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  entree_type_nom = gtk_entry_new ();
  gtk_widget_set_usize ( entree_type_nom,
			 100,
			 FALSE );
  gtk_signal_connect ( GTK_OBJECT ( entree_type_nom ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modification_entree_nom_type ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_type_nom ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       entree_type_nom,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_type_nom );


  /* mise en place du bouton fait apparaitre l'entrée */

  bouton_type_apparaitre_entree = gtk_check_button_new_with_label ( _("Peut entrer des données supplémentaires") );
  gtk_signal_connect ( GTK_OBJECT ( bouton_type_apparaitre_entree ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( modification_type_affichage_entree ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_apparaitre_entree ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_type_apparaitre_entree,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( bouton_type_apparaitre_entree );

  /* mise en place de la ligne numérotation auto / numéro en cours */

  hbox2 = gtk_hbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox2,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( hbox2 );

 
  bouton_type_numerotation_automatique = gtk_check_button_new_with_label ( _("Numérotation automatique") );
  gtk_signal_connect ( GTK_OBJECT (bouton_type_numerotation_automatique  ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( modification_type_numerotation_auto ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_numerotation_automatique ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       bouton_type_numerotation_automatique,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (bouton_type_numerotation_automatique  );

  entree_type_dernier_no = gtk_entry_new ();
  gtk_widget_set_usize ( entree_type_dernier_no,
			 100,
			 FALSE );
  gtk_signal_connect ( GTK_OBJECT (entree_type_dernier_no  ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modification_entree_type_dernier_no ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_type_dernier_no ),
			      "changed",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       entree_type_dernier_no,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_type_dernier_no );


/* mise en place du choix neutre / débit / crédit */

  hbox2 = gtk_hbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox2,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( hbox2 );

  bouton_signe_type = gtk_option_menu_new ();
  menu = gtk_menu_new();

  item = gtk_menu_item_new_with_label ( _("Neutre") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Débit") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      GINT_TO_POINTER (1) );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Crédit") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      GINT_TO_POINTER (2) );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_signe_type ),
			     menu );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       bouton_signe_type,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_signe_type );

  /* mise en place du choix par défaut */

  bouton_type_choix_defaut = gtk_check_button_new_with_label ( _("Par-défaut") );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_choix_defaut ),
			      "toggled",
			      GTK_SIGNAL_FUNC ( modification_type_par_defaut ),
			      NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_choix_defaut ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       bouton_type_choix_defaut,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_type_choix_defaut );


  /* on connecte les sélection de la liste */

  gtk_signal_connect ( GTK_OBJECT ( arbre_types_operations ),
		       "tree-select-row",
		       GTK_SIGNAL_FUNC ( selection_ligne_arbre_types ),
		       vbox );
  gtk_signal_connect ( GTK_OBJECT ( arbre_types_operations ),
		       "tree-unselect-row",
		       GTK_SIGNAL_FUNC ( deselection_ligne_arbre_types ),
		       vbox );

  /* mise en place de l'affichage séparé débit/crédit ou tout ensemble */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  bouton_type_choix_affichage_formulaire = gtk_check_button_new_with_label ( _("Afficher toujours tous les types dans le formulaire") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON( bouton_type_choix_affichage_formulaire ),
				 etat.affiche_tous_les_types );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_choix_affichage_formulaire ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_type_choix_affichage_formulaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_type_choix_affichage_formulaire );

  if ( !nb_comptes )
    gtk_widget_set_sensitive ( bouton_type_choix_affichage_formulaire,
			       FALSE );

  /* mise en place de la liste de tri */


  frame = gtk_frame_new ( _("Rapprochement : tri des opérations") );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( frame );

  /* on met une vbox dans la frame */

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox2 ),
				   5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );

  /*   la partie du haut : tri par date ou par type */

  bouton_type_tri_date = gtk_radio_button_new_with_label ( NULL,
							   _("Tri par date") );
  gtk_widget_set_sensitive ( bouton_type_tri_date,
			     FALSE );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_tri_date ),
				 TRUE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_type_tri_date ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( modif_tri_date_ou_type ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_tri_date ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_type_tri_date,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_type_tri_date );

  bouton_type_tri_type = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_type_tri_date )),
							   _("Tri par type d'opération") );
  gtk_widget_set_sensitive ( bouton_type_tri_type,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_type_tri_type,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_type_tri_type );


  /* la partie du milieu est une hbox avec les types */

  hbox = gtk_hbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox );


  /* mise en place de la liste qui contient les types classés */

  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
  gtk_widget_show ( scrolled_window );


  type_liste_tri = gtk_clist_new ( 1 );
  gtk_widget_set_sensitive ( type_liste_tri,
			     FALSE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( type_liste_tri ) ,
				     0,
				     TRUE );
  gtk_clist_set_reorderable ( GTK_CLIST ( type_liste_tri ),
			      TRUE );
  gtk_clist_set_use_drag_icons ( GTK_CLIST ( type_liste_tri ),
				 TRUE );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      type_liste_tri );
  gtk_signal_connect ( GTK_OBJECT ( type_liste_tri ),
		       "select_row",
		       (GtkSignalFunc ) selection_type_liste_tri,
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( type_liste_tri ),
		       "unselect_row",
		       (GtkSignalFunc ) deselection_type_liste_tri,
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( type_liste_tri ),
			      "row_move",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_signal_connect_after ( GTK_OBJECT ( type_liste_tri ),
			     "row_move",
			     GTK_SIGNAL_FUNC ( save_ordre_liste_type_tri ),
			     NULL );
  gtk_widget_show ( type_liste_tri );

      
  /* on place ici les flèches sur le côté de la liste */

  vbox_fleches_tri = gtk_vbutton_box_new ();
  gtk_widget_set_sensitive ( vbox_fleches_tri,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox_fleches_tri,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( vbox_fleches_tri );

  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_UP );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       (GtkSignalFunc ) deplacement_type_tri_haut,
		       NULL );
  gtk_container_add ( GTK_CONTAINER ( vbox_fleches_tri ),
		      bouton );
  gtk_widget_show ( bouton );

  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_DOWN );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       (GtkSignalFunc ) deplacement_type_tri_bas,
		       NULL);
  gtk_container_add ( GTK_CONTAINER ( vbox_fleches_tri ),
		      bouton );
  gtk_widget_show ( bouton );

  /* la partie du bas contient des check buttons */

  bouton_type_neutre_inclut = gtk_check_button_new_with_label ( _("Inclure les types mixtes dans les Crédits / Débits") );
  gtk_widget_set_sensitive ( bouton_type_neutre_inclut,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_type_neutre_inclut ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( inclut_exclut_les_neutres ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_neutre_inclut ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_type_neutre_inclut,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( bouton_type_neutre_inclut );

return ( onglet );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void selection_ligne_arbre_types ( GtkWidget *arbre,
				   GtkCTreeNode *node,
				   gint col,
				   GtkWidget *vbox )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node_banque;
  gint no_compte;

  gtk_widget_set_sensitive ( bouton_ajouter_type,
			     TRUE );

  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );

  /* on resensitive les boutons du tri */

  gtk_widget_set_sensitive ( bouton_type_tri_date,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_type_tri_type,
			     TRUE );


  /* on va remonter jusqu'au nom de la banque pour afficher les tris correspondant */

  node_banque = node;
  while ( GTK_CTREE_ROW ( node_banque ) -> level != 1 )
    node_banque = GTK_CTREE_ROW ( node_banque ) -> parent;

  no_compte = GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
							      node_banque ));

  /* on associe le no de la banque au bouton bouton_type_tri_date */

  gtk_object_set_data ( GTK_OBJECT ( bouton_type_tri_date ),
			"no_compte",
			GINT_TO_POINTER ( no_compte ));

  /* on affiche les caractéristiques du tri de la banque */

  /*   on commence par remplir la liste */

  remplit_liste_tri_par_type(no_compte);

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_tri_date ),
				     gnome_property_box_changed,
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_neutre_inclut ),
				     gnome_property_box_changed,
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_neutre_inclut ),
				     inclut_exclut_les_neutres,
				     NULL );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_tri_type ),
				 tri_tmp[no_compte] );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_neutre_inclut ),
				 neutres_inclus_tmp[no_compte] );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_tri_date ),
				       gnome_property_box_changed,
				       fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_neutre_inclut ),
				       gnome_property_box_changed,
				       fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_neutre_inclut ),
				       inclut_exclut_les_neutres,
				       NULL );



  /*   si on est sur débit ou crédit ou sur un nom de banque on se barre */

  if ( !type_ope || GTK_CTREE_ROW ( node ) -> level == 1 )
    return;

  /*   une structure est associée à la ligne sélectionnée : */

  /* on commence par annuler toutes les connection des widgets qu'on va changer ici */

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_nom ),
				     modification_entree_nom_type,
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_apparaitre_entree ),
				     modification_type_affichage_entree,
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_numerotation_automatique ),
				     modification_type_numerotation_auto,
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				     modification_entree_type_dernier_no,
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_choix_defaut ),
				     modification_type_par_defaut,
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_nom ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_apparaitre_entree ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_numerotation_automatique ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_choix_defaut ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );



  gtk_widget_set_sensitive ( bouton_retirer_type,
			     TRUE );

  gtk_widget_set_sensitive ( vbox,
			     TRUE );

  gtk_entry_set_text ( GTK_ENTRY ( entree_type_nom ),
		       type_ope -> nom_type );

  if ( type_ope -> affiche_entree )
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_apparaitre_entree ),
				     TRUE );
      gtk_widget_set_sensitive ( bouton_type_numerotation_automatique,
				 TRUE );

      if ( type_ope -> numerotation_auto )
	{
	  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_numerotation_automatique ),
					 TRUE );
	  gtk_widget_set_sensitive ( entree_type_dernier_no,
				     TRUE );
	  gtk_entry_set_text ( GTK_ENTRY ( entree_type_dernier_no ),
			       itoa ( type_ope -> no_en_cours ));
	}
      else
	{
	  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_numerotation_automatique ),
					 FALSE );
	  gtk_widget_set_sensitive ( entree_type_dernier_no,
				     FALSE );
	}
    }
  else
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_apparaitre_entree ),
				     FALSE );
      gtk_widget_set_sensitive ( bouton_type_numerotation_automatique,
				 FALSE );
      gtk_widget_set_sensitive ( entree_type_dernier_no,
				 FALSE );
    }

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_signe_type ),
				type_ope -> signe_type );

  if ( type_ope -> signe_type )
    {
      gtk_widget_set_sensitive ( bouton_type_choix_defaut,
				 TRUE );

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + type_ope -> no_compte;

      if ( type_ope -> no_type == type_defaut_debit[type_ope -> no_compte]
	   ||
	   type_ope -> no_type == type_defaut_credit[type_ope -> no_compte] )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_choix_defaut ),
				       TRUE );
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_choix_defaut ),
				       FALSE );
    }
  else
    gtk_widget_set_sensitive ( bouton_type_choix_defaut,
			       FALSE );
  
  /* on associe le node à l'entrée du nom */

  gtk_object_set_data ( GTK_OBJECT ( entree_type_nom ),
			"adr_node",
			node );

  /* on remet les connections */

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_nom ),
				       modification_entree_nom_type,
				       NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_apparaitre_entree ),
				       modification_type_affichage_entree,
				       NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_numerotation_automatique ),
				       modification_type_numerotation_auto,
				       NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				       modification_entree_type_dernier_no,
				       NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_choix_defaut ),
				       modification_type_par_defaut,
				       NULL );

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_nom ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_apparaitre_entree ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_numerotation_automatique ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_choix_defaut ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );


}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void deselection_ligne_arbre_types ( GtkWidget *arbre,
				     GtkCTreeNode *node,
				     gint col,
				     GtkWidget *vbox )
{


  /* on vire le no de la banque associé au bouton bouton_type_tri_date */

  gtk_object_set_data ( GTK_OBJECT ( bouton_type_tri_date ),
			"no_compte",
			NULL );

  /* on se remet sur classement par date pour désensitiver la liste */

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_tri_date ),
				     gnome_property_box_changed,
				     fenetre_preferences );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_tri_date ),
				     modif_tri_date_ou_type,
				     NULL );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_tri_date ),
				 TRUE );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_tri_date ),
				       modif_tri_date_ou_type,
				       NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_tri_date ),
				       gnome_property_box_changed,
				       fenetre_preferences );

  /* on desensitive les boutons du tri */

  gtk_widget_set_sensitive ( bouton_type_tri_date,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_type_tri_type,
			     FALSE );

  gtk_widget_set_sensitive ( bouton_type_neutre_inclut,
			     FALSE );
  gtk_widget_set_sensitive ( type_liste_tri,
			     FALSE );


  /* on efface la liste des tris */

  gtk_clist_clear ( GTK_CLIST ( type_liste_tri ));


  gtk_widget_set_sensitive ( vbox,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_ajouter_type,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_retirer_type,
			     FALSE );

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_nom ),
				     modification_entree_nom_type,
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_nom ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_entry_set_text ( GTK_ENTRY ( entree_type_nom ),
		       "" );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_nom ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_nom ),
				       modification_entree_nom_type,
				       NULL );

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				     modification_entree_type_dernier_no,
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_entry_set_text ( GTK_ENTRY ( entree_type_dernier_no ),
		       "" );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				       modification_entree_type_dernier_no,
				       NULL );


  gtk_object_set_data ( GTK_OBJECT ( entree_type_nom ),
			"adr_node",
			NULL );
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
void modification_entree_nom_type ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );

  /* on affiche tout de suite le texte */

  gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
			    node,
			    0,
			    g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_type_nom ))));

  /* et on le sauve dans les types tmp */

  type_ope -> nom_type = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entree_type_nom ))));

  /* on réaffiche la liste du tri pour appliquer le changement */

  remplit_liste_tri_par_type ( type_ope -> no_compte );
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
void modification_type_affichage_entree ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_apparaitre_entree )))
    {
      gtk_widget_set_sensitive ( bouton_type_numerotation_automatique,
				 TRUE );
      type_ope -> affiche_entree = 1;
    }
  else
    {
      gtk_widget_set_sensitive ( bouton_type_numerotation_automatique,
				 FALSE );
      type_ope -> affiche_entree = 0;
      gtk_widget_set_sensitive ( entree_type_dernier_no,
				 FALSE );
    }

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_numerotation_automatique ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_numerotation_automatique ),
				 FALSE );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_numerotation_automatique ),
				     GTK_SIGNAL_FUNC ( gnome_property_box_changed ),
				     fenetre_preferences );

}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
void modification_type_numerotation_auto (void)
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_numerotation_automatique )))
    {
      gtk_widget_set_sensitive ( entree_type_dernier_no,
				 TRUE );
      type_ope -> numerotation_auto = 1;
    }
  else
    {
      type_ope -> numerotation_auto = 0;
      gtk_widget_set_sensitive ( entree_type_dernier_no,
				 FALSE );
    }
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void modification_entree_type_dernier_no ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );

  type_ope -> no_en_cours = atoi ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_type_dernier_no ))));

}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void modification_type_signe ( gint *no_menu )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;
  GtkCTreeNode *node_parent;


  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );

  /*   s'il n'y a pas eu de changement, on vire */

  if ( GPOINTER_TO_INT ( no_menu ) == type_ope -> signe_type )
    return;

  /*   si on est sur neutre, on insensitive le par défaut */

  if ( GPOINTER_TO_INT ( no_menu ))
    gtk_widget_set_sensitive ( bouton_type_choix_defaut,
			       TRUE );
  else
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_choix_defaut ),
				     FALSE );
      gtk_widget_set_sensitive ( bouton_type_choix_defaut,
				 FALSE );
    }

  /*   si le type changé était par défaut, on vire le par-défaut qu'on met à 0 */

  if ( type_defaut_debit[type_ope->no_compte] == type_ope->no_type )
    {
      type_defaut_debit[type_ope->no_compte] = 0;
      gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
				node,
				1,
				"" );
    }
  else
    if ( type_defaut_credit[type_ope->no_compte] == type_ope->no_type )
      {
	type_defaut_credit[type_ope->no_compte] = 0;
	gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
				  node,
				  1,
				  "" );
      }


  switch ( GPOINTER_TO_INT ( no_menu ))
    {

      /*   cas le plus simple, on passe à neutre, dans ce cas le parent du node devient le compte */

    case 0:
      node_parent = GTK_CTREE_ROW ( GTK_CTREE_ROW ( node ) -> parent ) -> parent;

      gtk_ctree_move ( GTK_CTREE ( arbre_types_operations ),
		       node,
		       node_parent,
		       NULL );
      break;

      /* si c'est un débit */

    case 1:

      node_parent = GTK_CTREE_ROW ( node ) -> parent;

      if ( GTK_CTREE_ROW ( node_parent ) -> level == 2 )
	node_parent = GTK_CTREE_ROW ( node_parent ) -> parent;

      node_parent = GTK_CTREE_ROW ( node_parent ) -> children;
      gtk_ctree_move ( GTK_CTREE ( arbre_types_operations ),
		       node,
		       node_parent,
		       NULL );
      break;

      /* si c'est un credit */

    case 2:
      node_parent = GTK_CTREE_ROW ( node ) -> parent;

      if ( GTK_CTREE_ROW ( node_parent ) -> level == 2 )
	node_parent = GTK_CTREE_ROW ( node_parent ) -> parent;

      node_parent = GTK_CTREE_ROW ( GTK_CTREE_ROW ( node_parent ) -> children ) -> sibling;
      gtk_ctree_move ( GTK_CTREE ( arbre_types_operations ),
		       node,
		       node_parent,
		       NULL );
      break;

    }

  type_ope -> signe_type = GPOINTER_TO_INT ( no_menu );


  /*   pour les tris, il suffit de retirer les négatifs correspondant au type */
  /* puis réafficher la liste, sauf si on est passé sur */
  /* neutre et que les neutres sont inclus dans les débits et crédits */
  /*     dans ce cas, on ajoute à la liste l'opposé */

  if ( neutres_inclus_tmp[type_ope->no_compte] )
    {
      if ( no_menu )
	/* 	  on retire le signe opposé du type, juste au cas où on est passé d'un neutre à l'actuel */
	liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte],
							      GINT_TO_POINTER ( -type_ope->no_type ));
      else
	liste_tri_tmp[type_ope->no_compte] = g_slist_append ( liste_tri_tmp[type_ope->no_compte],
							      GINT_TO_POINTER ( -type_ope->no_type ));
    }

  remplit_liste_tri_par_type ( type_ope->no_compte );



}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void modification_type_par_defaut ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );


  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_choix_defaut )))
    {
      /* on vient de choisir ce type par défaut */

      GSList *liste_tmp;
      struct struct_type_ope *type_ope_defaut;
      GtkCTreeNode *node_defaut;

      
      liste_tmp = liste_tmp_types[type_ope->no_compte];


      if ( type_ope->signe_type == 2 )
	{
	  /* s'il y avait déjà un défaut, on vire la croix à côté de celui ci */

	  if ( type_defaut_credit[type_ope->no_compte] )
	    {
	      type_ope_defaut = g_slist_find_custom ( liste_tmp,
						      GINT_TO_POINTER (type_defaut_credit[type_ope->no_compte]),
						      (GCompareFunc) recherche_type_ope_par_no ) -> data;

	      node_defaut = gtk_ctree_find_by_row_data ( GTK_CTREE ( arbre_types_operations ),
							 GTK_CTREE_ROW ( node ) -> parent,
							 type_ope_defaut );
	      gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
					node_defaut,
					1,
					"" );


	    }

	  type_defaut_credit[type_ope->no_compte] = type_ope->no_type;
	  gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
				    node,
				    1,
				    "x" );
	}
      else
	{
	  /* s'il y avait déjà un défaut, on vire la croix à côté de celui ci */

	  if ( type_defaut_debit[type_ope->no_compte] )
	    {
	      type_ope_defaut = g_slist_find_custom ( liste_tmp,
						      GINT_TO_POINTER (type_defaut_debit[type_ope->no_compte]),
						      (GCompareFunc) recherche_type_ope_par_no ) -> data;

	      node_defaut = gtk_ctree_find_by_row_data ( GTK_CTREE ( arbre_types_operations ),
							 GTK_CTREE_ROW ( node ) -> parent,
							 type_ope_defaut );
	      gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
					node_defaut,
					1,
					"" );


	    }

	  type_defaut_debit[type_ope->no_compte] = type_ope->no_type;
	  gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
				    node,
				      1,
				    "x" );
	}
    }
  else
    {
      /* on retire ce type du défaut */

      if ( type_ope->signe_type == 2 )
	{
	  type_defaut_credit[type_ope->no_compte] = 0;
	  gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
				    node,
				    1,
				    "" );
	}
      else
	if ( type_ope->signe_type == 1 )
	  {
	    type_defaut_debit[type_ope->no_compte] = 0;
	    gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ),
				      node,
				      1,
				      "" );
	  }
    }
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
gint recherche_type_ope_par_no ( struct struct_type_ope *type_ope,
				 gint *no_type )
{

  return ( !(type_ope->no_type == GPOINTER_TO_INT(no_type)) );

}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void ajouter_type_operation ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node_banque;
  GtkCTreeNode *nouveau_node;
  gint no_compte;
  gchar *ligne[2];


  node_banque = GTK_CLIST ( arbre_types_operations ) -> selection -> data;

  /* on remonte jusqu'au node de la banque */

  while ( GTK_CTREE_ROW ( node_banque ) -> level != 1 )
    node_banque = GTK_CTREE_ROW ( node_banque ) -> parent;

  no_compte = GPOINTER_TO_INT (gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
							     node_banque ));

  type_ope = malloc ( sizeof ( struct struct_type_ope ));

  if ( liste_tmp_types[no_compte] )
    type_ope -> no_type = ((struct struct_type_ope *)(g_slist_last ( liste_tmp_types[no_compte] )->data))->no_type + 1;
  else
    type_ope -> no_type = 1;

  type_ope -> nom_type = g_strdup ( _("Nouveau") );
  type_ope -> signe_type = 0;
  type_ope -> affiche_entree = 0;
  type_ope -> numerotation_auto = 0;
  type_ope -> no_en_cours = 0;
  type_ope -> no_compte = no_compte;

  liste_tmp_types[no_compte] = g_slist_append ( liste_tmp_types[no_compte],
						type_ope );

  ligne[0] = type_ope -> nom_type;
  ligne[1] = NULL;

  nouveau_node = gtk_ctree_insert_node ( GTK_CTREE ( arbre_types_operations ),
					 node_banque,
					 NULL,
					 ligne,
					 0,
					 NULL, NULL,
					 NULL, NULL,
					 FALSE, FALSE );

  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_types_operations ),
				nouveau_node,
				type_ope );


  /* on ajoute ce type à la liste des tris */

  liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],
					      GINT_TO_POINTER ( type_ope -> no_type ));

  /*   si les neutres doivent être intégrés dans les débits crédits, on ajoute son opposé */

  if ( neutres_inclus_tmp[no_compte] )
    liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],
						GINT_TO_POINTER ( -type_ope -> no_type ));

  remplit_liste_tri_par_type ( no_compte );

  /* on ouvre le node de la banque au cas où celui ci ne le serait pas */

  gtk_ctree_expand ( GTK_CTREE ( arbre_types_operations ),
		     node_banque );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void supprimer_type_operation ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;
  GSList *pointeur_tmp;
  GSList *ope_a_changer;
  gint save_pref;


  /* sera mis à 1 s'il faut sauver les préférences des types */

  save_pref = 0;

  /* récupère le type concerné */

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );


  /*   on fait le tour du compte concerné pour voir si des opés avaient ce type, */
  /*     si oui, on les met dans une liste */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + type_ope -> no_compte;
  pointeur_tmp = LISTE_OPERATIONS;
  ope_a_changer = NULL;

  while ( pointeur_tmp )
    {
      struct structure_operation *operation;

      operation = pointeur_tmp -> data;

      if ( operation -> type_ope == type_ope -> no_type )
	ope_a_changer = g_slist_append ( ope_a_changer,
					 operation );
      pointeur_tmp = pointeur_tmp -> next;
    }

  /*   à ce niveau, soit ope_a_changer est null, et on supprime le type dans la liste_tmp */
  /* donc possibiliter d'annuler */
  /* soit c'est pas nul, et on présente un dialogue qui permet de rappatrier les opés */
  /*     sur cet autre type ; par contre là on ne peut annuler la suppression */

  if ( ope_a_changer )
    {
      /* des opés sont à changer */

      GtkWidget *dialog;
      GtkWidget *label;
      gint resultat;
      GtkWidget * option_menu;
      GtkWidget *separateur;
      GtkWidget *hbox;
      GtkWidget *menu;
      gint nouveau_type;

      dialog = gnome_dialog_new ( _( "Suppression d'un type d'opération" ),
				  GNOME_STOCK_BUTTON_OK,
				  GNOME_STOCK_BUTTON_CANCEL,
				  NULL );

      label = gtk_label_new ( _( "Des opérations sont encore affectées au type sélectionné,\ncette suppression sera donc irréversible et les modifications concernant\nles types d'opérations seront enregistrées." ));
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      separateur = gtk_hseparator_new ();
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   separateur,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( separateur );

      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( hbox );



      label = gtk_label_new ( _( "Transférer les opérations sur " ));
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      /* on va mettre ici le bouton des type de la liste tmp car on peut déjà avoir */
      /* ajouté ou retiré des types */

      option_menu = gtk_option_menu_new ();
      menu = gtk_menu_new ();


      pointeur_tmp = liste_tmp_types[type_ope->no_compte];

      while ( pointeur_tmp )
	{
	  struct struct_type_ope *type;
	  GtkWidget *menu_item;

	  type = pointeur_tmp -> data;

	  if ( type -> no_type != type_ope -> no_type
	       &&
	       ( type -> signe_type == type_ope -> signe_type
		 ||
		 !type -> signe_type ))
	    {
	      menu_item = gtk_menu_item_new_with_label ( type -> nom_type );
	      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				    "no_type",
				    GINT_TO_POINTER ( type -> no_type ));
	      gtk_menu_append ( GTK_MENU ( menu ),
				menu_item );
	      gtk_widget_show ( menu_item );
	    }
	  pointeur_tmp = pointeur_tmp -> next;
	}

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ),
				 menu );
      gtk_widget_show ( menu );

      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   option_menu,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( option_menu );

      /*       s'il n'y a aucun autre types, on grise le choix de transfert */

      if ( !GTK_MENU_SHELL ( menu ) -> children )
	gtk_widget_set_sensitive ( hbox,
				   FALSE );

      resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

      if ( resultat )
	{
	  if ( GNOME_IS_DIALOG ( dialog ))
	    gnome_dialog_close ( GNOME_DIALOG ( dialog ));
	  return;
	}

      /* récupération du nouveau type d'opé */

      if ( GTK_MENU_SHELL ( menu ) -> children )
	nouveau_type = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
							       "no_type" ));
      else
	nouveau_type = 0;

      /* on change le type des opés concernées */

      pointeur_tmp = ope_a_changer;

      while ( pointeur_tmp )
	{
	  struct structure_operation *operation;

	  operation = pointeur_tmp -> data;

	  operation -> type_ope = nouveau_type;
	  pointeur_tmp = pointeur_tmp -> next;
	}

      /* on sauvegarde les préf des types */

      save_pref = 1;

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
    }


  /* on vire le type de l'arbre */

  gtk_ctree_remove_node ( GTK_CTREE ( arbre_types_operations ),
			  node );

  /* on retire le no de type dans la liste de tri et on réaffiche la liste */

  liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte],
							GINT_TO_POINTER ( type_ope -> no_type ));

  if ( !type_ope -> signe_type && neutres_inclus_tmp[type_ope->no_compte] )
    liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte],
							  GINT_TO_POINTER ( -type_ope -> no_type ));

  remplit_liste_tri_par_type ( type_ope->no_compte );

  /*   si le type était par défaut, on met le défaut à 0 */

  if ( type_ope -> signe_type == 1
       &&
       type_defaut_debit[type_ope->no_compte] == type_ope -> no_type )
    type_defaut_debit[type_ope->no_compte] = 0;

  if ( type_ope -> signe_type == 2
       &&
       type_defaut_credit[type_ope->no_compte] == type_ope -> no_type )
    type_defaut_credit[type_ope->no_compte] = 0;

  liste_tmp_types[type_ope->no_compte] = g_slist_remove ( liste_tmp_types[type_ope->no_compte],
							  type_ope );
    

  if ( save_pref )
    changement_preferences ( fenetre_preferences,
			     8,
			     NULL );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* Appelée quand on change le tri par date ou par type */
/* rend sensitif ou non la liste des types du tri */
/* ************************************************************************************************************** */

void modif_tri_date_ou_type ( void )
{
  gint no_compte;

  no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_tri_date ),
						      "no_compte" ));

  if ( (tri_tmp[no_compte] = !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_tri_date ))))
    {
      gtk_widget_set_sensitive ( bouton_type_neutre_inclut,
				 TRUE );
      gtk_widget_set_sensitive ( type_liste_tri,
				 TRUE );
    }
  else
    {
      gtk_widget_set_sensitive ( bouton_type_neutre_inclut,
				 FALSE );
      gtk_widget_set_sensitive ( type_liste_tri,
				 FALSE );
    }
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void inclut_exclut_les_neutres ( void )
{
  gint no_compte;
  GSList *liste_tmp;


  no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_tri_date ),
						      "no_compte" ));

  if ( (neutres_inclus_tmp[no_compte] = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_neutre_inclut ))))
    {
      /* on inclut les neutres dans les débits et crédits */
      /*   on fait le tour de tous les types du compte, et pour chaque type neutre, */
      /* on rajoute son numéro en négatif négatif à la liste */

      liste_tmp = liste_tri_tmp[no_compte];

      while ( liste_tmp )
	{
	  struct struct_type_ope *type_ope;

	  if ( GPOINTER_TO_INT ( liste_tmp->data ) > 0 )
	    {
	      type_ope = g_slist_find_custom ( liste_tmp_types[no_compte],
					       liste_tmp->data,
					       (GCompareFunc) recherche_type_ope_par_no ) -> data;

	      if ( !type_ope->signe_type )
		liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],
							    GINT_TO_POINTER ( - GPOINTER_TO_INT ( liste_tmp->data )));

	    }
	  liste_tmp = liste_tmp -> next;
	}
    }
  else
    {
      /* on efface tous les nombres négatifs de la liste */

      liste_tmp = liste_tri_tmp[no_compte];

      while ( liste_tmp )
	{
	  if ( GPOINTER_TO_INT ( liste_tmp->data ) < 0 )
	    {
	      liste_tri_tmp[no_compte] = g_slist_remove ( liste_tri_tmp[no_compte],
							  liste_tmp -> data );
	      liste_tmp = liste_tri_tmp[no_compte];
	    }
	  else
	    liste_tmp = liste_tmp -> next;
	}
    }
   
  remplit_liste_tri_par_type ( no_compte );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void remplit_liste_tri_par_type ( gint no_compte )
{
  GSList *liste_tmp;

  gtk_clist_clear ( GTK_CLIST ( type_liste_tri ));
  deselection_type_liste_tri ();

  liste_tmp = liste_tri_tmp[no_compte];
  
  while ( liste_tmp )
    {
      GSList *liste_tmp2;
      struct struct_type_ope *type_ope;
      gchar *texte[1];
      gint no_ligne;

      liste_tmp2 = g_slist_find_custom ( liste_tmp_types[no_compte],
					 GINT_TO_POINTER ( abs ( GPOINTER_TO_INT ( liste_tmp -> data ))),
					 (GCompareFunc) recherche_type_ope_par_no );

      if ( liste_tmp2 )
	{
	  type_ope = liste_tmp2 -> data;

	  texte[0] = type_ope -> nom_type;

	  if ( type_ope -> signe_type == 1 )
	    texte[0] = g_strconcat ( texte[0],
				     " ( - )",
				     NULL );
	  else
	    if ( type_ope -> signe_type == 2 )
	      texte[0] = g_strconcat ( texte[0],
				       " ( + )",
				       NULL );
	    else
	      if ( neutres_inclus_tmp[no_compte] )
		{
		  /* si c'est un type neutre et qu'ils sont inclus, celui-ci est soit positif soit négatif */
	    
		  if ( GPOINTER_TO_INT ( liste_tmp -> data ) < 0 )
		    texte[0] = g_strconcat ( texte[0],
					     " ( - )",
					     NULL );
		  else
		    texte[0] = g_strconcat ( texte[0],
					     " ( + )",
					     NULL );

		}

	  no_ligne = gtk_clist_append ( GTK_CLIST ( type_liste_tri ),
					texte );

	  gtk_clist_set_row_data ( GTK_CLIST ( type_liste_tri ),
				   no_ligne,
				   liste_tmp -> data );
	}
      liste_tmp = liste_tmp -> next;
    }
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
void selection_type_liste_tri ( void )
{

  /* on rend sensible les boutons de déplacement */

  gtk_widget_set_sensitive ( vbox_fleches_tri,
			     TRUE );

}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deselection_type_liste_tri ( void )
{

  /* on rend non sensible les boutons de déplacement */

  gtk_widget_set_sensitive ( vbox_fleches_tri,
			     FALSE );


}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deplacement_type_tri_haut ( void )
{

  if ( GTK_CLIST ( type_liste_tri ) -> selection -> data )
    {
      gtk_clist_swap_rows ( GTK_CLIST ( type_liste_tri ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ) - 1 );
      save_ordre_liste_type_tri();
    }

}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deplacement_type_tri_bas ( void )
{
  if ( GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ) < GTK_CLIST ( type_liste_tri ) -> rows )
    {
      gtk_clist_swap_rows ( GTK_CLIST ( type_liste_tri ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ) + 1 );
      save_ordre_liste_type_tri();
    }
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* cette fonction est appelée chaque fois qu'on modifie l'ordre de la liste des tris */
/* et elle save cet ordre dans la liste temporaire */
/* ************************************************************************************************************** */

void save_ordre_liste_type_tri ( void )
{
  gint no_compte;
  gint i;

  no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_tri_date ),
						      "no_compte" ));
  g_slist_free ( liste_tri_tmp[no_compte] );
  liste_tri_tmp[no_compte] = NULL;

  for ( i=0 ; i < GTK_CLIST ( type_liste_tri ) -> rows ; i++ )
    liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],
						gtk_clist_get_row_data ( GTK_CLIST ( type_liste_tri ),
									 i ));
}
/* ************************************************************************************************************** */





/* ************************************************************************************************************** */
/* Fonction creation_menu_types */
/* argument : 1 : renvoie un menu de débits */
/* 2 : renvoie un menu de crédits */
/* ou renvoie le tout si c'est désiré dans les paramètres */
/* l'origine est 0 si vient des opérations, 1 si vient des échéances, 2 pour ne pas mettre de signal quand il y a un chgt */
/* ************************************************************************************************************** */

GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine )
{
  GtkWidget *menu;
  GSList *liste_tmp;
  gpointer **save_ptab;

  save_ptab = p_tab_nom_de_compte_variable;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

  /*   s'il n'y a pas de menu, on se barre */

  if ( !(liste_tmp = TYPES_OPES ))
    {
      p_tab_nom_de_compte_variable = save_ptab;
      return ( NULL );
    }

  menu = NULL;

  while ( liste_tmp )
    {
      struct struct_type_ope *type;

      type = liste_tmp -> data;

      if ( type -> signe_type == demande
	   ||
	   !type -> signe_type
	   ||
	   etat.affiche_tous_les_types )
	{
	  GtkWidget *item;

	  /* avant de mettre l'item, on crée le menu si nécessaire */
	  /* le faire ici permet de retourner null si il n'y a rien */
	  /*   dans le menu (sinon, si rien dans les crédits, mais qque */
	  /* chose dans les débits, renvoie un menu vide qui sera affiché */

	  if ( !menu )
	    {
	      menu = gtk_menu_new();
	      
	      /* on associe au menu la valeur 1 pour menu de débit et 2 pour menu de crédit */

	      gtk_object_set_data ( GTK_OBJECT ( menu ),
				    "signe_menu",
				    GINT_TO_POINTER ( demande ) );
	      gtk_widget_show ( menu );
	    }


	  item = gtk_menu_item_new_with_label ( type -> nom_type );

	  if ( !origine )
	    switch ( origine )
	      {
	      case 0:
		gtk_signal_connect_object ( GTK_OBJECT ( item ),
					    "activate",
					    GTK_SIGNAL_FUNC ( changement_choix_type_formulaire ),
					    (GtkObject *) type );
		break;
	      case 1:
		gtk_signal_connect_object ( GTK_OBJECT ( item ),
					    "activate",
					    GTK_SIGNAL_FUNC ( changement_choix_type_echeancier ),
					    (GtkObject *) type );
		break;
	      default:
	      }

	  gtk_object_set_data ( GTK_OBJECT ( item ),
				"adr_type",
				type );
	  gtk_object_set_data ( GTK_OBJECT ( item ),
				"no_type",
				GINT_TO_POINTER ( type -> no_type ));
	  gtk_menu_append ( GTK_MENU ( menu ),
			    item );
	  gtk_widget_show ( item );
	}
      liste_tmp = liste_tmp -> next;
    }

  p_tab_nom_de_compte_variable = save_ptab;
  return ( menu );
}
/* ************************************************************************************************************** */


  
/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type */
/*   argument : le numéro du type demandé */
/* renvoie la place demandée dans l'option menu du formulaire */
/* pour mettre l'history et affiche l'entrée du chq si nécessaire */
/* retourne -1 si pas trouvé */
/* ************************************************************************************************************** */

gint cherche_no_menu_type ( gint demande )
{
  GList *liste_tmp;
  gint retour;
  gint i;

  if ( !demande )
    return ( FALSE );

  liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu ) -> children;
  retour = -1;
  i=0;

  while ( liste_tmp && retour == -1 )
    {
      if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				 "no_type" ) == GINT_TO_POINTER ( demande ))
	{
	  struct struct_type_ope *type;

	  retour = i;

	  /* affiche l'entrée chq du formulaire si nécessaire */

	  type = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				       "adr_type");

	  if ( type -> affiche_entree )
	    gtk_widget_show ( widget_formulaire_operations[10] );
	  else
	    gtk_widget_hide ( widget_formulaire_operations[10] );
	}
      i++;
      liste_tmp = liste_tmp -> next;
    }

  return ( retour );
}
/* ************************************************************************************************************** */



  
/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type_echeancier */
/*   argument : le numéro du type demandé */
/* renvoie la place demandée dans l'option menu du formulaire */
/* pour mettre l'history et affiche l'entrée du chq si nécessaire */
/* retourne -1 si pas trouvé */
/* ************************************************************************************************************** */

gint cherche_no_menu_type_echeancier ( gint demande )
{
  GList *liste_tmp;
  gint retour;
  gint i;


  if ( !demande )
    return ( FALSE );

  liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_echeancier[7] ) -> menu ) -> children;
  retour = -1;
  i = 0;

  while ( liste_tmp && retour == -1 )
    {
      if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				 "no_type" ) == GINT_TO_POINTER ( demande ))
	{
	  struct struct_type_ope *type;

	  retour = i;

	  /* affiche l'entrée chq du formulaire si nécessaire */

	  type = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				       "adr_type");

	  /* soit c'est un type qui affiche l'entrée et qui n'est pas numéroté automatiquement */
	  /* soit c'est un type numéroté auto et c'est une saisie */
 
	  if ( ( type -> affiche_entree && !type -> numerotation_auto)
	       ||
	       ( type -> numerotation_auto && !strcmp ( GTK_LABEL ( label_saisie_modif ) -> label,
							_("Saisie") )))
	    {
	      /* si c'est une saisie, mais le numéro de chq */

	      if ( type -> numerotation_auto )
		{
		  entree_prend_focus ( widget_formulaire_echeancier[8] );
		  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[8] ),
				       itoa ( type -> no_en_cours + 1 ));
		}
	      gtk_widget_show ( widget_formulaire_echeancier[8] );
	    }
	  else
	    gtk_widget_hide ( widget_formulaire_echeancier[8] );
	}
      i++;
      liste_tmp = liste_tmp -> next;
    }

  if ( retour == -1 )
    return ( FALSE );
  else
    return ( retour );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void changement_choix_type_formulaire ( struct struct_type_ope *type )
{

  /* affiche l'entrée de chèque si nécessaire */

  if ( type -> affiche_entree )
    {
      gtk_widget_show ( widget_formulaire_operations[10] );

      /* met le no suivant si nécessaire */

      if ( type -> numerotation_auto )
	{
	  entree_prend_focus ( widget_formulaire_operations[10] );
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[10] ),
			       itoa ( type -> no_en_cours  + 1));
	}
      else
	{
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[10] ),
			       "" );
	  entree_perd_focus ( widget_formulaire_operations[10],
			      FALSE,
			      GINT_TO_POINTER ( 10 ));
	}
    }
  else
    gtk_widget_hide ( widget_formulaire_operations[10] );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void changement_choix_type_echeancier ( struct struct_type_ope *type )
{

  /* affiche l'entrée de chèque si nécessaire */

  if ( ( type -> affiche_entree && !type -> numerotation_auto )
       ||
       ( type -> numerotation_auto && !strcmp ( GTK_LABEL ( label_saisie_modif ) -> label,
						_("Saisie") )))
    {
      /* si c'est une saisie, met le numéro de chq */
      
      if ( type -> numerotation_auto )
	{
	  entree_prend_focus ( widget_formulaire_echeancier[8] );
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[8] ),
			       itoa ( type -> no_en_cours + 1 ));
	}
      gtk_widget_show ( widget_formulaire_echeancier[8] );
    }
  else
    gtk_widget_hide ( widget_formulaire_echeancier[8] );
}
/* ************************************************************************************************************** */
