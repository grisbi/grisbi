/* ************************************************************************** */
/* Ce fichier s'occupe de la configuration des états                          */
/*                      etats_config.c                                        */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2006-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org/                                            */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "etats_config.h"
#include "etats_calculs.h"
#include "tiers_onglet.h"
#include "utils.h"
#include "dialog.h"
#include "gsb_calendar_entry.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_data_report_text_comparison.h"
#include "utils_dates.h"
#include "navigation.h"
#include "gsb_real.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "structures.h"
#include "fenetre_principale.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajoute_ligne_liste_comparaisons_montants_etat ( gint last_amount_comparison_number );
static void ajoute_ligne_liste_comparaisons_textes_etat ( gint last_text_comparison_number );
static void change_comparaison_montant ( GtkWidget *menu_item,
				  gint amount_comparison_number );
static void change_comparaison_texte ( GtkWidget *menu_item,
				gint text_comparison_number );
static void click_bas_classement_etat ( void );
static void click_haut_classement_etat ( void );
static void click_liste_etat ( GtkCList *liste,
			GdkEventButton *evenement,
			gint origine );
static GtkWidget *cree_bouton_champ ( gint text_comparison_number );
static GtkWidget *cree_bouton_comparateur_montant ( gint amount_comparison_number );
static GtkWidget *cree_bouton_comparateur_texte ( gint text_comparison_number );
static GtkWidget *cree_bouton_lien ( GtkWidget *hbox );
static GtkWidget *cree_bouton_lien_lignes_comparaison ( void );
static GtkWidget *cree_bouton_operateur_txt ( gint text_comparison_number );
static GtkWidget *cree_ligne_comparaison_montant ( gint amount_comparison_number );
static GtkWidget *cree_ligne_comparaison_texte ( gint text_comparison_number );
static void etat_option_menu_changed (GtkWidget *optionmenu, GtkWidget *user_data);
static GtkWidget *onglet_affichage_etat_devises ( void );
static GtkWidget *onglet_affichage_etat_divers ( void );
static GtkWidget *onglet_affichage_etat_generalites ( void );
static GtkWidget *onglet_affichage_etat_operations ( void );
static GtkWidget *onglet_etat_categories ( void );
static GtkWidget *onglet_etat_comptes ( void );
static GtkWidget *onglet_etat_dates ( void );
static GtkWidget *onglet_etat_divers ( void );
static GtkWidget *onglet_etat_ib ( void );
static GtkWidget *onglet_etat_mode_paiement ( void );
static GtkWidget *onglet_etat_montant ( void );
static GtkWidget *onglet_etat_texte ( void );
static GtkWidget *onglet_etat_tiers ( void );
static GtkWidget *onglet_etat_virements ( void );
static GtkWidget *page_data_grouping ( void );
static GtkWidget *page_data_separation ( void );
static void recuperation_info_perso_etat ( void );
static void remplissage_liste_comptes_etats ( void );
static void remplissage_liste_comptes_virements ( void );
static void remplissage_liste_exo_etats ( void );
static void remplissage_liste_modes_paiement_etats ( void );
static void remplissage_liste_tiers_etats ( void );
static void remplit_liste_comparaisons_montants_etat ( void );
static void remplit_liste_comparaisons_textes_etat ( void );
static gboolean report_config_budget_select_all ( GtkWidget *button,
						  gboolean *select_ptr );
static gboolean report_config_budget_select_type ( GtkWidget *button,
						   gboolean *type_ptr );
static GSList *report_config_categ_budget_get_selected ( gboolean is_categ );
static gboolean report_config_categ_budget_toggled ( GtkCellRendererToggle *radio_renderer,
						     gchar *path_str,
						     GtkTreeStore *store );
static gboolean report_config_category_select_all ( GtkWidget *button,
						    gboolean *select_ptr );
static gboolean report_config_category_select_type ( GtkWidget *button,
						     gboolean *type_ptr );
static void report_config_category_toggle_categ ( struct_categ_budget_sel *categ_budget_struct,
						  gboolean is_categ );
static void report_config_category_update_treeview ( gboolean is_categ );
static GtkWidget *report_config_create_categ_budget_list ( void );
static gboolean report_config_fill_categ_budget_list ( gboolean is_categ );
static gboolean report_config_mix_select_all ( GtkTreeModel *model,
					       gboolean toggle_value );
static gboolean report_config_mix_select_type ( gboolean is_categ,
						gboolean type );
static void report_config_mixed_list_add_div ( gboolean is_categ,
					       const gchar *name,
					       gint number );
static void report_config_mixed_list_add_sub_div ( gboolean is_categ,
						   const gchar *name,
						   gint number );
static gboolean report_tree_selectable_func (GtkTreeSelection *selection,
				      GtkTreeModel *model,
				      GtkTreePath *path,
				      gboolean path_currently_selected,
				      gpointer data);
static gboolean report_tree_update_style ( gint * page_number );
static gboolean report_tree_update_style_iterator ( GtkTreeModel * tree_model,
					     GtkTreePath *path,
					     GtkTreeIter *iter,
					     gpointer data );
static gboolean report_tree_view_selection_changed ( GtkTreeSelection *selection,
					      GtkTreeModel *model );
static void retire_ligne_liste_comparaisons_montants_etat ( gint last_amount_comparison_number );
static void retire_ligne_liste_comparaisons_textes_etat ( gint last_text_comparison_number );
static void selectionne_devise_categ_etat_courant ( void );
static void selectionne_devise_ib_etat_courant ( void );
static void selectionne_devise_tiers_etat_courant ( void );
static void selectionne_liste_comptes_etat_courant ( void );
static void selectionne_liste_exo_etat_courant ( void );
static void selectionne_liste_modes_paiement_etat_courant ( void );
static void selectionne_liste_tiers_etat_courant ( void );
static void selectionne_liste_virements_etat_courant ( void );
static void selectionne_partie_liste_compte_etat ( gint *type_compte );
static void selectionne_partie_liste_compte_vir_etat ( gint *type_compte );
static void sensitive_hbox_fonction_bouton_txt ( gint text_comparison_number );
/*END_STATIC*/

/* the def of the columns in the categ and budget list
 * to filter by categ and budget */
enum
{
    REPORT_CATEG_BUDGET_LIST_NAME = 0,
    REPORT_CATEG_BUDGET_LIST_ACTIVE,
    REPORT_CATEG_BUDGET_LIST_ACTIVATABLE,
    REPORT_CATEG_BUDGET_LIST_NUMBER,
    REPORT_CATEG_BUDGET_LIST_SUB_NUMBER,

    REPORT_CATEG_BUDGET_LIST_NB,
};




/***********************************/
/* fichier etats_config.c */
/***********************************/

gchar *liste_plages_dates[] = {
    N_("All"),
    N_("Custom"),
    N_("Total to now"),
    N_("Current month"),
    N_("Current year"),
    N_("Current month to now"),
    N_("Current year to now"),
    N_("Previous month"),
    N_("Previous year"),
    N_("Last 30 days"),
    N_("Last 3 months"),
    N_("Last 6 months"),
    N_("Last 12 months"),
    NULL };

gchar *jours_semaine[] = {
    N_("Monday"),
    N_("Tuesday"),
    N_("Wednesday"),
    N_("Thursday"),
    N_("Friday"),
    N_("Saturday"),
    N_("Sunday"),
    NULL };

static GtkWidget *entree_nom_etat = NULL;
static GtkWidget *liste_type_classement_etat = NULL;
static GtkWidget *bouton_opes_r_et_non_r_etat = NULL;
static GtkWidget *bouton_opes_non_r_etat = NULL;
static GtkWidget *bouton_opes_r_etat = NULL;
static GtkWidget *bouton_afficher_opes = NULL;
static GtkWidget *bouton_afficher_nb_opes = NULL;
static GtkWidget *bouton_afficher_date_opes = NULL;
static GtkWidget *bouton_afficher_value_date_opes = NULL;
static GtkWidget *bouton_afficher_tiers_opes = NULL;
static GtkWidget *bouton_afficher_categ_opes = NULL;
static GtkWidget *bouton_afficher_sous_categ_opes = NULL;
static GtkWidget *bouton_afficher_ib_opes = NULL;
static GtkWidget *bouton_afficher_sous_ib_opes = NULL;
static GtkWidget *bouton_afficher_notes_opes = NULL;
static GtkWidget *bouton_afficher_pc_opes = NULL;
static GtkWidget *bouton_afficher_infobd_opes = NULL;
static GtkWidget *bouton_afficher_no_ope = NULL;
static GtkWidget *bouton_afficher_type_ope = NULL;
static GtkWidget *bouton_afficher_no_rappr = NULL;
static GtkWidget *bouton_afficher_no_cheque = NULL;
static GtkWidget *bouton_afficher_exo_opes = NULL;
static GtkWidget *bouton_pas_detailler_ventilation = NULL;
static GtkWidget *bouton_separer_revenus_depenses = NULL;
static GtkWidget *bouton_devise_general_etat = NULL;
static GtkWidget *bouton_inclure_dans_tiers = NULL;
static GtkWidget *bouton_afficher_titres_colonnes = NULL;
static GtkWidget *bouton_titre_changement = NULL;
static GtkWidget *bouton_titre_en_haut = NULL;
static GtkWidget *bouton_choix_classement_ope_etat = NULL;
static GtkWidget *bouton_rendre_ope_clickables = NULL;
static GtkWidget *vbox_show_transactions = NULL;

static GtkWidget *radio_button_utilise_exo = NULL;
static GtkWidget *radio_button_utilise_dates = NULL;
static GtkWidget *liste_exo_etat = NULL;
static GtkWidget *vbox_generale_exo_etat = NULL;
static GtkWidget *bouton_exo_tous = NULL;
static GtkWidget *bouton_exo_courant = NULL;
static GtkWidget *bouton_exo_precedent = NULL;
static GtkWidget *bouton_detaille_exo_etat = NULL;
static GtkWidget *bouton_separe_exo_etat = NULL;
static GtkWidget *liste_plages_dates_etat = NULL;
static GtkWidget *entree_date_init_etat = NULL;
static GtkWidget *entree_date_finale_etat = NULL;
static GtkWidget *vbox_utilisation_exo = NULL;
static GtkWidget *vbox_utilisation_date = NULL;
static GtkWidget *bouton_separe_plages_etat = NULL;
static GtkWidget *bouton_type_separe_plages_etat = NULL;
static GtkWidget *bouton_debut_semaine = NULL;

static GtkWidget *liste_comptes_etat = NULL;
static GtkWidget *bouton_detaille_comptes_etat = NULL;
static GtkWidget *vbox_generale_comptes_etat = NULL;
static GtkWidget *bouton_regroupe_ope_compte_etat = NULL;
static GtkWidget *bouton_affiche_sous_total_compte = NULL;
static GtkWidget *bouton_afficher_noms_comptes = NULL;

static GtkWidget *bouton_inclusion_virements_actifs_etat = NULL;
static GtkWidget *bouton_inclusion_virements_hors_etat = NULL;
static GtkWidget *bouton_non_inclusion_virements = NULL;
static GtkWidget *bouton_inclusion_virements_perso = NULL;
static GtkWidget *liste_comptes_virements = NULL;
static GtkWidget *bouton_exclure_non_virements_etat = NULL;
static GtkWidget *hbox_liste_comptes_virements = NULL;

static GtkWidget *button_group_by_categ = NULL;
static GtkWidget *button_detail_categ = NULL;
static GtkWidget *hbox_detaille_categ_etat = NULL;
static GtkWidget *bouton_afficher_sous_categ = NULL;
static GtkWidget *tree_view_categ = NULL;
static GtkWidget *bouton_affiche_sous_total_categ = NULL;
static GtkWidget *bouton_affiche_sous_total_sous_categ = NULL;
static GtkWidget *bouton_devise_categ_etat = NULL;
static GtkWidget *bouton_afficher_pas_de_sous_categ = NULL;
static GtkWidget *bouton_afficher_noms_categ = NULL;

static GtkWidget *bouton_utilise_ib_etat = NULL;
static GtkWidget *bouton_detaille_ib_etat = NULL;
static GtkWidget *hbox_detaille_ib_etat = NULL;
static GtkWidget *tree_view_budget = NULL;
static GtkWidget *bouton_afficher_sous_ib = NULL;
static GtkWidget *bouton_affiche_sous_total_ib = NULL;
static GtkWidget *bouton_affiche_sous_total_sous_ib = NULL;
static GtkWidget *bouton_devise_ib_etat = NULL;
static GtkWidget *bouton_afficher_pas_de_sous_ib = NULL;
static GtkWidget *bouton_afficher_noms_ib = NULL;

static GtkWidget *bouton_utilise_tiers_etat = NULL;
static GtkWidget *bouton_detaille_tiers_etat = NULL;
static GtkWidget *hbox_detaille_tiers_etat = NULL;
static GtkWidget *liste_tiers_etat = NULL;
static GtkWidget *bouton_affiche_sous_total_tiers = NULL;
static GtkWidget *bouton_devise_tiers_etat = NULL;
static GtkWidget *bouton_afficher_noms_tiers = NULL;

static GtkWidget *bouton_utilise_texte = NULL;
static GtkWidget *vbox_generale_textes_etat = NULL;
static GtkWidget *liste_textes_etat = NULL;

static GtkWidget *bouton_utilise_montant = NULL;
static GtkWidget *vbox_generale_montants_etat = NULL;
static GtkWidget *liste_montants_etat = NULL;
static GtkWidget *bouton_devise_montant_etat = NULL;
static GtkWidget *bouton_exclure_ope_nulles_etat = NULL;

gint ancienne_ib_etat;
gint ancienne_sous_ib_etat;
gint ancienne_categ_etat;
gint ancienne_categ_speciale_etat;
gint ancienne_sous_categ_etat;
gint ancien_compte_etat;
gint ancien_tiers_etat;
gsb_real montant_categ_etat;
gsb_real montant_sous_categ_etat;
gsb_real montant_ib_etat;
gsb_real montant_sous_ib_etat;
gsb_real montant_compte_etat;
gsb_real montant_tiers_etat;
gsb_real montant_periode_etat;
gsb_real montant_exo_etat;
gint nb_ope_categ_etat;
gint nb_ope_sous_categ_etat;
gint nb_ope_ib_etat;
gint nb_ope_sous_ib_etat;
gint nb_ope_compte_etat;
gint nb_ope_tiers_etat;
gint nb_ope_periode_etat;
gint nb_ope_exo_etat;
gint nb_ope_general_etat;
gint nb_ope_partie_etat;
GDate *date_debut_periode;
gint exo_en_cours_etat;
gint changement_de_groupe_etat;
gint debut_affichage_etat;
gint devise_compte_en_cours_etat;
gint devise_categ_etat;
gint devise_ib_etat;
gint devise_tiers_etat;
gint devise_generale_etat;

const gchar *nom_categ_en_cours;
const gchar *nom_ss_categ_en_cours;
const gchar *nom_ib_en_cours;
const gchar *nom_ss_ib_en_cours;
const gchar *nom_compte_en_cours;
const gchar *nom_tiers_en_cours;
gint titres_affiches;

static GtkWidget *bouton_detaille_mode_paiement_etat = NULL;
static GtkWidget *vbox_mode_paiement_etat = NULL;
static GtkWidget *liste_mode_paiement_etat = NULL;


static GtkTreeStore * report_tree_model = NULL;
static GtkWidget * report_tree_view = NULL;

/** globals used to speed up the category/budget list
 * and clarify the code */
static GtkTreeModel *model_categ;
static GtkTreeModel *model_budget;
static GtkTreeIter parent_iter;


/*START_EXTERN*/
extern GtkWidget * navigation_tree_view;
extern GtkWidget *notebook_config_etat;
extern GtkWidget *notebook_etats;
extern GtkWidget *notebook_general;
extern GtkWidget *window;
/*END_EXTERN*/



