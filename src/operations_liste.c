/* ************************************************************************** */
/*  Fichier qui gère la liste des opérations                                  */
/* 			liste_operations.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004      Benjamin Drieu (bdrieu@april.org) 	      */
/*			2003-2005 Alain Portal (aportal@univ-montp2.fr)	      */
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
#include "mouse.h"
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
#include "affichage_liste.h"


GtkJustification col_justs[] = { GTK_JUSTIFY_CENTER,
    GTK_JUSTIFY_CENTER,
    GTK_JUSTIFY_LEFT,
    GTK_JUSTIFY_CENTER,
    GTK_JUSTIFY_RIGHT,
    GTK_JUSTIFY_RIGHT,
    GTK_JUSTIFY_RIGHT };

gint allocation_precedente = 0;
gint allocation_encore_avant = 0;
gint allocation_compte_precedent = -1;

/* contient la chaine de la dernière date entrée */

gchar *derniere_date;


extern struct operation_echeance *echeance_selectionnnee;
extern gint no_derniere_echeance;
extern GSList *gsliste_echeances; 
extern gint nb_echeances;
extern GSList *echeances_saisies;


/* FIXME INSTABLE : REPRENDRE AUSSI L'ANCIEN FICHIER, VERIFIER QU'IL N'Y A PAS EU DE CORRECTIONS DE BUGS DANS CE FICHIER */
/* AUTRES QUE L'AJUSTEMENT DE LA TAILLE DES COLONNES */

/******************************************************************************/
/*  Routine qui crée la fenêtre de la liste des opérations */
/******************************************************************************/
GtkWidget *creation_fenetre_operations ( void )
{
    GtkWidget *win_operations;
    GtkWidget *solde_box;
    GtkWidget *frame;

    /*   la fenetre des opés est une vbox : la liste en haut, le solde et  */
    /*     des boutons de conf au milieu, le formulaire en bas */

/* FIXME: à virer sur l'instable */
    allocation_precedente = 0;

    win_operations = gtk_vbox_new ( FALSE,
				    5 );

    /* création de la barre d'outils */

    barre_outils = creation_barre_outils ();
    gtk_box_pack_start ( GTK_BOX ( win_operations ),
			 barre_outils,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( barre_outils );

    /* création du notebook des opé */

    notebook_listes_operations = initialisation_notebook_operations ();
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
/* Création du notebook des opérations					      */
/* C'est en fait un notebook dont chaque onglet sera une clist qui contient   */
/* les opérations d'un compte. Il y a autant d'onglet que de comptes	      */
/* Cette fonction crée le notebook et initialise les variables générales      */
/* utilisées par les listes						      */
/******************************************************************************/
GtkWidget *initialisation_notebook_operations ( void )
{
    /* création du notebook */

    notebook_listes_operations = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook_listes_operations ),
				 FALSE );

    /* création de l'onglet de la ventilation */

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			       creation_fenetre_ventilation (),
			       gtk_label_new ( _("Account breakdown") ) );

    return ( notebook_listes_operations );
}
/******************************************************************************/

/******************************************************************************/
/* Création de la liste des opé */
/* cette fonction crée autant d'onglets que de comptes dans le notebook */
/* elle y met les clist et les remplit */
/******************************************************************************/
void creation_listes_operations ( void )
{
    gint i;
    GtkTooltips *tooltip;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	GtkWidget *onglet;
	GtkWidget *liste;
	gint j;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;


	/*  Création de la scrolled window qui contient la clist */

	onglet = gtk_scrolled_window_new ( NULL,
					   NULL);
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( onglet ),
					 GTK_POLICY_AUTOMATIC,
					 GTK_POLICY_AUTOMATIC);
	gtk_widget_show ( onglet );


	/* création de l'onglet */

	liste = gtk_clist_new_with_titles ( TRANSACTION_LIST_COL_NB,
					    titres_colonnes_liste_operations );
	gtk_widget_set_usize ( GTK_WIDGET ( liste ),
			       1,
			       FALSE );
	gtk_signal_connect ( GTK_OBJECT ( liste ),
			     "size-allocate",
			     GTK_SIGNAL_FUNC ( changement_taille_liste_ope ),
			     GINT_TO_POINTER ( i ) );
	gtk_signal_connect ( GTK_OBJECT ( liste ),
			     "map",
			     GTK_SIGNAL_FUNC ( verification_mise_a_jour_liste ),
			     NULL );
	gtk_signal_connect_after ( GTK_OBJECT ( onglet ),
				   "realize",
				   GTK_SIGNAL_FUNC ( onglet_compte_realize ),
				   liste );
	gtk_container_add ( GTK_CONTAINER ( onglet ),
			    liste );
	gtk_widget_show ( liste );

	/*       si on utilise la police perso, on adapte la hauteur des lignes en fonction */

	if ( etat.utilise_fonte_listes )
	{
	    gint size = pango_font_description_get_size (pango_font_description_from_string(fonte_liste));
	    gtk_clist_set_row_height ( GTK_CLIST ( liste ),
				       (size/PANGO_SCALE) + 7 );
	}

	/* on met les tooltips aux boutons de la clist */
	/* le fait de mettre des tips sur les titres rend les boutons sensitifs */
	/* on va détourner le click pour ne pas faire enfoncer le bouton */

	tooltip = gtk_tooltips_new ();

	for ( j = 0 ; j < TRANSACTION_LIST_COL_NB ; j++ )
	{
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltip ),
				   GTK_CLIST ( liste ) -> column[j].button,
				   tips_col_liste_operations[j],
				   tips_col_liste_operations[j] );
	    gtk_signal_connect ( GTK_OBJECT ( GTK_CLIST ( liste ) -> column[j].button ),
				 "button-press-event",
				 GTK_SIGNAL_FUNC ( empeche_pression_titre_colonne ),
				 NULL );
	}

	/* on permet la sélection de plusieurs lignes */

	gtk_clist_set_selection_mode ( GTK_CLIST ( liste ),
				       GTK_SELECTION_MULTIPLE );

	/* On annule la fonction bouton des titres */

	gtk_clist_column_titles_passive ( GTK_CLIST ( liste ) );

	/* justification du contenu des cellules */
	/* colonnes redimensionnables ou non */

	for ( j = 0 ; j < TRANSACTION_LIST_COL_NB ; j++ )
	{
	    gtk_clist_set_column_justification ( GTK_CLIST ( liste ),
						 j,
						 col_justs[j] );
	    gtk_clist_set_column_resizeable ( GTK_CLIST ( liste ),
					      j,
					      !etat.largeur_auto_colonnes );
	}

	/* vérifie le simple ou double click */

	gtk_signal_connect ( GTK_OBJECT ( liste ),
			     "button_press_event",
			     GTK_SIGNAL_FUNC ( selectionne_ligne_souris ),
			     NULL );

	/* vérifie la touche entrée, haut et bas */

	gtk_signal_connect ( GTK_OBJECT ( liste ),
			     "key_press_event",
			     GTK_SIGNAL_FUNC ( traitement_clavier_liste ),
			     NULL );

	/* sauvegarde les redimensionnement des colonnes */

    gtk_signal_connect ( GTK_OBJECT ( liste ),
			 "resize_column",
			 GTK_SIGNAL_FUNC ( changement_taille_colonne ),
			 NULL );

    /* on ajoute l'onglet au notebook des comptes */

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
				   onglet,
				   gtk_label_new ( NOM_DU_COMPTE ) );

	CLIST_OPERATIONS = liste;

	/* on classe la liste en fonction de la date */

	LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
					  (GCompareFunc) classement_sliste );

	remplissage_liste_operations ( i );
    }

}
/******************************************************************************/

