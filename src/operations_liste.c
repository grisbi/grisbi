/* ************************************************************************** */
/*  Fichier qui gère la liste des opérations                                  */
/* 			liste_operations.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004      Benjamin Drieu (bdrieu@april.org) 	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org) 	      */
/*			http://www.grisbi.org   			      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "constants.h"
#include "operations_liste.h"



#include "accueil.h"
#include "barre_outils.h"
#include "categories_onglet.h"
#include "comptes_traitements.h"
#include "devises.h"
#include "dialog.h"
#include "echeancier_formulaire.h"
#include "echeancier_liste.h"
#include "equilibrage.h"
#include "exercice.h"
#include "imputation_budgetaire.h"
#include "operations_classement.h"
#include "operations_formulaire.h"
#include "search_glist.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "type_operations.h"
#include "utils.h"
#include "ventilation.h"
#include "constants.h"





#define TRANSACTION_COL_NB_CHECK 0
#define TRANSACTION_COL_NB_DATE 1
#define TRANSACTION_COL_NB_PARTY 2
#define TRANSACTION_COL_NB_PR 3
#define TRANSACTION_COL_NB_DEBIT 4
#define TRANSACTION_COL_NB_CREDIT 5
#define TRANSACTION_COL_NB_BALANCE 6

#define TRANSACTION_LIST_ROWS_NB 4

GtkJustification col_justs[] = { GTK_JUSTIFY_CENTER,
    GTK_JUSTIFY_CENTER,
    GTK_JUSTIFY_LEFT,
    GTK_JUSTIFY_CENTER,
    GTK_JUSTIFY_RIGHT,
    GTK_JUSTIFY_RIGHT,
    GTK_JUSTIFY_RIGHT };

/* tree_view qui affiche les listes d'opé ou les opés ventilées */

GtkWidget *tree_view_listes_operations;

/* les colonnes de la liste des opés */

GtkTreeViewColumn *colonnes_liste_opes[7];

/* les colonnes de la liste des ventils */

GtkTreeViewColumn *colonnes_liste_ventils[3];

/* liste des gtk_list_store, 1 store par compte, à NULL si non initialisé */

GSList *list_store_comptes;

/* hauteur d'une ligne de la liste des opés */

gint hauteur_ligne_liste_opes;

/* on va essayer de créer un object tooltip général pour grisbi */
/* donc associer tous les autres tooltips à ce tooltip (FIXME) */

GtkTooltips *tooltips_general_grisbi;

/* le GdkGC correspondant aux lignes, créés au début une fois pour toute */

GdkGC *gc_separateur_operation;

/* utilisé pour éviter que ça s'emballe lors du réglage de la largeur des colonnes */

gint allocation_precedente;

GtkTreeViewColumn *colonne_classement_tmp;


extern struct operation_echeance *echeance_selectionnnee;
extern gint no_derniere_echeance;
extern GSList *liste_struct_echeances; 
extern gint nb_echeances;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_selection;
extern PangoFontDescription *pango_desc_fonte_liste;
extern GSList *liste_labels_titres_colonnes_liste_ope;
extern GtkWidget *widget_formulaire_ventilation[8];
extern gint ligne_selectionnee_ventilation;
extern GtkWidget *label_equilibrage_pointe;
extern GtkWidget *label_equilibrage_ecart;
extern gdouble operations_pointees;
extern gdouble solde_initial;
extern gdouble solde_final;
extern GtkWidget *bouton_ok_equilibrage;





/******************************************************************************/
/*  Routine qui crée la fenêtre de la liste des opé  */
/******************************************************************************/
GtkWidget *creation_fenetre_operations ( void )
{
    GtkWidget *win_operations;
    GtkWidget *solde_box;
    GtkWidget *frame;

    /*   la fenetre des opé est une vbox : la liste en haut, le solde et  */
    /*     des boutons de conf au milieu, le formulaire en bas */

/* FIXME: à virer sur l'instable */
    allocation_precedente = 0;

    win_operations = gtk_vbox_new ( FALSE,
				    5 );
    gtk_signal_connect ( GTK_OBJECT ( win_operations ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( traitement_clavier_liste ),
			 NULL );

    /* création de la barre d'outils */

    barre_outils = creation_barre_outils ();
    gtk_box_pack_start ( GTK_BOX ( win_operations ),
			 barre_outils,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( barre_outils );

    /* création du notebook des opé */

    notebook_listes_operations = creation_tree_view_operations ();
    gtk_box_pack_start ( GTK_BOX ( win_operations ),
			 notebook_listes_operations,
			 TRUE,
			 TRUE,
			 0);
    gtk_widget_show ( notebook_listes_operations );

    /*   création de la ligne contenant le solde ( sous la liste des opérations ) et les boutons */

    solde_box = gtk_table_new ( 1,
				3,
				TRUE);

    gtk_box_pack_start ( GTK_BOX ( win_operations ),
			 solde_box,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( solde_box );


    /* mise en forme du solde pointé */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_ETCHED_OUT );
    gtk_table_attach ( GTK_TABLE ( solde_box ),
		       frame,
		       0, 1,
		       0, 1,
		       GTK_EXPAND | GTK_SHRINK,
		       FALSE,
		       0, 0);
    gtk_widget_show ( frame );


    solde_label_pointe = gtk_label_new ( SPACIFY(COLON(_("Reconciled balance"))) );
    gtk_label_set_justify ( GTK_LABEL ( solde_label_pointe ),
			    GTK_JUSTIFY_LEFT);
    gtk_container_add ( GTK_CONTAINER ( frame ),
			solde_label_pointe );
    gtk_widget_show ( solde_label_pointe );


    /* mise en forme du solde courant */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_ETCHED_OUT );
    gtk_table_attach ( GTK_TABLE ( solde_box ),
		       frame,
		       2, 3,
		       0, 1,
		       GTK_EXPAND | GTK_SHRINK,
		       FALSE,
		       0, 0);
    gtk_widget_show ( frame );


    solde_label = gtk_label_new ( SPACIFY(COLON(_("Curent balance"))) );
    gtk_label_set_justify ( GTK_LABEL ( solde_label ),
			    GTK_JUSTIFY_RIGHT);
    gtk_container_add ( GTK_CONTAINER ( frame ),
			solde_label );
    gtk_widget_show ( solde_label );

    return ( win_operations );
}
/******************************************************************************/

/******************************************************************************/
/* Création du treeview des opérations					      */
/* y ajoute les colonnes de la liste des opés */
/* retour le treeviw */
/******************************************************************************/
GtkWidget *creation_tree_view_operations ( void )
{
    GtkWidget *scrolled_window;

    /*     nouveauté de gtk 2: on ne met plus d'onglets, on a différents gtk_list_store */
    /* 	mais un seul gtk_tree_view */
    /* donc remplacement de notebook_listes_operations par tree_view_listes_operations */

    scrolled_window = gtk_scrolled_window_new ( NULL,
						NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );

    /*     création proprement dite du treeview */

    tree_view_listes_operations = gtk_tree_view_new ();
    g_signal_connect_after ( G_OBJECT ( tree_view_listes_operations ),
			     "expose-event",
			     G_CALLBACK ( affichage_traits_liste_operation ),
			     NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			tree_view_listes_operations );
    gtk_widget_show ( tree_view_listes_operations );

    /*     on peut sélectionner plusieurs lignes */

    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection ( GTK_TREE_VIEW( tree_view_listes_operations ))),
				  GTK_SELECTION_MULTIPLE );

    /* vérifie le simple ou double click */

    g_signal_connect ( G_OBJECT ( tree_view_listes_operations ),
		       "button_press_event",
		       G_CALLBACK ( selectionne_ligne_souris ),
		       NULL );

    /* vérifie la touche entrée, haut et bas */

    g_signal_connect ( G_OBJECT ( tree_view_listes_operations ),
		       "key_press_event",
		       G_CALLBACK ( traitement_clavier_liste ),
		       NULL );

    /*     ajuste les colonnes si modification de la taille */

    g_signal_connect ( G_OBJECT ( tree_view_listes_operations ),
		       "size-allocate",
		       G_CALLBACK ( changement_taille_liste_ope ),
		       NULL );

    /*     la hauteur est mise à 0 pour l'instant, elle sera remplie dès que nécessaire */

    hauteur_ligne_liste_opes = 0;
    gc_separateur_operation = NULL;

    /*     normalement les colonnes sont déjà créés */
    /* mais bon, on teste, sait on jamais... */

    if ( colonnes_liste_opes[0] )
    {
	gint i;

	for ( i=0 ; i<7 ; i++ )
	{
	    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view_listes_operations ),
					  GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] ));
	    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] ),
						 TRUE );
	    g_signal_connect ( G_OBJECT ( colonnes_liste_opes[i] ),
			       "clicked",
			       G_CALLBACK ( click_sur_titre_colonne_operations ),
			       GINT_TO_POINTER (i));

	}

	/* 	on fait le tour des comptes pour associer COLONNE_CLASSEMENT à l'adr de la colonne */

	for ( i=0 ; i<nb_comptes ; i++ )
	{
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
	    COLONNE_CLASSEMENT = colonnes_liste_opes[GPOINTER_TO_INT (COLONNE_CLASSEMENT)];
	}

/* 	on ajoute tout de suite les colonnes de la ventil, mais elles sont invisibles */

	for ( i=0 ; i<3 ; i++ )
	    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view_listes_operations ),
					  colonnes_liste_ventils[i] );
    }
    else
	printf ( "bizarre, les colonnes n'ont pas encore été créés (operation_liste.c)...\n" );

    
    return ( scrolled_window );
}
/******************************************************************************/


/******************************************************************************/
/* cette fonction est appelée pour créer les tree_view_column des listes d'opé et */
/* de ventil */
/******************************************************************************/
void creation_titres_tree_view ( void )
{
    gint i;
    gfloat alignement[] = {
	0.5,
	0.5,
	0.0,
	0.5,
	1.0,
	1.0,
	1.0
    };
    gchar *titres_liste_ventil[] = { 
	_("Category"),
	_("Notes"),
	_("Amount")
    };
    gfloat alignement_ventil[] = {
	0.0,
	0.0,
	1.0
    };


    if ( !titres_colonnes_liste_operations )
	return;

    if ( !tooltips_general_grisbi )
	tooltips_general_grisbi = gtk_tooltips_new ();

    /*     on commence par s'occuper des listes d'opérations */

    for ( i=0 ; i<7 ; i++ )
    {
	colonnes_liste_opes[i] = gtk_tree_view_column_new_with_attributes ( titres_colonnes_liste_operations[i],
									    gtk_cell_renderer_text_new (),
									    "text", i,
									    "background-gdk", 7,
									    "font-desc", 11,
									    NULL );
	gtk_tree_view_column_set_alignment ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] ),
					     alignement[i] );
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] ),
					  GTK_TREE_VIEW_COLUMN_FIXED );

	if ( etat.largeur_auto_colonnes )
	    gtk_tree_view_column_set_resizable ( colonnes_liste_opes[i],
						 FALSE );
	else
	    gtk_tree_view_column_set_resizable ( colonnes_liste_opes[i],
						 TRUE );

    }

    /*     pour la colonne no 6 (le solde), on rajoute le foreground */

    gtk_tree_view_column_add_attribute ( colonnes_liste_opes[6],
					 gtk_tree_view_column_get_cell_renderers ( colonnes_liste_opes[6] ) -> data,
					 "foreground", 8 );

    /*    on crée les titres de la liste de ventilation  */

    for ( i=0 ; i<3 ; i++ )
    {
	colonnes_liste_ventils[i] = gtk_tree_view_column_new_with_attributes ( titres_liste_ventil[i],
									       gtk_cell_renderer_text_new (),
									       "text", i,
									       "background-gdk", 4,
									       "font-desc", 6,
									       NULL );
	gtk_tree_view_column_set_visible ( colonnes_liste_ventils[i],
					   FALSE );
	gtk_tree_view_column_set_alignment ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_ventils[i] ),
					     alignement_ventil[i] );
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_ventils[i] ),
					  GTK_TREE_VIEW_COLUMN_FIXED );
    }
}
/******************************************************************************/


