/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2010 Pierre Biava (grisbi@pierre.biava.name)                      */
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
#include <config.h>


/*START_INCLUDE*/
#include "print_tree_view_list.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "gsb_data_print_config.h"
#include "print_config.h"
#include "print_dialog_config.h"
#include "print_transactions_list.h"
#include "structures.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean print_tree_view_list_begin ( GtkPrintOperation *operation,
                        GtkPrintContext *context,
                        gpointer data );
static void print_tree_view_list_calculate_columns_width ( GtkTreeView *tree_view, gdouble page_width );
static void print_tree_view_list_draw_background ( GtkPrintContext *context,
                        GtkTreeView *tree_view,
                        gint line_position );
static gint print_tree_view_list_draw_cell ( GtkPrintContext *context,
                        gint line_position,
                        gint column_position,
                        gint column,
                        const gchar *text );
static gint print_tree_view_list_draw_column ( gint column_position, gint line_position );
static gint print_tree_view_list_draw_columns_title ( GtkPrintContext *context,
                        gint line_position,
                        GtkTreeView *tree_view );
static gboolean print_tree_view_list_draw_page ( GtkPrintOperation *operation,
                        GtkPrintContext *context,
                        gint page,
                        gpointer data );
static gint print_tree_view_list_draw_row ( GtkPrintContext *context,
                        GtkTreeView *tree_view,
                        gint line_position );
static gint print_tree_view_list_draw_rows_data ( GtkPrintContext *context,
                        gint line_position,
                        GtkTreeView *tree_view,
                        gint page );
static gint print_tree_view_list_draw_title ( GtkPrintContext *context,
                        gint line_position );
static gboolean print_tree_view_list_foreach_callback ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static gint print_tree_view_list_get_columns_data_nbre_lines ( GtkTreeView *tree_view );
static gint print_tree_view_list_get_columns_title_nbre_lines ( GtkTreeView *tree_view );
static gint print_tree_view_list_get_columns_title_size ( gint nbre_lines );
static gint print_tree_view_list_get_title_size ( void );
static gint print_tree_view_list_set_rows_to_print ( GtkTreeView *tree_view );
static void print_tree_view_list_init_tree_view_data ( GtkTreeView *tree_view );
static GtkWidget *print_tree_view_list_layout_config ( GtkPrintOperation *operation, gpointer data );
/*END_STATIC*/

/*START_EXTERN*/
extern GdkColor couleur_bet_division;
extern GdkColor couleur_bet_future;
extern GdkColor couleur_bet_transfert;
/*END_EXTERN*/

#define MAX_COLS 32

/*tree_view_data */
static gint nbre_cols;
static gint *tree_view_cols_width;
static gint *alignment;
static gint total_lines_to_print = 0;


static gint lines_per_page;
static gint lines_in_first_page;

/* nb of lines already printed */
static gint total_lines_printed;

/* current row of page to print */
static gint current_row_to_print;

/* current row of model to print */
static GtkTreePath *tree_path_to_print = NULL;

/* title of page */
static gchar *title_string;

/* size and pos of the columns calculated when begin the print */
static gdouble columns_position[MAX_COLS];
static gint columns_width[MAX_COLS];
static gint size_title = 0;
static gint size_columns_title = 0;
static gint size_row = 0;
static gint nbre_lines_col_data;
static gint nbre_lines_col_title;
static cairo_t *cr = NULL;
static gdouble page_width = 0.0;
static gdouble page_height = 0.0;


/**
 *
 *
 *
 *
 * */
gboolean print_tree_view_list ( GtkWidget *menu_item, GtkTreeView *tree_view )
{
    GtkWidget *label_title;
    GtkTreeSelection *tree_selection;

    devel_debug (NULL);
    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ), NULL, NULL ) )
        return FALSE;

    label_title = g_object_get_data ( G_OBJECT ( tree_view ), "label_title" );
    title_string = g_strdup ( gtk_label_get_text ( GTK_LABEL ( label_title ) ) );

    print_dialog_config ( G_CALLBACK ( print_tree_view_list_begin ),
                        G_CALLBACK ( print_tree_view_list_draw_page ),
                        _("Fonts & layout"),
                        G_CALLBACK ( print_tree_view_list_layout_config ),
                        G_CALLBACK ( print_transactions_list_apply ),
                        ( gpointer ) tree_view );

    return TRUE;
}


