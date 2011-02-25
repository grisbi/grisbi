/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2003 Cédric Auger  (cedric@grisbi.org)           */
/*          2004-2006 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2009 Pierre Biava (grisbi@pierre.biava.name)     */
/*          http://www.grisbi.org                                             */
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
#include "import_csv.h"
#include "csv_parse.h"
#include "dialog.h"
#include "utils_dates.h"
#include "gsb_automem.h"
#include "utils_str.h"
#include "import.h"
#include "utils.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern GSList *liste_comptes_importes_error;
extern GtkWidget *window;
/*END_EXTERN*/

/*START_STATIC*/
static gboolean csv_find_field_config ( gint searched );
static GSList * csv_get_next_line ( gchar ** contents, gchar * separator );
static gboolean csv_import_change_field ( GtkWidget * item, gint no_menu );
static gboolean csv_import_change_separator ( GtkEntry * entry,
                        GtkWidget *assistant );
static gboolean csv_import_combo_changed ( GtkComboBox * combo, GtkEntry * entry );
static gint csv_import_count_columns ( gchar * contents, gchar * separator );
static GtkTreeModel * csv_import_create_model ( GtkTreeView * tree_preview, gchar * contents,
                        gchar * separator );
static GtkWidget * csv_import_fields_menu ( GtkTreeViewColumn * col, gint field,
                        GtkWidget * assistant );
static gint * csv_import_guess_fields_config ( gchar * contents, gint size, gchar * separator );
static gchar * csv_import_guess_separator ( gchar * contents );
static gboolean csv_import_header_on_click ( GtkWidget * button, gint *no_column );
static gint csv_import_try_separator ( gchar * contents, gchar * separator );
static gint * csv_import_update_fields_config ( gchar * contents, gint size, gchar * separator );
static gboolean csv_import_update_preview ( GtkWidget * assistant );
static void csv_import_update_validity_check ( GtkWidget * assistant );
static gint csv_skip_lines ( gchar ** contents, gint num_lines, gchar * separator );
static gboolean safe_contains ( gchar * original, gchar * substring );
static void skip_line_toggled ( GtkCellRendererToggle * cell, gchar * path_str,
                        GtkTreeView * tree_preview );
/*END_STATIC*/


/** Array of pointers to fields.  */
static gint * csv_fields_config = NULL;

/** Contain configuration of CSV fields.  */
struct csv_field csv_fields[18] = {
    { N_("Unknown field"),  0.0, NULL,			     NULL		     , "" },
    { N_("Currency"),	    0.0, csv_import_validate_string, csv_import_parse_currency, "" },
    { N_("Date"),	    0.0, csv_import_validate_date,   csv_import_parse_date, "" },
    { N_("Value date"),	    0.0, csv_import_validate_date,   csv_import_parse_value_date, "" },
    { N_("Payee"),	    0.0, csv_import_validate_string, csv_import_parse_payee, N_("Wording") },
    { N_("Notes"),	    0.0, csv_import_validate_string, csv_import_parse_notes, "" },
    { N_("Voucher number"), 0.0, csv_import_validate_number, csv_import_parse_voucher, "" },
    { N_("Category"),	    0.0, csv_import_validate_string, csv_import_parse_category, "" },
    { N_("Sub-categories"),   0.0, csv_import_validate_string, csv_import_parse_sub_category, "" },
	{ N_("Budgetary lines"), 0.0, csv_import_validate_string, csv_import_parse_budget, "" },
	{ N_("Sub-budgetary lines"), 0.0, csv_import_validate_string, csv_import_parse_sub_budget, "" },
    { N_("Balance"),        0.0, csv_import_validate_amount, csv_import_parse_balance, "" },
    { N_("Credit (amount)"), 0.0, csv_import_validate_amount, csv_import_parse_credit, "" },
    { N_("Debit (absolute)"),0.0, csv_import_validate_amount, csv_import_parse_debit, N_("Debit") },
    { N_("Debit (negative)"),0.0, csv_import_validate_amount, csv_import_parse_credit, "" },
    { N_("C/R"),	    0.0, csv_import_validate_string, csv_import_parse_p_r, "" },
    { N_("Split"),	    0.0, csv_import_validate_string, csv_import_parse_split, "" },
    { NULL },
};


/** Contain pre-defined CSV separators */
struct csv_separators {
    gchar * name;		/** Visible name of CSV separator */
    gchar * value; 		/** Real value */
} csv_separators[] =		/* Contains all pre-defined CSV separators. */
{
    { N_("Comma"),		"," },
    { N_("Semi-colon"),	";" },
    { N_("Colon"),		":" },
    { N_("Tabulation"),	"\t" },
    { N_("Other"),		NULL },
};



