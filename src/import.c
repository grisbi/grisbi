/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2008 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "./utils.h"
#include "./import_csv.h"
#include "./dialog.h"
#include "./utils_file_selection.h"
#include "./go-charmap-sel.h"
#include "./gsb_account.h"
#include "./gsb_account_property.h"
#include "./gsb_assistant.h"
#include "./gsb_assistant_file.h"
#include "./gsb_automem.h"
#include "./utils_buttons.h"
#include "./gsb_combo_box.h"
#include "./gsb_currency.h"
#include "./gsb_data_account.h"
#include "./gsb_data_category.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_form.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_import_rule.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file.h"
#include "./gsb_form_scheduler.h"
#include "./utils_dates.h"
#include "./fenetre_principale.h"
#include "./navigation.h"
#include "./menu.h"
#include "./tiers_onglet.h"
#include "./gsb_real.h"
#include "./gsb_status.h"
#include "./gsb_transactions_list.h"
#include "./traitement_variables.h"
#include "./main.h"
#include "./accueil.h"
#include "./utils_str.h"
#include "./qif.h"
#include "./transaction_list.h"
#include "./structures.h"
#include "./gsb_transactions_list.h"
#include "./go-charmap-sel.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_account.h"
#include "./gsb_form_scheduler.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean affichage_recapitulatif_importation ( GtkWidget * assistant );
static const gchar * autodetect_file_type ( gchar * filename,
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
static gchar **gsb_import_by_rule_ask_filename ( gint rule );
static gboolean gsb_import_by_rule_get_file ( GtkWidget *button,
				       GtkWidget *entry );
static GSList *gsb_import_create_file_chooser (void);
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
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *menu_import_rules;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget *window ;
/*END_EXTERN*/


/** Suppported import formats.  Plugins may register themselves. */
static GSList * import_formats = NULL;

/** Known built-in import formats.  Others are plugins.  */
struct import_format builtin_formats[] = {
{ "CSV", "Comma Separated Values",     "csv", (import_function) csv_import_csv_account },
{ "QIF", "Quicken Interchange Format", "qif", (import_function) recuperation_donnees_qif },
{ NULL,  NULL,				NULL,		NULL },
};


/** used to keep the number of the mother transaction while importing breakdown transactions */
static gint mother_transaction_number;
gint valeur_echelle_recherche_date_import;
GSList *liste_comptes_importes;
GSList *liste_comptes_importes_error;
static gint virements_a_chercher;

static GtkWidget *go_charmap_sel;

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



/**
 * Register built-in import formats as known.
 */
void register_import_formats ()
{
    gint i;
    for ( i = 0; builtin_formats [ i ] . name != NULL ; i ++ )
    {
	register_import_format ( &builtin_formats [ i ] );
    }
}



/**
 * Register a known import format as known.
 *
 * \param format		A pointer to a structure describing
 *				this import format.
 *  
 */
void register_import_format ( struct import_format * format )
{
    gchar* tmpstr = g_strdup_printf ( "Adding '%s' as an import format", format -> name );
    devel_debug ( tmpstr );
    g_free ( tmpstr );
    import_formats = g_slist_append ( import_formats, format );
}



/**
 *
 *
 *
 */
void importer_fichier ( void )
{
    GSList * tmp = import_formats;
    gchar * formats = g_strdup("");
    GtkWidget * assistant;

    /* if nothing opened, we need to create a new file to set up all the variables */
    if (!gsb_data_currency_get_currency_list ())
    {
	init_variables ();
	gsb_assistant_file_run (FALSE, TRUE);
	return;
    }

    liste_comptes_importes = NULL;
    liste_comptes_importes_error = NULL;
    virements_a_chercher = 0;

    while ( tmp )
    {
	struct import_format * format = (struct import_format *) tmp -> data;
	gchar* old_str = formats;
	formats = g_strconcat ( formats, 
				"	• ",
				format -> complete_name,
				" (", format -> name, ")\n",
				NULL );
	g_free ( old_str );
	tmp = tmp -> next;
    }

    gchar* tmpstr = g_strconcat ( "This assistant will help you import one or several files into Grisbi."
				  "\n\n"
				  "Grisbi will try to do its best to guess which format are imported, but you may have to manually set them in the list of next page.  "
				  "So far, the following formats are supported:"
				  "\n\n",
				  formats, NULL );
    assistant = gsb_assistant_new ( "Importing transactions into Grisbi",
			    tmpstr,
			    "csv.png",
			    NULL );
    g_free (formats);
    g_free (tmpstr);

    gsb_assistant_add_page ( assistant, import_create_file_selection_page ( assistant ), 
			     IMPORT_FILESEL_PAGE, IMPORT_STARTUP_PAGE, IMPORT_RESUME_PAGE, 
			     G_CALLBACK ( import_enter_file_selection_page ) );
    gsb_assistant_add_page ( assistant, import_create_csv_preview_page ( assistant ), 
			     IMPORT_CSV_PAGE, IMPORT_FILESEL_PAGE, IMPORT_RESUME_PAGE,
			     G_CALLBACK ( import_enter_csv_preview_page ) );
    gsb_assistant_add_page ( assistant, import_create_resume_page ( assistant ), 
			     IMPORT_RESUME_PAGE, IMPORT_FILESEL_PAGE, 
			     IMPORT_FIRST_ACCOUNT_PAGE, 
			     G_CALLBACK ( import_enter_resume_page ) );

    if ( gsb_assistant_run ( assistant ) == GTK_RESPONSE_APPLY )
    {
	gsb_status_wait ( TRUE );
	traitement_operations_importees ();
	gtk_widget_destroy ( assistant );
	gsb_status_stop_wait ( TRUE );
    }
    else 
    {
	gtk_widget_destroy ( assistant );
    }
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
    GSList * tmp;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    paddingbox = new_paddingbox_with_title ( vbox, TRUE, _("Choose file to import"));
    
    chooser = gtk_button_new_with_label ( _("Add file to import..." ));
    gchar* tmpstr = g_build_filename ( PIXMAPS_DIR,
				       "import.png", NULL );
    gtk_button_set_image ( GTK_BUTTON(chooser), 
			   gtk_image_new_from_file ( tmpstr ) );
    g_free ( tmpstr );
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
						  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING));
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

    tmp = import_formats;
    while ( tmp )
    {
	GtkTreeIter iter; 
	struct import_format * format = (struct import_format *) tmp -> data;

	gtk_list_store_append (GTK_LIST_STORE (list_acc), &iter);
	gtk_list_store_set (GTK_LIST_STORE (list_acc), &iter, 0, 
			    format -> name, -1);

	tmp = tmp -> next;
    }

    g_object_set ( renderer, 
		   "model", list_acc, 
		   "text-column", 0, 
		   "editable", TRUE, 
		   "editable-set", FALSE, 
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
     GtkWidget * assistant;

     assistant = g_object_get_data ( G_OBJECT (model), "assistant" );

     if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( model ), &iter, path ))
     {
	 GSList * tmp = import_formats;

	 while ( tmp )
	 {
	     struct import_format * format = (struct import_format *) tmp -> data;

	     if ( ! strcmp ( value, format -> name ) )
	     {
		 gtk_tree_store_set ( GTK_TREE_STORE (model), &iter,
				      IMPORT_FILESEL_TYPENAME, value, 
				      IMPORT_FILESEL_TYPE, format -> name, 
				      -1 );
		 
		 /* CSV is special because it needs configuration, so
		  * we add a conditional jump there. */
		 if ( ! strcmp ( value, "CSV" ) )
		 {
		     gsb_assistant_set_next ( assistant, IMPORT_FILESEL_PAGE, 
					      IMPORT_CSV_PAGE );
		     gsb_assistant_set_prev ( assistant, IMPORT_RESUME_PAGE, 
					      IMPORT_CSV_PAGE );
		 }

		 import_preview_maybe_sensitive_next ( assistant, GTK_TREE_MODEL ( model ));
	     }

	     tmp = tmp -> next;
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
	gchar * type;

	gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 
			     IMPORT_FILESEL_SELECTED, &selected, 
			     IMPORT_FILESEL_TYPE, &type,
			     -1 );
	if ( selected && strcmp ( type, _("Unknown") ) )
	{
	    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant), 
							   "button_next" ), 
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
    GSList * filenames, * iterator;
    GtkTreeModel * model;

    filenames = gsb_import_create_file_chooser ();
    if (!filenames)
	return FALSE;

    iterator = filenames;

    model = g_object_get_data ( G_OBJECT ( assistant ), "model" );

    while ( iterator && model )
    {
	GtkTreeIter iter;
	const gchar * type;
	gchar * pointeur_char;
	GError * error;

	/* Open file */
	if ( ! g_file_get_contents ( iterator -> data, &pointeur_char, NULL, &error ) )
	{
	    g_print ("Unable to read file: %s\n", error -> message);
	    return FALSE;
	}

	type = autodetect_file_type ( iterator -> data, pointeur_char );
	g_free (pointeur_char);

	gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, NULL );
	gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter, 
			     IMPORT_FILESEL_SELECTED, TRUE,
			     IMPORT_FILESEL_TYPENAME, type,
			     IMPORT_FILESEL_FILENAME, g_path_get_basename ( iterator -> data ),
			     IMPORT_FILESEL_REALNAME, iterator -> data,
			     IMPORT_FILESEL_TYPE, type,
			     IMPORT_FILESEL_CODING, go_charmap_sel_get_encoding ( (GOCharmapSel * )go_charmap_sel ),
			     -1 ); 

	/* CSV is special because it needs configuration, so we
	 * add a conditional jump there. */
	if ( ! strcmp ( type, "CSV" ) )
	{
	    gsb_assistant_set_next ( assistant, IMPORT_FILESEL_PAGE, IMPORT_CSV_PAGE );
	    gsb_assistant_set_prev ( assistant, IMPORT_RESUME_PAGE, IMPORT_CSV_PAGE );
	}

	if ( strcmp ( type, _("Unknown") ) )
	{
	    /* A valid file was selected, so we can now go ahead. */
	    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant), "button_next" ), 
				       TRUE );
	}

	iterator = iterator -> next;
    }

    if ( filenames )
	g_slist_free ( filenames );
    return FALSE;
}


