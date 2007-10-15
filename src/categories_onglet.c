/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2007 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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
#include "categories_onglet.h"
#include "./metatree.h"
#include "./erreur.h"
#include "./dialog.h"
#include "./utils_file_selection.h"
#include "./gsb_autofunc.h"
#include "./gsb_automem.h"
#include "./gsb_data_category.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file.h"
#include "./gsb_file_others.h"
#include "./gsb_transactions_list.h"
#include "./main.h"
#include "./utils.h"
#include "./utils_buttons.h"
#include "./utils_file_selection.h"
#include "./metatree.h"
#include "./gsb_transactions_list.h"
#include "./include.h"
#include "./structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean categ_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
			       GtkSelectionData * selection_data );
static GtkWidget *creation_barre_outils_categ ( void );
static gboolean edit_category ( GtkTreeView * view );
static gboolean exporter_categ ( GtkButton * widget, gpointer data );
static gboolean gsb_category_page_sub_entry_changed ( GtkWidget *entry,
					       gint *sub_categ_tmp );
static void importer_categ ( void );
static gboolean popup_category_view_mode_menu ( GtkWidget * button );
/*END_STATIC*/



/* widgets */
GtkWidget *entree_nom_categ, *bouton_categ_debit, *bouton_categ_credit;
GtkWidget *bouton_modif_categ_modifier, *bouton_modif_categ_annuler;
GtkWidget *bouton_supprimer_categ, *bouton_ajouter_categorie;
GtkWidget *bouton_ajouter_sous_categorie;

/* Category tree model & view */
GtkTreeStore * categ_tree_model;
GtkWidget *arbre_categ;
int no_devise_totaux_categ;


/*START_EXTERN*/
extern MetatreeInterface * category_interface ;
extern GtkTreeSelection * selection;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *window;
/*END_EXTERN*/




/**
 * Create and return contents of the "Category" notebook page. 
 *
 * \return A newly allocated hbox.
 */
GtkWidget *onglet_categories ( void )
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkWidget *scroll_window, *vbox;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };

    /* We create the gtktreeview and model early so that they can be referenced. */
    arbre_categ = gtk_tree_view_new();
    categ_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, META_TREE_COLUMN_TYPES );

    /* We create the main vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_show ( vbox );

    /* on y ajoute la barre d'outils */
    gtk_box_pack_start ( GTK_BOX ( vbox ), creation_barre_outils_categ(), 
			 FALSE, FALSE, 0 );

    /* création de l'arbre principal */

    scroll_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scroll_window), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ), scroll_window, TRUE, TRUE, 0 );
    gtk_widget_show ( scroll_window );

    /* Create model */
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(categ_tree_model), 
					   META_TREE_TEXT_COLUMN, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(categ_tree_model), 
				      META_TREE_TEXT_COLUMN, metatree_sort_column,
				      NULL, NULL );
    g_object_set_data ( G_OBJECT (categ_tree_model), "metatree-interface", 
			category_interface );

    /* Create container + TreeView */
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (arbre_categ), TRUE);
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
    g_object_set_data ( G_OBJECT(categ_tree_model), "tree-view", arbre_categ );

    /* Make category column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Category"), cell, 
						       "text", META_TREE_TEXT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
#if GTK_CHECK_VERSION(2,4,0)
    gtk_tree_view_column_set_expand ( column, TRUE );
#endif
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( arbre_categ ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make account column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Account"), cell, 
						       "text", META_TREE_ACCOUNT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( arbre_categ ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make balance column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Balance"), cell, 
						       "text", META_TREE_BALANCE_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
						       "xalign", META_TREE_XALIGN_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( arbre_categ ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    gtk_container_add ( GTK_CONTAINER ( scroll_window ), arbre_categ );
    gtk_widget_show ( arbre_categ );

    /* Connect to signals */
    g_signal_connect ( G_OBJECT(arbre_categ), "row-expanded", 
		       G_CALLBACK(division_column_expanded), NULL );
    g_signal_connect( G_OBJECT(arbre_categ), "row-activated",
		      G_CALLBACK(division_activated), NULL);

    dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (categ_tree_model);
    if ( dst_iface )
    {
	dst_iface -> drag_data_received = &division_drag_data_received;
	dst_iface -> row_drop_possible = &division_row_drop_possible;
    }

    src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (categ_tree_model);
    if ( src_iface )
    {
	gtk_selection_add_target (arbre_categ,
				  GDK_SELECTION_PRIMARY,
				  GDK_SELECTION_TYPE_ATOM,
				  1);
	src_iface -> drag_data_get = &categ_drag_data_get;
    }

    g_signal_connect ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(arbre_categ)),
		       "changed", G_CALLBACK(metatree_selection_changed),
		       categ_tree_model );
    return ( vbox );
}



/**
 * Fill category tree with data.
 */