/**
 * Create the preview page of the import assistant.
 *
 * \param assistant	GsbAssistant to add page into.
 *
 * \return		A newly-allocated GtkVBox.
 */
GtkWidget * import_create_csv_preview_page ( GtkWidget * assistant )
{
    GtkWidget * vbox, * paddingbox, * tree_preview, * entry, * sw, * validity_label;
    GtkWidget * warn, * hbox, * combobox;
    int i = 0;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Choose CSV separator") );

    hbox = gtk_hbox_new ( FALSE, 12 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    combobox = gtk_combo_box_new_text ();
    do
    {
	gchar * complete_name = g_strdup_printf ( "%s : \"%s\"",
						  _(csv_separators [ i ] . name),
						  ( csv_separators [ i ] . value ?
						    csv_separators [ i ] . value : "" ) );
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), complete_name );
	g_free ( complete_name );

    }
    while ( csv_separators [ i ++ ] . value );

    gtk_box_pack_start ( GTK_BOX(hbox), combobox, TRUE, TRUE, 0 );

    entry = gsb_automem_entry_new ( NULL,
                        G_CALLBACK ( csv_import_change_separator ),
                        assistant );
    g_object_set_data ( G_OBJECT(entry), "assistant", assistant );
    g_object_set_data ( G_OBJECT(entry), "combobox", combobox );
    g_object_set_data ( G_OBJECT(assistant), "entry", entry );
    gtk_box_pack_start ( GTK_BOX(hbox), entry, FALSE, FALSE, 0 );

    g_signal_connect ( G_OBJECT ( combobox ),
                        "changed",
		                G_CALLBACK ( csv_import_combo_changed ),
                        entry );

    paddingbox = new_paddingbox_with_title ( vbox, TRUE, _("Select CSV fields") );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request ( sw, 480, 120 );
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX(paddingbox), sw, TRUE, TRUE, 6 );

    tree_preview = gtk_tree_view_new ();
    g_object_set_data ( G_OBJECT(assistant), "tree_preview", tree_preview );
    g_object_set_data ( G_OBJECT(tree_preview), "assistant", assistant );
    gtk_container_add (GTK_CONTAINER (sw), tree_preview);

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    warn = gtk_image_new_from_stock ( GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_BUTTON );
    gtk_box_pack_start ( GTK_BOX(hbox), warn, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT(assistant), "validity_icon", warn );

    validity_label = gtk_label_new (NULL);
    gtk_misc_set_alignment ( GTK_MISC ( validity_label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( validity_label ), GTK_JUSTIFY_LEFT );
    g_object_set_data ( G_OBJECT(assistant), "validity_label", validity_label );
    gtk_box_pack_start ( GTK_BOX(hbox), validity_label, TRUE, TRUE, 0 );

    return vbox;
}



/**
 * Create the model containing CSV file preview.
 *
 * \param tree_preview		GtkTreeView to associate model to.
 * \param contents		Contents to place into model
 *
 *
 */
GtkTreeModel * csv_import_create_model ( GtkTreeView * tree_preview, gchar * contents,
                        gchar * separator )
{
    GtkWidget * assistant;
    GtkTreeStore *model;
    GtkTreeViewColumn * col;
    GtkCellRenderer * cell;
    GType *types;
    gint size, i;
    GList *list;

    size = csv_import_count_columns ( contents, separator );
    if ( ! size || ! contents)
	    return NULL;

    csv_fields_config = csv_import_update_fields_config ( contents, size, separator );

    assistant = g_object_get_data ( G_OBJECT(tree_preview), "assistant" );

    /* Remove previous columns if any. */
    list = gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_preview ) );

    while ( list )
    {
        gtk_tree_view_remove_column ( tree_preview, list -> data );
        list = list -> next;
    }

    types = (GType *) g_malloc0 ( ( size + 2 ) * sizeof ( GType * ) );

    types[0] = G_TYPE_BOOLEAN;
    cell = gtk_cell_renderer_toggle_new ();
    col = gtk_tree_view_column_new_with_attributes ( _("Skip"),
					    cell,
                        "active", 0,
					    NULL);
    gtk_tree_view_append_column ( tree_preview, col );
    g_object_set_data ( G_OBJECT ( col ), "assistant", assistant );
    g_signal_connect ( cell,
                        "toggled",
                        G_CALLBACK ( skip_line_toggled ),
                        tree_preview );

    for ( i = 0 ; i < size ; i ++ )
    {
        GtkWidget *label;
        gchar *name;

        types[i+1] = G_TYPE_STRING;

        cell = gtk_cell_renderer_text_new ();
        col = gtk_tree_view_column_new_with_attributes ( 
                        NULL,
                        cell,
                        "text", i + 1,
                        "strikethrough", 0,
                        NULL);

        if ( csv_fields_config[i] > 0 )
        {
            name = g_strconcat ( "<b><u>", _( csv_fields [ csv_fields_config[i] ] . name ),
                     "</u></b>", NULL );
        }
        else
        {
            name = my_strdup (_( csv_fields [ csv_fields_config[i] ] . name ));
        }

        label = gtk_label_new ( NULL );
        gtk_label_set_markup ( GTK_LABEL(label), name );
        gtk_widget_show ( label );
        g_free ( name );

        gtk_tree_view_column_set_widget ( col, label );
        gtk_tree_view_append_column ( tree_preview, col );
        gtk_tree_view_column_set_clickable ( col, TRUE );
        g_object_set_data ( G_OBJECT ( col ), "column", col );
        g_object_set_data ( G_OBJECT ( col ), "assistant", assistant );
        g_signal_connect ( G_OBJECT ( col ),
                   "clicked",
                   G_CALLBACK ( csv_import_header_on_click ),
                   GINT_TO_POINTER ( i + 1 ) );
    }

    model =  gtk_tree_store_newv ( size + 1, types );

    return (GtkTreeModel *) model;
}