/******************************************************************************/
/* Fontion personnalistation_etat */
/* affiche la fenetre de personnalisation */
/******************************************************************************/
void personnalisation_etat (void)
{
    GtkWidget * dialog, *sw, *paned;
    GtkCTreeNode *parent;
    GSList *list_tmp;
    gint current_report_number, page = 0;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeIter iter, iter2;
    GtkTreeSelection *selection ;

    if ( !(current_report_number = gsb_gui_navigation_get_current_report()))
	return;

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general),
				GSB_REPORTS_PAGE );

    dialog = gtk_dialog_new_with_buttons ( _("Report properties"),
					   GTK_WINDOW ( window ),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );

    /* Create model */
    report_tree_model = gtk_tree_store_new ( REPORT_TREE_NUM_COLUMNS,
					     G_TYPE_STRING,
					     G_TYPE_INT,
					     G_TYPE_INT,
					     G_TYPE_INT );
    /* Create container + TreeView */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);
    report_tree_view = gtk_tree_view_new();
    gtk_tree_view_set_model (GTK_TREE_VIEW (report_tree_view),
			     GTK_TREE_MODEL (report_tree_model));
    g_object_unref (G_OBJECT(report_tree_model));

    /* Make column */
    cell = gtk_cell_renderer_text_new ();
    column =
	gtk_tree_view_column_new_with_attributes ("Categories",
						  cell,
						  "text", REPORT_TREE_TEXT_COLUMN,
						  "weight", REPORT_TREE_BOLD_COLUMN,
						  "style", REPORT_TREE_ITALIC_COLUMN,
						  NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (report_tree_view),
				 GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW (report_tree_view), FALSE );

    /* Handle select */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (report_tree_view));
    g_signal_connect (selection, "changed",
		      ((GCallback) report_tree_view_selection_changed),
		      report_tree_model);

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function ( selection, report_tree_selectable_func,
					     NULL, NULL );

    /* Put the tree in the scroll */
    gtk_container_add (GTK_CONTAINER (sw), report_tree_view);

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect (report_tree_view, "realize",
		      ((GCallback)gtk_tree_view_expand_all), NULL);

    paned = gtk_hpaned_new();
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), paned,
			 TRUE, TRUE, 0 );
    gtk_box_set_spacing ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER ( paned ), 0 );

    gtk_paned_add1(GTK_PANED(paned), sw);

    /* on commence par créer le notebook parent de la config des états */
    notebook_config_etat = gtk_notebook_new ();
    gtk_paned_add2(GTK_PANED(paned), notebook_config_etat );

    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook_config_etat ),
				 FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK ( notebook_config_etat ),
				   FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER ( notebook_config_etat ), 0 );

    /* on ajoute les onglets dans l'onglet de base correspondant */

    /* remplissage de l'onglet de sélection */


    /* Display subtree */
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter,
			REPORT_TREE_TEXT_COLUMN, _("Data selection"),
			REPORT_TREE_PAGE_COLUMN, -1,
			REPORT_TREE_BOLD_COLUMN, 800,
			-1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_dates (),
			       gtk_label_new ( _("Dates") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Dates"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_virements (),
			       gtk_label_new ( _("Transfers") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Transfers"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_comptes (),
			       gtk_label_new ( _("Accounts") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Accounts"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_tiers (),
			       gtk_label_new ( _("Payee") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Payee"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_categories (),
			       gtk_label_new ( _("Categories") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Categories"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_ib (),
			       gtk_label_new ( _("Budgetary lines") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Budgetary lines"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_texte (),
			       gtk_label_new ( _("Texts") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Texts"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_montant (),
			       gtk_label_new ( _("Amounts") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Amounts"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_mode_paiement (),
			       gtk_label_new ( _("Payment methods") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Payment methods"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_etat_divers (),
			       gtk_label_new ( _("Misc.") ) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Miscellaneous"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);


    /* remplissage de l'onglet d'organisation */
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter,
			REPORT_TREE_TEXT_COLUMN, _("Data organization"),
			REPORT_TREE_PAGE_COLUMN, -1,
			REPORT_TREE_BOLD_COLUMN, 800, -1);

    /* Data grouping */
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter );
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Data grouping"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       page_data_grouping(),
			       gtk_label_new (_("Data grouping")) );

    /* Data separation */
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Data separation"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       page_data_separation(),
			       gtk_label_new (_("Data separation")) );

    /* remplissage de l'onglet d'affichage */
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter,
			REPORT_TREE_TEXT_COLUMN, _("Data display"),
			REPORT_TREE_PAGE_COLUMN, -1,
			REPORT_TREE_BOLD_COLUMN, 800, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_affichage_etat_generalites (),
			       gtk_label_new (_("Generalities")) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Generalities"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_affichage_etat_divers (),
			       gtk_label_new (_("Titles")) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Titles"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_affichage_etat_operations (),
			       gtk_label_new (_("Transactions")) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Transactions"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
			       onglet_affichage_etat_devises (),
			       gtk_label_new (_("Currencies")) );
    gtk_tree_store_append (GTK_TREE_STORE (report_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (report_tree_model), &iter2,
			REPORT_TREE_TEXT_COLUMN, _("Currencies"),
			REPORT_TREE_PAGE_COLUMN, page++,
			REPORT_TREE_BOLD_COLUMN, 400, -1);


    /* on va maintenant remplir toutes les infos de l'état */

    /* onglet généralités */

    /* on met le name de l'état */
    gtk_entry_set_text ( GTK_ENTRY ( entree_nom_etat ),
			 gsb_data_report_get_report_name (current_report_number) );

    /* on remplit le ctree en fonction du classement courant */

    list_tmp = gsb_data_report_get_sorting_type (current_report_number);
    parent = NULL;
    gtk_clist_clear ( GTK_CLIST ( liste_type_classement_etat ));

    while ( list_tmp )
    {
	gchar *text[1];

	text[0] = NULL;

	switch ( GPOINTER_TO_INT ( list_tmp -> data ))
	{
	    case 1:
		text[0] = _("Category");
		break;

	    case 3:
		text[0] = _("Budgetary line");
		break;

	    case 5:
		text[0] = _("Account");
		break;

	    case 6:
		text[0] = _("Payee");
		break;

	}

	if ( text[0] )
	{
	    parent = gtk_ctree_insert_node ( GTK_CTREE ( liste_type_classement_etat ),
					     parent,
					     NULL,
					     text,
					     5,
					     NULL,
					     NULL,
					     NULL,
					     NULL,
					     FALSE,
					     TRUE );
	    gtk_ctree_node_set_row_data ( GTK_CTREE ( liste_type_classement_etat ),
					  GTK_CTREE_NODE ( parent ),
					  list_tmp -> data );
	}

	list_tmp = list_tmp -> next;
    }

    gtk_clist_select_row ( GTK_CLIST ( liste_type_classement_etat ),
			   0,
			   0 );

    if ( gsb_data_report_get_show_r (current_report_number))
    {
	if ( gsb_data_report_get_show_r (current_report_number) == 1 )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_non_r_etat ),
					   TRUE );
	else
	    if (gsb_data_report_get_show_r (current_report_number))
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_etat ),
					       TRUE );
    }
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat ),
				       TRUE );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ),
				   gsb_data_report_get_show_report_transactions (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_opes ),
				   gsb_data_report_get_show_report_transaction_amount (current_report_number));

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_ope ),
				   gsb_data_report_get_show_report_transaction_number (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ),
				   gsb_data_report_get_show_report_date (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_value_date_opes ),
				   gsb_data_report_get_show_report_value_date (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ),
				   gsb_data_report_get_show_report_payee (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ),
				   gsb_data_report_get_show_report_category (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ),
				   gsb_data_report_get_show_report_sub_category (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_type_ope ),
				   gsb_data_report_get_show_report_method_of_payment (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ),
				   gsb_data_report_get_show_report_budget (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ),
				   gsb_data_report_get_show_report_sub_budget (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_cheque ),
				   gsb_data_report_get_show_report_method_of_payment_content (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ),
				   gsb_data_report_get_show_report_note (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ),
				   gsb_data_report_get_show_report_voucher (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_rappr ),
				   gsb_data_report_get_show_report_marked (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ),
				   gsb_data_report_get_show_report_bank_references (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation ),
				   gsb_data_report_get_not_detail_split (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separer_revenus_depenses ),
				   gsb_data_report_get_split_credit_debit (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_exo_opes ),
				   gsb_data_report_get_show_report_financial_year (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_titres_colonnes ),
				   gsb_data_report_get_column_title_show (current_report_number));

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_classement_ope_etat ),
				  gsb_data_report_get_sorting_report (current_report_number));

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_rendre_ope_clickables ),
				   gsb_data_report_get_report_can_click (current_report_number));

    if ( !gsb_data_report_get_column_title_type (current_report_number))
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_titre_en_haut ),
				       TRUE );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclure_dans_tiers ),
				   gsb_data_report_get_append_in_payee (current_report_number));

    /* on rend insensitif les sous qque choses si nécessaire */
    sens_desensitive_pointeur ( bouton_afficher_opes,
				vbox_show_transactions );
    sens_desensitive_pointeur ( bouton_afficher_categ_opes,
				bouton_afficher_sous_categ_opes );
    sens_desensitive_pointeur ( bouton_afficher_ib_opes,
				bouton_afficher_sous_ib_opes );
    sens_desensitive_pointeur ( bouton_afficher_titres_colonnes,
				bouton_titre_changement );
    sens_desensitive_pointeur ( bouton_afficher_titres_colonnes,
				bouton_titre_en_haut );

    /* mise en forme de la devise */
    gsb_currency_set_combobox_history ( bouton_devise_general_etat,
					gsb_data_report_get_currency_general (current_report_number));

    /* onglet dates */


    if ( gsb_data_report_get_use_financial_year (current_report_number))
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ),
				       TRUE );
	gtk_widget_set_sensitive ( vbox_utilisation_date,
				   FALSE );
    }
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_dates ),
				       TRUE );

    sens_desensitive_pointeur ( radio_button_utilise_exo,
				bouton_separe_exo_etat );

    if ( gsb_data_report_get_financial_year_type (current_report_number) == 3 )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat ),
				       TRUE );
    else
    {
	gtk_widget_set_sensitive ( vbox_generale_exo_etat,
				   FALSE );

	if ( gsb_data_report_get_financial_year_type (current_report_number))
	{
	    if ( gsb_data_report_get_financial_year_type (current_report_number) == 1 )
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_courant ),
					       TRUE );
	    else
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_precedent ),
					       TRUE );
	}
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_tous ),
					   TRUE );
    }


    /* on sélectionne les exercices */

    selectionne_liste_exo_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ),
				   gsb_data_report_get_financial_year_split (current_report_number));

    /* on sélectionne la plage de date */

    gtk_clist_select_row ( GTK_CLIST ( liste_plages_dates_etat ),
			   gsb_data_report_get_date_type (current_report_number),
			   0 );

    if ( gsb_data_report_get_date_type (current_report_number) != 1 )
    {
	gtk_widget_set_sensitive ( entree_date_init_etat,
				   FALSE );
	gtk_widget_set_sensitive ( entree_date_finale_etat,
				   FALSE );
    }
    else
    {
	gtk_widget_set_sensitive ( entree_date_init_etat,
				   TRUE );
	gtk_widget_set_sensitive ( entree_date_finale_etat,
				   TRUE );
    }

    /* on remplit les dates perso si elles existent */

    if ( gsb_data_report_get_personal_date_start (current_report_number))
	gsb_calendar_entry_set_date ( entree_date_init_etat,
				      gsb_data_report_get_personal_date_start (current_report_number));

    if ( gsb_data_report_get_personal_date_end (current_report_number))
	gsb_calendar_entry_set_date ( entree_date_finale_etat,
				      gsb_data_report_get_personal_date_end (current_report_number));

    /* on remplit les détails de la séparation des dates */
    if ( gsb_data_report_get_period_split (current_report_number))
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat ),
				       TRUE );
	if (gtk_option_menu_get_history ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat )) == 1)
        gtk_widget_set_sensitive ( bouton_debut_semaine,
				   TRUE );
    else
        gtk_widget_set_sensitive ( bouton_debut_semaine,
				   FALSE );
    }
    else
    {
	gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				   FALSE );
	gtk_widget_set_sensitive ( bouton_debut_semaine,
				   FALSE );
    }

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ),
				  gsb_data_report_get_period_split_type (current_report_number));
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_debut_semaine ),
				  gsb_data_report_get_period_split_day (current_report_number));

    /* onglet comptes */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat ),
				   gsb_data_report_get_account_use_chosen (current_report_number));

    sens_desensitive_pointeur ( bouton_detaille_comptes_etat,
				vbox_generale_comptes_etat );


    selectionne_liste_comptes_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat ),
				   gsb_data_report_get_account_group_reports (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte ),
				   gsb_data_report_get_account_show_amount (current_report_number));

    sens_desensitive_pointeur ( bouton_regroupe_ope_compte_etat,
				bouton_affiche_sous_total_compte );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_comptes ),
				   gsb_data_report_get_account_show_name (current_report_number));


    /* onglet virements */


    if ( gsb_data_report_get_transfer_choice (current_report_number))
    {
	if ( gsb_data_report_get_transfer_choice (current_report_number)== 1 )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat ),
					   TRUE );
	else
	{
	    if ( gsb_data_report_get_transfer_choice (current_report_number)== 2 )
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat ),
					       TRUE );
	    else
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_perso ),
					       TRUE );
	}
    }
    else
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements ),
				       TRUE );
	gtk_widget_set_sensitive ( bouton_exclure_non_virements_etat,
				   FALSE );
    }

    sens_desensitive_pointeur ( bouton_inclusion_virements_perso,
				hbox_liste_comptes_virements );

    selectionne_liste_virements_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_non_virements_etat ),
				   gsb_data_report_get_transfer_reports_only (current_report_number));


    /* onglet catégories */
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_group_by_categ ),
				   gsb_data_report_get_category_used (current_report_number));

    sens_desensitive_pointeur ( button_group_by_categ,
				bouton_affiche_sous_total_categ );
    sens_desensitive_pointeur ( button_group_by_categ,
				bouton_afficher_sous_categ );
    sens_desensitive_pointeur ( button_group_by_categ,
				bouton_affiche_sous_total_sous_categ );
    sens_desensitive_pointeur ( button_group_by_categ,
				bouton_afficher_pas_de_sous_categ );
    sens_desensitive_pointeur ( button_group_by_categ,
				bouton_afficher_noms_categ );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_detail_categ ),
				   gsb_data_report_get_category_detail_used (current_report_number));

    sens_desensitive_pointeur ( button_detail_categ,
				hbox_detaille_categ_etat );

    /* select the categories and sub-categories */
    report_config_category_update_treeview (TRUE);

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ ),
				   gsb_data_report_get_category_show_category_amount (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ ),
				   gsb_data_report_get_category_show_sub_category (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ ),
				   gsb_data_report_get_category_show_sub_category_amount (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_categ ),
				   gsb_data_report_get_category_show_without_category (current_report_number));


    /* mise en forme de la devise */

    selectionne_devise_categ_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_categ ),
				   gsb_data_report_get_category_show_name (current_report_number));


    /* onglet ib */
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat ),
				   gsb_data_report_get_budget_used (current_report_number));

    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_affiche_sous_total_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_afficher_sous_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_affiche_sous_total_sous_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_afficher_pas_de_sous_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_afficher_noms_ib );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ),
				   gsb_data_report_get_budget_detail_used (current_report_number));

    sens_desensitive_pointeur ( bouton_detaille_ib_etat,
				hbox_detaille_ib_etat );

    /* select the budgets and sub-budgets */
    report_config_category_update_treeview (FALSE);

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ),
				   gsb_data_report_get_budget_show_budget_amount (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib ),
				   gsb_data_report_get_budget_show_sub_budget (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ),
				   gsb_data_report_get_budget_show_sub_budget_amount (current_report_number));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_ib ),
				   gsb_data_report_get_budget_show_without_budget (current_report_number));

    /* mise en forme de la devise */

    selectionne_devise_ib_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_ib ),
				   gsb_data_report_get_budget_show_name (current_report_number));

    /* onglet tiers */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat ),
				   gsb_data_report_get_payee_used (current_report_number));

    sens_desensitive_pointeur ( bouton_utilise_tiers_etat,
				bouton_afficher_noms_tiers );
    sens_desensitive_pointeur ( bouton_utilise_tiers_etat,
				bouton_affiche_sous_total_tiers );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat ),
				   gsb_data_report_get_payee_detail_used (current_report_number));

    sens_desensitive_pointeur ( bouton_detaille_tiers_etat,
				hbox_detaille_tiers_etat );


    /* on sélectionne les tiers choisies */

    selectionne_liste_tiers_etat_courant();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers ),
				   gsb_data_report_get_payee_show_payee_amount (current_report_number));

    /* mise en forme de la devise */

    selectionne_devise_tiers_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_tiers ),
				   gsb_data_report_get_payee_show_name (current_report_number));

    /*  onglet texte */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_texte ),
				   gsb_data_report_get_text_comparison_used (current_report_number));
    sens_desensitive_pointeur ( bouton_utilise_texte,
				vbox_generale_textes_etat );
    remplit_liste_comparaisons_textes_etat ();


    /* onglet montant */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_montant ),
				   gsb_data_report_get_amount_comparison_used (current_report_number));
    gsb_currency_set_combobox_history ( bouton_devise_montant_etat,
					gsb_data_report_get_amount_comparison_currency (current_report_number));
    sens_desensitive_pointeur ( bouton_utilise_montant,
				vbox_generale_montants_etat );
    remplit_liste_comparaisons_montants_etat ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_nulles_etat ),
				   gsb_data_report_get_amount_comparison_only_report_non_null (current_report_number));

    /* onglet modes de paiement */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_mode_paiement_etat ),
				   gsb_data_report_get_method_of_payment_used (current_report_number));

    sens_desensitive_pointeur ( bouton_detaille_mode_paiement_etat,
				vbox_mode_paiement_etat );

    selectionne_liste_modes_paiement_etat_courant ();

    gtk_widget_show_all ( dialog );

    switch ( gtk_dialog_run ( GTK_DIALOG(dialog) ) )
    {
	case GTK_RESPONSE_OK:
	    recuperation_info_perso_etat ();
	    break;

	default:
	    break;
    }

    gtk_widget_destroy ( dialog );
}



/**
 *
 *
 *
 */
gboolean report_tree_view_selection_changed ( GtkTreeSelection *selection,
					      GtkTreeModel *model )
{
    GtkTreeIter iter;
    gint selected;

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    gtk_tree_model_get ( model, &iter, 1, &selected, -1 );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_config_etat ), selected);

    return FALSE;
}



gboolean report_tree_selectable_func (GtkTreeSelection *selection,
				      GtkTreeModel *model,
				      GtkTreePath *path,
				      gboolean path_currently_selected,
				      gpointer data)
{
    GtkTreeIter iter;
    gint selectable;

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get ( model, &iter, 1, &selectable, -1 );

    return ( selectable != -1 );
}



/******************************************************************************/
void selectionne_liste_exo_etat_courant ( void )
{
    GSList *pointeur_sliste;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    gtk_clist_unselect_all ( GTK_CLIST ( liste_exo_etat ));

    pointeur_sliste = gsb_data_report_get_financial_year_list (current_report_number);

    while ( pointeur_sliste )
    {
	gint row;

	row = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_exo_etat ),
					     pointeur_sliste -> data );
	gtk_clist_select_row ( GTK_CLIST ( liste_exo_etat ),
			       row,
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_comptes_etat_courant ( void )
{
    GSList *pointeur_sliste;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( !liste_comptes_etat )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_etat ));

    pointeur_sliste = gsb_data_report_get_account_numbers (current_report_number);

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_comptes_etat ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_comptes_etat ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_virements_etat_courant ( void )
{
    GSList *pointeur_sliste;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_virements ));

    pointeur_sliste = gsb_data_report_get_transfer_account_numbers (current_report_number);

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_comptes_virements ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_comptes_virements ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}



void selectionne_devise_categ_etat_courant ( void )
{
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    gsb_currency_set_combobox_history ( bouton_devise_categ_etat,
					gsb_data_report_get_category_currency (current_report_number));
}
/******************************************************************************/


/******************************************************************************/
void selectionne_devise_ib_etat_courant ( void )
{
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();
    gsb_currency_set_combobox_history ( bouton_devise_ib_etat,
					gsb_data_report_get_budget_currency (current_report_number));
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_tiers_etat_courant ( void )
{
    GSList *pointeur_sliste;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( !liste_tiers_etat )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_tiers_etat ));

    pointeur_sliste = gsb_data_report_get_payee_numbers (current_report_number);

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_tiers_etat ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_tiers_etat ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_devise_tiers_etat_courant ( void )
{
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();
    gsb_currency_set_combobox_history ( bouton_devise_tiers_etat,
					gsb_data_report_get_payee_currency (current_report_number));
}
/******************************************************************************/

