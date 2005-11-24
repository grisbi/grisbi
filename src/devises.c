/* Fichier devises.c */
/* s'occupe de tout ce qui concerne les devises */

/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org) */
/*			2003 Benjamin Drieu (bdrieu@april.org) */
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
#include "devises.h"
#include "dialog.h"
#include "utils_devises.h"
#include "gsb_data_account.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "comptes_gestion.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "utils_editables.h"
#include "etats_config.h"
#include "utils_buttons.h"
#include "affichage_formulaire.h"
#include "operations_formulaire.h"
#include "structures.h"
#include "devises.h"
#include "gsb_file_config.h"
#include "echeancier_formulaire.h"
#include "include.h"
#include "devises_constants.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void append_currency_to_currency_list ( GtkTreeStore * model, struct struct_devise * devise );
static struct cached_exchange_rate *cached_exchange_rate ( struct struct_devise * currency1, 
						    struct struct_devise * currency2 );
static gboolean changement_code_entree_devise ( GtkEditable *editable, gchar * text,
					 gint length, gpointer data );
static gboolean changement_iso_code_entree_devise ( GtkEditable *editable, gchar * text,
					     gint length, gpointer data );
static gboolean changement_nom_entree_devise ( GtkEditable *editable, gchar * text,
					gint length, gpointer data );
static struct struct_devise *create_currency ( gchar * nom_devise, gchar * code_devise, 
					gchar * code_iso4217_devise );
static struct struct_devise * currency_get_selected ( GtkTreeView * view );
static gboolean devise_selectionnee ( GtkWidget *menu_shell, gint origine );
static void fill_currency_list ( GtkTreeView * view, gboolean include_obsolete );
static gint gsb_currency_find_currency_in_option_menu ( GtkWidget *option_menu,
						 gint no_currency );
static gboolean gsb_gui_select_default_currency ( GtkTreeModel * tree_model, GtkTreePath * path, 
					   GtkTreeIter * iter, GtkTreeView * view );
static GtkWidget * new_currency_option_menu ( gint * value, GCallback hook );
static GtkWidget * new_currency_tree ();
static GtkWidget * new_currency_vbox ();
static gboolean rebuild_currency_list ( GtkWidget * checkbox, GtkTreeView * view );
static void remove_selected_currency_from_currency_view ( GtkTreeView * view );
static void retrait_devise ( GtkWidget *bouton, GtkWidget * view );
static gboolean select_currency_in_iso_list ( GtkTreeSelection *selection, GtkTreeModel *model );
static void update_currency_widgets();
static void update_exchange_rate_cache ( struct struct_devise * currency1, 
				  struct struct_devise * currency2,
				  gdouble change, gdouble fees );
/*END_STATIC*/


GtkWidget *entree_nom, *entree_code, *entree_iso_code;

GtkTreeModel * currency_list_model;
GtkTreeView * currency_list_view;

/** Exchange rates cache, used by update_exchange_rate_cache and
  cached_exchange_rates */
GSList * cached_exchange_rates = NULL;

/** This structure holds informations needed for exchange rates
  cache. */
struct cached_exchange_rate {
    struct struct_devise * currency1;	/** First currency */
    struct struct_devise * currency2;	/** Second currency */
    gdouble rate;				/** Exchange rate betweend
						  currency1 and currency 2 */
    gdouble fees;				/** Fees associated with
						  exchange rate */
};


GSList *liste_struct_devises;
gint nb_devises;
gint no_derniere_devise;
struct struct_devise *devise_nulle;
gdouble taux_de_change[2];
GtkWidget *option_menu_devise_1;
GtkWidget *option_menu_devise_2;


GtkWidget *bouton_supprimer_devise;
GtkWidget *entree_nom_devise_parametres;
GtkWidget *entree_iso_code_devise_parametres;
GtkWidget *entree_code_devise_parametres;

struct struct_devise *devise_compte;
struct struct_devise *devise_operation;




/*START_EXTERN*/
extern GtkWidget *bouton_devise_categ_etat;
extern GtkWidget *bouton_devise_ib_etat;
extern GtkWidget *bouton_devise_montant_etat;
extern GtkWidget *bouton_devise_tiers_etat;
extern GtkWidget *detail_devise_compte;
extern GtkWidget *hbox_boutons_modif;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern GtkTreeStore *model;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkWidget *onglet_config_etat;
extern GtkTreeSelection * selection;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/*END_EXTERN*/



/**
 *
 *
 *
 */
gboolean select_currency_in_iso_list ( GtkTreeSelection *selection, GtkTreeModel *model )
{
    gchar * currency_name, * currency_iso_code, * currency_nickname;
    GtkWidget * entry_name, * entry_iso_code, * entry_code;
    GtkTreeIter iter;

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    entry_name = g_object_get_data ( G_OBJECT(model), "entry_name" );
    entry_iso_code = g_object_get_data ( G_OBJECT(model), "entry_iso_code" );
    entry_code = g_object_get_data ( G_OBJECT(model), "entry_code" );

    gtk_tree_model_get ( model, &iter, 
			 CURRENCY_NAME_COLUMN, &currency_name,
			 CURRENCY_ISO_CODE_COLUMN, &currency_iso_code,
			 CURRENCY_NICKNAME_COLUMN, &currency_nickname, 
			 -1 );

    if ( ! currency_name ) 
	currency_name = "";
    if ( ! currency_nickname ) 
	currency_nickname = "";
    if ( ! currency_iso_code ) 
	currency_iso_code = "";

    gtk_entry_set_text ( GTK_ENTRY ( entry_name ), currency_name );
    gtk_entry_set_text ( GTK_ENTRY ( entry_iso_code ), currency_iso_code );
    gtk_entry_set_text ( GTK_ENTRY ( entry_code ), currency_nickname );

    return ( FALSE );
} 



