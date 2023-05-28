/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2009-2011 Pierre Biava (grisbi@pierre.biava.name)     */
/*          https://www.grisbi.org/                                            */
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
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "categories_onglet.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "gsb_data_category.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_file_others.h"
#include "gsb_transactions_list.h"
#include "meta_categories.h"
#include "metatree.h"
#include "mouse.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void appui_sur_ajout_category ( GtkTreeModel *model,
                        GtkButton *button );
static gboolean categ_drag_data_get ( GtkTreeDragSource *drag_source,
                        GtkTreePath *path,
                        GtkSelectionData *selection_data );
static gboolean category_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null );
static void category_list_popup_context_menu ( void );
static GtkWidget *creation_barre_outils_categ ( void );
static gboolean edit_category ( GtkTreeView *tree_view );
static gboolean popup_category_view_mode_menu ( GtkWidget *button );
static void selectionne_sub_category ( GtkTreeModel *model );
/*END_STATIC*/


/* Category toolbar, tree model & tree view */
static GtkWidget *category_toolbar = NULL;
static GtkTreeStore *categ_tree_model = NULL;
static GtkWidget *arbre_categ = NULL;

/* variable for the management of the cancelled edition */
static gboolean sortie_edit_category = FALSE;

/* structure pour la sauvegarde de la position */
struct MetatreeHoldPosition *category_hold_position;


/*START_EXTERN*/
/*END_EXTERN*/


/**
 * réinitialisation des variables globales
 *
 * \param
 *
 * \return
 * */
void categories_init_variables_list ( void )
{
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
    categ_tree_model = NULL;
    category_toolbar = NULL;
    arbre_categ = NULL;
    w_etat->no_devise_totaux_categ = 1;
    sortie_edit_category = FALSE;
}


/**
 * Create and return contents of the "Category" notebook page.
 *
 * \return A newly allocated hbox.
 */
