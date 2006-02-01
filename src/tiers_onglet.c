/* fichier qui s'occupe de l'onglet de gestion du tiers */
/*           tiers_onglet.c */

/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include "include.h"




/*START_INCLUDE*/
#include "tiers_onglet.h"
#include "erreur.h"
#include "metatree.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "gsb_form.h"
#include "utils_editables.h"
#include "gtk_combofix.h"
#include "utils_buttons.h"
#include "utils.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *creation_barre_outils_tiers ( void );
static gboolean edit_payee ( GtkTreeView * view );
static gboolean payee_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
			       GtkSelectionData * selection_data );
static gboolean popup_payee_view_mode_menu ( GtkWidget * button );
/*END_STATIC*/



#include "xpm/book-closed.xpm"
#include "xpm/book-open.xpm"



gint mise_a_jour_combofix_tiers_necessaire;

GtkWidget *arbre_tiers;
GtkWidget *entree_nom_tiers;
GtkWidget *text_box;
GtkWidget *bouton_modif_tiers_modifier;
GtkWidget *bouton_modif_tiers_annuler;
GtkWidget *bouton_supprimer_tiers;
GdkPixmap *pixmap_ouvre;
GdkBitmap *masque_ouvre;
GdkPixmap *pixmap_ferme;
GdkBitmap *masque_ferme;
GtkWidget *bouton_ajouter_tiers;

gint no_devise_totaux_tiers;

GtkWidget *payee_tree;
GtkTreeStore *payee_tree_model;




/*START_EXTERN*/
extern MetatreeInterface * payee_interface ;
extern GtkTreeSelection * selection;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *window;
/*END_EXTERN*/




/* **************************************************************************************************** */
/* Fonction onglet_tiers : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_tiers ( void )
{
    GtkWidget *onglet, *scroll_window;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };


    /* création des pixmaps pour la liste */
    pixmap_ouvre = gdk_pixmap_create_from_xpm_d ( GTK_WIDGET(window) -> window,
						  &masque_ouvre, NULL, 
						  (gchar **) book_open_xpm );
    pixmap_ferme = gdk_pixmap_create_from_xpm_d ( GTK_WIDGET(window) -> window,
						  &masque_ferme, NULL, 
						  (gchar **) book_closed_xpm );

    /* création de la fenêtre qui sera renvoyée */

    onglet = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_show ( onglet );

    /* We create the gtktreeview and model early so that they can be referenced. */
    payee_tree = gtk_tree_view_new();
    payee_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, META_TREE_COLUMN_TYPES );

    /* on y ajoute la barre d'outils */
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 creation_barre_outils_tiers(),
			 FALSE,
			 FALSE,
			 0 );

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
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(payee_tree_model), 
				      META_TREE_TEXT_COLUMN, metatree_sort_column,
				      NULL, NULL );
    g_object_set_data ( G_OBJECT ( payee_tree_model), "metatree-interface", 
			payee_interface );

    /* Create container + TreeView */
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (payee_tree), TRUE);
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
    g_object_set_data ( G_OBJECT(payee_tree_model), "tree-view", 
			payee_tree );

    /* Make category column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Payees"), cell, 
						       "text", META_TREE_TEXT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
#if GTK_CHECK_VERSION(2,4,0)
    gtk_tree_view_column_set_expand ( column, TRUE );
#endif
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( payee_tree ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make account column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Account", cell, 
						       "text", META_TREE_ACCOUNT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( payee_tree ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make balance column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Balance", cell, 
						       "text", META_TREE_BALANCE_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
						       "xalign", META_TREE_XALIGN_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( payee_tree ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    gtk_container_add ( GTK_CONTAINER ( scroll_window ), payee_tree );
    gtk_widget_show ( payee_tree );

    /* Connect to signals */
    g_signal_connect ( G_OBJECT(payee_tree), "row-expanded", 
		       G_CALLBACK(division_column_expanded), NULL );
    g_signal_connect( G_OBJECT(payee_tree), "row-activated",
		      G_CALLBACK(division_activated), NULL);

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

    g_signal_connect ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(payee_tree)),
		       "changed", G_CALLBACK(metatree_selection_changed),
		       payee_tree_model );

    return ( onglet );
}



