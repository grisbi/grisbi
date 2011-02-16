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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_partial_balance.h"
#include "dialog.h"
#include "gsb_currency_config.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "navigation.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "utils.h"
#include "structures.h"
#include "erreur.h"
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
    gboolean colorise;
} struct_partial_balance;


/*START_STATIC*/
static void _gsb_data_partial_balance_free ( struct_partial_balance *partial_balance);
static gpointer gsb_data_partial_balance_get_structure ( gint partial_balance_number );
static gboolean gsb_data_partial_balance_init_from_liste_cptes ( gint partial_balance_number,
                        GtkWidget *parent );
static gboolean gsb_data_partial_balance_move ( gint orig_partial_number, gint dest_pos );
static GtkWidget *gsb_partial_balance_create_dialog ( gint action, gint spin_value );
static GtkWidget *gsb_partial_balance_create_list_accounts ( GtkWidget *entry );
static gint gsb_partial_balance_new ( const gchar *name );
static void gsb_partial_balance_renumerote ( void );
static gint gsb_partial_balance_request_currency ( GtkWidget *parent );
static gboolean gsb_partial_balance_select_account ( GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        GObject *entry );
static void gsb_partial_balance_selectionne_cptes ( GtkWidget *tree_view,
                        const gchar *liste_cptes );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/

/* devise de base des soldes partiels de comptes ayant des devises différentes */
gint no_devise_solde_partiels;

/** contains the g_slist of struct_partial_balance */
static GSList *partial_balance_list = NULL;

/** a pointer to the last partial_balance used (to increase the speed) */
static struct_partial_balance *partial_balance_buffer;

static GtkListStore *model_accueil;

/*********************************************************************************************/
/*              Fonctions générales                                                          */
/*********************************************************************************************/
/**
 * create a new partial_balance, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the partial_balance (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new partial_balance
 * */
gint gsb_partial_balance_new ( const gchar *name )
{
    struct_partial_balance *partial_balance;

    partial_balance = g_malloc0 ( sizeof ( struct_partial_balance ) );
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

    partial_balance_list = g_slist_append ( partial_balance_list, partial_balance );

    partial_balance_buffer = partial_balance;

    return partial_balance -> partial_balance_number;
}


/**
 * create a new partial balance and insert it at position "pos"
 * and return the number
 *
 * \param name the name of the partial_balance (can be freed after, it's a copy) or NULL
 * \param pos
 *
 * \return the number of the new partial_balance
 * */
gint gsb_partial_balance_new_at_position ( const gchar *name, gint pos )
{
    struct_partial_balance *partial_balance;

    partial_balance = g_malloc0 ( sizeof ( struct_partial_balance ) );
    if ( ! partial_balance )
    {
        dialogue_error_memory ( );
        return 0;
    }

    if ( name )
        partial_balance -> balance_name = my_strdup ( name );
    else 
        partial_balance -> balance_name = NULL;

    partial_balance_list = g_slist_insert ( partial_balance_list, partial_balance, pos - 1 );
    gsb_partial_balance_renumerote ( );

    partial_balance_buffer = partial_balance;

    return partial_balance -> partial_balance_number;
}


/** 
 *
 * \param 
 * \param 
 *
 * */
GtkListStore *gsb_partial_balance_create_model ( void )
{

    model_accueil = gtk_list_store_new ( 6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                        G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN );

    return model_accueil;
}


/** 
 *
 * \param 
 * \param 
 *
 * */
void gsb_partial_balance_fill_model ( GtkListStore *list_store )
{
    GSList *list_tmp;
    GtkTreeIter iter;

    list_tmp = partial_balance_list;
    gtk_list_store_clear ( GTK_LIST_STORE (list_store) );

    while ( list_tmp )
    {
        struct_partial_balance *partial_balance;
        gchar *kind_str = "";
        gchar *currency_str = "";

        partial_balance = list_tmp -> data;

        switch ( partial_balance -> kind )
        {
        case -1:
            kind_str = g_strdup ( _("Additional balance") );
            break;

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
                    5, partial_balance -> colorise,
                    -1);
        list_tmp = list_tmp -> next;
    }
}


/**
 * add a partial_balance
 *
 * */