GtkWidget *categories_create_list ( void )
{
	GtkWidget *window;
    GtkWidget *scroll_window, *vbox;
    GtkWidget *frame;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {{(gchar*)"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }};
    MetatreeInterface *category_interface;

	window = GTK_WIDGET (grisbi_app_get_active_window (NULL));

	/* We create the main vbox */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );

    /* frame pour la barre d'outils */
    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame, FALSE, FALSE, 0 );

    category_interface = category_get_metatree_interface ( );

    /* We create the gtktreeview and model early so that they can be referenced. */
    arbre_categ = gtk_tree_view_new();

    /* set the color of selected row */
	gtk_widget_set_name (arbre_categ, "colorized_tree_view");

    /* Create model */
    categ_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, META_TREE_COLUMN_TYPES );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(categ_tree_model),
					   META_TREE_TEXT_COLUMN, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(categ_tree_model),
				      META_TREE_TEXT_COLUMN, metatree_sort_column,
				      NULL, NULL );
    g_object_set_data ( G_OBJECT (categ_tree_model), "metatree-interface", category_interface );

    /* on y ajoute la barre d'outils après la création du model */
    category_toolbar = creation_barre_outils_categ ();
    gtk_container_add ( GTK_CONTAINER ( frame ), category_toolbar );

    /* création de l'arbre principal */
    scroll_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scroll_window),
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ), scroll_window, TRUE, TRUE, 0 );
    gtk_widget_show ( scroll_window );

    /* Create container + TreeView */
    gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(arbre_categ),
					   GDK_BUTTON1_MASK, row_targets, 1,
					   GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW(arbre_categ), row_targets,
					   1, GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW(arbre_categ), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(arbre_categ)),
				  GTK_SELECTION_SINGLE );
    gtk_tree_view_set_model (GTK_TREE_VIEW (arbre_categ),
			     GTK_TREE_MODEL (categ_tree_model));
    g_object_unref (G_OBJECT(categ_tree_model));
    g_object_set_data ( G_OBJECT(categ_tree_model), "tree-view", arbre_categ );

    /* Make category column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Category"), cell,
						       "text", META_TREE_TEXT_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( arbre_categ ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make account column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Account"), cell,
						       "text", META_TREE_ACCOUNT_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( arbre_categ ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make amount column */
    cell = gtk_cell_renderer_text_new ();
	gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (cell), MARGIN_BOX, 0);
    column = gtk_tree_view_column_new_with_attributes (_("Amount"), cell,
						       "text", META_TREE_BALANCE_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
						       "xalign", META_TREE_XALIGN_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_column_set_alignment ( column, 1.0 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( arbre_categ ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    gtk_container_add ( GTK_CONTAINER ( scroll_window ), arbre_categ );
    gtk_widget_show ( arbre_categ );

    /* Connect to signals */
    g_signal_connect ( G_OBJECT ( arbre_categ ),
                        "row-collapsed",
                        G_CALLBACK ( division_column_collapsed ),
                        NULL );

    g_signal_connect ( G_OBJECT ( arbre_categ ),
                        "row-expanded",
                        G_CALLBACK ( division_column_expanded ),
                        NULL );

    g_signal_connect( G_OBJECT ( arbre_categ ),
                        "row-activated",
                        G_CALLBACK ( division_activated ),
                        NULL);

    g_signal_connect ( G_OBJECT ( arbre_categ ),
                        "button-press-event",
                        G_CALLBACK ( category_list_button_press ),
                        NULL );

    dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (categ_tree_model);
    if ( dst_iface )
    {
	dst_iface -> drag_data_received = &division_drag_data_received;
	dst_iface -> row_drop_possible = &division_row_drop_possible;
    }

    src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (categ_tree_model);
    if ( src_iface )
    {
	gtk_selection_add_target (window,
				  GDK_SELECTION_PRIMARY,
				  GDK_SELECTION_TYPE_ATOM,
				  1);
	src_iface -> drag_data_get = &categ_drag_data_get;
    }

    g_signal_connect ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( arbre_categ ) ),
                        "changed",
                        G_CALLBACK ( metatree_selection_changed ),
                        categ_tree_model );

    /* création de la structure de sauvegarde de la position */
    category_hold_position = g_malloc0 ( sizeof ( struct MetatreeHoldPosition ) );

    gtk_widget_show_all ( vbox );

    return ( vbox );
}



/**
 * Fill category tree with data.
 */
void categories_fill_list ( void )
{
    GSList *category_list;
    GtkTreeIter iter_categ, iter_sous_categ;
    MetatreeInterface *category_interface;

    devel_debug (NULL);

    /* Model might be empty because we are importing categories
     * before file is opened.  So don't do anything. */
    if ( ! categ_tree_model || ! GTK_IS_TREE_STORE(categ_tree_model) )
	return;

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (categ_tree_model) );

    /* Compute category balances. */
    gsb_data_category_update_counters ();

    /** Then, populate tree with categories. */
    category_list = gsb_data_category_get_categories_list ();

    /* add first the empty category */
    category_list = g_slist_prepend ( category_list, gsb_data_category_get_empty_category ());

    category_interface = category_get_metatree_interface ( );

    while ( category_list )
    {
	gint category_number;

	category_number = gsb_data_category_get_no_category (category_list -> data);

	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), &iter_categ, NULL);
	fill_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface,
			    &iter_categ, category_number);

	/** Each category has subcategories. */
	if ( category_number )
	{
	    GSList *sub_category_list;

	    sub_category_list = gsb_data_category_get_sub_category_list ( category_number );

	    while ( sub_category_list )
	    {
		gint sub_category_number;

		sub_category_number = gsb_data_category_get_no_sub_category (sub_category_list -> data);

		gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model),
				       &iter_sous_categ, &iter_categ);
		fill_sub_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface,
					&iter_sous_categ,
					category_number,
					sub_category_number);

		sub_category_list = sub_category_list -> next;
	    }
	}

	/* add the no sub category only if category */
	if (category_number)
	{
	    gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model),
				   &iter_sous_categ, &iter_categ);
	    fill_sub_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface,
				    &iter_sous_categ, category_number, 0 );
	}

	category_list = category_list -> next;
    }

    if ( category_hold_position -> path )
    {
        GtkTreeSelection *selection;

        if ( category_hold_position -> expand )
        {
            GtkTreePath *ancestor;

            ancestor = gtk_tree_path_copy ( category_hold_position -> path );
            gtk_tree_path_up ( ancestor );
            gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( arbre_categ ), ancestor );
            gtk_tree_path_free (ancestor );
        }
        /* on colorise les lignes du tree_view */
        utils_set_tree_store_background_color ( arbre_categ, META_TREE_BACKGROUND_COLOR );
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( arbre_categ ) );
        gtk_tree_selection_select_path ( selection, category_hold_position -> path );
        gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( arbre_categ ),
                        category_hold_position -> path,
                        NULL, TRUE, 0.5, 0.5 );
    }
    else
    {
        gchar *title;

        /* on colorise les lignes du tree_view */
        utils_set_tree_store_background_color ( arbre_categ, META_TREE_BACKGROUND_COLOR );
	    title = g_strdup(_("Categories"));
        grisbi_win_headings_update_title ( title );
        g_free ( title );
        grisbi_win_headings_update_suffix ( "" );
    }
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
gboolean categ_drag_data_get ( GtkTreeDragSource *drag_source,
                        GtkTreePath *path,
                        GtkSelectionData *selection_data )
{
    if ( path )
    {
	gtk_tree_set_row_drag_data (selection_data,
				    GTK_TREE_MODEL(categ_tree_model), path);
    }

    return FALSE;
}



