/* Ce fichier s'occupe de la gestion des types d'opérations */
/* type_operations.c */

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


/* Columns for payment methods tree */
enum payment_methods_columns {
  PAYMENT_METHODS_NAME_COLUMN = 0,
  PAYMENT_METHODS_NUMBERING_COLUMN,
  PAYMENT_METHODS_DEFAULT_COLUMN,
  PAYMENT_METHODS_ACTIVABLE_COLUMN,
  PAYMENT_METHODS_VISIBLE_COLUMN,
  NUM_PAYMENT_METHODS_COLUMNS,
};



static void
item_toggled (GtkCellRendererToggle *cell,
	      gchar                 *path_str,
	      gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter;
  gboolean toggle_item;

  gint *column;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 
		      PAYMENT_METHODS_DEFAULT_COLUMN, &toggle_item, 
		      -1);

  /* do something with the value */
  toggle_item ^= 1;

  /* set new value */
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
		      PAYMENT_METHODS_DEFAULT_COLUMN, toggle_item, 
		      -1);

  /* clean up */
  gtk_tree_path_free (path);
}



/**
 * Creates the "Payment methods" tab.  It uses a nice GtkTreeView.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_types_operations ( void )
{
  GtkWidget *vbox_pref, *hbox, *scrolled_window, *paddingbox;
  GtkWidget *vbox, *table, *menu, *item, *label, *bouton;
  GtkWidget *treeview;
  GtkTreeStore *model;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkTreeIter account_iter, debit_iter, credit_iter, child_iter;
  gint i, col_offset;

  vbox_pref = new_vbox_with_title_and_icon ( _("Reconciliation"),
					     "reconciliation.png" );

  /* Copy lists so that we work on temporary lists. */
  liste_tmp_types = malloc ( nb_comptes * sizeof (gpointer));
  type_defaut_debit = malloc ( nb_comptes * sizeof (gint));
  type_defaut_credit = malloc ( nb_comptes * sizeof (gint));
  liste_tri_tmp = malloc ( nb_comptes * sizeof (gpointer));
  tri_tmp = malloc ( nb_comptes * sizeof (gint));
  neutres_inclus_tmp = malloc ( nb_comptes * sizeof (gint));

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i = 0 ; i < nb_comptes ; i++ )
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

  /* Now we have a model, create view */
  vbox_pref = new_vbox_with_title_and_icon ( _("Payment methods"),
					     "payment-methods.png" );

  /* Known payment methods */
  paddingbox = new_paddingbox_with_title (vbox_pref, TRUE,
					  _("Known payment methods"));
  hbox = gtk_hbox_new ( FALSE, 6 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       TRUE, TRUE, 0 );


  /* Create tree */
  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					GTK_SHADOW_ETCHED_IN );
  gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
		       TRUE, TRUE, 0 );

  /* Create tree view */
  model = gtk_tree_store_new (NUM_PAYMENT_METHODS_COLUMNS,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_BOOLEAN,
			      G_TYPE_BOOLEAN,
			      G_TYPE_BOOLEAN);
  treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (model) );
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
/*   g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),  */
/* 		    "changed",  */
/* 		    G_CALLBACK (select_currency_in_iso_list), */
/* 		    model); */

  cell = gtk_cell_renderer_text_new ();
  col_offset = 
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						 -1, _("Accounts"),
						 cell, "text",
						 PAYMENT_METHODS_NAME_COLUMN,
						 NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  cell = gtk_cell_renderer_text_new ();
  col_offset = 
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						 -1, _("Numbering"),
						 cell, "text",
						 PAYMENT_METHODS_NUMBERING_COLUMN,
						 NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  cell = gtk_cell_renderer_toggle_new ();
  g_signal_connect (cell, "toggled", G_CALLBACK (item_toggled), model);
  gtk_tree_view_column_set_sizing ( cell, GTK_TREE_VIEW_COLUMN_AUTOSIZE );
  gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), TRUE );
  col_offset = 
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						 -1, _("Default"), cell, 
						 "active", PAYMENT_METHODS_DEFAULT_COLUMN,
						 "activatable", PAYMENT_METHODS_ACTIVABLE_COLUMN,
						 "visible", PAYMENT_METHODS_VISIBLE_COLUMN,
						 NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  /* Sort columns accordingly */