/**
 * Update various widgets related to currencies
 */
void update_currency_widgets()
{
    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_DEVISE ))
    {
	gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_DEVISE) ) -> menu );
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_DEVISE) ),
				   creation_option_menu_devises ( -1,
								  liste_struct_devises ));
    }

    gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ) -> menu );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
			       creation_option_menu_devises ( -1,
							      liste_struct_devises ));

    /* on modifie la liste des devises de l'option menu du detail des comptes */

    gtk_widget_destroy ( GTK_OPTION_MENU ( detail_devise_compte ) -> menu );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_devise_compte ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte  ) ),
				"changed",
				GTK_SIGNAL_FUNC ( modif_detail_compte ),
				GTK_OBJECT ( hbox_boutons_modif ) );
    gtk_option_menu_set_history ( GTK_OPTION_MENU (  detail_devise_compte),
				  g_slist_index ( liste_struct_devises,
						  devise_par_no ( gsb_data_account_get_currency (gsb_data_account_get_current_account ()) ))); 


    /* on recrÃ©e les boutons de devises dans la conf de l'Ã©tat */

    if ( onglet_config_etat )
    {
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_montant_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));

	selectionne_devise_categ_etat_courant ();
	selectionne_devise_ib_etat_courant ();
	selectionne_devise_tiers_etat_courant ();
    }

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_echeances_auto_accueil = 1;
}



/* **************************************************************************************************** */
/* Fonction creation_option_menu_devises */
/* retourne le menu d'un option menu avec les diffÃ©rentes devises dÃ©finies */
/* si devise_cachee = -1, met toutes les devises sous forme de leur sigle */
/* sinon, ne met pas la devise correspondant Ã  devise_cachee ( 0 = aucune ) */
/* liste_tmp est la liste utilisÃ©e : soit liste_struct_devises dans le cas gÃ©nÃ©ral, */
/*                       soit liste_struct_devises dans le cas des paramÃ¨tres */
/* **************************************************************************************************** */

GtkWidget *creation_option_menu_devises ( gint devise_cachee, GSList *liste_tmp )
{
    GtkWidget *menu;
    GtkWidget *menu_item;


    menu = gtk_menu_new ();

    if ( devise_cachee > 0 )
    {

	/* le 1er item c'est aucune devise */

	menu_item = gtk_menu_item_new_with_label ( _("No associated currency") );
	g_object_set_data ( G_OBJECT ( menu_item ),
			    "adr_devise",
			    devise_nulle );
	g_object_set_data ( G_OBJECT ( menu_item ),
			    "no_devise",
			    GINT_TO_POINTER ( 0 ) );

	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );

	gtk_widget_show ( menu_item );
    }


    while ( liste_tmp )
    {
	struct struct_devise *devise;

	devise = liste_tmp -> data;

	if ( devise_cachee != devise -> no_devise )
	{
	    if ( devise_cachee == -1 )
		menu_item = gtk_menu_item_new_with_label ( devise_code ( devise ) );
	    else
		menu_item = gtk_menu_item_new_with_label ( g_strconcat ( devise -> nom_devise,
									 " ( ",
									 devise_code ( devise ),
									 " )",
									 NULL ));

	    g_object_set_data ( G_OBJECT ( menu_item ),
				"adr_devise",
				devise );
	    g_object_set_data ( G_OBJECT ( menu_item ),
				"no_devise",
				GINT_TO_POINTER ( devise -> no_devise ) );


	    gtk_menu_append ( GTK_MENU ( menu ),
			      menu_item );

	    gtk_widget_show ( menu_item );
	}
	liste_tmp = liste_tmp -> next;
    }

    gtk_widget_show ( menu );

    return ( menu );

}



/**
 * TODO: document this
 *
 */
void fill_currency_list ( GtkTreeView * view, gboolean include_obsolete )
{
    GtkTreeModel * model;
    GtkTreeIter child_iter;
    struct iso_4217_currency * currency = iso_4217_currencies;

    model = gtk_tree_view_get_model ( view );

    while (currency -> country_name )
    {
	if ( include_obsolete || currency -> active )
	{
	    GdkPixbuf * pixbuf;
	    
	    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, 
							     C_DIRECTORY_SEPARATOR,
							     "flags", 
							     C_DIRECTORY_SEPARATOR,
							     currency -> flag_filename, 
							     NULL ),
						NULL );	
	    
	    gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, NULL);
	    gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
				CURRENCY_FLAG_COLUMN, pixbuf,
				COUNTRY_NAME_COLUMN, g_strconcat ( " ", 
								   _(currency -> country_name),
								   NULL ),
				CURRENCY_NAME_COLUMN, _(currency -> currency_name),
				CURRENCY_ISO_CODE_COLUMN, _(currency -> currency_code),
				CURRENCY_NICKNAME_COLUMN, _(currency -> currency_nickname),
				CURRENCY_POINTER_COLUMN, currency,
				CURRENCY_HAS_FLAG, TRUE,
				-1);
	}
	currency++;
    }

    gtk_tree_view_expand_all ( view );
}



