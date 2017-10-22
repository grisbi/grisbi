/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
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



#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "imputation_budgetaire.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "grisbi_app.h"
#include "grisbi_win.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "gsb_data_budget.h"
#include "gsb_data_form.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_file_others.h"
#include "gsb_form_widget.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "meta_budgetary.h"
#include "metatree.h"
#include "mouse.h"
#include "transaction_list.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void appui_sur_ajout_imputation ( GtkTreeModel * model, GtkButton *button );
static gboolean budgetary_line_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
					GtkSelectionData * selection_data );
static gboolean budgetary_line_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null );
static void budgetary_line_list_popup_context_menu ( void );
static GtkWidget *creation_barre_outils_ib ( void );
static gboolean edit_budgetary_line ( GtkTreeView * view );
static gboolean popup_budgetary_line_view_mode_menu ( GtkWidget * button );
static void selectionne_sub_budgetary ( GtkTreeModel * model );
/*END_STATIC*/

static GtkWidget *budgetary_toolbar;
static GtkWidget *budgetary_line_tree = NULL;
static GtkTreeStore *budgetary_line_tree_model = NULL;

/* variable for the management of the cancelled edition */
static gboolean sortie_edit_budgetary_line = FALSE;

/* structure pour la sauvegarde de la position */
static struct metatree_hold_position *budgetary_hold_position;

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * réinitialisation des variables globales
 *
 * \param
 *
 * \return
 * */
void budgetary_lines_init_variables_list ( void )
{
    budgetary_line_tree_model = NULL;

    budgetary_toolbar = NULL;
    budgetary_line_tree = NULL;
    etat.no_devise_totaux_ib = 1;

    sortie_edit_budgetary_line = FALSE;
}


/**
 * crée et renvoie le widget contenu dans l'onglet
 *
 *
 *
 */
GtkWidget *budgetary_lines_create_list ( void )
{
    GtkWidget *scroll_window;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };

    /* We create the gtktreeview and model early so that they can be referenced. */
    budgetary_line_tree = gtk_tree_view_new();

	/* set the color of selected row */
	gtk_widget_set_name (budgetary_line_tree, "tree_view");

    budgetary_line_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS,
						     META_TREE_COLUMN_TYPES );

    /* We create the main vbox */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );

    /* frame pour la barre d'outils */
    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame, FALSE, FALSE, 0 );

    /* on y ajoute la barre d'outils */
    budgetary_toolbar = creation_barre_outils_ib ();
    gtk_container_add ( GTK_CONTAINER ( frame ), budgetary_toolbar );

    /* création de l'arbre principal */
    scroll_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scroll_window),
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ), scroll_window, TRUE, TRUE, 0 );
    gtk_widget_show ( scroll_window );

    /* Create model */
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(budgetary_line_tree_model),
                        META_TREE_TEXT_COLUMN, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(budgetary_line_tree_model),
                        META_TREE_TEXT_COLUMN, metatree_sort_column,
                        NULL, NULL );
    g_object_set_data ( G_OBJECT ( budgetary_line_tree_model), "metatree-interface",
                        budgetary_line_get_metatree_interface ( ) );

    /* Create container + TreeView */
    gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(budgetary_line_tree),
					   GDK_BUTTON1_MASK, row_targets, 1,
					   GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW(budgetary_line_tree), row_targets,
					   1, GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW(budgetary_line_tree), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(budgetary_line_tree)),
				  GTK_SELECTION_SINGLE );
    gtk_tree_view_set_model (GTK_TREE_VIEW (budgetary_line_tree),
			     GTK_TREE_MODEL (budgetary_line_tree_model));
    g_object_unref ( G_OBJECT(budgetary_line_tree_model));
    g_object_set_data ( G_OBJECT(budgetary_line_tree_model), "tree-view",
			budgetary_line_tree );

    /* Make category column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Budgetary line"), cell,
						       "text", META_TREE_TEXT_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( budgetary_line_tree ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make account column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Account"), cell,
						       "text", META_TREE_ACCOUNT_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( budgetary_line_tree ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make amount column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Amount"), cell,
						       "text", META_TREE_BALANCE_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
						       "xalign", META_TREE_XALIGN_COLUMN,
                               "cell-background-rgba", META_TREE_BACKGROUND_COLOR,
						       NULL);
    gtk_tree_view_column_set_alignment ( column, 1.0 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( budgetary_line_tree ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    gtk_container_add ( GTK_CONTAINER ( scroll_window ), budgetary_line_tree );
    gtk_widget_show ( budgetary_line_tree );

    /* Connect to signals */
    g_signal_connect ( G_OBJECT ( budgetary_line_tree ),
                        "row-collapsed",
                        G_CALLBACK ( division_column_collapsed ),
                        NULL );

    g_signal_connect ( G_OBJECT ( budgetary_line_tree ),
                        "row-expanded",
                        G_CALLBACK ( division_column_expanded ),
                        NULL );

    g_signal_connect( G_OBJECT ( budgetary_line_tree ),
                        "row-activated",
                        G_CALLBACK ( division_activated ),
                        NULL );

    g_signal_connect ( G_OBJECT ( budgetary_line_tree ),
                        "button-press-event",
                        G_CALLBACK ( budgetary_line_list_button_press ),
                        NULL );

    dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (budgetary_line_tree_model);
    if ( dst_iface )
    {
	dst_iface -> drag_data_received = &division_drag_data_received;
	dst_iface -> row_drop_possible = &division_row_drop_possible;
    }

    src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (budgetary_line_tree_model);
    if ( src_iface )
    {
	gtk_selection_add_target (budgetary_line_tree,
				  GDK_SELECTION_PRIMARY,
				  GDK_SELECTION_TYPE_ATOM,
				  1);
	src_iface -> drag_data_get = &budgetary_line_drag_data_get;
    }

    g_signal_connect ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(budgetary_line_tree)),
		       "changed", G_CALLBACK(metatree_selection_changed),
		       budgetary_line_tree_model );

    /* création de la structure de sauvegarde de la position */
    budgetary_hold_position = g_malloc0 ( sizeof ( struct metatree_hold_position ) );

    gtk_widget_show_all ( vbox );

    return ( vbox );
}
/* **************************************************************************************************** */


