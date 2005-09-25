/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2005 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org) 	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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


/*START_INCLUDE*/
#include "operations_liste.h"
#include "accueil.h"
#include "utils_montants.h"
#include "utils_exercices.h"
#include "classement_echeances.h"
#include "operations_formulaire.h"
#include "barre_outils.h"
#include "type_operations.h"
#include "comptes_traitements.h"
#include "echeancier_formulaire.h"
#include "utils_devises.h"
#include "dialog.h"
#include "echeancier_liste.h"
#include "equilibrage.h"
#include "gsb_data_account.h"
#include "gsb_data_category.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "classement_operations.h"
#include "gtk_cell_renderer_expander.h"
#include "gtk_combofix.h"
#include "main.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "operations_comptes.h"
#include "traitement_variables.h"
#include "utils_ib.h"
#include "parametres.h"
#include "utils_rapprochements.h"
#include "utils_types.h"
#include "utils.h"
#include "utils_operations.h"
#include "utils_str.h"
#include "structures.h"
#include "operations_liste.h"
#include "include.h"
#include "mouse.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean assert_selected_transaction ();
static gboolean changement_taille_liste_ope ( GtkWidget *tree_view, GtkAllocation *allocation );
static gpointer cherche_operation_from_ligne ( gint ligne,
					gint no_account );
static void creation_titres_tree_view ( void );
static gint find_balance_col ( void );
static gint find_balance_line ( void );
static gint find_p_r_line ();
static gboolean gsb_gui_change_cell_content ( GtkWidget * item, gint number );
static GtkWidget * gsb_gui_create_cell_contents_menu ( int x, int y );
static gboolean gsb_gui_update_row_foreach ( GtkTreeModel *model, GtkTreePath *path,
				      GtkTreeIter *iter, gint coords[2] );
static gboolean gsb_transactions_list_change_sort_type ( GtkWidget *menu_item,
						  gint *no_column );
static gboolean gsb_transactions_list_check_mark ( gpointer transaction );
static gboolean gsb_transactions_list_columns_changed ( GtkTreeView *treeview, gpointer user_data );
static GtkListStore *gsb_transactions_list_create_store ( void );
static GtkWidget *gsb_transactions_list_create_tree_view ( GtkTreeModel *model );
static void gsb_transactions_list_create_tree_view_columns ( void );
static gboolean gsb_transactions_list_current_transaction_down ( gint no_account );
static gboolean gsb_transactions_list_current_transaction_up ( gint no_account );
static gboolean gsb_transactions_list_delete_transaction_from_tree_view ( gpointer transaction );
static gboolean gsb_transactions_list_fill_store ( GtkListStore *store );
static gpointer gsb_transactions_list_find_white_breakdown ( gpointer *breakdown_mother );
static GtkTreeModel *gsb_transactions_list_get_filter (void);
static GtkTreePath *gsb_transactions_list_get_list_path_from_sorted_path ( GtkTreePath *path_sorted );
static GtkTreePath *gsb_transactions_list_get_path_from_transaction ( gpointer *transaction );
static GtkTreeModel *gsb_transactions_list_get_sortable (void);
static GtkTreePath *gsb_transactions_list_get_sorted_path_from_list_path ( GtkTreePath *path,
								    gint no_account );
static gint gsb_transactions_list_get_transaction_from_path ( GtkTreePath *path );
static gint gsb_transactions_list_get_transaction_next ( gint transaction_number,
						  gint mother_transaction_number );
static gchar *gsb_transactions_list_grep_cell_content ( gint transaction_number,
						 gint cell_content_number );
static void gsb_transactions_list_set_filter (GtkTreeModel *filter_model);
static GtkTreeModel *gsb_transactions_list_set_filter_store ( GtkListStore *store );
static void gsb_transactions_list_set_sortable (GtkTreeModel *sortable_model);
static GtkTreeModel *gsb_transactions_list_set_sorting_store ( GtkTreeModel *filter_model );
static void gsb_transactions_list_set_store (GtkListStore *store);
static void gsb_transactions_list_set_tree_view (GtkWidget *tree_view);
static gboolean gsb_transactions_list_sort_column_changed ( GtkTreeViewColumn *tree_view_column );
static gboolean gsb_transactions_list_title_column_button_press ( GtkWidget *button,
							   GdkEventButton *ev,
							   gint *no_column );
static gboolean move_operation_to_account ( gint transaction_number,
				     gint target_account );
static void move_selected_operation_to_account ( GtkMenuItem * menu_item );
static void p_press (void);
static void popup_transaction_context_menu ( gboolean full, int x, int y );
static void r_press (void);
static struct operation_echeance *schedule_transaction ( gpointer * transaction );
static gdouble solde_debut_affichage ( gint no_account );
static void update_titres_tree_view ( void );
/*END_STATIC*/



/*  adr du notebook qui contient les opés de chaque compte */
GtkWidget *tree_view_vbox;

/* the columns of the tree_view */
GtkTreeViewColumn *transactions_tree_view_columns[TRANSACTION_LIST_COL_NB];

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
GtkWidget *solde_label = NULL;

/*  pointeur vers le label qui contient le solde pointe sous la liste des opé */
GtkWidget *solde_label_pointe = NULL;

GtkWidget *transactions_tree_view = NULL;
GtkListStore *transactions_store = NULL;
GtkTreeModel *transactions_filter_model = NULL;
GtkTreeModel *transactions_sortable_model = NULL;


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
extern gchar *labels_boutons [] ;
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
extern GtkTreeSelection * selection;
extern gdouble solde_final;
extern gdouble solde_initial;
extern GtkStyle *style_entree_formulaire[2];
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
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

    /* tree_view_vbox will contain the tree_view, we will see later to set it directly */

    tree_view_vbox = gtk_vbox_new ( FALSE,
				    0 );
    /*     tree_view_vbox = creation_tree_view_operations (); */
    gtk_box_pack_start ( GTK_BOX ( win_operations ),
			 tree_view_vbox,
			 TRUE, TRUE, 0);
    gtk_widget_show ( tree_view_vbox );

    return ( win_operations );
}
/******************************************************************************/



/** create fully the gui list and fill it
 * \param
 * \return the widget which contains the list, to set at the right place
 * */
GtkWidget *gsb_transactions_list_make_gui_list ( void )
{
    GtkWidget *tree_view;
    GtkListStore *store;
    GtkTreeModel *filter_model;
    GtkTreeModel *sortable_model;
    GtkWidget *scrolled_window;

    /* we have first to create the store and fill it now to increase the speed */

    store = gsb_transactions_list_create_store ();
    gsb_transactions_list_fill_store (store);

    /* now we can set the filter and sorting the store */

    filter_model = gsb_transactions_list_set_filter_store (store);
    sortable_model = gsb_transactions_list_set_sorting_store (filter_model);

    /* we add the tree view in a scrolled window which will be returned */

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN );

    /* and now we can create the tree_view */

    tree_view = gsb_transactions_list_create_tree_view (sortable_model);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			tree_view );
    gtk_widget_show ( tree_view );

    /* we save the values */

    gsb_transactions_list_set_store (store);
    gsb_transactions_list_set_filter (filter_model);
    gsb_transactions_list_set_sortable (sortable_model);
    gsb_transactions_list_set_tree_view (tree_view);

    return scrolled_window;
}

/**
 * called after a click on a column title ; the sort of the list is automatic,
 * that function make the background color and the rest to be updated
 *
 * \param tree_view_column the tree_view_column clicked
 * \return FALSE
 * */
gboolean gsb_transactions_list_sort_column_changed ( GtkTreeViewColumn *tree_view_column )
{
    GtkSortType sort_type;
    gint sort_column_id;

    if ( DEBUG )
	printf ( "gsb_transactions_list_sort_column_changed\n" );

    /* the third click give no sort for the column
     * we forbid that here, so now, there is always a column which sort the tree view */

    gtk_tree_sortable_get_sort_column_id ( GTK_TREE_SORTABLE ( gsb_transactions_list_get_sortable()),
					   &sort_column_id,
					   &sort_type );

    if ( sort_column_id == -1 )
    {
	gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE (gsb_transactions_list_get_sortable()),
					       gtk_tree_view_column_get_sort_column_id ( tree_view_column ),
					       sort_type );
	return FALSE;
    }

    gsb_transactions_list_set_background_color ( gsb_data_account_get_current_account ());
    gsb_transactions_list_set_transactions_balances ( gsb_data_account_get_current_account ());

    modification_fichier ( TRUE );
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

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	/* 	creation_colonnes_tree_view_par_compte (i); */

	list_tmp = list_tmp -> next;
    }
}


/** 
 * creates the columns of the tree_view
 * */
void gsb_transactions_list_create_tree_view_columns ( void )
{
    gint i;
    gfloat alignment[] = {
	COLUMN_CENTER, COLUMN_CENTER, COLUMN_CENTER, COLUMN_LEFT, 
	COLUMN_CENTER, COLUMN_RIGHT, COLUMN_RIGHT, COLUMN_RIGHT
    };
    gint column_balance;

    /* get the position of the amount column to set it in red */

    column_balance = find_balance_col ();

    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	GtkCellRenderer *cell_renderer;

	/* 	if it's the column 0, we put an expander, else it's a text column */

	if ( i )
	{
	    cell_renderer = gtk_cell_renderer_text_new ();

	    transactions_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes ( _(titres_colonnes_liste_operations[i]),
											   cell_renderer,
											   "text", i,
											   "cell-background-gdk", TRANSACTION_COL_NB_BACKGROUND,
											   "font-desc", TRANSACTION_COL_NB_FONT,
											   NULL );
	}
	else
	{
	    cell_renderer = gtk_cell_renderer_expander_new ();

	    transactions_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes ( _(titres_colonnes_liste_operations[i]),
											   cell_renderer,
											   "is-expanded", TRANSACTION_COL_NB_IS_EXPANDED,
											   "is-expander", TRANSACTION_COL_NB_IS_EXPANDER,
											   "visible", TRANSACTION_COL_NB_IS_NOT_BREAKDOWN,
											   "cell-background-gdk", TRANSACTION_COL_NB_BACKGROUND,
											   NULL );
	}

	g_object_set ( G_OBJECT ( GTK_CELL_RENDERER (cell_renderer)),
		       "xalign", alignment[i],
		       NULL );

	gtk_tree_view_column_set_alignment ( transactions_tree_view_columns[i],
					     alignment[i] );
	gtk_tree_view_column_set_sizing ( transactions_tree_view_columns[i],
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_sort_column_id ( transactions_tree_view_columns[i], i );
	gtk_tree_view_column_set_expand ( transactions_tree_view_columns[i], TRUE );

	gtk_tree_view_column_set_resizable ( transactions_tree_view_columns[i], TRUE );
    }

    /*     pour la colonne du solde, on rajoute le foreground */

    if ( column_balance != -1 )
	gtk_tree_view_column_add_attribute ( transactions_tree_view_columns[column_balance],
					     gtk_tree_view_column_get_cell_renderers ( transactions_tree_view_columns[column_balance]) -> data,
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

    if ( !transactions_tree_view_columns[TRANSACTION_COL_NB_CHECK])
	creation_titres_tree_view ();


    /*     on s'occupe des listes d'opérations */

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint j;

	j = gsb_data_account_get_no_account ( list_tmp -> data );

	for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
	{
	    gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN (transactions_tree_view_columns[i]),
					     _(titres_colonnes_liste_operations[i]) );

	    if ( GTK_TREE_VIEW_COLUMN (transactions_tree_view_columns[i])->button )
	    {
		gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				       GTK_TREE_VIEW_COLUMN (transactions_tree_view_columns[i])->button,
				       tips_col_liste_operations[i],
				       tips_col_liste_operations[i] ); 
	    }
	}

	list_tmp = list_tmp -> next;
    }
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

    gtk_list_store_clear ( gsb_transactions_list_get_store()  );

    /*     plus rien n'est affiché */


    /*     on remplit le list_store */

    /*     gsb_transactions_list_fill_store ( compte, */
    /* 				       0 ); */

    /*     on met les couleurs du fond */

    gsb_transactions_list_set_background_color ( compte );

    /*     on met les soldes */

    gsb_transactions_list_set_transactions_balances ( compte );

}
/******************************************************************************/