/**
 * TODO: document this
 *
 */
gboolean rebuild_currency_list ( GtkWidget * checkbox, GtkTreeView * view )
{
    GtkTreeModel * model;

    model = gtk_tree_view_get_model ( view );
    gtk_tree_store_clear ( GTK_TREE_STORE (model) );
    fill_currency_list ( view, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) );
    return FALSE;
}



/**
 *
 *
 */
GtkWidget * new_currency_tree ()
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeStore * model;
    GtkWidget * treeview;
    gint col_offset;

    /* Create tree store */
    model = gtk_tree_store_new (NUM_CURRENCIES_COLUMNS,
				GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_POINTER );

    /* Create tree view */
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

    /* Flag */
    cell = gtk_cell_renderer_pixbuf_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Country name"),
						     cell, "pixbuf",
						     CURRENCY_FLAG_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), cell,
				       "visible", CURRENCY_HAS_FLAG);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Country name */
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), cell, TRUE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), cell, 
				       "text", COUNTRY_NAME_COLUMN);

    /* Currency name */
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    cell = gtk_cell_renderer_text_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Currency name"),
						     cell, "text",
						     CURRENCY_NAME_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    cell = gtk_cell_renderer_text_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("ISO Code"), 
						     cell, "text",
						     CURRENCY_ISO_CODE_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    cell = gtk_cell_renderer_text_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Sign"), 
						     cell, "text",
						     CURRENCY_NICKNAME_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), 
					  COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

    return treeview;
}


/**
 * TODO: document this
 *
 */
GtkWidget * new_currency_vbox ()
{ 
    GtkWidget * sw, * treeview, * vbox, * checkbox;
    GtkTreeModel * model;

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_ALWAYS);

    treeview = new_currency_tree ();
    gtk_widget_set_usize ( treeview, FALSE, 200 );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW(treeview) );
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW ( treeview ) ), 
		       "changed", G_CALLBACK ( select_currency_in_iso_list ), 
		       model );

    gtk_container_add (GTK_CONTAINER (sw), treeview);
    gtk_container_set_resize_mode (GTK_CONTAINER (sw), GTK_RESIZE_PARENT);

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(vbox), sw,
			 TRUE, TRUE, 0 );

    checkbox = gtk_check_button_new_with_label ( _("Include obsolete currencies"));
    gtk_box_pack_start ( GTK_BOX(vbox), checkbox,
			 FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT(checkbox), "toggled",
		       (GCallback) rebuild_currency_list, treeview );

    fill_currency_list ( GTK_TREE_VIEW(treeview), FALSE );

    g_object_set_data ( G_OBJECT(vbox), "model", model );
    g_object_set_data ( G_OBJECT(vbox), "treeview", treeview );

    return vbox;
}



/**
 * Pop up a dialog to create a new currency, do some sanity checks and
 * call the create_currency() function to do the grunt work.
 * 
 * \param widget	GtkButton that triggered event.
 * 
 * \return TRUE if currency has been created.
 */
gboolean ajout_devise ( GtkWidget *widget )
{
    GtkWidget *dialog, *label, *table, *model, *list, *paddingbox;
    gchar *nom_devise, *code_devise, *code_iso4217_devise;
    struct struct_devise *devise;
    gint resultat;

    dialog = gtk_dialog_new_with_buttons ( _("Add a currency"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL,0,
					   GTK_STOCK_OK,1,
					   NULL );

    gtk_container_set_border_width ( GTK_CONTAINER ( dialog ), 12 );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   TRUE, _("ISO 4217 currencies"));

    /* Create list */
    list = new_currency_vbox ();
    model = g_object_get_data ( G_OBJECT(list), "model" );

    gtk_box_pack_start ( GTK_BOX(paddingbox) , list, TRUE, TRUE, 5 );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   FALSE, _("Currency details"));

    /* Create table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Currency name */
    label = gtk_label_new (COLON(_("Currency name")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_nom = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree_nom ), TRUE );
    gtk_table_attach ( GTK_TABLE ( table ), entree_nom, 1, 2, 0, 1,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_name", entree_nom );

    /* Currency ISO code */
    label = gtk_label_new (COLON(_("Currency ISO 4217 code")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_iso_code = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entree_iso_code, 1, 2, 1, 2,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_iso_code", entree_iso_code );

    /* Currency usual sign */
    label = gtk_label_new (COLON(_("Currency sign")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_code = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entree_code, 1, 2, 2, 3,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_code", entree_code );

    /* Select default currency. */
    gtk_tree_model_foreach ( GTK_TREE_MODEL(model), 
			     (GtkTreeModelForeachFunc) gsb_gui_select_default_currency, 
			     g_object_get_data ( G_OBJECT(list), "treeview" ) );

  reprise_dialog:
    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case 1 :

	    nom_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom ))));
	    code_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_code ))));
	    code_iso4217_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code ))));

	    if ( strlen ( nom_devise ) && 
		 (strlen ( code_devise ) ||
		  strlen ( code_iso4217_devise )))
	    {
		/* 		recherche si la devise existe dÃ©jÃ , si c'est le cas, on refuse */

		if ( devise_par_nom ( nom_devise )
		     ||
		     devise_par_code_iso ( code_iso4217_devise ))
		{
		    dialogue_error_hint ( _("Currency names or iso 4217 codes should be unique.  Please choose a new name for the currency."),
					  g_strdup_printf ( _("Currency '%s' already exists." ), nom_devise ));
		    goto reprise_dialog;
		}

		devise = create_currency ( nom_devise, code_devise, code_iso4217_devise);

		if ( widget )
		{
		    append_currency_to_currency_list ( GTK_TREE_STORE ( currency_list_model ),
						       devise );
		    update_currency_widgets();
		    modification_fichier ( TRUE );
		    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
		    return TRUE;
		}
	    }
	    else
	    {
		dialogue_warning_hint ( _("Currency name and either currency ISO4217 code or currency nickname should be set."),
					_("All fields are not filled in") );
		goto reprise_dialog;
	    }
	    break;

	default:
	    /* 	    si on veut partir, mais qu'aucune devise n'existe, on met un message et on retourne au */
	    /* 		choix de la devise */

	    if ( !liste_struct_devises )
	    {
		gtk_widget_destroy ( GTK_WIDGET ( dialog ));
		return FALSE;
	    }
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
    return TRUE;
}



