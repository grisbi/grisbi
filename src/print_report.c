/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
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
 * \file print_report.c
 * print a report
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "print_report.h"
#include "dialog.h"
#include "gsb_data_print_config.h"
#include "utils.h"
#include "print_dialog_config.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static GtkWidget * print_config_show_config ( GtkPrintOperation * operation,
				       gpointer null );
static gboolean print_config_show_config_apply ( GtkPrintOperation * operation,
					  GtkWidget * widget,
					  gpointer null );
static gboolean print_report_begin ( GtkPrintOperation *operation,
			      GtkPrintContext *context,
			      gpointer null );
static void print_report_draw_column ( GtkTableChild *child,
				       gint line_position  );
static void print_report_draw_line ( GtkTableChild *child,
				     gint line_position  );
static gboolean print_report_draw_page ( GtkPrintOperation *operation,
				  GtkPrintContext *context,
				  gint page,
				  gpointer null );
static void print_report_draw_row ( GtkPrintContext *context,
				    GtkTableChild *child,
				    gint line_position,
				    gint is_title );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *table_etat;
/*END_EXTERN*/


/* size and pos of the columns calculated when begin the print */
static gint size_row = 0;
static gint size_title = 0;
static cairo_t *cr = NULL;
static gdouble page_width = 0.0;
static gdouble page_height = 0.0;
static gint nb_rows_per_page = 0;
static gint nb_rows_first_page = 0;
static gdouble *columns_width = NULL;

static GList *current_child_table = NULL;
static gint current_child_line = 0;



/**
 * print the current report
 *
 * \param button	toobar button
 * \param null
 *
 * \return FALSE
 * */
gboolean print_report ( GtkWidget *button,
			gpointer null )
{
    if (!table_etat)
    {
	dialogue_error_hint (_("Please select a report before trying to print it."), _("No report selected"));
	return FALSE;
    }

    print_dialog_config ( G_CALLBACK (print_report_begin),
			  G_CALLBACK (print_report_draw_page),
			  _("Fonts"),
			  G_CALLBACK (print_config_show_config),
			  G_CALLBACK (print_config_show_config_apply),
              NULL );
    return FALSE;
}


/**
 * function called first when try to print the transaction list
 * initialize the variables and calculate the number of pages
 *
 * \param operation	GtkPrintOperation
 * \param context	GtkPrintContext
 * \param null
 *
 * \return FALSE
 * */
gboolean print_report_begin ( GtkPrintOperation *operation,
			      GtkPrintContext *context,
			      gpointer null )
{
    gint nb_pages;
    gint i;
    gint table_size = 0;

    devel_debug (NULL);

    /* initialize globals variables */
    cr = gtk_print_context_get_cairo_context (context);
    size_row = pango_font_description_get_size (gsb_data_print_config_get_report_font_transactions ())/PANGO_SCALE;
    size_title = pango_font_description_get_size (gsb_data_print_config_get_report_font_title ())/PANGO_SCALE;

    /* get the size of a complete transaction and an archive */
    /* the heigh of a page decrease of 1 line if we use the columns titles */
    page_height = gtk_print_context_get_height (context);
    page_width = gtk_print_context_get_width (context);

    /* get the width of each columns */
    if (columns_width)
	g_free (columns_width);
    columns_width = g_malloc0 (GTK_TABLE (table_etat) -> ncols * sizeof (gdouble));

    /* get first the size of the table */
    for (i=0 ; i<GTK_TABLE (table_etat) -> ncols ; i++)
	table_size = table_size + (GTK_TABLE (table_etat) -> cols)[i].allocation;
    for (i=0 ; i<GTK_TABLE (table_etat) -> ncols ; i++)
	columns_width[i] = (page_width * (GTK_TABLE (table_etat) -> cols)[i].allocation) / table_size;

    /* calculate the nb of rows in 1 page and in the first page */
    nb_rows_per_page = page_height / size_row;
    nb_rows_first_page = (page_height - size_title) / size_row;

    /* calculate the number of pages,
     * it's not too difficult because each line has the same size
     * except the title */
    nb_pages = ceil (((GTK_TABLE (table_etat) -> nrows - 1)*size_row + size_title) / page_height);

    gtk_print_operation_set_n_pages ( GTK_PRINT_OPERATION (operation),
				      nb_pages );

    current_child_table = NULL;
    current_child_line = 0;

    return FALSE;
}



/**
 * print the page
 * use the GtkTable already showen instead of calculating again
 * because it's sometimes very slow
 *
 * \param operation	GtkPrintOperation
 * \param context	GtkPrintContext
 * \param page		page to print
 * \param null
 *
 * \return FALSE
 * */