/******************************************************************************/
void recuperation_info_perso_etat ( void )
{
    GSList *comparison_list;
    GList *list_tmp;
    const gchar *pointeur_char;
    gint i;
    gint amount_comparison_number;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    /* Check that custom dates are OK, but only if custom date range
     * has been selected. */
    if ( GPOINTER_TO_INT ( GTK_CLIST ( liste_plages_dates_etat ) -> selection -> data ) == 1 &&
	 !gsb_date_check_entry ( entree_date_init_etat ) )
    {
	dialogue_error_hint ( _("Grisbi can't parse date.  For a list of date formats that Grisbi can use, refer to Grisbi manual."),
			      g_strdup_printf ( _("Invalid initial date '%s'"),
						gtk_entry_get_text(GTK_ENTRY(entree_date_init_etat)) ) );
	return;
    }

    if ( GPOINTER_TO_INT ( GTK_CLIST ( liste_plages_dates_etat ) -> selection -> data ) == 1 &&
	 !gsb_date_check_entry (entree_date_finale_etat ) )
    {
	dialogue_error_hint ( _("Grisbi can't parse date.  For a list of date formats that Grisbi can use, refer to Grisbi manual."),
			      g_strdup_printf ( _("Invalid final date '%s'"),
						gtk_entry_get_text(GTK_ENTRY(entree_date_finale_etat)) ) );
	return;
    }


    /* on récupère maintenant toutes les données */
    /* récupération du name du rapport */

    pointeur_char = gtk_entry_get_text ( GTK_ENTRY ( entree_nom_etat ));

    if ( strlen ( pointeur_char )
	 &&
	 strcmp ( pointeur_char,
		  gsb_data_report_get_report_name (current_report_number) ))
    {
	gsb_data_report_set_report_name ( current_report_number,
					  pointeur_char );
    }

    /* récupération du type de classement */

    g_slist_free ( gsb_data_report_get_sorting_type (current_report_number));

    gsb_data_report_set_sorting_type ( current_report_number,
				       NULL );

    for ( i=0 ; i<GTK_CLIST ( liste_type_classement_etat ) -> rows ; i++ )
    {
	gint no;

	no = GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( liste_type_classement_etat ),
							     gtk_ctree_node_nth ( GTK_CTREE ( liste_type_classement_etat ),
										  i )));

	gsb_data_report_set_sorting_type ( current_report_number,
					   g_slist_append ( gsb_data_report_get_sorting_type (current_report_number),
							    GINT_TO_POINTER ( no )));

	/* rajoute les ss categ et ss ib */

	if ( no == 1 )
	    gsb_data_report_set_sorting_type ( current_report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (current_report_number),
								GINT_TO_POINTER ( 2 )));
	if ( no == 3 )
	    gsb_data_report_set_sorting_type ( current_report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (current_report_number),
								GINT_TO_POINTER ( 4 )));
    }

    /* récupération de l'affichage ou non des R */

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat )) )
	gsb_data_report_set_show_r ( current_report_number,
				     0 );
    else
    {
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_non_r_etat )) )
	    gsb_data_report_set_show_r ( current_report_number,
					 1 );
	else
	    gsb_data_report_set_show_r ( current_report_number,
					 2 );
    }

    /* récupération de l'affichage des opés */

    gsb_data_report_set_show_report_transactions ( current_report_number,
						   gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes )));
    gsb_data_report_set_show_report_transaction_amount ( current_report_number,
							 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_opes )));

    gsb_data_report_set_show_report_transaction_number ( current_report_number,
							 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_ope )));
    gsb_data_report_set_show_report_date ( current_report_number,
					   gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes )));
    gsb_data_report_set_show_report_value_date ( current_report_number,
						 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_value_date_opes )));
    gsb_data_report_set_show_report_payee ( current_report_number,
					    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes )));
    gsb_data_report_set_show_report_category ( current_report_number,
					       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes )));
    gsb_data_report_set_show_report_sub_category ( current_report_number,
						   gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes )));
    gsb_data_report_set_show_report_method_of_payment ( current_report_number,
							gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_type_ope )));
    gsb_data_report_set_show_report_budget ( current_report_number,
					     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes )));
    gsb_data_report_set_show_report_sub_budget ( current_report_number,
						 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes )));
    gsb_data_report_set_show_report_method_of_payment_content ( current_report_number,
								gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_cheque )));
    gsb_data_report_set_show_report_note ( current_report_number,
					   gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes )));
    gsb_data_report_set_show_report_voucher ( current_report_number,
					      gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes )));
    gsb_data_report_set_show_report_marked ( current_report_number,
					     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_rappr )));
    gsb_data_report_set_show_report_bank_references ( current_report_number,
						      gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes )));
    gsb_data_report_set_not_detail_split ( current_report_number,
					       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation )));
    gsb_data_report_set_split_credit_debit ( current_report_number,
					     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separer_revenus_depenses )));
    gsb_data_report_set_show_report_financial_year ( current_report_number,
						     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_exo_opes )));
    gsb_data_report_set_column_title_show ( current_report_number,
					    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_titres_colonnes )));
    gsb_data_report_set_column_title_type ( current_report_number,
					    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_titre_changement )));

    gsb_data_report_set_sorting_report ( current_report_number,
					 GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_choix_classement_ope_etat ) -> menu_item ),
										 "no_classement" )));

    gsb_data_report_set_report_can_click ( current_report_number,
					   gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_rendre_ope_clickables )));

    gsb_data_report_set_currency_general ( current_report_number,
					   gsb_currency_get_currency_from_combobox (bouton_devise_general_etat));
    gsb_data_report_set_append_in_payee ( current_report_number,
					  gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclure_dans_tiers )));


    /* récupération des dates */

    gsb_data_report_set_use_financial_year ( current_report_number,
					     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo )));

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_tous )) )
	gsb_data_report_set_financial_year_type ( current_report_number,
						  0 );
    else
    {
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_courant )) )
	    gsb_data_report_set_financial_year_type ( current_report_number,
						      1 );
	else
	{
	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_precedent )) )
		gsb_data_report_set_financial_year_type ( current_report_number,
							  2 );
	    else
		gsb_data_report_set_financial_year_type ( current_report_number,
							  3 );
	}
    }

    if ( gsb_data_report_get_financial_year_list (current_report_number))
    {
	g_slist_free ( gsb_data_report_get_financial_year_list (current_report_number));
	gsb_data_report_set_financial_year_list ( current_report_number,
						  NULL );
    }

    list_tmp = GTK_CLIST ( liste_exo_etat ) -> selection;

    while ( list_tmp )
    {
	gsb_data_report_set_financial_year_list ( current_report_number,
						  g_slist_append ( gsb_data_report_get_financial_year_list (current_report_number),
								   gtk_clist_get_row_data ( GTK_CLIST ( liste_exo_etat ),
											    GPOINTER_TO_INT ( list_tmp -> data ))));
	list_tmp = list_tmp -> next;
    }

    /*   si tous les exos ont été sélectionnés, on met bouton_detaille_exo_etat à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_exo_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_exo_etat ) -> rows )
	 &&
	 gsb_data_report_get_financial_year_type (current_report_number) == 3 )
    {
	dialogue_special ( GTK_MESSAGE_INFO,
			   make_hint ( _("Performance issue."),
				       _("All financial years have been selected.  Grisbi will run faster without the \"Detail financial years\" option activated.") ) );
	gsb_data_report_set_financial_year_type ( current_report_number,
						  0 );
    }


    gsb_data_report_set_financial_year_split ( current_report_number,
					       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat )));

    gsb_data_report_set_date_type ( current_report_number,
				    GPOINTER_TO_INT ( GTK_CLIST ( liste_plages_dates_etat ) -> selection -> data ));

    gsb_data_report_set_personal_date_start ( current_report_number,
					      gsb_calendar_entry_get_date (entree_date_init_etat));
    gsb_data_report_set_personal_date_end ( current_report_number,
					    gsb_calendar_entry_get_date (entree_date_finale_etat));

    gsb_data_report_set_period_split ( current_report_number,
				       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat )));
    gsb_data_report_set_period_split_type ( current_report_number,
					    GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ) -> menu_item ),
										    "type" )));
    gsb_data_report_set_period_split_day ( current_report_number,
					   GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_debut_semaine ) -> menu_item ),
										   "day" )));

    /* récupération des comptes */

    gsb_data_report_set_account_use_chosen ( current_report_number,
					     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat )));

    if ( gsb_data_report_get_account_numbers (current_report_number))
    {
	g_slist_free ( gsb_data_report_get_account_numbers (current_report_number));
	gsb_data_report_set_account_numbers ( current_report_number,
					      NULL );
    }

    list_tmp = GTK_CLIST ( liste_comptes_etat ) -> selection;

    while ( list_tmp )
    {
	gsb_data_report_set_account_numbers ( current_report_number,
					      g_slist_append ( gsb_data_report_get_account_numbers (current_report_number),
							       gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_etat ),
											GPOINTER_TO_INT ( list_tmp -> data ))));
	list_tmp = list_tmp -> next;
    }

    /*   si tous les comptes ont été sélectionnés, on met utilise_detail_comptes à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_comptes_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_comptes_etat ) -> rows )
	 &&
	 gsb_data_report_get_account_use_chosen (current_report_number))
    {
	dialogue_special ( GTK_MESSAGE_INFO,
			   make_hint ( _("Performance issue."),
				       _("All accounts have been selected.  Grisbi will run faster without the \"Detail accounts used\" option activated.") ) );
	gsb_data_report_set_account_use_chosen ( current_report_number,
						 0 );
    }

    gsb_data_report_set_account_group_reports ( current_report_number,
						gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat )));
    gsb_data_report_set_account_show_amount ( current_report_number,
					      gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte )));
    gsb_data_report_set_account_show_name ( current_report_number,
					    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_comptes )));


    /* récupération des virements */

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat )))
	gsb_data_report_set_transfer_choice ( current_report_number,
					      1 );
    else
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat )))
	    gsb_data_report_set_transfer_choice ( current_report_number,
						  2 );
	else
	{
	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements )))
		gsb_data_report_set_transfer_choice ( current_report_number,
						      0 );
	    else
		gsb_data_report_set_transfer_choice ( current_report_number,
						      3 );
	}

    if ( gsb_data_report_get_transfer_account_numbers (current_report_number))
    {
	g_slist_free ( gsb_data_report_get_transfer_account_numbers (current_report_number));
	gsb_data_report_set_transfer_account_numbers ( current_report_number,
						       NULL );
    }

    list_tmp = GTK_CLIST ( liste_comptes_virements ) -> selection;

    while ( list_tmp )
    {
	gsb_data_report_set_transfer_account_numbers ( current_report_number,
						       g_slist_append ( gsb_data_report_get_transfer_account_numbers (current_report_number),
									gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_virements ),
												 GPOINTER_TO_INT ( list_tmp -> data ))));
	list_tmp = list_tmp -> next;
    }

    gsb_data_report_set_transfer_reports_only ( current_report_number,
						gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_non_virements_etat )));


    /* get the categories */
    gsb_data_report_set_category_used ( current_report_number,
					gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_group_by_categ )));

    gsb_data_report_set_category_detail_used ( current_report_number,
					       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_detail_categ )));

    gsb_data_report_set_category_struct (current_report_number, report_config_categ_budget_get_selected (TRUE));

    /* if all was selected, remove the category_detail_used */
    if (!gsb_data_report_get_category_struct (current_report_number))
	gsb_data_report_set_category_detail_used ( current_report_number,
						   FALSE );

    gsb_data_report_set_category_show_category_amount ( current_report_number,
							gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ )));
    gsb_data_report_set_category_show_sub_category ( current_report_number,
						     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ )));
    gsb_data_report_set_category_show_sub_category_amount ( current_report_number,
							    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ )));
    gsb_data_report_set_category_show_without_category ( current_report_number,
							 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_categ )));

    gsb_data_report_set_category_currency ( current_report_number,
					    gsb_currency_get_currency_from_combobox (bouton_devise_categ_etat));
    gsb_data_report_set_category_show_name ( current_report_number,
					     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_categ )));


    /* get the budget */
    gsb_data_report_set_budget_used ( current_report_number,
				      gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat )));

    gsb_data_report_set_budget_detail_used ( current_report_number,
					     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat )));

    gsb_data_report_set_budget_struct (current_report_number, report_config_categ_budget_get_selected (FALSE));

    /* if all was selected, remove the budget_detail_used */
    if (!gsb_data_report_get_budget_struct (current_report_number))
	gsb_data_report_set_budget_detail_used ( current_report_number,
						 FALSE );

    gsb_data_report_set_budget_show_budget_amount ( current_report_number,
						    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib )));
    gsb_data_report_set_budget_show_sub_budget ( current_report_number,
						 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (bouton_afficher_sous_ib)));
    gsb_data_report_set_budget_show_sub_budget_amount ( current_report_number,
							gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib )));
    gsb_data_report_set_budget_show_without_budget ( current_report_number,
						     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_ib )));

    gsb_data_report_set_budget_currency ( current_report_number,
					  gsb_currency_get_currency_from_combobox (bouton_devise_ib_etat));
    gsb_data_report_set_budget_show_name ( current_report_number,
					   gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_ib )));



    /*   récupération des tiers */

    gsb_data_report_set_payee_used ( current_report_number,
				     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat )));

    gsb_data_report_set_payee_detail_used ( current_report_number,
					    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat )));

    if ( gsb_data_report_get_payee_numbers (current_report_number))
    {
	g_slist_free ( gsb_data_report_get_payee_numbers (current_report_number));
	gsb_data_report_set_payee_numbers ( current_report_number,
					    NULL );
    }

    list_tmp = GTK_CLIST ( liste_tiers_etat ) -> selection;

    while ( list_tmp )
    {
	gsb_data_report_set_payee_numbers ( current_report_number,
					    g_slist_append ( gsb_data_report_get_payee_numbers (current_report_number),
							     gtk_clist_get_row_data ( GTK_CLIST ( liste_tiers_etat ),
										      GPOINTER_TO_INT ( list_tmp -> data ))));
	list_tmp = list_tmp -> next;
    }

    /*   si tous les tiers ont été sélectionnés, on met utilise_detail_tiers à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_tiers_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_tiers_etat ) -> rows )
	 &&
	 gsb_data_report_get_payee_detail_used (current_report_number))
    {
	dialogue_special ( GTK_MESSAGE_INFO,
			   make_hint ( _("Performance issue."),
				       _("All payees have been selected.  Grisbi will run faster without the \"Detail payees used\" option activated.") ) );
	gsb_data_report_set_payee_detail_used ( current_report_number,
						0 );
    }

    gsb_data_report_set_payee_show_payee_amount ( current_report_number,
						  gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers )));

    gsb_data_report_set_payee_currency ( current_report_number,
					 gsb_currency_get_currency_from_combobox (bouton_devise_tiers_etat));
    gsb_data_report_set_payee_show_name ( current_report_number,
					  gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_tiers )));



    /* récupération du texte */

    gsb_data_report_set_text_comparison_used ( current_report_number,
					       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_texte )));

    /* récupération de la liste des comparaisons de texte */
    /*   il y a au moins une structure de créée, si celle si a l'entrée txt et 2 montants vides, */
    /* c'est qu'il n'y a aucune liste */

    /*     text_comparison_number = GPOINTER_TO_INT (gsb_data_report_get_text_comparison_list (current_report_number)-> data); */

    /* on a rentré au moins une comparaison */
    /* on rempli les champs de la structure */

    comparison_list = gsb_data_report_get_text_comparison_list (current_report_number);

    while ( comparison_list )
    {
	const gchar *string;
	gint text_comparison_number;

	text_comparison_number = GPOINTER_TO_INT (comparison_list -> data);

	if ( gsb_data_report_text_comparison_get_button_link (text_comparison_number))
	    gsb_data_report_text_comparison_set_link_to_last_text_comparison ( text_comparison_number,
									       GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_link (text_comparison_number)) -> menu_item ),
														       "no_lien" )));
	else
	    gsb_data_report_text_comparison_set_link_to_last_text_comparison ( text_comparison_number,
									       -1 );

	gsb_data_report_text_comparison_set_field ( text_comparison_number,
						    GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_field (text_comparison_number)) -> menu_item ),
											    "no_champ" )));
	gsb_data_report_text_comparison_set_use_text ( text_comparison_number,
						       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number))));
	gsb_data_report_text_comparison_set_operator ( text_comparison_number,
						       GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_operator (text_comparison_number)) -> menu_item ),
											       "no_operateur" )));

	string = gtk_entry_get_text ( GTK_ENTRY (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)));
	gsb_data_report_text_comparison_set_text ( text_comparison_number,
						   string );

	gsb_data_report_text_comparison_set_first_comparison ( text_comparison_number,
							       GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_first_comparison (text_comparison_number)) -> menu_item ),
												       "no_comparateur" )));
	gsb_data_report_text_comparison_set_link_first_to_second_part ( text_comparison_number,
									GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_link_first_to_second_part (text_comparison_number)) -> menu_item ),
														"no_lien" )));
	gsb_data_report_text_comparison_set_second_comparison ( text_comparison_number,
								GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_second_comparison (text_comparison_number)) -> menu_item ),
													"no_comparateur" )));
	gsb_data_report_text_comparison_set_first_amount ( text_comparison_number,
							   utils_str_atoi ( gtk_entry_get_text ( GTK_ENTRY (gsb_data_report_text_comparison_get_entry_first_amount (text_comparison_number)))));
	gsb_data_report_text_comparison_set_second_amount ( text_comparison_number,
							    utils_str_atoi (gtk_entry_get_text ( GTK_ENTRY (gsb_data_report_text_comparison_get_entry_second_amount (text_comparison_number)))));

	comparison_list = comparison_list -> next;
    }

    /* récupération du montant */

    gsb_data_report_set_amount_comparison_used ( current_report_number,
						 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_montant )));
    gsb_data_report_set_amount_comparison_currency ( current_report_number,
						     gsb_currency_get_currency_from_combobox (bouton_devise_montant_etat));

    /* récupération de la liste des comparaisons de montant */
    /*   il y a au moins une structure de créé, si celle si a les 2 montants vides, */
    /* c'est qu'il n'y a aucune liste */

    amount_comparison_number = GPOINTER_TO_INT (gsb_data_report_get_amount_comparison_list (current_report_number)-> data);

    if ( g_slist_length ( gsb_data_report_get_amount_comparison_list (current_report_number)) == 1
	 &&
	 !strlen ( gtk_entry_get_text ( GTK_ENTRY (gsb_data_report_amount_comparison_get_entry_first_amount (amount_comparison_number))))
	 &&
	 !strlen ( gtk_entry_get_text ( GTK_ENTRY (gsb_data_report_amount_comparison_get_entry_second_amount (amount_comparison_number))))
	 &&
	 GTK_WIDGET_SENSITIVE (gsb_data_report_amount_comparison_get_entry_first_amount (amount_comparison_number))
	 &&
	 GTK_WIDGET_SENSITIVE (gsb_data_report_amount_comparison_get_entry_second_amount (amount_comparison_number)))
    {
	g_slist_free ( gsb_data_report_get_amount_comparison_list (current_report_number));
	gsb_data_report_set_amount_comparison_list ( current_report_number,
						     NULL );
    }
    else
    {
	/* on a rentré au moins une comparaison */
	/* on rempli les champs de la structure */

	GSList *comparison_list;

	comparison_list = gsb_data_report_get_amount_comparison_list (current_report_number);

	while ( comparison_list )
	{
	    amount_comparison_number = GPOINTER_TO_INT (comparison_list -> data);

	    if ( gsb_data_report_amount_comparison_get_button_link (amount_comparison_number))
		gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( amount_comparison_number,
										       GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_link (amount_comparison_number)) -> menu_item ),
															       "no_lien" )));
	    else
		gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( amount_comparison_number,
										       -1 );

	    gsb_data_report_amount_comparison_set_first_comparison ( amount_comparison_number,
								     GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_first_comparison (amount_comparison_number)) -> menu_item ),
													     "no_comparateur" )));

	    gsb_data_report_amount_comparison_set_link_first_to_second_part ( amount_comparison_number,
									      GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_link_first_to_second_part (amount_comparison_number)) -> menu_item ),
														      "no_lien" )));
	    gsb_data_report_amount_comparison_set_second_comparison ( amount_comparison_number,
								      GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_second_comparison (amount_comparison_number)) -> menu_item ),
													      "no_comparateur" )));
	    gsb_data_report_amount_comparison_set_first_amount ( amount_comparison_number,
								 gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY (gsb_data_report_amount_comparison_get_entry_first_amount (amount_comparison_number)))));
	    gsb_data_report_amount_comparison_set_second_amount ( amount_comparison_number,
								  gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY (gsb_data_report_amount_comparison_get_entry_second_amount (amount_comparison_number)))));

	    comparison_list = comparison_list -> next;
	}
    }

    gsb_data_report_set_amount_comparison_only_report_non_null ( current_report_number,
								 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_nulles_etat )));

    /* récupération des modes de paiement */

    gsb_data_report_set_method_of_payment_used ( current_report_number,
						 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_mode_paiement_etat )));

    if ( gsb_data_report_get_method_of_payment_list (current_report_number))
    {
	g_slist_free ( gsb_data_report_get_method_of_payment_list (current_report_number));
	gsb_data_report_set_method_of_payment_list ( current_report_number,
						     NULL );
    }

    list_tmp = GTK_CLIST ( liste_mode_paiement_etat ) -> selection;

    while ( list_tmp )
    {
	gsb_data_report_set_method_of_payment_list ( current_report_number,
						     g_slist_append ( gsb_data_report_get_method_of_payment_list (current_report_number),
								      gtk_clist_get_row_data ( GTK_CLIST ( liste_mode_paiement_etat ),
											       GPOINTER_TO_INT ( list_tmp -> data ))));
	list_tmp = list_tmp -> next;
    }

    /*   si tous les modes de paiement ont été sélectionnés, on met utilise_mode_paiement à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_mode_paiement_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_mode_paiement_etat ) -> rows )
	 &&
	 gsb_data_report_get_method_of_payment_used (current_report_number))
    {
	dialogue_special ( GTK_MESSAGE_INFO,
			   make_hint ( _("Performance issue."),
				       _("All methods of payment have been selected.  Grisbi will run faster without the \"Detail methods of payment used\" option activated.") ) );
	gsb_data_report_set_method_of_payment_used ( current_report_number,
						     0 );
    }

    /* update the payee combofix in the form, to add that report if asked */
    gsb_payee_update_combofix ();

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    /* on réaffiche l'état */

    rafraichissement_etat ( gsb_gui_navigation_get_current_report ());

    /* on repasse à la 1ère page du notebook */
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_etats ),
			    0 );
    gtk_widget_set_sensitive ( navigation_tree_view, TRUE );

    gsb_gui_navigation_update_report ( gsb_gui_navigation_get_current_report ());
}
/******************************************************************************/