/**
 * Remove selected currency from currency list.  First, be sure there
 * is no use for it.
 *
 * \param bouton	Widget that triggered event.
 * \param view		GtkTreeView that contains selected currency.
 */
void retrait_devise ( GtkWidget *bouton, GtkWidget * view )
{
    gint devise_trouvee = 0;
    struct struct_devise *devise;
    GSList *list_tmp_transactions;

    devise = currency_get_selected ( view );
    if ( ! devise )
    {
	return;
    }

    /* we look for that currency in all the transactions and scheduler,
     * if we find it, we cannot delete it */

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number;
	transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_currency_number (transaction_number) == devise -> no_devise )
	{
	    devise_trouvee = 1;
	    list_tmp_transactions = NULL;
	}
	else
	    list_tmp_transactions = list_tmp_transactions -> next;
    }

    if ( !devise_trouvee )
    {
	GSList *list_tmp;

	list_tmp = gsb_data_scheduled_get_scheduled_list ();

	while ( list_tmp )
	{
	    gint scheduled_number;

	    scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp -> data);
	    if ( gsb_data_scheduled_get_currency_number (scheduled_number) == devise -> no_devise )
	    {
		devise_trouvee = 1;
		list_tmp = NULL;
	    }
	    else
		list_tmp = list_tmp -> next;
	}
    }

    if ( devise_trouvee )
    {
	dialogue_error_hint ( g_strdup_printf ( _("Currency '%s' is used in current account.  Grisbi can't delete it."),
						devise -> nom_devise ),
			      g_strdup_printf ( _("Impossible to remove currency '%s'"), 
						devise -> nom_devise) );
	return;
    }

    remove_selected_currency_from_currency_view ( view );
    liste_struct_devises = g_slist_remove ( liste_struct_devises, devise );
    nb_devises--;
}



/***********************************************************************************************************/
/* Fonction demande_taux_de_change : */
/* affiche une fenetre permettant d'entrer le taux de change entre la devise du compte et la devise demandÃÂ©e */
/* renvoie ce taux de change */
/* le taux renvoyÃÂ© est <0 si une_devise_compte_egale_x_devise_ope = 1, > 0 sinon */
/***********************************************************************************************************/