/******************************************************************************/
/* Fonction empeche_pression_titre_colonne */
/* permet d'éviter que le bouton s'enfonce si on clicke dessus */
/******************************************************************************/
gint empeche_pression_titre_colonne ( GtkWidget *bouton )
{
    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( bouton ),
				   "button_press_event");
    return ( TRUE );
}
/******************************************************************************/

/******************************************************************************/
/* Création de la liste des opérations d'un nouveau compte */
/******************************************************************************/
void ajoute_nouvelle_liste_operation ( gint no_compte )
{
    GtkWidget *onglet;
    GtkWidget *liste;
    GtkTooltips *tooltip;
    gint i;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

    /*  Création de la scrolled window qui contient la clist */

    onglet = gtk_scrolled_window_new ( NULL,
				       NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( onglet ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC);
    gtk_widget_show ( onglet );

    /* création de l'onglet */

    liste = gtk_clist_new_with_titles ( TRANSACTION_LIST_COL_NB,
					titres_colonnes_liste_operations );
    gtk_widget_set_usize ( GTK_WIDGET ( liste ),
			   1,
			   FALSE );
    gtk_signal_connect ( GTK_OBJECT ( liste ),
			 "size-allocate",
			 GTK_SIGNAL_FUNC ( changement_taille_liste_ope ),
			 GINT_TO_POINTER ( no_compte ) );
    gtk_signal_connect ( GTK_OBJECT ( liste ),
			 "map",
			 GTK_SIGNAL_FUNC ( verification_mise_a_jour_liste ),
			 NULL );
    gtk_signal_connect_after ( GTK_OBJECT ( onglet ),
			       "realize",
			       GTK_SIGNAL_FUNC ( onglet_compte_realize ),
			       liste );
    gtk_container_add ( GTK_CONTAINER ( onglet ),
			liste );
    gtk_widget_show ( liste );

    /* le fait de mettre des tips sur les titres rend les boutons sensitifs ; */
    /* on va détourner le click pour ne pas faire enfoncer le bouton */

    tooltip = gtk_tooltips_new ();

    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltip ),
			       GTK_CLIST ( liste ) -> column[i].button,
			       tips_col_liste_operations[i],
			       tips_col_liste_operations[i] );
	gtk_signal_connect ( GTK_OBJECT ( GTK_CLIST ( liste ) -> column[i].button ),
			     "button-press-event",
			     GTK_SIGNAL_FUNC ( empeche_pression_titre_colonne ),
			     NULL );
    }

    /* on permet la sélection de plusieurs lignes */

    gtk_clist_set_selection_mode ( GTK_CLIST ( liste ),
				   GTK_SELECTION_MULTIPLE );

    /* On annule la fonction bouton des titres */

    gtk_clist_column_titles_passive ( GTK_CLIST ( liste ) );

    /* justification du contenu des cellules */

    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	gtk_clist_set_column_justification ( GTK_CLIST ( liste ),
					     i,
					     col_justs[i] );
	gtk_clist_set_column_resizeable ( GTK_CLIST ( liste ),
					  i,
					  !etat.largeur_auto_colonnes );
    }

    /* véifie le simple ou double click */

    gtk_signal_connect ( GTK_OBJECT ( liste ),
			 "button_press_event",
			 GTK_SIGNAL_FUNC ( selectionne_ligne_souris ),
			 NULL );

    /*   vérifie la touche entrée, haut et bas */

    gtk_signal_connect ( GTK_OBJECT ( liste ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( traitement_clavier_liste ),
			 NULL );

    /* sauvegarde les redimensionnement des colonnes */

    gtk_signal_connect ( GTK_OBJECT ( liste ),
			 "resize_column",
			 GTK_SIGNAL_FUNC ( changement_taille_colonne ),
			 NULL );

    CLIST_OPERATIONS = liste;

    /* par défaut, le classement de la liste s'effectue par date */

    LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				      (GCompareFunc) classement_sliste );

    /* on ajoute l'onglet au notebook des comptes */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			       onglet,
			       gtk_label_new ( NOM_DU_COMPTE ) );

    remplissage_liste_operations ( no_compte );
}
/******************************************************************************/


/******************************************************************************/
void update_titres_colonnes ( void )
{
    gint i, j;
    GtkTooltips *tooltip;

    recuperation_noms_colonnes_et_tips ();
    
    tooltip = gtk_tooltips_new ();

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	for ( j = 0 ; j<7 ; j++ )
	{
	    gtk_clist_set_column_title ( GTK_CLIST ( CLIST_OPERATIONS ),
					 j,
					 titres_colonnes_liste_operations[j] );
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltip ),
				   GTK_CLIST ( CLIST_OPERATIONS ) -> column[j].button,
				   tips_col_liste_operations[j],
				   tips_col_liste_operations[j] );
	}
    }
}
/******************************************************************************/


/******************************************************************************/
/* Fonction onglet_compte_realize */
/* appelée lorsque la liste est affichée la 1ère fois */
/* permet de se placer en bas de toutes les opés au départ */
/******************************************************************************/
void onglet_compte_realize ( GtkWidget *onglet,
			     GtkWidget *liste )
{
    GtkAdjustment *adr_ajustement;

    adr_ajustement = gtk_clist_get_vadjustment ( GTK_CLIST ( liste ) );

    gtk_adjustment_set_value ( adr_ajustement,
			       adr_ajustement -> upper - adr_ajustement -> page_size );
}
/******************************************************************************/

