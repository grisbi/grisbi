/* ************************************************************************** */
/* fichier qui s'occupe de tout ce qui concerne l'échéancier                  */
/* 			echeances_liste.c                                     */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)  	      */
/* 			http://www.grisbi.org   			      */
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
#include "echeancier_formulaire_constants.h"
#include "echeancier_liste_constants.h"

/*START_INCLUDE*/
#include "echeancier_liste.h"
#include "echeancier_formulaire.h"
#include "utils_exercices.h"
#include "type_operations.h"
#include "barre_outils.h"
#include "echeancier_ventilation.h"
#include "devises.h"
#include "utils_devises.h"
#include "dialog.h"
#include "operations_formulaire.h"
#include "utils_dates.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "gtk_combofix.h"
#include "utils_str.h"
#include "echeancier_infos.h"
#include "traitement_variables.h"
#include "utils_categories.h"
#include "utils_ib.h"
#include "utils_comptes.h"
#include "operations_liste.h"
#include "utils_tiers.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajuste_scrolling_liste_echeances_a_selection ( void );
static gboolean changement_taille_liste_echeances ( GtkWidget *tree_view,
					     GtkAllocation *allocation );
static struct operation_echeance *cherche_echeance_from_ligne ( gint ligne );
static GtkTreeIter *cherche_iter_echeance ( struct operation_echeance *echeance );
static gint cherche_ligne_echeance ( struct operation_echeance *echeance );
static gboolean click_ligne_echeance ( GtkWidget *tree_view,
				GdkEventButton *evenement );
static GDate *date_fin_affichage_liste_echeance ( void );
static GDate *date_suivante_echeance ( struct operation_echeance *echeance,
				GDate *pGDateCurrent );
static gboolean traitement_clavier_liste_echeances ( GtkWidget *tree_view_liste_echeances,
					      GdkEventKey *evenement );
static void update_couleurs_background_echeancier ( void );
/*END_STATIC*/



/* contient la largeur de la colonne en % de la largeur de la liste */

gint scheduler_col_width[NB_COLS_SCHEDULER] ;

/* cette variable empêche qu'au redimensionnement de la liste */
/* les colonnes soient diminuées */

gint bloque_taille_colonne;

/* cette variable est à 1 une fois qu'une échéance est sélectionnée */
/* remise à zéro lors d'un réaffichage de la liste ou de background */

gint selection_echeance_finie;

GtkWidget *frame_formulaire_echeancier;
GtkWidget *formulaire_echeancier;

GtkWidget *tree_view_liste_echeances;
GtkTreeViewColumn *colonnes_liste_echeancier[NB_COLS_SCHEDULER];

GtkWidget *bouton_saisir_echeancier;
struct operation_echeance *echeance_selectionnnee;

GSList *liste_struct_echeances;                 /* contient la liste des struct échéances */
gint nb_echeances;
gint no_derniere_echeance;

gint decalage_echeance;      /* nb de jours avant l'échéance pour prévenir */

gint ancienne_largeur_echeances;

GSList *echeances_a_saisir;
GSList *echeances_saisies;


/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_j_m_a;
extern gint affichage_echeances_perso_nb_libre;
extern GtkWidget *bouton_personnalisation_affichage_echeances;
extern GtkWidget *bouton_valider_echeance_perso;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_grise;
extern GdkColor couleur_selection;
extern struct struct_devise *devise_compte;
extern GtkWidget *entree_personnalisation_affichage_echeances;
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GdkGC *gc_separateur_operation;
extern gint hauteur_ligne_liste_opes;
extern GtkWidget *label_saisie_modif;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern GtkTreeStore *model;
extern gdouble taux_de_change[2];
extern GtkWidget *tree_view;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/*END_EXTERN*/



/*****************************************************************************/
/* fonction creation_liste_echeances					     */
/* renvoie la tree_view pour y mettre les échéances				     */
/*****************************************************************************/
GtkWidget *creation_liste_echeances ( void )
{
    GtkWidget *vbox;
    GtkWidget *scrolled_window;
    gint i;

    gchar *titres_echeance[] = {
	_("Date"),
	_("Account"),
	_("Third party"),
	_("Frequency"),
	_("Mode"),
	_("Notes"),
	_("Amount"),
	_("Balance")
    };

    gfloat col_justs[] = {
	COLUMN_CENTER,
	COLUMN_LEFT,
	COLUMN_LEFT,
	COLUMN_CENTER,
	COLUMN_CENTER,
	COLUMN_LEFT,
	COLUMN_RIGHT,
	COLUMN_RIGHT
    };
    GtkListStore *store;


    /*   à la base, on a une vbox */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     5 );
    gtk_widget_show ( vbox );


    /* création de la barre d'outils */

    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 creation_barre_outils_echeancier(),
			 FALSE,
			 FALSE,
			 0 );

    /* création de la scrolled window */

    scrolled_window = gtk_scrolled_window_new ( NULL,
						NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 5 );
    gtk_widget_show ( scrolled_window );

    /*     création du tree view */

    tree_view_liste_echeances = gtk_tree_view_new ();
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			tree_view_liste_echeances );
    gtk_widget_show ( tree_view_liste_echeances );

    /*     on ne peut sélectionner qu'une ligne */

    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection ( GTK_TREE_VIEW( tree_view_liste_echeances ))),
				  GTK_SELECTION_NONE );

    /* 	met en place la grille */

    if ( etat.affichage_grille )
	g_signal_connect_after ( G_OBJECT ( tree_view_liste_echeances ),
				 "expose-event",
				 G_CALLBACK ( affichage_traits_liste_echeances ),
				 NULL );

    /* vérifie le simple ou double click */

    g_signal_connect ( G_OBJECT ( tree_view_liste_echeances ),
		       "button_press_event",
		       G_CALLBACK ( click_ligne_echeance ),
		       NULL );

    /* vérifie la touche entrée, haut et bas */

    g_signal_connect ( G_OBJECT ( tree_view_liste_echeances ),
		       "key_press_event",
		       G_CALLBACK ( traitement_clavier_liste_echeances ),
		       NULL );

    /*     ajuste les colonnes si modification de la taille */

    g_signal_connect ( G_OBJECT ( tree_view_liste_echeances ),
		       "size-allocate",
		       G_CALLBACK ( changement_taille_liste_echeances ),
		       NULL );


    /*     création des colonnes */

    for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
    {
	GtkCellRenderer *cell_renderer;

	cell_renderer = gtk_cell_renderer_text_new ();

	g_object_set ( G_OBJECT (GTK_CELL_RENDERER ( cell_renderer )),
		       "xalign",
		       col_justs[i],
		       NULL );

	colonnes_liste_echeancier[i] = gtk_tree_view_column_new_with_attributes ( titres_echeance[i],
										  cell_renderer,
										  "text", i,
										  "cell-background-gdk", 8,
										  "font-desc", 12,
										  NULL );
	gtk_tree_view_column_set_alignment ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ),
					     col_justs[i] );

	gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view_liste_echeances ),
				      GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ));
	gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ),
					     TRUE );
	gtk_tree_view_column_set_resizable ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ),
					     TRUE );
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ),
					  GTK_TREE_VIEW_COLUMN_FIXED );