void gsb_partial_balance_add ( GtkWidget *button, GtkWidget *main_widget )
{
    GtkWidget *dialog;
    GtkWidget *entry_name;
    GtkWidget *entry_list;
    GtkWidget *spin_bouton;
    GtkWidget *colorise_bouton;
    gint action = 1; /* 1 create 2 modify */
    gint result;

    devel_debug ( NULL);

    dialog = gsb_partial_balance_create_dialog ( action,
                        g_slist_length ( partial_balance_list ) + 1 );

    entry_name = g_object_get_data ( G_OBJECT ( dialog ), "entry_name" );
    entry_list = g_object_get_data ( G_OBJECT ( dialog ), "entry_list" );
    spin_bouton = g_object_get_data ( G_OBJECT ( dialog ), "spin_bouton" );
    colorise_bouton = g_object_get_data ( G_OBJECT ( dialog ), "colorise_bouton" );

    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );

dialog_return:
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( result == 1)
    {
        GtkTreeView *treeview;
        GtkTreeModel *model;
        const gchar *name, *liste_cptes;
        gint partial_balance_number;
        gint position;

        name = gtk_entry_get_text ( GTK_ENTRY ( entry_name ) );
        liste_cptes = gtk_entry_get_text ( GTK_ENTRY ( entry_list ) );

        if ( strlen ( name ) && strlen ( liste_cptes ) 
         && 
         g_utf8_strchr  ( liste_cptes, -1, ';' ) )
        {
            position = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_bouton ) );
            if ( position > g_slist_length ( partial_balance_list ) )
                partial_balance_number = gsb_partial_balance_new ( name );
            else
                partial_balance_number = gsb_partial_balance_new_at_position (
                        name, position );

            gsb_data_partial_balance_set_liste_cptes ( partial_balance_number,
                        liste_cptes );
            gsb_data_partial_balance_init_from_liste_cptes ( partial_balance_number,
                        dialog );
            gsb_data_partial_balance_set_colorise ( partial_balance_number,
                        gtk_toggle_button_get_active (
                        GTK_TOGGLE_BUTTON ( colorise_bouton ) ) );

            /* on met à jour le model */
            treeview = g_object_get_data ( G_OBJECT (main_widget), "treeview" );
            model = gtk_tree_view_get_model ( treeview );
            gsb_partial_balance_fill_model ( GTK_LIST_STORE ( model ) );

            /* MAJ HOME_PAGE */
            gsb_gui_navigation_update_home_page ( );
        }
        else if (  g_utf8_strchr ( liste_cptes, -1, ';' ) == NULL )
        {
            dialogue_warning_hint ( _("You must select at least two accounts."),
                        _("Only one account is selected.") );
            goto dialog_return;
        }
        else
        {
            dialogue_warning_hint ( _("The name of the partial balance "
                        "and the list of accounts must be completed."),
                        _("All fields are not filled in") );
            goto dialog_return;
        }
    }
    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
}


/**
 * Edit a partial_balance
 *
 * */
void gsb_partial_balance_edit ( GtkWidget *button, GtkWidget *main_widget )
{
    GtkWidget *dialog;
    GtkWidget *entry_name;
    GtkWidget *entry_list;
    GtkWidget *spin_bouton;
    GtkWidget *colorise_bouton;
    GtkWidget *account_list;
    GtkWidget *treeview;
    GtkWidget *account_treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *balance_name;
    gchar *liste_cptes;
    gint partial_balance_number;
    gint action = 2; /* 1 create 2 modify */
    gint result;

    devel_debug ( NULL);

    dialog = gsb_partial_balance_create_dialog ( action,
                        g_slist_length ( partial_balance_list ) + 1 );

    entry_name = g_object_get_data ( G_OBJECT ( dialog ), "entry_name" );
    entry_list = g_object_get_data ( G_OBJECT ( dialog ), "entry_list" );
    account_list = g_object_get_data ( G_OBJECT ( dialog ), "account_list" );
    spin_bouton = g_object_get_data ( G_OBJECT ( dialog ), "spin_bouton" );
    colorise_bouton = g_object_get_data ( G_OBJECT ( dialog ), "colorise_bouton" );

    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );

    /* initialisation des données */
    treeview = g_object_get_data ( G_OBJECT ( main_widget ), "treeview" );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (treeview) );
    if ( !gtk_tree_selection_get_selected ( selection, &model, &iter) )
        return ;

    gtk_tree_model_get ( model, &iter,
                        0, &balance_name,
                        1, &liste_cptes,
                        4, &partial_balance_number,
                        -1);

    gtk_entry_set_text ( GTK_ENTRY ( entry_name ), balance_name );
    gtk_entry_set_text ( GTK_ENTRY ( entry_list ), liste_cptes );
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_bouton ),
                        (gdouble) partial_balance_number );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( colorise_bouton ),
                        gsb_data_partial_balance_get_colorise (
                        partial_balance_number ) );
    account_treeview = g_object_get_data ( G_OBJECT ( account_list ), "account_treeview");
    gsb_partial_balance_selectionne_cptes ( account_treeview, liste_cptes );

    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );

dialog_return:
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( result == 1)
    {
        const gchar *name, *liste_cptes;
        gint position;

        name = gtk_entry_get_text ( GTK_ENTRY ( entry_name ) );
        liste_cptes = gtk_entry_get_text ( GTK_ENTRY ( entry_list ) );

        if ( strlen ( name ) && strlen ( liste_cptes ) && 
         g_utf8_strchr  ( liste_cptes, -1, ';' ) )
        {
            gsb_data_partial_balance_set_name ( partial_balance_number, name );
            gsb_data_partial_balance_set_liste_cptes ( partial_balance_number, liste_cptes );
            gsb_data_partial_balance_init_from_liste_cptes ( partial_balance_number,
                        dialog );
            gsb_data_partial_balance_set_colorise ( partial_balance_number,
                        gtk_toggle_button_get_active (
                        GTK_TOGGLE_BUTTON ( colorise_bouton ) ) );

            position = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_bouton ) );
            if ( position != partial_balance_number )
            gsb_data_partial_balance_move ( partial_balance_number, position );

            /* on met à jour le model */
            gsb_partial_balance_fill_model ( GTK_LIST_STORE ( model ) );

            /* MAJ HOME_PAGE */
            gsb_gui_navigation_update_home_page ( );
        }
        else if (  g_utf8_strchr ( liste_cptes, -1, ';' ) == NULL )
        {
            dialogue_warning_hint ( _("You must select at least two accounts."),
                        _("Only one account is selected.") );
            goto dialog_return;
        }
        else
        {
            dialogue_warning_hint ( _("The name of the partial balance "
                        "and the list of accounts must be completed."),
                        _("All fields are not filled in") );
            goto dialog_return;
        }
    }
    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
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
                GtkWidget *edit_button;

                partial_balance_list = g_slist_remove (
                        partial_balance_list, partial_balance );
                gsb_partial_balance_renumerote ( );
                gtk_list_store_remove  ( GTK_LIST_STORE ( model ), &iter );

                /* MAJ HOME_PAGE */
                gsb_gui_navigation_update_home_page ( );

                edit_button = g_object_get_data ( G_OBJECT (main_widget), "edit_button" );
                gtk_widget_set_sensitive ( edit_button, FALSE );
                gtk_widget_set_sensitive ( button, FALSE );
                break;
            }
            list_tmp = list_tmp -> next;
        }
    }
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


/**
 * Fonction appellée quand on sélectionne un solde partiel
 *
 * */
gboolean gsb_partial_balance_select_func ( GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        GObject *main_widget )
{
    GtkWidget *button;

    button = g_object_get_data ( G_OBJECT (main_widget), "edit_button" );
    gtk_widget_set_sensitive ( button, TRUE );
    button = g_object_get_data ( G_OBJECT (main_widget), "remove_button" );
    gtk_widget_set_sensitive ( button, TRUE );

    return TRUE;
}


/**
 * sélectionne les comptes donnés en paramètre
 *
 * */
void gsb_partial_balance_selectionne_cptes ( GtkWidget *tree_view,
                        const gchar *liste_cptes )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar **tab;
    gint i;
    gint num_cpte;
    gboolean valid;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( liste_cptes == NULL || strlen ( liste_cptes ) == 0 )
        return;

    tab = g_strsplit ( liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        num_cpte = utils_str_atoi ( tab[i] );

        valid = gtk_tree_model_get_iter_first ( model, &iter);
        while ( valid )
        {
            gint   account_nb;

            gtk_tree_model_get ( model, &iter, 1, &account_nb, -1 );
            if ( account_nb == num_cpte )
                gtk_tree_selection_select_iter ( selection, &iter);

            valid = gtk_tree_model_iter_next ( model, &iter );
        }
    }
}