/**
 * create an appropriate dialog file chooser
 * for importing files to grisbi and return the selected files
 *
 * \param
 *
 * \return a GtkFileChooser
 * */
GSList *gsb_import_create_file_chooser (void)
{
    GtkWidget * dialog, * hbox, * go_charmap_sel;
    GtkFileFilter * filter, * default_filter;
    gchar * files;
    GSList * tmp;
    struct import_format * format;
    GSList *filenames = NULL;

    dialog = gtk_file_chooser_dialog_new ( _("Choose files to import."),
					   NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					   NULL );
    gtk_file_chooser_set_select_multiple ( GTK_FILE_CHOOSER ( dialog ), TRUE );
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
					 gsb_file_get_last_path ());

    /* Import filters */
    tmp = import_formats;
    format = (struct import_format *) tmp -> data;
    files = g_strconcat ( "*.", format -> extension, NULL );
    tmp = tmp -> next;

    while ( tmp )
    {
	format = (struct import_format *) tmp -> data;
	gchar* old_str = files;
	files = g_strconcat ( files, ", *.", format -> extension, NULL );
	g_free ( old_str );
	tmp = tmp -> next;
    }

    default_filter = gtk_file_filter_new ();
    gchar* tmpstr = g_strdup_printf ( _("Known files (%s)"), files );
    gtk_file_filter_set_name ( default_filter, tmpstr );
    g_free ( tmpstr );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), default_filter );
    gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( dialog ), default_filter );

    tmp = import_formats;
    while ( tmp )
    {
	GtkFileFilter * format_filter;	

	format = (struct import_format *) tmp -> data;

	format_filter = gtk_file_filter_new ();
	gchar* tmpstr = g_strdup_printf ( _("%s files (*.%s)"),
						     format -> name, 
						     format -> extension );
	gtk_file_filter_set_name ( format_filter, tmpstr );
	g_free ( tmpstr );
	tmpstr = g_strconcat ( "*.", format -> extension, NULL );
	gtk_file_filter_add_pattern ( format_filter, 
				      tmpstr );
	g_free ( tmpstr );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), format_filter );

	/* Global filter */
	tmpstr = g_strconcat ( "*.", format -> extension, NULL );
	gtk_file_filter_add_pattern ( default_filter, tmpstr );
	g_free ( tmpstr );

	tmp = tmp -> next;
    }

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("All files") );
    gtk_file_filter_add_pattern ( filter, "*" );

    /* Add encoding preview */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_file_chooser_set_extra_widget ( GTK_FILE_CHOOSER ( dialog ), hbox );
    gtk_box_pack_start ( GTK_BOX ( hbox ), gtk_label_new ( COLON(_("Encoding")) ), 
			 FALSE, FALSE, 0 );
    go_charmap_sel = go_charmap_sel_new (GO_CHARMAP_SEL_TO_UTF8);
    gtk_box_pack_start ( GTK_BOX ( hbox ), go_charmap_sel, TRUE, TRUE, 0 );
    gtk_widget_show_all ( hbox );

    if ( gtk_dialog_run ( GTK_DIALOG (dialog ) ) == GTK_RESPONSE_ACCEPT )
	filenames = gtk_file_chooser_get_filenames ( GTK_FILE_CHOOSER (dialog));

    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE));
    gtk_widget_destroy (dialog);
    return filenames;
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
    GtkTextBuffer * buffer;
    GtkTextIter iter;
    gchar * error_message = "";

    liste_comptes_importes_error = NULL;
    liste_comptes_importes = NULL;

    while ( files )
    {
	struct imported_file * imported = files -> data;
	GSList * tmp = import_formats;

	while ( tmp )
	{
	    struct import_format * format = (struct import_format *) tmp -> data;

	    if ( !strcmp ( imported -> type, format -> name ) )
	    {
		format -> import ( assistant, imported );
		tmp = tmp -> next;
		continue;
	    }

	    tmp = tmp -> next;
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
				COLON ( _("You successfully imported files into Grisbi.  The following pages will help you set up imported data for the following files") ), 
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

	    gchar* tmpstr = g_strconcat ( "• ", compte -> nom_de_compte,
						" (", 
						compte -> origine,
						")\n\n", 
						NULL );
	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, 
						      tmpstr ,
						      -1, "indented", NULL );
	    g_free ( tmpstr );

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
	if ( strlen ( error_message ) )
	{
	    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1 );
	    gtk_text_buffer_insert (buffer, &iter, error_message, -1 );
	}
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

	    gchar* tmpstr = g_strconcat ( "• ", compte -> nom_de_compte,
						" (", 
						compte -> origine,
						")\n\n", 
						NULL );
	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, 
						      tmpstr,
						      -1, "indented", NULL );
	    g_free ( tmpstr );

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
 * this is the summary page of the import assistant
 * show what accounts will be imported in grisbi
 * and create the next pages of the assistant, one per account
 *
 * \param assistant
 *
 * \return  FALSE
 */
