/* ************************************************************************** */
/*     Copyright (C)	2004-2005 Alain Portal (aportal@univ-montp2.fr)	      */
/*                  	2006-2006 Benjamin Drieu (bdrieu@april.org)	          */
/*			http://www.grisbi.org   			                              */
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
#include "gsb_debug.h"
#include "dialog.h"
#include "gsb_assistant.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "gsb_status.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_debug_add_report_page ( GtkWidget * assistant, gint page, 
				 struct gsb_debug_test * test, gchar * summary );
static gchar *gsb_debug_budget_test  ( void );
static gboolean gsb_debug_budget_test_fix ();
static gchar *gsb_debug_category_test  ( void );
static gboolean gsb_debug_category_test_fix ();
static gboolean gsb_debug_enter_test_page ( GtkWidget * assistant );
static gchar *gsb_debug_payee_test  ( void );
static gboolean gsb_debug_payee_test_fix ();
static gchar * gsb_debug_reconcile_test ( void );
static gchar * gsb_debug_transfer_test ( void );
static gboolean gsb_debug_try_fix ( gboolean (* fix) () );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/



/** Tests  */
struct gsb_debug_test debug_tests [8] = {
    /* Check for reconciliation inconcistency.  */
    { N_("Incorrect reconcile totals"),
      N_("This test will look for accounts where reconcile totals do not match reconciled transactions."),
      N_("Grisbi found accounts where reconciliation totals are inconsistent with the "
        "sum of reconcilied transactions and initial balance.\n\n"
        "The cause may be the elimination or modification of reconciliations or changes "
        "in the balance of reconciliations in the preferences."),
      gsb_debug_reconcile_test, NULL },

    { N_("Duplicate sub-categories check"),
      N_("free"),
      N_("Due to a bug in previous versions of Grisbi, "
	 "sub-categories may share the same numeric identifier in some "
	 "cases, resulting in transactions having two sub-categories.  "
	 "If you choose to continue, Grisbi will "
	 "remove one of each duplicates and "
	 "recreate it with a new identifier.\n\n"
	 "No transaction will be lost, but in some cases, you "
	 "will have to manually move transactions to this new "
	 "sub-category."),
      gsb_debug_duplicate_categ_check, gsb_debug_duplicate_categ_fix },

    { N_("Duplicate sub-budgetary lines check"),
      N_("free"),
      N_("Due to a bug in previous versions of Grisbi, "
	 "sub-budgetary lines may share the same numeric id in some "
	 "cases, resulting in transactions having two sub-budgetary lines.  "
	 "If you choose to continue, Grisbi will "
	 "remove one of each duplicates and "
	 "recreate it with a new id.entifier\n\n"
	 "No transactions will be lost, but in some cases, you "
	 "will have to manually move transactions to this new "
	 "sub-budgetary line."),
      gsb_debug_duplicate_budget_check, gsb_debug_duplicate_budget_fix },

    { N_("Orphan countra-transactions check"),
      N_("free"),
      N_("In some rare cases, transfers are incorrectly linked to contra-transactions.  "
	 "This might be because of bugs or because of imports that failed.\n"
	 "To fix this, you will have to manually edit your .gsb file "
	 "(with a text editor) and fix transactions using their numeric ID."),
      gsb_debug_transfer_test, NULL },

    { N_("Incorrect category/sub-category number"),
      N_("This test will look for transactions wich have non existant categories/sub-categories."),
      N_("Grisbi found some transactions with non existants categories/sub-categories "
	 "If you choose to continue, Grisbi will remove that category error "
	 "and that transactions will have no categories." ),
      gsb_debug_category_test, gsb_debug_category_test_fix },

    { N_("Incorrect budget/sub-budget number"),
      N_("This test will look for transactions wich have non existant budgets/sub-budgets."),
      N_("Grisbi found some transactions with non existants budgets/sub-budgets "
	 "If you choose to continue, Grisbi will remove that budget error "
	 "and that transactions will have no budgets." ),
      gsb_debug_budget_test, gsb_debug_budget_test_fix },