/*   gtk_tree_sortable_set_default_sort_func (model, sort_tree, NULL, NULL); */
/*   gtk_tree_sortable_set_sort_func (model, COUNTRY_NAME_COLUMN, sort_tree, NULL, NULL); */
/*   gtk_tree_sortable_set_sort_column_id (model, COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING); */

  /* expand all rows after the treeview widget has been realized */
  g_signal_connect (treeview, "realize",
		    G_CALLBACK (gtk_tree_view_expand_all), NULL);
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      treeview );

  /* Fill tree, iter over with accounts */
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GtkCTreeNode *node_compte;
      GtkCTreeNode *node_debit;
      GtkCTreeNode *node_credit;
      gchar *ligne[2];
      GSList *liste_tmp;

      gtk_tree_store_append (model, &account_iter, NULL);
      gtk_tree_store_set (model, &account_iter,
			  PAYMENT_METHODS_NAME_COLUMN, NOM_DU_COMPTE,
			  PAYMENT_METHODS_NUMBERING_COLUMN, FALSE,
			  PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			  PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			  -1 );

      /* Create the "Debit" node */
      gtk_tree_store_append (model, &debit_iter, &account_iter);
      gtk_tree_store_set (model, &debit_iter,
			  PAYMENT_METHODS_NAME_COLUMN, _("Debit"),
			  PAYMENT_METHODS_NUMBERING_COLUMN, FALSE,
			  PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			  PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			  -1 );

      /* Create the "Debit" node */
      gtk_tree_store_append (model, &credit_iter, &account_iter);
      gtk_tree_store_set (model, &credit_iter,
			  PAYMENT_METHODS_NAME_COLUMN, _("Credit"),
			  PAYMENT_METHODS_NUMBERING_COLUMN, FALSE,
			  PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			  PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			  -1 );


      /* Iter over account payment methods */
      liste_tmp = liste_tmp_types[i];

      while ( liste_tmp )
	{
	  struct struct_type_ope *type_ope;
	  GtkTreeIter * parent_iter;
	  GtkTreeIter method_iter;
	  gboolean isdefault;
	  gchar * number;

	  type_ope = liste_tmp->data;

	  if ( type_ope -> no_type == TYPE_DEFAUT_DEBIT
	       ||
	       type_ope -> no_type == TYPE_DEFAUT_CREDIT )
	    isdefault = 1;
	  else
	    isdefault = 0;

	  if ( !type_ope -> signe_type )
	    parent_iter = &account_iter;
	  else
	    if ( type_ope -> signe_type == 1 )
	      parent_iter = &debit_iter;
	    else
	      parent_iter = &credit_iter;

	  if ( type_ope -> numerotation_auto )
	    {
	      number = itoa ( type_ope -> no_en_cours );
	    }
	  else
	    {
	      number = "";
	    }

	  /* Insert a child node */
	  gtk_tree_store_append (model, &method_iter, parent_iter);
	  gtk_tree_store_set (model, &method_iter,
			      PAYMENT_METHODS_NAME_COLUMN, type_ope -> nom_type,
			      PAYMENT_METHODS_NUMBERING_COLUMN, number,
			      PAYMENT_METHODS_DEFAULT_COLUMN, isdefault,
			      PAYMENT_METHODS_ACTIVABLE_COLUMN, TRUE, 
			      PAYMENT_METHODS_VISIBLE_COLUMN, TRUE, 
			      -1 );

	  liste_tmp = liste_tmp -> next;
	}

      p_tab_nom_de_compte_variable++;
    }


  /* Create "Add" & "Remove" buttons */
  vbox = gtk_vbox_new ( FALSE, 6 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
		       FALSE, FALSE, 0 );

  /* "Add payment method" button */
  bouton_ajouter_type = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_type ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_ajouter_type, FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_type ),
		       "clicked",
		       (GtkSignalFunc ) ajouter_type_operation,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_ajouter_type,
		       TRUE, FALSE, 5 );

  /* "Remove payment method" button */
  bouton_retirer_type = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  gtk_button_set_relief ( GTK_BUTTON ( bouton_retirer_type ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_retirer_type, FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_retirer_type ),
		       "clicked",
		       (GtkSignalFunc ) supprimer_type_operation,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_retirer_type,
		       TRUE, FALSE, 5 );

  /* Payment method details */
  paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					  _("Payment method details"));
  gtk_widget_set_sensitive ( paddingbox, FALSE );

  /* Payment method name */
  table = gtk_table_new ( 2, 2, FALSE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
  gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
		       TRUE, TRUE, 6 );

  label = gtk_label_new ( COLON(_("Name")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 0, 1,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );
  entree_type_nom = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_type_nom ), /* FIXME: check this*/
		       "changed",
		       GTK_SIGNAL_FUNC ( modification_entree_nom_type ),
		       NULL );
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_type_nom, 1, 2, 0, 1,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );

  /* Automatic numbering */
  label = gtk_label_new ( COLON(_("Automatic numbering")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 1, 2,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );
  entree_type_dernier_no = gtk_entry_new ();
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_type_dernier_no, 1, 2, 1, 2,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );
  gtk_signal_connect ( GTK_OBJECT (entree_type_dernier_no  ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modification_entree_type_dernier_no ),
		       NULL );

  /* Payment method type */
  label = gtk_label_new ( COLON(_("Type")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 2, 3,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );

  /* Create menu */
  bouton_signe_type = gtk_option_menu_new ();
  menu = gtk_menu_new();
  /* Neutral type */
  item = gtk_menu_item_new_with_label ( _("Neutral") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      NULL );
  gtk_menu_append ( GTK_MENU ( menu ), item );
  /* Debit type */
  item = gtk_menu_item_new_with_label ( _("Debit") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ), item );
  /* Credit type */
  item = gtk_menu_item_new_with_label ( _("Credit") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      GINT_TO_POINTER (2) );
  gtk_menu_append ( GTK_MENU ( menu ), item );
  /* Set menu */
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_signe_type ), menu );
  gtk_table_attach ( GTK_TABLE ( table ),
		     bouton_signe_type, 1, 2, 2, 3,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );

  /* FIXME: put this in the treeview */