/** create the store which contains the transactions
 * it only make the main store, no filter and no sort, the purpose
 * is to fill it just after to increase the speed, and only after
 * we can connect a filter, a sorting function and a tree_view
 * \param
 * \return the store
 * */
GtkListStore *gsb_transactions_list_create_store ( void )
{
    GtkListStore *store;

    if ( DEBUG )
	printf ( "gsb_transactions_list_create_store\n" );

    /* definitions of the columns in operations_liste.h */

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


    return store;
}


/** 
 * create a filter model from a filled store,
 * set the visible column
 * 
 * \param store the filled store
 * 
 * \return the filter model
 * */
GtkTreeModel *gsb_transactions_list_set_filter_store ( GtkListStore *store )
{
    GtkTreeModel *filter_model;

    if ( DEBUG )
	printf ( "gsb_transactions_list_set_filter_store\n" );

    filter_model = gtk_tree_model_filter_new ( GTK_TREE_MODEL ( store ),
					       NULL );
    gtk_tree_model_filter_set_visible_column ( GTK_TREE_MODEL_FILTER ( filter_model ),
					       TRANSACTION_COL_NB_VISIBLE );

    return filter_model;
}



/** 
 * create a sorting model from a filter model,
 * set the visible column
 * 
 * \param store the filled store
 * 
 * \return the filter model
 * */
GtkTreeModel *gsb_transactions_list_set_sorting_store ( GtkTreeModel *filter_model )
{
    GtkTreeModel *sortable_model;

    printf ( "gsb_transactions_list_set_sorting_store\n" );
    /* xxx à virer le no_account */
    gint no_account = 0;

    /* make the model_sort, on the model_filter */

    sortable_model = gtk_tree_model_sort_new_with_model ( filter_model );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( sortable_model ),
					   gsb_data_account_get_sort_column ( no_account ),
					   gsb_data_account_get_sort_type ( no_account ));

    /*     set the compare functions by click on the column */

    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
				      TRANSACTION_COL_NB_CHECK,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_0,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
				      TRANSACTION_COL_NB_DATE,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_1,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
				      TRANSACTION_COL_NB_PARTY,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_2,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
				      TRANSACTION_COL_NB_PR,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_3,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
				      TRANSACTION_COL_NB_DEBIT,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_4,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
				      TRANSACTION_COL_NB_CREDIT,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_5,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
				      TRANSACTION_COL_NB_BALANCE,
				      (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column_6,
				      GINT_TO_POINTER ( no_account ),
				      NULL );
    return sortable_model;
}


/** create a tree view from a filled, filtred and sorted store,
 * \param store the store
 * \return the tree_view
 * */
GtkWidget *gsb_transactions_list_create_tree_view ( GtkTreeModel *model )
{
    GtkWidget *tree_view;
    gint i;

    tree_view = gtk_tree_view_new ();

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

    /* we create the columns of the tree view */

    gsb_transactions_list_create_tree_view_columns ();

    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
				      transactions_tree_view_columns[i] );

	if ( !i )
	    gtk_tree_view_column_set_clickable ( transactions_tree_view_columns[i],
						 FALSE );

	/* 	    set the tooltips */
	gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
			       transactions_tree_view_columns[i] ->button,
			       tips_col_liste_operations[i],
			       tips_col_liste_operations[i] ); 

	g_signal_connect ( G_OBJECT ( transactions_tree_view_columns[i] -> button),
			   "button-press-event",
			   G_CALLBACK ( gsb_transactions_list_title_column_button_press ),
			   GINT_TO_POINTER (i));

	/* 	    after changing the sort of the list, we update the background color */
	g_signal_connect_after ( G_OBJECT ( transactions_tree_view_columns[i] ),
				 "clicked",
				 G_CALLBACK ( gsb_transactions_list_sort_column_changed ),
				 NULL );
    }

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view),
			      GTK_TREE_MODEL (model));

    return tree_view;
}


/** fill the new store with the all the transactions
 * normally called at the opening of a file
 * \param by_part if TRUE, stop after a max number of transactions and return like that
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_store ( GtkListStore *store )
{
    GSList *transactions_list;

    if ( DEBUG )
	printf ( "gsb_transactions_list_fill_store\n" );

    transactions_list = gsb_data_transaction_get_transactions_list ();

    /* add all the transactions */

    while (transactions_list)
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (transactions_list -> data );

	gsb_transactions_list_append_transaction ( transaction_number,
						   store );

	/* if the transaction is a breakdown mother, we happen a white line,
	 * which is a normal transaction but with nothing and with the breakdown
	 * relation to the last transaction */

	if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	    gsb_transactions_list_append_white_line (transaction_number,
						     store );

	transactions_list = transactions_list -> next;
    }

    /* the list of transactions is now complete,
     * we happen the white line */

    gsb_transactions_list_append_white_line ( 0,
					      store );

    return FALSE; 
}
/******************************************************************************/



/**
 * Create a white line and append it to the gtk list of transactions
 * if there is a mother, do as for breakdowns children, so only 1 line
 *
 * \param mother_transaction_number The mother of the white breakdown or 0 for the last white line
 * \param store
 *
 * \return FALSE; 
 * */
gboolean gsb_transactions_list_append_white_line ( gint mother_transaction_number,
						   GtkListStore *store )
{
    gint i;

    for ( i = 0 ; i < TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	GtkTreeIter iter;

	gtk_list_store_append ( store,
				&iter );

	/* draw the first column for the general white line */
	/* set the number of line in the transaction */
	/* set the address of the transaction */

	gtk_list_store_set ( store,
			     &iter,
			     TRANSACTION_COL_NB_IS_NOT_BREAKDOWN, !mother_transaction_number,
			     TRANSACTION_COL_NB_TRANSACTION_LINE, i,
			     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, gsb_data_transaction_new_white_line (mother_transaction_number),
			     -1 );

	/* if it's a breakdown, there is only 1 line */

	if ( mother_transaction_number )
	    i = TRANSACTION_LIST_ROWS_NB;
    }
    return FALSE;
}


/** append the transaction to the store of the account
 * make the 4 lines, after the filter will decide if we show them or not
 * \param transaction_number
 * \param store
 * \return FALSE
 * */
gboolean gsb_transactions_list_append_transaction ( gint transaction_number,
						    GtkListStore *store )
{
    gint i;

    for ( i = 0 ; i < TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	GtkTreeIter iter;

	gtk_list_store_append ( store,
				&iter );

	gsb_transactions_list_fill_row ( transaction_number,
					 &iter,
					 store,
					 i );

	/* if it's a breakdown, there is only 1 line */

	if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
	    i = TRANSACTION_LIST_ROWS_NB;
    }
    return FALSE;
}



/** fill the row given in param by iter
 * \param transaction_number
 * \param iter the iter corresponding to the row
 * \param store
 * \param line_in_transaction
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_row ( gint transaction_number,
					  GtkTreeIter *iter,
					  GtkListStore *store,
					  gint line_in_transaction )
{
    gint i;

    for ( i = 1 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	gtk_list_store_set ( store,
			     iter,
			     i, gsb_transactions_list_grep_cell_content ( transaction_number,
									  tab_affichage_ope[line_in_transaction][i] ),
			     -1 );
    }

    /* set the expander if necessary */

    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number)
	 &&
	 !line_in_transaction )
	gtk_list_store_set ( store,
			     iter,
			     TRANSACTION_COL_NB_IS_EXPANDER, TRUE,
			     TRANSACTION_COL_NB_IS_EXPANDED, FALSE,
			     -1 );
    else
	gtk_list_store_set ( store,
			     iter,
			     TRANSACTION_COL_NB_IS_EXPANDER, FALSE,
			     TRANSACTION_COL_NB_IS_EXPANDED, FALSE,
			     -1 );

    /* if we use a custom font... */

    if ( etat.utilise_fonte_listes )
	gtk_list_store_set ( store,
			     iter,
			     TRANSACTION_COL_NB_FONT, pango_desc_fonte_liste,
			     -1 );

    /* draw the first column for all transactions, except for children of breakdowns */
    /* set the number of line in the transaction */
    /* set the address of the transaction */

    gtk_list_store_set ( store,
			 iter,
			 TRANSACTION_COL_NB_IS_NOT_BREAKDOWN, !gsb_data_transaction_get_mother_transaction_number (transaction_number),
			 TRANSACTION_COL_NB_TRANSACTION_LINE, line_in_transaction,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, gsb_data_transaction_get_pointer_to_transaction (transaction_number),
			 -1 );

    return FALSE;
}



/** take in a transaction the content to set in a cell of the transaction's list
 * \param transaction_number
 * \param cell_content_number what we need in the transaction
 * \return the content of the transaction, in gchar
 * */