/******************************************************************************/
/* remplissage de la liste des opérations du compte donné en argument */
/* par les opérations du compte courant */
/******************************************************************************/
void remplissage_liste_operations ( gint compte )
{
    GSList *liste_operations_tmp;
    gint couleur_en_cours;
    gchar *ligne_clist[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
    gint i, j;
    gint ligne;
    gdouble montant;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;
    
    if ( ! GTK_IS_CLIST (CLIST_OPERATIONS) )
      return;

    /* freeze la clist */

    gtk_clist_freeze ( GTK_CLIST ( CLIST_OPERATIONS ));

    /* efface la clist */

    gtk_clist_clear ( GTK_CLIST ( CLIST_OPERATIONS ));

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

	    if ( !operation -> no_operation_ventilee_associee &&
		 operation -> pointe == RECONCILED_TRANSACTION )
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

    while ( liste_operations_tmp )
    {
	struct structure_operation *operation;

	operation = liste_operations_tmp -> data;

	/* si c'est une opé de ventilation, on la saute */

	if ( !operation -> no_operation_ventilee_associee )
	{

	    /* si l'opération est relevée et qu'on ne désire pas les afficher, on passe la suite  */

	    if ( AFFICHAGE_R || operation -> pointe != RECONCILED_TRANSACTION )
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
		    /* on ne passe que si la ligne doit êre affichée */

		    if ( !i ||
			 NB_LIGNES_OPE == 4 ||
			 (( i == 1 || i == 2 ) && NB_LIGNES_OPE == 3 ) ||
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

			/* on affiche la ligne */

			ligne = gtk_clist_append ( GTK_CLIST ( CLIST_OPERATIONS ),
						   ligne_clist[i] );

			/* on associe à cette ligne l'adr de la struct de l'opé */

			gtk_clist_set_row_data ( GTK_CLIST (CLIST_OPERATIONS),
						 ligne,
						 operation);

			/* on met la couleur de la ligne */

			gtk_clist_set_row_style ( GTK_CLIST ( CLIST_OPERATIONS ),
						  ligne,
						  style_couleur [ couleur_en_cours ] );

			/* si le solde courant est < 0, on recherche la position de l'affichage */
			/* du solde et on le met en rouge si on le trouve */

			if ( solde_courant_affichage_liste < 0.0 )
			    for ( j = 0 ; j < TRANSACTION_LIST_COL_NB ; j++ )
				if ( tab_affichage_ope[ligne_affichee][j] == 7 )
				    gtk_clist_set_cell_style ( GTK_CLIST ( CLIST_OPERATIONS ),
							       ligne,
							       j,
							       style_rouge_couleur [ couleur_en_cours ] );
		    }
		}
		couleur_en_cours = ( couleur_en_cours + 1 ) % 2;
	    }
	}
	liste_operations_tmp = liste_operations_tmp -> next;
    }


    /* affiche la ligne blanche du bas */

    for ( j = 0 ; j < NB_LIGNES_OPE ; j++ )
    {
	/* on met à NULL tout les pointeurs */

	for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
	    ligne_clist[0][i] = NULL;

	ligne = gtk_clist_append ( GTK_CLIST ( CLIST_OPERATIONS ),
				   ligne_clist[0] );

	/* on associe à cette ligne à -1 */

	gtk_clist_set_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
				 ligne,
				 GINT_TO_POINTER ( -1 ));

	/* on met la couleur de la ligne */

	gtk_clist_set_row_style ( GTK_CLIST ( CLIST_OPERATIONS ),
				  ligne,
				  style_couleur [ couleur_en_cours ] );
    }


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



    selectionne_ligne ( compte );

    gtk_clist_thaw ( GTK_CLIST ( CLIST_OPERATIONS ));
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
	    if ( operation -> tiers )
	    {
		liste_tmp = g_slist_find_custom ( liste_struct_tiers,
						  GINT_TO_POINTER ( operation -> tiers ),
						  ( GCompareFunc ) recherche_tiers_par_no );

		if ( liste_tmp )
		    return ( (( struct struct_tiers * )( liste_tmp -> data )) -> nom_tiers );
		else
		    return ( NULL );
	    }
	    else
		return ( NULL );
	    break;

	    /* mise en forme de l'ib */

	case 4:
	    temp = NULL;

	    if ( operation -> imputation )
	    {
		liste_tmp = g_slist_find_custom ( liste_struct_imputation,
						  GINT_TO_POINTER ( operation -> imputation ),
						  ( GCompareFunc ) recherche_imputation_par_no );

		if ( liste_tmp )
		{
		    GSList *liste_tmp_2;

		    liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( liste_tmp -> data )) -> liste_sous_imputation,
							GINT_TO_POINTER ( operation -> sous_imputation ),
							( GCompareFunc ) recherche_sous_imputation_par_no );
		    if ( liste_tmp_2 )
			temp = g_strconcat ( (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation,
					     " : ",
					     (( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation,
					     NULL );
		    else
			temp = (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation;
		}
	    }

	    return ( temp );
	    break;


	    /* mise en forme du débit */
	case 5:
	    if ( operation -> montant < -0.001 )
	      /* -0.001 is to handle float approximations */
	    {
		temp = g_strdup_printf ( "%4.2f", -operation -> montant );

		/* si la devise en cours est différente de celle de l'opé on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != operation -> devise )
		    devise_operation = g_slist_find_custom ( liste_struct_devises,
							     GINT_TO_POINTER ( operation -> devise ),
							     ( GCompareFunc ) recherche_devise_par_no ) -> data;

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

		/* si la devise en cours est différente de celle de l'opé on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != operation -> devise )
		    devise_operation = g_slist_find_custom ( liste_struct_devises,
							     GINT_TO_POINTER ( operation -> devise ),
							     ( GCompareFunc ) recherche_devise_par_no ) -> data;

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
	    if ( operation -> type_ope
		 &&
		 TYPES_OPES )
	    {
		liste_tmp = g_slist_find_custom ( TYPES_OPES,
						  GINT_TO_POINTER ( operation -> type_ope ),
						  (GCompareFunc) recherche_type_ope_par_no );

		if ( liste_tmp )
		    return ( ((struct struct_type_ope *)(liste_tmp -> data)) -> nom_type );
		else
		    return ( NULL );
	    }
	    else
		return ( NULL );

	    break;

	    /* mise en forme du no de rapprochement */

	case 10:
	    if ( operation -> no_rapprochement )
	    {
		liste_tmp = g_slist_find_custom ( liste_no_rapprochements,
						  GINT_TO_POINTER ( operation -> no_rapprochement ),
						  (GCompareFunc) recherche_no_rapprochement_par_no );

		if ( liste_tmp )
		    return ( ((struct struct_no_rapprochement *)(liste_tmp->data)) -> nom_rapprochement );
		else
		    return ( NULL );
	    }
	    else
		return ( NULL );

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
	    {
		/* c'est une categ : ss categ */

		liste_tmp = g_slist_find_custom ( liste_struct_categories,
						  GINT_TO_POINTER ( operation -> categorie ),
						  ( GCompareFunc ) recherche_categorie_par_no );

		if ( liste_tmp )
		{
		    GSList *liste_tmp_2;

		    liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
							GINT_TO_POINTER ( operation -> sous_categorie ),
							( GCompareFunc ) recherche_sous_categorie_par_no );
		    if ( liste_tmp_2 )
			temp = g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
					     " : ",
					     (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
					     NULL );
		    else
			temp = (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ;

		}
	    }
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
		    /* vérification si l'opération est ventilée*/

		    if ( operation -> operation_ventilee )
			temp = _("Breakdown of transaction");
		}
	    }
	    return ( temp );

	    break;

	    /* mise en forme R/P */

	case 13:
	    if ( operation -> pointe == CHECKED_TRANSACTION )
		return ( _("P") );
	    else
	    {
		if ( operation -> pointe == RECONCILED_TRANSACTION )
		    return ( _("R") );
		else
		{
		    if ( operation -> pointe == TELECHECKED_TRANSACTION )
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
	    return ( utils_itoa ( operation -> no_operation ));
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
gboolean selectionne_ligne_souris ( GtkCList *liste,
				GdkEventButton *evenement,
				gpointer data)
{
    gint ligne, colonne, x, y;
    gint i, j, col = -1;

    for ( i=0 ; i<4 ; i++ )
    {
      for ( j=0 ; j<7 ; j++ )
      {
	if ( tab_affichage_ope[i][j] == 13 )
	  col = j;
      }
    }

    /* si le click se situe dans les menus, c'est qu'on redimensionne, on fait rien */

    if ( evenement -> window != liste -> clist_window )
	return FALSE;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( CLIST_OPERATIONS ),
				   "button_press_event");

    /* Récupération des coordonnées de la souris */

    gdk_window_get_pointer ( GTK_CLIST ( CLIST_OPERATIONS ) -> clist_window,
			     &x,
			     &y,
			     FALSE );

    gtk_clist_get_selection_info ( GTK_CLIST ( CLIST_OPERATIONS ),
				   x,
				   y,
				   &ligne,
				   &colonne);

    if ( !gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ), ligne ) )
    {
	if ( evenement -> button == RIGHT_BUTTON )
	    popup_transaction_context_menu ( FALSE );
	return FALSE;
    }

    if ( etat.equilibrage &&
	 colonne == col &&
	 !(ligne % NB_LIGNES_OPE) )
    {
	pointe_equilibrage ( ligne );
	p_tab_nom_de_compte_variable =  p_tab_nom_de_compte + compte_courant;
    }

    /* Récupération de la 1ère ligne de l'opération cliquée */

    ligne = ligne / NB_LIGNES_OPE * NB_LIGNES_OPE;

    /*   vire l'ancienne sélection */

    gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ) );

    /* on met l'adr de la struct dans OPERATION_SELECTIONNEE */

    OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
						      ligne );

    selectionne_ligne( compte_courant );

    /* si on a cliqué sur la colonne P/R alors que la touche CTRL
       est enfoncée, alors on (dépointe l'opération */

    if ( ( ( evenement -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK ) &&
	 colonne == col )
	p_press ();

    if ( evenement -> type == GDK_2BUTTON_PRESS )
	edition_operation ();
    else if ( evenement -> button == RIGHT_BUTTON )
	popup_transaction_context_menu ( TRUE );
    else
	focus_a_la_liste ();

    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction traitement_clavier_liste */
/* gère le clavier sur la clist */
/******************************************************************************/
gboolean traitement_clavier_liste ( GtkCList *liste,
				    GdkEventKey *evenement,
				    gpointer origine)
{
    gint ligne;


    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste ),
				   "key_press_event");

    switch ( evenement -> keyval )
    {
	case GDK_Return :		/* entrée */
	case GDK_KP_Enter :

	    edition_operation ();
	    break;

	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
						   OPERATION_SELECTIONNEE );
	    if ( ligne )
	    {
		ligne= ligne - NB_LIGNES_OPE;

		OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
								  ligne );
		gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
		selectionne_ligne ( compte_courant );

	    }
	    return TRUE;

	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    if ( OPERATION_SELECTIONNEE != GINT_TO_POINTER ( -1 ) )
	    {
		ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
						       OPERATION_SELECTIONNEE );

		ligne= ligne + NB_LIGNES_OPE;

		OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
								  ligne );
		gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
		selectionne_ligne ( compte_courant );

	    }
	    return TRUE;

	case GDK_Delete:		/*  del  */
	    {
		if ( question_conditional_yes_no_hint ( _("Confirmation of transaction removal"),
							_("You are currently trying to remove a transaction.  There "
							  "is no undo for this.  Do you confirm the removal?"),
							&etat.display_message_remove_transaction))
		{
		    supprime_operation ( OPERATION_SELECTIONNEE );
		    echap_formulaire ();
		}
		
	    }
	    return TRUE;

	case GDK_P:			/* touche P */
	case GDK_p:			/* touche p */

	    if ( ( evenement -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		p_press ();
	    break;

	case GDK_r:			/* touche r */
	case GDK_R:			/* touche R */

	    if ( ( evenement -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
	    {
		if ( question_conditional_yes_no_hint ( _("Confirmation of manual (un)reconciliation"),
							_("You are currently trying to reconcile or unreconcile "
							  "manually a transaction which isn't a regular way to do.\n\n"
							  "Are you really sure to know what you do?"),
							 &etat.display_message_reconcile_transaction))
		    r_press ();
	    }
	    break;

	default: 
	    return FALSE;
    }

    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
/* Routine qui sélectionne ou désélectionne l'opération */
/******************************************************************************/
void selectionne_ligne ( gint compte )
{
    gint ligne;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

    gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ));

    ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
					   OPERATION_SELECTIONNEE );

    if ( ligne == -1 )
    {
	OPERATION_SELECTIONNEE = GINT_TO_POINTER ( -1 );
	ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
					       OPERATION_SELECTIONNEE );
    }


    switch ( NB_LIGNES_OPE )
    {
	case 4:
	    gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
				   ligne+3,
				   0);
	case 3:
	    gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
				   ligne+2,
				   0);
	case 2:
	    gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
				   ligne+1,
				   0);
	case 1:
	    gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
				   ligne,
				   0);
    }

    if ( gtk_clist_row_is_visible ( GTK_CLIST ( CLIST_OPERATIONS ), ligne + NB_LIGNES_OPE - 1) != GTK_VISIBILITY_FULL ||
	 gtk_clist_row_is_visible ( GTK_CLIST ( CLIST_OPERATIONS ), ligne ) != GTK_VISIBILITY_FULL )
    {
	if ( ligne > gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value  / GTK_CLIST ( CLIST_OPERATIONS ) -> row_height  )
	    gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
			       ligne + NB_LIGNES_OPE - 1,
			       0,
			       1,
			       0 );
	else
	    gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
			       ligne,
			       0,
			       0,
			       0 );
    }
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

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    operation = OPERATION_SELECTIONNEE;
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
	if ( gtk_widget_get_style ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ) == style_entree_formulaire[ENGRIS] )
	{
	    entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_DATE] );

	    if ( gtk_widget_get_style ( widget_formulaire_operations[TRANSACTION_FORM_VALUE_DATE] ) == style_entree_formulaire[ENGRIS] )
	    {
		if ( !derniere_date )
		    derniere_date = gsb_today();

		gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ),
				     derniere_date );
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
			 utils_itoa ( operation -> no_operation ));

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
	liste_tmp = g_slist_find_custom ( liste_struct_tiers,
					  GINT_TO_POINTER ( operation -> tiers ),
					  ( GCompareFunc ) recherche_tiers_par_no );

	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_PARTY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_PARTY] ),
				(( struct struct_tiers * )( liste_tmp -> data )) -> nom_tiers );
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

    /* si l'opération est relevé , on désensitive les entrées de crédit et débit */

    if ( operation -> pointe == RECONCILED_TRANSACTION )
    {
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_CREDIT],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_DEBIT],
				   FALSE );
    }

    /* mise en forme de la devise */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_DEVISE] ),
				  g_slist_position ( liste_struct_devises,
						     g_slist_find_custom ( liste_struct_devises,
									   GINT_TO_POINTER ( operation -> devise ),
									   ( GCompareFunc ) recherche_devise_par_no )));

    /*   si la devise n'est pas celle du compte ni l'euro si le compte va y passer, affiche le bouton change */

    if ( !devise_compte
	 ||
	 devise_compte -> no_devise != DEVISE )
	devise_compte = g_slist_find_custom ( liste_struct_devises,
					      GINT_TO_POINTER ( DEVISE ),
					      ( GCompareFunc ) recherche_devise_par_no ) -> data;
    devise = g_slist_find_custom ( liste_struct_devises,
				   GINT_TO_POINTER ( operation -> devise ),
				   ( GCompareFunc ) recherche_devise_par_no ) -> data;

    if ( !( devise -> no_devise == DEVISE
	    ||
	    ( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	    ||
	    ( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
	gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_CHANGE] );

    /* mise en forme des catégories */

    liste_tmp = g_slist_find_custom ( liste_struct_categories,
				      GINT_TO_POINTER ( operation -> categorie ),
				      ( GCompareFunc ) recherche_categorie_par_no );

    if ( liste_tmp )
    {
	GSList *liste_tmp_2;

	liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
					    GINT_TO_POINTER ( operation -> sous_categorie ),
					    ( GCompareFunc ) recherche_sous_categorie_par_no );

	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] );

	if ( liste_tmp_2 )
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
				    g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
						  " : ",
						  (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
						  NULL ));
	else
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
				    (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ );

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

	    /* si l'opération est relevée , on empèche le changement de virement */

	    if ( operation -> pointe == RECONCILED_TRANSACTION )
		gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY],
					   FALSE );

	    /* récupération de la contre opération */

	    operation_2 = g_slist_find_custom ( LISTE_OPERATIONS,
						GINT_TO_POINTER ( operation -> relation_no_operation ),
						(GCompareFunc) recherche_operation_par_no ) -> data;

	    /* 	  si la contre opération est relevée , on désensitive les categ et les montants */
	    if ( operation_2 -> pointe == RECONCILED_TRANSACTION )
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
		gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_operations[TRANSACTION_FORM_CONTRA] ),
				      "compte_virement",
				      GINT_TO_POINTER ( operation -> relation_no_compte ));
		gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_CONTRA] );
	    }
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	}
    }

    /* mise en forme si l'opération est ventilée*/

    if ( operation -> operation_ventilee )
    {
	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_CATEGORY] ),
				_("Breakdown of transaction") );
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET], FALSE);
	gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_BREAKDOWN] );

	/* met la liste des opé de ventilation dans liste_adr_ventilation */

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

    liste_tmp = g_slist_find_custom ( liste_struct_imputation,
				      GINT_TO_POINTER ( operation -> imputation ),
				      ( GCompareFunc ) recherche_imputation_par_no );

    if ( liste_tmp )
    {
	GSList *liste_tmp_2;

	entree_prend_focus ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET]);

	liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( liste_tmp -> data )) -> liste_sous_imputation,
					    GINT_TO_POINTER ( operation -> sous_imputation ),
					    ( GCompareFunc ) recherche_sous_imputation_par_no );
	if ( liste_tmp_2 )
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET] ),
				    g_strconcat ( (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation,
						  " : ",
						  (( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation,
						  NULL ));
	else
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET] ),
				    (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation );
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

    /* numéro de rapprochement */

    if ( operation -> no_rapprochement )
    {
	gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[TRANSACTION_FORM_RECONCILIATION]),
			     ((gchar *) ((struct struct_no_rapprochement *) g_slist_find_custom ( liste_no_rapprochements,
					     GINT_TO_POINTER ( operation -> no_rapprochement ),
					     (GCompareFunc) recherche_no_rapprochement_par_no ) -> data) -> nom_rapprochement));
    }
    else
	gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[TRANSACTION_FORM_RECONCILIATION]),
			     _("No reconciliation number"));

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
    gint i, j, col = -1;

    for ( i=0 ; i<4 ; i++ )
    {
      for ( j=0 ; j<7 ; j++ )
      {
	if ( tab_affichage_ope[i][j] == 13 )
	  col = j;
      }
    }

    if ( col == -1 )
      return;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    /* si on est sur l'opération vide -> on se barre */

    if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER ( -1 ) )
	return;

    if ( OPERATION_SELECTIONNEE -> pointe == RECONCILED_TRANSACTION )
	return;

    if ( OPERATION_SELECTIONNEE -> pointe )
    {
	montant = calcule_montant_devise_renvoi ( OPERATION_SELECTIONNEE -> montant,
						  DEVISE,
						  OPERATION_SELECTIONNEE -> devise,
						  OPERATION_SELECTIONNEE -> une_devise_compte_egale_x_devise_ope,
						  OPERATION_SELECTIONNEE -> taux_change,
						  OPERATION_SELECTIONNEE -> frais_change );

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees - montant;

	SOLDE_POINTE = SOLDE_POINTE - montant;
	OPERATION_SELECTIONNEE -> pointe = UNCHECKED_TRANSACTION;

	gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							    OPERATION_SELECTIONNEE ),
			     col,
			     NULL );

	modification_fichier( TRUE );
    }
    else
    {
	montant = calcule_montant_devise_renvoi ( OPERATION_SELECTIONNEE -> montant,
						  DEVISE,
						  OPERATION_SELECTIONNEE -> devise,
						  OPERATION_SELECTIONNEE -> une_devise_compte_egale_x_devise_ope,
						  OPERATION_SELECTIONNEE -> taux_change,
						  OPERATION_SELECTIONNEE -> frais_change );

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees + montant;

	SOLDE_POINTE = SOLDE_POINTE + montant;
	OPERATION_SELECTIONNEE -> pointe = CHECKED_TRANSACTION;

	gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							    OPERATION_SELECTIONNEE ),
			     col,
			     _("P"));
	modification_fichier( TRUE );
    }

    /* si c'est une opé ventilée , on recherche les opés filles pour leur mettre le même pointage que la mère */

    if ( OPERATION_SELECTIONNEE -> operation_ventilee )
    {
	/* p_tab est déjà pointé sur le compte courant */

	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *ope_fille;

	    ope_fille = liste_tmp -> data;

	    if ( ope_fille -> no_operation_ventilee_associee == OPERATION_SELECTIONNEE -> no_operation )
		ope_fille -> pointe = OPERATION_SELECTIONNEE -> pointe;

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

/*     si on a choisi de classer la liste avec les rp, on met à jour ici */

    if ( etat.classement_rp )
	demande_mise_a_jour_tous_comptes ();

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
    gint i, j, col = -1;

    for ( i=0 ; i<4 ; i++ )
    {
      for ( j=0 ; j<7 ; j++ )
      {
	if ( tab_affichage_ope[i][j] == 13 )
	  col = j;
      }
    }

    if ( col == -1 )
      return;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    /* si on est sur l'opération vide -> on se barre */

    if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER ( -1 ) )
	return;

    if ( OPERATION_SELECTIONNEE -> pointe == UNCHECKED_TRANSACTION )
    {
	/* on relève l'opération */

	OPERATION_SELECTIONNEE -> pointe = RECONCILED_TRANSACTION;

	/* on met soit le R, soit on change la sélection vers l'opé suivante */

	if ( AFFICHAGE_R )
	    gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
				 gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
								OPERATION_SELECTIONNEE ),
				 col,
				 _("R"));
	else
	{
	    /* l'opération va disparaitre, on met donc la sélection sur l'opé suivante */

	    gint ligne;

	    ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
						   OPERATION_SELECTIONNEE );
	    OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							      ligne + NB_LIGNES_OPE );
	    remplissage_liste_operations ( compte_courant );
	}

	modification_fichier( TRUE );
    }
    else
	if ( OPERATION_SELECTIONNEE -> pointe == RECONCILED_TRANSACTION )
	{
	    /* dérelève l'opération */
	    OPERATION_SELECTIONNEE -> pointe = UNCHECKED_TRANSACTION;
	    gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
				 gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
								OPERATION_SELECTIONNEE ),
				 col,
				 NULL );

	    modification_fichier( TRUE );
	}

    /* À ce niveau, on reteste OPERATION_SELECTIONNEE car comme on a peut être */
    /* déplacé la sélection vers le bas, elle peut être revenue à -1 */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER ( -1 ) )
	return;

    /* si c'est une ventilation, on fait le tour des opérations du compte pour */
    /* rechercher les opérations de ventilation associées à cette ventilation */

    if ( OPERATION_SELECTIONNEE -> operation_ventilee )
    {
	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( operation -> no_operation_ventilee_associee == OPERATION_SELECTIONNEE -> no_operation )
		operation -> pointe = OPERATION_SELECTIONNEE -> pointe;

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

    if ( operation -> pointe == RECONCILED_TRANSACTION )
    {
	dialogue_error ( _("Impossible to delete a reconciled transaction.") );
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

	    if ( ope_liee -> pointe == RECONCILED_TRANSACTION )
	    {
		dialogue_error ( _("The contra-transaction of this transfer is reconciled, deletion impossible.") );
		return;
	    }

	    ope_liee -> relation_no_operation = 0;
	    ope_liee -> relation_no_compte = 0;

	    supprime_operation ( ope_liee );
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

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

		    if ( contre_operation -> pointe == RECONCILED_TRANSACTION )
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

    if ( OPERATION_SELECTIONNEE == operation )
    {
	gint ligne;

	ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
					       operation );
	OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							  ligne + NB_LIGNES_OPE );
	gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ) );
	selectionne_ligne ( operation -> no_compte );
    }

    /* supprime l'opération dans la liste des opé */

    LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS,
					operation );

    if ( g_slist_find ( echeances_saisies, operation ) )
    {
	echeances_saisies = g_slist_remove ( echeances_saisies, operation );
	update_liste_echeances_auto_accueil ();
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;
    }

    free ( operation );
    NB_OPE_COMPTE--;

    /* on met à jour, immédiatement si on a la liste affichée */

    MISE_A_JOUR = 1;

    if ( no_compte == compte_courant )
	verification_mise_a_jour_liste ();

    /* si on est en train d'équilibrer => recalcule le total pointé */

    if ( etat.equilibrage )
    {
	GSList *pointeur_liste_ope;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	pointeur_liste_ope = LISTE_OPERATIONS;
	operations_pointees = 0;

	while ( pointeur_liste_ope )
	{
	    struct structure_operation *operation;

	    operation = pointeur_liste_ope -> data;

	    if ( operation -> pointe == CHECKED_TRANSACTION )
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
    mise_a_jour_soldes_minimaux ();

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
gboolean changement_taille_liste_ope ( GtkWidget *clist,
				       GtkAllocation *allocation,
				       gint *compte )
{
    gint i, j;
    gint largeur;
    gint col0, col1, col2, col3, col4, col5, col6;

    /* Yet another kludge to avoid breakage due to #392 fix (closes: #470) */
    if ( (gint) compte != compte_courant_onglet )
	return TRUE;

    /* si la largeur de grisbi est < LOWEST_RESOLUTION, on fait rien */

    if ( window -> allocation.width < LOWEST_RESOLUTION )
	return TRUE;

    /*     pour éviter que le système ne s'emballe... */
    /*     encore plus grosse magouille avec allocation_encore_avant pour éviter */
    /* 	un joli effet glissant pendant l'affichage d'un bouton du formulaire */
    /* 	c'est résolu dans l'instable de manière plus jolie !! */

    if ( allocation_compte_precedent == (gint) compte &&
	 allocation -> width == allocation_precedente )
	return TRUE;

    if ( allocation_compte_precedent == (gint) compte &&
	 allocation -> width == allocation_encore_avant )
	return TRUE;

     allocation_precedente = allocation_encore_avant;
     allocation_encore_avant = allocation -> width;
     allocation_compte_precedent = (gint) compte;

    if ( allocation )
	largeur = allocation -> width;
    else
	largeur = clist -> allocation.width;

    /* si la largeur est automatique, on change la largeur des colonnes */
    /* sinon, on y met les valeurs fixes */

    for ( j=0 ; j<nb_comptes ; j++ )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + j;

	if ( etat.largeur_auto_colonnes )
	    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
		gtk_clist_set_column_width ( GTK_CLIST ( CLIST_OPERATIONS ),
					     i,
					     rapport_largeur_colonnes[i] * largeur / 100 );
	else
	    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
		gtk_clist_set_column_width ( GTK_CLIST ( CLIST_OPERATIONS ),
					     i,
					     taille_largeur_colonnes[i] );
    }

    /* met les entrées du formulaire selon une taille proportionnelle */

    col0 = largeur * 5 / 100;
    col1 = largeur * 12 / 100;
    col2 = largeur * 30 / 100;
    col3 = largeur * 12 / 100;
    col4 = largeur * 12 / 100;
    col5 = largeur * 12 / 100;
    col6 = largeur * 12 / 100;

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
			   col3+col4,
			   FALSE  );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_RECONCILIATION] ),
			   col5,
			   FALSE  );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[TRANSACTION_FORM_MODE] ),
			   col6,
			   FALSE  );

    return TRUE;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction focus_a_la_liste */