void remplit_arbre_categ ( void )
{
    GSList *category_list;
    GtkTreeIter iter_categ, iter_sous_categ;

    devel_debug ( "remplit_arbre_categ" );

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (categ_tree_model) );

    /* Compute category balances. */
    gsb_data_category_update_counters ();

    /** Then, populate tree with categories. */

    category_list = gsb_data_category_get_categories_list ();
    category_list = g_slist_prepend ( category_list, NULL );

    while ( category_list )
    {
	gint category_number;

	category_number = gsb_data_category_get_no_category (category_list -> data);

	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), &iter_categ, NULL);
	fill_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
			    &iter_categ, gsb_data_category_get_structure (category_number));

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
					gsb_data_category_get_structure (category_number),
					gsb_data_category_get_sub_category_structure( category_number,
										      sub_category_number));

		sub_category_list = sub_category_list -> next;
	    }
	}

	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), 
			       &iter_sous_categ, &iter_categ);
	fill_sub_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
				&iter_sous_categ, gsb_data_category_get_structure (category_number), NULL );
	
	category_list = category_list -> next;
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
gboolean categ_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
			       GtkSelectionData * selection_data )
{
    if ( path )
    {
/* 	gtk_tree_set_row_drag_data (selection_data, GTK_TREE_MODEL(categ_tree_model), path); */
    }

    return FALSE;
}



/**
 *
 *
 */
gboolean exporter_categ ( GtkButton * widget, gpointer data )
{
    GtkWidget * fenetre_nom;
    gint resultat;
    gchar *nom_categ;

    fenetre_nom = file_selection_new ( _("Export categories"), FILE_SELECTION_IS_SAVE_DIALOG );
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( fenetre_nom ), 
					  gsb_file_get_last_path () );
    file_selection_set_entry ( GTK_FILE_CHOOSER ( fenetre_nom ), ".cgsb" );
    
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( fenetre_nom );
	return FALSE;
    }

    nom_categ = file_selection_get_filename ( GTK_FILE_CHOOSER ( fenetre_nom ));
    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (fenetre_nom), TRUE));
    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

    gsb_file_others_save_category ( nom_categ );

    return FALSE;
}



/**
 *
 *
 */