void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise ,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change,
			      gboolean force )
{
    GtkWidget *dialog, *label, *entree, *menu, *item, *hbox, *entree_frais, *paddingbox;
    struct cached_exchange_rate * cache;
    gint resultat;

    if ( !force && 
	 (cache = cached_exchange_rate ( devise_compte, devise )) )
    {
	taux_de_change[0] = cache -> rate;
	taux_de_change[1] = cache -> fees;
	return;
    }

    dialog = gtk_dialog_new_with_buttons ( _("Enter exchange rate"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK, 0,
					   NULL );
    gtk_signal_connect ( GTK_OBJECT ( dialog), "delete_event",
			 GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
			 NULL );

    paddingbox = new_paddingbox_with_title ( GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ), FALSE, 
					     g_strdup_printf( _("Please enter exchange rate for %s"), 
							      devise->nom_devise) );
    gtk_container_set_border_width ( GTK_CONTAINER ( paddingbox ), 6 );
    gtk_box_set_spacing ( GTK_BOX ( GTK_DIALOG (dialog)->vbox ), 6 );
    

    /* crÃÂ©ation de la ligne du change */

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( POSTSPACIFY(_("A")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    option_menu_devise_1= gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), option_menu_devise_1,
			 TRUE, TRUE, 0);

    label = gtk_label_new ( SPACIFY(_("equals")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    entree = gtk_entry_new ();
    gtk_widget_set_usize ( entree, 100, FALSE );
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree ),
				      TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), entree,
			 TRUE, TRUE, 0);

    option_menu_devise_2 = gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), option_menu_devise_2,
			 TRUE, TRUE, 0);

    /* crÃÂ©ation du menu de la 1Ã,CE¡(Bre devise ( le menu comporte la devise
       courante et celle associÃÂ©e ) */
    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label ( devise -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise_compte );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_1 ), menu );
    g_signal_connect ( G_OBJECT ( option_menu_devise_1 ),
		       "changed",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       NULL );
    g_object_set_data ( G_OBJECT (option_menu_devise_1), "associate",
			option_menu_devise_2 );
    g_object_set_data ( G_OBJECT (option_menu_devise_1), "currency", devise );

    /* crÃÂ©ation du menu de la 2Ã,CE¡(Bme devise ( le menu comporte la devise
       courante et celle associÃÂ©e ) */
    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label ( devise -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise_compte );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_2 ), menu );
    g_signal_connect ( G_OBJECT ( option_menu_devise_2 ),
		       "changed",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       GINT_TO_POINTER ( 1 ));
    g_object_set_data ( G_OBJECT (option_menu_devise_2), "associate",
			option_menu_devise_1 );
    g_object_set_data ( G_OBJECT (option_menu_devise_2), "currency", devise );

    /* crÃÂ©ation de la ligne des frais de change */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 5 );

    label = gtk_label_new ( COLON(_("Exchange fees")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 5 );

    entree_frais = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree_frais ),
				      TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), entree_frais,
			 TRUE, TRUE, 5 );

    label = gtk_label_new ( devise_compte -> nom_devise );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 5 );

    gtk_widget_show_all ( dialog );

    /* choix des 1Ã,CE¡(Bre et 2Ã,CE¡(Bme devise */
    if ( taux_change || frais_change )
    {

	if ( une_devise_compte_egale_x_devise_ope )
	{
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					  1 );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					  0 );
	}
	else
	{
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					  0 );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					  1 );
	}
	gtk_entry_set_text ( GTK_ENTRY ( entree ),
			     g_strdup_printf ( "%f", taux_change ));
	gtk_entry_set_text ( GTK_ENTRY ( entree_frais ),
			     g_strdup_printf ( "%4.2f", fabs ( frais_change )));

    }
    else
    {
	/* vÃÂ©rifie s'il y a dÃÂ©jÃÂ  une association entre la devise du
	   compte et la devise de l'opÃÂ©ration */
	if ( devise_compte -> no_devise_en_rapport == devise -> no_devise )
	{
	    /* il y a une association de la devise du compte vers la
	       devise de l'opÃÂ©ration */
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					  devise_compte -> une_devise_1_egale_x_devise_2 );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					  !( devise_compte -> une_devise_1_egale_x_devise_2 ));

	    /* si un change est dÃÂ©jÃÂ  entrÃÂ©, on l'affiche */
	    if ( devise_compte -> date_dernier_change )
		gtk_entry_set_text ( GTK_ENTRY ( entree ),
				     g_strdup_printf ( "%f",
						       devise_compte -> change ));
	}
	else
	    if ( devise -> no_devise_en_rapport == devise_compte -> no_devise )
	    {
		/* il y a une association de la devise de l'opÃÂ©ration
		   vers la devise du compte */
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					      !(devise -> une_devise_1_egale_x_devise_2 ));
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					      devise -> une_devise_1_egale_x_devise_2 );
		/* si un change est dÃÂ©jÃÂ  entrÃÂ©, on l'affiche */
		if ( devise -> date_dernier_change )
		    gtk_entry_set_text ( GTK_ENTRY ( entree ),
					 g_strdup_printf ( "%f",
							   devise -> change ));
	    }
	    else
	    {
		/* il n'y a aucun rapport ÃÂ©tabli entre les 2 devises */
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					      1 );
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					      0 );
	    }
    }

    /* on lance la fenetre */
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( !resultat )
    {
	struct struct_devise *devise_tmp;

	taux_de_change[0] = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )),
					NULL );
	taux_de_change[1] = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_frais )),
					NULL );

	/* FIXME: use gtk_menu_get_active + gtk_option_menu_get_menu ? */
	devise_tmp = g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_1 ) -> menu_item ),
					 "adr_devise" );


	if ( devise_tmp -> no_devise != devise -> no_devise )
	    taux_de_change[0] = -taux_de_change[0];

	update_exchange_rate_cache ( devise_compte, devise, 
				     taux_de_change[0], taux_de_change[1] );

    }
    else
    {
	taux_de_change[0] = 0;
	taux_de_change[1] = 0;
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
}
/***********************************************************************************************************/





/* 
 * Handler that change the second option menu of a window that ask for
 * change.
 *
 * \param menu_shell The menu that triggered this handler.
 * \param origine Position of selected item in menu
 */
gboolean devise_selectionnee ( GtkWidget *menu_shell, gint origine )
{
    GtkWidget * associate;
    gint position;
    struct struct_devise *devise;

    devise = g_object_get_data( G_OBJECT(menu_shell), "currency");

    if ( devise )
    {
	position = gtk_option_menu_get_history ( GTK_OPTION_MENU(menu_shell) );
	devise -> une_devise_1_egale_x_devise_2 = (position != origine);
	associate = g_object_get_data ( G_OBJECT(menu_shell), "associate" );
	g_signal_handlers_block_by_func ( G_OBJECT(associate),
					  G_CALLBACK (devise_selectionnee), 
					  (gpointer) !origine );
	if (associate)
	    gtk_option_menu_set_history ( GTK_OPTION_MENU(associate), 1 - position );
	g_signal_handlers_unblock_by_func ( G_OBJECT(associate),
					    G_CALLBACK (devise_selectionnee), 
					    (gpointer) !origine );
    }

    return FALSE;
}



/**
 * Creates the currency list and associated form to configure them.
 *
 * \returns A newly created vbox
 */