/* donne le focus à la liste des opé en cours */
/******************************************************************************/
void focus_a_la_liste ( void )
{
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
    gtk_widget_grab_focus ( GTK_WIDGET ( CLIST_OPERATIONS ) );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction  demande_mise_a_jour_tous_comptes */
/* met la variable MISE_A_JOUR de tous les comptes à 1 */
/* ce qui fait que lorsqu'ils seront affichés, ils seront mis à jour avant */
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

    verification_mise_a_jour_liste();

    p_tab_nom_de_compte_variable = save_p_tab;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction verification_mise_a_jour_liste */
/* appelée à chaque affichage de la liste */
/* vérifie que la liste ne doit pas êre réaffichée */
/******************************************************************************/
void verification_mise_a_jour_liste ( void )
{
    GtkAdjustment *ajustement;
    gfloat haut, bas, value, page_size;
    gfloat new_bas, new_value;
    gint compte;

    compte = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_listes_operations )) - 1;

    if ( compte < 0 )
	return;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

    if ( !MISE_A_JOUR )
	return;

    ajustement = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS ));

    haut = ajustement -> upper;
    bas = ajustement -> lower + ajustement -> page_size;
    value = ajustement -> value;
    page_size = ajustement -> page_size;

    LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				      (GCompareFunc) classement_sliste );

    remplissage_liste_operations ( GPOINTER_TO_INT ( compte ) );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

    gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
		       gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
						      OPERATION_SELECTIONNEE ),
		       0,
		       0.5,
		       0 );
    selectionne_ligne ( compte );
 
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
/* appelée lorsqu'on évite une opé et que le formulaire n'était pas visible, */
/* pour éviter que la ligne éditée se retrouve hors du champ de vision */
/******************************************************************************/
void verifie_ligne_selectionnee_visible ( void )
{
    selectionne_ligne ( compte_courant );

    gtk_signal_disconnect_by_func ( GTK_OBJECT ( frame_droite_bas ),
				    GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_visible ),
				    NULL );
}
/******************************************************************************/