/* 	g_signal_connect ( G_OBJECT ( colonnes_liste_echeancier[i] ), */
/* 			   "clicked", */
/* 			   G_CALLBACK ( click_sur_titre_colonne_echeancier ), */
/* 			   GINT_TO_POINTER (i)); */
    }

    /*     on a créé les colonnes, on va afficher/cacher selon la préf de l'utilisateur */

    etat.affichage_commentaire_echeancier = 1 - etat.affichage_commentaire_echeancier;
    affiche_cache_commentaire_echeancier ();

    /*     on crée le list_store maintenant et on l'associe vide au tree_view */
    /* création de la liste des échéances */

    store = gtk_list_store_new ( SCHEDULER_COL_NB_TOTAL,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 GDK_TYPE_COLOR,
				 GDK_TYPE_COLOR,
				 G_TYPE_STRING,
				 G_TYPE_POINTER,
				 PANGO_TYPE_FONT_DESCRIPTION );

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view_liste_echeances),
			      GTK_TREE_MODEL ( store ));


    remplissage_liste_echeance();
    update_couleurs_background_echeancier ();
    echeance_selectionnnee = NULL;
    selectionne_echeance ( GINT_TO_POINTER (-1));

    return vbox;

}
/*****************************************************************************/





/*****************************************************************************/
/* cette fonction teste la touche entrée sur la liste d'échéances	     */
/*****************************************************************************/
gboolean traitement_clavier_liste_echeances ( GtkWidget *tree_view_liste_echeances,
					      GdkEventKey *evenement )
{
    gint ligne_selectionnee;
    struct operation_echeance *echeance;

    switch ( evenement->keyval )
    {
	case GDK_Return :		/* touches entrée */
	case GDK_KP_Enter :

	    edition_echeance ();
	    break;


	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :


	    ligne_selectionnee = cherche_ligne_echeance ( echeance_selectionnnee );

	    if ( ligne_selectionnee )
	    {
		/* 		on saute les doubles d'échéances */

		do
		    ligne_selectionnee--;
		while ( !(echeance = cherche_echeance_from_ligne ( ligne_selectionnee ))
			&&
			ligne_selectionnee ); 

		selectionne_echeance ( echeance );
	    }
	    break;


	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :


	    if ( echeance_selectionnnee != GINT_TO_POINTER ( -1 ) )
	    {
		ligne_selectionnee = cherche_ligne_echeance ( echeance_selectionnnee );

		/* 		on saute les doubles d'échéances */

		do
		    ligne_selectionnee++;
		while ( !(echeance = cherche_echeance_from_ligne ( ligne_selectionnee ))); 

		selectionne_echeance ( echeance );
	    }
	    break;


	case GDK_Delete :               /*  del  */

	    supprime_echeance ( echeance_selectionnnee );
	    break;
    }

    return ( TRUE );    
}
/*****************************************************************************/

/*****************************************************************************/
/* dOm fonction callback						     */
/*****************************************************************************/
void affiche_cache_commentaire_echeancier( void )
{
    gint largeur;
    
    etat.affichage_commentaire_echeancier = 1 - etat.affichage_commentaire_echeancier;

    /*     sans bloquer ici, un signal allocation va être envoyé, donc on évite de rerécupérer */
    /* 	toutes les tailles de colonnes */

    bloque_taille_colonne = 1;

    largeur = tree_view_liste_echeances -> allocation.width;

    if ( etat.affichage_commentaire_echeancier )
    {
	gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_FREQUENCY] ),
					   FALSE );
	gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_MODE] ),
					   FALSE );
	if ( GTK_WIDGET_REALIZED ( tree_view_liste_echeances ))
	    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_NOTES] ),
					       TRUE );
    }
    else
    {
	gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_FREQUENCY] ),
					   TRUE );
	gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_MODE] ),
					   TRUE );
	gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_NOTES] ),
					   FALSE );
	if ( GTK_WIDGET_REALIZED ( tree_view_liste_echeances ))
	{
	    gtk_tree_view_column_set_fixed_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_FREQUENCY] ),
						   (scheduler_col_width[COL_NB_FREQUENCY] * largeur ) / 100 );
	    gtk_tree_view_column_set_fixed_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[COL_NB_MODE] ),
						   (scheduler_col_width[COL_NB_MODE] * largeur ) / 100 );
	}
    }
}
/*****************************************************************************/

/*****************************************************************************/
void click_sur_saisir_echeance ( void )
{
    formulaire_echeancier_a_zero();
    degrise_formulaire_echeancier();

    gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			 _("Input") );
    edition_echeance ();

    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] );

}
/*****************************************************************************/