/******************************************************************************/
/* cette fonction est appelée une fois que les titres et tips pour les col */
/* de la liste d'opé ont été créés. donc soit on les crée, soit on les update */
/******************************************************************************/
void update_titres_tree_view ( void )
{
    gint i;

    if ( !titres_colonnes_liste_operations )
	return;

    if ( !colonnes_liste_opes[0] )
	creation_titres_tree_view ();


    /*     on commence par s'occuper des listes d'opérations */

    for ( i=0 ; i<7 ; i++ )
    {
	gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] ),
					 titres_colonnes_liste_operations[i] );

	if ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] )->button )
	{
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] )->button,
				   tips_col_liste_operations[i],
				   tips_col_liste_operations[i] ); 
	}
    }

}
/******************************************************************************/



/******************************************************************************/
/* Création de la liste des opé */
/* cette fonction crée autant de place pour les list_store que de comptes */
/* elle les initialise à NULL et les ajoute dans list_store_comptes */
/******************************************************************************/
void creation_listes_operations ( void )
{
    gint i;

    list_store_comptes = NULL;

    for ( i = 0 ; i < nb_comptes ; i++ )
	list_store_comptes = g_slist_append ( list_store_comptes,
					      NULL );

}
/******************************************************************************/




/******************************************************************************/
/* remplissage du list_store du compte donné en argument */
/* cette fonction efface l'ancienne liste et la remplit avec les données actuelles */
/* et place la sélection où elle était avant */
/******************************************************************************/
void remplissage_liste_operations ( gint compte )
{
    GtkListStore *list_store;
    GtkTreeIter iter;
    struct structure_operation *operation_selectionnee;
    GSList *liste_operations_tmp;
    gint couleur_en_cours;
    gchar *ligne_clist[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
    gint i, j;
    gdouble montant;
    gint ligne_selectionnee;

    if ( DEBUG )
	printf ( "remplissage_liste_operations\n" );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

    /*     on commence par récupérer le list_store correspondant */

    list_store = g_slist_nth_data ( list_store_comptes,
				    compte );

    /*     on récupère l'adr de l'opé sélectionnée pour pouvoir remettre la sélection ensuite */

    if ( LIGNE_SELECTIONNEE == -1
	 ||
	 !list_store )
	operation_selectionnee = GINT_TO_POINTER ( -1 );
    else
	operation_selectionnee = cherche_operation_from_ligne ( LIGNE_SELECTIONNEE );

    /* efface la liste si nécessaire */

    if ( list_store ) 
	gtk_list_store_clear ( GTK_LIST_STORE ( list_store ));
    else
    {
	/* 	structure de la liste : */
	/* 	    col 0 à 6 -> les données */
	/* 	    col 7 -> la couleur du background */
	/* 	    col 8 -> la couleur du solde */
	/*	    col 9 -> adr de l'opération */
	/*	    col 10 -> sauvegarde background quand ligne sélectionnée */
	/*	    col 11 -> contient NULL ou l'adr de la pangofontdescription utilisée */

	list_store = gtk_list_store_new ( 12,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  GDK_TYPE_COLOR,
					  G_TYPE_STRING,
					  G_TYPE_POINTER,
					  GDK_TYPE_COLOR,
					  PANGO_TYPE_FONT_DESCRIPTION );

	g_slist_nth ( list_store_comptes,
		      compte ) -> data = list_store;
    }


    /*   au départ, les soldes courant et pointé sont = au solde init */

    solde_courant_affichage_liste = SOLDE_INIT;
    solde_pointe_affichage_liste = SOLDE_INIT;
    couleur_en_cours = 0;

    /*   si on n'affiche pas les R, il faut faire un premier tour de liste pour ajouter/retirer */
    /*       les montants de tous les R au solde_init. */

    if ( !AFFICHAGE_R )
    {
	liste_operations_tmp = LISTE_OPERATIONS;

	while ( liste_operations_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_operations_tmp -> data;

	    /* si c'est une opé de ventilation, on la saute */

	    if ( !operation -> no_operation_ventilee_associee
		 &&
		 operation -> pointe == 2 )
	    {
		montant = calcule_montant_devise_renvoi ( operation -> montant,
							  DEVISE,
							  operation -> devise,
							  operation -> une_devise_compte_egale_x_devise_ope,
							  operation -> taux_change,
							  operation -> frais_change );

		solde_courant_affichage_liste = solde_courant_affichage_liste + montant;
		solde_pointe_affichage_liste = solde_pointe_affichage_liste + montant;

	    }
	    liste_operations_tmp = liste_operations_tmp -> next;
	}
    }

    /* on commence la boucle : fait le tour de toutes les opérations */
    /* met à jour les solde_courant_affichage_liste et solde_pointe_affichage_liste */
    /* affiche l'opération à l'écran en fonction de l'affichage de R */

    liste_operations_tmp = LISTE_OPERATIONS;
    ligne_selectionnee = 0;

    while ( liste_operations_tmp )
    {
	struct structure_operation *operation;

	operation = liste_operations_tmp -> data;

	/* si c'est une opé de ventilation, ou si elle est relevée mais que l'on ne veut pas les afficher */

	if ( !(operation -> no_operation_ventilee_associee
	       ||
	       ( !AFFICHAGE_R
		 &&
		 operation -> pointe == 2 )))
	{

	    /*  on calcule les soldes courant */

	    montant = calcule_montant_devise_renvoi ( operation -> montant,
						      DEVISE,
						      operation -> devise,
						      operation -> une_devise_compte_egale_x_devise_ope,
						      operation -> taux_change,
						      operation -> frais_change );

	    /*   dans tous les cas, on ajoute ce montant au solde courant */

	    solde_courant_affichage_liste = solde_courant_affichage_liste + montant;

	    /* si l'opé est pointée ou relevée, on ajoute ce montant au solde pointé */

	    if ( operation -> pointe )
		solde_pointe_affichage_liste = solde_pointe_affichage_liste + montant;

	    /* on fait le tour de tab_affichage_ope pour remplir les lignes du tableau */

	    for ( i = 0 ; i < TRANSACTION_LIST_ROWS_NB ; i++ )
	    {
		/* on ne passe que si la ligne doit être affichée */

		if ( !i 
		     ||
		     NB_LIGNES_OPE == 4
		     ||
		     ( ( i == 1 || i == 2 )
		       &&
		       NB_LIGNES_OPE == 3 )
		     ||
		     ( i == 1 && NB_LIGNES_OPE == 2 ))
		{
		    gint ligne_affichee;

		    /*  en fonction de i (la ligne en cours) et NB_LIGNES_OPE, on retrouve la ligne qu'il faut */
		    /* afficher selon les configurations */

		    switch ( NB_LIGNES_OPE )
		    {
			case 1:
			    ligne_affichee = ligne_affichage_une_ligne;
			    break;

			case 2:
			    if ( i )
				ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data );
			    else
				ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data );
			    break;

			case 3:
			    switch ( i )
			    {
				case 0:
				    ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data );
				    break;

				case 1:
				    ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data );
				    break;

				case 2:
				    ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data );
				    break;

				default:
				    ligne_affichee = 0;
			    }
			    break;

			case 4:
			    ligne_affichee = i;
			    break;

			default:
			    ligne_affichee = 0;
		    }


		    for ( j = 0 ; j < TRANSACTION_LIST_COL_NB ; j++ )
			ligne_clist[i][j] = recherche_contenu_cellule ( operation,
									tab_affichage_ope[ligne_affichee][j]  );

		    /* on affiche la ligne composée de TRANSACTION_LIST_COL_NB colonnes */

		    gtk_list_store_append ( list_store,
					    &iter );

		    for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
			gtk_list_store_set ( list_store,
					     &iter,
					     j, ligne_clist[i][j],
					     -1 );

		    /* 		    on met la couleur de la ligne */

		    if ( operation_selectionnee == operation )
		    {
			gtk_list_store_set ( list_store,
					     &iter,
					     7, &couleur_selection,
					     -1 );

			gtk_list_store_set ( list_store,
					     &iter,
					     10, &couleur_fond[couleur_en_cours],
					     -1 );
			
			if ( !ligne_selectionnee )
			    ligne_selectionnee = atoi ( gtk_tree_model_get_string_from_iter ( GTK_TREE_MODEL ( list_store ),
											      &iter ));
		    }
		    else
			gtk_list_store_set ( list_store,
					     &iter,
					     7, &couleur_fond[couleur_en_cours],
					     -1 );


		    /* si le solde est <0, on le met en rouge */

		    if ( solde_courant_affichage_liste <= 0.01 )
			gtk_list_store_set ( list_store,
					     &iter,
					     8, "red",
					     -1 );

		    /* on associe à cette ligne l'adr de la struct de l'opé */

		    gtk_list_store_set ( list_store,
					 &iter,
					 9, operation,
					 -1 );

		    /* 		    si on utilise une fonte perso, c'est ici */

		    if ( etat.utilise_fonte_listes )
			gtk_list_store_set ( list_store,
					     &iter,
					     11, pango_desc_fonte_liste,
					     -1 );
		}
	    }
	    couleur_en_cours = ( couleur_en_cours + 1 ) % 2;
	}
	liste_operations_tmp = liste_operations_tmp -> next;
    }


    /* affiche la ligne blanche du bas */

    for ( j = 0 ; j < NB_LIGNES_OPE ; j++ )
    {
	/* on met à NULL tout les pointeurs */

	gtk_list_store_append ( list_store,
				&iter );

	for ( i=0 ; i<TRANSACTION_LIST_COL_NB ; i++ )
	    gtk_list_store_set ( list_store,
				 &iter,
				 i, NULL,
				 -1 );


	/* 		    on met la couleur de la ligne */

	if ( operation_selectionnee == GINT_TO_POINTER (-1))
	{
	    gtk_list_store_set ( list_store,
				 &iter,
				 7, &couleur_selection,
				 -1 );

	    gtk_list_store_set ( list_store,
				 &iter,
				 10, &couleur_fond[couleur_en_cours],
				 -1 );
	    
	    if ( !ligne_selectionnee )
		ligne_selectionnee = atoi ( gtk_tree_model_get_string_from_iter ( GTK_TREE_MODEL ( list_store ),
										  &iter ));
	}
	else
	    gtk_list_store_set ( list_store,
				 &iter,
				 7, &couleur_fond[couleur_en_cours],
				 -1 );

	/* on met le no d'opération de cette ligne à -1 */

	gtk_list_store_set ( list_store,
			     &iter,
			     9, GINT_TO_POINTER (-1),
			     -1 );
    }

    LIGNE_SELECTIONNEE = ligne_selectionnee;


    /* on enregistre le solde final */

    SOLDE_COURANT = solde_courant_affichage_liste;
    SOLDE_POINTE = solde_pointe_affichage_liste;

    /* on met les soldes en bas */

    gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ),
			 g_strdup_printf ( PRESPACIFY(_("Checked balance: %4.2f %s")),
					   SOLDE_POINTE,
					   devise_name_by_no ( DEVISE )));
    gtk_label_set_text ( GTK_LABEL ( solde_label ),
			 g_strdup_printf ( PRESPACIFY(_("Current balance: %4.2f %s")),
					   SOLDE_COURANT,
					   devise_name_by_no ( DEVISE )));
}
/******************************************************************************/