/**
 * Callback triggered when the "skip" attribute of a line in the tree
 * preview is changed (that is, the checkbox associated is changed).
 * It marks this line as skipped.
 *
 * \param cell		Not used.
 * \param path_str	Textual representation of the path of modified
 *			checkbox.
 * \param tree_preview	GtkTreeView triggering event.
 */
void skip_line_toggled ( GtkCellRendererToggle * cell, gchar * path_str,
                        GtkTreeView * tree_preview )
{
    GtkTreeIter iter;
    gboolean toggle_item;
    gint * indices;
    GtkTreePath * path = gtk_tree_path_new_from_string (path_str);
    GtkTreeModel * tree_model = gtk_tree_view_get_model ( tree_preview );;

    /* Get toggled iter */
    gtk_tree_model_get_iter ( GTK_TREE_MODEL ( tree_model ), &iter, path );
    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), &iter, 0, &toggle_item, -1 );
    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ), &iter, 0, !toggle_item, -1);

    indices = gtk_tree_path_get_indices ( path );
    etat.csv_skipped_lines [ indices[0] ] = !toggle_item;
}



/**
 * Triggered when user clicks on a column header.  Create and pop up a
 * menu to allow CSV config change.
 *
 * \param button	Button placed at the title column.
 * \param ev		Not used.
 * \param no_column	Position of the column.
 *
 * \return		FALSE
 */
gboolean csv_import_header_on_click ( GtkWidget * button, gint *no_column )
{
    GtkWidget * menu, * col;

    col = g_object_get_data ( G_OBJECT ( button ), "column" );

    menu = csv_import_fields_menu ( GTK_TREE_VIEW_COLUMN ( col ),
				    GPOINTER_TO_INT ( no_column ),
				    g_object_get_data ( G_OBJECT(col), "assistant" ) );
    gtk_menu_popup ( GTK_MENU(menu), NULL, NULL, NULL, NULL, 3,
		     gtk_get_current_event_time());

    return FALSE;
}



/**
 * Based on raw text, try to estimate which character is the CSV
 * separator.  For each separator, try to estimate if this would make
 * consistent lines with the same amount of columns.  Of course, if
 * CSV is broken, it will fail and revert back to comma as a
 * separator, which seems the most used nowadays.
 *
 * This is black magic, believe me !
 *
 * \param contents	Raw CSV text to parse.
 *
 * \return		A newly-allocated string containing estimated
 *			or default CSV separator.
 */
gchar * csv_import_guess_separator ( gchar * contents )
{
    gchar * separators[5] = { ",", ";", "	", " ", NULL }, * cmax = NULL;
    gint i, max = 0;

    for ( i = 0 ; separators[i] ; i++ )
    {
	gchar * tmp = contents;
	int n = csv_import_try_separator ( tmp, separators[i] );

	if ( n > max )
	{
	    max = n;
	    cmax = separators[i];
	}
    }

    if ( cmax )
    {
	return my_strdup ( cmax );
    }

    /* Comma is the most used separator, so as we are puzzled we try
     * this one. */
    return my_strdup ( "," );
}



/**
 * Try to match separator against raw CSV contents and see if it would
 * be consistent, see csv_import_guess_separator().
 *
 * \param contents	Raw CSV contents to parse.
 * \param separator	Separator to try.
 *
 * \return		FALSE on failure, number of columns
 *			otherwise.
 */
