/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009-2011 Pierre Biava (grisbi@pierre.biava.name)     */
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
 * \file tiers_onglet.c
 * deal with the payee page
 */



#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

#include "dialog.h"

/*START_INCLUDE*/
#include "tiers_onglet.h"
#include "fenetre_principale.h"
#include "grisbi_app.h"
#include "gsb_assistant.h"
#include "gsb_automem.h"
#include "gsb_data_form.h"
#include "gsb_data_mix.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_form_widget.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "import.h"
#include "meta_payee.h"
#include "metatree.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_editables.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void appui_sur_ajout_payee ( GtkTreeModel * model );
static GtkWidget *creation_barre_outils_tiers ( void );
static gboolean edit_payee ( GtkTreeView * view );
static void gsb_assistant_payees_clicked ( GtkButton *button, GtkWidget *assistant );
static gboolean gsb_assistant_payees_enter_page_2 ( GtkWidget *assistant );
static gboolean gsb_assistant_payees_enter_page_3 ( GtkWidget *assistant );
static gboolean gsb_assistant_payees_enter_page_finish ( GtkWidget *assistant );
static void gsb_assistant_payees_entry_changed ( GtkEditable *editable,
                        GtkWidget *assistant );
static void gsb_assistant_payees_modifie_operations ( GSList *sup_payees,
                        gint transaction_number,
                        gint new_payee_number,
                        gboolean save_notes,
                        gboolean extract_num,
                        gboolean is_transaction );
static GtkWidget *gsb_assistant_payees_page_2 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_payees_page_3 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_payees_page_finish ( GtkWidget *assistant );
static gboolean gsb_assistant_payees_select_func (GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data);
static void gsb_assistant_payees_toggled ( GtkCellRendererToggle *cell,
                        gchar *path_str,
                        GtkWidget *assistant );
static gint gsb_assistant_payees_valide_model_recherche ( const gchar *needle );
static gboolean payee_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
                        GtkSelectionData * selection_data );
static gboolean payee_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null );
/* static gboolean payee_remove_unused ( GtkWidget *button,
 *                         gpointer null );
 */
static void payee_tree_update_transactions ( GtkTreeModel * model,
                        MetatreeInterface * iface, GtkTreeIter * iter,
                        gint division, gchar * old_payee );
static gboolean popup_payee_view_mode_menu ( GtkWidget * button );
/*END_STATIC*/


static GtkWidget *payee_toolbar;
static GtkWidget *payee_tree = NULL;
static GtkTreeStore *payee_tree_model = NULL;

/* variable for display payees without transactions */
static gboolean display_unused_payees;

/* variable for the management of the cancelled edition */
static gboolean sortie_edit_payee = FALSE;

/* structure pour la sauvegarde de la position */
static struct metatree_hold_position *payee_hold_position;

/*START_EXTERN*/
/*END_EXTERN*/

enum payees_assistant_page
{
    PAYEES_ASSISTANT_INTRO= 0,
    PAYEES_ASSISTANT_PAGE_2,
    PAYEES_ASSISTANT_PAGE_3,
    PAYEES_ASSISTANT_PAGE_FINISH,
};

enum {
  COLUMN_BOOLEAN,
  COLUMN_INT,
  COLUMN_STRING,
  N_COLUMNS
};

/**
 * réinitialisation des variables globales
 *
 * \param
 *
 * \return
 * */
void payees_init_variables_list ( void )
{
    payee_tree_model = NULL;

    payee_toolbar = NULL;
    payee_tree = NULL;
    etat.no_devise_totaux_tiers = 1;

    display_unused_payees = FALSE;
    sortie_edit_payee = FALSE;
}


/*
 * Crée la liste des Tiers
 *
 * \param
 *
 * \return	A newly-allocated widget.
* */
GtkWidget *payees_create_list ( void )
{
    GtkWidget *onglet, *scroll_window;
    GtkWidget *frame;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {
    { "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };

    /* création de la fenêtre qui sera renvoyée */
    onglet = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_widget_show ( onglet );

    /* frame pour la barre d'outils */
    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( onglet ), frame, FALSE, FALSE, 0 );

    /* We create the gtktreeview and model early so that they can be referenced. */
    payee_tree = gtk_tree_view_new();

    /* set the color of selected row */
	gtk_widget_set_name (payee_tree, "tree_view");

    payee_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, META_TREE_COLUMN_TYPES );

    /* on y ajoute la barre d'outils */
    payee_toolbar = creation_barre_outils_tiers ();
    gtk_container_add ( GTK_CONTAINER ( frame ), payee_toolbar );

    /* création de l'arbre principal */
    scroll_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_window ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scroll_window),
                        GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ), scroll_window, TRUE, TRUE, 0 );
    gtk_widget_show ( scroll_window );

    /* Create model */
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(payee_tree_model),
                        META_TREE_TEXT_COLUMN, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE (payee_tree_model),
                        META_TREE_TEXT_COLUMN, metatree_sort_column,
                        NULL, NULL );
    g_object_set_data ( G_OBJECT ( payee_tree_model), "metatree-interface",
                        payee_get_metatree_interface ( ) );

    /* Create container + TreeView */
    gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(payee_tree),
                        GDK_BUTTON1_MASK, row_targets, 1,
                        GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW(payee_tree), row_targets,
                        1, GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW(payee_tree), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(payee_tree)),
                    GTK_SELECTION_SINGLE );
    gtk_tree_view_set_model (GTK_TREE_VIEW (payee_tree),
                    GTK_TREE_MODEL (payee_tree_model));
    g_object_unref (G_OBJECT(payee_tree_model));
    g_object_set_data ( G_OBJECT(payee_tree_model), "tree-view", payee_tree );

    /* Make category column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Payees"), cell,
						       "text", META_TREE_TEXT_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( payee_tree ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make account column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Account"), cell,
						       "text", META_TREE_ACCOUNT_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( payee_tree ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make amount column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Amount"), cell,
						       "text", META_TREE_BALANCE_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
						       "xalign", META_TREE_XALIGN_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( payee_tree ),
				  GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_alignment ( column, 1.0 );
    gtk_container_add ( GTK_CONTAINER ( scroll_window ), payee_tree );
    gtk_widget_show ( payee_tree );

    /* Connect to signals */
    g_signal_connect ( G_OBJECT ( payee_tree ),
                        "row-collapsed",
                        G_CALLBACK ( division_column_collapsed ),
                        NULL );

    g_signal_connect ( G_OBJECT ( payee_tree ),
                        "row-expanded",
                        G_CALLBACK ( division_column_expanded ),
                        NULL );

    g_signal_connect ( G_OBJECT ( payee_tree ),
                        "row-activated",
                        G_CALLBACK ( division_activated ),
                        NULL );

    g_signal_connect ( G_OBJECT ( payee_tree ),
                        "button-press-event",
                        G_CALLBACK ( payee_list_button_press ),
                        NULL );

    dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (payee_tree_model);
    if ( dst_iface )
    {
	dst_iface -> drag_data_received = &division_drag_data_received;
	dst_iface -> row_drop_possible = &division_row_drop_possible;
    }

    src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (payee_tree_model);
    if ( src_iface )
    {
	gtk_selection_add_target (payee_tree,
				  GDK_SELECTION_PRIMARY,
				  GDK_SELECTION_TYPE_ATOM,
				  1);
	src_iface -> drag_data_get = &payee_drag_data_get;
    }

    g_signal_connect ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( payee_tree ) ),
                        "changed",
                        G_CALLBACK ( metatree_selection_changed ),
                        payee_tree_model );

    /* création de la structure de sauvegarde de la position */
    payee_hold_position = g_malloc0 ( sizeof ( struct metatree_hold_position ) );

    gtk_widget_show_all ( frame );

    return ( onglet );
}