/******************************************************************************/
gboolean changement_taille_colonne ( GtkWidget *clist, gint colonne, gint largeur )
{
    if ( !GTK_WIDGET_REALIZED(clist) )
    {
      return TRUE;
    }

    /* Why would we need to reset the form here? */
/*     echap_formulaire(); */
    taille_largeur_colonnes[colonne] = largeur;

    return TRUE;
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

    if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER(-1) )
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

    /* New transaction from */
    menu_item = gtk_image_menu_item_new_with_label ( _("Use selected transaction as a template") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_NEW,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", new_transaction_from_selected, NULL );
    gtk_menu_append ( menu, menu_item );

    /* Delete transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Delete transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_DELETE,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", remove_transaction, NULL );
    if ( !full || 
	 OPERATION_SELECTIONNEE -> pointe == RECONCILED_TRANSACTION ||
	 OPERATION_SELECTIONNEE -> pointe == TELECHECKED_TRANSACTION )
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
    if ( !full || 
	 OPERATION_SELECTIONNEE -> pointe == RECONCILED_TRANSACTION ||
	 OPERATION_SELECTIONNEE -> pointe == TELECHECKED_TRANSACTION )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Add accounts submenu */
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(menu_item), 
				GTK_WIDGET(creation_option_menu_comptes_nonclos(GTK_SIGNAL_FUNC(move_selected_operation_to_account), FALSE)) );

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

    if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER(-1) )
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

    gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ) );
    echap_formulaire();

    OPERATION_SELECTIONNEE = GINT_TO_POINTER(-1);
    edition_operation ();

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}