/**
 * gère le clavier sur la liste des soldes partiels
 *
**/
gboolean gsb_partial_balance_key_press ( GtkWidget *tree_view, GdkEventKey *ev )
{
    switch ( ev -> keyval )
    {
    case GDK_Return :   /* entrée */
    case GDK_KP_Enter :
    case GDK_Tab :
        g_object_set_data ( G_OBJECT ( tree_view ), "treeview", tree_view );
        gsb_partial_balance_edit ( NULL, tree_view );
        return TRUE;
        break;
    case GDK_Delete:    /*  del  */
        g_object_set_data ( G_OBJECT ( tree_view ), "treeview", tree_view );
        gsb_partial_balance_delete ( NULL, tree_view );
        return TRUE;
        break;
    }

    return FALSE;
}


/**
 * called when press a mouse button on the partial_balance_list
 *
 * \param tree_view
 * \param ev a GdkEventButton
 *
 * \return TRUE if double - click else FALSE
 * */
gboolean gsb_partial_balance_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null )
{
    /*     if we are not in the list, go away */
    if ( ev -> window != gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view ) ) )
        return FALSE;

    /*  if double - click */
    if ( ev -> type == GDK_2BUTTON_PRESS )
    {
        g_object_set_data ( G_OBJECT ( tree_view ), "treeview", tree_view );
        gsb_partial_balance_edit ( NULL, tree_view );
        return TRUE;
    }

    return FALSE;
}


/**
 * 
 *
**/
void gsb_partial_balance_colorise_toggled ( GtkCellRendererToggle *cell,
                        gchar *path_str,
                        GtkWidget *tree_view )
{
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean toggle;
    gint partial_number;
    GtkTreeModel *model;

    devel_debug (NULL);

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    /* invert the toggle */
    treepath = gtk_tree_path_new_from_string ( path_str );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (model), &iter, treepath );

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 
                        4, &partial_number,
                        5, &toggle, 
                        -1);
    toggle ^= 1;
    gtk_list_store_set (GTK_LIST_STORE ( model ), &iter, 5, toggle, -1);

     /* and save it */
    gsb_data_partial_balance_set_colorise ( partial_number, toggle );

    /* MAJ HOME_PAGE */
    gsb_gui_navigation_update_home_page ( );
}


/*********************************************************************************************/
/*              Données                                                                      */
/*********************************************************************************************/
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
 * carrefull : it's not a copy, so we must not free or change it
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
 * This internal function is called to free the memory used by an 
 * struct_partial_balance structure
 */
static void _gsb_data_partial_balance_free ( struct_partial_balance *partial_balance)
{
    if ( ! partial_balance )
        return;
    if ( partial_balance -> balance_name 
     &&
     strlen ( partial_balance -> balance_name ) )
        g_free ( partial_balance -> balance_name );
    if ( partial_balance -> liste_cptes 
     &&
     strlen ( partial_balance -> liste_cptes ) )
        g_free ( partial_balance -> liste_cptes );

    g_free ( partial_balance );

    if ( partial_balance_buffer == partial_balance )
        partial_balance_buffer = NULL;
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


/** 
 * get the kind of the partial_balance
 * \param partial_balance_number no of the partial_balance
 *
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


/** 
 * set the kind of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \param kind type to set
 *
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


/** 
 * get the currency of the partial_balance
 *
 * \param partial_balance_number no of the partial_balance
 *
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


/** 
 * set the currency of the partial_balance
 *
 * \param partial_balance_number no of the partial_balance
 * \param currency to set
 *
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


/** 
 * 
 *
 * \param partial_balance_number no of the partial_balance
 *
 * \return 
 * */
gboolean gsb_data_partial_balance_get_colorise ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    return partial_balance -> colorise;
}


/** 
 * \param partial_balance_number no of the partial_balance
 * \param colorise
 *
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_partial_balance_set_colorise ( gint partial_balance_number,
                        gboolean colorise )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    partial_balance -> colorise = colorise;

    return TRUE;
}


/**
 * 
 *
 * */