/**
 * Create a button bar allowing to act on the payee list.  Some of
 * these buttons are "linked" to the selection status of the payee
 * metatree.  That is, if nothing is selected, they will become
 * unsensitive.
 *
 * \return	A newly-allocated widget.
 */
GtkWidget *creation_barre_outils_tiers ( void )
{
    GtkWidget *toolbar;
    GtkToolItem *item;

    toolbar = gtk_toolbar_new ();

    /* New payee button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-new-payee-24.png", _("New payee"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Create a new payee") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( appui_sur_ajout_payee ),
                        payee_tree_model );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* delete button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-delete-24.png", _("Delete"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( payee_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Delete selected payee") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( supprimer_division ),
                        payee_tree );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* edit button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-edit-24.png", _("Edit"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( payee_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Edit selected payee") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( edit_payee ),
                        payee_tree );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Change view mode button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-select-color-24.png", _("View"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Change view mode") );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( popup_payee_view_mode_menu ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Manage payees button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-payees-manage-24.png", _("Manage payees"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Manage the payees") );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( payees_manage_payees ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Remove unused payees button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-delete-24.png", _("Remove unused payees"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Remove orphan payees") );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( payees_remove_unused_payees ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    metatree_set_linked_widgets_sensitive ( GTK_TREE_MODEL ( payee_tree_model ), FALSE, "selection" );

    return ( toolbar );
}


/**
 *
 *
 *
 */
void gsb_gui_payees_toolbar_set_style ( gint toolbar_style )
{
    gtk_toolbar_set_style ( GTK_TOOLBAR ( payee_toolbar ), toolbar_style );
}



/**
 * called by button delete unused payees
 * show a message to be sure and remove all the payees without transactions
 *
 * \param button	the button of the toolbar
 * \param null
 *
 * \return FALSE
 * */
void payees_remove_unused_payees ( void )
{
    gint result;

    result = question_yes_no ( _("This will remove all the payees wich are not used in any transactions.  "
                                 "Payees linked to an archived transactions will not be removed, even if not "
                                 "used outside the archive.\n\nAre you sure you want to do that?"),
                               _("Remove orphan payees"),
                               GTK_RESPONSE_CANCEL );

    if (result == TRUE)
    {
        gint nb_removed;
        gchar *tmpstr;

        nb_removed = gsb_data_payee_remove_unused ();
        if ( nb_removed > 0 )
        {
            payees_fill_list ();
            tmpstr = g_strdup_printf ( _("Removed %d payees."), nb_removed);
            gsb_file_set_modified ( TRUE );
        }
        else
        {
            tmpstr = g_strdup ( _("There is no payee to remove.") );
        }
        dialogue (tmpstr);
        g_free (tmpstr);
    }
}


/**
 * Fonction de callback d'affichage des tiers inutilisés
 *
 * \param
 * \param
 *
 * \return
 */
static void payee_menu_display_unused_payees_activate ( GtkWidget *item,
                        gpointer data )
{
    display_unused_payees = gtk_check_menu_item_get_active ( GTK_CHECK_MENU_ITEM ( item ) );

    payees_fill_list ();
}


/**
 * Popup a menu that allow changing the view mode of the category
 * metatree.
 *
 * \param button	Button that triggered the signal.
 *
 * \return		FALSE
 */
gboolean popup_payee_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *menu_item;
    gint nb_unused;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_menu_item_new_with_label ( _("Payee view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 0 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", payee_tree );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 2 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", payee_tree );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

    /* menu pour afficher les tiers inutilisés */
    if ( ( nb_unused = gsb_data_payee_get_unused_payees () ) == 0 )
    {
        menu_item = gtk_check_menu_item_new_with_label ( _("Display unused payees") );
        gtk_widget_set_sensitive ( menu_item, FALSE );
    }
    else
    {
        gchar *tmp_str;
        gchar *tmp_str_1;

        tmp_str = g_strdup_printf (" (%d)", nb_unused );
        tmp_str_1 = g_strconcat ( _("Display unused payees"), tmp_str, NULL );
        menu_item = gtk_check_menu_item_new_with_label ( tmp_str_1 );
        g_free ( tmp_str );
        g_free ( tmp_str_1 );
    }
    gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( menu_item ), display_unused_payees );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payee_menu_display_unused_payees_activate ),
                        NULL );

    gtk_widget_show_all ( menu );

#if GTK_CHECK_VERSION (3,22,0)
	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
#else
    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1,
		     gtk_get_current_event_time());
#endif
    return FALSE;
}



/**
 * fill the payee tree
 *
 * \param
 *
 * \return
 * */
