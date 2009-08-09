/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
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

/**
 * \file data_partial_balance.c
 */

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_partial_balance.h"
#include "./dialog.h"
#include "./gsb_data_account.h"
#include "./gsb_data_currency.h"
#include "./navigation.h"
#include "./gsb_real.h"
#include "./accueil.h"
#include "./utils_str.h"
#include "./utils.h"
#include "./structures.h"
#include "./fenetre_principale.h"
#include "./gsb_data_account.h"
#include "./include.h"
#include "./erreur.h"
#include "./gsb_real.h"
/*END_INCLUDE*/

/** \struct
 * contenant les éléments d'un solde partiel
 * */
typedef struct
{
    gint partial_balance_number;
    gchar *balance_name;
    gchar *liste_cptes;
    kind_account kind;
    gint currency;
} struct_partial_balance;


/*START_STATIC*/
static  void _gsb_data_partial_balance_free ( struct_partial_balance *partial_balance);
static gint gsb_data_partial_balance_cmp_func ( struct_partial_balance *partial_balance_1,
                        struct_partial_balance *partial_balance_2 );
static gpointer gsb_data_partial_balance_get_structure ( gint partial_balance_number );
static void gsb_partial_balance_renumerote ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *main_vbox;
extern gint mise_a_jour_liste_comptes_accueil;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/

/** contains the g_slist of struct_partial_balance */
GSList *partial_balance_list = NULL;

/** a pointer to the last partial_balance used (to increase the speed) */
static struct_partial_balance *partial_balance_buffer;


/**
 * set the partial_balance global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_partial_balance_init_variables ( void )
{
    if ( partial_balance_list )
    {
        GSList* tmp_list = partial_balance_list;
        while ( tmp_list )
        {
            struct_partial_balance *partial_balance;
            partial_balance = tmp_list -> data;
            tmp_list = tmp_list -> next;
            _gsb_data_partial_balance_free ( partial_balance ); 
        }
        g_slist_free ( partial_balance_list );
    }
    partial_balance_list = NULL;
    partial_balance_buffer = NULL;

    return FALSE;
}


/**
 * find and return the structure of the partial_balance asked
 *
 * \param partial_balance_number number of partial_balance
 *
 * \return the adr of the struct of the partial_balance (NULL if doesn't exit)
 * */
gpointer gsb_data_partial_balance_get_structure ( gint partial_balance_number )
{
    GSList *tmp;

    if ( !partial_balance_number )
        return NULL;

    /* before checking all the import rule, we check the buffer */
    if ( partial_balance_buffer
     &&
     partial_balance_buffer -> partial_balance_number == partial_balance_number )
        return partial_balance_buffer;

    tmp = partial_balance_list;

    while ( tmp )
    {
        struct_partial_balance *partial_balance;

        partial_balance = tmp -> data;

        if ( partial_balance -> partial_balance_number == partial_balance_number )
        {
            partial_balance_buffer = partial_balance;
            return partial_balance;
        }

        tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of partial_balance structure
 * usefull when want to check all partial_balance
 *
 * \param none
 *
 * \return the g_slist of partial_balance structure
 * */
GSList *gsb_data_partial_balance_get_list ( void )
{
    return partial_balance_list;
}


/**
 * return the number of the partial_balance given in param
 *
 * \param balance_ptr a pointer to the struct of the partial_balance
 *
 * \return the number of the partial_balance, 0 if problem
 * */
gint gsb_data_partial_balance_get_number ( gpointer balance_ptr )
{
    struct_partial_balance *partial_balance;

    if ( !balance_ptr )
        return 0;

    partial_balance = balance_ptr;
    partial_balance_buffer = partial_balance;
    return partial_balance -> partial_balance_number;
}


/**
 * create a new partial_balance, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the partial_balance (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new partial_balance
 * */
gint gsb_data_partial_balance_new ( const gchar *name )
{
    struct_partial_balance *partial_balance;

    partial_balance = g_malloc0 ( sizeof ( struct_partial_balance ));
    if ( ! partial_balance )
    {
        dialogue_error_memory ( );
        return 0;
    }
    partial_balance -> partial_balance_number = g_slist_length ( partial_balance_list ) + 1;

    if ( name )
        partial_balance -> balance_name = my_strdup ( name );
    else 
        partial_balance -> balance_name = NULL;

    partial_balance_list = g_slist_insert_sorted ( partial_balance_list,
                        partial_balance,
                        (GCompareFunc) gsb_data_partial_balance_cmp_func );

    partial_balance_buffer = partial_balance;

    return partial_balance -> partial_balance_number;
}


/**
 * This internal function is called to free the memory used by an 
 * struct_partial_balance structure
 */
static void _gsb_data_partial_balance_free ( struct_partial_balance *partial_balance)
{
    if ( ! partial_balance )
        return;
    if ( partial_balance -> balance_name )
        g_free ( partial_balance -> balance_name );
    g_free ( partial_balance );
    if ( partial_balance_buffer == partial_balance )
        partial_balance_buffer = NULL;
}


/**
 * set a new number for the partial_balance
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param partial_balance_number the number of the partial_balance
 * \param new_no_partial_balance the new number of the partial_balance
 *
 * \return the new number or 0 if the partial_balance doen't exist
 * */
gint gsb_data_partial_balance_set_new_number ( gint partial_balance_number,
                        gint new_no_partial_balance )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    partial_balance -> partial_balance_number = new_no_partial_balance;
    return new_no_partial_balance;
}


/**
 * return the liste_cptes of the partial_balance
 *
 * \param partial_balance_number the number of the partial_balance
 *
 * \return the liste_cptes of the partial_balance or NULL if fail
 * */
const gchar *gsb_data_partial_balance_get_liste_cptes ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return NULL;

    return partial_balance -> liste_cptes;
}


