/* ************************************************************************** */
/*  fichier qui gère la liste des opérations                                  */
/* 			liste_operations.c                                    */
/*                                                                            */
/*     copyright (c)	2000-2005 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org) 	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
/*			http://www.grisbi.org   			      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the gnu general public license as published by      */
/*  the free software foundation; either version 2 of the license, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but without any warranty; without even the implied warranty of            */
/*  merchantability or fitness for a particular purpose.  see the             */
/*  gnu general public license for more details.                              */
/*                                                                            */
/*  You should have received a copy of the gnu general public license         */
/*  along with this program; if not, write to the free software               */
/*  foundation, inc., 59 temple place, suite 330, boston, ma  02111-1307  usa */
/*                                                                            */
/* ************************************************************************** */

#include "include.h"
#include "mouse.h"
#include "operations_formulaire_constants.h"
#include "utils_c.h"

#include "gtk_cell_renderer_expander.h"


/*START_INCLUDE*/
#include "operations_liste.h"
#include "accueil.h"
#include "utils_montants.h"
#include "utils_exercices.h"
#include "operations_formulaire.h"
#include "echeancier_formulaire.h"
#include "barre_outils.h"
#include "type_operations.h"
#include "comptes_traitements.h"
#include "utils.h"
#include "utils_devises.h"
#include "dialog.h"
#include "echeancier_liste.h"
#include "equilibrage.h"
#include "gsb_account.h"
#include "utils_dates.h"
#include "format.h"
#include "gsb_transaction_data.h"
#include "classement_operations.h"
#include "gtk_cell_renderer_expander.h"
#include "gtk_combofix.h"
#include "utils_str.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "operations_comptes.h"
#include "traitement_variables.h"
#include "utils_categories.h"
#include "utils_ib.h"
#include "utils_operations.h"
#include "parametres.h"
#include "utils_rapprochements.h"
#include "utils_tiers.h"
#include "utils_types.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean assert_selected_transaction ();
static struct structure_operation *cherche_operation_from_ligne ( gint ligne,
							   gint no_account );
static void creation_titres_tree_view ( void );
static GtkWidget *creation_tree_view_operations ( void );
static gint find_balance_col ( void );
static gint find_balance_line ( void );
static gint find_p_r_line ();
static gboolean gsb_transactions_list_change_sort_type ( GtkWidget *menu_item,
						  gint *no_column );
static gboolean gsb_transactions_list_check_mark ( struct structure_operation *transaction );
static gboolean gsb_transactions_list_current_transaction_down ( gint no_account );
static gboolean gsb_transactions_list_current_transaction_up ( gint no_account );
static gboolean gsb_transactions_list_delete_transaction_from_slist ( struct structure_operation *transaction );
static gboolean gsb_transactions_list_delete_transaction_from_tree_view ( struct structure_operation *transaction );
static struct structure_operation *gsb_transactions_list_find_white_breakdown ( struct structure_operation *breakdown_mother );
static GtkTreePath *gsb_transactions_list_get_list_path_from_sorted_path ( GtkTreePath *path_sorted,
								    gint no_account );
static GtkTreePath *gsb_transactions_list_get_path_from_transaction ( struct structure_operation *transaction,
							       gint no_account );
static GtkTreePath *gsb_transactions_list_get_sorted_path_from_list_path ( GtkTreePath *path,
								    gint no_account );
static struct structure_operation *gsb_transactions_list_get_transaction_from_path ( GtkTreePath *path,
									      gint no_account );
static struct structure_operation *gsb_transactions_list_get_transaction_next ( struct structure_operation *transaction );
static gboolean gsb_transactions_list_sort_column_changed ( GtkTreeViewColumn *tree_view_column,
						     gint *no_account );
static gboolean gsb_transactions_list_title_column_button_press ( GtkWidget *button,
							   GdkEventButton *ev,
							   gint *no_column );
static gboolean move_operation_to_account ( struct structure_operation * transaction,
				     gint account );
static void move_selected_operation_to_account ( GtkMenuItem * menu_item );
static void p_press (void);
static void popup_transaction_context_menu ( gboolean full );
static void r_press (void);
static gchar *recherche_contenu_cellule ( struct structure_operation *transaction,
				   gint no_affichage );
static void schedule_selected_transaction ();
static struct operation_echeance *schedule_transaction ( struct structure_operation * transaction );
static gdouble solde_debut_affichage ( gint no_account );
/*END_STATIC*/



/*  adr du notebook qui contient les opés de chaque compte */

GtkWidget *notebook_listes_operations;

/* adr de la frame contenant le formulaire */

GtkWidget *frame_droite_bas;

/* adr de la barre d'outils */

GtkWidget *barre_outils;

/* contient les tips et titres des colonnes des listes d'opé */

gchar *tips_col_liste_operations[TRANSACTION_LIST_COL_NB];
gchar *titres_colonnes_liste_operations[TRANSACTION_LIST_COL_NB];


/* hauteur d'une ligne de la liste des opés */

gint hauteur_ligne_liste_opes;

/* on va essayer de créer un object tooltip général pour grisbi */
/* donc associer tous les autres tooltips à ce tooltip (fixme) */

GtkTooltips *tooltips_general_grisbi;

/* le GdkGc correspondant aux lignes, créés au début une fois pour toute */

GdkGC *gc_separateur_operation;

/* utilisé pour éviter que ça s'emballe lors du réglage de la largeur des colonnes */

gint allocation_precedente;

/* sauvegarde de la dernière date entree */

gchar *last_date;

/*  pointeur vers le label qui contient le solde sous la liste des opé */

GtkWidget *solde_label;

/*  pointeur vers le label qui contient le solde pointe sous la liste des opé */

GtkWidget *solde_label_pointe;


/*START_EXTERN*/
extern GtkWidget *bouton_ok_equilibrage;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_grise;
extern GdkColor couleur_selection;
extern struct struct_devise *devise_compte;
extern struct struct_devise *devise_operation;
extern struct operation_echeance *echeance_selectionnnee;
extern GtkWidget *formulaire;
extern GtkWidget *label_equilibrage_ecart;
extern GtkWidget *label_equilibrage_pointe;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GSList *liste_labels_titres_colonnes_liste_ope ;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_echeances;
extern gint max;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkTreeStore *model;
extern gint nb_colonnes;
extern gint nb_echeances;
extern gint no_derniere_echeance;
extern GtkWidget *notebook_general;
extern gdouble operations_pointees;
extern PangoFontDescription *pango_desc_fonte_liste;
extern gint rapport_largeur_colonnes[TRANSACTION_LIST_COL_NB];
extern GtkTreeSelection * selection;
extern gdouble solde_final;
extern gdouble solde_initial;
extern GtkStyle *style_entree_formulaire[2];
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern gint taille_largeur_colonnes[TRANSACTION_LIST_COL_NB];
extern GtkWidget *tree_view;
extern GtkWidget *treeview;
extern GtkWidget *window;
/*END_EXTERN*/




/******************************************************************************/
/*  routine qui crée la fenêtre de la liste des opé  */
/******************************************************************************/

GtkWidget *creation_fenetre_operations ( void )
{
    GtkWidget *win_operations;

    /*   la fenetre des opé est une vbox : la liste en haut, le solde et  */
    /*     des boutons de conf au milieu, le formulaire en bas */
    allocation_precedente = 0;

    win_operations = gtk_vbox_new ( FALSE, 6 );

    /* création de la barre d'outils */
    barre_outils = creation_barre_outils ();
    gtk_box_pack_start ( GTK_BOX ( win_operations ), barre_outils, FALSE, FALSE, 0);
    gtk_widget_show ( barre_outils );

    /* création du notebook des opé */

    notebook_listes_operations = creation_tree_view_operations ();
    gtk_box_pack_start ( GTK_BOX ( win_operations ), notebook_listes_operations,
			 TRUE, TRUE, 0);
    gtk_widget_show ( notebook_listes_operations );

    return ( win_operations );
}
/******************************************************************************/


/******************************************************************************/
/* création des treeview des opérations					      */
/* y ajoute les colonnes de la liste des opés */
/* retour le treeviw */
/******************************************************************************/
GtkWidget *creation_tree_view_operations ( void )
{
    GtkWidget *hbox;
    GSList *list_tmp;

    if ( DEBUG )
	printf ( "creation_tree_view_operations\n" );

    hbox = gtk_hbox_new ( FALSE,
			  0 );

    /*     1 tree view avec plusieurs list_store est trop lent */
    /* 	on crée donc une hbox contenant tous les tree_view (1 par compte) */
    /* 	les tree_view seront remplis petit à petit */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GtkWidget *tree_view;

	i = gsb_account_get_no_account ( list_tmp -> data );
	tree_view = creation_tree_view_operations_par_compte (i);

	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     tree_view,
			     TRUE,
			     TRUE,
			     0 );

	/* if we are on current account, we show the list */

	if ( i == gsb_account_get_current_account ())
	    gtk_widget_show ( tree_view );
	
	list_tmp = list_tmp -> next;
    }


    /*     la hauteur est mise à 0 pour l'instant, elle sera remplie dès que nécessaire */

    hauteur_ligne_liste_opes = 0;
    gc_separateur_operation = NULL;

    return ( hbox );
}
/******************************************************************************/



/******************************************************************************/
/* crée le tree_view avec les colonnes et les signaux pour le compte donné en */
/* argument */
/* le renvoie */
/* \param no_account no de compte dont le tree_view est à créer */
/******************************************************************************/
GtkWidget *creation_tree_view_operations_par_compte ( gint no_account )
{
    GtkListStore *store;
    GtkTreeModel *sortable;
    GtkTreeModel *filter;
    GtkWidget *tree_view;
    GtkWidget *scrolled_window;

    scrolled_window = gtk_scrolled_window_new ( NULL,
						NULL );
    gsb_account_set_scrolled_window ( no_account,
				      scrolled_window );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN );

    /*     création proprement dite du treeview */

    tree_view = gtk_tree_view_new ();

    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			tree_view );
    gtk_widget_show ( tree_view );

    gsb_account_set_tree_view ( no_account,  
				tree_view );

    /*  we cannot do a selection */

    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection ( GTK_TREE_VIEW( tree_view ))),
				  GTK_SELECTION_NONE );

    /* 	met en place la grille */

    if ( etat.affichage_grille )
	g_signal_connect_after ( G_OBJECT ( tree_view ),
				 "expose-event",
				 G_CALLBACK ( affichage_traits_liste_operation ),
				 NULL );

    /* vérifie le simple ou double click */

    g_signal_connect ( G_OBJECT ( tree_view ),
		       "button_press_event",
		       G_CALLBACK ( gsb_transactions_list_button_press ),
		       NULL );

    /* vérifie la touche entrée, haut et bas */

    g_signal_connect ( G_OBJECT ( tree_view ),
		       "key_press_event",
		       G_CALLBACK ( gsb_transactions_list_key_press ),
		       NULL );

    /*     ajuste les colonnes si modification de la taille */

    g_signal_connect ( G_OBJECT ( tree_view ),
		       "size-allocate",
		       G_CALLBACK ( changement_taille_liste_ope ),
		       NULL );

    /*     normalement les colonnes sont déjà créés */
    /* mais bon, on teste, sait on jamais... */

    if ( gsb_account_get_column ( no_account, TRANSACTION_COL_NB_CHECK) )
    {
	gint j;

	for ( j = 0 ; j < TRANSACTION_LIST_COL_NB ; j++ )
	{
	    GtkTreeViewColumn *column;

	    column = GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( no_account, j));

	    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
					  column );

	    if ( !j )
		gtk_tree_view_column_set_clickable ( gsb_account_get_column ( no_account, j),
						     FALSE );

	    /* 	    set the tooltips */

	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   column ->button,
				   tips_col_liste_operations[j],
				   tips_col_liste_operations[j] ); 

	    g_signal_connect ( G_OBJECT ( column -> button),
			       "button-press-event",
			       G_CALLBACK ( gsb_transactions_list_title_column_button_press ),
			       GINT_TO_POINTER (j));

	    /* 	    after changing the sort of the list, we update the background color */

	    g_signal_connect_after ( G_OBJECT ( column ),
				     "clicked",
				     G_CALLBACK ( gsb_transactions_list_sort_column_changed ),
				     GINT_TO_POINTER ( no_account ));

	}
    }
    else
	printf ( "bizarre, les colonnes d'opérations n'ont pas encore été créés (operation_liste.c)...\n" );

    /*     on crée le list_store maintenant et on l'associe vide au tree_view */

    /* description in the constants */

    store = gtk_list_store_new ( TRANSACTION_COL_NB_TOTAL,
				 G_TYPE_BOOLEAN,
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
				 PANGO_TYPE_FONT_DESCRIPTION,
				 G_TYPE_INT,
				 G_TYPE_BOOLEAN,
				 G_TYPE_OBJECT,
				 G_TYPE_OBJECT,
				 G_TYPE_BOOLEAN,
				 G_TYPE_BOOLEAN );

    /*     save the store */

    gsb_account_set_store ( no_account,
			    store );

    /*     make the model_filter, on the store */

    filter = gtk_tree_model_filter_new ( GTK_TREE_MODEL ( store ),
					 NULL );
    gtk_tree_model_filter_set_visible_column ( GTK_TREE_MODEL_FILTER ( filter ),
					       TRANSACTION_COL_NB_VISIBLE );

    /* make the model_sort, on the model_filter */

    sortable = gtk_tree_model_sort_new_with_model ( filter );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( sortable ),
					   gsb_account_get_sort_column ( no_account ),
					   gsb_account_get_sort_type ( no_account ));

    /*     set the compare functions by click on the column */

    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable ),
				      TRANSACTION_COL_NB_CHECK,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_0,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable ),
				      TRANSACTION_COL_NB_DATE,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_1,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable ),
				      TRANSACTION_COL_NB_PARTY,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_2,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable ),
				      TRANSACTION_COL_NB_PR,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_3,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable ),
				      TRANSACTION_COL_NB_DEBIT,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_4,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable ),
				      TRANSACTION_COL_NB_CREDIT,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_5,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable ),
				      TRANSACTION_COL_NB_BALANCE,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_6,
				      GINT_TO_POINTER ( no_account ),
				      NULL );

    /*     put the top in the tree_view */

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view),
			      GTK_TREE_MODEL ( sortable ));


    gsb_account_set_last_transaction ( no_account,
				       NULL );

    return scrolled_window;
}
/******************************************************************************/