gchar *gsb_data_partial_balance_get_marked_balance ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;
    gsb_real solde = null_real;
    gchar **tab;
    gchar *string;
    gint i;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return NULL;

    if ( partial_balance -> liste_cptes == NULL || 
     strlen ( partial_balance -> liste_cptes ) == 0 )
        return NULL;

    tab = g_strsplit ( partial_balance -> liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        gsb_real tmp_real;
        gint account_nb;
        gint account_currency;
        gint link_number;

        account_nb = utils_str_atoi ( tab[i] );
        account_currency = gsb_data_account_get_currency ( account_nb );
        tmp_real = gsb_data_account_get_marked_balance ( account_nb );

        if ( tmp_real.mantissa != 0 && partial_balance -> currency != account_currency )
        {
            if ( ( link_number = gsb_data_currency_link_search ( account_currency,
                        partial_balance -> currency ) ) )
            {
                if ( gsb_data_currency_link_get_first_currency (
                 link_number) == account_currency )
                    tmp_real = gsb_real_mul ( tmp_real,
                                gsb_data_currency_link_get_change_rate ( link_number ) );
                else
                    tmp_real = gsb_real_div ( tmp_real,
                                gsb_data_currency_link_get_change_rate ( link_number ) );
            }
            
        }
        solde = gsb_real_add ( solde, tmp_real );
    }

    if ( partial_balance -> colorise && solde.mantissa < 0 )
        string = g_strdup_printf ( "<span color=\"red\">%s</span>",
                        gsb_real_get_string_with_currency (
                        solde, partial_balance -> currency, TRUE ) );
    else
        string = gsb_real_get_string_with_currency (
                        solde, partial_balance -> currency, TRUE );

    return string;
}


/**
 * 
 *
 * */
gsb_real gsb_data_partial_balance_get_current_amount ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;
    gsb_real solde = null_real;
    gchar **tab;
    gint i;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return null_real;

    if ( partial_balance -> liste_cptes == NULL || 
     strlen ( partial_balance -> liste_cptes ) == 0 )
        return null_real;

    tab = g_strsplit ( partial_balance -> liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        gsb_real tmp_real;
        gint account_nb;
        gint account_currency;
        gint link_number;

        account_nb = utils_str_atoi ( tab[i] );
        account_currency = gsb_data_account_get_currency ( account_nb );
        tmp_real = gsb_data_account_get_current_balance ( account_nb );

        if ( tmp_real.mantissa != 0 && partial_balance -> currency != account_currency )
        {
            if ( ( link_number = gsb_data_currency_link_search ( account_currency,
                        partial_balance -> currency ) ) )
            {
                if ( gsb_data_currency_link_get_first_currency (
                        link_number) == account_currency )
                    tmp_real = gsb_real_mul ( tmp_real,
                                gsb_data_currency_link_get_change_rate ( link_number ) );
                else
                    tmp_real = gsb_real_div ( tmp_real,
                                gsb_data_currency_link_get_change_rate ( link_number ) );
            }
        }
        solde = gsb_real_add ( solde, tmp_real );
    }

    return solde;
}


/**
 * 
 *
 * */
gchar *gsb_data_partial_balance_get_current_balance ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;
    gsb_real solde = null_real;
    gchar *string;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return NULL;

    solde = gsb_data_partial_balance_get_current_amount ( partial_balance_number );

    if ( partial_balance -> colorise && solde.mantissa < 0 )
        string = g_strdup_printf ( "<span color=\"red\">%s</span>",
                        gsb_real_get_string_with_currency (
                        solde, partial_balance -> currency, TRUE ) );
    else
        string = gsb_real_get_string_with_currency (
                        solde, partial_balance -> currency, TRUE );

    return string;
}


/**
 * 
 *
 * */