gchar *gsb_transactions_list_grep_cell_content ( gint transaction_number,
						 gint cell_content_number )
{
    gchar *temp;
    gdouble amount;

    /* if it's a breakdown and we want to see the party,
     * we show the category */

    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number)
	 &&
	 cell_content_number == TRANSACTION_LIST_PARTY )
	cell_content_number = TRANSACTION_LIST_CATEGORY;

    switch ( cell_content_number )
    {
	/* mise en forme de la date */

	case TRANSACTION_LIST_DATE:
	    return gsb_format_gdate(gsb_data_transaction_get_date (transaction_number));

	    /* mise en forme de la date de valeur */

	case TRANSACTION_LIST_VALUE_DATE:
	    return gsb_format_gdate(gsb_data_transaction_get_value_date (transaction_number));

	    /* mise en forme du tiers */

	case TRANSACTION_LIST_PARTY:
	    return ( gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number), TRUE ));

	    /* mise en forme de l'ib */

	case TRANSACTION_LIST_BUDGET:

	    temp = nom_imputation_par_no ( gsb_data_transaction_get_budgetary_number ( transaction_number),
					   gsb_data_transaction_get_sub_budgetary_number ( transaction_number));
	    return temp;


	    /* mise en forme du débit */
	case TRANSACTION_LIST_DEBIT:
	    if ( gsb_data_transaction_get_amount ( transaction_number)< -0.001 ) 
		/* -0.001 is to handle float approximations */
	    {
		temp = g_strdup_printf ( "%4.2f", -gsb_data_transaction_get_amount ( transaction_number));

		/* si la devise en cours est différente de celle de l'opé, on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != gsb_data_transaction_get_currency_number ( transaction_number))
		    devise_operation = devise_par_no ( gsb_data_transaction_get_currency_number ( transaction_number));

		if ( devise_operation
		     &&
		     devise_operation -> no_devise != gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number)) )
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

	    if ( gsb_data_transaction_get_amount ( transaction_number)>= 0 )
	    {
		temp = g_strdup_printf ( "%4.2f", gsb_data_transaction_get_amount ( transaction_number));

		/* si la devise en cours est différente de celle de l'opé, on la retrouve */

		if ( !devise_operation
		     ||
		     devise_operation -> no_devise != gsb_data_transaction_get_currency_number ( transaction_number))
		    devise_operation = devise_par_no ( gsb_data_transaction_get_currency_number ( transaction_number));

		if ( devise_operation
		     &&
		     devise_operation -> no_devise != gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number)) )
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

	    /* mise en forme du amount dans la devise du compte */

	case TRANSACTION_LIST_AMOUNT:
	    /* on doit calculer et afficher le amount de l'ope */

	    amount = gsb_data_transaction_get_adjusted_amount ( transaction_number);

	    if ( !devise_compte
		 ||
		 devise_compte -> no_devise != gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number)))
		devise_compte = devise_par_no (gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number)));

	    return ( g_strdup_printf ( "%4.2f %s",
				       amount,
				       devise_code ( devise_compte ) ));

	    /* mise en forme du moyen de paiement */

	case TRANSACTION_LIST_TYPE:
	    return ( type_ope_name_by_no ( gsb_data_transaction_get_method_of_payment_number ( transaction_number),
					   gsb_data_transaction_get_account_number (transaction_number)));

	    /* mise en forme du no de rapprochement */

	case TRANSACTION_LIST_RECONCILE_NB:
	    return ( rapprochement_name_by_no ( gsb_data_transaction_get_reconcile_number ( transaction_number)));

	    /* mise en place de l'exo */

	case TRANSACTION_LIST_EXERCICE:
	    return ( exercice_name_by_no (  gsb_data_transaction_get_financial_year_number ( transaction_number)));

	    /* mise en place des catégories */

	case TRANSACTION_LIST_CATEGORY:

	    return ( gsb_transactions_get_category_real_name ( transaction_number ));

	    /* mise en forme R/P */

	case TRANSACTION_LIST_MARK:
	    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 1 )
		return ( _("P") );
	    else
	    {
		if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 2 )
		    return ( _("T") );
		else
		{
		    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 3 )
			return ( _("R"));
		    else
			return ( NULL );
		}
	    }
	    break;


	    /* mise en place de la pièce comptable */

	case TRANSACTION_LIST_VOUCHER:
	    return ( gsb_data_transaction_get_voucher ( transaction_number));

	    /* mise en forme des notes */

	case TRANSACTION_LIST_NOTES:
	    return ( gsb_data_transaction_get_notes ( transaction_number));

	    /* mise en place de l'info banque/guichet */

	case TRANSACTION_LIST_BANK:
	    return ( gsb_data_transaction_get_bank_references ( transaction_number));

	    /* mise en place du no d'opé */

	case TRANSACTION_LIST_NO:
	    return ( utils_str_itoa ( transaction_number ));

	    /* mise en place du no de chèque/virement */

	case TRANSACTION_LIST_CHQ:
	    if ( gsb_data_transaction_get_method_of_payment_content ( transaction_number))
		return ( g_strconcat ( "(",
				       gsb_data_transaction_get_method_of_payment_content ( transaction_number),
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

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());
    model_sort = GTK_TREE_MODEL_SORT (gsb_transactions_list_get_sortable());
    model_filter = GTK_TREE_MODEL_FILTER (gsb_transactions_list_get_filter());
    nb_rows_by_transaction = gsb_data_account_get_nb_rows ( no_account );

    couleur_en_cours = 0;
    i = 0;

    path_sort = gtk_tree_path_new_first ();

    while ( (path_filter = gtk_tree_model_sort_convert_path_to_child_path ( GTK_TREE_MODEL_SORT ( model_sort ),
									    path_sort )))
    {
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer transaction;
	gint transaction_number;
	gint color_column;

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

	transaction_number = gsb_data_transaction_get_transaction_number (transaction);

	/* if we are on the selection, set the selection's color and save the normal
	 * color */

	if ( transaction_number == gsb_data_account_get_current_transaction_number (no_account))
	{
	    color_column = TRANSACTION_COL_NB_SAVE_BACKGROUND;
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur_selection,
				 -1 );
	}
	else
	    color_column = TRANSACTION_COL_NB_BACKGROUND;

	if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 color_column, &couleur_grise,
				 -1 );
	else
	{
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 color_column, &couleur_fond[couleur_en_cours],
				 -1 );

	    if ( ++i == nb_rows_by_transaction )
	    {
		i = 0;
		couleur_en_cours = 1 - couleur_en_cours;
	    }
	}
	gtk_tree_path_next ( path_sort );
    }
    return FALSE;
}



/** 
 * set the balance for each transaction showed in the tree_view
 * 
 * \param no_account
 * 
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
	 line_balance >= gsb_data_account_get_nb_rows ( no_account ))
    {
	return FALSE;
    }

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    /* path sorted is the path in the sorted list */
    path_sorted = gtk_tree_path_new_first ();

    /* path is the path in the transactions list (origin) */
    path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted );

    /*     on calcule le solde de démarrage */

    solde_courant = solde_debut_affichage ( no_account );

    while ( path )
    {
	gint transaction_number;
	GtkTreeIter iter;
	gint i;

	transaction_number = gsb_transactions_list_get_transaction_from_path ( path );

	if ( transaction_number != -1 )
	{
	    /* if it's a breakdown, we do nothing */

	    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
	    {
		gtk_tree_path_next ( path_sorted );
		path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted);
		continue;
	    }

	    solde_courant = solde_courant + gsb_data_transaction_get_adjusted_amount (transaction_number);

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

	for ( i=0 ; i<gsb_data_account_get_nb_rows ( no_account ) ; i++ )
	    gtk_tree_path_next ( path_sorted );

	path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted );
    }
    return FALSE;
}


/**
 * set the vertical adjustment value of the account
 * if the value is -1, we set at the end of the list
 *
 * \param account_number
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_adjustment_value ( gint account_number )
{
    GtkAdjustment *adjustment;
    gdouble value;

    if ( DEBUG )
	printf ( "gsb_transactions_list_set_adjustment_value account %d\n",
		 account_number );

/*     update_ecran (); */

    adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()));
    value = gsb_data_account_get_vertical_adjustment_value (account_number);

    if ( value != -1 )
    {
	/* the value is already defined */

	gtk_adjustment_set_value ( adjustment,
				   value );
    }
    else
    {
	/* we go at the end of the list */

	gtk_adjustment_set_value ( adjustment,
				   adjustment -> upper - adjustment -> page_size );
	gsb_data_account_set_vertical_adjustment_value ( account_number,
						    adjustment -> upper - adjustment -> page_size);
    }
    return FALSE;
}



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
    GSList *list_tmp_transactions;

    solde = gsb_data_account_get_init_balance (no_account);

    if ( gsb_data_account_get_r (no_account) )
	return (solde);

    /*     les R ne sont pas affichés, on les déduit du solde initial */

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	/* 	si l'opé est ventilée ou non relevée, on saute */

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == no_account
	     &&
	     !gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp)
	     &&
	     gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == 3 )
	    solde = solde + gsb_data_transaction_get_adjusted_amount (transaction_number_tmp);

	list_tmp_transactions = list_tmp_transactions -> next;
    }
    return ( solde );
}
/******************************************************************************/



/**
 * called when press a mouse button on the transactions list
 * 
 * \param tree_view
 * \param ev a GdkEventButton
 *
 * \return normally TRUE to block the signal, if we are outside the tree_view, FALSE
 * */
gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
					      GdkEventButton *ev )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path_sorted;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gpointer transaction;
    gint transaction_number, column, line_in_transaction;

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
	    popup_transaction_context_menu ( FALSE, -1, -1 );

	return (TRUE);
    }

    path = gsb_transactions_list_get_list_path_from_sorted_path (path_sorted);
    model = GTK_TREE_MODEL ( gsb_transactions_list_get_store());

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get ( model, &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 TRANSACTION_COL_NB_TRANSACTION_LINE, &line_in_transaction,
			 -1 );
    column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
			    tree_column );

    transaction_number = gsb_data_transaction_get_transaction_number (transaction);

    /*     we check if we don't want open a breakdown, before changing the selection */

    if ( tree_column == transactions_tree_view_columns[0]
	 &&
	 gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
    {
	gboolean breakdown_shown;

	gtk_tree_model_get ( model,
			     &iter,
			     TRANSACTION_COL_NB_IS_EXPANDED, &breakdown_shown,
			     -1 );

	breakdown_shown = !breakdown_shown;

	/* 	we show/hide the breadowns of that transaction */

	gsb_data_account_list_set_breakdowns_visible ( transaction_number,
						  breakdown_shown );
	gsb_transactions_list_set_background_color (gsb_data_account_get_current_account ());
	gsb_transactions_list_set_transactions_balances ( gsb_data_account_get_current_account () );

	/* in that case, we don't check anything else */

	return TRUE;
    }
    else
	/* we select the transaction, if it's a white child of breakdown, the mother transaction number will be usefull */
	gsb_transactions_list_set_current_transaction ( transaction_number,
							gsb_data_transaction_get_mother_transaction_number (transaction_number));

    /*     if it's the right click, show the good popup */

    if ( ev -> button == RIGHT_BUTTON )
    {
	if ( transaction_number == -1)
	    popup_transaction_context_menu ( FALSE, -1, -1 );
	else
	    popup_transaction_context_menu ( TRUE, column, line_in_transaction );
	return(TRUE);
    }

    /*     check if we press on the mark */

    if ( transaction_number != -1 )
    {
	if ( column == find_p_r_col() )
	{
	    if ( etat.equilibrage &&
		 line_in_transaction == find_p_r_line())
	    {
		gsb_reconcile_mark_transaction (gsb_data_transaction_get_pointer_to_transaction (transaction_number));
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

	    gsb_transactions_list_current_transaction_up ( gsb_data_account_get_current_account () );
	    break;

	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    gsb_transactions_list_current_transaction_down ( gsb_data_account_get_current_account () );
	    break;

	case GDK_Delete:		/*  del  */
	    gsb_transactions_list_delete_transaction ( gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ()) );
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



/**
 *
 *
 *
 */
gboolean gsb_transactions_list_columns_changed ( GtkTreeView *treeview, gpointer user_data )
{
    printf ("> gsb_transactions_list_columns_changed\n");

    return FALSE;
}



/** change the selection to the transaction above
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_current_transaction_up ( gint no_account )
{
    gint transaction_number;
    GtkTreePath *path;
    GtkTreePath *path_sorted;

   if ( DEBUG )
	printf ( "gsb_transactions_list_current_transaction_up\n" );

    transaction_number = gsb_data_account_get_current_transaction_number ( no_account );

    path = gsb_transactions_list_get_path_from_transaction (gsb_data_transaction_get_pointer_to_transaction (transaction_number));
    path_sorted = gsb_transactions_list_get_sorted_path_from_list_path ( path,
									 no_account );

    gtk_tree_path_prev ( path_sorted );

    path = gsb_transactions_list_get_list_path_from_sorted_path ( path_sorted );

    transaction_number = gsb_transactions_list_get_transaction_from_path ( path );

    gsb_transactions_list_set_current_transaction ( transaction_number,
						    gsb_data_transaction_get_mother_transaction_number (transaction_number));
    return FALSE;
}


/** change the selection to the transaction below
 * \param no_account
 * \return FALSE
 * */
gboolean gsb_transactions_list_current_transaction_down ( gint no_account )
{
    gint transaction_number;
    gint current_transaction_number;

     if ( DEBUG )
	printf ( "gsb_transactions_list_current_transaction_down\n");

   current_transaction_number = gsb_data_account_get_current_transaction_number (no_account);

    transaction_number = gsb_transactions_list_get_transaction_next ( current_transaction_number,
								      gsb_data_transaction_get_mother_transaction_number (current_transaction_number));

    gsb_transactions_list_set_current_transaction ( transaction_number,
						    gsb_data_transaction_get_mother_transaction_number (transaction_number));
    return FALSE;
}



/**
 * Return a pointer to the transaction after the one in param
 * in the tree_view
 * 
 * \param transaction_number The transaction we want to move down
 * \param mother_transaction_number if the transaction is a white child of breakdown, we need the number of the mother
 * 
 * \return the number of the new transaction or 0 if problem
 * */
gint gsb_transactions_list_get_transaction_next ( gint transaction_number,
						  gint mother_transaction_number )
{
    GtkTreePath *path;
    GtkTreePath *path_sorted;

    /* if we are on the white line, do nothing */

    if ( transaction_number == -1)
	return 0;

    path = gsb_transactions_list_get_path_from_transaction (gsb_data_transaction_get_pointer_to_transaction (transaction_number));
    path_sorted = gsb_transactions_list_get_sorted_path_from_list_path ( path,
									 gsb_data_transaction_get_account_number (transaction_number));

    /* if the transaction is a breakdown child, the line is only 1 line */

    if ( mother_transaction_number )
	gtk_tree_path_next ( path_sorted );
    else
    {
	gint i;

	for ( i=0 ; i<gsb_data_account_get_nb_rows ( gsb_data_transaction_get_account_number (transaction_number)) ; i++ )
	    gtk_tree_path_next ( path_sorted );
    }

    path = gsb_transactions_list_get_list_path_from_sorted_path (path_sorted);

    transaction_number = gsb_transactions_list_get_transaction_from_path ( path);

    return transaction_number;
}



/** 
 * change the selection to the transaction
 *
 * \param transaction_number the transaction we want to move to
 * \param mother_transaction_number if it's a white breakdown child, it's number is -1 so we need the number
 * of the mother to find it
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_current_transaction ( gint transaction_number,
							 gint mother_transaction_number )
{
    GtkTreeIter *iter;
    gint i;
    GdkColor *couleur;
    GtkTreeModel *model;
    gint current_transaction_number;
    gint account_number;

    if ( DEBUG )
	printf ( "gsb_transactions_list_set_current_transaction %d\n",
		 transaction_number);
    
    if ( !gsb_transactions_list_get_tree_view()
	 ||
	 !transaction_number )
	return FALSE;
    
    /* the white number has no account number, so we take the current account */

    if ( transaction_number != -1 )
	account_number = gsb_data_transaction_get_account_number (transaction_number);
    else
	account_number = gsb_data_account_get_current_account ();

    current_transaction_number =  gsb_data_account_get_current_transaction_number (account_number);
    
    /* if there is no change in the selection, go away */

    if ( transaction_number == current_transaction_number )
	return FALSE;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    /*   vire l'ancienne sélection : consiste à remettre la couleur d'origine du background */

    iter = gsb_transactions_list_get_iter_from_transaction (current_transaction_number);

    if ( iter )
    {
	/* 	iter est maintenant positionné sur la 1ère ligne de l'opé à désélectionner */

	for ( i=0 ; i<gsb_data_account_get_nb_rows ( account_number ) ; i++ )
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

	    if ( gsb_data_transaction_get_mother_transaction_number (current_transaction_number))
		i = gsb_data_account_get_nb_rows (account_number);

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
	gtk_tree_iter_free (iter);
    }

    gsb_data_account_set_current_transaction_number ( account_number,
						 transaction_number);

    iter = gsb_transactions_list_get_iter_from_transaction (transaction_number);

    if ( iter )
    {
	/* 	iter est maintenant positionne sur la 1ère ligne de l'ope à sélectionner */

	for ( i=0 ; i<gsb_data_account_get_nb_rows ( account_number ) ; i++ )
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

	    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
		i = gsb_data_account_get_nb_rows (account_number);

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
	gtk_tree_iter_free (iter);
    }
    return FALSE;
}


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

    path = gsb_transactions_list_get_path_from_transaction ( gsb_data_transaction_get_pointer_to_transaction (gsb_data_account_get_current_transaction_number (no_account)));
    path_sorted = gsb_transactions_list_get_sorted_path_from_list_path ( path, no_account );

    /* Sometimes, the current transaction can be hidden, so we have to
     * check for each path if it's valid if it's not, we make a
     * selection on the white line */

    if ( !path_sorted )
    {
	gsb_transactions_list_set_current_transaction ( -1, 0 );
	return FALSE;
    }

    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view()),
				   path_sorted, NULL, FALSE, 0.5, 0.5 );


    return FALSE;
}


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

    model_sort = GTK_TREE_MODEL_SORT ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view() )));
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
GtkTreePath *gsb_transactions_list_get_list_path_from_sorted_path ( GtkTreePath *path_sorted )
{
    GtkTreeModelFilter *model_filter;
    GtkTreeModelSort *model_sort;
    GtkTreePath *path_filter;
    GtkTreePath *path;

    if ( !path_sorted )
	return NULL;

    model_sort = GTK_TREE_MODEL_SORT (gsb_transactions_list_get_sortable());
    model_filter = GTK_TREE_MODEL_FILTER (gsb_transactions_list_get_filter());

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
gpointer cherche_operation_from_ligne ( gint ligne,
					gint no_account )
{
    GtkTreeIter iter;
    gpointer transaction;
    GtkTreeModel *model;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());


    if ( !gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL ( model ),
					       &iter,
					       utils_str_itoa (ligne)))
	return NULL;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
			 &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 -1 );

    return ( transaction );

}
/******************************************************************************/


