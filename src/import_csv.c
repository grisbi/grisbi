/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger	(cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "import_csv.h"
#include "csv_parse.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "import.h"
#include "utils.h"
#include "utils_editables.h"
#include "structures.h"
#include "include.h"
#include "import_csv.h"
/*END_INCLUDE*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern GSList *liste_comptes_importes_error;
extern gint max;
extern GtkTreeStore *model;
/*END_EXTERN*/

/*START_STATIC*/
static gboolean csv_import_change_field ( GtkWidget * item, gint no_menu );
static gboolean csv_import_change_separator ( GtkEntry * entry, gchar * value, 
				       gint length, gint * position );
static gint csv_import_count_columns ( gchar * contents, gchar * separator );
static GtkTreeModel * csv_import_create_model ( GtkTreeView * tree_preview, gchar * contents, 
					 gchar * separator );
static GtkWidget * csv_import_fields_menu ( GtkTreeViewColumn * col, gint field );
static gint * csv_import_guess_fields_config ( gchar * contents, gint size );
static gchar * csv_import_guess_separator ( gchar * contents );
static gboolean csv_import_header_on_click ( GtkWidget * button, GdkEventButton * ev, 
				      gint *no_column );
static gboolean csv_import_try_separator ( gchar * contents, gchar * separator );
static gint * csv_import_update_fields_config ( gchar * contents, gint size );
static gboolean csv_import_update_preview ( GtkWidget * assistant );
/*END_STATIC*/



/* Globals */
gint * csv_fields_config = NULL; /** Array of pointers to fields.  */


/** Contain configuration of CSV fields.  */
struct csv_field csv_fields[14] = {
    { N_("Unknown field"),  0.0, NULL,			     NULL		     },
/*     { N_("Account"),	    0.0, csv_import_validate_string, NULL		     }, */
    { N_("Currency"),	    0.0, csv_import_validate_string, csv_import_parse_currency },
    { N_("Date"),	    0.0, csv_import_validate_date,   csv_import_parse_date },
    { N_("Value date"),	    0.0, csv_import_validate_date,   csv_import_parse_value_date },
    { N_("Payee"),	    0.0, csv_import_validate_string, csv_import_parse_payee },
    { N_("Notes"),	    0.0, csv_import_validate_string, csv_import_parse_notes },
    { N_("Voucher number"), 0.0, csv_import_validate_number, csv_import_parse_voucher },
    { N_("Category"),	    0.0, csv_import_validate_string, csv_import_parse_category },
    { N_("Sub-Category"),   0.0, csv_import_validate_string, csv_import_parse_sub_category },
    { N_("Amount"),	    0.0, csv_import_validate_amount, csv_import_parse_amount },
    { N_("Credit"),	    0.0, csv_import_validate_amount, csv_import_parse_credit },
    { N_("Debit (absolute)"),0.0, csv_import_validate_amount, csv_import_parse_debit },
    { N_("Debit (negative)"),0.0, csv_import_validate_amount, csv_import_parse_credit },
    { N_("P/R"),	    0.0, csv_import_validate_string, csv_import_parse_p_r },
    { N_("Breakdown"),	    0.0, csv_import_validate_string, csv_import_parse_breakdown },
    { NULL },
};


struct struct_compte_importation * compte;



/**
 *
 *
 *
 */
GtkWidget * import_create_csv_preview_page ( GtkWidget * assistant )
{
    GtkWidget * vbox, * paddingbox, * tree_preview, * entry, * sw;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    paddingbox = new_paddingbox_with_title ( vbox, TRUE, "Choose CSV separator" );

    entry = new_text_entry ( NULL, G_CALLBACK ( csv_import_change_separator ), assistant );
    g_object_set_data ( G_OBJECT(entry), "assistant", assistant );
    g_object_set_data ( G_OBJECT(assistant), "entry", entry );    
    gtk_box_pack_start ( GTK_BOX(paddingbox), entry, FALSE, FALSE, 6 );

    paddingbox = new_paddingbox_with_title ( vbox, TRUE, "Select CSV fields" );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_usize ( sw, 480, 200 );
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX(paddingbox), sw, TRUE, TRUE, 6 );

    tree_preview = gtk_tree_view_new ();
    g_object_set_data ( G_OBJECT(assistant), "tree_preview", tree_preview );
    gtk_container_add (GTK_CONTAINER (sw), tree_preview);

    return vbox;
}



