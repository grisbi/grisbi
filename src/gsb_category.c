/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
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

/**
 * \file gsb_category.c
 * fuctions to deal with the categories
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_category.h"
#include "etats_calculs.h"
#include "dialog.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_file_others.h"
#include "gsb_form_widget.h"
#include "gtk_combofix.h"
#include "utils_str.h"
#include "utils_files.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint compare_basename ( gchar * file1, gchar * file2 );
static void gsb_category_assistant_parse_file ( gchar * filename, GtkTreeModel * model );
static GSList * gsb_category_assistant_scan_directory ( gchar * basename, GtkTreeModel * model );
static void gsb_category_assistant_start_element ( GMarkupParseContext *context, const gchar * element_name,
					    const gchar ** attribute_names, const gchar ** attribute_values,
					    gchar ** return_string, GError ** error );
static gboolean gsb_category_display_foreign_toggled ( GtkWidget * button );
static void gsb_category_fill_sets ( GtkTreeView * view, GtkTreeModel * model, gboolean show_foreign );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/**
 * A handy GCompareFunc that compares two files depending on their
 * basename and not only the full string, containing a possibly
 * disturbing dirname.
 *
 * \param file1	String 1 to compare
 * \param file2	String 2 to compare
 *
 * \return A result suitable for use as of any GCompareFunc.
 */
gint compare_basename ( gchar * file1, gchar * file2 )
{
    gchar * base1 = g_path_get_basename ( file1 );
    gchar * base2 = g_path_get_basename ( file2 );
    gint result;
    
    result = strcmp ( base1, base2 );
    
    g_free ( base1 );
    g_free ( base2 );

    return result;
}



/**
 * Fill a GtkTreeModel with pre-defined category sets.
 *
 * \param view		A GtkTreeView that will be used to select
 *			first entry as de default choice.
 * \param model		A GtkListModel that will be filled.
 * \param show_foreign	Control whether foreign files should be included.
 */
void gsb_category_fill_sets ( GtkTreeView * view, GtkTreeModel * model, gboolean show_foreign )
{
    const gchar* const * languages = g_get_language_names ();
    GSList * category_files = NULL;

    /* First, we iterate over all locales except for C to avoid mixing
     * national languages and default one.  We will handle the C
     * locale later if nothing wirlds a match.  */
    while ( *languages )
    {
	GSList * list;

	if ( show_foreign || 
	     ( strlen ( (gchar *) *languages ) && strcmp ( (gchar *) *languages, "C" ) ) )
	{
	    list = gsb_category_assistant_scan_directory ( (gchar *) * languages, 
							   model );
	    while ( list )
	    {
		if ( ! g_slist_find_custom ( category_files, list -> data, 
					     (GCompareFunc) cherche_string_equivalente_dans_slist ) )
		{
		    category_files = g_slist_append ( category_files, list -> data );
		}
		list = list -> next;
	    }
	}

	languages++;
    }
    if ( ! category_files )
    {
	category_files = gsb_category_assistant_scan_directory ( "C", model );
    }
    
    category_files = g_slist_sort ( category_files, (GCompareFunc) compare_basename );

    while ( category_files )
    {
	gsb_category_assistant_parse_file ( (gchar * ) category_files -> data, 
					    model );

	category_files = category_files -> next;
    }

    gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( view ),
				     gtk_tree_path_new_first ( ) );
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( view ),
				  GTK_SELECTION_BROWSE );
}



/**
 * Handler that is responsible of toggling what is displayed in the 
 *
 * \param button		GtkButton that triggered this handler.
 *
 * \return			FALSE
 */
gboolean gsb_category_display_foreign_toggled ( GtkWidget * button )
{
    GtkTreeModel * model = g_object_get_data ( G_OBJECT(button), "model" );
    GtkTreeView * view = g_object_get_data ( G_OBJECT(button), "view" );

    gtk_list_store_clear ( GTK_LIST_STORE (model) );
    gsb_category_fill_sets ( view, model, (gboolean) gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(button) ) );

    return FALSE;
}