/**
 * Function called first when try to print the  list
 * initialize the variables and calculate the number of pages
 *
 * \param operation	GtkPrintOperation
 * \param context	GtkPrintContext
 * \param null
 *
 * \return FALSE
 * */
gboolean print_tree_view_list_begin ( GtkPrintOperation *operation,
                        GtkPrintContext *context,
                        gpointer data )
{
    GtkTreeView *tree_view = ( GtkTreeView * ) data;
    gint size_line;
    gdouble lines_per_page_double;
    gint nbre_pages;

    devel_debug (NULL);

    /* we need to calculate the number of pages */
    cr = gtk_print_context_get_cairo_context ( context );

    /* get the size of the title */
    if ( title_string && strlen ( title_string ) )
        size_title = print_tree_view_list_get_title_size ( );
    else
        size_title = 0;

    /* get the size of a row */
    size_row = pango_font_description_get_size ( gsb_data_print_config_get_font_transactions () ) / PANGO_SCALE;
    size_line = size_row + gsb_data_print_config_get_draw_lines ( );

    /* get the size of the titles of columns */
    if ( gsb_data_print_config_get_draw_columns_name () )
    {
        nbre_lines_col_title = print_tree_view_list_get_columns_title_nbre_lines ( tree_view );
        size_columns_title = print_tree_view_list_get_columns_title_size ( nbre_lines_col_title );
    }
    else
        size_columns_title = 0;

    /* the heigh of a page decrease size_columns_title */
    page_height = gtk_print_context_get_height ( context );

    /* get total lines to print */
    total_lines_to_print = 0;
    total_lines_to_print = print_tree_view_list_set_rows_to_print ( tree_view );

    /* how much transactions we can show in a page : */
    lines_per_page_double = page_height / size_line;
    /* on enlève les lignes des titres des colonnes */
    lines_per_page_double -= nbre_lines_col_title;

    if ( !size_title ) /* no title */
    {
        nbre_pages = ceil ( total_lines_to_print / lines_per_page_double );
        lines_in_first_page = floor ( lines_per_page_double );
    }
    else
    {
        gint first_page_height;

        first_page_height = page_height - size_title;

        lines_in_first_page = floor ( first_page_height / size_line ) - nbre_lines_col_title;
        nbre_pages = 1 + ceil (  ( total_lines_to_print - lines_in_first_page ) /
                                    lines_per_page_double );
    }

    /* set the number of page */
    gtk_print_operation_set_n_pages ( GTK_PRINT_OPERATION ( operation ), nbre_pages );
    /* save the nb of rows per page */
    lines_per_page = floor ( lines_per_page_double );

    /* calculate the size and position of the columns */
    page_width = gtk_print_context_get_width ( context );
    print_tree_view_list_calculate_columns_width ( tree_view, page_width );

    if ( tree_path_to_print )
        gtk_tree_path_free ( tree_path_to_print );
    tree_path_to_print = gtk_tree_path_new_first ( );
    total_lines_printed = 0;
    current_row_to_print = 0;

    return FALSE;
}


/**
 * print the page
 *
 * \param operation GtkPrintOperation
 * \param context   GtkPrintContext
 * \param page      page to print
 * \param null
 *
 * \return FALSE
 * */
 gboolean print_tree_view_list_draw_page ( GtkPrintOperation *operation,
                        GtkPrintContext *context,
                        gint page,
                        gpointer data )
{
    GtkTreeView *tree_view = ( GtkTreeView * ) data;
    gint line_position = 0;

    devel_debug_int (page);

    /* draw the title */
    if ( !page && title_string && strlen ( title_string ) )
        line_position = print_tree_view_list_draw_title ( context, line_position );

    /* draw the columns titles */
    if ( gsb_data_print_config_get_draw_columns_name () )
        line_position = print_tree_view_list_draw_columns_title ( context, line_position, tree_view );

    /* draw the lines */
    line_position = print_tree_view_list_draw_rows_data ( context, line_position, tree_view, page );

    return FALSE;
}


