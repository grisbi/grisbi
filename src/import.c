/* ************************************************************************** */
/* ce fichier de la gestion de l'import de fichiers (qif, ofx, csv, gnucash)  */
/*                                                                            */
/*                                  accueil.c                                 */
/*                                                                            */
/*     Copyright (C)	2000-2004 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "import.h"
#include "utils.h"
#include "utils_montants.h"
#include "comptes_gestion.h"
#include "comptes_traitements.h"
#include "import_csv.h"
#include "gsb_transactions_list.h"
#include "dialog.h"
#include "utils_files.h"
#include "go-charmap-sel.h"
#include "gsb_assistant.h"
#include "gsb_currency_config.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "operations_comptes.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "navigation.h"
#include "menu.h"
#include "gsb_status.h"
#include "fichiers_gestion.h"
#include "traitement_variables.h"
#include "accueil.h"
#include "categories_onglet.h"
#include "tiers_onglet.h"
#include "utils_str.h"
#include "utils_operations.h"
#include "gnucash.h"
#include "ofx.h"
#include "qif.h"
#include "utils_comptes.h"
#include "imputation_budgetaire.h"
#include "structures.h"
#include "gsb_file_config.h"
#include "import.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static enum import_type autodetect_file_type ( gchar * filename, FILE * fichier, 
					gchar * pointeur_char );
static gboolean changement_valeur_echelle_recherche_date_import ( GtkWidget *spin_button );
static gboolean click_dialog_ope_orphelines ( GtkWidget *dialog,
				       gint result,
				       GtkWidget *liste_ope_celibataires );
static gboolean click_sur_liste_opes_orphelines ( GtkCellRendererToggle *renderer, 
					   gchar *ligne,
					   GtkTreeModel *store );
static void confirmation_enregistrement_ope_import ( struct struct_compte_importation *imported_account );
static void cree_liens_virements_ope_import ( void );
static GtkWidget * cree_ligne_recapitulatif ( struct struct_compte_importation * compte );
static void gsb_import_add_imported_transactions ( struct struct_compte_importation *imported_account,
					    gint account_number );
static gint gsb_import_create_imported_account ( struct struct_compte_importation *imported_account );
static gint gsb_import_create_transaction ( struct struct_ope_importation *imported_transaction,
				     gint account_number );
static gboolean import_account_action_activated ( GtkWidget * radio, gint action );
static gboolean import_active_toggled ( GtkCellRendererToggle * cell, gchar *path_str,
				 gpointer model );
static GtkWidget * import_create_file_selection_page ( GtkWidget * assistant );
static GtkWidget * import_create_final_page ( GtkWidget * assistant );
static GtkWidget * import_create_resume_page ( GtkWidget * assistant );
static gboolean import_enter_file_selection_page ( GtkWidget * assistant );
static gboolean import_enter_resume_page ( GtkWidget * assistant );
static void import_preview_maybe_sensitive_next ( GtkWidget * assistant, GtkTreeModel * model ) ;
static gboolean import_select_file ( GtkWidget * button, GtkWidget * assistant );
static gboolean import_switch_type ( GtkCellRendererText *cell, const gchar *path,
			      const gchar *value, GtkListStore * model );
static void pointe_opes_importees ( struct struct_compte_importation *imported_account );
static void traitement_operations_importees ( void );
static gchar * type_string_representation ( enum import_type type );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *formulaire;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget *preview;
extern GtkWidget *tree_view;
extern GtkWidget *tree_view_vbox;
extern GtkWidget *window;
/*END_EXTERN*/



/** used to keep the number of the mother transaction while importing breakdown transactions */
static gint mother_transaction_number;
gint valeur_echelle_recherche_date_import;
GSList *liste_comptes_importes;
GSList *liste_comptes_importes_error;
GtkWidget *dialog_recapitulatif;
GtkWidget *table_recapitulatif;
gint virements_a_chercher;

enum import_type file_type;
enum import_filesel_columns { 
    IMPORT_FILESEL_SELECTED = 0,
    IMPORT_FILESEL_TYPENAME,
    IMPORT_FILESEL_FILENAME,
    IMPORT_FILESEL_REALNAME,
    IMPORT_FILESEL_TYPE,
    IMPORT_FILESEL_CODING,
    IMPORT_FILESEL_NUM_COLS,
};

/** Page numbering for the import wizard. */
enum import_pages {
    IMPORT_STARTUP_PAGE,
    IMPORT_FILESEL_PAGE,
    IMPORT_CSV_PAGE,
    IMPORT_RESUME_PAGE,
    IMPORT_FIRST_ACCOUNT_PAGE,
};



/* *******************************************************************************/
/* fonction importer_fichier */
/* appelée par le menu importer */
/* *******************************************************************************/

void importer_fichier ( void )
{
    GtkWidget * a;

    liste_comptes_importes = NULL;
    liste_comptes_importes_error = NULL;
    virements_a_chercher = 0;


    a = gsb_assistant_new ( "Importing transactions into Grisbi",
			    "This assistant will help you import one or several files into Grisbi."
			    "\n\n"
			    "Grisbi will try to do its best to guess which format are imported, but you may have to manually set them in the list of next page.  "
			    "So far, the following formats are supported:"
			    "\n\n"
			    ""
			    "	• Quicken Interchange format (QIF)\n"
			    "	• Open Financial Exchange Format (OFX)\n"
			    "	• Gnucash format\n"
			    "	• Comma separated-values format (CSV/TSV)",
			    "csv.png" );

    gsb_assistant_add_page ( a, import_create_file_selection_page ( a ), 
			     IMPORT_FILESEL_PAGE, IMPORT_STARTUP_PAGE, IMPORT_RESUME_PAGE, 
			     G_CALLBACK ( import_enter_file_selection_page ) );
    gsb_assistant_add_page ( a, import_create_csv_preview_page ( a ), 
			     IMPORT_CSV_PAGE, IMPORT_FILESEL_PAGE, IMPORT_RESUME_PAGE,
			     G_CALLBACK ( import_enter_csv_preview_page ) );
    gsb_assistant_add_page ( a, import_create_resume_page ( a ), 
			     IMPORT_RESUME_PAGE, IMPORT_FILESEL_PAGE, 
			     IMPORT_FIRST_ACCOUNT_PAGE, 
			     G_CALLBACK ( import_enter_resume_page ) );

    if ( gsb_assistant_run ( a ) == GTK_RESPONSE_APPLY )
    {
	traitement_operations_importees ();
	remplit_arbre_categ ();
	remplit_arbre_imputation ();
	remplit_arbre_tiers ();
    }

    gtk_widget_destroy ( a );
}



/**
 *
 *
 *
 */
GtkWidget * import_create_file_selection_page ( GtkWidget * assistant )
{
    GtkWidget * vbox, * paddingbox, * chooser, * tree_view, * sw;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeModel * model, * list_acc;
    int i;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    paddingbox = new_paddingbox_with_title ( vbox, TRUE, "Choose file to import" );
    
    chooser = gtk_button_new_with_label ( "Add file to import" );
    gtk_box_pack_start ( GTK_BOX(paddingbox), chooser, FALSE, FALSE, 6 );
    g_signal_connect ( G_OBJECT ( chooser ), "clicked", G_CALLBACK ( import_select_file ),
		       assistant );

    /* Scroll for tree view. */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_usize ( sw, 480, 120 );
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX(paddingbox), sw, TRUE, TRUE, 6 );

    /* Tree view and model. */
    model = GTK_TREE_MODEL ( gtk_tree_store_new ( IMPORT_FILESEL_NUM_COLS, G_TYPE_BOOLEAN, 
						  G_TYPE_STRING, G_TYPE_STRING, 
						  G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING));
    tree_view = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( model ) );
    gtk_container_add ( GTK_CONTAINER ( sw ), tree_view );

    /* Toggle column. */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect ( renderer, "toggled", G_CALLBACK (import_active_toggled), model);
    column = gtk_tree_view_column_new_with_attributes ( _("Import"), renderer,
							"active", IMPORT_FILESEL_SELECTED, 
							NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ), column );

    /* Type column. */
    renderer = gtk_cell_renderer_combo_new ();
    g_signal_connect ( G_OBJECT (renderer), "edited", G_CALLBACK ( import_switch_type), 
		       model );
    list_acc = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
    for ( i = 0 ; i < TYPE_MAX ; i++ )
    {
	GtkTreeIter iter; 

	if ( i != TYPE_GBANKING )
	{
	    gtk_list_store_append (GTK_LIST_STORE (list_acc), &iter);
	    gtk_list_store_set (GTK_LIST_STORE (list_acc), &iter, 0, 
				type_string_representation ( i ), -1);
	}
    }
    g_object_set ( renderer, 
		   "model", list_acc, 
		   "text-column", 0, 
		   "editable", TRUE, 
		   "has-entry", FALSE, 
		   NULL );
    column = gtk_tree_view_column_new_with_attributes ( _("Type"), renderer,
							"text", IMPORT_FILESEL_TYPENAME, 
							NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ), column );

    /* Name column. */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ( _("File name"), renderer,
							"text", IMPORT_FILESEL_FILENAME,
							NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ), column );

    g_object_set_data ( G_OBJECT(assistant), "tree_view", model );
    g_object_set_data ( G_OBJECT(assistant), "model", model );
    g_object_set_data ( G_OBJECT(model), "assistant", assistant );

    return vbox;
}



/**
 *
 *
 */
gboolean import_switch_type ( GtkCellRendererText *cell, const gchar *path,
			      const gchar *value, GtkListStore * model )
{
     GtkTreeIter iter;
     guint i = 0;
     GtkWidget * assistant;

     assistant = g_object_get_data ( G_OBJECT (model), "assistant" );

     if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( model ), &iter, path ))
     {
	 for ( i = 0 ; i < TYPE_MAX ; i ++ )
	 {
	     if ( ! strcmp ( value, type_string_representation ( i ) ) )
	     {
		 gtk_tree_store_set ( GTK_TREE_STORE (model), &iter,
				      IMPORT_FILESEL_TYPENAME, value, 
				      IMPORT_FILESEL_TYPE, i, 
				      -1 );
		 
		 /* CSV is special because it needs configuration, so
		  * we add a conditional jump there. */
		 if ( i == TYPE_CSV )
		 {
		     gsb_assistant_set_next ( assistant, IMPORT_FILESEL_PAGE, 
					      IMPORT_CSV_PAGE );
		     gsb_assistant_set_prev ( assistant, IMPORT_RESUME_PAGE, 
					      IMPORT_CSV_PAGE );
		 }

		 import_preview_maybe_sensitive_next ( assistant, GTK_TREE_MODEL ( model ));
	     }
	 }
     }

     return FALSE;
}    