    { N_("Incorrect payee number"),
      N_("This test will look for transactions wich have non existant payees."),
      N_("Grisbi found some transactions with non existants payees "
	 "If you choose to continue, Grisbi will "
	 "remove them and that transactions will have no payee." ),
      gsb_debug_payee_test, gsb_debug_payee_test_fix },


    { NULL, NULL, NULL, NULL, NULL },
};



/**
 * Performs various checks on Grisbi files.
 *
 * @return TRUE
 */
gboolean gsb_debug ( void )
{
    GtkWidget * assistant, * text_view;
    GtkTextBuffer * text_buffer;
    GtkWidget *scrolled_window;

    gsb_status_message ( _("Checking file for possible corruption...") );

    assistant = gsb_assistant_new ( _("Grisbi accounts debug"),
				    _("This assistant will help you to search your account "
				      "file for inconsistencies, which can be caused either "
				      "by bugs or by erroneous manipulation."),
				    "bug.png",
				    NULL );

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC );

    text_view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD );
    gtk_text_view_set_editable ( GTK_TEXT_VIEW(text_view), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW(text_view), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW(text_view), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW(text_view), 12 );
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			text_view );

    text_buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( text_view ) );
    g_object_set_data ( G_OBJECT ( assistant ), "text-buffer", text_buffer );
    gtk_text_buffer_create_tag ( text_buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);  
    gtk_text_buffer_create_tag ( text_buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag ( text_buffer, "indented", "left-margin", 24, NULL);

    gsb_assistant_add_page ( assistant, scrolled_window, 1, 0, -1, 
			     G_CALLBACK ( gsb_debug_enter_test_page ) );
    
    gsb_assistant_run ( assistant );
    gtk_widget_destroy ( assistant );
    
    return FALSE;
}



/**
 *
 *
 *
 */
gboolean gsb_debug_enter_test_page ( GtkWidget * assistant )
{
    GtkTextBuffer * text_buffer = NULL;
    GtkTextIter text_iter;
    gboolean inconsistency = FALSE;
    gint i, page = 2;
	gchar* tmpstr;

    text_buffer = g_object_get_data ( G_OBJECT(assistant), "text-buffer" );

    while ( gtk_notebook_get_n_pages ( g_object_get_data ( G_OBJECT (assistant), 
							   "notebook" ) ) > 2 )
    {
	gtk_notebook_remove_page ( g_object_get_data ( G_OBJECT (assistant), "notebook" ), 
				   -1 );
    }

    gtk_text_buffer_set_text ( text_buffer, "\n", -1);
    gtk_text_buffer_get_iter_at_offset ( text_buffer, &text_iter, 1 );

    for ( i = 0 ; debug_tests [i] . name != NULL ; i ++ )
    {
	gchar * result = debug_tests [ i ] . test ();

	if ( result )
	{
	    if ( ! inconsistency )
	    {
		/* No inconsistency found yet so put title. */
		gtk_text_buffer_insert_with_tags_by_name ( text_buffer, &text_iter,
							   _("Inconsistencies found\n\n"), 
							   -1, "x-large", NULL );
		gtk_text_buffer_insert ( text_buffer, &text_iter,
					 _("The following debug tests found inconsistencies "
					   "in this accounts file:\n\n"),
					 -1 );
	    }

	    tmpstr = g_strconcat ( "• ", _( debug_tests[i] . name ), "\n", NULL );
	    gtk_text_buffer_insert_with_tags_by_name ( text_buffer, &text_iter,
						       tmpstr,
						       -1, "indented", NULL );
            g_free ( tmpstr );

	    inconsistency = TRUE;
	    gsb_debug_add_report_page ( assistant, page, &(debug_tests[i]), result );
	    page ++;
	}
    }
    
    gsb_status_message ( _("Done") );

    if ( !inconsistency )
    {
	gtk_text_buffer_insert_with_tags_by_name ( text_buffer, &text_iter,
						   _("No inconsistency found\n\n"), 
						   -1, "x-large", NULL );
	gtk_text_buffer_insert ( text_buffer, &text_iter,
				 _("Congratulations, your account file is in good shape!\n"),
				 -1 );
    }
    
    return TRUE;
}



/**
 *
 *
 */
