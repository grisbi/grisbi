/* ************************************************************************** */
/* fichier qui s'occupe de l'onglet de gestion des imputations                */
/* 			imputation_budgetaire.c                               */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/*			2004 Alain Portal (aportal@univ-monpt2.fr) 	      */
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
#include "ventilation_constants.h"
#include "operations_formulaire_constants.h"



/*START_INCLUDE*/
#include "affichage_formulaire.h"
#include "barre_outils.h"
#include "dialog.h"
#include "etats_config.h"
#include "fichiers_io.h"
#include "gtk_combofix.h"
#include "imputation_budgetaire.h"
#include "meta_budgetary.h"
#include "metatree.h"
#include "operations_comptes.h"
#include "operations_formulaire.h"
#include "operations_liste.h"
#include "search_glist.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_devises.h"
#include "utils_editables.h"
#include "utils_file_selection.h"
#include "utils_files.h"
#include "utils_ib.h"
#include "utils_operations.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void appui_sur_ajout_imputation ( void );
static void appui_sur_ajout_sous_imputation ( void );
static void clique_sur_annuler_imputation ( void );
static void clique_sur_modifier_imputation ( void );
static void enleve_selection_ligne_imputation ( void );
static void exporter_ib ( void );
static void fusion_categories_imputation ( void );
static void importer_ib ( void );
static void modification_du_texte_imputation ( void );
static void supprimer_imputation ( void );
static void supprimer_sous_imputation ( void );
static gboolean budgetary_line_drag_data_get ( GtkTreeDragSource * drag_source, 
					       GtkTreePath * path,
					       GtkSelectionData * selection_data );
GtkWidget *creation_barre_outils_ib ( void );
gboolean popup_budgetary_line_view_mode_menu ( GtkWidget * button );
gboolean edit_budgetary_line ( GtkWidget * button, GtkTreeView * view );
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
gfloat *tab_montant_imputation;             /* buffer */
gfloat **tab_montant_sous_imputation;            /* buffer */
gint *nb_ecritures_par_imputation;           /* buffer */
gint **nb_ecritures_par_sous_imputation;           /* buffer */
gint mise_a_jour_combofix_imputation_necessaire;
gint no_devise_totaux_ib;


/*START_EXTERN*/
extern gint compte_courant;
extern gchar *dernier_chemin_de_travail;
extern struct struct_devise *devise_compte;
extern struct struct_devise *devise_operation;
extern struct struct_etat *etat_courant;
extern GtkWidget *formulaire;
extern GSList *liste_struct_categories;
extern GSList *liste_struct_echeances;
extern GdkBitmap *masque_ferme;
extern GdkBitmap *masque_ouvre;
extern gint modif_imputation;
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
extern GtkWidget *window;
/*END_EXTERN*/