gboolean print_report_draw_page ( GtkPrintOperation *operation,
				  GtkPrintContext *context,
				  gint page,
				  gpointer null )
{
    GList *children;
    gint nb_columns;
    gint nb_lines;
    gint rows_drawed = 0;
    gint previous_line = -1;
    gboolean is_title = FALSE;

    devel_debug_int (page);

    nb_columns = GTK_TABLE (table_etat) -> ncols;
    nb_lines = GTK_TABLE (table_etat) -> nrows;

    /* children begins with the last GtkTableChild added, so we need to inverse */
    if (page)
	children = current_child_table;
    else
    {
	children = g_list_last (GTK_TABLE (table_etat) -> children);
	is_title = TRUE;
    }

    while (children)
    {
	GtkTableChild *child;
	gint line_position;

	child = children -> data;

	/* if at the to of the page, set previous_line to the first line
	 * else we check if the page is not finished */
	if (previous_line == -1)
	    previous_line = child -> top_attach;
	else
	    if (child -> top_attach != previous_line)
	    {
		if ((rows_drawed == nb_rows_per_page && page)
		    ||
		    (rows_drawed == nb_rows_first_page && !page))
		{
		    current_child_table = children;
		    current_child_line = child -> top_attach;
		    break;
		}
		rows_drawed++;
		previous_line = child -> top_attach;
	    }

	/* calculate the line position, if first page, add the size title except for the title itself */
	line_position = (child -> top_attach - current_child_line) * size_row + (!page * !is_title * size_title);

	if (GTK_IS_LABEL (child -> widget) || GTK_IS_EVENT_BOX (child -> widget))
	    /* we are on a label, draw the text */
	    print_report_draw_row (context, child, line_position, is_title);
	else
	{
	    /* we are on a separator, draw it */
	    if (GTK_IS_VSEPARATOR (child->widget))
		print_report_draw_column (child, line_position);
	    else
		print_report_draw_line (child, line_position);
	}
	if (is_title)
	    is_title = FALSE;

	children = children -> prev;
    }
    return FALSE;
}



/**
 * draw the line before the transaction
 *
 * \param cr		cairo context
 * \param page_width	width of the page given by the context
 * \param line_position	position where drawing the line
 *
 * \return the new line position
 * */
static void print_report_draw_line ( GtkTableChild *child,
				     gint line_position  )
{
    /* add +1 is to avoid to have the line sticked with the text )*/
    line_position = line_position + 1;

    cairo_move_to (cr, 0, line_position);
    cairo_set_line_width (cr, 0.5);
    cairo_line_to (cr, page_width, line_position);
    cairo_stroke (cr);
}

/**
 * draw a column line 
 *
 * \param column_position	column position
 * \param line_position		line position
 *
 * \return the new column position
 * */
static void print_report_draw_column ( GtkTableChild *child,
				       gint line_position  )
{
    gint column_position = 0;
    gint i;

    /* calculate the column position */
    for (i=0 ; i<child -> left_attach ; i++)
	column_position = column_position + columns_width[i];

    /* draw the line */
    cairo_move_to (cr, column_position, line_position);
    cairo_set_line_width (cr, 0.5);
    cairo_line_to (cr, column_position, line_position + size_row );
    cairo_stroke (cr);
}

/**
 * draw a line of a transaction
 *
 * \param
 *
 * \return the new line_position
 * */
static void print_report_draw_row ( GtkPrintContext *context,
				    GtkTableChild *child,
				    gint line_position,
				    gint is_title )
{
    PangoLayout *layout;
    gint column_position = 0;
    gint width = 0;
    gint i;
    const gchar *text;
    PangoAlignment pango_alignment;
    GtkWidget *label = NULL;
    gfloat alignment;

    /* calculate the width of the column in pango mode */
    for (i=child -> left_attach ; i<=child -> right_attach ; i++)
	width = width + columns_width[i];
    width = width * PANGO_SCALE;

    /* calculate the column position */
    for (i=0 ; i<child -> left_attach ; i++)
	column_position = column_position + columns_width[i];

    /* get the text */
    if (GTK_IS_LABEL (child -> widget))
	label = child->widget;
    if (GTK_IS_EVENT_BOX (child -> widget))
	label = gtk_bin_get_child (GTK_BIN (child->widget));
    text = gtk_label_get_text (GTK_LABEL (label));

    /* get the alignment */
    gtk_misc_get_alignment (GTK_MISC (label),
			    &alignment,
			    NULL );
    if (!alignment)
	pango_alignment = PANGO_ALIGN_LEFT;
    else
    {
	if (alignment == 1)
	    pango_alignment = PANGO_ALIGN_RIGHT;
	else
	    pango_alignment = PANGO_ALIGN_CENTER;
    }

    /* now can create the layout */
    cairo_move_to (cr, column_position, line_position);

    /* create the new layout */
    layout = gtk_print_context_create_pango_layout (context);
    pango_layout_set_text (layout, text, -1);
    pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);

    if (is_title)
    {
	/* it's the title, special values */
	pango_layout_set_font_description (layout, gsb_data_print_config_get_report_font_title ());
	pango_layout_set_width (layout,page_width * PANGO_SCALE);
	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    }
    else
    {
	/* it's a normal line */
	pango_layout_set_font_description (layout, gsb_data_print_config_get_report_font_transactions ());
	pango_layout_set_width (layout,width);
	pango_layout_set_alignment (layout, pango_alignment);
    }

    pango_cairo_show_layout (cr, layout);
    g_object_unref (layout);
}



