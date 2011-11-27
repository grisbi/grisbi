/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2009 Pierre Biava (grisbi@pierre.biava.name)     */
/*          http://www.grisbi.org                                             */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_transactions_list.h"
#include "accueil.h"
#include "affichage_liste.h"
#include "barre_outils.h"
#include "bet_data.h"
#include "custom_list.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_account.h"
#include "gsb_data_account.h"
#include "gsb_data_archive.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_form.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form.h"
#include "gsb_form_transaction.h"
#include "gsb_real.h"
#include "gsb_reconcile.h"
#include "gsb_scheduler_list.h"
#include "main.h"
#include "menu.h"
#include "mouse.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "transaction_list_sort.h"
#include "transaction_model.h"
#include "utils_dates.h"
#include "utils_operations.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean assert_selected_transaction ();
static gboolean gsb_gui_change_cell_content ( GtkWidget * item, gint *element_ptr );
static GtkWidget *gsb_gui_create_cell_contents_menu ( int x, int y );
static gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null );
static gboolean gsb_transactions_list_change_alignement ( GtkWidget *menu_item,
                        gint *no_column );
static gboolean gsb_transactions_list_change_sort_column ( GtkTreeViewColumn *tree_view_column,
                        gint *column_ptr );
static gboolean gsb_transactions_list_change_sort_type ( GtkWidget *menu_item,
                        gint *no_column );
static gboolean gsb_transactions_list_check_mark ( gint transaction_number );
static gint gsb_transactions_list_choose_reconcile ( gint account_number,
                        gint selected_reconcile_number );
static gboolean gsb_transactions_list_clone_template ( GtkWidget *menu_item,
                        gpointer null );
static gint gsb_transactions_list_clone_transaction ( gint transaction_number,
                        gint mother_transaction_number );
static GtkWidget *gsb_transactions_list_create_tree_view ( GtkTreeModel *model );
static void gsb_transactions_list_create_tree_view_columns ( void );
static void gsb_transactions_list_display_contra_transaction ( gint *transaction_number );
static gboolean gsb_transactions_list_fill_model ( void );
static gint gsb_transactions_list_get_valid_element_sort ( gint account_number,
                        gint column_number,
                        gint element_number );
static gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
                        GdkEventKey *ev );
static gboolean gsb_transactions_list_move_transaction_to_account ( gint transaction_number,
                        gint target_account );
static void gsb_transactions_list_set_tree_view (GtkWidget *tree_view);
static gboolean gsb_transactions_list_size_allocate ( GtkWidget *tree_view,
                        GtkAllocation *allocation,
                        gpointer null );
static gboolean gsb_transactions_list_switch_R_mark ( gint transaction_number );
static gboolean gsb_transactions_list_switch_mark ( gint transaction_number );
static gboolean gsb_transactions_list_title_column_button_press ( GtkWidget *button,
                        GdkEventButton *ev,
                        gint *no_column );
static gboolean move_selected_operation_to_account ( GtkMenuItem * menu_item,
                        gpointer null );
static void popup_transaction_context_menu ( gboolean full, int x, int y );
static gint schedule_transaction ( gint transaction_number );
static void update_titres_tree_view ( void );
/*END_STATIC*/


/*  adr du notebook qui contient les opés de chaque compte */
GtkWidget *tree_view_vbox = NULL;

/* the columns of the tree_view */
GtkTreeViewColumn *transactions_tree_view_columns[CUSTOM_MODEL_N_VISIBLES_COLUMN];

/* the initial width of each column */
gint transaction_col_width[CUSTOM_MODEL_N_VISIBLES_COLUMN];

/* the initial alignment of each column */
gint transaction_col_align[CUSTOM_MODEL_N_VISIBLES_COLUMN];

/* adr de la barre d'outils */
GtkWidget *barre_outils;

/* contient les tips et titres des colonnes des listes d'opé */
gchar *tips_col_liste_operations[CUSTOM_MODEL_N_VISIBLES_COLUMN];
gchar *titres_colonnes_liste_operations[CUSTOM_MODEL_N_VISIBLES_COLUMN];

/*  pointeur vers le label qui contient le solde sous la liste des opé */
GtkWidget *solde_label = NULL;

GtkWidget *solde_label_pointe = NULL;

static GtkWidget *transactions_tree_view = NULL;

gint current_tree_view_width = 0;

/** this is used when opening a file and filling the model,
 * when some children didn't find their mother */
GSList *orphan_child_transactions = NULL;

/* names of the cells */
gchar *cell_views[] = {
    N_("Date"),
    N_("Value date"),
    N_("Payee"),
    N_("Budgetary line"),
    N_("Debit"),
    N_("Credit"),
    N_("Balance"),
    N_("Amount"),
    N_("Method of payment"),
    N_("Reconciliation reference"),
    N_("Financial year"),
    N_("Category"),
    N_("C/R"),
    N_("Voucher"),
    N_("Notes"),
    N_("Bank references"),
    N_("Transaction number"),
    N_("Cheque number"),
    NULL };


/*START_EXTERN*/
extern GSList *liste_labels_titres_colonnes_liste_ope;
extern struct conditional_message messages[];
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget * navigation_tree_view;
extern GtkWidget *notebook_general;
extern GtkWidget *reconcile_sort_list_button;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];
extern GtkWidget *window;
/*END_EXTERN*/

/** All delete messages */
struct conditional_message delete_msg[] =
{
    { "delete-child-transaction", N_("Delete a child transaction."),
      NULL, 
      FALSE, FALSE, },

    { "delete-transaction",  N_("Delete a transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-child-scheduled", N_("Delete a child of scheduled transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-scheduled", N_("Delete a scheduled transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-scheduled-occurences", N_("Delete one or all occurences of scheduled "
      "transaction."),
      NULL,
      FALSE, FALSE, },

/*
    { "", N_(),
      N_(), 
      FALSE, FALSE, },
*/
    { NULL },
};


/**
 * save the transactions tree_view
 *
 * \param tree_view
 *
 * \return
 * */
void gsb_transactions_list_set_tree_view (GtkWidget *tree_view)
{
    transactions_tree_view = tree_view;
}

/**
 * get the transactions tree_view
 *
 * \param
 *
 * \return the GtkTreeView
 * */
GtkWidget *gsb_transactions_list_get_tree_view (void)
{
    return (transactions_tree_view);
}


/**
 * update the tree view, ie :
 * 	filter it according to the account
 * 	sort it
 * 	colorize it
 * 	set the balances
 * 	keep the selected transaction if we want
 * this should be called all the time, except when we need only 1 of all of this
 *
 * \param account_number
 * \param keep_selected_transaction	TRUE if we want to keep the selected transaction
 * 					else we need to select another transaction after that call
 *
 * \return
 * */
void gsb_transactions_list_update_tree_view ( gint account_number,
                        gboolean keep_selected_transaction )
{
    gint selected_transaction = 0;

    /* called sometimes with gsb_gui_navigation_get_current_account, so check we are 
     * on an account */
    if ( account_number == -1 )
        return;

    if ( keep_selected_transaction )
        selected_transaction = transaction_list_select_get ( );
    transaction_list_filter ( account_number );
    transaction_list_set_balances ( );
    transaction_list_sort ();
    transaction_list_colorize ();
    if ( conf.show_transaction_gives_balance )
        transaction_list_set_color_jour ( account_number );
    if ( keep_selected_transaction )
        transaction_list_select ( selected_transaction );
    else
        transaction_list_select ( -1 );
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
    /*     des boutons de conf au milieu, le transaction_form en bas */
    win_operations = gtk_vbox_new ( FALSE, 6 );

    /* création de la barre d'outils */
    barre_outils = gtk_handle_box_new ();
    gsb_gui_update_transaction_toolbar ();
    gtk_box_pack_start ( GTK_BOX ( win_operations ), barre_outils, FALSE, FALSE, 0);

    /* tree_view_vbox will contain the tree_view, we will see later to set it directly */
    tree_view_vbox = gtk_vbox_new ( FALSE, 0 );


    gtk_box_pack_start ( GTK_BOX ( win_operations ),
			 tree_view_vbox,
			 TRUE, TRUE, 0);

    gtk_widget_show_all (win_operations);
    return ( win_operations );
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
    GtkWidget *scrolled_window;

    /* we have first to create and fill the model */
    gsb_transactions_list_fill_model ();

    /* we add the tree view in a scrolled window which will be returned */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_SHADOW_IN );

    /* and now we can create the tree_view */
    tree_view = gsb_transactions_list_create_tree_view ( GTK_TREE_MODEL (
                        transaction_model_get_model ()) );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );

    /* we save the values */
    gsb_transactions_list_set_tree_view (tree_view);

    gtk_widget_show_all (scrolled_window);
    return scrolled_window;
}




/**
 * creates the columns of the tree_view
 * */
void gsb_transactions_list_create_tree_view_columns ( void )
{
    gint i;
    gint column_balance;

    /* get the position of the amount column to set it in red */
    column_balance = find_element_col (ELEMENT_BALANCE);

    /* create the columns */
    for ( i = 0 ; i < CUSTOM_MODEL_N_VISIBLES_COLUMN ; i++ )
    {
	GtkCellRenderer *cell_renderer;

	cell_renderer = gtk_cell_renderer_text_new ();
	g_object_set ( G_OBJECT ( cell_renderer ),
		                "xalign", ( gfloat )transaction_col_align[i]/2,
		                NULL );
	transactions_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes (
                        _(titres_colonnes_liste_operations[i]),
					    cell_renderer,
					    "text", i,
					    "cell-background-gdk", CUSTOM_MODEL_BACKGROUND,
					    "font", CUSTOM_MODEL_FONT,
					    NULL );

    g_object_set_data ( G_OBJECT ( transactions_tree_view_columns[i] ),
                        "cell_renderer", cell_renderer );

	if ( i == column_balance )
	    gtk_tree_view_column_add_attribute ( transactions_tree_view_columns[i],
						cell_renderer,
						"foreground", CUSTOM_MODEL_AMOUNT_COLOR);
	else
	    gtk_tree_view_column_add_attribute ( transactions_tree_view_columns[i],
						cell_renderer,
						"foreground-gdk", CUSTOM_MODEL_TEXT_COLOR );


	if ( i == find_element_col (ELEMENT_MARK))
	{
	    GtkCellRenderer * radio_renderer = gtk_cell_renderer_toggle_new ( );
	    gtk_tree_view_column_pack_start ( transactions_tree_view_columns[i],
					    radio_renderer,
					     FALSE );
	    gtk_tree_view_column_set_attributes (transactions_tree_view_columns[i], radio_renderer,
						"active", CUSTOM_MODEL_CHECKBOX_ACTIVE,
						"activatable", CUSTOM_MODEL_CHECKBOX_VISIBLE,
						"visible", CUSTOM_MODEL_CHECKBOX_VISIBLE,
						"cell-background-gdk", CUSTOM_MODEL_BACKGROUND,
						NULL );
	    g_object_set_data ( G_OBJECT ( transactions_tree_view_columns[i] ),
                        "radio_renderer", radio_renderer );
	}

	gtk_tree_view_column_set_alignment ( transactions_tree_view_columns[i],
					     ( gfloat )transaction_col_align[i]/2 );

    gtk_tree_view_column_set_sizing ( transactions_tree_view_columns[i],
					    GTK_TREE_VIEW_COLUMN_FIXED );
	gtk_tree_view_column_set_resizable ( transactions_tree_view_columns[i],
					    TRUE );
    }

}


/**
 * update the titles of the tree view
 *
 * \param
 *
 * \return
 * */
void update_titres_tree_view ( void )
{
    gint i;

    for ( i = 0 ; i < CUSTOM_MODEL_N_VISIBLES_COLUMN ; i++ )
    {
        gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN (
                        transactions_tree_view_columns[i] ),
                        _(titres_colonnes_liste_operations[i] ) );

        if ( GTK_TREE_VIEW_COLUMN ( transactions_tree_view_columns[i] )->button )
        {
            gtk_widget_set_tooltip_text ( GTK_WIDGET ( GTK_TREE_VIEW_COLUMN (
                        transactions_tree_view_columns[i] )->button ),
                        tips_col_liste_operations[i] );
        }
    }
}