/**
 * draw a line of a model
 *
 * \param context           the GtkPrintContext
 * \param line_position     the position to insert the titles
 * \param
 *
 * \return the new line_position
 * */
static gint print_tree_view_list_draw_row ( GtkPrintContext *context,
                        GtkTreeView *tree_view,
                        gint line_position )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint column = 0;
    GList *list_tmp;

    model = gtk_tree_view_get_model ( tree_view );
    if ( !gtk_tree_model_get_iter ( model, &iter, tree_path_to_print ) )
        return line_position;

    if ( nbre_lines_col_data == 1 )
    {
        list_tmp = gtk_tree_view_get_columns ( tree_view );
        while ( list_tmp )
        {
            GtkTreeViewColumn *col;
            gchar *text;
            gint column_position;
            gint col_num_model;
            GType col_type_model;

            col = list_tmp -> data;
            col_num_model = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( col ), "num_col_model" ) );
            col_type_model = gtk_tree_model_get_column_type ( model, col_num_model );
            column_position = columns_position[column];

            /* get the text */
            if ( col_type_model == G_TYPE_STRING )
                gtk_tree_model_get ( model, &iter, col_num_model, &text, -1 );
            else if ( col_type_model == G_TYPE_INT )
            {
                gint number;

                gtk_tree_model_get ( model, &iter, col_num_model, &number, -1 );
                text = utils_str_itoa ( number );
            }
            else
                text = NULL;

            if ( !text )
            {
                /* draw first the column */
                column_position = print_tree_view_list_draw_column ( column_position, line_position );

                list_tmp  = list_tmp -> next;
                column++;
                continue;
            }

            print_tree_view_list_draw_cell ( context, line_position, column_position, column, text );
            list_tmp  = list_tmp -> next;
            column++;
            g_free ( text );
        }
        /* go to the next row */
        line_position = line_position + size_row;
    }
    else
    {
        gchar **tab;
        gchar *str_tmp;
        gint i = 0;

        for ( i = 0; i < nbre_lines_col_data; i ++ )
        {
            /* draw the last column */
            print_tree_view_list_draw_column ( page_width, line_position );
            list_tmp = gtk_tree_view_get_columns ( tree_view );

            while ( list_tmp )
            {
                GtkTreeViewColumn *col;
                gchar *text;
                gint column_position;
                gint col_num_model;
                GType col_type_model;

                col = ( GtkTreeViewColumn * ) list_tmp -> data;
                col_num_model = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( col ), "num_col_model" ) );
                col_type_model = gtk_tree_model_get_column_type ( model, col_num_model );
                column_position = columns_position[column];

                /* get the text */
                if ( col_type_model == G_TYPE_STRING )
                    gtk_tree_model_get ( model, &iter, col_num_model, &text, -1 );
                else if ( col_type_model == G_TYPE_INT )
                {
                    gint number;

                    gtk_tree_model_get ( model, &iter, col_num_model, &number, -1 );
                    text = utils_str_itoa ( number );
                }
                else
                    text = NULL;

                if ( text == NULL || strlen ( text ) == 0 )
                {
                    print_tree_view_list_draw_column ( column_position, line_position );
                    list_tmp  = list_tmp -> next;
                    column++;
                    continue;
                }

                str_tmp = gsb_string_uniform_new_line ( text, strlen ( text ) );
                if ( str_tmp == NULL )
                {
                    if ( i == 0 )
                        print_tree_view_list_draw_cell ( context, line_position, column_position, column, text );
                    else
                        print_tree_view_list_draw_column ( column_position, line_position );
                    list_tmp  = list_tmp -> next;
                    column++;
                    continue;
                }

                tab = g_strsplit ( str_tmp, "\n", 0 );

                if ( tab[i] && strlen ( tab[i] ) )
                    print_tree_view_list_draw_cell ( context, line_position, column_position, column, tab[i] );
                else
                    print_tree_view_list_draw_column ( column_position, line_position );

                list_tmp  = list_tmp -> next;
                column++;
                g_strfreev ( tab );
                g_free ( str_tmp );
            }
            line_position = line_position + size_row + gsb_data_print_config_get_draw_lines ( );
            column = 0;
        }
    }

    return line_position;
}


