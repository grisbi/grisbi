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
#include "echeancier_formulaire_constants.h"
#include "ventilation_constants.h"
#include "echeancier_ventilation_constants.h"
#include "categories_onglet_constants.h"
#include "operations_formulaire_constants.h"

/*START_INCLUDE*/
#include "categories_onglet.h"
#include "utils_devises.h"
#include "utils_categories.h"
#include "operations_comptes.h"
#include "tiers_onglet.h"
#include "fichiers_io.h"
#include "barre_outils.h"
#include "operations_liste.h"
#include "dialog.h"
#include "metatree.h"
#include "meta_categories.h"
#include "gtk_combofix.h"
#include "utils_str.h"
#include "traitement_variables.h"
#include "utils_operations.h"
#include "etats_config.h"
#include "affichage_formulaire.h"
#include "operations_formulaire.h"
#include "utils_file_selection.h"
#include "utils_files.h"
#include "utils_buttons.h"
#include "utils_comptes.h"
#include "utils_tiers.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void appui_sur_ajout_categorie ( void );
static void appui_sur_ajout_sous_categorie ( void );
static void clique_sur_annuler_categ ( void );
static void clique_sur_modifier_categ ( void );
static gboolean enleve_selection_ligne_categ ( void );
static gboolean exporter_categ ( GtkButton * widget, gpointer data );
static void importer_categ ( void );
static gboolean keypress_category ( GtkWidget *widget, GdkEventKey *ev, gint *no_origine );
static void merge_liste_categories ( void );
static void modification_du_texte_categ ( void );
/*END_STATIC*/


/* VARIABLES */

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

/* buffers */
gfloat *tab_montant_categ, **tab_montant_sous_categ;
gint *nb_ecritures_par_categ, **nb_ecritures_par_sous_categ;

/* Category tree model & view */
GtkTreeStore * categ_tree_model;
GtkWidget *arbre_categ;


/*START_EXTERN*/
extern gint compte_courant;
extern gchar *dernier_chemin_de_travail;
extern struct struct_devise *devise_compte;
extern struct struct_etat *etat_courant;
extern GtkWidget *formulaire;
extern GSList *liste_categories_ventilation_combofix;
extern GSList *liste_struct_echeances;
extern GdkBitmap *masque_ferme;
extern GdkBitmap *masque_ouvre;
extern gint modif_categ;
extern gint nb_comptes;
extern gint nb_ecritures_par_comptes;
extern gint no_derniere_operation;
extern gint no_devise_totaux_tiers;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GdkPixmap *pixmap_ferme;
extern GdkPixmap *pixmap_ouvre;
extern GtkTreeSelection * selection;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_TOTAL_WIDGET];
extern GtkWidget *widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/* struct struct_categ * without_category; */
/*END_EXTERN*/