/*****************************************************************************/
void remplissage_liste_echeance ( void )
{
    GtkListStore *store;
    GSList *pointeur_liste;
    GDate *date_fin;
    gint i;
    GtkTreeIter iter;


    if ( DEBUG )
	printf ( "remplissage_liste_echeance\n" );

    /*     récupération du store */

    store = GTK_LIST_STORE (gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances )));
    
    /*     récupération de la date de la fin de l'affichage */

    date_fin = date_fin_affichage_liste_echeance ();

     /* on efface la liste */

    gtk_list_store_clear ( GTK_LIST_STORE ( store ));

    /*     remplissage de la liste */

    pointeur_liste = liste_struct_echeances;

    while ( pointeur_liste )
    {
	struct operation_echeance *echeance;

	echeance = pointeur_liste -> data;

	if ( !echeance -> no_operation_ventilee_associee )
	{
	    gchar *ligne[NB_COLS_SCHEDULER];

	    /* 	    mise en forme de la date */

	    ligne[COL_NB_DATE] = renvoie_date_formatee ( echeance -> date );

	    /* 	    mise en forme de la périodicité */

	    switch ( echeance ->periodicite )
	    {
		case 0 :
		    ligne[COL_NB_FREQUENCY] = _("Once");
		    break;
		case 1 :
		    ligne[COL_NB_FREQUENCY] = _("Weekly");
		    break;
		case 2 : 
		    ligne[COL_NB_FREQUENCY] = _("Monthly");
		    break;
		case 3 : 
		    ligne[COL_NB_FREQUENCY] = _("Yearly");
		    break;
		case 4 :
		    if ( echeance -> intervalle_periodicite_personnalisee )
			if ( echeance -> intervalle_periodicite_personnalisee == 1 )
			    ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d months"),
									echeance -> periodicite_personnalisee );
			else
			    ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d years"),
									echeance -> periodicite_personnalisee );
		    else
			ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d days"),
								    echeance -> periodicite_personnalisee );
		    break;
	    }

	    /* mise en forme du compte */

	    ligne[COL_NB_ACCOUNT] = gsb_account_get_name ( echeance -> compte );

	    /* mise en forme du tiers */

	    ligne[COL_NB_PARTY] = tiers_name_by_no (echeance -> tiers, TRUE );

	    /* mise en forme de auto/man */

	    if ( echeance -> auto_man )
		ligne[COL_NB_MODE]=_("Automatic");
	    else
		ligne[COL_NB_MODE] = _("Manual");

	    /* mise en forme des notes */

	    ligne[COL_NB_NOTES] = echeance -> notes;

	    /* mise en forme du montant */

	    ligne[COL_NB_AMOUNT] = g_strdup_printf ( "%4.2f",
						     echeance -> montant );

	    /* on va ajouter l'échéance une ou plusieurs fois en changeant juste sa date */
	    /* en fontion de l'affichage de l'échéance */

	    if ( !( g_date_compare ( date_fin, echeance -> date ) <= 0
		    &&
		    affichage_echeances != 3 ))
	    {
		gint boucle;
		GDate *pGDateCurrent;
		struct operation_echeance *echeance_tmp;

		/* 		echeance_tmp contient l'adr de l'échéance mère, puis NULL pour les */
		/* 		    échéances calculées */

		echeance_tmp = echeance;

		boucle = 1;
		pGDateCurrent = g_date_new_dmy ( echeance -> date -> day,
						 echeance -> date -> month,
						 echeance -> date -> year );

		while ( boucle )
		{
		    gtk_list_store_append ( GTK_LIST_STORE (store),
					    &iter );

		    for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
			gtk_list_store_set ( GTK_LIST_STORE ( store ),
					     &iter,
					     i, ligne[i],
					     -1 );

		    /* on met le numéro de l'échéance celui ci est à NULL si c'est une échéance calculée */

		    gtk_list_store_set ( GTK_LIST_STORE ( store ),
					 &iter,
					 SCHEDULER_COL_NB_TRANSACTION_ADDRESS, echeance_tmp,
					 -1 );

		    /* c'est maintenant qu'on voit si on sort ou pas ... */

		    pGDateCurrent = date_suivante_echeance ( echeance,
							     pGDateCurrent );

		    if ( pGDateCurrent
			 &&
			 g_date_compare ( date_fin, pGDateCurrent ) > 0
			 &&
			 affichage_echeances != 3
			 &&
			 echeance -> periodicite )
		    {
			ligne[COL_NB_DATE] = g_strdup_printf ( "%02d/%02d/%d",
							       g_date_day ( pGDateCurrent ),
							       g_date_month ( pGDateCurrent ),
							       g_date_year ( pGDateCurrent ));
			echeance_tmp = NULL;
		    }
		    else
			boucle = 0;
		}
	    }
	}
	pointeur_liste = pointeur_liste -> next;
    }


    /* met la ligne blanche */

    gtk_list_store_append ( GTK_LIST_STORE (store),
			    &iter );

    /* on met le numéro de l'échéance à -1 */

    gtk_list_store_set ( GTK_LIST_STORE ( store ),
			 &iter,
			 SCHEDULER_COL_NB_TRANSACTION_ADDRESS, GINT_TO_POINTER (-1),
			 -1 );
}
/*****************************************************************************/


/*****************************************************************************/
void update_couleurs_background_echeancier ( void )
{
    GtkTreeIter iter;
    GtkListStore *store;
    gint couleur_en_cours;
    gboolean result_iter;

    if ( DEBUG )
	printf ( "update_couleurs_background_echeancier\n" );


    store = GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances )));
    couleur_en_cours = 0;

    result_iter = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( store ),
						  &iter );

    while ( result_iter )
    {
	struct operation_echeance *echeance;

	gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
			     &iter,
			     11, &echeance,
			     -1 );

	if ( echeance )
	    gtk_list_store_set ( store,
				 &iter,
				 SCHEDULER_COL_NB_BACKGROUND, &couleur_fond[couleur_en_cours],
				 -1 );
	else
	    gtk_list_store_set ( store,
				 &iter,
				 SCHEDULER_COL_NB_BACKGROUND, &couleur_grise,
				 -1 );

	result_iter = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( store ),
						 &iter );
	couleur_en_cours = 1 - couleur_en_cours;
    }
    
    /*     il n'y a plus de sélection */

    selection_echeance_finie = 0;
}
/*****************************************************************************/





/*****************************************************************************/
void selectionne_echeance ( struct operation_echeance *echeance )
{
    GtkTreeIter *iter;
    GdkColor *couleur;
    GtkTreeModel *model;

    if ( DEBUG )
    {
	if ( echeance == GINT_TO_POINTER (-1))
	    printf ( "selectionne_echeance blanche\n");
	else
	    printf ( "selectionne_echeance %d\n",echeance ->no_operation );
    }

    /*     si echeance_selectionnnee est nulle, c'est qu'il n'y a encore aucune sélection, donc */
    /* 	on ne retire pas l'ancienne sélection */

    if ( (echeance == echeance_selectionnnee
	 &&
	 echeance_selectionnnee
	 &&
	 selection_echeance_finie)
	 ||
	 !echeance )
	return;


    model = GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances )));

    /*   vire l'ancienne sélection : consiste à remettre la couleur d'origine du background */

    if ( echeance_selectionnnee )
    {
	iter = cherche_iter_echeance ( echeance_selectionnnee );

	/* 	iter est maintenant positionné sur la 1ère ligne de l'opé à désélectionner */

	gtk_tree_model_get ( model,
			     iter,
			     SCHEDULER_COL_NB_SAVE_BACKGROUND, &couleur,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE (model),
			     iter,
			     SCHEDULER_COL_NB_BACKGROUND,couleur,
			     9, NULL,
			     -1 );
    }

    echeance_selectionnnee = echeance;

    iter = cherche_iter_echeance ( echeance_selectionnnee );

    /* 	iter est maintenant positionnÃ© sur la 1ère ligne de l'opÃ© à sélectionner */

    gtk_tree_model_get ( model,
			 iter,
			 SCHEDULER_COL_NB_BACKGROUND, &couleur,
			 -1 );
    gtk_list_store_set ( GTK_LIST_STORE (model),
			 iter,
			 SCHEDULER_COL_NB_BACKGROUND, &couleur_selection,
			 SCHEDULER_COL_NB_SAVE_BACKGROUND, couleur,
			 -1 );

    /*     on déplace le scrolling de la liste si nécessaire pour afficher la sélection */

    ajuste_scrolling_liste_echeances_a_selection ();

    selection_echeance_finie = 1;

    if ( echeance_selectionnnee != GINT_TO_POINTER ( -1 ) )
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_saisir_echeancier ),
				   TRUE );
    else
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_saisir_echeancier ),
				   FALSE );
}
/*****************************************************************************/


/******************************************************************************/
void ajuste_scrolling_liste_echeances_a_selection ( void )
{
    GtkAdjustment *v_adjustment;
    gint y_ligne;

    /*     si on n'a pas encore récupéré la hauteur des lignes, on va le faire ici */

    if ( !hauteur_ligne_liste_opes )
	hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_liste_echeances );

    v_adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_liste_echeances ));

    y_ligne = cherche_ligne_echeance ( echeance_selectionnnee ) * hauteur_ligne_liste_opes;

    /*     si l'opé est trop haute, on la rentre et la met en haut */

    if ( y_ligne < v_adjustment -> value )
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				   y_ligne );
    else
	if ( (y_ligne + hauteur_ligne_liste_opes ) > ( v_adjustment -> value + v_adjustment -> page_size ))
	    gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				       y_ligne + hauteur_ligne_liste_opes - v_adjustment -> page_size );

}
/******************************************************************************/