gboolean gsb_partial_balance_select_account ( GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        GObject *entry )
{
    GList *list;

    if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) > 0 )
        gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );

    list = gtk_tree_selection_get_selected_rows ( selection, NULL );

    if ( path_currently_selected )
    {
        while ( list )
        {
            GtkTreeIter iter;
            gint account_nb;

            if ( gtk_tree_path_compare ( path, list -> data ) != 0 )
            {
                gtk_tree_model_get_iter ( model, &iter, list -> data );
                gtk_tree_model_get ( model, &iter, 1, &account_nb, -1);
                if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) > 0 )
                {
                    gtk_entry_set_text ( GTK_ENTRY ( entry ), 
                                g_strconcat ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ), ";",
                                g_strdup_printf ( "%d", account_nb ), NULL ) );
                }
                else
                    gtk_entry_set_text ( GTK_ENTRY ( entry ), g_strdup_printf ( "%d", account_nb ) );
            }
            list = list -> next;
        }
    }
    else
    {
        list = g_list_append ( list, gtk_tree_path_copy ( path ) );

        while ( list )
        {
            GtkTreeIter iter;
            gint account_nb;

            gtk_tree_model_get_iter ( model, &iter, list -> data );
            gtk_tree_model_get ( model, &iter, 1, &account_nb, -1);
            if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) > 0 )
            {
                gtk_entry_set_text ( GTK_ENTRY ( entry ), 
                            g_strconcat ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ), ";",
                            g_strdup_printf ( "%d", account_nb ), NULL ) );
            }
            else
                gtk_entry_set_text ( GTK_ENTRY ( entry ), g_strdup_printf ( "%d", account_nb ) );

            list = list -> next;
        }
    }

    g_list_foreach ( list, ( GFunc ) gtk_tree_path_free, NULL );
    g_list_free ( list );

    return TRUE;
}


/**
 * Détermine la devise et le type de compte pour le solde partiel
 *
 * \param partial_balance_number
 *
 * \return FALSE si tous les comptes n'ont pas les mêmes données sinon TRUE
 * */
gboolean gsb_data_partial_balance_init_from_liste_cptes ( gint partial_balance_number,
                        GtkWidget *parent )
{
    struct_partial_balance *partial_balance;
    gchar **tab;
    gint i;
    gint account_nb;
    gint currency_nb = 0;
    kind_account kind = -1;
    kind_account kind_nb = -1;
    gchar *tmp_str;
    gboolean return_val = TRUE;
    gboolean currency_mixte = FALSE;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    if ( partial_balance -> liste_cptes == NULL || 
     strlen ( partial_balance -> liste_cptes ) == 0 )
        return FALSE;

    tab = g_strsplit ( partial_balance -> liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        gint account_currency;

        account_nb = utils_str_atoi ( tab[i] );
        account_currency = gsb_data_account_get_currency ( account_nb );
        if ( currency_nb == 0 )
            currency_nb = account_currency;
        else if ( currency_nb != account_currency )
        {
            if ( currency_mixte == FALSE )
            {
                no_devise_solde_partiels = gsb_partial_balance_request_currency ( parent );
                if ( gsb_data_currency_link_search ( currency_nb, account_currency ) == 0 )
                {
                    tmp_str = g_strdup_printf (
                            _("You need to create a link between currency %s and %s."),
                            gsb_data_currency_get_name ( currency_nb ),
                            gsb_data_currency_get_name ( account_currency ) );
                    dialogue_warning_hint ( tmp_str,
                            _("Attention missing link between currencies") );
                }
            }
            currency_mixte = TRUE;
            return_val = FALSE;
        }
        if ( kind == -1 )
            kind = gsb_data_account_get_kind ( account_nb );
        else if ( ( kind_nb = gsb_data_account_get_kind ( account_nb ) ) != kind )
        {
            switch ( kind )
            {
            case GSB_TYPE_BANK:
            case GSB_TYPE_CASH:
                if ( kind_nb >= GSB_TYPE_LIABILITIES )
                    return_val = FALSE;
                break;

            case GSB_TYPE_LIABILITIES:
                return_val = FALSE;
                break;

            case GSB_TYPE_ASSET:
                return_val = FALSE;
                break;
            }
        }
    }
    if ( currency_mixte )
        gsb_data_partial_balance_set_currency ( partial_balance_number,
                        no_devise_solde_partiels );
    else
        gsb_data_partial_balance_set_currency ( partial_balance_number, currency_nb );
    if ( return_val == FALSE )
        gsb_data_partial_balance_set_kind ( partial_balance_number, -1 );
    else
        gsb_data_partial_balance_set_kind ( partial_balance_number, kind );

    return return_val;
}


/**
 * callback when tree_view Home tab receive a drag and drop signal
 *
 * \param drag_dest
 * \param dest_path
 * \param selection_data
 *
 * \return FALSE
 */