/******************************************************************************/
/* Fonction recherche_contenu_cellule */
/* prend en argument l'opération concernée */
/* et le numéro de l'argument qu'on veut afficher (tab_affichage_ope) */
/* renvoie la chaine à afficher ou NULL */
/******************************************************************************/
gchar *recherche_contenu_cellule ( struct structure_operation *operation,
				   gint no_affichage )
{
    GSList *liste_tmp;
    gchar *temp;
    gdouble montant;

    switch ( no_affichage )
    {
	case 0:
	    return ( NULL );
	    break;

	    /* mise en forme de la date */

	case 1:
	    /* FIXME: should use locale + strftime for that */
	    return ( g_strdup_printf ("%02d/%02d/%04d", 
				      operation -> jour,
				      operation -> mois,
				      operation -> annee ));
	    break;

	    /* mise en forme de la date de valeur */

	case 2:
	    if ( operation -> jour_bancaire )
		/* FIXME: should use locale + strftime for that */
		return ( g_strdup_printf ("%02d/%02d/%04d",
					  operation -> jour_bancaire,
					  operation -> mois_bancaire,
					  operation -> annee_bancaire ));
	    else
		return ( NULL );
	    break;

	    /* mise en forme du tiers */

	case 3:
	    return ( tiers_name_by_no ( operation -> tiers, TRUE ));
	    break;

	    /* mise en forme de l'ib */

	case 4:

	    temp = ib_name_by_no ( operation -> imputation,
				   operation -> sous_imputation );
	    break;


	    /* mise en forme du débit */
	case 5:
	    if ( operation -> montant < 0 )
	    {
		temp = g_strdup_printf ( "%4.2f", -operation -> montant );

		/* si la devise en cours est différente de celle de l'opé, on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != operation -> devise )
		    devise_operation = devise_par_no ( operation -> devise );

		if ( devise_operation -> no_devise != DEVISE )
		    temp = g_strconcat ( temp,
					 "(",
					 devise_name ( devise_operation ),
					 ")",
					 NULL );

		return ( temp );
	    }
	    else
		return ( NULL );

	    break;

	    /* mise en forme du crédit */
	case 6:
	    if ( operation -> montant >= 0 )
	    {
		temp = g_strdup_printf ( "%4.2f", operation -> montant );

		/* si la devise en cours est différente de celle de l'opé, on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != operation -> devise )
		    devise_operation = devise_par_no ( operation -> devise );

		if ( devise_operation -> no_devise != DEVISE )
		    temp = g_strconcat ( temp,
					 "(",
					 devise_name ( devise_operation ),
					 ")",
					 NULL );

		return ( temp );
	    }
	    else
		return ( NULL );

	    break;

	    /* mise en forme du solde */
	    /* il a déjà été calculé pour l'opération courante, il suffit de le renvoyer */

	case 7:
	    return ( g_strdup_printf ( "%4.2f",
				       solde_courant_affichage_liste ));
	    break;

	    /* mise en forme du montant dans la devise du compte */

	case 8:
	    if ( operation -> devise != DEVISE )
	    {
		/* on doit calculer et afficher le montant de l'opé */

		montant = calcule_montant_devise_renvoi ( operation -> montant,
							  DEVISE,
							  operation -> devise,
							  operation -> une_devise_compte_egale_x_devise_ope,
							  operation -> taux_change,
							  operation -> frais_change );

		return ( g_strdup_printf ( "(%4.2f %s)",
					   montant,
					   devise_name ( devise_compte ) ));
	    }
	    else
		return ( NULL );

	    break;

	    /* mise en forme du moyen de paiement */

	case 9:
	    return ( type_ope_name_by_no ( operation -> type_ope,
					   operation -> no_compte ));
	    break;

	    /* mise en forme du no de rapprochement */

	case 10:
	    return ( rapprochement_name_by_no ( operation -> no_rapprochement ));
	    break;

	    /* mise en place de l'exo */

	case 11:
	    if ( operation -> no_exercice )
	    {
		liste_tmp = g_slist_find_custom ( liste_struct_exercices,
						  GINT_TO_POINTER ( operation -> no_exercice ),
						  (GCompareFunc) recherche_exercice_par_no );

		if (liste_tmp)
		    return ( ((struct struct_exercice *)(liste_tmp->data)) -> nom_exercice );
		else
		    return ( NULL );
	    }
	    else
		return ( NULL );
	    break;

	    /* mise en place des catégories */

	case 12:
	    temp = NULL;

	    if ( operation -> categorie )
		/* c'est une categ : ss categ */
		temp = nom_categ_par_no ( operation -> categorie,
					  operation -> sous_categorie );
	    else
	    {
		if ( operation -> relation_no_operation )
		{
		    /* c'est un virement */

		    if ( operation -> relation_no_compte == -1 )
		    {
			if ( operation -> montant < 0 )
			    temp = _("Transfer to a deleted account");
			else
			    temp = _("Transfer from a deleted account");
		    }
		    else
		    {
			gpointer **save_ptab;

			save_ptab = p_tab_nom_de_compte_variable;
			p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

			if ( operation -> montant < 0 )
			    temp = g_strdup_printf ( _("Transfer to %s"),
						     NOM_DU_COMPTE );
			else
			    temp = g_strdup_printf ( _("Transfer from %s"),
						     NOM_DU_COMPTE );

			p_tab_nom_de_compte_variable = save_ptab;
		    }
		}
		else
		{
		    /* vérification si l'opération est ventilée */

		    if ( operation -> operation_ventilee )
			temp = _("Breakdown of transaction");
		}
	    }
	    return ( temp );

	    break;

	    /* mise en forme R/P */

	case 13:
	    if ( operation -> pointe == 1 )
		return ( _("P") );
	    else
	    {
		if ( operation -> pointe == 2 )
		    return ( _("R") );
		else
		{
		    if ( operation -> pointe == 3 )
			return ( _("T"));
		    else
			return ( NULL );
		}
	    }
	    break;


	    /* mise en place de la pièce comptable */

	case 14:
	    return ( operation -> no_piece_comptable );
	    break;

	    /* mise en forme des notes */

	case 15:
	    return ( operation -> notes );
	    break;

	    /* mise en place de l'info banque/guichet */

	case 16:
	    return ( operation -> info_banque_guichet );
	    break;

	    /* mise en place du no d'opé */

	case 17:
	    return ( itoa ( operation -> no_operation ));
	    break;

	    /* mise en place du no de chèque/virement */

	case 18:

	    if ( operation -> contenu_type )
		return ( g_strconcat ( "(",
				       operation -> contenu_type,
				       ")",
				       NULL ));
	    else
		return ( NULL );
	    break;
    }
    return ( NULL );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction selectionne_ligne_souris */
/* place la sélection sur l'opé clickée */
/******************************************************************************/
gboolean selectionne_ligne_souris ( GtkWidget *tree_view,
				    GdkEventButton *evenement )
{
    gint x, y;
    gint ligne, colonne;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_colonne;

    /* si le click se situe dans les menus, c'est qu'on redimensionne, on fait rien */

    if ( evenement -> window != gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )) )
	return(FALSE);

    /*     si on est en train de faire les ventils, on fait suivre */

    if ( etat.ventilation_en_cours )
	return ( selectionne_ligne_souris_ventilation ( tree_view,
							evenement ));

    /* Récupération des coordonnées de la souris */


    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );

    /*     on récupère le path aux coordonnées */
    /* 	si ce n'est pas une ligne de la liste, on se barre */

    if ( !gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
					  x,
					  y,
					  &path,
					  &tree_colonne,
					  NULL,
					  NULL ))
    {
	/* 	éventuellement, si c'est un clic du bouton droit, on affiche la popup partielle */
	if ( evenement -> button == 3 )
	    popup_transaction_context_menu ( FALSE );

	return (TRUE);
    }

    /* Récupération de la 1ère ligne de l'opération cliquée */

    ligne = atoi ( gtk_tree_path_to_string ( path ));
    ligne = ligne / NB_LIGNES_OPE * NB_LIGNES_OPE;

    selectionne_ligne( compte_courant,
		       ligne );

    /*     si on est sur la ligne blanche et qu'on a fait un clic droit, on met le menu contextuel adapté */

    if ( evenement -> button == 3 )
    {
	struct structure_operation *operation = NULL;
	GtkTreeIter iter;

	gtk_tree_model_get_iter ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ))),
				  &iter,
				  path );

	gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ))),
			     &iter,
			     9, &operation,
			     -1 );

	if ( operation == GINT_TO_POINTER (-1))
	{
	    popup_transaction_context_menu ( FALSE );
	    return(TRUE);
	}
    }

    /*     on récupère la colonne cliquée au cas où on clique sur les P */

    colonne = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
			     tree_colonne );

    /*     si on est en train d'équilibrer, on fait le boulot */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    if ( etat.equilibrage 
	 &&
	 colonne == TRANSACTION_COL_NB_PR 
	 &&
	 !(ligne % NB_LIGNES_OPE) )
	pointe_equilibrage ( ligne );

    /* si on a cliqué sur la colonne P/R alors que la touche CTRL
       est enfoncée, alors on (dé)pointe l'opération */

    if ( ( ( evenement -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
	 &&
	 colonne == TRANSACTION_COL_NB_PR )
	p_press ();

    /*  si on a double-cliqué ou bouton droit sur une opération, c'est ici */

    if ( evenement -> type == GDK_2BUTTON_PRESS )
	edition_operation ();
    else
	if ( evenement -> button == 3 )
	    popup_transaction_context_menu ( TRUE );

    return ( TRUE );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction traitement_clavier_liste */
/* gère le clavier sur la liste des opés */
/******************************************************************************/
gboolean traitement_clavier_liste ( GtkWidget *widget_variable,
				    GdkEventKey *evenement )
{
    if ( etat.ventilation_en_cours )
	return ( traitement_clavier_liste_ventilation ( evenement ));

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    switch ( evenement -> keyval )
    {
	case GDK_Return :		/* entrée */
	case GDK_KP_Enter :

	    edition_operation ();
	    break;

	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    if ( LIGNE_SELECTIONNEE )
		selectionne_ligne ( compte_courant,
				    LIGNE_SELECTIONNEE - NB_LIGNES_OPE );
	    return TRUE;

	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    if ( LIGNE_SELECTIONNEE
		 !=
		 (GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations )))->length - NB_LIGNES_OPE))
		selectionne_ligne ( compte_courant,
				    LIGNE_SELECTIONNEE + NB_LIGNES_OPE );
	    return TRUE;

	case GDK_Delete:		/*  del  */
	    supprime_operation ( cherche_operation_from_ligne (LIGNE_SELECTIONNEE ));
	    return TRUE;

	case GDK_P:			/* touche P */
	case GDK_p:			/* touche p */

	    if ( ( evenement -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		p_press ();
	    break;

	case GDK_r:			/* touche r */
	case GDK_R:			/* touche R */

	    if ( ( evenement -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		r_press ();
	    break;

	default: 
	    return FALSE;
    }

    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
/* Routine qui sélectionne l'opération de la ligne envoyée en argument */
/******************************************************************************/
void selectionne_ligne ( gint compte,
			 gint nouvelle_ligne )
{
    GtkTreeIter iter;
    gint i;
    GdkColor *couleur;

    if ( DEBUG )
	printf ( "selectionne_ligne\n" );

    /*     si on est déjà dessus, on se barre */

    if ( nouvelle_ligne == LIGNE_SELECTIONNEE )
	return;

    /*   vire l'ancienne sélection : consiste à remettre la couleur d'origine du background */

    if ( LIGNE_SELECTIONNEE != -1
	 &&
	 gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
					       &iter,
					       itoa ( LIGNE_SELECTIONNEE )))
    {
	/* 	iter est maintenant positionné sur la 1ère ligne de l'opé à désélectionner */

	for ( i=0 ; i<NB_LIGNES_OPE ; i++ )
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 10, &couleur,
				 -1 );
	    gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 7,couleur,
				 -1 );
	    gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 10, NULL,
				 -1 );

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				       &iter );
	}
    }

    LIGNE_SELECTIONNEE = nouvelle_ligne;

    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
					       &iter,
					       itoa ( LIGNE_SELECTIONNEE )))
    {

	/* 	iter est maintenant positionné sur la 1ère ligne de l'opé à sélectionner */

	for ( i=0 ; i<NB_LIGNES_OPE ; i++ )
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 7, &couleur,
				 -1 );
	    gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 7, &couleur_selection,
				 -1 );
	    gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 10, couleur,
				 -1 );

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				       &iter );
	}
    }

    /*     on déplace le scrolling de la liste si nécessaire pour afficher la sélection */

    ajuste_scrolling_liste_operations_a_selection ( compte_courant );

}
/******************************************************************************/