GtkWidget *onglet_devises ( void )
{
    GtkWidget *vbox_pref, *label, *paddingbox, *bouton, *hbox;
    GtkWidget *scrolled_window, *vbox, *table;
    GSList *liste_tmp;
    
    vbox_pref = new_vbox_with_title_and_icon ( _("Currencies"), "currencies.png" ); 
    paddingbox = new_paddingbox_with_title (vbox_pref, TRUE, _("Known currencies"));
    
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, TRUE, TRUE, 0);

    /* Currency list */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    /* Create it. */
    currency_list_view = GTK_TREE_VIEW ( new_currency_tree () );
    currency_list_model = gtk_tree_view_get_model ( currency_list_view );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), GTK_WIDGET(currency_list_view) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window, TRUE, TRUE, 0);
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW ( currency_list_view ) ), 
		       "changed", G_CALLBACK ( select_currency_in_iso_list ), 
		       currency_list_model );

    /*   s'il n'y a pas de fichier ouvert, on grise */
    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    else
    {
	/* remplissage de la liste avec les devises temporaires */

	liste_tmp = liste_struct_devises;

	while ( liste_tmp )
	{
	    struct struct_devise *devise;

	    devise = liste_tmp -> data;
	    append_currency_to_currency_list ( GTK_TREE_STORE ( currency_list_model ),
					       devise );
	    liste_tmp = liste_tmp -> next;
	}

    }

    /* Create Add/Remove buttons */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, FALSE, FALSE, 0 );

    /* Button "Add" */
    bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ), "clicked",
				GTK_SIGNAL_FUNC  ( ajout_devise ),
				currency_list_model );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton, FALSE, FALSE, 5 );

    /* Button "Remove" */
    bouton_supprimer_devise = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    g_object_set_data ( G_OBJECT(bouton_supprimer_devise), "view", currency_list_view );
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_devise ), "clicked",
			 GTK_SIGNAL_FUNC  ( retrait_devise ), currency_list_view );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_devise, FALSE, FALSE, 5 );

    /* Input form for currencies */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Currency properties"));

    /* Create table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Create currency name entry */
    label = gtk_label_new (COLON(_("Name")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_nom_devise_parametres = new_text_entry ( NULL, (GCallback) changement_nom_entree_devise, NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entree_nom_devise_parametres, 1, 2, 0, 1, 
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_list_model), "entry_name", entree_nom_devise_parametres );
    g_object_set_data ( G_OBJECT(entree_nom_devise_parametres), "view", currency_list_view );

    /* Create code entry */
    label = gtk_label_new (COLON(_("Sign")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_code_devise_parametres = new_text_entry ( NULL, (GCallback) changement_code_entree_devise, NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entree_code_devise_parametres, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_list_model), "entry_code", entree_code_devise_parametres );
    g_object_set_data ( G_OBJECT(entree_code_devise_parametres), "view", currency_list_view );

    /* Create code entry */
    label = gtk_label_new ( COLON(_("ISO code")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_iso_code_devise_parametres = new_text_entry ( NULL, (GCallback) changement_iso_code_entree_devise, 
							 NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entree_iso_code_devise_parametres, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_list_model), "entry_iso_code", entree_iso_code_devise_parametres );
    g_object_set_data ( G_OBJECT(entree_iso_code_devise_parametres), "view", currency_list_view );

    return ( vbox_pref );
}



/**
 *
 *
 *
 */
void append_currency_to_currency_list ( GtkTreeStore * model, struct struct_devise * devise )
{
    GdkPixbuf * pixbuf;
    GtkTreeIter iter;

    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "flags", C_DIRECTORY_SEPARATOR,
						     devise -> code_iso4217_devise,
						     ".png", NULL ),
					NULL );	

    gtk_tree_store_append (GTK_TREE_STORE(model), &iter, NULL);
    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
			 CURRENCY_FLAG_COLUMN, pixbuf,
			 COUNTRY_NAME_COLUMN, "",
			 CURRENCY_NAME_COLUMN, devise -> nom_devise,
			 CURRENCY_ISO_CODE_COLUMN, devise -> code_iso4217_devise,
			 CURRENCY_NICKNAME_COLUMN, devise -> code_devise,
			 CURRENCY_POINTER_COLUMN, devise,
			 CURRENCY_HAS_FLAG, TRUE,
			 -1);
}



/**
 * Create a new GtkOptionMenu with a pointer to an integerthat will be
 * modified according to the entry's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler
 *
 * \return A newly allocated option menu.
 */
GtkWidget * new_currency_option_menu ( gint * value, GCallback hook )
{
    GtkWidget * currency_list, *currency_menu;

    currency_list = gtk_option_menu_new ();
    currency_menu = creation_option_menu_devises ( 0, liste_struct_devises );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( currency_list ), currency_menu );
   if (value && *value)
	gtk_option_menu_set_history ( GTK_OPTION_MENU(currency_list),
				      gsb_currency_find_currency_in_option_menu ( currency_list,
										  *value ));

    g_signal_connect ( GTK_OBJECT (currency_list), "changed", (GCallback) set_int_from_menu, value );
    g_signal_connect ( GTK_OBJECT (currency_list), "changed", (GCallback) hook, value );
    g_object_set_data ( G_OBJECT ( currency_list ), "pointer", value);

    return currency_list;
}



/** look for a specific currency in an option menu and return its place in
 * the menu, used to set the option menu on that currency with gtk_option_menu_set_history
 * \param option_menu
 * \param no_currency
 * \return the place of the currency in the menu
 * */