/**
 * draw a column line 
 *
 * \param column_position   column position
 * \param line_position     line position
 *
 * \return the new column position
 * */
static gint print_tree_view_list_draw_column ( gint column_position, gint line_position )
{
    if ( !gsb_data_print_config_get_draw_column () )
        return column_position;

    cairo_move_to ( cr, column_position, line_position );
    cairo_set_line_width ( cr, 0.5 );
    cairo_line_to ( cr, column_position, line_position + size_row + 2*gsb_data_print_config_get_draw_lines () );
    cairo_stroke ( cr );

    /* add a space with the next column */
    column_position++;

    return column_position;
}


/**
 * draw the line before the row data
 *
 * \param line_position     position where drawing the line
 *
 * \return the new line position
 * */
static gint print_tree_view_list_draw_line ( gint line_position )
{
    if ( !gsb_data_print_config_get_draw_lines () )
        return line_position;

    /* add a space with the last transaction */
    line_position++;
    cairo_move_to ( cr, 0, line_position );
    cairo_set_line_width ( cr, 0.5 );
    cairo_line_to ( cr, page_width, line_position );
    cairo_stroke ( cr );

    return line_position;
}


/**
 *
 *
 *
 *
 * */
static gint print_tree_view_list_draw_rows_data ( GtkPrintContext *context,
                        gint line_position,
                        GtkTreeView *tree_view,
                        gint page )
{
    gint lines_to_draw;
    devel_debug_int (line_position);

    /* calcul du nombre de lignes à éditer */
    if (page)
        lines_to_draw = lines_per_page;
    else
        lines_to_draw = lines_in_first_page;
    if ( lines_to_draw > ( total_lines_to_print - total_lines_printed ) )
        lines_to_draw = total_lines_to_print - total_lines_printed;

    while ( lines_to_draw )
    {
        nbre_lines_col_data = print_tree_view_list_get_columns_data_nbre_lines ( tree_view );
        print_tree_view_list_draw_background ( context, tree_view, line_position );
        /* begin a row : fill the line before the row */
        line_position = print_tree_view_list_draw_line ( line_position );

        /* draw the last column */
        print_tree_view_list_draw_column ( page_width, line_position );

        /* draw the row */
        line_position = print_tree_view_list_draw_row ( context, tree_view, line_position );

        gtk_tree_path_next ( tree_path_to_print );
        lines_to_draw--;
        total_lines_printed++;
    }

    /* draw the last line */
    print_tree_view_list_draw_line ( line_position );

    return line_position;
}


/**
 * draw a cell of a model
 *
 * \param context           the GtkPrintContext
 * \param line_position     the position to insert the column
 * \param column_position   the position to insert the data
 *
 * \return the new column_position
 * */
static gint print_tree_view_list_draw_cell ( GtkPrintContext *context,
                        gint line_position,
                        gint column_position,
                        gint column,
                        const gchar *text )
{
    PangoLayout *layout;

    /* draw first the column */
    column_position = print_tree_view_list_draw_column ( column_position, line_position );

    cairo_move_to (cr, column_position, line_position);

    /* create the new layout */
    layout = gtk_print_context_create_pango_layout (context);
    pango_layout_set_text ( layout, text, -1 );
    pango_layout_set_font_description ( layout, gsb_data_print_config_get_font_transactions () );
    pango_layout_set_width ( layout,columns_width[column] );
    pango_layout_set_alignment ( layout, alignment[column] );
    pango_layout_set_ellipsize ( layout, PANGO_ELLIPSIZE_END );

    pango_cairo_show_layout ( cr, layout );
    g_object_unref ( layout );

    return column_position;
}


/**
 * draw the title of the columns
 *
 * \param context           the GtkPrintContext
 * \param line_position     the position to insert the titles
 * 
 * \return the new line_position to continue to fill the page
 * */