/**
 * Remove selected transaction if any.
 */
void remove_transaction ()
{
    if (! assert_selected_transaction()) return;

    supprime_operation ( OPERATION_SELECTIONNEE );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
    echap_formulaire ();
}


/**
 * Clone selected transaction if any.  Update user interface as well.
 */
void clone_selected_transaction ()
{
    gint compte;

    compte = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_listes_operations )) - 1;

    if ( compte < 0 )
	return;
    
    if (! assert_selected_transaction()) return;

    OPERATION_SELECTIONNEE = clone_transaction ( OPERATION_SELECTIONNEE );

    gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
		       gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
						      OPERATION_SELECTIONNEE ),
		       0,
		       0.5,
		       0 );
    selectionne_ligne ( compte );

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

    mise_a_jour_tiers ();
    mise_a_jour_categ ();
    mise_a_jour_imputation ();

    modification_fichier ( TRUE );
}

/**
 * Clone selected transaction but set date, reset value date and checking.
 */
void new_transaction_from_selected ()
{
    gint compte;

    compte = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_listes_operations )) - 1;

    if ( compte < 0 )
	return;
    
    if (! assert_selected_transaction()) return;

    OPERATION_SELECTIONNEE = new_transaction_from ( OPERATION_SELECTIONNEE );

    gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
		       gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
						      OPERATION_SELECTIONNEE ),
		       0,
		       0.5,
		       0 );
    selectionne_ligne ( compte );

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

    mise_a_jour_tiers ();
    mise_a_jour_categ ();
    mise_a_jour_imputation ();

    modification_fichier ( TRUE );
    edition_operation ();
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

    memcpy(new_transaction, operation, sizeof(struct structure_operation) );

    new_transaction -> no_operation = 0;
    new_transaction -> no_rapprochement = 0;

    if ( operation -> pointe == RECONCILED_TRANSACTION ||
	 operation -> pointe == TELECHECKED_TRANSACTION )
      {
	new_transaction -> pointe = UNCHECKED_TRANSACTION;
      }

    ajout_operation ( new_transaction );

    if ( new_transaction -> relation_no_operation != 0 || new_transaction -> relation_no_compte != 0 )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + new_transaction -> relation_no_compte;
	validation_virement_operation ( new_transaction, 0, NOM_DU_COMPTE );
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
 * Clone transaction but set date, reset value date and checking.
 * If it is a breakdown or a transfer, perform all
 * needed operations, like cloning associated transactions as well.
 *
 * \param operation Initial transaction to clone
 *
 * \return A newly created operation.
 */