/** called after a click on a column title ; the sort of the list is automatic,
 * that function make the background color and the rest to be updated
 * to make sure that we don't forget anything, call demarrage_idle
 *
 * \param tree_view_column the tree_view_column clicked
 * \param no_account a pointer which contains the number of account
 * \return FALSE
 * */
gboolean gsb_transactions_list_sort_column_changed ( GtkTreeViewColumn *tree_view_column,
						     gint *no_account )
{
    GtkSortType sort_type;
    gint sort_column_id;

    if ( DEBUG )
	printf ( "gsb_transactions_list_sort_column_changed\n" );

    /* the third click give no sort for the column
     * we forbid that here, so now, there is always a column which sort the tree view */

    gtk_tree_sortable_get_sort_column_id ( GTK_TREE_SORTABLE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view ( gsb_account_get_current_account ())))),
					   &sort_column_id,
					   &sort_type );

    if ( sort_column_id == -1 )
    {
	gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view ( gsb_account_get_current_account ())))),
					       gtk_tree_view_column_get_sort_column_id ( tree_view_column ),
					       sort_type );
	return FALSE;
    }

    gsb_account_set_finished_background_color ( GPOINTER_TO_INT ( no_account ),
						0 );
    modification_fichier ( TRUE );
    demarrage_idle ();
    return FALSE;
}

/******************************************************************************/
/* cette fonction est appelée pour créer les tree_view_column des listes d'opé et */
/* de ventil */
/******************************************************************************/
void creation_titres_tree_view ( void )
{
    GSList *list_tmp;


    if ( !titres_colonnes_liste_operations )
	return;

    if ( !tooltips_general_grisbi )
	tooltips_general_grisbi = gtk_tooltips_new ();


    /*     on commence par s'occuper des listes d'opérations */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	creation_colonnes_tree_view_par_compte (i);

	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction crée les colonnes du tree view du compte donné en argument */
/* \param no_account le compte dont on veut créer les colonnes */
/******************************************************************************/
void creation_colonnes_tree_view_par_compte ( gint no_account )
{
    gint i;
    gfloat alignement[] = {
	COLUMN_CENTER,
	COLUMN_CENTER,
	COLUMN_CENTER,
	COLUMN_LEFT,
	COLUMN_CENTER,
	COLUMN_RIGHT,
	COLUMN_RIGHT,
	COLUMN_RIGHT
    };
    gint column_balance;

    /*     on récupère la position de cellule du solde pour pouvoir le mettre en rouge */

    column_balance = find_balance_col ();

    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell_renderer;

	/* 	if it's the column 0, we put an expander, else it's a text column */

	if ( i )
	{
	    cell_renderer = gtk_cell_renderer_text_new ();

	    column = gtk_tree_view_column_new_with_attributes ( titres_colonnes_liste_operations[i],
								cell_renderer,
								"text", i,
								"cell-background-gdk", TRANSACTION_COL_NB_BACKGROUND,
								"font-desc", TRANSACTION_COL_NB_FONT,
								NULL );
	}
	else
	{
	    cell_renderer = gtk_cell_renderer_expander_new ();

	    column = gtk_tree_view_column_new_with_attributes ( titres_colonnes_liste_operations[i],
								cell_renderer,
								"is-expanded", TRANSACTION_COL_NB_IS_EXPANDED,
								"is-expander", TRANSACTION_COL_NB_IS_EXPANDER,
								"visible", TRANSACTION_COL_NB_IS_NOT_BREAKDOWN,
								"cell-background-gdk", TRANSACTION_COL_NB_BACKGROUND,
								NULL );
	}

	g_object_set ( G_OBJECT ( GTK_CELL_RENDERER (cell_renderer )),
		       "xalign",
		       alignement[i],
		       NULL );


	gtk_tree_view_column_set_alignment ( column,
					     alignement[i] );
	gtk_tree_view_column_set_sizing ( column,
					  GTK_TREE_VIEW_COLUMN_FIXED );
	gtk_tree_view_column_set_sort_column_id ( column,
						  i );

	if ( etat.largeur_auto_colonnes )
	    gtk_tree_view_column_set_resizable ( column,
						 FALSE );
	else
	    gtk_tree_view_column_set_resizable ( column,
						 TRUE );

	gsb_account_set_column ( no_account,
				 i,
				 column);
    }

    /*     pour la colonne du solde, on rajoute le foreground */

    if ( column_balance != -1 )
	gtk_tree_view_column_add_attribute ( gsb_account_get_column ( no_account, column_balance),
					     gtk_tree_view_column_get_cell_renderers ( gsb_account_get_column ( no_account, column_balance)) -> data,
					     "foreground", TRANSACTION_COL_NB_AMOUNT_COLOR );
}
/******************************************************************************/




/******************************************************************************/
/* cette fonction est appelée une fois que les titres et tips pour les col */
/* de la liste d'opé ont été créés. donc soit on les crée, soit on les update */
/******************************************************************************/
void update_titres_tree_view ( void )
{
    gint i;
    GSList *list_tmp;

    if ( !titres_colonnes_liste_operations )
	return;

    if ( !gsb_account_get_column ( gsb_account_get_current_account (), TRANSACTION_COL_NB_CHECK))
	creation_titres_tree_view ();


    /*     on s'occupe des listes d'opérations */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint j;

	j = gsb_account_get_no_account ( list_tmp -> data );

	for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
	{
	    gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( j, i)),
					     titres_colonnes_liste_operations[i] );

	    if ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( j, i))->button )
	    {
		gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				       GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( j, i))->button,
				       tips_col_liste_operations[i],
				       tips_col_liste_operations[i] ); 
	    }
	}

	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/




/******************************************************************************/
/* cette fonction vérifie que la liste d'opé du compte demandée est comlètement */
/* terminée. elle est normalement appelée en cas de changement de comptef */
/* si la liste n'est pas finie, on la termine ici */
/******************************************************************************/
void verification_list_store_termine ( gint no_account )
{
    if ( gsb_account_get_last_transaction (no_account) != GINT_TO_POINTER (-1))
	gsb_transactions_list_fill_store ( no_account,
					   0 );

    if ( !gsb_account_get_finished_background_color (no_account) )
	gsb_transactions_list_set_background_color ( no_account );

    if ( !gsb_account_get_finished_balance_showed (no_account) )
	gsb_transactions_list_set_transactions_balances ( no_account);

    return;
}
/******************************************************************************/





/******************************************************************************/
/* remplissage du list_store du compte donné en argument */
/* cette fonction efface l'ancienne liste et la remplit avec les données actuelles */
/* elle remplit la liste d'un seul coup */
/* elle met à jour les soldes courant et pointés */
/******************************************************************************/
void remplissage_liste_operations ( gint compte )
{
    if ( DEBUG )
	printf ( "remplissage_liste_operations compte %d\n", compte );

    /*     on efface la liste */

    gtk_list_store_clear ( gsb_account_get_store (compte) );

    /*     plus rien n'est affiché */

    gsb_account_set_last_transaction ( compte,
				       NULL );
    gsb_account_set_finished_background_color ( compte,
						0 );
    /*     affichage_solde_fini = 0; */
    gsb_account_set_finished_selection_transaction ( compte,
						     0 );

    /*     on remplit le list_store */

    gsb_transactions_list_fill_store ( compte,
				       0 );

    /*     on met les couleurs du fond */

    gsb_transactions_list_set_background_color ( compte );

    /*     on met les soldes */

    gsb_transactions_list_set_transactions_balances ( compte );

}
/******************************************************************************/


/** fill the list_store with the transactions of the account
 * begin at the last transaction stored in the account, or at the begining of the account
 * used especially to fill all the accounts by idle
 * \param no_account
 * \param by_part if TRUE, stop after a max number of transactions and return like that
 * in that case, the last transaction is stored in the account
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_store ( gint no_account,
					    gboolean by_part )
{
    GSList *liste_transactions_tmp;
    gint nb_max_ope_a_ajouter;
    gint nb_ope_ajoutees;

    /*     on décide pour l'instant arbitrairement du nb d'opé à ajouter si on */
    /* 	le fait par partie ; peut être dans les préférences un jour */

    if ( by_part )
	nb_max_ope_a_ajouter = 20;
    else
	nb_max_ope_a_ajouter = -1;

    nb_ope_ajoutees = 0;

    /*     si gsb_account_get_last_transaction () n'est pas nul, on se place sur l'opé suivante */

    if ( gsb_account_get_last_transaction (no_account) )
    {
	liste_transactions_tmp = gsb_account_get_last_transaction (no_account);
	liste_transactions_tmp = liste_transactions_tmp -> next;
    }
    else
	liste_transactions_tmp = gsb_account_get_transactions_list (no_account);

    /*     on commence la boucle  */

    while ( liste_transactions_tmp
	    &&
	    ( nb_ope_ajoutees < nb_max_ope_a_ajouter
	      ||
	      nb_max_ope_a_ajouter == -1))
    {
	struct structure_operation *transaction;

	transaction = liste_transactions_tmp -> data;

	/* 	    on met tout de suite l'opé en cours pour permettre de reprendre à la suivante lors */
	/* 		du remplissage de la liste */

	gsb_account_set_last_transaction ( no_account,
					   liste_transactions_tmp );

	nb_ope_ajoutees++;

	/* 	    on ajoute l'opé à la fin du list_store */

	gsb_transactions_list_append_transaction ( transaction,
						   no_account );

	/* if the transaction is a breakdown mother, we happen a white line,
	 * which is a normal transaction but with nothing and with the breakdown
	 * relation to the last transaction */

	if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
	    gsb_transactions_list_append_white_breakdown ( transaction );

	liste_transactions_tmp = liste_transactions_tmp -> next;
    }

    /*     si liste_transactions_tmp n'est pas NULL, c'est qu'on n'est pas allé jusqu'au bout */
    /* 	des opés du comptes, donc on s'en va maintenant */

    if ( liste_transactions_tmp )
	return FALSE;

    /* the list of transactions is now complete,
     * we happen the white line */

    gsb_transactions_list_append_transaction ( GINT_TO_POINTER ( -1 ),
					       no_account );

    /*     on est allé jusqu'au bout du no_account */

    gsb_account_set_last_transaction ( no_account,
				       GINT_TO_POINTER ( -1 ));
    gsb_account_set_finished_visible_rows ( no_account,
					    0 );
    return FALSE; 
}
/******************************************************************************/



/**
 * Create a white breakdown of transaction, associate with the
 * mother given in param and append that line to the list
 *
 * \param transaction The mother of the white breakdown
 *
 * \return a pointer to that new transaction
 * */
struct structure_operation *gsb_transactions_list_append_white_breakdown ( struct structure_operation *transaction )
{
    struct structure_operation *breakdown_transaction;
    /* FIXME : doit passer pas la création d'une opé, donc s'arrête là pour le moment */
exit (0);
    breakdown_transaction = calloc ( 1,
				     sizeof ( struct structure_operation ));
    gsb_transaction_data_set_transaction_number ( breakdown_transaction,
						  -2 );
/*     breakdown_transaction -> date = g_date_new_dmy ( g_date_get_day ( transaction -> date ), */
/* 						     g_date_get_month ( transaction -> date ), */
/* 						     g_date_get_year ( transaction -> date )); */
/*     breakdown_transaction -> no_compte = transaction -> no_compte; */
/*     breakdown_transaction -> tiers = transaction -> tiers; */
/*     breakdown_transaction -> no_operation_ventilee_associee = gsb_transaction_data_get_transaction_number (transaction); */
/*     gsb_transactions_list_append_transaction ( breakdown_transaction, */
/* 					       transaction -> no_compte ); */

    return breakdown_transaction;
}


/** append the transaction to the store of the account
 * make the 4 lines, after the filter will decide if we show them or not
 * \param transaction
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_append_transaction ( struct structure_operation *transaction,
						    gint no_account )
{
    gint j;
    GtkListStore *store;

    store = gsb_account_get_store (no_account);

    for ( j = 0 ; j < TRANSACTION_LIST_ROWS_NB ; j++ )
    {
	GtkTreeIter iter;

	gtk_list_store_append ( store,
				&iter );

	gsb_transactions_list_fill_row ( transaction,
					 &iter,
					 store,
					 j );

	/* if it's a breakdown, there is only 1 line */

	if ( transaction != GINT_TO_POINTER (-1)
	     &&
	     transaction -> no_operation_ventilee_associee )
	    j = TRANSACTION_LIST_ROWS_NB;
    }
    return FALSE;
}
/******************************************************************************/



/** fill the row given in param by iter
 * \param transaction
 * \param iter the iter corresponding to the row
 * \param store
 * \param line_in_transaction
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_row ( struct structure_operation *transaction,
					  GtkTreeIter *iter,
					  GtkListStore *store,
					  gint line_in_transaction )
{
    gint i;

    /* if it's a white line (the last one or for breakdowns, we don't fill */

    if ( transaction != GINT_TO_POINTER (-1)
	 &&
	 gsb_transaction_data_get_transaction_number (transaction) != -2 )
    {
	for ( i = 1 ; i < TRANSACTION_LIST_COL_NB ; i++ )
	{
	    gtk_list_store_set ( store,
				 iter,
				 i, recherche_contenu_cellule ( transaction,
								tab_affichage_ope[line_in_transaction][i] ),
				 -1 );
	}

	/* work with the col 0 */

	gtk_list_store_set ( store,
			     iter,
			     TRANSACTION_COL_NB_IS_NOT_BREAKDOWN, TRUE,
			     -1 );

	if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction ))
	     &&
	     !line_in_transaction )
	{
		gtk_list_store_set ( store,
				     iter,
				     TRANSACTION_COL_NB_IS_EXPANDER, TRUE,
				     TRANSACTION_COL_NB_IS_EXPANDED, FALSE,
				     -1 );
	}
    }

    /* if we use a custom font... */

    if ( etat.utilise_fonte_listes )
	gtk_list_store_set ( store,
			     iter,
			     TRANSACTION_COL_NB_FONT, pango_desc_fonte_liste,
			     -1 );

    /* set the number of line in the transaction */

    gtk_list_store_set ( store,
			 iter,
			 TRANSACTION_COL_NB_TRANSACTION_LINE, line_in_transaction,
			 -1 );

    /* set the address of the transaction */

    gtk_list_store_set ( store,
			 iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, transaction,
			 -1 );

    return FALSE;
}
/******************************************************************************/