/**
 * set the liste_cptes of the partial_balance
 * the value is dupplicate in memory
 *
 * \param partial_balance_number the number of the partial_balance
 * \param liste_cptes of the partial_balance
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_partial_balance_set_liste_cptes ( gint partial_balance_number,
                        const gchar *liste_cptes )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    /* we free the last name */
    if ( partial_balance -> liste_cptes )
        g_free (partial_balance -> liste_cptes);

    /* and copy the new one */
    partial_balance -> liste_cptes = my_strdup ( liste_cptes );

    return TRUE;
}


/**
 * return the name of the partial_balance
 *
 * \param partial_balance_number the number of the partial_balance
 *
 * \return the name of the partial_balance or NULL if fail
 * */
const gchar *gsb_data_partial_balance_get_name ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return NULL;

    return partial_balance -> balance_name;
}


/**
 * set the name of the import_rule
 * the value is dupplicate in memory
 *
 * \param import_rule_number the number of the import_rule
 * \param name the name of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_partial_balance_set_name ( gint partial_balance_number,
                        const gchar *name )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    /* we free the last name */
    if ( partial_balance -> balance_name )
        g_free (partial_balance -> balance_name);

    /* and copy the new one */
    partial_balance -> balance_name = my_strdup ( name );

    return TRUE;
}


/** get the kind of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \return partial_balance type or 0 if the partial_balance doesn't exist
 * */
kind_account gsb_data_partial_balance_get_kind ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    return partial_balance -> kind;
}


/** set the kind of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \param kind type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_partial_balance_set_kind ( gint partial_balance_number,
                        kind_account kind )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    partial_balance -> kind = kind;

    return TRUE;
}


/** get the currency of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \return partial_balance currency or 0 if the partial_balance doesn't exist
 * */
gint gsb_data_partial_balance_get_currency ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    return partial_balance -> currency;
}


/** set the currency of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \param partial_balance_kind type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_partial_balance_set_currency ( gint partial_balance_number,
                        gint currency )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    partial_balance -> currency = currency;

    return TRUE;
}


gchar *gsb_data_partial_balance_get_marked_balance ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;
    gsb_real solde = null_real;
    gchar **tab;
    gint i;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    tab = g_strsplit ( partial_balance -> liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        solde = gsb_real_add ( solde,
                        gsb_data_account_get_marked_balance (
                        utils_str_atoi ( tab[i] ) ) );
    }
    return gsb_real_get_string_with_currency (solde, partial_balance -> currency, TRUE);
}


gchar *gsb_data_partial_balance_get_current_balance ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;
    gsb_real solde = null_real;
    gchar **tab;
    gint i;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    tab = g_strsplit ( partial_balance -> liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        solde = gsb_real_add ( solde,
                        gsb_data_account_get_current_balance (
                        utils_str_atoi ( tab[i] ) ) );
    }
    return gsb_real_get_string_with_currency (solde, partial_balance -> currency, TRUE);
}


gint gsb_data_partial_balance_cmp_func ( struct_partial_balance *partial_balance_1,
                        struct_partial_balance *partial_balance_2 )
{
    if ( partial_balance_1 -> partial_balance_number 
        < partial_balance_2 -> partial_balance_number )
        return -1;
    else if ( partial_balance_1 -> partial_balance_number 
        == partial_balance_2 -> partial_balance_number )
        return 0;
    else
        return 1;
}


/** 
 *
 * \param 
 * \param 
 *
 * */