gboolean affichage_recapitulatif_importation ( GtkWidget * assistant )
{
    gint page;
    GSList *list_tmp;

    /* FIXME */
    if (!assistant)
	return FALSE;

    /* Initial page is fourth. */
    page = IMPORT_FIRST_ACCOUNT_PAGE;

    /* First, iter to see if we need to create currencies */
    list_tmp = liste_comptes_importes;
    while ( list_tmp )
    {
	struct struct_compte_importation * compte;
	compte = list_tmp -> data;

	/* add one page per account */
	gsb_assistant_add_page ( assistant, cree_ligne_recapitulatif ( list_tmp -> data ), 
				 page, page - 1, page + 1, G_CALLBACK ( NULL ) );
	page ++;

	list_tmp = list_tmp -> next;
    }

    /* And final page */
    gsb_assistant_add_page ( assistant, import_create_final_page ( assistant ), 
			     page, page - 1, -1, G_CALLBACK ( NULL ) );

    /* Replace button. */
    gsb_assistant_change_button_next ( assistant, GTK_STOCK_GO_FORWARD, GTK_RESPONSE_YES );

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
    gint account_number;
    GtkWidget *button;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    if ( compte -> filename )
	short_filename = g_path_get_basename (compte -> filename);
    else
    {
	if (compte -> real_filename)
	    short_filename = g_path_get_basename (compte -> real_filename);
	else
	    short_filename = g_strdup (_("file"));
    }

    label = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gchar* tmpstr = g_strdup_printf ( _("<span size=\"x-large\">%s</span>\n\n"
					       "What do you want to do with contents from <tt>%s</tt>?\n"),
					     compte -> nom_de_compte, short_filename );
    gtk_label_set_markup ( GTK_LABEL ( label ), tmpstr );
    g_free ( tmpstr );
    g_free (short_filename);
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 0 );

    /* New account */
    radio = gtk_radio_button_new_with_label ( NULL, _("Create a new account") );
    radiogroup = radio;
    gtk_box_pack_start ( GTK_BOX ( vbox ), radio, FALSE, FALSE, 0 );
    gtk_widget_style_get (radio, "indicator_size", &size, NULL);
    gtk_widget_style_get (radio, "indicator_spacing", &spacing, NULL);

    compte -> hbox1 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), compte -> hbox1, FALSE, FALSE, 0 );
    label = gtk_label_new ( COLON ( _("Account type") ) );
    alignement = gtk_alignment_new ( 0.5, 0.5, 1, 1 );
    gtk_container_set_border_width ( GTK_CONTAINER ( alignement ), 2 );
    gtk_alignment_set_padding ( GTK_ALIGNMENT ( alignement ), 0, 0, 2 * spacing + size, 0 );
    gtk_container_add ( GTK_CONTAINER ( alignement ), label );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox1 ), alignement, FALSE, FALSE, 0 );

    compte -> bouton_type_compte = gsb_combo_box_new_with_index_by_list ( gsb_account_property_create_combobox_list (),
									  NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox1 ), compte -> bouton_type_compte, 
			 TRUE, TRUE, 0 );

    /* at this level imported_account -> type_de_compte was filled while importing transactions,
     * in qif.c or ofx.c ; but we have only 4 kind of account for now, so try to place the combobox correctly
     * and it's the user who will choose the kind of account */
    switch (compte -> type_de_compte)
    {
	case 3:
	    gsb_combo_box_set_index ( compte -> bouton_type_compte,
				      GSB_TYPE_LIABILITIES );
	    break;

	case 7:
	    gsb_combo_box_set_index ( compte -> bouton_type_compte,
				      GSB_TYPE_CASH );
	    break;

	default:
	    gsb_combo_box_set_index ( compte -> bouton_type_compte,
				      GSB_TYPE_BANK );
    }


    g_object_set_data ( G_OBJECT ( radio ), "associated", compte -> hbox1 );
    g_object_set_data ( G_OBJECT ( radio ), "account", compte );
    g_signal_connect ( G_OBJECT ( radio ), "toggled", 
		       G_CALLBACK ( import_account_action_activated ), IMPORT_CREATE_ACCOUNT );


    /* Add to account */
    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( radiogroup ), 
							  _("Add transactions to an account") );
    gtk_box_pack_start ( GTK_BOX ( vbox ), radio, FALSE, FALSE, 0 );
    gtk_widget_set_sensitive  ( radio, assert_account_loaded ( ) );

    compte -> hbox2 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), compte -> hbox2, FALSE, FALSE, 0 );
    label = gtk_label_new ( COLON ( _("Account name") ) );
    alignement = gtk_alignment_new ( 0.5, 0.5, 1, 1 );
    gtk_container_set_border_width ( GTK_CONTAINER ( alignement ), 2 );
    gtk_alignment_set_padding ( GTK_ALIGNMENT ( alignement ), 0, 0, 2 * spacing + size, 0 );
    gtk_container_add ( GTK_CONTAINER ( alignement ), label );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox2 ), alignement, FALSE, FALSE, 0 );

    compte -> bouton_compte_add = gsb_account_create_combo_list (NULL, NULL, FALSE);
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox2 ), compte -> bouton_compte_add, TRUE, TRUE, 0 );
    gtk_widget_set_sensitive ( compte -> hbox2, FALSE );

    g_object_set_data ( G_OBJECT ( radio ), "associated", compte -> hbox2 );
    g_object_set_data ( G_OBJECT ( radio ), "account", compte );
    g_signal_connect ( G_OBJECT ( radio ), "toggled", 
		       G_CALLBACK ( import_account_action_activated ), GINT_TO_POINTER (IMPORT_ADD_TRANSACTIONS));

    /* set on the right account, (Yoann) */
    account_number = gsb_data_account_get_account_by_id (compte->id_compte);
    if(account_number >= 0)
    {
	g_object_set_data ( G_OBJECT ( radio ), "associated", compte -> hbox2 );
	g_object_set_data ( G_OBJECT ( radio ), "account", compte );	
	import_account_action_activated(radio,IMPORT_ADD_TRANSACTIONS);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
	gsb_account_set_combo_account_number (compte -> bouton_compte_add, account_number);
	gsb_account_set_combo_account_number (compte -> bouton_compte_mark, account_number);
    }

    /* Mark account */
    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( radiogroup ), 
							  _("Mark transactions of an account") );
    gtk_box_pack_start ( GTK_BOX ( vbox ), radio, FALSE, FALSE, 0 );
    gtk_widget_set_sensitive  ( radio, assert_account_loaded ( ) );

    compte -> hbox3 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), compte -> hbox3, FALSE, FALSE, 0 );
    label = gtk_label_new ( COLON ( _("Account name") ) );
    alignement = gtk_alignment_new ( 0.5, 0.5, 1, 1 );
    gtk_container_set_border_width ( GTK_CONTAINER ( alignement ), 2 );
    gtk_alignment_set_padding ( GTK_ALIGNMENT ( alignement ), 0, 0, 2 * spacing + size, 0 );
    gtk_container_add ( GTK_CONTAINER ( alignement ), label );
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox3 ), alignement, FALSE, FALSE, 0 );

    compte -> bouton_compte_mark = gsb_account_create_combo_list (NULL, NULL, FALSE);
    gtk_box_pack_start ( GTK_BOX ( compte -> hbox3 ), compte -> bouton_compte_mark, TRUE, TRUE, 0 );
    gtk_widget_set_sensitive ( compte -> hbox3, FALSE );

    g_object_set_data ( G_OBJECT ( radio ), "associated", compte -> hbox3 );
    g_object_set_data ( G_OBJECT ( radio ), "account", compte );
    g_signal_connect ( G_OBJECT ( radio ), "toggled", 
		       G_CALLBACK ( import_account_action_activated ), GINT_TO_POINTER (IMPORT_MARK_TRANSACTIONS));

    /* Currency */
    hbox = gtk_hbox_new ( FALSE, 6 );
    label = gtk_label_new ( COLON ( _("Account currency") ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    compte -> bouton_devise = gsb_currency_make_combobox ( TRUE );

    /* create the currency if doesn't exist */
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
	    gchar* tmpstr1 = g_strdup_printf ( _( "Currency of imported account '%s' is %s.  Either this currency doesn't exist so you have to create it in next window, or this currency already exists but the ISO code is wrong.\nTo avoid this message, please set its ISO code in configuration."),
						      compte -> nom_de_compte,
						      compte -> devise );
	    gchar* tmpstr2 = g_strdup_printf ( _("Can't associate ISO 4217 code for currency '%s'."),  compte -> devise );
	    dialogue_warning_hint ( tmpstr1, tmpstr2);
	    g_free ( tmpstr1 );
	    g_free ( tmpstr2 );
	}
    }

    gtk_box_pack_start ( GTK_BOX ( hbox ), compte -> bouton_devise, FALSE, FALSE, 0 );

    /* invert amount of transactions */
    button = gsb_automem_checkbutton_new (_("Invert the amount of the imported transactions"),
					  &compte -> invert_transaction_amount, NULL, NULL);
    gtk_box_pack_start ( GTK_BOX ( vbox ), 
			 button,
			 FALSE, FALSE, 0 );

    /* propose to create a rule */
    compte -> hbox_rule = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX ( vbox ), 
			 compte -> hbox_rule,
			 FALSE, FALSE, 0 );
    compte -> entry_name_rule = gtk_entry_new ();
    button = gsb_automem_checkbutton_new (_("Create a rule for this import. Name of the rule : "),
					  &compte -> create_rule, G_CALLBACK (gsb_button_sensitive_by_checkbutton), compte -> entry_name_rule);
    gtk_box_pack_start ( GTK_BOX (compte -> hbox_rule), 
			 button,
			 FALSE, FALSE, 0 );

    /* we can create a rule only for qif or ofx */
    if (strcmp (compte -> origine, "QIF") && strcmp (compte -> origine, "OFX"))
	gtk_widget_set_sensitive (button, FALSE);

    gtk_widget_set_sensitive (compte -> entry_name_rule, FALSE);
    gtk_box_pack_start ( GTK_BOX (compte -> hbox_rule), 
			 compte -> entry_name_rule,
			 FALSE, FALSE, 0 );

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

    gtk_widget_set_sensitive ( account -> hbox_rule,
			       action != IMPORT_CREATE_ACCOUNT);
    return FALSE;    
}