/**
 *
 *
 */
gboolean import_enter_file_selection_page ( GtkWidget * assistant )
{
    GtkTreeModel * model;

    model = g_object_get_data ( G_OBJECT ( assistant ), "model" );
    import_preview_maybe_sensitive_next ( assistant, model );

    return FALSE;
}



/**
 *
 *
 *
 */
gboolean import_active_toggled ( GtkCellRendererToggle * cell, gchar *path_str,
				 gpointer model )
{
    GtkWidget * assistant;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter;
    gboolean toggle_item;

    assistant = g_object_get_data ( G_OBJECT ( model ), "assistant" );

    gtk_tree_model_get_iter ( GTK_TREE_MODEL ( model ), &iter, path);
    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 
			 IMPORT_FILESEL_SELECTED, &toggle_item, -1 );
    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter, 
			 IMPORT_FILESEL_SELECTED, !toggle_item, -1 );

    import_preview_maybe_sensitive_next ( assistant, model );

    return FALSE;
}



/**
 *
 *
 *
 */
void import_preview_maybe_sensitive_next ( GtkWidget * assistant, GtkTreeModel * model ) 
{
    GtkTreeIter iter;

    /* Don't allow going to next page if no file is selected yet. */
    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant), "button_next" ), FALSE );

    gtk_tree_model_get_iter_first ( model, &iter );
    if ( ! gtk_tree_store_iter_is_valid ( GTK_TREE_STORE (model), &iter ))
    {
	return;
    }

    /* Iterate over lines so we check if some are checked. */
    do 
    {
	gboolean selected;
	enum import_type type;
	gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 
			     IMPORT_FILESEL_SELECTED, &selected, 
			     IMPORT_FILESEL_TYPE, &type,
			     -1 );
	if ( selected && type != TYPE_UNKNOWN )
	{
	    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant), "button_next" ), 
				       TRUE );
	    return;
	}
    }
    while ( gtk_tree_model_iter_next ( model, &iter ) );
}



/**
 *
 *
 *
 */
gboolean import_select_file ( GtkWidget * button, GtkWidget * assistant )
{
    GtkWidget * dialog, * hbox, * go_charmap_sel;
    GtkFileFilter * filter;

    dialog = gtk_file_chooser_dialog_new ( _("Choose files to import."),
					   NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					   NULL );
    gtk_file_chooser_set_select_multiple ( GTK_FILE_CHOOSER ( dialog ), TRUE );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Known files (QIF, OFX, CSV, Gnucash, CSV)") );
    gtk_file_filter_add_pattern ( filter, "*.qif" );
    gtk_file_filter_add_pattern ( filter, "*.ofx" );
    gtk_file_filter_add_pattern ( filter, "*.gnc" );
    gtk_file_filter_add_pattern ( filter, "*.gnucash" );
    gtk_file_filter_add_pattern ( filter, "*.csv" );
    gtk_file_filter_add_pattern ( filter, "*.tsv" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );
    gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("QIF files (*.qif)") );
    gtk_file_filter_add_pattern ( filter, "*.qif" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("OFX files (*.ofx)") );
    gtk_file_filter_add_pattern ( filter, "*.ofx" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Gnucash files (*.gnc)") );
    gtk_file_filter_add_pattern ( filter, "*.gnc" );
    gtk_file_filter_add_pattern ( filter, "*.gnucash" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("CSV files (*.csv,*.tsv)") );
    gtk_file_filter_add_pattern ( filter, "*.csv" );
    gtk_file_filter_add_pattern ( filter, "*.tsv" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("All files") );
    gtk_file_filter_add_pattern ( filter, "*" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );

    /* Add encoding preview */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_file_chooser_set_extra_widget ( GTK_FILE_CHOOSER ( dialog ), hbox );
    gtk_box_pack_start ( GTK_BOX ( hbox ), gtk_label_new ( COLON(_("Encoding")) ), 
			 FALSE, FALSE, 0 );
    go_charmap_sel = go_charmap_sel_new (GO_CHARMAP_SEL_TO_UTF8);
    gtk_box_pack_start ( GTK_BOX ( hbox ), go_charmap_sel, TRUE, TRUE, 0 );
    gtk_widget_show_all ( hbox );

    if ( gtk_dialog_run ( GTK_DIALOG (dialog ) ) == GTK_RESPONSE_ACCEPT )
    {
	GSList * filenames, * iterator;
	GtkTreeModel * model;

	filenames = gtk_file_chooser_get_filenames ( GTK_FILE_CHOOSER ( dialog ) );
	iterator = filenames;
	
	model = g_object_get_data ( G_OBJECT ( assistant ), "model" );
	
	while ( iterator && model )
	{
	    GtkTreeIter iter;
	    enum import_type type = autodetect_file_type ( iterator -> data, NULL, NULL );

	    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, NULL );
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter, 
				 IMPORT_FILESEL_SELECTED, TRUE,
				 IMPORT_FILESEL_TYPENAME, type_string_representation (type),
				 IMPORT_FILESEL_FILENAME, g_path_get_basename ( iterator -> data ),
				 IMPORT_FILESEL_REALNAME, iterator -> data,
				 IMPORT_FILESEL_TYPE, type,
				 IMPORT_FILESEL_CODING, go_charmap_sel_get_encoding ( (GOCharmapSel * )go_charmap_sel ),
				 -1 ); 

	    /* CSV is special because it needs configuration, so we
	     * add a conditional jump there. */
	    if ( type == TYPE_CSV )
	    {
		gsb_assistant_set_next ( assistant, IMPORT_FILESEL_PAGE, IMPORT_CSV_PAGE );
		gsb_assistant_set_prev ( assistant, IMPORT_RESUME_PAGE, IMPORT_CSV_PAGE );
	    }

	    if ( type != TYPE_UNKNOWN )
	    {
		/* A valid file was selected, so we can now go ahead. */
		gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant), "button_next" ), 
					   TRUE );
	    }

	    iterator = iterator -> next;
	}
	
	if ( filenames )
	{
	    g_slist_free ( filenames );
	}
    }

    gtk_widget_destroy ( dialog );


    return FALSE;
}



/**
 *
 *
 */
GtkWidget * import_create_resume_page ( GtkWidget * assistant )
{
    GtkWidget * view;
    GtkTextBuffer * buffer;

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

    gtk_text_view_set_editable ( GTK_TEXT_VIEW (view), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW (view), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW (view), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW (view), 12 );

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_create_tag ( buffer, "bold",
				 "weight", PANGO_WEIGHT_BOLD, NULL);  
    gtk_text_buffer_create_tag ( buffer, "x-large",
				 "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag (buffer, "indented",
				"left-margin", 24, NULL);
  
    g_object_set_data ( G_OBJECT ( assistant ), "text-buffer", buffer );

    return view;
}



/**
 *
 *
 *
 */
gboolean import_enter_resume_page ( GtkWidget * assistant )
{
    GSList * files = import_selected_files ( assistant ), * list;
    FILE * qif_fd;
    GtkTextBuffer * buffer;
    GtkTextIter iter;

    liste_comptes_importes_error = NULL;
    liste_comptes_importes = NULL;

    while ( files )
    {
	struct imported_file * imported = files -> data;

	switch ( imported -> type )
	{
	    case TYPE_CSV :
		csv_import_csv_account ( assistant, imported );
		break;

	    case TYPE_OFX :
		recuperation_donnees_ofx ( imported );
		break;

	    case TYPE_QIF :
		if ( (qif_fd = utf8_fopen ( imported -> name, "r" )))
		{
		    recuperation_donnees_qif ( qif_fd, imported );
		    fclose ( qif_fd );
		}
		break;

	    case TYPE_GNUCASH:
		recuperation_donnees_gnucash ( imported );
		break;

	    default:
		break;
	}

	files = files -> next;
    }

    buffer = g_object_get_data ( G_OBJECT ( assistant ), "text-buffer" );
    gtk_text_buffer_set_text (buffer, "\n", -1 );
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);

    if ( liste_comptes_importes )
    {
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("Congratulations !"), -1,
						  "x-large", NULL);
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );

	gtk_text_buffer_insert (buffer, &iter, 
				_("You successfully imported files into Grisbi.  The following pages will help you set up imported data for the following files:"), 
				-1 );
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );

	list = liste_comptes_importes;
	while ( list )
	{
	    struct struct_compte_importation * compte;
	    compte = list -> data;

	    /* Fix account name if needed. */
	    if ( ! compte -> nom_de_compte )
	    {
		compte -> nom_de_compte = _("Unnamed Imported account");
	    }

	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, 
						      g_strconcat ( "• ",
								    compte -> nom_de_compte,
								    " (", 
								    type_string_representation ( compte -> origine ),
								    ")\n\n", 
								    NULL ),
						      -1, "indented", NULL );

	    list = list -> next;
	}

	while ( gtk_notebook_get_n_pages ( g_object_get_data ( G_OBJECT (assistant), "notebook" ) ) >
		IMPORT_FIRST_ACCOUNT_PAGE )
	{
	    gtk_notebook_remove_page ( g_object_get_data ( G_OBJECT (assistant), "notebook" ), -1 );
	}
	affichage_recapitulatif_importation ( assistant );
    }
    else 
    {
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("Error !"), -1,
						  "x-large", NULL);
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );

	gtk_text_buffer_insert (buffer, &iter, 
				_("No file has been imported, please double check that they are valid files.  Please make sure that they are not compressed and that their format is valid."), 
				-1 );
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );
    }

    if ( liste_comptes_importes_error )
    {
	gtk_text_buffer_insert (buffer, &iter, "The following files are in error:", -1 );
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );

	list = liste_comptes_importes_error;
	while ( list )
	{
	    struct struct_compte_importation * compte;
	    compte = list -> data;

	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, 
						      g_strconcat ( "• ",
								    compte -> nom_de_compte,
								    " (", 
								    type_string_representation ( compte -> origine ),
								    ")\n\n", 
								    NULL ),
						      -1, "indented", NULL );

	    list = list -> next;
	}
    }

    return FALSE;
}



/**
 *
 *
 */
