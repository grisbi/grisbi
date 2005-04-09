/* ************************************************************************** */
/* fichier qui s'occupe de l'onglet de gestion des imputations                */
/* 			imputation_budgetaire.c                               */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
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
#include "echeancier_formulaire_constants.h"
#include "operations_formulaire_constants.h"



/*START_INCLUDE*/
#include "imputation_budgetaire.h"
#include "metatree.h"
#include "utils_ib.h"
#include "fichiers_io.h"
#include "utils_devises.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "gsb_transaction_data.h"
#include "gtk_combofix.h"
#include "main.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_editables.h"
#include "parametres.h"
#include "etats_config.h"
#include "affichage_formulaire.h"
#include "operations_formulaire.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean budgetary_line_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
					GtkSelectionData * selection_data );
static GtkWidget *creation_barre_outils_ib ( void );
static gboolean edit_budgetary_line ( GtkTreeView * view );
static void exporter_ib ( void );
static void fusion_categories_imputation ( void );
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

GSList *liste_struct_imputation;    /* liste des structures de catég */
GSList *liste_imputations_combofix;        /*  liste des noms des imputation et sous imputation pour le combofix */
gint nb_enregistrements_imputations;        /* nombre de catégories */
gint no_derniere_imputation;
gint mise_a_jour_combofix_imputation_necessaire;
gint no_devise_totaux_ib;


/*START_EXTERN*/
extern MetatreeInterface * budgetary_interface ;
extern gchar *dernier_chemin_de_travail;
extern struct struct_devise *devise_compte;
extern struct struct_etat *etat_courant;
extern GtkWidget *formulaire;
extern GSList *liste_struct_categories;
extern GtkTreeStore *model;
extern gint modif_imputation;
extern gint no_devise_totaux_tiers;
extern GtkTreeSelection * selection;
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
						     G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
						     G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT, 
						     G_TYPE_INT, G_TYPE_FLOAT );

    /* We create the main vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 6 );
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
    column = gtk_tree_view_column_new_with_attributes (_("Budgetary lines"), cell, 
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
    column = gtk_tree_view_column_new_with_attributes ("Account", cell, 
						       "text", META_TREE_ACCOUNT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( budgetary_line_tree ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make balance column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Balance", cell, 
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
    GSList *liste_budgetary_line_tmp;
    GtkTreeIter iter_budgetary_line, iter_sub_budgetary_line;

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (budgetary_line_tree_model) );

    /** Currency used for totals is then chosen from preferences.  */
    if ( !devise_compte
	 ||
	 devise_compte -> no_devise != no_devise_totaux_tiers )
	devise_compte = devise_par_no ( no_devise_totaux_tiers );

    /* Compute budgetary lines balances. */
    calcule_total_montant_budgetary_line ();

    /** Then, populate tree with budgetary lines. */
    liste_budgetary_line_tmp = g_slist_prepend ( liste_struct_imputation, NULL );
    while ( liste_budgetary_line_tmp )
    {
	struct struct_imputation *budgetary_line;
	GSList *liste_sub_budgetary_line_tmp = NULL;

	budgetary_line = liste_budgetary_line_tmp -> data;

	gtk_tree_store_append (GTK_TREE_STORE (budgetary_line_tree_model), &iter_budgetary_line, NULL);
	fill_division_row ( GTK_TREE_MODEL(budgetary_line_tree_model), budgetary_interface, 
			    &iter_budgetary_line, budgetary_line );

	/** Each budgetary line has sub budgetary lines. */
	if ( budgetary_line )
	    liste_sub_budgetary_line_tmp = budgetary_line -> liste_sous_imputation;

	while ( liste_sub_budgetary_line_tmp )
	{
	    struct struct_sous_imputation *sub_budgetary_line;

	    sub_budgetary_line = liste_sub_budgetary_line_tmp -> data;

	    gtk_tree_store_append (GTK_TREE_STORE (budgetary_line_tree_model), 
				   &iter_sub_budgetary_line, &iter_budgetary_line);
	    fill_sub_division_row ( GTK_TREE_MODEL(budgetary_line_tree_model), budgetary_interface, 
				    &iter_sub_budgetary_line, budgetary_line, sub_budgetary_line );

	    liste_sub_budgetary_line_tmp = liste_sub_budgetary_line_tmp -> next;
	}

	gtk_tree_store_append (GTK_TREE_STORE (budgetary_line_tree_model), 
			       &iter_sub_budgetary_line, &iter_budgetary_line);
	fill_sub_division_row ( GTK_TREE_MODEL(budgetary_line_tree_model), budgetary_interface, 
				&iter_sub_budgetary_line, budgetary_line, NULL );
	
	liste_budgetary_line_tmp = liste_budgetary_line_tmp -> next;
    }

    modif_imputation = 0;
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
/*  Routine qui crée la liste des catégories pour les combofix du formulaire et de la ventilation */
/* c'est à dire 3 listes dans 1 liste : */
/* la première contient les catégories de débit */
/* la seconde contient les catégories de crédit */
/* la troisième contient les catégories spéciales ( virement, retrait, ventilation ) */
/* la ventilation n'apparait pas dans les échéances ( et dans la ventilation ) */
/***********************************************************************************************************/