static gint print_tree_view_list_draw_columns_title ( GtkPrintContext *context,
                        gint line_position,
                        GtkTreeView *tree_view )
{
    GList *list;
    GList *list_tmp;
    gint column = 0;
    gint column_position;

    devel_debug_int ( line_position );

    if ( !gsb_data_print_config_get_draw_columns_name () )
        return line_position;

    /* begin a row : fill the line before the row */
    line_position = print_tree_view_list_draw_line ( line_position );

    list = gtk_tree_view_get_columns ( tree_view );

    if ( nbre_lines_col_title == 1 )
    {
        /* draw the last column */
        print_tree_view_list_draw_column ( page_width, line_position );
        list_tmp = list;

        while ( list_tmp )
        {
            GtkTreeViewColumn *col;
            const gchar *text;

            col = ( GtkTreeViewColumn * ) list_tmp -> data;
            column_position = columns_position[column];

            /* get the text */
            text = gtk_tree_view_column_get_title ( col );
            if (!text)
            {
                list_tmp  = list_tmp -> next;
                column++;
                continue;
            }

            print_tree_view_list_draw_cell ( context, line_position, column_position, column, text );

            list_tmp  = list_tmp -> next;
            column++;
        }
        line_position = line_position + size_columns_title;
    }
    else
    {
        gchar **tab;
        gchar *str_tmp;
        gint i = 0;

        for ( i = 0; i < nbre_lines_col_title; i ++ )
        {
            /* draw the last column */
            print_tree_view_list_draw_column ( page_width, line_position );
            list_tmp = list;

            while ( list_tmp )
            {
                GtkTreeViewColumn *col;
                const gchar *text;

                col = ( GtkTreeViewColumn * ) list_tmp -> data;
                column_position = columns_position[column];

                /* get the text */
                text = gtk_tree_view_column_get_title ( col );

                if ( text == NULL || strlen ( text ) == 0 )
                {
                    print_tree_view_list_draw_column ( column_position, line_position );
                    list_tmp  = list_tmp -> next;
                    column++;
                    continue;
                }

                str_tmp = gsb_string_uniform_new_line ( text, strlen ( text ) );
                if ( str_tmp == NULL )
                {
                    if ( i == 0 )
                        print_tree_view_list_draw_cell ( context, line_position, column_position, column, text );
                    else
                        print_tree_view_list_draw_column ( column_position, line_position );
                    list_tmp  = list_tmp -> next;
                    column++;
                    continue;
                }

                tab = g_strsplit ( str_tmp, "\n", 0 );

                if ( tab[i] && strlen ( tab[i] ) )
                    print_tree_view_list_draw_cell ( context, line_position, column_position, column, tab[i] );
                else
                    print_tree_view_list_draw_column ( column_position, line_position );

                list_tmp  = list_tmp -> next;
                column++;
                g_strfreev ( tab );
                g_free ( str_tmp );
            }
            line_position = line_position + size_row + gsb_data_print_config_get_draw_lines ( );
            column = 0;
        }
    }

    return line_position;
}


/**
 * draw the title if asked
 *
 * \param context           the GtkPrintContext
 * \param line_position     the position to insert the title
 * \param page_width        the page width
 *
 * \return the new line_position to continue to fill the page
 * */
static gint print_tree_view_list_draw_title ( GtkPrintContext *context,
                        gint line_position )
{
    if ( title_string && strlen ( title_string ) )
    {
        PangoLayout *layout;
        devel_debug_int (line_position);

        cairo_move_to ( cr, 0, line_position );

        /* create the new layout */
        layout = gtk_print_context_create_pango_layout ( context );

        pango_layout_set_text ( layout, title_string, -1 );
        pango_layout_set_font_description ( layout, gsb_data_print_config_get_font_title () );
        pango_layout_set_width ( layout, page_width * PANGO_SCALE );
        pango_layout_set_alignment ( layout, PANGO_ALIGN_CENTER );
        pango_layout_set_ellipsize ( layout, PANGO_ELLIPSIZE_END );
        pango_cairo_show_layout ( cr, layout );
        g_object_unref ( layout );

        /* add title line and blank line */
        line_position = line_position + size_title;
    }

    return line_position;
}


/**
 *
 *
 *
 *
 * */
