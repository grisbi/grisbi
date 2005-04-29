/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			     2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "utils_categories.h"
#include "fichiers_io.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "gtk_combofix.h"
#include "main.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_editables.h"
#include "etats_config.h"
#include "affichage_formulaire.h"
#include "operations_formulaire.h"
#include "structures.h"
#include "echeancier_ventilation.h"
#include "echeancier_formulaire.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean categ_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
			       GtkSelectionData * selection_data );
static GtkWidget *creation_barre_outils_categ ( void );
static gboolean edit_category ( GtkWidget * button, GtkTreeView * view );
static gboolean exporter_categ ( GtkButton * widget, gpointer data );
static void importer_categ ( void );
static void merge_liste_categories ( void );
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

/* liste des structures de catég */
GSList *liste_struct_categories;

/*  liste des noms des categ et sous categ pour le combofix */
GSList *liste_categories_combofix;

/* nombre de catégories */
gint nb_enregistrements_categories, no_derniere_categorie;

gint mise_a_jour_combofix_categ_necessaire;

/* Category tree model & view */
GtkTreeStore * categ_tree_model;
GtkWidget *arbre_categ;
int no_devise_totaux_categ;


/*START_EXTERN*/
extern MetatreeInterface * category_interface ;
extern gchar *dernier_chemin_de_travail;
extern struct struct_etat *etat_courant;
extern GtkWidget *formulaire;
extern GSList *liste_categories_ventilation_combofix;
extern GtkTreeStore *model;
extern gint modif_categ;
extern GtkTreeSelection * selection;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_TOTAL_WIDGET];
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
    categ_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, 
					    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
					    G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT, 
					    G_TYPE_INT, G_TYPE_FLOAT );

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
    column = gtk_tree_view_column_new_with_attributes ("Category", cell, 
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
    column = gtk_tree_view_column_new_with_attributes ("Account", cell, 
						       "text", META_TREE_ACCOUNT_COLUMN, 
						       "weight", META_TREE_FONT_COLUMN,
						       NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( arbre_categ ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* Make balance column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Balance", cell, 
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

    /* la 1ère fois qu'on affichera les catég, il faudra remplir la liste */
    modif_categ = 1;

    return ( vbox );
}



/**
 * Fill category tree with data.
 */
void remplit_arbre_categ ( void )
{
    GSList *liste_categ_tmp;
    GtkTreeIter iter_categ, iter_sous_categ;

    /** First, remove previous tree */
    gtk_tree_store_clear ( GTK_TREE_STORE (categ_tree_model) );

    /* Compute category balances. */
    calcule_total_montant_categ ();

    /** Then, populate tree with categories. */
    liste_categ_tmp = g_slist_prepend ( liste_struct_categories, NULL );
    while ( liste_categ_tmp )
    {
	struct struct_categ *categ;
	GSList *liste_sous_categ_tmp = NULL;

	categ = liste_categ_tmp -> data;

	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), &iter_categ, NULL);
	fill_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
			    &iter_categ, categ );

	/** Each category has subcategories. */
	if ( categ )
	    liste_sous_categ_tmp = categ -> liste_sous_categ;

	while ( liste_sous_categ_tmp )
	{
	    struct struct_sous_categ *sous_categ;

	    sous_categ = liste_sous_categ_tmp -> data;

	    gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), 
				   &iter_sous_categ, &iter_categ);
	    fill_sub_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
				    &iter_sous_categ, categ, sous_categ );

	    liste_sous_categ_tmp = liste_sous_categ_tmp -> next;
	}

	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), 
			       &iter_sous_categ, &iter_categ);
	fill_sub_division_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
				&iter_sous_categ, categ, NULL );
	
	liste_categ_tmp = liste_categ_tmp -> next;
    }

    modif_categ = 0;
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
	gtk_tree_set_row_drag_data (selection_data, GTK_TREE_MODEL(categ_tree_model), path);
    }

    return FALSE;
}



/**
 * Reset some variables related to categories and create the base
 * category list.
 */
void creation_liste_categories ( void )
{
    liste_struct_categories = NULL;
    nb_enregistrements_categories = 0;
    no_derniere_categorie = 0;

    /** In fact, we merge the category list with nothing, ending in
     * creating the base categories. */
    merge_liste_categories ();
}



/**
 * Merge existing category list with base categories.
 */