/**
 * create a widget to include in an assistant
 * it gives the choice between the kind of categories list we can create
 * later, to create the categories according to the choice,
 * 	call gsb_category_assistant_create_categories
 *
 * \param assistant the assistant wich that page will be added to
 * 	the choice will be set into the assistant widget with the key "choice_value"
 *		CATEGORY_CHOICE_NONE
 *		CATEGORY_CHOICE_DEFAULT
 *		CATEGORY_CHOICE_ASSOCIATION
 *		CATEGORY_CHOICE_LIBERAL
 *
 * \return a box to include in an assistant
 * */
GtkWidget *gsb_category_assistant_create_choice_page ( GtkWidget *assistant )
{
    GtkWidget * page, * sw, * radiobutton;
    GtkTreeModel * builtin_category_model;
    GtkTreeView * builtin_category_view;
    GtkCellRenderer *cell;
    gint col_offset;

    page = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(page), 12 );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_ALWAYS);

    builtin_category_model = (GtkTreeModel *) gtk_list_store_new (BUILTIN_CATEGORY_MODEL_COLUMNS,
								  G_TYPE_STRING, G_TYPE_STRING );
    builtin_category_view = (GtkTreeView *) gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( builtin_category_model ) );

    gtk_container_add ( GTK_CONTAINER ( sw ), (GtkWidget *) builtin_category_view );

    cell = gtk_cell_renderer_text_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes ( GTK_TREE_VIEW ( builtin_category_view ),
						      -1, _("Category list"),
						      cell, "markup",
						      BUILTIN_CATEGORY_MODEL_NAME,
						      NULL );
    gtk_box_pack_start ( GTK_BOX ( page ), GTK_WIDGET ( sw ),
			 TRUE, TRUE, 0 );

    gsb_category_fill_sets ( builtin_category_view, builtin_category_model, FALSE );

    g_object_set_data ( G_OBJECT (assistant), "builtin_category_view", builtin_category_view );

    radiobutton = gtk_check_button_new_with_label ( _("Display foreign category sets") );
    gtk_box_pack_start ( GTK_BOX ( page ), radiobutton, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT(radiobutton), "model", builtin_category_model );
    g_object_set_data ( G_OBJECT(radiobutton), "view", builtin_category_view );
    g_signal_connect ( radiobutton, "toggled", G_CALLBACK(gsb_category_display_foreign_toggled), NULL );

    gtk_widget_show_all (page);
    return page;
}



/**
 * get the choice in the assistant page created by gsb_category_assistant_create_choice_page
 * and create the category list
 *
 * \param assistant
 *
 * \return FALSE
 * */
gboolean gsb_category_assistant_create_categories ( GtkWidget *assistant )
{
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeView * view;
    GtkTreeIter iter;
    gchar * filename;

    view = g_object_get_data ( G_OBJECT(assistant), "builtin_category_view" );
    model = gtk_tree_view_get_model ( view );
    selection = gtk_tree_view_get_selection ( view );

    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
	gtk_tree_model_get ( GTK_TREE_MODEL (model),
			     &iter,
			     BUILTIN_CATEGORY_MODEL_FILENAME, &filename,
			     -1);
	
	gsb_file_others_load_category ( filename );

    }
    return FALSE;
}



/**
 * update the form's combofix category
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_category_update_combofix ( gboolean force )
{
    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY ) || force )
    {
        gtk_combofix_set_list ( GTK_COMBOFIX ( gsb_form_widget_get_widget (
                        TRANSACTION_FORM_CATEGORY ) ),
                        gsb_data_category_get_name_list ( TRUE, TRUE, TRUE, TRUE ) );
    }
 
    return FALSE;
}


/**
 * Scan a directory for Grisbi category files in order to put their
 * titles in a GtkTreeModel.
 *
 * \param basename	Basename of the directory to scan.  The 
 *			data directory will be prepended.
 * \param model		A GtkTreeModel to fill with information found.
 *
 * \return A newly allocated list of files found.
 */