/**
 * If applicable, update report navigation tree style to reflect which
 * pages have been changed.
 *
 * \param page_number	Page that contained an interface element just
 *			changed that triggered this event.
 *
 * \return		FALSE
 */
gboolean report_tree_update_style ( gint * page_number )
{

    gtk_tree_model_foreach ( GTK_TREE_MODEL(report_tree_model),
			     (GtkTreeModelForeachFunc) report_tree_update_style_iterator,
			     GINT_TO_POINTER ( page_number ) );

    return FALSE;
}



/**
 * Iterator that iterates over the report tree model and updates
 * iter of report that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Page to be updated.
 *
 * \return TRUE if this iter matches.
 */
gboolean report_tree_update_style_iterator ( GtkTreeModel * tree_model,
					     GtkTreePath *path,
					     GtkTreeIter *iter,
					     gpointer data )
{
    gint page_number = GPOINTER_TO_INT(data);
    gint iter_page_number;
    gint italic = FALSE;

    gtk_tree_model_get ( tree_model, iter, REPORT_TREE_PAGE_COLUMN, &iter_page_number, -1 );

    if ( iter_page_number != page_number )
    {
	return FALSE;
    }

    switch ( iter_page_number )
    {
	/* rien pour l'instant */
	break;

	case 1:
	    /* page des virements */
	    if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements )))
		italic = TRUE;
	    break;

	case 2:
	    /* page des comptes */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat )))
		italic = TRUE;
	    break;

	case 3:
	    /* page des tiers */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat )))
		italic = TRUE;
	    break;

	case 4:
	    /* page des catégories */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_detail_categ )))
		italic = TRUE;
	    break;

	case 5:
	    /* page des ib */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat )))
		italic = TRUE;
	    break;

	case 6:
	    /* page des textes */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_texte )))
		italic = TRUE;
	    break;

	case 7:
	    /* page des montants */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_montant ))
		 ||
		 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_nulles_etat )))
		italic = TRUE;
	    break;

	case 8:
	    /* page des modes de paiement */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_mode_paiement_etat )))
		italic = TRUE;
	    break;

	case 9:
	    /* page des divers */

	    if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat ))
		 ||
		 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation )))
		italic = TRUE;
	    break;
    }

    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ),
			 iter, REPORT_TREE_ITALIC_COLUMN, italic,
			 -1 );

    return TRUE;
}



GtkWidget *onglet_etat_dates ( void )
{
    GtkWidget *scrolled_window, *separateur, *vbox;
    GtkWidget *hbox, *label, *frame, *hbox_onglet;
    GtkWidget *vbox_onglet;
    gchar **plages_dates;
    gint i;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Date selection"), "scheduler.png" );
    gtk_widget_show ( vbox_onglet );

    hbox_onglet = gtk_hbox_new ( FALSE,
				 5 );
    gtk_widget_show ( hbox_onglet );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_onglet,
			 TRUE,
			 TRUE,
			 0 );


    /* mise en place de la plage de dates */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    radio_button_utilise_dates = gtk_radio_button_new_with_label ( NULL,
								   _("Use dates ranges") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 radio_button_utilise_dates,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( radio_button_utilise_dates );

    /* on met en dessous une liste avec les plages de date proposées */

    frame = gtk_frame_new (NULL);
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( frame );

    vbox_utilisation_date = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_utilisation_date ),
				     10 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox_utilisation_date );
    gtk_widget_show ( vbox_utilisation_date );

    /* on met la connection pour rendre sensitif cette frame */

    g_signal_connect ( G_OBJECT ( radio_button_utilise_dates ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 vbox_utilisation_date );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );


    liste_plages_dates_etat = gtk_clist_new ( 1 );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_plages_dates_etat ),
				       0,
				       TRUE );
    g_signal_connect ( G_OBJECT ( liste_plages_dates_etat ),
			 "button_press_event",
			 G_CALLBACK ( click_liste_etat ),
			 NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_plages_dates_etat );
    gtk_widget_show ( liste_plages_dates_etat );

    /* on remplit la liste des dates */

    plages_dates = liste_plages_dates;

    i = 0;

    while ( plages_dates[i] )
    {
	gint row;
	gchar * plage = _(plages_dates[i]);

	row = gtk_clist_append ( GTK_CLIST ( liste_plages_dates_etat ),
				   &plage );
	i++;
    }

    /* séparation gauche-droite */

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    /*   on met en forme la partie de droite : utilisation des exercices */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    radio_button_utilise_exo = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( radio_button_utilise_dates )),
								 _("Use financial years") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 radio_button_utilise_exo,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( radio_button_utilise_exo );

    /* on met la liste des exos sous ce radio button */

    frame = gtk_frame_new ( FALSE );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( frame );


    /* on met la connection pour rendre sensitif cette frame */

    g_signal_connect ( G_OBJECT (radio_button_utilise_exo ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 frame );

    vbox_utilisation_exo = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_utilisation_exo ),
				     10 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox_utilisation_exo );
    gtk_widget_show ( vbox_utilisation_exo );


    /*   on met le détail, exo courant ou précédent */

    bouton_exo_tous = gtk_radio_button_new_with_label ( NULL,
							_("All financial years"));
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_exo_tous,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exo_tous );

    bouton_exo_courant = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
							   _("Current financial year"));
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_exo_courant,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exo_courant );

    bouton_exo_precedent = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
							     _("Former financial year"));
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_exo_precedent,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exo_precedent );



    bouton_detaille_exo_etat = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
								 _("Detail financial years") );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_detaille_exo_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_exo_etat );

    vbox_generale_exo_etat = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 vbox_generale_exo_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_exo_etat );

    g_signal_connect ( G_OBJECT ( bouton_detaille_exo_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 vbox_generale_exo_etat );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_exo_etat ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_exo_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_exo_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_exo_etat ),
				       0,
				       TRUE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_exo_etat );
    gtk_widget_show ( liste_exo_etat );

    /* on remplit la liste des exercices */

    remplissage_liste_exo_etats ();


    /* on met ensuite la date perso de début */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Initial date: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    entree_date_init_etat = gsb_calendar_entry_new (FALSE);
    gtk_widget_set_size_request ( entree_date_init_etat,
			   100,
			   -1 );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_date_init_etat,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( entree_date_init_etat );

    /* on met ensuite la date perso de fin */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Final date: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    entree_date_finale_etat = gsb_calendar_entry_new (FALSE);
    gtk_widget_set_size_request ( entree_date_finale_etat,
			   100,
			   -1 );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_date_finale_etat,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( entree_date_finale_etat );

    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
/* vérifie où l'on clique et empèche la désélection de row */
/******************************************************************************/
void click_liste_etat ( GtkCList *liste,
			GdkEventButton *evenement,
			gint origine )
{
    gint colonne, x, y;
    gint row;

    /*   origine = 0 si ça vient des dates, dans ce cas on sensitive les entrées init et fin */
    /* origine = 1 si ça vient du choix de type de classement */


    g_signal_stop_emission_by_name ( G_OBJECT ( liste ),
				   "button_press_event");

    /* Récupération des coordonnées de la souris */

    gdk_window_get_pointer ( GTK_CLIST ( liste ) -> clist_window,
			     &x,
			     &y,
			     FALSE );

    gtk_clist_get_selection_info ( GTK_CLIST ( liste ),
				   x,
				   y,
				   &row,
				   &colonne);

    if ( GTK_CLIST ( liste ) -> selection
	 &&
	 GPOINTER_TO_INT ( GTK_CLIST ( liste ) -> selection -> data ) == row )
	return;

    gtk_clist_select_row ( GTK_CLIST ( liste ),
			   row,
			   0 );

    if ( !origine )
    {
	if ( row == 1 )
	{
	    gtk_widget_set_sensitive ( entree_date_init_etat,
				       TRUE );
	    gtk_widget_set_sensitive ( entree_date_finale_etat,
				       TRUE );
	}
	else
	{
	    gtk_widget_set_sensitive ( entree_date_init_etat,
				       FALSE );
	    gtk_widget_set_sensitive ( entree_date_finale_etat,
				       FALSE );
	}
    }
}
/******************************************************************************/


/******************************************************************************/
void remplissage_liste_exo_etats ( void )
{
    GSList *list_tmp;

    if ( !liste_exo_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_exo_etat ) );

    list_tmp = gsb_data_fyear_get_fyears_list ();

    while ( list_tmp )
    {
	gint fyear_number;
	gchar *name[1];
	gint row;

	fyear_number = gsb_data_fyear_get_no_fyear (list_tmp -> data);

	name[0] = my_strdup (gsb_data_fyear_get_name (fyear_number));

	/* TODO dOm : when is the memory used by name freed ?*/
	row = gtk_clist_append ( GTK_CLIST ( liste_exo_etat ),
				   name );
	gtk_clist_set_row_data ( GTK_CLIST ( liste_exo_etat ),
				 row,
				 GINT_TO_POINTER (fyear_number));

	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_comptes ( void )
{
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Account selection"), "ac_bank.png" );
    gtk_widget_show ( vbox_onglet );

    /* on met dans la partie de gauche une liste contenant les comptes à */
    /* sélectionner */

    bouton_detaille_comptes_etat = gtk_check_button_new_with_label ( _("Select transactions only for given accounts"));
    g_signal_connect_swapped ( G_OBJECT ( bouton_detaille_comptes_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 2 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_detaille_comptes_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_comptes_etat );

    vbox_generale_comptes_etat = gtk_vbox_new ( FALSE,
						5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_generale_comptes_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_comptes_etat );


    g_signal_connect ( G_OBJECT ( bouton_detaille_comptes_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 vbox_generale_comptes_etat );

    label = gtk_label_new ( _("Select the accounts included in the report: ") );
	gtk_misc_set_padding ( GTK_MISC ( label ), 3, 0 );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );

    gtk_box_pack_start ( GTK_BOX ( vbox_generale_comptes_etat ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_comptes_etat ),
			 hbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_comptes_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_comptes_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_comptes_etat ),
				       0,
				       TRUE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_comptes_etat );
    gtk_widget_show ( liste_comptes_etat );

    /* on remplit la liste des comptes */

    remplissage_liste_comptes_etats ();


    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_select_all ),
				G_OBJECT  ( liste_comptes_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_unselect_all ),
				G_OBJECT  ( liste_comptes_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select bank accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 0 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select cash accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select liabilities accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 2 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select assets accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 3 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_comptes_etats ( void )
{
    GSList *list_tmp;

    if ( !liste_comptes_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_comptes_etat ) );

   list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	gchar *name[1];
	gint row;


	i = gsb_data_account_get_no_account ( list_tmp -> data );

	name[0] = gsb_data_account_get_name (i);

	row = gtk_clist_append ( GTK_CLIST ( liste_comptes_etat ),
				   name );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_etat ),
				 row,
				 GINT_TO_POINTER (i));
	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_partie_liste_compte_etat ( gint *type_compte )
{
    gint i;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_etat ));

    for ( i=0 ; i<gsb_data_account_get_accounts_amount () ; i++ )
    {
	gint no_compte;

	no_compte = GPOINTER_TO_INT ( gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_etat ),
							       i ));

	if ( gsb_data_account_get_kind (no_compte) == GPOINTER_TO_INT ( type_compte ))
	    gtk_clist_select_row ( GTK_CLIST ( liste_comptes_etat ),
				   i,
				   0 );
    }
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_virements ( void )
{
    GtkWidget *scrolled_window;
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *bouton;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Transfers"), "transfer.png" );
    gtk_widget_show ( vbox_onglet );

    /*   on met les boutons d'inclusion ou non des virements */

    bouton_non_inclusion_virements = gtk_radio_button_new_with_label ( NULL,
								       _("Do not include transfers") );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_non_inclusion_virements,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_non_inclusion_virements );

    bouton_inclusion_virements_actifs_etat = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									       _("Include transfers from or to assets or liabilities accounts") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_inclusion_virements_actifs_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_inclusion_virements_actifs_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclusion_virements_actifs_etat );

    bouton_inclusion_virements_hors_etat = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									     _("Include transfers from or to accounts not in this report") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_inclusion_virements_hors_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_inclusion_virements_hors_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclusion_virements_hors_etat );

    bouton_inclusion_virements_perso = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									 _("Include transfers from or to these accounts") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_inclusion_virements_perso ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_inclusion_virements_perso,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclusion_virements_perso );

    hbox_liste_comptes_virements = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_liste_comptes_virements,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_liste_comptes_virements );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( hbox_liste_comptes_virements ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_comptes_virements = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_comptes_virements ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_comptes_virements ),
				       0,
				       TRUE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_comptes_virements );
    gtk_widget_show ( liste_comptes_virements );

    /* on remplit la liste des comptes */
    remplissage_liste_comptes_virements ();

    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */
    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_liste_comptes_virements ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_select_all ),
				G_OBJECT  ( liste_comptes_virements ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_unselect_all ),
				G_OBJECT  ( liste_comptes_virements ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select bank accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 0 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select cash accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select liabilities accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 2 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select assets accounts") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 3 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    g_signal_connect ( G_OBJECT ( bouton_inclusion_virements_perso ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 hbox_liste_comptes_virements );


    /* on rajoute le bouton exclure les opé non virement */

    bouton_exclure_non_virements_etat = gtk_check_button_new_with_label ( _("Exclude the transactions which are not transfers") );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_exclure_non_virements_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exclure_non_virements_etat );

    g_signal_connect ( G_OBJECT ( bouton_inclusion_virements_perso ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_exclure_non_virements_etat );
    g_signal_connect ( G_OBJECT ( bouton_inclusion_virements_hors_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_exclure_non_virements_etat );
    g_signal_connect ( G_OBJECT ( bouton_inclusion_virements_actifs_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_exclure_non_virements_etat );


    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_comptes_virements ( void )
{
    GSList *list_tmp;

    if ( !liste_comptes_virements )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_comptes_virements ) );

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	gchar *name[1];
	gint row;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	name[0] = gsb_data_account_get_name (i);

	row = gtk_clist_append ( GTK_CLIST ( liste_comptes_virements ),
				   name );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_virements ),
				 row,
				 GINT_TO_POINTER (i));
	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_partie_liste_compte_vir_etat ( gint *type_compte )
{
    gint i;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_virements ));

    for ( i=0 ; i<gsb_data_account_get_accounts_amount () ; i++ )
    {
	gint no_compte;

	no_compte = GPOINTER_TO_INT ( gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_virements ),
							       i ));

	if ( gsb_data_account_get_kind (no_compte) == GPOINTER_TO_INT ( type_compte ))
	    gtk_clist_select_row ( GTK_CLIST ( liste_comptes_virements ),
				   i,
				   0 );
    }
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_categories ( void )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Categories"), "categories.png" );
    gtk_widget_show ( vbox_onglet );
    /* on propose de détailler les categ utilisées */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    button_detail_categ = gtk_check_button_new_with_label ( _("Detail categories") );
    g_signal_connect_swapped ( G_OBJECT ( button_detail_categ ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 4 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 button_detail_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( button_detail_categ );


    /* mise en place de la liste et des boutons de détaillage */
    hbox_detaille_categ_etat = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_detaille_categ_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_detaille_categ_etat );

    g_signal_connect ( G_OBJECT ( button_detail_categ ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 hbox_detaille_categ_etat );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( _("Select categories to include: ") );
	gtk_misc_set_padding ( GTK_MISC ( label ), 3, 0 );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );

    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    /* add the categories list */
    tree_view_categ = report_config_create_categ_budget_list ();
    model_categ = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view_categ));
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			tree_view_categ );
    gtk_widget_show ( tree_view_categ );

    /* fill the list */
    report_config_fill_categ_budget_list (TRUE);

    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */
    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_category_select_all),
		       GINT_TO_POINTER (TRUE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_category_select_all),
		       GINT_TO_POINTER (FALSE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Income categories") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_category_select_type),
		       GINT_TO_POINTER (FALSE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Outgoing categories") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_category_select_type),
		       GINT_TO_POINTER (TRUE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    return ( vbox_onglet );
}



/**
 * create the category/budget list for reports
 * it's a tree with categories/budget and sub-categories/budgets,
 * and a check-button to select them
 *
 * \param
 *
 * \return a GtkWidget : the GtkTreeView
 * */
static GtkWidget *report_config_create_categ_budget_list ( void )
{
    GtkWidget *tree_view;
    GtkTreeStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkCellRenderer *radio_renderer;

    store = gtk_tree_store_new ( REPORT_CATEG_BUDGET_LIST_NB,
				 G_TYPE_STRING,
				 G_TYPE_BOOLEAN,
				 G_TYPE_BOOLEAN,
				 G_TYPE_INT,
				 G_TYPE_INT );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE (store),
					   REPORT_CATEG_BUDGET_LIST_NAME, GTK_SORT_ASCENDING );

    column = gtk_tree_view_column_new ();

    /* create the toggle button part */
    radio_renderer = gtk_cell_renderer_toggle_new ();
    g_object_set ( G_OBJECT (radio_renderer),
		   "xalign", 0.0,
		   NULL );
    gtk_tree_view_column_pack_start ( column,
				      radio_renderer,
				      FALSE );
    gtk_tree_view_column_set_attributes (column,
					 radio_renderer,
					 "active", REPORT_CATEG_BUDGET_LIST_ACTIVE,
					 "activatable", REPORT_CATEG_BUDGET_LIST_ACTIVATABLE,
					 NULL);
    g_signal_connect ( G_OBJECT (radio_renderer),
		       "toggled",
		       G_CALLBACK (report_config_categ_budget_toggled),
		       store );

    /* create the text part */
    cell_renderer = gtk_cell_renderer_text_new ();
    g_object_set ( G_OBJECT (cell_renderer),
		   "xalign", 0.0,
		   NULL );
    gtk_tree_view_column_pack_start ( column,
				      cell_renderer,
				      TRUE );
    gtk_tree_view_column_set_attributes (column,
					 cell_renderer,
					 "text", REPORT_CATEG_BUDGET_LIST_NAME,
					 NULL );

    /* create the tree view */
    tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    gtk_tree_view_append_column ( GTK_TREE_VIEW (tree_view),
				  column );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW (tree_view),
					FALSE );
    return tree_view;
}


/**
 * fill the categories selection list for report
 *
 * \param is_categ	TRUE for category, FALSE for budget
 *
 * \return FALSE
 * */