gint csv_import_try_separator ( gchar * contents, gchar * separator )
{
    GSList * list;
    int cols, i = 0;

    csv_skip_lines ( &contents, 3, separator);

    list = csv_get_next_line ( &contents, separator );
    cols = g_slist_length ( list );
    g_print ("> I believe first line is %d cols\n", cols );

    do
    {
	list = csv_get_next_line ( &contents, separator );

	if ( list && ( cols != g_slist_length ( list ) || cols == 1 ) )
	{
	    g_print ("> %d != %d, not %s\n", cols, g_slist_length ( list ), separator );
	    return FALSE;
	}

	i++;
    }
    while ( list && i < CSV_MAX_TOP_LINES );

    g_print ("> I believe separator could be %s\n", separator );
    return cols;
}



/**
 * Count number of columns if a raw CSV text were parsed using
 * a separator.
 *
 * \param contents	Raw CSV contents to parse.
 * \param separator	Separator.
 *
 * \return		Number of columns.
 */
gint csv_import_count_columns ( gchar * contents, gchar * separator )
{
    gint max = 0, i = 0;
    GSList * list;
    gchar * tmp = contents;

    if ( ! contents )
	return 0;

    do
    {
	list = csv_get_next_line ( &tmp, separator );

	if ( g_slist_length ( list ) > max )
	{
	    max = g_slist_length ( list );
	}

	i++;
    }
    while ( list && i < CSV_MAX_TOP_LINES );

    return max;
}



/**
 * Parse raw CSV text using separator and return a list containing all
 * fields.
 *
 * \param contents	A pointer to raw CSV contents to parse.
 *			Pointer content will be changed to the end of
 *			parsed line.
 * \param separator	Separator to use.
 *
 * \return		Parsed list or NULL upon failure (last line).
 */
GSList * csv_get_next_line ( gchar ** contents, gchar * separator )
{
    GSList * list;

    do
    {
	list = csv_parse_line ( contents, separator );
    }
    while ( list == GINT_TO_POINTER(-1) );

    return list;
}



/**
 * Skip n lines in the CSV stream.
 *
 * \param contents	Pointer to CSV data.
 * \param num_lines	Number of lines to skip.
 *
 * \return Actual number of lines skipped.
 */
gint csv_skip_lines ( gchar ** contents, gint num_lines, gchar * separator )
{
    GSList * list;
    int i;

    /* g_print ("Skipping %d lines\n", num_lines ); */

    for ( i = 0; i < num_lines; i ++ )
    {
	list = csv_get_next_line ( contents, separator );

	if ( ! list )
	{
	    return i;
	}
    }

    return i;
}



/**
 * Update the CSV fields config structure to a new size.  Mainly used
 * when separator changes so that we do the allocation.
 *
 * \param contents	Raw CSV contents, used to guess config if not
 *			set already.
 * \param size		New size of allocation.
 * \paran separator	Separator, used to guess config if not set
 *			already.
 *
 * \return		A newly allocated integers array.
 */
gint * csv_import_update_fields_config ( gchar * contents, gint size, gchar * separator )
{
    gint i, * old_csv_fields_config = csv_fields_config;

    g_return_val_if_fail ( size, NULL );

    if ( ! old_csv_fields_config )
    {
	return csv_import_guess_fields_config ( contents, size, separator );
    }

    csv_fields_config = (gint *) g_malloc ( ( size + 2 ) * sizeof ( gint ) );

    for ( i = 0; i < size && old_csv_fields_config [ i ] != -1 ; i ++ )
    {
	csv_fields_config [ i ] = old_csv_fields_config [ i ];
    }

    for ( ; i < size ; i ++ )
    {
	csv_fields_config[i] = 0;
    }

    if ( old_csv_fields_config )
    {
	g_free ( old_csv_fields_config );
    }
    csv_fields_config [ i ] = -1;

    return csv_fields_config;
}



/**
 * Safely checks if a string is contained in another one.
 *
 * \param original	String to search substring into.
 * \param substring	Substring to search into original string.
 *
 * \return		TRUE if substring is contained into original.
 */
gboolean safe_contains ( gchar * original, gchar * substring )
{
    g_return_val_if_fail ( original, FALSE );
    g_return_val_if_fail ( substring, FALSE );

    return GPOINTER_TO_INT( g_strstr_len (
                        g_utf8_strdown ( original, -1 ),
                        strlen ( original ),
                        g_utf8_strdown ( substring, -1 ) ) );
}



/**
 * Pre-fills the CSV field configuration using various heuristics.
 * Should match most cases thought this is purely statistical.
 *
 * \param contents	Data extracted from CSV file.
 * \param size		Number of columns of data (dependant of
 *			separator).
 * \param separator	Columns separator.
 *
 * \return		A newly allocated int array, containg guessed fields.
 */
