/* ************************************************************************** */
/* fichier qui s'occupe de tout ce qui concerne l'échéancier                */
/* 			echeances_liste.c                                     */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)  	      */
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

/*START_INCLUDE*/
#include "echeancier_liste.h"
#include "utils_exercices.h"
#include "type_operations.h"
#include "barre_outils.h"
#include "echeancier_formulaire.h"
#include "utils_devises.h"
#include "dialog.h"
#include "operations_formulaire.h"
#include "utils_dates.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "fenetre_principale.h"
#include "gtk_combofix.h"
#include "echeancier_infos.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_comptes.h"
#include "operations_liste.h"
#include "utils_str.h"
#include "structures.h"
#include "echeancier_liste.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajuste_scrolling_liste_echeances_a_selection ( void );
static gboolean changement_taille_liste_echeances ( GtkWidget *tree_view,
					     GtkAllocation *allocation );
static gint cherche_echeance_from_ligne ( gint ligne );
static GtkTreeIter *cherche_iter_echeance ( gint scheduled_number );
static gint cherche_ligne_echeance ( gint scheduled_number );
static gboolean click_ligne_echeance ( GtkWidget *tree_view,
				GdkEventButton *evenement );
static GDate *date_fin_affichage_liste_echeance ( void );
static GDate *date_suivante_echeance ( gint scheduled_number,
				GDate *pGDateCurrent );
static gboolean gsb_gui_popup_custom_periodicity_dialog ();
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

gint nb_days_before_scheduled;      /* nb de jours avant l'échéance pour prévenir */

gint ancienne_largeur_echeances;

/** lists of number of scheduled transactions taken or to be taken */
GSList *scheduled_transactions_to_take;
GSList *scheduled_transactions_taken;


/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_grise;
extern GdkColor couleur_selection;
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GdkGC *gc_separateur_operation;
extern gint hauteur_ligne_liste_opes;
extern GtkWidget *label_saisie_modif;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern GtkTreeStore *model;
extern GtkTreeSelection * selection;
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
    GtkWidget *vbox, *scrolled_window;
    GtkListStore *store;
    gint i;
    gchar *titres_echeance[] = {
	_("Date"), _("Account"), _("Payee"), _("Frequency"), 
	_("Mode"), _("Notes"), _("Amount"), _("Balance")
    };
    gfloat col_justs[] = {
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_LEFT, COLUMN_CENTER, 
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_RIGHT, COLUMN_RIGHT
    };

    /*   à la base, on a une vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 0 );
    gtk_widget_show ( vbox );

    /* création de la barre d'outils */ 
    gtk_box_pack_start ( GTK_BOX ( vbox ), creation_barre_outils_echeancier(),
			 FALSE, FALSE, 0 );

    /* création de la scrolled window */ 
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ), scrolled_window, TRUE, TRUE, 0 );
    gtk_widget_show ( scrolled_window );

    /*     création du tree view */ 
    tree_view_liste_echeances = gtk_tree_view_new ();
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view_liste_echeances );
    gtk_widget_show ( tree_view_liste_echeances );

    /*     on ne peut sélectionner qu'une ligne */

    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection ( GTK_TREE_VIEW( tree_view_liste_echeances ))),
				  GTK_SELECTION_NONE );

    /* 	met en place la grille */

    if ( etat.affichage_grille )
	g_signal_connect_after ( G_OBJECT ( tree_view_liste_echeances ), "expose-event",
				 G_CALLBACK ( affichage_traits_liste_echeances ),
				 NULL );

    /* vérifie le simple ou double click */ 
    g_signal_connect ( G_OBJECT ( tree_view_liste_echeances ), "button_press_event",
		       G_CALLBACK ( click_ligne_echeance ),
		       NULL );

    /* vérifie la touche entrée, haut et bas */ 
    g_signal_connect ( G_OBJECT ( tree_view_liste_echeances ), "key_press_event",
		       G_CALLBACK ( traitement_clavier_liste_echeances ),
		       NULL );
    