/**
 * fill the tree of budget
 *
 * \param
 *
 * \return
 * */
void budgetary_lines_fill_list ( void )
{
    GSList *budget_list;
    GtkTreeIter iter_budgetary_line, iter_sub_budgetary_line;
    GtkTreeSelection *selection;

    devel_debug (NULL);

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (budgetary_line_tree_model));

    /* Compute budget balances. */
    gsb_data_budget_update_counters ();

    /** Then, populate tree with budgetary lines. */
    budget_list = gsb_data_budget_get_budgets_list ();

    /* add first the empty budget */
    budget_list = g_slist_prepend ( budget_list,
				    gsb_data_budget_get_empty_budget ());

    while ( budget_list )
    {
	gint budget_number;

	budget_number = gsb_data_budget_get_no_budget (budget_list -> data);

	gtk_tree_store_append ( GTK_TREE_STORE ( budgetary_line_tree_model ), &iter_budgetary_line, NULL);
	fill_division_row ( GTK_TREE_MODEL ( budgetary_line_tree_model ),
                        budgetary_line_get_metatree_interface ( ),
                        &iter_budgetary_line,
                        budget_number );

	/** Each budget has sub budgetary lines. */
	if ( budget_number )
	{
	    GSList *sub_budget_list;

	    sub_budget_list = gsb_data_budget_get_sub_budget_list ( budget_number );

	    while ( sub_budget_list )
	    {
		gint sub_budget_number;

		sub_budget_number = gsb_data_budget_get_no_sub_budget (sub_budget_list -> data);

		gtk_tree_store_append ( GTK_TREE_STORE (budgetary_line_tree_model),
				       &iter_sub_budgetary_line, &iter_budgetary_line);
		fill_sub_division_row ( GTK_TREE_MODEL ( budgetary_line_tree_model ),
                        budgetary_line_get_metatree_interface ( ),
                        &iter_sub_budgetary_line,
                        budget_number,
                        sub_budget_number );

		sub_budget_list = sub_budget_list -> next;
	    }
	}

	/* add the no-sub-budget only if budget exists */
	if (budget_number)
	{
	    gtk_tree_store_append ( GTK_TREE_STORE ( budgetary_line_tree_model ),
                        &iter_sub_budgetary_line,
                        &iter_budgetary_line );
	    fill_sub_division_row ( GTK_TREE_MODEL(budgetary_line_tree_model),
                        budgetary_line_get_metatree_interface ( ),
                        &iter_sub_budgetary_line,
                        budget_number,
                        0 );
	}
	budget_list = budget_list -> next;
    }
    /* replace le curseur sur la division, sub_division ou opération initiale */
    if ( budgetary_hold_position -> path )
    {
        if ( budgetary_hold_position -> expand )
        {
            GtkTreePath *ancestor;

            ancestor = gtk_tree_path_copy ( budgetary_hold_position -> path );
            gtk_tree_path_up ( ancestor );
            gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( budgetary_line_tree ), ancestor );
            gtk_tree_path_free (ancestor );
        }
        /* on colorise les lignes du tree_view */
        utils_set_tree_view_background_color ( budgetary_line_tree, META_TREE_BACKGROUND_COLOR );
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( budgetary_line_tree ) );
        gtk_tree_selection_select_path ( selection, budgetary_hold_position -> path );
        gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( budgetary_line_tree ),
                        budgetary_hold_position -> path,
                        NULL, TRUE, 0.5, 0.5 );
    }
    else
    {
        gchar *title;

        /* on colorise les lignes du tree_view */
        utils_set_tree_view_background_color ( budgetary_line_tree, META_TREE_BACKGROUND_COLOR );
	    title = g_strdup(_("Budgetary lines"));
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
gboolean budgetary_line_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
					GtkSelectionData * selection_data )
{
    if ( path )
    {
	gtk_tree_set_row_drag_data (selection_data,
				    GTK_TREE_MODEL(budgetary_line_tree_model), path);
    }

    return FALSE;
}