GtkWidget * import_create_final_page ( GtkWidget * assistant )
{
    GtkWidget * view;
    GtkTextBuffer * buffer;
    GtkTextIter iter;

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

    gtk_text_view_set_editable ( GTK_TEXT_VIEW (view), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW (view), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW (view), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW (view), 12 );

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_create_tag ( buffer, "x-large",
				 "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);
  
    gtk_text_buffer_insert (buffer, &iter, "\n", -1 );
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _("Import terminated"), -1,
					      "x-large", NULL);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );
    gtk_text_buffer_insert (buffer, &iter, 
			    _("You have successfully set up transactions import into Grisbi.  Press the 'Close' button to terminate import."), 
			    -1 );
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );

    return view;
}



/**
 *
 *
 *
 */
GSList * import_selected_files ( GtkWidget * assistant )
{
    GSList * list = NULL;
    GtkTreeModel * model;
    GtkTreeIter iter;

    model = g_object_get_data ( G_OBJECT ( assistant ), "model" );
    g_return_val_if_fail ( model, NULL );

    gtk_tree_model_get_iter_first ( model, &iter );
    
    do 
    {
	struct imported_file * imported;
	gboolean selected;

	imported = g_malloc0 ( sizeof ( struct imported_file ) );
	gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 
			     IMPORT_FILESEL_SELECTED, &selected,
			     IMPORT_FILESEL_REALNAME, &(imported -> name), 
			     IMPORT_FILESEL_TYPE, &(imported -> type), 
			     IMPORT_FILESEL_CODING, &(imported -> coding_system),
			     -1 );

	if ( selected )
	{
	    list = g_slist_append ( list, imported );
	}
    }
    while ( gtk_tree_model_iter_next ( model, &iter ) );

    return list;
}



/**
 *
 *
 */
gboolean affichage_recapitulatif_importation ( GtkWidget * assistant )
{
    GtkWidget *button_next;
    gint page;
    GSList *list_tmp;

    /* FIXME */
    if (!assistant)
	return FALSE;

    /* We have to do that as soon as possible since this would reset currencies */
    if ( !gsb_data_account_get_accounts_amount () )
      {
	init_variables ();
      }

    /* Initial page is fourth. */
    page = IMPORT_FIRST_ACCOUNT_PAGE;

    /* First, iter to see if we need to create currencies */
    list_tmp = liste_comptes_importes;
    while ( list_tmp )
    {
	struct struct_compte_importation * compte;
	compte = list_tmp -> data;
	gint currency_number = 0;
		
	if ( compte -> devise )
	{
	    /* First, we search currency from ISO4217 code for
	       existing currencies */
	    currency_number = gsb_data_currency_get_number_by_code_iso4217 ( compte -> devise );

	    /* Then, by nickname for existing currencies */
	    if ( !currency_number )
		currency_number = gsb_data_currency_get_number_by_name ( compte -> devise );

	    /* Last ressort, we browse ISO4217 currency list and create
	       currency if found */
	    if ( !currency_number )
		currency_number = gsb_currency_config_create_currency_from_iso4217list ( compte -> devise );
	}
	else
	{
	    struct lconv * conv = localeconv();

	    /* We try to set default currency of account according to
	     * locale.  We will not prompt user since import dialog is
	     * too long already. */
	    if ( conv && conv -> int_curr_symbol && strlen ( conv -> int_curr_symbol ) )
	    {
		gchar * name = g_strstrip ( my_strdup ( conv -> int_curr_symbol ) );
		currency_number =  gsb_currency_config_create_currency_from_iso4217list ( name );
		g_free ( name );
	    }
	    if ( ! currency_number &&
		 ! gsb_currency_config_create_currency_from_iso4217list ( "USD" ) )
	    {
		dialogue_error_brain_damage ();
	    }
	}

	gsb_assistant_add_page ( assistant, cree_ligne_recapitulatif ( list_tmp -> data ), 
				 page, page - 1, page + 1, G_CALLBACK ( NULL ) );
	page ++;

	list_tmp = list_tmp -> next;
    }

    /* And final page */
    gsb_assistant_add_page ( assistant, import_create_final_page ( assistant ), 
			     page, page - 1, -1, G_CALLBACK ( NULL ) );

    /* Replace button. */
    button_next = g_object_get_data ( G_OBJECT (assistant), "button_next" );
    gtk_widget_destroy ( button_next );
    button_next = gtk_dialog_add_button ( GTK_DIALOG (assistant), GTK_STOCK_GO_FORWARD,
					  GTK_RESPONSE_YES );
    g_object_set_data ( G_OBJECT (assistant), "button_next", button_next );

    printf (">>> NTH pages %d\n", gtk_notebook_get_n_pages ( g_object_get_data ( G_OBJECT (assistant), "notebook" ) ) );

    /* si aucun compte n'est ouvert, on crée les devises de base */

/*     if ( !gsb_data_account_get_accounts_amount () ) */
/*     { */
/* 	menus_sensitifs ( FALSE ); */
/* 	gsb_currency_config_add_currency (NULL); */
/*     } */

    return ( FALSE );
}



/**
 *
 *
 *
 */
GtkWidget * cree_ligne_recapitulatif ( struct struct_compte_importation * compte )
{
    GtkWidget * vbox, * hbox, * label, * radio, * radiogroup;
    GtkWidget * alignement;
    gchar * short_filename;
    gint size = 0, spacing = 0;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    if ( compte -> filename )
    {
	short_filename = g_strrstr ( compte -> filename, C_DIRECTORY_SEPARATOR );
	if ( ! short_filename )
	{
	    short_filename = compte -> filename;
	}
	else 
	{
	    short_filename ++;
	}
    }
    else
    {
	short_filename = _("file");
    }

    label = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_label_set_markup ( GTK_LABEL ( label ),
			   g_strdup_printf ( _("<span size=\"x-large\">%s</span>\n\n"
					       "What do you want to do with contents from <tt>%s</tt>?\n"),
					     compte -> nom_de_compte, short_filename ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 0 );

    /* New account */
    radio = gtk_radio_button_new_with_label ( NULL, _("Create a new account") );
    radiogroup = radio;
    gtk_box_pack_start ( GTK_BOX ( vbox ), radio, FALSE, FALSE, 0 );
    gtk_widget_style_get (radio, "indicator_size", &size, NULL);
    gtk_widget_style_get (radio, "indicator_spacing", &spacing, NULL);

    compte -> hbox1 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), compte -> hbox1, FALSE, FALSE, 0 );
    label = gtk_label_new ( _("Account type:") );
    alignement = gtk_alignment_new ( 0.5, 0.5, 1, 1 );
    gtk_container_set_border_width ( GTK_CONTAINER ( alignement ), 2 );
    gtk_alignment_set_padding ( GTK_ALIGNMENT ( alignement ), 0, 0, 2 * spacing + size, 0 );
    gtk_container_add ( GTK_CONTAINER ( alignement ), label );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox1 ), alignement, FALSE, FALSE, 0 );
    compte -> bouton_type_compte = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_type_compte ), 
			       creation_menu_type_compte () );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox1 ), compte -> bouton_type_compte, 
			 TRUE, TRUE, 0 );

    g_object_set_data ( G_OBJECT ( radio ), "associated", compte -> hbox1 );
    g_object_set_data ( G_OBJECT ( radio ), "account", compte );
    g_signal_connect ( G_OBJECT ( radio ), "toggled", 
		       G_CALLBACK ( import_account_action_activated ), 0 );


    /* Add to account */
    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( radiogroup ), 
							  _("Add transactions to an account") );
    gtk_box_pack_start ( GTK_BOX ( vbox ), radio, FALSE, FALSE, 0 );

    compte -> hbox2 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), compte -> hbox2, FALSE, FALSE, 0 );
    label = gtk_label_new ( _("Account name:") );
    alignement = gtk_alignment_new ( 0.5, 0.5, 1, 1 );
    gtk_container_set_border_width ( GTK_CONTAINER ( alignement ), 2 );
    gtk_alignment_set_padding ( GTK_ALIGNMENT ( alignement ), 0, 0, 2 * spacing + size, 0 );
    gtk_container_add ( GTK_CONTAINER ( alignement ), label );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox2 ), alignement, FALSE, FALSE, 0 );
    compte -> bouton_compte_add = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_compte_add ), 
			       creation_option_menu_comptes ( NULL, TRUE, FALSE ) );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox2 ), compte -> bouton_compte_add, TRUE, TRUE, 0 );
    gtk_widget_set_sensitive ( compte -> hbox2, FALSE );

    g_object_set_data ( G_OBJECT ( radio ), "associated", compte -> hbox2 );
    g_object_set_data ( G_OBJECT ( radio ), "account", compte );
    g_signal_connect ( G_OBJECT ( radio ), "toggled", 
		       G_CALLBACK ( import_account_action_activated ), GINT_TO_POINTER (1));

    /* Mark account */
    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( radiogroup ), 
							  _("Mark transactions of an account") );
    gtk_box_pack_start ( GTK_BOX ( vbox ), radio, FALSE, FALSE, 0 );

    compte -> hbox3 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), compte -> hbox3, FALSE, FALSE, 0 );
    label = gtk_label_new ( _("Account name:") );
    alignement = gtk_alignment_new ( 0.5, 0.5, 1, 1 );
    gtk_container_set_border_width ( GTK_CONTAINER ( alignement ), 2 );
    gtk_alignment_set_padding ( GTK_ALIGNMENT ( alignement ), 0, 0, 2 * spacing + size, 0 );
    gtk_container_add ( GTK_CONTAINER ( alignement ), label );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox3 ), alignement, FALSE, FALSE, 0 );
    compte -> bouton_compte_mark = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_compte_mark ), 
			       creation_option_menu_comptes ( NULL, TRUE, FALSE ) );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox3 ), compte -> bouton_compte_mark, TRUE, TRUE, 0 );
    gtk_widget_set_sensitive ( compte -> hbox3, FALSE );

    g_object_set_data ( G_OBJECT ( radio ), "associated", compte -> hbox3 );
    g_object_set_data ( G_OBJECT ( radio ), "account", compte );
    g_signal_connect ( G_OBJECT ( radio ), "toggled", 
		       G_CALLBACK ( import_account_action_activated ), GINT_TO_POINTER (2));

    /* Currency */
    hbox = gtk_hbox_new ( FALSE, 6 );
    label = gtk_label_new ( _("Account currency:") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    compte -> bouton_devise = gsb_currency_make_combobox (FALSE);

    if ( compte -> devise )
    {
	gint currency_number;

	/* First, we search currency from ISO4217 code for existing currencies */
	currency_number = gsb_data_currency_get_number_by_code_iso4217 ( compte -> devise );
	/* Then, by nickname for existing currencies */
	if ( !currency_number )
	  currency_number = gsb_data_currency_get_number_by_name ( compte -> devise );

	if (currency_number)
	    gsb_currency_set_combobox_history ( compte -> bouton_devise,
						currency_number);
	else
	{
	    /* 	    la devise avait un nom mais n'a pas été retrouvée; 2 possibilités : */
	    /* 		- soit elle n'est pas cree (l'utilisateur
			  la créera une fois la fenetre affichée) */
	    /* 		- soit elle est créé mais pas avec le bon code */
	    dialogue_warning_hint ( g_strdup_printf ( _( "Currency of imported account '%s' is %s.  Either this currency doesn't exist so you have to create it in next window, or this currency already exists but the ISO code is wrong.\nTo avoid this message, please set its ISO code in configuration."),
						      compte -> nom_de_compte,
						      compte -> devise ),
				    g_strdup_printf ( _("Can't associate ISO 4217 code for currency '%s'."),  compte -> devise ));

	}
    }
    gtk_box_pack_start ( GTK_BOX ( hbox ), compte -> bouton_devise, FALSE, FALSE, 0 );

    return vbox;

}