/**
 * find and return the iter in the model list of the transaction
 * 
 * \param transaction_number
 *
 * \return the iter of the transaction or NULL
 * */
GtkTreeIter *gsb_transactions_list_get_iter_from_transaction ( gint transaction_number )
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    if ( !model)
	return NULL;

    /* we go throw the list in the model untill we find the transaction */

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					 &iter ))
    {
	do
	{
	    gpointer transaction_pointer;

	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_pointer,
				 -1 );

	    if ( gsb_data_transaction_get_transaction_number (transaction_pointer) == transaction_number )
		return ( gtk_tree_iter_copy (&iter));
	}
	while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					   &iter ));
    }
    return NULL;
}



/**
 * Find the address of the white line of the breakdown given in param
 *
 * \param breakdown_mother The transaction mother of the line looked for
 *
 * \return the adress of the transaction or NULL
 * */
gpointer gsb_transactions_list_find_white_breakdown ( gpointer *breakdown_mother )
{
    gpointer transaction_pointer;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( !breakdown_mother)
	return NULL;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

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
	while ( gsb_data_transaction_get_transaction_number (transaction_pointer) != -2
		&&
		gsb_data_transaction_get_mother_transaction_number ( gsb_data_transaction_get_transaction_number (transaction_pointer ))!= gsb_data_transaction_get_transaction_number (breakdown_mother)
		&&
		gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					   &iter ));
    }

    if ( gsb_data_transaction_get_transaction_number (transaction_pointer) == -2
	 &&
	 gsb_data_transaction_get_mother_transaction_number ( gsb_data_transaction_get_transaction_number (transaction_pointer ))== gsb_data_transaction_get_transaction_number (breakdown_mother))
	return ( transaction_pointer );
    else
	return NULL;
}



/** give back the path of the transaction given
 * \param transaction the transaction we want to find the path
 * \return the GtkTreePath
 * */
GtkTreePath *gsb_transactions_list_get_path_from_transaction ( gpointer *transaction )
{
    GtkTreePath *path;
    GtkTreeIter *iter;

    if ( !transaction || ! gsb_transactions_list_get_tree_view() )
	return NULL;

    iter = gsb_transactions_list_get_iter_from_transaction (gsb_data_transaction_get_transaction_number (transaction));
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL (gsb_transactions_list_get_store() ),
				     iter );
    return path;
}



/** give back the transaction on the path
 * \param path the path in the normal list
 * \return the transaction number
 * */
gint gsb_transactions_list_get_transaction_from_path ( GtkTreePath *path )
{
    gpointer transaction;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( !path )
	return 0;

    model = GTK_TREE_MODEL ( gsb_transactions_list_get_store());

    gtk_tree_model_get_iter ( model,
			      &iter,
			      path );
    gtk_tree_model_get ( model,
			 &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 -1 );

    return gsb_data_transaction_get_transaction_number (transaction);
}



