/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2007 Benjamin Drieu (bdrieu@april.org) 	      */
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
#include "gsb_transactions_list.h"
#include "accueil.h"
#include "comptes_traitements.h"
#include "erreur.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "gsb_form.h"
#include "gsb_form_transaction.h"
#include "fenetre_principale.h"
#include "navigation.h"
#include "barre_outils.h"
#include "gsb_real.h"
#include "gsb_reconcile.h"
#include "gsb_scheduler_list.h"
#include "utils_str.h"
#include "gsb_transaction_model.h"
#include "gsb_transactions_list_sort.h"
#include "main.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "parametres.h"
#include "utils.h"
#include "utils_operations.h"
#include "gsb_transactions_list.h"
#include "include.h"
#include "mouse.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean assert_selected_transaction ();
static gpointer cherche_operation_from_ligne ( gint ligne );
static void creation_titres_tree_view ( void );
static gint find_element_col ( gint element_number );
static gint find_element_line ( gint element_number );
static gint find_p_r_col ();
static gint find_p_r_line ();
static gboolean gsb_gui_change_cell_content ( GtkWidget * item, gint number );
static GtkWidget * gsb_gui_create_cell_contents_menu ( int x, int y );
static gboolean gsb_gui_update_row_foreach ( GtkTreeModel *model, GtkTreePath *path,
				      GtkTreeIter *iter, gint coords[2] );
static gboolean gsb_transactions_list_append_transaction ( gint transaction_number,
						    GtkTreeStore *store );
static gint gsb_transactions_list_append_white_line ( gint mother_transaction_number,
					       GtkTreeStore *store );
static gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
					      GdkEventButton *ev );
static void gsb_transactions_list_change_expanders ( gint only_current_account );
static gboolean gsb_transactions_list_change_sort_type ( GtkWidget *menu_item,
						  gint *no_column );
static gboolean gsb_transactions_list_check_mark ( gint transaction_number );
static gint gsb_transactions_list_choose_reconcile ( gint account_number,
					      gint selected_reconcile_number );
static GtkTreeStore *gsb_transactions_list_create_store ( void );
static GtkWidget *gsb_transactions_list_create_tree_view ( GtkTreeModel *model );
static void gsb_transactions_list_create_tree_view_columns ( void );
static gboolean gsb_transactions_list_current_transaction_down ( gint no_account );
static gboolean gsb_transactions_list_current_transaction_up ( gint no_account );
static gboolean gsb_transactions_list_delete_transaction_from_tree_view ( gint transaction_number );
static gboolean gsb_transactions_list_expand_row ( GtkTreeView *tree_view,
					    GtkTreeIter *iter_in_sort,
					    GtkTreePath *path,
					    gpointer null );
static gboolean gsb_transactions_list_fill_row ( gint transaction_number,
					  GtkTreeIter *iter,
					  GtkTreeStore *store,
					  gint line_in_transaction );
static gboolean gsb_transactions_list_fill_store ( GtkTreeStore *store );
static gint gsb_transactions_list_get_transaction_next ( gint transaction_number );
static gchar *gsb_transactions_list_grep_cell_content ( gint transaction_number,
						 gint cell_content_number );
static void gsb_transactions_list_set_filter (GtkTreeModel *filter_model);
static GtkTreeModel *gsb_transactions_list_set_filter_store ( GtkTreeStore *store );
static void gsb_transactions_list_set_sortable (GtkTreeModel *sortable_model);
static GtkTreeModel *gsb_transactions_list_set_sorting_store ( GtkTreeModel *filter_model );
static void gsb_transactions_list_set_tree_view (GtkWidget *tree_view);
static gboolean gsb_transactions_list_set_visibles_rows_on_transaction ( gint transaction_number );
static gboolean gsb_transactions_list_sort_column_changed ( GtkTreeViewColumn *tree_view_column );
static void gsb_transactions_list_swap_children ( GtkTreeIter *new_mother_iter,
					   GtkTreeIter *last_mother_iter );
static gboolean gsb_transactions_list_switch_R_mark ( gint transaction_number );
static gboolean gsb_transactions_list_switch_expander ( gint transaction_number );
static gboolean gsb_transactions_list_switch_mark ( gint transaction_number );
static gboolean gsb_transactions_list_title_column_button_press ( GtkWidget *button,
							   GdkEventButton *ev,
							   gint *no_column );
static gboolean move_operation_to_account ( gint transaction_number,
				     gint target_account );
static void move_selected_operation_to_account ( GtkMenuItem * menu_item );
static void popup_transaction_context_menu ( gboolean full, int x, int y );
static gint schedule_transaction ( gint transaction_number );
static gsb_real solde_debut_affichage ( gint no_account,
				 gint floating_point);
static void update_titres_tree_view ( void );
static gchar *gsb_transactions_list_grep_cell_content_trunc ( gint transaction_number,
							      gint cell_content_number );
/*END_STATIC*/




/*  adr du notebook qui contient les opés de chaque compte */
GtkWidget *tree_view_vbox;

/* the columns of the tree_view */
GtkTreeViewColumn *transactions_tree_view_columns[TRANSACTION_LIST_COL_NB];

/* adr de la barre d'outils */
GtkWidget *barre_outils;

/* contient les tips et titres des colonnes des listes d'opé */
gchar *tips_col_liste_operations[TRANSACTION_LIST_COL_NB];
gchar *titres_colonnes_liste_operations[TRANSACTION_LIST_COL_NB];

/* hauteur d'une ligne de la liste des opés */
gint hauteur_ligne_liste_opes;

/* on va essayer de créer un object tooltip général pour grisbi */
/* donc associer tous les autres tooltips à ce tooltip (FIXME) */
GtkTooltips *tooltips_general_grisbi;

/* le GdkGc correspondant aux lignes, créés au début une fois pour toute */
GdkGC *gc_separateur_operation;

/* utilisé pour éviter que ça s'emballe lors du réglage de la largeur des colonnes */
gint allocation_precedente;

/*  pointeur vers le label qui contient le solde sous la liste des opé */
GtkWidget *solde_label = NULL;

/*  pointeur vers le label qui contient le solde pointe sous la liste des opé */
GtkWidget *solde_label_pointe = NULL;

static GtkWidget *transactions_tree_view = NULL;
static GtkTreeStore *transactions_store = NULL;
static GtkTreeModel *transactions_filter_model = NULL;
static GtkTreeModel *transactions_sortable_model = NULL;


/** Determine how we display cells according to their type.  Only used
 * by gsb_transactions_list_grep_cell_content_trunc().  */
struct cell_view {
    gchar * name;		/* Name of the cell. */
    gint max_size;		/* Max size to display (will truncate
				 * the rest). */
    gboolean hard_trunc;	/* Should truncation be hard. */
} cell_views[] = {
    { "", 0, FALSE, },
    { N_("Date"), 16, FALSE },
    { N_("Value date"), 16, FALSE },
    { N_("Payee"), 32, FALSE },
    { N_("Budgetary line"), 32, FALSE },
    { N_("Debit"), 32, FALSE },
    { N_("Credit"), 32, FALSE },
    { N_("Balance"), 32, FALSE },
    { N_("Amount"), 32, FALSE },
    { N_("Method of payment"), 24, FALSE },
    { N_("Reconciliation reference"), 16, FALSE },
    { N_("Financial year"), 16, FALSE },
    { N_("Category"), 64, FALSE },
    { N_("C/R"), 2, FALSE },
    { N_("Voucher"), 10, TRUE },
    { N_("Notes"), 32, FALSE },
    { N_("Bank references"), 10, TRUE },
    { N_("Transaction number"), 10, TRUE },
    { N_("Cheque number"), 10, TRUE },
    { NULL, 0, FALSE },
};


/*START_EXTERN*/
extern GdkColor breakdown_background;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_selection;
extern GtkWidget *formulaire;
extern gchar *labels_boutons [] ;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GSList *liste_labels_titres_colonnes_liste_ope ;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget * navigation_tree_view;
extern GtkWidget *notebook_general;
extern PangoFontDescription *pango_desc_fonte_liste;
extern GtkTreeSelection * selection;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern GtkWidget *tree_view;
extern GtkWidget *window;
/*END_EXTERN*/




/**
 * save the transactions store
 * 
 * \param store
 * */
void gsb_transactions_list_set_store (GtkTreeStore *store)
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


/**
 * give the transactions store
 * 
 * \param store
 * */
GtkTreeStore *gsb_transactions_list_get_store (void)
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



/**
 * Create the transaction window with all components needed.
 *
 *
 */
GtkWidget *creation_fenetre_operations ( void )
{
    GtkWidget *win_operations;

    /*   la fenetre des opé est une vbox : la liste en haut, le solde et  */
    /*     des boutons de conf au milieu, le formulaire en bas */
    allocation_precedente = 0;

    win_operations = gtk_vbox_new ( FALSE, 6 );

    /* création de la barre d'outils */
    barre_outils = gtk_handle_box_new ();
    gsb_gui_update_transaction_toolbar ();
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

    gtk_widget_show (win_operations);
    return ( win_operations );
}


/**
 * change the selection to the transaction above
 * 
 * \param no_account
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_current_transaction_up ( gint no_account )
{
    gint transaction_number;
    GtkTreePath *path_sorted;

    devel_debug ( "gsb_transactions_list_current_transaction_up" );

    transaction_number = gsb_data_account_get_current_transaction_number ( no_account );

    /* first, get the sorted path and up it */
    path_sorted = gsb_transactions_list_get_sorted_path (transaction_number, 0);
    gtk_tree_path_prev ( path_sorted );

    transaction_number = gsb_transaction_model_get_transaction_from_sorted_path (path_sorted);

    /* if the transaction is a breakdown, we go on the white line of it */
    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number)
	 &&
	 gtk_tree_view_row_expanded ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
				      path_sorted ))
    {
	gtk_tree_path_down (path_sorted);

	do
	{
	    transaction_number = gsb_transaction_model_get_transaction_from_sorted_path (path_sorted);
	    gtk_tree_path_next ( path_sorted );
	}
	while ( transaction_number > 0 );
    }

    /* if there is no change with the current transaction, we check if we are not
     * in a child and must go to the mother */
    if ( transaction_number == gsb_data_account_get_current_transaction_number (no_account)
	 &&
	 gtk_tree_path_get_depth (path_sorted) == 2 )
    {
	gtk_tree_path_up (path_sorted);
	transaction_number = gsb_transaction_model_get_transaction_from_sorted_path (path_sorted);
    }

    gsb_transactions_list_select ( transaction_number );

    return FALSE;
}


/**
 * change the selection to the transaction below
 * 
 * \param no_account
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_current_transaction_down ( gint no_account )
{
    gint transaction_number;
    gint current_transaction_number;

    devel_debug ( "gsb_transactions_list_current_transaction_down");

    current_transaction_number = gsb_data_account_get_current_transaction_number (no_account);

    transaction_number = gsb_transactions_list_get_transaction_next (current_transaction_number);

    gsb_transactions_list_select ( transaction_number);
    return FALSE;
}


/**
 * Return the next transaction after the one in param
 * in the tree_view
 * 
 * \param transaction_number The transaction we want to move down
 * 
 * \return the number of the new transaction or 0 if problem
 * */
gint gsb_transactions_list_get_transaction_next ( gint transaction_number )
{
    GtkTreePath *path_sorted;
    gint account_number;
    gint i;

    /* if we are on the white line, do nothing */
    if ( transaction_number == -1)
	return 0;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    path_sorted = gsb_transactions_list_get_sorted_path (transaction_number, 0);

    /* if the transaction is a breakdown child, the line is only 1 line,
     * and if we are on the white line, we have to go to the next transaction */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
	if ( transaction_number < 0 )
	{
	    gtk_tree_path_up (path_sorted);
	    for ( i=0 ; i<gsb_data_account_get_nb_rows (account_number) ; i++ )
		gtk_tree_path_next ( path_sorted );
	}
	else
	    gtk_tree_path_next ( path_sorted );
    }
    else
    {
	/* if the current transaction is a breakdown, we need to go in the child,
	 * first go to the last line and check if we go to the child or
	 * next transaction */

	for ( i=0 ; i<(gsb_data_account_get_nb_rows (account_number) - 1); i++ )
	    gtk_tree_path_next ( path_sorted );

	if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number)
	     &&
	     gtk_tree_view_row_expanded ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
					  path_sorted ))
	    gtk_tree_path_down (path_sorted);
	else
	    gtk_tree_path_next ( path_sorted );
    }

    return gsb_transaction_model_get_transaction_from_sorted_path (path_sorted);
}



/**
 * create fully the gui list and fill it
 * 
 * \param
 * 
 * \return the widget which contains the list, to set at the right place
 * */
GtkWidget *gsb_transactions_list_make_gui_list ( void )
{
    GtkWidget *tree_view;
    GtkTreeStore *store;
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

    gsb_transactions_list_set_filter (filter_model);
    gsb_transactions_list_set_sortable (sortable_model);
    gsb_transactions_list_set_tree_view (tree_view);

    gtk_widget_show (scrolled_window);
    return scrolled_window;
}


/**
 * called by a click on a column title ; the sort of the list is automatic,
 * that function make the background color and the rest to be updated
 *
 * \param tree_view_column the tree_view_column clicked
 * \return FALSE
 * */