/**
 *
 *
 */
gboolean import_account_action_activated ( GtkWidget * radio, gint action )
{
    struct struct_compte_importation * account;

    account = g_object_get_data ( G_OBJECT ( radio ), "account" );
    
    gtk_widget_set_sensitive ( account -> hbox1, FALSE );
    gtk_widget_set_sensitive ( account -> hbox2, FALSE );
    gtk_widget_set_sensitive ( account -> hbox3, FALSE );
    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT ( radio ), "associated" ), TRUE );

    account -> action = action;

    return FALSE;    
}



/* *******************************************************************************/
void traitement_operations_importees ( void )
{
    /* cette fonction va faire le tour de liste_comptes_importes */
    /* et faire l'action demandée pour chaque compte importé */

    GSList *list_tmp;
    gint new_file;

    /* fait le nécessaire si aucun compte n'est ouvert */

    if ( gsb_data_account_get_accounts_amount () )
	new_file = 0;
    else
    {
	init_variables_new_file ();
	new_file = 1;
    }


    list_tmp = liste_comptes_importes;

    while ( list_tmp )
    {
	struct struct_compte_importation *compte;
	gint account_number;

	compte = list_tmp -> data;

	switch ( compte -> action )
	{
	    case 0:
		/* create */

		account_number = gsb_import_create_imported_account ( compte );
		if ( !new_file )
		    gsb_gui_navigation_add_account ( account_number );

		if ( account_number != -1 )
		    gsb_import_add_imported_transactions ( compte,
							   account_number );
		else
		    dialogue_error ( "An error occured while creating the new account,\nthe import is stopped." );
		break;

	    case 1:
		/* add */

		gsb_import_add_imported_transactions ( compte,
						       recupere_no_compte ( compte -> bouton_compte_add ));

		break;

	    case 2:
		/* pointer */

		pointe_opes_importees ( compte );

		break;
	}
	list_tmp = list_tmp -> next;
    }

    /*     à ce niveau, il y a forcemment des comptes de créés donc si rien */
    /* 	c'est que pb, on se barre */

    if (!gsb_data_account_get_accounts_amount ())
	return;

    /* les différentes liste d'opérations ont été créés, on va faire le tour des opés */
    /* pour retrouver celles qui ont relation_no_compte à -2 */
    /* c'est que c'est un virement, il reste à retrouver le compte et l'opération correspondants */

    /* virements_a_chercher est à 1 si on doit chercher des relations entre opés importées */

    if ( virements_a_chercher )
	cree_liens_virements_ope_import ();


    /* création des listes d'opé */

    gsb_status_message ( _("Please wait") );

    if ( ! new_file )
    {
	/* on fait le tour des comptes ajoutés pour leur créer une liste d'opé */
	/* 	et mettre à jour ceux qui le doivent */

	GSList *list_tmp;

	list_tmp = gsb_data_account_get_list_accounts ();
	demande_mise_a_jour_tous_comptes ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_data_account_get_no_account ( list_tmp -> data );

	    if ( !gsb_transactions_list_get_tree_view()  )
	    {
		/*     on crée le tree_view du compte */

/* 		creation_colonnes_tree_view_par_compte (i); */

		/* xxx ici voir pour le tree_view */
/* 		gtk_box_pack_start ( GTK_BOX ( tree_view_vbox ), */
/* 				     creation_tree_view_operations_par_compte (i), */
/* 				     TRUE, */
/* 				     TRUE, */
/* 				     0 ); */
		/* on met à jour l'option menu du formulaire des échéances */

		update_options_menus_comptes ();


		/* 	on réaffiche la liste des comptes */

		gsb_menu_update_accounts_in_menus();
	    }

	    gsb_data_account_list_gui_change_current_account ( GINT_TO_POINTER(i) );
	    remplissage_details_compte ();

	    list_tmp = list_tmp -> next;
	}

	/* 	mise à jour de l'accueil */

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
	mise_a_jour_accueil (FALSE);


    }
    else
    {
	init_gui_new_file ();
    }

    /* on recrée les combofix des tiers et des catégories */

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ();

/*     gsb_status_clear(); */

    modification_fichier ( TRUE );
}
/* *******************************************************************************/

/* *******************************************************************************/
/* cette fontion recherche des opés qui sont des virements non encore reliés après un import
   dans ce cas ces opé sont marquées à -2 en relation_no_compte et info_banque_guichet contient
   le nom du compte de virement. la fonction crée donc les liens entre virements */
/* *******************************************************************************/

void cree_liens_virements_ope_import ( void )
{
    /* on fait le tour de toutes les opés des comptes */
    /* si une opé à un relation_no_compte à -2 , */
    /* on recherche le compte associé dont le nom est dans info_banque_guichet */
    /*   et une opé ayant une relation_no_compte à -2, le nom du compte dans info_banque_guichet */
    /* le même montant, le même jour avec le même tiers */

    GSList *list_tmp_transactions;
    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	/* if the account number of transfer is -2, it's a transfer */

	if ( gsb_data_transaction_get_account_number_transfer (transaction_number_tmp)== -2
	     &&
	     gsb_data_transaction_get_bank_references (transaction_number_tmp))
	{
	    /* the name of the contra account is in the bank references with [ and ] */

	    gchar *contra_account_name;
	    gint contra_account_number;

	    contra_account_name = gsb_data_transaction_get_bank_references (transaction_number_tmp);
	    if ( contra_account_name && strlen ( contra_account_name ) )
	    {
		contra_account_name++;
		contra_account_name[strlen(contra_account_name)-1] = 0;
	    }
	    contra_account_number = gsb_data_account_get_no_account_by_name ( contra_account_name );

	    if ( contra_account_number == -1 )
	    {
		/* we have not found the contra-account */

		gsb_data_transaction_set_account_number_transfer ( transaction_number_tmp,
								   0);
		gsb_data_transaction_set_transaction_number_transfer ( transaction_number_tmp,
								       0);
	    }
	    else
	    {
		/* we have found the contra-account, we look for the contra-transaction */

		GSList *list_tmp_transactions_2;
		list_tmp_transactions_2 = gsb_data_transaction_get_transactions_list ();

		while ( list_tmp_transactions_2 )
		{
		    gint contra_transaction_number_tmp;
		    contra_transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions_2 -> data);

		    if ( gsb_data_transaction_get_account_number (contra_transaction_number_tmp) == contra_account_number
			 &&
			 gsb_data_transaction_get_account_number_transfer ( contra_transaction_number_tmp ) == -2
			 &&
			 gsb_data_transaction_get_bank_references ( contra_transaction_number_tmp )
			 &&
			 (!g_strcasecmp ( g_strconcat ("[",
						       gsb_data_account_get_name (transaction_number_tmp),
						       "]",
						       NULL),
					  g_strstrip ( gsb_data_transaction_get_bank_references ( contra_transaction_number_tmp )))
			  ||
			  g_strcasecmp ( gsb_data_account_get_name (transaction_number_tmp),
					 g_strstrip ( gsb_data_transaction_get_bank_references ( contra_transaction_number_tmp)))) 
			 &&
			 ( fabs ( gsb_data_transaction_get_amount (transaction_number_tmp))
			   ==
			   fabs ( gsb_data_transaction_get_adjusted_amount_for_currency ( contra_transaction_number_tmp,
											  gsb_data_account_get_currency (transaction_number_tmp))))
			 &&
			 ( gsb_data_transaction_get_party_number (transaction_number_tmp)
			   ==
			   gsb_data_transaction_get_party_number ( contra_transaction_number_tmp ))
			 &&
			 !g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
					   gsb_data_transaction_get_date (contra_transaction_number_tmp)))
		    {
			/* la 2ème opération correspond en tout point à la 1ère, on met les relations */

			gsb_data_transaction_set_transaction_number_transfer ( transaction_number_tmp,
									       contra_transaction_number_tmp );
			gsb_data_transaction_set_account_number_transfer ( transaction_number_tmp,
									   gsb_data_transaction_get_account_number (contra_transaction_number_tmp));

			gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number_tmp,
									       transaction_number_tmp);
			gsb_data_transaction_set_account_number_transfer ( contra_transaction_number_tmp,
									   gsb_data_transaction_get_account_number (transaction_number_tmp));

			gsb_data_transaction_set_bank_references ( transaction_number_tmp,
								   "" );
			gsb_data_transaction_set_bank_references ( contra_transaction_number_tmp,
								   "" );
		    }
		    list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
		}

		/* if no contra-transaction, that transaction becomes normal */

		if ( gsb_data_transaction_get_account_number_transfer (transaction_number_tmp) == -2 )
		{
		    gsb_data_transaction_set_account_number_transfer ( transaction_number_tmp,
								       0);
		    gsb_data_transaction_set_transaction_number_transfer ( transaction_number_tmp,
									   0);
		    gsb_data_transaction_set_bank_references ( transaction_number_tmp, "" );
		}
	    }
	}

	list_tmp_transactions = list_tmp_transactions -> next;
    }
}
/* *******************************************************************************/



/** create a new account with the datas in the imported account given in args
 * \param imported_account the account we want to create
 * \return the number of the new account
 * */
