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
 * \file print_transactions_list.c
 * print the transactions list
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "print_transactions_list.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_print_config.h"
#include "gsb_data_transaction.h"
#include "utils.h"
#include "print_dialog_config.h"
#include "transaction_model.h"
#include "custom_list.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean print_transactions_list_begin ( GtkPrintOperation *operation,
					 GtkPrintContext *context,
					 gpointer null );
static void print_transactions_list_calculate_columns ( gdouble page_width );
static void print_transactions_list_draw_background ( CustomRecord *record,
						      gboolean color_bg,
						      gint line_position );
static gint print_transactions_list_draw_column ( gint column_position,
						 gint line_position );
static gint print_transactions_list_draw_columns_title ( GtkPrintContext *context,
							 gint line_position);
static gint print_transactions_list_draw_line ( gint line_position );
static gboolean print_transactions_list_draw_page ( GtkPrintOperation *operation,
					     GtkPrintContext *context,
					     gint page,
					     gpointer null );
static gint print_transactions_list_draw_row ( GtkPrintContext *context,
					      CustomRecord *record,
					      gint line_position );
static gint print_transactions_list_draw_title ( GtkPrintContext *context,
						 gint line_position );
static gboolean print_transactions_list_get_visibles_lines ( gint *number_of_archives,
						      gint *number_of_transactions );
static GtkWidget * print_transactions_list_layout_config ( GtkPrintOperation * operation,
						    gpointer null );
/*END_STATIC*/


/*START_EXTERN*/
extern GdkColor archive_background_color;
extern GdkColor couleur_fond[2];
extern gchar *titres_colonnes_liste_operations[CUSTOM_MODEL_N_VISIBLES_COLUMN];
extern gint transaction_col_width[CUSTOM_MODEL_N_VISIBLES_COLUMN];
/*END_EXTERN*/


/** save the nb of transactions on 1 page, calculated by print_transactions_list_begin */
static gint transactions_per_page;
static gint transactions_in_first_page;

/** nb of transactions to print (without archives, only transactions */
static gint total_transactions_to_print;

/** nb of transactions already printed */
static gint total_transactions_printed;

/** current row to print (this is row, not transaction) */
static gint current_row_to_print;

static gchar *title_string;
static GDate *draw_initial_date = NULL;
static GDate *draw_final_date = NULL;


/* size and pos of the columns calculated when begin the print */
static gdouble columns_position[CUSTOM_MODEL_VISIBLE_COLUMNS];
static gint columns_width[CUSTOM_MODEL_VISIBLE_COLUMNS];
static gint size_row = 0;
static cairo_t *cr = NULL;
static gdouble page_width = 0.0;
static gdouble page_height = 0.0;



/**
 * Show a dialog to set wether we want the rows/columns lines,
 * the background color, the titles...
 *
 * \param operation	GtkPrintOperation responsible of this job.
 * \param null		Not used.
 *
 * \return		A newly allocated widget.
 */