gboolean gsb_transactions_list_sort_column_changed ( GtkTreeViewColumn *tree_view_column )
{
    GtkSortType sort_type;
    gint sort_column_id;
    gint account_number;
    GtkTreeSortable *sortable;
    gint new_column;
    GSList *tmp_list;

    devel_debug ( "gsb_transactions_list_sort_column_changed" );

    account_number = gsb_gui_navigation_get_current_account ();
    sortable = GTK_TREE_SORTABLE (gsb_transactions_list_get_sortable());

    gtk_tree_sortable_get_sort_column_id ( sortable,
					   &sort_column_id,
					   &sort_type );

    /* if sort_column_id == -1, it's because we changed 3 times the direction,
     * and there is now no column sort, so we get back to the last column */
    if (sort_column_id  == -1)
	new_column = gtk_tree_view_column_get_sort_column_id ( tree_view_column );
    else
	new_column = sort_column_id;

    /* now have to save the new column and sort type in the account
     * or in all account if global conf for all accounts */
    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
	gint tmp_account;

	tmp_account = gsb_data_account_get_no_account (tmp_list -> data);

	if (tmp_account == account_number
	    ||
	    !etat.retient_affichage_par_compte)
	{
	    /* set the new column to sort */
	    gsb_data_account_set_sort_column ( tmp_account,
					       new_column );
	    /* save the sort_type */
	    gsb_data_account_set_sort_type ( tmp_account,
					     sort_type );
	}
	tmp_list = tmp_list -> next;
    }
 
    /* the third click give no sort for the column
     * we forbid that here, so now, there is always a column which sort the tree view */
    if ( sort_column_id == -1 )
    {
	gtk_tree_sortable_set_sort_column_id ( sortable,
					       new_column,
					       sort_type );

	return FALSE;
    }

    gsb_transactions_list_set_background_color (account_number);
    gsb_transactions_list_set_transactions_balances (account_number);

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
	COLUMN_CENTER, COLUMN_CENTER, COLUMN_LEFT, 
	COLUMN_CENTER, COLUMN_RIGHT, COLUMN_RIGHT, COLUMN_RIGHT
    };
    gint column_balance;

    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	GtkCellRenderer *cell_renderer;

	cell_renderer = gtk_cell_renderer_text_new ();

	transactions_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes ( _(titres_colonnes_liste_operations[i]),
										       cell_renderer,
										       "text", i,
										       "cell-background-gdk", TRANSACTION_COL_NB_BACKGROUND,
										       "font-desc", TRANSACTION_COL_NB_FONT,
										       NULL );

	gtk_tree_view_column_set_alignment ( transactions_tree_view_columns[i],
					     alignment[i] );
	gtk_tree_view_column_set_sizing ( transactions_tree_view_columns[i],
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_sort_column_id ( transactions_tree_view_columns[i], i );
	gtk_tree_view_column_set_expand ( transactions_tree_view_columns[i], TRUE );
	gtk_tree_view_column_set_resizable ( transactions_tree_view_columns[i], TRUE );

	gtk_tree_view_column_set_min_width ( transactions_tree_view_columns[i], 1 );
    }

    /* get the position of the amount column to set it in red */

    column_balance = find_element_col (TRANSACTION_LIST_BALANCE);

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
    devel_debug ( g_strdup_printf ("remplissage_liste_operations compte %d", compte ));

    /* FIXME  cette fonction est à virer */

    /*     on efface la liste */

    gtk_tree_store_clear ( gsb_transactions_list_get_store()  );

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

/**
 * create the store which contains the transactions
 * it only make the main store, no filter and no sort, the purpose
 * is to fill it just after to increase the speed, and only after
 * we can connect a filter, a sorting function and a tree_view
 * 
 * \param
 * 
 * \return the store
 * */
GtkTreeStore *gsb_transactions_list_create_store ( void )
{
    GtkTreeStore *store;

    devel_debug ( "gsb_transactions_list_create_store" );

    /* definitions of the columns in operations_liste.h */

    store = gtk_tree_store_new ( TRANSACTION_COL_NB_TOTAL,
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
				 G_TYPE_BOOLEAN );

    gsb_transactions_list_set_store (store);

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
GtkTreeModel *gsb_transactions_list_set_filter_store ( GtkTreeStore *store )
{
    GtkTreeModel *filter_model;

    devel_debug ( "gsb_transactions_list_set_filter_store" );

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
    gint i;

    devel_debug ("gsb_transactions_list_set_sorting_store");

    /* make the model_sort, on the model_filter */
    sortable_model = gtk_tree_model_sort_new_with_model ( filter_model );

    /*     set the compare functions by click on the column */
    for (i=0 ; i<TRANSACTION_LIST_COL_NB ; i++)
	gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( sortable_model ),
					  i,
					  (GtkTreeIterCompareFunc) gsb_transactions_list_sort_column,
					  GINT_TO_POINTER (i),
					  NULL );
    return sortable_model;
}


/**
 * create a tree view from a filled, filtred and sorted store,
 * 
 * \param store the store
 * 
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

    /* we have to check when a row is expanded (breakdown child) because the filter doesn't show the
     * background and the row when we set the show_column to TRUE and the expander is closed,
     * so, we set to TRUE the show_column and set the background color at the opening of the expander */
    g_signal_connect ( G_OBJECT ( tree_view ),
		       "row-expanded",
		       G_CALLBACK ( gsb_transactions_list_expand_row ),
		       NULL );

    /* we create the columns of the tree view */

    gsb_transactions_list_create_tree_view_columns ();

    for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
    {
	gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
				      transactions_tree_view_columns[i] );

	gtk_tree_view_column_set_clickable ( transactions_tree_view_columns[i],
					     TRUE );

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


/**
 * fill the new store with the all the transactions
 * normally called at the opening of a file
 * 
 * \param store the store to fill (normally... it exists only one...)
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_store ( GtkTreeStore *store )
{
    GSList *transactions_list;

    devel_debug ( "gsb_transactions_list_fill_store" );

    transactions_list = gsb_data_transaction_get_transactions_list ();

    /* add all the transactions */

    while (transactions_list)
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (transactions_list -> data);

	/* normally fill only the non-R transactions, which is much faster,
	 * except if the user choose to load them at the begining */

	if ( !etat.fill_r_at_begining
	     &&
	     gsb_data_transaction_get_marked_transaction (transaction_number) == 3 )
	{
	    transactions_list = transactions_list -> next;
	    continue;
	}

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



/**
 * Create a white line and append it to the gtk list of transactions
 * if there is a mother, do as for breakdowns children, so only 1 line
 *
 * \param mother_transaction_number The mother of the white breakdown or 0 for the last white line
 * \param store
 *
 * \return the number of the white line; 
 * */
gint gsb_transactions_list_append_white_line ( gint mother_transaction_number,
					       GtkTreeStore *store )
{
    gint line;
    gint nb_lines;
    gint white_line_number;
    GtkTreeIter *mother_iter;

    devel_debug ( g_strdup_printf ( "gsb_transactions_list_append_white_line ; mother : %d", mother_transaction_number));

    white_line_number = gsb_data_transaction_new_white_line (mother_transaction_number);

    /* if it's a breakdown, we set it as a child so find the mother here */
    if ( mother_transaction_number )
    {
	mother_iter = gsb_transaction_model_get_iter_from_transaction ( mother_transaction_number,
									gsb_data_account_get_nb_rows ( gsb_data_transaction_get_account_number (mother_transaction_number)) - 1);
	nb_lines = 1;
    }
    else
    {
	mother_iter = NULL;
	nb_lines = TRANSACTION_LIST_ROWS_NB;
    }


    for ( line = 0 ; line < nb_lines ; line++ )
    {
	GtkTreeIter iter;

	gtk_tree_store_append ( store,
				&iter,
				mother_iter );

	/* draw the first column for the general white line */
	/* set the number of line in the transaction */
	/* set the address of the transaction */

	gtk_tree_store_set ( store,
			     &iter,
			     TRANSACTION_COL_NB_TRANSACTION_LINE, line,
			     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, gsb_data_transaction_get_pointer_to_transaction (white_line_number),
			     TRANSACTION_COL_NB_VISIBLE, TRUE,
			     -1 );
    }
    return white_line_number;
}


/** 
 * append a new transaction in the tree_view
 * if the transaction is a breakdown, append a white line and open
 * the breakdown to see the daughters
 *
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number )
{
    GtkTreeStore *store;
    gint account_number;
    
    devel_debug ( g_strdup_printf ("gsb_transactions_list_append_new_transaction %d",
				   transaction_number ));

    store = gsb_transactions_list_get_store ();

    if ( !store)
	return FALSE;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* append the transaction to the tree view */
    gsb_transactions_list_append_transaction ( transaction_number,
					       store);

    /* if the transaction is a breakdown mother, we happen a white line,
     * which is a normal transaction but with nothing and with the breakdown
     * relation to the last transaction */
    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	gsb_transactions_list_append_white_line ( transaction_number,
						  store);

    /*  set the new current balance */
    gsb_data_account_set_current_balance ( account_number,
					   gsb_real_add ( gsb_data_account_get_current_balance (account_number),
							  gsb_data_transaction_get_adjusted_amount (transaction_number, -1)));

    /* update the transaction list only if the account is showed,
     * else it's because we execute a scheduled transaction and all
     * of that stuff will be done when we will show the account */
    if (gsb_form_get_origin () == gsb_data_transaction_get_account_number (transaction_number))
    {
	gsb_transactions_list_set_visibles_rows_on_transaction (transaction_number);
	gsb_transactions_list_set_background_color (account_number);
	gsb_transactions_list_set_transactions_balances (account_number);
	gsb_transactions_list_move_to_current_transaction (account_number);

	gsb_gui_headings_update_suffix ( gsb_real_format_currency_from_locale ( gsb_data_account_get_current_balance (account_number) ) );
    }

    /* on réaffichera l'accueil */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}


/* xxx voir la différence entre ces 2 fonctions et en virer une si nécessaire */
/**
 * append the transaction to the store of the account
 * make the 4 lines, after the filter will decide if we show them or not
 * 
 * \param transaction_number
 * \param store
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_append_transaction ( gint transaction_number,
						    GtkTreeStore *store )
{
    gint line;
    GtkTreeIter *mother_iter;
    gint mother_transaction_number;
    gint nb_lines;

    /* if it's the child of a breakdown, we append it at the last line of the mother,
     * wich depends of the account ...
     * and there is only 1 line */

    mother_transaction_number = gsb_data_transaction_get_mother_transaction_number (transaction_number);

    if (mother_transaction_number)
    {
	mother_iter = gsb_transaction_model_get_iter_from_transaction ( mother_transaction_number,
									gsb_data_account_get_nb_rows ( gsb_data_transaction_get_account_number (mother_transaction_number)) -1 );
	nb_lines = 1;
    }
    else
    {
	mother_iter = NULL;
	nb_lines = TRANSACTION_LIST_ROWS_NB;
    }

    for ( line = 0 ; line < nb_lines ; line++ )
    {
	GtkTreeIter iter;

	gtk_tree_store_append ( store,
				&iter,
				mother_iter );

	gsb_transactions_list_fill_row ( transaction_number,
					 &iter,
					 store,
					 line );
    }

    if ( mother_iter )
	gtk_tree_iter_free (mother_iter);

    return FALSE;
}



/**
 * fill the row given in param by iter
 * 
 * \param transaction_number
 * \param iter the iter corresponding to the row
 * \param store
 * \param line_in_transaction
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_row ( gint transaction_number,
					  GtkTreeIter *iter,
					  GtkTreeStore *store,
					  gint line_in_transaction )
{
    gint column;

    for ( column = 0 ; column < TRANSACTION_LIST_COL_NB ; column++ )
    {
	gchar *string;

	string = gsb_transactions_list_grep_cell_content_trunc ( transaction_number,
								 tab_affichage_ope[line_in_transaction][column] );
	gtk_tree_store_set ( store,
			     iter,
			     column, string,
			     -1 );
	if (string)
	    g_free (string);
    }

    /* if we use a custom font... */
    if ( etat.utilise_fonte_listes )
	gtk_tree_store_set ( store,
			     iter,
			     TRANSACTION_COL_NB_FONT, pango_desc_fonte_liste,
			     -1 );

    /* draw the first column for all transactions, except for children of breakdowns */
    /* set the number of line in the transaction */
    /* set the address of the transaction */
    gtk_tree_store_set ( store,
			 iter,
			 TRANSACTION_COL_NB_TRANSACTION_LINE, line_in_transaction,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, gsb_data_transaction_get_pointer_to_transaction (transaction_number),
			 -1 );

    return FALSE;
}


/**
 * Take in a transaction the content to set in a cell of the
 * transaction's list.  Truncate to a fixed size to avoid taking too
 * much space.
 * 
 * \param transaction_number
 * \param cell_content_number what we need in the transaction
 * 
 * \return the content of the transaction, in gchar, need to be freed after
 * */
gchar *gsb_transactions_list_grep_cell_content_trunc ( gint transaction_number,
						       gint cell_content_number )
{
    return gsb_string_truncate_n ( gsb_transactions_list_grep_cell_content ( transaction_number, cell_content_number ),
				   cell_views [ cell_content_number ] . max_size,
				   cell_views [ cell_content_number ] . hard_trunc );
}