/******************************************************************************/
void ajuste_scrolling_liste_operations_a_selection ( gint compte )
{
/*     si compte = -1, c'est que l'on est sur les ventilations */

    GtkAdjustment *v_adjustment;
    gint y_ligne;
    gint ligne_selectionnee;
    gint nb_lignes_ope;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;
    
    if ( compte == -1 )
    {
	nb_lignes_ope = 1;
	ligne_selectionnee = ligne_selectionnee_ventilation;
    }
    else
    {
	nb_lignes_ope = NB_LIGNES_OPE;
	ligne_selectionnee = LIGNE_SELECTIONNEE;
    }

    /*     si on n'a pas encore récupéré la hauteur des lignes, on va le faire ici */

    if ( !hauteur_ligne_liste_opes )
	hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_listes_operations );

    v_adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_listes_operations ));

    y_ligne = ligne_selectionnee * hauteur_ligne_liste_opes;

    /*     si l'opé est trop haute, on la rentre et la met en haut */

    if ( y_ligne < v_adjustment -> value )
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				   y_ligne );
    else
	if ( (y_ligne + hauteur_ligne_liste_opes*nb_lignes_ope ) > ( v_adjustment -> value + v_adjustment -> page_size ))
	    gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				       y_ligne + hauteur_ligne_liste_opes*nb_lignes_ope - v_adjustment -> page_size );

}
/******************************************************************************/



/******************************************************************************/
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view )
{
    GdkRectangle rectangle;

    gtk_tree_view_get_background_area ( GTK_TREE_VIEW ( tree_view ),
					gtk_tree_path_new_from_string ( "1" ),
					NULL,
					&rectangle );
    return ( rectangle.height );
}
/******************************************************************************/




/******************************************************************************/
/* renvoie l'adr de l'opération correspondant  à la ligne envoyées */
/* en argument */
/******************************************************************************/
struct structure_operation *cherche_operation_from_ligne ( gint ligne )
{
    GtkTreeIter iter;
    struct structure_operation *operation;

    if ( !gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
					       &iter,
					       itoa (ligne)))
	return NULL;

    gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
			 &iter,
			 9, &operation,
			 -1 );

    return ( operation );

}
/******************************************************************************/




/******************************************************************************/
/* fonction cherche_iter_operation */
/* retrouve l'iter correspondant à l'opération donnée en argument dans la tree_view des opérations */
/* renvoie null si pas trouvé */
/******************************************************************************/
GtkTreeIter *cherche_iter_operation ( struct structure_operation *operation )
{
    struct structure_operation *operation_tmp;
    GtkTreeIter iter;

    if ( !operation )
	return NULL;

    /*     on va faire le tour de la liste, et dès qu'une opé = operation */
    /* 	on retourne son iter */

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				    &iter );
    operation_tmp = NULL;

    do
    {
	gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
			     &iter,
			     9, &operation_tmp,
			     -1 );
    }
    while ( operation_tmp != operation
	    &&
	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				       &iter ));

    if ( operation_tmp == operation )
	return ( gtk_tree_iter_copy ( &iter ));
    else
	return NULL;

}
/******************************************************************************/




/******************************************************************************/
/* Fonction edition_operation */
/* appelée lors d'un double click sur une ligne ou entree */
/* place l'opération sélectionnée dans le formulaire */
/******************************************************************************/
void edition_operation ( void )
{
    struct structure_operation *operation;
    gchar *date ;
    gchar *date_bancaire ;
    GSList *liste_tmp;
    struct struct_devise *devise;
    gchar *char_temp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    operation = cherche_operation_from_ligne (LIGNE_SELECTIONNEE );
    formulaire_a_zero ();

    /* on affiche le formulaire sans modifier l'état => si il n'est pas affiché normalement,
       il sera efface lors du prochain formulaire_a_zero */

    if ( !etat.formulaire_toujours_affiche )
    {
	gtk_widget_show ( frame_droite_bas );
	gtk_signal_connect ( GTK_OBJECT ( frame_droite_bas ),
			     "map",
			     GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_visible ),
			     NULL );
    }

    gtk_widget_set_sensitive ( bouton_affiche_cache_formulaire, FALSE );

    degrise_formulaire_operations ();

    /* si l'opé est -1, c'est que c'est une nouvelle opé */

    if ( operation == GINT_TO_POINTER ( -1 ) )
    {
	if ( gtk_widget_get_style ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ) == style_entree_formulaire[1] )
	{
	    entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_DATE] );

	    if ( gtk_widget_get_style ( widget_formulaire_operations[TRANSACTION_FORM_VALUE_DATE] ) == style_entree_formulaire[1] )
	    {
		gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ),
				     gsb_today() );
	    }
	}
	gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ), 0, -1);
	gtk_widget_grab_focus ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ) );
	return;
    }

    /*   l'opé n'est pas -1, c'est une modif, on remplit les champs */


    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "adr_struct_ope",
			  operation );


    /* on met le no de l'opé */

    gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[TRANSACTION_FORM_OP_NB] ),
			 itoa ( operation -> no_operation ));

    /* mise en forme de la date */

    date = g_strdup_printf ( "%02d/%02d/%04d",
			     g_date_day ( operation -> date ),
			     g_date_month ( operation -> date ),
			     g_date_year ( operation -> date ) );

    entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_DATE] );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ),
			 date );

    /* mise en forme du tiers */

    if ( operation -> tiers )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_PARTY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_PARTY] ),
				tiers_name_by_no ( operation -> tiers, TRUE ));
    }

    /* mise en forme du débit / crédit */

    if ( operation -> montant < 0 )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_DEBIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_DEBIT] ),
			     g_strdup_printf ( "%4.2f", -operation -> montant ));
	/* met le menu des types débits */

	if ( !etat.affiche_tous_les_types )
	{
	    GtkWidget *menu;

	    if ( (menu = creation_menu_types ( 1, compte_courant, 0  )))
	    {
		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
					   menu );
		gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
	    }
	    else
		gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
	}
    }
    else
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_CREDIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_CREDIT] ),
			     g_strdup_printf ( "%4.2f", operation -> montant ));
	/* met le menu des types crédits */

	if ( !etat.affiche_tous_les_types )
	{
	    GtkWidget *menu;

	    if ( (menu = creation_menu_types ( 2, compte_courant, 0 )))
	    {
		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
					   menu );
		gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
	    }
	    else
	    {
		gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
		gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_CHEQUE] );
	    }
	}
    }

    /* si l'opération est relevée, on désensitive les entrées de crédit et débit */

    if ( operation -> pointe == 2 )
    {
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_CREDIT],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_DEBIT],
				   FALSE );
    }

    /* mise en forme de la devise */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_DEVISE] ),
				  g_slist_index ( liste_struct_devises,
						  devise_par_no ( operation -> devise )));

    /*   si la devise n'est pas celle du compte ni l'euro si le compte va y passer, affiche le bouton change */

    if ( !devise_compte
	 ||
	 devise_compte -> no_devise != DEVISE )
	devise_compte = devise_par_no ( DEVISE );

    devise = devise_par_no ( operation -> devise );

    if ( !( devise -> no_devise == DEVISE
	    ||
	    ( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	    ||
	    ( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
	gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_CHANGE] );

    /* mise en forme des catégories */

    char_temp = nom_categ_par_no ( operation -> categorie,
				   operation -> sous_categorie );

    if ( char_temp )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
				char_temp );
    }

    /* mise en forme de la date réelle */

    if ( operation -> date_bancaire )
    {
	date_bancaire = g_strdup_printf ( "%02d/%02d/%04d",
					  g_date_day ( operation -> date_bancaire ),
					  g_date_month ( operation -> date_bancaire ),
					  g_date_year ( operation -> date_bancaire ) );

	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_VALUE_DATE] );

	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_VALUE_DATE] ),
			     date_bancaire );
    }

    /* si l'opération est liée, marque le virement */
    /* et si la contre opération est relevée, on désensitive la categ et le montant */

    if ( operation -> relation_no_operation )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] );

	if ( operation -> relation_no_compte == -1 )
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
				    _("Transfer: deleted account") );
	else
	{
	    GtkWidget *menu;
	    struct structure_operation *operation_2;

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	    /* on met le nom du compte du virement */

	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
				    g_strconcat ( COLON(_("Transfer")),
						  NOM_DU_COMPTE,
						  NULL ));

	    /* si l'opération est relevée, on empêche le changement de virement */

	    if ( operation -> pointe == 2 )
		gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY],
					   FALSE );

	    /* récupération de la contre opération */

	    operation_2 = g_slist_find_custom ( LISTE_OPERATIONS,
						GINT_TO_POINTER ( operation -> relation_no_operation ),
						(GCompareFunc) recherche_operation_par_no ) -> data;

	    /* 	  si la contre opération est relevée, on désensitive les categ et les montants */
	    if ( operation_2 -> pointe == 2 )
	    {
		gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_CREDIT],
					   FALSE );
		gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_DEBIT],
					   FALSE );
		gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY],
					   FALSE );
	    }

	    /* comme c'est un virement, on affiche s'il existe l'option menu du type de l'autre opé */

	    if ( operation -> montant >= 0 )
		menu = creation_menu_types ( 1, operation -> relation_no_compte, 2  );
	    else
		menu = creation_menu_types ( 2, operation -> relation_no_compte, 2  );

	    /*  on ne continue que si un menu a été créé */
	    /*    dans ce cas, on va chercher l'autre opé et retrouve le type */

	    if ( menu )
	    {
		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_CONTRA] ),
					   menu );
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_CONTRA] ),
					      cherche_no_menu_type_associe ( operation_2 -> type_ope,
									     0 ));
		gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_CONTRA] );
	    }
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	}
    }

    /* mise en forme si l'opération est ventilée */

    if ( operation -> operation_ventilee )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
				_("Breakdown of transaction") );
	gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_BREAKDOWN] );
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_EXERCICE],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET],
				   FALSE );

	/* met la liste des opés de ventilation dans liste_adr_ventilation */

	gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			      "liste_adr_ventilation",
			      creation_liste_ope_de_ventil ( operation ));

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
    }

    /* met l'option menu du type d'opé */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ))
    {
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
				      cherche_no_menu_type ( operation -> type_ope ));
	if ( operation -> type_ope
	     &&
	     ( liste_tmp = g_slist_find_custom ( TYPES_OPES,
						 GINT_TO_POINTER ( operation -> type_ope ),
						 (GCompareFunc) recherche_type_ope_par_no )))
	{
	    struct struct_type_ope *type;

	    type = liste_tmp -> data;

	    if ( type -> affiche_entree
		 &&
		 operation -> contenu_type )
	    {
		entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_CHEQUE] );
		gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[TRANSACTION_FORM_CHEQUE] ),
				     operation -> contenu_type );
	    }
	}
    }

    /* met en place l'exercice */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( operation -> no_exercice,
							      widget_formulaire_operations[TRANSACTION_FORM_EXERCICE] ));

    /* met en place l'imputation budgétaire */
    /* si c'est une opé ventilée, on met rien */

    if ( !operation -> operation_ventilee )
    {
	gchar *char_tmp;

	char_tmp = ib_name_by_no ( operation -> imputation,
				   operation -> sous_imputation );
	if ( char_tmp )
	{
	    entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET]);
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET] ),
				    char_tmp );
	}
    }

    /* mise en place de la pièce comptable */

    if ( operation -> no_piece_comptable )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_VOUCHER] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_VOUCHER] ),
			     operation -> no_piece_comptable );
    }

    /* remplit les notes */

    if ( operation -> notes )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_NOTES] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_NOTES] ),
			     operation -> notes );
    }

    /* remplit les infos guichet / banque */

    if ( operation -> info_banque_guichet )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_BANK] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_BANK] ),
			     operation -> info_banque_guichet );
    }

    /* mise en forme de auto / man */

    if ( operation -> auto_man )
	gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[TRANSACTION_FORM_MODE]),
			     _("Auto"));
    else
	gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[TRANSACTION_FORM_MODE]),
			     _("Manual"));

    /*   on a fini de remplir le formulaire, on donne le focus à la date */

    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ),
			      0,
			      -1);
    gtk_widget_grab_focus ( widget_formulaire_operations[TRANSACTION_FORM_DATE] );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction p_press */