/*     ajuste les colonnes si modification de la taille */ 
    g_signal_connect ( G_OBJECT ( tree_view_liste_echeances ), "size-allocate",
		       G_CALLBACK ( changement_taille_liste_echeances ),
		       NULL );
    
    /*     création des colonnes */
    for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
    {
	GtkCellRenderer *cell_renderer;

	cell_renderer = gtk_cell_renderer_text_new ();

	g_object_set ( G_OBJECT (GTK_CELL_RENDERER ( cell_renderer )), "xalign",
		       col_justs[i], NULL );

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
				 G_TYPE_INT,
				 PANGO_TYPE_FONT_DESCRIPTION );

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view_liste_echeances),
			      GTK_TREE_MODEL ( store ));

    remplissage_liste_echeance();
    update_couleurs_background_echeancier ();
    selectionne_echeance (-1);

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
    gint scheduled_number;

    switch ( evenement->keyval )
    {
	case GDK_Return :		/* touches entrée */
	case GDK_KP_Enter :

	    edition_echeance (gsb_data_scheduled_get_current_scheduled_number ());
	    break;


	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    ligne_selectionnee = cherche_ligne_echeance ( gsb_data_scheduled_get_current_scheduled_number () );

	    if ( ligne_selectionnee )
	    {
		/* 		on saute les doubles d'échéances */

		do
		    ligne_selectionnee--;
		while ( !(scheduled_number = cherche_echeance_from_ligne ( ligne_selectionnee ))
			&&
			ligne_selectionnee ); 

		selectionne_echeance (scheduled_number);
	    }
	    break;


	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :


	    if ( gsb_data_scheduled_get_current_scheduled_number () != -1 )
	    {
		ligne_selectionnee = cherche_ligne_echeance ( gsb_data_scheduled_get_current_scheduled_number () );

		/* 		on saute les doubles d'échéances */

		do
		    ligne_selectionnee++;
		while ( !(scheduled_number = cherche_echeance_from_ligne ( ligne_selectionnee ))); 

		selectionne_echeance (scheduled_number);
	    }
	    break;


	case GDK_Delete :               /*  del  */

	    supprime_echeance ( gsb_data_scheduled_get_current_scheduled_number () );
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
void click_sur_saisir_echeance ( gint scheduled_number )
{
    formulaire_echeancier_a_zero();
    degrise_formulaire_echeancier();

    gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			 _("Input") );
    edition_echeance (scheduled_number);

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
    GSList *slist_ptr;
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

    slist_ptr = gsb_data_scheduled_get_scheduled_list ();

    while ( slist_ptr )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (slist_ptr -> data);

	if ( !gsb_data_scheduled_get_mother_scheduled_number (scheduled_number))
	{
	    gchar *ligne[NB_COLS_SCHEDULER];
	    GDate *pGDateCurrent;
	    gint scheduled_number_buf;

	    if ( !g_date_valid ( gsb_data_scheduled_get_date (scheduled_number)))
		continue;

	    ligne[COL_NB_DATE] = gsb_format_gdate (gsb_data_scheduled_get_date (scheduled_number));

	    if ( gsb_data_scheduled_get_frequency (scheduled_number) == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
	    {
		switch (gsb_data_scheduled_get_user_interval (scheduled_number))
		{
		    case 0:
			ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d days"),
								    gsb_data_scheduled_get_user_entry (scheduled_number));
			break;

		    case 1:
			ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d months"),
								    gsb_data_scheduled_get_user_entry (scheduled_number));
			break;

		    case 2:
			ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d years"),
								    gsb_data_scheduled_get_user_entry (scheduled_number));
		}
	    }
	    else
		if ( gsb_data_scheduled_get_frequency (scheduled_number) < SCHEDULER_PERIODICITY_NB_CHOICES
		     &&
		     gsb_data_scheduled_get_frequency (scheduled_number) > 0 )
		{
		    gchar * names[] = { _("Once"), _("Weekly"), _("Montly"), 
			_("Bimonthly"), _("Quarterly"), _("Yearly") };
		    ligne[COL_NB_FREQUENCY] = names [gsb_data_scheduled_get_frequency (scheduled_number)];
		}

	    ligne[COL_NB_ACCOUNT] = gsb_data_account_get_name (gsb_data_scheduled_get_account_number (scheduled_number));
	    ligne[COL_NB_PARTY] = gsb_data_payee_get_name (gsb_data_scheduled_get_party_number (scheduled_number),
							   TRUE );
	    ligne[COL_NB_NOTES] = gsb_data_scheduled_get_notes (scheduled_number);
	    ligne[COL_NB_AMOUNT] = g_strdup_printf ( "%4.2f", gsb_data_scheduled_get_amount (scheduled_number));

	    if ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number))
		ligne[COL_NB_MODE]=_("Automatic");
	    else
		ligne[COL_NB_MODE] = _("Manual");

	    /* scheduled_number_buf contient l'adr de
	     * l'échéance mère, puis NULL pour les échéances
	     * calculées */
	    scheduled_number_buf = scheduled_number;

	    pGDateCurrent = gsb_date_copy (gsb_data_scheduled_get_date (scheduled_number));

	    do
	    {
		gtk_list_store_append ( GTK_LIST_STORE (store), &iter );

		for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
		    gtk_list_store_set ( GTK_LIST_STORE ( store ), &iter,
					 i, ligne[i], -1 );

		/* on met le numéro de l'échéance celui ci est
		 * à NULL si c'est une échéance calculée */
		gtk_list_store_set ( GTK_LIST_STORE ( store ), &iter,
				     SCHEDULER_COL_NB_TRANSACTION_NUMBER, scheduled_number_buf,
				     -1 );

		pGDateCurrent = date_suivante_echeance ( scheduled_number, pGDateCurrent );
		    
		ligne[COL_NB_DATE] = gsb_format_gdate ( pGDateCurrent );
		scheduled_number_buf = 0;
	    }
	    while ( pGDateCurrent &&
		    g_date_compare ( date_fin, pGDateCurrent ) > 0 &&
		    affichage_echeances != SCHEDULER_PERIODICITY_ONCE_VIEW &&
		    gsb_data_scheduled_get_frequency (scheduled_number));
	}
	slist_ptr = slist_ptr -> next;
    }


    /* met la ligne blanche */

    gtk_list_store_append ( GTK_LIST_STORE (store),
			    &iter );

    /* on met le numéro de l'échéance à -1 */

    gtk_list_store_set ( GTK_LIST_STORE ( store ),
			 &iter,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, -1,
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
	gint scheduled_number;

	gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
			     &iter,
			     SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number,
			     -1 );

	if ( scheduled_number )
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
void selectionne_echeance ( gint scheduled_number )
{
    GtkTreeIter *iter;
    GdkColor *couleur;
    GtkTreeModel *model;

    if ( DEBUG )
    {
	if ( scheduled_number < 0)
	    printf ( "White scheduled selection\n");
	else
	    printf ( "scheduled selection number %d\n",scheduled_number );
    }

    if ( (scheduled_number == gsb_data_scheduled_get_current_scheduled_number ()
	 &&
	 gsb_data_scheduled_get_current_scheduled_number ()
	 &&
	 selection_echeance_finie)
	 ||
	 !scheduled_number )
	return;


    model = GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances )));

    /*   vire l'ancienne sélection : consiste à remettre la couleur d'origine du background */

    if ( gsb_data_scheduled_get_current_scheduled_number ())
    {
	iter = cherche_iter_echeance (gsb_data_scheduled_get_current_scheduled_number ());

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

    gsb_data_scheduled_set_current_scheduled_number (scheduled_number);

    iter = cherche_iter_echeance (scheduled_number);

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

    if ( gsb_data_scheduled_get_current_scheduled_number () > 0)
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_saisir_echeancier ),
				   TRUE );
    else
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_saisir_echeancier ),
				   FALSE );
}