void importer_categ ( void )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *category_name;
    gint last_transaction_number;
    GtkFileFilter * filter;

    dialog = file_selection_new ( _("Import categories"),
				  FILE_SELECTION_IS_OPEN_DIALOG | FILE_SELECTION_MUST_EXIST);
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), gsb_file_get_last_path () );
    file_selection_set_entry ( GTK_FILE_CHOOSER ( dialog ), ".cgsb" );

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

    category_name = file_selection_get_filename ( GTK_FILE_CHOOSER ( dialog ));
    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE));
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    last_transaction_number = gsb_data_transaction_get_last_number();

    /* on permet de remplacer/fusionner la liste */

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				       make_hint ( _("Merge imported categories with existing?"),
						   ( last_transaction_number ?
						     _("File already contains transactions.  If you decide to continue, existing categories will be merged with imported ones.") :
						     _("File does not contain transactions.  "
						       "If you decide to continue, existing categories will be merged with imported ones.  "
						       "Once performed, there is no undo for this.\n"
						       "You may also decide to replace existing categories with imported ones." ) ) ) );

    if ( !last_transaction_number)
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Replace existing"), 2,
				 NULL );

    gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
			     GTK_STOCK_CANCEL, 0,
			     GTK_STOCK_OK, 1,
			     NULL );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    switch ( resultat )
    {
	case 2 :
	    /* we want to replace the list */

	    if ( !last_transaction_number )
		gsb_data_category_init_variables ();

        case 1 :
	    if ( !gsb_file_others_load_category ( category_name ))
	    {
		return;
	    }
	    break;

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
    GtkWidget * handlebox, * hbox2, * button;
    
    /* HandleBox */
    handlebox = gtk_handle_box_new ();
    
    /* Hbox2 */
    hbox2 = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox2 );

    /* New category button */
    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("New\ncategory"), 
					       "new-categ.png", 
					       G_CALLBACK(metatree_new_division),
					       categ_tree_model );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Create a new category"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* New sub category button */
    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("New sub\ncategory"), 
					       "new-sub-categ.png",
					       G_CALLBACK(appui_sur_ajout_sub_division),
					       categ_tree_model );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(categ_tree_model), button, "selection" );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(categ_tree_model), button, "sub-division" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Create a new sub-category"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Import button */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_OPEN, 
					   _("Import"),
					   G_CALLBACK(importer_categ),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Import a Grisbi category file (.cgsb)"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Export button */
    button = gsb_automem_stock_button_new ( etat.display_toolbar, 
					   GTK_STOCK_SAVE, 
					   _("Export"),
					   G_CALLBACK(exporter_categ),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Export a Grisbi category file (.cgsb)"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Delete button */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_DELETE, _("Delete"),
					   G_CALLBACK(supprimer_division), arbre_categ );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(categ_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Delete selected category"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Properties button */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_PROPERTIES, _("Properties"),
					   G_CALLBACK(edit_category), arbre_categ );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(categ_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Edit selected category"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* View button */
    button = gsb_automem_stock_button_menu_new ( etat.display_toolbar,
						GTK_STOCK_SELECT_COLOR, 
						_("View"),
						G_CALLBACK(popup_category_view_mode_menu),
						NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Change view mode"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    gtk_widget_show_all ( handlebox );

    metatree_set_linked_widgets_sensitive ( GTK_TREE_MODEL(categ_tree_model),
					    FALSE, "selection" );

    return ( handlebox );
}



/**
 * Popup a menu that allow changing the view mode of the category
 * metatree.
 *
 * \param button	Button that triggered the signal.
 *
 * \return		FALSE
 */
gboolean popup_category_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Category view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 0 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_append ( menu, menu_item );

    menu_item = gtk_image_menu_item_new_with_label ( _("Subcategory view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 1 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_append ( menu, menu_item );

    menu_item = gtk_image_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 2 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_append ( menu, menu_item );

    gtk_widget_show_all ( menu );

    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}



/**
 *
 *
 */
gboolean edit_category ( GtkTreeView * view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry, *hbox, *radiogroup;
    gint category_number = -1, sub_category_number = -1;
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gchar * title;

    /* fill category_number and sub_category_number */

    selection = gtk_tree_view_get_selection ( view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( model, &iter, 
			     META_TREE_NO_DIV_COLUMN, &category_number,
			     META_TREE_NO_SUB_DIV_COLUMN, &sub_category_number,
			     -1 );
    }

    if ( !selection || category_number <= 0 )
	return FALSE;

    if ( sub_category_number > 0 )
	title = g_strdup_printf ( _("Properties for %s"), gsb_data_category_get_sub_category_name ( category_number,
												    sub_category_number,
												    _("No sub-category defined" )));
    else
	title = g_strdup_printf ( _("Properties for %s"), gsb_data_category_get_name ( category_number,
										       0,
										       _("No category defined") ));

    dialog = gtk_dialog_new_with_buttons ( title, GTK_WINDOW (window), GTK_DIALOG_MODAL,
					   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0 );
    paddingbox = new_paddingbox_with_title ( hbox, TRUE, title );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(paddingbox), 6 );

    table = gtk_table_new ( 0, 2, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 6 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    /* Name entry */
    label = gtk_label_new ( _("Name"));
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    if ( sub_category_number > 0 )
    {
	/* we have a problem because gsb_autofunc_entry_new need a function gsb_data_..._set_... with 2 args,
	 * but gsb_data_category_set_sub_category_name has 3 args, need to use hook
	 * the hook function will receive the sub_category number, and the category_number will be associated
	 * automatickly with the entry by autofunc_entry_new */
	entry = gsb_autofunc_entry_new ( gsb_data_category_get_sub_category_name ( category_number,
										   sub_category_number,
										   NULL ),
					 G_CALLBACK (gsb_category_page_sub_entry_changed), GINT_TO_POINTER (sub_category_number),
					 NULL, category_number);
    }
    else
	entry = gsb_autofunc_entry_new ( gsb_data_category_get_name ( category_number, 0, NULL ),
					 NULL, NULL,
					 G_CALLBACK (gsb_data_category_set_name), category_number );

    gtk_widget_set_usize ( entry, 400, 0 );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    if ( sub_category_number <= 0 )
    {
	/* Description entry */
	label = gtk_label_new ( _("Type"));
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
	gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
			   GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

	radiogroup = gsb_autofunc_radiobutton_new (  _("Credit"), _("Debit"),
						     gsb_data_category_get_type (category_number),
						     NULL, NULL,
						     G_CALLBACK (gsb_data_category_set_type), category_number );
	gtk_table_attach ( GTK_TABLE(table), radiogroup, 
			   1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    }

    gtk_widget_show_all ( dialog );
    free ( title );

    gtk_dialog_run ( GTK_DIALOG(dialog) );
    /* changes are done automatickly, so just close the dialog */
    gtk_widget_destroy ( dialog );

    if ( sub_category_number > 0 )
    {
	fill_sub_division_row ( model, category_interface,
				get_iter_from_div ( model, category_number, sub_category_number ), 
				gsb_data_category_get_structure ( category_number ),
				gsb_data_category_get_sub_category_structure ( category_number,
									       sub_category_number));
    }
    else
    {
	fill_division_row ( model, category_interface,
			    get_iter_from_div ( model, category_number, -1 ),
			    gsb_data_category_get_structure ( category_number ));
    }

    /* update the transactions list */
    gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_CATEGORY);

    return TRUE;
}


/**
 * called when there is a change in the sub-category entry to modify it
 * cannot use the autofunc function because 2 arguments : categ_number and
 * sub_categ_number, so come here
 *
 * \param entry the GtkEntry wich contains the sub-category
 * \param sub_categ_tmp a pointer wich is the sub_category_number
 *
 * \return FALSE 
 * */
gboolean gsb_category_page_sub_entry_changed ( GtkWidget *entry,
					       gint *sub_categ_tmp )
{
    gint category_number;
    gint sub_category_number;

    if (!entry)
	return FALSE;

    category_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry),
							  "number_for_func" ));
    sub_category_number = GPOINTER_TO_INT (sub_categ_tmp);
    gsb_data_category_set_sub_category_name ( category_number,
					      sub_category_number,
					      gtk_entry_get_text (GTK_ENTRY (entry)));
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