void creation_liste_imputation_combofix ( void )
{
    GSList *pointeur;
    GSList *liste_imputation_credit;
    GSList *liste_imputation_debit;

    if ( DEBUG )
	printf ( "creation_liste_imputation_combofix\n" );

    liste_imputations_combofix = NULL;
    liste_imputation_credit = NULL;
    liste_imputation_debit = NULL;


    pointeur = liste_struct_imputation;

    while ( pointeur )
    {
	struct struct_imputation *imputation;
	GSList *sous_pointeur;

	imputation = pointeur -> data;

	if ( imputation -> type_imputation )
	    liste_imputation_debit = g_slist_append ( liste_imputation_debit,
						      g_strdup ( imputation -> nom_imputation ) );
	else
	    liste_imputation_credit = g_slist_append ( liste_imputation_credit,
						       g_strdup ( imputation -> nom_imputation ) );

	sous_pointeur = imputation -> liste_sous_imputation;

	while ( sous_pointeur )
	{
	    struct struct_sous_imputation *sous_imputation;

	    sous_imputation = sous_pointeur -> data;

	    if ( imputation -> type_imputation )
		liste_imputation_debit = g_slist_append ( liste_imputation_debit,
							  g_strconcat ( "\t",
									sous_imputation -> nom_sous_imputation,
									NULL ) );
	    else
		liste_imputation_credit = g_slist_append ( liste_imputation_credit,
							   g_strconcat ( "\t",
									 sous_imputation -> nom_sous_imputation,
									 NULL ) );
	    sous_pointeur = sous_pointeur -> next;
	}
	pointeur = pointeur -> next;
    }


    /*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

    liste_imputations_combofix = g_slist_append ( liste_imputations_combofix,
						  liste_imputation_debit );
    liste_imputations_combofix = g_slist_append ( liste_imputations_combofix,
						  liste_imputation_credit );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction mise_a_jour_combofix_imputation */
/* recrée les listes de catégories des combofix */
/* et remet les combofix à jour */
/***********************************************************************************************************/

void mise_a_jour_combofix_imputation ( void )
{
    if ( DEBUG )
	printf ( "mise_a_jour_combofix_imputation\n" );

    creation_liste_imputation_combofix ();

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_BUDGET ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_BUDGET) ),
				liste_imputations_combofix,
				TRUE,
				TRUE );

    gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
			    liste_imputations_combofix,
			    TRUE,
			    TRUE );

    /* FIXME : ça ne devrait pas se trouver dans cette fonction */

    if ( etat_courant )
    {
	remplissage_liste_ib_etats ();
	selectionne_devise_ib_etat_courant ();
    }

    mise_a_jour_combofix_imputation_necessaire = 0;
    modif_imputation = 1;
}
/***********************************************************************************************************/