/******************************************************************************/
void ajuste_scrolling_liste_echeances_a_selection ( void )
{
    GtkAdjustment *v_adjustment;
    gint y_ligne;

    /*     si on n'a pas encore récupéré la hauteur des lignes, on va le faire ici */

    if ( !hauteur_ligne_liste_opes )
	hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_liste_echeances );

    v_adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_liste_echeances ));

    y_ligne = cherche_ligne_echeance ( gsb_data_scheduled_get_current_scheduled_number () ) * hauteur_ligne_liste_opes;

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




/* 
 * fonction cherche_iter_echeance
 * retrouve l'iter correspondant à l'opération donnée en argument dans la tree_view des opérations
 * renvoie null si pas trouvé
 * 
 * \param scheduled_number
 * 
 * \return iter correspondant
 * */
GtkTreeIter *cherche_iter_echeance ( gint scheduled_number )
{
    gint scheduled_transaction_buf;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( !scheduled_number )
	return NULL;

    model = GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances )));

    /*     on va faire le tour de la liste, et dès qu'une opé = echeance */
    /* 	on retourne son iter */

    gtk_tree_model_get_iter_first ( model,
				    &iter );
    scheduled_transaction_buf = 0;

    do
    {
	gtk_tree_model_get ( model,
			     &iter,
			     SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_transaction_buf,
			     -1 );
    }
    while ( scheduled_transaction_buf != scheduled_number
	    &&
	    gtk_tree_model_iter_next ( model,
				       &iter ));

    if ( scheduled_transaction_buf == scheduled_number )
	return ( gtk_tree_iter_copy ( &iter ));
    else
	return NULL;
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction renvoie le no de ligne de l'opération en argument */
/******************************************************************************/
gint cherche_ligne_echeance ( gint scheduled_number )
{
    GtkTreeIter *iter;

    iter = cherche_iter_echeance ( scheduled_number );

    return ( utils_str_atoi ( gtk_tree_model_get_string_from_iter (  GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances ))),
							      iter )));
}
/******************************************************************************/