void merge_liste_categories ( void )
{
    gint i;
    gchar **categ;
    gint debit;
    struct struct_categ *categorie;

    debit = 0;

    /* récupération des crédits puis des débits*/
    retour_recuperation :
	if ( debit )
	    categ = categories_de_base_debit;
	else
	    categ = categories_de_base_credit;

    i = 0;

    while ( categ[i] )
    {
	gchar **split_categ;
	struct struct_sous_categ *sous_categ;

	split_categ = g_strsplit ( _(categ[i]), " : ", 2 );

	categorie = categ_par_nom( g_strstrip (g_strdup (split_categ[0])),
				   1, debit, 0 );

	if ( split_categ[1] )
	    sous_categ = sous_categ_par_nom ( categorie,
					      g_strstrip(g_strdup (split_categ[1])), 1 );

	/* libère le tableau créé */
	g_strfreev ( split_categ );

	i++;
    }

    /*       si on a fait les crédits, on fait les débits */
    if ( !debit )
    {
	debit = 1;
	goto retour_recuperation;
    }

}



/**
 * Create category list suitable for transaction form combofix.  In
 * fact, it create three lists in one : debit categories, credit
 * categories, special categories (transfer, breakdown, ...).
 */
void creation_liste_categ_combofix ( void )
{
    GSList *pointeur, *liste_categ_credit, *liste_categ_debit, *liste_categ_special;
    GSList *list_tmp;


    if ( DEBUG )
	printf ( "creation_liste_categ_combofix\n" );

    liste_categories_combofix = NULL;
    liste_categories_ventilation_combofix = NULL;
    liste_categ_credit = NULL;
    liste_categ_debit = NULL;

    pointeur = liste_struct_categories;

    while ( pointeur )
    {
	struct struct_categ *categorie;
	GSList *sous_pointeur;

	categorie = pointeur -> data;

	if ( categorie -> type_categ )
	    liste_categ_debit = g_slist_append ( liste_categ_debit,
						 g_strdup ( categorie -> nom_categ ) );
	else
	    liste_categ_credit = g_slist_append ( liste_categ_credit,
						  g_strdup ( categorie -> nom_categ ) );


	sous_pointeur = categorie -> liste_sous_categ;

	while ( sous_pointeur )
	{
	    struct struct_sous_categ *sous_categ;

	    sous_categ = sous_pointeur -> data;

	    if ( categorie -> type_categ )
		liste_categ_debit = g_slist_append ( liste_categ_debit,
						     g_strconcat ( "\t",
								   sous_categ -> nom_sous_categ,
								   NULL ) );
	    else
		liste_categ_credit = g_slist_append ( liste_categ_credit,
						      g_strconcat ( "\t",
								    sous_categ -> nom_sous_categ,
								    NULL ) );

	    sous_pointeur = sous_pointeur -> next;
	}
	pointeur = pointeur -> next;
    }


    /*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */
    liste_categories_combofix = g_slist_append ( liste_categories_combofix,
						 liste_categ_debit );
    liste_categories_combofix = g_slist_append ( liste_categories_combofix,
						 liste_categ_credit );

    /*   on ajoute les listes des crédits / débits à la liste du
     *   combofix des échéances  */
    liste_categories_ventilation_combofix = g_slist_append ( liste_categories_ventilation_combofix,
							     liste_categ_debit );
    liste_categories_ventilation_combofix = g_slist_append ( liste_categories_ventilation_combofix,
							     liste_categ_credit );

    /* création des catégories spéciales : les virements et la
     * ventilation pour le formulaire */
    liste_categ_special = NULL;

    liste_categ_special = g_slist_append ( liste_categ_special,
					   _("Breakdown of transaction") );
    liste_categ_special = g_slist_append ( liste_categ_special,
					   _("Transfer") );

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );
	if ( ! gsb_account_get_closed_account (i) )
	    liste_categ_special = g_slist_append ( liste_categ_special,
						   g_strconcat ( "\t",
								 gsb_account_get_name (i),
								 NULL ));
	list_tmp = list_tmp -> next;
    }

    liste_categories_combofix = g_slist_append ( liste_categories_combofix,
						 liste_categ_special );

    /* on saute le texte Opération ventilée */
    liste_categ_special = liste_categ_special -> next;
    liste_categories_ventilation_combofix = g_slist_append ( liste_categories_ventilation_combofix,
							     liste_categ_special );

}



/**
 * Create category lists via creation_liste_categ_combofix() and
 * update category combofixes.
 */
void mise_a_jour_combofix_categ ( void )
{
    if ( DEBUG )
	printf ( "mise_a_jour_combofix_categ\n" );

    creation_liste_categ_combofix ();

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CATEGORY )
	 &&
	 GTK_IS_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY)))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ),
				liste_categories_combofix, TRUE, TRUE );

    if ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
				liste_categories_combofix, TRUE, TRUE );

    if ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
				liste_categories_ventilation_combofix, TRUE, TRUE );

    /* FIXME : this should not be in this function */
    if ( etat_courant )
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
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ), dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( fenetre_nom ), ".cgsb" );
    
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( fenetre_nom );
	return FALSE;
    }

    nom_categ = file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));
    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

    enregistre_categ ( nom_categ );

    return FALSE;
}



