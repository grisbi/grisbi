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
static void expand_selected_category ();
static gboolean exporter_categ ( GtkButton * widget, gpointer data );
static void importer_categ ( void );
static gboolean keypress_category ( GtkWidget *widget, GdkEventKey *ev, gint *no_origine );
static void merge_liste_categories ( void );
static void modification_du_texte_categ ( void );
static void supprimer_categ ( void );
static void supprimer_sous_categ ( struct struct_sous_categ *sous_categ, gint no_categ );
static gboolean categ_column_expanded  (GtkTreeView * treeview, GtkTreeIter * iter, 
					GtkTreePath * tree_path, gpointer user_data );
static gboolean categ_activated ( GtkTreeView * treeview, GtkTreePath * path,
				  GtkTreeViewColumn * col, gpointer userdata );
static void fill_categ_row ( GtkTreeIter * iter, struct struct_categ * categ );
static void fill_sub_categ_row ( GtkTreeIter * iter, struct struct_categ * categ,
				 struct struct_sous_categ * sous_categ );
static gboolean categ_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
				      GtkSelectionData * selection_data );
static gboolean categ_row_drop_possible (GtkTreeDragDest * drag_dest, 
					 GtkTreePath * dest_path,
					 GtkSelectionData * selection_data);
static gboolean categ_drag_data_received ( GtkTreeDragDest   *drag_dest,
					   GtkTreePath       *dest_path,
					   GtkSelectionData  *selection_data );
void move_transaction_to_sub_category ( struct structure_operation * transaction,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_categ, gint no_sub_categ );
gboolean find_destination_blob ( struct struct_categ * categ, 
				 struct struct_sous_categ * sous_categ, 
				 gint * no_div, gint * no_sub_div );
gboolean find_associated_transactions ( gint no_categ, gint no_sous_categ );
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
struct struct_categ * without_category;
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
			 NULL );
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

    /* Create container + TreeView */
    arbre_categ = gtk_tree_view_new();
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (arbre_categ), TRUE);
    gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(arbre_categ),
					   GDK_BUTTON1_MASK, row_targets, 1,
					   GDK_ACTION_MOVE );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW(arbre_categ), row_targets,
					   1, GDK_ACTION_MOVE );
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW(arbre_categ), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(arbre_categ)),
				  GTK_SELECTION_SINGLE );
    gtk_tree_view_set_model (GTK_TREE_VIEW (arbre_categ), 
			     GTK_TREE_MODEL (categ_tree_model));

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

    liste_categ_tmp = g_slist_prepend ( liste_struct_categories, without_category );

    while ( liste_categ_tmp )
    {
	struct struct_categ *categ;
	GSList *liste_sous_categ_tmp;

	categ = liste_categ_tmp -> data;

	/* Populate tree */
	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), &iter_categ, NULL);
	fill_categ_row ( &iter_categ, categ );

	/*       pour chaque categ, on met ses sous-categ */
	liste_sous_categ_tmp = categ -> liste_sous_categ;

	while ( liste_sous_categ_tmp )
	{
	    struct struct_sous_categ *sous_categ;

	    sous_categ = liste_sous_categ_tmp -> data;

	    /* Populate tree */
	    gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), 
				   &iter_sous_categ, &iter_categ);
	    fill_sub_categ_row ( &iter_sous_categ, categ, sous_categ );

	    liste_sous_categ_tmp = liste_sous_categ_tmp -> next;
	}

	/* Treat the no sub-category sub division. */
	gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), 
			       &iter_sous_categ, &iter_categ);
	fill_sub_categ_row ( &iter_sous_categ, categ, NULL );
	
	liste_categ_tmp = liste_categ_tmp -> next;
    }

    enleve_selection_ligne_categ ();
    modif_categ = 0;
}



/**
 * Fill a division row with textual representation of a division
 * structure, in the form: "Name (num transactions) Balance".
 *
 * \param iter		Iter to fill with division data.
 * \param categ		Division structure.
 */
void fill_categ_row ( GtkTreeIter * iter, struct struct_categ * categ )
{
    gchar * label = NULL, * balance = NULL;

    label = ( categ ? categ -> nom_categ : _("No category") );

    if ( etat.affiche_nb_ecritures_listes
	 &&
	 categ -> nb_transactions )
	label = g_strconcat ( label, " (",
			      itoa ( categ -> nb_transactions ), ")",
			      NULL );

    if ( categ -> nb_transactions )
	balance = g_strdup_printf ( _("%4.2f %s"), categ -> balance,
				    devise_code ( devise_compte ) );
    
    gtk_tree_store_set (GTK_TREE_STORE (categ_tree_model), iter,
			META_TREE_TEXT_COLUMN, label,
			META_TREE_POINTER_COLUMN, categ,
			META_TREE_BALANCE_COLUMN, balance,
			META_TREE_XALIGN_COLUMN, 1.0,
			META_TREE_NO_DIV_COLUMN, (categ ? categ -> no_categ : 0 ),
			META_TREE_NO_SUB_DIV_COLUMN, -1,
			META_TREE_FONT_COLUMN, 800,
			-1);
}