/**
 * update the form's combofix for the budget
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_budget_update_combofix ( gboolean force )
{
    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_BUDGET ) || force )
        gtk_combofix_set_list ( GTK_COMBOFIX ( gsb_form_widget_get_widget (
                        TRANSACTION_FORM_BUDGET ) ),
                        gsb_data_budget_get_name_list ( TRUE, TRUE ) );

    return FALSE;
}


/**
 *
 *
 */
void budgetary_lines_exporter_list ( void )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *nom_ib;
    gchar *tmp_last_directory;

    dialog = gtk_file_chooser_dialog_new ( _("Export the budgetary lines"),
					   GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
					   GTK_FILE_CHOOSER_ACTION_SAVE,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-save", GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( dialog ),  _("Budgetary-lines.igsb"));
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), gsb_file_get_last_path () );
    gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( dialog ), TRUE);
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_ib = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ));
        tmp_last_directory = utils_files_selection_get_last_directory ( GTK_FILE_CHOOSER ( dialog ), TRUE );
        gsb_file_update_last_path ( tmp_last_directory );
        g_free ( tmp_last_directory );
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

	    /* vérification que c'est possible est faite par la boite de dialogue */

	    if ( !gsb_file_others_save_budget ( nom_ib ))
	    {
		dialogue_error ( _("Cannot save file.") );
		return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	    return;
    }
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void budgetary_lines_importer_list ( void )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *budget_name;
    gint last_transaction_number;
    GtkFileFilter * filter;
    gchar *tmp_last_directory;

    dialog = gtk_file_chooser_dialog_new ( _("Import budgetary lines"),
					   GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
					   GTK_FILE_CHOOSER_ACTION_OPEN,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-open", GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), gsb_file_get_last_path () );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Grisbi budgetary lines files (*.igsb)") );
    gtk_file_filter_add_pattern ( filter, "*.igsb" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );
    gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Grisbi category files (*.cgsb)") );
    gtk_file_filter_add_pattern ( filter, "*.cgsb" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

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

    budget_name = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ));
    tmp_last_directory = utils_files_selection_get_last_directory ( GTK_FILE_CHOOSER ( dialog ), TRUE );
    gsb_file_update_last_path ( tmp_last_directory );
    g_free ( tmp_last_directory );
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    last_transaction_number = gsb_data_transaction_get_last_number();

    /* on permet de remplacer/fusionner la liste */

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				       ( last_transaction_number ?
				         _("File already contains budgetary lines.  If you decide to continue, existing budgetary lines will be merged with imported ones.") :
				         _("File does not contain budgetary lines.  "
				           "If you decide to continue, existing budgetary lines will be merged with imported ones.  "
				           "Once performed, there is no undo for this.\n"
				           "You may also decide to replace existing budgetary lines with imported ones." ) ),
				       _("Merge imported budgetary lines with existing?") );

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
	    case 2 :
            /* we want to replace the list */
            if ( !last_transaction_number )
                gsb_data_budget_init_variables ();

		/* FALLTHRU */
        case 1 :
            if ( g_str_has_suffix ( budget_name, ".cgsb" ) )
                gsb_file_others_load_budget_from_category ( budget_name );
            else
                gsb_file_others_load_budget ( budget_name );
	    break;
	default :
	    return;
    }
}


