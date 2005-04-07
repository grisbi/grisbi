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
#include "echeancier_formulaire_constants.h"
#include "operations_formulaire_constants.h"




/*START_INCLUDE*/
#include "tiers_onglet.h"
#include "metatree.h"
#include "utils_tiers.h"
#include "gtk_combofix.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_editables.h"
#include "etats_config.h"
#include "affichage_formulaire.h"
#include "operations_formulaire.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *creation_barre_outils_tiers ( void );
static gboolean edit_payee ( GtkWidget * button, GtkTreeView * view );
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

gint nb_enregistrements_tiers;
gint no_dernier_tiers;

/* contient la liste des struct liste_tiers de tous les tiers */

GSList *liste_struct_tiers;

/* liste des tiers pour le combofix */

GSList *liste_tiers_combofix;

/* liste des tiers pour le combofix de l'échéancier */

GSList *liste_tiers_combofix_echeancier;

gint no_devise_totaux_tiers;

GtkWidget *payee_tree;
GtkTreeStore *payee_tree_model;




/*START_EXTERN*/
extern struct struct_etat *etat_courant;
extern GtkWidget *formulaire;
extern GSList *liste_struct_etats;
extern GtkTreeStore *model;
extern gint modif_tiers;
extern MetatreeInterface * payee_interface ;
extern GtkTreeSelection * selection;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/*END_EXTERN*/




/* **************************************************************************************************** */
/* Fonction onglet_tiers : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_tiers ( void )
{
    GtkWidget *onglet, *scroll_window, *scrolled_window_text;
    GtkWidget *vbox, *vbox_frame, *hbox;
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

    onglet = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10 );
    gtk_widget_show ( onglet );

    /* We create the gtktreeview and model early so that they can be referenced. */
    payee_tree = gtk_tree_view_new();
    payee_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, 
					    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
					    G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT, 
					    G_TYPE_INT, G_TYPE_FLOAT );

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

    /* la 1ère fois qu'on affichera les tiers, il faudra remplir la liste */
    modif_tiers = 1;

    return ( onglet );
}



GtkWidget *creation_barre_outils_tiers ( void )
{
    GtkWidget *hbox, *separateur, *handlebox, *hbox2;

    hbox = gtk_hbox_new ( FALSE, 5 );

    /* HandleBox */
    handlebox = gtk_handle_box_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), handlebox, FALSE, FALSE, 0 );
    /* Hbox2 */
    hbox2 = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox2 );

    /* Add various icons */
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_button_with_label_and_image ( GSB_BUTTON_ICON,
							   _("New payee"), "new-payee.png",
							   G_CALLBACK(appui_sur_ajout_division),
							   payee_tree_model ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GSB_BUTTON_ICON,
						       GTK_STOCK_DELETE, 
						       _("Delete"),
						       G_CALLBACK(supprimer_division),
						       payee_tree ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), /* FIXME: write the property dialog */
			 new_stock_button_with_label ( GSB_BUTTON_ICON,
						       GTK_STOCK_PROPERTIES, 
						       _("Properties"),
						       G_CALLBACK(edit_payee), 
						       payee_tree ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label_menu ( GSB_BUTTON_ICON,
							    GTK_STOCK_SELECT_COLOR, 
							    _("View"),
							    G_CALLBACK(popup_payee_view_mode_menu),
							    NULL ),
			 FALSE, TRUE, 0 );

    /* Vertical separator */
    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), separateur, FALSE, FALSE, 0 );
    gtk_widget_show_all ( hbox );

    return ( hbox );
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
    GSList *liste_payee_tmp;
    GtkTreeIter iter_payee;

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (payee_tree_model) );

    /* Compute payee balances. */
    calcule_total_montant_payee ();

    /** Then, populate tree with payee. */
    liste_payee_tmp = g_slist_prepend ( liste_struct_tiers, NULL );
    while ( liste_payee_tmp )
    {
	struct struct_tiers *payee;

	payee = liste_payee_tmp -> data;

	gtk_tree_store_append (GTK_TREE_STORE (payee_tree_model), &iter_payee, NULL);
	fill_division_row ( GTK_TREE_MODEL(payee_tree_model), payee_interface, 
			    &iter_payee, payee );

	liste_payee_tmp = liste_payee_tmp -> next;
    }

    modif_tiers = 0;
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