void payees_fill_list ( void )
{
    void *empty_payee;
    gint payee_number;
    GSList *payee_list_tmp;
    GtkTreeIter iter_payee;
    GtkTreeSelection *selection;

    devel_debug (NULL);

    grisbi_win_status_bar_wait ( FALSE );

    /* on bloque la fonction pendant la mise à jour du model */
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( payee_tree ) );
    g_signal_handlers_block_by_func ( G_OBJECT ( selection ),
                        G_CALLBACK ( metatree_selection_changed ),
                        payee_tree_model );

    /* Dettach the model so that insertion will be much faster */
    g_object_ref ( G_OBJECT(payee_tree_model) );
    gtk_tree_view_set_model ( GTK_TREE_VIEW (payee_tree), NULL );

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (payee_tree_model) );

    /* Compute payee balances. */
    gsb_data_payee_update_counters ();

    /* add the virtually unused payee to the top of the list */
    empty_payee = gsb_data_payee_get_empty_payee ( );
    payee_number = gsb_data_payee_get_no_payee ( empty_payee );
    gtk_tree_store_append (GTK_TREE_STORE (payee_tree_model), &iter_payee, NULL);
    fill_division_row ( GTK_TREE_MODEL(payee_tree_model),
                payee_get_metatree_interface ( ),
                &iter_payee, payee_number );

    /** Then, populate tree with payee. */
    payee_list_tmp = g_slist_copy ( gsb_data_payee_get_payees_list () );

    payee_list_tmp = g_slist_sort ( payee_list_tmp, (GCompareFunc) gsb_data_payee_compare_payees_by_name);

    while ( payee_list_tmp )
    {
        payee_number = gsb_data_payee_get_no_payee ( payee_list_tmp->data);

        if ( display_unused_payees || gsb_data_payee_get_nb_transactions ( payee_number ) )
        {
            gtk_tree_store_append ( GTK_TREE_STORE ( payee_tree_model ), &iter_payee, NULL );
            fill_division_row ( GTK_TREE_MODEL ( payee_tree_model ),
                        payee_get_metatree_interface (),
                        &iter_payee,
                        payee_number );
        }
        payee_list_tmp = payee_list_tmp -> next;
    }

    g_slist_free ( payee_list_tmp );

    /* Reattach the model */
    gtk_tree_view_set_model (GTK_TREE_VIEW (payee_tree),
			     GTK_TREE_MODEL (payee_tree_model));

    /* on débloque la fonction de callback */
    g_signal_handlers_unblock_by_func ( G_OBJECT ( selection ),
                        G_CALLBACK ( metatree_selection_changed ),
                        payee_tree_model );

    /* replace le curseur sur la division, sub_division ou opération initiale */
    if ( payee_hold_position -> path )
    {
        if ( payee_hold_position -> expand )
        {
            GtkTreePath *ancestor;

            ancestor = gtk_tree_path_copy ( payee_hold_position -> path );
            gtk_tree_path_up ( ancestor );
            gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( payee_tree ), ancestor );
            gtk_tree_path_free (ancestor );
        }

        /* on colorise les lignes du tree_view */
        utils_set_tree_view_background_color ( payee_tree, META_TREE_BACKGROUND_COLOR );
        gtk_tree_selection_select_path ( selection, payee_hold_position -> path );
        gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( payee_tree ),
                        payee_hold_position -> path,
                        NULL, TRUE, 0.5, 0.5 );
    }
    else
    {
        gchar *title;

        /* on colorise les lignes du tree_view */
        utils_set_tree_view_background_color ( payee_tree, META_TREE_BACKGROUND_COLOR );
        /* on fixe le titre et le suffixe de la barre d'information */
	    title = g_strdup(_("Payees"));
        grisbi_win_headings_update_title ( title );
        g_free ( title );
        grisbi_win_headings_update_suffix ( "" );
    }

    g_object_unref ( G_OBJECT ( payee_tree_model ) );

    grisbi_win_status_bar_stop_wait ( FALSE );
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
gboolean payee_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
                        GtkSelectionData * selection_data )
{
    if ( path )
    {
	gtk_tree_set_row_drag_data (selection_data,
				    GTK_TREE_MODEL(payee_tree_model), path);
    }

    return FALSE;
}


/**
 *
 *
 */
gboolean edit_payee ( GtkTreeView * view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry_name;
    GtkWidget *entry_description, *hbox, *scrolled_window;
    GtkTreeSelection * selection;
    GtkTreeModel * model = NULL;
    GtkTreeIter iter;
    GtkTreePath *path;
    gint no_division = -1;
    gint payee_number = 0;
    gchar * title;
    gchar * old_payee;
    GtkTreeIter *div_iter;
    MetatreeInterface *payee_interface;

    devel_debug (NULL);

    selection = gtk_tree_view_get_selection ( view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
    gtk_tree_model_get ( model, &iter,
                        META_TREE_POINTER_COLUMN, &payee_number,
                        META_TREE_NO_DIV_COLUMN, &no_division,
                        -1 );
    }

    if ( !selection || no_division <= 0 )
        return FALSE;

    old_payee = g_strdup ( gsb_data_payee_get_name ( payee_number, TRUE ) );
    title = g_strdup_printf ( _("Properties for %s"), old_payee );

    dialog = gtk_dialog_new_with_buttons ( title,
                        GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
                        GTK_DIALOG_MODAL,
                        "gtk-cancel", GTK_RESPONSE_NO,
                        "gtk-apply", GTK_RESPONSE_OK,
                        NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_pack_start ( GTK_BOX ( dialog_get_content_area ( dialog ) ), hbox, FALSE, FALSE, 0 );
    paddingbox = new_paddingbox_with_title ( hbox, TRUE, title );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(paddingbox), 6 );

    table = gtk_grid_new ();
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 6 );
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);

    /* Name entry */
    label = gtk_label_new ( _("Name"));
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0.0, 0.5 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

    entry_name = gtk_entry_new ();
    gtk_entry_set_text ( GTK_ENTRY ( entry_name ),
                        gsb_data_payee_get_name(payee_number,
                        TRUE));
    gtk_widget_set_size_request ( entry_name, 400, -1 );
    gtk_grid_attach (GTK_GRID (table), entry_name, 1, 0, 1, 1);

    /* Description entry */
    label = gtk_label_new ( _("Description"));
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0.0, 0.5 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

    entry_description = gsb_editable_text_view_new (gsb_data_payee_get_description (payee_number));
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window),
                        GTK_SHADOW_IN );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), entry_description );
    gtk_grid_attach (GTK_GRID (table), scrolled_window, 1, 1, 1, 1);

    gtk_widget_show_all ( dialog );

    while ( 1 )
    {
        if ( gtk_dialog_run ( GTK_DIALOG(dialog) ) != GTK_RESPONSE_OK )
        {
            sortie_edit_payee = TRUE;
            gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
            g_free ( title );
            g_free ( old_payee );

            return FALSE;
        }

        if ( ! gsb_data_payee_get_number_by_name ( gtk_entry_get_text (
         GTK_ENTRY ( entry_name ) ), FALSE )
         ||
         gsb_data_payee_get_number_by_name ( gtk_entry_get_text (
         GTK_ENTRY ( entry_name ) ), FALSE ) == payee_number )
        {
            gsb_data_payee_set_name ( payee_number,
                        gtk_entry_get_text ( GTK_ENTRY (entry_name)));
            break;
        }
        else
        {
            gchar *message;

            message = g_strdup_printf ( _("You tried to rename current payee to '%s' "
                            "but this payee already exists.  Please "
                            "choose another name."),
                            gtk_entry_get_text ( GTK_ENTRY ( entry_name ) ) );
            dialogue_warning_hint ( message, _("Payee already exists") );
            g_free ( message );
        }
    }

    /* get the description */
    gsb_data_payee_set_description ( payee_number,
                        gsb_editable_text_view_get_content ( entry_description ));

    gtk_widget_destroy ( dialog );

    payee_interface = payee_get_metatree_interface ( );
    div_iter = get_iter_from_div ( model, payee_number, 0 );
    fill_division_row ( model, payee_interface,
                        div_iter, payee_number );
    payee_tree_update_transactions ( model, payee_interface,
                        div_iter, payee_number, old_payee );

    /* et on centre l'affichage dessus */
    div_iter = get_iter_from_div ( model, payee_number, 0 );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), div_iter );
    gtk_tree_selection_select_iter (selection, div_iter);
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (payee_tree), path, NULL, TRUE, 0.5, 0.5 );
    gtk_tree_path_free ( path );

    gtk_widget_grab_focus ( GTK_WIDGET ( view ) );
    gtk_tree_iter_free (div_iter);
    g_free ( title );
    g_free ( old_payee );

    /* update the transactions list */
    transaction_list_update_element (ELEMENT_PARTY);
    gsb_file_set_modified ( TRUE );
    return FALSE;
}