gint gsb_currency_find_currency_in_option_menu ( GtkWidget *option_menu,
						 gint no_currency )
{
    GList *children;
    gint pos = 0;

    children = GTK_MENU_SHELL ( gtk_option_menu_get_menu ( GTK_OPTION_MENU ( option_menu))) -> children;
    
    while ( children )
    {
	if ( GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( children -> data ),
						   "no_devise" )) == no_currency )
	    return pos;
	pos++;

	children = children -> next;
    }

    return 0;
}



/**
 *
 *
 */
GtkWidget *tab_display_totals ( void )
{
    GtkWidget *vbox_pref, *currency_list, *table, *label;

    vbox_pref = new_vbox_with_title_and_icon ( _("Totals currencies"),
					       "currencies.png" );

    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );

    label = gtk_label_new (_("Currency for payees tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 
		       0, 1, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    currency_list = new_currency_option_menu ( &no_devise_totaux_tiers, 
					       remplit_arbre_tiers );
    gtk_table_attach ( GTK_TABLE ( table ), currency_list,
		       1, 2, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    label = gtk_label_new (_("Currency for categories tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    currency_list = new_currency_option_menu ( &no_devise_totaux_categ, 
					       remplit_arbre_categ );
    gtk_table_attach ( GTK_TABLE ( table ), currency_list,
		       1, 2, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    label = gtk_label_new (_("Currency for budgetary lines tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 2, 3, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    currency_list = new_currency_option_menu ( &no_devise_totaux_ib, 
					       remplit_arbre_imputation );
    gtk_table_attach ( GTK_TABLE ( table ), currency_list,
		       1, 2, 2, 3, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), table, TRUE, TRUE, 0);

    return ( vbox_pref );
}



/**
 * Obtain selected currency from currency tree.
 *
 * \param view		GtkTreeView to remove currency from.
 * 
 * \return		A pointer to selected currency.
 */
struct struct_devise * currency_get_selected ( GtkTreeView * view )
{
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( view );
    GtkTreeIter iter;
    GtkTreeModel * tree_model;
    struct struct_devise * currency; 

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gtk_tree_model_get ( tree_model, &iter, 
			 CURRENCY_POINTER_COLUMN, &currency,
			 -1 );

    return currency;
}



/**
 * Remove selected currency from tree.  In fact, this is a generic
 * function that could be used for any purpose (and could be then
 * renamed).
 *
 * \param view	GtkTreeView to remove selected entry from.
 */
void remove_selected_currency_from_currency_view ( GtkTreeView * view )
{
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( view );
    GtkTreeIter iter;
    GtkTreeModel * tree_model;
    struct struct_devise * currency; 

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gtk_tree_store_remove ( tree_model, &iter );
}



/**
 *
 *
 */
gboolean changement_nom_entree_devise ( GtkEditable *editable, gchar * text,
					gint length, gpointer data )
{
    struct struct_devise *devise;
    GtkTreeSelection * selection;
    GtkTreeModel * tree_model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "view" ) );
    devise = currency_get_selected ( g_object_get_data ( G_OBJECT (editable), "view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    devise -> nom_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_devise_parametres ))));

    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ), &iter,
			 CURRENCY_NAME_COLUMN, devise -> nom_devise,
			 -1);

    return FALSE;
}



gboolean changement_code_entree_devise ( GtkEditable *editable, gchar * text,
					 gint length, gpointer data )
{
    struct struct_devise *devise;
    GtkTreeModel * tree_model;
    GtkTreeSelection * selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "view" ) );
    devise = currency_get_selected ( g_object_get_data ( G_OBJECT (editable), "view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    devise -> code_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_code_devise_parametres ))));

    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ), &iter,
			 CURRENCY_NICKNAME_COLUMN, devise -> code_devise,
			 -1);

    return FALSE;
}



gboolean changement_iso_code_entree_devise ( GtkEditable *editable, gchar * text,
					     gint length, gpointer data )
{
    struct struct_devise *devise;
    GtkTreeModel * tree_model;
    GtkTreeSelection * selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "view" ) );
    devise = currency_get_selected ( g_object_get_data ( G_OBJECT (editable), "view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    devise -> code_iso4217_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code_devise_parametres ))));


    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ), &iter,
			 CURRENCY_ISO_CODE_COLUMN, devise -> code_iso4217_devise,
			 -1);

    return FALSE;
}
/* **************************************************************************************************************************** */




/**
 * Find whether echange rate between two currencies is known.  If so,
 * returns a cached_exchange_rate structure with exchange rate
 * information.
 *
 * \param currency1 First currency
 * \param currency2 Second currency
 *
 * \return FALSE on failure, a pointer to a cached_exchange_rate
 * structure on success.
 */
struct cached_exchange_rate *cached_exchange_rate ( struct struct_devise * currency1, 
						    struct struct_devise * currency2 )
{
    GSList * liste_tmp = cached_exchange_rates;
    struct cached_exchange_rate * tmp;

    while ( liste_tmp )
    {
	tmp = liste_tmp -> data;
	if ( currency1 == tmp -> currency1 && currency2 == tmp -> currency2 )
	    return tmp;

	liste_tmp = liste_tmp -> next;
    }

    return NULL;
}


/**
 * Update exchange rate cache according to arguments.
 *
 * \param currency1 First currency.
 * \param currency2 Second currency.
 * \param change    Exchange rate between two currencies.
 * \param fees      Fees of transaction.
 */