void print_tree_view_list_calculate_columns_width ( GtkTreeView *tree_view, gdouble page_width )
{
    gint total_text_width = 0;
    gint column;

    print_tree_view_list_init_tree_view_data ( tree_view );
    columns_position[0] = 0.0;

    for ( column = 1 ; column < nbre_cols ; column++ )
        columns_position[column] = (gdouble) ( tree_view_cols_width[column - 1]* page_width / 100 ) +
                        columns_position[column - 1];

    /* set the page_width of the columns */
    for (column = 0 ; column < ( nbre_cols  - 1 ); column++ )
    {
        columns_width[column] = ( columns_position[column + 1] - columns_position[column] - 
                                    3 * gsb_data_print_config_get_draw_column () ) * PANGO_SCALE;

        total_text_width = total_text_width + columns_width[column];
    }

    /* last column is the rest of the line */
    columns_width[nbre_cols - 1] = ( page_width -3 * gsb_data_print_config_get_draw_column () * (nbre_cols) ) *
                                    PANGO_SCALE - total_text_width;
}



/**
 *
 *
 *
 *
 * */
void print_tree_view_list_init_tree_view_data ( GtkTreeView *tree_view )
{
    GList *list;
    GList *list_tmp;
    gint col_width = 0;
    gint i = 0;

    devel_debug (NULL);

    /* get the number of columns */
    list = gtk_tree_view_get_columns ( tree_view );
    nbre_cols = g_list_length ( list );

    tree_view_cols_width = g_malloc0 ( nbre_cols * sizeof ( gint ) );
    alignment = g_malloc0 ( nbre_cols * sizeof ( gint ) );

    list_tmp = list;
    while ( list_tmp )
    {
        GtkTreeViewColumn *col;

        col = list_tmp -> data;
        col_width += gtk_tree_view_column_get_width ( col );

        list_tmp  = list_tmp -> next;
    }
    
    list_tmp = list;
    while ( list_tmp )
    {
        GtkTreeViewColumn *col;
        gfloat number;

        col = ( GtkTreeViewColumn * ) list_tmp -> data;
        tree_view_cols_width[i] = ( gtk_tree_view_column_get_width ( col ) * 100 ) / col_width + 1;

        number = gtk_tree_view_column_get_alignment ( col );
        if ( number == 0.0 )
            alignment[i] = PANGO_ALIGN_LEFT;
        else if ( number == 1.0 )
            alignment[i] = PANGO_ALIGN_RIGHT;
        else
            alignment[i] = PANGO_ALIGN_CENTER;

        list_tmp  = list_tmp -> next;
        i++;
    }

    g_list_free ( list );
}


/**
 *
 *
 *
 *
 * */
static gint print_tree_view_list_set_rows_to_print ( GtkTreeView *tree_view )
{
    GtkTreeModel *model;
    gint nbre_lines;

    nbre_lines = 0;
    model = gtk_tree_view_get_model ( tree_view );
    gtk_tree_model_foreach ( model,
                        ( GtkTreeModelForeachFunc ) print_tree_view_list_foreach_callback,
                        &nbre_lines );

    return nbre_lines;
}


/**
 *
 *
 *
 *
 * */
static gboolean print_tree_view_list_foreach_callback ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint *number = (gint *) data;

    *number = *number + 1;

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
static gint print_tree_view_list_get_title_size ( void )
{
    gint size_tmp = 0;
    gint size_line;
    gint nbre_lines = 1;

    if ( g_utf8_strchr ( title_string, -1, '\n' ) )
        nbre_lines = utils_str_get_nbre_motifs ( title_string, "\n" ) + 1;

    /* Add a blank line */
    nbre_lines++;

    size_line = pango_font_description_get_size ( gsb_data_print_config_get_font_title () );
    
    size_tmp =  size_line * nbre_lines / PANGO_SCALE;

    return size_tmp;
}


/**
 *
 *
 *
 *
 * */
static gint print_tree_view_list_get_columns_title_size ( gint nbre_lines )
{
    gint size_tmp = 0;

    devel_debug_int(nbre_lines);
    
    size_tmp =  size_row * nbre_lines + gsb_data_print_config_get_draw_lines ( );

    return size_tmp;
}


/**
 *
 *
 *
 *
 * */