/**
 * Create a button bar allowing to act on the budgetary lines list.
 * Some of these buttons are "linked" to the selection status of the
 * budgetary lines metatree.  That is, if nothing is selected, they will
 * become unsensitive.
 *
 * \return	A newly-allocated widget.
 */
GtkWidget *creation_barre_outils_ib ( void )
{
    GtkWidget *toolbar;
    GtkToolItem *item;

    toolbar = gtk_toolbar_new ();

    /* New budgetary line button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-new-ib-24.png", _("New\nbudgetary line"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Create a new budgetary line") );
    g_object_set_data ( G_OBJECT ( item ), "type", GINT_TO_POINTER (1) );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( appui_sur_ajout_imputation ),
                        budgetary_line_tree_model );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* New sub budgetary line button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-new-sub-ib-24.png", _("New sub\nbudgetary line"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( budgetary_line_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( budgetary_line_tree_model ),
                        GTK_WIDGET ( item ),
                        "sub-division" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Create a new sub-budgetary line") );
    g_object_set_data ( G_OBJECT ( item ), "type", GINT_TO_POINTER (2) );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( appui_sur_ajout_imputation ),
                        budgetary_line_tree_model );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Import button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-import-24.png", _("Import"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ),
                        _("Import a Grisbi budgetary line file (.igsb)"
                        " or create from a list of categories (.cgsb)" ) );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( budgetary_lines_importer_list ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Export button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-export-24.png", _("Export"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ),
                        _("Export a Grisbi budgetary line file (.igsb)") );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( budgetary_lines_exporter_list ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Delete button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-delete-24.png", _("Delete"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( budgetary_line_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Delete selected budgetary line") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( supprimer_division ),
                        budgetary_line_tree );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* Properties button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-edit-24.png", _("Edit"));
    metatree_register_widget_as_linked ( GTK_TREE_MODEL ( budgetary_line_tree_model ),
                        GTK_WIDGET ( item ),
                        "selection" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Edit selected budgetary line") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( edit_budgetary_line ),
                        budgetary_line_tree );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* View button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-select-color-24.png", _("View"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), _("Change display mode") );
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( popup_budgetary_line_view_mode_menu ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    metatree_set_linked_widgets_sensitive ( GTK_TREE_MODEL ( budgetary_line_tree_model ), FALSE, "selection" );

    return toolbar;
}


/**
 *
 *
 *
 */
void gsb_gui_budgetary_lines_toolbar_set_style ( gint toolbar_style )
{
    gtk_toolbar_set_style ( GTK_TOOLBAR ( budgetary_toolbar ), toolbar_style );
}


/**
 * Popup a menu that allow changing the view mode of the category
 * metatree.
 *
 * \param button	Button that triggered the signal.
 *
 * \return		FALSE
 */
gboolean popup_budgetary_line_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_menu_item_new_with_label ( _("Budgetary line view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 0 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", budgetary_line_tree );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Sub-budgetary line view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 1 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", budgetary_line_tree );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate",
		       G_CALLBACK(expand_arbre_division), (gpointer) 2 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", budgetary_line_tree );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

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
 *
 *
 */
gboolean edit_budgetary_line ( GtkTreeView * view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry, *hbox, *radiogroup;
    GtkTreeSelection * selection;
    GtkTreeModel * model = NULL;
    GtkTreeIter iter;
    gint budget_number = -1, sub_budget_number = -1;
    gchar * title;
    GtkTreeIter *div_iter;

    selection = gtk_tree_view_get_selection ( view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( model, &iter,
			     META_TREE_NO_DIV_COLUMN, &budget_number,
			     META_TREE_NO_SUB_DIV_COLUMN, &sub_budget_number,
			     -1 );
    }

    if ( !selection || budget_number <= 0 )
	return FALSE;

    if ( sub_budget_number > 0 )
	title = g_strdup_printf ( _("Properties for %s"),
				  gsb_data_budget_get_sub_budget_name ( budget_number,
									sub_budget_number,
									_("No sub-budget defined" )));
    else
	title = g_strdup_printf ( _("Properties for %s"),
				  gsb_data_budget_get_name ( budget_number,
							     0,
							     _("No budget defined") ));

    dialog = gtk_dialog_new_with_buttons ( title ,
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

    entry = gtk_entry_new ( );
    if ( sub_budget_number > 0 )
    {
	gtk_entry_set_text ( GTK_ENTRY ( entry ),
			     gsb_data_budget_get_sub_budget_name ( budget_number,
								   sub_budget_number,
								   NULL ) );
    }
    else
    {
	gtk_entry_set_text ( GTK_ENTRY ( entry ),
			     gsb_data_budget_get_name ( budget_number, 0, NULL ) );
    }

    gtk_widget_set_size_request ( entry, 400, -1 );
    gtk_grid_attach (GTK_GRID (table), entry, 1, 0, 1, 1);

    if ( sub_budget_number <= 0 )
    {
	/* Description entry */
	label = gtk_label_new ( _("Type"));
	utils_labels_set_alignement ( GTK_LABEL ( label ), 0.0, 0.5 );
	gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

	radiogroup = gsb_autofunc_radiobutton_new ( _("Credit"), _("Debit"),
						    gsb_data_budget_get_type (budget_number),
						    NULL, NULL,
						    G_CALLBACK (gsb_data_budget_set_type), budget_number);
	gtk_grid_attach (GTK_GRID (table), radiogroup, 1, 1, 1, 1);
    }

    gtk_widget_show_all ( dialog );
    g_free ( title );

    while ( 1 )
    {
	if ( gtk_dialog_run ( GTK_DIALOG(dialog) ) != GTK_RESPONSE_OK )
	{
        sortie_edit_budgetary_line = TRUE;
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	    return FALSE;
	}

	if ( ( sub_budget_number > 0 &&
	       gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
							       gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
							       FALSE ) &&
	       gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
							       gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
							       FALSE ) != sub_budget_number) ||
	     ( sub_budget_number <= 0 &&
	       gsb_data_budget_get_number_by_name ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
						    FALSE, 0 ) &&
	       gsb_data_budget_get_number_by_name ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
						    FALSE, 0 ) != budget_number ) )
	{
	    gchar * message;
	    if ( sub_budget_number > 0 )
	    {
		message = g_strdup_printf ( _("You tried to rename current sub-budgetary line to '%s' "
					      "but this sub-budgetary line already exists.  Please "
					      "choose another name."),
					    gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
	    }
	    else
	    {
		message = g_strdup_printf ( _("You tried to rename current budgetary line to '%s' "
					      "but this budgetary line already exists.  Please "
					      "choose another name."),
					    gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
	    }
	    dialogue_warning_hint ( message, _("Budgetary line already exists") );
	    g_free ( message );
	}
	else
	{
	    if ( sub_budget_number > 0 )
	    {
		gsb_data_budget_set_sub_budget_name ( budget_number,
						      sub_budget_number,
						      gtk_entry_get_text ( GTK_ENTRY (entry)));
	    }
	    else
	    {
		gsb_data_budget_set_name ( budget_number,
					   gtk_entry_get_text ( GTK_ENTRY (entry)));
	    }
	    break;
	}
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    if ( sub_budget_number > 0 )
    {
	div_iter = get_iter_from_div ( model, budget_number, sub_budget_number );
	fill_sub_division_row ( model,
                        budgetary_line_get_metatree_interface ( ),
                        div_iter,
                        budget_number,
                        sub_budget_number );
    }
    else
    {
	div_iter = get_iter_from_div ( model, budget_number, 0 );
	fill_division_row ( model,
                        budgetary_line_get_metatree_interface ( ),
                        div_iter,
                        budget_number );
    }
    gtk_tree_iter_free ( div_iter );

    /* update the transactions list */
    transaction_list_update_element ( ELEMENT_BUDGET );

    return TRUE;
}


/**
 * function to create and editing a new budgetary line.
 *
 * \param the model for the division
 */
void appui_sur_ajout_imputation ( GtkTreeModel * model, GtkButton *button )
{
    gint type;

    type = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (button), "type" ) );
    if ( type == 1 )
        metatree_new_division ( model );
    else
    {
        appui_sur_ajout_sub_division ( model );
        selectionne_sub_budgetary ( model );
    }
    sortie_edit_budgetary_line = FALSE;
    edit_budgetary_line ( GTK_TREE_VIEW ( budgetary_line_tree ) );
    if ( sortie_edit_budgetary_line )
        supprimer_division ( GTK_TREE_VIEW ( budgetary_line_tree ) );
    sortie_edit_budgetary_line = FALSE;
}