/**
 * take in a transaction the content to set in a cell of the transaction's list
 * 
 * \param transaction_number
 * \param cell_content_number what we need in the transaction
 * 
 * \return the content of the transaction, in gchar, need to be freed after
 * */
gchar *gsb_transactions_list_grep_cell_content ( gint transaction_number,
						 gint cell_content_number )
{

    /* FIXME: this function is a massive memory leaker !  -- benj */

    gint account_currency;

    /* for a child of breakdown, we show only the category (instead of party or category), the
     * debit and credit, nothing else */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
	/* FIXME : if the user show the payee and the category both on the 1st line,
	 * we will have the categ showed 2 times for a breakdown child. rare, but
	 * possible... try to fix this ? */
	switch (cell_content_number)
	{
	    case TRANSACTION_LIST_PARTY:
		/* here want to show the category instead of the payee */
		cell_content_number = TRANSACTION_LIST_CATEGORY;
		break;

	    case TRANSACTION_LIST_CATEGORY:
	    case TRANSACTION_LIST_DEBIT:
	    case TRANSACTION_LIST_CREDIT:
		break;

	    default:
		return NULL;
	}
    }

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
	    return ( my_strdup (gsb_data_payee_get_name ( gsb_data_transaction_get_party_number (transaction_number), TRUE )));

	    /* mise en forme de l'ib */

	case TRANSACTION_LIST_BUDGET:

	    return ( my_strdup (gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number (transaction_number),
							   gsb_data_transaction_get_sub_budgetary_number (transaction_number),
							   NULL )));

	case TRANSACTION_LIST_DEBIT:
	case TRANSACTION_LIST_CREDIT:
	    /* give the amount of the transaction on the transaction currency */
	    if ( (cell_content_number == TRANSACTION_LIST_DEBIT
		  &&
		  gsb_data_transaction_get_amount ( transaction_number).mantissa < 0 )
		 ||
		 (cell_content_number == TRANSACTION_LIST_CREDIT
		  &&
		  gsb_data_transaction_get_amount ( transaction_number).mantissa >= 0 ))
	    {
		if ( gsb_data_transaction_get_currency_number (transaction_number) != gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number)))
		    return ( g_strconcat ( gsb_real_get_string ( gsb_data_transaction_get_amount (transaction_number)),
					   "(",
					   gsb_data_currency_get_code (gsb_data_transaction_get_currency_number (transaction_number)),
					   ")",
					   NULL ));
		else
		    return gsb_real_format_currency_from_locale ( gsb_data_transaction_get_amount ( transaction_number ) );
	    }
	    else
		return NULL;
	    break;

	case TRANSACTION_LIST_BALANCE:
	    return NULL;

	case TRANSACTION_LIST_AMOUNT:
	    /* give the amount of the transaction in the currency of the account */

	    account_currency = gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number));
	    if ( account_currency != gsb_data_transaction_get_currency_number (transaction_number))
		return ( g_strconcat ( "(",
				       gsb_real_get_string (gsb_data_transaction_get_adjusted_amount (transaction_number,
												      gsb_data_currency_get_floating_point (account_currency))),
				       gsb_data_currency_get_code (account_currency),
				       ")",
				       NULL ));
	    else 
		return NULL;
	    break;

	    /* mise en forme du moyen de paiement */

	case TRANSACTION_LIST_TYPE:
	    return ( my_strdup (gsb_data_payment_get_name ( gsb_data_transaction_get_method_of_payment_number ( transaction_number))));

	    /* mise en forme du no de rapprochement */

	case TRANSACTION_LIST_RECONCILE_NB:
	    return ( my_strdup (gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( transaction_number))));

	    /* mise en place de l'exo */

	case TRANSACTION_LIST_EXERCICE:
	    return ( my_strdup (gsb_data_fyear_get_name (gsb_data_transaction_get_financial_year_number ( transaction_number))));

	    /* mise en place des catégories */

	case TRANSACTION_LIST_CATEGORY:

	    return ( my_strdup (gsb_transactions_get_category_real_name ( transaction_number )));

	    /* mise en forme R/P */

	case TRANSACTION_LIST_MARK:
	    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 1 )
		return ( my_strdup (_("P")));
	    else
	    {
		if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 2 )
		    return ( my_strdup (_("T")));
		else
		{
		    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== 3 )
			return ( my_strdup (_("R")));
		    else
			return ( NULL );
		}
	    }
	    break;

	    /* mise en place de la pièce comptable */

	case TRANSACTION_LIST_VOUCHER:
	    return ( my_strdup (gsb_data_transaction_get_voucher ( transaction_number)));

	    /* mise en forme des notes */

	case TRANSACTION_LIST_NOTES:
	    return ( my_strdup (gsb_data_transaction_get_notes ( transaction_number)));

	    /* mise en place de l'info banque/guichet */

	case TRANSACTION_LIST_BANK:
	    return ( my_strdup (gsb_data_transaction_get_bank_references ( transaction_number)));

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

/** 
 * update the transaction given in the tree_view
 * 
 * \param transaction transaction to update
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_update_transaction ( gint transaction_number )
{
    gint j;
    GtkTreeStore *store;
    GtkTreeIter *iter;
    gint account_number;

    devel_debug ( g_strdup_printf ( "gsb_transactions_list_update_transaction no %d",
				    transaction_number));

    account_number = gsb_data_transaction_get_account_number (transaction_number);
    store = gsb_transactions_list_get_store();
    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number,
							    0 );

    if (iter)
    {
	for ( j = 0 ; j < TRANSACTION_LIST_ROWS_NB ; j++ )
	{
	    gsb_transactions_list_fill_row ( transaction_number,
					     iter,
					     store,
					     j );

	    /* if it's a breakdown, there is only 1 line */
	    if ( transaction_number != -1
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number))
		j = TRANSACTION_LIST_ROWS_NB;

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL (store),
				       iter );
	}
	gtk_tree_iter_free ( iter );
    }

    gsb_transactions_list_set_transactions_balances (account_number);

    /*     calcul du solde courant */
    gsb_data_account_set_current_balance ( account_number,
					   gsb_real_add ( gsb_data_account_get_current_balance (account_number),
							  gsb_data_transaction_get_adjusted_amount (transaction_number, -1)));

    /* on met à jour les labels des soldes */
    gsb_gui_headings_update_suffix ( g_strdup_printf ( "%s %s", 
						       gsb_real_get_string (gsb_data_account_get_current_balance (account_number)),
						       gsb_data_currency_get_code (gsb_data_account_get_currency (account_number))));

    /* on réaffichera l'accueil */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}



/**
 * update the given element for all the transactions in the tree_model
 * this is usefull when we have to change one thing for a lot of transactions,
 * ie for example, changing a payee name or a payment method name
 *
 * \param element_number
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_update_transaction_value ( gint element_number )
{
    gint column_element;
    gint line_element;
    GtkTreeModel *model;
    GtkTreeIter iter;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_update_transaction_value"));

    /* for now, this is the same position for all accounts, so no problem
     * later we will be able to change the position for each account, so at this
     * time we will only be able to change for one account (ex method of payment),
     * and repeat that function for all accounts if necessary (ex payees) */

    /* get the position of the element */
    column_element = find_element_col (element_number);
    line_element = find_element_line (element_number);

    if ( column_element == -1
	 ||
	 line_element == -1 )
	return FALSE;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());
    if (gtk_tree_model_get_iter_first (model, &iter))
    {
	gboolean next_ok = TRUE;

	do
	{
	    gpointer transaction_ptr;
	    gint transaction_number;
	    gchar *string;
	    gint i;

	    gtk_tree_model_get ( model,
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_ptr,
				 -1 );
	    transaction_number = gsb_data_transaction_get_transaction_number (transaction_ptr);
	    if (transaction_number > 0)
	    {
		string = gsb_transactions_list_grep_cell_content_trunc ( transaction_number,
									 element_number );

		/* go to the good line */
		for (i=0 ; i<line_element ; i++)
		    next_ok = gtk_tree_model_iter_next ( model, &iter );

		gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				     &iter,
				     column_element, string,
				     -1 );
		if (string)
		    g_free (string);

		/* go to the next transaction */
		for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB - line_element ; i++ )
		    next_ok = gtk_tree_model_iter_next ( model, &iter );
	    }
	    else
		/* we are on the white line, at the end */
		next_ok = FALSE;
	}
	while (next_ok);
    }
    return FALSE;
}


/** 
 * set the alterance color of the background or the transactions list for the given account
 * at the end, select the current transaction
 * we don't go into the children because it doesn't work when they are not expanded,
 * so the color will be set at the opening of the expander
 * 
 * \param no_account account
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_background_color ( gint no_account )
{
    GtkTreeModel *model;
    GtkTreePath *path_sort;
    GtkTreePath *path;
    gint current_color = 0;
    gint i = 0;
    gint nb_rows_by_transaction;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_set_background_color :  no_account %d", no_account ));

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());
    nb_rows_by_transaction = gsb_data_account_get_nb_rows ( no_account );

    path_sort = gtk_tree_path_new_first ();

    while (( path = gsb_transaction_model_get_model_path_from_sorted_path ( path_sort )))
    {
	GtkTreeIter iter;
	gpointer transaction;
	gint transaction_number;
	gint color_column;
	gint line_in_transaction;

	gtk_tree_model_get_iter ( GTK_TREE_MODEL ( model ),
				  &iter,
				  path );
	gtk_tree_model_get ( model,
			     &iter,
			     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			     TRANSACTION_COL_NB_TRANSACTION_LINE, &line_in_transaction,
			     -1 );

	transaction_number = gsb_data_transaction_get_transaction_number (transaction);

	/* if we are on the selection, set the selection's color and save the normal
	 * color */

	if ( transaction_number == gsb_data_account_get_current_transaction_number (no_account))
	{
	    color_column = TRANSACTION_COL_NB_SAVE_BACKGROUND;
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 &iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur_selection,
				 -1 );
	}
	else
	    color_column = TRANSACTION_COL_NB_BACKGROUND;

	gtk_tree_store_set ( GTK_TREE_STORE ( model ),
			     &iter,
			     color_column, &couleur_fond[current_color],
			     -1 );

	if ( ++i == nb_rows_by_transaction)
	{
	    i = 0;
	    current_color = !current_color;
	}

	/* needn't to go in a child because the color is always the same, so
	 * gtk_tree_path_next is enough */
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
    gsb_real current_total;
    gint column_balance;
    gint line_balance;
    GtkTreePath *path_sorted;
    GtkTreeModel *model;
    gint floating_point;
    gint transaction_number;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_set_transactions_balances, no_account : %d", no_account ));

    /* column and line of balance are user defined */
    column_balance = find_element_col (TRANSACTION_LIST_BALANCE);
    line_balance = find_element_line (TRANSACTION_LIST_BALANCE);

    if ( line_balance == -1 
	 ||
	 line_balance >= gsb_data_account_get_nb_rows ( no_account ))
    {
	return FALSE;
    }

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    floating_point = gsb_data_currency_get_floating_point (gsb_data_account_get_currency (no_account));

    /* get the begining balance */
    current_total = solde_debut_affichage ( no_account,
					    floating_point);

    /* path sorted is the path in the sorted list */
    path_sorted = gtk_tree_path_new_first ();
    transaction_number = gsb_transaction_model_get_transaction_from_sorted_path (path_sorted);

    /* continue transaction by transaction untill the white line */
    while (transaction_number != -1)
    {
	GtkTreeIter model_iter;
	gint i;
	gchar *string;
	gchar *color;
	GtkTreePath *path;

	/* if it's a breakdown, we do nothing,
	 * normally should not come here but protect that shouldn't lose a lot of time */
	if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
	{
	    gtk_tree_path_next ( path_sorted );
	    transaction_number = gsb_transaction_model_get_transaction_from_sorted_path (path_sorted);
	    continue;
	}

	/* calculate the new balance */
	current_total = gsb_real_add ( current_total,
				       gsb_data_transaction_get_adjusted_amount (transaction_number, floating_point));
	string = gsb_real_format_currency_from_locale ( current_total );

	/* set the color */
	if ( current_total.mantissa >= 0 )
	    color = NULL;
	else
	    color = "red";

	/* and set it in the list */
	path= gsb_transaction_model_get_model_path_from_sorted_path (path_sorted);
	gtk_tree_model_get_iter ( model,
				  &model_iter,
				  path );
	for (i=0 ; i<line_balance ; i++)
	    gtk_tree_model_iter_next ( model, &model_iter );

	gtk_tree_store_set ( GTK_TREE_STORE ( model ),
			     &model_iter,
			     column_balance, string,
			     TRANSACTION_COL_NB_AMOUNT_COLOR, color,
			     -1 );
	g_free (string);
	gtk_tree_path_free (path);

	/* go to the next transaction */
	for ( i=0 ; i<gsb_data_account_get_nb_rows ( no_account ) - line_balance ; i++ )
	    gtk_tree_path_next ( path_sorted );

	transaction_number = gsb_transaction_model_get_transaction_from_sorted_path (path_sorted);
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
    GtkTreePath *path;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_set_adjustment_value account %d",
				   account_number ));

    path = gsb_data_account_get_vertical_adjustment_value (account_number);

    if (!path)
    {
	gint cell;

	/* no path defined, scroll to the end of the list */
	cell = (gsb_data_transaction_get_number_by_account (account_number, FALSE, FALSE) + 1) * gsb_data_account_get_nb_rows (account_number) - 1;
	path = gtk_tree_path_new_from_indices (cell, -1);
	gsb_data_account_set_vertical_adjustment_value ( account_number,
							 path);
    }

    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view()),
				   path,
				   NULL,
				   FALSE, 0.0, 0.0 );
    gtk_tree_path_free (path);

    return FALSE;
}