/**
 * Fill a sub-division row with textual representation of a
 * sub-division structure, in the form: "Name (num transactions)
 * Balance".
 *
 * \param iter		Iter to fill with sub-division data.
 * \param categ		Division structure (parent).
 * \param sous_categ	Sub-division structure.
 */
void fill_sub_categ_row ( GtkTreeIter * iter, struct struct_categ * categ,
			  struct struct_sous_categ * sous_categ )
{
    gchar * balance = NULL, *label;
    GtkTreeIter dumb_iter;
    gint nb_ecritures = 0;

    label = ( sous_categ ? sous_categ -> nom_sous_categ : _("No sub-category") );

    if ( sous_categ && sous_categ -> nb_transactions )
    {
	nb_ecritures = sous_categ -> nb_transactions;
    }
    else if ( categ && !sous_categ )
    {
	nb_ecritures = categ -> nb_direct_transactions;
    }
    
    if ( nb_ecritures )
    {
	if ( etat.affiche_nb_ecritures_listes )
	    label = g_strconcat ( label, " (", itoa ( nb_ecritures ), ")", NULL );
	
	if ( ! gtk_tree_model_iter_has_child ( GTK_TREE_MODEL(categ_tree_model), iter ) )
	{
	    gtk_tree_store_append (GTK_TREE_STORE (categ_tree_model), &dumb_iter, iter );
	}
    }

    if ( sous_categ && sous_categ -> balance )
	balance = g_strdup_printf ( _("%4.2f %s"),
				    sous_categ -> balance,
				    devise_code ( devise_compte ) );
    else if ( categ && categ -> balance )
	balance = g_strdup_printf ( _("%4.2f %s"),
				    categ -> direct_balance,
				    devise_code ( devise_compte ) );

    gtk_tree_store_set ( GTK_TREE_STORE (categ_tree_model), iter,
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_POINTER_COLUMN, sous_categ,
			 META_TREE_BALANCE_COLUMN, balance,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_NO_DIV_COLUMN, ( categ ? categ -> no_categ : 0 ),
			 META_TREE_NO_SUB_DIV_COLUMN, ( sous_categ ? 
							sous_categ -> no_sous_categ :
							0 ),
			 META_TREE_FONT_COLUMN, 400,
			 -1 );
}



/**
 *
 *
 */
void fill_transaction_row ( GtkTreeIter * iter, struct structure_operation * operation )
{
    gchar *montant, * label, * notes = NULL; /* free */

    if ( operation -> notes )
    {
	if ( strlen ( operation -> notes ) > 30 )
	{
	    gchar * tmp = (operation -> notes) + 30;

	    tmp = strchr ( tmp, ' ' );
	    if ( !tmp )
	    {
		/* We do not risk splitting the string
		   in the middle of a UTF-8 accent
		   ... the end is probably near btw. */
		notes = operation -> notes;
	    }
	    else 
	    {
		gchar * trunc = g_strndup ( operation -> notes, 
					    ( tmp - operation -> notes ) );
		notes = g_strconcat ( trunc, " ...", NULL );
		free ( trunc );
	    }
	}
	else 
	{
	    notes = operation -> notes;
	}
    }
    else
    {
	struct struct_tiers * tiers = tiers_par_no ( operation -> tiers );
	if ( tiers )
	{
	    notes = tiers -> nom_tiers;
	}
    }

    label = g_strdup_printf ( _("%02d/%02d/%04d"),
			      operation -> jour,
			      operation -> mois,
			      operation -> annee );
 
    if ( notes )
    {
	label = g_strconcat ( label, " : ", notes, NULL );
    }

    if ( operation -> no_operation_ventilee_associee )
    {
	label = g_strconcat ( label, " (", _("breakdown"), ")", NULL );
    }

    montant = g_strdup_printf ( "%4.2f %s", operation -> montant,
				devise_code ( devise_par_no ( operation -> devise ) ) );
    gtk_tree_store_set (GTK_TREE_STORE(categ_tree_model), iter, 
			META_TREE_POINTER_COLUMN, operation,
			META_TREE_TEXT_COLUMN, label,
			META_TREE_ACCOUNT_COLUMN, compte_name_by_no(operation -> no_compte),
			META_TREE_BALANCE_COLUMN, montant,
			META_TREE_NO_DIV_COLUMN, -1,
			META_TREE_NO_SUB_DIV_COLUMN, -1,
			META_TREE_XALIGN_COLUMN, 1.0,
			-1);
    
}


/* **************************************************************************************************** */



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

	expand_selected_category ();
    }

    return FALSE;
}