/******************************************************************************/
/* fonction cherche_iter_echeance */
/* retrouve l'iter correspondant à l'opération donnée en argument dans la tree_view des opérations */
/* renvoie null si pas trouvé
* \param echeance
* \return iter correspondant
*****************************************************************************/
GtkTreeIter *cherche_iter_echeance ( struct operation_echeance *echeance )
{
    struct operation_echeance *echeance_tmp;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( !echeance )
	return NULL;

    model = GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances )));

    /*     on va faire le tour de la liste, et dès qu'une opé = echeance */
    /* 	on retourne son iter */

    gtk_tree_model_get_iter_first ( model,
				    &iter );
    echeance_tmp = NULL;

    do
    {
	gtk_tree_model_get ( model,
			     &iter,
			     11, &echeance_tmp,
			     -1 );
    }
    while ( echeance_tmp != echeance
	    &&
	    gtk_tree_model_iter_next ( model,
				       &iter ));

    if ( echeance_tmp == echeance )
	return ( gtk_tree_iter_copy ( &iter ));
    else
	return NULL;

}
/******************************************************************************/



/******************************************************************************/
/* cette fonction renvoie le no de ligne de l'opération en argument */
/******************************************************************************/
gint cherche_ligne_echeance ( struct operation_echeance *echeance )
{
    GtkTreeIter *iter;

    iter = cherche_iter_echeance ( echeance );

    return ( my_atoi ( gtk_tree_model_get_string_from_iter (  GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances ))),
							      iter )));
}
/******************************************************************************/



/******************************************************************************/
/* renvoie l'adr de l'opération correspondant  à la ligne envoyées */
/* en argument */
/******************************************************************************/
struct operation_echeance *cherche_echeance_from_ligne ( gint ligne )
{
    GtkTreeIter iter;
    struct operation_echeance *echeance;

    if ( !gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances ))),
					       &iter,
					       itoa (ligne)))
	return NULL;

    gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances ))),
			 &iter,
			 11, &echeance,
			 -1 );

    return ( echeance );
}
/******************************************************************************/



/*****************************************************************************/
GDate *date_fin_affichage_liste_echeance ( void )
{
    GDate *date_fin;
    
   /* on récupère la date du jour et la met dans date_fin pour les vérifications ultérieures */

    date_fin = gdate_today ();

    /* on calcule la date de fin de l'affichage */

    switch ( affichage_echeances )
    {
	case 0:
	    g_date_add_months ( date_fin,
				1 );
	    date_fin -> day = 1;
	    break;

	case 1:
	    g_date_add_months ( date_fin,
				2 );
	    date_fin -> day = 1;
	    break;

	case 2:
	    g_date_add_years ( date_fin,
			       1 );
	    date_fin -> day = 1;
	    date_fin -> month = 1;
	    break;

	case 4:
	    if ( !affichage_echeances_perso_j_m_a )
		g_date_add_days ( date_fin,
				  affichage_echeances_perso_nb_libre );
	    else
	    {
		if ( affichage_echeances_perso_j_m_a == 1 )
		    g_date_add_months ( date_fin,
					affichage_echeances_perso_nb_libre );
		else
		    g_date_add_years ( date_fin,
				       affichage_echeances_perso_nb_libre );
	    }
    }
    return date_fin;
}
/*****************************************************************************/



/*****************************************************************************/
gboolean click_ligne_echeance ( GtkWidget *tree_view,
				GdkEventButton *evenement )
{
    gint x, y;
    GtkTreePath *path;
    GtkTreeIter iter;
    struct operation_echeance *echeance;

    /* si le click se situe dans les menus, c'est qu'on redimensionne, on fait rien */

    if ( evenement -> window != gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )) )
	return(FALSE);


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
					  NULL,
					  NULL,
					  NULL ))
    {
	/* 	si on met un jour un click droit, c'est ici pour en dehors de la liste */

	return (TRUE);
    }

    gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ))),
					  &iter,
					  gtk_tree_path_to_string ( path ));

    /*     on récupère l'échéance clickée, si elle est nulle c'est que c'est un doublon */
    /* 	et on fait rien */

    gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ))),
					  &iter,
					  11, &echeance,
					  -1 );

    if ( !echeance )
	return TRUE;

    selectionne_echeance (echeance);


    if ( evenement -> type == GDK_2BUTTON_PRESS )
	edition_echeance ();
    else
	gtk_widget_grab_focus ( tree_view );

    return TRUE;
}
/*****************************************************************************/