gint * csv_import_guess_fields_config ( gchar * contents, gint size, gchar * separator )
{
    gchar * string;
    gint line, i, * default_config;
    GSList * list;

    default_config = (gint *) g_malloc0 ( ( size + 1 ) * sizeof ( int ) );

    list = csv_get_next_line ( &contents, separator );
    if ( ! list )
	return default_config;

    /** First, we try to match first line because it might contains of
     * the fields.  */
    for ( i = 0 ; i < size && list ; i ++ )
    {
	gint field;

	gchar * value = list -> data;

	for ( field = 0 ; csv_fields [ field ] . name != NULL ; field ++ )
	{
	    if ( strlen ( value ) > 1 &&
		 strlen ( csv_fields [ field ] . name ) > 1 )
        {
            if ( strlen ( csv_fields [ field ] . alias ) > 1 
             &&
             ( safe_contains ( csv_fields [ field ] . name, value ) ||
               safe_contains ( _( csv_fields [ field ] . name ), value ) ||
               safe_contains ( csv_fields [ field ] . alias, value ) ||
               safe_contains ( _( csv_fields [ field ] . alias ), value ) ) )
            {
                if ( !default_config [ i ] )
                {
                    default_config [ i ] = field;
                    etat.csv_skipped_lines [ 0 ] = 1;
                }

            }
            else if ( safe_contains ( csv_fields [ field ] . name, value ) ||
             safe_contains ( _( csv_fields [ field ] . name ), value ) )
            {
                if ( !default_config [ i ] )
                {
                    default_config [ i ] = field;
                    etat.csv_skipped_lines [ 0 ] = 1;
               }
            }
        }
	}

	list = list -> next;
    }

    csv_skip_lines ( &contents, 3, separator );

    /** Then, we try using heuristics to determine which field is date
     * and which ones contain amounts.  We cannot guess payees or
     * comments so we only auto-detect these fields. */
    for ( line = 0; line < CSV_MAX_TOP_LINES ; line ++ )
    {
	gboolean date_validated = 0;

	list = csv_get_next_line ( &contents, separator );

	if ( ! list )
	    return default_config;

	for ( i = 0 ; i < size && list ; i ++ )
	{
	    string = list -> data;

	    if ( strlen ( string ) )
	    {
		if ( csv_import_validate_date ( string ) && ! date_validated &&
		     ! default_config [ i ] )
		{
		    default_config [ i ] = 2; /* Date */
		    date_validated = TRUE;
		}
		else if ( csv_import_validate_amount ( string ) &&
			  ! csv_import_validate_number ( string ) &&
			  strlen(string) <= 9 )	/* This is a hack
						 * since most numbers
						 are smaller than 8 chars. */
		{
		    if ( g_strrstr ( string, "-" ) ) /* This is negative */
		    {
                if ( ! default_config [ i ] )
                {
                    default_config [ i ] = 14; /* Negative debit */
                }
                else if ( default_config [ i ] == 12 )
                {
                    default_config [ i ] = 11; /* Neutral amount */
                }
		    }
		    else
		    {
                if ( ! default_config [ i ] )
                {
                    default_config [ i ] = 12; /* Negative debit */
                }
                else if ( default_config [ i ] == 14 )
                {
                    default_config [ i ] = 11; /* Neutral amount */
                }
		    }
		}
	    }
	    list = list -> next;
	}

	default_config [ size ] = -1;
    }

    return default_config;
}



/**
 * Callback triggered when user changed the pre-defined csv separators
 * combobox.  Update the text entry and thus the preview.
 *
 * \param combo		GtkComboBox that triggered event.
 * \param entry		Associated entry to change.
 *
 * \return		FALSE
 */
gboolean csv_import_combo_changed ( GtkComboBox * combo, GtkEntry * entry )
{
    gint active = gtk_combo_box_get_active ( combo );

    if ( csv_separators [ active ] . value )
    {
	gtk_entry_set_text ( entry, csv_separators [ active ] . value );
    }
    else
    {
	gtk_entry_set_text ( entry, "" );
    }

    return FALSE;
}



/**
 * Callback triggered when separator is changed in the GtkEntry
 * containing it.
 *
 * \param entry		Entry that triggered event.
 * \param value		New value of entry (not used).
 * \param length	Length of the change (not used).
 * \param position	Position of the change (not used).
 *
 * \return		FALSE
 */
