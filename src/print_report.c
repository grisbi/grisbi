/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*         2008-2021 Pierre Biava (grisbi@pierre.biava.name)                  */
/*         https://www.grisbi.org/                                            */
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
#include "config.h"
#endif

#include "include.h"
#include <math.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "print_report.h"
#include "dialog.h"
#include "etats_gtktable.h"
#include "grisbi_app.h"
#include "gsb_data_print_config.h"
#include "gsb_file.h"
#include "print_dialog_config.h"
#include "structures.h"
#include "utils.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static GtkWidget *	table_etat = NULL;
/*END_STATIC*/

/*START_EXTERN*/
extern gint 		nb_colonnes;
extern gint 		nb_lignes;
/*END_EXTERN*/


/* size and pos of the columns calculated when begin the print */
static gint 		size_row = 0;
static gint 		size_title = 0;
static cairo_t *	cr = NULL;
static gdouble 		page_width = 0.0;
static gdouble 		page_height = 0.0;
static gint 		nb_rows_per_page = 0;
static gint 		nb_rows_first_page = 0;
static gdouble *	columns_width = NULL;

static gint 		current_child_line = 0;

/* GtkPrintSettings pour export pdf */
static GtkPrintSettings *print_settings = NULL;

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void  print_report_init_columns_width (gint table_size,
                                               gint width_page)
{
	gint row;
	gint i;
	gint x_dim;

	for (row = 0; row < nb_lignes; row++)
	{
		for (i = 0; i < nb_colonnes; i++)
		{
			GtkAllocation allocation;
			gint new_size;
			GtkWidget *child;

			child = gtk_grid_get_child_at (GTK_GRID (table_etat), i, row);

			if (child)
			{
				if (GTK_IS_LABEL (child) || GTK_IS_EVENT_BOX (child))
				{
					x_dim = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (child), "x_dim"));

					if (x_dim == 1)
					{
						gtk_widget_get_allocation (child, &allocation);
						new_size = (width_page * allocation.width) / table_size;
						if (new_size > columns_width[i])
						{
							columns_width[i] = new_size;
							//~ printf ("row = %d col = %d columns_width[%d] = %g x_dim = %d\n", row, i, i, columns_width[i], x_dim);
						}
					}
				}
				else
				{
					if (GTK_IS_SEPARATOR (child))
					{
						gint type_separator;

						if (row > 15)
						{
							continue;
						}
						type_separator = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (child), "type_separator"));
						if (type_separator == GTK_ORIENTATION_VERTICAL)
						{
							gtk_widget_get_allocation (child, &allocation);
							columns_width[i] = 4;
							//~ printf ("row = %d col = %d columns_width[%d] = %g SEPARATEUR VERTICAL\n", row, i, i, columns_width[i]);
						}
					}
				}
			}
		}
	}
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
static gboolean print_report_begin (GtkPrintOperation *operation,
                                    GtkPrintContext *context,
									gpointer null)
{
    gint nb_pages;
    gint table_size = 0;
	gint minimum_width;
	gint natural_width;

    /* initialize globals variables */
    cr = gtk_print_context_get_cairo_context (context);
    size_row = pango_font_description_get_size (gsb_data_print_config_get_report_font_transactions ())/PANGO_SCALE;
	size_row +=2;	/* added to separate the labels */
    size_title = pango_font_description_get_size (gsb_data_print_config_get_report_font_title ())/PANGO_SCALE;

    /* get the size of a complete transaction and an archive */
    /* the heigh of a page decrease of 1 line if we use the columns titles */
    page_height = gtk_print_context_get_height (context);
    page_width = gtk_print_context_get_width (context);

	/* get first the size of the table */
	gtk_widget_get_preferred_width (table_etat, &minimum_width, &natural_width);
    table_size = natural_width;

	/* get the width of each columns */
    if (columns_width)
        g_free (columns_width);

    columns_width = g_malloc0 (nb_colonnes * sizeof (gdouble));
	print_report_init_columns_width (table_size, page_width);

	/* calculate the nb of rows in 1 page and in the first page */
    nb_rows_per_page = page_height / size_row;
    nb_rows_first_page = (page_height - size_title) / size_row;

    /* calculate the number of pages,
     * it's not too difficult because each line has the same size
     * except the title */
    nb_pages = ceil (((nb_lignes - 1) * size_row + size_title) / page_height);

    gtk_print_operation_set_n_pages (GTK_PRINT_OPERATION (operation), nb_pages);

    current_child_line = 0;

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
static void print_report_draw_line (GtkWidget *child,
                                    gint line_position)
{
    /* add +1 is to avoid to have the line sticked with the text) */
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
static void print_report_draw_column (GtkWidget *child,
									  gint line_position,
									  gint col)
{
    gint column_position = 0;
    gint i;

    /* calculate the column position */
    for (i=0 ; i < col ; i++)
		column_position = column_position + columns_width[i];

    /* draw the line */
    cairo_move_to (cr, column_position, line_position);
    cairo_set_line_width (cr, 0.5);
    cairo_line_to (cr, column_position, line_position + size_row);
    cairo_stroke (cr);
}

/**
 * draw a line of a transaction
 *
 * \param
 *
 * \return the new line_position
 * */
static void print_report_draw_row (GtkPrintContext *context,
                                   GtkWidget *child,
                                   gint line_position,
								   gint col,
								   gint x_dim,
                                   gint is_title)
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
	if (x_dim == nb_colonnes)
	{
		width = page_width;
	}
	else
	{
		for (i = col; i < col+x_dim; i++)
		{
			//~ printf ("row = %d col = %d columns_width[%d] = %g\n", current_child_line, col, i, columns_width[i]);
			width = width + columns_width[i];
		}
	}
    width = width * PANGO_SCALE;

    /* calculate the column position */
    for (i=0 ; i < col ; i++)
		column_position = column_position + columns_width[i];

    /* get the text */
    if (GTK_IS_LABEL (child))
		label = child;
    if (GTK_IS_EVENT_BOX (child))
		label = gtk_bin_get_child (GTK_BIN (child));
    text = gtk_label_get_text (GTK_LABEL (label));
	//~ printf ("row = %d col = %d column_position = %d x_dim = %d width = %d text = %s\n", current_child_line, col, column_position, x_dim, width, text);

	/* get the alignment */
    alignment = gtk_label_get_xalign (GTK_LABEL (label));
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
		pango_layout_set_width (layout, page_width * PANGO_SCALE);
		pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    }
    else
    {
		/* it's a normal line */
		pango_layout_set_font_description (layout, gsb_data_print_config_get_report_font_transactions ());
		pango_layout_set_width (layout, width);
		pango_layout_set_alignment (layout, pango_alignment);
    }

    pango_cairo_show_layout (cr, layout);
    g_object_unref (layout);
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
static gboolean print_report_draw_page (GtkPrintOperation *operation,
                                        GtkPrintContext *context,
                                        gint page,
                                        gpointer null)
{
	gint row;
	gint i;
	gint rows_drawed = 0;
    gboolean is_title = FALSE;

    if (!page)
		is_title = TRUE;

	for (row = current_child_line; row < nb_lignes; row++)
	{
		gint line_position;

		line_position = (rows_drawed * size_row) + (!page * !is_title * size_title);
		for (i = 0; i < nb_colonnes; i++)
		{
			GtkWidget *child;
			gint x_dim;

			//~ printf ("row = %d col = %d\n", row, i);
			child = gtk_grid_get_child_at (GTK_GRID (table_etat), i, row);
			if (child)
			{
				x_dim = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (child), "x_dim"));
				if (GTK_IS_LABEL (child) || GTK_IS_EVENT_BOX (child))
				{
					if (x_dim == nb_colonnes)
					{
						/* we are on a label, draw the text */
						print_report_draw_row (context, child, line_position, i, x_dim, is_title);
						if (is_title)
							is_title = FALSE;
						break;
					}
					else
					{
						print_report_draw_row (context, child, line_position, i, x_dim, is_title);
						i += x_dim-1;
					}
				}
				else
				{
					/* we are on a separator, draw it */
					if (GTK_IS_SEPARATOR (child))
					{
						gint type_separator;

						type_separator = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (child), "type_separator"));
						if (type_separator == GTK_ORIENTATION_VERTICAL)
						{
							print_report_draw_column (child, line_position, i);
						}
						else
						{
							print_report_draw_line (child, line_position);
							break;
						}
					}
				}
			}
		}
		current_child_line++;
		rows_drawed++;
		if ((rows_drawed == nb_rows_per_page && page)
		||
		(rows_drawed == nb_rows_first_page && !page))
		{
			break;
		}
	}

	return FALSE;
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
static GtkWidget *print_config_show_config (GtkPrintOperation *operation,
                                            gpointer null)
{
    GtkWidget *hbox, *label, *paddingbox;
    GtkWidget *font_button_transactions, *vbox, *font_button_title;
    GtkSizeGroup *size_group;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);
    paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Fonts"));

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    /* set up the font of the transactions,
     * by default use the font of the lists */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new (_("Lines font"));
    gtk_size_group_add_widget (size_group, label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    font_button_transactions = gtk_font_button_new_with_font (pango_font_description_to_string (gsb_data_print_config_get_report_font_transactions ()));
    gtk_font_button_set_use_font (GTK_FONT_BUTTON(font_button_transactions), TRUE);
    gtk_font_button_set_use_size (GTK_FONT_BUTTON(font_button_transactions), TRUE);
    gtk_font_button_set_title (GTK_FONT_BUTTON(font_button_transactions), _("Choosing font"));
    gtk_box_pack_start (GTK_BOX (hbox), font_button_transactions, TRUE, TRUE, 0);

    /* set up the font for the title */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new (_("Title's font"));
    gtk_size_group_add_widget (size_group, label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    font_button_title = gtk_font_button_new_with_font (pango_font_description_to_string (gsb_data_print_config_get_report_font_title ()));
    gtk_font_button_set_use_font (GTK_FONT_BUTTON(font_button_title), TRUE);
    gtk_font_button_set_use_size (GTK_FONT_BUTTON(font_button_title), TRUE);
    gtk_font_button_set_title (GTK_FONT_BUTTON(font_button_title), _("Choosing font"));
    gtk_box_pack_start (GTK_BOX (hbox), font_button_title, TRUE, TRUE, 0);

    g_object_set_data (G_OBJECT(operation), "font_transaction_button", font_button_transactions);
    g_object_set_data (G_OBJECT(operation), "font_title_button", font_button_title);

    gtk_widget_show_all (vbox);

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
static gboolean print_config_show_config_apply (GtkPrintOperation *operation,
                                                GtkWidget *widget,
                                                gpointer null)
{
    GtkFontButton *font_button_transactions, *font_button_title;
	const gchar *tmp_str1;
	gchar *tmp_str2;

    font_button_transactions = g_object_get_data (G_OBJECT(operation), "font_transaction_button");
    font_button_title = g_object_get_data (G_OBJECT(operation), "font_title_button");

	tmp_str1 = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (font_button_transactions));
	tmp_str2 = pango_font_description_to_string (gsb_data_print_config_get_report_font_transactions ());
	if (strcmp (tmp_str1, tmp_str2))
	{
		gsb_data_print_config_set_report_font_transaction (pango_font_description_from_string (tmp_str1));
		gsb_file_set_modified (TRUE);
	}
	g_free (tmp_str2);

	tmp_str1 = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (font_button_title));
	tmp_str2 = pango_font_description_to_string (gsb_data_print_config_get_report_font_title ());
	if (strcmp (tmp_str1, tmp_str2))
	{
		gsb_data_print_config_set_report_font_title (pango_font_description_from_string (tmp_str1));
		gsb_file_set_modified (TRUE);
	}
	g_free (tmp_str2);

    return FALSE;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * print the current report
 *
 * \param button	toobar button
 * \param null
 *
 * \return FALSE
 * */
gboolean print_report (GtkWidget *button,
                       gpointer null)
{
	table_etat = etats_gtktable_get_table_etat ();

    if (!table_etat)
    {
		dialogue_error_hint (_("Please select a report before trying to print it."),
		                     _("No report selected"));
		return FALSE;
    }

    print_dialog_config (G_CALLBACK (print_report_begin),
                         G_CALLBACK (print_report_draw_page),
                         _("Fonts"),
                         G_CALLBACK (print_config_show_config),
                         G_CALLBACK (print_config_show_config_apply),
                         NULL);
    return FALSE;
}

void print_report_export_pdf (const gchar *pdf_name)
{
	GtkPrintOperation *print;
	GtkPrintOperationResult res;
	GValue value = G_VALUE_INIT;

    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, pdf_name);

	table_etat = etats_gtktable_get_table_etat ();
	print = gtk_print_operation_new ();

	if (print_settings != NULL)
		gtk_print_operation_set_print_settings (print, print_settings);

	g_object_set_property (G_OBJECT (print),
						   "export-filename",
						   &value);
	g_signal_connect (print,
					  "begin_print",
					  G_CALLBACK (print_report_begin),
					  NULL);
	g_signal_connect (print,
					  "draw_page",
					  G_CALLBACK (print_report_draw_page),
					  NULL);

	res = gtk_print_operation_run (print,
								   GTK_PRINT_OPERATION_ACTION_EXPORT,
								   GTK_WINDOW (grisbi_app_get_active_window (NULL)),
								   NULL);

	if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
	{
	  if (print_settings != NULL)
		g_object_unref (print_settings);
	  print_settings = g_object_ref (gtk_print_operation_get_print_settings (print));
	}

	g_object_unref (print);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