/******************************************************************************/
/* cette fonction renvoie le no de ligne de l'opération en argument */
/******************************************************************************/
gint cherche_ligne_operation ( gint transaction_number,
			       gint no_account )
{
    GtkTreeIter *iter;

    iter = gsb_transactions_list_get_iter_from_transaction (transaction_number);

    return ( utils_str_atoi ( gtk_tree_model_get_string_from_iter (  GTK_TREE_MODEL ( gsb_transactions_list_get_store() ),
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
    gchar *char_tmp;
    gint i, j;
    struct organisation_formulaire *form_organization;
    GtkWidget *menu;
    gint focus_to;
    gint transaction_number;

    transaction_number = gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ());

    formulaire_a_zero ();
    degrise_formulaire_operations ();
    gtk_expander_set_expanded ( GTK_EXPANDER ( frame_droite_bas ), TRUE );

    /* if the transaction is the white line, we set the date and go away */

    if ( transaction_number == -1 )
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

    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
	gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_DATE),
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY),
				   FALSE );
    }


    /*   l'opé n'est pas -1, c'est une modif, on remplit les champs */

    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "transaction_number_in_form",
			  GINT_TO_POINTER (transaction_number));

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
					 utils_str_itoa (transaction_number));
		    break;

		case TRANSACTION_FORM_DATE:

		    entree_prend_focus ( widget );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 gsb_format_gdate ( gsb_data_transaction_get_date (transaction_number)));
		    break;

		case TRANSACTION_FORM_VALUE_DATE:

		    if ( gsb_data_transaction_get_value_date (transaction_number))
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_format_gdate ( gsb_data_transaction_get_value_date (transaction_number)));
		    }
		    break;

		case TRANSACTION_FORM_EXERCICE:

		    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget ),
						   cherche_no_menu_exercice ( gsb_data_transaction_get_financial_year_number (transaction_number),
									      widget ));

		    /* 		    si l'opé est ventilée, on désensitive l'exo */

		    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
			gtk_widget_set_sensitive ( widget,
						   FALSE );

		    break;

		case TRANSACTION_FORM_PARTY:

		    if ( gsb_data_transaction_get_party_number (transaction_number))
		    {
			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						gsb_data_payee_get_name ( gsb_data_transaction_get_party_number (transaction_number), TRUE ));
		    }
		    break;

		case TRANSACTION_FORM_DEBIT:

		    if ( gsb_data_transaction_get_amount (transaction_number)< 0 )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       -gsb_data_transaction_get_amount (transaction_number)));
		    }

		    /* 		    si l'opé est relevée, on ne peut modifier le montant */

		    if ( gsb_data_transaction_get_marked_transaction (transaction_number)== 3 )
			gtk_widget_set_sensitive ( widget,
						   FALSE );

		    break;

		case TRANSACTION_FORM_CREDIT:

		    if ( gsb_data_transaction_get_amount ( transaction_number)>= 0 )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       gsb_data_transaction_get_amount ( transaction_number)));
		    }

		    /* 		    si l'opé est relevée, on ne peut modifier le montant */

		    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 3 )
			gtk_widget_set_sensitive ( widget,
						   FALSE );

		    break;

		case TRANSACTION_FORM_CATEGORY:

		    if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number))
		    {
			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Breakdown of transaction") );
		    }
		    else
		    {
			if ( gsb_data_transaction_get_transaction_number_transfer ( transaction_number))
			{
			    /* c'est un virement */

			    entree_prend_focus ( widget );

			    if ( gsb_data_transaction_get_transaction_number_transfer ( transaction_number)!= -1 )
			    {
				gint contra_transaction_number;

				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							g_strconcat ( COLON(_("Transfer")),
								      gsb_data_account_get_name (gsb_data_transaction_get_account_number_transfer ( transaction_number)),
								      NULL ));

				/* récupération de la contre opération */

				contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number);

				/* 	  si la contre opération est relevée, on désensitive les categ et les montants */

				if ( contra_transaction_number
				     &&
				     gsb_data_transaction_get_marked_transaction (contra_transaction_number) == OPERATION_RAPPROCHEE )
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

			    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 3 )
				gtk_widget_set_sensitive ( widget,
							   FALSE );
			}
			else
			{
			    /* c'est des catégories normales */

			    char_tmp = gsb_data_category_get_name ( gsb_data_transaction_get_category_number ( transaction_number),
								    gsb_data_transaction_get_sub_category_number ( transaction_number),
								    NULL );
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

		    char_tmp = nom_imputation_par_no ( gsb_data_transaction_get_budgetary_number ( transaction_number),
						       gsb_data_transaction_get_sub_budgetary_number ( transaction_number));
		    if ( char_tmp )
		    {
			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						char_tmp );
		    }

		    /* 		    si l'opé est ventilée, on dÃ©sensitive l'ib */

		    if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number))
			gtk_widget_set_sensitive ( widget,
						   FALSE );
		    break;

		case TRANSACTION_FORM_NOTES:

		    if ( gsb_data_transaction_get_notes ( transaction_number))
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_data_transaction_get_notes ( transaction_number));
		    }
		    break;

		case TRANSACTION_FORM_TYPE:

		    if ( gsb_data_transaction_get_amount ( transaction_number)< 0 )
			menu = creation_menu_types ( 1, gsb_data_account_get_current_account (), 0  );
		    else
			menu = creation_menu_types ( 2, gsb_data_account_get_current_account (), 0  );

		    if ( menu )
		    {
			/* on met en place les types et se place sur celui correspondant à l'opé */

			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						   menu );
			gtk_widget_show ( widget );

			place_type_formulaire ( gsb_data_transaction_get_method_of_payment_number ( transaction_number),
						TRANSACTION_FORM_TYPE,
						gsb_data_transaction_get_method_of_payment_content ( transaction_number));
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
								  devise_par_no ( gsb_data_transaction_get_currency_number ( transaction_number))));
		    verification_bouton_change_devise ();
		    break;

		case TRANSACTION_FORM_BANK:

		    if ( gsb_data_transaction_get_bank_references ( transaction_number))
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_data_transaction_get_bank_references ( transaction_number));
		    }
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    if ( gsb_data_transaction_get_voucher ( transaction_number))
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_data_transaction_get_voucher ( transaction_number));
		    }
		    break;

		case TRANSACTION_FORM_CONTRA:

		    if ( gsb_data_transaction_get_transaction_number_transfer ( transaction_number))
		    {
			if ( gsb_data_transaction_get_amount ( transaction_number)< 0 )
			    menu = creation_menu_types ( 2, gsb_data_transaction_get_account_number_transfer ( transaction_number), 0  );
			else
			    menu = creation_menu_types ( 1, gsb_data_transaction_get_account_number_transfer ( transaction_number), 0  );

			if ( menu )
			{
			    gint contra_transaction_number;

			    /* on met en place les types et se place sur celui correspondant à l'opé */

			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						       menu );
			    gtk_widget_show ( widget );

			    contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number);

			    if  (contra_transaction_number )
				place_type_formulaire ( gsb_data_transaction_get_method_of_payment_number (contra_transaction_number),
							TRANSACTION_FORM_CONTRA,
							NULL );
			}
			else
			    gtk_widget_hide ( widget );
		    }
		    break;

		case TRANSACTION_FORM_MODE:

		    if ( gsb_data_transaction_get_automatic_transaction ( transaction_number))
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

    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
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
    gint transaction_number;
    GtkTreeIter *iter;
    gint col;
    GtkTreeModel *model;

    col = find_p_r_col ();
    if ( col == -1 )
	return;

    transaction_number = gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ());

    /*     take the model of the account */

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    /* si on est sur l'opération vide -> on se barre */

    if ( transaction_number == -1
	 ||
	 gsb_data_transaction_get_marked_transaction ( transaction_number)== 3 )
	return;

    iter = gsb_transactions_list_get_iter_from_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ()));

    if ( gsb_data_transaction_get_marked_transaction ( transaction_number))
    {
	montant = gsb_data_transaction_get_adjusted_amount ( transaction_number);

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees - montant;

	gsb_data_account_set_marked_balance ( gsb_data_account_get_current_account (),
					 gsb_data_account_get_marked_balance (gsb_data_account_get_current_account ()) - montant );
	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      0 );

	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     col, NULL,
			     -1 );
    }
    else
    {
	montant = gsb_data_transaction_get_adjusted_amount ( transaction_number);

	if ( etat.equilibrage )
	    operations_pointees = operations_pointees + montant;

	gsb_data_account_set_marked_balance ( gsb_data_account_get_current_account (),
					 gsb_data_account_get_marked_balance (gsb_data_account_get_current_account ()) + montant );
	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      1 );

	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     col, _("P"),
			     -1 );
    }

    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

    if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number))
    {
	GSList *list_tmp_transactions;

	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_account_get_current_account ()
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
		gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
							      gsb_data_transaction_get_marked_transaction ( transaction_number));

	    list_tmp_transactions = list_tmp_transactions -> next;
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
    gint transaction_number;
    GtkTreeIter *iter;
    gint col;
    GtkTreeModel *model;

    col = find_p_r_col ();
    if ( col == -1 )
	return;

    transaction_number = gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ());

    /* si on est sur l'opération vide -> on se barre */

    if ( transaction_number == -1 )
	return;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    iter = gsb_transactions_list_get_iter_from_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ()));

    if ( !gsb_data_transaction_get_marked_transaction ( transaction_number))
    {
	/* on relève l'opération */

	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      3);

	/* on met soit le R, soit on change la sélection vers l'opé suivante */

	if ( gsb_data_account_get_r (gsb_data_account_get_current_account ()) )
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 col, _("R"),
				 -1 );
	else
	{
	    /* we don't want to see the R, so the transaction will diseapear */

	    gint next_transaction_number;

	    next_transaction_number = gsb_transactions_list_get_transaction_next (transaction_number,
										  gsb_data_transaction_get_mother_transaction_number (transaction_number));
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_VISIBLE, FALSE,
				 -1 );
	    gsb_data_account_set_current_transaction_number ( gsb_data_account_get_current_account (),
							 next_transaction_number);
	    gsb_transactions_list_set_background_color (gsb_data_account_get_current_account ());
	    gsb_transactions_list_set_transactions_balances (gsb_data_account_get_current_account ());
	}

	modification_fichier( TRUE );
    }
    else
	if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 3 )
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

    if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number))
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_account_get_current_account ()
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
		gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
							      gsb_data_transaction_get_marked_transaction ( transaction_number));

	    list_tmp_transactions = list_tmp_transactions -> next;
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
gboolean gsb_transactions_list_delete_transaction ( gint transaction_number )
{
    gint transaction_number_tmp;
    gdouble montant;

    /* vérifications de bases */

    if ( !transaction_number
	 ||
	 transaction_number < 0 )
	return FALSE;

    if ( DEBUG )
	printf ( "gsb_transactions_list_delete_transaction no %d\n",
		 transaction_number );

    /* check if the transaction is not reconciled */

    if ( gsb_transactions_list_check_mark ( gsb_data_transaction_get_pointer_to_transaction (transaction_number)))
    {
	dialogue_error ( _("Impossible to delete a reconciled transaction.\nThe transaction, the contra-transaction or the children if it is a breakdown are reconciled. You can remove the reconciliation with Ctrl R if it is really necessary.") );
	return FALSE;
    }

    if ( !question_yes_no_hint ( _("Delete a transaction"),
				 g_strdup_printf ( _("Do you really want to delete the transaction whith the party '%s' ?"),
						   gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number),
								      FALSE ))))
	return FALSE;

    /* find the next transaction to be selected */

    if ( gsb_data_account_get_current_transaction_number (gsb_data_transaction_get_account_number (transaction_number)) == transaction_number)
    {
	gint next_transaction_number;

	next_transaction_number = transaction_number;

	/* xxx à vérifier que le changemment de sélection lors de la supression avec ventil marche bien... */
	while ( next_transaction_number
		&&
		( next_transaction_number == transaction_number 
		  ||
		  gsb_data_transaction_get_mother_transaction_number (next_transaction_number) == transaction_number ))
	    next_transaction_number = gsb_transactions_list_get_transaction_next ( next_transaction_number,
										   gsb_data_transaction_get_mother_transaction_number (next_transaction_number));

	gsb_data_account_set_current_transaction_number ( gsb_data_transaction_get_account_number (transaction_number),
						     next_transaction_number);
    }

    /* if it's a transfer, delete the contra-transaction */

    if ( gsb_data_transaction_get_transaction_number_transfer ( transaction_number)
	 &&
	 gsb_data_transaction_get_account_number_transfer ( transaction_number)!= -1 )
    {
	transaction_number_tmp = gsb_data_transaction_get_transaction_number_transfer ( transaction_number);

	gsb_transactions_list_delete_transaction_from_tree_view (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp));
	gsb_data_transaction_remove_transaction (transaction_number_tmp);
    }

    /* if it's a breakdown, delete all the childs */

    if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number))
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (transaction_number))
	    {
		if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number )
		{
		    /* on se place tout de suite sur l'opé suivante */

		    list_tmp_transactions = list_tmp_transactions -> next;

		    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number_tmp))
		    {
			/* the breakdown is a transfer, delete the contra-transaction */

			gint contra_transaction_number;

			contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number_tmp );

			gsb_transactions_list_delete_transaction_from_tree_view ( gsb_data_transaction_get_pointer_to_transaction (contra_transaction_number));
			gsb_data_transaction_remove_transaction (contra_transaction_number);
		    }

		    gsb_transactions_list_delete_transaction_from_tree_view ( gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp));
		    gsb_data_transaction_remove_transaction (transaction_number_tmp);
		}
		else
		    list_tmp_transactions = list_tmp_transactions -> next;
	    }
	    else
		list_tmp_transactions = list_tmp_transactions -> next;
	}

	/* we have also to destroy the white line of the breakdown */

	gsb_transactions_list_delete_transaction_from_tree_view ( gsb_transactions_list_find_white_breakdown ( gsb_data_transaction_get_pointer_to_transaction (transaction_number)));
    }

    /* now we delete the original transaction */

    gsb_transactions_list_delete_transaction_from_tree_view ( gsb_data_transaction_get_pointer_to_transaction (transaction_number));
    gsb_data_transaction_remove_transaction ( transaction_number);

    gsb_transactions_list_set_background_color (gsb_data_transaction_get_account_number (transaction_number));
    gsb_transactions_list_set_transactions_balances (gsb_data_transaction_get_account_number (transaction_number));

    /*     calcul des nouveaux soldes */

    montant = gsb_data_transaction_get_adjusted_amount ( transaction_number);

    gsb_data_account_set_current_balance ( gsb_data_transaction_get_account_number (transaction_number),
				      gsb_data_account_get_current_balance (gsb_data_transaction_get_account_number (transaction_number)) - montant );

    if ( gsb_data_transaction_get_marked_transaction ( transaction_number))
	gsb_data_account_set_marked_balance ( gsb_data_transaction_get_account_number (transaction_number),
					 gsb_data_account_get_marked_balance (gsb_data_transaction_get_account_number (transaction_number)) - montant );

    /*     on met à jour les labels de solde */

    mise_a_jour_labels_soldes ();

    /* si on est en train d'équilibrer => recalcule le total pointé */

    if ( etat.equilibrage )
	calcule_total_pointe_compte ( gsb_data_transaction_get_account_number (transaction_number));

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
gboolean gsb_transactions_list_check_mark ( gpointer transaction )
{
    gpointer transactions_tmp;

    /* vérifications de bases */

    if ( !transaction )
	return FALSE;

    if ( gsb_data_transaction_get_marked_transaction ( gsb_data_transaction_get_transaction_number (transaction ))== OPERATION_RAPPROCHEE )
	return TRUE;

    /* if it's a transfer, check the contra-transaction */

    if ( gsb_data_transaction_get_transaction_number_transfer ( gsb_data_transaction_get_transaction_number (transaction ))
	 &&
	 gsb_data_transaction_get_account_number_transfer ( gsb_data_transaction_get_transaction_number (transaction ))!= -1 )
    {
	transactions_tmp = gsb_data_transaction_get_pointer_to_transaction (gsb_data_transaction_get_transaction_number_transfer (gsb_data_transaction_get_transaction_number (transaction)));

	if ( transactions_tmp
	     &&
	     gsb_data_transaction_get_marked_transaction ( gsb_data_transaction_get_transaction_number (transactions_tmp ))== OPERATION_RAPPROCHEE )
	    return TRUE;
    }

    /* if it's a breakdown of transaction, check all the children
     * if there is not a transfer which is marked */

    if ( gsb_data_transaction_get_breakdown_of_transaction ( gsb_data_transaction_get_transaction_number (transaction )))
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (gsb_data_transaction_get_transaction_number (transaction))
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == gsb_data_transaction_get_transaction_number (transaction))
	    {
		/* transactions_tmp is a child of transaction */

		if ( gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE )
		    return TRUE;

		if (  gsb_data_transaction_get_transaction_number_transfer (transaction_number_tmp))
		{
		    /* the breakdown is a transfer, we check the contra-transaction */

		    gint contra_transaction_number;

		    contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number_tmp);

		    if ( contra_transaction_number
			 &&
			 gsb_data_transaction_get_marked_transaction (contra_transaction_number)== OPERATION_RAPPROCHEE )
			return TRUE;
		}
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
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
gboolean gsb_transactions_list_delete_transaction_from_tree_view ( gpointer transaction )
{
    GtkTreeIter *iter;

    if ( !transaction )
	return FALSE;

    if ( DEBUG )
	printf ( "gsb_transactions_list_delete_transaction_from_tree_view no %d\n",
		 gsb_data_transaction_get_transaction_number (transaction) );

    iter = gsb_transactions_list_get_iter_from_transaction (gsb_data_transaction_get_transaction_number (transaction));

    if ( iter )
    {
	GtkTreeModel *model;
	gint i;

	model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

	for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	{
	    gtk_list_store_remove ( GTK_LIST_STORE ( model ),
				    iter );

	    /* if we are on a breakdown child, it's only 1 line */

	    if ( gsb_data_transaction_get_mother_transaction_number ( gsb_data_transaction_get_transaction_number (transaction )))
		i = TRANSACTION_LIST_ROWS_NB;
	}

	gtk_tree_iter_free (iter);
    }
    return FALSE;
}