/******************************************************************************/
/* renvoie l'adr de l'opération correspondant  à la ligne envoyées */
/* en argument */
/******************************************************************************/
gint cherche_echeance_from_ligne ( gint ligne )
{
    GtkTreeIter iter;
    gint scheduled_number;

    if ( !gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances ))),
					       &iter,
					       utils_str_itoa (ligne)))
	return 0;

    gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_echeances ))),
			 &iter,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number,
			 -1 );

    return ( scheduled_number );
}
/******************************************************************************/



/*****************************************************************************/
GDate *date_fin_affichage_liste_echeance ( void )
{
    GDate *date_fin;
    
   /* on récupère la date du jour et la met dans date_fin pour les
    * vérifications ultérieures */

    date_fin = gdate_today ();

    /* on calcule la date de fin de l'affichage */

    switch ( affichage_echeances )
    {
	case SCHEDULER_PERIODICITY_WEEK_VIEW:
	    g_date_add_days ( date_fin, 7 );
	    g_date_add_months ( date_fin, 0 );
	    break;

	case SCHEDULER_PERIODICITY_MONTH_VIEW:
	    g_date_add_months ( date_fin, 1 );
	    date_fin -> day = 1;
	    break;

	case SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW:
	    g_date_add_months ( date_fin, 2 );
	    date_fin -> day = 1;
	    break;

	case SCHEDULER_PERIODICITY_TRIMESTER_VIEW:
	    g_date_add_months ( date_fin, 3 );
	    date_fin -> day = 1;
	    break;

	case SCHEDULER_PERIODICITY_YEAR_VIEW:
	    g_date_add_years ( date_fin, 1 );
	    date_fin -> day = 1;
	    date_fin -> month = 1;
	    break;

	case SCHEDULER_PERIODICITY_CUSTOM_VIEW:
	    switch ( affichage_echeances_perso_j_m_a )
	    {
		case PERIODICITY_DAYS:
		    g_date_add_days ( date_fin, affichage_echeances_perso_nb_libre );
		    break;

		case PERIODICITY_WEEKS:
		    g_date_add_days ( date_fin, affichage_echeances_perso_nb_libre * 7 );
		    break;

		case PERIODICITY_MONTHS:
		    g_date_add_months ( date_fin, affichage_echeances_perso_nb_libre );
		    break;

		case PERIODICITY_YEARS:
		    g_date_add_years ( date_fin, affichage_echeances_perso_nb_libre );
		    break;
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
    gint scheduled_number;

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
					  SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number,
					  -1 );

    if ( !scheduled_number )
	return TRUE;

    selectionne_echeance (scheduled_number);


    if ( evenement -> type == GDK_2BUTTON_PRESS )
	edition_echeance (scheduled_number);
    else
	gtk_widget_grab_focus ( tree_view );

    return TRUE;
}
/*****************************************************************************/



/**
 * Ready the form to create a new scheduled transaction
 */
void new_scheduled_transaction ( void )
{
    selectionne_echeance ( -1 );
    gsb_data_scheduled_set_current_scheduled_number (-1);
    edition_echeance (-1);

/*     affiche_nb_ecritures_listes (); */
}


/*****************************************************************************/
/* Fonction edition_echeance						     */
/* appelée par double click ou entrée sur une échéance. Elle recopie	     */
/* l'échéance sélectionnée dans le formulaire d'échéance pour la modifier    */
/*****************************************************************************/
void edition_echeance ( gint scheduled_number )
{
    gchar *char_tmp;
    
    /*   si le formulaire est caché, on le montre */

    gtk_widget_show ( frame_formulaire_echeancier );
    gtk_expander_set_expanded ( GTK_EXPANDER(frame_formulaire_echeancier), TRUE );
    degrise_formulaire_echeancier ();

    /* si c'est une nouvelle échéance, on met la date courante,
       la sélectionne et lui donne le focus */

    if ( scheduled_number < 0)
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
			 gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)));

    /* mise en place du tiers */

    char_tmp = gsb_data_payee_get_name ( gsb_data_scheduled_get_party_number (scheduled_number),
					 TRUE );

    if ( char_tmp )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
				char_tmp );
    }


    /* mise en place du compte */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
				  recherche_compte_dans_option_menu (  widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
								       gsb_data_scheduled_get_account_number (scheduled_number)));

    /* mise en place des montants et des types associés */

    if ( gsb_data_scheduled_get_amount (scheduled_number) < 0 )
    {
	GtkWidget *menu;
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			     g_strdup_printf ( "%4.2f",
					       -gsb_data_scheduled_get_amount (scheduled_number)));
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
					       gsb_data_scheduled_get_amount (scheduled_number)));
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
				  devise_par_no (gsb_data_scheduled_get_currency_number (scheduled_number)) -> no_devise - 1 );

    /* si le compte de virement est -1, c'est qu'il n'y a pas de categ, ni de virements, ni de ventil */

    if ( gsb_data_scheduled_get_account_number_transfer (scheduled_number) != -1 )
    {
	gchar *texte;

	if ( gsb_data_scheduled_get_category_number (scheduled_number))
	{
	    /* 	    il y a donc des catégs/sous-catég */

	    texte = gsb_data_category_get_name ( gsb_data_scheduled_get_category_number (scheduled_number),
						 gsb_data_scheduled_get_sub_category_number (scheduled_number),
						 NULL );
	}
	else
	{
	    /* 	    c'est soit un virement, soit une opé ventilée */

	    if ( gsb_data_scheduled_get_account_number_transfer (scheduled_number))
	    {
		texte = g_strconcat ( COLON(_("Transfer")),
				      gsb_data_account_get_name (gsb_data_scheduled_get_account_number_transfer (scheduled_number)),
				      NULL );
	    }
	    else
	    {
		if ( gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
		{
		    texte =  g_strdup (_("Breakdown of transaction"));

		    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
					       FALSE );
		    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY],
					       FALSE );
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
				      cherche_no_menu_type_echeancier (gsb_data_scheduled_get_method_of_payment_number (scheduled_number)));

    if ( gsb_data_scheduled_get_method_of_payment_content (scheduled_number))
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			     gsb_data_scheduled_get_method_of_payment_content (scheduled_number));
    }


    /* met en place l'exercice */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( gsb_data_scheduled_get_financial_year_number (scheduled_number),
							      widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ));

    /* met en place l'imputation budgétaire */

    char_tmp = gsb_data_budget_get_name ( gsb_data_scheduled_get_budgetary_number (scheduled_number),
					  gsb_data_scheduled_get_sub_budgetary_number (scheduled_number),
					  NULL );

    if ( char_tmp )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY]);
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
				char_tmp );
    }

    /* mise en place de l'automatisme */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
				  gsb_data_scheduled_get_automatic_scheduled (scheduled_number));

    /*   remplit les notes */

    if ( gsb_data_scheduled_get_notes  (scheduled_number))
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			     gsb_data_scheduled_get_notes (scheduled_number));
    }

    /* mise en place de la périodicité */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
				  gsb_data_scheduled_get_frequency (scheduled_number));

    /* mise en place de la date limite */

    if ( gsb_data_scheduled_get_frequency (scheduled_number))
    {
	if (gsb_data_scheduled_get_limit_date (scheduled_number))
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
				 gsb_format_gdate ( gsb_data_scheduled_get_limit_date (scheduled_number)));
	}

	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
    }


    /* mise en place de la périodicité perso si nécessaire */

    if ( gsb_data_scheduled_get_frequency (scheduled_number) == 4 )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
	gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] ),
			     utils_str_itoa (gsb_data_scheduled_get_user_entry (scheduled_number)));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
				      gsb_data_scheduled_get_user_interval (scheduled_number));
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
			  "scheduled_number",
			   GINT_TO_POINTER (scheduled_number));
}
/*****************************************************************************/