/** find column number for the transaction element number
 *
 * \param element_number the element we look for
 *
 * \return column number
 * */
gint find_element_col ( gint element_number )
{
    gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( tab_affichage_ope[i][j] == element_number )
		return j;
	}
    }

    return -1;
}

/** find line number for the transaction element number
 *
 * \param element_number the element we look for
 *
 * \return line number
 * */
gint find_element_line ( gint element_number )
{
    gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( tab_affichage_ope[i][j] == element_number )
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
gsb_real solde_debut_affichage ( gint no_account,
				 gint floating_point)
{
    gsb_real solde;
    GSList *list_tmp_transactions;

    solde = gsb_data_account_get_init_balance (no_account,
					       floating_point );

    if ( gsb_data_account_get_r (no_account) )
	return solde;

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
	    solde = gsb_real_add ( solde,
				   gsb_data_transaction_get_adjusted_amount (transaction_number_tmp,
									     floating_point ));

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

    /* first, give the focus to the list */
    gtk_widget_grab_focus (tree_view);

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

    path = gsb_transaction_model_get_model_path_from_sorted_path (path_sorted);
    model = GTK_TREE_MODEL ( gsb_transactions_list_get_store());

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get ( model, &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 TRANSACTION_COL_NB_TRANSACTION_LINE, &line_in_transaction,
			 -1 );
    column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
			    tree_column );

    transaction_number = gsb_data_transaction_get_transaction_number (transaction);

    /* we select the transaction */
    gsb_transactions_list_select ( transaction_number );

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
    if ( transaction_number != -1
	 &&
	 column == find_p_r_col()
	 &&
	 (( etat.equilibrage
	    &&
	    line_in_transaction == find_p_r_line())
	  ||
	  (( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )))
    {
	gsb_transactions_list_switch_mark (transaction_number);
	return TRUE;
    }

    /*  if double - click */
    if ( ev -> type == GDK_2BUTTON_PRESS )
	gsb_transactions_list_edit_transaction (transaction_number);

    /* have to return FALSE to continue the signal and open eventually
     * a child in the tree... but returning FALSE make the selection slower...
     * so check here x of mouse, if <16, so at left of list, perhaps it's
     * for open a child, so return FALSE, in other case return TRUE because
     * much much faster. next way, perhaps check if it's for open and do it
     * manually ??? */
    if ( ev -> x <16 )
	return FALSE;

    return TRUE;
}

/******************************************************************************/
/* Fonction gsb_transactions_list_key_press */
/* gère le clavier sur la liste des opés */
/******************************************************************************/
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev )
{
    gint account_number;
    gint transaction_number;

    account_number = gsb_gui_navigation_get_current_account ();

    switch ( ev -> keyval )
    {
	case GDK_Return :		/* entrée */
	case GDK_KP_Enter :

	    gsb_transactions_list_edit_transaction (gsb_data_account_get_current_transaction_number (account_number));
	    break;

	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    gsb_transactions_list_current_transaction_up (account_number);
	    break;

	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    gsb_transactions_list_current_transaction_down (account_number);
	    break;

	case GDK_Delete:		/*  del  */
	    gsb_transactions_list_delete_transaction (gsb_data_account_get_current_transaction_number (account_number));
	    break;

	case GDK_P:			/* touche P */
	case GDK_p:			/* touche p */

	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		gsb_transactions_list_switch_mark (gsb_data_account_get_current_transaction_number (account_number));
	    break;

	case GDK_r:			/* touche r */
	case GDK_R:			/* touche R */

	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		gsb_transactions_list_switch_R_mark (gsb_data_account_get_current_transaction_number (account_number));
	    break;

	case GDK_space:
	    transaction_number = gsb_data_account_get_current_transaction_number (account_number);
	    if ( transaction_number > 0)
	    {
		if (etat.equilibrage)
		{
		    /* we are reconciling, so mark/unmark the transaction */
		    gsb_transactions_list_switch_mark (transaction_number);
		    gsb_transactions_list_current_transaction_down (account_number);
		}
		else
		    /* space open/close a breakdown */
		    gsb_transactions_list_switch_expander (transaction_number);
	    }
	    break;

	case GDK_Left:
	    /* if we press left, give back the focus to the tree at left */
	    gtk_widget_grab_focus (navigation_tree_view);
	    break;
    }

    return TRUE;
}



/** 
 * change the selection to the transaction
 *
 * \param transaction_number the transaction we want to move to
 * of the mother to find it
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_select ( gint transaction_number )
{
    GtkTreeIter *iter;
    gint i;
    GdkColor *couleur;
    GtkTreeModel *model;
    gint current_transaction_number;
    gint account_number;
    gint nb_rows;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_select %d",
				   transaction_number ));

    if ( !gsb_transactions_list_get_tree_view()
	 ||
	 !transaction_number )
	return FALSE;

    /* the white number has no account number, so we take the current account */
    if ( transaction_number != -1 )
	account_number = gsb_data_transaction_get_account_number (transaction_number);
    else
	account_number = gsb_gui_navigation_get_current_account ();

    current_transaction_number =  gsb_data_account_get_current_transaction_number (account_number);

    /* if there is no change in the selection, go away */
    if ( transaction_number == current_transaction_number )
	return FALSE;

    nb_rows = gsb_data_account_get_nb_rows ( account_number );

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    /* first unselect the current transaction */
    iter = gsb_transaction_model_get_iter_from_transaction (current_transaction_number,
							    0 );

    if ( iter )
    {
	/* we need to unselect the transaction here */
	for ( i=0 ; i<nb_rows ; i++ )
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 iter,
				 TRANSACTION_COL_NB_SAVE_BACKGROUND, &couleur,
				 -1 );
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_BACKGROUND,couleur,
				 -1 );
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_SAVE_BACKGROUND, NULL,
				 -1 );

	    /* if it's a breakdown of transaction, it's only 1 line */
	    if ( gsb_data_transaction_get_mother_transaction_number (current_transaction_number))
		i = nb_rows;

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
	gtk_tree_iter_free (iter);
    }

    /* save the new current transaction */
    gsb_data_account_set_current_transaction_number ( account_number,
						      transaction_number);

    /* select now the new transaction */
    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number,
							    0 );

    if ( iter )
    {
	GtkTreeIter parent_iter;

	/* it's possible that we are on a child of breakdown, in that case,
	 * we will open the tree to see that transaction */
	if (gtk_tree_model_iter_parent ( GTK_TREE_MODEL ( model ),
					 &parent_iter,
					 iter ))
	{
	    /* ok, we are a child, so open the parent */
	    GtkTreePath *path_sorted;

	    path_sorted = gsb_transactions_list_get_sorted_path ( gsb_data_transaction_get_mother_transaction_number (transaction_number),
								  gsb_data_account_get_nb_rows (account_number) - 1);
	    gtk_tree_view_expand_row ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
				       path_sorted,
				       TRUE );
	    gtk_tree_path_free (path_sorted);
	}

	for ( i=0 ; i<nb_rows ; i++ )
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur,
				 -1 );
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_BACKGROUND, &couleur_selection,
				 -1 );
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_SAVE_BACKGROUND, couleur,
				 -1 );

	    /* if it's a breakdown of transaction, it's only 1 line */
	    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
		i = nb_rows;

	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
	gtk_tree_iter_free (iter);
    }
    return FALSE;
}


/** 
 * set the tree view on the current transaction
 * 
 * \param no_account
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_move_to_current_transaction ( gint no_account )
{
    GtkTreePath *path_sorted;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_move_to_current_transaction, compte %d", no_account ));

    if ( !gsb_transactions_list_get_tree_view() )
	return FALSE;

    path_sorted = gsb_transactions_list_get_sorted_path (gsb_data_account_get_current_transaction_number (no_account), 0);

    /* Sometimes, the current transaction can be hidden, so we have to
     * check for each path if it's valid if it's not, we make a
     * selection on the white line */

    if ( !path_sorted )
    {
	gsb_transactions_list_select ( -1 );
	return FALSE;
    }

    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view()),
				   path_sorted, NULL, FALSE, 0.5, 0.5 );
    gtk_tree_path_free (path_sorted);

    return FALSE;
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
gpointer cherche_operation_from_ligne ( gint ligne )
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



/******************************************************************************/
/* cette fonction renvoie le no de ligne de l'opération en argument */
/******************************************************************************/
gint cherche_ligne_operation ( gint transaction_number,
			       gint no_account )
{
    GtkTreeIter *iter;

    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number,
							    0 );

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
 * Called to edit a specific transaction
 * 
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_edit_transaction ( gint transaction_number )
{
    devel_debug ( g_strdup_printf ( "gsb_transactions_list_edit_transaction : %d",
				    transaction_number ));
    gsb_form_fill_by_transaction ( transaction_number, TRUE );
    return FALSE;
}


/** 
 * Called to edit a specific transaction but the number of transaction
 * is passed via a pointer (by g_signal_connect)
 * 
 * \param transaction_number a pointer wich is the number of the transaction
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_edit_transaction_by_pointer ( gint *transaction_number )
{
    devel_debug ( g_strdup_printf ( "gsb_transactions_list_edit_transaction_by_pointer : %d",
				    GPOINTER_TO_INT (transaction_number)));
    gsb_transactions_list_edit_transaction ( GPOINTER_TO_INT (transaction_number));
    return FALSE;
}



/** 
 * Called to edit a the current transaction
 * 
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_edit_current_transaction ( void )
{
    devel_debug ( "gsb_transactions_list_edit_current_transaction");
    gsb_transactions_list_edit_transaction (gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ()));
    return FALSE;
}



/**
 * switch the mark of the transaction in the list between P or empty
 * it will mark/unmark the transaction and update the marked amount
 * if we are reconciling, update too the amounts of the reconcile panel
 * 
 * \param transaction_number
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_switch_mark ( gint transaction_number )
{
    GtkTreeIter *iter;
    gint col;
    gsb_real amount;
    gint account_number;
    GtkTreeModel *model;

    /* if no P/R column, cannot really mark/unmark the transaction... */
    col = find_p_r_col ();
    if ( col == -1 )
	return FALSE;

    /* if we are on the white line, a R transaction or a child of breakdown, do nothing */
    if ( transaction_number == -1
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_mother_transaction_number (transaction_number))
	return FALSE;

    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number, 0 );

    if (!iter)
	return FALSE;

    account_number = gsb_gui_navigation_get_current_account ();
    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());
    amount = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);

    if (gsb_data_transaction_get_marked_transaction (transaction_number))
    {
	gsb_data_account_set_marked_balance ( account_number,
					      gsb_real_sub ( gsb_data_account_get_marked_balance (account_number),
							     amount ));
	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      OPERATION_NORMALE );
	gtk_tree_store_set ( GTK_TREE_STORE ( model ),
			     iter,
			     col, NULL,
			     -1 );
    }
    else
    {
	gsb_data_account_set_marked_balance ( account_number,
					      gsb_real_add ( gsb_data_account_get_marked_balance (account_number),
							     amount ));
	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      OPERATION_POINTEE );
	gtk_tree_store_set ( GTK_TREE_STORE ( model ),
			     iter,
			     col, _("P"),
			     -1 );
    }

    /* if it's a breakdown, set the mark to the children */
    if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number))
    {
	GSList *list_tmp_transactions;
	gint mark;

	mark = gsb_data_transaction_get_marked_transaction ( transaction_number);
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
		gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
							      mark );
	    list_tmp_transactions = list_tmp_transactions -> next;
	}
    }

    /* if we are reconciling, update the amounts label */
    if ( etat.equilibrage )
	gsb_reconcile_update_amounts (NULL, NULL);

    /* need to update the marked amount on the home page */
    mise_a_jour_liste_comptes_accueil = 1;

    gtk_tree_iter_free (iter);
    modification_fichier( TRUE );

    return FALSE;
}