GtkWidget * print_transactions_list_layout_config ( GtkPrintOperation * operation,
						    gpointer null )
{
    GtkWidget *check_button;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *entry;
    GtkWidget *font_button_transactions;
    GtkWidget *font_button_title;
    gchar *fontname_transactions;
    gchar *fontname_title;
    GtkWidget *init_date_entry;
    GtkWidget *final_date_entry;
    GtkWidget *vbox;
    GtkWidget *paddingbox;
    GtkSizeGroup * size_group;

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Layout") );

    /* set up the title and dates, this is never saved, so ask each time */
    /* title line */
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			hbox,
			FALSE, FALSE, 0);

    entry = gsb_automem_entry_new (&title_string, NULL, NULL);

    check_button = gsb_autofunc_checkbutton_new (_("Print a title: "),
						 gsb_data_print_config_get_draw_title (),
						 G_CALLBACK (sens_desensitive_pointeur), entry,
						 G_CALLBACK (gsb_data_print_config_set_draw_title), 0);
    gtk_box_pack_start (GTK_BOX (hbox),
			check_button,
			FALSE, FALSE, 0);

    gtk_widget_set_sensitive (entry, gsb_data_print_config_get_draw_title ());
    gtk_box_pack_start (GTK_BOX (hbox),
			entry,
			TRUE, TRUE, 0);

    /* add the dates interval */
    hbox = gtk_hbox_new (FALSE, 10);

    check_button = gsb_autofunc_checkbutton_new (_("Select dates interval: "),
						 gsb_data_print_config_get_draw_interval_dates (),
						 G_CALLBACK (sens_desensitive_pointeur), hbox,
						 G_CALLBACK (gsb_data_print_config_set_draw_interval_dates), 0);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			check_button,
			FALSE, FALSE, 0);


    gtk_box_pack_start (GTK_BOX (paddingbox),
			hbox,
			FALSE, FALSE, 0);

    label = gtk_label_new (_("Initial date : "));
    gtk_box_pack_start (GTK_BOX (hbox),
			label,
			FALSE, FALSE, 0);
    
    init_date_entry = gsb_calendar_entry_new (FALSE);
    if (draw_initial_date)
	gsb_calendar_entry_set_date (init_date_entry, draw_initial_date);
    gtk_box_pack_start (GTK_BOX (hbox),
			init_date_entry,
			FALSE, FALSE, 0);

    label = gtk_label_new (_("Final date : "));
    gtk_box_pack_start (GTK_BOX (hbox),
			label,
			FALSE, FALSE, 0);
    
    final_date_entry = gsb_calendar_entry_new (FALSE);
    if (draw_final_date)
	gsb_calendar_entry_set_date (final_date_entry, draw_final_date);
    gtk_box_pack_start (GTK_BOX (hbox),
			final_date_entry,
			FALSE, FALSE, 0);

    g_object_set_data ( G_OBJECT ( operation ), "init_date_entry", init_date_entry );
    g_object_set_data ( G_OBJECT ( operation ), "final_date_entry", final_date_entry );

    check_button = gsb_autofunc_checkbutton_new (_("Use value date"),
						 gsb_data_print_config_get_draw_dates_are_value_dates (),
						 NULL, NULL,
						 G_CALLBACK (gsb_data_print_config_set_draw_dates_are_value_dates), 0);
    gtk_box_pack_start (GTK_BOX (hbox),
			check_button,
			FALSE, FALSE, 0);

    if (!gsb_data_print_config_get_draw_interval_dates ())
	gtk_widget_set_sensitive (hbox, FALSE);

    /* set up all the checkbuttons */
    check_button = gsb_autofunc_checkbutton_new (_("Draw the lines between transactions"),
						 gsb_data_print_config_get_draw_lines (),
						 NULL, NULL,
						 G_CALLBACK (gsb_data_print_config_set_draw_lines), 0);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			check_button,
			FALSE, FALSE, 0);
    check_button = gsb_autofunc_checkbutton_new (_("Draw the lines between the columns"),
						 gsb_data_print_config_get_draw_column (),
						 NULL, NULL,
						 G_CALLBACK (gsb_data_print_config_set_draw_column), 0);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			check_button,
			FALSE, FALSE, 0);

    check_button = gsb_autofunc_checkbutton_new (_("Fill the background as the transactions list"),
						 gsb_data_print_config_get_draw_background (),
						 NULL, NULL,
						 G_CALLBACK (gsb_data_print_config_set_draw_background), 0);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			check_button,
			FALSE, FALSE, 0);

    check_button = gsb_autofunc_checkbutton_new (_("Print the archives lines"),
						 gsb_data_print_config_get_draw_archives (),
						 NULL, NULL,
						 G_CALLBACK (gsb_data_print_config_set_draw_archives), 0);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			check_button,
			FALSE, FALSE, 0);

    check_button = gsb_autofunc_checkbutton_new (_("Print the names of the columns"),
						 gsb_data_print_config_get_draw_columns_name (),
						 NULL, NULL,
						 G_CALLBACK (gsb_data_print_config_set_draw_columns_name), 0);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			check_button,
			FALSE, FALSE, 0);

    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Fonts") );

    /* set up the font of the transactions,
     * by default use the font of the lists */
    hbox = gtk_hbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			hbox,
			FALSE, FALSE, 0);

    label = gtk_label_new (_("Transactions font"));
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_size_group_add_widget ( size_group, label );
    gtk_box_pack_start (GTK_BOX (hbox),
			label,
			FALSE, FALSE, 0);

    fontname_transactions = pango_font_description_to_string (gsb_data_print_config_get_font_transactions ());
    font_button_transactions = gtk_font_button_new_with_font ( fontname_transactions );
    gtk_font_button_set_use_font ( GTK_FONT_BUTTON(font_button_transactions), TRUE );
    gtk_font_button_set_use_size ( GTK_FONT_BUTTON(font_button_transactions), TRUE );
    gtk_font_button_set_title ( GTK_FONT_BUTTON(font_button_transactions), _("Choosing font") );
    gtk_box_pack_start (GTK_BOX (hbox),
			font_button_transactions,
			TRUE, TRUE, 0);

    /* set up the font for the title */
    hbox = gtk_hbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX (paddingbox),
			hbox,
			FALSE, FALSE, 0);

    label = gtk_label_new (_("Title font"));
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_size_group_add_widget ( size_group, label );
    gtk_box_pack_start (GTK_BOX (hbox),
			label,
			FALSE, FALSE, 0);

    fontname_title = pango_font_description_to_string (gsb_data_print_config_get_font_title ());
    font_button_title =  gtk_font_button_new_with_font ( fontname_title );
    gtk_font_button_set_use_font ( GTK_FONT_BUTTON(font_button_title), TRUE );
    gtk_font_button_set_use_size ( GTK_FONT_BUTTON(font_button_title), TRUE );
    gtk_font_button_set_title ( GTK_FONT_BUTTON(font_button_title), _("Choosing font") );
    gtk_box_pack_start (GTK_BOX (hbox),
			font_button_title,
			TRUE, TRUE, 0);


    /* save what we have done in all cases, so if we cancel and come back, our values
     * come back */
    gsb_data_print_config_set_font_transaction (pango_font_description_from_string (fontname_transactions));
    gsb_data_print_config_set_font_title (pango_font_description_from_string (fontname_title));
    draw_initial_date = gsb_calendar_entry_get_date (init_date_entry);
    draw_final_date = gsb_calendar_entry_get_date (final_date_entry);

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
gboolean print_transactions_list_apply ( GtkPrintOperation * operation,
					 GtkWidget * widget,
					 gpointer null )
{
    GtkFontButton * font_button_transactions, * font_button_title;
    GtkWidget *calendar;

    font_button_transactions = g_object_get_data ( G_OBJECT ( operation ), "font_transaction_button" );
    font_button_title = g_object_get_data ( G_OBJECT( operation ), "font_title_button" );
    
    gsb_data_print_config_set_font_transaction ( pango_font_description_from_string (
                        gtk_font_button_get_font_name ( font_button_transactions ) ) );
    gsb_data_print_config_set_font_title ( pango_font_description_from_string (
                        gtk_font_button_get_font_name ( font_button_title ) ) );


    if ( gsb_data_print_config_get_draw_interval_dates () )
    {
        calendar = g_object_get_data ( G_OBJECT(operation), "init_date_entry" );
        draw_initial_date = gsb_calendar_entry_get_date ( calendar );

        calendar = g_object_get_data ( G_OBJECT(operation), "final_date_entry" );
        draw_final_date = gsb_calendar_entry_get_date ( calendar );
    }

    return FALSE;
}