/* **************************************************************************************************** */
/* Fonction onglet_categories : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_categories ( void )
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    static GtkTargetEntry row_targets[] = {
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };

    GtkWidget *onglet;
    GtkWidget *scroll_window;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkWidget *vbox_frame;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *separateur;
    GtkWidget *bouton;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;


    /* création de la fenêtre qui sera renvoyée */

    onglet = gtk_hbox_new ( FALSE,
			    5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10 );
    gtk_widget_show ( onglet );


    /*   création de la frame de gauche */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (frame );

    /* mise en place du gtk_text */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     15 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );




    frame = gtk_frame_new ( SPACIFY(COLON(_("Information"))) );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox_frame = gtk_vbox_new ( FALSE,
				5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_frame ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox_frame );
    gtk_widget_show ( vbox_frame );

    entree_nom_categ = gtk_entry_new ();
    gtk_widget_set_sensitive ( entree_nom_categ,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_nom_categ ),
			 "changed",
			 GTK_SIGNAL_FUNC ( modification_du_texte_categ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 entree_nom_categ,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( entree_nom_categ );


    /* création des radio bouton débit/crédit */

    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    label = gtk_label_new ( COLON(_("Sorting")) );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 label,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( label );

    bouton_categ_debit = gtk_radio_button_new_with_label ( NULL,
							   _("Debit") );
    gtk_widget_set_sensitive ( bouton_categ_debit,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_categ_debit ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( modification_du_texte_categ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_categ_debit,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_categ_debit );


    bouton_categ_credit = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( bouton_categ_debit ),
									_("Credit") );
    gtk_widget_set_sensitive ( bouton_categ_credit,
			       FALSE );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_categ_credit,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_categ_credit );


    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 separateur,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( separateur );




    /*   création des boutons modifier et annuler */

    hbox = gtk_hbox_new ( TRUE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_modif_categ_modifier = gtk_button_new_from_stock (GTK_STOCK_APPLY);
    gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_categ_modifier ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_modif_categ_modifier ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( clique_sur_modifier_categ ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_modif_categ_modifier,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_modif_categ_modifier );

    bouton_modif_categ_annuler = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_categ_annuler ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_modif_categ_annuler ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( clique_sur_annuler_categ ),
			 NULL );
    gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			       FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_modif_categ_annuler,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_modif_categ_annuler);

    bouton_supprimer_categ = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_categ ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_supprimer_categ,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_categ ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( supprimer_categ ),
			 arbre_categ );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_supprimer_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_supprimer_categ );


    /* mise en place des boutons ajout d'1 categ / sous-categ */

    bouton_ajouter_categorie = gtk_button_new_with_label ( _("Add a category") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_categorie ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_categorie ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( appui_sur_ajout_categorie ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_ajouter_categorie,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_ajouter_categorie );

    bouton_ajouter_sous_categorie = gtk_button_new_with_label ( _("Add a subcategory") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_sous_categorie ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_ajouter_sous_categorie,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_sous_categorie ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( appui_sur_ajout_sous_categorie ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_ajouter_sous_categorie,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_ajouter_sous_categorie );

    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );

    /* on met le bouton exporter */

    bouton = gtk_button_new_with_label ( _("Export") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( exporter_categ ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    /* on met le bouton importer */

    bouton = gtk_button_new_with_label ( _("Import") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( importer_categ ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );



    /*   création de la frame de droite */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 frame,
			 TRUE,
			 TRUE,
			 5 );
    gtk_widget_show (frame );


    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     10 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );

    /* on y ajoute la barre d'outils */

    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 creation_barre_outils_categ(),
			 FALSE,
			 FALSE,
			 0 );

    /* création de l'arbre principal */

    scroll_window = gtk_scrolled_window_new ( NULL,
					      NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scroll_window), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scroll_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scroll_window );

    /* Create model */
    categ_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, 
					    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
					    G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT, 
					    G_TYPE_INT, G_TYPE_FLOAT );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(categ_tree_model), 
					   META_TREE_TEXT_COLUMN, GTK_SORT_ASCENDING );
    g_object_set_data ( G_OBJECT (categ_tree_model), "metatree-interface", 
			category_interface );

    /* Create container + TreeView */
    arbre_categ = gtk_tree_view_new();
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
		       G_CALLBACK(categ_column_expanded), NULL );
    g_signal_connect( G_OBJECT(arbre_categ), "row-activated",
		      G_CALLBACK(categ_activated), NULL);

    dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (categ_tree_model);
    if ( dst_iface )
    {
	dst_iface -> drag_data_received = &categ_drag_data_received;
	dst_iface -> row_drop_possible = &categ_row_drop_possible;
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

    /* TODO: FIXME  */
    /* on met la fontion de tri alphabétique en prenant en compte les accents */
/*     gtk_clist_set_compare_func ( GTK_CLIST ( arbre_categ ), */
/* 				 (GtkCListCompareFunc) classement_alphabetique_tree ); */

    /* la 1ère fois qu'on affichera les catég, il faudra remplir la liste */
    modif_categ = 1;

    return ( onglet );
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* Fonction remplit_arbre_categ */
/* prend en argument le clist arbre_categ, */
/* le vide et le remplit */
/* on y met les categ et sous categ et leur montant */
/* **************************************************************************************************** */

void remplit_arbre_categ ( void )
{
    GSList *liste_categ_tmp;
    GtkTreeIter iter_categ, iter_sous_categ;

    /*   efface l'ancien arbre */
    gtk_tree_store_clear ( GTK_TREE_STORE (categ_tree_model) );

    /*   le devise est choisie dans les paramètres */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    if ( !devise_compte
	 ||
	 devise_compte -> no_devise != no_devise_totaux_tiers )
	devise_compte = devise_par_no ( no_devise_totaux_tiers );

    /* calcule les montants des catég et sous categ */

    calcule_total_montant_categ ();

    /* remplit l'arbre */

    liste_categ_tmp = g_slist_prepend ( liste_struct_categories, NULL );
    
    while ( liste_categ_tmp )
    {
	struct struct_categ *categ;
	GSList *liste_sous_categ_tmp = NULL;

	categ = liste_categ_tmp -> data;

	/* Populate tree */
	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), &iter_categ, NULL);
	fill_categ_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
			 &iter_categ, categ );

	/*       pour chaque categ, on met ses sous-categ */
	if ( categ )
	    liste_sous_categ_tmp = categ -> liste_sous_categ;

	while ( liste_sous_categ_tmp )
	{
	    struct struct_sous_categ *sous_categ;

	    sous_categ = liste_sous_categ_tmp -> data;

	    /* Populate tree */
	    gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), 
				   &iter_sous_categ, &iter_categ);
	    fill_sub_categ_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
				 &iter_sous_categ, categ, sous_categ );

	    liste_sous_categ_tmp = liste_sous_categ_tmp -> next;
	}

	/* Treat the no sub-category sub division. */
	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), 
			       &iter_sous_categ, &iter_categ);
	fill_sub_categ_row ( GTK_TREE_MODEL(categ_tree_model), category_interface, 
			     &iter_sous_categ, categ, NULL );
	
	liste_categ_tmp = liste_categ_tmp -> next;
    }

    enleve_selection_ligne_categ ();
    modif_categ = 0;
}