/*****************************************************************************/
void supprime_echeance ( gint scheduled_number )
{
    gint resultat;

    if ( DEBUG )
	printf ( "supprime_echeance\n" );

    if ( !scheduled_number )
	scheduled_number = gsb_data_scheduled_get_current_scheduled_number ();

    if ( !scheduled_number
	 ||
	 scheduled_number < 0 )
	return;

    /* si la périodicité n'est pas de 1 fois demande juste celle ci ou toutes,
       sinon, choisit automatiquement toutes */

    if ( gsb_data_scheduled_get_frequency (scheduled_number))
    {
	GtkWidget * dialog;
	gchar * occurences = "";

	occurences = g_strdup_printf ( _("%s : %s [%4.2f %s]"),
				       gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)),
				       gsb_data_payee_get_name ( gsb_data_scheduled_get_party_number (scheduled_number), FALSE ),
				       gsb_data_scheduled_get_amount (scheduled_number),
				       devise_name ( devise_par_no (gsb_data_scheduled_get_currency_number (scheduled_number))));

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
	    gsb_scheduler_increase_scheduled_transaction (scheduled_number);
	    remplissage_liste_echeance ();
	    break;

	case 1:
	    gsb_scheduler_delete_scheduled_transaction (scheduled_number);
	    remplissage_liste_echeance ();
	    break;
    }

    mise_a_jour_calendrier();
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    modification_fichier (TRUE);
}
/*****************************************************************************/