gboolean csv_import_change_separator ( GtkEntry * entry,
                        GtkWidget *assistant )
{
    GtkWidget *combobox;
    gchar *separator;
    int i = 0;

    combobox = g_object_get_data ( G_OBJECT(entry), "combobox" );
    separator = g_strdup ( gtk_entry_get_text ( GTK_ENTRY (entry) ) );
    if ( strlen ( separator ) > 0 )
    {
        g_object_set_data ( G_OBJECT(assistant), "separator", separator );
        csv_import_update_preview ( assistant );
        etat.csv_separator = my_strdup ( separator );
    }
    else
    {
        etat.csv_separator = "";
        g_object_set_data ( G_OBJECT(assistant), "separator", NULL );
    }

    /* Update combobox if we can. */
    while ( csv_separators [ i ] . value )
    {
        if ( strcmp ( csv_separators [ i ] . value, separator ) == 0 )
        {
            break;
        }
        i ++ ;
    }
    g_signal_handlers_block_by_func ( combobox, csv_import_combo_changed, entry );
    gtk_combo_box_set_active ( GTK_COMBO_BOX(combobox), i );
    g_signal_handlers_unblock_by_func ( combobox, csv_import_combo_changed, entry );

    return FALSE;
}



/**
 * Update tree view containing preview of CSV import.
 *
 * \param assistant	GsbAssistant containg all UI.
 *
 * \return		FALSE
 */
gboolean csv_import_update_preview ( GtkWidget * assistant )
{
    gchar * contents, * separator;
    GtkTreeModel * model;
    GtkTreeView * tree_preview;
    GSList * list;
    gint line = 0;

    separator = g_object_get_data ( G_OBJECT(assistant), "separator" );
    tree_preview = g_object_get_data ( G_OBJECT(assistant), "tree_preview" );
    contents = g_object_get_data ( G_OBJECT(assistant), "contents" );

    if ( ! contents || ! tree_preview || ! separator )
	return FALSE;

    assistant = g_object_get_data ( G_OBJECT(tree_preview), "assistant" );
    model = csv_import_create_model ( tree_preview, contents, separator );
    if ( model )
    {
        gtk_tree_view_set_model ( GTK_TREE_VIEW(tree_preview), model );
        g_object_unref (G_OBJECT(model));
    }

    while ( line < CSV_MAX_TOP_LINES )
    {
        GtkTreeIter iter;
        gint i = 1;

        list = csv_get_next_line ( &contents, separator );

        if ( ! list )
        {
            return FALSE;
        }

        gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, NULL);
        while ( list )
        {
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter, i,
                     gsb_string_truncate ( list -> data ), -1 );
            i++;
            list = list -> next;
        }

        if ( etat.csv_skipped_lines [ line ] )
        {
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter, 0, TRUE, -1 );
        }

        line++;
    }

    csv_import_update_validity_check ( assistant );

    return FALSE;
}



/**
 * Find if a specific option is set in CSV field config.
 *
 * \param searched	Field to test presence.
 *
 * \return		TRUE is found.  FALSE otherwise.
 */
gboolean csv_find_field_config ( gint searched )
{
    gint f;

    for ( f = 0 ; csv_fields_config [ f ] != -1 ; f ++ )
    {
	if ( csv_fields_config [ f ] == searched )
	{
	    return TRUE;
	}
    }

    return FALSE;
}



/**
 * Perform various checks on csv field configs.
 *
 * \param assistant	A pointer to the GsbAssistant holding CSV
 *			configuration.
 */
void csv_import_update_validity_check ( GtkWidget * assistant )
{
    int i, needed[] = { 2, 4, -1 };
    gchar * label = NULL;

    if ( ! csv_fields_config )
	return;

    /* Check all needed fields.  */
    for ( i = 0 ; needed [ i ] != -1 ; i ++ )
    {
	if ( ! csv_find_field_config ( needed [ i ] ) )
	{
	    if ( label )
	    {
		label = g_strconcat ( label, ", ", _( csv_fields [ needed [ i ] ] . name ),
				      NULL );
	    }
	    else
	    {
		label = _( csv_fields [ needed [ i ] ] . name );
	    }
	}
    }

    /** After checking all required fields, check the conformity of
     * transaction amount, which is somewhat complicated. */
    if ( !( ( csv_find_field_config ( 11 ) && !csv_find_field_config ( 12 ) &&
           !csv_find_field_config ( 13 ) && !csv_find_field_config ( 14 ) )
     ||
     ( !csv_find_field_config ( 11 ) &&  csv_find_field_config ( 12 ) && 
       ( csv_find_field_config ( 13 ) || csv_find_field_config ( 14 ) ) &&
       !( csv_find_field_config ( 13 ) && csv_find_field_config ( 14 ) ) ) ) )
    {
        if ( label )
        {
            label = g_strconcat ( label, ", ", _("transaction amount"), NULL );
        }
        else
        {
            label = my_strdup ( _("transaction amount") );
        }
    }

    /** Then, fill in a GtkLabel containing diagnostic message and
     * show or hide a warning icon.  */
    if ( label )
    {
	gtk_label_set_markup ( g_object_get_data ( G_OBJECT(assistant), "validity_label" ),
                        g_markup_printf_escaped (
                        _("<b>The following fields are missing or inconsistent:</b> %s"),
                        label ) );
	gtk_widget_show ( g_object_get_data ( G_OBJECT(assistant), "validity_icon" ) );
	gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant),
						       "button_next" ), FALSE );
    }
    else
    {
	gtk_label_set_markup ( g_object_get_data ( G_OBJECT(assistant), "validity_label" ),
			       _("All mandatory fields are filed in.") );
	gtk_widget_hide ( g_object_get_data ( G_OBJECT(assistant), "validity_icon" ) );
	gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant),
						       "button_next" ), TRUE );
    }
}