gint gsb_import_create_imported_account ( struct struct_compte_importation *imported_account )
{
    /* crée un nouveau compte contenant les données de la structure importée */
    /* ajoute ce compte aux anciens */

    gint account_number;

    /*     on crée et initialise le nouveau compte  */
    /*     le type par défaut est 0 (compte bancaire) */

    account_number = gsb_data_account_new( GSB_TYPE_BANK );

    /*     si ça c'est mal passé, on se barre */

    if ( account_number == -1 )
	return -1;

    /*     met l'id du compte s'il existe (import ofx) */

    if ( imported_account -> id_compte )
    {
	gchar **tab_str;

	gsb_data_account_set_id (account_number,
			    my_strdup ( imported_account -> id_compte ));

	/* 	en théorie cet id est "no_banque no_guichet no_comptecle" */
	/* on va essayer d'importer ces données ici */
	/* si on rencontre un null, on s'arrête */

	tab_str = g_strsplit ( gsb_data_account_get_id (account_number),
			       " ",
			       3 );
	if ( tab_str[1] )
	{
	    gsb_data_account_set_bank_branch_code ( account_number,
					       my_strdup ( tab_str[1] ) );

	    if ( tab_str[2] )
	    {
		gchar *temp;

		gsb_data_account_set_bank_account_key ( account_number,
						   my_strdup ( tab_str[2] + strlen ( tab_str[2] ) - 1 ) );

		temp = my_strdup ( tab_str[2] );

		temp[strlen (temp) - 1 ] = 0;
		gsb_data_account_set_bank_account_number ( account_number,
						      temp );
	    }
	}
	g_strfreev ( tab_str );
    }

    /* met le nom du compte */

    if ( imported_account -> nom_de_compte )
	gsb_data_account_set_name ( account_number,
			       g_strstrip ( imported_account -> nom_de_compte ) );
    else
	gsb_data_account_set_name ( account_number,
			       my_strdup ( _("Imported account")) );

    /* choix de la devise du compte */

    gsb_data_account_set_currency ( account_number,
				    gsb_currency_get_currency_from_combobox (imported_account -> bouton_devise));

    /* met le type de compte si différent de 0 */

    switch ( imported_account -> type_de_compte )
    {
	case 3:
	    gsb_data_account_set_kind (account_number,
				  GSB_TYPE_LIABILITIES);
	    break;

	case 7:
	    gsb_data_account_set_kind (account_number,
				  GSB_TYPE_CASH);
	    break;
    }

    /* met le solde init */

    gsb_data_account_set_init_balance ( account_number,
				   imported_account -> solde);
    gsb_data_account_set_current_balance ( account_number, 
				      gsb_data_account_get_init_balance (account_number));
    gsb_data_account_set_marked_balance ( account_number, 
				     gsb_data_account_get_init_balance (account_number));

    /* Use two lines view by default. */
    gsb_data_account_set_nb_rows ( account_number, 2 );

    return account_number;
}


/** import the transactions in an existent account
 * check the id of the account and if the transaction already exists
 * \param imported_account an imported structure account which contains the transactions
 * \param account_number the number of account where we want to put the new transations
 * \return
 * */
void gsb_import_add_imported_transactions ( struct struct_compte_importation *imported_account,
					    gint account_number )
{
    GSList *list_tmp;
    GDate *last_date_import;
    gint demande_confirmation;
    GSList *list_tmp_transactions;

    /* si le compte importé a une id, on la vérifie ici */
    /*     si elle est absente, on met celle importée */
    /*     si elle est différente, on demande si on la remplace */

    if ( imported_account -> id_compte )
    {
	if ( gsb_data_account_get_id (account_number) )
	{
	    if ( g_strcasecmp ( gsb_data_account_get_id (account_number),
				imported_account -> id_compte ))
	    {
		/* 		l'id du compte choisi et l'id du compte importé sont différents */
		/* 		    on propose encore d'arrêter... */


		if ( question_yes_no_hint ( _("The id of the imported and chosen accounts are different"),
					    _("Perhaps you choose a wrong account ?  If you choose to continue, the id of the account will be changed.  Do you want to continue ?")))
		    gsb_data_account_set_id (account_number,
					     my_strdup ( imported_account -> id_compte ));
		else
		    return;
	    }
	}
	else
	    gsb_data_account_set_id (account_number,
				     my_strdup ( imported_account -> id_compte ));

    }


    /* on fait un premier tour de la liste des opés pour repérer celles qui sont déjà entrées */
    /*   si on n'importe que du ofx, c'est facile, chaque opé est repérée par une id */
    /*     donc si l'opé importée a une id, il suffit de rechercher l'id dans le compte, si elle */
    /*     n'y est pas l'opé est à enregistrer */
    /*     si on importe du qif ou du html, il n'y a pas d'id. donc soit on retrouve une opé semblable */
    /*     (cad même montant et même date, on ne fait pas joujou avec le tiers car l'utilisateur */
    /* a pu le changer), et on demande à l'utilisateur quoi faire, sinon on enregistre l'opé */


    /*   pour gagner en rapidité, on va récupérer la dernière date du compte, toutes les opés importées */
    /* qui ont une date supérieure sont automatiquement acceptées */


    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();
    last_date_import = NULL;

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number )
	{
	    if ( !last_date_import
		 ||
		 g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
				  last_date_import ) > 0 )
		last_date_import = gsb_data_transaction_get_date (transaction_number_tmp);
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }

    list_tmp = imported_account -> operations_importees;
    demande_confirmation = 0;

    while ( list_tmp )
    {
	struct struct_ope_importation *imported_transaction;
	imported_transaction = list_tmp -> data;

	/* on ne fait le tour de la liste des opés que si la date de l'opé importée est inférieure à la dernière date */
	/* de la liste */

	if ( last_date_import && g_date_compare ( last_date_import,
					   imported_transaction -> date ) >= 0 )
	{
	    /* 	    si l'opé d'import a une id, on recherche ça en priorité */

	    if ( imported_transaction -> id_operation
		 &&
		 operation_par_id ( imported_transaction -> id_operation,
				    account_number ))
		/* comme on est sûr que cette opé a déjà été enregistree, on met l'action à 2, cad on demande l'avis de personne pour */
		/*     pas l'enregistrer */
		imported_transaction -> action = 2;

	    /* 	    si l'opé d'import a un no de chq, on le recherche */

	    if ( imported_transaction -> action != 2
		 &&
		 imported_transaction -> cheque
		 &&
		 operation_par_cheque ( imported_transaction -> cheque,
					account_number ))
		/* 	comme on est sûr que cette opé a déjà été enregistree, on met l'action à 2, cad on demande l'avis de personne pour */
		/*  pas l'enregistrer */
		imported_transaction -> action = 2;

	    /* on fait donc le tour de la liste des opés pour retrouver une opé comparable */
	    /* si elle n'a pas déjà été retrouvée par id... */

	    if ( imported_transaction -> action != 2 )
	    {
		GDate *date_debut_comparaison;
		GDate *date_fin_comparaison;

		date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( imported_transaction -> date ),
							  g_date_get_month ( imported_transaction -> date ),
							  g_date_get_year ( imported_transaction -> date ));
		g_date_subtract_days ( date_debut_comparaison,
				       valeur_echelle_recherche_date_import );

		date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( imported_transaction -> date ),
							g_date_get_month ( imported_transaction -> date ),
							g_date_get_year ( imported_transaction -> date ));
		g_date_add_days ( date_fin_comparaison,
				  valeur_echelle_recherche_date_import );

		list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

		while ( list_tmp_transactions )
		{
		    gint transaction_number_tmp;
		    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number )
		    {
			if ( fabs ( gsb_data_transaction_get_amount (transaction_number_tmp)- imported_transaction -> montant ) < 0.01
			     &&
			     ( g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
						date_debut_comparaison ) >= 0 )
			     &&
			     ( g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
						date_fin_comparaison ) <= 0 )

			     &&
			     !imported_transaction -> ope_de_ventilation )
			{
			    /* l'opé a la même date et le même montant, on la marque pour demander quoi faire à l'utilisateur */
			    imported_transaction -> action = 1; 
			    imported_transaction -> ope_correspondante = gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp);
			    demande_confirmation = 1;
			}
		    }
		    list_tmp_transactions = list_tmp_transactions -> next;
		}
	    }
	}
	list_tmp = list_tmp -> next;
    }

    /*   à ce niveau, toutes les opés douteuses ont été marquées, on appelle la fonction qui */
    /* se charge de demander à l'utilisateur que faire */

    if ( demande_confirmation )
	confirmation_enregistrement_ope_import ( imported_account );


    /* on fait le tour des opés de ce compte et enregistre les opés */

    /* la variable derniere_operation est utilisée pour garder le numéro de l'opé */
    /* précédente pour les ventilations */

    mother_transaction_number = 0;

    list_tmp = imported_account -> operations_importees;

    while ( list_tmp )
    {
	struct struct_ope_importation *imported_transaction;

	imported_transaction = list_tmp -> data;

	/* vérifie qu'on doit bien l'enregistrer */
	if ( imported_transaction -> action == 0 )
	{
	    /* on récupère à ce niveau la devise choisie dans la liste */

	    imported_transaction -> devise = gsb_currency_get_currency_from_combobox (imported_account -> bouton_devise);
	    gsb_import_create_transaction ( imported_transaction,
					    account_number );
	} 
	list_tmp = list_tmp -> next;
    }


    gsb_data_account_set_update_list ( account_number,
				       1 );
/*     calcule_solde_compte ( account_number ); */
/*     calcule_solde_pointe_compte ( account_number ); */

}
/* *******************************************************************************/


