/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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

/**
 * \file gsb_assistant_archive.c
 * we find here the complete assistant to archive some transactions
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_archive_export.h"
#include "gsb_assistant.h"
#include "export_csv.h"
#include "gsb_data_archive.h"
#include "gsb_data_fyear.h"
#include "gsb_file.h"
#include "gsb_file_save.h"
#include "gsb_file_util.h"
#include "utils_dates.h"
#include "qif.h"
#include "utils_str.h"
#include "utils.h"
#include "structures.h"
#include "gsb_transactions_list.h"
/*END_INCLUDE*/


enum archive_export_assistant_page
{
    ARCHIVE_EXPORT_ASSISTANT_INTRO= 0,
    ARCHIVE_EXPORT_ASSISTANT_CHOOSE,
    ARCHIVE_EXPORT_ASSISTANT_NAME,
    ARCHIVE_EXPORT_ASSISTANT_SUCCESS
};

/** Columns for payment methods tree */
enum archives_export_columns {
    ARCHIVES_EXPORT_SELECT_COLUMN = 0,
    ARCHIVES_EXPORT_NAME_COLUMN,
    ARCHIVES_EXPORT_INIT_DATE,
    ARCHIVES_EXPORT_FINAL_DATE,
    ARCHIVES_EXPORT_FYEAR_NAME,
    ARCHIVES_EXPORT_REPORT_TITLE,
    ARCHIVES_EXPORT_NUMBER,
    NUM_ARCHIVES_EXPORT_COLUMNS,
};

static GtkWidget *archive_export_treeview = NULL;
static GtkWidget *archive_export_label = NULL;
static GtkWidget *archive_export_filechooser = NULL;
static GtkWidget *archive_export_combobox = NULL;
static GtkWidget *archive_export_label_success = NULL;

/*START_STATIC*/
static gint gsb_assistant_archive_export_get_selected_archive ( GtkTreeModel *model );
static GtkWidget *gsb_assistant_archive_export_page_choose ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_archive_export_page_name ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_archive_export_page_succes ( GtkWidget *assistant );
static gboolean gsb_assistant_archive_export_toggled ( GtkCellRendererToggle *renderer,
						       gchar *path_string,
						       GtkWidget *assistant );
static gboolean gsb_assistant_archive_switch ( GtkWidget *assistant,
					       gint new_page );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/