/**
 * switch the mark of the transaction in the list between R or empty
 * it will mark/unmark the transaction and update the marked amount
 * when we mark, we show a list of reconcile and try to find the last reconcile used by that
 * 	transaction if it exists, from the 0.6.0, a transaction marked R shouldn't be without
 * 	reconcile number (but if come from before, it could happen)
 * when we unmark, we keep the reconcile number into the transaction to find it easily when the user
 * 	will re-R again
 * 
 * \param transaction_number
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_switch_R_mark ( gint transaction_number )
{
    gsb_real amount;
    gint account_number;
    GtkTreeIter *iter;
    gint col;
    GtkTreeModel *model;

    col = find_p_r_col ();
    if ( col == -1 )
	return FALSE;

    /* if we are on the white line or a child of breakdown, do nothing */
    if ( transaction_number == -1 )
	return FALSE;

    /* if we are reconciling, cancel the action */
    if (etat.equilibrage)
    {
	dialogue_error ( _("You cannot switch a transaction between R and non R while reconciling.\nPlease finish or cancel the reconciliation first."));
	return FALSE;
    }

    /* if it's a child, we ask if we want to work with the mother */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
	if (question_yes_no_hint ( _("Confirmation of manual (un)reconciliation"),
				   _("You are currently trying to reconcile or unreconcile "
				     "manually a transaction which isn't a regular way to do.\n\n"
				     "And moreover the transaction you try to reconcile is a child of breakdown, so "
				     "the modification will be done on the mother and all its children.\n\n"
				     "Are you really sure to know what you do?")))
	    /* he says ok, so transaction_number becomes the mother */
	    transaction_number = gsb_data_transaction_get_mother_transaction_number (transaction_number);
	else
	    return FALSE;
    }
    else
	/* it's a normal transaction, ask to be sure */
	if ( !question_yes_no_hint ( _("Confirmation of manual (un)reconciliation"),
				     _("You are currently trying to reconcile or unreconcile "
				       "manually a transaction which isn't a regular way to do.\n\n"
				       "Are you really sure to know what you do?")))
	    return FALSE;

    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number, 0 );

    if (!iter)
	return FALSE;

    account_number = gsb_gui_navigation_get_current_account ();
    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());
    amount = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);

    if ( gsb_data_transaction_get_marked_transaction ( transaction_number) == OPERATION_RAPPROCHEE)
    {
	/* ok, this is a R transaction, we just un-R it but keep the reconcile_number into the transaction */
	gsb_data_account_set_marked_balance ( account_number,
					      gsb_real_sub ( gsb_data_account_get_marked_balance (account_number),
							     amount ));
	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      OPERATION_NORMALE );
	gtk_tree_store_set ( GTK_TREE_STORE ( model ),
			     iter,
			     col, NULL,
			     -1 );
    }
    else
    {
	/* this is a non R transaction we want to mark R
	 * we show a list of possible reconcile to the user ; he must
	 * associate the transaction with a reconcile */

	gint reconcile_number;

	reconcile_number = gsb_transactions_list_choose_reconcile ( account_number,
								    gsb_data_transaction_get_reconcile_number (transaction_number));
	if (!reconcile_number)
	    return FALSE;

	gsb_data_account_set_marked_balance ( account_number,
					      gsb_real_add ( gsb_data_account_get_marked_balance (account_number),
							     amount ));
	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      OPERATION_RAPPROCHEE );
	gsb_data_transaction_set_reconcile_number ( transaction_number,
						    reconcile_number );

	/* now, either we show the R, either we hide that transaction if we don't want to see the R transactions */
	if ( gsb_data_account_get_r (account_number) )
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 iter,
				 col, _("R"),
				 -1 );
	else
	{
	    /* we don't want to see the R, so the transaction will disappear */
	    gint next_transaction_number;
	    gint i;

	    /* if the transaction is a breakdown and the children are showed, transaction_next will
	     * return the child of the breakdown, but it will disappear, so continue until we change the mother */
	    next_transaction_number = transaction_number;
	    do
	    {
		next_transaction_number = gsb_transactions_list_get_transaction_next (next_transaction_number);
	    }
	    while (gsb_data_transaction_get_mother_transaction_number (next_transaction_number) == transaction_number);

	    /* if there is some children, they will be hidden automatickly */
	    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	    {
		gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				     iter,
				     TRANSACTION_COL_NB_VISIBLE, FALSE,
				     -1 );
		gtk_tree_model_iter_next (GTK_TREE_MODEL (model), iter);
	    }

	    gsb_data_account_set_current_transaction_number ( account_number,
							      next_transaction_number);
	    gsb_transactions_list_set_background_color (account_number);
	    gsb_transactions_list_set_transactions_balances (account_number);

	    /* we warn the user the transaction disappear */
	    dialogue_hint ( _("The transaction has disappear from the list...\nDon't worry, it's because you marked it as R, and you choosed not to show the R transactions into the list ; show them if you want to check what you did."), 
			   _("Marking a transaction as R"));
	}
    }

    /* if it's a breakdown, set the mark to the children */
    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
    {
	GSList *list_tmp_transactions;
	gint mark;

	mark = gsb_data_transaction_get_marked_transaction ( transaction_number);
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
		gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
							      mark );

	    list_tmp_transactions = list_tmp_transactions -> next;
	}
    }
    /* need to update the marked amount on the home page */
    mise_a_jour_liste_comptes_accueil = 1;

    gtk_tree_iter_free (iter);
    modification_fichier( TRUE );

    return FALSE;
}


/**
 * create a popup with the list of the reconciles for the given account
 * to choose one
 *
 * \param account_number
 * \param selected_reconcile_number if not null, we will select that reconcile in the list
 *
 * \return the number of the chosen reconcile or 0 if cancel
 * */
gint gsb_transactions_list_choose_reconcile ( gint account_number,
					      gint selected_reconcile_number )
{
    GtkWidget *dialog;
    GtkWidget *tree_view;
    GtkListStore *store;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    gint return_value;
    GList *tmp_list;
    gint i;
    enum reconcile_choose_column {
	RECONCILE_CHOOSE_NAME = 0,
	RECONCILE_CHOOSE_INIT_DATE,
	RECONCILE_CHOOSE_FINAL_DATE,
	RECONCILE_NUMBER,
	RECONCILE_NB_COL };
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint reconcile_number;

    dialog = gtk_dialog_new_with_buttons ( _("Choosing a reconcile"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   NULL );
    /* need to set a size, else the list will be small */
    gtk_widget_set_usize ( dialog,
			   FALSE,
			   300 );

    label = gtk_label_new ( _("Select the reconcile wich the transaction should be associated with :"));
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 label,
			 FALSE, FALSE, 0 );
    gtk_widget_show (label);

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW (scrolled_window),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 scrolled_window,
			 TRUE, TRUE, 0 );
    gtk_widget_show (scrolled_window);

    /* set up the tree view */
    store = gtk_list_store_new ( RECONCILE_NB_COL,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_INT );
    tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
				  GTK_SELECTION_SINGLE );
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			tree_view );
    gtk_widget_show (tree_view);

    /* set the columns */
    for (i=RECONCILE_CHOOSE_NAME ; i<RECONCILE_NUMBER ; i++)
    {
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	gchar *titles[] = {
	    _("Name"), _("Init date"), _("Final date")
	};
	gfloat alignment[] = {
	    COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER
	};

	cell = gtk_cell_renderer_text_new ();
	g_object_set ( G_OBJECT (cell),
		       "xalign", alignment[i],
		       NULL );
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_sizing ( column,
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_alignment ( column,
					     alignment[i] );
	gtk_tree_view_column_pack_start ( column, cell, TRUE );
	gtk_tree_view_column_set_title ( column, titles[i] );
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", i,
					     NULL);
	gtk_tree_view_column_set_expand ( column, TRUE );
	gtk_tree_view_column_set_resizable ( column,
					     TRUE );
	gtk_tree_view_append_column ( GTK_TREE_VIEW(tree_view), column);
    }

    /* get the tree view selection here to select the good reconcile */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    /* fill the list */
    tmp_list = gsb_data_reconcile_get_reconcile_list ();
    while (tmp_list)
    {
	reconcile_number = gsb_data_reconcile_get_no_reconcile (tmp_list -> data);

	if (gsb_data_reconcile_get_account (reconcile_number) == account_number)
	{
	    gchar *init_date, *final_date;

	    init_date = gsb_format_gdate (gsb_data_reconcile_get_init_date (reconcile_number));
	    final_date = gsb_format_gdate (gsb_data_reconcile_get_final_date (reconcile_number));

	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( GTK_LIST_STORE (store),
				 &iter,
				 RECONCILE_CHOOSE_NAME, gsb_data_reconcile_get_name (reconcile_number),
				 RECONCILE_CHOOSE_INIT_DATE, init_date,
				 RECONCILE_CHOOSE_FINAL_DATE, final_date,
				 RECONCILE_NUMBER, reconcile_number,
				 -1 );
	    g_free (init_date);
	    g_free (final_date);

	    /* if we are on the reconcile to select, do it here */
	    if (selected_reconcile_number == reconcile_number)
		gtk_tree_selection_select_iter ( selection,
						 &iter );
	}
	tmp_list = tmp_list -> next;
    }

    /* run the dialog */
    return_value = gtk_dialog_run (GTK_DIALOG (dialog));

    if (return_value != GTK_RESPONSE_OK)
    {
	gtk_widget_destroy (dialog);
	return 0;
    }

    if (gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (selection),
					  NULL,
					  &iter))
    {
	/* ok, we have a selection */

	gtk_tree_model_get ( GTK_TREE_MODEL (store),
			     &iter,
			     RECONCILE_NUMBER, &reconcile_number,
			     -1 );
	gtk_widget_destroy (dialog);
	return reconcile_number;
    }
    
    dialogue_error ( _("Grisbi couldn't get the selection, operation canceled..."));
    gtk_widget_destroy (dialog);
    return 0;
}



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
    gsb_real amount;
    gint account_number;
    gchar *string;

    /* vérifications de bases */
    if ( !transaction_number
	 ||
	 transaction_number < 0 )
	return FALSE;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_delete_transaction no %d",
				   transaction_number ));

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* check if the transaction is not reconciled */
    if ( gsb_transactions_list_check_mark (transaction_number))
    {
	dialogue_error ( _("Impossible to delete a reconciled transaction.\nThe transaction, the contra-transaction or the children if it is a breakdown are reconciled. You can remove the reconciliation with Ctrl R if it is really necessary.") );
	return FALSE;
    }

    /* show a warning */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
	if ( !question_yes_no_hint ( _("Delete a transaction"),
				     g_strdup_printf ( _("Do you really want to delete the child of the transaction with party '%s' ?"),
						       gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number),
										 FALSE ))))
	    return FALSE;
    }
    else
    {
	if ( !question_yes_no_hint ( _("Delete a transaction"),
				     g_strdup_printf ( _("Do you really want to delete transaction with party '%s' ?"),
						       gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number),
										 FALSE ))))
	    return FALSE;
    }

    /* find the next transaction to be selected */
    if ( gsb_data_account_get_current_transaction_number (account_number) == transaction_number)
    {
	gint next_transaction_number;

	next_transaction_number = gsb_transactions_list_get_transaction_next (transaction_number);
	gsb_data_account_set_current_transaction_number ( account_number,
							  next_transaction_number);
    }

    /* calculate the new balances */
    amount = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);
    gsb_data_account_set_current_balance ( account_number,
					   gsb_real_sub ( gsb_data_account_get_current_balance (account_number),
							  amount ));
    if ( gsb_data_transaction_get_marked_transaction (transaction_number))
	gsb_data_account_set_marked_balance ( account_number,
					      gsb_real_sub ( gsb_data_account_get_marked_balance (account_number),
							     amount ));

    /* update the headings balance */
    string = gsb_real_format_currency_from_locale ( gsb_data_account_get_current_balance (account_number) );
    gsb_gui_headings_update_suffix ( string );
    g_free (string);


    /* delete the transaction from the tree view,
     * all the children and contra transaction will be removed with that */
    gsb_transactions_list_delete_transaction_from_tree_view (transaction_number);

    /* delete the transaction in memory,
     * all the children and contra transaction will be removed with that */
    gsb_data_transaction_remove_transaction ( transaction_number);

    /* update the tree view */
    gsb_transactions_list_set_background_color (account_number);
    gsb_transactions_list_set_transactions_balances (account_number);

    /* if we are reconciling, update the amounts */
    if ( etat.equilibrage )
	gsb_reconcile_update_amounts (NULL, NULL);


    /* we will update the home page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    affiche_dialogue_soldes_minimaux ();

    /* We blank form. */
    gsb_form_escape_form ();

    /* FIXME : on devrait réafficher les listes de tiers, categ, ib... */
    modification_fichier( TRUE );
    return FALSE;
}



/**
 * Check if the transaction (or linked transactions) is not marked as Reconciled
 *
 * \param transaction_number
 *
 * \return TRUE : it's marked R ; FALSE : it's normal, P or T transaction
 * */