/**
 * function to expand budgetary and select new sub-budgetary.
 *
 * \param the model for the division
 */
void selectionne_sub_budgetary ( GtkTreeModel * model )
{
    GtkTreeSelection * selection;
    GtkTreeIter parent;
    GtkTreeIter iter;
	GtkTreePath * path;
    gchar * name;
    gint budget_number = -1, sub_budget_number = -1;
    gint i = 0,j = 0;

    selection = gtk_tree_view_get_selection (
                        GTK_TREE_VIEW (budgetary_line_tree) );
    if ( selection && gtk_tree_selection_get_selected (
                        selection, &model, &parent ) )
    {
	gtk_tree_model_get ( model, &parent,
                        META_TREE_NO_DIV_COLUMN, &budget_number,
                        META_TREE_NO_SUB_DIV_COLUMN, &sub_budget_number,
                        -1 );
    }

    if ( !selection || budget_number <= 0 )
        return;

    if ( sub_budget_number > 0 )
        return;

    name =  my_strdup (_("New sub-budget"));
    sub_budget_number = gsb_data_budget_get_sub_budget_number_by_name (
                        budget_number, name, FALSE );
    j = gtk_tree_model_iter_n_children ( model, &parent );
    for (i = 0; i < j; i++ )
    {
        gint numero;

        gtk_tree_model_iter_nth_child ( model, &iter, &parent, i );
        gtk_tree_model_get ( model, &iter,
                        META_TREE_NO_SUB_DIV_COLUMN, &numero,
                        -1 );
        if ( numero == sub_budget_number )
            break;
    }
    path = gtk_tree_model_get_path ( model, &iter );
    gtk_tree_view_expand_to_path ( GTK_TREE_VIEW (budgetary_line_tree), path );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (budgetary_line_tree) );
    gtk_tree_selection_select_path ( selection, path );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (budgetary_line_tree), path,
                        NULL, TRUE, 0.5, 0.5 );
    gtk_tree_path_free ( path );
    g_free ( name );
}