/**
 *
 *
 *
 */
GtkTreeModel * csv_import_create_model ( GtkTreeView * tree_preview, gchar * contents, 
					 gchar * separator )
{
    GtkTreeStore * model;
    GType * types;
    gint size, i;
    GSList * list;

    size = csv_import_count_columns ( contents, separator );
    printf (">> SIZE is %d\n", size );
    if ( ! size )
    {
	return NULL;
    }

    csv_fields_config = csv_import_update_fields_config ( contents, size );

    /* Remove previous columns if any. */
    list = (GSList *) gtk_tree_view_get_columns ( GTK_TREE_VIEW(tree_preview) );
    while ( list )
    {
	gtk_tree_view_remove_column ( tree_preview, list -> data );
	list = list -> next;
    }

    types = (GType *) malloc ( size * sizeof ( GType ) );
    for ( i = 0 ; i < size ; i ++ ) 
    {
	GtkTreeViewColumn * col;
	GtkCellRenderer * cell;
	GtkWidget * button;

	types[i] = G_TYPE_STRING;
	cell = gtk_cell_renderer_text_new ();

	col = gtk_tree_view_column_new_with_attributes ( csv_fields [ csv_fields_config[i] ] . name,
							 cell, "text", i,
							 NULL);
	gtk_tree_view_append_column ( tree_preview, col );

	gtk_tree_view_column_set_clickable ( col, TRUE );
	g_object_set_data ( G_OBJECT ( col -> button ), "column", col );
	g_signal_connect ( G_OBJECT ( col -> button ),
			   "button-press-event",
			   G_CALLBACK ( csv_import_header_on_click ),
			   GINT_TO_POINTER ( i ) );
    }

    model =  gtk_tree_store_newv ( size, types );

    return (GtkTreeModel *) model;
}



/**
 *
 *
 *
 */