/** delete the scheduled transaction, delete the children if it's a breakdown
 * \param scheduled_number the transaction to delete
 * \return FALSE
 * */
gboolean gsb_scheduler_delete_scheduled_transaction ( gint scheduled_number )
{
    /* check here if it's a breakdown */
    
    if ( gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
    {
	/* it is a breakdown, delete the children */

	GSList *list_ptr;

	list_ptr = gsb_data_scheduled_get_scheduled_list ();

	while ( list_ptr )
	{
	    gint scheduled_transaction_buf;

	    scheduled_transaction_buf = gsb_data_scheduled_get_scheduled_number (list_ptr -> data);

	    if ( gsb_data_scheduled_get_mother_scheduled_number (scheduled_transaction_buf) == scheduled_number )
	    {
		list_ptr = list_ptr -> next;
		gsb_data_scheduled_remove_scheduled (scheduled_transaction_buf);
	    }
	    else
		list_ptr = list_ptr -> next;
	}
    }

    /* now remove really the scheduled transaction */

    gsb_data_scheduled_remove_scheduled (scheduled_number);

    /* FIXME : faire descendre la sélection */
    return FALSE;
}

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



/** check the scheduled transactions if the are in time limit
 * and make the transactions if necessary
 * \param
 * \return
 * */
void gsb_scheduler_check_scheduled_transactions_time_limit ( void )
{
    GDate *pGDateCurrent;
    GSList *slist_ptr;
    GSList *last_slist_ptr;

    if ( DEBUG )
	printf ( "gsb_scheduler_check_scheduled_transactions_time_limit\n" );

    /* the scheduled transactions to take will be check here,
     * but the scheduled transactions taked will be add to the append ones */

    scheduled_transactions_to_take = NULL;

    /* get the date today + nb_days_before_scheduled */

    pGDateCurrent = gdate_today ();
    g_date_add_days ( pGDateCurrent,
		      nb_days_before_scheduled );

    /* check all the scheduled transactions,
     * if automatic, it's taken
     * if manual, appened into scheduled_transactions_to_take */

    slist_ptr = gsb_data_scheduled_get_scheduled_list ();
    last_slist_ptr = NULL;

    while ( slist_ptr )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (slist_ptr -> data);

	/* we check that scheduled transaction only if it's not a child of a breakdown */

	if ( !gsb_data_scheduled_get_mother_scheduled_number (scheduled_number)
	     &&
	     g_date_compare ( gsb_data_scheduled_get_date (scheduled_number),
			      pGDateCurrent ) <= 0 )
	{
	    if ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number))
	    {
		/* take automaticly the scheduled transaction untill today */

		while ( slist_ptr != last_slist_ptr
			&&
			g_date_compare ( gsb_data_scheduled_get_date (scheduled_number), pGDateCurrent ) <= 0 )
		{
		    gint transaction_number;

		    transaction_number = gsb_scheduler_create_transaction_from_scheduled_transaction (scheduled_number);

		    scheduled_transactions_taken = g_slist_append ( scheduled_transactions_taken,
								    GINT_TO_POINTER (transaction_number));

		    /* set the scheduled transaction to the next date,
		     * if it finished, we set slist_ptr on the slist just before that one
		     * and finish the while */

		    if ( !gsb_scheduler_increase_scheduled_transaction (scheduled_number))
		    {
			if ( !last_slist_ptr )
			{
			    last_slist_ptr = gsb_data_scheduled_get_scheduled_list ();
			    /* as we will check again all the scheduled transactions, set that list
			     * to NULL */
			    scheduled_transactions_to_take = NULL;
			}
			slist_ptr = last_slist_ptr;
		    }
		}
	    }
	    else
		/* it's a manual scheduled transaction, we put it in the slist */
		scheduled_transactions_to_take = g_slist_append ( scheduled_transactions_to_take ,
								  GINT_TO_POINTER (scheduled_number));
	}


	if ( last_slist_ptr == gsb_data_scheduled_get_scheduled_list ()
	     &&
	     slist_ptr == last_slist_ptr )
	    last_slist_ptr = NULL;
	else
	{
	    last_slist_ptr = slist_ptr;
	    slist_ptr = slist_ptr -> next;
	}
    }

    if ( scheduled_transactions_taken )
    {
	mise_a_jour_liste_echeances_auto_accueil = 1;
	modification_fichier ( TRUE );
    }

    if ( scheduled_transactions_to_take )
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
}
/*****************************************************************************/