/**
 * renvoie le chemin de la dernière IB sélectionnée.
 *
 * \return une copie de budgetary_hold_position -> path
 */
GtkTreePath *budgetary_hold_position_get_path ( void )
{
    return gtk_tree_path_copy ( budgetary_hold_position -> path );
}

/**
 * sauvegarde le chemin de la dernière imputation sélectionnée.
 *
 * \param path
 */
gboolean budgetary_hold_position_set_path ( GtkTreePath *path )
{
    budgetary_hold_position -> path = gtk_tree_path_copy ( path );

    return TRUE;
}
/**
 * sauvegarde l'attribut expand.
 *
 * \param expand
 */
gboolean budgetary_hold_position_set_expand ( gboolean expand )
{
    budgetary_hold_position -> expand = expand;

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
gboolean budgetary_line_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null )
{
    if ( ev -> button == RIGHT_BUTTON )
    {
        budgetary_line_list_popup_context_menu ( );

        return TRUE;
    }
    else if ( ev -> type == GDK_2BUTTON_PRESS )
    {
        GtkTreeSelection *selection;
        GtkTreeModel *model = NULL;
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        enum meta_tree_row_type type_division;

        type_division = metatree_get_row_type_from_tree_view ( tree_view );
        if ( type_division == META_TREE_TRANSACTION )
            return FALSE;

        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
        if ( selection && gtk_tree_selection_get_selected (selection, &model, &iter ) )
            path = gtk_tree_model_get_path  ( model, &iter);
		else
			return FALSE;

        if ( conf.metatree_action_2button_press == 0 || type_division == META_TREE_DIV )
        {
            if ( gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( tree_view ), path ) )
                gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
            else
                gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );

            gtk_tree_path_free ( path );
            return FALSE;
        }
        else if ( conf.metatree_action_2button_press == 1 )
        {
            edit_budgetary_line ( GTK_TREE_VIEW ( tree_view ) );

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
void budgetary_line_list_popup_context_menu ( void )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    gchar *title;
    enum meta_tree_row_type type_division;

    type_division = metatree_get_row_type_from_tree_view ( budgetary_line_tree );

    if ( type_division == META_TREE_INVALID )
        return;

    menu = gtk_menu_new ();

    if ( type_division == META_TREE_TRANSACTION )
    {
        title = g_strdup ( _("Transfers the identical transactions in another sub-budgetary line") );

        menu_item = utils_menu_item_new_from_image_label ("gsb-convert-16.png", title);
        g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( metatree_transfer_identical_transactions ),
                        budgetary_line_tree );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        g_free ( title );
    }

    if ( type_division == META_TREE_DIV || type_division == META_TREE_SUB_DIV )
    {

        /* Edit transaction */
        if ( type_division == META_TREE_DIV )
            title = g_strdup ( _("Edit selected budgetary line") );
        else
            title = g_strdup ( _("Edit selected sub-budgetary line") );

        menu_item = utils_menu_item_new_from_image_label ("gtk-edit-16.png", title);
        g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                            "activate",
                            G_CALLBACK ( edit_budgetary_line ),
                            budgetary_line_tree );
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
            title = g_strdup ( _("Manage sub-budgetary line") );
        }
        else
            title = g_strdup ( _("Transfer all transactions in another sub-budgetary line") );

        menu_item = utils_menu_item_new_from_image_label ("gsb-convert-16.png", title);
        g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( metatree_manage_sub_divisions ),
                        budgetary_line_tree );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        g_free ( title );
    }

    /* Finish all. */
    gtk_widget_show_all ( menu );