/***************************************************************************************************/
void fusion_categories_imputation ( void )
{
    /* on fait le tour des catégories et on ajoute aux imputations celles qui n'existent pas */

    GSList *liste_tmp;

    if ( !question ( _("Warning: this will add all the categories and subcategories to the budgetary lines!\nBesides you can't cancel this afterwards.\nWe advise you not to use this unless you know exactly what you are doing.\nDo you want to continue anyway?") ))
	return;

    liste_tmp = liste_struct_categories;

    while ( liste_tmp )
    {
	struct struct_categ *categorie;
	struct struct_imputation *imputation;
	GSList *liste_sous_tmp;

	categorie = liste_tmp -> data;

	/* vérifie si une imputation du nom de la catégorie existe */

	imputation = imputation_par_nom ( categorie -> nom_categ,
					  1,
					  categorie -> type_categ,
					  0 );
	
	if ( imputation )
	{
	    /* on fait maintenant la comparaison avec les sous catég et les sous imputations */

	    liste_sous_tmp = categorie -> liste_sous_categ;

	    while ( liste_sous_tmp )
	    {
		struct struct_sous_categ *sous_categ;
		struct struct_sous_imputation *sous_ib;

		sous_categ = liste_sous_tmp -> data;

		sous_ib = sous_imputation_par_nom ( imputation,
						    sous_categ -> nom_sous_categ,
						    1 );

		liste_sous_tmp = liste_sous_tmp -> next;
	    }
	}
	liste_tmp = liste_tmp -> next;
    }

    /* on met à jour les listes */

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();
    remplit_arbre_imputation ();

    modification_fichier(TRUE);
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

	    if ( !enregistre_ib ( nom_ib ))
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
    GtkWidget *dialog, *fenetre_nom;
    gint resultat;
    gchar *nom_ib;
    gint last_transaction_number;

    fenetre_nom = file_selection_new (_("Import budgetary lines" ),
				      FILE_SELECTION_MUST_EXIST);
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				  dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( fenetre_nom ), ".igsb" );

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( fenetre_nom );
	return;
    }

    nom_ib = file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));
    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

    last_transaction_number = gsb_transaction_data_get_last_number ();

    if ( last_transaction_number )
    {
	/*       il y a déjà des opérations dans le fichier, on ne peut que fusionner */

	resultat = question_yes_no_hint ( _("Merge imported budgetary lines with existing?"),
					  _("File already contains transactions.  If you decide to continue, existing budgetary lines will be merged with imported ones.") );
    }
    else
    {
	/* on permet de remplacer/fusionner la liste */
	dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
					   make_hint ( _("Merge imported budgetary lines with existing?"),
						       _("File does not contain transactions.  "
							 "If you decide to continue, existing budgetary lines will be merged with imported ones.  "
							 "Once performed, there is no undo for this.\n"
							 "You may also decide to replace existing budgetary lines with imported ones.") ) );
	
	gtk_dialog_add_buttons ( GTK_DIALOG ( dialog ), 
				 _("Replace existing"), 2,
				 GTK_STOCK_CANCEL, 0,
				 GTK_STOCK_OK, 1,
				 NULL);
    
	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));
	gtk_widget_destroy ( GTK_WIDGET ( dialog ));
    }

    switch ( resultat )
    {
	case 2 :
	    /* si on a choisi de remplacer l'ancienne liste, on la vire ici */
	    if ( !last_transaction_number )
	    {
		g_slist_free ( liste_struct_imputation );
		liste_struct_imputation = NULL;
		no_derniere_imputation = 0;
		nb_enregistrements_imputations = 0;
	    }

	case 1:
	    if ( !charge_ib ( nom_ib ))
	    {
		dialogue_error ( _("Cannot import file.") );
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
							   _("Budgetary line"), 
							   "new-ib.png",
							   G_CALLBACK(appui_sur_ajout_division),
							   budgetary_line_tree_model ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_button_with_label_and_image ( GSB_BUTTON_ICON,
							   _("Sub-budgetary line"), 
							   "new-sub-ib.png",
							   G_CALLBACK(appui_sur_ajout_sub_division),
							   budgetary_line_tree_model ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GSB_BUTTON_ICON,
						       GTK_STOCK_OPEN, 
						       _("Import"),
						       G_CALLBACK(importer_ib),
						       NULL ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GSB_BUTTON_ICON,
						       GTK_STOCK_SAVE, 
						       _("Export"),
						       G_CALLBACK(exporter_ib),
						       NULL ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GSB_BUTTON_ICON,
						       GTK_STOCK_DELETE, 
						       _("Delete"),
						       G_CALLBACK(supprimer_division),
						       budgetary_line_tree ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), /* FIXME: write the property dialog */
			 new_stock_button_with_label ( GSB_BUTTON_ICON,
						       GTK_STOCK_PROPERTIES, 
						       _("Properties"),
						       G_CALLBACK(edit_budgetary_line), 
						       budgetary_line_tree ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label_menu ( GSB_BUTTON_ICON,
							    GTK_STOCK_SELECT_COLOR, 
							    _("View"),
							    G_CALLBACK(popup_budgetary_line_view_mode_menu),
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
    gint no_division = -1, no_sub_division = -1;
    struct struct_imputation * budgetary_line = NULL;
    struct struct_sous_imputation * sub_budgetary_line = NULL;
    gchar * title;

    selection = gtk_tree_view_get_selection ( view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( model, &iter, 
			     META_TREE_POINTER_COLUMN, &budgetary_line,
			     META_TREE_NO_DIV_COLUMN, &no_division,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			     -1 );
    }

    if ( !selection || no_division <= 0 || ! budgetary_line)
	return FALSE;

    if ( no_sub_division > 0 )
    {
	sub_budgetary_line = (struct struct_sous_imputation *) budgetary_line;
	title = g_strdup_printf ( _("Properties for %s"), sub_budgetary_line -> nom_sous_imputation );
    }
    else
    {
	title = g_strdup_printf ( _("Properties for %s"), budgetary_line -> nom_imputation );
    }

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

    if ( sub_budgetary_line )
	entry = new_text_entry ( &(sub_budgetary_line -> nom_sous_imputation), NULL );
    else
	entry = new_text_entry ( &(budgetary_line -> nom_imputation), NULL );
    gtk_widget_set_usize ( entry, 400, 0 );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    if ( ! sub_budgetary_line )
    {
	/* Description entry */
	label = gtk_label_new ( _("Type"));
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
	gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
			   GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
	radiogroup = new_radiogroup ( _("Credit"), _("Debit"), 
				      &(budgetary_line -> type_imputation), NULL );
	gtk_table_attach ( GTK_TABLE(table), radiogroup, 
			   1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    }

    gtk_widget_show_all ( dialog );
    free ( title );

    gtk_dialog_run ( GTK_DIALOG(dialog) );
    gtk_widget_destroy ( dialog );

    mise_a_jour_combofix_imputation ();

    if ( sub_budgetary_line )
    {
	fill_sub_division_row ( model, budgetary_interface,
				get_iter_from_div ( model, no_division, no_sub_division ), 
				imputation_par_no ( no_division ), sub_budgetary_line );
    }
    else
    {
	fill_division_row ( model, budgetary_interface,
			    get_iter_from_div ( model, no_division, -1 ), budgetary_line );
    }

    return TRUE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