void creation_liste_tiers_combofix ( void )
{
    GSList *pointeur;
    GSList *liste_tmp;

    /* on commence à créer les 2 listes semblables de tous les tiers */
    /*   celle du formulaire est de type complex, cad qu'elle contiendra 2 listes : */
    /* les tiers et les états sélectionnés */

    pointeur = liste_struct_tiers;
    liste_tiers_combofix = NULL;
    liste_tiers_combofix_echeancier = NULL;
    liste_tmp = NULL;

    while ( pointeur )
    {
	liste_tmp = g_slist_append ( liste_tmp,
				     ((struct struct_tiers * )( pointeur -> data )) -> nom_tiers );
	liste_tiers_combofix_echeancier = g_slist_append ( liste_tiers_combofix_echeancier,
							   ((struct struct_tiers * )( pointeur -> data )) -> nom_tiers );
	pointeur = pointeur -> next;
    }

    /* on ajoute liste tmp à liste_tiers_combofix */

    liste_tiers_combofix = g_slist_append ( liste_tiers_combofix,
					    liste_tmp );

    /* on fait maintenant le tour des états pour rajouter ceux qui ont été sélectionnés */

    liste_tmp = NULL;
    pointeur = liste_struct_etats;

    while ( pointeur )
    {
	struct struct_etat *etat;

	etat = pointeur -> data;

	if ( etat -> inclure_dans_tiers )
	{
	    if ( liste_tmp )
		liste_tmp = g_slist_append ( liste_tmp,
					     g_strconcat ( "\t",
							   g_strdup ( etat -> nom_etat ),
							   NULL ));
	    else
	    {
		liste_tmp = g_slist_append ( liste_tmp,
					     _("Report"));
		liste_tmp = g_slist_append ( liste_tmp,
					     g_strconcat ( "\t",
							   g_strdup ( etat -> nom_etat ),
							   NULL ));
	    }
	}
	pointeur = pointeur -> next;
    }

    /* on ajoute liste tmp à liste_tiers_combofix */

    liste_tiers_combofix = g_slist_append ( liste_tiers_combofix,
					    liste_tmp );
}



/**
 *
 *
 */
gboolean edit_payee ( GtkWidget * button, GtkTreeView * view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry, *hbox, *scrolled_window;
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gint no_division = -1;
    struct struct_tiers * payee = NULL;
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

    title = g_strdup_printf ( _("Properties for %s"), payee -> nom_tiers);
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

    entry = new_text_entry ( &(payee -> nom_tiers), NULL );
    gtk_widget_set_usize ( entry, 400, 0 );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    /* Description entry */
    label = gtk_label_new ( _("Description"));
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    entry = new_text_area ( &(payee -> texte), NULL );
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), entry );
    gtk_table_attach ( GTK_TABLE(table), scrolled_window, 
		       1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    gtk_widget_show_all ( dialog );
    free ( title );

    gtk_dialog_run ( GTK_DIALOG(dialog) );
    gtk_widget_destroy ( dialog );

    mise_a_jour_combofix_tiers ();

    fill_division_row ( model, payee_interface,
			get_iter_from_div ( model, payee -> no_tiers, -1 ), payee );

    return TRUE;
}




/* ***************************************************************************************************** */
/* Fonction mise_a_jour_combofix_tiers */
/* recrée la liste des combofix et l'applique à tous les combofix du tiers */
/* ***************************************************************************************************** */
void mise_a_jour_combofix_tiers ( void )
{
    if ( DEBUG )
	printf ( "mise_a_jour_combofix_tiers\n" );

    creation_liste_tiers_combofix ();

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_PARTY )
	 &&
	 GTK_IS_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY) ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY) ),
				liste_tiers_combofix,
				TRUE,
				TRUE );
    if ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
				liste_tiers_combofix_echeancier,
				FALSE,
				TRUE );

    /* FIXME : ça ne devrait pas se trouver dans cette fonction */

    if ( etat_courant )
    {
	remplissage_liste_tiers_etats ();
	selectionne_liste_tiers_etat_courant ();
    }

    mise_a_jour_combofix_tiers_necessaire = 0;
    modif_tiers = 1;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