void gsb_partial_balance_fill_model ( GtkListStore *list_store, kind_account kind )
{
    GSList *list_tmp;
    GtkTreeIter iter;
    gboolean test = FALSE;

    list_tmp = partial_balance_list;
    gtk_list_store_clear ( GTK_LIST_STORE (list_store) );

    if ( kind < GSB_TYPE_LIABILITIES )
        test = TRUE;

    while ( list_tmp )
    {
        struct_partial_balance *partial_balance;

        partial_balance = list_tmp -> data;

        if ( test || partial_balance -> kind == kind )
        {
            gchar *kind_str = "";
            gchar *currency_str = "";

            switch ( partial_balance -> kind )
            {
            case GSB_TYPE_CASH:
                kind_str = g_strdup ( _("Cash account") );
                break;

            case GSB_TYPE_LIABILITIES:
                kind_str = g_strdup ( _("Liabilities account") );
                break;

            case GSB_TYPE_ASSET:
                kind_str = g_strdup ( _("Assets account") );
                break;

            default:
                kind_str = g_strdup ( _("Bank account") );
            }
            currency_str = gsb_data_currency_get_name ( partial_balance -> currency );

            gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
            gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                        0, partial_balance -> balance_name,
                        1, partial_balance -> liste_cptes,
                        2, kind_str,
                        3, currency_str,
                        4, partial_balance -> partial_balance_number,
                        -1);
        }
        list_tmp = list_tmp -> next;
    }
}


/**
 * add a partial_balance
 *
 * */
void gsb_partial_balance_add ( GtkWidget *button, GtkWidget *main_widget )
{
    GtkTreeView *treeview;
    GtkWidget *dialog, *label, *table, *paddingbox, * main_vbox, * vbox;
    GtkWidget *entry_name, *entry_list, *bouton;
    //~ GtkTreeModel *model;
    //~ GtkTreeIter iter;
    //~ GSList *list_tmp;
    //~ gint partial_balance_number;
    gint result;

    devel_debug ( NULL);

    dialog = gtk_dialog_new_with_buttons ( _("Add a partial balance"),
                        GTK_WINDOW ( window ),
                        GTK_DIALOG_MODAL,
                        GTK_STOCK_CANCEL, 0,
                        GTK_STOCK_OK, 1,
                        NULL );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    main_vbox = new_vbox_with_title_and_icon ( _("Add a partial balance"), NULL );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), main_vbox, TRUE, TRUE, 0 );

    vbox = gtk_vbox_new ( FALSE, 12 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), vbox, TRUE, TRUE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 12 );

    paddingbox = new_paddingbox_with_title ( vbox,FALSE, _("Details") );

    /* Create table */
    table = gtk_table_new ( 5, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Partial balance name */
    label = gtk_label_new ( COLON ( _("Name") ) );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1 );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_name = gtk_entry_new ( );
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry_name ), TRUE );
    gtk_table_attach ( GTK_TABLE ( table ), entry_name, 1, 2, 0, 1,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    /* List of the accounts */
    label = gtk_label_new ( COLON ( _("Accounts list") ) );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_list = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entry_list, 1, 2, 1, 2,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    /* create the position */
    label = gtk_label_new ( COLON ( _("Position in the list of accounts") ) );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    bouton = gtk_spin_button_new_with_range ( 1.0, 100.0, 1.0);
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( bouton ),
                        g_slist_length ( partial_balance_list ) + 1 );
    gtk_table_attach ( GTK_TABLE ( table ), bouton, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );

dialog_return:
    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( result == 1)
    {
        const gchar *name, *list;
        gint position;

        name = gtk_entry_get_text ( GTK_ENTRY ( entry_name ) );
        list = gtk_entry_get_text ( GTK_ENTRY ( entry_list ) );
        //~ kind = gtk_entry_get_text ( GTK_ENTRY ( entry_kind ) );

        if ( strlen ( name ) && strlen ( list ) )
        {
            //~ currency_name = gtk_entry_get_text ( GTK_ENTRY ( entry_currency ));
            position = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( bouton ) );
            treeview = g_object_get_data ( G_OBJECT (main_widget), "treeview" );
            //~ if ( !gtk_tree_selection_get_selected (
                                //~ gtk_tree_view_get_selection (treeview),
                                //~ &model,
                                //~ &iter ))
                //~ return;

            //~ gtk_tree_model_get ( model, &iter, 4, &partial_balance_number, -1 );

            //~ if ( partial_balance_number > 0 )
            //~ {
            //~ list_tmp = partial_balance_list;
            //~ while ( list_tmp )
            //~ {
                //~ struct_partial_balance *partial_balance;

                //~ partial_balance = list_tmp -> data;

                //~ if ( partial_balance -> partial_balance_number == partial_balance_number )
                //~ {
                    //~ GtkWidget *del_button;
                    //~ gint partial_balance_kind;

                    //~ partial_balance_kind = partial_balance -> kind;
                    //~ partial_balance_list = g_slist_remove (
                            //~ partial_balance_list, partial_balance );
                    //~ gsb_partial_balance_fill_model ( GTK_LIST_STORE ( model ),
                            //~ partial_balance_kind );
                    //~ del_button = g_object_get_data ( G_OBJECT (main_widget), "remove_button" );
                    //~ gtk_widget_set_sensitive ( button, FALSE );
                    //~ break;
                //~ }
                //~ list_tmp = list_tmp -> next;
            //~ }
            //~ }
        }
        else
        {
            dialogue_warning_hint ( _("The name of the partial balance "
                        "and the list of accounts must be completed."),
                        _("All fields are not filled in") );
            goto dialog_return;
        }
    }
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
}


/**
 * remove a partial_balance
 *
 * */
void gsb_partial_balance_delete ( GtkWidget *button, GtkWidget *main_widget )
{
    GtkTreeView *treeview;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *list_tmp;
    gint partial_balance_number;

    treeview = g_object_get_data ( G_OBJECT (main_widget), "treeview" );
    if ( !gtk_tree_selection_get_selected (
                        gtk_tree_view_get_selection (treeview),
                        &model,
                        &iter ))
        return;

    gtk_tree_model_get ( model, &iter, 4, &partial_balance_number, -1 );

    if ( partial_balance_number > 0 )
    {
        list_tmp = partial_balance_list;
        while ( list_tmp )
        {
            struct_partial_balance *partial_balance;

            partial_balance = list_tmp -> data;

            if ( partial_balance -> partial_balance_number == partial_balance_number )
            {
                GtkWidget *del_button;

                partial_balance_list = g_slist_remove (
                        partial_balance_list, partial_balance );
                gsb_partial_balance_renumerote ( );
                gtk_list_store_remove  ( GTK_LIST_STORE ( model ), &iter );
                if ( gsb_gui_navigation_get_current_page ( ) == GSB_HOME_PAGE )
                    mise_a_jour_accueil ( TRUE );
                else
                    mise_a_jour_liste_comptes_accueil = ( TRUE );
                del_button = g_object_get_data ( G_OBJECT (main_widget), "remove_button" );
                gtk_widget_set_sensitive ( button, FALSE );
                break;
            }
            list_tmp = list_tmp -> next;
        }
    }
}


gboolean gsb_partial_balance_select_func ( GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        GObject *main_widget )
{
    GtkWidget *button;

    button = g_object_get_data ( G_OBJECT (main_widget), "remove_button" );
    gtk_widget_set_sensitive ( button, TRUE );

    return TRUE;
}


/**
 * renumerote la liste des soldes partiels
 *
 * */
void gsb_partial_balance_renumerote ( void )
{
    GSList *list_tmp;
    gint i = 1;

    list_tmp = partial_balance_list;
    while ( list_tmp )
    {
        struct_partial_balance *partial_balance;

        partial_balance = list_tmp -> data;
        partial_balance -> partial_balance_number = i;
        i++;
        list_tmp = list_tmp -> next;
    }
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