/*   bouton_type_choix_defaut = gtk_check_button_new_with_label ( _("Default") ); */
/*   gtk_signal_connect_object ( GTK_OBJECT ( bouton_type_choix_defaut ), */
/* 			      "toggled", */
/* 			      GTK_SIGNAL_FUNC ( modification_type_par_defaut ), */
/* 			      NULL ); */

  /* Trap list changes */
  gtk_signal_connect ( GTK_OBJECT ( arbre_types_operations ),
		       "tree-select-row",
		       GTK_SIGNAL_FUNC ( selection_ligne_arbre_types ),
		       paddingbox );
  gtk_signal_connect ( GTK_OBJECT ( arbre_types_operations ),
		       "tree-unselect-row",
		       GTK_SIGNAL_FUNC ( deselection_ligne_arbre_types ),
		       paddingbox );

  return ( vbox_pref );
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

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_type_neutre_inclut ),
				     inclut_exclut_les_neutres,
				     NULL );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_tri_type ),
				 tri_tmp[no_compte] );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_neutre_inclut ),
				 neutres_inclus_tmp[no_compte] );
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
				     modif_tri_date_ou_type,
				     NULL );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_tri_date ),
				 TRUE );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_type_tri_date ),
				       modif_tri_date_ou_type,
				       NULL );

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
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_type_nom ),
				       modification_entree_nom_type,
				       NULL );

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_type_dernier_no ),
				     modification_entree_type_dernier_no,
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_type_dernier_no ), "" );
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
			    g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_type_nom ))));

  /* et on le sauve dans les types tmp */

  type_ope -> nom_type = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_type_nom ))));

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

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_numerotation_automatique ),
				 FALSE );
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

  type_ope -> no_en_cours = atoi ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_type_dernier_no ))));

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

  type_ope -> nom_type = g_strdup ( _("New") );
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

      dialog = gnome_dialog_new ( _("Delete a method of payment"),
				  GNOME_STOCK_BUTTON_OK,
				  GNOME_STOCK_BUTTON_CANCEL,
				  NULL );

      label = gtk_label_new ( _("Some transactions are still registered with this method of payment,\nthough this deletion is irreversible. The changes about the method\nof payment will be registered."));
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



      label = gtk_label_new ( POSTSPACIFY(_("Move the transactions to")));
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
	      gtk_object_set_data ( GTK_OBJECT ( menu ),
				    "no_compte",
				    GINT_TO_POINTER ( compte ) );
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
/* Fonction cherche_no_menu_type_associe */
/*   argument : le numéro du type demandé */
/* renvoie la place demandée dans l'option menu du formulaire du type associé */
/* retourne -1 si pas trouvé */
/* origine = 0 pour les opérations */
/* origine = 1 pour les ventilations */
/* ************************************************************************************************************** */

gint cherche_no_menu_type_associe ( gint demande,
				    gint origine )
{
  GList *liste_tmp;
  gint retour;
  gint i;

  if ( !demande )
    return ( FALSE );

  if ( origine )
    liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_ventilation[5] ) -> menu ) -> children;
  else
    liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ) -> menu ) -> children;

  retour = -1;
  i=0;

  while ( liste_tmp && retour == -1 )
    {
      if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				 "no_type" ) == GINT_TO_POINTER ( demande ))
	retour = i;

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
							_("Input") )))
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
						_("Input") )))
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