/******************************************************************************/
/* Fonction recherche_contenu_cellule */
/* prend en argument l'opération concernée */
/* et le numéro de l'argument qu'on veut afficher (tab_affichage_ope) */
/* renvoie la chaine à afficher ou NULL */
/******************************************************************************/
gchar *recherche_contenu_cellule ( struct structure_operation *transaction,
				   gint no_affichage )
{
    gchar *temp;
    gdouble montant;

    /* if it's a breakdown and we want to see the party,
     * we show the category */

    if ( transaction -> no_operation_ventilee_associee
	 &&
	 no_affichage == TRANSACTION_LIST_PARTY )
	no_affichage = TRANSACTION_LIST_CATEGORY;

    switch ( no_affichage )
    {
	/* mise en forme de la date */

	case TRANSACTION_LIST_DATE:
	    return gsb_format_gdate(gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (transaction)));
	    break;

	    /* mise en forme de la date de valeur */

	case TRANSACTION_LIST_VALUE_DATE:
	    return gsb_format_gdate(gsb_transaction_data_get_value_date (gsb_transaction_data_get_transaction_number (transaction)));
	    break;

	    /* mise en forme du tiers */

	case TRANSACTION_LIST_PARTY:
	    return ( tiers_name_by_no ( gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (transaction )), TRUE ));
	    break;

	    /* mise en forme de l'ib */

	case TRANSACTION_LIST_BUDGET:

	    temp = nom_imputation_par_no ( transaction -> imputation,
					   transaction -> sous_imputation );
	    break;


	    /* mise en forme du débit */
	case TRANSACTION_LIST_DEBIT:
	    if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))< -0.001 ) 
		/* -0.001 is to handle float approximations */
	    {
		temp = g_strdup_printf ( "%4.2f", -gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction )));

		/* si la devise en cours est différente de celle de l'opé, on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction )))
		    devise_operation = devise_par_no ( gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction )));

		if ( devise_operation -> no_devise != gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))) )
		    temp = g_strconcat ( temp,
					 "(",
					 devise_code ( devise_operation ),
					 ")",
					 NULL );

		return ( temp );
	    }
	    else
		return ( NULL );

	    break;

	    /* mise en forme du crédit */
	case TRANSACTION_LIST_CREDIT:

	    if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))>= 0 )
	    {
		temp = g_strdup_printf ( "%4.2f", gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction )));

		/* si la devise en cours est différente de celle de l'opé, on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction )))
		    devise_operation = devise_par_no ( gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction )));

		if ( devise_operation -> no_devise != gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))) )
		    temp = g_strconcat ( temp,
					 "(",
					 devise_code ( devise_operation ),
					 ")",
					 NULL );

		return ( temp );
	    }
	    else
		return ( NULL );

	    break;

	case TRANSACTION_LIST_BALANCE:
	    return NULL;
	    break;

	    /* mise en forme du montant dans la devise du compte */

	case TRANSACTION_LIST_AMOUNT:
	    if ( gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction ))!= gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))) )
	    {
		/* on doit calculer et afficher le montant de l'ope */

		montant = gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

		return ( g_strdup_printf ( "(%4.2f %s)",
					   montant,
					   devise_code ( devise_compte ) ));
	    }
	    else
		return ( NULL );

	    break;

	    /* mise en forme du moyen de paiement */

	case TRANSACTION_LIST_TYPE:
	    return ( type_ope_name_by_no ( transaction -> type_ope,
					   gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))));
	    break;

	    /* mise en forme du no de rapprochement */

	case TRANSACTION_LIST_RECONCILE_NB:
	    return ( rapprochement_name_by_no ( transaction -> no_rapprochement ));
	    break;

	    /* mise en place de l'exo */

	case TRANSACTION_LIST_EXERCICE:
	    return ( exercice_name_by_no (  transaction -> no_exercice ));
	    break;

	    /* mise en place des catégories */

	case TRANSACTION_LIST_CATEGORY:

	    return ( gsb_transactions_get_category_real_name ( transaction ));
	    break;

	    /* mise en forme R/P */

	case TRANSACTION_LIST_MARK:
	    if ( transaction -> pointe == 1 )
		return ( _("P") );
	    else
	    {
		if ( transaction -> pointe == 2 )
		    return ( _("T") );
		else
		{
		    if ( transaction -> pointe == 3 )
			return ( _("R"));
		    else
			return ( NULL );
		}
	    }
	    break;


	    /* mise en place de la pièce comptable */

	case TRANSACTION_LIST_VOUCHER:
	    return ( transaction -> no_piece_comptable );
	    break;

	    /* mise en forme des notes */

	case TRANSACTION_LIST_NOTES:
	    return ( gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (transaction )));
	    break;

	    /* mise en place de l'info banque/guichet */

	case TRANSACTION_LIST_BANK:
	    return ( transaction -> info_banque_guichet );
	    break;

	    /* mise en place du no d'opé */

	case TRANSACTION_LIST_NO:
	    return ( itoa ( gsb_transaction_data_get_transaction_number (transaction) ));
	    break;

	    /* mise en place du no de chèque/virement */

	case TRANSACTION_LIST_CHQ:
	    if ( transaction -> contenu_type )
		return ( g_strconcat ( "(",
				       transaction -> contenu_type,
				       ")",
				       NULL ));
	    else
		return ( NULL );
	    break;

    }
    return ( NULL );
}
/******************************************************************************/


/** 
 * set the alterance color of the background or the transactions list for the given account
 * at the end, select the current transaction
 * 
 * \param no_account account
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_background_color ( gint no_account )
{
    gint couleur_en_cours;
    GtkTreeModel *model;
    GtkTreeModelSort *model_sort;
    GtkTreeModelFilter *model_filter;
    GtkTreePath *path_filter;
    GtkTreePath *path_sort;
    gint i;
    gint nb_rows_by_transaction;

    if ( DEBUG )
	printf ( "gsb_transactions_list_set_background_color :  no_account %d\n", no_account );

    model = GTK_TREE_MODEL (gsb_account_get_store (no_account));
    model_sort = GTK_TREE_MODEL_SORT (gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view ( no_account ))));
    model_filter = GTK_TREE_MODEL_FILTER ( gtk_tree_model_sort_get_model ( GTK_TREE_MODEL_SORT ( model_sort )));
    nb_rows_by_transaction = gsb_account_get_nb_rows ( no_account );

    couleur_en_cours = 0;
    i = 0;

    path_sort = gtk_tree_path_new_first ();

    while ( (path_filter = gtk_tree_model_sort_convert_path_to_child_path ( GTK_TREE_MODEL_SORT ( model_sort ),
									    path_sort )))
    {
	GtkTreePath *path;
	GtkTreeIter iter;
	struct structure_operation *transaction;

	/* 	now, normally we needn't to verify something, they must exit */

	path = gtk_tree_model_filter_convert_path_to_child_path ( GTK_TREE_MODEL_FILTER ( model_filter ),
								  path_filter );
	gtk_tree_model_get_iter ( GTK_TREE_MODEL ( model ),
				  &iter,
				  path );

	/* if it's a breakdown, it's not the same color */

	gtk_tree_model_get ( model,
			     &iter,
			     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			     -1 );

	if ( transaction != GINT_TO_POINTER (-1)
	     &&
	     transaction -> no_operation_ventilee_associee )
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur_grise,
				 -1 );
	else
	{
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur_fond[couleur_en_cours],
				 -1 );

	    if ( ++i == nb_rows_by_transaction )
	    {
		i = 0;
		couleur_en_cours = 1 - couleur_en_cours;
	    }
	}

	gtk_tree_path_next ( path_sort );
    }

    gsb_account_set_finished_background_color ( no_account,
						1 );

    /*     on a viré la sélection en remettant la couleur de fond */

    gsb_account_set_finished_balance_showed ( no_account,
					      0 );
    gsb_account_set_finished_selection_transaction ( no_account,
						     0 );

    /* show the selected transaction */

    gsb_transactions_list_set_current_transaction ( gsb_account_get_current_transaction ( no_account ),
						    no_account );

    return FALSE;
}
/******************************************************************************/



/** put the balance for each transaction showed in the tree_view
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_transactions_balances ( gint no_account )
{
    gdouble solde_courant;
    gint column_balance;
    gint line_balance;
    GtkTreePath *path_sorted;
    GtkTreePath *path;
    GtkTreeModel *model;

    if ( DEBUG )
	printf ( "gsb_transactions_list_set_transactions_balances, no_account : %d\n", no_account );

    /*     on commence par retrouver la colonne et la ligne pour afficher le solde de l'opé */

    column_balance = find_balance_col ();
    line_balance = find_balance_line ();

    if ( line_balance == -1 
	 ||
	 line_balance >= gsb_account_get_nb_rows ( no_account ))
    {
	/* 	le solde n'est pas affiché, on peut repartir */
	gsb_account_set_finished_balance_showed ( no_account,
						  1 );
	return FALSE;
    }

    model = GTK_TREE_MODEL (gsb_account_get_store ( no_account ));

    path_sorted = gtk_tree_path_new_first ();
    path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted,
								  no_account );

    /*     on calcule le solde de démarrage */

    solde_courant = solde_debut_affichage ( no_account );

    while ( path )
    {
	struct structure_operation *transaction;
	GtkTreeIter iter;
	gint i;

	transaction = gsb_transactions_list_get_transaction_from_path ( path,
									no_account );

	if ( transaction != GINT_TO_POINTER (-1))
	{
	    /* if it's a breakdown, we do nothing */

	    if ( transaction -> no_operation_ventilee_associee )
	    {
		gtk_tree_path_next ( path_sorted );
		path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted,
									      no_account );
		continue;
	    }

	    solde_courant = solde_courant + gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

	    gtk_tree_model_get_iter ( model,
				      &iter,
				      path );

	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 column_balance, g_strdup_printf ( "%4.2f",
								   solde_courant ),
				 -1 );

	    /* 	on met la couleur du solde */

	    if ( solde_courant > -0.01 )
		gtk_list_store_set ( GTK_LIST_STORE ( model ),
				     &iter,
				     TRANSACTION_COL_NB_AMOUNT_COLOR, NULL,
				     -1 );

	    else
		gtk_list_store_set ( GTK_LIST_STORE ( model ),
				     &iter,
				     TRANSACTION_COL_NB_AMOUNT_COLOR, "red",
				     -1 );
	}

	for ( i=0 ; i<gsb_account_get_nb_rows ( no_account ) ; i++ )
	    gtk_tree_path_next ( path_sorted );

	path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted,
								      no_account );
    }

    gsb_account_set_finished_balance_showed ( no_account,
					      1 );
    return FALSE;
}
/******************************************************************************/


/** find column number for the transaction balance
 * \return column number
 * */
gint find_balance_col ( void )
{
   gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( tab_affichage_ope[i][j] == TRANSACTION_LIST_BALANCE )
		return j;
	}
    }

    return -1;
}

/** find line number for the transaction balance
 * \return line number
 * */
gint find_balance_line ( void )
{
   gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( tab_affichage_ope[i][j] == TRANSACTION_LIST_BALANCE )
		return i;
	}
    }

    return -1;
}





/******************************************************************************/
/* cette fonction calcule le solde de départ pour l'affichage de la première opé */
/* du compte */
/* c'est soit le solde initial du compte si on affiche les R */
/* soit le solde initial - les opés R si elles ne sont pas affichées */
/******************************************************************************/
gdouble solde_debut_affichage ( gint no_account )
{
    gdouble solde;
    GSList *liste_tmp;

    solde = gsb_account_get_init_balance (no_account);

    if ( gsb_account_get_r (no_account) )
	return (solde);

    /*     les R ne sont pas affichés, on les déduit du solde initial */

    liste_tmp = gsb_account_get_transactions_list (no_account);

    while ( liste_tmp )
    {
	struct structure_operation *transaction;

	transaction = liste_tmp -> data;

	/* 	si l'opé est ventilée ou non relevée, on saute */

	if ( !(transaction -> no_operation_ventilee_associee
	       ||
	       transaction -> pointe != 3 ))
	    solde = solde + gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

	liste_tmp = liste_tmp -> next;
    }

    return ( solde );
}
/******************************************************************************/



/** called when press a mouse button on the transactions list
 * \param tree_view
 * \param ev a GdkEventButton
 * \return FALSE or TRUE
 * */
gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
					      GdkEventButton *ev )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path_sorted;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    struct structure_operation *transaction;

    /*     if we are not in the list, go away */

    if ( ev -> window != gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )) )
	return(FALSE);

    /* get the path, 
     * if it's a right button and we are not in the list, show the partial popup
     * else go away */

    if ( !gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
					  ev -> x,
					  ev -> y,
					  &path_sorted,
					  &tree_column,
					  NULL,
					  NULL ))
    {
	/* 	éventuellement, si c'est un clic du bouton droit, on affiche la popup partielle */

	if ( ev -> button == RIGHT_BUTTON )
	    popup_transaction_context_menu ( FALSE );

	return (TRUE);
    }

    path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted,
								  gsb_account_get_current_account () );

    model = GTK_TREE_MODEL ( gsb_account_get_store ( gsb_account_get_current_account () ));

    /*     normally path cannot be NULL, in doubt, if it is, we select the white line
     *     we begin to get the selected transaction */

    if ( path )
    {

	gtk_tree_model_get_iter ( model,
				  &iter,
				  path );
	gtk_tree_model_get ( model,
			     &iter,
			     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			     -1 );
    }
    else
	transaction = GINT_TO_POINTER (-1);

    /*     we check if we don't want open a breakdown, before changing the selection */

    if ( tree_column == gsb_account_get_column ( gsb_account_get_current_account (),
						 0 )
	 &&
	 gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
    {
	gboolean breakdown_shown;

	gtk_tree_model_get ( model,
			     &iter,
			     TRANSACTION_COL_NB_IS_EXPANDED, &breakdown_shown,
			     -1 );

	breakdown_shown = !breakdown_shown;

	/* 	we show/hide the breadowns of that transaction */

	gsb_account_list_set_breakdowns_visible ( gsb_account_get_current_account (),
						  transaction,
						  breakdown_shown );
	gsb_transactions_list_set_background_color (gsb_account_get_current_account ());
	gsb_transactions_list_set_transactions_balances ( gsb_account_get_current_account () );

	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     &iter,
			     TRANSACTION_COL_NB_IS_EXPANDED, breakdown_shown,
			     -1 );



	/* in that case, we don't check anything else */

	return TRUE;
    }
    else
	gsb_transactions_list_set_current_transaction ( transaction,
							gsb_account_get_current_account () );

    /*     if it's the right click, put the good popup */

    if ( ev -> button == RIGHT_BUTTON
	 &&
	 transaction )
    {
	if ( transaction == GINT_TO_POINTER (-1))
	    popup_transaction_context_menu ( FALSE );
	else
	    popup_transaction_context_menu ( TRUE );
	return(TRUE);
    }

    /*     check if we press on the mark */

    if ( transaction != GINT_TO_POINTER ( -1 ))
    {
	gint column;

	column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
				tree_column );

	if ( column == find_p_r_col() )
	{
	    gint line_in_transaction;

	    gtk_tree_model_get ( model,
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_LINE, &line_in_transaction,
				 -1 );

	    if ( etat.equilibrage
		 &&
		 line_in_transaction == find_p_r_line())
	    {
		gsb_reconcile_mark_transaction (transaction);
		return TRUE;
	    }

	    if ( ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK ))
	    {
		p_press ();
		return TRUE;
	    }
	}
    }


    /*  if double - click */

    if ( ev -> type == GDK_2BUTTON_PRESS )
	gsb_transactions_list_edit_current_transaction ();

    return ( TRUE );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction gsb_transactions_list_key_press */