/* **************************************************************************************************** */
/* Fonction onglet_imputations : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_imputations ( void )
{
    GtkWidget *onglet;
    GtkWidget *scroll_window;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkWidget *vbox_frame;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *separateur;
    GtkWidget *bouton;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };


    /* création de la fenêtre qui sera renvoyée */

    onglet = gtk_hbox_new ( FALSE,
			    5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10 );
    gtk_widget_show ( onglet );



    /* mise en place de la partie de gauche */

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

    entree_nom_imputation = gtk_entry_new ();
    gtk_widget_set_sensitive ( entree_nom_imputation,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_nom_imputation ),
			 "changed",
			 GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 entree_nom_imputation,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( entree_nom_imputation );


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

    bouton_imputation_debit = gtk_radio_button_new_with_label ( NULL,
								_("Debit") );
    gtk_widget_set_sensitive ( bouton_imputation_debit,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_imputation_debit ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_imputation_debit,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_imputation_debit );


    bouton_imputation_credit = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( bouton_imputation_debit ),
									     _("Credit") );
    gtk_widget_set_sensitive ( bouton_imputation_credit,
			       FALSE );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_imputation_credit,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_imputation_credit );


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

    /* FIXME */
    bouton_modif_imputation_modifier = gtk_button_new_from_stock (GTK_STOCK_APPLY);
    /*   bouton_modif_imputation_modifier = GTK_STOCK ( GTK_STOCK_APPLY ); */
    gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_imputation_modifier ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_modif_imputation_modifier ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( clique_sur_modifier_imputation ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_modif_imputation_modifier,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_modif_imputation_modifier );

    /* FIXME */
    bouton_modif_imputation_annuler = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    /*   bouton_modif_imputation_annuler = GTK_STOCK ( GTK_STOCK_CANCEL ); */
    gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_imputation_annuler ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_modif_imputation_annuler ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( clique_sur_annuler_imputation ),
			 NULL );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_modif_imputation_annuler,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_modif_imputation_annuler);

    /* FIXME */
    bouton_supprimer_imputation = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    /*   bouton_supprimer_imputation = GTK_STOCK ( GNOME_STOCK_PIXMAP_REMOVE ); */
    gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_imputation ), GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_supprimer_imputation, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_imputation ), "clicked",
			 GTK_SIGNAL_FUNC ( supprimer_imputation ), NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ), bouton_supprimer_imputation, FALSE, FALSE, 0 );
    gtk_widget_show ( bouton_supprimer_imputation );

    /* mise en place des boutons ajout d'1 imput / sous-imput */
    bouton_ajouter_imputation = gtk_button_new_with_label ( _("Add a budget line") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_imputation ), GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_imputation ), "clicked",
			 GTK_SIGNAL_FUNC ( appui_sur_ajout_imputation ), NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_ajouter_imputation, FALSE, FALSE, 0 );
    gtk_widget_show ( bouton_ajouter_imputation );

    bouton_ajouter_sous_imputation = gtk_button_new_with_label ( _("Add a sub-budget line") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_sous_imputation ), GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_ajouter_sous_imputation, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_sous_imputation ), "clicked",
			 GTK_SIGNAL_FUNC ( appui_sur_ajout_sous_imputation ), NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_ajouter_sous_imputation, FALSE, FALSE, 0 );
    gtk_widget_show ( bouton_ajouter_sous_imputation );

    /* séparation */
    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox ), separateur, FALSE, FALSE, 5 );
    gtk_widget_show ( separateur );

    /* mise en place du bouton fusionner avec les catégories */
    bouton = gtk_button_new_with_label ( _("Merge categories") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ), GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
			 GTK_SIGNAL_FUNC ( fusion_categories_imputation ), NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton, FALSE, FALSE, 0 );
    gtk_widget_show ( bouton );

    /* séparation */
    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox ), separateur, FALSE, FALSE, 5 );
    gtk_widget_show ( separateur );

    /* mise en place du bouton exporter */
    bouton = gtk_button_new_with_label ( _("Export") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ), GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked", 
			 GTK_SIGNAL_FUNC ( exporter_ib ), NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton, FALSE, FALSE, 0 );
    gtk_widget_show ( bouton );

    /* mise en place du bouton importer */
    bouton = gtk_button_new_with_label ( _("Import") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ), GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
			 GTK_SIGNAL_FUNC ( importer_ib ), NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton, FALSE, FALSE, 0 );
    gtk_widget_show ( bouton );


    /*   création de la frame de droite */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ), frame, TRUE, TRUE, 5 );
    gtk_widget_show (frame );

    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 10 );
    gtk_container_add ( GTK_CONTAINER ( frame ), vbox );
    gtk_widget_show ( vbox );

    /* We create the gtktreeview and model early so that they can be referenced. */
    budgetary_line_tree = gtk_tree_view_new();
    budgetary_line_tree_model = gtk_tree_store_new ( META_TREE_NUM_COLUMNS, 
						     G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
						     G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT, 
						     G_TYPE_INT, G_TYPE_FLOAT );

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

    return ( onglet );
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

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

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

    enleve_selection_ligne_imputation ();
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