void payee_tree_update_transactions ( GtkTreeModel * model,
                        MetatreeInterface * iface, GtkTreeIter * iter,
                        gint division, gchar * old_payee )
{
    GtkTreeIter child_iter;
    gchar *name;
    gint number_transactions;
    gint transaction_number;
    gint i = 0;

    number_transactions = iface -> div_nb_transactions (division);

    for ( i = 0; i < number_transactions; i++ )
    {
        gtk_tree_model_iter_nth_child ( model, &child_iter, iter, i );
        gtk_tree_model_get ( model, &child_iter, META_TREE_TEXT_COLUMN, &name, -1 );
        if ( !name )
            return;
        gtk_tree_model_get ( model, &child_iter, META_TREE_NO_TRANSACTION_COLUMN,
                        &transaction_number, -1 );
        name = gsb_string_remplace_string ( name, old_payee, (gchar *)
                        gsb_data_payee_get_name ( division, FALSE ) );
        gtk_tree_store_set ( GTK_TREE_STORE(model), &child_iter,
                        META_TREE_TEXT_COLUMN, name, -1 );
    }
}


/**
 * function to create and editing a new payee.
 *
 * \param the model for the division
 */
void appui_sur_ajout_payee ( GtkTreeModel * model )
{
    metatree_new_division ( model );
    sortie_edit_payee = FALSE;
    edit_payee ( GTK_TREE_VIEW ( payee_tree ) );
    if ( sortie_edit_payee )
        supprimer_division ( GTK_TREE_VIEW ( payee_tree ) );
    sortie_edit_payee = FALSE;
}


/**
 * fonction pour sauvegarder le chemin du dernier tiers sélectionné.
 *
 * \param path
 */
gboolean payees_hold_position_set_path ( GtkTreePath *path )
{
    payee_hold_position -> path = gtk_tree_path_copy ( path );

    return TRUE;
}
/**
 * sauvegarde l'attribut expand.
 *
 * \param expand
 */
gboolean payees_hold_position_set_expand ( gboolean expand )
{
    payee_hold_position -> expand = expand;

    return TRUE;
}
/* ******************************************************************************/
/* assistant de gestion des tiers :                                             */
/* permet de modifier en masse le nom d'un tiers contenant une chaine           */
/* par une autre chaine                                                         */
/* **************************************************************************** */

/**
 * assistant pour nettoyer les tiers
 *
 *
 */
void payees_manage_payees ( void )
{
    GtkWidget *assistant;
    GtkResponseType return_value;

    devel_debug ( "PAGE 1" );

    assistant = gsb_assistant_new ( _("Manage the payees"),
                        _("This wizard will help you to simplify the list of payees.\n\n"
                        "Warning the changes you will make be irreparable.\n\n"
                        "It is better to make a backup of your Grisbi file if you have not yet done. "),
                        "gsb-payees-32.png",
                        NULL );

    gsb_assistant_add_page ( assistant,
                        gsb_assistant_payees_page_2 ( assistant ),
                        PAYEES_ASSISTANT_PAGE_2,
                        PAYEES_ASSISTANT_INTRO,
                        PAYEES_ASSISTANT_PAGE_3,
                            G_CALLBACK ( gsb_assistant_payees_enter_page_2 ) );

    gsb_assistant_add_page ( assistant,
                        gsb_assistant_payees_page_3 ( assistant ),
                    PAYEES_ASSISTANT_PAGE_3,
                    PAYEES_ASSISTANT_PAGE_2,
                    PAYEES_ASSISTANT_PAGE_FINISH,
                    G_CALLBACK ( gsb_assistant_payees_enter_page_3 ) );

    gsb_assistant_add_page ( assistant,
                    gsb_assistant_payees_page_finish ( assistant ),
                    PAYEES_ASSISTANT_PAGE_FINISH,
                    PAYEES_ASSISTANT_PAGE_3,
                    0,
                    G_CALLBACK ( gsb_assistant_payees_enter_page_finish) );

    return_value = gsb_assistant_run ( assistant );

    if ( return_value == GTK_RESPONSE_APPLY )
    {
        GSList *sup_payees;
        GSList *tmp_list;
        GtkTreeSelection *selection;
        GtkTreeIter *iter;
        GtkTreePath *path = NULL;
        GtkComboFix *combo;
        const gchar *str_cherche;
        gchar *tmpstr;
        gint new_payee_number = 0;
        gint nb_removed;
        gboolean save_notes = FALSE;
        gboolean extract_num = FALSE;

        /* on remplace les anciens tiers par le nouveau et on sauvegarde si nécessaire */
        grisbi_win_status_bar_wait ( TRUE );

        sup_payees = g_object_get_data ( G_OBJECT (assistant), "sup_payees" );
        if ( (nb_removed = g_slist_length ( sup_payees ) ) == 1 )
            new_payee_number = GPOINTER_TO_INT ( sup_payees -> data );
        else
            new_payee_number = gsb_data_payee_get_number_by_name (
                        gtk_entry_get_text ( g_object_get_data (
                        G_OBJECT (assistant), "new_payee") ), TRUE );

        /* on sauvegarde la chaine de recherche */
        combo = g_object_get_data ( G_OBJECT (assistant), "payee");
        str_cherche = gtk_combofix_get_text ( combo );
        gsb_data_payee_set_search_string ( new_payee_number, str_cherche );
        extract_num = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (
                        g_object_get_data ( G_OBJECT (assistant),
                        "check_option_1" ) ) );
        save_notes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (
                        g_object_get_data ( G_OBJECT (assistant),
                        "check_option_2" ) ) );

        /* on ajoute la nouvelle association à la liste des assoc */
		gsb_import_associations_add_assoc (new_payee_number, str_cherche);

		if ( nb_removed > 1 )
        {
            tmp_list = gsb_data_transaction_get_complete_transactions_list ( );
            while ( tmp_list )
            {
                gint transaction_number;

                transaction_number = gsb_data_transaction_get_transaction_number (
                        tmp_list -> data );
                gsb_assistant_payees_modifie_operations ( sup_payees,
                        transaction_number,
                        new_payee_number,
                        save_notes,
                        extract_num,
                        TRUE );
                tmp_list = tmp_list -> next;
            }
            transaction_list_update_element (ELEMENT_PARTY);

            /* on fait la même chose pour les opérations planifiées */
            tmp_list = gsb_data_scheduled_get_scheduled_list ( );

            while ( tmp_list )
            {
                gint scheduled_number;

                scheduled_number = gsb_data_scheduled_get_scheduled_number (
                        tmp_list -> data );
                gsb_assistant_payees_modifie_operations ( sup_payees,
                        scheduled_number,
                        new_payee_number,
                        save_notes,
                        extract_num,
                        FALSE );
                tmp_list = tmp_list -> next;
            }
            /* on efface les tiers inutilisés */
            nb_removed = gsb_data_payee_remove_unused ();
            payees_fill_list ();
            if ( nb_removed == 1 )
            {
                tmpstr = g_strdup_printf ( _("One payee was replaced with a new one."));
            }
            else
            {
                tmpstr = g_strdup_printf ( _("%d payees were replaced with a new one."),
                            nb_removed);
            }
            dialogue (tmpstr);
            g_free (tmpstr);
        }
        else
        {
            gsb_data_payee_set_name ( new_payee_number,
                        gtk_entry_get_text ( g_object_get_data (
                        G_OBJECT (assistant), "new_payee") ) );
            payees_fill_list ();
        }

        gsb_file_set_modified ( TRUE );

        /* On sélectionne le nouveau tiers */
        iter = get_iter_from_div ( GTK_TREE_MODEL ( payee_tree_model ), new_payee_number, 0 );
        path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( payee_tree_model ), iter );

        /* et on centre l'affichage dessus */
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (payee_tree) );
        gtk_tree_selection_select_iter (selection, iter);
        gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (payee_tree), path,
                        NULL, TRUE, 0.5, 0.5 );
        gtk_tree_path_free ( path );
        grisbi_win_status_bar_stop_wait ( TRUE );
    }

    g_slist_free ( g_object_get_data ( G_OBJECT (assistant), "sup_payees" ) );
    gtk_widget_destroy (assistant);
}