/* gère le clavier sur la liste des opés */
/******************************************************************************/
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev )
{
    switch ( ev -> keyval )
    {
	case GDK_Return :		/* entrée */
	case GDK_KP_Enter :

	    gsb_transactions_list_edit_current_transaction ();
	    break;

	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    gsb_transactions_list_current_transaction_up ( gsb_account_get_current_account () );
	    break;

	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    gsb_transactions_list_current_transaction_down ( gsb_account_get_current_account () );
	    break;

	case GDK_Delete:		/*  del  */
	    gsb_transactions_list_delete_transaction ( gsb_account_get_current_transaction (gsb_account_get_current_account ()) );
	    break;

	case GDK_P:			/* touche P */
	case GDK_p:			/* touche p */

	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		p_press ();
	    break;

	case GDK_r:			/* touche r */
	case GDK_R:			/* touche R */

	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		r_press ();
	    break;
    }

    return TRUE;
}
/******************************************************************************/


/** change the selection to the transaction above
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_current_transaction_up ( gint no_account )
{
    struct structure_operation *transaction;
    GtkTreePath *path;
    GtkTreePath *path_sorted;

    transaction = gsb_account_get_current_transaction ( no_account );

    path = gsb_transactions_list_get_path_from_transaction ( transaction,
							     no_account );
    path_sorted = gsb_transactions_list_get_sorted_path_from_list_path ( path,
									 no_account );

    gtk_tree_path_prev ( path_sorted );

    path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted,
								  no_account );

    if ( path )
    {
	transaction = gsb_transactions_list_get_transaction_from_path ( path,
									no_account );
	gsb_transactions_list_set_current_transaction ( transaction,
							no_account );
    }

    return FALSE;
}




/** change the selection to the transaction below
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_current_transaction_down ( gint no_account )
{
    struct structure_operation *transaction;

    transaction = gsb_transactions_list_get_transaction_next ( gsb_account_get_current_transaction ( no_account ));

    if ( transaction )
	gsb_transactions_list_set_current_transaction ( transaction,
							no_account );
    return FALSE;
}



/**
 * Return a pointer to the transaction after the one in param
 * in the tree_view
 * 
 * \param transaction The transaction we want to move down
 * 
 * \return the pointer to the new transaction or NULL if problem
 * */
struct structure_operation *gsb_transactions_list_get_transaction_next ( struct structure_operation *transaction )
{
    GtkTreePath *path;
    GtkTreePath *path_sorted;
    gint i;
    
    if ( transaction == GINT_TO_POINTER (-1))
	return NULL;

    path = gsb_transactions_list_get_path_from_transaction ( transaction,
							     gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));
    path_sorted = gsb_transactions_list_get_sorted_path_from_list_path ( path,
									 gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));
    /* if the transaction is a breakdown, the line is only 1 line */

    if ( transaction != GINT_TO_POINTER (-1)
	 &&
	 transaction -> no_operation_ventilee_associee )
	gtk_tree_path_next ( path_sorted );
    else
	for ( i=0 ; i<gsb_account_get_nb_rows ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))) ; i++ )
	    gtk_tree_path_next ( path_sorted );

    path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted,
								  gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));
    if ( path )
	transaction = gsb_transactions_list_get_transaction_from_path ( path,
									gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));
    else
	transaction = NULL;


    return transaction;
}



/** change the selection to the transaction
 * \param new_transaction
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_current_transaction ( struct structure_operation *new_transaction,
							 gint no_account )
{
    GtkTreeIter *iter;
    gint i;
    GdkColor *couleur;
    GtkTreeModel *model;
    struct structure_operation *current_transaction;

    if ( ! gsb_account_get_tree_view (no_account))
	return FALSE;

    if ( DEBUG )
    {
	if ( new_transaction == GINT_TO_POINTER (-1))
	    printf ( "gsb_transactions_list_set_current_transaction white\n");
	else
	    printf ( "gsb_transactions_list_set_current_transaction %d\n", gsb_transaction_data_get_transaction_number (new_transaction));
    }

    current_transaction =  gsb_account_get_current_transaction (no_account);

    /*     si gsb_account_get_finished_selection_transaction ()=0, c'est qu'il n'y a encore aucune sélection sur la liste */
    /*     donc new_transaction = current_transaction = -1, mais on ne se barre pas */
    /*     sinon si on est déjà dessus, on se barre */

    if ( new_transaction == current_transaction
	 &&
	 gsb_account_get_finished_selection_transaction (no_account))
	return FALSE;

    model = gsb_account_get_store ( no_account );

    /*   vire l'ancienne sélection : consiste à remettre la couleur d'origine du background */

    if ( gsb_account_get_finished_selection_transaction (no_account) )
    {
	iter = cherche_iter_operation ( current_transaction,
					no_account );

	if ( iter )
	{
	    /* 	iter est maintenant positionné sur la 1ère ligne de l'opé à désélectionner */

	    for ( i=0 ; i<gsb_account_get_nb_rows ( no_account ) ; i++ )
	    {
		gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				     iter,
				     TRANSACTION_COL_NB_SAVE_BACKGROUND, &couleur,
				     -1 );
		gtk_list_store_set ( GTK_LIST_STORE ( model ),
				     iter,
				     TRANSACTION_COL_NB_BACKGROUND,couleur,
				     -1 );
		gtk_list_store_set ( GTK_LIST_STORE ( model ),
				     iter,
				     TRANSACTION_COL_NB_SAVE_BACKGROUND, NULL,
				     -1 );

		/* if it's a breakdown of transaction, it's only 1 line */

		if ( current_transaction != GINT_TO_POINTER (-1)
		     &&
		     current_transaction -> no_operation_ventilee_associee )
		    i = gsb_account_get_nb_rows (no_account);

		gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					   iter );
	    }
	}
    }

    gsb_account_set_current_transaction ( no_account,
					  new_transaction );

    iter = cherche_iter_operation ( new_transaction,
				    no_account );

    if ( iter )
    {
	/* 	iter est maintenant positionne sur la 1ère ligne de l'ope à sélectionner */

	for ( i=0 ; i<gsb_account_get_nb_rows ( no_account ) ; i++ )
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur,
				 -1 );
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur_selection,
				 -1 );
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_SAVE_BACKGROUND, couleur,
				 -1 );

	    /* if it's a breakdown of transaction, it's only 1 line */

	    if ( new_transaction != GINT_TO_POINTER (-1)
		 &&
		 new_transaction -> no_operation_ventilee_associee )
		i = gsb_account_get_nb_rows (no_account);

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
    }

    gsb_account_set_finished_selection_transaction ( no_account,
						     1 );
    return FALSE;
}
/******************************************************************************/


/** put the tree view on the current transaction
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_move_to_current_transaction ( gint no_account )
{
    GtkTreePath *path;
    GtkTreePath *path_sorted;

    if ( DEBUG )
	printf ( "gsb_transactions_list_move_to_current_transaction, compte %d\n", no_account );

    path = gsb_transactions_list_get_path_from_transaction ( gsb_account_get_current_transaction (no_account),
							     no_account );
    path_sorted = gsb_transactions_list_get_sorted_path_from_list_path ( path,
									 no_account );

    /*     sometimes, the current transaction can be hidden, so we have to check for each path if it's valid */
    /* 	if it's not, we make a selection on the white line */

    if ( !path_sorted )
    {
/* 	gsb_transactions_list_set_current_transaction ( GINT_TO_POINTER (-1), */
/* 							no_account ); */
	return FALSE;
    }

    gtk_tree_view_scroll_to_cell ( gsb_account_get_tree_view ( no_account ),
				   path_sorted,
				   NULL,
				   FALSE,
				   0,
				   0 );


    return FALSE;
}
/******************************************************************************/


/** give back the path in the sorted tree_view from the path in the general transaction store
 * \param no_account
 * \return GtkTreePath or NULL if path is not in the sorted tree_view
 * */
GtkTreePath *gsb_transactions_list_get_sorted_path_from_list_path ( GtkTreePath *path,
								    gint no_account )
{
    GtkTreeModelFilter *model_filter;
    GtkTreeModelSort *model_sort;
    GtkTreePath *path_filter;
    GtkTreePath *path_sorted;

    if ( !path )
	return NULL;

    model_sort = GTK_TREE_MODEL_SORT ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view ( no_account ))));
    model_filter = GTK_TREE_MODEL_FILTER (  gtk_tree_model_sort_get_model( model_sort ));

    path_filter = gtk_tree_model_filter_convert_child_path_to_path ( model_filter,
								     path );

    if ( path_filter )
	path_sorted = gtk_tree_model_sort_convert_child_path_to_path ( model_sort,
								       path_filter );
    else
	path_sorted = NULL;

    return path_sorted;
}


/** give back the path in the general transaction store from the path in the sorted tree_view
 * \param no_account
 * \return GtkTreePath or NULL if path is not in the sorted tree_view (normally, cannot happen)
 * */
GtkTreePath *gsb_transactions_list_get_list_path_from_sorted_path ( GtkTreePath *path_sorted,
								    gint no_account )
{
    GtkTreeModelFilter *model_filter;
    GtkTreeModelSort *model_sort;
    GtkTreePath *path_filter;
    GtkTreePath *path;

    if ( !path_sorted )
	return NULL;

    model_sort = GTK_TREE_MODEL_SORT ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view ( no_account ))));
    model_filter = GTK_TREE_MODEL_FILTER (  gtk_tree_model_sort_get_model( model_sort ));

    path_filter = gtk_tree_model_sort_convert_path_to_child_path ( model_sort,
								   path_sorted );

    if ( path_filter )
	path = gtk_tree_model_filter_convert_path_to_child_path ( model_filter,
								  path_filter );
    else
	path = NULL;

    return path;
}


/******************************************************************************/
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view )
{
    GdkRectangle rectangle;

    gtk_tree_view_get_background_area ( GTK_TREE_VIEW ( tree_view ),
					gtk_tree_path_new_from_string ( "0" ),
					NULL,
					&rectangle );
    return ( rectangle.height );
}
/******************************************************************************/




/******************************************************************************/
/* renvoie l'adr de l'opération correspondant  à la ligne envoyées */
/* en argument */
/******************************************************************************/
struct structure_operation *cherche_operation_from_ligne ( gint ligne,
							   gint no_account )
{
    GtkTreeIter iter;
    struct structure_operation *transaction;
    GtkTreeModel *model;

    model = gsb_account_get_store ( no_account );

    if ( !gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL ( model ),
					       &iter,
					       itoa (ligne)))
	return NULL;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
			 &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 -1 );

    return ( transaction );

}
/******************************************************************************/


/******************************************************************************/
/* fonction cherche_iter_operation */
/* retrouve l'iter correspondant à l'opération donnée en argument dans la tree_view des opérations */
/* renvoie NULL si pas trouvé */
/******************************************************************************/
GtkTreeIter *cherche_iter_operation ( struct structure_operation *transaction,
				      gint no_account )
{
    struct structure_operation *transaction_pointer;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( !transaction || ! gsb_account_get_tree_view (no_account))
	return NULL;

    model = gsb_account_get_store ( no_account );

    /*     on va faire le tour de la liste, et dès qu'une opé = transaction */
    /* 	on retourne son iter */

    transaction_pointer = NULL;

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					 &iter ))
    {
	do
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_pointer,
				 -1 );
	}
	while ( transaction_pointer != transaction
		&&
		gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					   &iter ));
    }

    if ( transaction_pointer == transaction )
	return ( gtk_tree_iter_copy ( &iter ));
    else
	return NULL;
}
/******************************************************************************/



/**
 * Find the address of the white line of the breakdown given in param
 *
 * \param breakdown_mother The transaction mother of the line looked for
 *
 * \return the adress of the transaction or NULL
 * */
struct structure_operation *gsb_transactions_list_find_white_breakdown ( struct structure_operation *breakdown_mother )
{
    struct structure_operation *transaction_pointer;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( !breakdown_mother)
	return NULL;

    model = gsb_account_get_store ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (breakdown_mother)));

    transaction_pointer = NULL;

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					 &iter ))
    {
	do
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_pointer,
				 -1 );
	}
	while ( gsb_transaction_data_get_transaction_number (transaction_pointer) != -2
		&&
		transaction_pointer -> no_operation_ventilee_associee != gsb_transaction_data_get_transaction_number (breakdown_mother)
		&&
		gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					   &iter ));
    }

    if ( gsb_transaction_data_get_transaction_number (transaction_pointer) == -2
	 &&
	 transaction_pointer -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (breakdown_mother))
	return ( transaction_pointer );
    else
	return NULL;
}
/******************************************************************************/



/** give back the path of the transaction given
 * \param transaction the transaction we want to find the path
 * \param no_account
 * \return the GtkTreePath
 * */
GtkTreePath *gsb_transactions_list_get_path_from_transaction ( struct structure_operation *transaction,
							       gint no_account )
{
    GtkTreePath *path;
    GtkTreeIter *iter;

    if ( !transaction || ! gsb_account_get_tree_view (no_account ))
	return NULL;

    iter = cherche_iter_operation ( transaction,
				    no_account );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL (gsb_account_get_store ( no_account )),
				     iter );
    return path;
}



/** give back the transaction on the path
 * \param path the path in the normal list
 * \param no_account
 * \return the transaction
 * */
