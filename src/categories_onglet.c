/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "metatree.h"
#include "erreur.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_data_transaction.h"
#include "gsb_file_others.h"
#include "gsb_form.h"
#include "navigation.h"
#include "gtk_combofix.h"
#include "main.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_editables.h"
#include "etats_config.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean categ_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
			       GtkSelectionData * selection_data );
static GtkWidget *creation_barre_outils_categ ( void );
static gboolean edit_category ( GtkTreeView * view );
static gboolean exporter_categ ( GtkButton * widget, gpointer data );
static void importer_categ ( void );
static gboolean popup_category_view_mode_menu ( GtkWidget * button );
/*END_STATIC*/


/* VARIABLES */

gchar *categories_de_base_debit [] = {
    N_("Food : Bar"),
    N_("Food : Baker"),
    N_("Food : Canteen"),
    N_("Food : Sweets"),
    N_("Food : Misc"),
    N_("Food : Grocery"),
    N_("Food : Restaurant"),
    N_("Food : Self-service"),
    N_("Food : Supermarket"),
    N_("Pets : Food"),
    N_("Pets : Various supplies"),
    N_("Pets : Grooming"),
    N_("Pets : Veterinary surgeon"),
    N_("Insurance : Car"),
    N_("Insurance : Health"),
    N_("Insurance : House"),
    N_("Insurance : Civil liability"),
    N_("Insurance : Life"),
    N_("Car : Fuel"),
    N_("Car : Repairs"),
    N_("Car : Maintenance"),
    N_("Car : Parking"),
    N_("Car : Fines"),
    N_("Misc."),
    N_("Gifts"),
    N_("Children : Nurse"),
    N_("Children : Misc."),
    N_("Children : Studies"),
    N_("Studies : Lessons"),
    N_("Studies : Scool fees"),
    N_("Studies : Books"),
    N_("Miscelanious : Gifts"),
    N_("Financial expenses : Miscelanious"),
    N_("Financial expenses : Bank charges"),
    N_("Financial expenses : Loan/Mortgage"),
    N_("Financial expenses : Charges"),
    N_("Financial expenses : Refunding"),
    N_("Professionnal expenses : Non refundable"),
    N_("Professionnal expenses : Refundable"),
    N_("Taxes : Miscelanious"),
    N_("Taxes : Income"),
    N_("Taxes : Land"),
    N_("Housing : Hotel"),
    N_("Housing : Rent"),
    N_("Housing : TV"),
    N_("Housing : Furnitures"),
    N_("Housing : Charges"),
    N_("Housing : Heating"),
    N_("Housing : Decoration"),
    N_("Housing : Water"),
    N_("Housing : Electricity"),
    N_("Housing : White products"),
    N_("Housing : Equipment"),
    N_("Housing : Gaz"),
    N_("Housing : Garden"),
    N_("Housing : House keeper"),
    N_("Housing : Phone"),
    N_("Housing : Mobile Phone"),
    N_("Leisures : Snooker"),
    N_("Leisures : Bowling"),
    N_("Leisures : Movies"),
    N_("Leisures : Night club"),
    N_("Leisures : IT"),
    N_("Leisures : Games"),
    N_("Leisures : Books"),
    N_("Leisures : Parks"),
    N_("Leisures : Concerts"),
    N_("Leisures : Sports"),
    N_("Leisures : Video"),
    N_("Leisures : Travels"),
    N_("Leisures : Equipment"),
    N_("Leisures : Museums/Exhibitions"),
    N_("Loan/Mortgage : Capital"),
    N_("Health : Insurance"),
    N_("Health : Dentist"),
    N_("Health : Hospital"),
    N_("Health : Kinesitherapist"),
    N_("Health : Doctor"),
    N_("Health : Ophtalmologist"),
    N_("Health : Osteopath"),
    N_("Health : Chemist"),
    N_("Health : Social security"),
    N_("Care : Hairdresser"),
    N_("Care : Clothing"),
    N_("Transport : Bus"),
    N_("Transport : Metro"),
    N_("Transport : Toll"),
    N_("Transport : Train"),
    N_("Transport : Tramway"),
    N_("Transport : Travels"),
    N_("Transport : Train"),
    N_("Hollydays : Housing"),
    N_("Hollydays : Visits"),
    N_("Hollydays : Travels"),
    NULL };

gchar *categories_de_base_credit [] = {
    N_("Other incomes : Unemployment benefit"),
    N_("Other incomes : Family allowance"),
    N_("Other incomes : Tax credit"),
    N_("Other incomes : Gamble"),
    N_("Other incomes : Mutual insurance"),
    N_("Other incomes : Social security"),
    N_("Retirement : Retirement Fund"),
    N_("Retirement : Pension"),
    N_("Retirement : Supplementary pension"),
    N_("Investment incomes : Dividends"),
    N_("Investment incomes : Interests"),
    N_("Investment incomes : Capital gain"),
    N_("Salary : Overtime"),
    N_("Salary : Leave allowance"),
    N_("Salary : Misc. premiums"),
    N_("Salary : Success fee"),
    N_("Salary : Net salary"),
    N_("Misc. incomes : Gifts"),
    N_("Misc. incomes : Refunds"),
    N_("Misc. incomes : Second hand sales"),
    NULL
};