static gboolean report_config_fill_categ_budget_list ( gboolean is_categ )
{
    GSList *list_tmp;
    gchar *name;
    gchar *without_name, *without_sub_name;

    if (is_categ)
    {
	list_tmp = gsb_data_category_get_categories_list ();
	gtk_tree_store_clear (GTK_TREE_STORE (model_categ));
	without_name = _("Empty category");
	without_sub_name = _("Empty sub-category");
    }
    else
    {
	list_tmp = gsb_data_budget_get_budgets_list ();
	gtk_tree_store_clear (GTK_TREE_STORE (model_budget));
	without_name = _("Empty budget");
	without_sub_name = _("Empty sub-budget");
    }

    while ( list_tmp )
    {
	gint div_number;
	GSList *tmp_list_sub_div;

	/* get the category to append */
	if (is_categ)
	{
	    div_number = gsb_data_category_get_no_category ( list_tmp -> data );
	    name = gsb_data_category_get_name (div_number, 0, NULL );
	    tmp_list_sub_div = gsb_data_category_get_sub_category_list (div_number);
	}
	else
	{
	    div_number = gsb_data_budget_get_no_budget ( list_tmp -> data );
	    name = gsb_data_budget_get_name (div_number, 0, NULL );
	    tmp_list_sub_div = gsb_data_budget_get_sub_budget_list (div_number);
	}

	/* append to the model */
	report_config_mixed_list_add_div (is_categ, name, div_number);
	g_free (name);

	/* append the sub categories */
	while (tmp_list_sub_div)
	{
	    gint sub_div_number;

	    if (is_categ)
	    {
		sub_div_number = gsb_data_category_get_no_sub_category (tmp_list_sub_div -> data);
		name = gsb_data_category_get_sub_category_name (div_number, sub_div_number, NULL);
	    }
	    else
	    {
		sub_div_number = gsb_data_budget_get_no_sub_budget (tmp_list_sub_div -> data);
		name = gsb_data_budget_get_sub_budget_name (div_number, sub_div_number, NULL);
	    }

	    /* append to the model */
	    report_config_mixed_list_add_sub_div (is_categ, name, sub_div_number);
	    g_free (name);
	    tmp_list_sub_div = tmp_list_sub_div -> next;
	}

	/* append without sub-div */
	report_config_mixed_list_add_sub_div (is_categ, without_sub_name, 0);
	list_tmp = list_tmp -> next;
    }
    /* append without div and sub-div*/
    report_config_mixed_list_add_div (is_categ, without_name, 0);
    report_config_mixed_list_add_sub_div (is_categ, without_sub_name, 0);

    return FALSE;
}


/**
 * append the div given in param to the categ/budget list
 * this place parent_iter on the place of the div appended
 *
 * \param is_categ 	TRUE for categ, FALSE for budget
 * \param name
 * \param number
 *
 * \return
 * */
static void report_config_mixed_list_add_div ( gboolean is_categ,
					       const gchar *name,
					       gint number )
{
    GtkTreeModel *model;

    if (is_categ)
	model = model_categ;
    else
	model = model_budget;
    gtk_tree_store_append ( GTK_TREE_STORE (model),
			    &parent_iter,
			    NULL );
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&parent_iter,
			REPORT_CATEG_BUDGET_LIST_NAME, name,
			REPORT_CATEG_BUDGET_LIST_NUMBER, number,
			REPORT_CATEG_BUDGET_LIST_SUB_NUMBER, -1,
			REPORT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
			-1 );
}

/**
 * append the sub-div given in param to the categ/budget list
 *
 * \param is_categ 	TRUE for categ, FALSE for budget
 * \param name
 * \param number
 *
 * \return
 * */
static void report_config_mixed_list_add_sub_div ( gboolean is_categ,
						   const gchar *name,
						   gint number )
{
    GtkTreeModel *model;
    GtkTreeIter child_iter;

    if (is_categ)
	model = model_categ;
    else
	model = model_budget;
    gtk_tree_store_append ( GTK_TREE_STORE (model),
			    &child_iter,
			    &parent_iter );
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&child_iter,
			REPORT_CATEG_BUDGET_LIST_NAME, name,
			REPORT_CATEG_BUDGET_LIST_NUMBER, -1,
			REPORT_CATEG_BUDGET_LIST_SUB_NUMBER, number,
			REPORT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
			-1 );
}


/**
 * callback if we toggle a checkbox in the category/budget list
 * if we toggle a div, toggle all the sub-div
 * if we toggle a sub-div, toggle also the div
 *
 * \param radio_renderer
 * \param path			the string of path
 * \param store			the GtkTreeStore of categ/budget
 *
 * \return FALSE
 * */
static gboolean report_config_categ_budget_toggled ( GtkCellRendererToggle *radio_renderer,
						     gchar *path_str,
						     GtkTreeStore *store )
{
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeIter iter_children;
    gboolean toggle_value;

    g_return_val_if_fail (path_str != NULL, FALSE);
    g_return_val_if_fail (store != NULL && GTK_IS_TREE_STORE (store), FALSE);

    /* first get the iter and the value of the checkbutton */
    path = gtk_tree_path_new_from_string (path_str);
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (store),
			      &iter, path );
    gtk_tree_model_get ( GTK_TREE_MODEL (store),
			 &iter,
			 REPORT_CATEG_BUDGET_LIST_ACTIVE, &toggle_value,
			 -1 );

    /* ok, we invert the button */
    toggle_value = !toggle_value;

    gtk_tree_store_set ( GTK_TREE_STORE (store),
			 &iter,
			 REPORT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
			 -1 );

    /* if we are on a mother, we set the same value to all the children */
    if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (store), &iter_children, &iter))
    {
	/* we are on the children */
	do
	    gtk_tree_store_set ( GTK_TREE_STORE (store),
				 &iter_children,
				 REPORT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
				 -1 );
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter_children));
    }

    /* if we are activating a child, activate the mother */
    if (toggle_value &&
	gtk_tree_model_iter_parent (GTK_TREE_MODEL (store), &iter_children, &iter))
	gtk_tree_store_set ( GTK_TREE_STORE (store),
			     &iter_children,
			     REPORT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
			     -1 );

    gtk_tree_path_free (path);
    return FALSE;
}


/**
 * select or unselect all the categories
 *
 * \param button
 * \param select_ptr	TRUE or FALSE to select/unselect all
 *
 * \return FALSE
 * */
static gboolean report_config_category_select_all ( GtkWidget *button,
						    gboolean *select_ptr )
{
    return (report_config_mix_select_all (model_categ, GPOINTER_TO_INT (select_ptr)));
}



/**
 * select or unselect all the budgets
 *
 * \param button
 * \param select_ptr	TRUE or FALSE to select/unselect all
 *
 * \return FALSE
 * */
static gboolean report_config_budget_select_all ( GtkWidget *button,
						  gboolean *select_ptr )
{
    return (report_config_mix_select_all (model_budget, GPOINTER_TO_INT (select_ptr)));
}



/**
 * select or unselect all the budgets or categories
 *
 * \param model		the model to fill (is model_categ or model_budget
 * \param select_ptr	TRUE or FALSE to select/unselect all
 *
 * \return FALSE
 * */
static gboolean report_config_mix_select_all ( GtkTreeModel *model,
					       gboolean toggle_value )
{
    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
	return FALSE;

    do
    {
	GtkTreeIter iter_children;

	gtk_tree_store_set ( GTK_TREE_STORE (model),
			     &parent_iter,
			     REPORT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
			     -1 );

	if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (model), &iter_children, &parent_iter))
	{
	    /* we are on the children */
	    do
		gtk_tree_store_set ( GTK_TREE_STORE (model),
				     &iter_children,
				     REPORT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
				     -1 );
	    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_children));
	}
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));
    return FALSE;
}


/**
 * select the income or outgoing categories
 *
 * \param button
 * \param type_ptr	FALSE to income categories, TRUE for outgoing categories
 *
 * \return FALSE
 * */
static gboolean report_config_category_select_type ( GtkWidget *button,
						     gboolean *type_ptr )

{
    return (report_config_mix_select_type (TRUE, GPOINTER_TO_INT (type_ptr)));
}

/**
 * select the income or outgoing budget
 *
 * \param button
 * \param type_ptr	FALSE to income budgets, TRUE for outgoing budgets
 *
 * \return FALSE
 * */
static gboolean report_config_budget_select_type ( GtkWidget *button,
						   gboolean *type_ptr )
{
    return (report_config_mix_select_type (FALSE, GPOINTER_TO_INT (type_ptr)));
}

/**
 * select the income or outgoing categories or budget
 *
 * \param is_categ	TRUE for category, FALSE for budget
 * \param type		FALSE to income item, TRUE for outgoing item
 *
 * \return FALSE
 * */
static gboolean report_config_mix_select_type ( gboolean is_categ,
						gboolean type )

{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (is_categ)
	model = model_categ;
    else
	model = model_budget;

    report_config_mix_select_all (model, FALSE);

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	return FALSE;

    do
    {
	gint div_number;

	gtk_tree_model_get (GTK_TREE_MODEL (model),
			    &iter,
			    REPORT_CATEG_BUDGET_LIST_NUMBER, &div_number,
			    -1 );

	/* we always select without categories/budget ?? set yes by default */
	if ( !div_number
	     ||
	     (is_categ && (gsb_data_category_get_type (div_number) == type))
	     ||
	     (!is_categ && (gsb_data_budget_get_type (div_number) == type)))
	{
	    GtkTreeIter iter_children;

	    gtk_tree_store_set ( GTK_TREE_STORE (model),
				 &iter,
				 REPORT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
				 -1 );

	    if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (model), &iter_children, &iter))
	    {
		/* we are on the children */
		do
		    gtk_tree_store_set ( GTK_TREE_STORE (model),
					 &iter_children,
					 REPORT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
					 -1 );
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_children));
	    }
	}
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    return FALSE;
}


/**
 * get all the selected categ and sub-categ and place them in a list of struct_categ_budget_sel
 * to be saved in the report
 *
 * \param is_categ	TRUE for categ, FALSE for budget
 *
 * \return a GSList of struct_categ_budget_sel or NULL if all the categories/sub-categ were selected
 * 		to avoid to filter by categ, to improve speed
 * */
static GSList *report_config_categ_budget_get_selected ( gboolean is_categ )
{
    GtkTreeModel *model;
    gboolean all_selected = TRUE;
    GSList *tmp_list = NULL;

    if (is_categ)
	model = model_categ;
    else
	model = model_budget;

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
	return NULL;

    do
    {
	gint div_number;
	gboolean active;
	struct_categ_budget_sel *categ_budget_struct;
	GtkTreeIter iter_children;

	gtk_tree_model_get (GTK_TREE_MODEL (model),
			    &parent_iter,
			    REPORT_CATEG_BUDGET_LIST_NUMBER, &div_number,
			    REPORT_CATEG_BUDGET_LIST_ACTIVE, &active,
			    -1 );
	if (!active)
	{
	    all_selected = FALSE;
	    continue;
	}

	/* ok, we are on a selected category/budget, create and fill the structure */
	categ_budget_struct = g_malloc0 (sizeof (struct_categ_budget_sel));

	categ_budget_struct -> div_number = div_number;
	tmp_list = g_slist_append (tmp_list, categ_budget_struct);

	/* check the children */
	if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (model), &iter_children, &parent_iter))
	{
	    /* we are on the children */
	    do
	    {
		gint sub_div_number;

		gtk_tree_model_get (GTK_TREE_MODEL (model),
				    &iter_children,
				    REPORT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_div_number,
				    REPORT_CATEG_BUDGET_LIST_ACTIVE, &active,
				    -1 );
		if (active)
		    categ_budget_struct -> sub_div_numbers = g_slist_append ( categ_budget_struct -> sub_div_numbers,
									      GINT_TO_POINTER (sub_div_number));
		else
		    all_selected = FALSE;
	    }
	    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_children));
	}
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));

    /* if all is selected, erase the new list and set an info message */
    if (all_selected)
    {
	if (is_categ)
	    dialogue_special ( GTK_MESSAGE_INFO,
			       make_hint ( _("Performance issue."),
					   _("All categories have been selected.  Grisbi will run faster without the \"Detail categories used\" option activated.")));
	else
	    dialogue_special ( GTK_MESSAGE_INFO,
			       make_hint ( _("Performance issue."),
					   _("All budgets have been selected.  Grisbi will run faster without the \"Detail budgets used\" option activated.")));

	report_config_mix_select_all (model, FALSE);
	gsb_data_report_free_categ_budget_struct (tmp_list);
	tmp_list = NULL;
    }
    return tmp_list;
}


/**
 * toggle the categories and sub-categories selected
 *
 * \param is_categ	TRUE for categ, FALSE for budgets
 *
 * \return
 * */
static void report_config_category_update_treeview ( gboolean is_categ )
{
    GSList *tmp_list;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if (is_categ)
    {
	report_config_mix_select_all (model_categ, FALSE);
	tmp_list = gsb_data_report_get_category_struct (current_report_number);
    }
    else
    {
	report_config_mix_select_all (model_budget, FALSE);
	tmp_list = gsb_data_report_get_budget_struct (current_report_number);
    }

    while ( tmp_list )
    {
	struct_categ_budget_sel *categ_budget_struct = tmp_list -> data;
	report_config_category_toggle_categ (categ_budget_struct, is_categ);
	tmp_list = tmp_list -> next;
    }
}

/**
 * get a struct_categ_budget_sel and toggle on the tree view the corresponding category
 * and the sub-categories
 *
 * \param categ_budget_struct	a struct_categ_budget_sel
 * \param is_categ		TRUE for categ, FALSE for budget
 *
 * \return
 * */
static void report_config_category_toggle_categ ( struct_categ_budget_sel *categ_budget_struct,
						  gboolean is_categ )
{
    GtkTreeModel *model;

    if (is_categ)
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view_categ));
    else
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view_budget));

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
	return;

    do
    {
	gint div_number;

	gtk_tree_model_get ( GTK_TREE_MODEL (model),
			     &parent_iter,
			     REPORT_CATEG_BUDGET_LIST_NUMBER, &div_number,
			     -1 );

	/* we check the children only if the category is selected */
	if (div_number == categ_budget_struct -> div_number)
	{
	    gtk_tree_store_set ( GTK_TREE_STORE (model),
				 &parent_iter,
				 REPORT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
				 -1 );
	    if (categ_budget_struct -> sub_div_numbers)
	    {
		GtkTreeIter iter_child;
		if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter_child, &parent_iter))
		    do
		    {
			gint sub_div_number;

			gtk_tree_model_get ( GTK_TREE_MODEL (model),
					     &iter_child,
					     REPORT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_div_number,
					     -1 );
			if (g_slist_find (categ_budget_struct -> sub_div_numbers, GINT_TO_POINTER (sub_div_number)))
			    gtk_tree_store_set ( GTK_TREE_STORE (model),
						 &iter_child,
						 REPORT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
						 -1 );
		    }
		    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_child));
	    }
	    /* we have found the category, can stop here */
	    break;
	}
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));
}


GtkWidget *onglet_etat_ib ( void )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Budgetary lines"), "budgetary_lines.png" );
    gtk_widget_show ( vbox_onglet );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    bouton_detaille_ib_etat = gtk_check_button_new_with_label ( _("Detail budgetary lines") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_detaille_ib_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 5 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_detaille_ib_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_ib_etat );


    /* mise en place de la liste et des boutons de détaillage */

    hbox_detaille_ib_etat = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_detaille_ib_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_detaille_ib_etat );

    g_signal_connect ( G_OBJECT ( bouton_detaille_ib_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 hbox_detaille_ib_etat );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( _("Select the budgetary lines to include in the report: ") );
	gtk_misc_set_padding ( GTK_MISC ( label ), 3, 0 );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );

    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    /* add the budget list */
    tree_view_budget = report_config_create_categ_budget_list ();
    model_budget = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view_budget));
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			tree_view_budget );
    gtk_widget_show ( tree_view_budget );

    /* fill the list */
    report_config_fill_categ_budget_list (FALSE);

    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */
    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_budget_select_all),
		       GINT_TO_POINTER (TRUE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_budget_select_all),
		       GINT_TO_POINTER (FALSE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Income budget lines") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_budget_select_type),
		       GINT_TO_POINTER (FALSE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Outgoing budget lines") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (bouton),
		       "clicked",
		       G_CALLBACK (report_config_budget_select_type),
		       GINT_TO_POINTER (TRUE));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    return ( vbox_onglet );
}



/**
 * create the budget widget to select them in a list
 *
 * \param
 *
 * \return the GtkWidget
 * */
GtkWidget *onglet_etat_tiers ( void )
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *vbox_onglet;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Payees"), "payees.png" );
    gtk_widget_show ( vbox_onglet );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_detaille_tiers_etat = gtk_check_button_new_with_label ( _("Detail payees") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_detaille_tiers_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 3 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_detaille_tiers_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_tiers_etat );


    /* mise en place de la liste et des boutons de détaillage */

    hbox_detaille_tiers_etat = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_detaille_tiers_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_detaille_tiers_etat );

    g_signal_connect ( G_OBJECT ( bouton_detaille_tiers_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 hbox_detaille_tiers_etat );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( _("Select payees to include in this report: ") );
	gtk_misc_set_padding ( GTK_MISC ( label ), 3, 0 );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );

    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_tiers_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_tiers_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_tiers_etat ),
				       0,
				       TRUE );
    gtk_clist_set_compare_func ( GTK_CLIST ( liste_tiers_etat ),
				 (GtkCListCompareFunc) classement_alphabetique_tree );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_tiers_etat );
    gtk_widget_show ( liste_tiers_etat );

    /* on va remplir la liste avec les tiers */

    remplissage_liste_tiers_etats ();

    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_select_all ),
				G_OBJECT  ( liste_tiers_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_unselect_all ),
				G_OBJECT  ( liste_tiers_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( vbox_onglet );
}