void expand_selected_category ()
{
    struct structure_operation *operation;

    if ( GTK_CLIST ( arbre_categ ) -> selection
	 &&
	 ( GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) -> level == 4
	   ||
	   ( GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) -> level == 3
	     &&
	     gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					   GTK_CTREE_ROW ( GTK_CTREE_ROW (( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) -> parent ) -> parent ) > 0)))
    {
	/* passage sur le compte concerné */

	operation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
						  GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) );

	if (!operation || operation == GINT_TO_POINTER(-1))
	    return;

	/* si c'est une opé de ventilation, on se place sur l'opé
	   ventilée correspondante */ 

	operation = operation_par_no ( operation -> no_operation_ventilee_associee,
				       operation -> no_compte );

	if ( operation )
	{
	    changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

	    if ( operation -> pointe == 3 && !AFFICHAGE_R )
	    {
		AFFICHAGE_R = 1;
		remplissage_liste_operations ( compte_courant );
	    }

	    selectionne_ligne ( operation );
	}
    }

}


/* **************************************************************************************************** */
gboolean enleve_selection_ligne_categ ( void )
{
    gtk_widget_set_sensitive ( bouton_supprimer_categ,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			       FALSE );
    gtk_widget_set_sensitive ( entree_nom_categ,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_categ_debit,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_categ_credit,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_ajouter_sous_categorie,
			       FALSE );


    gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_categ ),
				       GTK_SIGNAL_FUNC ( modification_du_texte_categ),
				       NULL );

    gtk_editable_delete_text ( GTK_EDITABLE ( entree_nom_categ ),
			       0,
			       -1 );

    gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_categ ),
					 GTK_SIGNAL_FUNC ( modification_du_texte_categ),
					 NULL );

    return FALSE;
}
/* **************************************************************************************************** */