/**
 *
 *
 */
void importer_categ ( void )
{
    GtkWidget *dialog, *fenetre_nom;
    gint resultat;
    gchar *nom_categ;
    gint last_transaction_number;

    fenetre_nom = file_selection_new ( _("Import categories"),
				       FILE_SELECTION_IS_OPEN_DIALOG | FILE_SELECTION_MUST_EXIST);
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ), dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( fenetre_nom ), ".cgsb" );

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    if ( resultat != GTK_RESPONSE_OK  )
    {
	gtk_widget_destroy ( fenetre_nom );
	return;
    }

    nom_categ = file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));
    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

    last_transaction_number = gsb_transaction_data_get_last_number();

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
	    /* si on a choisi de remplacer l'ancienne liste, on la vire ici */

	    if ( !last_transaction_number )
	    {
		g_slist_free ( liste_struct_categories );
		liste_struct_categories = NULL;
		no_derniere_categorie = 0;
		nb_enregistrements_categories = 0;
	    }

        case 1 :
	    if ( !charge_categ ( nom_categ ))
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
			 new_button_with_label_and_image ( etat.display_toolbar,
							   _("Category"), "new-categ.png", 
							   G_CALLBACK(appui_sur_ajout_division),
							   categ_tree_model ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_button_with_label_and_image ( etat.display_toolbar,
							   _("Sub-category"), "new-sub-categ.png",
							   G_CALLBACK(appui_sur_ajout_sub_division),
							   categ_tree_model ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_OPEN, 
						       _("Import"),
						       G_CALLBACK(importer_categ),
						       NULL ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_SAVE, 
						       _("Export"),
						       G_CALLBACK(exporter_categ),
						       NULL ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_DELETE, 
						       _("Delete"),
						       G_CALLBACK(supprimer_division),
						       arbre_categ ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_PROPERTIES, 
						       _("Properties"),
						       G_CALLBACK(edit_category), 
						       arbre_categ ),
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label_menu ( etat.display_toolbar,
							    GTK_STOCK_SELECT_COLOR, 
							    _("View"),
							    G_CALLBACK(popup_category_view_mode_menu),
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
gboolean edit_category ( GtkWidget * button, GtkTreeView * view )
{
    GtkWidget * dialog, *paddingbox, *table, *label, *entry, *hbox, *radiogroup;
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gint no_division = -1, no_sub_division = -1;
    struct struct_categ * categ = NULL;
    struct struct_sous_categ * sous_categ = NULL;
    gchar * title;

    selection = gtk_tree_view_get_selection ( view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( model, &iter, 
			     META_TREE_POINTER_COLUMN, &categ,
			     META_TREE_NO_DIV_COLUMN, &no_division,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			     -1 );
    }

    if ( !selection || no_division <= 0 || ! categ)
	return FALSE;

    if ( no_sub_division > 0 )
    {
	sous_categ = (struct struct_sous_categ *) categ;
	title = g_strdup_printf ( _("Properties for %s"), sous_categ -> nom_sous_categ );
    }
    else
    {
	title = g_strdup_printf ( _("Properties for %s"), categ -> nom_categ );
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

    if ( sous_categ )
	entry = new_text_entry ( &(sous_categ -> nom_sous_categ), NULL );
    else
	entry = new_text_entry ( &(categ -> nom_categ), NULL );
    gtk_widget_set_usize ( entry, 400, 0 );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    if ( ! sous_categ )
    {
	/* Description entry */
	label = gtk_label_new ( _("Type"));
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
	gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
			   GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
	radiogroup = new_radiogroup ( _("Credit"), _("Debit"), &(categ -> type_categ), NULL );
	gtk_table_attach ( GTK_TABLE(table), radiogroup, 
			   1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    }

    gtk_widget_show_all ( dialog );
    free ( title );

    gtk_dialog_run ( GTK_DIALOG(dialog) );
    gtk_widget_destroy ( dialog );

    mise_a_jour_combofix_categ ();

    if ( sous_categ )
    {
	fill_sub_division_row ( model, category_interface,
				get_iter_from_div ( model, no_division, no_sub_division ), 
				categ_par_no ( no_division ), sous_categ );
    }
    else
    {
	fill_division_row ( model, category_interface,
			    get_iter_from_div ( model, no_division, -1 ), categ );
    }

    return TRUE;
}




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