void update_exchange_rate_cache ( struct struct_devise * currency1, 
				  struct struct_devise * currency2,
				  gdouble change, gdouble fees )
{
    struct cached_exchange_rate * tmp;

    tmp = (struct cached_exchange_rate *) g_malloc(sizeof(struct cached_exchange_rate));

    tmp -> currency1 = currency1;
    tmp -> currency2 = currency2;
    tmp -> rate = change;
    tmp -> fees = fees;

    cached_exchange_rates = g_slist_append ( cached_exchange_rates, tmp );
}
/* ***************************************************************************************** */






/* ***************************************************************************************** */
struct struct_devise *create_currency ( gchar * nom_devise, gchar * code_devise, 
					gchar * code_iso4217_devise )
{
  struct struct_devise * devise;
  
  devise = g_malloc ( sizeof ( struct struct_devise ));
  devise -> nom_devise = nom_devise;
  devise -> code_devise = code_devise;
  devise -> code_iso4217_devise = code_iso4217_devise;
  devise -> passage_euro = 0;
  devise -> no_devise_en_rapport = 0;
  devise -> date_dernier_change = NULL;
  devise -> une_devise_1_egale_x_devise_2 = 0;

  devise -> change = 0;

  devise -> no_devise = ++no_derniere_devise;
  liste_struct_devises = g_slist_append ( liste_struct_devises, devise );
  nb_devises++;

  return devise;
}



/**
 *
 *
 *
 */
struct struct_devise * find_currency_from_iso4217_list ( gchar * currency_name )
{
  struct iso_4217_currency * currency = iso_4217_currencies;

  while ( currency -> country_name )
    {
      if ( !strcmp ( currency -> currency_code, currency_name ) )
	return create_currency ( currency -> currency_name, 
				 currency -> currency_nickname, 
				 currency -> currency_code );
      currency++;
    }

  return NULL;
}



/** 
 * Get and return the number of the currency in the option_menu given
 * in param
 * 
 * \param currency_option_menu an option menu with the currencies
 * 
 * \return the number of currency
 * */
gint gsb_currency_get_option_menu_currency ( GtkWidget *currency_option_menu )
{
    struct struct_devise *currency;

    if ( !currency_option_menu )
	return 0;

    currency = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU (currency_option_menu) -> menu_item ),
				     "adr_devise" );

    if ( currency )
	return currency -> no_devise;

    return 0;
}



/** 
 * Check if a transaction need an exchange rate and fees with its
 * account if yes, ask for that and set the in the transaction.
 * 
 * \param no_transaction
 */
void gsb_currency_check_for_change ( gint no_transaction )
{
    struct struct_devise *transaction_currency;
    struct struct_devise *account_currency;

    account_currency = devise_par_no ( gsb_data_account_get_currency (gsb_data_transaction_get_account_number (no_transaction)));
    transaction_currency = devise_par_no ( gsb_data_transaction_get_currency_number (no_transaction));

    /* si c'est la devise du compte ou */
    /* si c'est un compte qui doit passer Ã  l'euro ( la transfo se fait au niveau de l'affichage de la liste ) */
    /* ou si c'est un compte en euro et l'opÃ© est dans une devise qui doit passer Ã  l'euro -> pas de change Ã  demander */

    if ( !( transaction_currency -> no_devise == account_currency -> no_devise
	    ||
	    ( account_currency -> passage_euro
	      &&
	      !strcmp ( transaction_currency -> nom_devise, _("Euro") ))
	    ||
	    ( !strcmp ( account_currency -> nom_devise, _("Euro") )
	      &&
	      transaction_currency -> passage_euro )))
    {
	/* it's a foreign currency, ask for the exchange rate and fees */

	/* FIXME : utilise et remplit les variables globales taux_de_change, voir pour passer Ã§a en local */

	demande_taux_de_change ( account_currency,
				 transaction_currency,
				 1,
				 ( gdouble ) 0,
				 ( gdouble ) 0,
				 FALSE );

	gsb_data_transaction_set_exchange_rate ( no_transaction,
						 fabs (taux_de_change[0] ));
	gsb_data_transaction_set_exchange_fees ( no_transaction,
						 taux_de_change[1] );

	if ( taux_de_change[0] < 0 )
	    gsb_data_transaction_set_change_between ( no_transaction,
						      1 );
    }
}



/**
 *
 *
 */
gboolean gsb_gui_select_default_currency ( GtkTreeModel * tree_model, GtkTreePath * path, 
					   GtkTreeIter * iter, GtkTreeView * view )
{
    struct lconv * conv = localeconv();
    gchar * code, * symbol, * country;
    gboolean good = FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 CURRENCY_ISO_CODE_COLUMN, &code, 
			 COUNTRY_NAME_COLUMN, &country, 
			 -1 );
    if ( conv && conv -> int_curr_symbol && strlen ( conv -> int_curr_symbol ) )
    {
	symbol = g_strstrip ( g_strdup ( conv -> int_curr_symbol ));
	if ( ! strcmp ( code, symbol ) )
	{
	    good = TRUE;
	}
	free ( symbol );
    }
    else
    {
	symbol = g_strstrip ( g_strdup ( country ) );
	if ( ! strcmp ( symbol, _("United States") ) )
	{
	    good = TRUE;
	}
	free ( symbol );
    }

    if ( good )
    {
	gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( view ), path );
	gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (view), path, NULL, TRUE, 0.5, 0 );
	return TRUE;
    }
    
    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