/*****************************************************************************/
/* Fonction edition_echeance						     */
/* appelée par double click ou entrée sur une échéance. Elle recopie	     */
/* l'échéance sélectionnée dans le formulaire d'échéance pour la modifier    */
/*****************************************************************************/
void edition_echeance ( void )
{
    gchar *char_tmp;
    
    /*   si le formulaire est caché, on le montre */

    gtk_widget_show ( frame_formulaire_echeancier );

    /* si c'est une nouvelle échéance, on met la date courante,
       la sélectionne et lui donne le focus */

    if ( echeance_selectionnnee == GINT_TO_POINTER ( -1 ))
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			     gsb_today() );
	gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
				  0,
				  -1 );
	gtk_widget_grab_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );

	return;
    }


    /* c'est une modif d'échéance ou une saisie */

    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 g_strdup_printf ( "%02d/%02d/%d",
					   echeance_selectionnnee -> jour,
					   echeance_selectionnnee -> mois,
					   echeance_selectionnnee -> annee ));

    /* mise en place du tiers */

    char_tmp = tiers_name_by_no ( echeance_selectionnnee -> tiers, TRUE );

    if ( char_tmp )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
				char_tmp );
    }


    /* mise en place du compte */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
				  recherche_compte_dans_option_menu (  widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
								       echeance_selectionnnee -> compte ));

    /* mise en place des montants et des types associés */

    if ( echeance_selectionnnee -> montant < 0 )
    {
	GtkWidget *menu;
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			     g_strdup_printf ( "%4.2f",
					       -echeance_selectionnnee -> montant ));
	/* met le menu des types débits */


	if ( (menu = creation_menu_types ( 1,
					   recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }
    else
    {
	GtkWidget *menu;
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			     g_strdup_printf ( "%4.2f",
					       echeance_selectionnnee -> montant ));
	/* met le menu des types crédits */


	if ( (menu = creation_menu_types ( 2,
					   recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }


    /* mise en place des devises */
    /* FIXME : me demande si yaurait pas un g_slist_index à mettre ici... */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
				  devise_par_no ( echeance_selectionnnee -> devise ) -> no_devise - 1 );

    /* si le compte de virement est -1, c'est qu'il n'y a pas de categ, ni de virements, ni de ventil */

    if ( echeance_selectionnnee -> compte_virement != -1 )
    {
	gchar *texte;

	if ( echeance_selectionnnee -> categorie )
	{
	    /* 	    il y a donc des catégs/sous-catég */

	    texte =  nom_categ_par_no ( echeance_selectionnnee -> categorie,
					echeance_selectionnnee -> sous_categorie );
	}
	else
	{
	    /* 	    c'est soit un virement, soit une opé ventilée */

	    if ( echeance_selectionnnee -> compte_virement )
	    {
		texte = g_strconcat ( COLON(_("Transfer")),
				      gsb_account_get_name (echeance_selectionnnee -> compte_virement),
				      NULL );
	    }
	    else
	    {
		if ( echeance_selectionnnee -> operation_ventilee )
		{
		    texte =  g_strdup (_("Breakdown of transaction"));

/* 		    on n'affiche le bouton de ventilation que si on est dans l'onglet des échéances, pas */
/* 			en formulaire flottant */

		    if ( !etat.formulaire_echeance_dans_fenetre ) 
			gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] );

		    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
					       FALSE );
		    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY],
					       FALSE );

		    /* met la liste des opés de ventilation dans liste_adr_ventilation */

		    gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
					  "liste_adr_ventilation",
					  creation_liste_ope_de_ventil_echeances ( echeance_selectionnnee ));
		}
		else
		{ 
		    /* 		    normalement, on devrait pas arriver là... bon, on laisse comme ça */
		    texte = NULL;
		}
	    }
	}

	if ( texte )
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] );
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
				    texte );
	}
    }


    /* met l'option menu du type d'opé */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				      cherche_no_menu_type_echeancier ( echeance_selectionnnee -> type_ope ));

    if ( echeance_selectionnnee -> contenu_type )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			     echeance_selectionnnee -> contenu_type );
    }


    /* met en place l'exercice */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( echeance_selectionnnee -> no_exercice,
							      widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ));

    /* met en place l'imputation budgétaire */

    char_tmp = nom_imputation_par_no ( echeance_selectionnnee -> imputation,
				       echeance_selectionnnee -> sous_imputation );

    if ( char_tmp )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY]);
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
				char_tmp );
    }

    /* mise en place de l'automatisme */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
				  echeance_selectionnnee -> auto_man );

    /*   remplit les notes */

    if ( echeance_selectionnnee -> notes )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			     echeance_selectionnnee -> notes );
    }

    /* mise en place de la périodicité */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
				  echeance_selectionnnee -> periodicite );

    /* mise en place de la date limite */

    if ( echeance_selectionnnee -> periodicite )
    {
	if ( echeance_selectionnnee ->  date_limite )
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
				 g_strdup_printf ( "%02d/%02d/%d",
						   echeance_selectionnnee ->  jour_limite,
						   echeance_selectionnnee ->  mois_limite,
						   echeance_selectionnnee ->  annee_limite ));
	}

	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
    }


    /* mise en place de la périodicité perso si nécessaire */

    if ( echeance_selectionnnee -> periodicite == 4 )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
	gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] ),
			     itoa ( echeance_selectionnnee ->  periodicite_personnalisee) );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
				      echeance_selectionnnee -> intervalle_periodicite_personnalisee );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );

    }


    /* sélectionne la date et lui donne le focus */

    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			      0,
			      -1 );
    gtk_widget_grab_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );

    /* met l'adr de l'échéance courante dans l'entrée de la date */

    gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			  "adr_echeance",
			  echeance_selectionnnee );
}
/*****************************************************************************/

/*****************************************************************************/
void supprime_echeance ( struct operation_echeance *echeance )
{
    gint resultat;

    if ( DEBUG )
	printf ( "supprime_echeance\n" );

    if ( echeance == NULL )
	echeance = echeance_selectionnnee;

    if ( !echeance
	 ||
	 echeance == GINT_TO_POINTER ( -1 ))
	return;

    /* si la périodicité n'est pas de 1 fois demande juste celle ci ou toutes,
       sinon, choisit automatiquement toutes */

    if ( echeance -> periodicite )
    {
	GtkWidget * dialog;
	gchar * occurences = "";

	occurences = g_strdup_printf ( _("%02d/%02d/%d : %s [%4.2f %s]"),
				       echeance -> jour,
				       echeance -> mois,
				       echeance -> annee,
				       tiers_name_by_no ( echeance -> tiers, FALSE ),
				       echeance -> montant,
				       devise_name ( devise_par_no (echeance->devise) ) );

	dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
					   make_hint ( _("Delete this scheduled transaction?"),
						       g_strconcat ( _("Do you want to delete just this occurrence or the whole scheduled transaction?\n\n"),
								     occurences, NULL )));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 GTK_STOCK_CANCEL, 2,
				 _("All the occurences"), 1,
				 _("Only this one"), 0,
				 NULL );

	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));
	gtk_widget_destroy ( dialog );
    }
    else
	resultat = 1;

    switch ( resultat )
    {
	case 0:
	    incrementation_echeance ( echeance );
	    remplissage_liste_echeance ();
	    break;

	case 1:
	    /* 	    si c'est une ventil, on supprime les opés filles */

	    if ( echeance -> operation_ventilee )
	    {
		GSList *liste_tmp;

		liste_tmp = liste_struct_echeances;

		while ( liste_tmp )
		{
		    struct operation_echeance *echeance_tmp;

		    echeance_tmp = liste_tmp -> data;
		    
		    if ( echeance_tmp -> no_operation_ventilee_associee == echeance -> no_operation )
		    {
			liste_tmp = liste_tmp -> next;
			liste_struct_echeances = g_slist_remove ( liste_struct_echeances, 
								  echeance_tmp );
			free ( echeance_tmp );
			nb_echeances--;
		    }
		    else
			liste_tmp = liste_tmp -> next;
		}
	    }

	    /* 	    on vire l'échéance elle même */

	    liste_struct_echeances = g_slist_remove ( liste_struct_echeances, 
						      echeance );
	    free ( echeance );
	    echeance = gtk_clist_get_row_data ( GTK_CLIST ( tree_view_liste_echeances ),
						gtk_clist_find_row_from_data ( GTK_CLIST ( tree_view_liste_echeances ),
									       echeance ) + 1);

	    nb_echeances--;

	    remplissage_liste_echeance ();
	    break;

    }

    mise_a_jour_calendrier();
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    modification_fichier (TRUE);
}
/*****************************************************************************/