gboolean gsb_gui_popup_custom_periodicity_dialog ()
{
    GtkWidget * dialog, *hbox, *hbox2, *paddingbox, *omenu, *menu, *label, *entry, *item;
    gchar * names[] = { _("days"), _("weeks"), _("months"), _("years"), NULL };
    int i;

    dialog = gtk_dialog_new_with_buttons ( _("Scheduler frequency"), 
					   GTK_WINDOW (window), GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_APPLY, GTK_RESPONSE_OK,
					   NULL);

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0 );
    paddingbox = new_paddingbox_with_title ( hbox, TRUE, _("Scheduler frequency") );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(paddingbox), 6 );

    hbox2 = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox2, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Show transactions for the next "));
    gtk_box_pack_start ( GTK_BOX(hbox2), label, FALSE, FALSE, 0 );
    entry = new_int_spin_button ( &affichage_echeances_perso_nb_libre, 
				  0.0, 65536.0, 1.0, 5.0, 1.0, 1.0, 0, NULL );
    gtk_box_pack_start ( GTK_BOX(hbox2), entry, FALSE, FALSE, 6 );

    omenu = gtk_option_menu_new ();
    menu = gtk_menu_new();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(omenu), menu );
    gtk_box_pack_start ( GTK_BOX(hbox2), omenu, FALSE, FALSE, 0 );

    for ( i = 0; names[i]; i++ )
    {
	item = gtk_menu_item_new_with_label ( names[i] );
	gtk_menu_append ( menu, item );
    }
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( omenu ),
				  affichage_echeances_perso_j_m_a );

    gtk_widget_show_all ( dialog );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
	case GTK_RESPONSE_OK:
	    affichage_echeances_perso_j_m_a = gtk_option_menu_get_history ( GTK_OPTION_MENU ( omenu ) );
	    affichage_echeances_perso_nb_libre = utils_str_atoi ( gtk_entry_get_text ( GTK_ENTRY(entry)) );
	    gtk_widget_destroy ( dialog );
	    return TRUE;
    }

    gtk_widget_destroy ( dialog );
    return FALSE;
}