gboolean csv_import_header_on_click ( GtkWidget * button, GdkEventButton * ev, 
				      gint *no_column )
{
    GtkWidget * menu, * col;

    col = g_object_get_data ( G_OBJECT ( button ), "column" );

    menu = csv_import_fields_menu ( col, GPOINTER_TO_INT ( no_column ) );
    gtk_menu_popup ( GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
    
    return FALSE;
}



/**
 *
 *
 *
 */
gchar * csv_import_guess_separator ( gchar * contents )
{
    gchar * separators[] = { ",", ";", "	", " ", NULL };
    gint i;

    for ( i = 0 ; separators[i] ; i++ )
    {
	if ( csv_import_try_separator ( contents, separators[i] ) )
	{
	    return g_strdup ( separators[i] );
	}
    }

    /* Comma is the most used separator, so as we are puzzled we try
     * this one. */
    return ",";
}



/**
 *
 *
 *
 */
gboolean csv_import_try_separator ( gchar * contents, gchar * separator )
{
    GSList * list;
    int cols, i = 0;

    do 
    {
	list = csv_parse_line ( &contents, separator );
    }
    while ( list ==  GINT_TO_POINTER(-1) );
    cols = g_slist_length ( list );
    printf ("> I believe first line is %d cols\n", cols );

    do
    {
	list = csv_parse_line ( &contents, separator );
	if ( list == GINT_TO_POINTER(-1) )
	{
	    continue;
	}

	if ( cols != g_slist_length ( list ) || cols == 1 )
	{
	    printf ("> %d != %d, not %s\n", cols, g_slist_length ( list ), separator );
	    return FALSE;
	}
	
	i++;
    } 
    while ( list && i < 10 );

    printf ("> I believe separator is %s\n", separator );
    return TRUE;
}



/**
 *
 *
 */
gint csv_import_count_columns ( gchar * contents, gchar * separator )
{
    gint max = 0, i = 0;
    GSList * list;

    do
    {
	list = csv_parse_line ( &contents, separator );
	
	if ( list == GINT_TO_POINTER(-1) )
	{
	    continue;
	}

	if ( g_slist_length ( list ) > max )
	{
	    max = g_slist_length ( list );
	}

	i++;
    } 
    while ( list && i < 5 );

    return max;
}



/**
 *
 *
 */
gint * csv_import_update_fields_config ( gchar * contents, gint size )
{
    gint i, * old_csv_fields_config = csv_fields_config;

    g_return_if_fail ( size );

    if ( ! old_csv_fields_config )
    {
	return csv_import_guess_fields_config ( contents, size );
    }

    csv_fields_config = (gint *) malloc ( ( size + 2 ) * sizeof ( gint ) );

    for ( i = 0; i < size && old_csv_fields_config [ i ] != -1 ; i ++ )
    {
	csv_fields_config [ i ] = old_csv_fields_config [ i ];
    }

    for ( ; i < size ; i ++ )
    {
	csv_fields_config[i] = 0;
    }

    free ( old_csv_fields_config );
    csv_fields_config [ i ] = -1;    

    return csv_fields_config;
}



/**
 *
 *
 *
 */
gint * csv_import_guess_fields_config ( gchar * contents, gint size )
{
    gint * default_config;
    gint benj_config[13] = { 0, 0, 2, 13, 4, 10, 12, 0, 7, 8, 5, 0 };

    default_config = (gint *) malloc ( size * sizeof ( int ) );
/*     bzero ( default_config, size * sizeof(int) );  */
    bcopy ( benj_config, default_config, size * sizeof(int) );

    default_config [ size ] = -1;

    return default_config;
}



/**
 *
 *
 *
 */
gboolean csv_import_change_separator ( GtkEntry * entry, gchar * value, 
				       gint length, gint * position )
{
    gchar * separator = (gchar *) gtk_entry_get_text ( GTK_ENTRY (entry) );
    GtkWidget * assistant = g_object_get_data ( G_OBJECT(entry), "assistant" );

    printf (">> change separator\n" );
    g_object_set_data ( G_OBJECT(assistant), "separator", separator );
    
    if ( strlen ( separator ) )
    {
	csv_import_update_preview ( assistant );
    }

    return FALSE;
}



gboolean csv_import_update_preview ( GtkWidget * assistant )
{
    gchar * contents, * separator, * label = g_strdup ("");
    GtkTreeModel * model;
    GtkTreeView * tree_preview;
    GSList * list;
    gint line = 0;

    separator = g_object_get_data ( G_OBJECT(assistant), "separator" );
    tree_preview = g_object_get_data ( G_OBJECT(assistant), "tree_preview" );
    contents =  g_object_get_data ( G_OBJECT(assistant), "contents" );

    model = csv_import_create_model ( tree_preview, contents, separator );
    if ( model )
    {
	gtk_tree_view_set_model ( GTK_TREE_VIEW(tree_preview), model );
    }

    while ( line < 5 )
    {
	GtkTreeIter iter;
	gint i = 0;

	do
	{
	    list = csv_parse_line ( &contents, separator );
	}
	while ( list == GINT_TO_POINTER(-1) );

	if ( ! list )
	{
	    return FALSE;
	}

	gtk_tree_store_append (GTK_TREE_STORE(model), &iter, NULL);
	while ( list )
	{
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter, i, 
				 gsb_string_truncate ( list -> data ), -1 ); 
	    if ( list -> data )
		label = g_strconcat ( label, "[[", list -> data, "]]\n", NULL );

	    i++;

	    list = list -> next;
	}

	line++;
    }

    return FALSE;
}



/**
 *
 *
 *
 */
GtkWidget * csv_import_fields_menu ( GtkTreeViewColumn * col, gint field )
{
    GtkWidget * menu, * item;
    int i;

    menu = gtk_menu_new();

    for ( i = 0 ; csv_fields[i] . name ; i++ )
    {
	item = gtk_menu_item_new_with_label ( (gchar *) csv_fields[i] . name );
	g_object_set_data ( G_OBJECT ( item ), "column", col );
	g_object_set_data ( G_OBJECT ( item ), "field", (gpointer) field );
	gtk_signal_connect ( GTK_OBJECT ( item ), "activate",
			     GTK_SIGNAL_FUNC ( csv_import_change_field ), (gpointer) i );
	gtk_menu_append ( GTK_MENU ( menu ), item );
    }

    gtk_widget_show_all ( menu );
    return menu;
}



/**
 *
 *
 */