struct structure_operation *gsb_transactions_list_get_transaction_from_path ( GtkTreePath *path,
									      gint no_account )
{
    struct structure_operation *transaction;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( ! gsb_account_get_tree_view (no_account ))
	return NULL;

    model = GTK_TREE_MODEL ( gsb_account_get_store ( no_account ));

    gtk_tree_model_get_iter ( model,
			      &iter,
			      path );
    gtk_tree_model_get ( model,
			 &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 -1 );

    return transaction;
}



/******************************************************************************/
/* cette fonction renvoie le no de ligne de l'opération en argument */
/******************************************************************************/
gint cherche_ligne_operation ( struct structure_operation *transaction,
			       gint no_account )
{
    GtkTreeIter *iter;

    iter = cherche_iter_operation ( transaction,
				    no_account );

    return ( my_atoi ( gtk_tree_model_get_string_from_iter (  GTK_TREE_MODEL ( gsb_account_get_store ( no_account )),
							      iter )));
}
/******************************************************************************/



/**
 * Find column number for the P_R cell.  Usefull because in some
 * occasions, we need to know where is this column and it can be
 * changed in preferences.
 *
 * \return column number for the P_R cell.
 */
gint find_p_r_col ()
{
    gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j< TRANSACTION_LIST_COL_NB; j++ )
	{
	    if ( tab_affichage_ope[i][j] == TRANSACTION_LIST_MARK )
		return j;
	}
    }

    return -1;
}


/**
 * Find line number in a transaction for the P_R cell.  Usefull because in some
 * occasions, we need to know where is this column and it can be
 * changed in preferences.
 *
 * \return column line for the P_R cell.
 */
gint find_p_r_line ()
{
    gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( tab_affichage_ope[i][j] == TRANSACTION_LIST_MARK )
		return i;
	}
    }

    return -1;
}




/** 
 * Called to edit a transaction, put it in the form unsensitive some
 * form part if necessary (for breakdowns for example)
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_edit_current_transaction ( void )
{
    struct structure_operation *transaction;
    gchar *char_tmp;
    gint i, j;
    struct organisation_formulaire *form_organization;
    GtkWidget *menu;
    gint focus_to;

    transaction = gsb_account_get_current_transaction (gsb_account_get_current_account ());

    formulaire_a_zero ();
    degrise_formulaire_operations ();
    gtk_expander_set_expanded ( GTK_EXPANDER ( frame_droite_bas ), TRUE );

    /* if the transaction is the white line, we set the date and go away */
    
    if ( transaction == GINT_TO_POINTER ( -1 ) )
    {
	GtkWidget *entree_date;

	entree_date = widget_formulaire_par_element (TRANSACTION_FORM_DATE);

	if ( gtk_widget_get_style ( entree_date ) == style_entree_formulaire[ENGRIS] )
	    clique_champ_formulaire ( entree_date,
				      NULL,
				      GINT_TO_POINTER ( TRANSACTION_FORM_DATE ) );

	gtk_entry_select_region ( GTK_ENTRY ( entree_date ), 0, -1);
	gtk_widget_grab_focus ( GTK_WIDGET ( entree_date ) );
	return FALSE;
    }

    /* if the transaction is a breakdown, we unsensitive the date and party
     * and we do as if it was a modification of transaction */

    if ( transaction -> no_operation_ventilee_associee )
    {
	gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_DATE),
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY),
				   FALSE );
    }
    

    /*   l'opé n'est pas -1, c'est une modif, on remplit les champs */

    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "adr_struct_ope",
			  transaction );

    /*     on fait le tour du formulaire en ne remplissant que ce qui est nécessaire */

    form_organization = renvoie_organisation_formulaire ();

    for ( i=0 ; i < form_organization -> nb_lignes ; i++ )
	for ( j=0 ; j <  form_organization -> nb_colonnes ; j++ )
	{
	    GtkWidget *widget;

	    widget =  widget_formulaire_par_element ( form_organization -> tab_remplissage_formulaire[i][j] );

	    switch ( form_organization -> tab_remplissage_formulaire[i][j] )
	    {
		case TRANSACTION_FORM_OP_NB:

		    gtk_label_set_text ( GTK_LABEL ( widget ),
					 itoa ( gsb_transaction_data_get_transaction_number (transaction) ));
		    break;

		case TRANSACTION_FORM_DATE:

		    entree_prend_focus ( widget );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 renvoie_date_formatee ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (transaction))) );
		    break;

		case TRANSACTION_FORM_VALUE_DATE:

		    if ( gsb_transaction_data_get_value_date (gsb_transaction_data_get_transaction_number (transaction)))
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     renvoie_date_formatee ( gsb_transaction_data_get_value_date (gsb_transaction_data_get_transaction_number (transaction))) );
		    }
		    break;

		case TRANSACTION_FORM_EXERCICE:

		    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget ),
						   cherche_no_menu_exercice ( transaction -> no_exercice,
									      widget ));

		    /* 		    si l'opé est ventilée, on désensitive l'exo */

		    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
			gtk_widget_set_sensitive ( widget,
						   FALSE );

		    break;

		case TRANSACTION_FORM_PARTY:

		    if ( gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (transaction )))
		    {
			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						tiers_name_by_no ( gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (transaction )), TRUE ));
		    }
		    break;

		case TRANSACTION_FORM_DEBIT:

		    if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))< 0 )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       -gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))));
		    }

		    /* 		    si l'opé est relevée, on ne peut modifier le montant */

		    if ( transaction -> pointe == 3 )
			gtk_widget_set_sensitive ( widget,
						   FALSE );

		    break;

		case TRANSACTION_FORM_CREDIT:

		    if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))>= 0 )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))));
		    }

		    /* 		    si l'opé est relevée, on ne peut modifier le montant */

		    if ( transaction -> pointe == 3 )
			gtk_widget_set_sensitive ( widget,
						   FALSE );

		    break;

		case TRANSACTION_FORM_CATEGORY:

		    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
		    {
			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Breakdown of transaction") );
		    }
		    else
		    {
			if ( transaction -> relation_no_operation )
			{
			    /* c'est un virement */

			    entree_prend_focus ( widget );

			    if ( transaction -> relation_no_operation != -1 )
			    {
				struct structure_operation *contra_transaction;

				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							g_strconcat ( COLON(_("Transfer")),
								      gsb_account_get_name (transaction -> relation_no_compte),
								      NULL ));

				/* récupération de la contre opération */

				contra_transaction = operation_par_no ( transaction -> relation_no_operation,
								      transaction -> relation_no_compte );

				/* 	  si la contre opération est relevée, on désensitive les categ et les montants */

				if ( contra_transaction
				     &&
				     contra_transaction -> pointe == 3 )
				{
				    gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT),
							       FALSE );
				    gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT),
							       FALSE );
				    gtk_widget_set_sensitive ( widget,
							       FALSE );
				}
			    }
			    else
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							_("Transfer") );

			    /* si l'opération est relevée, on empêche le changement de virement */

			    if ( transaction -> pointe == 3 )
				gtk_widget_set_sensitive ( widget,
							   FALSE );
			}
			else
			{
			    /* c'est des catégories normales */

			    char_tmp = nom_categ_par_no ( gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (transaction )),
							  gsb_transaction_data_get_sub_category_number ( gsb_transaction_data_get_transaction_number (transaction )));
			    if ( char_tmp )
			    {
				entree_prend_focus ( widget );
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							char_tmp );
			    }
			}
		    }
		    break;

		case TRANSACTION_FORM_BUDGET:

		    char_tmp = nom_imputation_par_no ( transaction -> imputation,
						       transaction -> sous_imputation );
		    if ( char_tmp )
		    {
			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						char_tmp );
		    }

		    /* 		    si l'opé est ventilée, on dÃ©sensitive l'ib */

		    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
			gtk_widget_set_sensitive ( widget,
						   FALSE );
		    break;

		case TRANSACTION_FORM_NOTES:

		    if ( gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (transaction )))
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (transaction )));
		    }
		    break;

		case TRANSACTION_FORM_TYPE:

		    if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))< 0 )
			menu = creation_menu_types ( 1, gsb_account_get_current_account (), 0  );
		    else
			menu = creation_menu_types ( 2, gsb_account_get_current_account (), 0  );

		    if ( menu )
		    {
			/* on met en place les types et se place sur celui correspondant à l'opé */

			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						   menu );
			gtk_widget_show ( widget );

			place_type_formulaire ( transaction -> type_ope,
						TRANSACTION_FORM_TYPE,
						transaction -> contenu_type );
		    }
		    else
		    {
			gtk_widget_hide ( widget );
			gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
		    }

		    break;

		case TRANSACTION_FORM_DEVISE:

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						  g_slist_index ( liste_struct_devises,
								  devise_par_no ( gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction )))));
		    verification_bouton_change_devise ();
		    break;

		case TRANSACTION_FORM_BANK:

		    if ( transaction -> info_banque_guichet )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     transaction -> info_banque_guichet );
		    }
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    if ( transaction -> no_piece_comptable )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     transaction -> no_piece_comptable );
		    }
		    break;

		case TRANSACTION_FORM_CONTRA:

		    if ( transaction -> relation_no_operation )
		    {
			if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))< 0 )
			    menu = creation_menu_types ( 2, transaction -> relation_no_compte, 0  );
			else
			    menu = creation_menu_types ( 1, transaction -> relation_no_compte, 0  );

			if ( menu )
			{
			    struct structure_operation *contra_transaction;

			    /* on met en place les types et se place sur celui correspondant à l'opé */

			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						       menu );
			    gtk_widget_show ( widget );

			    contra_transaction = operation_par_no ( transaction -> relation_no_operation,
								  transaction -> relation_no_compte );
			    if ( contra_transaction )
				place_type_formulaire ( contra_transaction -> type_ope,
							TRANSACTION_FORM_CONTRA,
							NULL );
			}
			else
			    gtk_widget_hide ( widget );
		    }
		    break;

		case TRANSACTION_FORM_MODE:

		    if ( transaction -> auto_man )
			gtk_label_set_text ( GTK_LABEL ( widget ),
					     _("Auto"));
		    else
			gtk_label_set_text ( GTK_LABEL ( widget ),
					     _("Manual"));
		    break;
	    }
	}


    /* the form is full, if it's not a breakdown, we give the focus to the date
     * else, we give the focus to the first free form element */

    if ( transaction -> no_operation_ventilee_associee )
	focus_to = recherche_element_suivant_formulaire ( TRANSACTION_FORM_DATE,
							  1 );
    else
	focus_to = TRANSACTION_FORM_DATE;

    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_par_element (focus_to) ),
			      0,
			      -1);
    gtk_widget_grab_focus ( widget_formulaire_par_element (focus_to) );

    return FALSE;
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
    struct structure_operation *transaction;
    GtkTreeIter *iter;
    gint col;
    GtkTreeModel *model;

    col = find_p_r_col ();
    if ( col == -1 )
	return;

    transaction = gsb_account_get_current_transaction (gsb_account_get_current_account ());

    /*     take the model of the account */

    model = gsb_account_get_store ( gsb_account_get_current_account () );

    /* si on est sur l'opération vide -> on se barre */

    if (transaction  == GINT_TO_POINTER ( -1 )
	||
	transaction -> pointe == 3 )
	return;

    iter = cherche_iter_operation ( gsb_account_get_current_transaction (gsb_account_get_current_account ()),
				    gsb_account_get_current_account () );

    if ( transaction -> pointe )
    {
	montant = gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees - montant;

	gsb_account_set_marked_balance ( gsb_account_get_current_account (),
					 gsb_account_get_marked_balance (gsb_account_get_current_account ()) - montant );
	transaction -> pointe = 0;

	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     col, NULL,
			     -1 );
    }
    else
    {
	montant = gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees + montant;

	gsb_account_set_marked_balance ( gsb_account_get_current_account (),
					 gsb_account_get_marked_balance (gsb_account_get_current_account ()) + montant );
	transaction -> pointe = 1;

	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     col, _("P"),
			     -1 );
    }

    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
    {
	/* p_tab est déjà pointé sur le compte courant */

	GSList *liste_tmp;

	liste_tmp = gsb_account_get_transactions_list (gsb_account_get_current_account ());

	while ( liste_tmp )
	{
	    struct structure_operation *ope_fille;

	    ope_fille = liste_tmp -> data;

	    if ( ope_fille -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction) )
		ope_fille -> pointe = transaction -> pointe;

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


    /*     met à jour les labels des soldes  */

    mise_a_jour_labels_soldes ();

    modification_fichier( TRUE );
    /* ALAIN-FIXME : solution batarde me semble-t'il pour actualiser le solde pointé
       sur la fenêtre d'accueil après que l'on ait pointé l'opération */

    mise_a_jour_liste_comptes_accueil = 1;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction r_press */
/* appelÃ©e lorsque la touche r est pressée sur la liste */
/* relève ou dérelève l'opération courante */
/******************************************************************************/
void r_press (void)
{
    struct structure_operation *transaction;
    GtkTreeIter *iter;
    gint col;
    GtkTreeModel *model;

    col = find_p_r_col ();
    if ( col == -1 )
	return;

    transaction = gsb_account_get_current_transaction (gsb_account_get_current_account ());

    /* si on est sur l'opération vide -> on se barre */

    if ( transaction == GINT_TO_POINTER ( -1 ))
	return;

    model = gsb_account_get_store ( gsb_account_get_current_account () );


    iter = cherche_iter_operation ( gsb_account_get_current_transaction (gsb_account_get_current_account ()),
				    gsb_account_get_current_account () );

    if ( !transaction -> pointe )
    {
	/* on relève l'opération */

	transaction -> pointe = 3;

	/* on met soit le R, soit on change la sélection vers l'opé suivante */

	if ( gsb_account_get_r (gsb_account_get_current_account ()) )
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 col, _("R"),
				 -1 );
	else
	{
	    /* we don't want to see the R, so the transaction will diseapear */

	    struct structure_operation *next_transaction;

	    next_transaction = gsb_transactions_list_get_transaction_next ( transaction );
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_VISIBLE, FALSE,
				 -1 );
	    if ( next_transaction )
		gsb_account_set_current_transaction ( gsb_account_get_current_account (),
						      next_transaction );
	    gsb_transactions_list_set_background_color (gsb_account_get_current_account ());
	    gsb_transactions_list_set_transactions_balances (gsb_account_get_current_account ());
	}

	modification_fichier( TRUE );
    }
    else
	if ( transaction -> pointe == 3 )
	{
	    /* dé-relève l'opération */

	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 col, NULL,
				 -1 );

	    modification_fichier( TRUE );
	}

    /* si c'est une ventilation, on fait le tour des opérations du compte pour */
    /* rechercher les opérations de ventilation associées à cette ventilation */

    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
    {
	GSList *liste_tmp;

	liste_tmp = gsb_account_get_transactions_list (gsb_account_get_current_account ());

	while ( liste_tmp )
	{
	    struct structure_operation *transaction;

	    transaction = liste_tmp -> data;

	    if ( transaction -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction))
		transaction -> pointe = transaction -> pointe;

	    liste_tmp = liste_tmp -> next;
	}
    }
}
/******************************************************************************/