/* **************************************************************************************************** */
void modification_du_texte_categ ( void )
{
    gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			       TRUE );
    gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			       TRUE );
}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void clique_sur_modifier_categ ( void )
{
    GtkCTreeNode *node;


    /* si c'est une categ et si c'est une modif du nom, on doit réafficher la liste des categ et les listes des opés, sinon, on change juste la categ et */
    /* met à jour les listes de categories */
    /* si c'est une sous categ, c'est obligmy_atoirement un changement de nom */

    if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
			       "adr_struct_categ" ) )
    {
	/* c'est une categorie */

	struct struct_categ *categ;


	categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
				       "adr_struct_categ" );


	if ( strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_categ ))),
		      categ -> nom_categ ))
	{
	    free ( categ -> nom_categ );

	    categ -> nom_categ = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_categ ))) );


	    node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data );

	    if ( GTK_CTREE_ROW ( node ) -> expanded )
		gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_categ ),
					     node,
					     0,
					     categ -> nom_categ,
					     10,
					     pixmap_ouvre,
					     masque_ouvre );
	    else
		gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_categ ),
					     node,
					     0,
					     categ -> nom_categ,
					     10,
					     pixmap_ferme,
					     masque_ferme );

	    demande_mise_a_jour_tous_comptes ();
	}
	else
	    categ -> type_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_categ_debit ));
    }
    else
    {
	/* c'est une sous categorie */

	struct struct_sous_categ *sous_categ;

	sous_categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
					    "adr_struct_sous_categ" );

	free ( sous_categ -> nom_sous_categ );

	sous_categ -> nom_sous_categ = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_categ ))) );


	node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data );

	gtk_ctree_node_set_text ( GTK_CTREE ( arbre_categ ),
				  node,
				  0,
				  sous_categ -> nom_sous_categ );

	demande_mise_a_jour_tous_comptes ();
    }

    gtk_clist_unselect_all ( GTK_CLIST ( arbre_categ ));

    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();

    gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			       FALSE );

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void clique_sur_annuler_categ ( void )
{
    if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
			       "adr_struct_categ" ))
    {
	/* c'est une categorie */

	struct struct_categ *categ;

	categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
				       "adr_struct_categ" );

	gtk_entry_set_text ( GTK_ENTRY ( entree_nom_categ ),
			     categ -> nom_categ );

	if ( categ -> type_categ )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_categ_debit ),
					   TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_categ_credit ),
					   TRUE );
    }
    else
    {
	/* c'est une sous categorie */

	struct struct_sous_categ *sous_categ;

	sous_categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
					    "adr_struct_sous_categ" );

	gtk_entry_set_text ( GTK_ENTRY ( entree_nom_categ ),
			     sous_categ -> nom_sous_categ );

    }

    gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			       FALSE );

}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void supprimer_categ ( void )
{
    struct struct_categ * categ = NULL;
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gint i, ope_trouvee, echeance_trouvee, no_categ = 0, no_sub_categ = 0;
    /* ALAIN-FIXME il y a des GSList *liste_tmp un peu partout dans cette fonction,
       il faudrait voir si on ne peut pas se contenter d'une seule */
    GSList *liste_tmp2;
    gpointer pointer;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(arbre_categ));
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( GTK_TREE_MODEL(categ_tree_model), &iter, 
			     META_TREE_POINTER_COLUMN, &pointer,
			     META_TREE_NO_DIV_COLUMN, &no_categ,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sub_categ,
			     -1 );
    }
    else 
    {
	dialogue_warning ( "Tamere" );
	return;
    }

    if ( pointer == NULL )
    {
	dialogue_warning ( "Tamere" );
	return;
    }

    if ( no_sub_categ != 0 )
    {
	supprimer_sous_categ ( (struct struct_sous_categ *) pointer, no_categ );
	return;
    }

    categ = (struct struct_categ *) pointer;

    /* fait le tour des opés pour en trouver une qui a cette catégorie */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
    ope_trouvee = 0;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( operation -> categorie == categ -> no_categ )
	    {
		ope_trouvee = 1;
		liste_tmp = NULL;
		i = nb_comptes;
	    }
	    else
		liste_tmp = liste_tmp -> next;
	}
	p_tab_nom_de_compte_variable++;
    }

    /* fait le tour des échéances pour en trouver une qui a cette catégorie  */

    liste_tmp2 = liste_struct_echeances;
    echeance_trouvee = 0;

    while ( liste_tmp2 )
    {
	struct operation_echeance *echeance;

	echeance = liste_tmp2 -> data;

	if ( echeance -> categorie == categ -> no_categ )
	{
	    echeance_trouvee = 1;
	    liste_tmp2 = NULL;
	}
	else
	    liste_tmp2 = liste_tmp2 -> next;
    }


    if ( ope_trouvee || echeance_trouvee )
    {
	GtkWidget *hbox, *bouton_categ_generique, *combofix, *dialog, *bouton_transfert;
	GSList *liste_combofix, *pointeur, *liste_tmp, *liste_categ_credit, *liste_categ_debit;
	gint resultat, i, nouveau_no_categ, nouveau_no_sous_categ;
	struct struct_categ *nouvelle_categ;
	struct struct_sous_categ *nouvelle_sous_categ;
	gchar **split_categ;

	dialog = dialogue_special_no_run ( GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
					   make_hint ( _("Selected category still contains transactions."),
						       _("If you want to remove this category but want to keep transactions, you can transfer them to another category.  Otherwise, transactions can be simply deleted along with their category.") ));

	/*       mise en place du choix tranfert vers un autre categ */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							     COLON(_("Transfer transactions to category"))  );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_transfert,
			     FALSE,
			     FALSE,
			     0 );


	pointeur = liste_struct_categories;
	liste_combofix = NULL;
	liste_categ_credit = NULL;
	liste_categ_debit = NULL;

	while ( pointeur )
	{
	    struct struct_categ *categorie;
	    GSList *sous_pointeur;

	    categorie = pointeur -> data;

	    if ( categorie-> no_categ != categ -> no_categ )
	    {
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
	    }
	    pointeur = pointeur -> next;
	}

	/*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

	liste_combofix = g_slist_append ( liste_combofix,
					  liste_categ_debit );
	liste_combofix = g_slist_append ( liste_combofix,
					  liste_categ_credit );


	combofix = gtk_combofix_new_complex ( liste_combofix,
					      TRUE,
					      TRUE,
					      TRUE,
					      0 );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     combofix,
			     TRUE,
			     TRUE,
			     0 );

	/*       mise en place du choix supprimer le categ */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_categ_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
								   PRESPACIFY(_("Just remove this category.")) );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_categ_generique,
			     FALSE,
			     FALSE,
			     0 );

	gtk_widget_show_all ( dialog );


      retour_dialogue:
	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

	if ( resultat != GTK_RESPONSE_OK )
	{
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	    return;
	}

	nouveau_no_categ = 0;
	nouveau_no_sous_categ = 0;

	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
	{

	    if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	    {
		dialogue_warning_hint ( _("It is compulsory to specify a destination category to move transactions but no category was entered."),
					_("Please enter a category!"));
		goto retour_dialogue;
	    }

	    /* transfert des categ au niveau des listes */

	    split_categ = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
				       " : ",
				       2 );

	    nouvelle_categ = categ_par_nom ( split_categ[0],
					     0,
					     0,
					     0 );

	    if ( nouvelle_categ )
	    {
		nouveau_no_categ = nouvelle_categ -> no_categ;

		nouvelle_sous_categ = sous_categ_par_nom ( nouvelle_categ,
							   split_categ[1],
							   0 );

		if ( nouvelle_sous_categ )
		    nouveau_no_sous_categ = nouvelle_sous_categ -> no_sous_categ;
	    }

	    g_strfreev ( split_categ );
	}

	/* on fait le tour des opés pour mettre le nouveau numéro de categ et sous_categ */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    liste_tmp = LISTE_OPERATIONS;

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( operation -> categorie == categ -> no_categ )
		{
		    operation -> categorie = nouveau_no_categ;
		    operation -> sous_categorie = nouveau_no_sous_categ;
		}

		liste_tmp = liste_tmp -> next;
	    }

	    p_tab_nom_de_compte_variable++;
	}

	/* fait le tour des échéances pour mettre le nouveau numéro de categ et sous_categ  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( echeance -> categorie == categ -> no_categ )
	    {
		echeance -> categorie = nouveau_no_categ;
		echeance -> sous_categorie = nouveau_no_sous_categ;

		if ( !echeance -> categorie )
		    echeance -> compte_virement = 0;
	    }

	    liste_tmp = liste_tmp -> next;
	}


	demande_mise_a_jour_tous_comptes ();

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
    }


    /* supprime dans la liste des categ  */
    liste_struct_categories = g_slist_remove ( liste_struct_categories,
					       categ );
    nb_enregistrements_categories--;

    enleve_selection_ligne_categ();

    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ  ();

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(arbre_categ));
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(categ_tree_model), &iter );
    }    

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */








/* **************************************************************************************************** */
void supprimer_sous_categ ( struct struct_sous_categ *sous_categ, gint no_categ )
{
    struct struct_categ *categ;
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gint i;

    categ = categ_par_no ( no_categ );

    if ( find_associated_transactions ( no_categ, sous_categ -> no_sous_categ ) )
    {
	gint nouveau_no_categ, nouveau_no_sous_categ;
	GSList *liste_tmp;

	if ( ! find_destination_blob ( categ, sous_categ, 
				       &nouveau_no_categ, &nouveau_no_sous_categ ) )
	    return;

	/* on fait le tour des opés pour mettre le nouveau numéro de categ et sous_categ */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{

	    liste_tmp = LISTE_OPERATIONS;

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( operation -> categorie == categ -> no_categ
		     &&
		     operation -> sous_categorie == sous_categ -> no_sous_categ )
		{
		    operation -> categorie = nouveau_no_categ;
		    operation -> sous_categorie = nouveau_no_sous_categ;
		}

		liste_tmp = liste_tmp -> next;
	    }

	    p_tab_nom_de_compte_variable++;
	}

	/* fait le tour des échéances pour mettre le nouveau numéro de categ et sous_categ  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( echeance -> categorie == categ -> no_categ
		 &&
		 echeance -> sous_categorie == sous_categ -> no_sous_categ )
	    {
		echeance -> categorie = nouveau_no_categ;
		echeance -> sous_categorie = nouveau_no_sous_categ;

		if ( !echeance -> categorie )
		    echeance -> compte_virement = 0;
	    }

	    liste_tmp = liste_tmp -> next;
	}

	modification_fichier(TRUE);

	demande_mise_a_jour_tous_comptes ();

    }

    /* supprime dans la liste des categ  */
    categ -> liste_sous_categ = g_slist_remove ( categ -> liste_sous_categ,
						 sous_categ );
    enleve_selection_ligne_categ();

    mise_a_jour_combofix_categ ();

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(arbre_categ));
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(categ_tree_model), &iter );
    }    

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */




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
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK(expand_arbre_categ), (gpointer) 0 );
    gtk_menu_append ( menu, menu_item );
    menu_item = gtk_image_menu_item_new_with_label ( _("Subcategory view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK(expand_arbre_categ), (gpointer) 1 );
    gtk_menu_append ( menu, menu_item );
    menu_item = gtk_image_menu_item_new_with_label ( _("Complete view") );
    g_signal_connect ( G_OBJECT(menu_item), "activate", G_CALLBACK(expand_arbre_categ), (gpointer) 2 );
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
gboolean categ_column_expanded  (GtkTreeView * treeview, GtkTreeIter * iter, 
				 GtkTreePath * tree_path, gpointer user_data ) 
{
    GtkTreeIter child_iter;
    gchar *name;
    gint no_categ, no_sous_categ;

    gtk_tree_model_iter_children(GTK_TREE_MODEL(categ_tree_model), 
				 &child_iter, iter);

    gtk_tree_model_get ( GTK_TREE_MODEL(categ_tree_model), &child_iter,
			 META_TREE_TEXT_COLUMN, &name, 
			 -1 );

    if (!name)
    {
	gboolean first = TRUE;
	gint account;

	gtk_tree_model_get ( GTK_TREE_MODEL(categ_tree_model), iter,
			     META_TREE_NO_DIV_COLUMN, &no_categ,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sous_categ,
			     -1 );

	for ( account = 0; account < nb_comptes; account ++ )
	{
	    GSList *pointeur_ope;

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( account );
	    
	    pointeur_ope = LISTE_OPERATIONS;
	    
	    while ( pointeur_ope )
	    {
		struct structure_operation *operation;
		
		operation = pointeur_ope -> data;
		
		if ( operation &&
		     operation -> categorie == no_categ &&
		     operation -> sous_categorie == no_sous_categ &&
		     !operation -> relation_no_operation &&
		     !operation -> operation_ventilee )
		{
		    if ( !first )
		    {
			gtk_tree_store_append( GTK_TREE_STORE(categ_tree_model), 
					       &child_iter, iter);
		    }
		    else
		    {
			first = FALSE;
		    }
		
		    fill_transaction_row ( &child_iter, operation );
		}

		pointeur_ope = pointeur_ope -> next;
	    }
	}
    }

    return FALSE;
}



/**
 * 
 *
 */
gboolean categ_activated ( GtkTreeView * treeview, GtkTreePath * path,
			   GtkTreeViewColumn * col, gpointer userdata )
{
    struct structure_operation * operation;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint no_categ, no_sous_categ;

    model = gtk_tree_view_get_model(treeview);

    if ( gtk_tree_model_get_iter ( model, &iter, path ) )
    {
	gtk_tree_model_get( model, &iter, 
			    META_TREE_NO_DIV_COLUMN, &no_categ,
			    META_TREE_NO_SUB_DIV_COLUMN, &no_sous_categ,
			    META_TREE_POINTER_COLUMN, &operation, 
			    -1);

	/* We do not jump to a transaction if a category is specified */
	if ( operation && no_categ == -1 && no_sous_categ == -1 )
	{
	    changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;
	    if ( operation -> pointe == 3 && !AFFICHAGE_R )
		change_aspect_liste ( 5 );
	    selectionne_ligne ( operation );
	}
    }

    return FALSE;
}



/**
 *
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



/**
 *
 *
 */
gboolean categ_row_drop_possible ( GtkTreeDragDest * drag_dest, GtkTreePath * dest_path,
				   GtkSelectionData * selection_data )
{
    if ( dest_path && selection_data )
    {
	GtkTreePath * orig_path;
	GtkTreeModel * model;
	enum meta_tree_row_type orig_type, dest_type;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);
	orig_type = metatree_get_row_type ( model, orig_path );

	dest_type = metatree_get_row_type ( model, dest_path );

	switch ( orig_type )
	{
	    case META_TREE_SUB_DIV:
		if ( dest_type == META_TREE_DIV && 
		    ! gtk_tree_path_is_ancestor ( dest_path, orig_path ) )
		    return TRUE;
		break;

	    case META_TREE_TRANSACTION:
		if ( dest_type == META_TREE_DIV || 
		     dest_type == META_TREE_SUB_DIV )
		    return TRUE;
		break;

	    default:
		break;
	}
    }

    return FALSE;
}



/**
 *
 *
 */
void create_new_sub_div ( GtkTreeModel * model, GtkTreePath * parent,
			  gint no_categ, gint no_sub_categ )
{
    
}



/**
 * 
 *
 */
gboolean categ_drag_data_received ( GtkTreeDragDest * drag_dest, GtkTreePath * dest_path,
				    GtkSelectionData * selection_data )
{
    printf (">>> categ_drag_data_received %p, %p, %p\n", drag_dest, dest_path, selection_data);
    if ( dest_path && selection_data )
    {
	GtkTreeModel * model;
	GtkTreePath * orig_path;
	GtkTreeIter iter, iter_parent, orig_iter;
	gint no_dest_categ, no_dest_sub_categ, no_orig_categ, no_orig_sub_categ, account;
	gpointer pointer;
	enum meta_tree_row_type orig_type;
	struct structure_operation * transaction = NULL;
	struct struct_sous_categ * sub_categ, * dest_sub_categ;
	struct struct_categ * orig_categ, * dest_categ;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);
	metatree_get_row_properties ( model, orig_path,
				      NULL, &no_orig_categ, &no_orig_sub_categ, &pointer );
	orig_type = metatree_get_row_type ( model, orig_path );

	metatree_get_row_properties ( model, dest_path,
				      NULL, &no_dest_categ, &no_dest_sub_categ, NULL );
	
	switch ( orig_type )
	{
	    case META_TREE_TRANSACTION:
		transaction = (struct structure_operation *) pointer;
		if ( transaction )
		{
		    move_transaction_to_sub_category ( transaction, model, 
						       orig_path, dest_path,
						       no_dest_categ, no_dest_sub_categ );
		}
		break;

	    case META_TREE_SUB_DIV:
		sub_categ = sous_categ_par_no ( no_orig_categ, no_orig_sub_categ );
		dest_categ = categ_par_no ( no_dest_categ );
		orig_categ = categ_par_no ( no_orig_categ );

		dest_sub_categ = sous_categ_par_nom ( dest_categ,
						      sub_categ -> nom_sous_categ, 1 );
		no_dest_sub_categ = dest_sub_categ -> no_sous_categ;

		/* Populate tree */
		gtk_tree_model_get_iter ( model, &iter_parent, dest_path );
		gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, &iter_parent);
		fill_sub_categ_row ( &iter, categ_par_no(no_dest_categ), 
				     sous_categ_par_no(no_dest_categ, no_dest_sub_categ) );

		for ( account = 0; account < nb_comptes; account ++ )
		{
		    GSList *pointeur_ope;

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + 
			GPOINTER_TO_INT ( account );
	    
		    pointeur_ope = LISTE_OPERATIONS;
	    
		    while ( pointeur_ope )
		    {
			transaction = pointeur_ope -> data;
		
			if ( transaction &&
			     transaction -> categorie == no_orig_categ &&
			     transaction -> sous_categorie == no_orig_sub_categ &&
			     !transaction -> relation_no_operation &&
			     !transaction -> operation_ventilee )
			{
			    GtkTreePath * path;
			    path = gtk_tree_model_get_path ( model, &iter );
			    move_transaction_to_sub_category ( transaction, model, 
							       NULL, path,
							       no_dest_categ, 
							       no_dest_sub_categ );
			}

			pointeur_ope = pointeur_ope -> next;
		    }
		}

		gtk_tree_model_get_iter ( model, &orig_iter, orig_path );
		gtk_tree_store_remove ( GTK_TREE_STORE(categ_tree_model), &orig_iter );

		dest_categ -> liste_sous_categ = g_slist_append ( dest_categ -> liste_sous_categ,
								  sub_categ );
		orig_categ -> liste_sous_categ = g_slist_remove ( orig_categ -> liste_sous_categ,
								  sub_categ );
		mise_a_jour_combofix_categ();
		modification_fichier(TRUE);

		break;

	    default:
		break;
	}

    }
    
    return FALSE;
}