struct structure_operation *  new_transaction_from ( struct structure_operation *pTemplateTransaction )
{
  struct structure_operation *pNewTransaction, *pScheduledTransaction;

  pNewTransaction = (struct structure_operation *) malloc ( sizeof(struct structure_operation) );
  if ( !pNewTransaction )
  {
    dialogue ( _("Cannot allocate memory, bad things will happen soon") );
    return(FALSE);
  }

  memcpy( pNewTransaction, pTemplateTransaction, sizeof(struct structure_operation) );

  pNewTransaction -> no_operation = 0;

  pNewTransaction -> pointe = UNCHECKED_TRANSACTION;
  pNewTransaction -> date = gdate_today();
  pNewTransaction -> jour = g_date_day ( pNewTransaction -> date ) ;
  pNewTransaction -> mois = g_date_month ( pNewTransaction -> date ) ;
  pNewTransaction -> annee = g_date_year ( pNewTransaction -> date ) ;
  pNewTransaction -> date_bancaire = NULL;
  pNewTransaction -> no_rapprochement = 0;

  /* Si l'opération possède un exercice, alors voir
     s'il n'existe pas un exercice plus approprié */
  if ( pTemplateTransaction -> no_exercice )
  {
    GSList *pFinancialYearList;
    gboolean found = FALSE;

    pFinancialYearList = liste_struct_exercices;

    while ( pFinancialYearList )
    {
      struct struct_exercice *pFinancialYear;

      pFinancialYear = pFinancialYearList -> data;

      if ( g_date_compare ( pFinancialYear -> date_debut, pNewTransaction -> date ) <= 0 &&
	   g_date_compare ( pFinancialYear -> date_fin, pNewTransaction -> date ) >= 0 )
      {
	pNewTransaction -> no_exercice = pFinancialYear -> no_exercice;
	found = TRUE;
      }

      pFinancialYearList = pFinancialYearList -> next;
    }
    
    /* Si on n'a pas trouvé d'exercice,
       on met le numéro à 0, c-à-d aucun */
    if (!found)
      pNewTransaction -> no_exercice = 0;
  }