/**
 * Pop up a transaction print config dialog.
 *
 * \param button	Not used.
 * \param null		Not used.
 *
 * \return		FALSE
 */
gboolean print_transactions_list ( GtkWidget * button,
				   gpointer null )
{
    print_dialog_config ( G_CALLBACK (print_transactions_list_begin),
			  G_CALLBACK (print_transactions_list_draw_page),
			  _("Fonts & layout"),
			  G_CALLBACK (print_transactions_list_layout_config),
			  G_CALLBACK (print_transactions_list_apply),
              NULL );
    return FALSE;
}



/**
 * Function called first when try to print the transaction list
 * initialize the variables and calculate the number of pages
 *
 * \param operation	GtkPrintOperation
 * \param context	GtkPrintContext
 * \param null
 *
 * \return FALSE
 * */
gboolean print_transactions_list_begin ( GtkPrintOperation *operation,
					 GtkPrintContext *context,
					 gpointer null )
{
    CustomList *custom_list;
    gint size_transaction;
    gint size_title = 0;
    gint size_archive = 0;
    gdouble transactions_per_page_double;
    gint nb_pages;
    gint number_of_archives = 0;

    devel_debug (NULL);

    /* we need to calculate the number of pages */
    cr = gtk_print_context_get_cairo_context (context);
    custom_list = transaction_model_get_model ();

    /* get the size of the title */
    if (gsb_data_print_config_get_draw_title () && title_string && strlen (title_string))
	size_title = pango_font_description_get_size (gsb_data_print_config_get_font_title ())/PANGO_SCALE;

    /* we need the number of archives and of transactions to print
     * number_of_archives will be set to 0 if no draw archives */
    print_transactions_list_get_visibles_lines (&number_of_archives,
						&total_transactions_to_print );

/*  xxx   mk_include 2 fois de suite modifie qd même des fichiers */

    /* get the size of a complete transaction and an archive */
    size_row = pango_font_description_get_size (gsb_data_print_config_get_font_transactions ())/PANGO_SCALE;
    size_transaction = size_row * custom_list -> nb_rows_by_transaction + 2*gsb_data_print_config_get_draw_lines ();
    size_archive = size_row + 2*gsb_data_print_config_get_draw_lines ();

    /* the heigh of a page decrease of 1 line if we use the columns titles */
    page_height = gtk_print_context_get_height (context) - gsb_data_print_config_get_draw_columns_name ()*size_transaction;

    /* how much transactions we can show in a page : */
    transactions_per_page_double = page_height / size_transaction;

    if (!size_title && !gsb_data_print_config_get_draw_archives ())
    {
	/* simple way : no archives and no title */
	nb_pages = ceil ( total_transactions_to_print / transactions_per_page_double );
	transactions_in_first_page = floor (transactions_per_page_double);
    }
    else
    {
	/* there are title or archives, it's more complex because it will have less transactions
	 * on the first page */
	gint first_page_height = page_height - size_title - size_archive*number_of_archives;

	transactions_in_first_page = floor (first_page_height / size_transaction);
	nb_pages = 1 + ceil (  (total_transactions_to_print - transactions_in_first_page)/ transactions_per_page_double );
    }

    /* set the number of page */
    gtk_print_operation_set_n_pages ( GTK_PRINT_OPERATION (operation),
				      nb_pages );
    /* save the nb of transactions per page */
    transactions_per_page = floor (transactions_per_page_double);

    /* calculate the size and position of the columns */
    page_width = gtk_print_context_get_width (context);
    print_transactions_list_calculate_columns (page_width);

    total_transactions_printed = 0;
    current_row_to_print = 0;
    return FALSE;
}