/*****************************************************************************/
/* Fonction changement_taille_liste_echeances				     */
/* avec les tree view, cette fonction est appelée quand : */
/* -click sur un titre de colonne */
/* -modification de la taille d'une colonne */
/* -modification de la taille du tree_view */
/*****************************************************************************/
gboolean changement_taille_liste_echeances ( GtkWidget *tree_view,
					     GtkAllocation *allocation )
{
    gint largeur;
    gint i;
    gint col1, col2, col3, col4, col5, col6, col7;

    /*     on va séparer en 2 parties : */
    /* 	soit la largeur = ancienne_largeur_echeances, dans ce cas on dit que c'est un redimensionnement de colonne */
    /* 	soit la largeur != ancienne_largeur_echeances et c'est un redimensionnement de la liste */

    if ( tree_view -> allocation.width == ancienne_largeur_echeances )
    {
	/* 	c'est un redimensionnement de colonne */
	/* on ne fait juste que récupérer les largeurs de colonnes */

	gint rapport_frequence;

	/* 	si on venait juste de redimensionner la liste, on fait rien */

	if ( bloque_taille_colonne )
	{
	    bloque_taille_colonne = 0;
	    return FALSE; 
	}

	printf ( "redimension colonne\n" );

	/* 	avant de modifier, on va garder le rapport entre la fréquence et le mode */
	/* 	    pour les réappliquer derrière */

	if ( scheduler_col_width[COL_NB_NOTES] )
	    rapport_frequence = (scheduler_col_width[COL_NB_FREQUENCY] * 100 ) / (scheduler_col_width[COL_NB_NOTES]);
	else
	    rapport_frequence = 50;
	
	for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
	{
	    /* calcul de la valeur relative du redimensionnement de la colonne concernée */

	    largeur = gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ));

	    scheduler_col_width[i] = (largeur * 100) / allocation -> width;
	}

	/* 	en fonction de la conf, on calcule la largeur des notes ou remet le rapport entre */
	/* 	    la fréquence et le mode */

	if ( etat.affichage_commentaire_echeancier )
	{
	    /* 	    c'est la note qui est affichée, donc on applique le rapport aux */
	    /* 		2 colonnes non affichées */
	
	    scheduler_col_width[COL_NB_FREQUENCY] = (rapport_frequence * scheduler_col_width[COL_NB_NOTES]) / 100;
	    scheduler_col_width[COL_NB_MODE] = scheduler_col_width[COL_NB_NOTES] - scheduler_col_width[COL_NB_FREQUENCY];
	}
	else
	{
	    /* 	    la colonne cachée de notes vaut la somme des 2 affichées */

	    scheduler_col_width[COL_NB_NOTES] =  scheduler_col_width[COL_NB_FREQUENCY] + scheduler_col_width[COL_NB_MODE];
	}
	return FALSE;
    }

    /*     si on est ici, c'est qu'on redimensionne la liste */

    printf ( "redimension liste\n" );

    /*     on bloque le redimensionnement de colonnes qui va suivre le redimensionnement de la liste */

    bloque_taille_colonne = 1;

    largeur = tree_view -> allocation.width ;

    /* on établit alors les largeurs des colonnes */
	/* 	on ne met pas la valeur de la dernière colonne */

    for ( i = 0 ; i < NB_COLS_SCHEDULER - 1 ; i++ )
	gtk_tree_view_column_set_fixed_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ),
					       (scheduler_col_width[i] * largeur ) / 100 );

    /* on sauve la valeur courante de la largeur de la liste pour
       une utilisation ultérieure */

    ancienne_largeur_echeances = largeur ;

    /* met les entrées du formulaire à la même taille */

    col1 = ( 7 * largeur) / 100;
    col2 = ( 13 * largeur) / 100;
    col3 = ( 30 * largeur) / 100;
    col4 = ( 12 * largeur) / 100;
    col5 = ( 12 * largeur) / 100;
    col6 = ( 10 * largeur) / 100;
    col7 = ( 10 * largeur) / 100;

    /* 1ère ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( label_saisie_modif ),
			   col1,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			   col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			   col4,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			   col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
			   col6 + col7,
			   FALSE );

    /* 2ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
			   col1 + col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
			   col4 + col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			   col6 + col7,
			   FALSE );

    /* 3ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
			   col1 + col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] ),
			   col4 + col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER] ),
			   col6 + col7,
			   FALSE );

    /* 4ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
			   col1 + col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
			   col4,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
			   col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] ),
			   col6,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
			   col7,
			   FALSE );
    return FALSE;
}
/*****************************************************************************/