/**
 *
 *
 */
void move_transaction_to_sub_category ( struct structure_operation * transaction,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_categ, gint no_sub_categ )
{
    GtkTreeIter orig_iter, child_iter, dest_iter, parent_iter, gd_parent_iter;
    struct struct_categ * old_categ = categ_par_no ( transaction -> categorie);
    struct struct_categ * new_categ = categ_par_no (no_categ);
    struct struct_sous_categ * old_sous_categ = 
	sous_categ_par_no ( transaction -> categorie, transaction -> sous_categorie);
    struct struct_sous_categ * new_sous_categ = sous_categ_par_no (no_categ, no_sub_categ);
		    
    /* Insert new row */
    gtk_tree_model_get_iter ( model, &dest_iter, dest_path );
    gtk_tree_store_insert ( GTK_TREE_STORE(categ_tree_model),
			    &child_iter, &dest_iter, -1 );
    fill_transaction_row ( &child_iter, transaction );

    /* Update new parents */
    add_transaction_to_category ( transaction, new_categ, new_sous_categ );
    fill_sub_categ_row ( &dest_iter, new_categ, new_sous_categ );
    if ( gtk_tree_model_iter_parent ( model, &parent_iter, &dest_iter ) )
	fill_categ_row ( &parent_iter, new_categ );

    /* Change parameters */
    transaction -> categorie = no_categ;
    transaction -> sous_categorie = no_sub_categ;

    /* Update old parents */
    remove_transaction_from_category ( transaction, old_categ, old_sous_categ );
    if ( orig_path )
    {
	gtk_tree_model_get_iter ( model, &orig_iter, orig_path );
	if ( gtk_tree_model_iter_parent ( model, &parent_iter, &orig_iter ) )
	{
	    fill_sub_categ_row ( &parent_iter, old_categ, old_sous_categ );
	    if ( gtk_tree_model_iter_parent ( model, &gd_parent_iter, &parent_iter ) )
		fill_categ_row ( &gd_parent_iter, old_categ );
	}

	/* Remove old row */
	gtk_tree_store_remove ( GTK_TREE_STORE(categ_tree_model), &orig_iter );
    }
		    
    /* We did some modifications */
    modification_fichier ( TRUE );
}