/**
 * print the page
 *
 * \param operation	GtkPrintOperation
 * \param context	GtkPrintContext
 * \param page		page to print
 * \param null
 *
 * \return FALSE
 * */
gboolean print_transactions_list_draw_page ( GtkPrintOperation *operation,
					     GtkPrintContext *context,
					     gint page,
					     gpointer null )
{
    CustomList *custom_list;
    gboolean color_bg = TRUE;
    gint line_position = 0;
    gint transactions_to_draw;

    devel_debug_int (page);

    /* draw the title */
    if (!page)
	line_position = print_transactions_list_draw_title (context, line_position);

    /* draw the columns titles */
    if (gsb_data_print_config_get_draw_columns_name ())
	line_position = print_transactions_list_draw_columns_title (context, line_position);

    /* draw the transactions lines */
    custom_list = transaction_model_get_model ();

    if (page)
	transactions_to_draw = transactions_per_page;
    else
	transactions_to_draw = transactions_in_first_page;
    if (transactions_to_draw > (total_transactions_to_print - total_transactions_printed))
	transactions_to_draw = total_transactions_to_print - total_transactions_printed;

    while (transactions_to_draw)
    {
	gint i;
	CustomRecord *record = NULL;

	/* begin a transaction : fill the line before the transaction */
	line_position = print_transactions_list_draw_line (line_position);

	/* print the transaction */
	for (i=0 ; i<custom_list -> nb_rows_by_transaction ; i++)
	{
	    record = custom_list -> visibles_rows[current_row_to_print];

	    /* if it's an archive, check we want it */
	    if (record -> what_is_line == IS_ARCHIVE && !gsb_data_print_config_get_draw_archives ())
	    {
		/* go to the next row but come back to the first line of transaction */
		current_row_to_print++;
		i--;
		continue;
	    }

	    /* if we use the dates, it's here */
	    if (gsb_data_print_config_get_draw_interval_dates () && draw_initial_date && draw_final_date
		&& record -> what_is_line == IS_TRANSACTION)
	    {
		/* we want an interval, so check the transaction */
		gint transaction_number;
		const GDate *date = NULL;

		transaction_number = gsb_data_transaction_get_transaction_number (record -> transaction_pointer);
		if (gsb_data_print_config_get_draw_dates_are_value_dates ())
		    date = gsb_data_transaction_get_value_date (transaction_number);

		/* if no value date, get the date */
		if (!date)
		    date = gsb_data_transaction_get_date (transaction_number);

		if (g_date_compare (date,
				    draw_initial_date) < 0
		    ||
		    g_date_compare (date,
				    draw_final_date) > 0)
		{
		    /* the transaction is not into the dates, go to the next transaction */
		    current_row_to_print = current_row_to_print + custom_list -> nb_rows_by_transaction;
		    i--;
		    continue;
		}
	    }

	    /* begin the transaction, fill the background */
	    if (!i)
		print_transactions_list_draw_background (record, color_bg, line_position );

	    /* draw the last column */
	    print_transactions_list_draw_column (page_width, line_position);

	    line_position = print_transactions_list_draw_row (context, record, line_position);

	    current_row_to_print++;

	    /* an archive is 1 line */
	    if (record -> what_is_line == IS_ARCHIVE)
		break;
	}

	/* we are on the last record of the transaction or archive,
	 * decrease the number of transactions to draw */
	if (record -> what_is_line == IS_TRANSACTION)
	{
	    transactions_to_draw--;
	    total_transactions_printed++;
	}

	/* add a space between 2 transactions */
	line_position++;

	color_bg = !color_bg;
    }

    /* draw the last line */
    print_transactions_list_draw_line (line_position);
    return FALSE;
}