void gsb_debug_add_report_page ( GtkWidget * assistant, gint page, 
				 struct gsb_debug_test * test, gchar * summary )
{
    GtkWidget * vbox, * label, * button;
    GtkWidget *scrolled_window;
    gchar *tmp_str;

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window),
					    vbox );

    label = gtk_label_new ( NULL );
    tmp_str = g_strconcat ( make_pango_attribut (
                        "size=\"larger\" weight=\"bold\"",_( test -> name ) ),
                        "\n\n", summary, NULL );
    gtk_label_set_markup ( GTK_LABEL(label), tmp_str );
    gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    g_free ( tmp_str );

    gtk_box_pack_start ( GTK_BOX(vbox), label, FALSE, FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    if ( test -> instructions )
    {
	GtkWidget * expander, * label;
	gchar* tmpstr = g_strconcat ( "<b>",_("Details"), "</b>", NULL );
	expander = gtk_expander_new ( tmpstr );
	g_free ( tmpstr );
	gtk_expander_set_use_markup ( GTK_EXPANDER(expander), TRUE );
	label = gtk_label_new ( NULL );
	gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );
	gtk_label_set_markup ( GTK_LABEL(label), _( test -> instructions ) );
	gtk_misc_set_padding ( GTK_MISC(label), 12, 6 );
	gtk_container_add ( GTK_CONTAINER(expander), label );
	gtk_box_pack_start ( GTK_BOX(vbox), expander, FALSE, FALSE, 6 );
    }

    if ( test -> fix )
    {
	button = gtk_button_new_with_label ( _("Try to fix this inconsistency.") );
	gtk_box_pack_start ( GTK_BOX(vbox), button, FALSE, FALSE, 0 );
	g_signal_connect_swapped ( G_OBJECT(button), "clicked", 
				   G_CALLBACK ( gsb_debug_try_fix ), 
				   (gpointer) test -> fix );
    }

    gtk_widget_show_all ( scrolled_window );

    gsb_assistant_add_page ( assistant, scrolled_window, page, page - 1, -1, NULL );
    gsb_assistant_set_next ( assistant, page - 1, page );
    gsb_assistant_change_button_next ( assistant, GTK_STOCK_GO_FORWARD, GTK_RESPONSE_YES );
}



/**
 *
 *
 *
 */
gboolean gsb_debug_try_fix ( gboolean (* fix) () )
{

    if ( fix () )
    {
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
	dialogue_hint ( _("Grisbi successfully repaired this account file.  "
			  "You may now save your modifications."),
			_("Fix completed"));
    }
    else
    {
	dialogue_error_hint ( _("Grisbi was unable to repair this account file.  "
				"No modification has been done."),
			      _("Unable to fix account"));
    }

    return FALSE;
}