gboolean csv_import_change_field ( GtkWidget * item, gint no_menu )
{
    GtkTreeViewColumn * col;
    gint field;

    col = g_object_get_data ( G_OBJECT(item), "column" );
    field = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT(item), "field" ) );

    gtk_tree_view_column_set_title ( col, csv_fields [ no_menu ] . name );
    csv_fields_config [ field ] = no_menu;

    return FALSE;
}



/**
 *
 *
 *
 */
gboolean import_enter_csv_preview_page ( GtkWidget * assistant )
{
    GtkWidget * entry;
    GSList * files;
    gchar * contents, * filename;
    gsize * size;
    GError * error;

    /* Find first CSV to import. */
    files = import_selected_files ( assistant );
    while ( files )
    {
	struct imported_file * imported = files -> data;

	if ( imported -> type == TYPE_CSV )
	{
	    filename = imported -> name;
	    break;
	}
	files = files -> next;
    }
    g_return_val_if_fail ( filename, FALSE );

    /* Open file */
    if ( ! g_file_get_contents ( filename, &contents, &size, &error ) )
    {
	printf ("Unable to read file: %s\n", error -> message);
	return FALSE;
    }

    g_object_set_data ( G_OBJECT(assistant), "contents", contents );

    entry = g_object_get_data ( G_OBJECT(assistant), "entry" );
    if ( entry )
    {
	gtk_entry_set_text ( GTK_ENTRY(entry), csv_import_guess_separator ( contents ) );
    }

    return FALSE;
}



/**
 *
 *
 *
 */
gboolean csv_import_csv_account ( GtkWidget * assistant, gchar * filename )
{
    gchar * contents, * separator;
    GSList * list;

    compte = calloc ( 1, sizeof ( struct struct_compte_importation ));
    compte -> type_de_compte = 0;
    compte -> solde = 0;
    compte -> nom_de_compte = g_strdup ( _("Imported account with no name" ));
    compte -> origine = CSV_IMPORT;
    compte -> filename = filename;
    compte -> operations_importees = NULL;

    contents = g_object_get_data ( G_OBJECT(assistant), "contents" );
    separator = g_object_get_data ( G_OBJECT(assistant), "separator" );

    if ( ! csv_fields_config )
    {
	liste_comptes_importes_error = g_slist_append ( liste_comptes_importes_error, 
							compte );
	return FALSE;
    }

    do
    {
	list = csv_parse_line ( &contents, separator );
    }
    while ( list == GINT_TO_POINTER(-1) );

    list = csv_parse_line ( &contents, separator );
	
    do
    {
	struct struct_ope_importation * ope;
	int i;

	ope = malloc ( sizeof ( struct struct_ope_importation ) );
	bzero ( ope, sizeof ( struct struct_ope_importation ) );
	ope -> date = gdate_today ();
	ope -> id_operation = NULL;
	ope -> date_tmp = g_strdup ( "" );
	ope -> tiers = g_strdup ( "" );
	ope -> notes = g_strdup ( "" );
	ope -> categ = g_strdup ( "" );
	ope -> guid = g_strdup ( "" );

	if ( list == GINT_TO_POINTER(-1) )
	{
	    list = csv_parse_line ( &contents, separator );
	    continue;
	}

	for ( i = 0; csv_fields_config[i] != -1 && list ; i++)
	{
	    struct csv_field * field = & csv_fields [ csv_fields_config[i] ];
	    if ( field -> parse )
	    {
		printf ("> Parsing %s as %s ... ",  list -> data, field -> name );
		if ( field -> validate )
		{
		    if ( field -> validate ( list -> data ) )
		    {
			if ( ! field -> parse ( ope, list -> data ) )
			{
			    printf ("(failed)");
			}
		    }
		    else
		    {
			printf ("(invalid)");
		    }
		}
		printf ("\n");
	    }
	    list = list -> next;
	}

	list = csv_parse_line ( &contents, separator );

	printf (">> Appending new transaction %p\n", ope );
	compte -> operations_importees = g_slist_append ( compte -> operations_importees,
							  ope );
    }
    while ( list );

    if ( compte -> operations_importees )
    {
	/* Finally, we register it. */
	liste_comptes_importes = g_slist_append ( liste_comptes_importes, compte );
    }
    else
    {
	/* ... or not, if no transaction was imported (implement sanitizing). */
	liste_comptes_importes_error = g_slist_append ( liste_comptes_importes_error, 
							compte );
    }

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