/**
 * this function is called to launch the assistant to export archives
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_archive_export_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Export an archive"),
				    _("This assistant will help you to export an archive into gsb, QIF or CSV format."
				      "Note that nothing will be deleted in Grisbi, it's just an export."
				      "If you want to delete the archive from Grisbi, you need to do it from the preference window."),
				    "archive.png",
				    G_CALLBACK (gsb_assistant_archive_switch));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_archive_export_page_choose (assistant),
			     ARCHIVE_EXPORT_ASSISTANT_CHOOSE,
			     ARCHIVE_EXPORT_ASSISTANT_INTRO,
			     ARCHIVE_EXPORT_ASSISTANT_NAME,
			     G_CALLBACK (gsb_assistant_archive_switch));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_archive_export_page_name (assistant),
			     ARCHIVE_EXPORT_ASSISTANT_NAME,
			     ARCHIVE_EXPORT_ASSISTANT_CHOOSE,
			     ARCHIVE_EXPORT_ASSISTANT_SUCCESS,
			     G_CALLBACK (gsb_assistant_archive_switch));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_archive_export_page_succes (assistant),
			     ARCHIVE_EXPORT_ASSISTANT_SUCCESS,
			     ARCHIVE_EXPORT_ASSISTANT_NAME,
			     FALSE,
			     G_CALLBACK (gsb_assistant_archive_switch));
    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);

    return return_value;
}

/**
 * create the page 2 of the assistant
 * this page permit to choose the archive to export
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_archive_export_page_choose ( GtkWidget *assistant )
{
    GtkWidget *vbox_page, *scrolled_window;
    GtkWidget *paddingbox;
    GtkListStore *archive_model;
    gchar *titles[] = {
	"", _("Name"), _("Initial date"), _("Final date"), _("Financial year"), _("Report name")
    };
    gfloat alignment[] = {
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER , COLUMN_CENTER, COLUMN_CENTER
    };
    gint i;
    GSList *tmp_list;

    /* create the page */
    vbox_page = gtk_vbox_new ( FALSE, 0);
    gtk_container_set_border_width ( GTK_CONTAINER(vbox_page), 12 );
    paddingbox = new_paddingbox_with_title (vbox_page, TRUE,
					    _("Select the archive to export"));


    /* Create scrolled window */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN);
    gtk_box_pack_start ( GTK_BOX (paddingbox),
			 scrolled_window,
			 TRUE, TRUE, 0 );

    /* Create tree view */
    archive_model = gtk_list_store_new (NUM_ARCHIVES_EXPORT_COLUMNS,
					G_TYPE_BOOLEAN,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_INT );
    archive_export_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (archive_model) );
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (archive_export_treeview), TRUE);
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			archive_export_treeview );

    /* set the columns */
    for (i=0 ; i<6 ; i++)
    {
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_sizing ( column,
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_alignment ( column,
					     alignment[i] );
	gtk_tree_view_column_set_title ( column, titles[i] );
	gtk_tree_view_column_set_expand ( column, TRUE );
	gtk_tree_view_column_set_resizable ( column,
					     TRUE );

	if (i)
	{
	    /* we are on a text column */
	    renderer = gtk_cell_renderer_text_new ();
	    g_object_set ( G_OBJECT (renderer),
			   "xalign", alignment[i],
			   NULL );
	    gtk_tree_view_column_pack_start ( column, renderer, TRUE );
	    gtk_tree_view_column_set_attributes (column, renderer,
						 "text", i,
						 NULL);
	}
	else
	{
	    /* we are on the select column */
	    renderer = gtk_cell_renderer_toggle_new ();
	    g_signal_connect (renderer,
			      "toggled",
			      G_CALLBACK (gsb_assistant_archive_export_toggled),
			      assistant );
	    gtk_tree_view_column_pack_start ( column, renderer, TRUE );
	    gtk_tree_view_column_set_attributes (column, renderer,
						 "active", i,
						 NULL);
	}

	gtk_tree_view_append_column ( GTK_TREE_VIEW(archive_export_treeview), column);
    }

    /* fill the list */
    tmp_list = gsb_data_archive_get_archives_list ();

    while ( tmp_list )
    {
	gint archive_number;
	GtkTreeIter iter;
	gchar *init_date;
	gchar *final_date;

	archive_number = gsb_data_archive_get_no_archive (tmp_list -> data);

	init_date = gsb_format_gdate (gsb_data_archive_get_beginning_date (archive_number));
	final_date = gsb_format_gdate (gsb_data_archive_get_end_date (archive_number));

	gtk_list_store_append ( GTK_LIST_STORE (archive_model),
				&iter );
	gtk_list_store_set ( GTK_LIST_STORE (archive_model),
			     &iter,
			     ARCHIVES_EXPORT_NAME_COLUMN, gsb_data_archive_get_name (archive_number),
			     ARCHIVES_EXPORT_INIT_DATE, init_date,
			     ARCHIVES_EXPORT_FINAL_DATE, final_date,
			     ARCHIVES_EXPORT_FYEAR_NAME, gsb_data_fyear_get_name (gsb_data_archive_get_fyear (archive_number)),
			     ARCHIVES_EXPORT_REPORT_TITLE, gsb_data_archive_get_report_title (archive_number),
			     ARCHIVES_EXPORT_NUMBER, archive_number,
			     -1 );
	if (init_date)
	    g_free (init_date);
	if (final_date)
	    g_free (final_date);

	tmp_list = tmp_list -> next;
    }

    gtk_widget_show_all (vbox_page);
    return ( vbox_page );
}