/**
 * Update screen when screen size changes.
 *
 * \param tree_view	Unused.
 * \param allocation	Size of new allocation.
 *
 * \return FALSE
 */
gboolean changement_taille_liste_ope ( GtkWidget *tree_view, GtkAllocation *allocation )
{
    /* Check to avoid useless computation. */
    if ( allocation -> width == allocation_precedente )
	return FALSE;

    allocation_precedente = allocation -> width;

    /* Update various elements. */
    mise_a_jour_taille_formulaire ( allocation_precedente );
    update_ecran ();

    return ( FALSE );
}



/******************************************************************************/
/* Fonction  demande_mise_a_jour_tous_comptes */
/* met la variable update_list de tous les comptes à 1 */
/* ce qui fait que lorsqu'ils seront affichés, ils seront mis à jour avant */
/* appelle aussi verification_mise_a_jour_liste pour mettre à jour la liste courante */
/******************************************************************************/
void demande_mise_a_jour_tous_comptes ( void )
{
    gtk_list_store_clear ( GTK_LIST_STORE ( gsb_transactions_list_get_store()  ));
    gsb_transactions_list_fill_store ( gsb_transactions_list_get_store() );
}
/******************************************************************************/


/**
 * Pop up a menu with several actions to apply to current transaction.
 * 
 * \param xxx
 *
 */
void popup_transaction_context_menu ( gboolean full, int x, int y )
{
    GtkWidget *menu, *menu_item;

    if ( gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ()) == -1 )
	full = FALSE;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Edit transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_PROPERTIES,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK (gsb_transactions_list_edit_current_transaction), NULL );
    gtk_widget_set_sensitive ( menu_item, full );
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
    if ( !full
	 || 
	 gsb_data_transaction_get_marked_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ())) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ())) == OPERATION_TELERAPPROCHEE )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Clone transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Clone transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_COPY,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", clone_selected_transaction, NULL );
    gtk_widget_set_sensitive ( menu_item, full );
    gtk_menu_append ( menu, menu_item );

    /* Separator */
    gtk_menu_append ( menu, gtk_separator_menu_item_new() );

    /* Convert to scheduled transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Convert transaction to scheduled transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_CONVERT,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", schedule_selected_transaction, NULL );
    gtk_widget_set_sensitive ( menu_item, full );
    gtk_menu_append ( menu, menu_item );

    /* Move to another account */
    menu_item = gtk_image_menu_item_new_with_label ( _("Move transaction to another account") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_JUMP_TO,
							       GTK_ICON_SIZE_MENU ));
    if ( !full
	 || 
	 gsb_data_transaction_get_marked_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ())) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ())) == OPERATION_TELERAPPROCHEE )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_append ( menu, menu_item );

    /* Add accounts submenu */
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(menu_item), 
				GTK_WIDGET(creation_option_menu_comptes(GTK_SIGNAL_FUNC(move_selected_operation_to_account), FALSE, FALSE)) );

    /* Separator */
    gtk_menu_append ( menu, gtk_separator_menu_item_new() );

    /* Change cell content. */
    menu_item = gtk_menu_item_new_with_label ( _("Change cell content") );
    if ( full )
	gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( menu_item ), 
				    GTK_WIDGET ( gsb_gui_create_cell_contents_menu ( x, y ) ) );
    gtk_widget_set_sensitive ( menu_item, full );
    gtk_menu_append ( menu, menu_item );

    /* Finish all. */
    gtk_widget_show_all (menu);
    gtk_menu_popup ( GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
}



/**
 * Create and return a menu that contains all cell content types.
 * When a type is selected, the cell that triggered this pop-up menu
 * is changed accordingly.
 * 
 * \param x	Horizontal coordinate of the cell that will be modified.
 * \param y	Vertical coordinate of the cell that will be modified.
 *
 * \return	A newly-allocated menu.
 */
GtkWidget * gsb_gui_create_cell_contents_menu ( int x, int y )
{
    GtkWidget * menu, * item;
    gchar *labels_boutons [] = {
	_("Date"), _("Value date"), _("Payee"), _("Budgetary line"), _("Debit"), 
	_("Credit"), _("Balance"), _("Amount"), _("Method of payment"),
	_("Reconciliation ref."), _("Financial year"), _("Category"), _("C/R"),
	_("Voucher"), _("Notes"), _("Bank references"), _("Transaction number"),
	NULL };
    gint i;

    menu = gtk_menu_new ();

    for ( i = 0 ; labels_boutons[i] ; i++ )
    {
	item = gtk_menu_item_new_with_label ( labels_boutons[i] );
	g_object_set_data ( item, "x", x );
	g_object_set_data ( item, "y", y );
	g_signal_connect ( G_OBJECT(item), "activate", 
			   G_CALLBACK(gsb_gui_change_cell_content), i );
	gtk_menu_append ( menu, item );
    }
    
    return menu;
}



/**
 * Iterator to update one cell of the transaction list.
 *
 * \param model		Model to update.
 * \param path		GtkTreePath to the iter to modify.
 * \param iter		GtkTreeIter to modify.
 * \param coords	A couple of coordinates : coords[0] is
 *			vertical coordinate and coords[1] is
 *			horizontal coordinate.
 *
 * \return FALSE
 */
gboolean gsb_gui_update_row_foreach ( GtkTreeModel *model, GtkTreePath *path,
				      GtkTreeIter *iter, gint coords[2] )
{
    gint line, transaction_number;
    gpointer pointer;
    
    gtk_tree_model_get ( model, iter,
			 TRANSACTION_COL_NB_TRANSACTION_LINE, &line,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &pointer,
			 -1 );
    transaction_number = gsb_data_transaction_get_transaction_number ( pointer );

    if ( coords[1] == line )
    {
	gtk_list_store_set ( gsb_transactions_list_get_store (), iter, coords[0], 
			     gsb_transactions_list_grep_cell_content ( transaction_number,
								       tab_affichage_ope[coords[1]][coords[0]] ),
			     -1 );
    }

    return FALSE;
}



/**
 * Change the content of a cell.  This is triggered from the
 * activation of a menu item, so that we check the attributes of this
 * item to determine which cell is changed and with what.
 *
 * After this, iterate to update all GtkTreeIters of the transaction
 * list.
 *
 * \param item		The GtkMenuItem that triggered event.
 * \param number	Content to put in the cell.
 *
 * \return FALSE
 */
gboolean gsb_gui_change_cell_content ( GtkWidget * item, gint number )
{
    gint coords[2];

    coords[0] = g_object_get_data ( item, "x" );
    coords[1] = g_object_get_data ( item, "y" );

    tab_affichage_ope[coords[1]][coords[0]] = number + 1;
    
    gtk_tree_model_foreach ( gsb_transactions_list_get_store (),
			     gsb_gui_update_row_foreach, coords );

    return FALSE;
}



/**
 *  Check that a transaction is selected 
 *
 * \return TRUE on success, FALSE otherwise.
 */
gboolean assert_selected_transaction ()
{
    if ( gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ()) == -1 )
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
    gsb_data_account_set_current_transaction_number ( gsb_data_account_get_current_account (),
						 0 );
    gsb_transactions_list_edit_current_transaction ();

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
    gtk_expander_set_expanded ( GTK_EXPANDER ( frame_droite_bas ), TRUE );
}



/**
 * Remove selected transaction if any.
 */
void remove_transaction ()
{
    if (! assert_selected_transaction()) return;

    gsb_transactions_list_delete_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ()));
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}



/**
 * Clone selected transaction if any.  Update user interface as well.
 */
void clone_selected_transaction ()
{
    gint new_transaction_number;

    if (! assert_selected_transaction()) return;

    new_transaction_number = gsb_transactions_list_clone_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ()));

    update_transaction_in_trees ( gsb_data_transaction_get_pointer_to_transaction (new_transaction_number));

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
 * \param transaction_number Initial transaction to clone
 *
 * \return the number newly created transaction.
 */
gint gsb_transactions_list_clone_transaction ( gint transaction_number )
{
    gint new_transaction_number;

    /* dupplicate the transaction */

    new_transaction_number = gsb_data_transaction_new_transaction ( gsb_data_transaction_get_account_number (transaction_number));
    gsb_data_transaction_copy_transaction ( transaction_number,
					    new_transaction_number );

    /* create the contra-transaction if necessary */

    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number))
	gsb_form_validate_transfer ( new_transaction_number,
				     1,
				     gsb_data_account_get_name (gsb_data_transaction_get_account_number_transfer (transaction_number)) );

    gsb_transactions_list_append_new_transaction (new_transaction_number);

    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
    {
	/* the transaction was a breakdown, we look for the children to copy them */

	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (transaction_number)
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number )
	    {
		gint breakdown_transaction_number;

		breakdown_transaction_number = gsb_transactions_list_clone_transaction (transaction_number_tmp);
		gsb_data_transaction_set_mother_transaction_number ( breakdown_transaction_number,
								     transaction_number );
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}
    }
    return new_transaction_number;
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

    source_account = gsb_data_account_get_current_account ();
    target_account = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT(menu_item), 
							     "no_compte" ) );  

    if ( move_operation_to_account ( gsb_data_account_get_current_transaction_number (source_account),
				     target_account ))
    {
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	update_transaction_in_trees ( gsb_data_transaction_get_pointer_to_transaction (gsb_data_account_get_current_transaction_number (source_account)));

	if ( mise_a_jour_combofix_tiers_necessaire )
	    mise_a_jour_combofix_tiers ();
	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ ();
	if ( mise_a_jour_combofix_imputation_necessaire )
	    mise_a_jour_combofix_imputation ();

	gsb_data_account_set_current_balance ( source_account, 
					       calcule_solde_compte ( source_account ));
	gsb_data_account_set_marked_balance ( source_account, 
					      calcule_solde_pointe_compte ( source_account ));

	mise_a_jour_labels_soldes ();
	mise_a_jour_accueil ();

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

    source_account = gsb_data_account_get_current_account ();
    target_account = GPOINTER_TO_INT ( account );  

    if ( move_operation_to_account ( gsb_data_account_get_current_transaction_number (source_account),
				     target_account ))
    {
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	if ( mise_a_jour_combofix_tiers_necessaire )
	    mise_a_jour_combofix_tiers ();
	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ ();
	if ( mise_a_jour_combofix_imputation_necessaire )
	    mise_a_jour_combofix_imputation ();

	update_transaction_in_trees ( gsb_data_transaction_get_pointer_to_transaction (gsb_data_account_get_current_transaction_number (source_account)));

	gsb_data_account_set_current_balance ( source_account, 
					  calcule_solde_compte ( source_account ));
	gsb_data_account_set_marked_balance ( source_account, 
					 calcule_solde_pointe_compte ( source_account ));

	mise_a_jour_labels_soldes ();

	modification_fichier ( TRUE );
    }
}