/**
 *
 *
 */
void categories_exporter_list ( void )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *nom_categ;
    gchar *tmp_last_directory;

    dialog = gtk_file_chooser_dialog_new ( _("Export categories"),
					   GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
					   GTK_FILE_CHOOSER_ACTION_SAVE,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-save", GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( dialog ),  _("Categories.cgsb"));
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), gsb_file_get_last_path () );
    gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( dialog ), TRUE);
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat != GTK_RESPONSE_OK )
    {
        gtk_widget_destroy ( dialog );
        return;
    }

    nom_categ = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ));
    tmp_last_directory = utils_files_selection_get_last_directory ( GTK_FILE_CHOOSER ( dialog ), TRUE );
    gsb_file_update_last_path ( tmp_last_directory );
    g_free ( tmp_last_directory );
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    gsb_file_others_save_category ( nom_categ );
}



/**
 *
 *
 */
void categories_importer_list ( void )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *category_name;
    gint last_transaction_number;
    GtkFileFilter * filter;
    gchar *tmp_last_directory;

    dialog = gtk_file_chooser_dialog_new ( _("Import categories"),
					   GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
					   GTK_FILE_CHOOSER_ACTION_OPEN,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-open", GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), gsb_file_get_last_path () );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Grisbi category files (*.cgsb)") );
    gtk_file_filter_add_pattern ( filter, "*.cgsb" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );
    gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("All files") );
    gtk_file_filter_add_pattern ( filter, "*" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat != GTK_RESPONSE_OK  )
    {
	gtk_widget_destroy ( dialog );
	return;
    }

    category_name = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ));
    tmp_last_directory = utils_files_selection_get_last_directory ( GTK_FILE_CHOOSER ( dialog ), TRUE );
    gsb_file_update_last_path ( tmp_last_directory );
    g_free ( tmp_last_directory );
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    last_transaction_number = gsb_data_transaction_get_last_number();

    /* on permet de remplacer/fusionner la liste */

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				       ( last_transaction_number ?
				         _("File already contains categories.  If you decide to continue, existing categories will be merged with imported ones.") :
				         _("File does not contain categories.  "
				           "If you decide to continue, existing categories will be merged with imported ones.  "
				           "Once performed, there is no undo for this.\n"
				           "You may also decide to replace existing categories with imported ones." ) ),
				       _("Merge imported categories with existing?") );

    if ( !last_transaction_number)
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Replace existing"), 2,
				 NULL );

    gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
			     "gtk-cancel", 0,
			     "gtk-ok", 1,
			     NULL );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    switch ( resultat )
    {
		case 1 :
			gsb_file_others_load_category ( category_name );
			return;

		case 2 :
			/* we want to replace the list */
			if ( !last_transaction_number )
				gsb_data_category_init_variables (TRUE);
			gsb_file_others_load_category ( category_name );

		default :
			return;
    }
}