/**
 * create the page 3 of the assistant
 * this page permit to choose a name for the archive
 * and choose the format to export
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_archive_export_page_name ( GtkWidget *assistant )
{
    GtkWidget * vbox, * hbox;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    archive_export_label = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC ( archive_export_label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( archive_export_label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), archive_export_label, FALSE, FALSE, 0 );

    /* Layout */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 gtk_label_new ( _("Export format: ") ),
			 FALSE, FALSE, 0 );

    /* Combo box */
    archive_export_combobox = gtk_combo_box_new_text();
    gtk_combo_box_append_text ( GTK_COMBO_BOX(archive_export_combobox), _("Grisbi (GSB) format" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(archive_export_combobox), _("QIF format" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(archive_export_combobox), _("CSV format" ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), archive_export_combobox, TRUE, TRUE, 0 );

    archive_export_filechooser = gtk_file_chooser_widget_new ( GTK_FILE_CHOOSER_ACTION_SAVE );
    gtk_file_chooser_set_extra_widget ( GTK_FILE_CHOOSER(archive_export_filechooser), hbox );
    gtk_box_pack_start ( GTK_BOX ( vbox ), archive_export_filechooser, TRUE, TRUE, 0 );

    gtk_combo_box_set_active ( GTK_COMBO_BOX(archive_export_combobox), 0 );

    return vbox;
}


/**
 * create the page 4 of the assistant
 * this is the success page
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_archive_export_page_succes ( GtkWidget *assistant )
{
    GtkWidget *vbox;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    archive_export_label_success = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC (archive_export_label_success), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL (archive_export_label_success), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), archive_export_label_success, FALSE, FALSE, 0 );

    return vbox;
}



/**
 * called when switch page
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_archive_switch ( GtkWidget *assistant,
					       gint new_page )
{
    gint archive_number;
    GtkTreeModel *model;
    const gchar *export_name;
    gboolean success = FALSE;
    gchar* tmpstr;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (archive_export_treeview));
    archive_number = gsb_assistant_archive_export_get_selected_archive (model);

    switch (new_page)
    {
	case ARCHIVE_EXPORT_ASSISTANT_INTRO:
	    break;

	case ARCHIVE_EXPORT_ASSISTANT_CHOOSE:
	    if (archive_number)
		gsb_assistant_sensitive_button_next (assistant, TRUE);
	    else
		gsb_assistant_sensitive_button_next (assistant, FALSE);
	    break;

	case ARCHIVE_EXPORT_ASSISTANT_NAME:
	    /* if we come here, an archive must have been selected,
	     * so needn't to check */
	    tmpstr = g_markup_printf_escaped ( 
                        _("<span size=\"x-large\">Exporting the archive: %s</span>"),
                        gsb_data_archive_get_name (archive_number));
	    gtk_label_set_markup ( GTK_LABEL ( archive_export_label ), tmpstr);
	    g_free ( tmpstr );
        /* on remplace les slash des dates par des points
         * avant de fixer le nom du fichier par défaut */
        tmpstr = ( gchar * )gsb_data_archive_get_name ( archive_number );
        tmpstr = my_strdelimit (tmpstr, "/", "." );
	    gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER (archive_export_filechooser), tmpstr );
	    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (archive_export_filechooser), gsb_file_get_last_path () );
	    g_free ( tmpstr );
	    /* need to set the next button to the next function,
	     * because if the export failed and the user did previous button,
	     * the next button stay in the close state */
	    gsb_assistant_change_button_next ( assistant, GTK_STOCK_GO_FORWARD,
					       GTK_RESPONSE_YES );

	    break;

	case ARCHIVE_EXPORT_ASSISTANT_SUCCESS:
	    export_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (archive_export_filechooser));
	    switch (gtk_combo_box_get_active (GTK_COMBO_BOX(archive_export_combobox)))
	    {
		case 0:
		    /* GSB format */
		    /* the gsb_file_save_save_file function write directly, so we need to check before
		     * if the file exists */
            /* on vérifie juste que l'extension existe */
            if ( ! g_strrstr ( export_name, ".gsb" ) )
            {
               export_name = g_strconcat ( export_name, ".gsb", NULL );
            }
		    success = gsb_file_util_test_overwrite (export_name)
			&&
			gsb_file_save_save_file ( export_name, conf.compress_backup, archive_number);
		    break;

		case 1:
		    /* QIF format */
            if ( ! g_strrstr ( export_name, ".qif" ) )
            {
               export_name = g_strconcat ( export_name, ".qif", NULL );
            }
		    success = gsb_qif_export_archive ( export_name, archive_number);
		    break;

		case 2:
		    /* CSV format */
            if ( ! g_strrstr ( export_name, ".csv" ) )
            {
               export_name = g_strconcat ( export_name, ".csv", NULL );
            }
		    success = gsb_csv_export_archive ( export_name, archive_number);
		    break;
	    }
	    /* now success = TRUE or FALSE, show the good final page of assistant */
	    if (success)
		gtk_label_set_text ( GTK_LABEL (archive_export_label_success),
				     _("Success !\nThe export of the archive finished successfully.\n\nYou can now close the assistant."));
	    else
	    {
		gtk_label_set_text ( GTK_LABEL (archive_export_label_success),
				     _("Error !\nAn error occured while saving the archive.\n\nPlease press the Previous button to correct the problem,\nor the close button to cancel the action."));
		gsb_assistant_sensitive_button_prev ( assistant, TRUE );
	    }
    }
    return FALSE;
}