/**
 * Move transaction to another account
 *
 * \param transaction_number Transaction to move to other account
 * \param target_account Account to move the transaction to
 * return TRUE if ok
 */
gboolean move_operation_to_account ( gint transaction_number,
				     gint target_account )
{
    GtkTreeIter *iter;
    gint source_account;
    gint contra_transaction_number;

    source_account = gsb_data_transaction_get_account_number (transaction_number);

    /* if it's a transfer, update the contra-transaction */
    /* xxx FIXME : check also for children if breakdown */

    if ( ( contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number)))
    {
	/* if the transaction is a transfer, we check if the contra-transaction is not on the target account */

	if ( gsb_data_transaction_get_account_number_transfer (transaction_number) == target_account )
	{
	    dialogue_error ( _("Cannot move a transfer on his contra-account"));
	    return FALSE;
	}

	gsb_data_transaction_set_account_number_transfer ( contra_transaction_number,
							   target_account);
	gsb_transactions_list_update_transaction ( gsb_data_transaction_get_pointer_to_transaction (contra_transaction_number));
    }

    /* if it's a breakdown, move the children too */

    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp)
		 == 
		 transaction_number )
	    {
		list_tmp_transactions = list_tmp_transactions -> next;
		move_operation_to_account ( transaction_number_tmp,
					    target_account );
	    }
	    else
		list_tmp_transactions = list_tmp_transactions -> next;
	}
    }

    /* we remove the transaction from the tree_store */

    iter = gsb_transactions_list_get_iter_from_transaction (transaction_number);

    if ( iter )
    {
	gint i;

	/* change the selection if necessary */

	if ( gsb_data_account_get_current_transaction_number (source_account) == transaction_number)
	    gsb_data_account_set_current_transaction_number ( source_account,
							 gsb_data_transaction_get_transaction_number (cherche_operation_from_ligne ( cherche_ligne_operation ( gsb_data_account_get_current_transaction_number (source_account),
																			       source_account)
																     +
																     gsb_data_account_get_nb_rows (source_account),
																     source_account)));

	for ( i=0 ; i<gsb_data_account_get_nb_rows (source_account) ; i++ )
	    gtk_list_store_remove ( GTK_LIST_STORE ( gsb_transactions_list_get_store() ),
				    iter );

	/* update the colors and the store */

	gsb_transactions_list_set_background_color (source_account);
	gsb_transactions_list_set_transactions_balances (source_account);
    }

    /* we change now the account of the transaction */

    gsb_data_transaction_set_account_number ( transaction_number,
					      target_account );

    /* make the transaction in the tree_view */

    gsb_transactions_list_append_new_transaction (transaction_number);
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

    echeance = schedule_transaction ( gsb_data_transaction_get_pointer_to_transaction (gsb_data_account_get_current_transaction_number (gsb_data_account_get_current_account ())));

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
struct operation_echeance *schedule_transaction ( gpointer * transaction )
{
    struct operation_echeance *echeance;

    echeance = (struct operation_echeance *) calloc ( 1,
						      sizeof(struct operation_echeance) );
    if ( !echeance )
    {
	dialogue_error_memory ();
	return(FALSE);
    }

    echeance -> compte = gsb_data_transaction_get_account_number (gsb_data_transaction_get_transaction_number (transaction));
    echeance -> date = gsb_date_copy (gsb_data_transaction_get_date ( gsb_data_transaction_get_transaction_number ( transaction )));

    echeance -> montant = gsb_data_transaction_get_amount ( gsb_data_transaction_get_transaction_number (transaction ));
    echeance -> devise = gsb_data_transaction_get_currency_number ( gsb_data_transaction_get_transaction_number (transaction ));

    echeance -> tiers = gsb_data_transaction_get_party_number ( gsb_data_transaction_get_transaction_number (transaction ));
    echeance -> categorie = gsb_data_transaction_get_category_number ( gsb_data_transaction_get_transaction_number (transaction ));
    echeance -> sous_categorie = gsb_data_transaction_get_sub_category_number ( gsb_data_transaction_get_transaction_number (transaction ));

    /*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
    /* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement ni une ventil, compte_virement = -1 */
    /*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

    if ( gsb_data_transaction_get_transaction_number_transfer ( gsb_data_transaction_get_transaction_number (transaction )))
    {
	/* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

	gpointer contra_transaction;

	echeance -> compte_virement = gsb_data_transaction_get_account_number_transfer ( gsb_data_transaction_get_transaction_number (transaction ));

	contra_transaction = gsb_data_transaction_get_pointer_to_transaction (gsb_data_transaction_get_transaction_number_transfer (gsb_data_transaction_get_transaction_number (transaction)));

	if ( contra_transaction )
	    echeance -> type_contre_ope = gsb_data_transaction_get_method_of_payment_number ( gsb_data_transaction_get_transaction_number (contra_transaction ));
    }
    else
	if ( !echeance -> categorie
	     &&
	     !gsb_data_transaction_get_breakdown_of_transaction ( gsb_data_transaction_get_transaction_number (transaction )))
	    echeance -> compte_virement = -1;

    echeance -> notes = g_strdup ( gsb_data_transaction_get_notes ( gsb_data_transaction_get_transaction_number (transaction )));
    echeance -> type_ope = gsb_data_transaction_get_method_of_payment_number ( gsb_data_transaction_get_transaction_number (transaction ));
    echeance -> contenu_type = g_strdup ( gsb_data_transaction_get_method_of_payment_content ( gsb_data_transaction_get_transaction_number (transaction )));


    echeance -> no_exercice = gsb_data_transaction_get_financial_year_number ( gsb_data_transaction_get_transaction_number (transaction ));
    echeance -> imputation = gsb_data_transaction_get_budgetary_number ( gsb_data_transaction_get_transaction_number (transaction ));
    echeance -> sous_imputation = gsb_data_transaction_get_sub_budgetary_number ( gsb_data_transaction_get_transaction_number (transaction ));

    echeance -> operation_ventilee = gsb_data_transaction_get_breakdown_of_transaction ( gsb_data_transaction_get_transaction_number (transaction ));

    /*     par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
    /*     (c'est le cas, à 0 avec calloc) */
    /*     que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
    /* 	pour la même raison */

    echeance -> periodicite = 2;

    echeance -> no_operation = ++no_derniere_echeance;
    nb_echeances++;
    liste_struct_echeances = g_slist_insert_sorted ( liste_struct_echeances,
						     echeance,
						     (GCompareFunc) classement_sliste_echeance_par_date );

    /*     on récupère les opés de ventil si c'était une opé ventilée */

    if ( echeance -> operation_ventilee )
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (gsb_data_transaction_get_transaction_number (transaction))
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == gsb_data_transaction_get_transaction_number (transaction))
	    {
		struct operation_echeance *echeance_de_ventil;

		echeance_de_ventil = calloc ( 1,
					      sizeof ( struct operation_echeance));

		if ( !echeance_de_ventil )
		{
		    dialogue_error_memory ();
		    return(FALSE);
		}

		echeance_de_ventil -> compte = gsb_data_transaction_get_account_number (transaction_number_tmp);
		echeance_de_ventil -> date = gsb_date_copy (gsb_data_transaction_get_date (transaction_number_tmp));

		echeance_de_ventil -> montant = gsb_data_transaction_get_amount ( transaction_number_tmp);
		echeance_de_ventil -> devise = gsb_data_transaction_get_currency_number ( transaction_number_tmp);

		echeance_de_ventil -> tiers = gsb_data_transaction_get_party_number ( transaction_number_tmp);
		echeance_de_ventil -> categorie = gsb_data_transaction_get_category_number ( transaction_number_tmp);
		echeance_de_ventil -> sous_categorie = gsb_data_transaction_get_sub_category_number ( transaction_number_tmp);

		/*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
		/* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement, compte_virement = -1 */
		/*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

		if ( gsb_data_transaction_get_transaction_number_transfer ( transaction_number_tmp))
		{
		    /* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

		    gint contra_transaction_number;

		    echeance_de_ventil -> compte_virement = gsb_data_transaction_get_account_number_transfer (transaction_number_tmp);
		    contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number_tmp);

		    if ( contra_transaction_number )
			echeance_de_ventil -> type_contre_ope = gsb_data_transaction_get_method_of_payment_number (contra_transaction_number);
		}
		else
		    if ( !echeance_de_ventil -> categorie )
			echeance_de_ventil -> compte_virement = -1;

		echeance_de_ventil -> notes = g_strdup ( gsb_data_transaction_get_notes ( transaction_number_tmp));
		echeance_de_ventil -> type_ope = gsb_data_transaction_get_method_of_payment_number ( transaction_number_tmp);
		echeance_de_ventil -> contenu_type = g_strdup ( gsb_data_transaction_get_method_of_payment_content ( transaction_number_tmp));


		echeance_de_ventil -> no_exercice = gsb_data_transaction_get_financial_year_number ( transaction_number_tmp);
		echeance_de_ventil -> imputation = gsb_data_transaction_get_budgetary_number ( transaction_number_tmp);
		echeance_de_ventil -> sous_imputation = gsb_data_transaction_get_sub_budgetary_number ( transaction_number_tmp);

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
								 (GCompareFunc) classement_sliste_echeance_par_date );
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
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

    fenetre = gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view()));

    gdk_drawable_get_size ( GDK_DRAWABLE ( fenetre ),
			    &largeur,
			    &hauteur );

    if ( !gc_separateur_operation )
	gc_separateur_operation = gdk_gc_new ( GDK_DRAWABLE ( fenetre ));

    /*     si la hauteur des lignes n'est pas encore calculée, on le fait ici */

    hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( gsb_transactions_list_get_tree_view());

    /*     on commence par calculer la dernière ligne en pixel correspondant à la dernière opé de la liste */
    /* 	pour éviter de dessiner les traits en dessous */

    derniere_ligne = hauteur_ligne_liste_opes * GTK_LIST_STORE ( gsb_transactions_list_get_store()) -> length;
    hauteur = MIN ( derniere_ligne,
		    hauteur );

    /*     le plus facile en premier... les lignes verticales */
    /*     dépend de si on est en train de ventiler ou non */
    /*     on en profite pour ajuster nb_ligne_ope_tree_view */

    x=0;

    for ( i=0 ; i<TRANSACTION_LIST_COL_NB - 1 ; i++ )
    {
	x = x + gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( transactions_tree_view_columns[i]));
	gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			gc_separateur_operation,
			x - 1 , 0,
			x - 1, hauteur );
    }

    /*     les lignes horizontales : il faut calculer la position y de chaque changement d'opé à l'écran */
    /*     on calcule la position y de la 1ère ligne à afficher */

    if ( hauteur_ligne_liste_opes )
    {
	adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view()));

	y = ( hauteur_ligne_liste_opes * gsb_data_account_get_nb_rows ( gsb_data_account_get_current_account () ) ) * ( ceil ( adjustment->value / (hauteur_ligne_liste_opes* gsb_data_account_get_nb_rows ( gsb_data_account_get_current_account () )) )) - adjustment -> value;

	do
	{
	    if ( y )
		gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
				gc_separateur_operation,
				0, y, 
				largeur, y );
	    y = y + hauteur_ligne_liste_opes*gsb_data_account_get_nb_rows ( gsb_data_account_get_current_account () );
	}
	while ( y < ( adjustment -> page_size ) &&
		y < derniere_ligne );
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


	    active_sort = gsb_data_account_get_column_sort ( gsb_data_account_get_current_account (),
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

    gsb_data_account_set_column_sort ( gsb_data_account_get_current_account (),
				  GPOINTER_TO_INT ( no_column ),
				  GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( menu_item),
									"no_sort" )));
    sortable = GTK_TREE_SORTABLE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view())));


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
    gsb_transactions_list_sort_column_changed ( transactions_tree_view_columns[GPOINTER_TO_INT (no_column)]);
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
    /* 					   gsb_data_account_get_current_balance (gsb_data_account_get_current_account ()), */
    /* 					   devise_code_by_no ( gsb_data_account_get_currency (gsb_data_account_get_current_account ()) ))); */


    /* met le label du solde pointé */

    /*     gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ), */
    /* 			 g_strdup_printf ( _("Checked balance: %4.2f %s"), */
    /* 					   gsb_data_account_get_marked_balance (gsb_data_account_get_current_account ()), */
    /* 					   devise_code_by_no ( gsb_data_account_get_currency (gsb_data_account_get_current_account ()) ))); */
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
    /* 	if etat.retient_affichage_par_compte is set, only gsb_data_account_get_current_account () will change */
    /* 	else, all the accounts change */

    if ( affichage_r == gsb_data_account_get_r (gsb_data_account_get_current_account ()) )
	return;

    if ( DEBUG )
	printf ( "mise_a_jour_affichage_r afficher : %d\n", affichage_r );

    gsb_data_account_set_r (gsb_data_account_get_current_account (),
		       affichage_r );

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( !etat.retient_affichage_par_compte )
	    gsb_data_account_set_r ( i,
				affichage_r );

	list_tmp = list_tmp -> next;
    }
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

    if ( DEBUG )
	printf ( "mise_a_jour_affichage_lignes %d lignes\n", nb_lignes );

    /*     we check all the accounts */
    /* 	if etat.retient_affichage_par_compte is set, only gsb_data_account_get_current_account () will change */
    /* 	else, all the accounts change */

    if ( nb_lignes == gsb_data_account_get_nb_rows ( gsb_data_account_get_current_account () ) )
	return;

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( !etat.retient_affichage_par_compte
	     ||
	     i == gsb_data_account_get_current_account () )
	{
	    gsb_data_account_set_nb_rows ( i, 
				      nb_lignes );
	}
	list_tmp = list_tmp -> next;
    }

    /* we update the screen */

    gsb_transactions_list_set_visibles_rows_on_account (gsb_data_account_get_current_account ());
}
/******************************************************************************/