/******************************************************************************/
/* reconciliation_check.                                                      */
/* Cette fonction est appelée après la création de toutes les listes.         */
/* Elle permet de vérifier la cohérence des rapprochements suite à la         */
/* découverte des bogues #466 et #488.                                        */
/* return a newly allocated string or NULL.                                   */
/******************************************************************************/
gchar * gsb_debug_reconcile_test ( void )
{
    gint affected_accounts = 0;
    gint tested_account = 0;
    GSList *pUserAccountsList = NULL;
    gchar *pText = g_strdup("");
	gchar* tmprealstr1;
	gchar* tmprealstr2;
	gchar* tmpstr1;
	gchar* tmpstr2;

    /* S'il n'y a pas de compte, on quitte */
    if ( ! gsb_data_account_get_accounts_amount ( ) )
    {
	g_free ( pText );
	return NULL;
    }

    /* On fera la vérification des comptes dans l'ordre préféré
       de l'utilisateur. On fait une copie de la liste. */
    pUserAccountsList = g_slist_copy ( gsb_data_account_get_list_accounts ( ) );

    /* Pour chacun des comptes, faire */
    do
    {
        gpointer p_account = pUserAccountsList -> data;
        gint account_nb = gsb_data_account_get_no_account ( p_account );
        gint reconcile_number;

        /* Si le compte a été rapproché au moins une fois.
        * Seule la date permet de l'affirmer. */
        reconcile_number = gsb_data_reconcile_get_account_last_number (account_nb);
        if (reconcile_number)
        {
        GSList *pTransactionList;
        gsb_real reconcilied_amount = null_real;

        /* On va recalculer le montant rapproché du compte (c-à-d le solde initial
	     * plus le montant des opérations rapprochées) et le comparer à la valeur
	     * stockée dans le fichier. Si les valeurs diffèrent, on affiche une boite
	     * d'avertissement */
      
        reconcilied_amount = gsb_data_account_get_init_balance ( account_nb, -1 );

        /* On récupère la liste des opérations */
        pTransactionList = gsb_data_transaction_get_complete_transactions_list ();;

        while ( pTransactionList )
        {
            gint transaction = gsb_data_transaction_get_transaction_number (
                        pTransactionList -> data );

            /* On ne prend en compte que les opérations rapprochées.
             * On ne prend pas en compte les sous-opérations ventilées. 
             * modification aportée pour tenir compte de la transformation ultérieure
             d'une opération simple en opération ventilée et pour avoir une correspondance
             * entre le relevé et l'edition de déboggage */
            if ( gsb_data_transaction_get_account_number (transaction) == account_nb
            &&
            (gsb_data_transaction_get_marked_transaction ( transaction ) == OPERATION_RAPPROCHEE )
            &&
            ! gsb_data_transaction_get_mother_transaction_number ( transaction ) )
            {
            reconcilied_amount = gsb_real_add ( reconcilied_amount,
						      gsb_data_transaction_get_adjusted_amount_for_currency ( transaction, 
													      gsb_data_account_get_currency (account_nb),
													      -1 ));
            }
            pTransactionList = pTransactionList -> next;
        }

        if ( gsb_real_abs ( gsb_real_sub ( reconcilied_amount,
					     gsb_data_reconcile_get_final_balance (reconcile_number))).mantissa > 0 )
        {
            affected_accounts ++;

            tmprealstr1 = gsb_real_get_string_with_currency (
	                                gsb_data_reconcile_get_final_balance (reconcile_number),
	                                gsb_data_account_get_currency ( account_nb ), TRUE  );
            tmprealstr2 = gsb_real_get_string_with_currency (reconcilied_amount,
					gsb_data_account_get_currency ( account_nb ), TRUE  );
            tmpstr1 = g_strdup_printf ( _("<span weight=\"bold\">%s</span>\n"
					"  Last reconciliation amount : %s\n"
					"  Computed reconciliation amount : %s\n"),
					gsb_data_account_get_name ( account_nb ), 
					tmprealstr1,
					tmprealstr2 );
            tmpstr2 = pText;
            pText = g_strconcat ( tmpstr2, tmpstr1, NULL );
            g_free ( tmpstr2 );
            g_free ( tmpstr1 );
            g_free ( tmprealstr1 );
            g_free ( tmprealstr2 );
        }
        tested_account++;
        }
    }
    while ( (  pUserAccountsList = pUserAccountsList -> next ) );

    g_slist_free ( pUserAccountsList );

    if ( affected_accounts )
    {
	pText [ strlen(pText) - 1 ] = '\0';
	return pText;
    }

    return NULL;
}