gboolean keypress_category ( GtkWidget *widget, GdkEventKey *ev, gint *no_origine )
{
    GtkCTreeNode *node;

    if ( ev->keyval == GDK_Return || 
	 ev->keyval == GDK_KP_Enter )
    { 
	node = gtk_ctree_node_nth ( GTK_CTREE(arbre_categ), 
				    GTK_CLIST(arbre_categ) -> focus_row );
	gtk_ctree_select ( GTK_CTREE(arbre_categ), node );
	gtk_ctree_expand ( GTK_CTREE(arbre_categ), node );

/* 	expand_selected_category (); */
    }

    return FALSE;
}



/**
 * Clean
 *
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



gboolean enleve_selection_ligne_categ ( void )
{
    gtk_widget_set_sensitive ( bouton_supprimer_categ, FALSE );
    gtk_widget_set_sensitive ( bouton_modif_categ_modifier, FALSE );
    gtk_widget_set_sensitive ( bouton_modif_categ_annuler, FALSE );
    gtk_widget_set_sensitive ( entree_nom_categ, FALSE );
    gtk_widget_set_sensitive ( bouton_categ_debit, FALSE );
    gtk_widget_set_sensitive ( bouton_categ_credit, FALSE );
    gtk_widget_set_sensitive ( bouton_ajouter_sous_categorie, FALSE );

    gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_categ ),
				       GTK_SIGNAL_FUNC ( modification_du_texte_categ),
				       NULL );

    gtk_editable_delete_text ( GTK_EDITABLE ( entree_nom_categ ), 0, -1 );

    gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_categ ),
					 GTK_SIGNAL_FUNC ( modification_du_texte_categ),
					 NULL );

    return FALSE;
}



/**
 *
 *
 */
void modification_du_texte_categ ( void )
{
    gtk_widget_set_sensitive ( bouton_modif_categ_modifier, TRUE );
    gtk_widget_set_sensitive ( bouton_modif_categ_annuler, TRUE );
}



/**
 *
 *
 */
void clique_sur_modifier_categ ( void )
{
}



/**
 *
 *
 */
void clique_sur_annuler_categ ( void )
{
}



/***********************************************************************************************************/
/* Routine creation_liste_categories */
/* appelée lors d'un nouveau fichier */
/* crée la liste des catégories à partir de la liste ci dessus */
/* en fait, merge la liste de base avec rien */
/***********************************************************************************************************/
void creation_liste_categories ( void )
{

    liste_struct_categories = NULL;
    nb_enregistrements_categories = 0;
    no_derniere_categorie = 0;

    merge_liste_categories ();
}



/***********************************************************************************************************/
/* Routine merge_liste_categories */
/*   merge la liste de categories existante ( aucune quand vient de creation_liste_categories ) */
/* avec la liste de base du début du fichier */
/***********************************************************************************************************/
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

	split_categ = g_strsplit ( categ[i],
				   " : ",
				   2 );

	categorie = categ_par_nom( split_categ[0],
				   1,
				   debit,
				   0 );

	sous_categ = sous_categ_par_nom ( categorie,
					  split_categ[1],
					  1 );

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
/***********************************************************************************************************/