/**
 * create the tree view from the CustomList
 *
 * \param model	the CustomList
 *
 * \return the tree_view
 * */
GtkWidget *gsb_transactions_list_create_tree_view ( GtkTreeModel *model )
{
    GtkWidget *tree_view;
    gint i;

    tree_view = gtk_tree_view_new ();

    /*  we cannot do a selection */
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection (
                        GTK_TREE_VIEW( tree_view ))),
				        GTK_SELECTION_NONE );

    gtk_tree_view_set_fixed_height_mode ( GTK_TREE_VIEW ( tree_view ), TRUE );

    /* check the buttons on the list */
    g_signal_connect ( G_OBJECT ( tree_view ),
		                "button_press_event",
		                G_CALLBACK ( gsb_transactions_list_button_press ),
		                NULL );

    /* check the keys on the list */
    g_signal_connect ( G_OBJECT ( tree_view ),
		                "key_press_event",
		                G_CALLBACK ( gsb_transactions_list_key_press ),
		                NULL );

    g_signal_connect ( G_OBJECT ( tree_view ),
		                "size_allocate",
		                G_CALLBACK (gsb_transactions_list_size_allocate),
		                NULL );

    /* we create the columns of the tree view */
    gsb_transactions_list_create_tree_view_columns ();

    for ( i = 0 ; i < CUSTOM_MODEL_N_VISIBLES_COLUMN ; i++ )
    {
	    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
				        transactions_tree_view_columns[i] );

	    gtk_tree_view_column_set_clickable ( transactions_tree_view_columns[i],
					    TRUE );

	/* 	    set the tooltips */
	    gtk_widget_set_tooltip_text ( GTK_WIDGET (transactions_tree_view_columns[i] -> button),
				        tips_col_liste_operations[i] );

	    g_signal_connect ( G_OBJECT ( transactions_tree_view_columns[i] -> button),
			            "button-press-event",
			            G_CALLBACK ( gsb_transactions_list_title_column_button_press ),
			            GINT_TO_POINTER ( i ) );

	    /* use the click to sort the list */
	    g_signal_connect ( G_OBJECT ( transactions_tree_view_columns[i] ),
			            "clicked",
			            G_CALLBACK ( gsb_transactions_list_change_sort_column ),
			            GINT_TO_POINTER ( i ) );
    }

    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ),
			            GTK_TREE_MODEL ( model ) );
    return tree_view;
}


/**
 * fill the new store with the all the transactions
 * normally called at the opening of a file
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_model ( void )
{
    GSList *tmp_list;
    gint transaction_number;

    devel_debug (NULL);

    /* add the transations wich represent the archives to the store
     * 1 line per archive and per account */
    gsb_transactions_list_fill_archive_store ();

    orphan_child_transactions = NULL;

    /* add the transactions to the store */
    tmp_list = gsb_data_transaction_get_transactions_list ();
    while (tmp_list)
    {
	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	transaction_list_append_transaction (transaction_number);
	tmp_list = tmp_list -> next;
    }

    /* if orphan_child_transactions if filled, there are some children wich didn't fing their
     * mother, we try again now that all the mothers are in the model */
    if (orphan_child_transactions)
    {
	GSList *orphan_list_copy;

	orphan_list_copy = g_slist_copy (orphan_child_transactions);
	g_slist_free (orphan_child_transactions);
	orphan_child_transactions = NULL;

	tmp_list = orphan_list_copy;
	while (tmp_list)
	{
	    transaction_number = GPOINTER_TO_INT (tmp_list -> data);
	    transaction_list_append_transaction (transaction_number);
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (orphan_list_copy);

	/* if orphan_child_transactions is not null, there is still some children
	 * wich didn't find their mother. show them now */
	if (orphan_child_transactions)
	{
	    gchar *message = _("Some children didn't find their mother in the list, this "
                           "shouldn't happen and there is probably a bug behind that. "
                           "Please contact the Grisbi team.\n\nThe concerned children "
                           "number are :\n");
	    gchar *string_1;
	    gchar *string_2;

	    string_1 = g_strconcat (message, NULL);
	    tmp_list = orphan_child_transactions;
	    while (tmp_list)
	    {
		string_2 = g_strconcat ( string_1,
					 utils_str_itoa (GPOINTER_TO_INT (tmp_list -> data)),
					 " - ",
					 NULL);
		g_free (string_1);
		string_1 = string_2;
		tmp_list = tmp_list -> next;
	    }
	    dialogue_warning (string_1);
	    g_free (string_1);
	}
    }
    return FALSE;
}


/**
 * fill the store with the archive
 * it's only 1 line per archive, with a name,
 * a date (the initial date) and an amount (the balance of the archive for the account)
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_fill_archive_store ( void )
{
    GSList *tmp_list;

    devel_debug (NULL);

    tmp_list = gsb_data_archive_store_get_archives_list ();
    while (tmp_list)
    {
    gint archive_store_number;

    /* get the store archive struct */
    archive_store_number = gsb_data_archive_store_get_number (tmp_list -> data);
    transaction_list_append_archive (archive_store_number);
    tmp_list = tmp_list -> next;
    }
    return FALSE;
}



/**
 * append a new transaction in the tree_view
 * use gsb_transactions_list_append_transaction and do all the stuff
 * 	arround that, ie calculate the balances, show or not the line...
 * 	so this function is very good to append 1 transactions, but to append
 * 	several transactions, it's better to use gsb_transactions_list_append_transaction
 * 	and do the stuff separatly
 *
 * if the transaction is a split, append a white line and open
 * the split to see the daughters
 *
 * \param transaction_number
 * \param update_tree_view	should be TRUE, except if we append a lot of transactions (import...)
 * 				this must be FALSE to avoid time consuming, and need to update tree view
 * 				later
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number,
                        gboolean update_tree_view )
{
    gint account_number;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* append the transaction to the tree view */
    transaction_list_append_transaction (transaction_number);

    /* update the transaction list only if the account is showed,
     * else it's because we execute a scheduled transaction and all
     * of that stuff will be done when we will show the account */
    if (update_tree_view
     &&
     gsb_gui_navigation_get_current_account () == account_number
     &&
     !gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
        gsb_transactions_list_update_tree_view ( account_number, TRUE );
        gsb_data_account_colorize_current_balance ( account_number );

        /* if it's a mother, open the expander */
        if (gsb_data_transaction_get_split_of_transaction (transaction_number))
            gsb_transactions_list_switch_expander (transaction_number);
    }

    /* on réaffichera l'accueil */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;
    return FALSE;
}




/**
 * take in a transaction the content to set in a cell of the transaction's list
 * all the value are dupplicate and have to be freed after use (except when NULL)
 *
 * \param transaction_number
 * \param cell_content_number what we need in the transaction
 *
 * \return a newly allocated string which represent the content of the transaction, or NULL
 * */
gchar *gsb_transactions_list_grep_cell_content ( gint transaction_number,
                        gint cell_content_number )
{
    gint account_currency;

    /* for a child of split, we show only the category (instead of party or category), the
     * debit and credit, nothing else */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
	switch (cell_content_number)
	{
	    case ELEMENT_PARTY:
		/* here want to show the category instead of the payee */
		cell_content_number = ELEMENT_CATEGORY;
		break;

	    case ELEMENT_CATEGORY:
	    case ELEMENT_DEBIT:
	    case ELEMENT_CREDIT:
		break;

	    default:
		return NULL;
	}
    }

    switch ( cell_content_number )
    {
	/* mise en forme de la date */

	case ELEMENT_DATE:
	    return gsb_format_gdate(gsb_data_transaction_get_date (transaction_number));

	    /* mise en forme de la date de valeur */

	case ELEMENT_VALUE_DATE:
	    return gsb_format_gdate(gsb_data_transaction_get_value_date (transaction_number));

	    /* mise en forme du tiers */

	case ELEMENT_PARTY:
	    return ( my_strdup (gsb_data_payee_get_name ( gsb_data_transaction_get_party_number (transaction_number), TRUE )));

	    /* mise en forme de l'ib */

	case ELEMENT_BUDGET:
	    return ( my_strdup (gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number (transaction_number),
							   gsb_data_transaction_get_sub_budgetary_number (transaction_number),
							   NULL )));

	case ELEMENT_DEBIT:
	case ELEMENT_CREDIT:
	    /* give the amount of the transaction on the transaction currency */
	    if ( (cell_content_number == ELEMENT_DEBIT
		  &&
		  gsb_data_transaction_get_amount ( transaction_number).mantissa < 0 )
		 ||
		 (cell_content_number == ELEMENT_CREDIT
		  &&
		  gsb_data_transaction_get_amount ( transaction_number).mantissa >= 0 ))
		    return gsb_real_get_string_with_currency (
                        gsb_real_abs ( gsb_data_transaction_get_amount ( transaction_number ) ),
					    gsb_data_transaction_get_currency_number ( transaction_number ), TRUE );
	    else
		return NULL;
	    break;

	case ELEMENT_BALANCE:
	    return NULL;

	case ELEMENT_AMOUNT:
	    /* give the amount of the transaction in the currency of the account */
	    account_currency = gsb_data_account_get_currency (
                        gsb_data_transaction_get_account_number ( transaction_number ) );
	    if ( account_currency != gsb_data_transaction_get_currency_number (
         transaction_number ) )
	    {
            gchar* tmpstr;
            gchar* result;

            tmpstr = gsb_real_get_string (
                        gsb_data_transaction_get_adjusted_amount_for_currency (
                        transaction_number,
                        account_currency,
                        gsb_data_currency_get_floating_point ( account_currency ) ) );
            result = g_strconcat ( "(",
                        tmpstr,
                        " ",
                        gsb_data_currency_get_code_or_isocode ( account_currency ),
					    ")",
					    NULL );
            g_free ( tmpstr );
            return result;
	    }
	    else
            return NULL;
	    break;

	    /* mise en forme du moyen de paiement */

	case ELEMENT_PAYMENT_TYPE:
	    return ( my_strdup (gsb_data_payment_get_name ( gsb_data_transaction_get_method_of_payment_number ( transaction_number))));

	    /* mise en forme du no de rapprochement */

	case ELEMENT_RECONCILE_NB:
	    return ( my_strdup (gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( transaction_number))));

	    /* mise en place de l'exo */

	case ELEMENT_EXERCICE:
	    return ( my_strdup (gsb_data_fyear_get_name (gsb_data_transaction_get_financial_year_number ( transaction_number))));

	    /* mise en place des catégories */

	case ELEMENT_CATEGORY:

	    return ( gsb_transactions_get_category_real_name ( transaction_number ));

	    /* mise en forme R/P */

	case ELEMENT_MARK:
	    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== OPERATION_POINTEE )
		return ( my_strdup (_("P")));
	    else
	    {
		if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== OPERATION_TELERAPPROCHEE )
		    return ( my_strdup (_("T")));
		else
		{
		    if ( gsb_data_transaction_get_marked_transaction ( transaction_number)== OPERATION_RAPPROCHEE )
			return ( my_strdup (_("R")));
		    else
			return ( NULL );
		}
	    }
	    break;

	    /* mise en place de la pièce comptable */

	case ELEMENT_VOUCHER:
	    return ( my_strdup (gsb_data_transaction_get_voucher ( transaction_number)));

	    /* mise en forme des notes */

	case ELEMENT_NOTES:
	    return ( my_strdup (gsb_data_transaction_get_notes ( transaction_number)));

	    /* mise en place de l'info banque/guichet */

	case ELEMENT_BANK:
	    return ( my_strdup (gsb_data_transaction_get_bank_references ( transaction_number)));

	    /* mise en place du no d'opé */

	case ELEMENT_NO:
	    return ( utils_str_itoa ( transaction_number ));

	    /* mise en place du no de chèque/virement */

	case ELEMENT_CHQ:
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
    gint account_number;

    devel_debug_int (transaction_number);

    /* first, modify the transaction in the tree view */
    transaction_list_update_transaction (transaction_number);

    /* update the balances */
    transaction_list_set_balances ();

    account_number = gsb_data_transaction_get_account_number (transaction_number);
    gsb_data_account_colorize_current_balance ( account_number );

    /* update first page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}



/**
 * calculate the row_align of the current selected transaction
 * to set it later with gtk_tree_view_scroll_to_cell
 *
 * \param account_number
 *
 * \return a gfloat used as row_align
 * */