/******************************************************************************/
/* contra_transaction_check.                                                  */
/* Cette fonction est appelée après la création de toutes les listes.         */
/* Elle permet de vérifier la cohérence des virements entre comptes           */
/* suite à la découverte du bogue #542                                        */
/* return a newly allocated string or NULL                                    */
/******************************************************************************/
gchar * gsb_debug_transfer_test ( void )
{
    gboolean corrupted_file = FALSE;
    GSList * pUserAccountsList;
    gchar * pText = g_strdup("");
	gchar* tmpstr;
	gchar* oldstr;

    pUserAccountsList = gsb_data_account_get_list_accounts ();

    do
    {
    gboolean corrupted_account = FALSE;
    GSList *pTransactionList;
    gpointer p_account = pUserAccountsList -> data;
    gint account_nb = gsb_data_account_get_no_account ( p_account );

    pTransactionList = gsb_data_transaction_get_transactions_list ();

    while ( pTransactionList )
    {
	gint transaction, transfer_transaction;

	transaction = gsb_data_transaction_get_transaction_number ( pTransactionList -> data );
	transfer_transaction = gsb_data_transaction_get_contra_transaction_number ( transaction );

	/* Si l'opération est un virement vers un compte non supprimé */
	if ( gsb_data_transaction_get_account_number ( transaction ) == account_nb &&
	     transfer_transaction > 0 )
	{
	    if ( gsb_data_transaction_get_account_number ( transfer_transaction ) !=
		 gsb_data_transaction_get_contra_transaction_account ( transaction ) )
	    {
		/* S'il n'y avait pas eu encore d'erreur dans ce compte,
		   on affiche son nom */
		if ( !corrupted_account ) {
		    gchar* tmpstr = g_strdup_printf ( "\n<span weight=\"bold\">%s</span>\n",
							    gsb_data_account_get_name ( account_nb ) );
		    gchar* oldstr = pText;
		    pText = g_strconcat ( pText, tmpstr, NULL );
		    g_free ( oldstr );
		    g_free ( tmpstr );
		}
		tmpstr = g_strdup_printf ( _("Transaction #%d is linked to non existent transaction #%d.\n"),
							transaction, transfer_transaction );
		oldstr = pText;
		pText = g_strconcat ( pText , tmpstr, NULL );
		g_free ( oldstr );
		g_free ( tmpstr );
		corrupted_file = corrupted_account = TRUE;
	    }
	    else
	    {
		if ( gsb_data_transaction_get_contra_transaction_number ( transfer_transaction ) != transaction )
		{
		    /* S'il n'y avait pas eu encore d'erreur dans ce compte,
		       on affiche son nom */
		    if ( !corrupted_account ) {
			gchar* oldstr = pText;
			gchar* tmpstr = g_strdup_printf ( "\n<span weight=\"bold\">%s</span>\n",
								gsb_data_account_get_name ( account_nb ) );
			pText = g_strconcat ( pText, tmpstr , NULL );
		        g_free ( oldstr );
		        g_free ( tmpstr );
		    }
		    oldstr = pText;
		    tmpstr = g_strdup_printf ( _("Transaction #%d is linked to transaction #%d, "
							      "which is linked to transaction #%d.\n"),
							    transaction,
							    transfer_transaction,
							    gsb_data_transaction_get_contra_transaction_number ( transfer_transaction ) );
		    pText = g_strconcat ( pText , tmpstr , NULL );
		    g_free ( oldstr );
		    g_free ( tmpstr );
		    corrupted_file = corrupted_account = TRUE;
		}
	    }
	}

	pTransactionList = pTransactionList -> next;
    }

    pUserAccountsList = pUserAccountsList -> next;

    }
    while ( pUserAccountsList );

    if ( corrupted_file )
    {
	/* Skip both last and first carriage return. */
	pText [ strlen(pText) - 1 ] = '\0';
	printf ( "%s\n", pText);
	return pText + 1;
    }

    return NULL;
}


/**
 * check if all the categories into the transactions exist
 *
 * \param
 *
 * \return a gchar containing the transactions with problem or NULL
 * */
gchar *gsb_debug_category_test  ( void )
{
    GSList *tmp_list;
    gchar *returned_text = g_strdup (""); 	/* !!! don't set here my_strdup else returned_text becomes NULL */
    gchar *tmpstr;
    gchar *tmpstr1;
    gboolean invalid = FALSE;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	gint category_number = gsb_data_transaction_get_category_number (transaction_number);

	if ( gsb_data_category_get_structure (category_number))
	{
	    /* category found, check sub-category */
	    gint sub_category_number = gsb_data_transaction_get_sub_category_number (transaction_number);
	    if (sub_category_number &&
		!gsb_data_category_get_sub_category_structure (category_number, sub_category_number))
	    {
		/* sub-category not found */
		tmpstr = g_strdup_printf ( _("Transaction %d has category %d but invalid sub-category %d.\n"),
					   transaction_number, category_number, sub_category_number );
		tmpstr1 = g_strconcat ( returned_text,
					tmpstr,
					NULL );
		g_free (returned_text);
		g_free (tmpstr);
		returned_text = tmpstr1;
		invalid = TRUE;
	    }
	}
	else
	{
	    /* category not found */
	    tmpstr = g_strdup_printf ( _("Transaction %d has invalid category %d.\n"),
				       transaction_number, category_number );
	    tmpstr1 = g_strconcat ( returned_text,
				    tmpstr,
				    NULL );
	    g_free (returned_text);
	    g_free (tmpstr);
	    returned_text = tmpstr1;
	    invalid = TRUE;
	}
	tmp_list = tmp_list -> next;
    }

    if (invalid)
	return returned_text;
    else
    {
	g_free (returned_text);
	return NULL;
    }
}