/* *******************************************************************************/
void confirmation_enregistrement_ope_import ( struct struct_compte_importation *imported_account )
{
    /*   cette fonction fait le tour des opérations importées, et demande que faire pour celles */
    /* qui sont douteuses lors d'un ajout des opés à un compte existant */

    GSList *list_tmp;
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *scrolled_window;
    GtkWidget *label;
    gint action_derniere_ventilation;


    dialog = gtk_dialog_new_with_buttons ( _("Confirmation of importation of transactions"),
					   GTK_WINDOW ( window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK,
					   GTK_RESPONSE_OK,
					   NULL );
    gtk_widget_set_usize ( dialog,
			   FALSE,
			   300 );

    label = gtk_label_new ( _("Some imported transactions seem to be already saved. Please select the transactions to import." ));
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog )-> vbox ),
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
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog )-> vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    vbox );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     10 );
    gtk_widget_show ( vbox );

    /*   on fait maintenant le tour des opés importées et affichent celles à problème */

    list_tmp = imported_account -> operations_importees;

    while ( list_tmp )
    {
	struct struct_ope_importation *ope_import;

	ope_import = list_tmp -> data;

	/* on n'affiche pas si c'est des opés de ventil, si la mère est cochée, les filles seront alors cochées */
	/* on ne teste pas ici car ça a été testé avant */

	if ( ope_import -> action == 1 
	     &&
	     !ope_import -> ope_de_ventilation )
	{
	    gpointer operation;
	    gchar *tiers;

	    operation = ope_import -> ope_correspondante;

	    hbox = gtk_hbox_new ( FALSE,
				  5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ),
				 hbox,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( hbox );


	    ope_import -> bouton = gtk_check_button_new ();
	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 ope_import -> bouton,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( ope_import -> bouton );

	    label = gtk_label_new ( g_strdup_printf ( _("Transactions to import : %s ; %s ; %4.2f"),
						      gsb_format_gdate ( ope_import -> date ),
						      ope_import -> tiers,
						      ope_import -> montant ));
	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 label,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( label );


	    hbox = gtk_hbox_new ( FALSE,
				  5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ),
				 hbox,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( hbox );


	    label = gtk_label_new ( "       " );
	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 label,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( label );

	    tiers = gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( gsb_data_transaction_get_transaction_number (operation )), FALSE );

	    if ( gsb_data_transaction_get_notes ( gsb_data_transaction_get_transaction_number (operation)))
		label = gtk_label_new ( g_strdup_printf ( _("Transaction found : %s ; %s ; %4.2f ; %s"),
							  gsb_format_gdate ( gsb_data_transaction_get_date (gsb_data_transaction_get_transaction_number (operation) ) ),
							  tiers,
							  gsb_data_transaction_get_amount ( gsb_data_transaction_get_transaction_number (operation )),
							  gsb_data_transaction_get_notes ( gsb_data_transaction_get_transaction_number (operation ))));
	    else
		label = gtk_label_new ( g_strdup_printf ( _("Transaction found : %s ; %s ; %4.2f"),
							  gsb_format_gdate ( gsb_data_transaction_get_date (gsb_data_transaction_get_transaction_number (operation))),
							  tiers,
							  gsb_data_transaction_get_amount ( gsb_data_transaction_get_transaction_number (operation ))));

	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 label,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( label );
	}
	list_tmp = list_tmp -> next;
    }

    gtk_dialog_run ( GTK_DIALOG ( dialog ));

    /* on fait maintenant le tour des check buttons pour voir ce qu'on importe */

    list_tmp = imported_account -> operations_importees;
    action_derniere_ventilation = 1;

    while ( list_tmp )
    {
	struct struct_ope_importation *ope_import;

	ope_import = list_tmp -> data;

	/* si c'est une opé de ventil, elle n'était pas affichée, dans ce cas si l'action de la
	   dernière ventil était 0, on fait de même pour les filles */

	if ( ope_import -> ope_de_ventilation )
	{
	    if ( ope_import -> action )
		ope_import -> action = action_derniere_ventilation;
	}
	else
	    action_derniere_ventilation = 1;



	if ( ope_import -> bouton
	     &&
	     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( ope_import -> bouton )))
	{
	    ope_import -> action = 0;

	    /* si c'était une ventil on met l'action de la dernière ventil à 0 */

	    if ( ope_import -> operation_ventilee )
		action_derniere_ventilation = 0;	
	}

	list_tmp = list_tmp -> next;
    }

    gtk_widget_destroy ( dialog );
}
/* *******************************************************************************/


/** get an imported transaction structure in arg and create the corresponding transaction
 * \param imported_transaction the transaction to import
 * \param account_number the account where to put the new transaction
 * \return the number of the new transaction
 * */
gint gsb_import_create_transaction ( struct struct_ope_importation *imported_transaction,
				     gint account_number )
{
    gchar **tab_str;
    gint transaction_number;

    /* we create the new transaction */

    transaction_number = gsb_data_transaction_new_transaction ( account_number );

    /* récupération de l'id de l'opé s'il existe */

    if ( imported_transaction -> id_operation )
	gsb_data_transaction_set_transaction_id ( transaction_number,
						  my_strdup ( imported_transaction -> id_operation ));

    /* récupération de la date */

    gsb_data_transaction_set_date ( transaction_number,
				    gsb_date_copy ( imported_transaction -> date ));

    /* récupération de la date de valeur */

    gsb_data_transaction_set_value_date ( transaction_number,
					  gsb_date_copy ( imported_transaction -> date_de_valeur ));

    /* récupération du montant */

    gsb_data_transaction_set_amount ( transaction_number,
				      imported_transaction -> montant );

    /* 	  récupération de la devise */

    gsb_data_transaction_set_currency_number ( transaction_number,
					       imported_transaction -> devise );

    /* rÃ©cupération du tiers */

    if ( imported_transaction -> tiers 
	 &&
	 strlen ( g_strstrip ( imported_transaction -> tiers )))
	gsb_data_transaction_set_party_number ( transaction_number,
						gsb_data_payee_get_number_by_name ( imported_transaction -> tiers,
									       TRUE ));

    /* vérification si c'est ventilé, sinon récupération des catégories */

    if ( imported_transaction -> operation_ventilee )
    {
	/* l'opération est ventilée */

	gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
							    1 );
    }
    else
    {
	/* vérification que ce n'est pas un virement */

	if ( imported_transaction -> categ
	     &&
	     strlen ( g_strstrip (imported_transaction -> categ)) )
	{
	    if ( imported_transaction -> categ[0] == '[' )
	    {
		/* 		      c'est un virement, or le compte n'a peut être pas encore été créé, */
		/* on va mettre le nom du compte dans info_banque_guichet qui n'est jamais utilisé */
		/* lors d'import, et relation_no_compte sera mis à -2 (-1 est déjà utilisé pour les comptes supprimés */

		gsb_data_transaction_set_bank_references ( transaction_number,
							   imported_transaction -> categ);
		gsb_data_transaction_set_account_number_transfer ( transaction_number,
								   -2);
		gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
								       -1);
		virements_a_chercher = 1;
	    }
	    else
	    {
		/* it's a normal category */

		gint category_number;

		tab_str = g_strsplit ( imported_transaction -> categ,
				       ":",
				       2 );

		/* get the category and create it if doesn't exist */

		category_number = gsb_data_category_get_number_by_name ( g_strstrip (tab_str[0]),
									 TRUE,
									 imported_transaction -> montant < 0 );
		gsb_data_transaction_set_category_number ( transaction_number,
							   category_number );
		gsb_data_transaction_set_sub_category_number ( transaction_number,
							       gsb_data_category_get_sub_category_number_by_name ( category_number,
														   g_strstrip (tab_str[1]),
														   TRUE ));
	    }
	}
    }

    /* récupération des notes */

    gsb_data_transaction_set_notes ( transaction_number,
				     imported_transaction -> notes );

    /* récupération du chèque et mise en forme du type d'opération */

    if ( imported_transaction -> cheque )
    {
	/* c'est un chèque, on va rechercher un type à incrémentation automatique et mettre l'opé sous ce type */
	/* si un tel type n'existe pas, on met quand même le no dans contenu_type et on met le type par défaut */

	struct struct_type_ope *type_choisi;
	GSList *list_tmp;

	if ( gsb_data_transaction_get_amount (transaction_number)< 0 )
	    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								gsb_data_account_get_default_debit (account_number));
	else
	    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								gsb_data_account_get_default_credit (account_number));

	gsb_data_transaction_set_method_of_payment_content ( transaction_number,
							     utils_str_itoa ( imported_transaction -> cheque ) );

	list_tmp = gsb_data_account_get_method_payment_list (account_number);
	type_choisi = NULL;

	while ( list_tmp )
	{
	    struct struct_type_ope *type;

	    type = list_tmp -> data;

	    /* si l'opé on trouve un type à incrémentation automatique et que le signe du type est bon, on l'enregistre */
	    /*   et on arrête la recherche, sinon, on l'enregistre mais on continue la recherche dans l'espoir de trouver */
	    /* mieux */

	    if ( type -> numerotation_auto )
	    {
		if ( !type -> signe_type
		     ||
		     ( type -> signe_type == 1 && gsb_data_transaction_get_amount (transaction_number)< 0 )
		     ||
		     ( type -> signe_type == 2 && gsb_data_transaction_get_amount (transaction_number)> 0 ))
		{
		    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
									type -> no_type );
		    type_choisi = type;
		    list_tmp = NULL;
		}
		else
		{
		    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
									type -> no_type );
		    type_choisi = type;
		    list_tmp = list_tmp -> next;
		}
	    }
	    else
		list_tmp = list_tmp -> next;
	}

	/* type_choisi contient l'adr du type qui a été utilisé, on peut y mettre le dernier no de chèque */

	if ( type_choisi )
	    type_choisi -> no_en_cours = MAX ( imported_transaction -> cheque,
					       type_choisi -> no_en_cours );
    }
    else
    {
	/* comme ce n'est pas un chèque, on met sur le type par défaut */

	if ( gsb_data_transaction_get_amount (transaction_number)< 0 )
	    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								gsb_data_account_get_default_debit (account_number));
	else
	    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								gsb_data_account_get_default_credit (account_number));
	
	gsb_data_transaction_set_method_of_payment_content ( transaction_number,
							     utils_str_itoa ( imported_transaction -> cheque ) );

    }

    /* récupération du pointé */

    gsb_data_transaction_set_marked_transaction ( transaction_number,
						  imported_transaction -> p_r );

    /* Various things we have to set. */
    gsb_data_transaction_set_bank_references ( transaction_number, "" );
    gsb_data_transaction_set_voucher ( transaction_number, "" );

    /* si c'est une ope de ventilation, lui ajoute le no de l'opération précédente */

    if ( imported_transaction -> ope_de_ventilation )
	gsb_data_transaction_set_mother_transaction_number ( transaction_number,
							     mother_transaction_number );
    else
	mother_transaction_number  = transaction_number;

    gsb_data_account_set_update_list ( account_number,
				  1 );

    return (transaction_number);
}
/* *******************************************************************************/