gfloat gsb_transactions_list_get_row_align ( void )
{
    GtkTreePath *path;
    GtkTreeView *tree_view;
    GdkRectangle back_rect;
    gint height_win;
    gfloat row_align;
    gfloat height, rect_y;

    devel_debug (NULL);

    tree_view = GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ());
    path = transaction_list_select_get_path (0);

    if (!path)
	return 0;

    gdk_drawable_get_size ( GDK_DRAWABLE (gtk_tree_view_get_bin_window (tree_view)),
			    NULL, &height_win);
    gtk_tree_view_get_background_area (tree_view, path, NULL, &back_rect);
    gtk_tree_path_free (path);

    /* i didn't find another hack to transform gint to float */
    height = height_win;
    rect_y = back_rect.y;

    row_align = rect_y/height;
    return (row_align);
}



/**
 * set the list at the good position saved before with row_align
 *
 * \param row_align	a gfloat to aligne in the function gtk_tree_view_scroll_to_cell
 * 			if < 0, don't use row_align but let the tree view to move by himself
 * 			to the selected transaction
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_set_row_align ( gfloat row_align )
{
    GtkTreePath *path;
    gint transaction_number;
    gint mother_transaction;
    gint account_number;

    /* devel_debug (NULL); */

    if ( ( account_number = gsb_gui_navigation_get_current_account ( ) ) == -1)
	    return FALSE;

    /* if we just want to let the tree view by himself, it's here
     * we get the path of the last line in transaction because untill now,
     * we do that when open the form, so only the last line interest us */
    if (row_align < 0)
    {
        path = transaction_list_select_get_path ( transaction_list_get_last_line (
                            gsb_data_account_get_nb_rows ( account_number ) ) );
        if ( path )
        {
            gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
                           path, NULL,
                           FALSE, 0.0, 0.0 );
            gtk_tree_path_free (path);
        }
        return FALSE;
    }

    /* ok, we want to use row_align */
    transaction_number = transaction_list_select_get ();

    /* if we are on a child, open the mother */
    mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);
    if (mother_transaction)
	    gsb_transactions_list_switch_expander (mother_transaction);

    /* if we are on white line, go to the end directly,
     * else at the opening, the white line is hidden */
    if (transaction_number == -1)
    {
        path = transaction_list_select_get_path ( transaction_list_get_last_line (
                            gsb_data_account_get_nb_rows ( account_number ) ) );
        row_align = 1.0;
    }
    else
	    path = transaction_list_select_get_path (0);

    /* we need to use scroll_to_cell function because at this stade,
     * the tree view is not refreshed so all value with alignment don't work
     * but scroll_to_cell will place us on the good position despite that */
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
				   path, NULL,
				   TRUE, row_align, 0.0 );
    gtk_tree_path_free (path);
    return FALSE;
}



/**
 * find column number for the transaction element number
 *
 * \param element_number the element we look for
 *
 * \return column number or -1 if the element is not shown
 * */
gint find_element_col ( gint element_number )
{
    gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j<CUSTOM_MODEL_N_VISIBLES_COLUMN ; j++ )
	{
	    if ( tab_affichage_ope[i][j] == element_number )
		return j;
	}
    }

    return -1;
}

/** 
 * find line number for the transaction element number
 *
 * \param element_number the element we look for
 *
 * \return line number or -1 if the element is not shown
 * */
gint find_element_line ( gint element_number )
{
    gint i, j;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	for ( j=0 ; j<CUSTOM_MODEL_N_VISIBLES_COLUMN ; j++ )
	{
	    if ( tab_affichage_ope[i][j] == element_number )
		return i;
	}
    }

    return -1;
}

/**
 * find column number for the element, but for split transaction
 * there is no line find because only 1 line
 * for now, only payee, debit and credit are shown in a split child
 *
 * \param element_number the element we look for in a split child
 *
 * \return column number or -1 if the element is not shown
 * */
gint find_element_col_split ( gint element_number )
{
    switch (element_number)
    {
	case ELEMENT_CATEGORY:
	    return CUSTOM_MODEL_COL_2;

	case ELEMENT_CREDIT:
	    return CUSTOM_MODEL_COL_4;

	case ELEMENT_DEBIT:
	    return CUSTOM_MODEL_COL_5;
    }
    return -1;
}




/**
 * cette fonction calcule le solde de départ pour l'affichage de la première opé
 * du compte
 * c'est soit le solde initial du compte si on affiche les R
 * soit le solde initial - les opés R si elles ne sont pas affichées
 * */
gsb_real solde_debut_affichage ( gint account_number,
                        gint floating_point)
{
    gsb_real solde;
    GSList *list_tmp_transactions;

    solde = gsb_data_account_get_init_balance ( account_number, floating_point );

    if ( gsb_data_account_get_l ( account_number ) == 0 )
        solde = gsb_real_add ( solde,
                        gsb_data_archive_store_get_archives_balance ( account_number ) );

    if ( gsb_data_account_get_r ( account_number ) )
	    return solde;

    /* the marked R transactions are not showed, add their balance to the initial balance */
    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
        gint transaction_number_tmp;
        transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

        /* 	si l'opé est ventilée ou non relevée, on saute */

        if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number
             &&
             !gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp)
             &&
             gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE )
            solde = gsb_real_add ( solde,
                        gsb_data_transaction_get_adjusted_amount ( transaction_number_tmp,
                        floating_point ) );

        list_tmp_transactions = list_tmp_transactions -> next;
    }

    return ( solde );
}


/**
 * called when press a mouse button on the transactions list
 *
 * \param tree_view
 * \param ev a GdkEventButton
 *
 * \return normally TRUE to block the signal, if we are outside the tree_view, FALSE
 * */
gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gpointer transaction_pointer;
    gint transaction_number, column, line_in_transaction;
    gint what_is_line;

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
					  &path,
					  &tree_column,
					  NULL,
					  NULL ))
    {
	/* show the partial popup */
	if ( ev -> button == RIGHT_BUTTON )
	    popup_transaction_context_menu ( FALSE, -1, -1 );
	return (TRUE);
    }

    /* ok we are on the list, get the transaction */
    model = GTK_TREE_MODEL ( transaction_model_get_model());

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get ( model, &iter,
			 CUSTOM_MODEL_TRANSACTION_ADDRESS, &transaction_pointer,
			 CUSTOM_MODEL_TRANSACTION_LINE, &line_in_transaction,
			 CUSTOM_MODEL_WHAT_IS_LINE, &what_is_line,
			 -1 );

    /* for now for an archive, check the double-click */
    if (what_is_line == IS_ARCHIVE)
    {
	if (ev -> type == GDK_2BUTTON_PRESS)
	{
	    gint archive_number;
	    const gchar *name;

	    archive_number = gsb_data_archive_store_get_archive_number (gsb_data_archive_store_get_number (transaction_pointer));

	    /* i don't know why but i had a crash with a NULL name of archive, so prevent here */
	    name = gsb_data_archive_get_name (archive_number);
	    if (name)
	    {
	        gchar* tmpstr = g_strdup_printf (
                        _("Do you want to add the transactions of the archive %s "
                        "into the list ?"),
                        name );
		if (question_yes_no ( tmpstr , GTK_RESPONSE_CANCEL ))
		    gsb_transactions_list_restore_archive (archive_number, TRUE);
		g_free(tmpstr);
	    }
	    else
	    {
		if (archive_number)
		    warning_debug ( _("An archive was clicked but Grisbi is unable to get the name. "
                        "It seems like a bug.\nPlease try to reproduce and contact the "
                        "Grisbi team.") );
		else
		    warning_debug ( _("An archive was clicked but it seems to have the number 0, "
                        "wich should not happen.\nPlease try to reproduce and contact "
                        "the Grisbi team.") );
	    }
	}
	return FALSE;
    }

    /* it's not an archive, so it's a transaction and select it */
    transaction_number = gsb_data_transaction_get_transaction_number (transaction_pointer);
    transaction_list_select ( transaction_number );

    /* get the column */
    column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
			    tree_column );

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
	 column == find_element_col (ELEMENT_MARK)
	 &&
	 (( etat.equilibrage
	    &&
	    line_in_transaction == find_element_line (ELEMENT_MARK))
	  ||
	  (( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )))
    {
	gsb_transactions_list_switch_mark (transaction_number);
    transaction_list_set_balances ( );
	return TRUE;
    }

    /*  if double - click */
    if ( ev -> type == GDK_2BUTTON_PRESS )
	gsb_transactions_list_edit_transaction (transaction_number);

    /* have to return FALSE to continue the signal and open eventually
     * a child in the tree... but returning FALSE make the selection slower...
     * so check here x of mouse, if <16, so at left of list, perhaps it's
     * for open a child, so return FALSE, in other case return TRUE because
     * much much faster.*/
    if ( ev -> x <16 )
	return FALSE;

    return TRUE;
}


/**
 * gère le clavier sur la liste des opés
 * 
 * */
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
                        GdkEventKey *ev )
{
    gint account_number;
    gint transaction_number;

    account_number = gsb_gui_navigation_get_current_account ();

    switch ( ev -> keyval )
    {
    case GDK_Return :   /* entrée */
    case GDK_KP_Enter :
    case GDK_Tab :

        gsb_transactions_list_edit_transaction (
                        gsb_data_account_get_current_transaction_number ( account_number ) );
        break;

    case GDK_Up :       /* touches flèche haut */
    case GDK_KP_Up :

        transaction_list_select_up (FALSE);
        break;

    case GDK_Down :     /* touches flèche bas */
    case GDK_KP_Down :

        transaction_list_select_down (FALSE);
        break;

    case GDK_Delete:    /*  del  */
        gsb_transactions_list_delete_transaction (
                        gsb_data_account_get_current_transaction_number ( account_number ),
                        TRUE );
        break;

    case GDK_P:         /* touche P */
    case GDK_p:         /* touche p */
    case GDK_F12:       /* touche F12 pour pointer dépointer comme avec <ctrl>p*/ 

        if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK
            ||
            ev -> keyval == GDK_F12 )
        {
            gsb_transactions_list_switch_mark (
                        gsb_data_account_get_current_transaction_number ( account_number ) );
            transaction_list_set_balances ( );
        }
        break;

    case GDK_r:         /* touche r */
    case GDK_R:         /* touche R */

        if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
        {
            gsb_transactions_list_switch_R_mark (
                            gsb_data_account_get_current_transaction_number ( account_number ) );
            transaction_list_set_balances ( );
        }
        break;

    case GDK_t:         /* touche t */
    case GDK_T:         /* touche T */
        transaction_list_select ( -1 );
        break;

    case GDK_space:
        transaction_number = gsb_data_account_get_current_transaction_number ( account_number );
        if ( transaction_number > 0 )
        {
        if ( etat.equilibrage )
        {
            /* we are reconciling, so mark/unmark the transaction */
            gsb_transactions_list_switch_mark ( transaction_number );
            transaction_list_select_down ( FALSE );
        }
        else
            /* space open/close a split */
            gsb_transactions_list_switch_expander ( transaction_number );
        }
        break;

    case GDK_Left:
        /* if we press left, give back the focus to the tree at left */
        gtk_widget_grab_focus ( navigation_tree_view );
        break;

    case GDK_Home:
    case GDK_KP_Home:
        gtk_tree_view_scroll_to_point ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view ( ) ), 0, 0 );
        break;

    case GDK_End:
    case GDK_KP_End:
        gtk_tree_view_scroll_to_point ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view ( ) ), -1, 1024 );
        break;

    }

    return TRUE;
}