/**
 * finish the import
 * go throw all the imported accounts and does what the user asked with
 * each of them
 *
 * \param
 *
 * \return
 * */
void traitement_operations_importees ( void )
{
    GSList *list_tmp;
    gint new_file;

    /* when come here, all the currencies are already created
     * and init_variables is already called
     * ( see affichage_recapitulatif_importation) */
    
    /* if new file, init grisbi */
    if ( gsb_data_account_get_accounts_amount () )
	new_file = 0;
    else
    {
	/* Create initial lists. */
	new_file = 1;
    }

    /* go throw the accounts and do what is asked */
    list_tmp = liste_comptes_importes;

    while ( list_tmp )
    {
	struct struct_compte_importation *compte;
	gint account_number = 0;

	compte = list_tmp -> data;

	switch ( compte -> action )
	{
	    case IMPORT_CREATE_ACCOUNT:
		account_number = gsb_import_create_imported_account ( compte );

		if ( account_number == -1 )
		{
		    gchar* tmpstr = g_strdup_printf ("An error occured while creating the new account %s,\nWe try to continue to import but bad things can happen...", compte -> nom_de_compte);
		    dialogue_error ( tmpstr);
		    g_free ( tmpstr );
		    continue;
		}

		/* the next functions will add the transaction to the tree view list
		 * so if we create a new file, we need to finish the gui here to append
		 * the transactions */
		if (new_file)
		{
		    /* this should be the same as the end of gsb_file_new */
		    /* init the gui */
		    gsb_file_new_gui ();

		    /* 	set the grid */
		    gsb_transactions_list_draw_grid (etat.affichage_grille);

		    new_file = 0;
		}
		else
		    gsb_gui_navigation_add_account ( account_number, FALSE );

		gsb_import_add_imported_transactions ( compte,
						       account_number );
		break;

	    case IMPORT_ADD_TRANSACTIONS:
		account_number = gsb_account_get_combo_account_number ( compte -> bouton_compte_add );
		gsb_import_add_imported_transactions ( compte,
						       account_number);

		break;

	    case IMPORT_MARK_TRANSACTIONS:
		account_number = gsb_account_get_combo_account_number ( compte -> bouton_compte_mark );
		pointe_opes_importees ( compte );
		break;
	}

	/* update the current and marked balance */
	gsb_data_account_calculate_current_and_marked_balances (account_number);

	/* first, we create the rule if asked */
	if (compte -> create_rule && compte -> action != IMPORT_CREATE_ACCOUNT)
	{
	    /* ok, we create the rule */
	    gint rule;
	    const gchar *name;

	    name = gtk_entry_get_text (GTK_ENTRY (compte -> entry_name_rule));
	    if (!strlen (name))
	    {
		/* the user didn't enter a name, propose now */
		gchar *tmpstr;

		tmpstr = g_strdup_printf ( _("You want to create an import rule for the account %s but didn't give a name to that rule. Please set a name or let it empty to cancel the rule creation."),
					   gsb_data_account_get_name (account_number));
		name = dialogue_hint_with_entry (tmpstr, _("No name for the import rule"), _("Name of the rule :"));
		g_free (tmpstr);
	    }

	    if (!strlen (name))
		break;

	    rule = gsb_data_import_rule_new (name);
	    gsb_data_import_rule_set_account (rule, account_number);
	    gsb_data_import_rule_set_currency (rule, gsb_currency_get_currency_from_combobox (compte -> bouton_devise));
	    gsb_data_import_rule_set_invert (rule, compte -> invert_transaction_amount);
	    gsb_data_import_rule_set_last_file_name (rule, compte -> real_filename);
	    gsb_data_import_rule_set_action (rule, compte -> action);
	}

	list_tmp = list_tmp -> next;
    }

    /* if no account created, there is a problem
     * show an error and go away */
    if (!gsb_data_account_get_accounts_amount ())
    {
	dialogue_error (_("No account in memory now, this is bad...\nBetter to leave the import before a crash.\n\nPlease contact the grisbi team to find the problem."));
	return;
    }

    /* create the links between transactions (transfer) */
    if ( virements_a_chercher )
	cree_liens_virements_ope_import ();

    gsb_status_message ( _("Please wait") );

    /* update the name of accounts in scheduler form */
    gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
				    FALSE );

    /* set the rule button if necessary */
     if (gsb_data_import_rule_account_has_rule (gsb_gui_navigation_get_current_account ()))
	gtk_widget_show (menu_import_rules);
    else
	gtk_widget_hide (menu_import_rules);

    /* show the account list */
    gsb_menu_update_accounts_in_menus();

    /* update main page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_accueil (FALSE);

    gsb_status_clear();

    modification_fichier ( TRUE );
}




/**
 * called at the end of an import, check all the transactions with an account_number_transfer at -2
 * and try to find the contra-transaction to make a real transfer in grisbi
 *
 * */
void cree_liens_virements_ope_import ( void )
{
    GSList *list_tmp_transactions;
    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	/* if the account number of transfer is -2, it's a transfer,
	 * in that case, the name of the contra account is in the bank_references */
	if ( gsb_data_transaction_get_contra_transaction_account (transaction_number_tmp)== -2
	     &&
	     gsb_data_transaction_get_bank_references (transaction_number_tmp))
	{
	    /* the name of the contra account is in the bank references with [ and ] */
	    gchar *contra_account_name;
	    gint contra_account_number;

	    contra_account_name = my_strdelimit (gsb_data_transaction_get_bank_references (transaction_number_tmp),
						 "[]", "");
	    contra_account_number = gsb_data_account_get_no_account_by_name ( contra_account_name );
	    g_free (contra_account_name);

	    if ( contra_account_number == -1 )
	    {
		/* we have not found the contra-account */
		gsb_data_transaction_set_contra_transaction_account ( transaction_number_tmp,
								      0);
		gsb_data_transaction_set_contra_transaction_number ( transaction_number_tmp,
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
		    gchar *account_target_tmp = g_strconcat ("[",
							     gsb_data_account_get_name (transaction_number_tmp),
							     "]",
							     NULL);

		    contra_transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions_2 -> data);

		    if ( gsb_data_transaction_get_account_number (contra_transaction_number_tmp) == contra_account_number
			 &&
			 gsb_data_transaction_get_contra_transaction_account ( contra_transaction_number_tmp ) == -2
			 &&
			 gsb_data_transaction_get_bank_references ( contra_transaction_number_tmp )
			 &&
			 (!g_strcasecmp ( account_target_tmp,
					  gsb_data_transaction_get_bank_references ( contra_transaction_number_tmp ))
			  ||
			  g_strcasecmp ( gsb_data_account_get_name (transaction_number_tmp),
					 gsb_data_transaction_get_bank_references ( contra_transaction_number_tmp))) 
			 &&
			 !gsb_real_cmp ( gsb_real_abs (gsb_data_transaction_get_amount (transaction_number_tmp)),
					 gsb_real_abs (gsb_data_transaction_get_adjusted_amount_for_currency ( contra_transaction_number_tmp,
													       gsb_data_account_get_currency (transaction_number_tmp), -1)))
			 &&
			 ( gsb_data_transaction_get_party_number (transaction_number_tmp)
			   ==
			   gsb_data_transaction_get_party_number ( contra_transaction_number_tmp ))
			 &&
			 !g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
					   gsb_data_transaction_get_date (contra_transaction_number_tmp)))
		    {
			/* we have found the contra transaction, set all the values */
			gint payment_number;

			gsb_data_transaction_set_contra_transaction_number ( transaction_number_tmp,
									       contra_transaction_number_tmp );
			gsb_data_transaction_set_contra_transaction_account ( transaction_number_tmp,
									   gsb_data_transaction_get_account_number (contra_transaction_number_tmp));

			gsb_data_transaction_set_contra_transaction_number ( contra_transaction_number_tmp,
									       transaction_number_tmp);
			gsb_data_transaction_set_contra_transaction_account ( contra_transaction_number_tmp,
									   gsb_data_transaction_get_account_number (transaction_number_tmp));

			gsb_data_transaction_set_bank_references ( transaction_number_tmp,
								   NULL );
			gsb_data_transaction_set_bank_references ( contra_transaction_number_tmp,
								   NULL );

			/* try to set the good method of payment to transfer */
			payment_number = gsb_data_payment_get_transfer_payment_number (gsb_data_transaction_get_account_number (gsb_data_transaction_get_account_number (transaction_number_tmp)));
			if (payment_number)
			    gsb_data_transaction_set_method_of_payment_number (transaction_number_tmp, payment_number);

			payment_number = gsb_data_payment_get_transfer_payment_number (gsb_data_transaction_get_account_number (gsb_data_transaction_get_account_number (contra_transaction_number_tmp)));
			if (payment_number)
			    gsb_data_transaction_set_method_of_payment_number (contra_transaction_number_tmp, payment_number);
		    }

		    if (account_target_tmp)
			g_free (account_target_tmp);

		    list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
		}

		/* if no contra-transaction, that transaction becomes normal */
		if ( gsb_data_transaction_get_contra_transaction_account (transaction_number_tmp) == -2 )
		{
		    gsb_data_transaction_set_contra_transaction_account ( transaction_number_tmp,
								       0);
		    gsb_data_transaction_set_contra_transaction_number ( transaction_number_tmp,
									   0);
		    gsb_data_transaction_set_bank_references ( transaction_number_tmp, NULL );
		}
	    }
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }
    /* the transactions were already set in the list,
     * and the transfer was not written, we need to update the categories values
     * in the lists */
    transaction_list_update_element (ELEMENT_CATEGORY);
    transaction_list_update_element (ELEMENT_TYPE);
}