gboolean gsb_transactions_list_check_mark ( gint transaction_number )
{
    gint transaction_number_tmp;

    /* vérifications de bases */
    if (transaction_number <= 0)
	return FALSE;

    if ( gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE )
	return TRUE;

    /* if it's a transfer, check the contra-transaction */
    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number)
	 &&
	 gsb_data_transaction_get_account_number_transfer (transaction_number)!= -1 )
    {
	transaction_number_tmp = gsb_data_transaction_get_transaction_number_transfer (transaction_number);

	if ( transaction_number_tmp
	     &&
	     gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE )
	    return TRUE;
    }

    /* if it's a breakdown of transaction, check all the children
     * if there is not a transfer which is marked */
    if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number))
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (transaction_number)
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
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
 * Delete the transaction from the tree view
 * if it's a transfer, delete the contra transaction from the tree view
 * if it's a breakdown, delete a the children and if necessary the contra transactions
 * 
 * don't do any check about possible or marked... check before
 * do nothing in memory, only on the tree view
 *
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_delete_transaction_from_tree_view ( gint transaction_number )
{
    GtkTreeIter *iter;
    GtkTreeModel *model;
    gint i;
    gint transaction_number_tmp;

    if ( transaction_number <= 0 )
	return FALSE;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_delete_transaction_from_tree_view no %d",
				   transaction_number));

    /* first, check the transaction exits in the tree view */
    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number, 0 );
    if (!iter)
	return FALSE;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    /* check if it's a transfer, if yes, erase the contra transaction in the tree view */
    transaction_number_tmp = gsb_data_transaction_get_transaction_number_transfer (transaction_number);
    if (transaction_number_tmp)
    {
	GtkTreeIter *contra_iter;

	contra_iter = gsb_transaction_model_get_iter_from_transaction (transaction_number_tmp, 0);
	if (contra_iter)
	{
	    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	    {
		gtk_tree_store_remove ( GTK_TREE_STORE ( model ),
					contra_iter );

		/* if we are on a breakdown child, it's only 1 line */
		if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp))
		    i = TRANSACTION_LIST_ROWS_NB;
	    }
	    gtk_tree_iter_free (contra_iter);
	}
    }

    /* check if it's a breakdown, needn't to erase all breakdowns children, they will be deleted
     * with the mother, but if one of them if a transfer, we need to delete it now */
    if (gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
    {
	GSList *tmp_list;

	tmp_list = gsb_data_transaction_get_transactions_list ();
	while (tmp_list)
	{
	    gint test_transaction;

	    test_transaction = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	    if ( gsb_data_transaction_get_mother_transaction_number (test_transaction) == transaction_number
		 &&
		 (transaction_number_tmp = gsb_data_transaction_get_transaction_number_transfer (test_transaction)))
	    {
		/* here, transaction_number_tmp contains the contra transaction number of the child */
		GtkTreeIter *contra_iter;

		contra_iter = gsb_transaction_model_get_iter_from_transaction (transaction_number_tmp, 0);
		if (contra_iter)
		{
		    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
			gtk_tree_store_remove ( GTK_TREE_STORE ( model ),
						contra_iter );
		    gtk_tree_iter_free (contra_iter);
		}
	    }
	    tmp_list = tmp_list -> next;
	}
    }

    /* now we can just delete the wanted transaction */
    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	gtk_tree_store_remove ( GTK_TREE_STORE ( model ),
				iter );

	/* if we are on a breakdown child, it's only 1 line */
	if ( gsb_data_transaction_get_mother_transaction_number (transaction_number))
	    i = TRANSACTION_LIST_ROWS_NB;
    }
    gtk_tree_iter_free (iter);
    return FALSE;
}





/******************************************************************************/
/* Fonction  demande_mise_a_jour_tous_comptes */
/* met la variable update_list de tous les comptes à 1 */
/* ce qui fait que lorsqu'ils seront affichés, ils seront mis à jour avant */
/* appelle aussi verification_mise_a_jour_liste pour mettre à jour la liste courante */
/******************************************************************************/
void demande_mise_a_jour_tous_comptes ( void )
{
    /* xxx FIXME: ça doit disparaitre */
    gtk_tree_store_clear ( GTK_TREE_STORE ( gsb_transactions_list_get_store()  ));
    gsb_transactions_list_fill_store ( gsb_transactions_list_get_store() );
}
/******************************************************************************/


/**
 * Pop up a menu with several actions to apply to current transaction.
 * 
 * \param 
 *
 */
void popup_transaction_context_menu ( gboolean full, int x, int y )
{
    GtkWidget *menu, *menu_item;
    gint transaction_number;

    transaction_number = gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ());

    if ( transaction_number == -1 )
	full = FALSE;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Edit transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_PROPERTIES,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect_swapped ( G_OBJECT(menu_item),
			       "activate",
			       G_CALLBACK (gsb_transactions_list_edit_transaction_by_pointer),
			       GINT_TO_POINTER (transaction_number));
    gtk_widget_set_sensitive ( menu_item, full );
    gtk_menu_append ( menu, menu_item );

    /* Separator */
    gtk_menu_append ( menu, gtk_separator_menu_item_new() );

    /* New transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("New transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_NEW,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK (new_transaction), NULL );
    gtk_menu_append ( menu, menu_item );

    /* Delete transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Delete transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_DELETE,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", remove_transaction, NULL );
    if ( !full
	 || 
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELERAPPROCHEE )
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
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELERAPPROCHEE )
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
    gint i;

    menu = gtk_menu_new ();

    for ( i = 0 ; cell_views[i].name != NULL ; i++ )
    {
	item = gtk_menu_item_new_with_label ( _(cell_views[i] . name) );
	g_object_set_data ( G_OBJECT (item), "x", GINT_TO_POINTER (x) );
	g_object_set_data ( G_OBJECT (item), "y", GINT_TO_POINTER (y) );
	g_signal_connect ( G_OBJECT(item), "activate", 
			   G_CALLBACK(gsb_gui_change_cell_content), GINT_TO_POINTER (i));
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
	gtk_tree_store_set ( gsb_transactions_list_get_store (),
			     iter,
			     coords[0], 
			     gsb_transactions_list_grep_cell_content_trunc ( transaction_number,
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

    coords[0] = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT (item), "x" ));
    coords[1] = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT (item), "y" ));

    tab_affichage_ope[coords[1]][coords[0]] = number + 1;

    gtk_tree_model_foreach ( GTK_TREE_MODEL (gsb_transactions_list_get_store ()),
			     (GtkTreeModelForeachFunc) gsb_gui_update_row_foreach, coords );

    update_titres_tree_view ();

    return FALSE;
}



/**
 *  Check that a transaction is selected 
 *
 * \return TRUE on success, FALSE otherwise.
 */
gboolean assert_selected_transaction ()
{
    if ( gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ()) == -1 )
	return FALSE;

    return TRUE;
}


/**
 *  Empty transaction form and select transactions tab.
 */
gboolean new_transaction () 
{
    /* We do not test it since we don't care about no selected
       transaction */
    assert_selected_transaction();
    gsb_form_escape_form();
    gsb_data_account_set_current_transaction_number ( gsb_gui_navigation_get_current_account (),
						      -1 );
    gsb_transactions_list_edit_transaction (-1);

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
    gsb_form_show (TRUE);
    return FALSE;
}



/**
 * Remove selected transaction if any.
 */
void remove_transaction ()
{
    if (! assert_selected_transaction()) return;

    gsb_transactions_list_delete_transaction (gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ()));
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}



/**
 * Clone selected transaction if any.  Update user interface as well.
 */
void clone_selected_transaction ()
{
    gint new_transaction_number;

    if (! assert_selected_transaction()) return;

    new_transaction_number = gsb_transactions_list_clone_transaction (gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ()));

    update_transaction_in_trees (new_transaction_number);

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
				     gsb_data_transaction_get_account_number_transfer (transaction_number));

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

    source_account = gsb_gui_navigation_get_current_account ();
    target_account = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT(menu_item), 
							     "account_number" ) );  

    if ( move_operation_to_account ( gsb_data_account_get_current_transaction_number (source_account),
				     target_account ))
    {
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	update_transaction_in_trees (gsb_data_account_get_current_transaction_number (source_account));

	if ( mise_a_jour_combofix_tiers_necessaire )
	    mise_a_jour_combofix_tiers ();
	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ ();
	if ( mise_a_jour_combofix_imputation_necessaire )
	    mise_a_jour_combofix_imputation ();

	gsb_data_account_calculate_current_and_marked_balances (source_account);

	gsb_gui_headings_update_suffix ( gsb_real_format_currency_from_locale (gsb_data_account_get_current_balance (source_account ) ) );
	mise_a_jour_accueil (FALSE);

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

    source_account = gsb_gui_navigation_get_current_account ();
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

	update_transaction_in_trees (gsb_data_account_get_current_transaction_number (source_account));

	gsb_data_account_calculate_current_and_marked_balances (source_account);

	gsb_gui_headings_update_suffix ( gsb_real_format_currency_from_locale (gsb_data_account_get_current_balance (source_account ) ) );

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
	gsb_transactions_list_update_transaction (contra_transaction_number);
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

    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number,
							    0 );

    if ( iter )
    {
	gint i;

	/* change the selection if necessary */

	if ( gsb_data_account_get_current_transaction_number (source_account) == transaction_number)
	    gsb_data_account_set_current_transaction_number ( source_account,
							      gsb_data_transaction_get_transaction_number (cherche_operation_from_ligne ( cherche_ligne_operation ( gsb_data_account_get_current_transaction_number (source_account),
																				    source_account)
																	  +
																	  gsb_data_account_get_nb_rows (source_account))));

	for ( i=0 ; i<gsb_data_account_get_nb_rows (source_account) ; i++ )
	    gtk_tree_store_remove ( GTK_TREE_STORE ( gsb_transactions_list_get_store() ),
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
    gint scheduled_number;

    if (!assert_selected_transaction())
	return;

    scheduled_number = schedule_transaction ( gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ()));

    mise_a_jour_liste_echeances_auto_accueil = 1;
    gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());

    gsb_form_set_sensitive(gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number),
			   gsb_data_scheduled_get_mother_scheduled_number (scheduled_number));
    gsb_scheduler_list_edit_transaction (gsb_scheduler_list_get_current_scheduled_number ());

    gtk_notebook_set_current_page ( GTK_NOTEBOOK(notebook_general), 2 );

    modification_fichier ( TRUE );
}



/**
 *  Convert transaction to a template of scheduled transaction.
 *
 * \param transaction Transaction to use as a template.
 *
 * \return the number of the scheduled transaction
 */