/**
 * called when the selection changed
 * do some stuff wich happen in that case
 *
 * \param new_selected_transaction
 *
 * \return
 * */
void gsb_transactions_list_selection_changed ( gint new_selected_transaction )
{
    gint account_number;

    devel_debug_int (new_selected_transaction);

    /* the white number has no account number, so we take the current account */
    if ( new_selected_transaction != -1 )
    {
    account_number = gsb_data_transaction_get_account_number (new_selected_transaction);
    gsb_menu_transaction_operations_set_sensitive ( TRUE );
    }
    else
    {
    account_number = gsb_gui_navigation_get_current_account ();
    gsb_menu_transaction_operations_set_sensitive ( FALSE );
    }

    /* save the new current transaction */
    gsb_data_account_set_current_transaction_number ( account_number,
                        new_selected_transaction);

    /* show the content of the transaction in the form,
     * only if the form is shown */
    if ( conf.show_transaction_selected_in_form
    &&
    gsb_form_is_visible () )
        gsb_form_fill_by_transaction (new_selected_transaction, TRUE, FALSE);

    /* give the focus to the transaction_tree_view pbiava 02/09/2009 
     * edit due to a regression loss of <CtrlR> */
    if ( transactions_tree_view )
        gtk_widget_grab_focus ( transactions_tree_view );
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
    devel_debug_int (transaction_number);

    gsb_gui_navigation_set_selection ( GSB_ACCOUNT_PAGE,
                        gsb_data_transaction_get_account_number ( transaction_number ),
                        NULL );
    transaction_list_select ( transaction_number );
    gsb_form_fill_by_transaction ( transaction_number, TRUE, TRUE );

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
    devel_debug_int (GPOINTER_TO_INT (transaction_number));
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
    devel_debug (NULL);
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
    gint col;
    gsb_real amount;
    gint account_number;

    devel_debug_int (transaction_number);

    /* if no P/R column, cannot really mark/unmark the transaction... */
    col = find_element_col (ELEMENT_MARK);
    if ( col == -1 )
	    return FALSE;

    /* if we are on the white line, a R transaction or a child of split, do nothing */
    if ( transaction_number == -1
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_mother_transaction_number (transaction_number))
	return FALSE;

    account_number = gsb_gui_navigation_get_current_account ();
    amount = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);

    if (gsb_data_transaction_get_marked_transaction (transaction_number))
    {
        gsb_data_transaction_set_marked_transaction ( transaction_number,
                                  OPERATION_NORMALE );
    }
    else
    {
        gsb_data_transaction_set_marked_transaction ( transaction_number,
                                  OPERATION_POINTEE );
    }

    transaction_list_update_transaction (transaction_number);

    /* if it's a split, set the mark to the children */
    if ( gsb_data_transaction_get_split_of_transaction ( transaction_number))
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
    {
    /* pbiava 02/12/2009 : shows the balance after you mark the transaction */
    transaction_list_set_balances (  );
	gsb_reconcile_update_amounts (NULL, NULL);
    }
    /* need to update the marked amount on the home page */
    gsb_navigation_update_statement_label ( account_number );
    mise_a_jour_liste_comptes_accueil = 1;

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

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
    GtkTreeIter iter;
    gint r_column;
    GtkTreeModel *model;
    gint msg_no = 0;
    gchar *tmp_str;

    r_column = find_element_col (ELEMENT_MARK);
    if ( r_column == -1 )
	return FALSE;

    /* if we are on the white line or a child of split, do nothing */
    if ( transaction_number == -1 )
	return FALSE;

    /* if we are reconciling, cancel the action */
    if (etat.equilibrage)
    {
        dialogue_error ( _("You cannot switch a transaction between R and non R "
                         "while reconciling.\nPlease finish or cancel the "
                         "reconciliation first.") );
        return FALSE;
    }

    /* if it's a child, we ask if we want to work with the mother */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
        msg_no = question_conditional_yes_no_get_no_struct ( &messages[0],
                        "reconcile-transaction" );
    	tmp_str = g_strdup_printf ( 
				   _("You are trying to reconcile or unreconcile a transaction manually, "
				     "which is not a recommended action.This is the wrong approach.\n\n"
				     "And moreover the transaction you try to reconcile is a child of split, so "
				     "the modification will be done on the mother and all its children.\n\n"
				     "Are you really sure to know what you do?") );
        messages[msg_no].message = tmp_str;
        if ( question_conditional_yes_no_with_struct ( &messages[msg_no] ) )
        {
            /* he says ok, so transaction_number becomes the mother */
            transaction_number = gsb_data_transaction_get_mother_transaction_number (
                        transaction_number );
            g_free ( tmp_str );
        }
	    else
        {
            g_free ( tmp_str );
	        return FALSE;
        }
    }
    else
	    /* it's a normal transaction, ask to be sure */
        if ( !question_conditional_yes_no ( "reconcile-transaction" ) )
	        return FALSE;

    model = GTK_TREE_MODEL (transaction_model_get_model());

    if (!transaction_model_get_transaction_iter ( &iter,
						  transaction_number, 0))
	return FALSE;

    account_number = gsb_gui_navigation_get_current_account ();
    amount = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);

    if ( gsb_data_transaction_get_marked_transaction ( transaction_number) == OPERATION_RAPPROCHEE)
    {
	/* ok, this is a R transaction, we just un-R it but keep the reconcile_number into the transaction */
	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      OPERATION_NORMALE );
	transaction_list_set ( &iter, r_column, NULL, -1 );
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

	gsb_data_transaction_set_marked_transaction ( transaction_number,
						      OPERATION_RAPPROCHEE );
	gsb_data_transaction_set_reconcile_number ( transaction_number,
						    reconcile_number );

	/* set the R on the transaction */
	transaction_list_set ( &iter, r_column, g_strdup ( _("R") ), -1 );

	/* if we don't want to see the marked R transactions, we re-filter the model */
	if ( !gsb_data_account_get_r (account_number) )
	{
	    transaction_list_select_down (FALSE);
	    gsb_data_account_set_current_transaction_number ( account_number,
							      transaction_list_select_get ());
	    gsb_transactions_list_update_tree_view (account_number, TRUE);

	    /* we warn the user the transaction disappear
	     * don't laugh ! there were several bugs reports about a transaction wich disappear :-) */
	    dialogue_hint ( _("The transaction has disappear from the list...\nDon't worry, it's because you marked it as R, and you choosed not to show the R transactions into the list ; show them if you want to check what you did."),
			   _("Marking a transaction as R"));
	}
    }

    /* if it's a split, set the mark to the children */
    if ( gsb_data_transaction_get_split_of_transaction (transaction_number))
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
        transaction_list_update_element ( ELEMENT_MARK );
    }
    /* need to update the marked amount on the home page */
    mise_a_jour_liste_comptes_accueil = 1;

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

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

    dialog = gtk_dialog_new_with_buttons ( _("Selection of a reconciliation"),
					   GTK_WINDOW ( window ),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_default_size ( GTK_WINDOW ( dialog ), 770, 412 );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );
    gtk_container_set_border_width ( GTK_CONTAINER ( dialog ), 12 );

    label = gtk_label_new ( _("Select the reconciliation to associate to the selected transaction: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 label,
			 FALSE, FALSE, 10 );
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
    g_object_unref ( G_OBJECT(store) );
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
 * if it's a split, delete the childs
 *
 * \param transaction The transaction to delete
 * \param show_warning TRUE to ask if the user is sure, FALSE directly delete the transaction
 * 			if TRUE, the form will be reset too
 *
 * \return FALSE if canceled or nothing done, TRUE if ok
 * */
gboolean gsb_transactions_list_delete_transaction ( gint transaction_number,
                        gint show_warning )
{
    gchar *tmpstr;
    gint account_number;
    gint msg_no = 0;

    devel_debug_int (transaction_number);

    /* we cannot delete the general white line (-1), but all the others white lines are possibles
     * if show_warning it FALSE (ie this is automatic, and not done by user action */
    if ( !transaction_number
	 ||
	 transaction_number == -1 )
	return FALSE;

    /* if we cannot ask for a white line, so this shouldn't append,
     * if we want to delete a child white line, show_warning must be FALSE (force) */
    if ( show_warning
	 &&
	 transaction_number < 0 )
	return FALSE;

    /* if the transaction is archived, cannot delete it */
    if (gsb_data_transaction_get_archive_number (transaction_number))
    {
	dialogue_error ( _("Impossible to delete an archived transaction.") );
	return FALSE;
    }

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* check if the transaction is not reconciled */
    if ( gsb_transactions_list_check_mark (transaction_number))
    {
	dialogue_error ( _("Impossible to delete a reconciled transaction.\nThe transaction, "
                        "the contra-transaction or the children if it is a split are "
                        "reconciled. You can remove the reconciliation with Ctrl R if "
                        "it is really necessary.") );
	return FALSE;
    }

    /* show a warning */
    if (show_warning)
    {
        if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
        {
            msg_no = question_conditional_yes_no_get_no_struct ( &delete_msg[0],
                        "delete-child-transaction" );
            tmpstr = g_strdup_printf (
                        _("Do you really want to delete the child of the transaction "
                        "with party '%s' ?"),
                        gsb_data_payee_get_name (
                        gsb_data_transaction_get_party_number ( transaction_number ),
                        FALSE ) );
            delete_msg[msg_no].message = tmpstr;
        }
        else
        {
            msg_no = question_conditional_yes_no_get_no_struct ( &delete_msg[0],
                        "delete-transaction" );
            tmpstr = g_strdup_printf (
                         _("Do you really want to delete transaction with party '%s' ?"),
                         gsb_data_payee_get_name (
                         gsb_data_transaction_get_party_number ( transaction_number),
                                             FALSE ) );
            delete_msg[msg_no].message = tmpstr;
        }
        if ( !question_conditional_yes_no_with_struct ( &delete_msg[msg_no] ) )
        {
            g_free(tmpstr);
            return FALSE;
        }
        g_free(tmpstr);
    }

    /* move the selection */
    if (transaction_list_select_get () == transaction_number)
    {
	transaction_list_select_down (FALSE);
	gsb_data_account_set_current_transaction_number ( account_number,
							  transaction_list_select_get ());
    }

    /* delete the transaction from the tree view,
     * all the children and contra transaction will be removed with that */
    gsb_transactions_list_delete_transaction_from_tree_view (transaction_number);

    /*  update the metatrees, this MUST be before remove_transaction */
    delete_transaction_in_trees (transaction_number);

    /* delete the transaction in memory,
     * all the children and contra transaction will be removed with that */
    gsb_data_transaction_remove_transaction ( transaction_number);

    /* update the tree view */
    transaction_list_colorize ();
    if ( conf.show_transaction_gives_balance )
        transaction_list_set_color_jour ( account_number );
    transaction_list_set_balances ();
    transaction_list_select (gsb_data_account_get_current_transaction_number (account_number));

    /* if we are reconciling, update the amounts */
    if ( etat.equilibrage )
	gsb_reconcile_update_amounts (NULL, NULL);

    /* we will update the home page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    affiche_dialogue_soldes_minimaux ();

    /* We blank form. */
    if (show_warning)
	gsb_form_escape_form ();

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return TRUE;
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
    gint contra_transaction_number;

    /* vérifications de bases */
    if (transaction_number <= 0)
	return FALSE;

    if ( gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE )
	return TRUE;

    /* if it's a transfer, check the contra-transaction */
    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);
    if ( contra_transaction_number > 0 )
    {
	if (gsb_data_transaction_get_marked_transaction (contra_transaction_number) == OPERATION_RAPPROCHEE )
	    return TRUE;
    }

    /* if it's a split of transaction, check all the children
     * if there is not a transfer which is marked */
    if ( gsb_data_transaction_get_split_of_transaction ( transaction_number))
    {
	GSList *list_tmp_transactions;
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (transaction_number)
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
	    {
		/* transactions_tmp is a child of transaction */
		if ( gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE )
		    return TRUE;

		contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp);

		if ( contra_transaction_number > 0)
		{
		    /* the split is a transfer, we check the contra-transaction */
		    if ( gsb_data_transaction_get_marked_transaction (contra_transaction_number)== OPERATION_RAPPROCHEE )
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
 * if it's a split, delete a the children and if necessary the contra transactions
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
    gint contra_transaction_number;

    /* devel_debug_int (transaction_number); */

    if ( transaction_number == -1 )
	return FALSE;

    /* if the transaction is a transfer, erase the contra-transaction */
    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);
    if (contra_transaction_number > 0)
	transaction_list_remove_transaction (contra_transaction_number);

    /* check if it's a split, we needn't to erase all splits children, they will be deleted
     * with the mother, but if one of them if a transfer, we need to delete it now */
    if (gsb_data_transaction_get_split_of_transaction (transaction_number))
    {
	GSList *tmp_list;

	tmp_list = gsb_data_transaction_get_transactions_list ();
	while (tmp_list)
	{
	    gint test_transaction = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (test_transaction);

	    if ( gsb_data_transaction_get_mother_transaction_number (test_transaction) == transaction_number
		 &&
		 contra_transaction_number > 0)
		transaction_list_remove_transaction (contra_transaction_number);

	    tmp_list = tmp_list -> next;
	}
    }

    /* now we can just delete the wanted transaction */
    transaction_list_remove_transaction (transaction_number);
    return FALSE;
}



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
    gboolean mi_full = TRUE;
    gint contra_number;

    transaction_number = gsb_data_account_get_current_transaction_number (
                            gsb_gui_navigation_get_current_account ( ) );
    /* Add a sub menu to display the contra transaction */
    contra_number = gsb_data_transaction_get_contra_transaction_number ( transaction_number );

    /* full is used for the whites line, to unsensitive some fields in the menu */
    if ( transaction_number < 0 )
	full = FALSE;

    /* mi_full is used for children of transactions, to unselect some fields in the menu */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
	mi_full = FALSE;
    menu = gtk_menu_new ();

    if ( contra_number > 0 )
    {
        menu_item = gtk_image_menu_item_new_with_label ( _("Displays the contra-transaction") );
        gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_JUMP_TO,
							       GTK_ICON_SIZE_MENU ));
        g_signal_connect_swapped ( G_OBJECT(menu_item),
                        "activate",
                        G_CALLBACK ( gsb_transactions_list_display_contra_transaction ),
                        GINT_TO_POINTER ( contra_number ) );
        gtk_widget_set_sensitive ( menu_item, full );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        /* Separator */
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );
    }

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
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

    /* New transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("New transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_NEW,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK (new_transaction), NULL );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Delete transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Delete transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_DELETE,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK(remove_transaction), NULL );
    if ( !full
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELERAPPROCHEE )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* use transaction as template */
    menu_item = gtk_image_menu_item_new_with_label ( _("Use selected transaction as a template") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_COPY,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK (gsb_transactions_list_clone_template), NULL );
    gtk_widget_set_sensitive ( menu_item, full );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Clone transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Clone transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_COPY,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK (clone_selected_transaction), NULL );
    gtk_widget_set_sensitive ( menu_item, full );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

    /* Convert to scheduled transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Convert transaction to scheduled transaction") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_CONVERT,
							       GTK_ICON_SIZE_MENU ));
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK(schedule_selected_transaction), NULL );
    gtk_widget_set_sensitive ( menu_item, full && mi_full );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Move to another account */
    menu_item = gtk_image_menu_item_new_with_label ( _("Move transaction to another account") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM(menu_item),
				    gtk_image_new_from_stock ( GTK_STOCK_JUMP_TO,
							       GTK_ICON_SIZE_MENU ));
    if ( !full
	 ||
	 !mi_full
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELERAPPROCHEE )
	gtk_widget_set_sensitive ( menu_item, FALSE );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Add accounts submenu */
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(menu_item),
				GTK_WIDGET(gsb_account_create_menu_list (G_CALLBACK(move_selected_operation_to_account), FALSE, FALSE)) );


    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

    /* Change cell content. */
    menu_item = gtk_menu_item_new_with_label ( _("Change cell content") );
    if ( full )
	gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( menu_item ),
				    GTK_WIDGET ( gsb_gui_create_cell_contents_menu ( x, y ) ) );
    gtk_widget_set_sensitive ( menu_item, full );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

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
GtkWidget *gsb_gui_create_cell_contents_menu ( int x, int y )
{
    GtkWidget * menu, * item;
    gint i;

    menu = gtk_menu_new ();

    for ( i = 0 ; cell_views[i] != NULL ; i++ )
    {
        item = gtk_menu_item_new_with_label ( _(cell_views[i]) );

        g_object_set_data ( G_OBJECT (item), "x", GINT_TO_POINTER (x) );
        g_object_set_data ( G_OBJECT (item), "y", GINT_TO_POINTER (y) );
        g_signal_connect ( G_OBJECT(item),
                        "activate",
                        G_CALLBACK ( gsb_gui_change_cell_content ),
                        GINT_TO_POINTER ( i+1 ) );

        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    }
    /* set a menu to clear the cell except for the first line */
    if ( y > 0 )
    {
        item = gtk_menu_item_new_with_label ( _("Clear cell") );

        g_object_set_data ( G_OBJECT ( item ), "x", GINT_TO_POINTER ( x ) );
        g_object_set_data ( G_OBJECT ( item ), "y", GINT_TO_POINTER ( y ) );
        g_signal_connect ( G_OBJECT ( item ),
                        "activate",
			            G_CALLBACK ( gsb_gui_change_cell_content ),
                        GINT_TO_POINTER ( 0 ) );

        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    }
    return menu;
}