/******************************************************************************/
void remplissage_liste_tiers_etats ( void )
{
    GSList *payee_list;

    if ( !liste_tiers_etat )
	return;

    devel_debug (NULL);

    gtk_clist_clear ( GTK_CLIST ( liste_tiers_etat ) );

    payee_list = gsb_data_payee_get_payees_list();

    while ( payee_list )
    {
	gint payee_number;
	gchar *name[1];
	gint row;

	payee_number = gsb_data_payee_get_no_payee ( payee_list -> data );

	name[0] = my_strdup (gsb_data_payee_get_name (payee_number,
						     TRUE ));

	row = gtk_clist_append ( GTK_CLIST ( liste_tiers_etat ),
				 name );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_tiers_etat ),
				 row,
				 GINT_TO_POINTER (payee_number));

	payee_list = payee_list -> next;
    }

    gtk_clist_sort ( GTK_CLIST ( liste_tiers_etat ));
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_texte ( void )
{
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;
    GtkWidget *scrolled_window;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Transaction content"), "text.png" );
    gtk_widget_show ( vbox_onglet );


    /* on commence par créer le choix d'utiliser le montant */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_utilise_texte = gtk_check_button_new_with_label ( _("Select transactions according to content") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_utilise_texte ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 6 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_utilise_texte,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_utilise_texte );

    vbox_generale_textes_etat = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_generale_textes_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_textes_etat );

    g_signal_connect ( G_OBJECT ( bouton_utilise_texte ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 vbox_generale_textes_etat );


    /* on va ensuite créer la liste qui contiendra les critères */
    /* le remplissage ou la row vide se mettent plus tard */

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_textes_etat ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_textes_etat = gtk_vbox_new ( FALSE, 5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    liste_textes_etat );
    gtk_widget_show ( liste_textes_etat );

    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
void remplit_liste_comparaisons_textes_etat ( void )
{
    GSList *list_tmp;
    gint current_report_number;
	gchar* tmpstr;

    current_report_number = gsb_gui_navigation_get_current_report ();


    list_tmp = gsb_data_report_get_text_comparison_list (current_report_number);


    /* commence par effacer l'ancienne liste */

    while ( GTK_BOX ( liste_textes_etat ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( liste_textes_etat ),
			       (( GtkBoxChild *) ( GTK_BOX ( liste_textes_etat ) -> children -> data )) -> widget );


    /*   s'il n'y a rien dans la liste, on met juste une row vide */

    if ( !list_tmp )
    {
	ajoute_ligne_liste_comparaisons_textes_etat (0);
	return;
    }

    /*   on fait le tour de la liste des comparaisons de texte, ajoute une row */
    /* et la remplit à chaque fois */

    while ( list_tmp )
    {
	gint text_comparison_number;
	GtkWidget *widget;

	text_comparison_number = GPOINTER_TO_INT (list_tmp -> data);

	/* on crée la row et remplit les widget de la structure */

	widget = cree_ligne_comparaison_texte (text_comparison_number);
	gsb_data_report_text_comparison_set_vbox_line ( text_comparison_number,
							widget );
	gtk_box_pack_start ( GTK_BOX ( liste_textes_etat ),
			     widget,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( widget );

	/* on remplit maintenant les widget avec les valeurs de la stucture */

	/*       s'il n'y a pas de lien avec la struct précédente, on le vire */
	/* on rajoute le && car parfois le bouton de lien se met quand même en 1ère row */

	if ( gsb_data_report_text_comparison_get_link_to_last_text_comparison (text_comparison_number) != -1
	     &&
	     list_tmp != gsb_data_report_get_text_comparison_list (current_report_number))
	    gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_link (text_comparison_number)),
					  gsb_data_report_text_comparison_get_link_to_last_text_comparison (text_comparison_number));
	else
	{
	    gtk_widget_destroy (gsb_data_report_text_comparison_get_button_link (text_comparison_number));
	    gsb_data_report_text_comparison_set_button_link ( text_comparison_number,
							      NULL);
	}


	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_field (text_comparison_number)),
				      gsb_data_report_text_comparison_get_field (text_comparison_number));
	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_operator (text_comparison_number)),
				      gsb_data_report_text_comparison_get_operator (text_comparison_number));

	if (gsb_data_report_text_comparison_get_text (text_comparison_number))
	    gtk_entry_set_text ( GTK_ENTRY (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)),
				 gsb_data_report_text_comparison_get_text (text_comparison_number));

	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_first_comparison (text_comparison_number)),
				      gsb_data_report_text_comparison_get_first_comparison (text_comparison_number));
	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_link_first_to_second_part (text_comparison_number)),
				      gsb_data_report_text_comparison_get_link_first_to_second_part (text_comparison_number));
	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_second_comparison (text_comparison_number)),
				      gsb_data_report_text_comparison_get_second_comparison (text_comparison_number));

	tmpstr = utils_str_itoa (gsb_data_report_text_comparison_get_first_amount (text_comparison_number));
	gtk_entry_set_text ( GTK_ENTRY (gsb_data_report_text_comparison_get_entry_first_amount (text_comparison_number)), tmpstr);
	g_free ( tmpstr );

	tmpstr = utils_str_itoa (gsb_data_report_text_comparison_get_second_amount (text_comparison_number));
	gtk_entry_set_text ( GTK_ENTRY (gsb_data_report_text_comparison_get_entry_second_amount (text_comparison_number)), tmpstr);
	g_free ( tmpstr );

	if (gsb_data_report_text_comparison_get_use_text (text_comparison_number))
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)),
					   TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)),
					   TRUE );


	/* on désensitive tous ce qui est nécessaire */

	if ( gsb_data_report_text_comparison_get_field (text_comparison_number) == 8
	     ||
	     gsb_data_report_text_comparison_get_field (text_comparison_number) == 9
	     ||
	     gsb_data_report_text_comparison_get_field (text_comparison_number) == 10 )
	{
	    /* 	  on est sur un chq ou une pc */
	    /* on rend sensitif les check button et la hbox correspondante */

	    sensitive_widget (NULL, gsb_data_report_text_comparison_get_button_use_text (text_comparison_number));
	    sensitive_widget (NULL, gsb_data_report_text_comparison_get_button_use_number (text_comparison_number));
	    sensitive_hbox_fonction_bouton_txt (text_comparison_number);
	}
	else
	{
	    desensitive_widget (NULL, gsb_data_report_text_comparison_get_button_use_text (text_comparison_number));
	    desensitive_widget (NULL, gsb_data_report_text_comparison_get_button_use_number (text_comparison_number));
	    desensitive_widget (NULL, gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number));
	    sensitive_widget (NULL, gsb_data_report_text_comparison_get_hbox_text (text_comparison_number));
	}
	/* on sensitive/désensitive l'entrée txt*/

	if ( gsb_data_report_text_comparison_get_operator (text_comparison_number) >= 4 )
	    desensitive_widget (NULL, gsb_data_report_text_comparison_get_entry_text (text_comparison_number));

	/* on sensitive/désensitive les entrées de montant si nécessaire */

	if ( gsb_data_report_text_comparison_get_first_comparison (text_comparison_number) == 6 )
	    desensitive_widget (NULL, gsb_data_report_text_comparison_get_entry_first_amount (text_comparison_number));

	if ( gsb_data_report_text_comparison_get_second_comparison (text_comparison_number) == 6 )
	    desensitive_widget (NULL, gsb_data_report_text_comparison_get_entry_second_amount (text_comparison_number));

	/* on sensitive/désensitive la hbox_2 si nécessaire */

	if ( gsb_data_report_text_comparison_get_link_first_to_second_part (text_comparison_number) == 3 )
	    desensitive_widget (NULL, gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number));

	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction ajoute une row vierge */
/* si ancien_comp_textes n'est pas nul, la row est insérée juste après celle de l'argument */
/******************************************************************************/

void ajoute_ligne_liste_comparaisons_textes_etat ( gint last_text_comparison_number )
{
    gint text_comparison_number;
    gint position;
    gint current_report_number;
    GtkWidget *widget;

    current_report_number = gsb_gui_navigation_get_current_report ();

    /* on récupère tout de suite la position à laquelle il faut insérer la row */

    if ( last_text_comparison_number )
	position = g_slist_index ( gsb_data_report_get_text_comparison_list (current_report_number),
				   GINT_TO_POINTER (last_text_comparison_number)) + 1;
    else
	position = 0;

    /* on commence par créer une structure vide */

    text_comparison_number = gsb_data_report_text_comparison_new (0);
    gsb_data_report_text_comparison_set_report_number ( text_comparison_number,
							current_report_number );

    /* on crée la row et remplit les widget de la structure */

    widget = cree_ligne_comparaison_texte (text_comparison_number);
    gsb_data_report_text_comparison_set_vbox_line ( text_comparison_number,
						    widget );
    gtk_box_pack_start ( GTK_BOX ( liste_textes_etat ),
			 widget,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( widget );

    /* on vire le lien de la row s'il n'y a pas encore de liste */
    /*   (cad si c'est la 1ère row) */

    if ( !gsb_data_report_get_text_comparison_list (current_report_number))
    {
	gtk_widget_destroy (gsb_data_report_text_comparison_get_button_link (text_comparison_number));
	gsb_data_report_text_comparison_set_button_link ( text_comparison_number,
							  NULL );
    }

    /*   par défaut, le bouton bouton_lien_1_2 est sur stop */

    gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_text_comparison_get_button_link_first_to_second_part (text_comparison_number)),
				  3 );
    gsb_data_report_text_comparison_set_link_first_to_second_part ( text_comparison_number,
								    3 );
    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number),
			       FALSE );


    /*   par défaut, la row de chq est non sensitive */

    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_text (text_comparison_number),
			       FALSE );
    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_number (text_comparison_number),
			       FALSE );
    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number),
			       FALSE );


    /* on met la structure dans la liste à la position demandée */

    gsb_data_report_set_text_comparison_list ( current_report_number,
					       g_slist_insert ( gsb_data_report_get_text_comparison_list (current_report_number),
								GINT_TO_POINTER (text_comparison_number),
								position ));


    /* on met la row à sa place dans la liste */

    gtk_box_reorder_child ( GTK_BOX ( liste_textes_etat ),
			    gsb_data_report_text_comparison_get_vbox_line (text_comparison_number),
			    position );
}
/******************************************************************************/