/**
 * Create a menu containing a link to all possible CSV fields.  This
 * is normally an event triggered by a click on a GtkTreeViewColumn.
 *
 * \param col		Column that triggered event.
 * \param field		Position of column that triggered event.
 * \param assistant	Assistant that contains the column.
 *
 * \return		A newly-created GtkMenu.
 */
GtkWidget * csv_import_fields_menu ( GtkTreeViewColumn * col, gint field,
                        GtkWidget * assistant )
{
    GtkWidget * menu, * item;
    int i;

    menu = gtk_menu_new();

    for ( i = 0 ; csv_fields[i] . name ; i++ )
    {
	item = gtk_menu_item_new_with_label ( (gchar *) _( csv_fields[i] . name ) );
	g_object_set_data ( G_OBJECT ( item ), "column", col );
	g_object_set_data ( G_OBJECT ( item ), "field", GINT_TO_POINTER( field - 1 ) );
	g_object_set_data ( G_OBJECT ( item ), "assistant", assistant );
	g_signal_connect ( G_OBJECT ( item ), "activate",
			     G_CALLBACK ( csv_import_change_field ), GINT_TO_POINTER(  i ) );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    }

    gtk_widget_show_all ( menu );
    return menu;
}



/**
 * Event triggered when a pop-up menu associated to a column is
 * triggered and option is selected.  Changes the CSV field config for
 * this column.  Title label of column is changed too.
 *
 * \param item		GtkMenuItem that triggered event.
 * \param no_menu	Position of column that triggered pop up menu.
 *
 * \return		FALSE
 */
gboolean csv_import_change_field ( GtkWidget * item, gint no_menu )
{
    GtkTreeViewColumn * col;
    GtkWidget * label;
    gchar * name;
    gint field;

    col = g_object_get_data ( G_OBJECT(item), "column" );
    field = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT(item), "field" ) );

    gtk_tree_view_column_set_title ( col, _( csv_fields [ no_menu ] . name ) );

    if ( no_menu > 0 )
    {
	name = g_strconcat ( "<b><u>", _( csv_fields [ no_menu ] . name ),
			     "</u></b>", NULL );
    }
    else
    {
	name = _( csv_fields [ no_menu ] . name );
    }
    label = gtk_label_new ( name );
    gtk_label_set_markup ( GTK_LABEL(label), name );
    gtk_widget_show ( label );
    gtk_tree_view_column_set_widget ( col, label );

    csv_fields_config [ field ] = no_menu;

    csv_import_update_validity_check ( g_object_get_data ( G_OBJECT(col), "assistant" ) );

    return FALSE;
}



/**
 * Fill in all graphical elements of preview page with data from a CSV
 * file.
 *
 * \param assistant	Assistant that contains the page.
 *
 * \return		FALSE
 */