/**
 * Change the content of a cell.  This is triggered from the
 * activation of a menu item, so that we check the attributes of this
 * item to determine which cell is changed and with what.
 *
 * After this, iterate to update all GtkTreeIters of the transaction
 * list
 *
 * \param item		The GtkMenuItem that triggered event.
 * \param element	new element to put in the cell.
 *
 * \return FALSE
 */
gboolean gsb_gui_change_cell_content ( GtkWidget * item, gint *element_ptr )
{
    gint col, line;
    gint last_col = -1, last_line = -1;
    gint element;
    gint sort_column;
    gint current_account;

    element = GPOINTER_TO_INT ( element_ptr );
    devel_debug_int ( element );

    if ( element )
    {
        last_col = find_element_col ( element );
        last_line = find_element_line ( element );
    }

    current_account = gsb_gui_navigation_get_current_account ( );
    sort_column = gsb_data_account_get_sort_column ( current_account );

    col = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( item ), "x" ) );
    line = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( item ), "y" ) );

    /* if no change, change nothing */
    if ( last_col == col && last_line == line )
        return FALSE;

    /* save the new position */
    tab_affichage_ope[line][col] = element;

    if (last_col != -1 && last_line != -1)
    {
        /* the element was already showed, we need to erase the last cell first */
        tab_affichage_ope[last_line][last_col] = 0;
        transaction_list_update_cell (last_col, last_line);
    }

    /* now we can update the element */
    if ( element )
        transaction_list_update_element ( element );
    else
        transaction_list_update_cell (col, line);

    recuperation_noms_colonnes_et_tips ( );
    update_titres_tree_view ( );

    /* update the sort column */
    gsb_data_account_set_element_sort ( current_account, col, element );
    if ( sort_column == last_col )
    {
        gsb_data_account_set_sort_column ( current_account, col );
        transaction_list_sort_set_column ( col, 
                        gsb_data_account_get_sort_type ( current_account ) );
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

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
	if ( gsb_gui_navigation_get_current_account ( ) == -1 )
		return FALSE;

    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
    gsb_form_escape_form();
    gsb_form_show (TRUE);
    transaction_list_select ( -1 );
    gsb_transactions_list_edit_transaction (-1);

    return FALSE;
}



/**
 * Remove selected transaction if any.
 */
void remove_transaction ()
{
    if (! assert_selected_transaction()) return;

    gsb_transactions_list_delete_transaction (gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ()),
					      TRUE );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), 1 );
}



/**
 * Clone selected transaction if any.  Update user interface as well.
 *
 * \param menu_item
 * \param null
 *
 * \return FALSE
 */