/*****************************************************************************/
/* Fonction appelée lorsqu'on change le bouton pour l'affichage des	     */
/* échéances ( choix mois, 2 mois ... )					     */
/*****************************************************************************/
gboolean gsb_gui_change_scheduler_view ( enum scheduler_periodicity periodicity )
{
    gchar * names[] = { _("Unique view"), _("Week view"), _("Month view"), 
			_("Two months view"), _("Quarter view"), 
			_("Year view"), _("Custom view"), NULL };

    if ( periodicity == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
    {
	if ( ! gsb_gui_popup_custom_periodicity_dialog () )
	    return FALSE;
    }

    gsb_gui_headings_update ( g_strconcat ( _("Scheduled transactions"), " : ", 
					    names[periodicity], NULL), "" );

    affichage_echeances = periodicity;
    remplissage_liste_echeance ();
    update_couleurs_background_echeancier ();
    selectionne_echeance (gsb_data_scheduled_get_current_scheduled_number ());

    modification_fichier ( TRUE );

    return FALSE;
}




/*****************************************************************************/
/* reçoit en argument une échéance et une date, renvoie la date suivante     */
/* en accord avec la périodicité de l'échéance				     */
/* renvoie null si la date limite est dépassée ou si c'est une fois	     */
/*****************************************************************************/
GDate *date_suivante_echeance ( gint scheduled_number,
				GDate *pGDateCurrent )
{
    if ( !gsb_data_scheduled_get_frequency (scheduled_number))
    {
	pGDateCurrent = NULL;

	return ( pGDateCurrent );
    }

    switch (gsb_data_scheduled_get_frequency (scheduled_number))
    {
	case SCHEDULER_PERIODICITY_WEEK_VIEW:
	    g_date_add_days ( pGDateCurrent, 7 );
	    /* magouille car il semble y avoir un bug dans g_date_add_days
	       qui ne fait pas l'addition si on ne met pas la ligne suivante */
	    g_date_add_months ( pGDateCurrent, 0 );
	    break;

	case SCHEDULER_PERIODICITY_MONTH_VIEW:
	    g_date_add_months ( pGDateCurrent, 1 );
	    break;

	case SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW:
	    g_date_add_months ( pGDateCurrent, 2 );
	    break;

	case SCHEDULER_PERIODICITY_TRIMESTER_VIEW:
	    g_date_add_months ( pGDateCurrent, 3 );
	    break;

	case SCHEDULER_PERIODICITY_YEAR_VIEW:
	    g_date_add_years ( pGDateCurrent, 1 );

	case SCHEDULER_PERIODICITY_CUSTOM_VIEW:
	    switch (gsb_data_scheduled_get_user_interval (scheduled_number))
	    {
		case PERIODICITY_DAYS:
		    g_date_add_days ( pGDateCurrent, 
				      gsb_data_scheduled_get_user_entry (scheduled_number));
		    g_date_add_months ( pGDateCurrent, 0 );
		    break;

		case PERIODICITY_WEEKS:
		    g_date_add_days ( pGDateCurrent, 
				      gsb_data_scheduled_get_user_entry (scheduled_number) * 7 );
		    g_date_add_months ( pGDateCurrent, 0 );
		    break;

		case PERIODICITY_MONTHS:
		    g_date_add_months ( pGDateCurrent,
					gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_YEARS:
		    g_date_add_years ( pGDateCurrent,
				       gsb_data_scheduled_get_user_entry (scheduled_number));
		    g_date_add_months ( pGDateCurrent, 0 );
		    break;
	    }
	    break;
    }

    if ( gsb_data_scheduled_get_limit_date (scheduled_number)
	 &&
	 g_date_compare ( pGDateCurrent,
			  gsb_data_scheduled_get_limit_date (scheduled_number)) > 0 )
    {
	pGDateCurrent = NULL;
    }
    
    return ( pGDateCurrent );
}



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