  ajout_operation ( pNewTransaction );

  if ( pNewTransaction -> relation_no_operation != 0 || pNewTransaction -> relation_no_compte != 0 )
  {
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + pNewTransaction -> relation_no_compte;
    validation_virement_operation ( pNewTransaction, 0, NOM_DU_COMPTE );
  }

  if ( pTemplateTransaction -> operation_ventilee )
  {
    GSList *pTransactionList;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + pTemplateTransaction -> no_compte;

    pTransactionList = LISTE_OPERATIONS;

    while ( pTransactionList )
    {
      struct structure_operation *pTransactionTmp;

      pTransactionTmp = pTransactionList -> data;

      if ( pTransactionTmp -> no_operation_ventilee_associee == pTemplateTransaction -> no_operation )
      {
	pScheduledTransaction = new_transaction_from ( pTransactionTmp );
	pScheduledTransaction -> no_operation_ventilee_associee = pNewTransaction -> no_operation;
      }

      pTransactionList = pTransactionList -> next;
    }
  }
  return pNewTransaction;
}



/**
 * Move selected transaction to another account.  Normally called as a
 * handler.
 *
 * \param menu_item The GtkMenuItem that triggered this handler.
 */
void move_selected_operation_to_account ( GtkMenuItem * menu_item )
{
    gint target_account, source_account;

    if (! assert_selected_transaction()) return;

    source_account = NO_COMPTE;
    target_account = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT(menu_item), 
							     "no_compte" ) );  

    if ( move_operation_to_account ( OPERATION_SELECTIONNEE, target_account ))
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + source_account;
	MISE_A_JOUR = 1;
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + target_account;
	MISE_A_JOUR = 1;
	verification_mise_a_jour_liste ();

	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	mise_a_jour_tiers ();
	mise_a_jour_categ ();
	mise_a_jour_imputation ();
	update_liste_comptes_accueil ();

	modification_fichier ( TRUE );
    }
}



/**
 * Move selected transaction to another account.  Normally called as a
 * handler.
 *
 * \param menu_item The GtkMenuItem that triggered this handler.
 */
void move_selected_operation_to_account_nb ( gint *account )
{
    gint target_account, source_account;

    if (! assert_selected_transaction()) return;

    source_account = NO_COMPTE;
    target_account = GPOINTER_TO_INT ( account ) ;  

    if ( move_operation_to_account ( OPERATION_SELECTIONNEE, target_account ))
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + source_account;
	MISE_A_JOUR = 1;
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + target_account;
	MISE_A_JOUR = 1;
	verification_mise_a_jour_liste ();

	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	mise_a_jour_tiers ();
	mise_a_jour_categ ();
	mise_a_jour_imputation ();

	modification_fichier ( TRUE );
    }
}




/**
 * Move transaction to another account
 *
 * \param transaction Transaction to move to other account
 * \param account Account to move the transaction to
 */
gboolean move_operation_to_account ( struct structure_operation * transaction,
				     gint account )
{
    gpointer ** tmp = p_tab_nom_de_compte_variable;

    if ( transaction -> relation_no_compte )
    {
	struct structure_operation * contra_transaction;

	/* 	l'opération est un virement, si on veut la déplacer vers le compte */
	/* 	    viré, on refuse */

	if ( transaction -> relation_no_compte == account )
	{
	    dialogue_error ( _("Cannot move a transfer on his contra-account"));
	    return FALSE;
	}

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + transaction -> relation_no_compte;
	contra_transaction =  g_slist_find_custom ( LISTE_OPERATIONS,
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
	    }

	    liste_tmp = liste_tmp -> next;
	}

	g_slist_free ( liste_tmp );
    }

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + transaction -> no_compte;

    LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS, transaction );
    NB_OPE_COMPTE--;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + account;
    LISTE_OPERATIONS = g_slist_sort ( g_slist_append ( LISTE_OPERATIONS, transaction ),
				      (GCompareFunc) classement_sliste );
    NB_OPE_COMPTE++;

    transaction -> no_compte = account;
    p_tab_nom_de_compte_variable = tmp;

    return TRUE;
}



/**
 * Convert selected transaction to a template of scheduled transaction
 * via schedule_transaction().
 */
void schedule_selected_transaction ()
{
    struct operation_echeance * echeance;

    if (! assert_selected_transaction()) return;

    echeance = schedule_transaction ( OPERATION_SELECTIONNEE );

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
/*     on va changer à la prochaine version, dès que c'est pas un virement -> -1 */

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
    gsliste_echeances = g_slist_append ( gsliste_echeances,
					 echeance );

/*     on récupère les opé de ventil si c'était une opé ventilée*/

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
		/*     on va changer à la prochaine version, dès que c'est pas un virement -> -1 */

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
		gsliste_echeances = g_slist_append ( gsliste_echeances,
							    echeance_de_ventil );
	    }
	    liste_tmp = liste_tmp -> next;
	}
    }
    return echeance;
}