/**
 * create a new account with the datas in the imported account given in args
 * don't work with the gui here, all the gui will be done later
 *
 * \param imported_account the account we want to create
 * 
 * \return the number of the new account or -1 if problem
 * */
gint gsb_import_create_imported_account ( struct struct_compte_importation *imported_account )
{
    gint account_number;
    gint kind_account;

    if (!imported_account)
	return -1;

    /* create the new account,
     * for now 3 kind of account, GSB_TYPE_BANK, GSB_TYPE_LIABILITIES or GSB_TYPE_CASH */
    switch (gsb_combo_box_get_index (imported_account -> bouton_type_compte))
    {
	case 2:
	    kind_account = GSB_TYPE_LIABILITIES;
	    break;

	case 1:
	    kind_account = GSB_TYPE_CASH;
	    break;

	default:
	    kind_account = GSB_TYPE_BANK;
    }

    account_number = gsb_data_account_new (kind_account);

    if ( account_number == -1 )
	return -1;

    /* set the default method of payment */
    gsb_data_payment_create_default (account_number);

    /* set the id and try to find the bank */
    if ( imported_account -> id_compte )
    {
	gchar **tab_str;

	gsb_data_account_set_id (account_number,
				 imported_account -> id_compte );

	/* usually, the id is "bank_number guichet_number account_number key"
	 * try to get the datas */

	tab_str = g_strsplit ( gsb_data_account_get_id (account_number),
			       " ",
			       3 );
	if ( tab_str[1] )
	{
	    gsb_data_account_set_bank_branch_code ( account_number,
						    tab_str[1] );
	    if ( tab_str[2] )
	    {
		gchar *temp;

		gsb_data_account_set_bank_account_key ( account_number,
							tab_str[2] + strlen ( tab_str[2] ) - 1 );

		temp = my_strdup ( tab_str[2] );
		if ( temp && strlen(temp) )
		{
		    temp[strlen (temp) - 1 ] = 0;
		    gsb_data_account_set_bank_account_number ( account_number, temp );
		    g_free (temp);
		}
	    }
	}
	g_strfreev ( tab_str );
    }

    /* set the name (the g_strstrip is VERY important !!!) */
    if ( imported_account -> nom_de_compte )
	gsb_data_account_set_name ( account_number,
				    g_strstrip (imported_account -> nom_de_compte));
    else
	gsb_data_account_set_name ( account_number,
				    _("Imported account"));

    /* set the currency */
    gsb_data_account_set_currency ( account_number,
				    gsb_currency_get_currency_from_combobox (imported_account -> bouton_devise));

    /* set the initial balance */
    gsb_data_account_set_init_balance ( account_number,
					imported_account -> solde);
    gsb_data_account_set_current_balance ( account_number, 
					   imported_account -> solde);
    gsb_data_account_set_marked_balance ( account_number, 
					  imported_account -> solde);

    /* Use two lines view by default. */
    gsb_data_account_set_nb_rows ( account_number, 2 );

    /* set the new form organization */
    gsb_data_form_new_organization ( account_number );
    gsb_data_form_set_default_organization ( account_number );

    return account_number;
}


/**
 * import the transactions in an existent account
 * check the id of the account and if the transaction already exists
 * add too the transactions to the gui
 * 
 * \param imported_account an imported structure account which contains the transactions
 * \param account_number the number of account where we want to put the new transations
 * 
 * \return
 * */
void gsb_import_add_imported_transactions ( struct struct_compte_importation *imported_account,
					    gint account_number )
{
    GSList *list_tmp;
    GDate *last_date_import;
    gint demande_confirmation;
    GSList *list_tmp_transactions;

    /* check the imported account id, and set it in the grisbi account if it doesn't exist or if it's wrong */
    if ( imported_account -> id_compte )
    {
	if ( gsb_data_account_get_id (account_number) )
	{
	    if ( g_strcasecmp ( gsb_data_account_get_id (account_number),
				imported_account -> id_compte ))
	    {
		/* there is a difference between the imported account id and grisbi account id,
		 * ask to be sure */
		gchar* tmpstr = g_strdup_printf ( _("The Grisbi's id account is %s, but the imported id account is %s.\n\nPerhaps you choose a wrong account ?\nIf you choose to continue, the id of the Grisbi's account will be changed.  Do you want to continue ?"),
							      gsb_data_account_get_id (account_number),
							      imported_account -> id_compte );
		if ( question_yes_no_hint ( _("The id of the imported and chosen accounts are different"),
					    tmpstr,
					    GTK_RESPONSE_NO ))
		{
		    g_free ( tmpstr );
		    gsb_data_account_set_id ( account_number,
					      imported_account -> id_compte );
		}
		else
		{
		    g_free ( tmpstr );
		    return;
		}
	    }
	}
	else
	    gsb_data_account_set_id (account_number,
				     imported_account -> id_compte );
    }


    /* on fait un premier tour de la liste des opés pour repérer celles qui sont déjà entrées */
    /*   si on n'importe que du ofx, c'est facile, chaque opé est repérée par une id */
    /*     donc si l'opé importée a une id, il suffit de rechercher l'id dans le compte, si elle */
    /*     n'y est pas l'opé est à enregistrer */
    /*     si on importe du qif, il n'y a pas d'id. donc soit on retrouve une opé semblable */
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

    /* ok, now last_date_import contains the last transaction date used in that account,
     * can check the imported transactions */
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
	    /* that transaction is before the last transaction in the account,
	     * so check if the transaction already exists */

	    /* first check the id */
	    if ( imported_transaction -> id_operation
		 &&
		 gsb_data_transaction_find_by_id (imported_transaction -> id_operation))
		/* the id exists so the transaction is already in grisbi, we will forget that transaction */
		imported_transaction -> action = IMPORT_TRANSACTION_LEAVE_TRANSACTION;

	    /* if no id, check the cheque */
	    gchar* tmpstr = utils_str_itoa (imported_transaction -> cheque);
	    if ( imported_transaction -> action != IMPORT_TRANSACTION_LEAVE_TRANSACTION
		 &&
		 imported_transaction -> cheque
		 &&
		 gsb_data_transaction_find_by_payment_content ( tmpstr,
								account_number ))
		/* found the cheque, forget that transaction */
		imported_transaction -> action = IMPORT_TRANSACTION_LEAVE_TRANSACTION;

	    g_free ( tmpstr );

	    /* no id, no cheque, try to find the transaction */
	    if ( imported_transaction -> action != IMPORT_TRANSACTION_LEAVE_TRANSACTION )
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
			if ( !gsb_real_cmp ( gsb_data_transaction_get_amount (transaction_number_tmp),
					     imported_transaction -> montant )
			     &&
			     ( g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
						date_debut_comparaison ) >= 0 )
			     &&
			     ( g_date_compare ( gsb_data_transaction_get_date (transaction_number_tmp),
						date_fin_comparaison ) <= 0 )

			     &&
			     !imported_transaction -> ope_de_ventilation )
			{
			    /* the imported transaction has the same date and same amount, will ask the user */
			    imported_transaction -> action = IMPORT_TRANSACTION_ASK_FOR_TRANSACTION; 
			    imported_transaction -> ope_correspondante = transaction_number_tmp;
			    demande_confirmation = 1;
			}
		    }
		    list_tmp_transactions = list_tmp_transactions -> next;
		}
	    }
	}
	list_tmp = list_tmp -> next;
    }

    /* if we are not sure about some transactions, ask now */
    if ( demande_confirmation )
	confirmation_enregistrement_ope_import ( imported_account );

    /* ok, now we know what to do for each transactions, can import to the account */
    mother_transaction_number = 0;

    list_tmp = imported_account -> operations_importees;

    while ( list_tmp )
    {
	struct struct_ope_importation *imported_transaction;

	imported_transaction = list_tmp -> data;

	if ( imported_transaction -> action == IMPORT_TRANSACTION_GET_TRANSACTION )
	{
	    gint transaction_number;

	    /* set the account currency to the transaction and create the transaction */
	    imported_transaction -> devise = gsb_currency_get_currency_from_combobox (imported_account -> bouton_devise);
	    transaction_number = gsb_import_create_transaction ( imported_transaction,
								 account_number );

	    /* invert the amount of the transaction if asked */
	    if (imported_account -> invert_transaction_amount)
		gsb_data_transaction_set_amount ( transaction_number,
						  gsb_real_opposite (gsb_data_transaction_get_amount (transaction_number)));

	    /* we need to add the transaction now to the tree model here
	     * to avoid to write again all the account */
	    gsb_transactions_list_append_new_transaction (transaction_number, FALSE);
	} 
	list_tmp = list_tmp -> next;
    }

    /** some payee should have been added, so update the combofix */
    gsb_payee_update_combofix ();

    /* if we are on the current account, we need to update the tree_view */
    if (gsb_gui_navigation_get_current_account () == account_number)
    {
	gsb_transactions_list_update_tree_view (account_number, TRUE);
	gsb_gui_headings_update_suffix ( gsb_real_get_string_with_currency ( gsb_data_account_get_current_balance (account_number),
									     gsb_data_account_get_currency (account_number), TRUE));
    }
}