GtkWidget *creation_barre_outils_tiers ( void )
{
    GtkWidget *hbox, *handlebox, *button;

    /* HandleBox */
    handlebox = gtk_handle_box_new ();

    /* Hbox */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox );

    /* Add various icons */
    button = new_button_with_label_and_image ( etat.display_toolbar,
					       _("New payee"), "new-payee.png",
					       G_CALLBACK(metatree_new_division),
					       payee_tree_model );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Create a new payee"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, TRUE, 0 );

    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_DELETE, _("Delete"),
					   G_CALLBACK(supprimer_division),
					   payee_tree );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL (payee_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Delete selected payee"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, TRUE, 0 );

    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_PROPERTIES, _("Properties"),
					   G_CALLBACK(edit_payee), 
					   payee_tree );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL (payee_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Edit selected payee"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, TRUE, 0 );

    button = new_stock_button_with_label_menu ( etat.display_toolbar,
						GTK_STOCK_SELECT_COLOR, 
						_("View"),
						G_CALLBACK(popup_payee_view_mode_menu),
						NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Change view mode"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, TRUE, 0 );

    gtk_widget_show_all ( handlebox );

    metatree_set_linked_widgets_sensitive ( GTK_TREE_MODEL(payee_tree_model), 
					    FALSE, "selection" );

    return ( handlebox );
}



/** 
 * TODO: document this
 */
gboolean popup_payee_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Payee view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 0 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", payee_tree );
    gtk_menu_append ( menu, menu_item );

    menu_item = gtk_image_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 2 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", payee_tree );
    gtk_menu_append ( menu, menu_item );

    gtk_widget_show_all ( menu );

    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}



/* **************************************************************************************************** */
/* Fonction remplit_arbre_tiers */
/* prend en argument le clist arbre_tiers, */
/* le vide et le remplit */
/* **************************************************************************************************** */

void remplit_arbre_tiers ( void )
{
    GSList *payee_list_tmp;
    GtkTreeIter iter_payee;

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (payee_tree_model) );

    /* Compute payee balances. */
    gsb_data_payee_update_counters ();

    /** Then, populate tree with payee. */

    payee_list_tmp = gsb_data_payee_get_payees_list ();
    payee_list_tmp = g_slist_prepend ( payee_list_tmp, NULL );

    while ( payee_list_tmp )
    {
	gpointer payee;

	payee = payee_list_tmp -> data;

	gtk_tree_store_append (GTK_TREE_STORE (payee_tree_model), &iter_payee, NULL);
	fill_division_row ( GTK_TREE_MODEL(payee_tree_model), payee_interface, 
			    &iter_payee, payee );

	payee_list_tmp = payee_list_tmp -> next;
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
gboolean payee_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
			       GtkSelectionData * selection_data )
{
    if ( path )
    {
	gtk_tree_set_row_drag_data (selection_data, GTK_TREE_MODEL(payee_tree_model), path);
    }
    
    return FALSE;
}



gint classement_alphabetique_tree ( GtkWidget *tree,
				    GtkCListRow *ligne_1,
				    GtkCListRow *ligne_2 )
{
    return g_utf8_collate ( ligne_1->cell->u.text, ligne_2->cell->u.text );
}





/**
 *
 *
 */
gboolean edit_payee ( GtkTreeView * view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry_name, *entry_description, *hbox, *scrolled_window;
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gint no_division = -1;
    gpointer payee = NULL;
    gint payee_number;
    gchar * title;

    selection = gtk_tree_view_get_selection ( view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( model, &iter, 
			     META_TREE_POINTER_COLUMN, &payee,
			     META_TREE_NO_DIV_COLUMN, &no_division,
			     -1 );
    }

    if ( !selection || no_division <= 0 )
	return FALSE;

    /* FIXME : should set the number of payee in the list, not the address */

    payee_number = gsb_data_payee_get_no_payee (payee);

    title = g_strdup_printf ( _("Properties for %s"), gsb_data_payee_get_name(payee_number,
									 TRUE));
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

    entry_name = gtk_entry_new ();
    gtk_entry_set_text ( GTK_ENTRY ( entry_name ),
			 gsb_data_payee_get_name(payee_number,
					    TRUE));
    gtk_widget_set_usize ( entry_name, 400, 0 );
    gtk_table_attach ( GTK_TABLE(table), entry_name, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    /* Description entry */
    label = gtk_label_new ( _("Description"));
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    entry_description = gsb_new_text_view (gsb_data_payee_get_description (payee_number));
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), entry_description );
    gtk_table_attach ( GTK_TABLE(table), scrolled_window, 
		       1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    gtk_widget_show_all ( dialog );
    free ( title );

    gtk_dialog_run ( GTK_DIALOG(dialog) );

    /* get the name */
    gsb_data_payee_set_name( payee_number,
			gtk_entry_get_text ( GTK_ENTRY (entry_name)));

    /* get the description */
    gsb_data_payee_set_description ( payee_number,
				gsb_text_view_get_content ( entry_description ));

    gtk_widget_destroy ( dialog );

    mise_a_jour_combofix_tiers ();

    fill_division_row ( model, payee_interface,
			get_iter_from_div ( model, payee_number, -1 ), payee );

    return TRUE;
}




/* ***************************************************************************************************** */
/* Fonction mise_a_jour_combofix_tiers */
/* recrée la liste des combofix et l'applique à tous les combofix du tiers */
/* ***************************************************************************************************** */
void mise_a_jour_combofix_tiers ( void )
{
    devel_debug ( "mise_a_jour_combofix_tiers" );

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_PARTY ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY,
									    gsb_form_get_account_number_from_origin (gsb_form_get_origin ())) ),
				gsb_data_payee_get_name_and_report_list ());

    mise_a_jour_combofix_tiers_necessaire = 0;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