/**
 * callback called when toggle a radio button of an archive
 * we want only 1 toggle, so erase the others before
 *
 * \param renderer
 * \param path
 * \param assistant
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_archive_export_toggled ( GtkCellRendererToggle *renderer,
						       gchar *path_string,
						       GtkWidget *assistant )
{
    GtkTreeIter iter;
    gboolean value;
    GtkTreePath *path;
    GtkTreePath *path_clicked;
    GtkTreeModel *model;
    gboolean selected = FALSE;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (archive_export_treeview));
    path_clicked = gtk_tree_path_new_from_string (path_string);
    path = gtk_tree_path_new_first ();

    while (gtk_tree_model_get_iter ( GTK_TREE_MODEL (model),
				     &iter,
				     path ))
    {
	/* if we are on the clicked path, we invert the value,
	 * else, we set the value to 0 */
	if (gtk_tree_path_compare (path, path_clicked))
	    /* the 2 path are different */
	    value = 0;
	else
	{
	    /* the paths are equal */
	    gtk_tree_model_get ( GTK_TREE_MODEL (model),
				 &iter,
				 ARCHIVES_EXPORT_SELECT_COLUMN, &value,
				 -1 );
	    value = !value;

	    /* we will unsensitive the next button only if something chosen */
	    if (value)
		selected = TRUE;
	}
	gtk_list_store_set ( GTK_LIST_STORE (model),
			     &iter,
			     ARCHIVES_EXPORT_SELECT_COLUMN, value,
			     -1 );
	gtk_tree_path_next (path);
    }
    gtk_tree_path_free (path);
    gtk_tree_path_free (path_clicked);

    gsb_assistant_sensitive_button_next (assistant, selected);

    return FALSE;
}


/**
 * return the selected archive number
 *
 * \param model
 *
 * \return a gint, the selected archive number, or 0 if none selected
 * */
static gint gsb_assistant_archive_export_get_selected_archive ( GtkTreeModel *model )
{
    GtkTreeIter iter;

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	return 0;

    do
    {
	gint value;
	gint archive_number;

	gtk_tree_model_get ( GTK_TREE_MODEL (model),
			     &iter,
			     ARCHIVES_EXPORT_SELECT_COLUMN, &value,
			     ARCHIVES_EXPORT_NUMBER, &archive_number,
			     -1 );
	if (value)
	    return archive_number;
    }
    while (gtk_tree_model_iter_next ( GTK_TREE_MODEL (model), &iter));
    return 0;
}