/*****************************************************************************/
void verification_echeances_a_terme ( void )
{
    GDate *pGDateCurrent;
    GSList *pointeur_liste;
    GSList *ancien_pointeur;
    struct struct_devise *devise;

    if ( DEBUG )
	printf ( "verification_echeances_a_terme\n" );

    /* les échÃ©ances à saisir sont revérifiées à chaque fois. Par contre, les
       échéances saisies sont ajoutées à la liste de celles déjà saisies */

    echeances_a_saisir = NULL;

    /* récupère la date du jour et la met en format gdate */

    pGDateCurrent = g_date_new ();
    g_date_set_time ( pGDateCurrent,
		      time ( NULL ));

    /* on y ajoute le décalage de l'échéance */

    g_date_add_days ( pGDateCurrent,
		      decalage_echeance );


    /* on fait le tour des échéances : si elle est auto, elle est enregistrée,
       sinon elle est juste répertoriée */

    pointeur_liste = liste_struct_echeances;
    ancien_pointeur = NULL;


    while ( pointeur_liste )
    {
	if ( ECHEANCE_COURANTE -> auto_man &&
	     ! ECHEANCE_COURANTE -> no_operation_ventilee_associee )
	{
	    /* tant que cette échéance auto n'est pas arrivée à aujourd'hui, on recommence */

	    while ( pointeur_liste != ancien_pointeur
		    &&
		    g_date_compare ( ECHEANCE_COURANTE -> date, pGDateCurrent ) <= 0 )
	    {
		struct structure_operation *operation;
		gint virement;
		GSList *pointeur_liste_ventil;


		/* crée l'opÃ©ration */

		operation = calloc ( 1,
				     sizeof ( struct structure_operation ) ); 


		/* remplit l'opération */

		operation -> jour = ECHEANCE_COURANTE -> jour;
		operation -> mois = ECHEANCE_COURANTE -> mois;
		operation -> annee = ECHEANCE_COURANTE -> annee;

		gsb_transaction_data_set_date ( gsb_transaction_data_get_transaction_number ( operation ),
						g_date_new_dmy ( operation ->jour,
								 operation ->mois,
								 operation ->annee));


		gsb_transaction_data_set_account_number ( gsb_transaction_data_get_transaction_number ( operation ),
							  ECHEANCE_COURANTE -> compte );
		operation -> tiers = ECHEANCE_COURANTE -> tiers;
		operation -> montant = ECHEANCE_COURANTE -> montant;
		operation -> devise = ECHEANCE_COURANTE -> devise;


		/* demande si nécessaire la valeur de la devise et du change */
		/* récupération des devises de l'opé et du compte */

		devise = devise_par_no ( operation -> devise );

		if ( !devise_compte
		     ||
		     devise_compte -> no_devise != gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (operation))) )
		    devise_compte = devise_par_no ( gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (operation))) );

		if ( !( gsb_transaction_data_get_transaction_number (operation)
			||
			devise -> no_devise == gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (operation)))
			||
			( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
			||
			( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
		{
		    /* c'est une devise étrangère, on demande le taux de change et les frais de change */

		    demande_taux_de_change ( devise_compte, devise, 1,
					     (gdouble ) 0, (gdouble ) 0, FALSE );

		    operation -> taux_change = taux_de_change[0];
		    operation -> frais_change = taux_de_change[1];

		    if ( operation -> taux_change < 0 )
		    {
			operation -> taux_change = -operation -> taux_change;
			operation -> une_devise_compte_egale_x_devise_ope = 1;
		    }
		}
		else
		{
		    operation -> taux_change = 0;
		    operation -> frais_change = 0;
		}

		operation -> categorie = ECHEANCE_COURANTE -> categorie;
		operation -> sous_categorie = ECHEANCE_COURANTE -> sous_categorie;

		if ( !ECHEANCE_COURANTE -> categorie
		     &&
		     ECHEANCE_COURANTE -> compte_virement != -1
		     &&
		     !ECHEANCE_COURANTE -> operation_ventilee )
		    virement = 1;
		else
		    virement = 0;

		if ( ECHEANCE_COURANTE -> notes )
		    operation -> notes = g_strdup ( ECHEANCE_COURANTE -> notes );


		operation -> type_ope = ECHEANCE_COURANTE -> type_ope;
		if ( ECHEANCE_COURANTE -> contenu_type )
		    operation -> contenu_type = ECHEANCE_COURANTE -> contenu_type;

		operation -> auto_man = ECHEANCE_COURANTE -> auto_man;
		operation -> imputation = ECHEANCE_COURANTE -> imputation;
		operation -> sous_imputation = ECHEANCE_COURANTE -> sous_imputation;

		/* si l'exo est automatique (-2), c'est ici qu'on va le chercher */

		if ( ECHEANCE_COURANTE -> no_exercice == -2 )
		    operation -> no_exercice = recherche_exo_correspondant ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation)));
		else
		    operation -> no_exercice = ECHEANCE_COURANTE -> no_exercice;


		/*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

		gsb_transactions_append_transaction ( operation,
						      gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (operation)));

		/*   si c'était un virement, on crée une copie de l'opé, on l'ajoute à la liste puis on remplit les relations */

		/* FIXME : le jour où on choisira le type de la contre opération dans les échéances, il faudra changer ici */
		/* cette opé de ventil est un virement */

		if ( virement )
		    ajoute_contre_operation_echeance_dans_liste ( operation,
								  ECHEANCE_COURANTE -> compte_virement,
								  operation -> type_ope );

		/* 	si c'était une échéance ventilée, c'est ici qu'on fait joujou */

		pointeur_liste_ventil = creation_liste_ope_de_ventil_echeances ( ECHEANCE_COURANTE );

		while ( pointeur_liste_ventil
			&&
			pointeur_liste_ventil != GINT_TO_POINTER ( -1 ))
		{
		    struct struct_ope_ventil *ope_ventil;
		    struct structure_operation *operation_fille;

		    /* 	    la mère est donc une ventil */

		    operation -> operation_ventilee = 1;

		    ope_ventil = pointeur_liste_ventil -> data;
		    operation_fille = calloc ( 1,
					       sizeof ( struct structure_operation ));

		    /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

		    gsb_transactions_append_transaction ( operation_fille,
							  gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (operation)));


		    operation_fille -> montant = ope_ventil -> montant;
		    operation_fille -> categorie = ope_ventil -> categorie;
		    operation_fille -> sous_categorie = ope_ventil -> sous_categorie;

		    if ( ope_ventil -> notes )
			operation_fille -> notes = g_strdup ( ope_ventil -> notes );

		    operation_fille -> imputation = ope_ventil -> imputation;
		    operation_fille -> sous_imputation = ope_ventil -> sous_imputation;

		    if ( ope_ventil -> no_piece_comptable )
			operation_fille -> no_piece_comptable = g_strdup ( ope_ventil -> no_piece_comptable);

		    operation_fille -> no_exercice = ope_ventil -> no_exercice;

		    /* 	    le reste est identique à la mère */

		    operation_fille -> jour = operation -> jour;
		    operation_fille -> mois = operation -> mois;
		    operation_fille -> annee = operation -> annee;
		    gsb_transaction_data_set_date ( gsb_transaction_data_get_transaction_number ( operation_fille ),
						    g_date_new_dmy ( operation_fille -> jour,
								     operation_fille -> mois,
								     operation_fille -> annee ));

		    if ( operation -> jour_bancaire )
		    {
			operation_fille -> jour_bancaire = operation -> jour_bancaire;
			operation_fille -> mois_bancaire = operation -> mois_bancaire;
			operation_fille -> annee_bancaire = operation -> annee_bancaire;
			gsb_transaction_data_set_value_date ( gsb_transaction_data_get_transaction_number ( operation_fille ),
							      g_date_new_dmy ( operation_fille -> jour_bancaire,
									       operation_fille -> mois_bancaire,
									       operation_fille -> annee_bancaire ));
		    }

		    operation_fille -> devise = operation -> devise;
		    operation_fille -> une_devise_compte_egale_x_devise_ope = operation -> une_devise_compte_egale_x_devise_ope;
		    operation_fille -> taux_change = operation -> taux_change;
		    operation_fille -> frais_change = operation -> frais_change;
		    operation_fille -> tiers = operation -> tiers;
		    operation_fille -> pointe = operation -> pointe;
		    operation_fille -> auto_man = operation -> auto_man;
		    operation_fille -> no_operation_ventilee_associee = gsb_transaction_data_get_transaction_number (operation);



		    /* 	    on vérifie maintenant si c'est un virement */

		    if ( ope_ventil -> relation_no_operation == -1 )
		    {
			/* cette opé de ventil est un virement */

			ajoute_contre_operation_echeance_dans_liste ( operation_fille,
							 ope_ventil -> relation_no_compte,
							 ope_ventil -> no_type_associe );
		    }

		    pointeur_liste_ventil = pointeur_liste_ventil -> next;
		}


		echeances_saisies = g_slist_append ( echeances_saisies,
						     operation );


		/* passe l'échéance à la prochaine date */

		incrementation_echeance ( ECHEANCE_COURANTE );

		if ( !g_slist_find ( liste_struct_echeances,
				     ECHEANCE_COURANTE ) )
		{
		    if ( !ancien_pointeur )
			ancien_pointeur = liste_struct_echeances;
		    pointeur_liste = ancien_pointeur;
		}
	    }
	}
	else if ( ! ECHEANCE_COURANTE -> no_operation_ventilee_associee )
	    /* ce n'est pas une échéance automatique, on la répertorie dans la liste des échéances à saisir */

	    if ( g_date_compare ( ECHEANCE_COURANTE -> date,
				  pGDateCurrent ) <= 0
		 &&
		 !ECHEANCE_COURANTE -> no_operation_ventilee_associee )
		echeances_a_saisir = g_slist_append ( echeances_a_saisir ,
						      ECHEANCE_COURANTE );


	if ( ancien_pointeur == liste_struct_echeances && pointeur_liste == ancien_pointeur )
	    ancien_pointeur = NULL;
	else
	{
	    ancien_pointeur = pointeur_liste;
	    pointeur_liste = pointeur_liste -> next;
	}
    }


    if ( echeances_saisies )
    {
	mise_a_jour_liste_echeances_auto_accueil = 1;
	modification_fichier ( TRUE );
    }

    if ( echeances_a_saisir )
	mise_a_jour_liste_echeances_manuelles_accueil = 1;

}
/*****************************************************************************/