/**
 *
 *
 */
static GtkWidget *gsb_assistant_payees_page_2 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *paddingbox;
    GtkWidget *check_option_1;
    GtkWidget *check_option_2;
    gchar *texte;

    devel_debug ( "PAGE 2" );

    page = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER( page ), BOX_BORDER_WIDTH );

    paddingbox = new_paddingbox_with_title ( page, TRUE, _("Choose a payee"));

    texte = g_strdup ( _("Select one payee in the list that you modify to "
                        "serve as the basis for your research.\n"
                        "You can use % as wildcard. Templates availables:\n"
                        "   %string, string%\n"
                        "   %string%, string_1%string_2") );
	label = gtk_label_new ( texte );
	utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0);
	gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
	g_free ( texte );
	gtk_box_pack_start ( GTK_BOX(paddingbox), label, FALSE, FALSE, 6 );

    entry = gtk_combofix_new ( gsb_data_payee_get_name_and_report_list ( ) );
    gtk_combofix_set_force_text ( GTK_COMBOFIX (entry),FALSE );
    gtk_combofix_set_max_items ( GTK_COMBOFIX (entry),
                        etat.combofix_max_item );
    gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (entry),
                        etat.combofix_case_sensitive );
    gtk_box_pack_start ( GTK_BOX(paddingbox), entry, FALSE, FALSE, 6 );
    g_object_set_data ( G_OBJECT (assistant), "payee", entry );
    paddingbox = new_paddingbox_with_title ( page, TRUE, _("Enter the new payee"));

    texte = g_strdup ( _("Enter the name of the new payee") );
    label = gtk_label_new ( texte );
	utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0);
	gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    g_free ( texte );
    gtk_box_pack_start ( GTK_BOX(paddingbox), label, FALSE, FALSE, 6 );

    entry = gtk_entry_new ( );
    g_signal_connect ( entry,
                        "changed",
                        G_CALLBACK (gsb_assistant_payees_entry_changed),
                        assistant );
    gtk_box_pack_start ( GTK_BOX(paddingbox), entry, FALSE, FALSE, 6 );
    g_object_set_data ( G_OBJECT (assistant), "new_payee", entry );

    paddingbox = new_paddingbox_with_title ( page, TRUE, _("Options"));
    check_option_1 = gtk_check_button_new_with_label (
                        _("Extracting a number and save it in the field No Cheque/Virement") );

    gtk_box_pack_start ( GTK_BOX(paddingbox), check_option_1, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT (assistant), "check_option_1", check_option_1 );

    check_option_2 = gtk_check_button_new_with_label (
                        _("Save the payees in the notes") );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (check_option_2),
				       TRUE );
    gtk_box_pack_start ( GTK_BOX(paddingbox), check_option_2, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT (assistant), "check_option_2", check_option_2 );

    gtk_widget_show_all (page);
    return page;
}

/**
 *
 *
 */