/* appelée lorsque la touche p est pressée sur la liste */
/* pointe ou dépointe l'opération courante */
/******************************************************************************/
void p_press (void)
{
    gdouble montant;
    struct structure_operation *operation;
    GtkTreeIter iter;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    operation = cherche_operation_from_ligne ( LIGNE_SELECTIONNEE );

    /* si on est sur l'opération vide -> on se barre */

    if (operation  == GINT_TO_POINTER ( -1 )
	||
	operation -> pointe == 3 )
	return;
printf ( "ça passe\n" );
    gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
					  &iter,
					  itoa ( LIGNE_SELECTIONNEE ));

    if ( operation -> pointe )
    {
	montant = calcule_montant_devise_renvoi ( operation -> montant,
						  DEVISE,
						  operation -> devise,
						  operation -> une_devise_compte_egale_x_devise_ope,
						  operation -> taux_change,
						  operation -> frais_change );

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees - montant;

	SOLDE_POINTE = SOLDE_POINTE - montant;
	operation -> pointe = 0;

	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
			     &iter,
			     3, NULL,
			     -1 );
    }
    else
    {
	montant = calcule_montant_devise_renvoi ( operation -> montant,
						  DEVISE,
						  operation -> devise,
						  operation-> une_devise_compte_egale_x_devise_ope,
						  operation-> taux_change,
						  operation-> frais_change );

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees + montant;

	SOLDE_POINTE = SOLDE_POINTE + montant;
	operation -> pointe = 1;

	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
			     &iter,
			     3, _("P"),
			     -1 );
    }

    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

    if ( operation -> operation_ventilee )
    {
	/* p_tab est déjà pointé sur le compte courant */

	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *ope_fille;

	    ope_fille = liste_tmp -> data;

	    if ( ope_fille -> no_operation_ventilee_associee == operation -> no_operation )
		ope_fille -> pointe = operation -> pointe;

	    liste_tmp = liste_tmp -> next;
	}
    }

    if ( etat.equilibrage )
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
			     g_strdup_printf ("%4.2f",
					      operations_pointees ));

	if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
	{
	    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
				 g_strdup_printf ( "%4.2f",
						   0.0 ));
	    gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				       TRUE );
	}
	else
	{
	    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
				 g_strdup_printf ( "%4.2f",
						   solde_final - solde_initial - operations_pointees ));
	    gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				       FALSE );
	}
    }

    /* p_tab est déjà sur le compte courant */
    /* met le label du solde pointé */

    gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ),
			 g_strdup_printf ( PRESPACIFY(_("Checked balance: %4.2f %s")),
					   SOLDE_POINTE,
					   devise_name_by_no ( DEVISE )) );


    modification_fichier( TRUE );
/* ALAIN-FIXME : solution batarde me semble-t'il pour actualiser le solde pointé
       sur la fenêtre d'accueil après que l'on ait pointé l'opération */

    update_liste_comptes_accueil ();
}
/******************************************************************************/

/******************************************************************************/
/* Fonction r_press */
/* appelée lorsque la touche r est pressée sur la liste */
/* relève ou dérelève l'opération courante */
/******************************************************************************/
void r_press (void)
{
    struct structure_operation *operation;
    GtkTreeIter iter;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    operation = cherche_operation_from_ligne ( LIGNE_SELECTIONNEE );


    /* si on est sur l'opération vide -> on se barre */

    if ( operation == GINT_TO_POINTER ( -1 ))
	return;

    gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
					  &iter,
					  itoa ( LIGNE_SELECTIONNEE ));

    if ( !operation -> pointe )
    {
	/* on relève l'opération */

	operation -> pointe = 3;

	/* on met soit le R, soit on change la sélection vers l'opé suivante */

	if ( AFFICHAGE_R )
	    gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 3, _("R"),
				 -1 );
	else
	{
	    /*  l'opération va disparaitre, on met donc la sélection sur l'opé suivante */

	    LIGNE_SELECTIONNEE++;
 	    remplissage_liste_operations ( compte_courant );
 	}

	modification_fichier( TRUE );
    }
    else
	if ( operation -> pointe == 3 )
	{
	    /* dé-relève l'opération */

	    gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))),
				 &iter,
				 3, NULL,
				 -1 );

	    modification_fichier( TRUE );
	}

    /* si c'est une ventilation, on fait le tour des opérations du compte pour */
    /* rechercher les opérations de ventilation associées à cette ventilation */

    if ( operation -> operation_ventilee )
    {
	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( operation -> no_operation_ventilee_associee == operation -> no_operation )
		operation -> pointe = operation -> pointe;

	    liste_tmp = liste_tmp -> next;
	}
    }
}
/******************************************************************************/

/******************************************************************************/
/*  Routine qui supprime l'opération donnée en argument */
/******************************************************************************/
void supprime_operation ( struct structure_operation *operation )
{
    gint no_compte;
    
    if ( operation == GINT_TO_POINTER ( -1 ) )
	return;

    no_compte = operation -> no_compte;

    /* vérifications de bases */

    /* l'opération ne doit pas être pointée */

    if ( operation -> pointe == 2 )
    {
	dialogue_error ( _("Imposible to delete a reconciled transaction.") );
	return;
    }

    /* si l'opération est liée, on recherche l'autre opération, */
    /* on vire ses liaisons et on l'efface */
    /* sauf si elle est relevée, dans ce cas on annule tout */

    if ( operation -> relation_no_operation && operation -> relation_no_compte != -1 )
    {
	GSList *liste_tmp;
	struct structure_operation *ope_liee;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
					  GINT_TO_POINTER ( operation -> relation_no_operation ),
					  ( GCompareFunc ) recherche_operation_par_no );

	if ( liste_tmp )
	{
	    ope_liee =  liste_tmp -> data;

	    if ( ope_liee -> pointe == 2 )
	    {
		dialogue_error ( _("The contra-transaction of this transfer is reconciled, deletion impossible.") );
		return;
	    }

	    ope_liee -> relation_no_operation = 0;
	    ope_liee -> relation_no_compte = 0;

	    supprime_operation ( ope_liee );
	}
    }

    /* si c'est une ventilation, on fait le tour de ses opérations */
    /* de ventilation pour vérifier qu'il n'y en a pas une qui est un virement */
    /* vers une opération relevée */

    if ( operation -> operation_ventilee )
    {
	GSList *pointeur_tmp;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	pointeur_tmp = LISTE_OPERATIONS;

	while ( pointeur_tmp )
	{
	    struct structure_operation *ope_test;

	    ope_test = pointeur_tmp -> data;

	    if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation )
	    {
		/* ope_test est une opération de ventilation de l'opération */
		/* à supprimer, on recherche si c'est un virement */

		if ( ope_test -> relation_no_operation )
		{
		    /* c'est un virement, on va voir la contre opération */

		    struct structure_operation *contre_operation;

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_test -> relation_no_compte;

		    contre_operation = g_slist_find_custom ( LISTE_OPERATIONS,
							     GINT_TO_POINTER ( ope_test -> relation_no_operation ),
							     (GCompareFunc) recherche_operation_par_no ) -> data;

		    if ( contre_operation -> pointe == 2 )
		    {
			dialogue_error ( _("One of the breakdown lines is a transfer whose contra-transaction is reconciled.  Deletion canceled."));
			return;
		    }
		}
	    }
	    pointeur_tmp = pointeur_tmp -> next;
	}
    }

    /* les tests sont passés, si c'est une ventilation, */
    /* on vire toutes les opérations associées */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

    if ( operation -> operation_ventilee )
    {
	GSList *pointeur_tmp;

	pointeur_tmp = LISTE_OPERATIONS;

	while ( pointeur_tmp )
	{
	    struct structure_operation *ope_test;

	    ope_test = pointeur_tmp -> data;

	    if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation )
	    {
		/* on se place tout de suite sur l'opé suivante */

		pointeur_tmp = pointeur_tmp -> next;

		supprime_operation ( ope_test );
	    }
	    else
		pointeur_tmp = pointeur_tmp -> next;
	}
    }

    /* si la sélection est sur l'opération qu'on supprime, */
    /* on met la sélection sur celle du dessous */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

    if ( cherche_operation_from_ligne ( LIGNE_SELECTIONNEE ) == operation )
    {
	LIGNE_SELECTIONNEE = LIGNE_SELECTIONNEE + NB_LIGNES_OPE;
	selectionne_ligne ( operation -> no_compte,
			    LIGNE_SELECTIONNEE );
    }

    /* supprime l'opération dans la liste des opés */

    LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS,
					operation );
    NB_OPE_COMPTE--;

    /* on met à jour, immédiatement si on a la liste affichée */

    MISE_A_JOUR = 1;

    if ( no_compte == compte_courant )
	verification_mise_a_jour_liste ();

    /* si on est en train d'équilibrer => recalcule le total pointé */

    if ( etat.equilibrage )
    {
	GSList *pointeur_liste_ope;

	p_tab_nom_de_compte_variable =  p_tab_nom_de_compte_courant;

	pointeur_liste_ope = LISTE_OPERATIONS;
	operations_pointees = 0;

	while ( pointeur_liste_ope )
	{
	    struct structure_operation *operation;

	    operation = pointeur_liste_ope -> data;

	    if ( operation -> pointe == 1 )
		operations_pointees = operations_pointees + operation -> montant;

	    pointeur_liste_ope = pointeur_liste_ope -> next;
	}

	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
			     g_strdup_printf ( "%4.2f", 
					       operations_pointees ));

	if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
	{
	    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
				 g_strdup_printf ( "%4.2f",
						   0.0 ));
	    gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				       TRUE );
	}
	else
	{
	    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
				 g_strdup_printf ( "%4.2f",
						   solde_final - solde_initial - operations_pointees ));
	    gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				       FALSE );
	}
    }

    /* on réaffiche la liste de l'état des comptes de l'accueil */

    mise_a_jour_solde ( no_compte );
    update_liste_comptes_accueil ();

    /* on réaffiche la liste des tiers */

    mise_a_jour_tiers ();
    mise_a_jour_categ ();
    mise_a_jour_imputation ();
    modification_fichier( TRUE );
}
/******************************************************************************/