gboolean gsb_data_partial_balance_drag_data_received ( GtkTreeDragDest * drag_dest,
                        GtkTreePath * dest_path,
                        GtkSelectionData * selection_data )
{
    gchar *tmpstr = gtk_tree_path_to_string ( dest_path );
    gchar *tmpstr2 = g_strdup_printf ( "Dest path : %s", tmpstr);
    devel_debug (tmpstr2);
    g_free (tmpstr);
    g_free (tmpstr2);

    if ( dest_path && selection_data )
    {
        GtkTreeModel * model;
        GtkTreeIter iter;
        GtkTreePath * orig_path;
        gint orig_partial_number = 0;
        gint dest_pos;

        /* On récupère le model et le path d'origine */
        gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);
        
        if ( gtk_tree_model_get_iter ( model, &iter, orig_path ) )
            gtk_tree_model_get ( model, &iter, 4, &orig_partial_number, -1 );

        dest_pos = utils_str_atoi ( gtk_tree_path_to_string ( dest_path ) );

        gsb_data_partial_balance_move ( orig_partial_number, dest_pos );
        gsb_partial_balance_fill_model ( GTK_LIST_STORE ( model ) );

        /* MAJ HOME_PAGE */
            gsb_gui_navigation_update_home_page ( );
    }
    return FALSE;
}


/**
 * Fill the drag & drop structure with the path of selected column.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_source		Not used.
 * \param path			Original path for the gtk selection.
 * \param selection_data	A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 */
gboolean gsb_data_partial_balance_drag_data_get ( GtkTreeDragSource * drag_source,
                        GtkTreePath * path,
                        GtkSelectionData * selection_data )
{
    if ( path )
        gtk_tree_set_row_drag_data ( selection_data, GTK_TREE_MODEL( model_accueil ), path );

    return FALSE;
}


/**
 * change the position of partial_balance in the list
 *
 * \param account_number	the partial_balance we want to move
 * \param dest_account_number	partial_balance before we want to move
 *
 * \return FALSE
 * */
gboolean gsb_data_partial_balance_move ( gint orig_partial_number, gint dest_pos )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( orig_partial_number );

    if ( !orig_partial_number )
        return FALSE;

    partial_balance_list = g_slist_remove ( partial_balance_list, partial_balance );
    partial_balance_list = g_slist_insert ( partial_balance_list, partial_balance, dest_pos );
    gsb_partial_balance_renumerote ( );

    return FALSE;
}


/*********************************************************************************************/
/*              Interface                                                                    */
/*********************************************************************************************/
/**
 * 
 *
 * */
GtkWidget *gsb_partial_balance_create_list_accounts ( GtkWidget *entry )
{
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *treeview;
    GtkListStore *list_store;
    GtkTreeIter iter;
    GSList *list_tmp;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;
    gint i = 0;

    vbox = gtk_vbox_new ( FALSE, 12 );
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                        GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_NEVER,
                        GTK_POLICY_ALWAYS);
    gtk_widget_set_size_request( sw, 600, 200 );

    /* create the model */
    list_store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
        gint account_number;

        account_number = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( !gsb_data_account_get_closed_account ( account_number ) )
        {
            gtk_list_store_append ( GTK_LIST_STORE ( list_store), &iter );
            gtk_list_store_set ( GTK_LIST_STORE ( list_store ), &iter,
                            0,  gsb_data_account_get_name ( account_number ),
                            1, account_number,
                            -1 );
            i++;
        }
        list_tmp = list_tmp -> next;
    }
    if ( i > 10 )
        i = 10;
    /* create the treeview */
    treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( list_store ) );
    g_object_unref ( list_store );

    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( treeview ), TRUE );
    gtk_widget_set_size_request ( treeview, -1, i*20 );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( treeview ) );
    gtk_tree_selection_set_mode ( selection, GTK_SELECTION_MULTIPLE );
    gtk_tree_selection_set_select_function ( selection,
                        (GtkTreeSelectionFunc) gsb_partial_balance_select_account,
                        entry, NULL );
    gtk_container_add ( GTK_CONTAINER ( sw ), treeview );
    gtk_container_set_resize_mode ( GTK_CONTAINER ( sw ), GTK_RESIZE_PARENT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), sw, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT (vbox), "account_treeview", treeview );

     /* account name */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Account name"),
                        cell,
                        "text",
                        0,
                        NULL );
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_sort_column_id ( column, 0 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( treeview ), column );

    return vbox;
}


/**
 * 
 *
 * */