static GtkWidget *gsb_assistant_payees_page_3 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *label;
    GtkWidget *paddingbox;
    GtkWidget *sw;
    GtkWidget *treeview;
    GtkWidget *table;
    GtkWidget *button;
    GtkListStore *list_store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    devel_debug ( "PAGE 3" );

    page = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER( page ), BOX_BORDER_WIDTH );

    paddingbox = new_paddingbox_with_title ( page, TRUE,
                        _("List of payees who will be modified"));

    label = gtk_label_new ( "" );
	utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0);
	gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX (paddingbox), label, FALSE, FALSE, 6 );
    g_object_set_data ( G_OBJECT (assistant), "payee_search_label", label );

    label = gtk_label_new ( "" );
	utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0);
	gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX (paddingbox), label, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT (assistant), "new_payee_label", label );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                        GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_NEVER,
                        GTK_POLICY_ALWAYS);

    /* create the model */
    list_store = gtk_list_store_new ( N_COLUMNS, G_TYPE_BOOLEAN,
                        G_TYPE_UINT, G_TYPE_STRING );

    /* create the treeview */
    treeview = gtk_tree_view_new_with_model (
                        GTK_TREE_MODEL (list_store) );
	gtk_widget_set_name (treeview, "tree_view");
    g_object_unref (list_store);

    gtk_widget_set_size_request ( treeview, -1, 230 );
    gtk_container_add (GTK_CONTAINER (sw), treeview);
    gtk_box_pack_start ( GTK_BOX (paddingbox), sw, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT (assistant), "treeview", treeview );

    /* select payee */
    cell = gtk_cell_renderer_toggle_new ( );
    g_signal_connect ( cell,
                        "toggled",
                        G_CALLBACK (gsb_assistant_payees_toggled),
                        assistant );
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_object_set (cell, "xalign", 0.5, NULL);

    column = gtk_tree_view_column_new_with_attributes ( _("Select"),
						     cell,
						     "active", COLUMN_BOOLEAN,
						     NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview), column);

    /* payee_number */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ( _("Payee number"),
						     cell,
						     "text",
						     COLUMN_INT,
						     NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_INT);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (treeview), column );

    /* payee name */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Payee name"),
						     cell,
						     "text",
						     COLUMN_STRING,
						     NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_STRING);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (treeview), column);

    /* ajoute le nombre de tiers total */
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);

    gtk_box_pack_start ( GTK_BOX (page), table, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Total number of payees: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    label = gtk_label_new ( "   0" );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 1 );
    gtk_grid_attach (GTK_GRID (table), label, 1, 0, 1, 1);
    g_object_set_data ( G_OBJECT (assistant), "nbre_tiers_total", label );

    /* ajoute le nombre de tiers sélectionnés */
    label = gtk_label_new ( _("Number of selected payees: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0 );
    gtk_grid_attach (GTK_GRID (table), label, 2, 0, 1, 1);
    label = gtk_label_new ( "   0" );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 1 );
    gtk_grid_attach (GTK_GRID (table), label, 3, 0, 1, 1);
    g_object_set_data ( G_OBJECT (assistant), "nbre_tiers_select", label );

    /* ajoute le bouton (dé)sélectionne tout */
    button = gtk_button_new_with_label ( _("Unselect all") );
    g_signal_connect ( button,
                        "clicked",
                        G_CALLBACK (gsb_assistant_payees_clicked),
                        assistant );
	gtk_widget_set_size_request ( button, 60, -1 );
    gtk_grid_attach (GTK_GRID (table), button, 4, 0, 1, 1);
    g_object_set_data ( G_OBJECT (assistant), "button", button );

    gtk_widget_show_all (page);
    return page;
}


/**
 *
 *
 */
static GtkWidget *gsb_assistant_payees_page_finish ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *label;

    devel_debug ( "PAGE FINISH" );

    page = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER( page ), BOX_BORDER_WIDTH );

    label = gtk_label_new ( NULL );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0.0, 0.0 );
    gtk_box_pack_start ( GTK_BOX (page), label, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT (assistant), "finish_label", label );

    gtk_widget_show_all (page);
    return page;
}


/**
 *
 *
 */
static gboolean gsb_assistant_payees_enter_page_2 ( GtkWidget *assistant )
{
    GtkWidget *entry;

    devel_debug ("Enter page 2");

    entry = g_object_get_data ( G_OBJECT (assistant), "new_payee" );
    gtk_entry_set_text ( GTK_ENTRY ( entry), "" );

    gsb_assistant_change_button_next ( assistant,
				       "gtk-go-forward", GTK_RESPONSE_YES );
    gsb_assistant_sensitive_button_next ( assistant,FALSE );

    return FALSE;
}


/**
 *
 *
 */
static gboolean gsb_assistant_payees_enter_page_3 ( GtkWidget *assistant )
{
    GtkWidget *payee_search_label;
    GtkWidget *new_payee_label;
    GtkTreeView *treeview;
    GtkWidget *label;
    GtkListStore *list_store;
    GtkTreeIter iter;
    GSList *payee_list_tmp;
    GSList *sup_payees = NULL;
    gchar *str;
    const gchar *str_cherche;
    const gchar *new_tiers;
    gint i = 0;

    devel_debug ("Enter page 3");

    str_cherche = gtk_combofix_get_text ( g_object_get_data (
                        G_OBJECT (assistant), "payee") );

    str = g_strdup_printf ( _("Payee sought: %s"), str_cherche );
    payee_search_label = g_object_get_data (
                        G_OBJECT (assistant), "payee_search_label" );
    gtk_label_set_text ( GTK_LABEL (payee_search_label), str );
    g_free ( str );

    new_tiers = gtk_entry_get_text( g_object_get_data (
                        G_OBJECT (assistant), "new_payee") );
    str = g_strdup_printf ( _("New payee: %s"), new_tiers );
    new_payee_label = g_object_get_data (
                        G_OBJECT (assistant), "new_payee_label" );
    gtk_label_set_text ( GTK_LABEL (new_payee_label), str );
    g_free ( str );

    payee_list_tmp = gsb_data_payee_get_payees_list ();
    treeview = g_object_get_data ( G_OBJECT (assistant), "treeview" );
    list_store = GTK_LIST_STORE ( gtk_tree_view_get_model ( treeview ) );
    gtk_list_store_clear ( list_store );

    while ( payee_list_tmp )
    {
        gint payee_number;
        const gchar *tmpstr;

        payee_number = gsb_data_payee_get_no_payee (payee_list_tmp -> data);
        tmpstr = gsb_data_payee_get_name ( payee_number, FALSE );
        if ( gsb_string_is_trouve ( tmpstr, str_cherche ) )
        {
            if (!g_slist_find (sup_payees, GINT_TO_POINTER (payee_number)))
            {
                sup_payees = g_slist_append ( sup_payees,
                        GINT_TO_POINTER (payee_number));
            }
            gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
            gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                        COLUMN_BOOLEAN,  TRUE,
                        COLUMN_INT, payee_number,
                        COLUMN_STRING, tmpstr,
                        -1);
            i++;
        }
        payee_list_tmp = payee_list_tmp -> next;
    }

    if ( i > 0 )
    {
        label = g_object_get_data ( G_OBJECT (assistant), "nbre_tiers_total" );
        str = g_strdup_printf ("%d", i );
        gtk_label_set_text ( GTK_LABEL (label), str );
        g_free ( str );

        label = g_object_get_data ( G_OBJECT (assistant), "nbre_tiers_select" );
        str = g_strdup_printf ("%d", i );
        gtk_label_set_text ( GTK_LABEL (label), str );
        g_free ( str );

        g_slist_free ( g_object_get_data ( G_OBJECT (assistant), "sup_payees" ) );
        g_object_set_data ( G_OBJECT (assistant), "sup_payees", sup_payees );
    }
    return FALSE;
}