gboolean clone_selected_transaction ( GtkWidget *menu_item,
                        gpointer null )
{
    gint new_transaction_number;

    if (! assert_selected_transaction())
        return FALSE;

    new_transaction_number = gsb_transactions_list_clone_transaction (
                        gsb_data_account_get_current_transaction_number (
                        gsb_gui_navigation_get_current_account ()),
                        0 );

    update_transaction_in_trees (new_transaction_number);

    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

    /* force the update module budget */
    gsb_data_account_set_bet_maj ( gsb_gui_navigation_get_current_account ( ), BET_MAJ_ALL );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * use the current selected transaction as template
 *
 * \param menu_item
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_transactions_list_clone_template ( GtkWidget *menu_item,
                        gpointer null )
{
    gint new_transaction_number;

    if ( !assert_selected_transaction ( ) )
        return FALSE;

    new_transaction_number = gsb_transactions_list_clone_transaction (
                                    gsb_data_account_get_current_transaction_number (
                                    gsb_gui_navigation_get_current_account ( ) ),
								    0 );

    update_transaction_in_trees ( new_transaction_number );

    transaction_list_select ( new_transaction_number );
    gsb_transactions_list_edit_transaction ( new_transaction_number );
    g_object_set_data ( G_OBJECT ( gsb_form_get_form_widget ( ) ),
			    "transaction_selected_in_form",
			    GINT_TO_POINTER ( -1 ) );

    /* force the update module budget */
    gsb_data_account_set_bet_maj ( gsb_gui_navigation_get_current_account ( ), BET_MAJ_ALL );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * Clone transaction.  If it is a split or a transfer, perform all
 * needed operations, like cloning associated transactions as well.
 *
 * \param transaction_number 		Initial transaction to clone
 * \param mother_transaction_number	if the transaction cloned is a child with another mother, this is the new mother
 *
 * \return the number newly created transaction.
 */
gint gsb_transactions_list_clone_transaction ( gint transaction_number,
                        gint mother_transaction_number )
{
    gint new_transaction_number;

    /* dupplicate the transaction */
    new_transaction_number = gsb_data_transaction_new_transaction (
                                    gsb_data_transaction_get_account_number ( transaction_number ) );
    gsb_data_transaction_copy_transaction ( transaction_number,
					    new_transaction_number, TRUE );

    if ( gsb_data_transaction_get_mother_transaction_number ( transaction_number )
	 &&
	 mother_transaction_number )
	gsb_data_transaction_set_mother_transaction_number ( new_transaction_number,
							     mother_transaction_number );

    /* create the contra-transaction if necessary */
    if ( gsb_data_transaction_get_contra_transaction_number ( transaction_number ) > 0 )
    {
	gsb_form_transaction_validate_transfer ( new_transaction_number,
						1,
						gsb_data_transaction_get_contra_transaction_account (
                        transaction_number ) );

	/* we need to set the contra method of payment of the transfer */
	gsb_data_transaction_set_method_of_payment_number (
                        gsb_data_transaction_get_contra_transaction_number (
                        new_transaction_number ),
					    gsb_data_transaction_get_method_of_payment_number (
                        gsb_data_transaction_get_contra_transaction_number (
                        transaction_number ) ) );
    }

    gsb_transactions_list_append_new_transaction ( new_transaction_number, TRUE );

    if ( gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
    {
        /* the transaction was a split, we look for the children to copy them */

        GSList *list_tmp_transactions;
        list_tmp_transactions = gsb_data_transaction_get_transactions_list ( );

        while ( list_tmp_transactions )
        {
            gint transaction_number_tmp;
            transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                                            list_tmp_transactions -> data );

            if ( gsb_data_transaction_get_account_number ( transaction_number_tmp )
             == gsb_data_transaction_get_account_number ( transaction_number )
             &&
             gsb_data_transaction_get_mother_transaction_number ( transaction_number_tmp )
             == transaction_number )
                gsb_transactions_list_clone_transaction (
                            transaction_number_tmp, new_transaction_number );

            list_tmp_transactions = list_tmp_transactions -> next;
        }
    }
    if ( etat.equilibrage )
        transaction_list_show_toggle_mark ( TRUE );

    return new_transaction_number;
}



/**
 * Move selected transaction to another account.  Normally called as a
 * handler.
 *
 * \param menu_item The GtkMenuItem that triggered this handler.
 *
 * \return FALSE
 */
gboolean move_selected_operation_to_account ( GtkMenuItem * menu_item,
                        gpointer null )
{
    gint target_account, source_account;

    if (! assert_selected_transaction()) return FALSE;

    source_account = gsb_gui_navigation_get_current_account ();
    target_account = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT(menu_item),
							     "account_number" ) );

    if ( gsb_transactions_list_move_transaction_to_account ( gsb_data_account_get_current_transaction_number (source_account),
							     target_account ))
    {
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	update_transaction_in_trees (gsb_data_account_get_current_transaction_number (source_account));

    gsb_data_account_colorize_current_balance ( source_account );

    mise_a_jour_accueil (FALSE);

	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }
    return FALSE;
}



/**
 * Move selected transaction to another account.  Normally called as a
 * handler.
 *
 * \param menu_item The GtkMenuItem that triggered this handler.
 */
void move_selected_operation_to_account_nb ( GtkAction *action, gint *account )
{
    gint target_account, source_account;

    if (! assert_selected_transaction()) return;

    source_account = gsb_gui_navigation_get_current_account ();
    target_account = GPOINTER_TO_INT ( account );

    if ( gsb_transactions_list_move_transaction_to_account ( gsb_data_account_get_current_transaction_number (source_account),
							     target_account ))
    {
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), 1 );

	update_transaction_in_trees ( gsb_data_account_get_current_transaction_number (
                        source_account ) ) ;

    gsb_data_account_colorize_current_balance ( source_account );

	if ( etat.modification_fichier == 0 )
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
gboolean gsb_transactions_list_move_transaction_to_account ( gint transaction_number,
                        gint target_account )
{
    gint source_account;
    gint contra_transaction_number;
    gint current_account;

    devel_debug_int ( target_account );

    source_account = gsb_data_transaction_get_account_number (transaction_number);
    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (
                        transaction_number);

    /* if it's a transfer, update the contra-transaction category line */
    if (contra_transaction_number > 0)
    {
        /* the transaction is a transfer, we check if the contra-transaction is not on 
         * the target account */
        if ( gsb_data_transaction_get_account_number (
                        contra_transaction_number) == target_account )
        {
            dialogue_error ( _("Cannot move a transfer on his contra-account"));
            return FALSE;
        }
    }

    /* we change now the account of the transaction */
    gsb_data_transaction_set_account_number ( transaction_number,
					      target_account );

    /* update the field of the contra transaction if necessary. Ce transfert ne doit pas
     * modifier la balance du compte */
    if (contra_transaction_number > 0)
        transaction_list_update_transaction (contra_transaction_number);


    /* normally we can change the account only by right click button
     * so the current transaction is selected,
     * so move the selection down */
    transaction_list_select_down (FALSE);

    /* normally we are on the source account, if ever we are not, check here
     * what we have to update */
    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == source_account
	||
	current_account == target_account)
        gsb_transactions_list_update_tree_view ( current_account, FALSE );

    /* update the first page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;

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

    scheduled_number = schedule_transaction ( gsb_data_account_get_current_transaction_number (
                                    gsb_gui_navigation_get_current_account () ) );

    mise_a_jour_liste_echeances_auto_accueil = 1;

    if ( etat.equilibrage == 0 )
    {
        gsb_gui_navigation_set_selection (GSB_SCHEDULER_PAGE, 0, NULL);
        gsb_scheduler_list_select (scheduled_number);
        gsb_scheduler_list_edit_transaction (scheduled_number);
    }
    else
        gsb_reconcile_set_last_scheduled_transaction ( scheduled_number );

    if ( etat.modification_fichier == 0 )
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
				  gsb_data_transaction_get_date (transaction_number));
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

    if ( gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0)
    {
	/* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

	gint contra_transaction_number;

	gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
							 gsb_data_transaction_get_contra_transaction_account (transaction_number));

	contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);

	gsb_data_scheduled_set_contra_method_of_payment_number ( scheduled_number,
								 gsb_data_transaction_get_method_of_payment_number (contra_transaction_number));
    }
    else
	if ( !gsb_data_scheduled_get_category_number (scheduled_number)
	     &&
	     !gsb_data_transaction_get_split_of_transaction (transaction_number))
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
    gsb_data_scheduled_set_split_of_scheduled ( scheduled_number,
						gsb_data_transaction_get_split_of_transaction (transaction_number));

    /*     par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
    /*     (c'est le cas, à 0 avec g_malloc0) */
    /*     que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
    /* 	pour la même raison */

    gsb_data_scheduled_set_frequency ( scheduled_number,
				       2);

    /*     on récupère les opés de ventil si c'était une opé ventilée */

    if ( gsb_data_scheduled_get_split_of_scheduled ( scheduled_number))
	gsb_transactions_list_splitted_to_scheduled (transaction_number, scheduled_number);
    return scheduled_number;
}



/**
 * clone the splitted children of a transaction to splitted children of the scheduled transaction
 *
 * \param transaction_number	mother of the splitted children we look for
 * \param scheduled_number	splitted scheduled transaction we want to add the children
 *
 * \return
 * */
void gsb_transactions_list_splitted_to_scheduled ( gint transaction_number,
                        gint scheduled_number )
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
	    gint split_scheduled_number;

	    split_scheduled_number = gsb_data_scheduled_new_scheduled ();

	    if ( !split_scheduled_number)
		return;

	    gsb_data_scheduled_set_account_number ( split_scheduled_number,
						    gsb_data_transaction_get_account_number (transaction_number_tmp));
	    gsb_data_scheduled_set_date ( split_scheduled_number,
					  gsb_data_transaction_get_date (transaction_number_tmp));
	    gsb_data_scheduled_set_amount ( split_scheduled_number,
					    gsb_data_transaction_get_amount (transaction_number_tmp));
	    gsb_data_scheduled_set_currency_number ( split_scheduled_number,
						     gsb_data_transaction_get_currency_number (transaction_number_tmp));
	    gsb_data_scheduled_set_party_number ( split_scheduled_number,
						  gsb_data_transaction_get_party_number (transaction_number_tmp));
	    gsb_data_scheduled_set_category_number ( split_scheduled_number,
						     gsb_data_transaction_get_category_number (transaction_number_tmp));
	    gsb_data_scheduled_set_sub_category_number ( split_scheduled_number,
							 gsb_data_transaction_get_sub_category_number (transaction_number_tmp));

	    /*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
	    /* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement, compte_virement = -1 */
	    /*     on va changer ça la prochaine version, dès que c'est pas un virement -> -1 */

	    if ( gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) > 0)
	    {
		/* 	c'est un virement, on met la relation et on recherche le type de la contre opération */

		gint contra_transaction_number;

		gsb_data_scheduled_set_account_number_transfer ( split_scheduled_number,
								 gsb_data_transaction_get_contra_transaction_account (transaction_number_tmp));

		contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp);

		gsb_data_scheduled_set_contra_method_of_payment_number ( split_scheduled_number,
									 gsb_data_transaction_get_method_of_payment_number (contra_transaction_number));
	    }
	    else
		if ( !gsb_data_scheduled_get_category_number (split_scheduled_number))
		    gsb_data_scheduled_set_account_number_transfer ( split_scheduled_number,
								     -1 );

	    gsb_data_scheduled_set_notes ( split_scheduled_number,
					   gsb_data_transaction_get_notes (transaction_number_tmp));
	    gsb_data_scheduled_set_method_of_payment_number ( split_scheduled_number,
							      gsb_data_transaction_get_method_of_payment_number (transaction_number_tmp));
	    gsb_data_scheduled_set_method_of_payment_content ( split_scheduled_number,
							       gsb_data_transaction_get_method_of_payment_content (transaction_number_tmp));
	    gsb_data_scheduled_set_financial_year_number ( split_scheduled_number,
							   gsb_data_transaction_get_financial_year_number (transaction_number_tmp));
	    gsb_data_scheduled_set_budgetary_number ( split_scheduled_number,
						      gsb_data_transaction_get_budgetary_number (transaction_number_tmp));
	    gsb_data_scheduled_set_sub_budgetary_number ( split_scheduled_number,
							  gsb_data_transaction_get_sub_budgetary_number (transaction_number_tmp));

	    gsb_data_scheduled_set_mother_scheduled_number ( split_scheduled_number,
							     scheduled_number );

	    /*     par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
	    /*     (c'est le cas, à 0 avec g_malloc0) */
	    /*     que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
	    /* 	pour la même raison */

	    gsb_data_scheduled_set_frequency ( split_scheduled_number,
					       2);
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }
}