gint schedule_transaction ( gint transaction_number )
{
    gint scheduled_number;

    scheduled_number = gsb_data_scheduled_new_scheduled ();

    if ( !scheduled_number)
	return FALSE;

    gsb_data_scheduled_set_account_number ( scheduled_number,
					    gsb_data_transaction_get_account_number (transaction_number));
    gsb_data_scheduled_set_date ( scheduled_number,
				  gsb_date_copy (gsb_data_transaction_get_date (transaction_number)));
    gsb_data_scheduled_set_amount ( scheduled_number,
				    gsb_data_transaction_get_amount (transaction_number));
    gsb_data_scheduled_set_currency_number ( scheduled_number,
					     gsb_data_transaction_get_currency_number (transaction_number));
    gsb_data_scheduled_set_party_number ( scheduled_number,
					  gsb_data_transaction_get_party_number (transaction_number));
    gsb_data_scheduled_set_category_number ( scheduled_number,
					     gsb_data_transaction_get_category_number (transaction_number));
    gsb_data_scheduled_set_sub_category_number ( scheduled_number,
						 gsb_data_transaction_get_sub_category_number (transaction_number));

    /*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
    /* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement ni une ventil, compte_virement = -1 */
    /*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number))
    {
	/* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

	gint contra_transaction_number;

	gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
							 gsb_data_transaction_get_account_number_transfer (transaction_number));

	contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number);

	gsb_data_scheduled_set_contra_method_of_payment_number ( scheduled_number,
								 gsb_data_transaction_get_method_of_payment_number (contra_transaction_number));
    }
    else
	if ( !gsb_data_scheduled_get_category_number (scheduled_number)
	     &&
	     !gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	    gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
							     -1 );

    gsb_data_scheduled_set_notes ( scheduled_number,
				   gsb_data_transaction_get_notes (transaction_number));
    gsb_data_scheduled_set_method_of_payment_number ( scheduled_number,
						      gsb_data_transaction_get_method_of_payment_number (transaction_number));
    gsb_data_scheduled_set_method_of_payment_content ( scheduled_number,
						       gsb_data_transaction_get_method_of_payment_content (transaction_number));

    gsb_data_scheduled_set_financial_year_number ( scheduled_number,
						   gsb_data_transaction_get_financial_year_number (transaction_number));
    gsb_data_scheduled_set_budgetary_number ( scheduled_number,
					      gsb_data_transaction_get_budgetary_number (transaction_number));
    gsb_data_scheduled_set_sub_budgetary_number ( scheduled_number,
						  gsb_data_transaction_get_sub_budgetary_number (transaction_number));
    gsb_data_scheduled_set_breakdown_of_scheduled ( scheduled_number,
						    gsb_data_transaction_get_breakdown_of_transaction (transaction_number));

    /*     par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
    /*     (c'est le cas, à 0 avec calloc) */
    /*     que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
    /* 	pour la même raison */

    gsb_data_scheduled_set_frequency ( scheduled_number,
				       2);

    /*     on récupère les opés de ventil si c'était une opé ventilée */

    if ( gsb_data_scheduled_get_breakdown_of_scheduled ( scheduled_number))
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (transaction_number)
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
	    {
		gint breakdown_scheduled_number;

		breakdown_scheduled_number = gsb_data_scheduled_new_scheduled ();

		if ( !breakdown_scheduled_number)
		    return FALSE;

		gsb_data_scheduled_set_account_number ( breakdown_scheduled_number,
							gsb_data_transaction_get_account_number (transaction_number_tmp));
		gsb_data_scheduled_set_date ( breakdown_scheduled_number,
					      gsb_date_copy (gsb_data_transaction_get_date (transaction_number_tmp)));
		gsb_data_scheduled_set_amount ( breakdown_scheduled_number,
						gsb_data_transaction_get_amount (transaction_number_tmp));
		gsb_data_scheduled_set_currency_number ( breakdown_scheduled_number,
							 gsb_data_transaction_get_currency_number (transaction_number_tmp));
		gsb_data_scheduled_set_party_number ( breakdown_scheduled_number,
						      gsb_data_transaction_get_party_number (transaction_number_tmp));
		gsb_data_scheduled_set_category_number ( breakdown_scheduled_number,
							 gsb_data_transaction_get_category_number (transaction_number_tmp));
		gsb_data_scheduled_set_sub_category_number ( breakdown_scheduled_number,
							     gsb_data_transaction_get_sub_category_number (transaction_number_tmp));

		/*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
		/* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement, compte_virement = -1 */
		/*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

		if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number_tmp))
		{
		    /* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

		    gint contra_transaction_number;

		    gsb_data_scheduled_set_account_number_transfer ( breakdown_scheduled_number,
								     gsb_data_transaction_get_account_number_transfer (transaction_number_tmp));

		    contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number_tmp);

		    gsb_data_scheduled_set_contra_method_of_payment_number ( breakdown_scheduled_number,
									     gsb_data_transaction_get_method_of_payment_number (contra_transaction_number));
		}
		else
		    if ( !gsb_data_scheduled_get_category_number (breakdown_scheduled_number))
			gsb_data_scheduled_set_account_number_transfer ( breakdown_scheduled_number,
									 -1 );

		gsb_data_scheduled_set_notes ( breakdown_scheduled_number,
					       gsb_data_transaction_get_notes (transaction_number_tmp));
		gsb_data_scheduled_set_method_of_payment_number ( breakdown_scheduled_number,
								  gsb_data_transaction_get_method_of_payment_number (transaction_number_tmp));
		gsb_data_scheduled_set_method_of_payment_content ( breakdown_scheduled_number,
								   gsb_data_transaction_get_method_of_payment_content (transaction_number_tmp));
		gsb_data_scheduled_set_financial_year_number ( breakdown_scheduled_number,
							       gsb_data_transaction_get_financial_year_number (transaction_number_tmp));
		gsb_data_scheduled_set_budgetary_number ( breakdown_scheduled_number,
							  gsb_data_transaction_get_budgetary_number (transaction_number_tmp));
		gsb_data_scheduled_set_sub_budgetary_number ( breakdown_scheduled_number,
							      gsb_data_transaction_get_sub_budgetary_number (transaction_number_tmp));

		gsb_data_scheduled_set_mother_scheduled_number ( breakdown_scheduled_number,
								 scheduled_number );

		/*     par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
		/*     (c'est le cas, à 0 avec calloc) */
		/*     que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
		/* 	pour la même raison */

		gsb_data_scheduled_set_frequency ( breakdown_scheduled_number,
						   2);
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}
    }
    return scheduled_number;
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

    g_return_val_if_fail ( gsb_data_account_get_nb_rows ( gsb_gui_navigation_get_current_account () ),
			   FALSE );

    /*  FIXME   sachant qu'on appelle ça à chaque expose-event, cad très souvent ( dès que la souris passe dessus ), */
    /*     ça peut ralentir bcp... à vérifier  */
    /* xxx FIXME ça marche pas encore avec les ventilations (la grille) */

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

    /*     derniere_ligne = hauteur_ligne_liste_opes * GTK_TREE_STORE ( gsb_transactions_list_get_store()) -> length; */
    /* /\*     derniere_ligne = 10; /\\* FIXME àvirer *\\/ *\/ */
    /*     hauteur = MIN ( derniere_ligne, */
    /* 		    hauteur ); */

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

	y = ( hauteur_ligne_liste_opes * gsb_data_account_get_nb_rows ( gsb_gui_navigation_get_current_account () ) ) * ( ceil ( adjustment->value / (hauteur_ligne_liste_opes* gsb_data_account_get_nb_rows ( gsb_gui_navigation_get_current_account () )) )) - adjustment -> value;

	do
	{
	    if ( y )
		gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
				gc_separateur_operation,
				0, y, 
				largeur, y );
	    y = y + hauteur_ligne_liste_opes*gsb_data_account_get_nb_rows ( gsb_gui_navigation_get_current_account () );
	}
	while ( y < ( adjustment -> page_size ) /* && */
		/* 		y < derniere_ligne */ );
    }

    return FALSE;
}
/******************************************************************************/



/**
 * called when press a button on the title column
 * check in fact the right click and show a popup to chose the element number of
 * the column to sort the list
 *
 * \param button
 * \param ev
 * \param no_column a pointer wich is the number of the column the user press
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_title_column_button_press ( GtkWidget *button,
							   GdkEventButton *ev,
							   gint *no_column )
{
    GtkWidget *menu, *menu_item;
    gint i;
    gint active_sort;
    gint column_number;

    switch ( ev -> button )
    {
	case 3:
	    /* we press the right button, show the popup */

	    column_number = GPOINTER_TO_INT (no_column);

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


	    active_sort = gsb_data_account_get_element_sort ( gsb_gui_navigation_get_current_account (),
							      column_number);

	    /*     get the name of the labels of the columns and put them in a menu */

	    for ( i=0 ; i<4 ; i++ )
	    {
		gchar *temp;

		switch ( tab_affichage_ope[i][column_number] )
		{
		    case 0:
			temp = NULL;
			break;

		    default:
			temp = g_slist_nth_data ( liste_labels_titres_colonnes_liste_ope,
						  tab_affichage_ope[i][column_number] - 1 );
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

		    if ( tab_affichage_ope[i][column_number] == active_sort )
			gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( menu_item ),
							 TRUE );

		    g_object_set_data ( G_OBJECT ( menu_item ),
					"no_sort",
					GINT_TO_POINTER (tab_affichage_ope[i][column_number]));
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


/**
 * called when choose a new element to sort the list (from the popup of a right click on
 * the title of columns)
 *
 * \param menu_item The GtkMenuItem
 * \param no_column a pointer containing the number of the column we change
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_change_sort_type ( GtkWidget *menu_item,
						  gint *no_column )
{
    gint sort_column_id;
    GtkSortType order;
    GtkTreeSortable *sortable;
    gint column_number;
    gint account_number;

    if ( !gtk_check_menu_item_get_active ( GTK_CHECK_MENU_ITEM ( menu_item )))
	return FALSE;

    column_number = GPOINTER_TO_INT (no_column);
    sortable = GTK_TREE_SORTABLE (gsb_transactions_list_get_sortable ());
    account_number = gsb_gui_navigation_get_current_account ();

    /* set the new column to sort */
    gsb_data_account_set_sort_column ( account_number,
				       column_number );

    /* set the new element number used to sort this column */
    gsb_data_account_set_element_sort ( account_number,
					column_number,
					GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( menu_item),
									      "no_sort" )));

    /* get the current sorted to check if we don't change the column */
    gtk_tree_sortable_get_sort_column_id ( sortable,
					   &sort_column_id,
					   &order );

    gtk_tree_sortable_set_sort_column_id ( sortable,
					   column_number,
					   GTK_SORT_ASCENDING );

    /* we have to check because if we ask for a sort on another line but same column,
     * the tree will not update ; so in that case, we have to invert 2 times the order
     * */
    if ( sort_column_id == column_number
	 &&
	 order == GTK_SORT_ASCENDING )
    {
	gtk_tree_sortable_set_sort_column_id ( sortable,
					       column_number,
					       GTK_SORT_DESCENDING );
	gtk_tree_sortable_set_sort_column_id ( sortable,
					       column_number,
					       GTK_SORT_ASCENDING );
    }
    gsb_transactions_list_sort_column_changed ( transactions_tree_view_columns[GPOINTER_TO_INT (no_column)]);
    return FALSE;
}




/******************************************************************************/
/* cette fonction est appelée lorsqu'on veut changer l'état de l'affichage avec/sans R */
/* argument : le nouvel affichage */
/* en fonction de l'argument, elle retire les lignes R de la liste ou elle les ajoute */
/******************************************************************************/
void mise_a_jour_affichage_r ( gint affichage_r )
{
    gint current_account;

    devel_debug ( g_strdup_printf ("mise_a_jour_affichage_r afficher : %d", affichage_r ));

    current_account = gsb_gui_navigation_get_current_account ();

    /* if the R transactions are not loaded and we want to show them,
     * we do it here */
    if ( !etat.fill_r_done
	 &&
	 affichage_r == 1 )
	gsb_transactions_list_load_marked_r ();

    /*     we check all the accounts */
    /* 	if etat.retient_affichage_par_compte is set, only gsb_gui_navigation_get_current_account () will change */
    /* 	else, all the accounts change */

    if ( affichage_r == gsb_data_account_get_r (current_account))
	return;

    gsb_data_account_set_r ( current_account,
			     affichage_r );

    if ( !etat.retient_affichage_par_compte )
    {
	GSList *list_tmp;

	list_tmp = gsb_data_account_get_list_accounts ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_data_account_get_no_account ( list_tmp -> data );
	    gsb_data_account_set_r ( i,
				     affichage_r );

	    list_tmp = list_tmp -> next;
	}
    }

    gsb_transactions_list_set_visibles_rows_on_account (current_account);
    gsb_transactions_list_set_background_color (current_account);

    return;
}
/******************************************************************************/


/**
 * load the marked R transactions into the transactions list
 * called when the user doesn't load the at the begining (see config) and
 * want to show them while he is using grisbi, so we come here to set them
 * into the list
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_load_marked_r ( void )
{
    GtkTreeStore *store;
    GtkWidget *message_window;

    if ( etat.fill_r_done )
	return FALSE;

    /* we show a message because it can take some time */
    message_window = GTK_WIDGET (gsb_dialog_create_information_window ( make_hint(_("Loading archived reconciled transactions."),
										  _("This operation can take some time...") )));
    gtk_widget_show (message_window);
    update_ecran ();

    store = gsb_transactions_list_get_store ();
    gtk_tree_store_clear (store);

    /* we have to load all the R transactions,
     * the simplest way is to refill the store because
     * most of the time will be spend for R transactions */
    etat.fill_r_at_begining = 1;

    gsb_transactions_list_fill_store (store);

    etat.fill_r_at_begining = 0;
    etat.fill_r_done = 1;

    gtk_widget_destroy (message_window);
    return FALSE;
}




/**
 * called to change the number of visible rows
 * depends of the conf, change the number of rows of the current account,
 * and if necessary of all the accounts
 * adapts also the tree for the breakdowns to set the expander on the new last line
 *
 * \param rows_number the new number of lines we want to see
 *
 * \return
 * */
void gsb_transactions_list_set_visible_rows_number ( gint rows_number )
{
    GSList *list_tmp;
    gint current_account;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_set_visible_rows_number %d lines", rows_number ));

    current_account = gsb_gui_navigation_get_current_account ();
    if ( rows_number == gsb_data_account_get_nb_rows (current_account))
	return;

    /*     we check all the accounts */
    /* 	if etat.retient_affichage_par_compte is set, only the current account changes */
    /* 	else, all the accounts change */
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( !etat.retient_affichage_par_compte
	     ||
	     i == current_account)
	{
	    gsb_data_account_set_nb_rows ( i, 
					   rows_number );
	}
	list_tmp = list_tmp -> next;
    }

    /* we update the screen */
    gsb_transactions_list_set_visibles_rows_on_account (current_account);
    gsb_transactions_list_set_background_color (current_account);

    /* change the expanders for breakdowns */
    gsb_transactions_list_change_expanders ( etat.retient_affichage_par_compte );
}



/** 
 * check all the lines of the account and set if they are shown or not
 * 
 * \param no_account the account we want to set
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_visibles_rows_on_account ( gint no_account )
{
    GtkTreeModel *original_model;
    GtkTreeIter iter;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_set_visibles_rows_on_account %d", no_account ));

    original_model = GTK_TREE_MODEL (gsb_transactions_list_get_store ());

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( original_model ),
					 &iter ))
    {
	gint r_shown;
	gint nb_rows;

	r_shown = gsb_data_account_get_r ( no_account );
	nb_rows = gsb_data_account_get_nb_rows ( no_account );

	do
	{
	    gint transaction_number;
	    gpointer transaction;
	    gint current_line;
	    gint show;

	    /* 	    the current_line will contain 0, 1, 2 or 3 ; the line on the transaction */
	    /* 		we are on */
	    gtk_tree_model_get ( GTK_TREE_MODEL ( original_model ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
				 TRANSACTION_COL_NB_TRANSACTION_LINE, &current_line,
				 -1 );

	    transaction_number = gsb_data_transaction_get_transaction_number (transaction);

	    /* check the general white line (one for all the list, so no account number) */
	    if ( transaction_number == -1)
	    {
		if ( current_line < nb_rows )
		    show = TRUE;
		else
		    show = FALSE;

		gtk_tree_store_set ( GTK_TREE_STORE ( original_model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, show,
				     -1 );
		continue;
	    }

	    /* check the account */
	    if ( gsb_data_transaction_get_account_number (transaction_number) != no_account )
	    {
		gtk_tree_store_set ( GTK_TREE_STORE ( original_model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, FALSE,
				     -1 );
		continue;
	    }

	    /* 	    check if it's R and if r is shown */
	    if ( gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE
		 &&
		 !r_shown )
	    {
		gtk_tree_store_set ( GTK_TREE_STORE ( original_model ),
				     &iter,
				     TRANSACTION_COL_NB_VISIBLE, FALSE,
				     -1 );
		continue;
	    }

	    /* 	    now we check if we show 1, 2, 3 or 4 lines */

	    /* FIXME: lors de l'affichage de plusieurs lignes, les lignes du dessous doivent forcemment être
	     * plus bas que celles du desssus, par ex
	     * sur 3 lignes, ligne 1 -> 1, ligne 2-> 2, ligne 3-> 4   => ok
	     * mais ligne 1 -> 2 et ligne 2 -> 1  => pas bon ;
	     * if faut l'interdire dans les paramètres */

	    /* by default, we don't show the line */
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
	    gtk_tree_store_set ( GTK_TREE_STORE ( original_model ),
				 &iter,
				 TRANSACTION_COL_NB_VISIBLE, show,
				 -1 );
	}
	while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( original_model ),
					   &iter ));
    }
    return FALSE;
}