static gint print_tree_view_list_get_columns_title_nbre_lines ( GtkTreeView *tree_view )
{
    GList *list_tmp;
    gint nbre_lines = 0;

    list_tmp = gtk_tree_view_get_columns ( tree_view );
    while ( list_tmp )
    {
        GtkTreeViewColumn *col;
        const gchar *text;
        gint nbre_motifs = 0;

        col = ( GtkTreeViewColumn * ) list_tmp -> data;

        /* get the text */
        text = gtk_tree_view_column_get_title ( col );
        if ( text == NULL || strlen ( text ) == 0 )
        {
            list_tmp  = list_tmp -> next;
            continue;
        }

        if ( g_utf8_strchr ( text, -1, '\n' ) )
        {
            nbre_motifs = utils_str_get_nbre_motifs ( text, "\n" );
            if ( nbre_motifs > nbre_lines )
                nbre_lines = nbre_motifs;
        }

        list_tmp  = list_tmp -> next;
    }
    nbre_lines++;

    return nbre_lines;
}


/**
 * Show a dialog to set wether we want the rows/columns lines,
 * the background color, the titles...
 *
 * \param operation     GtkPrintOperation responsible of this job.
 * \param null          Not used.
 *
 * \return              A newly allocated widget.
 */
GtkWidget *print_tree_view_list_layout_config ( GtkPrintOperation *operation, gpointer data )
{
    GtkWidget *check_button;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *entry;
    GtkWidget *font_button_transactions;
    GtkWidget *font_button_title;
    gchar *fontname_transactions;
    gchar *fontname_title;
    GtkWidget *vbox;
    GtkWidget *paddingbox;
    GtkSizeGroup * size_group;

    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 12 );
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Layout") );

    /* set up the title, this is never saved, so ask each time */
    /* title line */
    hbox = gtk_hbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    entry = gsb_automem_entry_new ( &title_string, NULL, NULL);

    check_button = gsb_autofunc_checkbutton_new ( _("Print a title: "),
                        gsb_data_print_config_get_draw_title ( ),
                        G_CALLBACK ( sens_desensitive_pointeur ), entry,
                        G_CALLBACK ( gsb_data_print_config_set_draw_title ),
                        0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), check_button, FALSE, FALSE, 0);

    gtk_widget_set_sensitive ( entry, gsb_data_print_config_get_draw_title () );
    gtk_box_pack_start ( GTK_BOX ( hbox ), entry, TRUE, TRUE, 0 );

    /* set up all the checkbuttons */
    check_button = gsb_autofunc_checkbutton_new ( _("Draw the lines between transactions"),
                        gsb_data_print_config_get_draw_lines ( ),
                        NULL, NULL,
                        G_CALLBACK ( gsb_data_print_config_set_draw_lines ),
                        0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), check_button, FALSE, FALSE, 0 );

    check_button = gsb_autofunc_checkbutton_new ( _("Draw the lines between the columns"),
                        gsb_data_print_config_get_draw_column ( ),
                        NULL, NULL,
                        G_CALLBACK ( gsb_data_print_config_set_draw_column ),
                        0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), check_button, FALSE, FALSE, 0);

    check_button = gsb_autofunc_checkbutton_new ( _("Print the names of the columns"),
                        gsb_data_print_config_get_draw_columns_name (),
                        NULL, NULL,
                        G_CALLBACK ( gsb_data_print_config_set_draw_columns_name ),
                        0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), check_button, FALSE, FALSE, 0);

    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Fonts") );

    /* set up the font of the transactions,
     * by default use the font of the lists */
    hbox = gtk_hbox_new ( FALSE, 12 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Transactions font") );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5 );
    gtk_size_group_add_widget ( size_group, label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    fontname_transactions = pango_font_description_to_string ( gsb_data_print_config_get_font_transactions () );
    font_button_transactions = gtk_font_button_new_with_font ( fontname_transactions );
    gtk_font_button_set_use_font ( GTK_FONT_BUTTON ( font_button_transactions ), TRUE );
    gtk_font_button_set_use_size ( GTK_FONT_BUTTON ( font_button_transactions ), TRUE );
    gtk_font_button_set_title ( GTK_FONT_BUTTON ( font_button_transactions ), _("Choosing font") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), font_button_transactions, TRUE, TRUE, 0 );

    /* set up the font for the title */
    hbox = gtk_hbox_new (FALSE, 12);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Title font") );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_size_group_add_widget ( size_group, label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    fontname_title = pango_font_description_to_string ( gsb_data_print_config_get_font_title () );
    font_button_title =  gtk_font_button_new_with_font ( fontname_title );
    gtk_font_button_set_use_font ( GTK_FONT_BUTTON ( font_button_title ), TRUE );
    gtk_font_button_set_use_size ( GTK_FONT_BUTTON ( font_button_title ), TRUE );
    gtk_font_button_set_title ( GTK_FONT_BUTTON ( font_button_title ), _("Choosing font") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), font_button_title, TRUE, TRUE, 0);


    /* save what we have done in all cases, so if we cancel and come back, our values
     * come back */
    gsb_data_print_config_set_font_transaction (pango_font_description_from_string (fontname_transactions));
    gsb_data_print_config_set_font_title (pango_font_description_from_string (fontname_title));

    g_object_set_data ( G_OBJECT(operation), "font_transaction_button", font_button_transactions );
    g_object_set_data ( G_OBJECT(operation), "font_title_button", font_button_title );

    gtk_widget_show_all ( vbox );

    return vbox;
}