void enleve_selection_ligne_imputation ( void )
{
    gtk_widget_set_sensitive ( bouton_supprimer_imputation,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );
    gtk_widget_set_sensitive ( entree_nom_imputation,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_imputation_debit,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_imputation_credit,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_ajouter_sous_imputation,
			       FALSE );

    gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_imputation ),
				       GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
				       NULL );

    gtk_editable_delete_text ( GTK_EDITABLE ( entree_nom_imputation ),
			       0,
			       -1 );

    gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_imputation ),
					 GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
					 NULL );

}
/* **************************************************************************************************** */






/* **************************************************************************************************** */
void modification_du_texte_imputation ( void )
{
    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       TRUE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       TRUE );
}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void clique_sur_modifier_imputation ( void )
{
    GtkCTreeNode *node;


    /* si c'est une imputation et si c'est une modif du nom, on doit réafficher la liste des imputation et les listes des opés, sinon, on change juste la imputation et */
    /* met à jour les listes de imputations */
    /* si c'est une sous imputation, c'est obligmy_atoirement un changement de nom */

    if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
			       "adr_struct_imputation" ) )
    {
	/* c'est une imputation */

	struct struct_imputation *imputation;


	imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
					    "adr_struct_imputation" );


	if ( strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_imputation ))),
		      imputation -> nom_imputation ))
	{
	    free ( imputation -> nom_imputation );

	    imputation -> nom_imputation = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_imputation ))) );


	    node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

	    if ( GTK_CTREE_ROW ( node ) -> expanded )
		gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_imputation ),
					     node,
					     0,
					     imputation -> nom_imputation,
					     10,
					     pixmap_ouvre,
					     masque_ouvre );
	    else
		gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_imputation ),
					     node,
					     0,
					     imputation -> nom_imputation,
					     10,
					     pixmap_ferme,
					     masque_ferme );

	    demande_mise_a_jour_tous_comptes ();
	}
	else
	    imputation -> type_imputation = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_debit ));
    }
    else
    {
	/* c'est une sous imputation */

	struct struct_sous_imputation *sous_imputation;

	sous_imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
						 "adr_struct_sous_imputation" );

	free ( sous_imputation -> nom_sous_imputation );

	sous_imputation -> nom_sous_imputation = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_imputation ))) );


	node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

	gtk_ctree_node_set_text ( GTK_CTREE ( arbre_imputation ),
				  node,
				  0,
				  sous_imputation -> nom_sous_imputation );

	demande_mise_a_jour_tous_comptes ();
    }

    gtk_clist_unselect_all ( GTK_CLIST ( arbre_imputation ));

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();

    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void clique_sur_annuler_imputation ( void )
{
    if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
			       "adr_struct_imputation" ))
    {
	/* c'est une imputation */

	struct struct_imputation *imputation;

	imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
					    "adr_struct_imputation" );

	gtk_entry_set_text ( GTK_ENTRY ( entree_nom_imputation ),
			     imputation -> nom_imputation );

	if ( imputation -> type_imputation )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_debit ),
					   TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_credit ),
					   TRUE );
    }
    else
    {
	/* c'est une sous imputation */

	struct struct_sous_imputation *sous_imputation;

	sous_imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
						 "adr_struct_sous_imputation" );

	gtk_entry_set_text ( GTK_ENTRY ( entree_nom_imputation ),
			     sous_imputation -> nom_sous_imputation );

    }

    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );

}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void supprimer_imputation ( void )
{
    struct struct_imputation *imputation;
    GtkCTreeNode *node;
    gint i;
    gint ope_trouvee;


    if ( !gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
				"adr_struct_imputation" ))
    {
	supprimer_sous_imputation();
	return;
    }

    node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

    imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					       node );

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

	    if ( operation -> imputation == imputation -> no_imputation )
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


    if ( ope_trouvee )
    {
	GtkWidget *dialog;
	GtkWidget *label;
	gint resultat;
	GtkWidget *separation;
	GtkWidget *hbox;
	GtkWidget *bouton_imputation_generique;
	GtkWidget *combofix;
	GSList *liste_combofix;
	GSList *pointeur;
	GSList *liste_tmp;
	GtkWidget *bouton_transfert;
	gint i;
	struct struct_imputation *nouvelle_imputation;
	struct struct_sous_imputation *nouvelle_sous_imputation;
	GSList *liste_imputation_credit;
	GSList *liste_imputation_debit;
	gchar **split_imputation;
	gint nouveau_no_imputation = 0;
	gint nouveau_no_sous_imputation = 0;

	dialog = gtk_dialog_new_with_buttons ( _("Delete a budgetary line"),
					       GTK_WINDOW (window),
					       GTK_DIALOG_MODAL,
					       GTK_STOCK_CANCEL,1,
					       GTK_STOCK_OK,0,
					       NULL);

	label = gtk_label_new ( COLON(_("Some transactions are still assigned to this budgetary line.\n\nYou can")) );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );

	separation = gtk_hseparator_new ( );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     separation,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix tranfert vers un autre imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							     COLON(_("Move the transactions to"))  );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_transfert,
			     FALSE,
			     FALSE,
			     0 );


	pointeur = liste_struct_imputation;
	liste_combofix = NULL;
	liste_imputation_credit = NULL;
	liste_imputation_debit = NULL;

	while ( pointeur )
	{
	    struct struct_imputation *imputation_tmp;
	    GSList *sous_pointeur;

	    imputation_tmp = pointeur -> data;

	    if ( imputation_tmp -> no_imputation != imputation -> no_imputation )
	    {
		if ( imputation_tmp -> type_imputation )
		    liste_imputation_debit = g_slist_append ( liste_imputation_debit,
							      g_strdup ( imputation_tmp -> nom_imputation ) );
		else
		    liste_imputation_credit = g_slist_append ( liste_imputation_credit,
							       g_strdup ( imputation_tmp -> nom_imputation ) );


		sous_pointeur = imputation_tmp -> liste_sous_imputation;

		while ( sous_pointeur )
		{
		    struct struct_sous_imputation *sous_imputation;

		    sous_imputation = sous_pointeur -> data;

		    if ( imputation_tmp -> type_imputation )
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
	    }
	    pointeur = pointeur -> next;
	}

	/*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_debit );
	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_credit );


	combofix = gtk_combofix_new_complex ( liste_combofix,
					      TRUE,
					      TRUE,
					      TRUE,
					      0 );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     combofix,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix supprimer le imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_imputation_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
									_("Just delete the budgetary line.") );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_imputation_generique,
			     FALSE,
			     FALSE,
			     0 );

	gtk_widget_show_all ( dialog );


retour_dialogue:
	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

	if ( resultat )
	{
	    if ( resultat == 1 )
		gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	    return;
	}


	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
	{

	    if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	    {
		dialogue_error ( _("Please enter a budgetary line!") );
		goto retour_dialogue;
	    }

	    /* transfert des imputation au niveau des listes */

	    split_imputation = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
					    " : ",
					    2 );

	    nouvelle_imputation = imputation_par_nom ( split_imputation[0],
						       0,
						       0,
						       0 );

	    if ( nouvelle_imputation )
	    {
		nouveau_no_imputation = nouvelle_imputation -> no_imputation;

		nouvelle_sous_imputation = sous_imputation_par_nom ( nouvelle_imputation,
								     split_imputation[1],
								     0 );

		if ( nouvelle_sous_imputation )
		    nouveau_no_sous_imputation = nouvelle_sous_imputation -> no_sous_imputation;
		else
		    nouveau_no_sous_imputation = 0;
	    }
	    g_strfreev ( split_imputation );
	}
	else
	{
	    nouveau_no_imputation = 0;
	    nouveau_no_sous_imputation = 0;
	}


	/* on fait le tour des opés pour mettre le nouveau numéro de imputation et sous_imputation */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    liste_tmp = LISTE_OPERATIONS;

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( operation -> imputation == imputation -> no_imputation )
		{
		    operation -> imputation = nouveau_no_imputation;
		    operation -> sous_imputation = nouveau_no_sous_imputation;
		}

		liste_tmp = liste_tmp -> next;
	    }

	    p_tab_nom_de_compte_variable++;
	}

	/* fait le tour des échéances pour mettre le nouveau numéro de imputation et sous_imputation  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( echeance -> imputation == imputation -> no_imputation )
	    {
		echeance -> imputation = nouveau_no_imputation;
		echeance -> sous_imputation = nouveau_no_sous_imputation;

		if ( !echeance -> imputation )
		    echeance -> compte_virement = -1;
	    }

	    liste_tmp = liste_tmp -> next;
	}


	demande_mise_a_jour_tous_comptes ();

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    }


    /* supprime dans la liste des imputation  */

    liste_struct_imputation = g_slist_remove ( liste_struct_imputation,
					       imputation );
    nb_enregistrements_imputations--;

    enleve_selection_ligne_imputation();

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation  ();
    remplit_arbre_imputation ();

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */








/* **************************************************************************************************** */
void supprimer_sous_imputation ( void )
{
    struct struct_imputation *imputation;
    struct struct_sous_imputation *sous_imputation;
    GtkCTreeNode *node;
    gint i;
    gint ope_trouvee;



    node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

    sous_imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
						    node );

    node = GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data ) -> parent;

    imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					       node );

    /* fait le tour des opés pour en trouver une qui a cette sous-catégorie */

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

	    if ( operation -> imputation == imputation -> no_imputation
		 &&
		 operation -> sous_imputation == sous_imputation -> no_sous_imputation )
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


    if ( ope_trouvee )
    {
	GtkWidget *dialog;
	GtkWidget *label;
	gint resultat;
	GtkWidget *separation;
	GtkWidget *hbox;
	GtkWidget *bouton_imputation_generique;
	GtkWidget *combofix;
	GSList *liste_combofix;
	GSList *pointeur;
	GSList *liste_tmp;
	GtkWidget *bouton_transfert;
	gint i;
	struct struct_imputation *nouvelle_imputation;
	struct struct_sous_imputation *nouvelle_sous_imputation;
	GSList *liste_imputation_credit;
	GSList *liste_imputation_debit;
	gchar **split_imputation;
	gint nouveau_no_imputation = 0;
	gint nouveau_no_sous_imputation = 0;

	dialog = gtk_dialog_new_with_buttons ( _("Deleting a budgetary line"),
					       GTK_WINDOW (window),
					       GTK_DIALOG_MODAL,
					       GTK_STOCK_CANCEL,1,
					       GTK_STOCK_OK,0,
					       NULL);

	label = gtk_label_new ( COLON(_("Some transactions are still assigned to this sub-budgetary line.\n\nYou can")) );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );

	separation = gtk_hseparator_new ( );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     separation,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix tranfert vers un autre sous imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							     COLON(_("Move the transactions to"))  );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_transfert,
			     FALSE,
			     FALSE,
			     0 );


	pointeur = liste_struct_imputation;
	liste_combofix = NULL;
	liste_imputation_credit = NULL;
	liste_imputation_debit = NULL;

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
		struct struct_sous_imputation *sous_imputation_tmp;

		sous_imputation_tmp = sous_pointeur -> data;

		if ( sous_imputation_tmp -> no_sous_imputation != sous_imputation -> no_sous_imputation )
		{
		    if ( imputation -> type_imputation )
			liste_imputation_debit = g_slist_append ( liste_imputation_debit,
								  g_strconcat ( "\t",
										sous_imputation_tmp -> nom_sous_imputation,
										NULL ) );
		    else
			liste_imputation_credit = g_slist_append ( liste_imputation_credit,
								   g_strconcat ( "\t",
										 sous_imputation_tmp -> nom_sous_imputation,
										 NULL ) );
		}
		sous_pointeur = sous_pointeur -> next;
	    }
	    pointeur = pointeur -> next;
	}

	/*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_debit );
	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_credit );


	combofix = gtk_combofix_new_complex ( liste_combofix,
					      TRUE,
					      TRUE,
					      TRUE,
					      0 );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     combofix,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix supprimer le imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_imputation_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
									PRESPACIFY(_("Just delete the sub-budgetary line.")) );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_imputation_generique,
			     FALSE,
			     FALSE,
			     0 );

	gtk_widget_show_all ( dialog );


retour_dialogue:
	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

	if ( resultat )
	{
	    if ( resultat == 1 )
		gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	    return;
	}


	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
	{

	    if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	    {
		dialogue_error ( _("Please enter a budgetary line!") );
		goto retour_dialogue;
	    }

	    /* récupère les no de imputation et sous imputation */

	    split_imputation = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
					    " : ",
					    2 );

	    nouvelle_imputation = imputation_par_nom ( split_imputation[0],
						       0,
						       0,
						       0 );

	    if ( nouvelle_imputation )
	    {
		nouveau_no_imputation = nouvelle_imputation -> no_imputation;
		
		nouvelle_sous_imputation = sous_imputation_par_nom ( nouvelle_imputation,
								     split_imputation[1],
								     0 );

		if ( nouvelle_sous_imputation )
		nouveau_no_sous_imputation = nouvelle_sous_imputation -> no_sous_imputation;
		else
		    nouveau_no_sous_imputation = 0;
	    }
	    else
		nouveau_no_imputation = 0;

	    g_strfreev ( split_imputation );
	}
	else
	{
	    nouveau_no_imputation = 0;
	    nouveau_no_sous_imputation = 0;
	}


	/* on fait le tour des opés pour mettre le nouveau numéro de imputation et sous_imputation */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    liste_tmp = LISTE_OPERATIONS;

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( operation -> imputation == imputation -> no_imputation
		     &&
		     operation -> sous_imputation == sous_imputation -> no_sous_imputation )
		{
		    operation -> imputation = nouveau_no_imputation;
		    operation -> sous_imputation = nouveau_no_sous_imputation;
		}

		liste_tmp = liste_tmp -> next;
	    }

	    p_tab_nom_de_compte_variable++;
	}

	/* fait le tour des échéances pour mettre le nouveau numéro de imputation et sous_imputation  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( echeance -> imputation == imputation -> no_imputation
		 &&
		 echeance -> sous_imputation == sous_imputation -> no_sous_imputation )
	    {
		echeance -> imputation = nouveau_no_imputation;
		echeance -> sous_imputation = nouveau_no_sous_imputation;

		if ( !echeance -> imputation )
		    echeance -> compte_virement = -1;
	    }

	    liste_tmp = liste_tmp -> next;
	}


	demande_mise_a_jour_tous_comptes ();

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    }


    /* supprime dans la liste des imputation  */

    imputation -> liste_sous_imputation = g_slist_remove ( imputation -> liste_sous_imputation,
							   sous_imputation );


    enleve_selection_ligne_imputation();

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation  ();
    remplit_arbre_imputation ();

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */






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

    gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ),
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




