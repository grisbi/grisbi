/* ************************************************************************** */
/* fichier qui s'occupe de l'onglet de gestion des imputations                */
/* 			imputation_budgetaire.c                               */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
/*			http://www.grisbi.org   			      */
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
#include "imputation_budgetaire.h"
#include "metatree.h"
#include "erreur.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "gsb_data_budget.h"
#include "gsb_data_transaction.h"
#include "gsb_file_others.h"
#include "gtk_combofix.h"
#include "main.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_editables.h"
#include "gsb_form_config.h"
#include "operations_formulaire.h"
#include "echeancier_formulaire.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean budgetary_line_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
					GtkSelectionData * selection_data );
static GtkWidget *creation_barre_outils_ib ( void );
static gboolean edit_budgetary_line ( GtkTreeView * view );
static void exporter_ib ( void );
static void importer_ib ( void );
static gboolean popup_budgetary_line_view_mode_menu ( GtkWidget * button );
/*END_STATIC*/


GtkWidget *budgetary_line_tree;
GtkTreeStore *budgetary_line_tree_model;

GtkWidget *arbre_imputation;
GtkWidget *entree_nom_imputation;
GtkWidget *bouton_imputation_debit;
GtkWidget *bouton_imputation_credit;
GtkWidget *bouton_modif_imputation_modifier;
GtkWidget *bouton_modif_imputation_annuler;
GtkWidget *bouton_supprimer_imputation;
GtkWidget *bouton_ajouter_imputation;
GtkWidget *bouton_ajouter_sous_imputation;

gint mise_a_jour_combofix_imputation_necessaire;
gint no_devise_totaux_ib;


/*START_EXTERN*/
extern MetatreeInterface * budgetary_interface ;
extern gchar *dernier_chemin_de_travail;
extern GtkTreeStore *model;
extern gint modif_imputation;
extern GtkTreeSelection * selection;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/*END_EXTERN*/





/* **************************************************************************************************** */
/* Fonction onglet_imputations : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_imputations ( void )
{
    GtkWidget *scroll_window;
    GtkWidget *vbox;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };


    /* We create the gtktreeview and model early so that they can be referenced. */
    budgetary_line_tree = gtk_tree_view_new();
    budgetary_line_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, 
						     META_TREE_COLUMN_TYPES );

    /* We create the main vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_show ( vbox );

    /* on y ajoute la barre d'outils */
    gtk_box_pack_start ( GTK_BOX ( vbox ), creation_barre_outils_ib(), FALSE, FALSE, 0 );

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
			budgetary_interface );

    /* Create container + TreeView */
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (budgetary_line_tree), TRUE);
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
    g_object_set_data ( G_OBJECT(budgetary_line_tree_model), "tree-view", 
			budgetary_line_tree );

    /* Make category column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Budgetary line"), cell, 
						       "text", META_TREE_TEXT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
#if GTK_CHECK_VERSION(2,4,0)
    gtk_tree_view_column_set_expand ( column, TRUE );
#endif
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( budgetary_line_tree ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make account column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Account"), cell, 
						       "text", META_TREE_ACCOUNT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( budgetary_line_tree ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make balance column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Balance"), cell, 
						       "text", META_TREE_BALANCE_COLUMN,
						       "weight", META_TREE_FONT_COLUMN,
						       "xalign", META_TREE_XALIGN_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( budgetary_line_tree ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    gtk_container_add ( GTK_CONTAINER ( scroll_window ), budgetary_line_tree );
    gtk_widget_show ( budgetary_line_tree );

    /* Connect to signals */
    g_signal_connect ( G_OBJECT(budgetary_line_tree), "row-expanded", 
		       G_CALLBACK(division_column_expanded), NULL );
    g_signal_connect( G_OBJECT(budgetary_line_tree), "row-activated",
		      G_CALLBACK(division_activated), NULL);

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

    /* la 1ère fois qu'on affichera les catég, il faudra remplir la liste */

    modif_imputation = 1;

    return ( vbox );
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* Fonction remplit_arbre_imputation */
/* le vide et le remplit */
/* **************************************************************************************************** */