/**
 * fix the transactions with non-existant categories,
 * just remove the categories
 *
 * \param
 *
 * \return TRUE if ok
 * */
gboolean gsb_debug_category_test_fix ()
{
    GSList *tmp_list;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	gint category_number = gsb_data_transaction_get_category_number (transaction_number);

	if ( gsb_data_category_get_structure (category_number))
	{
	    /* category found, check sub-category */
	    gint sub_category_number = gsb_data_transaction_get_sub_category_number (transaction_number);
	    if (sub_category_number &&
		!gsb_data_category_get_sub_category_structure (category_number, sub_category_number))
		/* sub-category not found */
		gsb_data_transaction_set_sub_category_number (transaction_number, 0);
	}
	else
	{
	    /* category not found */
	    gsb_data_transaction_set_category_number (transaction_number, 0);
	    gsb_data_transaction_set_sub_category_number (transaction_number, 0);
	}
	tmp_list = tmp_list -> next;
    }

    return TRUE;
}

/**
 * check if all the budgets into the transactions exist
 *
 * \param
 *
 * \return a gchar containing the transactions with problem or NULL
 * */
gchar *gsb_debug_budget_test  ( void )
{
    GSList *tmp_list;
    gchar *returned_text = g_strdup (""); 	/* !!! don't set here my_strdup else returned_text becomes NULL */
    gchar *tmpstr;
    gchar *tmpstr1;
    gboolean invalid = FALSE;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while ( tmp_list )
    {
        gint transaction_number;
        gint budget_number;

        transaction_number = gsb_data_transaction_get_transaction_number ( tmp_list -> data );
        budget_number = gsb_data_transaction_get_budgetary_number ( transaction_number );

        if ( gsb_data_budget_get_structure ( budget_number ) )
        {
            gint sub_budget_number;

            sub_budget_number = gsb_data_transaction_get_sub_budgetary_number (transaction_number);
            /* budget found, check sub-budget */
            if ( sub_budget_number
             &&
             !gsb_data_budget_get_sub_budget_structure ( budget_number, sub_budget_number ) )
            {
            /* sub-budget not found */
                tmpstr = g_strdup_printf ( _("Transaction %d has budget %d but invalid sub-budget %d.\n"),
                               transaction_number, budget_number, sub_budget_number );
                tmpstr1 = g_strconcat ( returned_text, tmpstr, NULL );
                g_free (returned_text);
                g_free (tmpstr);
                returned_text = tmpstr1;
                invalid = TRUE;
            }
        }
        else
        {
            /* budget not found */
            tmpstr = g_strdup_printf ( _("Transaction %d has invalid budget %d.\n"),
                           transaction_number, budget_number );
            tmpstr1 = g_strconcat ( returned_text,
                        tmpstr,
                        NULL );
            g_free (returned_text);
            g_free (tmpstr);
            returned_text = tmpstr1;
            invalid = TRUE;
        }
        tmp_list = tmp_list -> next;
    }

    if (invalid)
        return returned_text;
    else
    {
        g_free (returned_text);
        return NULL;
    }

    return NULL;
}

/**
 * fix the transactions with non-existant budgets,
 * just remove the categories
 *
 * \param
 *
 * \return TRUE if ok
 * */