/**
 *
 *
 */
static gboolean gsb_assistant_payees_enter_page_finish ( GtkWidget *assistant )
{
    GtkLabel *label;
    GtkComboFix *combo;
    GtkEntry *entry;
    GSList *sup_payees;
    gchar *tmpstr;
    const gchar *str_cherche;
    gchar *str_replace_wildcard;

    devel_debug ("Enter page finish");
    sup_payees = g_object_get_data ( G_OBJECT (assistant), "sup_payees" );
    combo = g_object_get_data ( G_OBJECT (assistant), "payee");
    str_cherche = gtk_combofix_get_text ( combo );
    entry = g_object_get_data ( G_OBJECT (assistant), "new_payee");
    str_replace_wildcard = gsb_string_remplace_joker ( str_cherche, "..." );

    if ( g_slist_length (sup_payees) == 1 )
    {
    tmpstr = g_strdup_printf (
                        _("You are about to replace one payee which name contain %s by %s\n\n"
                        "Are you sure?"),
                        str_replace_wildcard,
                        gtk_entry_get_text ( entry) );
    }
    else
    {
    tmpstr = g_markup_printf_escaped (
                        _("You are about to replace %d payees whose names contain %s by %s\n\n"
                        "Are you sure?"),
                        g_slist_length (sup_payees),
                        str_replace_wildcard,
                        gtk_entry_get_text ( entry) );
    }
    label = g_object_get_data ( G_OBJECT (assistant), "finish_label" );
    gtk_label_set_markup ( label, tmpstr );

    g_free ( str_replace_wildcard );
    g_free ( tmpstr);

    return FALSE;
}


/**
 *
 *
 */
void gsb_assistant_payees_clicked ( GtkButton *button, GtkWidget *assistant )
{
    GtkTreeView *treeview;
    GtkLabel *label;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *sup_payees;
    gchar *tmpstr;
    gboolean select, valid = FALSE;

    treeview = g_object_get_data ( G_OBJECT (assistant), "treeview" );
    sup_payees = g_object_get_data ( G_OBJECT (assistant), "sup_payees" );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (treeview) );
    tmpstr = g_strdup ( gtk_button_get_label ( button ) );

    if ( my_strcmp ( tmpstr, _("Select all") ) == 0 )
    {
        gtk_button_set_label ( button, _("Unselect all") );
        valid = gtk_tree_model_get_iter_first (model, &iter);

        while (valid)
        {
            gint payee_number;

            /* remplit la liste et coche les cases */
            gtk_tree_model_get ( model, &iter, COLUMN_INT, &payee_number, -1 );
            sup_payees = g_slist_append ( sup_payees,
                        GINT_TO_POINTER (payee_number) );
            gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        COLUMN_BOOLEAN,  TRUE , -1);
            valid = gtk_tree_model_iter_next ( model, &iter );
        }
    }
    else
    {
        gtk_button_set_label ( button, _("Select all") );
        /* décoche les cases */
        select = FALSE;
        gtk_tree_model_foreach ( model, gsb_assistant_payees_select_func,
                        GINT_TO_POINTER ( select ) );
        g_slist_free ( sup_payees );
        sup_payees = NULL;
    }

    g_object_set_data ( G_OBJECT (assistant), "sup_payees", sup_payees );
    label = g_object_get_data ( G_OBJECT (assistant), "nbre_tiers_select" );
    gtk_label_set_text ( label, g_strdup_printf ("%d",
                        g_slist_length (sup_payees)) );

    /* gestion du bouton next */
    if ( g_slist_length (sup_payees) == 0 )
        gsb_assistant_sensitive_button_next ( assistant, FALSE );
    else
        gsb_assistant_sensitive_button_next ( assistant, TRUE );
    g_free ( tmpstr );
}


/**
 *
 *
 */
gboolean gsb_assistant_payees_select_func (GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data)
{
    gtk_list_store_set (GTK_LIST_STORE (model), iter,
                        COLUMN_BOOLEAN,  GPOINTER_TO_INT ( data ),
                        -1);
    return FALSE;
}


/**
 *
 *
 */
void gsb_assistant_payees_toggled ( GtkCellRendererToggle *cell,
                        gchar *path_str,
                        GtkWidget *assistant )
{
    GtkTreeView *treeview;
    GtkLabel *label;
    GtkTreeModel *model;
    GtkTreeIter  iter;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GSList *sup_payees;
    gint payee_number;
    gboolean fixed;

    /* get model, iter and data */
    treeview = g_object_get_data ( G_OBJECT (assistant), "treeview" );
    model = gtk_tree_view_get_model ( treeview );
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, COLUMN_BOOLEAN, &fixed,
                        COLUMN_INT, &payee_number, -1);

    /* do something with the value */
    sup_payees = g_object_get_data ( G_OBJECT (assistant), "sup_payees" );
    if ( g_slist_find (sup_payees, GINT_TO_POINTER (payee_number)) && fixed )
        sup_payees = g_slist_delete_link ( sup_payees,
                        g_slist_find ( sup_payees,
                        GINT_TO_POINTER (payee_number) ) );
    else
        sup_payees = g_slist_prepend ( sup_payees,
                        GINT_TO_POINTER (payee_number) );

    g_object_set_data ( G_OBJECT (assistant), "sup_payees", sup_payees );
    label = g_object_get_data ( G_OBJECT (assistant), "nbre_tiers_select" );
    gtk_label_set_text ( label, g_strdup_printf ("%d",
                        g_slist_length (sup_payees)) );
    fixed ^= 1;

    /* set new value */
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_BOOLEAN, fixed, -1);

    /* clean up */
    gtk_tree_path_free (path);

    /* gestion du bouton next */
    if ( g_slist_length (sup_payees) == 0 )
        gsb_assistant_sensitive_button_next ( assistant, FALSE );
    else
        gsb_assistant_sensitive_button_next ( assistant, TRUE );
}


/**
 *
 *
 */