/**
 * Create a button bar allowing to act on the categories list.  Some
 * of these buttons are "linked" to the selection status of the
 * category metatree.  That is, if nothing is selected, they will
 * become unsensitive.
 *
 * \return	A newly-allocated widget.
 */
GtkWidget *creation_barre_outils_categ ( void )
{
    GtkWidget *toolbar;
    GtkToolItem *item;

    toolbar = gtk_toolbar_new ();

    /* New category button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-new-categ-24.png", _("New\ncategory"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Create a new category") );
    g_object_set_data ( G_OBJECT ( item ), "type", GINT_TO_POINTER (1) );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( appui_sur_ajout_category ),
                        categ_tree_model );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* New sub category button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-new-sub-categ-24.png", _("New sub\ncategory"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( categ_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( categ_tree_model ),
                        GTK_WIDGET ( item ),
                        "sub-division" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Create a new sub-category") );
    g_object_set_data ( G_OBJECT ( item ), "type", GINT_TO_POINTER (2) );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( appui_sur_ajout_category ),
                        categ_tree_model );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Import button */
    item = utils_buttons_tool_button_new_from_image_resource ("gsb-import-24.png", _("Import"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Import a Grisbi category file (.cgsb)") );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( categories_importer_list ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Export button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-export-24.png", _("Export"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Export a Grisbi category file (.cgsb)") );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( categories_exporter_list ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Delete button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-delete-24.png", _("Delete"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( categ_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Delete selected category") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( supprimer_division ),
                        arbre_categ );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Properties button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-edit-24.png", _("Edit"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( categ_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Edit selected category") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( edit_category ),
                        arbre_categ );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* View button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-select-color-24.png", _("View"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Change view mode"));
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( popup_category_view_mode_menu ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    metatree_set_linked_widgets_sensitive ( GTK_TREE_MODEL ( categ_tree_model ), FALSE, "selection" );

    return ( toolbar );
}


/**
 *
 *
 *
 */
void gsb_gui_categories_toolbar_set_style ( gint toolbar_style )
{
    gtk_toolbar_set_style ( GTK_TOOLBAR ( category_toolbar ), toolbar_style );
}


/**
 * Popup a menu that allow changing the view mode of the category
 * metatree.
 *
 * \param button	Button that triggered the signal.
 *
 * \return		FALSE
 */
gboolean popup_category_view_mode_menu ( GtkWidget *button )
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_menu_item_new_with_label ( _("Category view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 0 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Subcategory view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 1 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 2 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    gtk_widget_show_all ( menu );

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);

	return FALSE;
}



/**
 *
 *
 */
gboolean edit_category ( GtkTreeView *tree_view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry, *hbox, *radiogroup;
    gint category_number = -1, sub_category_number = -1;
    GtkTreeSelection * selection;
    GtkTreeModel * model = NULL;
    GtkTreeIter iter;
    gchar * title;
    GtkTreeIter *div_iter;
    MetatreeInterface *category_interface;

    /* fill category_number and sub_category_number */

    selection = gtk_tree_view_get_selection ( tree_view );
    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
	gtk_tree_model_get ( model, &iter,
			     META_TREE_NO_DIV_COLUMN, &category_number,
			     META_TREE_NO_SUB_DIV_COLUMN, &sub_category_number,
			     -1 );
    }

    if ( !selection || category_number <= 0 )
	return FALSE;

    if ( sub_category_number > 0 )
	{
		gchar *tmp_str = gsb_data_category_get_sub_category_name ( category_number,
														   sub_category_number,
														   _("No sub-category defined" ));
		title = g_strdup_printf ( _("Properties for %s"), tmp_str);
		g_free(tmp_str);
	}
    else
	{
		gchar *tmp_str = gsb_data_category_get_name ( category_number,
													 0,
													 _("No category defined") );
		title = g_strdup_printf ( _("Properties for %s"), tmp_str);
		g_free(tmp_str);
	}

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
    g_free ( title );

    table = gtk_grid_new ();
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 6 );
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);

    /* Name entry */
    label = gtk_label_new ( _("Name"));
    utils_labels_set_alignment ( GTK_LABEL ( label ), 0.0, 0.5 );
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

    entry = gtk_entry_new ( );
    if ( sub_category_number > 0 )
    {
	gtk_entry_set_text ( GTK_ENTRY ( entry ),
			     gsb_data_category_get_sub_category_name ( category_number,
								       sub_category_number,
								       NULL ) );
    }
    else
    {
	gtk_entry_set_text ( GTK_ENTRY ( entry ),
			     gsb_data_category_get_name ( category_number, 0, NULL ) );
    }

    gtk_widget_set_size_request ( entry, 400, -1 );
    gtk_grid_attach (GTK_GRID (table), entry, 1, 0, 1, 1);

    if ( sub_category_number <= 0 )
    {
	/* Description entry */
	label = gtk_label_new ( _("Type"));
	utils_labels_set_alignment ( GTK_LABEL ( label ), 0.0, 0.5 );
	gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

	radiogroup = gsb_autofunc_radiobutton_new (  _("Credit"), _("Debit"),
						     gsb_data_category_get_type (category_number),
						     NULL, NULL,
						     G_CALLBACK (gsb_data_category_set_type), category_number );
	gtk_grid_attach (GTK_GRID (table), radiogroup, 1, 1, 1, 1);
    }

    gtk_widget_show_all ( dialog );

    while ( 1 )
    {
	if ( gtk_dialog_run ( GTK_DIALOG(dialog) ) != GTK_RESPONSE_OK )
	{
        sortie_edit_category = TRUE;
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	    return FALSE;
	}

	if ( ( sub_category_number > 0 &&
	       gsb_data_category_get_sub_category_number_by_name ( category_number,
								   gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
								   FALSE ) &&
	       gsb_data_category_get_sub_category_number_by_name ( category_number,
								   gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
								   FALSE ) != sub_category_number ) ||
	     ( sub_category_number <= 0 &&
	       gsb_data_category_get_number_by_name ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
						      FALSE, 0 ) &&
	       gsb_data_category_get_number_by_name ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
						      FALSE, 0 ) != category_number ) )
	{
	    gchar * message;
	    if ( sub_category_number > 0 )
	    {
		message = g_strdup_printf ( _("You tried to rename current sub-category to '%s' "
					      "but this sub-category already exists. Please "
					      "choose another name."),
					    gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
	    }
	    else
	    {
		message = g_strdup_printf ( _("You tried to rename current category to '%s' "
					      "but this category already exists. Please "
					      "choose another name."),
					    gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
	    }
	    dialogue_warning_hint ( message, _("Category already exists") );
	    g_free ( message );
	}
	else
	{
	    if ( sub_category_number > 0 )
	    {
		gsb_data_category_set_sub_category_name ( category_number,
							  sub_category_number,
							  gtk_entry_get_text ( GTK_ENTRY (entry)));
	    }
	    else
	    {
		gsb_data_category_set_name ( category_number,
					     gtk_entry_get_text ( GTK_ENTRY (entry)));
	    }
	    break;
	}
    }

    gtk_widget_destroy ( dialog );

    category_interface = category_get_metatree_interface ( );

    if ( sub_category_number > 0 )
    {
	div_iter = get_iter_from_div ( model, category_number, sub_category_number );
	fill_sub_division_row ( model, category_interface,
				div_iter,
				category_number,
				sub_category_number);
    }
    else
    {
	div_iter = get_iter_from_div ( model, category_number, 0 );
	fill_division_row ( model, category_interface,
			    div_iter,
			    category_number );
    }
    gtk_tree_iter_free (div_iter);

    /* update the transactions list */
    transaction_list_update_element (ELEMENT_CATEGORY);

    gsb_file_set_modified ( TRUE );

    return TRUE;
}