/**
 * delete a transaction
 * if it's a transfer, delete also the contra-transaction
 * if it's a breakdown, delete the childs
 * 
 * \param transaction The transaction to delete
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_delete_transaction ( struct structure_operation *transaction )
{
    GSList *transactions_list;
    struct structure_operation *transactions_tmp;
    gdouble montant;

    /* vérifications de bases */

    if ( !transaction
	 ||
	 transaction == GINT_TO_POINTER ( -1 ) )
	return FALSE;

    if ( DEBUG )
	printf ( "gsb_transactions_list_delete_transaction no %d\n",
		 gsb_transaction_data_get_transaction_number (transaction) );

    /* check if the transaction is not reconciled */

    if ( gsb_transactions_list_check_mark ( transaction ))
    {
	dialogue_error ( _("Impossible to delete a reconciled transaction.\nThe transaction, the contra-transaction or the children if it is a breakdown are reconciled. You can remove the reconciliation with Ctrl R if it is really necessary.") );
	return FALSE;
    }

    if ( !question_yes_no_hint ( _("Delete a transaction"),
				 g_strdup_printf ( _("Do you really want to delete the transaction whith the party '%s' ?"),
						  tiers_name_by_no ( gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (transaction )),
								     FALSE ))))
	return FALSE;

    /* find the next transaction to be selected */

    if ( gsb_account_get_current_transaction (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))) == transaction )
    {
	struct structure_operation *next_transaction;

	next_transaction = transaction;

	while ( next_transaction
		&&
		( next_transaction == transaction 
		  ||
		  next_transaction -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction)))
	    next_transaction = gsb_transactions_list_get_transaction_next ( next_transaction );

	gsb_account_set_current_transaction ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)),
					      next_transaction );
    }

    /* if it's a transfer, delete the contra-transaction */

    if ( transaction -> relation_no_operation
	 &&
	 transaction -> relation_no_compte != -1 )
    {
	transactions_tmp = operation_par_no ( transaction -> relation_no_operation,
					      transaction -> relation_no_compte );

	gsb_transactions_list_delete_transaction_from_tree_view ( transactions_tmp );
	gsb_transactions_list_delete_transaction_from_slist ( transactions_tmp );
    }

    /* if it's a breakdown, delete all the childs */

    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
    {
	transactions_list = gsb_account_get_transactions_list (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

	while ( transactions_list )
	{
	    transactions_tmp = transactions_list -> data;

	    if ( transactions_tmp -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction) )
	    {
		/* on se place tout de suite sur l'opé suivante */

		transactions_list = transactions_list -> next;

		if ( transactions_tmp -> relation_no_operation )
		{
		    /* the breakdown is a transfer, delete the contra-transaction */

		    struct structure_operation *contra_transaction;

		    contra_transaction = operation_par_no ( transactions_tmp -> relation_no_operation,
							    transactions_tmp -> relation_no_compte );

		    gsb_transactions_list_delete_transaction_from_tree_view ( contra_transaction );
		    gsb_transactions_list_delete_transaction_from_slist ( contra_transaction );
		}

		gsb_transactions_list_delete_transaction_from_tree_view ( transactions_tmp );
		gsb_transactions_list_delete_transaction_from_slist ( transactions_tmp );
	    }
	    else
		transactions_list = transactions_list -> next;
	}

	/* we have also to destroy the white line of the breakdown */

	gsb_transactions_list_delete_transaction_from_tree_view ( gsb_transactions_list_find_white_breakdown ( transaction ));
    }

    /* now we delete the original transaction */

    gsb_transactions_list_delete_transaction_from_tree_view ( transaction );
    gsb_transactions_list_delete_transaction_from_slist ( transaction );

    gsb_transactions_list_set_background_color (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));
    gsb_transactions_list_set_transactions_balances (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

    /*     calcul des nouveaux soldes */

    montant = gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

    gsb_account_set_current_balance ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)),
				      gsb_account_get_current_balance (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))) - montant );

    if ( transaction -> pointe )
	gsb_account_set_marked_balance ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)),
					 gsb_account_get_marked_balance (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))) - montant );

    /*     on met à jour les labels de solde */

    mise_a_jour_labels_soldes ();

    /* si on est en train d'équilibrer => recalcule le total pointé */

    if ( etat.equilibrage )
	calcule_total_pointe_compte ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

    /* on réaffiche la liste de l'état des comptes de l'accueil */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    affiche_dialogue_soldes_minimaux ();


    /* FIXME : on devrait réafficher les listes de tiers, categ, ib... */

    modification_fichier( TRUE );
    return FALSE;
}
/******************************************************************************/



/**
 * Check if the transaction (or linked transactions) is not marked as Reconciled
 *
 * \param transaction
 *
 * \return TRUE : it's marked R ; FALSE : it's normal, P or T transaction
 * */
gboolean gsb_transactions_list_check_mark ( struct structure_operation *transaction )
{
    GSList *transactions_list;
    struct structure_operation *transactions_tmp;

    /* vérifications de bases */

    if ( !transaction )
	return FALSE;

    if ( transaction -> pointe == OPERATION_RAPPROCHEE )
	return TRUE;

    /* if it's a transfer, check the contra-transaction */

    if ( transaction -> relation_no_operation
	 &&
	 transaction -> relation_no_compte != -1 )
    {
	transactions_tmp = operation_par_no ( transaction -> relation_no_operation,
					      transaction -> relation_no_compte );

	if ( transactions_tmp
	     &&
	     transactions_tmp -> pointe == OPERATION_RAPPROCHEE )
	    return TRUE;
    }

    /* if it's a breakdown of transaction, check all the children
     * if there is not a transfer which is marked */

    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
    {
	transactions_list = gsb_account_get_transactions_list (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

	while ( transactions_list )
	{
	    transactions_tmp = transactions_list -> data;

	    if ( transactions_tmp -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction) )
	    {
		/* transactions_tmp is a child of transaction */

		if ( transactions_tmp -> pointe == OPERATION_RAPPROCHEE )
		    return TRUE;

		if (  transactions_tmp -> relation_no_operation )
		{
		    /* the breakdown is a transfer, we check the contra-transaction */

		    struct structure_operation *contra_transaction;

		    contra_transaction = operation_par_no ( transactions_tmp -> relation_no_operation,
							    transactions_tmp -> relation_no_compte );

		    if ( contra_transaction
			 &&
			 contra_transaction -> pointe == OPERATION_RAPPROCHEE )
			return TRUE;
		}
	    }
	    transactions_list = transactions_list -> next;
	}
    }
    return FALSE;
}


/**
 * Delete from the tree_view the transaction given in param, don't check anything,
 * just erase the transaction in the tree_view
 *
 * \param transaction
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_delete_transaction_from_tree_view ( struct structure_operation *transaction )
{
    GtkTreeIter *iter;

    if ( !transaction )
	return FALSE;

    if ( DEBUG )
	printf ( "gsb_transactions_list_delete_transaction_from_tree_view no %d\n",
		 gsb_transaction_data_get_transaction_number (transaction) );

    iter = cherche_iter_operation ( transaction,
				    gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

   if ( iter )
    {
	GtkTreeModel *model;
	gint i;

	model = gsb_account_get_store ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

	for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	{
	    gtk_list_store_remove ( GTK_LIST_STORE ( model ),
				    iter );

	    /* if we are on a breakdown child, it's only 1 line */

	    if ( transaction -> no_operation_ventilee_associee )
		i = TRANSACTION_LIST_ROWS_NB;
	}

	gtk_tree_iter_free (iter);
    }
   return FALSE;
}


/**
 * Delete from the g_slist the transaction given in param, don't check anything,
 * just erase the transaction in the tree_view
 *
 * \param transaction
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_delete_transaction_from_slist ( struct structure_operation *transaction )
{
    if ( transaction )
	gsb_account_set_transactions_list ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)),
					    g_slist_remove ( gsb_account_get_transactions_list (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))),
							     transaction ));

    return FALSE;
}





/******************************************************************************/
/* Fonction changement_taille_liste_ope					      */
/* appelée dès que la taille de la clist a changé			      */
/* pour mettre la taille des différentes colonnes			      */
/******************************************************************************/
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation )
{
    gint i;
    GSList *list_tmp;

    /*     pour éviter que le système ne s'emballe... */

    if ( allocation -> width
	 ==
	 allocation_precedente )
	return FALSE;

    allocation_precedente = allocation -> width;

    /* si la largeur est automatique, on change la largeur des colonnes */
    /* sinon, on y met les valeurs fixes */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint j;

	j = gsb_account_get_no_account ( list_tmp -> data );
	
	if ( etat.largeur_auto_colonnes )
	    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
		gtk_tree_view_column_set_fixed_width ( gsb_account_get_column ( j, i),
						       rapport_largeur_colonnes[i] * allocation_precedente / 100 );
	else
	    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
		if ( taille_largeur_colonnes[i] )
		    gtk_tree_view_column_set_fixed_width ( gsb_account_get_column ( j, i),
							   taille_largeur_colonnes[i]  );

	list_tmp = list_tmp -> next;
    }

    /* met les entrées du formulaire selon une taille proportionnelle */

    mise_a_jour_taille_formulaire ( allocation_precedente );

    update_ecran ();

    return ( FALSE );
}
/******************************************************************************/


/******************************************************************************/
/* Fonction  demande_mise_a_jour_tous_comptes */
/* met la variable update_list de tous les comptes à 1 */
/* ce qui fait que lorsqu'ils seront affichés, ils seront mis à jour avant */
/* appelle aussi verification_mise_a_jour_liste pour mettre à jour la liste courante */
/******************************************************************************/
void demande_mise_a_jour_tous_comptes ( void )
{
    GSList *list_tmp;

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	gtk_list_store_clear ( GTK_LIST_STORE ( gsb_account_get_store (i) ));
	gsb_account_set_last_transaction ( i,
					   NULL );

	list_tmp = list_tmp -> next;
    }

    demarrage_idle ();
}
/******************************************************************************/


/**
 * Pop up a menu with several actions to apply to current transaction.
 */
void popup_transaction_context_menu ( gboolean full )
{
    GtkWidget *menu, *menu_item;

    if ( gsb_account_get_current_transaction (gsb_account_get_current_account ()) == GINT_TO_POINTER(-1) )
	full = FALSE;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Edit transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_PROPERTIES,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK (gsb_transactions_list_edit_current_transaction), NULL );
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
    if ( !full || 
	 ((struct structure_operation *) gsb_account_get_current_transaction (gsb_account_get_current_account ())) -> pointe == OPERATION_RAPPROCHEE ||
	 ((struct structure_operation *) gsb_account_get_current_transaction (gsb_account_get_current_account ())) -> pointe == OPERATION_TELERAPPROCHEE )
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
	 ((struct structure_operation *) gsb_account_get_current_transaction (gsb_account_get_current_account ())) -> pointe == OPERATION_RAPPROCHEE ||
	 ((struct structure_operation *) gsb_account_get_current_transaction (gsb_account_get_current_account ())) -> pointe == OPERATION_TELERAPPROCHEE )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Add accounts submenu */
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(menu_item), 
				GTK_WIDGET(creation_option_menu_comptes(GTK_SIGNAL_FUNC(move_selected_operation_to_account), FALSE, FALSE)) );

    gtk_widget_show_all (menu);
    gtk_menu_popup ( GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
}


/**
 *  Check that a transaction is selected 
 *
 * \return TRUE on success, FALSE otherwise.
 */
gboolean assert_selected_transaction ()
{
    if ( gsb_account_get_current_transaction (gsb_account_get_current_account ()) == GINT_TO_POINTER(-1) )
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
    echap_formulaire();
    gsb_account_set_current_transaction ( gsb_account_get_current_account (),
					  GINT_TO_POINTER (-1) );
    gsb_transactions_list_edit_current_transaction ();

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}


/**
 * Remove selected transaction if any.
 */
void remove_transaction ()
{
    if (! assert_selected_transaction()) return;

    gsb_transactions_list_delete_transaction ( gsb_account_get_current_transaction (gsb_account_get_current_account ()) );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}


/**
 * Clone selected transaction if any.  Update user interface as well.
 */
void clone_selected_transaction ()
{
    if (! assert_selected_transaction()) return;

    update_transaction_in_trees ( gsb_transactions_list_clone_transaction ( gsb_account_get_current_transaction (gsb_account_get_current_account ()) ) );

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();

    modification_fichier ( TRUE );
}


/**
 * Clone transaction.  If it is a breakdown or a transfer, perform all
 * needed operations, like cloning associated transactions as well.
 *
 * \param transaction Initial transaction to clone
 *
 * \return A newly created transaction.
 */
struct structure_operation *gsb_transactions_list_clone_transaction ( struct structure_operation *transaction )
{
    struct structure_operation * new_transaction, * ope_ventilee;

    new_transaction = (struct structure_operation *) malloc ( sizeof(struct structure_operation) );
    if ( !new_transaction )
    {
	dialogue_error_memory ();
	return(FALSE);
    }

    memcpy(new_transaction, transaction, sizeof(struct structure_operation) );

    gsb_transaction_data_set_transaction_number  (new_transaction,
						  0);
    new_transaction -> no_rapprochement = 0;

    if ( transaction -> pointe == OPERATION_RAPPROCHEE ||
	 transaction -> pointe == OPERATION_TELERAPPROCHEE )
    {
	new_transaction -> pointe = OPERATION_NORMALE;
    }

    gsb_transactions_append_transaction ( new_transaction,
					  gsb_transaction_data_get_transaction_number (new_transaction));

    if ( new_transaction -> relation_no_operation )
    {
	gsb_form_validate_transfer ( new_transaction, 1, gsb_account_get_name (new_transaction -> relation_no_compte) );
    }