gboolean gsb_debug_budget_test_fix ()
{
    GSList *tmp_list;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
        gint transaction_number;
        gint budget_number;

        transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
        budget_number = gsb_data_transaction_get_budgetary_number (transaction_number);

        if ( gsb_data_budget_get_structure ( budget_number ) )
        {
            gint sub_budget_number;

            /* budget found, check sub-budget */
            sub_budget_number = gsb_data_transaction_get_sub_budgetary_number ( transaction_number );
            if (sub_budget_number &&
            !gsb_data_budget_get_sub_budget_structure ( budget_number, sub_budget_number ) )
            /* sub-budget not found */
                gsb_data_transaction_set_sub_budgetary_number ( transaction_number, 0 );
        }
        else
        {
            /* budget not found */
            gsb_data_transaction_set_sub_budgetary_number (transaction_number, 0);
            gsb_data_transaction_set_budgetary_number (transaction_number, 0);
        }
        tmp_list = tmp_list -> next;
    }
    return TRUE;
}

/**
 * check if all the payees into the transactions exist
 *
 * \param
 *
 * \return a gchar containing the transactions with problem or NULL
 * */
gchar *gsb_debug_payee_test  ( void )
{
    GSList *tmp_list;
    gchar *returned_text = g_strdup (""); 	/* !!! don't set here my_strdup else returned_text becomes NULL */
    gchar *tmpstr;
    gchar *tmpstr1;
    gboolean invalid = FALSE;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	gint payee_number = gsb_data_transaction_get_party_number (transaction_number);

	if ( !gsb_data_payee_get_structure (payee_number))
	{
	    /* payee not found */
	    tmpstr = g_strdup_printf ( _("Transaction %d has invalid payee %d.\n"),
				       transaction_number, payee_number );
	    tmpstr1 = g_strconcat ( returned_text,
				    tmpstr,
				    NULL );
	    g_free (returned_text);
	    g_free (tmpstr);
	    returned_text = tmpstr1;
	    invalid = TRUE;
	}
	tmp_list = tmp_list -> next;
    }

    if (invalid)
	return returned_text;
    else
    {
	g_free (returned_text);
	return NULL;
    }
    return NULL;
}

/**
 * fix the transactions with non-existant payees,
 * just remove the categories
 *
 * \param
 *
 * \return TRUE if ok
 * */
gboolean gsb_debug_payee_test_fix ()
{
    GSList *tmp_list;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	gint payee_number = gsb_data_transaction_get_party_number (transaction_number);

	if ( !gsb_data_payee_get_structure (payee_number))
	    gsb_data_transaction_set_party_number (transaction_number, 0);

	tmp_list = tmp_list -> next;
    }
    return TRUE;
}


/* /\******************************************************************************\/ */
/* /\* financial_years_check.                                                     *\/ */
/* /\* Cette fonction est appelée après la création de toutes les listes          *\/ */
/* /\* Elle permet de vérifier la cohérence des exercices des opérations          *\/ */
/* /\* de ventilation avec l'opération mère (Bogue #546).                         *\/ */
/* /\******************************************************************************\/ */
/* gboolean financial_years_check ( void ) */
/* { */
/*   gint affected_accounts = 0; */
/*   gboolean corrupted_file = FALSE; */
/*   GSList *pUserAccountsList = NULL; */
/*   gchar *pHint = NULL, *pText = ""; */

/*   /\* S'il n'y a pas de compte, on quitte *\/ */
/*   if ( !nb_comptes ) */
/*     return FALSE; */

/*   /\* Si on n'utilise pas les exercices, on quitte *\/ */
/*   if ( !etat.utilise_exercice ) */
/*     return FALSE; */
    
/*   /\* On fera la vérification des comptes dans l'ordre préféré */
/*      de l'utilisateur. On fait une copie de la liste. *\/ */
/*   pUserAccountsList = g_slist_copy ( ordre_comptes ); */
  
/*   /\* Pour chacun des comptes, faire *\/ */
/*   do */
/*   { */
/*     gboolean corrupted_account = FALSE; */
/*     GSList *pTransactionList; */
/*     gchar *account_name = NULL; */

/*     p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( pUserAccountsList -> data ); */
      
/*     /\* On affiche le nom du compte testé. Si le compte n'est pas affecté, */
/*        on libèrera la mémoire *\/ */
/*     account_name = g_strdup_printf ("%s", NOM_DU_COMPTE); */

/*     /\* On récupère la liste des opérations *\/ */
/*     pTransactionList = LISTE_OPERATIONS; */