void gsb_assistant_payees_entry_changed ( GtkEditable *editable,
                        GtkWidget *assistant )
{
    GtkWidget *entry;
    const gchar *str_cherche;

    entry = g_object_get_data ( G_OBJECT (assistant), "payee" );
    str_cherche = gtk_combofix_get_text ( GTK_COMBOFIX (entry) );
    switch ( gsb_assistant_payees_valide_model_recherche ( str_cherche ) )
	{
        case 0:
        dialogue_warning ( _("You must choose a payee for searching.") );
        break;
        case -1:
        dialogue_warning (
                        _("Beware you don't use one of models for the research.") );
        break;
        case 1:
        gsb_assistant_sensitive_button_next ( assistant,TRUE );
        return;
        break;
    }
    g_signal_handlers_block_by_func ( G_OBJECT (editable),
                        G_CALLBACK (gsb_assistant_payees_entry_changed),
                        assistant );
    gtk_editable_delete_text ( editable, 0, -1 );
    gtk_widget_grab_focus ( GTK_COMBOFIX (entry) -> entry );
    g_signal_handlers_unblock_by_func ( G_OBJECT (editable),
                        G_CALLBACK (gsb_assistant_payees_entry_changed),
                        assistant );
}


/**
 * Valide les modèles possibles pour la recherche ( toto% %toto %toto% toto%tata)
 *
 \param needle chaine de recherche
 *
 * \return 1 si correct 0 si vide -1 si non conforme au modèle
 */
gint gsb_assistant_payees_valide_model_recherche ( const gchar *needle )
{
    gchar **tab_str;
    gboolean is_prefix = FALSE, is_suffix = FALSE;

    if ( needle && strlen ( needle ) )
    {
        if ( g_str_has_prefix ( needle, "%" )||
                        g_str_has_prefix ( needle, "*") )
            is_prefix = TRUE;

        if ( g_str_has_suffix ( needle, "%" ) ||
                        g_str_has_suffix ( needle, "*") )
            is_suffix = TRUE;

        if ( is_prefix && is_suffix )
        {
            tab_str = g_strsplit_set ( needle, "%*", 0 );
            if ( g_strv_length ( tab_str ) == 3 )
                return 1;
            else
                return -1;
        }
        else if ( is_prefix && ! is_suffix )
            return 1;
        else if ( is_suffix && ! is_prefix )
            return 1;
        else if ( ! is_prefix && ! is_suffix )
        {
            tab_str = g_strsplit_set ( needle, "%*", 0 );
            if ( g_strv_length ( tab_str ) == 2 )
                return 1;
            else if ( gsb_data_payee_get_number_by_name ( needle, FALSE ) )
                return 1;
            else
                return -1;
        }
        else
            return -1;
    }
    else
        return 0;
}


/**
 *
 *
 */
void gsb_assistant_payees_modifie_operations ( GSList *sup_payees,
                        gint transaction_number,
                        gint new_payee_number,
                        gboolean save_notes,
                        gboolean extract_num,
                        gboolean is_transaction )
{
    gchar *tmpstr;
    gint payee_number;
    gchar *nombre;
    gboolean question = TRUE;
    struct ConditionalMessage overwrite_payee;


    payee_number = gsb_data_mix_get_party_number ( transaction_number, is_transaction );
    if ( g_slist_find ( sup_payees, GINT_TO_POINTER ( payee_number ) ) )
    {
        gsb_data_mix_set_party_number ( transaction_number,
                new_payee_number, is_transaction );
        if ( save_notes )
        {
            tmpstr = g_strdup ( gsb_data_mix_get_notes (
                        transaction_number, is_transaction ) );
            if ( tmpstr && strlen ( tmpstr ) )
            {

                overwrite_payee.name = g_strdup ( "crush-existing-note" );
                overwrite_payee.hint = g_strdup ( _("Warning you will crush the existing note.") );
                overwrite_payee.hidden = FALSE;
                overwrite_payee.default_answer = FALSE;
                overwrite_payee.message = g_strdup_printf (
                    _("Do you want overwrite the existing note.\n\n"
                    "If you answer YES, the existing note will be replaced by %s."),
                    gsb_data_payee_get_name ( payee_number, TRUE ) );

                if ( question_conditional_yes_no_with_struct (
                 &overwrite_payee ) == FALSE )
                    question = FALSE;
                g_free ( (gchar*) overwrite_payee.name );
                g_free ( (gchar*) overwrite_payee.hint );
                g_free ( (gchar*) overwrite_payee.message );
                g_free ( tmpstr );
            }
            else
                question = TRUE;
            if ( question && my_strcmp ( (gchar *) gsb_data_payee_get_name (
                payee_number, TRUE ),
                (gchar *) gsb_data_payee_get_name (
                new_payee_number, TRUE ) ) != 0 )
            gsb_data_mix_set_notes ( transaction_number,
                gsb_data_payee_get_name ( payee_number, TRUE ), is_transaction );
        }
        if ( extract_num )
        {
            nombre = gsb_string_extract_int (
                gsb_data_payee_get_name ( payee_number, FALSE ) );
            gsb_data_mix_set_method_of_payment_content (
                transaction_number, nombre, is_transaction );
            g_free ( nombre );
        }
    }
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean payee_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null )
{
    if ( ev -> type == GDK_2BUTTON_PRESS )
    {
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        enum meta_tree_row_type type_division;

        type_division = metatree_get_row_type_from_tree_view ( tree_view );
        if ( type_division == META_TREE_TRANSACTION )
            return FALSE;

        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
        if ( selection && gtk_tree_selection_get_selected (selection, &model, &iter ) )
            path = gtk_tree_model_get_path  ( model, &iter);

        if ( conf.metatree_action_2button_press == 1 )
        {
            edit_payee ( GTK_TREE_VIEW ( tree_view ) );

            gtk_tree_path_free ( path );
            return TRUE;
        }
        else
        {
            if ( gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( tree_view ), path ) )
                gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
            else
                gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );

            gtk_tree_path_free ( path );
            return FALSE;
        }

        return TRUE;
    }
    else
        return FALSE;
}


/**
 *
 *
 *
 */
GtkTreeStore *payees_get_tree_store ( void )
{
    return payee_tree_model;
}


/**
 *
 *
 *
 */
GtkWidget *payees_get_tree_view ( void )
{
    return payee_tree;
}


/**
 *
 *
 *
 */
void payees_new_payee ( void )
{
    metatree_new_division ( GTK_TREE_MODEL ( payee_tree_model ) );

    sortie_edit_payee = FALSE;
    edit_payee ( GTK_TREE_VIEW ( payee_tree ) );
    if ( sortie_edit_payee )
    {
        supprimer_division ( GTK_TREE_VIEW ( payee_tree ) );
        sortie_edit_payee = FALSE;
    }
}


/**
 *
 *
 *
 */
void payees_delete_payee ( void )
{
    supprimer_division ( GTK_TREE_VIEW ( payee_tree ) );
}


/**
 *
 *
 *
 */
void payees_edit_payee ( void )
{
    edit_payee ( GTK_TREE_VIEW ( payee_tree ) );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