/*****************************************************************************/
/* Fonction appelée lorsqu'on change le bouton pour l'affichage des	     */
/* échéances ( choix mois, 2 mois ... )					     */
/*****************************************************************************/
gboolean modification_affichage_echeances ( gint *origine, GtkWidget * widget )
{
    switch ( GPOINTER_TO_INT ( origine ))
    {
	/* vient de l'entrée qui perd le focus */

	case 5:

	    affichage_echeances_perso_nb_libre = my_atoi ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_personnalisation_affichage_echeances ))));

	    break;

	    /* vient d'un chgt du bouton perso jour/mois/an */

	case 6:

	    affichage_echeances_perso_j_m_a = GPOINTER_TO_INT 
		( gtk_object_get_data ( GTK_OBJECT ( widget ), "intervalle_perso" ));
	    break;

	    /*       vient du reste, si c'est perso ( 4 ) , on affiche ce qu'il faut */

	case 4:

	    affichage_echeances = GPOINTER_TO_INT ( origine );
	    if ( affichage_echeances_perso_nb_libre )
		gtk_entry_set_text ( GTK_ENTRY ( entree_personnalisation_affichage_echeances ),
				     itoa ( affichage_echeances_perso_nb_libre ));
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_personnalisation_affichage_echeances ),
					  affichage_echeances_perso_j_m_a );
	    gtk_widget_show ( entree_personnalisation_affichage_echeances );
	    gtk_widget_show ( bouton_personnalisation_affichage_echeances );
	    gtk_widget_show ( bouton_valider_echeance_perso );
	    break;

	case 0:
	case 1:
	case 2:
	case 3:
	    affichage_echeances = GPOINTER_TO_INT ( origine );
	    gtk_widget_hide ( entree_personnalisation_affichage_echeances );
	    gtk_widget_hide ( bouton_personnalisation_affichage_echeances );
	    gtk_widget_hide ( bouton_valider_echeance_perso );

	    break;

    }

    remplissage_liste_echeance ();
    update_couleurs_background_echeancier ();
    selectionne_echeance ( echeance_selectionnnee );

    modification_fichier ( TRUE );

    return FALSE;
}
/*****************************************************************************/

/*****************************************************************************/
/* reçoit en argument une échéance et une date, renvoie la date suivante     */
/* en accord avec la périodicité de l'échéance				     */
/* renvoie null si la date limite est dépassée ou si c'est une fois	     */
/*****************************************************************************/
GDate *date_suivante_echeance ( struct operation_echeance *echeance,
				GDate *pGDateCurrent )
{
    if ( !echeance -> periodicite )
    {
	pGDateCurrent = NULL;

	return ( pGDateCurrent );
    }

    /* périodicité hebdomadaire */
    if ( echeance -> periodicite == 1 )
    {
	g_date_add_days ( pGDateCurrent,
			  7 );
	/* magouille car il semble y avoir un bug dans g_date_add_days
	   qui ne fait pas l'addition si on ne met pas la ligne suivante */
	g_date_add_months ( pGDateCurrent,
			    0 );
    }
    else
	/* périodicité mensuelle */
	if ( echeance -> periodicite == 2 )
	    g_date_add_months ( pGDateCurrent,
				1 );
	else
	    /* périodicité annuelle */
	    if ( echeance -> periodicite == 3 )
		g_date_add_years ( pGDateCurrent,
				   1 );
	    else
		/* périodicité perso */
		if ( !echeance -> intervalle_periodicite_personnalisee )
		{
		    g_date_add_days ( pGDateCurrent,
				      echeance -> periodicite_personnalisee );

		    /* magouille car il semble y avoir un bug dans g_date_add_days
		       qui ne fait pas l'addition si on ne met pas la ligne suivante */
		    g_date_add_months ( pGDateCurrent,
					0 );
		}
		else
		    if ( echeance -> intervalle_periodicite_personnalisee == 1 )
			g_date_add_months ( pGDateCurrent,
					    echeance -> periodicite_personnalisee );
		    else
			g_date_add_years ( pGDateCurrent,
					   echeance -> periodicite_personnalisee );

    if ( echeance -> date_limite
	 &&
	 g_date_compare ( pGDateCurrent,
			  echeance -> date_limite ) > 0 )
	pGDateCurrent = NULL;

    return ( pGDateCurrent );
}
/*****************************************************************************/



/******************************************************************************/
/* cette fonction affiche les traits verticaux et horizontaux sur la liste des échéances */
/******************************************************************************/
gboolean affichage_traits_liste_echeances ( void )
{

    GdkWindow *fenetre;
    gint i;
    gint largeur, hauteur;
    gint x, y;
    GtkAdjustment *adjustment;
    gint derniere_ligne;

    fenetre = gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view_liste_echeances ));

    gdk_drawable_get_size ( GDK_DRAWABLE ( fenetre ),
			    &largeur,
			    &hauteur );

    if ( !gc_separateur_operation )
	gc_separateur_operation = gdk_gc_new ( GDK_DRAWABLE ( fenetre ));

    /*     si la hauteur des lignes n'est pas encore calculée, on le fait ici */

    hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_liste_echeances );

    /*     on commence par calculer la dernière ligne en pixel correspondant à la dernière opé de la liste */
    /* 	pour éviter de dessiner les traits en dessous */

    derniere_ligne = hauteur_ligne_liste_opes * GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances ))) -> length;
    hauteur = MIN ( derniere_ligne,
		    hauteur );

    /*     le plus facile en premier... les lignes verticales */
    /*     dépend de si on est en train de ventiler ou non */
    /*     on en profite pour ajuster nb_ligne_ope_tree_view */

    x=0;

    for ( i=0 ; i<6 ; i++ )
    {
	x = x + gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_echeancier[i] ));
	gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			gc_separateur_operation,
			x, 0,
			x, hauteur );
    }

    /*     les lignes horizontales : il faut calculer la position y de chaque changement d'opé à l'écran */
    /*     on calcule la position y de la 1ère ligne à afficher */

    if ( hauteur_ligne_liste_opes )
    {
	adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_liste_echeances ));

	y = ( hauteur_ligne_liste_opes ) * ( ceil ( adjustment->value / hauteur_ligne_liste_opes )) - adjustment -> value;

	do
	{
	    gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			    gc_separateur_operation,
			    0, y, 
			    largeur, y );
	    y = y + hauteur_ligne_liste_opes;
	}
	while ( y < ( adjustment -> page_size )
		&&
		y <= derniere_ligne );
    }

    return FALSE;
}
/******************************************************************************/







/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