/**
 * find the number of archives and of transactions, according to the user conf
 * so if archives are not wanted, the value filled in number_of_archives will be always 0.
 *
 * \param number_of_archives	a pointer of gint to fill with the number of archives
 * \param number_of_transactions	a pointer to gint to fill with the number of transactions to show
 *
 * \return FALSE;
 * */
gboolean print_transactions_list_get_visibles_lines ( gint *number_of_archives,
						      gint *number_of_transactions )
{
    gint i;
    CustomList *custom_list;
    gint archives_nb = 0;
    gint transactions_nb = 0;

    custom_list = transaction_model_get_model ();

    for (i=0 ; i<custom_list -> num_visibles_rows ; i++)
    {
        CustomRecord *record;

        record = custom_list -> visibles_rows[i];
        switch (record -> what_is_line)
        {
            case IS_ARCHIVE:
            if (gsb_data_print_config_get_draw_archives ())
                archives_nb++;
            break;
            case IS_TRANSACTION:
            if ( gsb_data_print_config_get_draw_interval_dates () && draw_initial_date && draw_final_date )
            {
                /* we want an interval, so check the transaction */
                gint transaction_number;
                const GDate *date;

                transaction_number = gsb_data_transaction_get_transaction_number (record -> transaction_pointer);
                if (gsb_data_print_config_get_draw_dates_are_value_dates ())
                {
                    date = gsb_data_transaction_get_value_date (transaction_number);

                    /* if no value date, get the date */
                    if (!date)
                        date = gsb_data_transaction_get_date (transaction_number);
                }
                else
                    date = gsb_data_transaction_get_date (transaction_number);

                if (date
                &&
                g_date_compare (date,
                        draw_initial_date) >= 0
                &&
                g_date_compare (date,
                        draw_final_date) <= 0)
                transactions_nb++;
            }
            else
                transactions_nb++;

            break;
        }
    }

    /* before returning the value, transactions_nb is in fact the number of lines of visibles transactions,
     * so need to divide by the number of lines for 1 transaction, and remove the white line if necessary */
    if ( gsb_data_print_config_get_draw_interval_dates () && draw_initial_date && draw_final_date )
        transactions_nb = ( transactions_nb ) / custom_list -> nb_rows_by_transaction;
    else
        transactions_nb = (transactions_nb - custom_list -> nb_rows_by_transaction)
                        / custom_list -> nb_rows_by_transaction;

    *number_of_archives = archives_nb;
    *number_of_transactions = transactions_nb;
    return FALSE;
}