/******************************************************************************/
/* Fonction changement_taille_liste_ope					      */
/* appelée dès que la taille de la clist a changé			      */
/* pour mettre la taille des différentes colonnes			      */
/******************************************************************************/
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation )
{
    gint i;
    gint col0, col1, col2, col3, col4, col5, col6;

    /*     pour éviter que le système ne s'emballe... */

    if ( allocation -> width
	 ==
	 allocation_precedente )
	return FALSE;

    allocation_precedente = allocation -> width;

/*     on sépare ici entre ventilation et liste normale */

    if ( etat.ventilation_en_cours )
    {
	gint categorie;
	gint description;
	gint montant;

	categorie = ( 40 *allocation_precedente ) / 100;
	description = ( 35 * allocation_precedente) / 100;
	montant = ( 10 * allocation_precedente) / 100;

	gtk_tree_view_column_set_fixed_width ( colonnes_liste_ventils[0],
					       categorie );
	gtk_tree_view_column_set_fixed_width ( colonnes_liste_ventils[1],
					       description );
	gtk_tree_view_column_set_fixed_width ( colonnes_liste_ventils[2],
					       montant );


	/* met les entrées du formulaire à la même taille */

	col0 = allocation_precedente * 32  / 100;
	col1 = allocation_precedente * 32  / 100;
	col2 = allocation_precedente * 15  / 100;

	/* 1ère ligne */

	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[0] ),
			       col0,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[1] ),
			       col1,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[2] ),
			       col2,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[3] ),
			       col2,
			       FALSE );

	/* 2ème ligne */

	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[4] ),
			       col0,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[5] ),
			       col1 / 2,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[6] ),
			       col1/2,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[7] ),
			       col2,
			       FALSE );
    }
    else
    {
	/* si la largeur est automatique, on change la largeur des colonnes */
	/* sinon, on y met les valeurs fixes */

	if ( etat.largeur_auto_colonnes )
	    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
		gtk_tree_view_column_set_fixed_width ( colonnes_liste_opes[i],
						       rapport_largeur_colonnes[i] * allocation_precedente / 100 );
	else
	    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
		if ( taille_largeur_colonnes[i] )
		    gtk_tree_view_column_set_fixed_width ( colonnes_liste_opes[i],
							   taille_largeur_colonnes[i]  );

	/* met les entrées du formulaire selon une taille proportionnelle */

	col0 = allocation_precedente * 5 / 100;
	col1 = allocation_precedente * 12 / 100;
	col2 = allocation_precedente * 30 / 100;
	col3 = allocation_precedente * 12 / 100;
	col4 = allocation_precedente * 12 / 100;
	col5 = allocation_precedente * 12 / 100;
	col6 = allocation_precedente * 12 / 100;

	/* 1ère ligne */

	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_OP_NB] ),
			       col0,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ),
			       col1,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_PARTY] ),
			       col2,
			       FALSE  );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_DEBIT] ),
			       col3,
			       FALSE  );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_CREDIT] ),
			       col4,
			       FALSE  );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_DEVISE] ),
			       col5,
			       FALSE  );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_CHANGE] ),
			       col6,
			       FALSE  );

	/* 2ème ligne */

	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_VALUE_DATE] ),
			       col0+col1,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
			       col2,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
			       col3+col4,
			       FALSE  );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_CHEQUE] ),
			       col5,
			       FALSE  );

	/* 3ème ligne */

	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_EXERCICE] ),
			       col0+col1,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET] ),
			       col2,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_VOUCHER] ),
			       col5,
			       FALSE  );

	/* 4ème ligne */

	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_BREAKDOWN] ),
			       col0+col1,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_NOTES] ),
			       col2,
			       FALSE );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_BANK] ),
			       col3+col4+col5,
			       FALSE  );
	gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_MODE] ),
			       col6,
			       FALSE  );
    }
    return ( FALSE );
}
/******************************************************************************/


/******************************************************************************/
/* Fonction  demande_mise_a_jour_tous_comptes */
/* met la variable MISE_A_JOUR de tous les comptes à 1 */
/* ce qui fait que lorsqu'ils seront affichés, ils seront mis à jour avant */
/* appelle aussi verification_mise_a_jour_liste pour mettre à jour la liste courante */
/******************************************************************************/
void demande_mise_a_jour_tous_comptes ( void )
{
    gint i;
    gpointer **save_p_tab;

    save_p_tab = p_tab_nom_de_compte_variable;
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	MISE_A_JOUR = 1;
	p_tab_nom_de_compte_variable++;
    }

    verification_mise_a_jour_liste ();

    p_tab_nom_de_compte_variable = save_p_tab;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction verification_mise_a_jour_liste */
/* appelée à chaque affichage de la liste */
/* vérifie que la liste ne doit pas être réaffichée */
/******************************************************************************/
void verification_mise_a_jour_liste ( void )
{
    GtkAdjustment *ajustement;
    gfloat ratio;
    gfloat nouvelle_value;

    if ( DEBUG )
	printf ( "verifie_mise_a_jour_liste : compte %d\n", compte_courant );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    if ( !MISE_A_JOUR )
	return;
    
    if ( !g_slist_nth_data ( list_store_comptes,
			     compte_courant ))
	return;

    /*     on ne sait pas quelle va être la taille de la nouvelle liste */
    /* 	on va utiliser un ratio entre ce qu'il y a en dessous de la pos */
    /* 	actuelle et ce qu'il y a au dessus pour retrouver à peu près la */
    /* 	même position au réaffichage */

    ajustement = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_listes_operations ));

    ratio = (ajustement -> upper - ajustement -> value ) / ajustement -> value;


    LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				      (GCompareFunc) CLASSEMENT_COURANT );

    remplissage_liste_operations ( compte_courant );

    while ( g_main_iteration ( FALSE ));

    /*     si la nouvelle value est plus basse que le bas, on l'ajuste pour qu'on se mette au bout */
    /*  de la liste */

    nouvelle_value = MIN ( ajustement -> upper - ajustement -> page_size,
			   (ajustement -> upper) / (ratio + 1));

    gtk_adjustment_set_value ( ajustement,
			       nouvelle_value);
    MISE_A_JOUR = 0;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction mise_a_jour_solde */
/* recalcule le solde du compte demandé */
/******************************************************************************/
void mise_a_jour_solde ( gint compte )
{
    gdouble solde_courant;

    /* ALAIN-FIXME : si on recalcule le solde courant, il n'y a pas de raison d'en faire autant
       pour le solde pointé. C'est donc à vérifier */
    gdouble solde_pointe;

    GSList *liste_operations_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

    /* on fait le tour de toutes les opérations */

    solde_courant = SOLDE_INIT;

    /* ALAIN-FIXME : si on recalcule le solde courant, il n'y a pas de raison d'en faire autant
       pour le solde pointé. C'est donc à vérifier */
    solde_pointe = SOLDE_INIT;

    liste_operations_tmp = LISTE_OPERATIONS;

    while ( liste_operations_tmp )
    {
	struct structure_operation *operation;

	operation = liste_operations_tmp -> data;

	/* si c'est une opé de ventilation, on la saute */

	if ( !operation -> no_operation_ventilee_associee )
	{
	    /* calcul du solde courant */

	    gdouble montant;

	    montant = calcule_montant_devise_renvoi ( operation -> montant,
						      DEVISE,
						      operation -> devise,
						      operation -> une_devise_compte_egale_x_devise_ope,
						      operation -> taux_change,
						      operation -> frais_change );
	    solde_courant = solde_courant + montant;

	    /* ALAIN-FIXME : si on recalcule le solde courant, il n'y a pas de
	       raison de ne pas en faire autant pour le solde pointé.
	       C'est donc à vérifier */
	    /* si l'opé est pointée ou relevée,
	       on ajoute ce montant au solde pointé */
	    if ( operation -> pointe )
		solde_pointe = solde_pointe + montant;
	}
	liste_operations_tmp = liste_operations_tmp -> next;
    }

    SOLDE_COURANT = solde_courant;

    /* ALAIN-FIXME : si on recalcule le solde courant, il n'y a pas de raison d'en faire autant
       pour le solde pointé. C'est donc à vérifier */
    SOLDE_POINTE = solde_pointe;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction verifie_ligne_selectionnee_visible */
/* appelée lorsqu'on édite une opé et que le formulaire n'était pas visible, */
/* pour éviter que la ligne éditée se retrouve hors du champ de vision */
/******************************************************************************/
void verifie_ligne_selectionnee_visible ( void )
{
    /* FIXME : mettre l'opé et l'iter s'il existe */
    printf ( "FIXME : verifie_ligne_selectionnee_visible\n" );
/*     selectionne_ligne ( compte_courant, */
/* 			LIGNE_SELECTIONNEE ); */

/*     gtk_signal_disconnect_by_func ( GTK_OBJECT ( frame_droite_bas ), */
/* 				    GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_visible ), */
/* 				    NULL ); */
}
/******************************************************************************/

/**
 * Pop up a menu with several actions to apply to current transaction.
 */
void popup_transaction_context_menu ( gboolean full )
{
    GtkWidget *menu, *menu_item;

    if ( p_tab_nom_de_compte_courant )
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
    else 
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    if ( cherche_operation_from_ligne ( LIGNE_SELECTIONNEE ) == GINT_TO_POINTER(-1) )
	full = FALSE;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Edit transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_PROPERTIES,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", edition_operation, NULL );
    if ( !full )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Separator */
    gtk_menu_append ( menu, gtk_separator_menu_item_new() );

    /* New transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("New transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_NEW,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", new_transaction, NULL );
    gtk_menu_append ( menu, menu_item );

    /* Delete transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Delete transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_DELETE,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", remove_transaction, NULL );
    if ( !full )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Clone transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Clone transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_COPY,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", clone_selected_transaction, NULL );
    if ( !full )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Separator */
    gtk_menu_append ( menu, gtk_separator_menu_item_new() );

    /* Convert to scheduled transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Convert transaction to scheduled transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_CONVERT,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", schedule_selected_transaction, NULL );
    if ( !full )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Move to another account */
    menu_item = gtk_image_menu_item_new_with_label ( _("Move transaction to another account") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_JUMP_TO,
							       GTK_ICON_SIZE_MENU ));
    if ( !full )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Add accounts submenu */
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(menu_item), 
				GTK_WIDGET(creation_option_menu_comptes(GTK_SIGNAL_FUNC(move_selected_operation_to_account), FALSE)) );

    gtk_widget_show_all (menu);
    gtk_menu_popup ( GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
}