/** save the transactions store
 * \param store
 * */
void gsb_transactions_list_set_store (GtkListStore *store)
{
    transactions_store = store;
}

/** save the transactions filter_model
 * \param store
 * */
void gsb_transactions_list_set_filter (GtkTreeModel *filter_model)
{
    transactions_filter_model = filter_model;
}

/** save the transactions sortable_model
 * \param store
 * */
void gsb_transactions_list_set_sortable (GtkTreeModel *sortable_model)
{
    transactions_sortable_model = sortable_model;
}

/** save the transactions tree_view
 * \param store
 * */
void gsb_transactions_list_set_tree_view (GtkWidget *tree_view)
{
    transactions_tree_view = tree_view;
}


/** give the transactions store
 * \param store
 * */
GtkListStore *gsb_transactions_list_get_store (void)
{
    return (transactions_store);
}

/** give the transactions filter_model
 * \param store
 * */
GtkTreeModel *gsb_transactions_list_get_filter (void)
{
    return (transactions_filter_model);
}

/** give the transactions sortable_model
 * \param store
 * */
GtkTreeModel *gsb_transactions_list_get_sortable (void)
{
    return (transactions_sortable_model);
}

/** give the transactions tree_view
 * \param store
 * */
GtkWidget *gsb_transactions_list_get_tree_view (void)
{
    return (transactions_tree_view);
}




/** check all the lines of the account and set if they are shown or not
 * \param no_account the account we want to set
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_visibles_rows_on_account ( gint no_account )
{
    GtkTreeModel *sortable_model;
    GtkTreeIter iter;

    if ( DEBUG )
	printf ( "gsb_transactions_list_set_visibles_rows_on_account %d\n", no_account );

    sortable_model = GTK_TREE_MODEL (gsb_transactions_list_get_store ());

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( sortable_model ),
					 &iter ))
    {
	gint r_shown;
	gint nb_rows;

	r_shown = gsb_data_account_get_r ( no_account );
	nb_rows = gsb_data_account_get_nb_rows ( no_account );

	do
	{
	    gint no_transaction;
	    gpointer transaction;
	    gint current_line;
	    gint show;

	    /* 	    the current_line will contain 0, 1, 2 or 3 ; the line on the transaction */
	    /* 		we are on */

	    gtk_tree_model_get ( GTK_TREE_MODEL ( sortable_model ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
				 TRANSACTION_COL_NB_TRANSACTION_LINE, &current_line,
				 -1 );

	    no_transaction = gsb_data_transaction_get_transaction_number (transaction);

	    /* check the general white line (one for all the list, so no account number) */

	    if ( no_transaction == -1)
	    {
		if ( current_line < nb_rows )
		    show = TRUE;
		else
		    show = FALSE;

		gtk_list_store_set ( GTK_LIST_STORE ( sortable_model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, show,
				     -1 );
		continue;
	    }

	    /* check the account */

	    if ( gsb_data_transaction_get_account_number (no_transaction) != no_account )
	    {
		gtk_list_store_set ( GTK_LIST_STORE ( sortable_model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, FALSE,
				     -1 );
		continue;
	    }

	    /* 	    check first if it's R and if r is shown */

	    if ( gsb_data_transaction_get_marked_transaction (no_transaction)== OPERATION_RAPPROCHEE
		 &&
		 !r_shown )
	    {
		gtk_list_store_set ( GTK_LIST_STORE ( sortable_model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, FALSE,
				     -1 );
		continue;
	    }

	    /* if it's a breakdown, we do nothing, only the user choose
	     * what is shown or not */

	    if ( gsb_data_transaction_get_mother_transaction_number (no_transaction))
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

	    gtk_list_store_set ( GTK_LIST_STORE ( sortable_model ),
				 &iter,
				 TRANSACTION_COL_NB_VISIBLE, show,
				 -1 );
	}
	while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( sortable_model ),
					   &iter ));
    }
    return FALSE;
}

/** 
 * check if the transaction should be shown and do it, it's called when we add a new transaction
 * in the gtk list
 * \param transaction_number the transaction to check
 * \return TRUE if the transaction is shown, FALSE else
 * */
gboolean gsb_transactions_list_set_visibles_rows_on_transaction ( gint transaction_number )
{
    GtkListStore *model;
    GtkTreeIter *iter;
    gint nb_rows;
    gint i;

    model = gsb_transactions_list_get_store();
    iter = gsb_transactions_list_get_iter_from_transaction (transaction_number);

    if (!iter)
	return FALSE;

    /* begin to check with R */

    if ( !gsb_data_account_get_r ( gsb_data_transaction_get_account_number (transaction_number))
	 &&
	 gsb_data_transaction_get_marked_transaction ( transaction_number) == OPERATION_RAPPROCHEE )
    {
	/* that transaction shouldn't be shown, we hide it */

	for ( i=0 ; i< TRANSACTION_LIST_ROWS_NB; i++ )
	{
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_VISIBLE, FALSE,
				 -1 );
	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
	gtk_tree_iter_free ( iter );
	return FALSE;
    }

    /* if the transaction is a breakdown, we check with the mother
     * if it sould be shown or not */

    if ( gsb_data_transaction_get_mother_transaction_number ( transaction_number))
    {
	/* that transaction is a child breakdown, we show it or not, depends of the mother */

	GtkTreeIter *child_iter;
	gboolean mother_is_expanded;

	child_iter = gsb_transactions_list_get_iter_from_transaction (gsb_data_transaction_get_mother_transaction_number ( transaction_number));

	gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
			     child_iter,
			     TRANSACTION_COL_NB_IS_EXPANDED, &mother_is_expanded,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( model ),
			     iter,
			     TRANSACTION_COL_NB_VISIBLE, mother_is_expanded,
			     -1 );

	gtk_tree_iter_free ( child_iter );
	gtk_tree_iter_free ( iter );
	return mother_is_expanded;
    }

    /* now the transaction have to be shown completely or some lines */

    nb_rows = gsb_data_account_get_nb_rows ( gsb_data_transaction_get_account_number (transaction_number));

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
    return TRUE;
}


/** show or hide the breakdowns of the transaction given in param
 * and set the expander as we wish
 * \param mother_transaction_number the mother transaction
 * \param visible TRUE if we want to show the breakdowns
 * \return FALSE
 * */
gboolean gsb_data_account_list_set_breakdowns_visible ( gint mother_transaction_number,
						   gint visible )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());
    gtk_tree_model_get_iter_first ( model,
				    &iter );

    do
    {
	gpointer transaction_ptr;
	gint transaction_number;

	gtk_tree_model_get ( model,
			     &iter,
			     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_ptr,
			     -1 );

	transaction_number = gsb_data_transaction_get_transaction_number (transaction_ptr);

	/* if we are on the mother, we set the expander */

	if ( transaction_number == mother_transaction_number )
	    gtk_list_store_set ( GTK_LIST_STORE ( model ),
				 &iter,
				 TRANSACTION_COL_NB_IS_EXPANDED, visible,
				 -1 );

	/* if we are on a child, we show or not it */

	if ( gsb_data_transaction_get_mother_transaction_number (transaction_number) == mother_transaction_number)
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



/* get the real name of the category of the transaction
 * so return breakdown of transaction, transfer : ..., categ : under_categ
 * \param transaction the adr of the transaction
 * \return the real name
 * */
gchar *gsb_transactions_get_category_real_name ( gint transaction_number )
{
    gchar *tmp;

    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	tmp = _("Breakdown of transaction");
    else
    {
	if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number))
	{
	    /** it's a transfer */

	    if ( gsb_data_transaction_get_account_number_transfer (transaction_number)== -1 )
	    {
		if ( gsb_data_transaction_get_amount ( transaction_number)< 0 )
		    tmp = _("Transfer to a deleted account");
		else
		    tmp = _("Transfer from a deleted account");
	    }
	    else
	    {
		if ( gsb_data_transaction_get_amount (transaction_number)< 0 )
		    tmp = g_strdup_printf ( _("Transfer to %s"),
					    gsb_data_account_get_name ( gsb_data_transaction_get_account_number_transfer (transaction_number)) );
		else
		    tmp = g_strdup_printf ( _("Transfer from %s"),
					    gsb_data_account_get_name ( gsb_data_transaction_get_account_number_transfer (transaction_number)) );
	    }
	}
	else
	    /* it's a normal category */
	    tmp = gsb_data_category_get_name ( gsb_data_transaction_get_category_number (transaction_number),
					       gsb_data_transaction_get_sub_category_number (transaction_number),
					       NULL );
    }
    return tmp;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