/**
 * function to create and editing a new category.
 *
 * \param the model for the division
 */
void appui_sur_ajout_category ( GtkTreeModel *model,
                        GtkButton *button )
{
    gint type;

    type = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (button), "type" ) );

    if ( type == 1 )
        metatree_new_division ( model );
    else
    {
        appui_sur_ajout_sub_division ( model );
        selectionne_sub_category ( model );
    }
    sortie_edit_category = FALSE;
    edit_category ( GTK_TREE_VIEW ( arbre_categ ) );
    if ( sortie_edit_category )
        supprimer_division ( GTK_TREE_VIEW ( arbre_categ ) );
    sortie_edit_category = FALSE;
}


/**
 * function to expand category and select new sub-category.
 *
 * \param the model for the division
 */
void selectionne_sub_category ( GtkTreeModel *model )
{
    GtkTreeSelection * selection;
    GtkTreeIter parent;
    GtkTreeIter iter;
	GtkTreePath * path;
    gchar * name;
    gint category_number = -1, sub_category_number = -1;
    gint i = 0,j = 0;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (arbre_categ) );
    if ( selection && gtk_tree_selection_get_selected (
                        selection, &model, &parent ) )
    {
	gtk_tree_model_get ( model, &parent,
                        META_TREE_NO_DIV_COLUMN, &category_number,
                        META_TREE_NO_SUB_DIV_COLUMN, &sub_category_number,
                        -1 );
    }

    if ( !selection || category_number <= 0 )
        return;

    if ( sub_category_number > 0 )
        return;

    name =  my_strdup (_("New sub-category"));
    sub_category_number = gsb_data_category_get_sub_category_number_by_name (
                        category_number, name, FALSE );
    j = gtk_tree_model_iter_n_children ( model, &parent );
    for (i = 0; i < j; i++ )
    {
        gint numero;

        gtk_tree_model_iter_nth_child ( model, &iter, &parent, i );
        gtk_tree_model_get ( model, &iter,
                        META_TREE_NO_SUB_DIV_COLUMN, &numero,
                        -1 );
        if ( numero == sub_category_number )
            break;
    }
    path = gtk_tree_model_get_path ( model, &iter );
    gtk_tree_view_expand_to_path ( GTK_TREE_VIEW (arbre_categ), path );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (arbre_categ) );
    gtk_tree_selection_select_path ( selection, path );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (arbre_categ), path,
                        NULL, TRUE, 0.5, 0.5 );
    gtk_tree_path_free ( path );
    g_free ( name );
}