void remplit_arbre_imputation ( void )
{
    GSList *budget_list;
    GtkTreeIter iter_budgetary_line, iter_sub_budgetary_line;

    devel_debug ( "remplit_arbre_imputation" );

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (budgetary_line_tree_model));

    /* Compute budget balances. */
    gsb_data_budget_update_counters ();

    /** Then, populate tree with budgetary lines. */

    budget_list = gsb_data_budget_get_budgets_list ();
    budget_list = g_slist_prepend ( budget_list, NULL );

    while ( budget_list )
    {
	gint budget_number;

	budget_number = gsb_data_budget_get_no_budget (budget_list -> data);

	gtk_tree_store_append (GTK_TREE_STORE (budgetary_line_tree_model), &iter_budgetary_line, NULL);
	fill_division_row ( GTK_TREE_MODEL(budgetary_line_tree_model), budgetary_interface, 
			    &iter_budgetary_line, gsb_data_budget_get_structure (budget_number));

	/** Each budget has sub budgetary lines. */
	if ( budget_number )
	{
	    GSList *sub_budget_list;

	    sub_budget_list = gsb_data_budget_get_sub_budget_list ( budget_number );

	    while ( sub_budget_list )
	    {
		gint sub_budget_number;

		sub_budget_number = gsb_data_budget_get_no_sub_budget (sub_budget_list -> data);

		gtk_tree_store_append (GTK_TREE_STORE (budgetary_line_tree_model), 
				       &iter_sub_budgetary_line, &iter_budgetary_line);
		fill_sub_division_row ( GTK_TREE_MODEL(budgetary_line_tree_model), budgetary_interface, 
					&iter_sub_budgetary_line,
					gsb_data_budget_get_structure (budget_number),
					gsb_data_budget_get_sub_budget_structure( budget_number,
										  sub_budget_number));

		sub_budget_list = sub_budget_list -> next;
	    }
	}

	gtk_tree_store_append (GTK_TREE_STORE (budgetary_line_tree_model), 
			       &iter_sub_budgetary_line, &iter_budgetary_line);
	fill_sub_division_row ( GTK_TREE_MODEL(budgetary_line_tree_model), budgetary_interface, 
				&iter_sub_budgetary_line, gsb_data_budget_get_structure (budget_number), NULL );
	
	budget_list = budget_list -> next;
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





/***********************************************************************************************************/
/* Fonction mise_a_jour_combofix_imputation */
/* recrée les listes de catégories des combofix */
/* et remet les combofix à jour */
/***********************************************************************************************************/

void mise_a_jour_combofix_imputation ( void )
{
    devel_debug ( "mise_a_jour_combofix_imputation" );

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_BUDGET ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_BUDGET) ),
				gsb_data_budget_get_name_list (TRUE, TRUE),
				TRUE,
				TRUE );

    gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
			    gsb_data_budget_get_name_list (TRUE, TRUE),
			    TRUE,
			    TRUE );

    mise_a_jour_combofix_imputation_necessaire = 0;
    modif_imputation = 1;
}






/**
 *
 *
 */