    gsb_transactions_list_append_new_transaction ( new_transaction );

    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
    {
	GSList *liste_tmp;

	liste_tmp = gsb_account_get_transactions_list (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

	while ( liste_tmp )
	{
	    struct structure_operation *operation_2;

	    operation_2 = liste_tmp -> data;

	    if ( operation_2 -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction) )
	    {
		ope_ventilee = gsb_transactions_list_clone_transaction ( operation_2 );
		ope_ventilee -> no_operation_ventilee_associee = gsb_transaction_data_get_transaction_number (new_transaction);
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
    gint target_account, source_account;

    if (! assert_selected_transaction()) return;

    source_account = gsb_account_get_current_account ();
    target_account = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT(menu_item), 
							     "no_compte" ) );  

    if ( move_operation_to_account ( gsb_account_get_current_transaction (source_account), target_account ))
    {
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	update_transaction_in_trees ( gsb_account_get_current_transaction (source_account) );

	if ( mise_a_jour_combofix_tiers_necessaire )
	    mise_a_jour_combofix_tiers ();
	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ ();
	if ( mise_a_jour_combofix_imputation_necessaire )
	    mise_a_jour_combofix_imputation ();

	gsb_account_set_current_balance ( source_account, 
					  calcule_solde_compte ( source_account ));
	gsb_account_set_marked_balance ( source_account, 
					 calcule_solde_pointe_compte ( source_account ));

	mise_a_jour_labels_soldes ();

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

    source_account = gsb_account_get_current_account ();
    target_account = GPOINTER_TO_INT ( account );  

    if ( move_operation_to_account ( gsb_account_get_current_transaction (source_account), target_account ))
    {
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	if ( mise_a_jour_combofix_tiers_necessaire )
	    mise_a_jour_combofix_tiers ();
	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ ();
	if ( mise_a_jour_combofix_imputation_necessaire )
	    mise_a_jour_combofix_imputation ();

	update_transaction_in_trees ( gsb_account_get_current_transaction (source_account) );

	gsb_account_set_current_balance ( source_account, 
					  calcule_solde_compte ( source_account ));
	gsb_account_set_marked_balance ( source_account, 
					 calcule_solde_pointe_compte ( source_account ));

	mise_a_jour_labels_soldes ();

	modification_fichier ( TRUE );
    }
}



/**
 * Move transaction to another account
 *
 * \param transaction Transaction to move to other account
 * \param account Account to move the transaction to
 * return TRUE if ok
 */
gboolean move_operation_to_account ( struct structure_operation * transaction,
				     gint account )
{
    GtkTreeIter *iter;
    gint no_transaction;

    no_transaction = gsb_transaction_data_get_transaction_number (transaction);
    if ( transaction -> relation_no_operation )
    {
	struct structure_operation * contra_transaction;

	/* 	l'opération est un virement, si on veut la déplacer vers le compte */
	/* 	    viré, on refuse */

	if ( transaction -> relation_no_compte == account )
	{
	    dialogue_error ( _("Cannot move a transfer on his contra-account"));
	    return FALSE;
	}

	contra_transaction = operation_par_no (  transaction -> relation_no_operation,
						 transaction -> relation_no_compte );
	if ( contra_transaction )
	{
	    contra_transaction -> relation_no_compte = account;

	    /* 	    p_tab est placé par la fonction suivante, et on remet gsb_account_get_finished_balance_showed () à 1 */
	    /* 		car est remis à 0 mais on ne change pas le solde */

	    gsb_transactions_list_append_transaction ( contra_transaction,
						       gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (contra_transaction)));
	    gsb_account_set_finished_balance_showed ( gsb_transaction_data_get_account_number (no_transaction),
						      1 );
	}
    }

    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_copy ( gsb_account_get_transactions_list (gsb_transaction_data_get_account_number (no_transaction)) );

	while ( liste_tmp )
	{
	    struct structure_operation *transaction_2;

	    transaction_2 = liste_tmp -> data;

	    if ( transaction_2 -> no_operation_ventilee_associee == 
		 no_transaction )
	    {
		move_operation_to_account ( transaction_2, account );
		transaction_2 -> relation_no_compte = account;
	    }

	    liste_tmp = liste_tmp -> next;
	}

	g_slist_free ( liste_tmp );
    }

    gsb_account_set_transactions_list ( gsb_transaction_data_get_account_number (no_transaction),
					g_slist_remove ( gsb_account_get_transactions_list (gsb_transaction_data_get_account_number (no_transaction)),
							 transaction ));

    /*     si l'opération était affichée, on la retire du list_store */

    iter = cherche_iter_operation ( transaction,
				    gsb_transaction_data_get_account_number (no_transaction));

    if ( iter )
    {
	gint i;

	/* si la sélection est sur l'opération qu'on supprime, */
	/* on met la sélection sur celle du dessous */

	if ( gsb_account_get_current_transaction (gsb_transaction_data_get_account_number (no_transaction)) == transaction )
	    gsb_account_set_current_transaction ( gsb_transaction_data_get_account_number (no_transaction),
						  cherche_operation_from_ligne ( cherche_ligne_operation ( gsb_account_get_current_transaction (gsb_transaction_data_get_account_number (no_transaction)),
													   gsb_transaction_data_get_account_number (no_transaction)) + gsb_account_get_nb_rows ( gsb_transaction_data_get_account_number (no_transaction)),
										 gsb_transaction_data_get_account_number (no_transaction)) );

	for ( i=0 ; i<gsb_account_get_nb_rows ( gsb_transaction_data_get_account_number (no_transaction)) ; i++ )
	    gtk_list_store_remove ( GTK_LIST_STORE ( gsb_account_get_store (gsb_transaction_data_get_account_number (no_transaction)) ),
				    iter );

	/*     on met à jour les couleurs et les soldes */

	gsb_transactions_list_set_background_color ( gsb_transaction_data_get_account_number (no_transaction));
	gsb_transactions_list_set_transactions_balances ( gsb_transaction_data_get_account_number (no_transaction));
    }

    gsb_transaction_data_set_account_number ( no_transaction,
					      account );

    /*     comme l'opé contient déjà un no d'opération, on doit d'abord l'ajouter manuellement */
    /* 	à la liste avant d'appeler gsb_transactions_list_append_transaction */

    gsb_account_set_transactions_list ( account,
					g_slist_append ( gsb_account_get_transactions_list (account),
							 transaction ));
    gsb_transactions_append_transaction ( transaction,
					  gsb_transaction_data_get_transaction_number (transaction) );
    gsb_transactions_list_append_new_transaction ( transaction );
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

    echeance = schedule_transaction ( gsb_account_get_current_transaction (gsb_account_get_current_account ()));

    mise_a_jour_liste_echeances_auto_accueil = 1;
    remplissage_liste_echeance ();

    echeance_selectionnnee = echeance;
    formulaire_echeancier_a_zero();
    degrise_formulaire_echeancier();
    selectionne_echeance (echeance_selectionnnee);
    edition_echeance ();

    gtk_notebook_set_current_page ( GTK_NOTEBOOK(notebook_general), 2 );

    modification_fichier ( TRUE );
}



/**
 *  Convert transaction to a template of scheduled transaction.
 *
 * \param transaction Transaction to use as a template.
 */
struct operation_echeance *schedule_transaction ( struct structure_operation * transaction )
{
    struct operation_echeance *echeance;

    echeance = (struct operation_echeance *) calloc ( 1,
						      sizeof(struct operation_echeance) );
    if ( !echeance )
    {
	dialogue_error_memory ();
	return(FALSE);
    }

    echeance -> compte = gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction));
    echeance -> date = gsb_date_copy (gsb_transaction_data_get_date ( gsb_transaction_data_get_transaction_number ( transaction )));

    echeance -> montant = gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ));
    echeance -> devise = gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction ));

    echeance -> tiers = gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (transaction ));
    echeance -> categorie = gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (transaction ));
    echeance -> sous_categorie = gsb_transaction_data_get_sub_category_number ( gsb_transaction_data_get_transaction_number (transaction ));

    /*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
    /* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement ni une ventil, compte_virement = -1 */
    /*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

    if ( transaction -> relation_no_operation )
    {
	/* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

	struct structure_operation *contra_transaction;

	echeance -> compte_virement = transaction -> relation_no_compte;

	contra_transaction = operation_par_no ( transaction -> relation_no_operation,
					      echeance -> compte_virement );
	if ( contra_transaction )
	    echeance -> type_contre_ope = contra_transaction -> type_ope;
    }
    else
	if ( !echeance -> categorie
	     &&
	     !gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
	    echeance -> compte_virement = -1;

    echeance -> notes = g_strdup ( gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (transaction )));
    echeance -> type_ope = transaction -> type_ope;
    echeance -> contenu_type = g_strdup ( transaction -> contenu_type );


    echeance -> no_exercice = transaction -> no_exercice;
    echeance -> imputation = transaction -> imputation;
    echeance -> sous_imputation = transaction -> sous_imputation;

    echeance -> operation_ventilee = gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction ));

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

	liste_tmp = gsb_account_get_transactions_list (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

	while ( liste_tmp )
	{
	    struct structure_operation *transaction_de_ventil;

	    transaction_de_ventil = liste_tmp -> data;

	    if ( transaction_de_ventil -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction) )
	    {
		struct operation_echeance *echeance_de_ventil;

		echeance_de_ventil = calloc ( 1,
					      sizeof ( struct operation_echeance));

		if ( !echeance_de_ventil )
		{
		    dialogue_error_memory ();
		    return(FALSE);
		}

		echeance_de_ventil -> compte = gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_de_ventil));
		echeance_de_ventil -> date = gsb_date_copy (gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (transaction_de_ventil)));

		echeance_de_ventil -> montant = gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction_de_ventil ));
		echeance_de_ventil -> devise = gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (transaction_de_ventil ));

		echeance_de_ventil -> tiers = gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (transaction_de_ventil ));
		echeance_de_ventil -> categorie = gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (transaction_de_ventil ));
		echeance_de_ventil -> sous_categorie = gsb_transaction_data_get_sub_category_number ( gsb_transaction_data_get_transaction_number (transaction_de_ventil ));

		/*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
		/* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement, compte_virement = -1 */
		/*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

		if ( transaction_de_ventil -> relation_no_operation )
		{
		    /* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

		    struct structure_operation *contra_transaction;

		    echeance_de_ventil -> compte_virement = transaction_de_ventil -> relation_no_compte;

		    contra_transaction = operation_par_no ( transaction_de_ventil -> relation_no_operation,
							  echeance_de_ventil -> compte_virement );

		    if ( contra_transaction )
			echeance_de_ventil -> type_contre_ope = contra_transaction -> type_ope;
		}
		else
		    if ( !echeance_de_ventil -> categorie )
			echeance_de_ventil -> compte_virement = -1;

		echeance_de_ventil -> notes = g_strdup ( gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (transaction_de_ventil )));
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
gboolean affichage_traits_liste_operation ( void )
{

    GdkWindow *fenetre;
    gint i;
    gint largeur, hauteur;
    gint x, y;
    GtkAdjustment *adjustment;
    gint derniere_ligne;

    /*  FIXME   sachant qu'on appelle ça à chaque expose-event, cad très souvent ( dès que la souris passe dessus ), */
    /*     ça peut ralentir bcp... à vérifier  */

    fenetre = gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( gsb_account_get_tree_view (gsb_account_get_current_account ()) ));

    gdk_drawable_get_size ( GDK_DRAWABLE ( fenetre ),
			    &largeur,
			    &hauteur );
    
    if ( !gc_separateur_operation )
	gc_separateur_operation = gdk_gc_new ( GDK_DRAWABLE ( fenetre ));

    /*     si la hauteur des lignes n'est pas encore calculée, on le fait ici */

    hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( gsb_account_get_tree_view (gsb_account_get_current_account ()) );

    /*     on commence par calculer la dernière ligne en pixel correspondant à la dernière opé de la liste */
    /* 	pour éviter de dessiner les traits en dessous */

    derniere_ligne = hauteur_ligne_liste_opes * GTK_LIST_STORE ( gsb_account_get_store ( gsb_account_get_current_account () )) -> length;
    hauteur = MIN ( derniere_ligne,
		    hauteur );

    /*     le plus facile en premier... les lignes verticales */
    /*     dépend de si on est en train de ventiler ou non */
    /*     on en profite pour ajuster nb_ligne_ope_tree_view */

    x=0;

    for ( i=0 ; i<TRANSACTION_LIST_COL_NB ; i++ )
    {
	x = x + gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( gsb_account_get_current_account (), i)));
	gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			gc_separateur_operation,
			x, 0,
			x, hauteur );
    }

    /*     les lignes horizontales : il faut calculer la position y de chaque changement d'opé à l'écran */
    /*     on calcule la position y de la 1ère ligne à afficher */

    if ( hauteur_ligne_liste_opes )
    {
	adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_account_get_tree_view (gsb_account_get_current_account ()) ));

	y = ( hauteur_ligne_liste_opes * gsb_account_get_nb_rows ( gsb_account_get_current_account () ) ) * ( ceil ( adjustment->value / (hauteur_ligne_liste_opes* gsb_account_get_nb_rows ( gsb_account_get_current_account () )) )) - adjustment -> value;

	do
	{
	    gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			    gc_separateur_operation,
			    0, y, 
			    largeur, y );
	    y = y + hauteur_ligne_liste_opes*gsb_account_get_nb_rows ( gsb_account_get_current_account () );
	}
	while ( y < ( adjustment -> page_size )
		&&
		y <= derniere_ligne );
    }

    return FALSE;
}
/******************************************************************************/



/** called when press a button on the title column
 * if it's a right click, we show a menu to choose with what
 * we will sort
 * */