/* **************************************************************************************************** */
void appui_sur_ajout_imputation ( void )
{
    gchar *nom_imputation;
    struct struct_imputation *nouvelle_imputation;
    gchar *text[4];
    GtkCTreeNode *ligne;

    if ( !( nom_imputation = demande_texte ( _("New budgetary line"),
					     COLON(_("Enter the name of the new budgetary line")) )))
	return;

    if ( imputation_par_nom ( nom_imputation, 0, 0, 0 ) )
    {
	dialogue_warning_hint ( _("Budgetary lines names must be both unique and not empty.  Please use another name for this budgetary line."),
				g_strdup_printf ( _("Budgetary line '%s' already exists."),
						  nom_imputation ) );
	return;
    }

    /* on l'ajoute à la liste des opés */

    nouvelle_imputation = imputation_par_nom ( nom_imputation,
					       1,
					       0,
					       0 );

    /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

    text[0] = nouvelle_imputation -> nom_imputation;
    text[1] = NULL;
    text[2] = NULL;
    text[3] = NULL;

    ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
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

    /* on associe à ce imputation à l'adr de sa struct */

    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
				  ligne,
				  nouvelle_imputation );

    gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_imputation ),
			       NULL );

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation();
    modif_imputation = 0;
    modification_fichier(TRUE);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void appui_sur_ajout_sous_imputation ( void )
{
    gchar *nom_sous_imputation;
    struct struct_sous_imputation *nouvelle_sous_imputation;
    struct struct_imputation *imputation;
    gchar *text[4];
    GtkCTreeNode *ligne;
    GtkCTreeNode *node_parent;

    if ( !( nom_sous_imputation = demande_texte ( _("New sub-budgetary line"),
						  COLON(_("Enter the name of the new sub-budgetary line")) )))
	return;

    /* récupère le node parent */

    node_parent = GTK_CLIST ( arbre_imputation ) -> selection -> data;

    while ( GTK_CTREE_ROW ( node_parent ) -> level != 1 )
	node_parent = GTK_CTREE_ROW ( node_parent ) -> parent;

    /* on récupère l'imputation parente */

    imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					       node_parent );

    /* On vérifie si l'opération existe. */
    if ( sous_imputation_par_nom ( imputation, nom_sous_imputation, 0 ))
    {
	dialogue_warning_hint ( _("Budgetary lines names must be both unique and not empty.  Please use another name for this budgetary line."),
				g_strdup_printf ( _("Budgetary line '%s' already exists."),
						  nom_sous_imputation ) );
	return;
    }

    /* on l'ajoute à la liste des opés */

    nouvelle_sous_imputation = sous_imputation_par_nom ( imputation,
							 nom_sous_imputation,
							 1 );


    /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

    text[0] = nouvelle_sous_imputation -> nom_sous_imputation;
    text[1] = NULL;
    text[2] = NULL;
    text[3] = NULL;

    ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
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

    /* on associe à ce imputation à l'adr de sa struct */

    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
				  ligne,
				  nouvelle_sous_imputation );

    gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_imputation ),
			       NULL );


    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation();
    modif_imputation = 0;
    modification_fichier(TRUE);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
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

    if ( no_derniere_operation )
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
	    if ( !no_derniere_operation )
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
			 new_button_with_label_and_image ( _("Budgetary line"), "new-ib.png",
							   G_CALLBACK(appui_sur_ajout_division),
							   budgetary_line_tree_model ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_button_with_label_and_image ( _("Sub-budgetary line"), "new-sub-ib.png",
							   G_CALLBACK(appui_sur_ajout_sub_division),
							   budgetary_line_tree_model ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GTK_STOCK_OPEN, 
						       _("Import"),
						       G_CALLBACK(importer_ib),
						       NULL ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GTK_STOCK_SAVE, 
						       _("Export"),
						       G_CALLBACK(exporter_ib),
						       NULL ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( GTK_STOCK_DELETE, 
						       _("Delete"),
						       G_CALLBACK(supprimer_division),
						       budgetary_line_tree ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), /* FIXME: write the property dialog */
			 new_stock_button_with_label (GTK_STOCK_PROPERTIES, 
						      _("Properties"),
						      G_CALLBACK(edit_budgetary_line), 
						      budgetary_line_tree ), 
			 FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label_menu ( GTK_STOCK_SELECT_COLOR, 
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
gboolean edit_budgetary_line ( GtkWidget * button, GtkTreeView * view )
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