/***********************************************************************************************************/
/*  Routine qui crée la liste des catégories pour les combofix du formulaire et de la ventilation */
/* c'est à dire 3 listes dans 1 liste : */
/* la première contient les catégories de débit */
/* la seconde contient les catégories de crédit */
/* la troisième contient les catégories spéciales ( virement, retrait, ventilation ) */
/* la ventilation n'apparait pas dans les ventilations */
/***********************************************************************************************************/

void creation_liste_categ_combofix ( void )
{
    GSList *pointeur;
    GSList *liste_categ_credit;
    GSList *liste_categ_debit;
    GSList *liste_categ_special;
    gint i;

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




    /*   on ajoute les listes des crédits / débits à la liste du combofix des  échéances  */

    liste_categories_ventilation_combofix = g_slist_append ( liste_categories_ventilation_combofix,
							     liste_categ_debit );
    liste_categories_ventilation_combofix = g_slist_append ( liste_categories_ventilation_combofix,
							     liste_categ_credit );


    /* création des catégories spéciales : les virements et la ventilation pour le formulaire */

    liste_categ_special = NULL;

    liste_categ_special = g_slist_append ( liste_categ_special,
					   _("Breakdown of transaction") );

    liste_categ_special = g_slist_append ( liste_categ_special,
					   _("Transfer") );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	if ( ! COMPTE_CLOTURE )
	    liste_categ_special = g_slist_append ( liste_categ_special,
						   g_strconcat ( "\t",
								 NOM_DU_COMPTE ,
								 NULL ));
	p_tab_nom_de_compte_variable++;
    }

    liste_categories_combofix = g_slist_append ( liste_categories_combofix,
						 liste_categ_special );

    /* on saute le texte Opération ventilée */

    liste_categ_special = liste_categ_special -> next;

    liste_categories_ventilation_combofix = g_slist_append ( liste_categories_ventilation_combofix,
							     liste_categ_special );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction mise_a_jour_combofix_categ */
/* recrée les listes de catégories des combofix */
/* et remet les combofix à jour */
/***********************************************************************************************************/

void mise_a_jour_combofix_categ ( void )
{
    if ( DEBUG )
	printf ( "mise_a_jour_combofix_categ\n" );


    creation_liste_categ_combofix ();

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CATEGORY )
	 &&
	 GTK_IS_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY)))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ),
				liste_categories_combofix,
				TRUE,
				TRUE );

    if ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
				liste_categories_combofix,
				TRUE,
				TRUE );

    if ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
				liste_categories_ventilation_combofix,
				TRUE,
				TRUE );

    if ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
				liste_categories_ventilation_combofix,
				TRUE,
				TRUE );

    /* FIXME : ça ne devrait pas se trouver dans cette fonction */

    if ( etat_courant )
    {
	remplissage_liste_categ_etats ();
	selectionne_liste_categ_etat_courant ();
    }

    mise_a_jour_combofix_categ_necessaire = 0;
    modif_categ = 1;
}
/***********************************************************************************************************/