/* *******************************************************************************/
void pointe_opes_importees ( struct struct_compte_importation *imported_account )
{
    GSList *list_tmp;
    GSList *liste_opes_import_celibataires;
    gint account_number;


    /* on se place sur le compte dans lequel on va pointer les opés */

    account_number = recupere_no_compte ( imported_account -> bouton_compte_mark );

    /* si le compte importé a une id, on la vérifie ici */
    /*     si elle est absente, on met celle importée */
    /*     si elle est différente, on demande si on la remplace */

    if ( imported_account -> id_compte )
    {
	if ( gsb_data_account_get_id (account_number) )
	{
	    if ( g_strcasecmp ( gsb_data_account_get_id (account_number),
				imported_account -> id_compte ))
	    {
		/* 		l'id du compte choisi et l'id du compte importé sont différents */
		/* 		    on propose encore d'arrêter... */


		if ( question_yes_no_hint ( _("The id of the imported and chosen accounts are different"),
					    _("Perhaps you choose a wrong account ?  If you choose to continue, the id of the account will be changed.  Do you want to continue ?")))
		    gsb_data_account_set_id (account_number,
					my_strdup ( imported_account -> id_compte ));
		else
		    return;
	    }
	}
	else
	    gsb_data_account_set_id (account_number,
				my_strdup ( imported_account -> id_compte ));

    }

    /* on fait le tour des opés importées et recherche dans la liste d'opé s'il y a la correspondance */


    list_tmp = imported_account -> operations_importees;
    liste_opes_import_celibataires = NULL;

    while ( list_tmp )
    {
	GSList *liste_ope;
	GSList *ope_trouvees;
	struct struct_ope_importation *ope_import;
	gpointer operation;
	gint i;
	struct struct_ope_importation *autre_ope_import;

	ope_import = list_tmp -> data;
	ope_trouvees = NULL;
	operation = NULL;

	/* si l'opé d'import a une id, on recherche dans la liste d'opé pour trouver
	   une id comparable */

	if ( ope_import -> id_operation )
	{
	    gpointer ope;

	    ope = operation_par_id ( ope_import -> id_operation,
				     account_number );

	    if ( ope )
		ope_trouvees = g_slist_append ( ope_trouvees,
						ope );
	}

	/* si on n'a rien trouvé par id, */
	/* on fait le tour de la liste d'opés pour trouver des opés comparable */
	/* cad même date avec + ou - une échelle et même montant et pas une opé de ventil */


	if ( !ope_trouvees )
	{
	    GDate *date_debut_comparaison;
	    GDate *date_fin_comparaison;
	    GSList *list_tmp_transactions;

	    date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( ope_import -> date ),
						      g_date_get_month ( ope_import -> date ),
						      g_date_get_year ( ope_import -> date ));
	    g_date_subtract_days ( date_debut_comparaison,
				   valeur_echelle_recherche_date_import );

	    date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( ope_import -> date ),
						    g_date_get_month ( ope_import -> date ),
						    g_date_get_year ( ope_import -> date ));
	    g_date_add_days ( date_fin_comparaison,
			      valeur_echelle_recherche_date_import );

	    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	    while ( list_tmp_transactions )
	    {
		gint transaction_number_tmp;
		transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number )
		{
		    if ( fabs ( gsb_data_transaction_get_amount (transaction_number_tmp)- ope_import -> montant ) < 0.01
			 &&
			 ( g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
					    date_debut_comparaison ) >= 0 )
			 &&
			 ( g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
					    date_fin_comparaison ) <= 0 )

			 &&
			 !gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp))
			/* on a retouvé une opé de même date et même montant, on l'ajoute à la liste des opés trouvées */
			ope_trouvees = g_slist_append ( ope_trouvees,
							operation );
		}
		list_tmp_transactions = list_tmp_transactions -> next;
	    }
	}
	/*       à ce stade, ope_trouvees contient la ou les opés qui sont comparables à l'opé importée */
	/* soit il n'y en n'a qu'une, et on la pointe, soit il y en a plusieurs, et on recherche dans */
	/* 	les opés importées s'il y en a d'autre comparables, et on pointe les opés en fonction */
	/* du nb de celles importées */

	switch ( g_slist_length ( ope_trouvees ))
	{
	    case 0:
		/* aucune opé comparable n'a été retrouvée */
		/* on marque donc cette opé comme seule */
		/* sauf si c'est une opé de ventil  */

		if ( !ope_import -> ope_de_ventilation ) 
		{
		    /* on met le no de compte et la devise de l'opération si plus tard on l'enregistre */

		    ope_import -> no_compte = account_number;
		    ope_import -> devise = gsb_currency_get_currency_from_combobox (imported_account -> bouton_devise);
		    liste_opes_import_celibataires = g_slist_append ( liste_opes_import_celibataires,
								      ope_import );
		}

		break;

	    case 1:
		/*  il n'y a qu'une opé retrouvée, on la pointe */
		/* si elle est déjà pointée ou relevée, on ne fait rien */
		/* si l'opé d'import a une id et pas l'opé, on marque l'id dans l'opé */

		operation = ope_trouvees -> data;

		if ( !gsb_data_transaction_get_transaction_id ( gsb_data_transaction_get_transaction_number (operation))
		     &&
		     ope_import -> id_operation )
		    gsb_data_transaction_set_transaction_id ( gsb_data_transaction_get_transaction_number (operation),
							      ope_import -> id_operation );

		if ( !gsb_data_transaction_get_marked_transaction ( gsb_data_transaction_get_transaction_number (operation )))
		{
		    gsb_data_transaction_set_marked_transaction ( gsb_data_transaction_get_transaction_number (operation ),
								  2 );
		    gsb_data_account_set_update_list ( account_number,
						  1 );

		    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

		    if ( gsb_data_transaction_get_breakdown_of_transaction ( gsb_data_transaction_get_transaction_number (operation )))
		    {
			GSList *list_tmp_transactions;

			list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

			while ( list_tmp_transactions )
			{
			    gint transaction_number_tmp;
			    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

			    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number )
			    {
				if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == gsb_data_transaction_get_transaction_number (operation))
				    gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
										  2 );
			    }
			    list_tmp_transactions = list_tmp_transactions -> next;
			}
		    }
		}
		break;

	    default: 	   
		/* il y a plusieurs opé trouvées correspondant à l'opé importée */

		/* on va voir s'il y a d'autres opées importées ayant la même date et le même montant
		   si on retrouve autant d'opé importées que d'opé trouvées, on peut marquer cette
		   opé sans s'en préoccuper */


		i=0;
		liste_ope = imported_account -> operations_importees;

		while ( liste_ope )
		{
		    GDate *date_debut_comparaison;
		    GDate *date_fin_comparaison;

		    autre_ope_import = liste_ope -> data;
		    date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( autre_ope_import -> date ),
							      g_date_get_month ( autre_ope_import -> date ),
							      g_date_get_year ( autre_ope_import -> date ));
		    g_date_subtract_days ( date_debut_comparaison,
					   valeur_echelle_recherche_date_import );

		    date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( autre_ope_import -> date ),
							    g_date_get_month ( autre_ope_import -> date ),
							    g_date_get_year ( autre_ope_import -> date ));
		    g_date_add_days ( date_fin_comparaison,
				      valeur_echelle_recherche_date_import );


		    if ( fabs ( autre_ope_import -> montant - ope_import -> montant ) < 0.01
			 &&
			 ( g_date_compare ( gsb_data_transaction_get_date (gsb_data_transaction_get_transaction_number (operation)),
					    date_debut_comparaison ) >= 0 )
			 &&
			 ( g_date_compare ( gsb_data_transaction_get_date (gsb_data_transaction_get_transaction_number (operation)),
					    date_fin_comparaison ) <= 0 )

			 &&
			 !autre_ope_import -> ope_de_ventilation )
			/* on a retouvé une opé d'import de même date et même montant, on incrémente le nb d'opé d'import semblables trouvees */
			i++;

		    liste_ope = liste_ope -> next;
		}

		if ( i ==  g_slist_length ( ope_trouvees ))
		{
		    /* on a trouvé autant d'opé d'import semblables que d'opés semblables dans la liste d'opé
		       donc on peut marquer les opés trouvées */
		    /* pour celles qui sont déjà pointées, on ne fait rien */
		    /* si l'opé importée à une id, on met cette id dans l'opération si elle n'en a pas */

		    GSList *list_tmp_2;

		    list_tmp_2 = ope_trouvees;

		    while ( list_tmp_2 )
		    {
			operation = list_tmp_2 -> data;

			if ( !gsb_data_transaction_get_transaction_id ( gsb_data_transaction_get_transaction_number (operation))
			     &&
			     ope_import -> id_operation )
			    gsb_data_transaction_set_transaction_id ( gsb_data_transaction_get_transaction_number (operation),
								      ope_import -> id_operation );

			if ( !gsb_data_transaction_get_marked_transaction ( gsb_data_transaction_get_transaction_number (operation )))
			{
			    gsb_data_transaction_set_marked_transaction ( gsb_data_transaction_get_transaction_number (operation ),
									  2 );
			    gsb_data_account_set_update_list ( account_number,
							  1 );

			    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

			    if ( gsb_data_transaction_get_breakdown_of_transaction ( gsb_data_transaction_get_transaction_number (operation )))
			    {
				GSList *list_tmp_transactions;

				list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

				while ( list_tmp_transactions )
				{
				    gint transaction_number_tmp;
				    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

				    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number )
				    {
					if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == gsb_data_transaction_get_transaction_number (operation))
					    gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
											  2 );
				    }
				    list_tmp_transactions = list_tmp_transactions -> next;
				}
			    }
			}
			list_tmp_2 = list_tmp_2 -> next;
		    }
		}
		else
		{
		    /* on a trouvé un nombre différent d'opés d'import et d'opés semblables dans la liste d'opés
		       on marque donc cette opé d'import comme seule */

		    ope_import -> no_compte = account_number;
		    ope_import -> devise = gsb_currency_get_currency_from_combobox (imported_account -> bouton_devise);
		    liste_opes_import_celibataires = g_slist_append ( liste_opes_import_celibataires,
								      ope_import );

		}
	}
	list_tmp = list_tmp -> next;
    }

    /* a ce niveau, liste_opes_import_celibataires contient les opés d'import dont on n'a pas retrouvé
       l'opé correspondante
       on les affiche dans une liste en proposant de les ajouter à la liste */

    if ( liste_opes_import_celibataires )
    {
	GtkWidget *liste_ope_celibataires, *dialog, *label, *scrolled_window;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;



	dialog = gtk_dialog_new_with_buttons ( _("Orphaned transactions"),
					       GTK_WINDOW ( window ),
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       GTK_STOCK_APPLY, 1,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_OK, GTK_RESPONSE_OK,
					       NULL );

	label = gtk_label_new ( _("Mark transactions you want to add to the list and click the add button"));
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( label );

	store = gtk_list_store_new ( 4,
				     G_TYPE_BOOLEAN,
				     G_TYPE_STRING,
				     G_TYPE_STRING,
				     G_TYPE_DOUBLE );

	/* on remplit la liste */

	list_tmp = liste_opes_import_celibataires;

	while ( list_tmp ) 
	{
	    struct struct_ope_importation *ope_import;
	    GtkTreeIter iter;

	    ope_import = list_tmp -> data;

	    gtk_list_store_append ( store,
				    &iter );

	    gtk_list_store_set ( store,
				 &iter,
				 0, FALSE,
				 1, gsb_format_gdate ( ope_import -> date ),
				 2, ope_import -> tiers,
				 3, ope_import -> montant,
				 -1 );

	    list_tmp = list_tmp -> next;
	}

	/* on crée la liste des opés célibataires
	   et on y associe la gslist */

	liste_ope_celibataires = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (store));
	g_object_set_data ( G_OBJECT ( liste_ope_celibataires ),
			    "liste_ope",
			    liste_opes_import_celibataires );
	scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
	gtk_widget_set_usize ( scrolled_window, FALSE, 300 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     scrolled_window,
			     TRUE,
			     TRUE,
			     0 );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					 GTK_POLICY_AUTOMATIC,
					 GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
						liste_ope_celibataires );
	gtk_widget_show_all ( scrolled_window );

	/* on affiche les colonnes */

	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect ( renderer,
			   "toggled",
			   G_CALLBACK (click_sur_liste_opes_orphelines ),
			   store );
	column = gtk_tree_view_column_new_with_attributes ( _("Mark"),
							    renderer,
							    "active", 0,

							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ( _("Date"),
							    renderer,
							    "text", 1,
							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ( _("Payee"),
							    renderer,
							    "text", 2,
							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);


	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ( _("Amount"),
							    renderer,
							    "text", 3,
							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	g_signal_connect ( G_OBJECT ( dialog ),
			   "response",
			   G_CALLBACK ( click_dialog_ope_orphelines ),
			   liste_ope_celibataires );

	gtk_widget_show ( dialog );
    }
}
/* *******************************************************************************/


/* *******************************************************************************/
gboolean click_dialog_ope_orphelines ( GtkWidget *dialog,
				       gint result,
				       GtkWidget *liste_ope_celibataires )
{
    GSList *liste_opes_import_celibataires;
    GSList *list_tmp;
    GtkTreeIter iter;
    GtkTreeModel *model;

    switch ( result )
    {
	case 1:
	case GTK_RESPONSE_OK:
	    /* on ajoute la ou les opés marquées à la liste d'opés en les pointant d'un T
	       puis on les retire de la liste des orphelines
	       s'il ne reste plus d'opés orphelines, on ferme la boite de dialogue */

	    liste_opes_import_celibataires = g_object_get_data ( G_OBJECT ( liste_ope_celibataires ),
								 "liste_ope" );
	    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( liste_ope_celibataires ));
	    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					    &iter );

	    list_tmp = liste_opes_import_celibataires;

	    /* normalement, pas besoin de mettre ça à 0 car normalement pas de ventilations à ce stade... */

	    mother_transaction_number = 0;

	    while ( list_tmp )
	    {
		gboolean enregistre;
		GSList *last_item;

		gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				     &iter,
				     0, &enregistre,
				     -1 );

		if ( enregistre )
		{
		    /* à ce niveau, l'opé a été cochée donc on l'enregistre en la marquant T	 */

		    struct struct_ope_importation *ope_import;
		    gint transaction_number;

		    ope_import = list_tmp -> data;

		    transaction_number = gsb_import_create_transaction ( ope_import,
									 ope_import -> no_compte );
		    gsb_data_transaction_set_marked_transaction ( transaction_number,
								  2 );

		    /* on a enregistré l'opé, on la retire maintenant de la liste et de la sliste */

		    last_item = list_tmp;
		    list_tmp = list_tmp -> next;
		    liste_opes_import_celibataires = g_slist_remove_link ( liste_opes_import_celibataires,
									   last_item );

		    /* on retire la ligne qu'on vient d'enregistrer, celà met l'iter directement sur la suite */

		    gtk_list_store_remove ( GTK_LIST_STORE ( model),
					    &iter );
		}
		else
		{
		    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					       &iter );
		    list_tmp = list_tmp -> next;
		}
	    }

	    /* on enregistre la nouvelle liste d'opé pour la retrouver plus tard */

	    g_object_set_data ( G_OBJECT ( liste_ope_celibataires ),
				"liste_ope",
				liste_opes_import_celibataires );

	    /* il est possible que les opés importées soient des virements, il faut faire les
	       relations ici */
	    if ( virements_a_chercher )
		cree_liens_virements_ope_import ();

	    /* on recrée les combofix des tiers et des catégories */

	    if ( mise_a_jour_combofix_tiers_necessaire )
		mise_a_jour_combofix_tiers ();
	    if ( mise_a_jour_combofix_categ_necessaire )
		mise_a_jour_combofix_categ();

	    /* mise à jour de l'accueil */

	    mise_a_jour_liste_comptes_accueil = 1;
	    mise_a_jour_soldes_minimaux = 1;

	    modification_fichier ( TRUE );

	    if ( result != GTK_RESPONSE_OK
		 &&
		 g_slist_length ( liste_opes_import_celibataires ))
		break;

	default:
	    gtk_widget_destroy ( dialog );
	    break;
    }

    return ( FALSE );
}
/* *******************************************************************************/