/* widgets */
GtkWidget *entree_nom_categ, *bouton_categ_debit, *bouton_categ_credit;
GtkWidget *bouton_modif_categ_modifier, *bouton_modif_categ_annuler;
GtkWidget *bouton_supprimer_categ, *bouton_ajouter_categorie;
GtkWidget *bouton_ajouter_sous_categorie;

gint mise_a_jour_combofix_categ_necessaire;

/* Category tree model & view */
GtkTreeStore * categ_tree_model;
GtkWidget *arbre_categ;
int no_devise_totaux_categ;


/*START_EXTERN*/
extern MetatreeInterface * category_interface ;
extern gchar *dernier_chemin_de_travail;
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
 * update category combofixes.
 */
void mise_a_jour_combofix_categ ( void )
{
    GSList *list_tmp;
    gint account_number;

    devel_debug ( "mise_a_jour_combofix_categ" );

    account_number = gsb_form_get_account_number ();
    list_tmp = gsb_data_category_get_name_list ( TRUE,
						 TRUE,
						 TRUE,
						 TRUE );

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY,
									    account_number )),
				list_tmp );

    /* FIXME : this should not be in this function */
    if ( gsb_gui_navigation_get_current_report () )
    {
	remplissage_liste_categ_etats ();
	selectionne_liste_categ_etat_courant ();
    }
    mise_a_jour_combofix_categ_necessaire = 0;
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
    file_selection_set_filename ( GTK_FILE_CHOOSER ( fenetre_nom ), 
				  dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_CHOOSER ( fenetre_nom ), ".cgsb" );
    
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( fenetre_nom );
	return FALSE;
    }

    nom_categ = file_selection_get_filename ( GTK_FILE_CHOOSER ( fenetre_nom ));
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
    file_selection_set_filename ( GTK_FILE_CHOOSER ( dialog ), dernier_chemin_de_travail );
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
 * TODO: document this
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
    button = new_button_with_label_and_image ( etat.display_toolbar,
					       _("New\ncategory"), 
					       "new-categ.png", 
					       G_CALLBACK(metatree_new_division),
					       categ_tree_model );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Create a new category"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* New sub category button */
    button = new_button_with_label_and_image ( etat.display_toolbar,
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
    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_OPEN, 
					   _("Import"),
					   G_CALLBACK(importer_categ),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Import a Grisbi category file (.cgsb)"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Export button */
    button = new_stock_button_with_label ( etat.display_toolbar, 
					   GTK_STOCK_SAVE, 
					   _("Export"),
					   G_CALLBACK(exporter_categ),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Export a Grisbi category file (.cgsb)"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Delete button */
    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_DELETE, _("Delete"),
					   G_CALLBACK(supprimer_division), arbre_categ );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(categ_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Delete selected category"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Properties button */
    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_PROPERTIES, _("Properties"),
					   G_CALLBACK(edit_category), arbre_categ );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(categ_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Edit selected category"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* View button */
    button = new_stock_button_with_label_menu ( etat.display_toolbar,
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
 * TODO: document this
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
    gint category_number = -1, sub_category_number = -1, type;
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

    /* FIXME : should not work, replace new_text_entry ? */

    if ( sub_category_number > 0 )
    {
	gchar *sub_category_name;

	sub_category_name = gsb_data_category_get_sub_category_name ( category_number,
								      sub_category_number,
								      "" );
	entry = new_text_entry ( &sub_category_name, NULL, NULL );
    }
    else
    {
	gchar *category_name;
	category_name = gsb_data_category_get_name ( category_number, 0, "" );
	entry = new_text_entry ( &category_name, NULL, NULL );
    }

    gtk_widget_set_usize ( entry, 400, 0 );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    if ( sub_category_number <= 0 )
    {
	/* Description entry */
	label = gtk_label_new ( _("Type"));
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
	gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
			   GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
	/* FIXME : must use other than new_radiogroup because &gsb_data_category_get_type (category_number) don't compile */
	type = gsb_data_category_get_type (category_number);
	radiogroup = new_radiogroup ( _("Credit"), _("Debit"),
				      &type,
				      NULL );
	gtk_table_attach ( GTK_TABLE(table), radiogroup, 
			   1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    }

    gtk_widget_show_all ( dialog );
    free ( title );

    gtk_dialog_run ( GTK_DIALOG(dialog) );
    gtk_widget_destroy ( dialog );

    mise_a_jour_combofix_categ ();

    gsb_data_category_set_type ( category_number, type );

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

    return TRUE;
}




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