/**
 * called if we are not sure about some transactions to import
 * ask here to the user
 *
 * \param imported_account
 *
 * \return
 * */
void confirmation_enregistrement_ope_import ( struct struct_compte_importation *imported_account )
{
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
	    const gchar *tiers;

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

	    gchar* tmpstr2 = gsb_real_get_string (ope_import -> montant);
	    gchar* tmpstr = g_strdup_printf ( _("Transactions to import : %s ; %s ; %s"),
						      gsb_format_gdate ( ope_import -> date ),
						      ope_import -> tiers,
						      tmpstr2);
	    g_free ( tmpstr2 );
	    label = gtk_label_new ( tmpstr );
	    g_free ( tmpstr );
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

	    tiers = gsb_data_payee_get_name ( gsb_data_transaction_get_party_number (ope_import -> ope_correspondante), FALSE );

	    if ( gsb_data_transaction_get_notes (ope_import -> ope_correspondante))
	    {
		gchar* tmpstr2 = gsb_real_get_string (gsb_data_transaction_get_amount (
							ope_import -> ope_correspondante));
		gchar* tmpstr = g_strdup_printf ( _("Transaction found : %s ; %s ; %s ; %s"),
					gsb_format_gdate ( gsb_data_transaction_get_date (ope_import -> ope_correspondante)),
					tiers,
					tmpstr2,
					gsb_data_transaction_get_notes (ope_import -> ope_correspondante));
		g_free ( tmpstr2 );
		label = gtk_label_new ( tmpstr);
		g_free ( tmpstr );
	    }
	    else
	    {
		gchar* tmpstr2 = gsb_real_get_string (gsb_data_transaction_get_amount (
							ope_import -> ope_correspondante));
		gchar* tmpstr = g_strdup_printf ( _("Transaction found : %s ; %s ; %s"),
							  gsb_format_gdate ( gsb_data_transaction_get_date (ope_import -> ope_correspondante)),
							  tiers,
							  tmpstr2);
		g_free ( tmpstr2 );
		label = gtk_label_new ( tmpstr );
		g_free ( tmpstr );
	    }

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


/**
 * get an imported transaction structure in arg and create the corresponding transaction
 * 
 * \param imported_transaction the transaction to import
 * \param account_number the account where to put the new transaction
 * 
 * \return the number of the new transaction
 * */
gint gsb_import_create_transaction ( struct struct_ope_importation *imported_transaction,
				     gint account_number )
{
    gchar **tab_str;
    gint transaction_number;
    gint fyear;

    /* we create the new transaction */
    transaction_number = gsb_data_transaction_new_transaction ( account_number );

    /* get the id if exists */
    if ( imported_transaction -> id_operation )
	gsb_data_transaction_set_transaction_id ( transaction_number,
						  imported_transaction -> id_operation );

    /* get the date */
    gsb_data_transaction_set_date ( transaction_number,
				    imported_transaction -> date );

    /* récupération de la date de valeur */
    gsb_data_transaction_set_value_date ( transaction_number,
					  imported_transaction -> date_de_valeur );

    /* set the financial year according to the date,
     * accord to value date by default, and if not, by date
     * FIXME : propose in configuration to take by date by default ? */
    if (imported_transaction -> date_de_valeur)
	fyear = gsb_data_fyear_get_from_date (imported_transaction -> date_de_valeur );
    else
	fyear = gsb_data_fyear_get_from_date (imported_transaction -> date );
    if (fyear > 0)
	gsb_data_transaction_set_financial_year_number ( transaction_number,
							 fyear );

    /* récupération du montant */
    gsb_data_transaction_set_amount ( transaction_number,
				      imported_transaction -> montant );

    /* 	  récupération de la devise */
    gsb_data_transaction_set_currency_number ( transaction_number,
					       imported_transaction -> devise );

    /* rÃ©cupération du tiers */
    if ( imported_transaction -> tiers 
	 &&
	 strlen (imported_transaction -> tiers))
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
	     strlen (imported_transaction -> categ))
	{
	    if ( imported_transaction -> categ[0] == '[' )
	    {
		/* 		      c'est un virement, or le compte n'a peut être pas encore été créé, */
		/* on va mettre le nom du compte dans info_banque_guichet qui n'est jamais utilisé */
		/* lors d'import, et relation_no_compte sera mis à -2 (-1 est déjà utilisé pour les comptes supprimés */

		gsb_data_transaction_set_bank_references ( transaction_number,
							   imported_transaction -> categ);
		gsb_data_transaction_set_contra_transaction_account ( transaction_number,
								   -2);
		gsb_data_transaction_set_contra_transaction_number ( transaction_number,
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
		if (tab_str[0])
		    tab_str[0] = g_strstrip (tab_str[0]);
		category_number = gsb_data_category_get_number_by_name ( tab_str[0],
									 TRUE,
									 imported_transaction -> montant.mantissa < 0 );
		gsb_data_transaction_set_category_number ( transaction_number,
							   category_number );
		if (tab_str[1])
		    tab_str[1] = g_strstrip (tab_str[1]);
		gsb_data_transaction_set_sub_category_number ( transaction_number,
							       gsb_data_category_get_sub_category_number_by_name ( category_number,
														   tab_str[1],
														   TRUE ));
	    }
	}
	else
	{
	    gsb_data_transaction_set_category_number ( transaction_number, 0 );
	    gsb_data_transaction_set_sub_category_number ( transaction_number, 0 );
	}
    }

    /* récupération des notes */
    gsb_data_transaction_set_notes ( transaction_number,
				     imported_transaction -> notes );

    /* récupération du chèque et mise en forme du type d'opération */
    if ( imported_transaction -> cheque )
    {
	/* it's a cheque, try to find a method of payment with automatic increment, if don't find
	 * set in default method of payment */
	gint payment_number;
	gint sign;

	if ( gsb_data_transaction_get_amount (transaction_number).mantissa < 0 )
	{
	    sign = GSB_PAYMENT_DEBIT;
	    payment_number = gsb_data_account_get_default_debit (account_number);
	   }
	else
	{
	    sign = GSB_PAYMENT_CREDIT;
	    payment_number = gsb_data_account_get_default_credit (account_number);
	}

	if ( !gsb_data_payment_get_automatic_numbering (payment_number))
	{
	    /* the default method is not an automatic numbering, try to find one in the method of payment of this account */
	    GSList *list_tmp;

	    list_tmp = gsb_data_payment_get_payments_list ();
	    while (list_tmp)
	    {
		gint payment_number_tmp;

		payment_number_tmp = gsb_data_payment_get_number (list_tmp -> data);

		if ( gsb_data_payment_get_account_number (payment_number_tmp) == account_number
		     &&
		     gsb_data_payment_get_automatic_numbering (payment_number_tmp)
		     &&
		     ( gsb_data_payment_get_sign (payment_number_tmp) == sign
		       ||
		       gsb_data_payment_get_sign (payment_number_tmp) == GSB_PAYMENT_NEUTRAL ))
		{
		    payment_number = payment_number_tmp;
		    break;
		}
		list_tmp = list_tmp -> next;
	    }
	}
	/* now, either payment_number is an automatic numbering method of payment,
	 * either it's the default method of payment,
	 * first save it */
	gsb_data_transaction_set_method_of_payment_number ( transaction_number,
							    payment_number );

	gchar* tmpstr = utils_str_itoa ( imported_transaction -> cheque );
	if ( gsb_data_payment_get_automatic_numbering (payment_number))
	    /* we are on the default payment_number, save just the cheque number */
	    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
								 tmpstr );
	else
	{
	    /* we are on a automatic numbering payment, we will save the cheque only
	     * if it's not used before, else we show an warning message */
	    if (gsb_data_transaction_check_content_payment (payment_number, imported_transaction -> cheque))
	    {
		gchar* tmpstr = g_strdup_printf ( _("Warning : the cheque number %ld is already used.\nWe skip it"),
						     imported_transaction -> cheque );
		dialogue_warning ( tmpstr );
		g_free ( tmpstr );
	    }
	    else
		gsb_data_transaction_set_method_of_payment_content ( transaction_number,
								     utils_str_itoa ( imported_transaction -> cheque ) );
	}
	g_free ( tmpstr );
    }
    else
    {
	/* comme ce n'est pas un chèque, on met sur le type par défaut */
	if ( gsb_data_transaction_get_amount (transaction_number).mantissa < 0 )
	    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								gsb_data_account_get_default_debit (account_number));
	else
	    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								gsb_data_account_get_default_credit (account_number));
    }

    /* récupération du pointé */
    gsb_data_transaction_set_marked_transaction ( transaction_number,
						  imported_transaction -> p_r );

    /* si c'est une ope de ventilation, lui ajoute le no de l'opération précédente */
    if ( imported_transaction -> ope_de_ventilation )
	gsb_data_transaction_set_mother_transaction_number ( transaction_number,
							     mother_transaction_number );
    else
	mother_transaction_number  = transaction_number;

    return (transaction_number);
}