GSList * gsb_category_assistant_scan_directory ( gchar * basename, GtkTreeModel * model )
{
    gchar * dirname;
    GDir * dir;
    GSList * list = NULL;

    dirname = g_strconcat ( DATA_PATH, C_DIRECTORY_SEPARATOR, "categories", C_DIRECTORY_SEPARATOR, basename, NULL );
    if ( ! g_file_test ( dirname, G_FILE_TEST_IS_DIR ) )
    {
	gchar * pos = strchr ( basename, '.' );
	if ( pos ) 
	    *pos = '\0';
	dirname = g_strconcat ( DATA_PATH, C_DIRECTORY_SEPARATOR, "categories", C_DIRECTORY_SEPARATOR, basename, NULL );
    }
    if ( ! g_file_test ( dirname, G_FILE_TEST_IS_DIR ) )
    {
	gchar * pos = strchr ( basename, '_' );
	if ( pos ) 
	    *pos = '\0';
	dirname = g_strconcat ( DATA_PATH, C_DIRECTORY_SEPARATOR, "categories", C_DIRECTORY_SEPARATOR, basename, NULL );
    }

    dir = g_dir_open ( dirname, 0, NULL );
    if ( dir )
    {
	gchar * filename;
	while ( ( filename = (gchar *) g_dir_read_name ( dir ) ) )
	{
	    filename = g_strconcat ( dirname, "/", filename, NULL );
	    if ( g_file_test ( filename, G_FILE_TEST_IS_REGULAR ) &&
		 g_str_has_suffix ( filename, ".cgsb" ) )
	    {
		list = g_slist_append ( list, filename );
	    }
	}
    }

    return g_slist_sort ( list, (GCompareFunc) strcmp );
}



/**
 * Parse a category file just to get its name and description.
 *
 * \param filename	Filename to parse
 * \param model		A GtkTreeModel to fill with information found.
 */
void gsb_category_assistant_parse_file ( gchar * filename, GtkTreeModel * model )
{    
    GMarkupParser *markup_parser = g_malloc0 (sizeof (GMarkupParser));
    GMarkupParseContext * context;
    gchar * file_content, * description = NULL;
    GtkTreeIter iter;

    if (!g_file_get_contents ( filename,
			       &file_content,
			       NULL,
			       NULL ))
    {
	dialogue_error (_("Cannot open the category file."));
	return;
    }

    markup_parser -> start_element = (void *) gsb_category_assistant_start_element;

    context = g_markup_parse_context_new ( markup_parser,
					   0,
					   &description,
					   NULL );
    g_markup_parse_context_parse ( context,
				   file_content,
				   strlen (file_content),
				   NULL );

    if ( ! description )
    {
	description = g_strconcat ( "<span size=\"larger\" weight=\"bold\">", _("Unnamed category list"), 
				    "</span>", NULL );
    }

    gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
    gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter,
			 BUILTIN_CATEGORY_MODEL_FILENAME, filename,
			 BUILTIN_CATEGORY_MODEL_NAME, description,
			 -1);

    /* This has been allocated by gsb_category_assistant_start_element. */
    g_free ( description );
}



void gsb_category_assistant_start_element ( GMarkupParseContext *context, const gchar * element_name,
					    const gchar ** attribute_names, const gchar ** attribute_values,
					    gchar ** return_string, GError ** error )
{
    gchar * title = NULL, * description = NULL;

    if ( ! my_strcasecmp ( element_name, "general" ) )
    {
	
	while ( *attribute_names )
	{
	    if ( ! my_strcasecmp ( *attribute_names, "title" ) )
	    {
		title = (gchar *) *attribute_values;
	    }
	    if ( ! my_strcasecmp ( *attribute_names, "description" ) )
	    {
		description = (gchar *) *attribute_values;
	    }
	    attribute_names ++;
	    attribute_values ++;
	}

	if ( title && description )
	{
	    *return_string = g_strconcat ( "<span size=\"larger\" weight=\"bold\">", title, "</span>\n", description, NULL );
	}
	else if ( title )
	{
	    *return_string = g_strconcat ( "<span size=\"larger\" weight=\"bold\">", title, "</span>", NULL );
	}
	else
	{
	    *return_string = NULL;
	}
    }
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