/**
 *  Check that a transaction is selected and sets the
 *  p_tab_nom_de_compte_variable pointer accordingly.
 *
 * \return TRUE on success, FALSE otherwise.
 */
gboolean assert_selected_transaction ()
{
    if ( p_tab_nom_de_compte_courant )
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
    else 
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    if ( cherche_operation_from_ligne (LIGNE_SELECTIONNEE ) == GINT_TO_POINTER(-1) )
	return FALSE;

    return TRUE;
}


/**
 *  Empty transaction form and select transactions tab.
 */
void new_transaction () 
{
    /* We do not test it since we don't care about no selected
       transaction */
    assert_selected_transaction();
    /* FIXME : tester ce que ça donne sans désélectionner l'opé en cours et en faisant nouvelle opé par click droit */

    /*     gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ) ); */
    echap_formulaire();
    /* FIXME : cette histoire de ligne_selectionnee = -1 quand c'est la ligne blanche ? */
    LIGNE_SELECTIONNEE = -1;
    edition_operation ();

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}


/**
 * Remove selected transaction if any.
 */
void remove_transaction ()
{
    if (! assert_selected_transaction()) return;

    supprime_operation ( cherche_operation_from_ligne ( LIGNE_SELECTIONNEE ));
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}


/**
 * Clone selected transaction if any.  Update user interface as well.
 */
void clone_selected_transaction ()
{
    if (! assert_selected_transaction()) return;

    clone_transaction ( cherche_operation_from_ligne ( LIGNE_SELECTIONNEE ));

    MISE_A_JOUR = 1;
    verification_mise_a_jour_liste ();

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

    mise_a_jour_tiers ();
    mise_a_jour_categ ();
    mise_a_jour_imputation ();

    modification_fichier ( TRUE );
}


/**
 * Clone transaction.  If it is a breakdown or a transfer, perform all
 * needed operations, like cloning associated transactions as well.
 *
 * \param operation Initial transaction to clone
 *
 * \return A newly created operation.
 */
struct structure_operation *  clone_transaction ( struct structure_operation * operation )
{
    struct structure_operation * new_transaction, * ope_ventilee;

    new_transaction = (struct structure_operation *) malloc ( sizeof(struct structure_operation) );
    if ( !new_transaction )
    {
	dialogue ( _("Cannot allocate memory, bad things will happen soon") );
	return(FALSE);
    }

#ifndef _WIN32
    bcopy ( operation, new_transaction, sizeof(struct structure_operation) );
#else
    memcpy(new_transaction, operation, sizeof(struct structure_operation) );
#endif
    new_transaction -> no_operation = 0;
    ajout_operation ( new_transaction );

    if ( operation -> relation_no_operation )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;
	validation_virement_operation ( operation, 0, NOM_DU_COMPTE );
    }

    if ( operation -> operation_ventilee )
    {
	GSList *liste_tmp;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation_2;

	    operation_2 = liste_tmp -> data;

	    if ( operation_2 -> no_operation_ventilee_associee == operation -> no_operation )
	    {
		ope_ventilee = clone_transaction ( operation_2 );
		ope_ventilee -> no_operation_ventilee_associee = new_transaction -> no_operation;
	    }

	    liste_tmp = liste_tmp -> next;
	}
    }

    return new_transaction;
}



/**
 * Move selected transaction to another account.  Normally called as a
 * handler.
 *
 * \param menu_item The GtkMenuItem that triggered this handler.
 */
void move_selected_operation_to_account ( GtkMenuItem * menu_item )
{
    gint account;

    if (! assert_selected_transaction()) return;

    account = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT(menu_item), 
						      "no_compte" ) );  

    move_operation_to_account ( cherche_operation_from_ligne ( LIGNE_SELECTIONNEE ), account );

    MISE_A_JOUR = 1;
    verification_mise_a_jour_liste ();
    remplissage_liste_operations ( account );

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

    mise_a_jour_tiers ();
    mise_a_jour_categ ();
    mise_a_jour_imputation ();

    modification_fichier ( TRUE );
}



/**
 * Move transaction to another account
 *
 * \param transaction Transaction to move to other account
 * \param account Account to move the transaction to
 */
void move_operation_to_account ( struct structure_operation * transaction,
				 gint account )
{
    gpointer ** tmp = p_tab_nom_de_compte_variable;

    if ( transaction -> relation_no_compte )
    {
	struct structure_operation * contra_transaction;
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + transaction -> relation_no_compte;
	contra_transaction = 
	    g_slist_find_custom ( LISTE_OPERATIONS,
				  GINT_TO_POINTER ( transaction -> relation_no_operation ),
				  ( GCompareFunc ) recherche_operation_par_no ) -> data;
	contra_transaction -> relation_no_compte = account;

    }

    if ( transaction -> operation_ventilee )
    {
	GSList *liste_tmp;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + transaction -> no_compte;
	liste_tmp = g_slist_copy ( LISTE_OPERATIONS );

	while ( liste_tmp )
	{
	    struct structure_operation *transaction_2;

	    transaction_2 = liste_tmp -> data;

	    if ( transaction_2 -> no_operation_ventilee_associee == 
		 transaction -> no_operation )
	    {
		move_operation_to_account ( transaction_2, account );
		transaction_2 -> relation_no_compte = account;
	    }

	    liste_tmp = liste_tmp -> next;
	}

	g_slist_free ( liste_tmp );
    }

    LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS, transaction );
    NB_OPE_COMPTE--;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + account;
    LISTE_OPERATIONS = g_slist_sort ( g_slist_append ( LISTE_OPERATIONS, transaction ),
				      (GCompareFunc) CLASSEMENT_COURANT );
    NB_OPE_COMPTE++;

    transaction -> no_compte = account;
    p_tab_nom_de_compte_variable = tmp;
}



/**
 * Convert selected transaction to a template of scheduled transaction
 * via schedule_transaction().
 */
void schedule_selected_transaction ()
{
    struct operation_echeance * echeance;

    if (! assert_selected_transaction()) return;

    echeance = schedule_transaction ( cherche_operation_from_ligne ( LIGNE_SELECTIONNEE ));

    update_liste_echeances_auto_accueil ();
    remplissage_liste_echeance ();

    echeance_selectionnnee = echeance;
    formulaire_echeancier_a_zero();
    degrise_formulaire_echeancier();
    selectionne_echeance ();
    edition_echeance ();

    gtk_notebook_set_current_page ( GTK_NOTEBOOK(notebook_general), 2 );

    modification_fichier ( TRUE );
}



/**
 *  Convert transaction to a template of scheduled transaction.
 *
 * \param transaction Transaction to use as a template.
 */
    struct operation_echeance *
schedule_transaction ( struct structure_operation * transaction )
{
    struct operation_echeance *echeance;

    echeance = (struct operation_echeance *) calloc ( 1,
						      sizeof(struct operation_echeance) );
    if ( !echeance )
    {
	dialogue ( _("Cannot allocate memory, bad things will happen soon") );
	return(FALSE);
    }

    echeance -> compte = transaction -> no_compte;
    echeance -> jour = transaction -> jour;
    echeance -> mois = transaction -> mois;
    echeance -> annee = transaction -> annee;
    echeance -> date = g_date_new_dmy ( transaction -> jour,
					transaction -> mois,
					transaction -> annee );

    echeance -> montant = transaction -> montant;
    echeance -> devise = transaction -> devise;

    echeance -> tiers = transaction -> tiers;
    echeance -> categorie = transaction -> categorie;
    echeance -> sous_categorie = transaction -> sous_categorie;

/*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
/* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement ni une ventil, compte_virement = -1 */
/*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

    if ( transaction -> relation_no_operation )
    {
	/* 	c'est un virement, on met la relation et on recherche le type de la contre opération */
	
	gpointer **save_ptab;
	struct structure_operation *contre_operation;
	
	echeance -> compte_virement = transaction -> relation_no_compte;

	save_ptab = p_tab_nom_de_compte_variable;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + echeance -> compte_virement;
	contre_operation = g_slist_find_custom ( LISTE_OPERATIONS,
						 GINT_TO_POINTER ( transaction -> relation_no_operation ),
						 (GCompareFunc) recherche_operation_par_no ) -> data;
	echeance -> type_contre_ope = contre_operation -> type_ope;
	p_tab_nom_de_compte_variable = save_ptab;
    }
    else
	if ( !echeance -> categorie
	     &&
	     !transaction -> operation_ventilee )
	    echeance -> compte_virement = -1;

    echeance -> notes = g_strdup ( transaction -> notes );
    echeance -> type_ope = transaction -> type_ope;
    echeance -> contenu_type = g_strdup ( transaction -> contenu_type );


    echeance -> no_exercice = transaction -> no_exercice;
    echeance -> imputation = transaction -> imputation;
    echeance -> sous_imputation = transaction -> sous_imputation;

    echeance -> operation_ventilee = transaction -> operation_ventilee;

/*     par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
/*     (c'est le cas, à 0 avec calloc) */
/*     que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
/* 	pour la même raison */

    echeance -> periodicite = 2;
    
    echeance -> no_operation = ++no_derniere_echeance;
    nb_echeances++;
    liste_struct_echeances = g_slist_insert_sorted ( liste_struct_echeances,
						     echeance,
						     (GCompareFunc) comparaison_date_echeance );