gboolean import_enter_csv_preview_page ( GtkWidget * assistant )
{
    GtkWidget * entry;
    GSList * files;
    gchar *contents, *tmp_str, *filename = NULL;
    gsize size;
    gsize bytes_written;
    GError * error;
    struct imported_file * imported = NULL;

    /* Find first CSV to import. */
    files = import_selected_files ( assistant );
    while ( files )
    {
        imported = files -> data;

        if ( !strcmp ( imported -> type, "CSV" ) )
        {
            filename = imported -> name;
            break;
        }
        files = files -> next;
    }
    g_return_val_if_fail ( filename, FALSE );

    /* Open file */
    if ( ! g_file_get_contents ( filename, &tmp_str, &size, &error ) )
    {
        g_print ( _("Unable to read file: %s\n"), error -> message);
        g_error_free ( error );
        return FALSE;
    }

    /* Convert in UTF8 */
    error = NULL;
    contents = g_convert_with_fallback ( tmp_str, -1, "UTF-8", imported -> coding_system,
                        "?", &size, &bytes_written, &error );

    if ( contents == NULL )
    {
        g_error_free ( error );
        error = NULL;
        size = 0;
        bytes_written = 0;

        dialogue_special ( GTK_MESSAGE_WARNING, make_hint (
                            _("The conversion to utf8 went wrong."),
                            _("If the result does not suit you, try again by selecting the "
                            "correct character set in the window for selecting files.") ) );

        contents = g_convert_with_fallback ( tmp_str, -1, "UTF-8", "ISO-8859-1",
                        "?", &size, &bytes_written, &error );
        if ( bytes_written == 0 )
        {
            g_print ( _("Unable to read file: %s\n"), error -> message);
            g_error_free ( error );
            return FALSE;
        }
    }

    g_free ( tmp_str );
    g_object_set_data ( G_OBJECT(assistant), "contents", contents );

    entry = g_object_get_data ( G_OBJECT(assistant), "entry" );
    if ( entry )
    {
	if ( etat.csv_separator )
	{
	    gtk_entry_set_text ( GTK_ENTRY(entry), etat.csv_separator );
	}
	else
	{
	    gtk_entry_set_text ( GTK_ENTRY(entry), csv_import_guess_separator ( contents ) );
	}
    }

    csv_import_update_validity_check ( assistant );

    return FALSE;
}


/**
 * Actually do the grunt work, that is, parse the CSV file and create
 * importation structures in memory.
 *
 * \param assistant	The assistant that contains configuration for
 *			import.
 * \param imported	A pointer to the structure representing file
 *			import.
 *
 * \return		FALSE
 */
gboolean csv_import_csv_account ( GtkWidget * assistant, struct imported_file * imported )
{
    struct struct_compte_importation * compte;
    gchar * contents, * separator;
    GSList * list;
    int index = 0;

    compte = g_malloc0 ( sizeof ( struct struct_compte_importation ));
    compte -> nom_de_compte = unique_imported_name ( my_strdup ( _("Imported CSV account" ) ) );
    compte -> origine = my_strdup ( "CSV" );
    compte -> filename = my_strdup ( imported -> name );

    contents = g_object_get_data ( G_OBJECT(assistant), "contents" );
    separator = g_object_get_data ( G_OBJECT(assistant), "separator" );

    if ( ! csv_fields_config || ! contents )
    {
	liste_comptes_importes_error = g_slist_append ( liste_comptes_importes_error,
							compte );
	return FALSE;
    }

    list = csv_get_next_line ( &contents, separator );

    do
    {
        struct struct_ope_importation * ope;
        gint i;

        /* Check if this line was specified as to be skipped
         * earlier. */
        if ( index < CSV_MAX_TOP_LINES && etat.csv_skipped_lines [ index ] )
        {
            /* g_print ("Skipping line %d\n", index ); */
            list = csv_get_next_line ( &contents, separator );
            index++;
            continue;
        }
        index++;

        ope = g_malloc0 ( sizeof ( struct struct_ope_importation ) );
        ope -> date = gdate_today ();
        ope -> date_tmp = my_strdup ( "" );
        ope -> tiers = my_strdup ( "" );
        ope -> notes = my_strdup ( "" );
        ope -> categ = my_strdup ( "" );
        ope -> guid = my_strdup ( "" );

        for ( i = 0; csv_fields_config[i] != -1 && list ; i++)
        {
            struct csv_field * field = & csv_fields [ csv_fields_config[i] ];

            if ( field -> parse )
            {
            if ( field -> validate )
            {
                if ( field -> validate ( list -> data ) )
                {
                    if ( csv_fields_config[i] == 16 )
                    {
                        if ( field -> parse ( ope, list -> data ) )
                        {
                            gint nbre_element = g_slist_length (
                                compte -> operations_importees );
                            struct struct_ope_importation *ope_tmp;

                            ope_tmp = (struct struct_ope_importation *)
                                g_slist_nth_data  ( compte -> operations_importees,
                                                         nbre_element -1 );
                            if ( ope_tmp -> operation_ventilee == 0 &&
                             ope_tmp -> ope_de_ventilation == 0 )
                                ope_tmp -> operation_ventilee = 1;
                            ope -> ope_de_ventilation = 1;
                        }
                    }
                    else if ( ! field -> parse ( ope, list -> data ) )
                    {
                        /* g_print ("%s", "(failed)"); */
                    }
                }
                else
                    {
                    /* g_print ("%s", "(invalid)"); */
                    }
            }
            }
            list = list -> next;
        }

        /* g_print (">> Appending new transaction %p\n", ope ); */
        compte -> operations_importees = g_slist_append ( compte -> operations_importees,
                                  ope );

        list = csv_get_next_line ( &contents, separator );
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