/**
 * called when press a button on the title column
 * check the right click and show a popup to chose the element number of
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
    GtkWidget *menu = NULL;
    GtkWidget *menu_item = NULL;
    gint i;
    gint active_sort;
    gint column_number;

    column_number = GPOINTER_TO_INT (no_column);

    switch ( ev -> button )
    {
	case 3:
	    /* we press the right button, show the popup */
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
                temp = _(g_slist_nth_data ( liste_labels_titres_colonnes_liste_ope,
                              tab_affichage_ope[i][column_number] - 1 ));
            }

            if ( temp && strcmp ( temp, _("Balance") ) )
            {
                if ( menu == NULL )
                {
                    menu = gtk_menu_new ();
                    /* sort by line */
                    menu_item = gtk_menu_item_new_with_label ( _("Sort list by: ") );
                    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
                    gtk_widget_show ( menu_item );

                    menu_item = gtk_separator_menu_item_new ();
                    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
                    gtk_widget_show ( menu_item );
                }

                if ( i && GTK_IS_RADIO_MENU_ITEM ( menu_item ) )
                    menu_item = gtk_radio_menu_item_new_with_label_from_widget ( GTK_RADIO_MENU_ITEM ( menu_item ),
                                                 temp );
                else
                    menu_item = gtk_radio_menu_item_new_with_label ( NULL, temp );

                if ( tab_affichage_ope[i][column_number] == active_sort )
                    gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( menu_item ), TRUE );

                g_object_set_data ( G_OBJECT ( menu_item ),
                        "no_sort",
                        GINT_TO_POINTER (tab_affichage_ope[i][column_number]));
                g_signal_connect ( G_OBJECT(menu_item),
                        "activate",
                        G_CALLBACK ( gsb_transactions_list_change_sort_type ),
                        no_column );

                gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
                gtk_widget_show ( menu_item );
            }
	    }

        if ( menu )
        {
            gfloat alignement;

            menu_item = gtk_separator_menu_item_new ();
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            gtk_widget_show ( menu_item );

            /* alignement */
            alignement = gtk_tree_view_column_get_alignment (
                        gtk_tree_view_get_column ( GTK_TREE_VIEW ( transactions_tree_view ),
                        column_number ) );
            menu_item = gtk_menu_item_new_with_label ( _("alignment: ") );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            gtk_widget_show ( menu_item );

            menu_item = gtk_separator_menu_item_new ();
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            gtk_widget_show ( menu_item );

            menu_item = gtk_radio_menu_item_new_with_label ( NULL, _("LEFT") );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            if ( alignement == COLUMN_LEFT )
                gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( menu_item ), TRUE );
            g_object_set_data ( G_OBJECT ( menu_item ),
                        "alignement",
                        GINT_TO_POINTER ( ALIGN_LEFT ) );
            g_signal_connect ( G_OBJECT(menu_item),
                        "activate",
                        G_CALLBACK ( gsb_transactions_list_change_alignement ),
                        no_column );
            gtk_widget_show ( menu_item );

            menu_item = gtk_radio_menu_item_new_with_label_from_widget ( GTK_RADIO_MENU_ITEM (
                        menu_item ),_("CENTER") );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            if ( alignement == COLUMN_CENTER )
                gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( menu_item ), TRUE );
            g_object_set_data ( G_OBJECT ( menu_item ),
                        "alignement",
                        GINT_TO_POINTER ( ALIGN_CENTER ) );
            g_signal_connect ( G_OBJECT(menu_item),
                        "activate",
                        G_CALLBACK ( gsb_transactions_list_change_alignement ),
                        no_column );
            gtk_widget_show ( menu_item );

            menu_item = gtk_radio_menu_item_new_with_label_from_widget ( GTK_RADIO_MENU_ITEM (
                        menu_item ),_("RIGHT") );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            if ( alignement == COLUMN_RIGHT )
                gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( menu_item ), TRUE );
            g_object_set_data ( G_OBJECT ( menu_item ),
                        "alignement",
                        GINT_TO_POINTER ( ALIGN_RIGHT ) );
            g_signal_connect ( G_OBJECT(menu_item),
                        "activate",
                        G_CALLBACK ( gsb_transactions_list_change_alignement ),
                        no_column );
            gtk_widget_show ( menu_item );

            gtk_menu_popup ( GTK_MENU ( menu ), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time () );
            gtk_widget_show ( menu );
        }
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
    gint column_number;
    gint account_number;

    devel_debug_int (GPOINTER_TO_INT (no_column));

    if ( !gtk_check_menu_item_get_active ( GTK_CHECK_MENU_ITEM ( menu_item )))
	return FALSE;

    column_number = GPOINTER_TO_INT (no_column);
    account_number = gsb_gui_navigation_get_current_account ();

    /* set the new column to sort */
    gsb_data_account_set_sort_column ( account_number,
				       column_number );

    /* set the new element number used to sort this column */
    gsb_data_account_set_element_sort ( account_number,
					column_number,
					GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( menu_item),
									      "no_sort" )));

    /* we want a descending sort but gsb_transactions_list_change_sort_column will
     * invert the order, so set DESCENDING for now */
    transaction_list_sort_set_column ( column_number,
				        GTK_SORT_DESCENDING );
    gsb_transactions_list_change_sort_column (NULL, no_column);
    return FALSE;
}


/**
 * called by a click on a column title ; the sort of the list is automatic,
 * that function make the background color and the rest to be updated
 *
 * \param tree_view_column the tree_view_column clicked, not used, can be NULL
 * \param column_ptr	the number of column stocked as gint *, use GPOINTER_TO_INT
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_change_sort_column ( GtkTreeViewColumn *tree_view_column,
                        gint *column_ptr )
{
    GtkSortType sort_type;
    gint current_column;
    gint account_number;
    gint new_column;
    GSList *tmp_list;
    gint selected_transaction;
    gint element_number;

    devel_debug (NULL);

    account_number = gsb_gui_navigation_get_current_account ();
    transaction_list_sort_get_column ( &current_column, &sort_type );
    new_column = GPOINTER_TO_INT ( column_ptr );

    element_number = gsb_data_account_get_element_sort ( account_number, new_column );

    if ( element_number == ELEMENT_BALANCE )
        return FALSE;

    /* if we come here and the list was user custom sorted for reconcile,
     * we stop the reconcile sort and set what is asked by the user */
    if ( transaction_list_sort_get_reconcile_sort () )
    {
        transaction_list_sort_set_reconcile_sort ( FALSE );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( reconcile_sort_list_button ),
                                FALSE );
        /* if we asked the same last column, we invert the value,
         * to come back to the last sort_type before the sort reconciliation */
        if ( new_column == current_column )
        {
            if ( sort_type == GTK_SORT_ASCENDING )
                sort_type = GTK_SORT_DESCENDING;
            else
                sort_type = GTK_SORT_ASCENDING;
        }
    }

    /* if the new column is the same as the old one, we change
     * the sort type */
    if ( new_column == current_column )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            sort_type = GTK_SORT_DESCENDING;
        else
            sort_type = GTK_SORT_ASCENDING;
    }
    else
    {
        gint new_element;

        /* on vérifie que l'élément de tri existe sinon on met le premier élément de la colonne */
        new_element = gsb_transactions_list_get_valid_element_sort ( account_number,
                        new_column,
                        element_number );
        if ( new_element != element_number )
        {
            gsb_data_account_set_element_sort ( account_number, new_column, new_element );
            element_number = new_element;
        }
        /* we sort by another column, so sort type by default is descending */
        sort_type = GTK_SORT_ASCENDING;
    }
    /* now have to save the new column and sort type in the account
     * or in all account if global conf for all accounts */
    tmp_list = gsb_data_account_get_list_accounts ();
    while ( tmp_list )
    {
        gint tmp_account;

        tmp_account = gsb_data_account_get_no_account ( tmp_list -> data );

        if ( tmp_account == account_number
            ||
            !etat.retient_affichage_par_compte )
        {
            /* set the new column to sort */
            gsb_data_account_set_sort_column ( tmp_account, new_column );
            /* save the sort_type */
            gsb_data_account_set_sort_type ( tmp_account, sort_type );
        }
        tmp_list = tmp_list -> next;
    }

    selected_transaction = transaction_list_select_get ();

    /* now we can sort the list */
    transaction_list_sort_set_column ( new_column, sort_type );
    transaction_list_filter ( account_number );
    transaction_list_set_balances ();
    transaction_list_sort ();
    transaction_list_colorize ();
    if ( conf.show_transaction_gives_balance )
        transaction_list_set_color_jour ( account_number );
    transaction_list_select (selected_transaction);

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * switch the view between show the reconciled transactions or not
 *
 * \param show_r	TRUE or FALSE
 *
 * \return
 * */
void mise_a_jour_affichage_r ( gboolean show_r )
{
    gint current_account;

    devel_debug_int (show_r);

    current_account = gsb_gui_navigation_get_current_account ();

    /*  we check all the accounts */
    /* 	if etat.retient_affichage_par_compte is set, only gsb_gui_navigation_get_current_account () will change */
    /* 	else, all the accounts change */

    if ( show_r == gsb_data_account_get_r ( current_account ) )
    {
        gsb_transactions_list_update_tree_view ( current_account, show_r );
        gsb_menu_update_view_menu ( current_account );
        return;
    }

    gsb_data_account_set_r ( current_account, show_r );

    if ( !etat.retient_affichage_par_compte )
    {
	GSList *list_tmp;

	list_tmp = gsb_data_account_get_list_accounts ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_data_account_get_no_account ( list_tmp -> data );
	    gsb_data_account_set_r ( i, show_r );

	    list_tmp = list_tmp -> next;
	}
    }
    gsb_transactions_list_update_tree_view ( current_account, show_r );
    gsb_menu_update_view_menu ( current_account );

    return;
}


/**
 * switch the view between show the reconciled transactions or not
 *
 * \param show_r	TRUE or FALSE
 *
 * \return
 * */
void gsb_transactions_list_show_archives_lines ( gboolean show_l )
{
    gint current_account;

    devel_debug_int ( show_l );

    current_account = gsb_gui_navigation_get_current_account ( );

    /*  we check all the accounts */
    /* 	if etat.retient_affichage_par_compte is set, only gsb_gui_navigation_get_current_account () will change */
    /* 	else, all the accounts change */

    if ( show_l == gsb_data_account_get_l ( current_account ) )
    {
        gsb_transactions_list_update_tree_view ( current_account, show_l );
            return;
    }

    gsb_data_account_set_l ( current_account, show_l );

    if ( !etat.retient_affichage_par_compte )
    {
        GSList *list_tmp;

        list_tmp = gsb_data_account_get_list_accounts ( );

        while ( list_tmp )
        {
            gint i;

            i = gsb_data_account_get_no_account ( list_tmp -> data );
            gsb_data_account_set_l ( i, show_l );

            list_tmp = list_tmp -> next;
        }
    }
    gsb_transactions_list_update_tree_view ( current_account, show_l );

    return;
}


/**
 * called to change the number of visible rows
 * depends of the conf, change the number of rows of the current account,
 * and if necessary of all the accounts
 * adapts also the tree for the splits to set the expander on the new last line
 *
 * \param rows_number the new number of lines we want to see
 *
 * \return
 * */
void gsb_transactions_list_set_visible_rows_number ( gint rows_number )
{
    GSList *list_tmp;
    gint current_account;

    devel_debug_int (rows_number);

    current_account = gsb_gui_navigation_get_current_account ();
    if ( rows_number == gsb_data_account_get_nb_rows (current_account) && rows_number > 1 )
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
	    gsb_data_account_set_nb_rows ( i, rows_number );
	}
	list_tmp = list_tmp -> next;
    }

    /* we update the screen */
    gsb_transactions_list_update_tree_view ( current_account, FALSE );
}



/**
 * check if the transaction given in param should be visible or not
 * according to the account and the current line
 * this function is called to filter the model for each line in transaction_list_filter
 * 	so need to be very fast
 *
 * \param transaction_number 	the pointer in the model, usually a transaction, but can be archive
 * \param account_number	account number
 * \param line_in_transaction	the line in the transaction (0, 1, 2 or 3)
 * \param what_is_line		IS_TRANSACTION / IS_ARCHIVE
 *
 * \return TRUE if the transaction should be shown, FALSE else
 * */