/**
 * 
 *
 */
gboolean categ_node_maybe_expand ( GtkTreeModel *model, GtkTreePath *path, 
				   GtkTreeIter *iter, gpointer data )
{
    if ( (gint) data == gtk_tree_path_get_depth ( path ) )
	gtk_tree_view_expand_to_path ( GTK_TREE_VIEW(arbre_categ), path );

    return FALSE;
}



/**
 * Iterates over all categories tree nodes and expand nodes that are
 * not deeper than specified depth.
 *
 * \param bouton	Widget that triggered this callback.  Not used.
 * \param depth		Maximum depth for nodes to expand.
 */
void expand_arbre_categ ( GtkWidget *bouton, gint depth )
{
    gtk_tree_view_collapse_all ( GTK_TREE_VIEW(arbre_categ) );
    gtk_tree_model_foreach ( GTK_TREE_MODEL(categ_tree_model), 
			     categ_node_maybe_expand, (gpointer) depth );
}




gboolean find_destination_blob ( struct struct_categ * categ, 
				 struct struct_sous_categ * sous_categ, 
				 gint * no_div, gint * no_sub_div )
{
    GtkWidget *dialog, *hbox, *bouton_categ_generique, *combofix, *bouton_transfert;
    GSList *liste_combofix, *pointeur, *liste_categ_credit, *liste_categ_debit;
    gint resultat, nouveau_no_categ, nouveau_no_sous_categ;
    struct struct_categ *nouvelle_categ;
    struct struct_sous_categ *nouvelle_sous_categ;
    gchar **split_categ;

    dialog = dialogue_special_no_run ( GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
				       make_hint ( _("Selected sub-category still contains transactions."),
						   _("If you want to remove this sub-category but want to keep transactions, you can transfer them to another (sub-)category.  Otherwise, transactions can be simply deleted along with their category.") ));

    /*       mise en place du choix tranfert vers un autre categ */

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );

    bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							 COLON(_("Transfer transactions to category"))  );
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_transfert,
			 FALSE, FALSE, 0 );

    pointeur = liste_struct_categories;
    liste_combofix = NULL;
    liste_categ_credit = NULL;
    liste_categ_debit = NULL;

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
	    struct struct_sous_categ *sous_categorie;

	    sous_categorie = sous_pointeur -> data;

	    if ( sous_categorie -> no_sous_categ !=  sous_categ -> no_sous_categ )
	    {
		if ( categorie -> type_categ )
		    liste_categ_debit = g_slist_append ( liste_categ_debit,
							 g_strconcat ( "\t",
								       sous_categorie -> nom_sous_categ,
								       NULL ) );
		else
		    liste_categ_credit = g_slist_append ( liste_categ_credit,
							  g_strconcat ( "\t",
									sous_categorie -> nom_sous_categ,
									NULL ) );
	    }
	    sous_pointeur = sous_pointeur -> next;
	}
	pointeur = pointeur -> next;
    }

    /*   on ajoute les listes des crÃ©dits / débits à la liste
     *   du combofix du formulaire */
    liste_combofix = g_slist_append ( liste_combofix, liste_categ_debit );
    liste_combofix = g_slist_append ( liste_combofix, liste_categ_credit );

    combofix = gtk_combofix_new_complex ( liste_combofix, TRUE,
					  TRUE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combofix,
			 TRUE, TRUE, 0 );

    /*       mise en place du choix supprimer le categ */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );

    bouton_categ_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
							       PRESPACIFY(_("Just remove this sub-category.")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_categ_generique,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all ( dialog );


  retour_dialogue:
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	return FALSE;
    }

    nouveau_no_categ = 0;
    nouveau_no_sous_categ = 0;

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
    {

	if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	{
	    dialogue_warning_hint ( _("It is compulsory to specify a destination category to move transactions but no category was entered."),
				    _("Please enter a category!"));

	    goto retour_dialogue;
	}

	/* récupère les no de categ et sous categ */
	split_categ = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
				   " : ", 2 );

	nouvelle_categ = categ_par_nom ( split_categ[0], 0, 0, 0 );

	if ( nouvelle_categ )
	{
	    nouveau_no_categ = nouvelle_categ -> no_categ;
	    nouvelle_sous_categ =  sous_categ_par_nom ( nouvelle_categ, split_categ[1], 0 );

	    if ( nouvelle_sous_categ )
		nouveau_no_sous_categ = nouvelle_sous_categ -> no_sous_categ;
	}

	g_strfreev ( split_categ );
    }
    else
    {
	nouveau_no_categ = 0;
	nouveau_no_sous_categ = 0;
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    if ( no_div)
	*no_div = nouveau_no_categ;
    if ( no_sub_div )
	*no_sub_div = nouveau_no_sous_categ;

    return TRUE;
}



/** 
 * TODO: document this
 */
gboolean find_associated_transactions ( gint no_categ, gint no_sous_categ )
{
    GSList *liste_tmp;
    gint i;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    /* fait le tour des opés pour en trouver une qui a cette sous-catégorie */
    for ( i=0 ; i<nb_comptes ; i++ )
    {
	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( operation -> categorie == no_categ
		 &&
		 operation -> sous_categorie == no_sous_categ )
	    {
		return TRUE;
	    }
	    else
		liste_tmp = liste_tmp -> next;
	}
	p_tab_nom_de_compte_variable++;
    }

    liste_tmp = liste_struct_echeances;

    /* fait le tour des échéances pour en trouver une qui a cette catégorie  */
    while ( liste_tmp )
    {
	struct operation_echeance *echeance = liste_tmp -> data;

	if ( echeance -> categorie == no_categ && 
	     echeance -> sous_categorie == no_sous_categ )
	{
	    return TRUE;
	}
	else
	    liste_tmp = liste_tmp -> next;
    }

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