/**
 * renvoie le chemin de la dernière categorie sélectionnée.
 *
 * \return une copie de category_hold_position -> path
 */
GtkTreePath *categories_hold_position_get_path ( void )
{
    return gtk_tree_path_copy ( category_hold_position -> path );
}

/**
 * sauvegarde le chemin de la dernière categorie sélectionnée.
 *
 * \param path
 */
gboolean categories_hold_position_set_path ( GtkTreePath *path )
{
    category_hold_position -> path = gtk_tree_path_copy ( path );

    return TRUE;
}


/**
 * sauvegarde l'attribut expand.
 *
 * \param expand
 */
gboolean categories_hold_position_set_expand ( gboolean expand )
{
    category_hold_position -> expand = expand;

    return TRUE;
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean category_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null )
{
    if ( ev -> button == RIGHT_BUTTON )
    {
        category_list_popup_context_menu ( );

        return TRUE;
    }
    else if ( ev -> type == GDK_2BUTTON_PRESS )
    {
        GtkTreeSelection *selection;
        GtkTreeModel *model = NULL;
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        enum MetaTreeRowType type_division;
		GrisbiAppConf *a_conf;

        type_division = metatree_get_row_type_from_tree_view ( tree_view );
        if ( type_division == META_TREE_TRANSACTION )
            return FALSE;

        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
        if ( selection && gtk_tree_selection_get_selected (selection, &model, &iter ) )
            path = gtk_tree_model_get_path  ( model, &iter);
		else
			return FALSE;

		a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
        if ( a_conf->metatree_action_2button_press == 0 || type_division == META_TREE_DIV )
        {
            if ( gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( tree_view ), path ) )
                gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
            else
                gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );

            gtk_tree_path_free ( path );
            return FALSE;
        }
        else if ( a_conf->metatree_action_2button_press == 1 )
        {
            edit_category ( GTK_TREE_VIEW ( tree_view ) );

            gtk_tree_path_free ( path );
            return TRUE;
        }
        else
        {
            if ( type_division == META_TREE_SUB_DIV || type_division == META_TREE_TRANS_S_S_DIV )
            {
                    gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );

                    gtk_tree_path_free ( path );

                metatree_manage_sub_divisions ( tree_view );
                return TRUE;
            }
            else
                return FALSE;
        }
    }

    return FALSE;
}