void exporter_ib ( void )
{
    GtkWidget *fenetre_nom;
    gint resultat;
    gchar *nom_ib;

    fenetre_nom = file_selection_new (  _("Export the budgetary lines"),FILE_SELECTION_IS_SAVE_DIALOG);
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				      dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( fenetre_nom ), ".igsb" );
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_ib =file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* vérification que c'est possible est faite par la boite de dialogue */

	    if ( !gsb_file_others_save_budget ( nom_ib ))
	    {
		dialogue_error ( _("Cannot save file.") );
		return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return;
    }
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void importer_ib ( void )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *budget_name;
    gint last_transaction_number;

    dialog = file_selection_new ( _("Import budgetary lines"),
				  FILE_SELECTION_IS_OPEN_DIALOG | FILE_SELECTION_MUST_EXIST);
    file_selection_set_filename ( GTK_FILE_SELECTION ( dialog ), dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( dialog ), ".igsb" );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat != GTK_RESPONSE_OK  )
    {
	gtk_widget_destroy ( dialog );
	return;
    }

    budget_name = file_selection_get_filename ( GTK_FILE_SELECTION ( dialog ));
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    last_transaction_number = gsb_data_transaction_get_last_number();

    /* on permet de remplacer/fusionner la liste */

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				       make_hint ( _("Merge imported budgetary lines with existing?"),
						   ( last_transaction_number ?
						     _("File already contains transactions.  If you decide to continue, existing categories will be merged with imported ones.") :
						     _("File does not contain transactions.  "
						       "If you decide to continue, existing budgetary lines will be merged with imported ones.  "
						       "Once performed, there is no undo for this.\n"
						       "You may also decide to replace existing budgetary lines with imported ones." ) ) ) );

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
		gsb_data_budget_init_variables ();

        case 1 :
	    if ( !gsb_file_others_load_budget ( budget_name ))
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
GtkWidget *creation_barre_outils_ib ( void )
{
    GtkWidget * handlebox, * hbox2, * button;

    /* HandleBox */
    handlebox = gtk_handle_box_new ();

    /* Hbox2 */
    hbox2 = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox2 );

    /* New budgetary line button */
    button = new_button_with_label_and_image ( etat.display_toolbar,
					       _("New\nbudgetary line"), "new-ib.png",
					       G_CALLBACK(metatree_new_division),
					       budgetary_line_tree_model );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Create a new budgetary line"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* New sub budgetary line button */
    button = new_button_with_label_and_image ( etat.display_toolbar,
					       _("New sub\nbudgetary line"), 
					       "new-sub-ib.png",
					       G_CALLBACK(appui_sur_ajout_sub_division),
					       budgetary_line_tree_model );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(budgetary_line_tree_model), button, "selection" );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(budgetary_line_tree_model), button, "sub-division" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Create a new budgetary line"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Import button */
    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_OPEN, 
					   _("Import"),
					   G_CALLBACK(importer_ib),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Import a Grisbi budgetary line file (.igsb)"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Export button */
    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_SAVE, 
					   _("Export"),
					   G_CALLBACK(exporter_ib),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Export a Grisbi budgetary line file (.igsb)"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Delete button */
    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_DELETE, _("Delete"),
					   G_CALLBACK(supprimer_division),
					   budgetary_line_tree );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(budgetary_line_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Delete selected budgetary line"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* Properties button */
    button = new_stock_button_with_label ( etat.display_toolbar,
					   GTK_STOCK_PROPERTIES, _("Properties"),
					   G_CALLBACK(edit_budgetary_line), 
					   budgetary_line_tree );
    metatree_register_widget_as_linked ( GTK_TREE_MODEL(budgetary_line_tree_model), button, "selection" );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Edit selected budgetary line"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    /* View button */
    button = new_stock_button_with_label_menu ( etat.display_toolbar,
						GTK_STOCK_SELECT_COLOR, 
						_("View"),
						G_CALLBACK(popup_budgetary_line_view_mode_menu),
						NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Change display mode"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, TRUE, 0 );

    gtk_widget_show_all ( handlebox );

    metatree_set_linked_widgets_sensitive ( GTK_TREE_MODEL(budgetary_line_tree_model),
					    FALSE, "selection" );

    return ( handlebox );
}



/** 
 * TODO: document this
 */
gboolean popup_budgetary_line_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new ();

    /* Edit transaction */
    menu_item = gtk_image_menu_item_new_with_label ( _("Budgetary line view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 0 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", budgetary_line_tree );
    gtk_menu_append ( menu, menu_item );

    menu_item = gtk_image_menu_item_new_with_label ( _("Sub-budgetary line view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 1 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", budgetary_line_tree );
    gtk_menu_append ( menu, menu_item );

    menu_item = gtk_image_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_division), (gpointer) 2 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", budgetary_line_tree );
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
gboolean edit_budgetary_line ( GtkTreeView * view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry, *hbox, *radiogroup;
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gint budget_number = -1, sub_budget_number = -1;
    gchar * title;

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

    if ( sub_budget_number )
    {
	gchar *sub_budget_name;

	sub_budget_name = gsb_data_budget_get_sub_budget_name ( budget_number,
								sub_budget_number,
								"" );
	entry = new_text_entry ( &sub_budget_name, NULL, NULL );
    }
    else
    {
	gchar *budget_name;

	budget_name = gsb_data_budget_get_name ( budget_number,
						 0,
						 "" );
	entry = new_text_entry ( &budget_name, NULL, NULL );
    }

    gtk_widget_set_usize ( entry, 400, 0 );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    if ( !sub_budget_number )
    {
	gint type;

	/* Description entry */
	label = gtk_label_new ( _("Type"));
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
	gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
			   GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
	/* FIXME : must use other than new_radiogroup because &gsb_data_budget_get_type (budget_number) don't compile */
	type = gsb_data_budget_get_type (budget_number);
	radiogroup = new_radiogroup ( _("Credit"), _("Debit"), 
				      &type, NULL );
	gtk_table_attach ( GTK_TABLE(table), radiogroup, 
			   1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    }

    gtk_widget_show_all ( dialog );
    free ( title );

    gtk_dialog_run ( GTK_DIALOG(dialog) );
    gtk_widget_destroy ( dialog );

    mise_a_jour_combofix_imputation ();

    if ( sub_budget_number )
    {
	fill_sub_division_row ( model, budgetary_interface,
				get_iter_from_div ( model, budget_number, sub_budget_number ), 
				gsb_data_budget_get_structure ( budget_number ),
				gsb_data_budget_get_sub_budget_structure ( budget_number,
									   sub_budget_number));
    }
    else
    {
	fill_division_row ( model, budgetary_interface,
			    get_iter_from_div ( model, budget_number, -1 ),
			    gsb_data_budget_get_structure ( budget_number ));
    }

    return TRUE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