/**
 * Show a dialog to set wether we want the rows/columns lines,
 * the background color, the titles...
 *
 * \param operation	GtkPrintOperation responsible of this job.
 * \param null		Not used.
 *
 * \return		A newly allocated widget.
 */
GtkWidget * print_config_show_config ( GtkPrintOperation * operation,
				       gpointer null )
{
    GtkWidget * hbox, * label, * paddingbox;
    GtkWidget * font_button_transactions, * vbox, * font_button_title;
    GtkSizeGroup * size_group;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Fonts") );

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    /* set up the font of the transactions,
     * by default use the font of the lists */
    hbox = gtk_hbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX ( paddingbox ),
			hbox,
			FALSE, FALSE, 0);

    label = gtk_label_new (_("Lines font"));
    gtk_size_group_add_widget ( size_group, label );
    gtk_box_pack_start (GTK_BOX (hbox),
			label,
			FALSE, FALSE, 0);

    font_button_transactions = gtk_font_button_new_with_font ( pango_font_description_to_string ( gsb_data_print_config_get_report_font_transactions ( ) ) );
    gtk_font_button_set_use_font ( GTK_FONT_BUTTON(font_button_transactions), TRUE );
    gtk_font_button_set_use_size ( GTK_FONT_BUTTON(font_button_transactions), TRUE );
    gtk_font_button_set_title ( GTK_FONT_BUTTON(font_button_transactions), _("Choosing font") );
    gtk_box_pack_start (GTK_BOX (hbox),
			font_button_transactions,
			TRUE, TRUE, 0);

    /* set up the font for the title */
    hbox = gtk_hbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX ( paddingbox ),
			hbox,
			FALSE, FALSE, 0);

    label = gtk_label_new (_("Title's font"));
    gtk_size_group_add_widget ( size_group, label );
    gtk_box_pack_start (GTK_BOX (hbox),
			label,
			FALSE, FALSE, 0);

    font_button_title = gtk_font_button_new_with_font ( pango_font_description_to_string ( gsb_data_print_config_get_report_font_title ( ) ) );
    gtk_font_button_set_use_font ( GTK_FONT_BUTTON(font_button_title), TRUE );
    gtk_font_button_set_use_size ( GTK_FONT_BUTTON(font_button_title), TRUE );
    gtk_font_button_set_title ( GTK_FONT_BUTTON(font_button_title), _("Choosing font") );
    gtk_box_pack_start (GTK_BOX (hbox),
			font_button_title,
			TRUE, TRUE, 0);

    g_object_set_data ( G_OBJECT(operation), "font_transaction_button", font_button_transactions );
    g_object_set_data ( G_OBJECT(operation), "font_title_button", font_button_title );

    gtk_widget_show_all ( vbox );

    return vbox;
}



/**
 * Callback that is called when we hit the "Apply" button of a
 * transaction print config dialog.  It is responsible of setting the
 * fonts.
 *
 * \param operation	GtkPrintOperation responsible of this job.
 * \param widget	Custom widget.  Not used.
 * \param null		Not used.
 *
 * \return NULL
 */
gboolean print_config_show_config_apply ( GtkPrintOperation * operation,
					  GtkWidget * widget,
					  gpointer null )
{
    GtkFontButton * font_button_transactions, * font_button_title;

    font_button_transactions = g_object_get_data ( G_OBJECT(operation), "font_transaction_button" );
    font_button_title = g_object_get_data ( G_OBJECT(operation), "font_title_button" );

    gsb_data_print_config_set_report_font_transaction ( pango_font_description_from_string ( gtk_font_button_get_font_name ( font_button_transactions ) ) );
    gsb_data_print_config_set_report_font_title ( pango_font_description_from_string ( gtk_font_button_get_font_name ( font_button_title ) ) );
    
    return FALSE;
}




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