/**
 * Pop up a menu with several actions to apply to current selection.
 *
 * \param
 *
 */
void category_list_popup_context_menu ( void )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    gchar *title;
    enum MetaTreeRowType type_division;

    type_division = metatree_get_row_type_from_tree_view ( arbre_categ );

    if ( type_division == META_TREE_INVALID )
        return;

    menu = gtk_menu_new ();

    if ( type_division == META_TREE_TRANSACTION )
    {
        title = g_strdup ( _("Transfers the identical transactions in another sub-category") );

        menu_item = utils_menu_item_new_from_image_label ("gsb-convert-16.png", title);
        g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( metatree_transfer_identical_transactions ),
                        arbre_categ );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        g_free ( title );
    }

    if ( type_division == META_TREE_DIV || type_division == META_TREE_SUB_DIV )
    {

        /* Edit transaction */
        if ( type_division == META_TREE_DIV )
            title = g_strdup ( _("Edit selected category") );
        else
            title = g_strdup ( _("Edit selected sub-category") );

        menu_item = utils_menu_item_new_from_image_label ("gtk-edit-16.png", title);
        g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                            "activate",
                            G_CALLBACK ( edit_category ),
                            arbre_categ );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        g_free ( title );
    }

    if ( type_division == META_TREE_SUB_DIV || type_division == META_TREE_TRANS_S_S_DIV )
    {
        /* Manage sub_divisions */
        if ( type_division == META_TREE_SUB_DIV )
        {
            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new ( ) );
            title = g_strdup ( _("Manage sub-categories") );
        }
        else
            title = g_strdup ( _("Transfer all transactions in another sub-category") );

        menu_item = utils_menu_item_new_from_image_label ("gsb-convert-16.png", title);
        g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( metatree_manage_sub_divisions ),
                        arbre_categ );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        g_free ( title );
    }

    /* Finish all. */
    gtk_widget_show_all ( menu );

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
}


/**
 *
 *
 *
 */
GtkTreeStore *categories_get_tree_store ( void )
{
    return categ_tree_model;
}


/**
 *
 *
 *
 */
GtkWidget *categories_get_tree_view ( void )
{
    return arbre_categ;
}


/**
 *
 *
 *
 */
void categories_new_category ( void )
{
    metatree_new_division ( GTK_TREE_MODEL ( categ_tree_model ) );

    sortie_edit_category = FALSE;
    edit_category ( GTK_TREE_VIEW ( arbre_categ ) );
    if ( sortie_edit_category )
    {
        supprimer_division ( GTK_TREE_VIEW ( arbre_categ ) );
        sortie_edit_category = FALSE;
    }
}


/**
 *
 *
 *
 */
void categories_delete_category ( void )
{
    supprimer_division ( GTK_TREE_VIEW ( arbre_categ ) );
}


/**
 *
 *
 *
 */
void categories_edit_category ( void )
{
    edit_category ( GTK_TREE_VIEW ( arbre_categ ) );
}


/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