gboolean gsb_transactions_list_transaction_visible ( gpointer transaction_ptr,
                        gint account_number,
                        gint line_in_transaction,
                        gint what_is_line )
{
    gint transaction_number;
    gint r_shown;
    gint nb_rows;

    r_shown = gsb_data_account_get_r ( account_number );
    nb_rows = gsb_data_account_get_nb_rows ( account_number );

    /* first check if it's an archive, if yes and good account, always show it */
    if (what_is_line == IS_ARCHIVE)
    {
        if ( gsb_data_account_get_l ( account_number ) )
	        return ( gsb_data_archive_store_get_account_number (
                        gsb_data_archive_store_get_number ( transaction_ptr ) ) == account_number );
        else
            return FALSE;
    }

    /* we don't check now for the separator, because it won't be shown if the transaction
     * is not shown, so check the basics for the transaction, and show or not after the separator */

    /*  check now for transactions */
    transaction_number = gsb_data_transaction_get_transaction_number ( transaction_ptr );

    /* check the general white line (one for all the list, so no account number) */
    if ( transaction_number == -1 )
	    return ( display_mode_check_line ( line_in_transaction, nb_rows ) );

    /* check the account */
    if ( gsb_data_transaction_get_account_number (transaction_number) != account_number )
	return FALSE;

    /* 	    check if it's R and if r is shown */
    if ( gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 &&
	 !r_shown )
	return FALSE;

    /* 	    now we check if we show 1, 2, 3 or 4 lines */
    return display_mode_check_line ( line_in_transaction, nb_rows );
}



/*
 * get the real name of the category of the transaction
 * so return split of transaction, transfer : ..., categ : under_categ
 *
 * \param transaction the adr of the transaction
 *
 * \return the real name. It returns a newly allocated string which must be
 * freed when no more used.
 * */
gchar *gsb_transactions_get_category_real_name ( gint transaction_number )
{
    gchar *tmp;
    gint contra_transaction_number;

    if ( gsb_data_transaction_get_split_of_transaction (transaction_number))
	tmp = g_strdup(_("Split of transaction"));
    else
    {
	contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);
	switch (contra_transaction_number)
	{
	    case -1:
		/* transfer to deleted account */
		if ( gsb_data_transaction_get_amount ( transaction_number).mantissa < 0 )
		    tmp = g_strdup(_("Transfer to a deleted account"));
		else
		    tmp = g_strdup(_("Transfer from a deleted account"));
		break;
	    case 0:
		/* normal category */
		tmp = gsb_data_category_get_name ( gsb_data_transaction_get_category_number (transaction_number),
						   gsb_data_transaction_get_sub_category_number (transaction_number),
						   NULL );
		break;
	    default:
		/* transfer */
		if ( gsb_data_transaction_get_amount (transaction_number).mantissa < 0 )
		    tmp = g_strdup_printf ( _("Transfer to %s"),
					    gsb_data_account_get_name ( gsb_data_transaction_get_account_number (contra_transaction_number)));
		else
		    tmp = g_strdup_printf ( _("Transfer from %s"),
					    gsb_data_account_get_name ( gsb_data_transaction_get_account_number (contra_transaction_number)));
	}
    }
    return tmp;
}



/**
 * switch the expander of the split given in param
 *
 * \param transaction_number the split we want to switch
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_switch_expander ( gint transaction_number )
{
    GtkTreePath *path;

    devel_debug_int (transaction_number);

    if ( !gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
	    return FALSE;

    path = transaction_list_select_get_path ( transaction_list_get_last_line (
                            gsb_data_account_get_nb_rows (
                            gsb_gui_navigation_get_current_account ( ) ) ) );

    if ( gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( transactions_tree_view ), path ) )
	    gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( transactions_tree_view ), path );
    else
	    gtk_tree_view_expand_row ( GTK_TREE_VIEW ( transactions_tree_view ), path, FALSE );

    gtk_tree_path_free ( path );

    return FALSE;
}

/**
 * replace in the transactions list the archive lines by all the transactions in
 * the archive
 * remove too the archive_store from the archive_store list
 *
 * \param archive_number	the archive to restore
 * \param show_warning		TRUE to show a warning if the R transactions are not shown on the account
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_restore_archive ( gint archive_number,
                        gboolean show_warning )
{
    GSList *tmp_list;
    gint account_number;
    gint transaction_number;
    gboolean exist = FALSE;

    devel_debug_int (archive_number);

    /* remove the lines of the archive in the model */
    exist = transaction_list_remove_archive (archive_number);
    /* remove the structures of archive_model */
    gsb_data_archive_store_remove_by_archive (archive_number);

    /* si l'archive existait bien on ajoute les transactions dans la liste et dans le 
     * tree_view. Evite de charger deux fois les données si on supprime l'archive
     * après avoir ajouté les lignes */
    if (exist )
    {
        orphan_child_transactions = NULL;

        /* second step, we add all the archived transactions of that archive into the
         * transactions_list and into the store */
        tmp_list = gsb_data_transaction_get_complete_transactions_list ();
        while (tmp_list)
        {
        transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

        if ( gsb_data_transaction_get_archive_number (transaction_number) == archive_number)
        {
            /* append the transaction to the list of non archived transactions */
            gsb_data_transaction_add_archived_to_list (transaction_number);

            /* the transaction belongs to the archive we want to show, so append it to the list store */
            transaction_list_append_transaction ( transaction_number);
        }
        tmp_list = tmp_list -> next;
        }

        /* if orphan_child_transactions if filled, there are some children wich didn't find their
         * mother, we try again now that all the mothers are in the model */
        if (orphan_child_transactions)
        {
        GSList *orphan_list_copy;

        orphan_list_copy = g_slist_copy (orphan_child_transactions);
        g_slist_free (orphan_child_transactions);
        orphan_child_transactions = NULL;

        tmp_list = orphan_list_copy;
        while (tmp_list)
        {
            transaction_number = GPOINTER_TO_INT (tmp_list -> data);
            transaction_list_append_transaction (transaction_number);
            tmp_list = tmp_list -> next;
        }
        g_slist_free (orphan_list_copy);

        /* if orphan_child_transactions is not null, there is still some children
         * wich didn't find their mother. show them now */
        if (orphan_child_transactions)
        {
            gchar *message = _("Some children didn't find their mother in the list, this shouldn't happen and there is probably a bug behind that. Please contact the Grisbi team.\n\nThe concerned children number are :\n");
            gchar *string_1;
            gchar *string_2;

            string_1 = g_strconcat (message, NULL);
            tmp_list = orphan_child_transactions;
            while (tmp_list)
            {
            string_2 = g_strconcat ( string_1,
                         utils_str_itoa (GPOINTER_TO_INT (tmp_list -> data)),
                         " - ",
                         NULL);
            g_free (string_1);
            string_1 = string_2;
            tmp_list = tmp_list -> next;
            }
            dialogue_warning (string_1);
            g_free (string_1);
        }
        }
    }

    /* all the transactions of the archive have been added, we just need to clean the list,
     * but don't touch to the main page and to the current balances... we didn't change anything */
    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number != -1)
        gsb_transactions_list_update_tree_view (account_number, TRUE);

    if (!gsb_data_account_get_r (account_number) && show_warning)
        dialogue ( _("You have just recovered an archive, if you don't see any new "
                        "transaction, remember that the R transactions are not showed "
                        "so the archived transactions are certainly hidden...\n\n"
                        "Show the R transactions to make them visible.") );

    return FALSE;
}



/**
 * called when the size of the tree view changed, to keep the same ration
 * between the columns
 *
 * \param tree_view	the tree view of the transactions list
 * \param allocation	the new size
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_size_allocate ( GtkWidget *tree_view,
                        GtkAllocation *allocation,
                        gpointer null )
{
    gint i;

    if ( allocation -> width == current_tree_view_width )
    {
        /* size of the tree view didn't change, but we received an allocated signal
         * it happens several times, and especially when we change the columns,
         * so we update the colums */

        /* sometimes, when the list is not visible, he will set all the columns to 1%... we block that here */
        if ( gtk_tree_view_column_get_width ( transactions_tree_view_columns[0]) == 1 )
            return FALSE;

        for ( i = 0 ; i<CUSTOM_MODEL_N_VISIBLES_COLUMN ; i++ )
            transaction_col_width[i] = ( gtk_tree_view_column_get_width (
                        transactions_tree_view_columns[i]) * 100) / allocation -> width + 1;

        return FALSE;
    }

    /* the size of the tree view changed, we keep the ration between the columns,
     * we don't set the size of the last column to avoid the calculate problems,
     * it will take the end of the width alone */
    current_tree_view_width = allocation -> width;

    for ( i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS - 1 ; i++ )
    {
        gint width;

        width = ( transaction_col_width[i] * ( allocation -> width ) )/ 100;
        if ( width > 0 )
            gtk_tree_view_column_set_fixed_width ( transactions_tree_view_columns[i], width );
    }
    return FALSE;
}


/**
 * find column number for the archive texte
 *
 * \param element_number the element we look for
 *
 * \return column number or an other element
 * */
gint find_element_col_for_archive ( void )
{
    gint retour;

    if ( (retour = find_element_col ( ELEMENT_PARTY )) >= 0 )
        return retour;
    if ( (retour = find_element_col ( ELEMENT_CATEGORY )) >= 0 )
        return retour;
    if ( (retour = find_element_col ( ELEMENT_BUDGET )) >= 0 )
        return retour;
    if ( (retour = find_element_col ( ELEMENT_NOTES )) >= 0 )
        return retour;

    return -1;
}


/**
 * display contra_transaction
 * 
 *
 * 
 */
void gsb_transactions_list_display_contra_transaction ( gint *element_ptr )
{
    gint target_account;
    gint transaction_number;

    transaction_number = GPOINTER_TO_INT ( element_ptr );
    target_account = gsb_data_transaction_get_account_number ( transaction_number );

    if ( gsb_gui_navigation_set_selection ( GSB_ACCOUNT_PAGE, target_account, NULL ) )
    {
        /* If transaction is reconciled, show reconciled transactions. */
        if ( gsb_data_transaction_get_marked_transaction ( transaction_number ) == OPERATION_RAPPROCHEE
         &&
         !gsb_data_account_get_r ( target_account ) )
        {
            mise_a_jour_affichage_r ( TRUE );
        }

        transaction_list_select ( transaction_number );
    }
}


/**
 *
 *
 *
 *
 * */
gboolean gsb_transactions_list_change_alignement ( GtkWidget *menu_item,
                        gint *no_column )
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    gint column_number;
    gint alignement;
    gfloat xalign = 0.0;

    if ( !gtk_check_menu_item_get_active ( GTK_CHECK_MENU_ITEM ( menu_item ) ) )
        return FALSE;

    column_number = GPOINTER_TO_INT ( no_column );
    column = gtk_tree_view_get_column ( GTK_TREE_VIEW (
                        transactions_tree_view ),
                        column_number );
    alignement = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( menu_item ), "alignement" ) );
    cell_renderer = g_object_get_data ( G_OBJECT ( column ), "cell_renderer" );

    switch ( alignement )
    {
        case ALIGN_LEFT:
            xalign = 0.0;
            break;
        case ALIGN_CENTER:
            xalign = 0.5;
            break;
        case ALIGN_RIGHT:
            xalign = 1.0;
            break;
    }

    transaction_col_align[column_number] = alignement;
    gtk_tree_view_column_set_alignment  ( column, xalign );
    g_object_set ( G_OBJECT ( cell_renderer ),
		                "xalign", xalign,
		                NULL );
    
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return FALSE;
}


/**
 *
 *
 *
 *
 **/
gboolean gsb_transactions_list_set_largeur_col ( void )
{
    gint i;
    gint width;

    for ( i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++ )
    {
        width = ( transaction_col_width[i] * ( current_tree_view_width ) ) / 100;
        if ( width > 0 )
            gtk_tree_view_column_set_fixed_width ( transactions_tree_view_columns[i], width );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 **/
gint gsb_transactions_list_get_valid_element_sort ( gint account_number,
                        gint column_number,
                        gint element_number )
{
    gint i;

    for ( i = 0 ; i < 4 ; i++ )
    {
        if ( tab_affichage_ope[i][column_number] == element_number )
            return element_number;
    }

    return tab_affichage_ope[0][column_number];
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