#if GTK_CHECK_VERSION (3,22,0)
	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
#else
    gtk_menu_popup ( GTK_MENU ( menu ), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time ( ) );
#endif
}


/**
 *
 *
 *
 */
GtkTreeStore *budgetary_lines_get_tree_store ( void )
{
    return budgetary_line_tree_model;
}


/**
 *
 *
 *
 */
GtkWidget *budgetary_lines_get_tree_view ( void )
{
    return budgetary_line_tree;
}


/**
 *
 *
 *
 */
void budgetary_lines_new_budgetary_line ( void )
{
    metatree_new_division ( GTK_TREE_MODEL ( budgetary_line_tree_model ) );

    sortie_edit_budgetary_line = FALSE;
    edit_budgetary_line ( GTK_TREE_VIEW ( budgetary_line_tree ) );
    if ( sortie_edit_budgetary_line )
    {
        supprimer_division ( GTK_TREE_VIEW ( budgetary_line_tree ) );
        sortie_edit_budgetary_line = FALSE;
    }
}


/**
 *
 *
 *
 */
void budgetary_lines_delete_budgetary_line ( void )
{
    supprimer_division ( GTK_TREE_VIEW ( budgetary_line_tree ) );
}


/**
 *
 *
 *
 */
void budgetary_lines_edit_budgetary_line ( void )
{
    edit_budgetary_line ( GTK_TREE_VIEW ( budgetary_line_tree ) );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