/*     on récupère les opés de ventil si c'était une opé ventilée */

    if ( echeance -> operation_ventilee )
    {
	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *transaction_de_ventil;

	    transaction_de_ventil = liste_tmp -> data;

	    if ( transaction_de_ventil -> no_operation_ventilee_associee == transaction -> no_operation )
	    {
		struct operation_echeance *echeance_de_ventil;

		echeance_de_ventil = calloc ( 1,
					      sizeof ( struct operation_echeance));

		if ( !echeance_de_ventil )
		{
		    dialogue ( _("Cannot allocate memory, bad things will happen soon") );
		    return(FALSE);
		}

		echeance_de_ventil -> compte = transaction_de_ventil -> no_compte;
		echeance_de_ventil -> jour = transaction_de_ventil -> jour;
		echeance_de_ventil -> mois = transaction_de_ventil -> mois;
		echeance_de_ventil -> annee = transaction_de_ventil -> annee;
		echeance_de_ventil -> date = g_date_new_dmy ( transaction_de_ventil -> jour,
						    transaction_de_ventil -> mois,
						    transaction_de_ventil -> annee );

		echeance_de_ventil -> montant = transaction_de_ventil -> montant;
		echeance_de_ventil -> devise = transaction_de_ventil -> devise;

		echeance_de_ventil -> tiers = transaction_de_ventil -> tiers;
		echeance_de_ventil -> categorie = transaction_de_ventil -> categorie;
		echeance_de_ventil -> sous_categorie = transaction_de_ventil -> sous_categorie;

		/*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
		/* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement, compte_virement = -1 */
		/*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

		if ( transaction_de_ventil -> relation_no_operation )
		{
		    /* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

		    gpointer **save_ptab;
		    struct structure_operation *contre_operation;

		    echeance_de_ventil -> compte_virement = transaction_de_ventil -> relation_no_compte;

		    save_ptab = p_tab_nom_de_compte_variable;

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + echeance_de_ventil -> compte_virement;
		    contre_operation = g_slist_find_custom ( LISTE_OPERATIONS,
							     GINT_TO_POINTER ( transaction_de_ventil -> relation_no_operation ),
							     (GCompareFunc) recherche_operation_par_no ) -> data;
		    echeance_de_ventil -> type_contre_ope = contre_operation -> type_ope;
		    p_tab_nom_de_compte_variable = save_ptab;
		}
		else
		    if ( !echeance_de_ventil -> categorie )
			echeance_de_ventil -> compte_virement = -1;

		echeance_de_ventil -> notes = g_strdup ( transaction_de_ventil -> notes );
		echeance_de_ventil -> type_ope = transaction_de_ventil -> type_ope;
		echeance_de_ventil -> contenu_type = g_strdup ( transaction_de_ventil -> contenu_type );


		echeance_de_ventil -> no_exercice = transaction_de_ventil -> no_exercice;
		echeance_de_ventil -> imputation = transaction_de_ventil -> imputation;
		echeance_de_ventil -> sous_imputation = transaction_de_ventil -> sous_imputation;

		echeance_de_ventil-> no_operation_ventilee_associee = echeance -> no_operation;

		/*     par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
		/*     (c'est le cas, à 0 avec calloc) */
		/*     que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
		/* 	pour la même raison */

		echeance_de_ventil -> periodicite = 2;

		echeance_de_ventil -> no_operation = ++no_derniere_echeance;
		nb_echeances++;
		liste_struct_echeances = g_slist_insert_sorted ( liste_struct_echeances,
								 echeance_de_ventil,
								 (GCompareFunc) comparaison_date_echeance );
	    }
	    liste_tmp = liste_tmp -> next;
	}
    }
    return echeance;
}


/******************************************************************************/
/* cette fonction affiche les traits verticaux et horizontaux sur la liste des opés */
/******************************************************************************/
void affichage_traits_liste_operation ( void )
{

    GdkWindow *fenetre;
    gint i;
    gint largeur, hauteur;
    gint x, y;
    GtkAdjustment *adjustment;
    gint derniere_ligne;
    gint nb_ligne_ope_tree_view;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    /*  FIXME   sachant qu'on appelle ça à chaque expose-event, cad très souvent ( dès que la souris passe dessus ), */
    /*     ça peut ralentir bcp... à vérifier  */

    fenetre = gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view_listes_operations ));
    gdk_drawable_get_size ( GDK_DRAWABLE ( fenetre ),
			    &largeur,
			    &hauteur );

    if ( !gc_separateur_operation )
	gc_separateur_operation = gdk_gc_new ( GDK_DRAWABLE ( fenetre ));

    /*     si la hauteur des lignes n'est pas encore calculée, on le fait ici */

    if ( !hauteur_ligne_liste_opes )
	hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_listes_operations );

    /*     on commence par calculer la dernière ligne en pixel correspondant à la dernière opé de la liste */
    /* 	pour éviter de dessiner les traits en dessous */

    derniere_ligne = hauteur_ligne_liste_opes * GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_listes_operations ))) -> length;
    hauteur = MIN ( derniere_ligne,
		    hauteur );

    /*     le plus facile en premier... les lignes verticales */
    /*     dépend de si on est en train de ventiler ou non */
    /*     on en profite pour ajuster nb_ligne_ope_tree_view */

    x=0;

    if ( etat.ventilation_en_cours )
    {
	nb_ligne_ope_tree_view = 1;

	for ( i=0 ; i<3 ; i++ )
	{
	    x = x + gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_ventils[i] ));
	    gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			    gc_separateur_operation,
			    x, 0,
			    x, hauteur );
	}
    }
    else
    {
	nb_ligne_ope_tree_view = NB_LIGNES_OPE;

	for ( i=0 ; i<6 ; i++ )
	{
	    x = x + gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_opes[i] ));
	    gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			    gc_separateur_operation,
			    x, 0,
			    x, hauteur );
	}
    }

    /*     les lignes horizontales : il faut calculer la position y de chaque changement d'opé à l'écran */

    /*     on calcule la position y de la 1ère ligne à afficher */

    adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_listes_operations ));

    y = ( hauteur_ligne_liste_opes * nb_ligne_ope_tree_view ) * ( ceil ( adjustment->value / (hauteur_ligne_liste_opes* nb_ligne_ope_tree_view) )) - adjustment -> value;

    do
    {
	gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			gc_separateur_operation,
			0, y, 
			largeur, y );
	y = y + hauteur_ligne_liste_opes*nb_ligne_ope_tree_view;
    }
    while ( y < ( adjustment -> value + adjustment -> page_size )
	    &&
	    y <= derniere_ligne );
}
/******************************************************************************/






/******************************************************************************/
gboolean click_sur_titre_colonne_operations ( GtkTreeViewColumn *colonne,
					      gint *no_colonne )
{
    GtkWidget *menu, *menu_item;
    gchar **tab_char;
    gint i;


/*     on récupère les popups du titre de la colonne */
/* 	s'il n'y a rien dans la colonne, on se barre */
/* 	celà veut dire aussi que pour trier en fonction d'un paramètre, */
/*     ce paramètre doit être affiché dans la liste */

    tab_char = g_strsplit ( tips_col_liste_operations[GPOINTER_TO_INT(no_colonne)],
			    ", ",
			    0 );
    if ( !tab_char[0] )
	return FALSE;

/*     on met la de colonne en tmp */

    colonne_classement_tmp = colonne;

	/*     s'il n'y a qu'un choix possible, on n'affiche pas la popup, on trie */
/* 	directement */

    if ( !tab_char[1] )
    {
	if ( strcmp ( tab_char[0],
		      N_("Balance")))
	{
	    changement_choix_tri_liste_operations ( tab_char[0] );
	    g_strfreev ( tab_char );
	    return FALSE;
	}
	else
	    return FALSE;
    }

     menu = gtk_menu_new ();

    /*  ligne trier par */

    menu_item = gtk_menu_item_new_with_label ( _("Sort list by :") );

/*     les 2 signaux sont bloqués pour éviter que la ligne s'affiche comme un bouton */
/* pas réussi à faire autrement... */

    g_signal_connect ( G_OBJECT ( menu_item),
		       "enter-notify-event",
		       G_CALLBACK ( gtk_true ),
		       NULL );
    g_signal_connect ( G_OBJECT ( menu_item),
		       "motion-notify-event",
		       G_CALLBACK ( gtk_true ),
		       NULL );
  
    gtk_menu_append ( menu,
		      menu_item );
    gtk_widget_show_all ( menu_item );

    menu_item = gtk_separator_menu_item_new ();
    gtk_menu_append ( menu,
		      menu_item );
    gtk_widget_show ( menu_item );


    i = 0;

    while ( tab_char[i] )
    {
	if ( strcmp ( tab_char[i],
		      N_("Balance")))
	{
	    menu_item = gtk_menu_item_new_with_label ( tab_char[i] );
	    g_signal_connect_swapped ( G_OBJECT(menu_item),
				       "activate",
				       G_CALLBACK ( changement_choix_tri_liste_operations ),
				       tab_char[i] );
	    g_signal_connect_data ( G_OBJECT(menu_item),
				    "activate",
				    G_CALLBACK ( g_strfreev ),
				    tab_char,
				    NULL,
				    G_CONNECT_AFTER | G_CONNECT_SWAPPED);
	    gtk_menu_append ( menu,
			      menu_item );
	    gtk_widget_show ( menu_item );
	}
	i++;
    }


    gtk_menu_popup ( GTK_MENU(menu),
		     NULL,
		     NULL,
		     NULL,
		     NULL,
		     3,
		     gtk_get_current_event_time());
    gtk_widget_show (menu);
    
    return FALSE;
}					      
/******************************************************************************/


/******************************************************************************/
void changement_choix_tri_liste_operations ( gchar *nom_tri )
{
/*  cette fonction est appelée quand on a cliqué sur un titre de colonne pour changer */
/*      le tri de la liste d'opé */

    gint no_tri;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    no_tri = g_slist_position ( liste_labels_titres_colonnes_liste_ope,
				g_slist_find_custom ( liste_labels_titres_colonnes_liste_ope,
						      nom_tri,
						      (GCompareFunc) cherche_string_equivalente_dans_slist ));

    CLASSEMENT_COURANT = recupere_classement_par_no ( no_tri );
    
    gtk_tree_view_column_set_sort_indicator ( COLONNE_CLASSEMENT,
					      FALSE );

    if ( COLONNE_CLASSEMENT == colonne_classement_tmp )
    {
	/* la liste était déjà trié par cette colonne, on change le sens de tri */

	CLASSEMENT_CROISSANT = !CLASSEMENT_CROISSANT;
    }
    else
    {
	COLONNE_CLASSEMENT = colonne_classement_tmp;
	CLASSEMENT_CROISSANT = 1;
    }

     gtk_tree_view_column_set_sort_indicator ( COLONNE_CLASSEMENT,
					      TRUE );

     if ( CLASSEMENT_CROISSANT )
	 gtk_tree_view_column_set_sort_order ( COLONNE_CLASSEMENT,
					       GTK_SORT_ASCENDING );
     else
	 gtk_tree_view_column_set_sort_order ( COLONNE_CLASSEMENT,
					       GTK_SORT_DESCENDING );
   
    MISE_A_JOUR = 1;
    verification_mise_a_jour_liste ();

    modification_fichier ( TRUE );
}
/******************************************************************************/

/******************************************************************************/
gpointer recupere_classement_par_no ( gint no_tri )
{

    switch ( no_tri )
    {
	case 0:
	    etat.classement_par_date = 1;
	    return ( classement_sliste_par_date);
	    break;
	case 1:
	    etat.classement_par_date = 0;
	    return ( classement_sliste_par_date);
	    break;
	case 2:
	    return ( classement_sliste_par_tiers);
	    break;
	case 3:
	    return ( classement_sliste_par_imputation);
	    break;
	case 4:
	    return ( classement_sliste_par_credit);
	    break;
	case 5:
	    return ( classement_sliste_par_debit);
	    break;
	case 6:
	    /* 	    balance, normalement ne devrait pas venir ici, dans le doute renvoie par date */
	    return ( classement_sliste_par_date );
	    break;
	case 7:
	    return ( classement_sliste_par_montant);
	    break;
	case 8:
	    return ( classement_sliste_par_type_ope);
	    break;
	case 9:
	    return ( classement_sliste_par_no_rapprochement);
	    break;
	case 10:
	    return ( classement_sliste_par_exercice);
	    break;
	case 11:
	    return ( classement_sliste_par_categories);
	    break;
	case 12:
	    return ( classement_sliste_par_pointage);
	    break;
	case 13:
	    return ( classement_sliste_par_pc);
	    break;
	case 14:
	    return ( classement_sliste_par_notes);
	    break;
	case 15:
	    return ( classement_sliste_par_ibg);
	    break;
	case 16:
	    return ( classement_sliste_par_no);
	    break;
	case 17:
	    return ( classement_sliste_par_no_rapprochement);
	    break;
	default :
	    return ( classement_sliste_par_date);
    }
}
/******************************************************************************/