/**
 * change the expanders for breakdowns, set the arrow on the last
 * line of the transaction ; it depends of the account
 *
 * \param only_current_account if FALSE, do for all accounts
 *
 * \return
 * */
void gsb_transactions_list_change_expanders ( gint only_current_account )
{
    GtkTreeModel *tree_store;
    GtkTreeIter iter;

    devel_debug ( g_strdup_printf ("gsb_transactions_list_change_expanders, only_current_account =  %d", only_current_account ));

    tree_store = GTK_TREE_MODEL (gsb_transactions_list_get_store ());

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( tree_store ),
					 &iter ))
    {
	gint current_account;
	gint iter_next_result = 0;

	current_account = gsb_gui_navigation_get_current_account ();

	do
	{
	    gint transaction_number;
	    gint transaction_account_number;
	    gpointer transaction;
	    gpointer check_transaction;
	    gint current_line;
	    GtkTreeIter *new_mother_iter;
	    GtkTreeIter *last_mother_iter;
	    gint account_number_rows;

	    /* 	    the current_line will contain 0, 1, 2 or 3 ; the line on the transaction */
	    /* 		we are on */

	    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_store ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
				 TRANSACTION_COL_NB_TRANSACTION_LINE, &current_line,
				 -1 );

	    transaction_number = gsb_data_transaction_get_transaction_number (transaction);
	    transaction_account_number = gsb_data_transaction_get_account_number (transaction_number);

	    /* we work only on breakdowns, so if not, go directly on the next transaction
	     * check at the same time if the account is valid (when only current account) */

	    if ( !gsb_data_transaction_get_breakdown_of_transaction (transaction_number)
		 ||
		 ( only_current_account
		   &&
		   transaction_account_number != current_account))
	    {
		gint i;

		for ( i=0 ; i<4 ; i++ )
		{
		    iter_next_result = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( tree_store ),
								  &iter );
		}
		continue;
	    }

	    /* here we have to change the place of the expander, we just need to fill 
	     * new_mother_iter and last_mother_iter and call gsb_transactions_list_swap_children */

	    new_mother_iter = NULL;
	    last_mother_iter = NULL;
	    account_number_rows = gsb_data_account_get_nb_rows (transaction_account_number);
	    check_transaction = transaction;

	    while ( (!new_mother_iter
		     ||
		     !last_mother_iter)
		    &&
		    check_transaction == transaction)
	    {
		if ( current_line == (account_number_rows - 1))
		{
		    new_mother_iter = gtk_tree_iter_copy (&iter);
		}

		if ( gtk_tree_model_iter_has_child ( GTK_TREE_MODEL ( tree_store ),
						     &iter ))
		{
		    last_mother_iter = gtk_tree_iter_copy (&iter);
		}

		/* go to the next iter but we want to stay in the same transaction */

		iter_next_result = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( tree_store ),
							      &iter );
		gtk_tree_model_get ( GTK_TREE_MODEL ( tree_store ),
				     &iter,
				     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &check_transaction,
				     TRANSACTION_COL_NB_TRANSACTION_LINE, &current_line,
				     -1 );
	    }

	    /* at this step if one of the 2 iters is NULL, it shows that we went to the next transaction
	     * without fill the 2 iters... so problem... normally souldn't append */

	    if ( (!new_mother_iter
		  ||
		  !last_mother_iter)
		 &&
		 check_transaction != transaction)
	    {
		printf ( "Should not append : in gsb_transactions_list_change_expanders, the function didn't\nfind the last or the new place of the expander of a breakdown of transaction...\n" );

		/* we free the iter and simply continue */

		if (new_mother_iter)
		{
		    gtk_tree_iter_free (new_mother_iter);
		    new_mother_iter = NULL;
		}
		if (last_mother_iter)
		{
		    gtk_tree_iter_free (last_mother_iter);
		    last_mother_iter = NULL;
		}
		continue;
	    }

	    /* i didn't succed to open an expander here if it was already opened,
	     * so, the best way for now, we select the mother and it will be
	     * automatickly closed */

	    gsb_transactions_list_select ( transaction_number );

	    /* the 2 iter are filled, we can swap the children */

	    gsb_transactions_list_swap_children ( new_mother_iter,
						  last_mother_iter );

	    gtk_tree_iter_free (new_mother_iter);
	    gtk_tree_iter_free (last_mother_iter);
	    new_mother_iter = NULL;
	    last_mother_iter = NULL;

	    /* go to the next transaction */

	    while ( check_transaction == transaction)
	    {
		iter_next_result = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( tree_store ),
							      &iter );

		gtk_tree_model_get ( GTK_TREE_MODEL ( tree_store ),
				     &iter,
				     TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &check_transaction,
				     -1 );
	    }
	}
	while (iter_next_result);
    }
}


/**
 * change the children between 2 iters of the same transaction
 *
 * \param new_mother_iter
 * \param last_mother_iter
 *
 * \return
 * */
void gsb_transactions_list_swap_children ( GtkTreeIter *new_mother_iter,
					   GtkTreeIter *last_mother_iter )
{
    GtkTreePath *new_mother_path;
    GtkTreePath *last_mother_path;
    GtkTreeModel *tree_store;
    GtkTreeIter tmp_iter;
    GtkTreeIter child_iter;

    tree_store = GTK_TREE_MODEL (gsb_transactions_list_get_store ());

    /* we have first to check if the 2 iters are the same */

    new_mother_path = gtk_tree_model_get_path ( GTK_TREE_MODEL (tree_store),
						new_mother_iter );
    last_mother_path = gtk_tree_model_get_path ( GTK_TREE_MODEL (tree_store),
						 last_mother_iter);
    if ( !gtk_tree_path_compare ( new_mother_path,
				  last_mother_path ))
	return;

    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL (tree_store),
					&child_iter,
					last_mother_iter ))
    {
	gint return_value;

	/* didn't find any function to move the children between 2 mother, so we need to
	 * create an new row in the new mother and move all the children before or after that
	 * new row... after the new row is deleted */

	do
	{
	    gint i;

	    gtk_tree_store_append ( GTK_TREE_STORE (tree_store),
				    &tmp_iter,
				    new_mother_iter );

	    /* copy 1 row on the other. */

	    for ( i=0 ; i<TRANSACTION_COL_NB_TOTAL ; i++ )
	    {
		gpointer tmp_pointer;
		gint tmp_gint;

		switch (i)
		{
		    case TRANSACTION_COL_NB_TRANSACTION_LINE:
		    case TRANSACTION_COL_NB_VISIBLE :
			gtk_tree_model_get ( GTK_TREE_MODEL (tree_store),
					     &child_iter,
					     i, &tmp_gint,
					     -1 );
			gtk_tree_store_set ( GTK_TREE_STORE (tree_store),
					     &tmp_iter,
					     i, tmp_gint,
					     -1 );
			break;

		    default:
			gtk_tree_model_get ( GTK_TREE_MODEL (tree_store),
					     &child_iter,
					     i, &tmp_pointer,
					     -1 );
			gtk_tree_store_set ( GTK_TREE_STORE (tree_store),
					     &tmp_iter,
					     i, tmp_pointer,
					     -1 );
		}
	    }			 
	    return_value = gtk_tree_store_remove ( GTK_TREE_STORE (tree_store),
						   &child_iter );
	}
	while ( return_value );
    }
}



/** 
 * check if the transaction should be shown and do it, it's called when we add a new transaction
 * in the gtk list
 * 
 * \param transaction_number the transaction to check
 * 
 * \return TRUE if the transaction is shown, FALSE else
 * */
gboolean gsb_transactions_list_set_visibles_rows_on_transaction ( gint transaction_number )
{
    GtkTreeStore *model;
    GtkTreeIter *iter;
    gint i;

    devel_debug (g_strdup_printf ( "gsb_transactions_list_set_visibles_rows_on_transaction %d", transaction_number));

    model = gsb_transactions_list_get_store();
    iter = gsb_transaction_model_get_iter_from_transaction (transaction_number, 0 );

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
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_VISIBLE, FALSE,
				 -1 );
	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
	gtk_tree_iter_free ( iter );
	return FALSE;
    }

    /* now the transaction have to be shown completely or some lines */

    /* if it's a breakdown child, only one line and always showed
     * and set the background now */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
	gtk_tree_store_set ( GTK_TREE_STORE ( model ),
			     iter,
			     TRANSACTION_COL_NB_VISIBLE, TRUE,
			     TRANSACTION_COL_NB_BACKGROUND, &breakdown_background,
			     -1 );
    }
    else
    {
	gint show;
	gint nb_rows;

	nb_rows = gsb_data_account_get_nb_rows ( gsb_data_transaction_get_account_number (transaction_number));

	for ( i=0 ; i< TRANSACTION_LIST_ROWS_NB; i++ )
	{
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
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
				 iter,
				 TRANSACTION_COL_NB_VISIBLE, show,
				 -1 );
	    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
				       iter );
	}
    }
    gtk_tree_iter_free ( iter );
    return TRUE;
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
		if ( gsb_data_transaction_get_amount ( transaction_number).mantissa < 0 )
		    tmp = _("Transfer to a deleted account");
		else
		    tmp = _("Transfer from a deleted account");
	    }
	    else
	    {
		if ( gsb_data_transaction_get_amount (transaction_number).mantissa < 0 )
		    tmp = g_strdup_printf ( _("Transfer to %s"),
					    gsb_data_account_get_name ( gsb_data_transaction_get_account_number_transfer (transaction_number)) );
		else
		    tmp = g_strdup_printf ( _("Transfer from %s"),
					    gsb_data_account_get_name ( gsb_data_transaction_get_account_number_transfer (transaction_number)) );
	    }
	}
	else
	    /* it's a normal category */
	    tmp = my_strdup (gsb_data_category_get_name ( gsb_data_transaction_get_category_number (transaction_number),
							  gsb_data_transaction_get_sub_category_number (transaction_number),
							  NULL ));
    }
    return tmp;
}




/**
 * set the TRANSACTION_COL_NB_VISIBLE and TRANSACTION_COL_NB_BACKGROUND at the
 * right values at the opening of the expander, because if we do this when the
 * expander is closed, gtk seems to forget it...
 * it's a callback called when the signal "expand-row" is emitted
 *
 * \param tree_view
 * \param iter_in_sort the iter of the breakdown (expander) in the sorted model
 * \param path the path of the breakdown (expander) in the sorted model
 * \null not used
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_expand_row ( GtkTreeView *tree_view,
					    GtkTreeIter *iter_in_sort,
					    GtkTreePath *path,
					    gpointer null )
{
    GtkTreeIter child_iter;
    GtkTreeStore *model_tree;
    GtkTreeIter iter_in_tree;

    devel_debug ( "gsb_transactions_list_expand_row" );

    /* change from iter_in_sort to iter_in_tree */

    gsb_transaction_model_convert_sorted_iter_to_model_iter ( &iter_in_tree,
							      iter_in_sort );

    model_tree = gsb_transactions_list_get_store ();

    if ( !gtk_tree_model_iter_children ( GTK_TREE_MODEL (model_tree),
					 &child_iter,
					 &iter_in_tree ))
    {
	warning_debug ( "Opening an expander when there is no child... Strange... shouldn't happen..." );
	return FALSE;
    }

    /* set now what we have to do in the children */

    do
    {
	gtk_tree_store_set ( GTK_TREE_STORE (model_tree),
			     &child_iter,
			     TRANSACTION_COL_NB_VISIBLE, TRUE,
			     TRANSACTION_COL_NB_BACKGROUND, &breakdown_background,
			     -1 );
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL (model_tree),
				       &child_iter ));

    return FALSE;
}


/**
 * switch the expander of the breakdown given in param
 *
 * \param transaction_number the breakdown we want to switch
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_switch_expander ( gint transaction_number )
{
    GtkTreePath *path_sorted;

    if ( !gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	return FALSE;

    path_sorted = gsb_transactions_list_get_sorted_path ( transaction_number,
							  gsb_data_account_get_nb_rows (gsb_gui_navigation_get_current_account ()) -1);

    if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (transactions_tree_view), path_sorted))
	gtk_tree_view_collapse_row (GTK_TREE_VIEW (transactions_tree_view), path_sorted);
    else
	gtk_tree_view_expand_row (GTK_TREE_VIEW (transactions_tree_view), path_sorted, FALSE);

    gtk_tree_path_free (path_sorted);
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