/* *******************************************************************************/
gboolean click_sur_liste_opes_orphelines ( GtkCellRendererToggle *renderer, 
					   gchar *ligne,
					   GtkTreeModel *store )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( store ),
					       &iter,
					       ligne ))
    {
	gboolean valeur;

	gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
			     &iter,
			     0, &valeur,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( store ),
			     &iter,
			     0, 1 - valeur,
			     -1 );
    }
    return ( FALSE );
}		



/**
 *
 *
 */
gchar * unique_imported_name ( gchar * account_name )
{
    GSList * tmp_list = liste_comptes_importes;
    gchar * basename = account_name;
    gint iter = 1;

    if ( ! liste_comptes_importes )
    {
	return account_name;
    }

    do
    {
	struct struct_compte_importation * tmp_account;

	tmp_account = (struct struct_compte_importation *) tmp_list -> data;

	if ( !strcmp ( basename, tmp_account -> nom_de_compte ) )
	{
	    tmp_list = liste_comptes_importes;
	    basename = g_strdup_printf ( _("%s #%d"), account_name, ++iter );
	}
	else
	{
	    tmp_list = tmp_list -> next;
	}
    }
    while ( tmp_list );
    
    return basename;
}



/* *******************************************************************************/
/* page de configuration pour l'importation */
/* *******************************************************************************/
GtkWidget *onglet_importation (void)
{
    GtkWidget *vbox_pref;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *bouton;

    vbox_pref = new_vbox_with_title_and_icon ( _("Import"),
					       "files.png" ); 
    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Search the transaction "));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    bouton = gtk_spin_button_new_with_range ( 0.0,
					      100.0,
					      1.0);
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( bouton ),
				valeur_echelle_recherche_date_import );
    g_signal_connect ( G_OBJECT ( bouton ),
		       "value-changed",
		       G_CALLBACK ( changement_valeur_echelle_recherche_date_import ),
		       NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    label = gtk_label_new ( _("days around the date in the imported transaction."));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    if ( ! assert_account_loaded() )
      gtk_widget_set_sensitive ( vbox_pref, FALSE );

    return ( vbox_pref );
}
/* *******************************************************************************/


/* *******************************************************************************/
gboolean changement_valeur_echelle_recherche_date_import ( GtkWidget *spin_button )
{
    valeur_echelle_recherche_date_import = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button ));
    modification_fichier ( TRUE );
    return ( FALSE );
}



enum import_type autodetect_file_type ( gchar * filename, FILE * fichier, 
					gchar * pointeur_char )
{
    gchar * extension;
    enum import_type type = TYPE_UNKNOWN;

    extension = strrchr ( filename, '.' );
    if ( extension )
    {
	if ( !strcasecmp ( extension + 1, "csv" ) )
	{
	    return TYPE_CSV;
	}
	if ( !strcasecmp ( extension + 1, "ofx" ) )
	{
	    return TYPE_OFX;
	}
	if ( !strcasecmp ( extension + 1, "gnc" ) )
	{
	    return TYPE_GNUCASH;
	}
	if ( !strcasecmp ( extension + 1, "gnucash" ) )
	{
	    return TYPE_GNUCASH;
	}
	if ( !strcasecmp ( extension + 1, "qif" ) )
	{
	    return TYPE_QIF;
	}
    }
    
    if ( ! pointeur_char )
    {
	return TYPE_UNKNOWN;
    }
    
    if ( g_strrstr ( pointeur_char, "ofx" ) || g_strrstr ( pointeur_char, "OFX" ))
    {
	type = TYPE_OFX;
    }
    else if ( !my_strncasecmp ( pointeur_char, "!Type", 5 ) ||
	      !my_strncasecmp ( pointeur_char, "!Account", 8 ) || 
	      !my_strncasecmp ( pointeur_char, "!Option", 7 ))
    {
	type = TYPE_QIF;
    }
    else
    {
	if ( !strncmp ( pointeur_char, "<?xml", 5 ) &&
	     fichier != NULL )
	{
	    get_line_from_file ( fichier, &pointeur_char );
	    if ( !strncmp ( pointeur_char, "<gnc-v2", 7 ))
	    {
		type = TYPE_GNUCASH;
	    }
	    else
	    {
		type = TYPE_UNKNOWN;
	    }
	}
	else
	{
	    type = TYPE_UNKNOWN;
	}
    }

  return type;
}



/**
 *
 *
 *
 */
gchar * type_string_representation ( enum import_type type )
{
    switch ( type )
    {
	case TYPE_OFX:
	    return _("OFX file");

	case TYPE_QIF:
	    return _("QIF file");

	case TYPE_CSV:
	    return _("CSV file");

	case TYPE_GNUCASH:
	    return _("Gnucash file");

	case TYPE_GBANKING:
	    return _("HBCI import");

	default:
	    return _("Unknown format");
    }
    
    return _("Unknown format");
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