GtkWidget *gsb_partial_balance_create_dialog ( gint action, gint spin_value )
{
    GtkWidget *dialog, *label, *table, *paddingbox, *main_vbox, *vbox;
    GtkWidget *entry_name, *entry_list, *account_list, *bouton;

    devel_debug ( NULL);

    if ( action == 1 )
        dialog = gtk_dialog_new_with_buttons ( _("Add a partial balance"),
                            GTK_WINDOW ( window ),
                            GTK_DIALOG_MODAL,
                            GTK_STOCK_CANCEL, 0,
                            GTK_STOCK_OK, 1,
                            NULL );
    else
        dialog = gtk_dialog_new_with_buttons ( _("Modify a partial balance"),
                            GTK_WINDOW ( window ),
                            GTK_DIALOG_MODAL,
                            GTK_STOCK_CANCEL, 0,
                            GTK_STOCK_OK, 1,
                            NULL );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    main_vbox = new_vbox_with_title_and_icon ( _("Partial balance details"), "payment.png" );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), main_vbox, TRUE, TRUE, 0 );

    vbox = gtk_vbox_new ( FALSE, 12 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), vbox, TRUE, TRUE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 12 );

    paddingbox = new_paddingbox_with_title ( vbox,FALSE, _("Details") );

    /* Create table */
    table = gtk_table_new ( 6, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Partial balance name */
    label = gtk_label_new ( _("Name: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1 );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_name = gtk_entry_new ( );
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry_name ), TRUE );
    gtk_table_attach ( GTK_TABLE ( table ), entry_name, 1, 2, 0, 1,
                        GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    /* List of the accounts */
    label = gtk_label_new ( _("Accounts list: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_list = gtk_entry_new ( );
    gtk_editable_set_editable ( GTK_EDITABLE ( entry_list ), FALSE );
    gtk_widget_set_sensitive ( entry_list, FALSE );
    gtk_table_attach ( GTK_TABLE ( table ), entry_list, 1, 2, 1, 2,
                        GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    account_list = gsb_partial_balance_create_list_accounts ( entry_list );
    gtk_table_attach ( GTK_TABLE ( table ), account_list, 0, 2, 2, 4,
                        GTK_EXPAND | GTK_FILL, 0, 0, 0 );

    /* create the position */
    label = gtk_label_new ( _("Position in the list of accounts: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 4, 5,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    bouton = gtk_spin_button_new_with_range ( 1.0, spin_value, 1.0);
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( bouton ),
                        g_slist_length ( partial_balance_list ) + 1 );
    gtk_table_attach ( GTK_TABLE ( table ), bouton, 1, 2, 4, 5,
                        GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT ( dialog ), "spin_bouton", bouton );

    /* create the colorized button */
    bouton = gtk_check_button_new_with_label ( _("Colorized in red if the balance is negative") );
    gtk_table_attach ( GTK_TABLE ( table ), bouton, 0, 2, 5, 6,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT ( dialog ), "colorise_bouton", bouton );
    
    g_object_set_data ( G_OBJECT ( dialog ), "entry_name", entry_name );
    g_object_set_data ( G_OBJECT ( dialog ), "entry_list", entry_list );
    g_object_set_data ( G_OBJECT ( dialog ), "account_list", account_list );

    return dialog;
}


/**
 * 
 *
 * */
gint gsb_partial_balance_request_currency ( GtkWidget *parent )
{
    GtkWidget *dialog, *hbox, *label, *combo_devise;
    gint currency_nb;
    gint result;

    dialog = gtk_dialog_new_with_buttons ( _("Enter the currency of the balance part"),
                            GTK_WINDOW ( parent ),
                            GTK_DIALOG_MODAL,
                            GTK_STOCK_CANCEL, 0,
                            GTK_STOCK_OK, 1,
                            NULL );
    gtk_widget_set_size_request ( dialog, -1, 150 );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox, TRUE, FALSE, 0 );

    label = gtk_label_new ( _("Select the currency of the partial balance: ") );
    gtk_misc_set_alignment ( GTK_MISC  ( label ), 0, 1 );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, TRUE, TRUE, 0 );

    combo_devise = gsb_currency_config_new_combobox ( &currency_nb, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo_devise, FALSE, FALSE, 10 );

    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );

    gsb_currency_set_combobox_history ( combo_devise, 2 );
    gsb_currency_set_combobox_history ( combo_devise, 1 );
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    return currency_nb;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