/**
 * fill the columns positions and columns width
 *
 * \param page_width	the page width given by the print_context
 *
 * \return
 * */
static void print_transactions_list_calculate_columns ( gdouble page_width )
{
    gint total_text_width = 0;
    gint column;

    /* set the positions of the columns */
    columns_position[0] = 0.0;
    for ( column=1 ; column<CUSTOM_MODEL_VISIBLE_COLUMNS ; column++)
	columns_position[column] = (gdouble) (transaction_col_width[column - 1]*page_width/100) + columns_position[column - 1];

    /* set the page_width of the columns */
    for (column = 0 ; column < (CUSTOM_MODEL_VISIBLE_COLUMNS  - 1); column++ )
    {
	columns_width[column] = (columns_position[column + 1] - columns_position[column] - 3*gsb_data_print_config_get_draw_column ()) * PANGO_SCALE;
	total_text_width = total_text_width + columns_width[column];
    }
    /* last column is the rest of the line */
    columns_width[CUSTOM_MODEL_VISIBLE_COLUMNS - 1] = (page_width -3*gsb_data_print_config_get_draw_column ()*(CUSTOM_MODEL_VISIBLE_COLUMNS)) * PANGO_SCALE - total_text_width;
}

/**
 * draw the title if asked
 *
 * \param cr		the cairo_t
 * \param context	the GtkPrintContext
 * \param line_position	the position to insert the title
 * \param page_width	the pag
 *
 * \return the new line_position to continue to fill the page
 * */
static gint print_transactions_list_draw_title ( GtkPrintContext *context,
						 gint line_position )
{
    if (gsb_data_print_config_get_draw_title () && title_string && strlen (title_string))
    {
	PangoLayout *layout;

	cairo_move_to (cr, 0, line_position);

	/* create the new layout */
	layout = gtk_print_context_create_pango_layout (context);

	pango_layout_set_text (layout, title_string, -1);
	pango_layout_set_font_description (layout, gsb_data_print_config_get_font_title ());
	pango_layout_set_width (layout,page_width*PANGO_SCALE);
	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
	pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
	pango_cairo_show_layout (cr, layout);
	g_object_unref (layout);
	line_position = line_position + pango_font_description_get_size (gsb_data_print_config_get_font_title ())/PANGO_SCALE;
    }
    return line_position;
}

/**
 * draw the title of the columns
 *
 * \param line_position	the position to insert the titles
 * 
 * \return the new line_position to continue to fill the page
 * */
static gint print_transactions_list_draw_columns_title ( GtkPrintContext *context,
							 gint line_position)
{
    gint column;

    if (!gsb_data_print_config_get_draw_columns_name ())
	return line_position;

    for (column=0 ; column<CUSTOM_MODEL_VISIBLE_COLUMNS ; column++)
    {
	PangoLayout *layout;
	gchar *text;
	gint column_position;

	column_position = columns_position[column];

	/* get the text */
	text = _(titres_colonnes_liste_operations[column]);
	if (!text)
	    continue;

	cairo_move_to (cr, column_position, line_position);

	/* create the new layout */
	layout = gtk_print_context_create_pango_layout (context);

	pango_layout_set_text (layout, text, -1);
	pango_layout_set_font_description (layout, gsb_data_print_config_get_font_transactions ());
	pango_layout_set_width (layout,columns_width[column]);
	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
	pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);

	pango_cairo_show_layout (cr, layout);
	g_object_unref (layout);
    }
    line_position = line_position + size_row + gsb_data_print_config_get_draw_lines ();

    return line_position;
}


/**
 * draw the background of all the rows of the transaction
 *
 * \param cr		cairo context
 * \param record	record to draw
 * \param color_bg	TRUE to colorize, FALSE to let blank
 * \param line_position	position where drawing the line
 *
 * \return
 * */