/******************************************************************************/
/* crée la hbox de la row et la renvoie */
/* remplie en même temps les widget de la struct envoyée en argument, sauf hbox_ligne */
/******************************************************************************/
GtkWidget *cree_ligne_comparaison_texte ( gint text_comparison_number )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *bouton;
    GtkWidget *hbox;

    /*   on laisse les infos vides, on met juste les boutons et les labels */

    vbox = gtk_vbox_new ( FALSE,
			  5 );


    /* création de la 1ère row */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    gsb_data_report_text_comparison_set_button_link ( text_comparison_number,
						      cree_bouton_lien_lignes_comparaison ());
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 gsb_data_report_text_comparison_get_button_link (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( _("Transactions whose ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    /*   avant de créer le bouton des champs, on doit créer hbox_txt, hbox_chq et les 2 check button */

    gsb_data_report_text_comparison_set_hbox_text ( text_comparison_number,
						    gtk_hbox_new ( FALSE,
								   5 ));
    gsb_data_report_text_comparison_set_hbox_cheque ( text_comparison_number,
						      gtk_hbox_new ( FALSE,
								     5 ));
    gsb_data_report_text_comparison_set_button_use_text ( text_comparison_number,
							  gtk_radio_button_new ( NULL ));
    gsb_data_report_text_comparison_set_button_use_number ( text_comparison_number,
							    gtk_radio_button_new ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON(gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)))));

    gsb_data_report_text_comparison_set_button_field ( text_comparison_number,
						       cree_bouton_champ ( text_comparison_number ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 gsb_data_report_text_comparison_get_button_field (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );

    /* la suite se met dans hbox_txt */
    /* en 2ème row */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( NULL );
    gtk_widget_set_size_request ( label,
			   12,
			   -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    /* on met le check button utilisé en cas de champ à no */

    g_signal_connect ( G_OBJECT ( gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 gsb_data_report_text_comparison_get_hbox_text (text_comparison_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 gsb_data_report_text_comparison_get_button_use_text (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( gsb_data_report_text_comparison_get_button_use_text (text_comparison_number));


    /* on met maintenant le comparateur txt */

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 gsb_data_report_text_comparison_get_hbox_text (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number));

    /* avant de créer l'opérateur, on doit créer l'entrée de txt */

    gsb_data_report_text_comparison_set_entry_text ( text_comparison_number,
						     gtk_entry_new ());

    gsb_data_report_text_comparison_set_button_operator ( text_comparison_number,
							  cree_bouton_operateur_txt (text_comparison_number));
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)),
			 gsb_data_report_text_comparison_get_button_operator (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );

    /* on peut maintenant mettre l'entrée de txt */

    gtk_widget_set_size_request ( gsb_data_report_text_comparison_get_entry_text (text_comparison_number),
			   150,
			   -1 );
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)),
			 gsb_data_report_text_comparison_get_entry_text (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (gsb_data_report_text_comparison_get_entry_text (text_comparison_number));


    /* on crée maintenant la 2ème row qui concerne les tests de chq */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( NULL );
    gtk_widget_set_size_request ( label,
			   12,
			   -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    /* on met le check button utilisé en cas de champ à no */

    g_signal_connect ( G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 gsb_data_report_text_comparison_get_button_use_number (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number));


    /* mise en place de la hbox des montants de chq */

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number));


    label = gtk_label_new ( _("is ") );
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    gsb_data_report_text_comparison_set_button_first_comparison ( text_comparison_number,
								  cree_bouton_comparateur_texte (text_comparison_number));
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)),
			 gsb_data_report_text_comparison_get_button_first_comparison (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( _("to"));
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    gsb_data_report_text_comparison_set_entry_first_amount ( text_comparison_number,
							     gtk_entry_new ());
    gtk_widget_set_size_request ( gsb_data_report_text_comparison_get_entry_first_amount (text_comparison_number),
			   100,
			   -1 );
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)),
			 gsb_data_report_text_comparison_get_entry_first_amount (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (gsb_data_report_text_comparison_get_entry_first_amount (text_comparison_number));


    /* la fonction cree_bouton_lien_montant va se servir de hbox_partie_2 */
    /* il faut donc créer celle ci avant l'appel de la fonction */

    gsb_data_report_text_comparison_set_hbox_second_part ( text_comparison_number,
							   gtk_hbox_new ( FALSE,
									  5 ));

    /* on crée alors le bouton de lien */

    gsb_data_report_text_comparison_set_button_link_first_to_second_part ( text_comparison_number,
									   cree_bouton_lien (gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number)));
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)),
			 gsb_data_report_text_comparison_get_button_link_first_to_second_part (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );

    /* on ajoute la hbox2 */

    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)),
			 gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number));

    /* on peut maintenant ajouter dans hbox_partie_2 */

    gsb_data_report_text_comparison_set_button_second_comparison ( text_comparison_number,
								   cree_bouton_comparateur_texte (text_comparison_number));
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number)),
			 gsb_data_report_text_comparison_get_button_second_comparison (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( _("to") );
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number)),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    gsb_data_report_text_comparison_set_entry_second_amount ( text_comparison_number,
							      gtk_entry_new ());
    gtk_widget_set_size_request ( gsb_data_report_text_comparison_get_entry_second_amount (text_comparison_number),
			   100,
			   -1 );
    gtk_box_pack_start ( GTK_BOX (gsb_data_report_text_comparison_get_hbox_second_part (text_comparison_number)),
			 gsb_data_report_text_comparison_get_entry_second_amount (text_comparison_number),
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (gsb_data_report_text_comparison_get_entry_second_amount (text_comparison_number));

    /* on met les bouton ajouter et supprimer */

    bouton = gtk_button_new_with_label ( _("Add"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT ( bouton ),
			       "clicked",
			       G_CALLBACK ( ajoute_ligne_liste_comparaisons_textes_etat ),
			       GINT_TO_POINTER (text_comparison_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Remove"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT ( bouton ),
			       "clicked",
			       G_CALLBACK ( retire_ligne_liste_comparaisons_textes_etat ),
			       GINT_TO_POINTER (text_comparison_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( vbox );
}
/******************************************************************************/

/******************************************************************************/
void retire_ligne_liste_comparaisons_textes_etat ( gint last_text_comparison_number )
{
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    /* il faut qu'il y ai plus d'une row affichée */

    if ( g_slist_length ( gsb_data_report_get_text_comparison_list (current_report_number)) < 2 )
	return;

    /* on commence par supprimer la row dans la liste */

    gtk_widget_destroy (gsb_data_report_text_comparison_get_vbox_line (last_text_comparison_number));

    /* si la structure qu'on retire est la 1ère, on vire le widget de lient */

    if ( !g_slist_index ( gsb_data_report_get_text_comparison_list (current_report_number),
			  GINT_TO_POINTER (last_text_comparison_number)))
    {
	gint text_comparison_number;

	text_comparison_number = GPOINTER_TO_INT (gsb_data_report_get_text_comparison_list (current_report_number)-> next -> data);
	gtk_widget_destroy (gsb_data_report_text_comparison_get_button_link (text_comparison_number));
	gsb_data_report_text_comparison_set_button_link ( text_comparison_number,
							  NULL );
    }

    /* et on retire la struct de la sliste */

    gsb_data_report_set_text_comparison_list ( current_report_number,
					       g_slist_remove ( gsb_data_report_get_text_comparison_list (current_report_number),
								GINT_TO_POINTER (last_text_comparison_number)));
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec les noms de champs de txt, et rend (non-)sensitif */
/* la hbox correspondante ( txt ou chq ) */
/* il faut donc que hbox_txt,  hbox_chq et les 2 check button soient déjà créées */
/******************************************************************************/
GtkWidget *cree_bouton_champ ( gint text_comparison_number )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    /*   pour chaque item, on désensitive les check button et la row des tests de no, et */
    /* on rend sensitif la row des test en txt */
    /*     sauf pour les items à no (chq et pc) où on rend sensitif les check button et la */
    /*     row correspondans au check button, et désensitive celle où le check button n'est pas */
    /* mis */

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("payee"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 0 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("payee information"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 1 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("category"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 2 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("sub-category"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 3 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("budgetary line"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 4 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("sub-budgetary line"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 5 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("note"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 6 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("bank reference"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 7 ));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( gtk_toggle_button_set_active ),
				G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("voucher"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 8 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
			       "activate",
			       G_CALLBACK ( sensitive_hbox_fonction_bouton_txt ),
			       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );


    menu_item = gtk_menu_item_new_with_label ( _("cheque number"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 9 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( sensitive_hbox_fonction_bouton_txt ),
				GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("reconciliation reference"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 10 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_text (text_comparison_number)));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_button_use_number (text_comparison_number)));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
				"activate",
				G_CALLBACK ( sensitive_hbox_fonction_bouton_txt ),
				GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction est appellée lorsqu'on sélectionne un champ de texte à no */
/* elle rend sensitif la hbox correspondant au check button */
/******************************************************************************/
void sensitive_hbox_fonction_bouton_txt ( gint text_comparison_number )
{
    sens_desensitive_pointeur ( gsb_data_report_text_comparison_get_button_use_text (text_comparison_number),
				gsb_data_report_text_comparison_get_hbox_text (text_comparison_number));
    sens_desensitive_pointeur ( gsb_data_report_text_comparison_get_button_use_number (text_comparison_number),
				gsb_data_report_text_comparison_get_hbox_cheque (text_comparison_number));
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec contient, ne contient pas ... */
/******************************************************************************/
GtkWidget *cree_bouton_operateur_txt ( gint text_comparison_number )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("contains"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 0 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("doesn't contain"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 1 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("begins with"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 2 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("ends with"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 3 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("is empty"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 4 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("isn't empty"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 5 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT (gsb_data_report_text_comparison_get_entry_text (text_comparison_number)));
    gtk_widget_show ( menu_item );


    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_montant ( void )
{
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;
    GtkWidget *scrolled_window;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Amount"), "amount.png" );
    gtk_widget_show ( vbox_onglet );


    /* on commence par créer le choix d'utiliser le montant */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_utilise_montant = gtk_check_button_new_with_label ( _("Select the transactions by amount") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_utilise_montant ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 7 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_utilise_montant,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_utilise_montant );

    vbox_generale_montants_etat = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_generale_montants_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_montants_etat );

    g_signal_connect ( G_OBJECT ( bouton_utilise_montant ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 vbox_generale_montants_etat );



    /* on va ensuite créer la liste qui contiendra les critères */
    /* le remplissage ou la row vide se mettent plus tard */

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_montants_etat ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_montants_etat = gtk_vbox_new ( FALSE, 5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    liste_montants_etat );
    gtk_widget_show ( liste_montants_etat );


    /* on rajoute le bouton exclure les opé dont le montant est nul */

    bouton_exclure_ope_nulles_etat = gtk_check_button_new_with_label ( _("Exclude the transactions with a null amount") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_exclure_ope_nulles_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 7 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_exclure_ope_nulles_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exclure_ope_nulles_etat );


    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
void remplit_liste_comparaisons_montants_etat ( void )
{
    GSList *list_tmp;
    gint current_report_number;
	gchar* tmpstr;

    current_report_number = gsb_gui_navigation_get_current_report ();


    list_tmp = gsb_data_report_get_amount_comparison_list (current_report_number);


    /* commence par effacer l'ancienne liste */

    while ( GTK_BOX ( liste_montants_etat ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( liste_montants_etat ),
			       (( GtkBoxChild *) ( GTK_BOX ( liste_montants_etat ) -> children -> data )) -> widget );


    /*   s'il n'y a rien dans la liste, on met juste une row vide */

    if ( !list_tmp )
    {
	ajoute_ligne_liste_comparaisons_montants_etat (0);
	return;
    }

    /*   on fait le tour de la liste des comparaisons de montant, ajoute une row */
    /* et la remplit à chaque fois */

    while ( list_tmp )
    {
	gint amount_comparison_number;
	GtkWidget *hbox;

	amount_comparison_number = GPOINTER_TO_INT (list_tmp -> data);
	hbox = cree_ligne_comparaison_montant (amount_comparison_number);

	/* on crée la row et remplit les widget de la structure */

	gsb_data_report_amount_comparison_set_hbox_line ( amount_comparison_number,
							  hbox);
	gtk_box_pack_start ( GTK_BOX ( liste_montants_etat ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( hbox );

	/* on remplit maintenant les widget avec les valeurs de la stucture */

	/*       s'il n'y a pas de lien avec la struct précédente, on le vire */
	/* on rajoute le && car parfois le bouton de lien se met quand même en 1ère row */

	if ( gsb_data_report_amount_comparison_get_link_to_last_amount_comparison (amount_comparison_number) != -1
	     &&
	     list_tmp != gsb_data_report_get_amount_comparison_list (current_report_number))
	    gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_link (amount_comparison_number)),
					  gsb_data_report_amount_comparison_get_link_to_last_amount_comparison (amount_comparison_number));
	else
	{
	    gtk_widget_destroy (gsb_data_report_amount_comparison_get_button_link (amount_comparison_number));
	    gsb_data_report_amount_comparison_set_button_link ( amount_comparison_number,
								NULL );
	}


	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_first_comparison (amount_comparison_number)),
				      gsb_data_report_amount_comparison_get_first_comparison (amount_comparison_number));
	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_link_first_to_second_part (amount_comparison_number)),
				      gsb_data_report_amount_comparison_get_link_first_to_second_part (amount_comparison_number));
	gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_second_comparison (amount_comparison_number)),
				      gsb_data_report_amount_comparison_get_second_comparison (amount_comparison_number));

	tmpstr = gsb_real_get_string (gsb_data_report_amount_comparison_get_first_amount (amount_comparison_number));
	gtk_entry_set_text ( GTK_ENTRY (gsb_data_report_amount_comparison_get_entry_first_amount (amount_comparison_number)), tmpstr);
	g_free ( tmpstr );

	tmpstr = gsb_real_get_string (gsb_data_report_amount_comparison_get_second_amount (amount_comparison_number));
	gtk_entry_set_text ( GTK_ENTRY (gsb_data_report_amount_comparison_get_entry_second_amount (amount_comparison_number)), tmpstr);
	g_free ( tmpstr );

	/* on sensitive/désensitive les entrées si nécessaire */

	if ( gsb_data_report_amount_comparison_get_first_comparison (amount_comparison_number) > 5 )
	    desensitive_widget (NULL, gsb_data_report_amount_comparison_get_entry_first_amount (amount_comparison_number));
	else
	    sensitive_widget (NULL, gsb_data_report_amount_comparison_get_entry_first_amount (amount_comparison_number));

	if ( gsb_data_report_amount_comparison_get_second_comparison (amount_comparison_number) > 5 )
	    desensitive_widget (NULL, gsb_data_report_amount_comparison_get_entry_second_amount (amount_comparison_number));
	else
	    sensitive_widget (NULL, gsb_data_report_amount_comparison_get_entry_second_amount (amount_comparison_number));


	/* on sensitive/désensitive la hbox_2 si nécessaire */

	if ( gsb_data_report_amount_comparison_get_link_first_to_second_part (amount_comparison_number) == 3 )
	    desensitive_widget (NULL, gsb_data_report_amount_comparison_get_hbox_second_part (amount_comparison_number));
	else
	    sensitive_widget (NULL, gsb_data_report_amount_comparison_get_hbox_second_part (amount_comparison_number));

	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction ajoute une row vierge */
/* si ancien_comp_montants n'est pas nul, la row est insérée juste après celle de l'argument */
/******************************************************************************/

void ajoute_ligne_liste_comparaisons_montants_etat ( gint last_amount_comparison_number )
{
    gint position;
    gint current_report_number;
    gint amount_comparison_number;
    GtkWidget *hbox;

    current_report_number = gsb_gui_navigation_get_current_report ();


    /* on récupère tout de suite la position à laquelle il faut insérer la row */

    if ( last_amount_comparison_number )
	position = g_slist_index ( gsb_data_report_get_amount_comparison_list (current_report_number),
				   GINT_TO_POINTER (last_amount_comparison_number)) + 1;
    else
	position = 0;

    /* on commence par créer une structure vide */

    amount_comparison_number = gsb_data_report_amount_comparison_new (0);
    gsb_data_report_amount_comparison_set_report_number ( amount_comparison_number,
							  current_report_number);

    /* on crée la row et remplit les widget de la structure */

    hbox = cree_ligne_comparaison_montant (amount_comparison_number);
    gsb_data_report_amount_comparison_set_hbox_line ( amount_comparison_number,
						      hbox);
    gtk_box_pack_start ( GTK_BOX ( liste_montants_etat ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    /* on vire le lien de la row s'il n'y a pas encore de liste */
    /*   (cad si c'est la 1ère row) */

    if ( !gsb_data_report_get_amount_comparison_list (current_report_number))
    {
	gtk_widget_destroy (gsb_data_report_amount_comparison_get_button_link (amount_comparison_number));
	gsb_data_report_amount_comparison_set_button_link ( amount_comparison_number,
							    NULL );
    }

    /*   par défaut, le bouton bouton_lien_1_2 est sur stop */

    gtk_option_menu_set_history ( GTK_OPTION_MENU (gsb_data_report_amount_comparison_get_button_link_first_to_second_part (amount_comparison_number)),
				  3 );
    gsb_data_report_amount_comparison_set_link_first_to_second_part ( amount_comparison_number,
								      3 );
    gtk_widget_set_sensitive ( gsb_data_report_amount_comparison_get_hbox_second_part (amount_comparison_number),
			       FALSE );

    /* on met la structure dans la liste à la position demandée */

    gsb_data_report_set_amount_comparison_list ( current_report_number,
						 g_slist_insert ( gsb_data_report_get_amount_comparison_list (current_report_number),
								  GINT_TO_POINTER (amount_comparison_number),
								  position ));


    /* on met la row à sa place dans la liste */

    gtk_box_reorder_child ( GTK_BOX ( liste_montants_etat ),
			    gsb_data_report_amount_comparison_get_hbox_line (amount_comparison_number),
			    position );
}
/******************************************************************************/

/******************************************************************************/
/* crée la hbox de la row et la renvoie */
/* remplie en même temps les widget de la struct envoyée en argument, sauf hbox_ligne */
/******************************************************************************/
GtkWidget *cree_ligne_comparaison_montant ( gint amount_comparison_number )
{
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *bouton;
    GtkWidget *widget;
    GtkWidget *widget_1;

    /*   on laisse les infos vides, on met juste les boutons et les labels */

    hbox = gtk_hbox_new ( FALSE,
			  5 );

    widget = cree_bouton_lien_lignes_comparaison ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 widget,
			 FALSE,
			 FALSE,
			 0 );
    gsb_data_report_amount_comparison_set_button_link ( amount_comparison_number,
							widget );

    label = gtk_label_new ( _("Transactions with an amount of") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    widget = cree_bouton_comparateur_montant (amount_comparison_number);
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 widget,
			 FALSE,
			 FALSE,
			 0 );
    gsb_data_report_amount_comparison_set_button_first_comparison ( amount_comparison_number,
								    widget );

    label = gtk_label_new ( _("to"));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    widget = gtk_entry_new ();
    gtk_widget_set_size_request ( widget,
			   50,
			   -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 widget,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( widget );
    gsb_data_report_amount_comparison_set_entry_first_amount ( amount_comparison_number,
							       widget);

    /* la fonction cree_bouton_lien va se servir de comp_montants -> hbox_partie_2 */
    /* il faut donc créer celle ci avant l'appel de la fonction */
    /* the hbox is created in widget_1 and is used later */

    widget_1 = gtk_hbox_new ( FALSE,
			      5 );

    /* on crée alors le bouton de lien */

    widget = cree_bouton_lien ( widget_1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 widget,
			 FALSE,
			 FALSE,
			 0 );
    gsb_data_report_amount_comparison_set_button_link_first_to_second_part ( amount_comparison_number,
									     widget);

    /* on ajoute la hbox2 */

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 widget_1,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( widget_1 );
    gsb_data_report_amount_comparison_set_hbox_second_part ( amount_comparison_number,
							     widget_1);

    /* on peut maintenant ajouter dans comp_montants -> hbox_partie_2 */

    widget = cree_bouton_comparateur_montant ( amount_comparison_number );
    gtk_box_pack_start ( GTK_BOX ( widget_1 ),
			 widget,
			 FALSE,
			 FALSE,
			 0 );
    gsb_data_report_amount_comparison_set_button_second_comparison ( amount_comparison_number,
								     widget);

    label = gtk_label_new ( _("to"));
    gtk_box_pack_start ( GTK_BOX ( widget_1 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    widget = gtk_entry_new ();
    gtk_widget_set_size_request ( widget,
			   50,
			   -1 );
    gtk_box_pack_start ( GTK_BOX ( widget_1 ),
			 widget,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( widget );
    gsb_data_report_amount_comparison_set_entry_second_amount ( amount_comparison_number,
								widget);

    /* on met les bouton ajouter et supprimer */

    bouton = gtk_button_new_with_label ( _("Add"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT (bouton),
			       "clicked",
			       G_CALLBACK (ajoute_ligne_liste_comparaisons_montants_etat),
			       GINT_TO_POINTER (amount_comparison_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Remove"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT (bouton),
			       "clicked",
			       G_CALLBACK (retire_ligne_liste_comparaisons_montants_etat),
			       GINT_TO_POINTER (amount_comparison_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    return ( hbox );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec et/ou/sauf */
/******************************************************************************/
GtkWidget *cree_bouton_lien_lignes_comparaison ( void )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("and"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 0 ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("or"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 1 ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("except"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 2 ));
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec =,<,<=,>,>=, nul, non nul, positif, nég */
/******************************************************************************/
GtkWidget *cree_bouton_comparateur_montant ( gint amount_comparison_number )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("equal"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 0 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 1 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than or equal"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 2 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 3 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than or equal"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 4 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("different from"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 5 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("null"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 6 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("not null"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 7 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("positive"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 8 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("negative"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 9 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_montant ),
		       GINT_TO_POINTER (amount_comparison_number));
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction rend sensitive ou non l'entrée après le label après l'option menu du menu_item en argument */
/* donc si on choisit nul, non nul, pos ou nég, on grise l'entrée */
/******************************************************************************/
void change_comparaison_montant ( GtkWidget *menu_item,
				  gint amount_comparison_number )
{
    gint sensitif;

    switch ( GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( menu_item ),
						     "no_comparateur" )))
    {
	case 6:
	case 7:
	case 8:
	case 9:

	    sensitif = 0;
	    break;

	default:

	    sensitif = 1;
    }

    if ( gtk_menu_get_attach_widget ( GTK_MENU ( menu_item -> parent ))
	 ==
	 gsb_data_report_amount_comparison_get_button_first_comparison (amount_comparison_number))
	gtk_widget_set_sensitive ( gsb_data_report_amount_comparison_get_entry_first_amount (amount_comparison_number),
				   sensitif );
    else
	gtk_widget_set_sensitive ( gsb_data_report_amount_comparison_get_entry_second_amount (amount_comparison_number),
				   sensitif );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec =,<,<=,>,>= */
/******************************************************************************/
GtkWidget *cree_bouton_comparateur_texte ( gint text_comparison_number )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("equal"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 0 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_texte ),
		       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 1 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_texte ),
		       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than or equal"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 2 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_texte ),
		       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 3 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_texte ),
		       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than or equal"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 4 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_texte ),
		       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("different from"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 5 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_texte ),
		       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the biggest"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 6 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
		       "activate",
		       G_CALLBACK ( change_comparaison_texte ),
		       GINT_TO_POINTER (text_comparison_number));
    gtk_widget_show ( menu_item );


    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction rend sensitive ou non l'entrée après le label après l'option menu du menu_item en argument */
/* donc si on choisit nul, non nul, pos ou nég, on grise l'entrée */
/******************************************************************************/
void change_comparaison_texte ( GtkWidget *menu_item,
				gint text_comparison_number )
{
    gint sensitif;

    if ( GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( menu_item ),
						 "no_comparateur" )) == 6 )
	sensitif = 0;
    else
	sensitif = 1;


    if ( gtk_menu_get_attach_widget ( GTK_MENU ( menu_item -> parent )) ==  gsb_data_report_text_comparison_get_button_first_comparison (text_comparison_number))
	gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_entry_first_amount (text_comparison_number),
				   sensitif );
    else
	gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_entry_second_amount (text_comparison_number),
				   sensitif );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec et/ou/aucun */
/******************************************************************************/

GtkWidget *cree_bouton_lien ( GtkWidget *hbox )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("and"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 0 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("or"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 1 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("except"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 2 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( sensitive_widget ),
			 G_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("stop"));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 3 ));
    g_signal_connect ( G_OBJECT ( menu_item ),
			 "activate",
			 G_CALLBACK ( desensitive_widget ),
			 G_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton ),
				  3 );
    return ( bouton );
}
/******************************************************************************/



/******************************************************************************/
void retire_ligne_liste_comparaisons_montants_etat ( gint last_amount_comparison_number )
{
     gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    /* il faut qu'il y ai plus d'une row affichée */

    if ( g_slist_length ( gsb_data_report_get_amount_comparison_list (current_report_number)) < 2 )
	return;

    /* on commence par supprimer la row dans la liste */

    gtk_widget_destroy ( gsb_data_report_amount_comparison_get_hbox_line (last_amount_comparison_number));

    /* si la structure qu'on retire est la 1ère, on vire le widget de lient */

    if ( !g_slist_index ( gsb_data_report_get_amount_comparison_list (current_report_number),
			  GINT_TO_POINTER (last_amount_comparison_number)))
    {
	gint amount_comparison_number;

	amount_comparison_number = GPOINTER_TO_INT (gsb_data_report_get_amount_comparison_list (current_report_number)-> next -> data);
	gtk_widget_destroy ( gsb_data_report_amount_comparison_get_button_link (amount_comparison_number));
	gsb_data_report_amount_comparison_set_button_link ( amount_comparison_number,
							    NULL );
    }

    /* et on retire la struct de la sliste */

    gsb_data_report_set_amount_comparison_list ( current_report_number,
						 g_slist_remove ( gsb_data_report_get_amount_comparison_list (current_report_number),
								  GINT_TO_POINTER (last_amount_comparison_number)));
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_divers ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *frame;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Miscellaneous"), "generalities.png" );
    gtk_widget_show ( vbox_onglet );



    /* on peut sélectionner les opé R ou non R */
    frame = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Reconciled transactions") );

    bouton_opes_r_et_non_r_etat = gtk_radio_button_new_with_label ( NULL,
								    _("Select all transactions") );
    gtk_box_pack_start ( GTK_BOX ( frame ),
			 bouton_opes_r_et_non_r_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_opes_r_et_non_r_etat );

    bouton_opes_non_r_etat = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_opes_r_et_non_r_etat )),
							       _("Select unreconciled transactions") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_opes_non_r_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 9 ));
    gtk_box_pack_start ( GTK_BOX ( frame ),
			 bouton_opes_non_r_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_opes_non_r_etat );

    bouton_opes_r_etat = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_opes_r_et_non_r_etat )),
							   _("Select reconciled transactions") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_opes_r_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 9 ));
    gtk_box_pack_start ( GTK_BOX ( frame ),
			 bouton_opes_r_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_opes_r_etat );

    frame = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Split of transactions detail") );

    bouton_pas_detailler_ventilation = gtk_check_button_new_with_label ( _("Do not detail split of transactions") );
    g_signal_connect_swapped ( G_OBJECT ( bouton_pas_detailler_ventilation ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 9 ));
    gtk_box_pack_start ( GTK_BOX ( frame ),
			 bouton_pas_detailler_ventilation,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_pas_detailler_ventilation );

    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *page_data_grouping ( void )
{
    GtkWidget *hbox;
    GtkWidget *scrolled_window;
    GtkWidget *vbox;
    GtkWidget *fleche;
    GtkWidget *vbox_onglet;
    GtkWidget * paddingbox;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Data grouping"), "organization.png" );
    gtk_widget_show ( vbox_onglet );

    /* choix de ce qu'on utilise dans le classement */

    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Group transactions") );

    bouton_regroupe_ope_compte_etat = gtk_check_button_new_with_label ( _("Group transactions by account") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_regroupe_ope_compte_etat, FALSE, FALSE, 0 );

    bouton_utilise_tiers_etat = gtk_check_button_new_with_label ( _("Group transactions by payee") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_utilise_tiers_etat, FALSE, FALSE, 0 );

    button_group_by_categ = gtk_check_button_new_with_label ( _("Group transactions by category") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_group_by_categ, FALSE, FALSE, 0 );

    bouton_utilise_ib_etat = gtk_check_button_new_with_label ( _("Group transactions by budgetary line") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_utilise_ib_etat, FALSE, FALSE, 0 );

    /* choix du type de classement */

    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Group level organisation") );
    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( hbox ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 hbox,
			 TRUE,
			 TRUE,
			 0 );

    gtk_widget_show ( hbox );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );


    liste_type_classement_etat = gtk_ctree_new ( 1, 0 );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_type_classement_etat ),
				       0,
				       TRUE );
    gtk_ctree_set_line_style ( GTK_CTREE ( liste_type_classement_etat ),
			       GTK_CTREE_LINES_NONE );
    gtk_ctree_set_expander_style ( GTK_CTREE ( liste_type_classement_etat ),
				   GTK_CTREE_EXPANDER_NONE );

    g_signal_connect ( G_OBJECT ( liste_type_classement_etat ),
			 "button_press_event",
			 G_CALLBACK ( click_liste_etat ),
			 GINT_TO_POINTER (1) );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_type_classement_etat );
    gtk_widget_show ( liste_type_classement_etat );

    /* on place ici les flèches sur le côté de la liste */

    vbox = gtk_vbutton_box_new ();

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 vbox,
			 FALSE,
			 FALSE,
			 0);

    fleche = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
    gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( fleche ),
			 "clicked",
			 G_CALLBACK ( click_haut_classement_etat ),
			 NULL );
    gtk_container_add ( GTK_CONTAINER ( vbox ),
			fleche  );

    fleche = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
    gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( fleche ),
			 "clicked",
			 G_CALLBACK ( click_bas_classement_etat ),
			 NULL);
    gtk_container_add ( GTK_CONTAINER ( vbox ),
			fleche  );
    gtk_widget_show_all ( vbox );

    return vbox_onglet;
}


GtkWidget *page_data_separation ( void )
{
    GtkWidget *label;
    GtkWidget *vbox_onglet;
    GtkWidget *menu;
    GtkWidget *menu_item;
	GtkWidget *table;
    GtkWidget * paddingbox;
    gint i;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Data separation"), "organization.png" );
    gtk_widget_show ( vbox_onglet );

    /* choix de ce qu'on utilise dans le classement */
    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Data separation") );

    /* on permet la séparation des revenus et des dépenses */
    bouton_separer_revenus_depenses = gtk_check_button_new_with_label ( _("Split incomes and outgoings") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_separer_revenus_depenses, FALSE, FALSE, 0 );

    /* on permet la séparation par exercice */
    bouton_separe_exo_etat = gtk_check_button_new_with_label ( _("Split by financial year") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_separe_exo_etat, FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT ( radio_button_utilise_exo ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_separe_exo_etat );

    bouton_separe_plages_etat = gtk_check_button_new_with_label ( _("Split by period") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_separe_plages_etat, FALSE, FALSE, 0 );

    /* on permet ensuite la séparation des résultats par période */
    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Separation by period") );

    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 5 );

    label = gtk_label_new ( _("Separation by time period: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* add split button by day, week... */
    bouton_type_separe_plages_etat = gtk_option_menu_new ();
    g_signal_connect ( G_OBJECT (bouton_separe_plages_etat),
		       "toggled",
		       G_CALLBACK (sens_desensitive_pointeur),
		       bouton_type_separe_plages_etat );

    gtk_table_attach ( GTK_TABLE ( table ), bouton_type_separe_plages_etat,
		       1, 2, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    menu = gtk_menu_new ();
    menu_item = gtk_menu_item_new_with_label ( _("Day") );
    g_object_set_data ( G_OBJECT ( menu_item ), "type", NULL );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Week") );
    g_object_set_data ( G_OBJECT ( menu_item ), "type", GINT_TO_POINTER (1) );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Month") );
    g_object_set_data ( G_OBJECT ( menu_item ), "type", GINT_TO_POINTER (2) );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Year") );
    g_object_set_data ( G_OBJECT ( menu_item ), "type", GINT_TO_POINTER (3));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ), menu );

    /* add beginning day of the week */
    label = gtk_label_new ( _("Beginning of week: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    bouton_debut_semaine = gtk_option_menu_new ();
    g_signal_connect ( G_OBJECT (bouton_separe_plages_etat),
		       "toggled",
		       G_CALLBACK (sens_desensitive_pointeur),
		       bouton_debut_semaine );

    gtk_table_attach ( GTK_TABLE ( table ), bouton_debut_semaine,
		       1, 2, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    menu = gtk_menu_new ();

    i = 0;

    while ( jours_semaine[i] )
    {
	menu_item = gtk_menu_item_new_with_label ( _(jours_semaine[i]) );
	g_object_set_data ( G_OBJECT ( menu_item ), "day", GINT_TO_POINTER (i));
	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
	i++;
    }
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_debut_semaine ), menu );

    /* on connecte le signal "changed" au bouton bouton_type_separe_plages_etat
     * pour rendre insensible le choix du jour de la semaine pour les choix
     * autres que la semaine. On le met ici pour que l'initialisation se fasse
     * proprement */
    g_signal_connect ( G_OBJECT ( bouton_type_separe_plages_etat ), 
                  "changed",
		          G_CALLBACK ( etat_option_menu_changed ), 
                  bouton_debut_semaine );

    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
void click_haut_classement_etat ( void )
{
    GtkCTreeNode *node_parent;
    GtkCTreeNode *node;
    GtkCTreeNode *node_enfant;
    GtkCTreeNode *nouveau_parent;

    node = GTK_CLIST ( liste_type_classement_etat )->selection -> data;

    /*   si on est au niveau 1, peut pas plus haut */

    if ( GTK_CTREE_ROW ( node ) -> level == 1 )
	return;

    node_parent = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->parent;
    node_enfant = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->children;

    nouveau_parent = GTK_CTREE_ROW ( node_parent )->parent;

    /* on remonte le node */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node,
		     nouveau_parent,
		     NULL );

    /* on descend celui du dessus */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node_parent,
		     node,
		     NULL );

    /* on attache l'enfant à son nouveau parent */

    if ( node_enfant )
	gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
			 node_enfant,
			 node_parent,
			 NULL );

    gtk_ctree_expand_recursive ( GTK_CTREE ( liste_type_classement_etat ),
				 node );
}
/******************************************************************************/

/******************************************************************************/
void click_bas_classement_etat ( void )
{
    GtkCTreeNode *node_parent;
    GtkCTreeNode *node;
    GtkCTreeNode *node_enfant;
    GtkCTreeNode *node_enfant_enfant;

    node = GTK_CLIST ( liste_type_classement_etat )->selection -> data;

    /*   si on est au niveau 4, peut pas plus bas */

    if ( GTK_CTREE_ROW ( node ) -> level == 4 )
	return;

    node_parent = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->parent;
    node_enfant = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->children;

    node_enfant_enfant = GTK_CTREE_ROW ( node_enfant )->children;

    /* on remonte le node enfant */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node_enfant,
		     node_parent,
		     NULL );

    /* on descend le node */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node,
		     node_enfant,
		     NULL );

    /* on attache l'enfant de l'enfant à son nouveau parent */

    if ( node_enfant_enfant )
	gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
			 node_enfant_enfant,
			 node,
			 NULL );

    gtk_ctree_expand_recursive ( GTK_CTREE ( liste_type_classement_etat ),
				 node );
}



GtkWidget *onglet_affichage_etat_generalites ( void )
{
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Generalities"), "generalities.png" );
    gtk_widget_show ( vbox_onglet );

    /* choix du name du rapport */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Report name: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    entree_nom_etat = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 entree_nom_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( entree_nom_etat );


    /* afficher ou non le nb d'opés avec les totaux */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_afficher_nb_opes = gtk_check_button_new_with_label ( 
                        _("Display the number of transactions with the totals") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_nb_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_nb_opes );

    /* mise en place du bouton pour faire apparaitre l'état dans la liste des tiers */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_inclure_dans_tiers = gtk_check_button_new_with_label ( _("Consider the payees of this report as a multiple payee.") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_inclure_dans_tiers,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclure_dans_tiers );

    return ( vbox_onglet );
}



/**
 *
 *
 *
 *
 */
GtkWidget *onglet_affichage_etat_operations ( void )
{
    GtkWidget * table, * vbox_onglet, * menu, * menu_item, * paddingbox;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Transactions display"), "transdisplay.png" );
    gtk_widget_show ( vbox_onglet );

    /* afficher ou non les opés */

    bouton_afficher_opes = gtk_check_button_new_with_label ( _("Display transactions") );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), bouton_afficher_opes, FALSE, FALSE, 0 );

    vbox_show_transactions = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (vbox_onglet),
			 vbox_show_transactions,
			 FALSE, FALSE, 0);
    g_signal_connect ( G_OBJECT (bouton_afficher_opes),
		       "toggled",
		       G_CALLBACK (sens_desensitive_pointeur),
		       vbox_show_transactions );

    /* demande les détails affichés dans les opérations */
    paddingbox = new_paddingbox_with_title ( vbox_show_transactions, FALSE,
                        _("Include following information") );
    /* connection pour rendre sensitif la frame */
    table = gtk_table_new ( 12, 3, FALSE );
    gtk_box_pack_start ( GTK_BOX (paddingbox), table, FALSE, FALSE, 0 );

    bouton_afficher_no_ope = gtk_check_button_new_with_label ( _("transaction number") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_no_ope, 0, 1, 0, 1 );

    bouton_afficher_date_opes = gtk_check_button_new_with_label ( _("date") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_date_opes, 1, 2, 0, 1 );

    bouton_afficher_value_date_opes = gtk_check_button_new_with_label ( _("value date") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_value_date_opes, 2, 3, 0, 1 );

    bouton_afficher_tiers_opes = gtk_check_button_new_with_label ( _("payee") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_tiers_opes, 0, 1, 1, 2 );

    bouton_afficher_categ_opes = gtk_check_button_new_with_label ( _("category") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_categ_opes, 1, 2, 1, 2 );

    bouton_afficher_sous_categ_opes = gtk_check_button_new_with_label ( _("sub-category") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_sous_categ_opes, 2, 3, 1, 2 );

    bouton_afficher_ib_opes = gtk_check_button_new_with_label ( _("budgetary line") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_ib_opes, 0, 1, 2, 3 );

    bouton_afficher_sous_ib_opes = gtk_check_button_new_with_label ( _("sub-budgetary line") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_sous_ib_opes, 1, 2, 2, 3 );

    bouton_afficher_notes_opes = gtk_check_button_new_with_label ( _("notes") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_notes_opes, 2, 3, 2, 3 );

    bouton_afficher_type_ope = gtk_check_button_new_with_label ( _("method of payment") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_type_ope, 0, 1, 3, 4 );

    bouton_afficher_no_cheque = gtk_check_button_new_with_label ( _("cheque/transfer number") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_no_cheque, 1, 2, 3, 4 );

    bouton_afficher_pc_opes = gtk_check_button_new_with_label ( _("voucher") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_pc_opes, 2, 3, 3, 4 );

    bouton_afficher_exo_opes = gtk_check_button_new_with_label ( _("financial year") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_exo_opes, 2, 3, 4, 5 );

    bouton_afficher_infobd_opes = gtk_check_button_new_with_label ( _("bank reference") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_infobd_opes, 0, 1, 4, 5 );

    bouton_afficher_no_rappr = gtk_check_button_new_with_label ( _("reconciliation reference") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_afficher_no_rappr, 1, 2, 4, 5  );


    /* on propose d'afficher le titre des colonnes */
    paddingbox = new_paddingbox_with_title ( vbox_show_transactions, FALSE, _("Columns") );

    bouton_afficher_titres_colonnes = gtk_check_button_new_with_label ( _("Display column titles") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_afficher_titres_colonnes, FALSE, FALSE, 0 );

    bouton_titre_changement = gtk_radio_button_new_with_label ( NULL, _("every section") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_titre_changement, FALSE, FALSE, 0 );

    bouton_titre_en_haut = gtk_radio_button_new_with_label ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( bouton_titre_changement )),
							     _("at the top of the report") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_titre_en_haut, FALSE, FALSE, 0 );


    /* mise en place du type de classement des opés */
    paddingbox = new_paddingbox_with_title ( vbox_show_transactions, FALSE, _("Sort transactions by") );

    bouton_choix_classement_ope_etat = gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_choix_classement_ope_etat, FALSE, FALSE, 0 );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("date"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 0 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("value date"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("transaction number"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 2 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("payee"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 3 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("category"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 4 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("budgetary line"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 5 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("note"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 6 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("method of payment"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 7 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("cheque/transfer number"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 8 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("voucher"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 9 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("bank reference"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 10 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("reconciliation reference"));
    g_object_set_data ( G_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 11 ));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ),
		      menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_classement_ope_etat ),
			       menu );


    /* on propose de rendre clickable le détails des opérations */
    paddingbox = new_paddingbox_with_title ( vbox_show_transactions, FALSE, _("Clickable transactions") );

    bouton_rendre_ope_clickables = gtk_check_button_new_with_label ( _("Make transactions clickable"));
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_rendre_ope_clickables, FALSE, FALSE, 0 );


    /* on met les connexions */
    g_signal_connect ( G_OBJECT ( bouton_afficher_categ_opes ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_categ_opes );
    g_signal_connect ( G_OBJECT ( bouton_afficher_ib_opes ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_ib_opes );
    g_signal_connect ( G_OBJECT ( bouton_afficher_titres_colonnes ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_titre_changement );
    g_signal_connect ( G_OBJECT ( bouton_afficher_titres_colonnes ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_titre_en_haut );

    gtk_widget_show_all ( vbox_onglet );

    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_affichage_etat_devises ( void )
{
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;


    vbox_onglet = new_vbox_with_title_and_icon ( _("Totals currencies"), "currencies.png" );
    gtk_widget_show ( vbox_onglet );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("General totals currency: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    bouton_devise_general_etat = gsb_currency_make_combobox (FALSE);
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_devise_general_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_general_etat );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    label = gtk_label_new ( _("Payees currency: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    bouton_devise_tiers_etat = gsb_currency_make_combobox (FALSE);
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_devise_tiers_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_tiers_etat );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Categories currency: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );


    bouton_devise_categ_etat = gsb_currency_make_combobox (FALSE);
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_devise_categ_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_categ_etat );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Budgetary lines currency: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    bouton_devise_ib_etat = gsb_currency_make_combobox (FALSE);
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_devise_ib_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_ib_etat );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Amount comparison currency: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    bouton_devise_montant_etat = gsb_currency_make_combobox (FALSE);
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_devise_montant_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_montant_etat );


    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_affichage_etat_divers ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *paddingbox;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Titles"), "title.png" );
    gtk_widget_show ( vbox_onglet );

    /* mise en place de la paddingbox des comptes */
    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Accounts") );

    /* on permet d'afficher le name du compte */
    bouton_afficher_noms_comptes = gtk_check_button_new_with_label ( _("Display account name") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_noms_comptes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_comptes );


    /* on permet d'afficher un ss total lors de chgt de compte */
    /* activé uniquement si on a regroupé les opés par compte */

    bouton_affiche_sous_total_compte = gtk_check_button_new_with_label ( _("Display a sub-total for each account") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_affiche_sous_total_compte,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_compte );

    g_signal_connect ( G_OBJECT ( bouton_regroupe_ope_compte_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_compte );

    /* mise en place de la paddingbox des tiers */

    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Payee") );

    /* permet d'afficher le name du tiers, activé uniquement si on utilise les tiers */
    bouton_afficher_noms_tiers = gtk_check_button_new_with_label ( _("Display payee's name") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_noms_tiers,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_tiers );

    g_signal_connect ( G_OBJECT ( bouton_utilise_tiers_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_noms_tiers );

    /* permet d'afficher un ss total lors de chgt de tiers */
    /* activé uniquement si on utilise les tiers */

    bouton_affiche_sous_total_tiers = gtk_check_button_new_with_label ( 
                        _("Display a sub-total for each payee") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_affiche_sous_total_tiers,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_tiers );

    g_signal_connect ( G_OBJECT ( bouton_utilise_tiers_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_tiers );


    /* mise en place de la paddingbox des catégories */
    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Categories") );

    /* affichage possible du name de la categ */
    bouton_afficher_noms_categ = gtk_check_button_new_with_label ( _("Display the (sub)category's name") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_noms_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_categ );

    g_signal_connect ( G_OBJECT ( button_group_by_categ ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_noms_categ );

    /* permet d'afficher un sous total lors de chgt de categ */
    /* activé uniquement si on utilise les categ */

    bouton_affiche_sous_total_categ = gtk_check_button_new_with_label ( _("Display a sub-total for each category") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_affiche_sous_total_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_categ );

    g_signal_connect ( G_OBJECT ( button_group_by_categ ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_categ );

    /* mise en place du bouton pour afficher les sous categ */

    bouton_afficher_sous_categ = gtk_check_button_new_with_label ( _("Display sub-categories") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_sous_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_sous_categ );

    g_signal_connect ( G_OBJECT ( button_group_by_categ ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_categ );


    bouton_affiche_sous_total_sous_categ = gtk_check_button_new_with_label ( _("Display a sub-total for each sub-category") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_affiche_sous_total_sous_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_sous_categ );

    g_signal_connect ( G_OBJECT ( button_group_by_categ ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_sous_categ );


    bouton_afficher_pas_de_sous_categ = gtk_check_button_new_with_label ( _("Display \" No sub-category\" if none") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_pas_de_sous_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_pas_de_sous_categ );

    g_signal_connect ( G_OBJECT ( button_group_by_categ ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_pas_de_sous_categ );


    /* mise en place de la paddingbox des ib */
    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Budgetary lines") );

    bouton_afficher_noms_ib = gtk_check_button_new_with_label ( _("Display the (sub-)budget line name") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_noms_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_ib );

    g_signal_connect ( G_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_noms_ib );

    /* permet d'afficher un sous total lors de chgt d'ib */
    /* activé uniquement si on utilise les ib */

    bouton_affiche_sous_total_ib = gtk_check_button_new_with_label ( _("Display a sub-total for each budgetary line") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_affiche_sous_total_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_ib );

    g_signal_connect ( G_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_ib );

    /* mise en place du bouton pour afficher les sous categ */

    bouton_afficher_sous_ib = gtk_check_button_new_with_label ( _("Display sub-budget lines") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_sous_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_sous_ib );

    g_signal_connect ( G_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_ib );

    bouton_affiche_sous_total_sous_ib = gtk_check_button_new_with_label ( _("Display a sub-total for each sub-budget line") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_affiche_sous_total_sous_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_sous_ib );

    g_signal_connect ( G_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_sous_ib );


    bouton_afficher_pas_de_sous_ib = gtk_check_button_new_with_label ( _("Display \"No sub-budgetary line\" if none") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 bouton_afficher_pas_de_sous_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_pas_de_sous_ib );

    g_signal_connect ( G_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 bouton_afficher_pas_de_sous_ib );

    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_mode_paiement ( void )
{
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    vbox_onglet = new_vbox_with_title_and_icon ( _("Payment methods"), "payment.png" );
    gtk_widget_show ( vbox_onglet );

    /* on met dans la partie de gauche une liste contenant les modes de paiement à */
    /* sélectionner */

    bouton_detaille_mode_paiement_etat = gtk_check_button_new_with_label ( _("Select the transactions by method of payment"));
    g_signal_connect_swapped ( G_OBJECT ( bouton_detaille_mode_paiement_etat ),
				"toggled",
				G_CALLBACK ( report_tree_update_style ),
				GINT_TO_POINTER ( 8 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_detaille_mode_paiement_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_mode_paiement_etat );

    vbox_mode_paiement_etat = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_mode_paiement_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_mode_paiement_etat );


    g_signal_connect ( G_OBJECT ( bouton_detaille_mode_paiement_etat ),
			 "toggled",
			 G_CALLBACK ( sens_desensitive_pointeur ),
			 vbox_mode_paiement_etat );

    label = gtk_label_new ( _("Select methods of payment to include: ") );
	gtk_misc_set_padding ( GTK_MISC ( label ), 3, 0 );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );

    gtk_box_pack_start ( GTK_BOX ( vbox_mode_paiement_etat ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_mode_paiement_etat ),
			 hbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_mode_paiement_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_mode_paiement_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_mode_paiement_etat ),
				       0,
				       TRUE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_mode_paiement_etat );
    gtk_widget_show ( liste_mode_paiement_etat );

    /* on remplit la liste des comptes */

    remplissage_liste_modes_paiement_etats ();



    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_select_all ),
				G_OBJECT  ( liste_mode_paiement_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
	gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.1, 0.0 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_swapped ( G_OBJECT  ( bouton ),
				"clicked",
				G_CALLBACK ( gtk_clist_unselect_all ),
				G_OBJECT  ( liste_mode_paiement_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( vbox_onglet );
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_modes_paiement_etats ( void )
{
    GSList *liste_nom_types = NULL;
    GSList *list_tmp;


    if ( !liste_comptes_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_mode_paiement_etat ) );

    /* create a list of unique names */

    list_tmp = gsb_data_payment_get_payments_list ();

    while (list_tmp)
    {
	gint payment_number;

	payment_number = gsb_data_payment_get_number (list_tmp -> data);

	if ( !g_slist_find_custom ( liste_nom_types,
				    gsb_data_payment_get_name (payment_number),
				    (GCompareFunc) cherche_string_equivalente_dans_slist ))
	    liste_nom_types = g_slist_append ( liste_nom_types,
					       my_strdup (gsb_data_payment_get_name (payment_number)));

	list_tmp = list_tmp -> next;
    }

    /* sort and set that list in the clist and associate the name to the row */
    liste_nom_types = g_slist_sort ( liste_nom_types, (GCompareFunc) my_strcasecmp );

    list_tmp = liste_nom_types;

    while ( list_tmp )
    {
	gint row;
	gchar * string = list_tmp -> data;

	row = gtk_clist_append ( GTK_CLIST ( liste_mode_paiement_etat ),
				 &string );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_mode_paiement_etat ),
				 row,
				 list_tmp -> data );

	list_tmp = list_tmp -> next;
    }
}
/******************************************************************************/



/******************************************************************************/
void selectionne_liste_modes_paiement_etat_courant ( void )
{
    gint i;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( !liste_comptes_etat )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_mode_paiement_etat ));

    if ( !gsb_data_report_get_method_of_payment_list (current_report_number))
	return;

    /* on fait le tour de la liste pour voir s'il y a un état sélectionné */

    for ( i=0 ; i < GTK_CLIST ( liste_mode_paiement_etat ) -> rows ; i++ )
    {
	if ( g_slist_find_custom ( gsb_data_report_get_method_of_payment_list (current_report_number),
				   gtk_clist_get_row_data ( GTK_CLIST ( liste_mode_paiement_etat ),
							    i ),
				   (GCompareFunc) cherche_string_equivalente_dans_slist ))
	    gtk_clist_select_row ( GTK_CLIST ( liste_mode_paiement_etat ),
				   i,
				   0 );
    }
}
/******************************************************************************/

/******************************************************************************/
void etat_option_menu_changed (GtkWidget *optionmenu, GtkWidget *user_data)
{
    gint current_report_number = gsb_gui_navigation_get_current_report();
    if ( gsb_data_report_get_period_split (current_report_number))
    {
    if (gtk_option_menu_get_history ( GTK_OPTION_MENU ( optionmenu )) == 1)
        gtk_widget_set_sensitive ( user_data, TRUE );
    else
        gtk_widget_set_sensitive ( user_data, FALSE );
    }
}
/******************************************************************************/

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