/* *******************************************************************************/
void pointe_opes_importees ( struct struct_compte_importation *imported_account )
{
    GSList *list_tmp;
    GSList *liste_opes_import_celibataires;
    gint account_number;


    /* on se place sur le compte dans lequel on va pointer les opés */
    account_number = gsb_account_get_combo_account_number ( imported_account -> bouton_compte_mark );

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
					    _("Perhaps you choose a wrong account ?  If you choose to continue, the id of the account will be changed.  Do you want to continue ?"),
					    GTK_RESPONSE_NO ))
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
	GSList *liste_ope_importees_tmp;
	GSList *ope_trouvees;
	struct struct_ope_importation *ope_import;
	gint transaction_number;
	gint i;

	ope_import = list_tmp -> data;
	ope_trouvees = NULL;

	/* set now the account number of the transaction */
	ope_import -> no_compte = account_number;

	/* si l'opé d'import a une id, on recherche dans la liste d'opé pour trouver
	   une id comparable */
	if ( ope_import -> id_operation
	     &&
	     (transaction_number = gsb_data_transaction_find_by_id (ope_import -> id_operation)))
	    ope_trouvees = g_slist_append ( ope_trouvees,
					    GINT_TO_POINTER (transaction_number));

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
		transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		if ( gsb_data_transaction_get_account_number (transaction_number) == account_number )
		{
		    if ( !gsb_real_cmp ( gsb_data_transaction_get_amount (transaction_number),
					 ope_import -> montant )
			 &&
			 ( g_date_compare ( gsb_data_transaction_get_date (transaction_number),
					    date_debut_comparaison ) >= 0 )
			 &&
			 ( g_date_compare ( gsb_data_transaction_get_date (transaction_number),
					    date_fin_comparaison ) <= 0 )

			 &&
			 !gsb_data_transaction_get_mother_transaction_number (transaction_number))
			/* on a retouvé une opé de même date et même montant, on l'ajoute à la liste des opés trouvées */
			ope_trouvees = g_slist_append ( ope_trouvees,
							GINT_TO_POINTER (transaction_number));
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

		    ope_import -> devise = gsb_currency_get_currency_from_combobox (imported_account -> bouton_devise);
		    liste_opes_import_celibataires = g_slist_append ( liste_opes_import_celibataires,
								      ope_import );
		}

		break;

	    case 1:
		/*  il n'y a qu'une opé retrouvée, on la pointe */
		/* si elle est déjà pointée ou relevée, on ne fait rien */
		/* si l'opé d'import a une id et pas l'opé, on marque l'id dans l'opé */

		transaction_number = GPOINTER_TO_INT (ope_trouvees -> data);

		if ( !gsb_data_transaction_get_transaction_id (transaction_number)
		     &&
		     ope_import -> id_operation )
		    gsb_data_transaction_set_transaction_id ( transaction_number,
							      ope_import -> id_operation );

		if ( !gsb_data_transaction_get_marked_transaction (transaction_number))
		{
		    gsb_data_transaction_set_marked_transaction ( transaction_number,
								  2 );
		    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

		    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
		    {
			GSList *list_tmp_transactions;

			list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

			while ( list_tmp_transactions )
			{
			    gint transaction_number_tmp;
			    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

			    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number
				 &&
				 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
				gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
									      2 );
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
		liste_ope_importees_tmp = imported_account -> operations_importees;

		while ( liste_ope_importees_tmp )
		{
		    struct struct_ope_importation *ope_import_tmp;
		    GDate *date_debut_comparaison;
		    GDate *date_fin_comparaison;

		    ope_import_tmp = liste_ope_importees_tmp -> data;

		    /* we look for a date around ope_import_tmp with +- valeur_echelle_recherche_date_import */
		    date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( ope_import_tmp -> date ),
							      g_date_get_month ( ope_import_tmp -> date ),
							      g_date_get_year ( ope_import_tmp -> date ));
		    g_date_subtract_days ( date_debut_comparaison,
					   valeur_echelle_recherche_date_import );

		    date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( ope_import_tmp -> date ),
							    g_date_get_month ( ope_import_tmp -> date ),
							    g_date_get_year ( ope_import_tmp -> date ));
		    g_date_add_days ( date_fin_comparaison,
				      valeur_echelle_recherche_date_import );

		    if ( !gsb_real_cmp ( ope_import_tmp -> montant,
					 ope_import -> montant  )
			 &&
			 ( g_date_compare ( ope_import -> date,
					    date_debut_comparaison ) >= 0 )
			 &&
			 ( g_date_compare ( ope_import -> date,
					    date_fin_comparaison ) <= 0 )

			 &&
			 !ope_import_tmp -> ope_de_ventilation )
			/* on a retouvé une opé d'import de même date et même montant, on incrémente le nb d'opé d'import semblables trouvees */
			i++;

		    liste_ope_importees_tmp = liste_ope_importees_tmp -> next;
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
			gint transaction_number;

			transaction_number = GPOINTER_TO_INT (list_tmp_2 -> data);

			if ( !gsb_data_transaction_get_transaction_id (transaction_number)
			     &&
			     ope_import -> id_operation )
			    gsb_data_transaction_set_transaction_id ( transaction_number,
								      ope_import -> id_operation );

			if ( !gsb_data_transaction_get_marked_transaction (transaction_number))
			{
			    gsb_data_transaction_set_marked_transaction ( transaction_number,
									  2 );

			    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

			    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
			    {
				GSList *list_tmp_transactions;

				list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

				while ( list_tmp_transactions )
				{
				    gint transaction_number_tmp;
				    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

				    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number )
				    {
					if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
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
 * return a newly allocated string or NULL
 */
gchar * unique_imported_name ( gchar * account_name )
{
    GSList * tmp_list = liste_comptes_importes;
    gchar * basename = account_name;
    gint iter = 1;

/* TODO dOm : fix memory leaks in this function */
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
					       "import.png" ); 
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



/**
 * try to autodetect the type of imported file
 *
 * \param filename
 * \param pointeur_char	pointer returned by g_file_get_contents
 * 			can be NULL, in that case only the extension will be used
 * 
 * \return a string : OFX, QIF, Gnucash
 *
 *
 */
const gchar * autodetect_file_type ( gchar * filename,
				     gchar * pointeur_char )
{
    gchar * extension;
    gchar * type;

    extension = strrchr ( filename, '.' );
    if ( extension )
    {
	GSList * tmp = import_formats;

	while ( tmp )
	{
	    struct import_format * format = (struct import_format *) tmp -> data;

	    if ( !strcasecmp ( extension + 1, format -> extension ) )
	    {
		return format -> name;
	    }	

	    tmp = tmp -> next;
	}
    }
    
    if ( ! pointeur_char )
    {
	return _("Unknown");
    }

    /** FIXME: add auto-detection to plugin. */
    
    if ( g_strrstr ( pointeur_char, "ofx" ) || g_strrstr ( pointeur_char, "OFX" ))
    {
	type = "OFX";
    }
    else if ( !my_strncasecmp ( pointeur_char, "!Type", 5 ) ||
	      !my_strncasecmp ( pointeur_char, "!Account", 8 ) || 
	      !my_strncasecmp ( pointeur_char, "!Option", 7 ))
    {
	type = "QIF";
    }
    else
    {
	if ( strstr ( pointeur_char, "<gnc-v2" ) )
	{
	    type = "Gnucash";
	}
	else
	{
	    type = _("Unknown");
	}
    }

    return type;
}



/**
 * Add an imported account to the list of imported accounts.
 *
 * \param account	Account to register.
 */
void gsb_import_register_account ( struct struct_compte_importation * account )
{
    liste_comptes_importes = g_slist_append ( liste_comptes_importes, account );
}



/**
 * Add an imported account to the list of imported accounts in error.
 *
 * \param account	Account to register.
 */
void gsb_import_register_account_error ( struct struct_compte_importation * account )
{
    liste_comptes_importes_error = g_slist_append ( liste_comptes_importes, account );
}

/**
 * import a file with a rule
 *
 * \param rule	the number of rule to use
 *
 * \return TRUE : ok, FALSE : nothing done
 * */
gboolean gsb_import_by_rule ( gint rule )
{
    gint account_number;
    gchar **array;
    gint i=0;


    array = gsb_import_by_rule_ask_filename (rule);
    if (!array)
	return FALSE;

    account_number = gsb_data_import_rule_get_account (rule);

    while (array[i])
    {
	gchar *filename = array[i];
	const gchar *type;
	struct imported_file imported;
	GSList * tmp = import_formats;
	struct struct_compte_importation *account;

	/* check if we are on ofx or qif file */
	type = autodetect_file_type (filename, NULL);
	if (strcmp (type, "OFX") && strcmp (type, "QIF"))
	{
	    gchar *tmpstr = g_path_get_basename (filename);
	    gchar *tmpstr2 = g_strdup_printf (_("%s is neither an OFX file, neither a QIF file. Nothing will be done for that file."),
					      tmpstr );
	    dialogue_error (tmpstr2);
	    g_free (tmpstr);
	    g_free (tmpstr2);
	    i++;
	    continue;
	}

	/* get the transactions */
	imported.name = filename;
	imported.coding_system = go_charmap_sel_get_encoding ( (GOCharmapSel * )go_charmap_sel );
	imported.type = type;

	liste_comptes_importes_error = NULL;
	liste_comptes_importes = NULL;

	while ( tmp )
	{
	    struct import_format * format = (struct import_format *) tmp -> data;

	    if ( !strcmp ( imported.type, format -> name ) )
	    {
		format -> import ( NULL, &imported );
		tmp = tmp -> next;
		continue;
	    }
	    tmp = tmp -> next;
	}

	/* now liste_comptes_importes contains the account structure of imported transactions */
	if (liste_comptes_importes_error)
	{
	    gchar *tmpstr = g_path_get_basename (filename);
	    gchar *tmpstr2 = g_strdup_printf (_("%s was not imported successfully. An error occured while getting the transactions."),
					      tmpstr );
	    dialogue_error (tmpstr2);
	    g_free (tmpstr);
	    g_free (tmpstr2);
	    i++;
	    continue;
	}

	account = liste_comptes_importes -> data;

	switch (gsb_data_import_rule_get_action (rule))
	{
	    case IMPORT_ADD_TRANSACTIONS:
		gsb_import_add_imported_transactions ( account,
						       account_number);

		break;

	    case IMPORT_MARK_TRANSACTIONS:
		pointe_opes_importees (account);
		break;
	}
	/* update the current and marked balance */
	gsb_data_account_calculate_current_and_marked_balances (account_number);

	g_slist_free (account -> operations_importees);
	g_free (account);
	g_slist_free (liste_comptes_importes);
	i++;
    }
    g_strfreev (array);

    /* update main page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_accueil (FALSE);

    modification_fichier ( TRUE );

    return FALSE;
}


/**
 * show a popup asking the name of the file to import with a rule
 *
 * \param rule
 *
 * \return a newly-allocated NULL-terminated array of strings of filenames, or NULL
 * 		use g_strfreev to free it
 * */
gchar **gsb_import_by_rule_ask_filename ( gint rule )
{
    gchar *tmpstr;
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *separator;
    GtkWidget *hbox;
    gchar **array = NULL;

    if (!rule)
	return NULL;

    dialog = gtk_dialog_new_with_buttons (_("Import a file with a rule"),
					  GTK_WINDOW (window),
					  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					  GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					  GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					  NULL);

    tmpstr = g_strdup_printf (_("Properties of the rule %s :\n"),
			      gsb_data_import_rule_get_name (rule));
    label = gtk_label_new (tmpstr);
    g_free (tmpstr);
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 label,
			 FALSE, FALSE, 0);

    if (gsb_data_import_rule_get_action (rule) == IMPORT_ADD_TRANSACTIONS)
	tmpstr = g_strdup_printf (_("Imported transactions will be added to the account %s.\n"),
				  gsb_data_account_get_name (gsb_data_import_rule_get_account (rule)));
    else
	tmpstr = g_strdup_printf (_("Imported transactions will mark transactions in the account %s.\n"),
				  gsb_data_account_get_name (gsb_data_import_rule_get_account (rule)));
    label = gtk_label_new (tmpstr);
    g_free (tmpstr);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 label,
			 FALSE, FALSE, 0);

    tmpstr = g_strdup_printf (_("Currency to import is %s.\n"),
			      gsb_data_currency_get_name (gsb_data_import_rule_get_currency (rule)));
    label = gtk_label_new (tmpstr);
    g_free (tmpstr);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 label,
			 FALSE, FALSE, 0);

    if (gsb_data_import_rule_get_invert (rule))
    {
	label = gtk_label_new (_("Amounts of the transactions will be inverted.\n"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
	gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			     label,
			     FALSE, FALSE, 0);
    }

    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 separator,
			 FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new (_("Name of the file to import : "));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0);

    /* i tried to use gtk_file_chooser_button, but the name of the file is showed only sometimes
     * so go back to the old method with a gtkentry */
    entry = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX (hbox),
			 entry,
			 FALSE, FALSE, 0);
    gtk_widget_grab_focus (entry);
    if (gsb_data_import_rule_get_last_file_name (rule))
    {
	gtk_entry_set_text ( GTK_ENTRY (entry),
			     gsb_data_import_rule_get_last_file_name (rule));
	gtk_entry_select_region ( GTK_ENTRY (entry),
				  0, -1);
    }

    button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
    g_signal_connect (G_OBJECT (button),
		      "clicked",
		      G_CALLBACK (gsb_import_by_rule_get_file),
		      entry );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 button,
			 FALSE, FALSE, 0);


    gtk_widget_show_all (dialog);
    if ( gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	array = g_strsplit ( gtk_entry_get_text (GTK_ENTRY (entry)),
			     ";", 0);

    gtk_widget_destroy (dialog);
    return array;
}



/**
 * callback when user click on the button to open a file by file chooser
 *
 * \param button	the button to open the file chooser
 * \param entry		the entry to fill with the new filename
 *
 * \return FALSE
 * */
gboolean gsb_import_by_rule_get_file ( GtkWidget *button,
				       GtkWidget *entry )
{
    GSList *filenames;
    GSList *tmp_list;
    gchar *string = NULL;

    filenames = gsb_import_create_file_chooser ();
    if (!filenames)
	return FALSE;

    /* separate all the files by ; */
    tmp_list = filenames;
    while (tmp_list)
    {
	if (string)
	{
	    gchar *last_string = string;

	    string = g_strconcat (string, ";", tmp_list -> data, NULL);
	    g_free (last_string);
	}
	else
	    string = my_strdup (tmp_list -> data);
	tmp_list = tmp_list -> next;
    }
    g_slist_free (filenames);
    gtk_entry_set_text (GTK_ENTRY (entry), string);
    g_free (string);
    return FALSE;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