static void print_transactions_list_draw_background ( CustomRecord *record,
						      gboolean color_bg,
						      gint line_position )
{
    if (!gsb_data_print_config_get_draw_background ())
	return;

    if (record -> what_is_line == IS_ARCHIVE)
    {
	cairo_rectangle (cr, 0, line_position, page_width, size_row + 2*gsb_data_print_config_get_draw_lines ());
	cairo_set_source_rgb (cr, (gdouble) archive_background_color.red/65535, (gdouble) archive_background_color.green/65535, (gdouble) archive_background_color.blue/65535);
    }
    else
    {
	if (color_bg)
	{
	    CustomList *custom_list = transaction_model_get_model ();
	    cairo_rectangle (cr, 0, line_position, page_width, custom_list -> nb_rows_by_transaction * size_row + 2*gsb_data_print_config_get_draw_lines ());
	    cairo_set_source_rgb (cr, (gdouble) couleur_fond[0].red/65535, (gdouble) couleur_fond[0].green/65535, (gdouble) couleur_fond[0].blue/65535);
	}
	else
	{
	    CustomList *custom_list = transaction_model_get_model ();
	    cairo_rectangle (cr, 0, line_position, page_width, custom_list -> nb_rows_by_transaction * size_row + 2*gsb_data_print_config_get_draw_lines ());
	    cairo_set_source_rgb (cr, (gdouble) couleur_fond[1].red/65535, (gdouble) couleur_fond[1].green/65535, (gdouble) couleur_fond[1].blue/65535);
	}
    }
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
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
static gint print_transactions_list_draw_line ( gint line_position )
{
    if (!gsb_data_print_config_get_draw_lines ())
	return line_position;

    /* add a space with the last transaction */
    line_position++;
    cairo_move_to (cr, 0, line_position);
    cairo_set_line_width (cr, 0.5);
    cairo_line_to (cr, page_width, line_position);
    cairo_stroke (cr);

    return line_position;
}

/**
 * draw a column line 
 *
 * \param column_position	column position
 * \param line_position		line position
 *
 * \return the new column position
 * */
static gint print_transactions_list_draw_column ( gint column_position,
						 gint line_position )
{
    if (!gsb_data_print_config_get_draw_column ())
	return column_position;

    cairo_move_to (cr, column_position, line_position);
    cairo_set_line_width (cr, 0.5);
    cairo_line_to (cr, column_position, line_position + size_row + 2*gsb_data_print_config_get_draw_lines ());
    cairo_stroke (cr);
    /* add a space with the next column */
    column_position++;

    return column_position;
}

/**
 * draw a line of a transaction
 *
 * \param
 *
 * \return the new line_position
 * */
static gint print_transactions_list_draw_row ( GtkPrintContext *context,
					      CustomRecord *record,
					      gint line_position )
{
    gint column;
    gfloat alignment[] = {
	PANGO_ALIGN_CENTER, PANGO_ALIGN_CENTER, PANGO_ALIGN_LEFT, 
	PANGO_ALIGN_CENTER, PANGO_ALIGN_RIGHT, PANGO_ALIGN_RIGHT, PANGO_ALIGN_RIGHT
    };

    for (column=0 ; column<CUSTOM_MODEL_VISIBLE_COLUMNS ; column++)
    {
	PangoLayout *layout;
	gchar *text;
	gint column_position;

	column_position = columns_position[column];

	/* draw first the column */
	column_position = print_transactions_list_draw_column (column_position, line_position);

	/* get the text */
	text = record -> visible_col[column];
	if (!text)
	    continue;

	cairo_move_to (cr, column_position, line_position);

	/* create the new layout */
	layout = gtk_print_context_create_pango_layout (context);

	pango_layout_set_text (layout, text, -1);
	pango_layout_set_font_description (layout, gsb_data_print_config_get_font_transactions ());
	pango_layout_set_width (layout,columns_width[column]);
	pango_layout_set_alignment (layout, alignment[column]);
	pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);

	pango_cairo_show_layout (cr, layout);
	g_object_unref (layout);
    }
    /* go to the next row */
    line_position = line_position + size_row;

    return line_position;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