/**
 * draw the background of the row
 *
 * \param cr                cairo context
 * \param tree_view         get_model
 * \param line_position     position where drawing the line
 *
 * \return
 * */
static void print_tree_view_list_draw_background ( GtkPrintContext *context,
                        GtkTreeView *tree_view,
                        gint line_position )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint col_origin = 0;
    gint origin;

    if ( !gsb_data_print_config_get_draw_background () )
        return;

    model = gtk_tree_view_get_model ( tree_view );
    if ( !gtk_tree_model_get_iter ( model, &iter, tree_path_to_print ) )
        return;
    
    col_origin = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( tree_view ), "origin_data_model" ) );
    if ( col_origin == 0 )
        return;

    gtk_tree_model_get ( model, &iter, col_origin, &origin, -1 );

    if ( origin == SPP_ORIGIN_ACCOUNT || origin == SPP_ORIGIN_FUTURE || origin == SPP_ORIGIN_HISTORICAL )
    {
        gchar *str_color;
        gint col_color;
        GdkColor color;

        col_color = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( tree_view ), "color_data_model" ) );
        gtk_tree_model_get ( model, &iter, col_color, &str_color, -1 );
        gdk_color_parse ( str_color, &color );

        cairo_rectangle ( cr, 0, line_position, page_width, size_row + 2 * gsb_data_print_config_get_draw_lines () );
        cairo_set_source_rgb ( cr,
                        (gdouble) color.red/65535,
                        (gdouble) color.green/65535,
                        (gdouble) color.blue/65535 );

        cairo_fill (cr);
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    }
}


/**
 *
 *
 *
 *
 * */
static gint print_tree_view_list_get_columns_data_nbre_lines ( GtkTreeView *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GList *list_tmp;
    gint nbre_lines = 0;

    model = gtk_tree_view_get_model ( tree_view );
    if ( !gtk_tree_model_get_iter ( model, &iter, tree_path_to_print ) )
        return 0;

    list_tmp = gtk_tree_view_get_columns ( tree_view );
    while ( list_tmp )
    {
        GtkTreeViewColumn *col;
        const gchar *text;
        gint nbre_motifs = 0;
        gint col_num_model;
        GType col_type_model;

        col = ( GtkTreeViewColumn * ) list_tmp -> data;

        col_num_model = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( col ), "num_col_model" ) );
        col_type_model = gtk_tree_model_get_column_type ( model, col_num_model );

        /* get the text */
        if ( col_type_model == G_TYPE_STRING )
            gtk_tree_model_get ( model, &iter, col_num_model, &text, -1 );

        if ( text == NULL || strlen ( text ) == 0 )
        {
            list_tmp  = list_tmp -> next;
            continue;
        }

        if ( g_utf8_strchr ( text, -1, '\n' ) )
        {
            nbre_motifs = utils_str_get_nbre_motifs ( text, "\n" );
            if ( nbre_motifs > nbre_lines )
                nbre_lines = nbre_motifs;
        }

        list_tmp  = list_tmp -> next;
    }
    nbre_lines++;

    return nbre_lines;
}


/**
 *
 *
 *
 *
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