/*     while ( pTransactionList ) */
/*     { */
/*       struct structure_operation *pSplitTransaction; */

/*       pSplitTransaction = pTransactionList -> data; */

/*       /\* si c'est une ventilation d'opération et que cette ventilation a un exercice, */
/*          on va voir si l'opération mère possède le même exercice *\/ */
/*       if ( pSplitTransaction -> no_operation_ventilee_associee && */
/*            pSplitTransaction -> no_exercice ) */
/*       { */
/* 	struct structure_operation *pTransaction; */

/* 	pTransaction = g_slist_find_custom ( LISTE_OPERATIONS, */
/* 					     GINT_TO_POINTER ( pSplitTransaction -> no_operation_ventilee_associee ), */
/* 					     (GCompareFunc) recherche_operation_par_no ) -> data; */
/* 	if (!pTransaction) */
/* 	{ */
/* 	  /\* S'il n'y avait pas eu encore d'erreur dans ce compte, */
/* 	     on affiche son nom *\/ */
/* 	  if ( !corrupted_account ) { */
/* 	    pText = g_strconcat ( pText, */
/* 				  g_strdup_printf ( "\n<span weight=\"bold\">%s</span>\n", */
/* 						    account_name),  */
/* 				  NULL ); */
/* 	  } */
/* 	  pText = g_strconcat ( pText, */
/* 				g_strdup_printf ( _("Split line #%d is orpheanous.\n"), */
/* 						  pSplitTransaction -> no_operation), */
/* 				NULL ); */
/* 	  corrupted_account = TRUE; */
/* 	} */
/* 	else */
/* 	{ */
/* 	  if( pTransaction -> no_exercice != pSplitTransaction -> no_exercice ) */
/* 	  { */
/* 	    /\* S'il n'y avait pas eu encore d'erreur dans ce compte, */
/* 	       on affiche son nom *\/ */
/* 	    if ( !corrupted_account ) { */
/* 	      pText = g_strconcat ( pText, */
/* 				    g_strdup_printf ( "\n<span weight=\"bold\">%s</span>\n", */
/* 						    account_name),  */
/* 				    NULL ); */
/* 	    } */
/* 	    pText = g_strconcat ( pText, */
/* 				  g_strdup_printf ( _("Transaction #%d has a financial year named %s and " */
/* 						      "split line #%d of this transaction has a " */
/* 						      "financial year named %s\n"), */
/* 						    pTransaction -> no_operation, */
/* 						    gsb_data_fyear_get_name ( pTransaction -> no_exercice ), */
/* 						    pSplitTransaction -> no_operation, */
/* 						    gsb_data_fyear_get_name ( pSplitTransaction -> no_exercice ) ), */
/* 				  NULL ); */
/* 	    corrupted_account = TRUE; */
/* 	  } */
/* 	} */
      
/*       } */
/*       pTransactionList = pTransactionList -> next; */
/*     } */
/*     if ( corrupted_account ) { */
/*       corrupted_file = TRUE; */
/*       affected_accounts++; */
/*     } */
/*     g_free ( account_name ); */
/*   } */
/*   while ( ( pUserAccountsList = pUserAccountsList -> next ) ); */

/*   if ( affected_accounts ) */
/*   { */
/*     pText = g_strconcat ( _("Grisbi found split lines that have financial years different " */
/* 			    "from the financial years of the related transaction.  Perhaps it isn't " */
/* 			    "a problem, but perhaps it is.\n" */
/* 			    "The following accounts seems inconsistent:\n"),  */
/* 			  pText, NULL ); */

/*     if ( affected_accounts > 1 ) */
/*     { */
/*       pHint = g_strdup_printf ( _("%d accounts have inconsistencies."),  */
/* 				affected_accounts ); */
/*     } */
/*     else */
/*     { */
/*       pHint = _("An account has inconsistencies."); */
/*     } */

/*     dialogue_warning_hint ( pText, pHint ); */

/*     g_free ( pText ); */
/*     g_free ( pHint ); */
/*   } */
/*   g_slist_free ( pUserAccountsList ); */

/*   return corrupted_file; */
/* } */



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