gboolean gsb_transactions_list_title_column_button_press ( GtkWidget *button,
							   GdkEventButton *ev,
							   gint *no_column )
{
    GtkWidget *menu, *menu_item;
    gint i;
    gint active_sort;

    switch ( ev -> button )
    {
	case 3:
	    /* we press the right button, show the popup */

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


	    active_sort = gsb_account_get_column_sort ( gsb_account_get_current_account (),
							GPOINTER_TO_INT ( no_column ));

	    /*     get the name of the labels of the columns and put them in a menu */

	    for ( i=0 ; i<4 ; i++ )
	    {
		gchar *temp;

		switch ( tab_affichage_ope[i][GPOINTER_TO_INT (no_column)] )
		{
		    case 0:
			temp = NULL;
			break;

		    default:
			temp = g_slist_nth_data ( liste_labels_titres_colonnes_liste_ope,
						  tab_affichage_ope[i][GPOINTER_TO_INT (no_column)] - 1 );
		}

		if ( temp
		     &&
		     strcmp ( temp,
			      N_("Balance")))
		{
		    if ( i )
			menu_item = gtk_radio_menu_item_new_with_label_from_widget ( GTK_RADIO_MENU_ITEM ( menu_item ),
										     temp );
		    else
			menu_item = gtk_radio_menu_item_new_with_label ( NULL,
									 temp );

		    if ( tab_affichage_ope[i][GPOINTER_TO_INT (no_column)] == active_sort )
			gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( menu_item ),
							 TRUE );

		    g_object_set_data ( G_OBJECT ( menu_item ),
					"no_sort",
					GINT_TO_POINTER (tab_affichage_ope[i][GPOINTER_TO_INT (no_column)]));
		    g_signal_connect ( G_OBJECT(menu_item),
				       "activate",
				       G_CALLBACK ( gsb_transactions_list_change_sort_type ),
				       no_column );
		    gtk_menu_append ( menu,
				      menu_item );
		    gtk_widget_show ( menu_item );
		}

	    }

	    gtk_menu_popup ( GTK_MENU(menu),
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     3,
			     gtk_get_current_event_time());
	    gtk_widget_show (menu);
	    
	    break;
    }

    return FALSE;
}
/******************************************************************************/


/** called when we click on the new sort type
 * \param menu_item The GtkMenuItem
 * \param no_column
 * \return FALSE
 * */
gboolean gsb_transactions_list_change_sort_type ( GtkWidget *menu_item,
						  gint *no_column )
{
    gint sort_column_id;
    GtkSortType order;
    GtkTreeSortable *sortable;
    
    if ( !gtk_check_menu_item_get_active ( GTK_CHECK_MENU_ITEM ( menu_item )))
	return FALSE;

    gsb_account_set_column_sort ( gsb_account_get_current_account (),
				  GPOINTER_TO_INT ( no_column ),
				  GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( menu_item),
									"no_sort" )));

    sortable = GTK_TREE_SORTABLE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view ( gsb_account_get_current_account () ))));


    gtk_tree_sortable_get_sort_column_id ( sortable,
					   &sort_column_id,
					   &order );

    gtk_tree_sortable_set_sort_column_id ( sortable,
					   GPOINTER_TO_INT ( no_column ),
					   GTK_SORT_ASCENDING );

    /* we have to check because if we ask for a sort on another line but same column,
     * the tree will not update ; so in that case, we have to invert 2 times the order
     * */

    if ( sort_column_id == GPOINTER_TO_INT ( no_column )
	 &&
	 order == GTK_SORT_ASCENDING )
    {
	gtk_tree_sortable_set_sort_column_id ( sortable,
					       GPOINTER_TO_INT ( no_column ),
					       GTK_SORT_DESCENDING );
	gtk_tree_sortable_set_sort_column_id ( sortable,
					       GPOINTER_TO_INT ( no_column ),
					       GTK_SORT_ASCENDING );
    }
    gsb_transactions_list_sort_column_changed ( gsb_account_get_column ( gsb_account_get_current_account (),
									 GPOINTER_TO_INT (no_column)),
						GINT_TO_POINTER (gsb_account_get_current_account ()));
    return FALSE;
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction met juste les labels sous la liste des opés à jour */
/* les soldes ont dû être calculé avant */
/******************************************************************************/
void mise_a_jour_labels_soldes ( void )
{
    /*     met le solde */

    /* TODO: update to new GUI */
/*     gtk_label_set_text ( GTK_LABEL ( solde_label ), */
/* 			 g_strdup_printf ( PRESPACIFY(_("Current balance: %4.2f %s")), */
/* 					   gsb_account_get_current_balance (gsb_account_get_current_account ()), */
/* 					   devise_code_by_no ( gsb_account_get_currency (gsb_account_get_current_account ()) ))); */


    /* met le label du solde pointé */

/*     gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ), */
/* 			 g_strdup_printf ( _("Checked balance: %4.2f %s"), */
/* 					   gsb_account_get_marked_balance (gsb_account_get_current_account ()), */
/* 					   devise_code_by_no ( gsb_account_get_currency (gsb_account_get_current_account ()) ))); */
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction est appelée lorsqu'on veut changer l'état de l'affichage avec/sans R */
/* argument : le nouvel affichage */
/* en fonction de l'argument, elle retire les lignes R de la liste ou elle les ajoute */
/******************************************************************************/
void mise_a_jour_affichage_r ( gint affichage_r )
{
    GSList *list_tmp;

    /*     we check all the accounts */
    /* 	if etat.retient_affichage_par_compte is set, only gsb_account_get_current_account () will change */
    /* 	else, all the accounts change */

    if ( affichage_r == gsb_account_get_r (gsb_account_get_current_account ()) )
	return;

    if ( DEBUG )
	printf ( "mise_a_jour_affichage_r afficher : %d\n", affichage_r );

    gsb_account_set_r (gsb_account_get_current_account (),
		       affichage_r );
    gsb_account_set_finished_visible_rows ( gsb_account_get_current_account (),
					    0 );

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !etat.retient_affichage_par_compte )
	    gsb_account_set_r ( i,
				affichage_r );

	list_tmp = list_tmp -> next;
    }

    /*     and we apply the changes now */

    demarrage_idle ();

    return;
}
/******************************************************************************/


/******************************************************************************/
/* cette fonction est appelée lorsqu'on demande un autre nb de lignes */
/* à afficher : elle va ajouter ou supprimer des lignes en fonction de */
/* l'affichage précédent */
/******************************************************************************/
void mise_a_jour_affichage_lignes ( gint nb_lignes )
{
    GSList *list_tmp;

    /*     we check all the accounts */
    /* 	if etat.retient_affichage_par_compte is set, only gsb_account_get_current_account () will change */
    /* 	else, all the accounts change */


    if ( nb_lignes == gsb_account_get_nb_rows ( gsb_account_get_current_account () ) )
	return;

    if ( DEBUG )
	printf ( "mise_a_jour_affichage_lignes %d lignes\n", nb_lignes );

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !etat.retient_affichage_par_compte
	     ||
	     i == gsb_account_get_current_account () )
	{
	    gsb_account_set_nb_rows ( i, 
				      nb_lignes );
	    gsb_account_set_finished_visible_rows ( i,
						    0 );
	}
	list_tmp = list_tmp -> next;
    }

    /*     and we apply the changes now */

    demarrage_idle ();
}
/******************************************************************************/



/** check all the lines of the account and set if they are shown or not
 * \param no_account the account we want to set
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_visibles_rows_on_account ( gint no_account )
{
    GtkListStore *model;
    GtkTreeIter iter;

    if ( DEBUG )
	printf ( "gsb_transactions_list_set_visibles_rows_on_account %d\n", no_account );

    model = gsb_account_get_store (no_account);

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					 &iter ))
    {
	gint r_shown;
	gint nb_rows;

	r_shown = gsb_account_get_r ( no_account );
	nb_rows = gsb_account_get_nb_rows ( no_account );

	do
	{
	    struct structure_operation *transaction;
	    gint current_line;
	    gint show;

	    /* 	    the current_line will contain 0, 1, 2 or 3 ; the line on the transaction */
	    /* 		we are on */

	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
				 TRANSACTION_COL_NB_TRANSACTION_LINE, &current_line,
				 -1 );

	    /* 	    do with the white line */

	    if ( transaction == GINT_TO_POINTER (-1))
	    {
		if ( current_line < nb_rows )
		    show = TRUE;
		else
		    show = FALSE;

		gtk_list_store_set ( GTK_LIST_STORE ( model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, show,
				     -1 );
		continue;
	    }

	    /* 	    check first if it's R and if r is shown */

	    if ( transaction -> pointe == OPERATION_RAPPROCHEE
		 &&
		 !r_shown )
	    {
		gtk_list_store_set ( GTK_LIST_STORE ( model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, FALSE,
				     -1 );
		continue;
	    }

	    /* if it's a breakdown, we do nothing, only the user choose
	     * what is shown or not */

	    if ( transaction -> no_operation_ventilee_associee )
		continue;

	    /* 	    now we check if we show 1, 2, 3 or 4 lines */

	    /* FIXME: lors de l'affichage de plusieurs lignes, les lignes du dessous doivent forcemment être
	     * plus bas que celles du desssus, par ex
	     * sur 3 lignes, ligne 1 -> 1, ligne 2-> 2, ligne 3-> 4   => ok
	     * mais ligne 1 -> 2 et ligne 2 -> 1  => pas bon ;
	     * if faut l'interdire dans les paramètres */

	    show = FALSE;

	    switch ( nb_rows )
	    {
		case 1:
		    if ( current_line == ligne_affichage_une_ligne )
			show = TRUE;
		    break;

		case 2:
		    if ( current_line == GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data )
			 ||
			 current_line == GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data ))
			show = TRUE;
		    break;

		case 3:
		    if ( current_line == GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data )
			 ||
			 current_line == GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data )
			 ||
			 current_line == GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data ))
			show = TRUE;
		    break;

		default:
		    show = TRUE;

	    }
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 TRANSACTION_COL_NB_VISIBLE, show,
				 -1 );
	}
	while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					   &iter ));
    }

    gsb_account_set_finished_visible_rows ( no_account,
					    1 );
    gsb_account_set_finished_selection_transaction ( no_account,
						     0 );
    gsb_account_set_finished_background_color ( no_account,
						0 );
    return FALSE;
}




/** check if the transaction should be shown and do it
 * \param transaction the transaction to check
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_visibles_rows_on_transaction ( struct structure_operation *transaction )
{
    GtkListStore *model;
    GtkTreeIter *iter;
    gint r_shown;
    gint nb_rows;
    gint i;

    if ( transaction == GINT_TO_POINTER (-1))
	return FALSE;

    model = gsb_account_get_store (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));
    iter = cherche_iter_operation ( transaction,
				    gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

    if (!iter)
	return FALSE;
    
    /* begin to check with R */
    
    r_shown = gsb_account_get_r ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

    if ( !r_shown
	 &&
	 transaction -> pointe == OPERATION_RAPPROCHEE )
    {
	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     TRANSACTION_COL_NB_VISIBLE, FALSE,
			     -1 );
	gtk_tree_iter_free ( iter );
	return FALSE;
    }
    
    /* if the transaction is a breakdown, we check with the mother
     * if it sould be shown or not */

    if ( transaction -> no_operation_ventilee_associee )
    {
	GtkTreeIter *iter_2;
	gboolean mother_is_expanded;

	iter_2 = cherche_iter_operation ( operation_par_no ( transaction -> no_operation_ventilee_associee,
							     gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction))),
					  gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

	gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
			     iter_2,
			     TRANSACTION_COL_NB_IS_EXPANDED, &mother_is_expanded,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     TRANSACTION_COL_NB_VISIBLE, mother_is_expanded,
			     -1 );

	gtk_tree_iter_free ( iter_2 );
	gtk_tree_iter_free ( iter );
	return FALSE;
    }

    /* now the transaction have to be shown completely or some
     * lines */

    nb_rows = gsb_account_get_nb_rows ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction)));

    for ( i=0 ; i< TRANSACTION_LIST_ROWS_NB; i++ )
    {
	gint show;

	/* FIXME: lors de l'affichage de plusieurs lignes, les lignes du dessous doivent forcemment être
	 * plus bas que celles du desssus, par ex
	 * sur 3 lignes, ligne 1 -> 1, ligne 2-> 2, ligne 3-> 4   => ok
	 * mais ligne 1 -> 2 et ligne 2 -> 1  => pas bon ;
	 * if faut l'interdire dans les paramètres */

	show = FALSE;

	switch ( nb_rows )
	{
	    case 1:
		if ( i == ligne_affichage_une_ligne )
		    show = TRUE;
		break;

	    case 2:
		if ( i == GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data )
		     ||
		     i == GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data ))
		    show = TRUE;
		break;

	    case 3:
		if ( i == GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data )
		     ||
		     i == GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data )
		     ||
		     i == GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data ))
		    show = TRUE;
		break;

	    default:
		show = TRUE;

	}
	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     TRANSACTION_COL_NB_VISIBLE, show,
			     -1 );

	gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				   iter );
    }

    gtk_tree_iter_free ( iter );
    return FALSE;
}



/* get the real name of the category of the transaction
 * so return breakdown of transaction, transfer : ..., categ : under_categ
 * \param transaction the adr of the transaction
 * \return the real name
 * */
gchar *gsb_transactions_get_category_real_name ( struct structure_operation *transaction )
{
    gchar *temp;

    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (transaction )))
	temp = _("Breakdown of transaction");
    else
    {
	if ( transaction -> relation_no_operation )
	{
	    /** it's a transfer */

	    if ( transaction -> relation_no_compte == -1 )
	    {
		if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))< 0 )
		    temp = _("Transfer to a deleted account");
		else
		    temp = _("Transfer from a deleted account");
	    }
	    else
	    {
		if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (transaction ))< 0 )
		    temp = g_strdup_printf ( _("Transfer to %s"),
					     gsb_account_get_name ( transaction -> relation_no_compte) );
		else
		    temp = g_strdup_printf ( _("Transfer from %s"),
					     gsb_account_get_name ( transaction -> relation_no_compte) );
	    }
	}
	else
	    /* it's a normal category */
	    temp = nom_categ_par_no ( gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (transaction )),
				      gsb_transaction_data_get_sub_category_number ( gsb_transaction_data_get_transaction_number (transaction )));
    }

    return temp;
}


/** show or hide the breakdowns of the transaction given
 * \param no_account
 * \param transaction the mother transaction
 * \param visible TRUE if we want to show the breakdowns
 * \return FALSE
 * */
gboolean gsb_account_list_set_breakdowns_visible ( gint no_account,
						   struct structure_operation *transaction,
						   gint visible )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = gsb_account_get_store ( no_account );
    gtk_tree_model_get_iter_first ( model,
				    &iter );

    do
    {
	struct structure_operation *child_transaction;

	gtk_tree_model_get ( model,
			     &iter,
			     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &child_transaction,
			     -1 );

	if ( child_transaction != GINT_TO_POINTER (-1)
	     &&
	     child_transaction -> no_operation_ventilee_associee == gsb_transaction_data_get_transaction_number (transaction))
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 TRANSACTION_COL_NB_VISIBLE, visible,
				 TRANSACTION_COL_NB_IS_NOT_BREAKDOWN, FALSE,
				 -1 );
    }
    while ( gtk_tree_model_iter_next ( model,
				       &iter ));

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