/* **************************************************************************************************** */
void appui_sur_ajout_categorie ( void )
{
    gchar *nom_categorie;
    struct struct_categ *nouvelle_categorie;
    gchar *text[4];
    GtkCTreeNode *ligne;

    if ( !( nom_categorie = demande_texte ( _("New category"),
					    COLON(_("Enter name for new category")) )))
	return;


    /* On vérifie si l'opération existe. */
    if ( categ_par_nom ( nom_categorie, 0, 0, 0 ))
    {
	dialogue_warning_hint ( _("Category must be both unique and not empty.  Please use another name for this category."),
				g_strdup_printf ( _("Category '%s' already exists."),
						  nom_categorie ) );
	return;
    }

    /* on l'ajoute à la liste des opés */

    nouvelle_categorie = categ_par_nom ( nom_categorie,
					 1,
					 0,
					 0 );


    /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

    text[0] = nouvelle_categorie -> nom_categ;
    text[1] = NULL;
    text[2] = NULL;
    text[3] = NULL;

    ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
				    NULL,
				    NULL,
				    text,
				    10,
				    pixmap_ferme,
				    masque_ferme,
				    pixmap_ouvre,
				    masque_ouvre,
				    FALSE,
				    FALSE );

    /* on associe à ce categorie à l'adr de sa struct */

    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
				  ligne,
				  nouvelle_categorie );

    gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_categ ),
			       NULL );

    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ();
    modif_categ = 0;
    modification_fichier(TRUE);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void appui_sur_ajout_sous_categorie ( void )
{
    gchar *nom_sous_categorie;
    struct struct_sous_categ *nouvelle_sous_categorie;
    struct struct_categ *categorie;
    gchar *text[4];
    GtkCTreeNode *ligne;
    GtkCTreeNode *node_parent;

    if ( !( nom_sous_categorie = demande_texte ( _("New sub-category"),
						 COLON(_("Enter name for new sub-category")) )))
	return;

    /* récupère le node parent */

    node_parent = GTK_CLIST ( arbre_categ ) -> selection -> data;

    while ( GTK_CTREE_ROW ( node_parent ) -> level != 1 )
	node_parent = GTK_CTREE_ROW ( node_parent ) -> parent;

    /* on récupère l'categorie parente */

    categorie = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					      node_parent );
    if ( categorie <= 0 )
	return;

    /* On vérifie si l'opération existe. */
    if ( sous_categ_par_nom ( categorie, nom_sous_categorie, 0 ))
    {
	dialogue_warning_hint ( _("Sub-category must be both unique and not empty.  Please use another name for this sub-category."),
				g_strdup_printf ( _("Sub-category '%s' already exists."),
						  nom_sous_categorie ) );
	return;
    }

    /* on l'ajoute à la liste des opés */

    nouvelle_sous_categorie = sous_categ_par_nom ( categorie,
						   nom_sous_categorie,
						   1 );

    /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

    text[0] = nouvelle_sous_categorie -> nom_sous_categ;
    text[1] = NULL;
    text[2] = NULL;
    text[3] = NULL;

    ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
				    node_parent,
				    NULL,
				    text,
				    10,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    FALSE,
				    FALSE );

    /* on associe à ce categorie à l'adr de sa struct */

    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
				  ligne,
				  nouvelle_sous_categorie );

    gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_categ ),
			       NULL );


    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ();
    modif_categ = 0;
    modification_fichier(TRUE);
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

    fenetre_nom = file_selection_new ( _("Import a category list"),
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

    /* on permet de remplacer/fusionner la liste */

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				       make_hint ( _("Merge category list with existing?"),
						   ( no_derniere_operation ?
						     _("Imported category list will be merged with existing.  There is no undo for this.") :
						     _("Imported category list can either be merged with existing or replace it.  Once performed, there is no undo for this.") ) ) );

    if ( !no_derniere_operation )
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Replace existing"), 0,
				 NULL );

    gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
			     GTK_STOCK_CANCEL, 1,
			     _("Merge categories"), 2,
			     NULL );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case 0 :
	    /* si on a choisi de remplacer l'ancienne liste, on la vire ici */

	    if ( !no_derniere_operation )
	    {
		g_slist_free ( liste_struct_categories );
		liste_struct_categories = NULL;
		no_derniere_categorie = 0;
		nb_enregistrements_categories = 0;
	    }

        case 2 :
	    if ( !charge_categ ( nom_categ ))
	    {
		gtk_widget_destroy ( GTK_WIDGET ( dialog ));
		return;
	    }
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
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
			 new_stock_button_with_label ( GTK_STOCK_NEW, 
						       _("New category"),
						       G_CALLBACK(appui_sur_ajout_categorie) ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GTK_STOCK_OPEN, 
						       _("Import"),
						       G_CALLBACK(importer_categ) ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GTK_STOCK_SAVE, 
						       _("Export"),
						       G_CALLBACK(exporter_categ) ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GTK_STOCK_DELETE, 
						       _("Delete"),
						       G_CALLBACK(supprimer_categ) ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), /* FIXME: write the property dialog */
			 new_stock_button_with_label (GTK_STOCK_PROPERTIES, 
						      _("Properties"),
						      NULL), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label_menu ( GTK_STOCK_SELECT_COLOR, 
							    _("View"),
							    G_CALLBACK(popup_category_view_mode_menu) ),
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
		       G_CALLBACK(expand_arbre_categ), (gpointer) 0 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_append ( menu, menu_item );

    menu_item = gtk_image_menu_item_new_with_label ( _("Subcategory view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_categ), (gpointer) 1 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_append ( menu, menu_item );

    menu_item = gtk_image_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", 
		       G_CALLBACK(expand_arbre_categ), (gpointer) 2 );
    g_object_set_data ( G_OBJECT(menu_item), "tree-view", arbre_categ );
    gtk_menu_append ( menu, menu_item );

    gtk_widget_show_all ( menu );

    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
