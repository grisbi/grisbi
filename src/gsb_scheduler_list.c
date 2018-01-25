/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*      2009 Thomas Peel (thomas.peel@live.fr)                                */
/*          2008-2013 Pierre Biava (grisbi@pierre.biava.name)                 */
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
 * \file gsb_scheduler_list.c
 * functions for the scheduled list
 */



#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_scheduler_list.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_calendar.h"
#include "gsb_data_account.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_real.h"
#include "gsb_rgba.h"
#include "gsb_scheduler.h"
#include "gsb_transactions_list.h"
#include "menu.h"
#include "mouse.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *creation_barre_outils_echeancier ( void );
static gboolean gsb_scheduler_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev );
static void gsb_scheduler_list_create_list_columns ( GtkWidget *tree_view );
static GtkTreeModel *gsb_scheduler_list_create_model ( void );
static GtkWidget *gsb_scheduler_list_create_tree_view (void);
static gint gsb_scheduler_list_default_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gpointer null );
static gboolean gsb_scheduler_list_edit_transaction_by_pointer ( gint *scheduled_number );
static gboolean gsb_scheduler_list_fill_transaction_row ( GtkTreeStore *store,
                        GtkTreeIter *iter,
                        const gchar *line[SCHEDULER_COL_VISIBLE_COLUMNS] );
static gboolean gsb_scheduler_list_fill_transaction_text ( gint scheduled_number,
						    const gchar *line[SCHEDULER_COL_VISIBLE_COLUMNS]  );
static GtkTreeIter *gsb_scheduler_list_get_iter_from_scheduled_number ( gint scheduled_number );
static GSList *gsb_scheduler_list_get_iter_list_from_scheduled_number ( gint scheduled_number );
static GtkTreeModel *gsb_scheduler_list_get_model ( void );
static gboolean gsb_scheduler_list_key_press ( GtkWidget *tree_view,
                        GdkEventKey *ev );
static gboolean gsb_scheduler_list_popup_custom_periodicity_dialog (void);
static void gsb_scheduler_list_process_orphan_list ( GSList *orphan_scheduled,
                        GDate *end_date );
static gboolean gsb_scheduler_list_selection_changed ( GtkTreeSelection *selection,
                        gpointer null );
static gboolean gsb_scheduler_list_set_color_of_mother ( gint mother_scheduled_number,
                        gboolean is_red );
static void gsb_scheduler_list_set_model ( GtkTreeModel *model );
static void gsb_scheduler_list_set_sorted_model ( GtkTreeModelSort *tree_model_sort );
static void gsb_scheduler_list_set_tree_view ( GtkWidget *tree_view );
static gboolean gsb_scheduler_list_show_notes ( GtkWidget *item );
static gboolean gsb_scheduler_list_size_allocate ( GtkWidget *tree_view,
                        GtkAllocation *allocation,
                        gpointer null );
static gboolean gsb_scheduler_list_sort_column_clicked ( GtkTreeViewColumn *tree_view_column,
                        gint *column_ptr );
static gint gsb_scheduler_list_sort_function_by_account ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gint *column_ptr );
static gint gsb_scheduler_list_sort_function_by_payee ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gint *column_ptr );
static gboolean gsb_scheduler_list_switch_expander ( gint scheduled_number );
static gboolean gsb_scheduler_list_update_white_child ( gint white_line_number,
                        gint mother_scheduled_number );
static void popup_scheduled_context_menu ( void );

/*END_STATIC*/


/*START_EXTERN*/
extern struct ConditionalMessage delete_msg[];
/*END_EXTERN*/


/** set the tree view and models as static, we can access to them
 * by the functions gsb_scheduler_list_get_tree_view...
 * don't call them directly */
static GtkWidget *tree_view_scheduler_list;
static GtkTreeModel *tree_model_scheduler_list;
static GtkTreeModelSort *tree_model_sort_scheduler_list;

static GtkSortType sort_type;

static GtkTreeViewColumn *scheduler_list_column[SCHEDULER_COL_VISIBLE_COLUMNS];

static gint last_scheduled_number;

/** lists of number of scheduled transactions taken or to be taken */
GSList *scheduled_transactions_to_take;
GSList *scheduled_transactions_taken;

/** used to save and restore the width of the scheduled list */
gint scheduler_col_width[SCHEDULER_COL_VISIBLE_COLUMNS];
gint scheduler_current_tree_view_width = 0;

/* toolbar */
static GtkWidget *scheduler_toolbar;
/** Used to display/hide comments in scheduler list */
static GtkWidget *scheduler_display_hide_notes = NULL;
/** here are the 3 buttons on the scheduler toolbar
 * which can been unsensitive or sensitive */
static GtkWidget *scheduler_button_execute = NULL;
static GtkWidget *scheduler_button_delete = NULL;
static GtkWidget *scheduler_button_edit = NULL;

/* popup view menu */
static gchar *periodicity_names[] = { N_("Unique view"), N_("Week view"), N_("Month view"),
            N_("Two months view"), N_("Quarter view"),
            N_("Year view"), N_("Custom view"), NULL, };

static gboolean view_menu_block_cb = FALSE;

static gchar *j_m_a_names[] = { N_("days"), N_("weeks"), N_("months"), N_("years"), NULL };


/**
 * called from the toolbar to change the scheduler view
 *
 * \param periodicity 	the new view wanted
 * \param item		not used
 *
 * \return FALSE
 * */
static void gsb_scheduler_list_change_scheduler_view ( GtkWidget *item,
                        gpointer pointer_periodicity )
{
    gchar *tmpstr;
    gint periodicity;

    periodicity = GPOINTER_TO_INT ( pointer_periodicity );
    if ( view_menu_block_cb || periodicity == etat.affichage_echeances )
        return;

    if ( periodicity == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
    {
        if ( !gsb_scheduler_list_popup_custom_periodicity_dialog () )
            return;
    }

    tmpstr = g_strconcat ( _("Scheduled transactions"), " : ",
                        g_dgettext ( NULL, periodicity_names[periodicity] ),
                        NULL );
    grisbi_win_headings_update_title ( tmpstr );
    grisbi_win_headings_update_suffix ( "" );
    g_free ( tmpstr );

    etat.affichage_echeances = periodicity;
    gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
    gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());
    gsb_scheduler_list_select (-1);

    gsb_file_set_modified ( TRUE );

    return;
}


/**
 * affichage du popup view menu
 *
 * \param button
 * \param event
 * \param user data
 *
 * \return
 */
static gboolean popup_scheduled_view_mode_menu ( GtkWidget *button,
                        GdkEvent  *event,
                        gpointer   user_data )
{
    GtkWidget *menu, *item;
    GSList *group = NULL;
    gint nbre_echeances;
    gint i;

    view_menu_block_cb = TRUE;
    menu = gtk_menu_new ();

    /* on enlève la ligne blanche */
    nbre_echeances = gtk_tree_model_iter_n_children ( tree_model_scheduler_list, NULL ) - 1;

    for ( i = 0 ; periodicity_names[i] ; i++ )
    {
        gchar *tmp_str;

        if ( i == etat.affichage_echeances )
        {
            if ( i == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
            {
                tmp_str = g_strdup_printf ("%s (%d - %d %s)",
                                g_dgettext ( NULL, periodicity_names[i] ),
                                nbre_echeances,
                                etat.affichage_echeances_perso_nb_libre,
                                g_dgettext ( NULL,
                                j_m_a_names[etat.affichage_echeances_perso_j_m_a] ) );
            }
            else
            {
                tmp_str = g_strdup_printf ("%s (%d)",
                                g_dgettext ( NULL, periodicity_names[i] ),
                                nbre_echeances );
            }
            item = gtk_radio_menu_item_new_with_label ( group, tmp_str );

            g_free ( tmp_str );
        }
        else
            item = gtk_radio_menu_item_new_with_label ( group,
                        g_dgettext ( NULL, periodicity_names[i] ) );

        group = gtk_radio_menu_item_get_group ( GTK_RADIO_MENU_ITEM ( item ) );
        if ( i == etat.affichage_echeances )
            gtk_check_menu_item_set_active ( GTK_CHECK_MENU_ITEM ( item ), TRUE );

        g_signal_connect ( G_OBJECT ( item ),
                        "toggled",
                        G_CALLBACK ( gsb_scheduler_list_change_scheduler_view ),
                        GINT_TO_POINTER ( i ) );

        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    }

    gtk_widget_show_all ( menu );
#if GTK_CHECK_VERSION (3,22,0)
	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
#else
    gtk_menu_popup ( GTK_MENU ( menu ), NULL, button, set_popup_position, button, 1,
                gtk_get_current_event_time () );
#endif
    view_menu_block_cb = FALSE;

    return TRUE;
}


/**
 *
 */
void gsb_scheduler_list_init_variables ( void )
{
    if ( scheduled_transactions_to_take )
    {
        g_slist_free ( scheduled_transactions_to_take );
        scheduled_transactions_to_take = NULL;
    }
    if ( scheduled_transactions_taken )
    {
        g_slist_free ( scheduled_transactions_taken );
        scheduled_transactions_taken = NULL;
    }

    /* on réinitialise tous les widgets */
    scheduler_display_hide_notes = NULL;
    scheduler_button_execute = NULL;
    scheduler_button_delete = NULL;
    scheduler_button_edit = NULL;
}

/**
 * create the scheduler list
 *
 * \param
 *
 * \return a vbox widget containing the list
 * */
GtkWidget *gsb_scheduler_list_create_list ( void )
{
    GtkWidget *vbox, *scrolled_window;
    GtkWidget *tree_view;
    GtkWidget *frame;
	GtkTreeModel *tree_model;

    devel_debug (NULL);

    /* first, a vbox */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );

    /* frame pour la barre d'outils */
    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame, FALSE, FALSE, 0 );

    /* création de la barre d'outils */
    scheduler_toolbar = creation_barre_outils_echeancier ();
    gtk_container_add ( GTK_CONTAINER ( frame ), scheduler_toolbar );

    /* create the scrolled window */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ), scrolled_window, TRUE, TRUE, 0 );
    gtk_widget_show ( scrolled_window );

    /* we create and set the tree_view in the page */
    tree_view = gsb_scheduler_list_create_tree_view ();
    gsb_scheduler_list_set_tree_view (tree_view);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );

    /* set the color of selected row */
	gtk_widget_set_name (tree_view, "tree_view");

    /* create the store and set it in the tree_view */
    tree_model = gsb_scheduler_list_create_model ();
    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view), tree_model);
    g_object_unref (G_OBJECT(tree_model));

    /* create the columns */
    gsb_scheduler_list_create_list_columns (tree_view);

    /* begin by hiding the notes */
    gsb_scheduler_list_show_notes ( scheduler_display_hide_notes );

    g_signal_connect ( G_OBJECT ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) ),
                        "changed",
                        G_CALLBACK ( gsb_scheduler_list_selection_changed ),
                        NULL );

    gtk_widget_show_all ( vbox );

    return vbox;
}


/**
 * Create the toolbar that contains all elements needed to manipulate
 * the scheduler.
 *
 * \param
 *
 * \return A newly created hbox.
 */
GtkWidget *creation_barre_outils_echeancier ( void )
{
    GtkWidget *toolbar;
    GtkToolItem *item;

    toolbar = gtk_toolbar_new ();

    /* new scheduled button */
    item = utils_buttons_tool_button_new_from_image_label ( "gsb-new-scheduled-24.png", _("New scheduled") );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ),
                        _("Prepare form to create a new scheduled transaction") );
    g_signal_connect_swapped ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( gsb_scheduler_list_edit_transaction ),
                        GINT_TO_POINTER ( -1 ) );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    /* delete button */
    scheduler_button_delete = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-delete-24.png", _("Delete")));
    gtk_widget_set_sensitive ( scheduler_button_delete, FALSE );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( scheduler_button_delete ),
                        _("Delete selected scheduled transaction"));
    g_signal_connect ( G_OBJECT ( scheduler_button_delete ),
                        "clicked",
                        G_CALLBACK ( gsb_scheduler_list_delete_scheduled_transaction_by_menu ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), GTK_TOOL_ITEM ( scheduler_button_delete ), -1 );

    /* edit button */
    scheduler_button_edit = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-edit-24.png", _("Edit")));
    gtk_widget_set_sensitive ( scheduler_button_edit, FALSE );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( scheduler_button_edit ),
                        _("Edit selected transaction"));
    g_signal_connect_swapped ( G_OBJECT ( scheduler_button_edit ),
                        "clicked",
                        G_CALLBACK ( gsb_scheduler_list_edit_transaction ),
                        0 );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), GTK_TOOL_ITEM ( scheduler_button_edit ), -1 );

    /* Display/hide comments */
    scheduler_display_hide_notes = GTK_WIDGET ( utils_buttons_tool_button_new_from_image_label ("gsb-comments-24.png", _("Notes") ) );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( scheduler_display_hide_notes ),
                        _("Display the notes of scheduled transactions") );
    g_signal_connect ( G_OBJECT ( scheduler_display_hide_notes ),
                        "clicked",
                        G_CALLBACK ( gsb_scheduler_list_show_notes ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), GTK_TOOL_ITEM ( scheduler_display_hide_notes ), -1 );

    /* Execute transaction */
    scheduler_button_execute = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-execute-24.png", _("Execute")));
    gtk_widget_set_sensitive ( scheduler_button_execute, FALSE );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( scheduler_button_execute ),
                        _("Execute current scheduled transaction"));
    g_signal_connect_swapped ( G_OBJECT ( scheduler_button_execute ),
                        "clicked",
                        G_CALLBACK ( gsb_scheduler_list_execute_transaction ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), GTK_TOOL_ITEM ( scheduler_button_execute ), -1 );

	/* View button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-select-color-24.png", _("View"));
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ),
                        _("Change display mode of scheduled transaction list"));
    g_signal_connect ( G_OBJECT ( item ),
                        "clicked",
                        G_CALLBACK ( popup_scheduled_view_mode_menu ),
                        NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );

    return toolbar;
}


/**
 *
 *
 *
 */
void gsb_gui_scheduler_toolbar_set_style ( gint toolbar_style )
{
    gtk_toolbar_set_style ( GTK_TOOLBAR ( scheduler_toolbar ), toolbar_style );
}


/**
 * return the scheduler tree view
 *
 * \param
 *
 * \return the scheduler tree_view
 * */
GtkWidget *gsb_scheduler_list_get_tree_view ( void )
{
    return tree_view_scheduler_list;
}

/**
 * set the scheduler tree view
 *
 * \param tree_view
 *
 * \return
 * */
void gsb_scheduler_list_set_tree_view ( GtkWidget *tree_view )
{
    tree_view_scheduler_list = tree_view;
}


/**
 * return the scheduler tree_model
 *
 * \param
 *
 * \return the scheduler tree_model
 * */
GtkTreeModel *gsb_scheduler_list_get_model ( void )
{
    return tree_model_scheduler_list;
}

/**
 * set the scheduler tree_model
 *
 * \param model
 *
 * \return
 * */
void gsb_scheduler_list_set_model ( GtkTreeModel *model )
{
    tree_model_scheduler_list = model;
}




/**
 * set the scheduler tree  model sort
 *
 * \param tree_model_sort
 *
 * \return
 * */
void gsb_scheduler_list_set_sorted_model ( GtkTreeModelSort *tree_model_sort )
{
    tree_model_sort_scheduler_list = tree_model_sort;
}

/**
 * return the content of the last_scheduled_number variable
 * that variable is filled when changing the selection of a scheduled transaction
 *
 * \param
 *
 * \return a gint
 * */
gint gsb_scheduler_list_get_last_scheduled_number ( void )
{
    return last_scheduled_number;
}

/**
 * create and configure the tree view of the scheduled transactions
 *
 * \param
 *
 * \return the tree_view
 * */
GtkWidget *gsb_scheduler_list_create_tree_view (void)
{
    GtkWidget * tree_view;

    tree_view = gtk_tree_view_new ( );
	gtk_widget_set_name (tree_view, "tree_view");

    /* can select only one line */
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW( tree_view ) ) ),
                        GTK_SELECTION_SINGLE );

    g_signal_connect ( G_OBJECT ( tree_view ),
                        "size_allocate",
                        G_CALLBACK (gsb_scheduler_list_size_allocate),
                        NULL );
    g_signal_connect ( G_OBJECT ( tree_view ),
                        "button-press-event",
                        G_CALLBACK ( gsb_scheduler_list_button_press ),
                        NULL );

    g_signal_connect ( G_OBJECT ( tree_view ),
                        "key-press-event",
                        G_CALLBACK ( gsb_scheduler_list_key_press ),
                        NULL );

    gtk_widget_show ( tree_view );

    last_scheduled_number = -1;

    return tree_view;
}


/**
 * create and append the columns of the tree_view
 *
 * \param tree_view the tree_view
 *
 * \return
 * */
void gsb_scheduler_list_create_list_columns ( GtkWidget *tree_view )
{
    gint i;
    gchar *scheduler_titles[] = {
	_("Date"), _("Account"), _("Payee"), _("Frequency"),
	_("Mode"), _("Notes"), _("Amount")
    };
    gfloat col_justs[] = {
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_LEFT, COLUMN_CENTER,
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_RIGHT, COLUMN_RIGHT
    };

    devel_debug (NULL);

    for ( i = 0 ; i < SCHEDULER_COL_VISIBLE_COLUMNS ; i++ )
    {
        GtkCellRenderer *cell_renderer;

        cell_renderer = gtk_cell_renderer_text_new ();

        g_object_set ( G_OBJECT (GTK_CELL_RENDERER ( cell_renderer )),
                        "xalign",
                        col_justs[i], NULL );
        switch ( i )
        {
            case 0:
                scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
									      cell_renderer,
                                            "text", i,
                                            "cell-background-rgba", SCHEDULER_COL_NB_BACKGROUND,
                                            "font-desc", SCHEDULER_COL_NB_FONT,
                                            "foreground", SCHEDULER_COL_NB_TEXT_COLOR,
									      NULL );
                gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( tree_model_sort_scheduler_list ),
                                            i,
                                            (GtkTreeIterCompareFunc) gsb_scheduler_list_default_sort_function,
                                            NULL,
                                            NULL );
                gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ), TRUE );

                /* use the click to sort the list */
                g_signal_connect ( G_OBJECT ( scheduler_list_column[i] ),
                                            "clicked",
                                            G_CALLBACK ( gsb_scheduler_list_sort_column_clicked ),
                                            GINT_TO_POINTER ( i ) );
                break;
            case 1:
                scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
                                            cell_renderer,
                                            "text", i,
                                            "cell-background-rgba", SCHEDULER_COL_NB_BACKGROUND,
                                            "font-desc", SCHEDULER_COL_NB_FONT,
                                            "foreground", SCHEDULER_COL_NB_TEXT_COLOR,
                                            NULL );
                gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( tree_model_sort_scheduler_list ),
                                            i,
                                            (GtkTreeIterCompareFunc) gsb_scheduler_list_sort_function_by_account,
                                            NULL,
                                            NULL );
                gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ), TRUE );

                /* use the click to sort the list */
                g_signal_connect ( G_OBJECT ( scheduler_list_column[i] ),
                                            "clicked",
                                            G_CALLBACK ( gsb_scheduler_list_sort_column_clicked ),
                                            GINT_TO_POINTER ( i ) );
                break;
            case 2:
                scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
                                            cell_renderer,
                                            "text", i,
                                            "cell-background-rgba", SCHEDULER_COL_NB_BACKGROUND,
                                            "font-desc", SCHEDULER_COL_NB_FONT,
                                            "foreground", SCHEDULER_COL_NB_TEXT_COLOR,
                                            NULL );
                gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( tree_model_sort_scheduler_list ),
                                            i,
                                            (GtkTreeIterCompareFunc) gsb_scheduler_list_sort_function_by_payee,
                                            NULL,
                                            NULL );
                gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ), TRUE );

                /* use the click to sort the list */
                g_signal_connect ( G_OBJECT ( scheduler_list_column[i] ),
                                            "clicked",
                                            G_CALLBACK ( gsb_scheduler_list_sort_column_clicked ),
                                            GINT_TO_POINTER ( i ) );
                break;
            case 3:
                scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
                                            cell_renderer,
                                            "text", i,
                                            "cell-background-rgba", SCHEDULER_COL_NB_BACKGROUND,
                                            "font-desc", SCHEDULER_COL_NB_FONT,
                                            "foreground", SCHEDULER_COL_NB_TEXT_COLOR,
                                            NULL );
                break;
            case 4:
                scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
                                            cell_renderer,
                                            "text", i,
                                            "cell-background-rgba", SCHEDULER_COL_NB_BACKGROUND,
                                            "font-desc", SCHEDULER_COL_NB_FONT,
                                            "foreground", SCHEDULER_COL_NB_TEXT_COLOR,
                                            NULL );
                break;
            case 5:
                scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
                                            cell_renderer,
                                            "text", i,
                                            "cell-background-rgba", SCHEDULER_COL_NB_BACKGROUND,
                                            "font-desc", SCHEDULER_COL_NB_FONT,
                                            "foreground", SCHEDULER_COL_NB_TEXT_COLOR,
                                            NULL );
                break;
            case 6:
                scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
                                            cell_renderer,
                                            "text", i,
                                            "cell-background-rgba", SCHEDULER_COL_NB_BACKGROUND,
                                            "foreground", SCHEDULER_COL_NB_AMOUNT_COLOR,
                                            "font-desc", SCHEDULER_COL_NB_FONT,
                                            NULL );
                break;
        }

        gtk_tree_view_column_set_alignment ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ), col_justs[i] );

        gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                          GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ));

        /* automatic and resizeable sizing */
        gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ),
                          GTK_TREE_VIEW_COLUMN_FIXED );
        gtk_tree_view_column_set_resizable ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ), TRUE );
    }
}


/**
 * create and return the tree store of the scheduler list
 *
 * \param
 *
 * \return a gtk_tree_store
 * */
GtkTreeModel *gsb_scheduler_list_create_model ( void )
{
    GtkTreeStore *store;
    GtkTreeModel *sortable;

    devel_debug (NULL);

    store = gtk_tree_store_new ( SCHEDULER_COL_NB_TOTAL,
				 G_TYPE_STRING,                 /* COL_NB_DATE */
				 G_TYPE_STRING,                 /* COL_NB_ACCOUNT */
				 G_TYPE_STRING,                 /* COL_NB_PARTY */
				 G_TYPE_STRING,                 /* COL_NB_FREQUENCY */
				 G_TYPE_STRING,                 /* COL_NB_MODE */
				 G_TYPE_STRING,                 /* COL_NB_NOTES */
				 G_TYPE_STRING,                 /* COL_NB_AMOUNT */
				 GDK_TYPE_RGBA,                /* SCHEDULER_COL_NB_BACKGROUND */
				 GDK_TYPE_RGBA,                /* SCHEDULER_COL_NB_SAVE_BACKGROUND */
				 G_TYPE_STRING,                 /* SCHEDULER_COL_NB_AMOUNT_COLOR */
				 G_TYPE_INT,                    /* SCHEDULER_COL_NB_TRANSACTION_NUMBER */
				 PANGO_TYPE_FONT_DESCRIPTION,   /* SCHEDULER_COL_NB_FONT */
				 G_TYPE_INT,                    /* SCHEDULER_COL_NB_VIRTUAL_TRANSACTION */
				 G_TYPE_STRING );               /* SCHEDULER_COL_NB_TEXT_COLOR */

    gsb_scheduler_list_set_model (GTK_TREE_MODEL (store));
    sortable = gtk_tree_model_sort_new_with_model ( GTK_TREE_MODEL (store));
    gsb_scheduler_list_set_sorted_model (GTK_TREE_MODEL_SORT (sortable));
    gtk_tree_sortable_set_sort_column_id (  GTK_TREE_SORTABLE (
                        tree_model_sort_scheduler_list ),
                        0, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_default_sort_func ( GTK_TREE_SORTABLE (sortable),
					      (GtkTreeIterCompareFunc) gsb_scheduler_list_default_sort_function,
					      NULL,
					      NULL );

    return sortable;
}


/**
 * default sorting function for scheduler list :
 * sort by date, the column number 0
 * perhaps later sort by different columns ??
 * */
gint gsb_scheduler_list_default_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gpointer null )
{
    GDate *date_1;
    GDate *date_2;
    gchar *date_str;
    gint number_1;
    gint number_2;
    gint return_value = 0;

    /* first, we sort by date (col 0) */
    gtk_tree_model_get ( model,
                        iter_1,
                        COL_NB_DATE, &date_str,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_1,
                        -1 );
    date_1 = gsb_parse_date_string ( date_str );
    g_free ( date_str );

    gtk_tree_model_get ( model,
                        iter_2,
                        COL_NB_DATE, &date_str,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_2,
                        -1 );
    date_2 = gsb_parse_date_string ( date_str );
    g_free ( date_str );

    if ( number_1 == -1 )
    {
        if ( date_1) g_date_free ( date_1);
        if ( date_2) g_date_free ( date_2);
        if ( sort_type == GTK_SORT_ASCENDING )
            return 1;
        else
            return -1;
    }
    else if ( number_2 == -1 )
    {
        if ( date_1) g_date_free ( date_1);
        if ( date_2) g_date_free ( date_2);
        if ( sort_type == GTK_SORT_ASCENDING )
            return -1;
        else
            return 1;
    }

    if ( date_1 &&  date_2 )
        return_value = g_date_compare ( date_1, date_2 );

    if ( return_value )
    {
        if ( date_1) g_date_free ( date_1);
        if ( date_2) g_date_free ( date_2);

        return return_value;
    }
    /* if we are here it's because we are in a child of split */
    if ( number_1 < 0 )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            return 1;
        else
            return -1;
    }
    if ( number_2 < 0 )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            return -1;
        else
            return 1;
    }

    if (! return_value )
        return_value = number_1 - number_2;

    if ( date_1) g_date_free ( date_1);
    if ( date_2) g_date_free ( date_2);

    return return_value;
}


/**
 * change the showed information on the list :
 * either show the frequency and mode of the scheduled
 * either show the notes
 *
 * \param
 *
 * \return FALSE
 */
gboolean gsb_scheduler_list_show_notes ( GtkWidget *item )
{
    if ( scheduler_display_hide_notes )
    {
        if ( conf.display_toolbar != GSB_BUTTON_ICON )
        {
            if ( etat.affichage_commentaire_echeancier )
                gtk_tool_button_set_label ( GTK_TOOL_BUTTON ( item ), _("Frequency/Mode") );
            else
                gtk_tool_button_set_label ( GTK_TOOL_BUTTON ( item ), _("Notes") );
        }

        if ( etat.affichage_commentaire_echeancier )
            gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ),
                _("Display the frequency and mode of scheduled transactions") );
        else
            gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ),
                        _("Display the notes of scheduled transactions") );
    }

    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[COL_NB_FREQUENCY] ),
				       !etat.affichage_commentaire_echeancier );
    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[COL_NB_MODE] ),
				       !etat.affichage_commentaire_echeancier );
    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[COL_NB_NOTES] ),
				       etat.affichage_commentaire_echeancier );

    etat.affichage_commentaire_echeancier = !etat.affichage_commentaire_echeancier;

    return FALSE;
}



/**
 * called to execute a sheduled transaction, either by the button on the toolbar,
 * either by click on the first page
 * if scheduled_number is 0, get the selected transaction
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_execute_transaction ( gint scheduled_number )
{
    devel_debug_int (scheduled_number);

    if ( !scheduled_number )
	scheduled_number = gsb_scheduler_list_get_current_scheduled_number ();

    gsb_scheduler_list_edit_transaction (scheduled_number);

    /* the only difference for now between an execution and a edition of scheduled is here :
     * set the flag to say that we execute the scheduled transaction
     * and hide the scheduler part of the form */
    g_object_set_data ( G_OBJECT (gsb_form_get_form_widget ()),
			"execute_scheduled", GINT_TO_POINTER (TRUE));
    gtk_widget_hide ( gsb_form_get_scheduler_part ());

    return FALSE;
}



/**
 * fill the scheduled transactions list
 *
 * \para tree_view
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_fill_list ( GtkWidget *tree_view )
{
    GSList *tmp_list;
    GDate *end_date;
    GtkTreeIter iter;
    GSList *orphan_scheduled = NULL;

    devel_debug (NULL);

    /* get the last date we want to see the transactions */
    end_date = gsb_scheduler_list_get_end_date_scheduled_showed ( );

    gtk_tree_store_clear ( GTK_TREE_STORE ( tree_model_scheduler_list ) );

    /* fill the list */
    tmp_list = gsb_data_scheduled_get_scheduled_list ( );

    while ( tmp_list )
    {
        gint scheduled_number;

        scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list -> data );

        if ( !end_date
         ||
         g_date_compare ( gsb_data_scheduled_get_date ( scheduled_number ), end_date) <= 0 )
        {
            if ( !gsb_scheduler_list_append_new_scheduled ( scheduled_number, end_date ) )
                /* the scheduled transaction was not added, add to orphan scheduledlist */
                orphan_scheduled = g_slist_append ( orphan_scheduled, tmp_list -> data );
        }
        tmp_list = tmp_list -> next;
    }

    /* if there are some orphan sheduler (children of breakdonw which didn't find their mother */
    if ( orphan_scheduled )
    {
        gsb_scheduler_list_process_orphan_list ( orphan_scheduled, end_date );
        g_slist_free ( orphan_scheduled );
    }

    /* create and append the white line */
    gtk_tree_store_append ( GTK_TREE_STORE ( tree_model_scheduler_list ), &iter, NULL );
    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model_scheduler_list ),
                        &iter,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, gsb_data_scheduled_new_white_line ( 0 ),
                        -1 );

    return FALSE;
}


/**
 * send a "row-changed" to all the row of the showed transactions,
 * so in fact re-draw the list and colors
 *
 * \param
 *
 * \return
 * */
gboolean gsb_scheduler_list_redraw ( void )
{
    GtkTreeIter iter;
    GtkTreeModel *tree_model;

    tree_model = gsb_scheduler_list_get_model ();

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (tree_model), &iter))
    {
	do
	{
	    GtkTreePath *path;
	    GtkTreeIter child_iter;

	    path = gtk_tree_model_get_path ( GTK_TREE_MODEL (tree_model),
					     &iter );
	    gtk_tree_model_row_changed ( GTK_TREE_MODEL (tree_model), path, &iter);
	    gtk_tree_path_free(path);

	    /* update the children if necessary */
	    if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (tree_model),
					       &child_iter,
					       &iter ))
	    {
		do
		{
		    path = gtk_tree_model_get_path ( GTK_TREE_MODEL (tree_model),
						     &child_iter );
		    gtk_tree_model_row_changed ( GTK_TREE_MODEL (tree_model), path, &child_iter);
		    gtk_tree_path_free(path);
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (tree_model), &child_iter));
	    }
	}
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (tree_model), &iter));
    }
    return FALSE;
}


/**
 * append the scheduled transaction to the tree_view given in param
 * if that transaction need to be appended several times (untill end_date),
 * it's done here
 *
 * \param scheduled_number
 * \param end_date
 *
 * \return TRUE : scheduled added, FALSE : not added (usually for children who didn't find their mother)
 * */
gboolean gsb_scheduler_list_append_new_scheduled ( gint scheduled_number,
                        GDate *end_date )
{
	GDate *tmp_date;
    GDate *pGDateCurrent;
    gint virtual_transaction = 0;
    GtkTreeIter *mother_iter = NULL;
    const gchar *line[SCHEDULER_COL_VISIBLE_COLUMNS];
    gint mother_scheduled_number;
	gint fixed_date = 0;

    /* devel_debug_int (scheduled_number); */
    if ( !tree_model_scheduler_list )
        return FALSE;

    /* get the mother iter if needed */
    mother_scheduled_number = gsb_data_scheduled_get_mother_scheduled_number ( scheduled_number );
    if ( mother_scheduled_number )
    {
        gint white_line_number;

        mother_iter = gsb_scheduler_list_get_iter_from_scheduled_number ( mother_scheduled_number );
        if ( !mother_iter )
            /* it's a child but didn't find the mother, it can happen in old files previous to 0.6
             * where the children wer saved before the mother, return FALSE here will add that
             * child to a list to append it again later */
            return FALSE;

        white_line_number = gsb_data_scheduled_get_white_line ( mother_scheduled_number );
        gsb_scheduler_list_update_white_child ( white_line_number, mother_scheduled_number );
    }

	/* set the good date */
	tmp_date = gsb_data_scheduled_get_date (scheduled_number);
	fixed_date = gsb_data_scheduled_get_fixed_date (scheduled_number);
	if (fixed_date)
			pGDateCurrent = gsb_date_copy (gsb_date_get_last_day_of_month (tmp_date));
	else
			pGDateCurrent = gsb_date_copy (tmp_date);

    /* fill the text line */
    gsb_scheduler_list_fill_transaction_text ( scheduled_number, line );

    do
    {
        GtkTreeIter iter;

        gtk_tree_store_append ( GTK_TREE_STORE ( tree_model_scheduler_list ), &iter, mother_iter );

        gsb_scheduler_list_fill_transaction_row ( GTK_TREE_STORE ( tree_model_scheduler_list ), &iter, line );

        /* set the number of scheduled transaction to 0 if it's not the first one
         * (when more than one showed ) */
        gtk_tree_store_set ( GTK_TREE_STORE ( tree_model_scheduler_list ),
                        &iter,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, scheduled_number,
                        SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, virtual_transaction,
                        -1 );

        /* if it's a split, we append a white line now */
        if (gsb_data_scheduled_get_split_of_scheduled ( scheduled_number ) && !virtual_transaction )
        {
            gint white_line_number;

            white_line_number = gsb_data_scheduled_get_white_line ( scheduled_number );
            if ( white_line_number == -1 )
                white_line_number = gsb_data_scheduled_new_white_line ( scheduled_number );

            gsb_scheduler_list_append_new_scheduled ( white_line_number, end_date );
            gsb_scheduler_list_update_white_child ( white_line_number, scheduled_number );
        }

        /* if it's a split, we show only one time and color the background */
        if ( mother_iter )
        {
            gtk_tree_store_set ( GTK_TREE_STORE ( tree_model_scheduler_list ),
                        &iter,
                        SCHEDULER_COL_NB_BACKGROUND, gsb_rgba_get_couleur ( "split_background" ),
                        -1 );
        }
        else
        {
            pGDateCurrent = gsb_scheduler_get_next_date ( scheduled_number, pGDateCurrent );

            line[COL_NB_DATE] = gsb_format_gdate ( pGDateCurrent );

            /* now, it's not real transactions */
            virtual_transaction ++;
        }
    }
    while ( pGDateCurrent &&
	    end_date &&
	    g_date_compare ( end_date, pGDateCurrent ) > 0 &&
	    !mother_iter );

    if ( mother_iter )
        gtk_tree_iter_free ( mother_iter );

    return TRUE;
}


/**
 * remove the given scheduled transaction from the list
 * and too all the corresponding virtual transactions
 *
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_remove_transaction_from_list ( gint scheduled_number )
{
    GSList *iter_list;

    devel_debug_int (scheduled_number);

    if (!scheduled_number
	||
	!gsb_scheduler_list_get_model ())
	return FALSE;

    /* at this level, normally the transaction is already deleted,
     * so we cannot call gsb_data_scheduled_... we have only the number
     * to remove it from the list */

    iter_list = gsb_scheduler_list_get_iter_list_from_scheduled_number ( scheduled_number );

    if ( iter_list )
    {
	GtkTreeStore *store;
	GSList *tmp_list;

	store = GTK_TREE_STORE (gsb_scheduler_list_get_model ());
	tmp_list = iter_list;

	while (tmp_list)
	{
	    GtkTreeIter *iter;

	    iter = tmp_list -> data;

	    gtk_tree_store_remove ( store,
				    iter );
	    gtk_tree_iter_free ( iter );
	    tmp_list = tmp_list -> next;
	}
    }
    else
    {
        gchar *tmpstr;

        tmpstr = g_strdup_printf ( _("in gsb_scheduler_list_remove_transaction_from_list, "
                            "ask to remove the transaction no %d,\nbut didn't find the iter in the list...\n"
                            "It's normal if appending a new scheduled transaction, but abnormal else..."),
                            scheduled_number );
        warning_debug ( tmpstr);
        g_free ( tmpstr );
    }
    return FALSE;
}


/**
 * update the scheduled transaction in the list (and all the virtuals too)
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_update_transaction_in_list ( gint scheduled_number )
{
    GtkTreeStore *store;
    GtkTreeIter iter;
    GDate *pGDateCurrent;

    /* TODO dOm : each line of the array `line' contains a newly allocated string. When are they freed ? */
    const gchar *line[SCHEDULER_COL_VISIBLE_COLUMNS];

    devel_debug_int (scheduled_number);

    if ( !scheduled_number || !gsb_scheduler_list_get_model ( ) )
        return FALSE;

    /* the same transaction can be showed more than one time because of the different views,
     * not so difficult, go throw the list and for each iter corresponding to the scheduled
     * transaction, re-fill the line */
    store = GTK_TREE_STORE ( gsb_scheduler_list_get_model ( ) );

    pGDateCurrent = gsb_date_copy ( gsb_data_scheduled_get_date ( scheduled_number ) );

    /* fill the text line */
    gsb_scheduler_list_fill_transaction_text ( scheduled_number, line );

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( store ), &iter ) )
    {
        do
        {
            gint scheduled_number_tmp;
            GtkTreeIter child_iter;

            gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
                                &iter,
                                SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number_tmp,
                                -1 );
            if ( scheduled_number_tmp == scheduled_number )
            {
                gsb_scheduler_list_fill_transaction_row ( GTK_TREE_STORE ( store ), &iter, line );

                /* go to the next date if ever there is several lines of that scheduled */
                pGDateCurrent = gsb_scheduler_get_next_date ( scheduled_number, pGDateCurrent );

                line[COL_NB_DATE] = gsb_format_gdate ( pGDateCurrent );
            }

            /* i still haven't found a function to go line by line, including the children,
             * so do another do/while into the first one */
            if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( store ), &child_iter, &iter ) )
            {
                gint white_line_number = 0;
                gint mother_number = 0;
                /* we are on the child */
                do
                {
                    gtk_tree_model_get ( GTK_TREE_MODEL (store),
                                        &child_iter,
                                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number_tmp,
                                        -1 );

                    if ( scheduled_number_tmp < -1 )
                    {
                        white_line_number = scheduled_number_tmp;
                        mother_number = gsb_data_scheduled_get_mother_scheduled_number ( white_line_number );
                    }

                    if (scheduled_number_tmp == scheduled_number)
                        gsb_scheduler_list_fill_transaction_row ( GTK_TREE_STORE (store),
                                        &child_iter,
                                        line );
                }
                while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( store ), &child_iter ) );

                gsb_scheduler_list_update_white_child ( white_line_number, mother_number );
            }
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( store ), &iter ) );
    }

    return FALSE;
}


/**
 * fill the char tab in the param with the transaction given in param
 *
 * \param scheduled_number
 * \param  line a tab of gchar with SCHEDULER_COL_VISIBLE_COLUMNS of size, which will contain the text of the line
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_fill_transaction_text ( gint scheduled_number,
						    const gchar *line[SCHEDULER_COL_VISIBLE_COLUMNS]  )
{
    if ( gsb_data_scheduled_get_mother_scheduled_number (scheduled_number))
    {
	/* for child split we set all to NULL except the party, we show the category instead */
	line[COL_NB_DATE] = NULL;
	line[COL_NB_FREQUENCY] = NULL;
	line[COL_NB_ACCOUNT] = NULL;
	line[COL_NB_MODE] = NULL;

	if ( gsb_data_scheduled_get_category_number ( scheduled_number ) )
	    line[COL_NB_PARTY] = gsb_data_category_get_name (
                        gsb_data_scheduled_get_category_number ( scheduled_number ),
                        gsb_data_scheduled_get_sub_category_number ( scheduled_number ),
                        NULL );
	else
	{
	    /* there is no category, it can be a transfer */
	    if ( gsb_data_scheduled_get_account_number_transfer ( scheduled_number ) >= 0
		 &&
		 scheduled_number > 0 )
	    {
            /* it's a transfer */
            if (gsb_data_scheduled_get_amount (scheduled_number).mantissa < 0)
                line[COL_NB_PARTY] = g_strdup_printf ( _("Transfer to %s"),
                        gsb_data_account_get_name (
                        gsb_data_scheduled_get_account_number_transfer ( scheduled_number ) ) );
            else
                line[COL_NB_PARTY] = g_strdup_printf ( _("Transfer from %s"),
                        gsb_data_account_get_name (
                        gsb_data_scheduled_get_account_number_transfer ( scheduled_number ) ) );
	    }
	    else
            /* it's not a transfer, so no category */
            line[COL_NB_PARTY] = NULL;
	}
    }
    else
    {
	/* fill her for normal scheduled transaction (not children) */
	gint frequency;

	line[COL_NB_DATE] = gsb_format_gdate (gsb_data_scheduled_get_date (scheduled_number));
	frequency = gsb_data_scheduled_get_frequency (scheduled_number);

	if ( frequency == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
	{
	    switch (gsb_data_scheduled_get_user_interval (scheduled_number))
	    {
		case PERIODICITY_DAYS:
		    line[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d days"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_WEEKS:
		    line[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d weeks"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_MONTHS:
		    line[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d months"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_YEARS:
		    line[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d years"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
	    }
	}
	else
	    if ( frequency < SCHEDULER_PERIODICITY_NB_CHOICES
		 &&
		 frequency >= 0 )
	    {
		gchar * names[] = { _("Once"), _("Weekly"), _("Monthly"),
		    _("Bimonthly"), _("Quarterly"), _("Yearly") };
		line[COL_NB_FREQUENCY] = names [frequency];
	    }

	line[COL_NB_ACCOUNT] = gsb_data_account_get_name (gsb_data_scheduled_get_account_number (scheduled_number));
	line[COL_NB_PARTY] = gsb_data_payee_get_name (gsb_data_scheduled_get_party_number (scheduled_number),
						       TRUE );
	if ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number))
	    line[COL_NB_MODE]=_("Automatic");
	else
	    line[COL_NB_MODE] = _("Manual");
    }

    /* that can be filled for mother and children of split */
    line[COL_NB_NOTES] = gsb_data_scheduled_get_notes ( scheduled_number );

    /* if it's a white line don't fill the amount
     * (in fact fill nothing, but normally all before was set to NULL,
     * there is only the amount, we want NULL and not 0) */
    if ( scheduled_number < 0 )
        line[COL_NB_AMOUNT] = NULL;
    else
        line[COL_NB_AMOUNT] = utils_real_get_string_with_currency (
                        gsb_data_scheduled_get_amount ( scheduled_number ),
                        gsb_data_scheduled_get_currency_number ( scheduled_number ),
                        TRUE );

    return FALSE;
}



/**
 * fill the row pointed by the iter with the content of
 * the char tab given in param
 *
 * \param store
 * \param iter
 * \param line a tab of gchar with SCHEDULER_COL_VISIBLE_COLUMNS of size, which is the text content of the line
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_fill_transaction_row ( GtkTreeStore *store,
                        GtkTreeIter *iter,
                        const gchar *line[SCHEDULER_COL_VISIBLE_COLUMNS] )
{
    gchar *color_str = NULL;
    gint i;

    if ( line[COL_NB_AMOUNT] && g_utf8_strchr ( line[COL_NB_AMOUNT], -1, '-' ) )
        color_str = "red";
    else
    {
        g_free ( color_str );
        color_str = NULL;
    }

    for ( i=0 ; i<SCHEDULER_COL_VISIBLE_COLUMNS ; i++ )
    {
        if ( i == 6 )
            gtk_tree_store_set ( store, iter, i, line[i], SCHEDULER_COL_NB_AMOUNT_COLOR, color_str, -1 );
        else
            gtk_tree_store_set ( store, iter, i, line[i], -1 );
    }

    return FALSE;
}


/**
 * set the background colors of the list
 * just for normal scheduled transactions, not for children of split
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_set_background_color ( GtkWidget *tree_view )
{
    GtkTreeStore *store;
    GtkTreeModel *sort_model;
    gint current_color;
    GtkTreePath *sorted_path;
    GtkTreePath *path;

    if (!tree_view)
        return FALSE;

    sort_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ));
    store = GTK_TREE_STORE (gtk_tree_model_sort_get_model (GTK_TREE_MODEL_SORT (sort_model)));

    current_color = 0;
    sorted_path = gtk_tree_path_new_first ();

    while ((path = gtk_tree_model_sort_convert_path_to_child_path ( GTK_TREE_MODEL_SORT (sort_model),
								    sorted_path )))
    {
        gint virtual_transaction;
        GtkTreeIter iter;

        gtk_tree_model_get_iter ( GTK_TREE_MODEL ( store ), &iter, path );

        gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
                        &iter,
                        SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_transaction,
                        -1 );

        if ( virtual_transaction )
            gtk_tree_store_set ( store,
                        &iter,
                        SCHEDULER_COL_NB_BACKGROUND, gsb_rgba_get_couleur ( "background_scheduled" ),
                        -1 );
        else
        {
            gtk_tree_store_set ( store,
                        &iter,
                        SCHEDULER_COL_NB_BACKGROUND,
                        gsb_rgba_get_couleur_with_indice ( "couleur_fond", current_color ),
                        -1 );
            current_color = !current_color;
        }

        gtk_tree_path_free (path);

        /* needn't to go in a child because the color is always the same, so
         * gtk_tree_path_next is enough */

        gtk_tree_path_next ( sorted_path );
    }

    return FALSE;
}


/**
 * select the given scheduled transaction
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_select ( gint scheduled_number )
{
    GtkTreeIter *iter;
    GtkTreeIter iter_sort;
    GtkTreePath *path = NULL;
    gint mother_number;

    devel_debug_int ( scheduled_number );

    /* if it's a split child, we must open the mother to select it */
    mother_number = gsb_data_scheduled_get_mother_scheduled_number ( scheduled_number );
    if ( mother_number )
    {
        GtkTreeIter *iter_mother;
        GtkTreeIter iter_mother_sort;

        iter_mother = gsb_scheduler_list_get_iter_from_scheduled_number ( mother_number );
        if ( iter_mother )
        {

            gtk_tree_model_sort_convert_child_iter_to_iter ( GTK_TREE_MODEL_SORT (
                        tree_model_sort_scheduler_list ),
                        &iter_mother_sort,
                        iter_mother );
            path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( tree_model_sort_scheduler_list ),
                        &iter_mother_sort );
            gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view_scheduler_list ),
                        path,
                        TRUE );
            gtk_tree_iter_free ( iter_mother );
        }
    }

    /* now can work with the transaction we want to select */
    iter = gsb_scheduler_list_get_iter_from_scheduled_number ( scheduled_number );

    if (!iter)
        return FALSE;

    gtk_tree_model_sort_convert_child_iter_to_iter ( GTK_TREE_MODEL_SORT ( tree_model_sort_scheduler_list ),
                    &iter_sort,
                    iter );

    gtk_tree_selection_select_iter ( GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view_scheduler_list ) ) ),
                        &iter_sort );

    /* move the tree view to the selection */
    if ( path == NULL )
        path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( tree_model_sort_scheduler_list ),
                        &iter_sort );

    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( tree_view_scheduler_list ),
                        path, NULL,
                        FALSE, 0.0, 0.0 );

    gtk_tree_iter_free ( iter );
    gtk_tree_path_free ( path );

    return FALSE;
}


/**
 * get the iter of the scheduled transaction given in param
 *
 * \param scheduled_number
 *
 * \return a newly allocated GtkTreeIter or NULL if not found
 * */
GtkTreeIter *gsb_scheduler_list_get_iter_from_scheduled_number ( gint scheduled_number )
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    model = GTK_TREE_MODEL (gsb_scheduler_list_get_model ());

    if ( !scheduled_number
	 ||
	 !model )
	return NULL;


    /* we go through the list in the model untill we find the transaction */
    if ( gtk_tree_model_get_iter_first ( model,
					 &iter ))
    {
	do
	{
	    gint scheduled_number_tmp;
	    GtkTreeIter iter_child;

	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 &iter,
				 SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number_tmp,
				 -1 );
	    if ( scheduled_number == scheduled_number_tmp )
		return ( gtk_tree_iter_copy (&iter));

	    /* gtk_tree_iter_next doesn't go in the children, so if the current transaction
	     * has children, we have to look for the transaction here, and go down into the children */
	    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL (model),
						&iter_child, &iter))
	    {
		/* ok so iter_child is on a split child, we go to see all the splits */
		do
		{
		    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
					 &iter_child,
					 SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number_tmp,
					 -1 );
		    if ( scheduled_number == scheduled_number_tmp )
			return ( gtk_tree_iter_copy (&iter_child));
		}
		while (gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),&iter_child ));
	    }
	}
	while (gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),&iter ));

    }
    return NULL;
}

/**
 * the same as gsb_scheduler_list_get_iter_from_scheduled_number but
 * return a gslist of iter corresponding to that scheduled number,
 * so there is only 1 iter for the once view, but more than 1 for the other views
 * use when changin the scheduled, to change also the virtuals ones on the screen
 *
 * \param scheduled_number
 *
 * \return a gslist of pointer to the iters, need to be free, or NULL if not found
 * */
GSList *gsb_scheduler_list_get_iter_list_from_scheduled_number ( gint scheduled_number )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint mother_number;
    gboolean return_iter = TRUE;
    GSList *iter_list = NULL;

    if ( !scheduled_number )
	return NULL;

    /* that function is called too for deleting a scheduled transaction,
     * and it can be already deleted, so we cannot call gsb_data_scheduled_... here
     * but... we need to know if it's a child split, so we call it, in all
     * the cases, if the transactions doesn't exist we will have no mother, so very good !*/

    mother_number = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);

    model = gsb_scheduler_list_get_model ();

    /* go throw the list to find the transaction */
    if (!gtk_tree_model_get_iter_first ( model,
					 &iter ))
	return NULL;

    while ( return_iter )
    {
	gint scheduled_transaction_buf;

	gtk_tree_model_get ( model,
			     &iter,
			     SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_transaction_buf,
			     -1 );

	if ( scheduled_transaction_buf == scheduled_number )
	{
	    iter_list = g_slist_append ( iter_list,
					 gtk_tree_iter_copy ( &iter ));

	    /* we have found the correct iter, but we want to continue to search,
	     * so we have to go back to the mother if necessary
	     * after that scheduled_transaction_buf is on the child, not on the
	     * mother so we will continue to the next scheduled transasction */

	    if ( mother_number )
	    {
		GtkTreeIter *child_iter;

		child_iter = gtk_tree_iter_copy (&iter);
		gtk_tree_model_iter_parent ( model,
					     &iter,
					     child_iter );
		gtk_tree_iter_free (child_iter);
	    }

	}

	if ( scheduled_transaction_buf == mother_number )
	{
	    GtkTreeIter *mother_iter;

	    mother_iter = gtk_tree_iter_copy (&iter);
	    return_iter = gtk_tree_model_iter_children ( model,
							 &iter,
							 mother_iter );
	    gtk_tree_iter_free (mother_iter);
	}
	else
	    return_iter = gtk_tree_model_iter_next ( model,
						     &iter );
    }
    return iter_list;
}



/**
 * find the date untill we want to show the scheduled transactions
 * on the scheduled list, with the user configuration
 *
 * \param
 *
 * \return a newly allocated final date or NULL for unique view
 * */
GDate *gsb_scheduler_list_get_end_date_scheduled_showed ( void )
{
    GDate *end_date;

    /* on récupère la date du jour et la met dans end_date pour les
    * vérifications ultérieures */

    end_date = gdate_today ();

    /* on calcule la date de fin de l'affichage */

    switch ( etat.affichage_echeances )
    {
	case SCHEDULER_PERIODICITY_ONCE_VIEW:
	    return NULL;
	    break;

	case SCHEDULER_PERIODICITY_WEEK_VIEW:
	    g_date_add_days ( end_date, 7 );
	    g_date_add_months ( end_date, 0 );
	    break;

	case SCHEDULER_PERIODICITY_MONTH_VIEW:
	    g_date_add_months ( end_date, 1 );
	    end_date -> day = 1;
	    g_date_subtract_days ( end_date, 1 );
	    break;

	case SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW:
	    g_date_add_months ( end_date, 2 );
	    end_date -> day = 1;
	    g_date_subtract_days ( end_date, 1 );
	    break;

	case SCHEDULER_PERIODICITY_TRIMESTER_VIEW:
	    g_date_add_months ( end_date, 3 );
	    end_date -> day = 1;
	    g_date_subtract_days ( end_date, 1 );
	    break;

	case SCHEDULER_PERIODICITY_YEAR_VIEW:
	    g_date_add_years ( end_date, 1 );
	    end_date -> day = 1;
	    end_date -> month = 1;
	    g_date_subtract_days ( end_date, 1 );
	    break;

	case SCHEDULER_PERIODICITY_CUSTOM_VIEW:
	    switch ( etat.affichage_echeances_perso_j_m_a )
	    {
		case PERIODICITY_DAYS:
		    g_date_add_days ( end_date, etat.affichage_echeances_perso_nb_libre );
		    break;

		case PERIODICITY_WEEKS:
		    g_date_add_days ( end_date, etat.affichage_echeances_perso_nb_libre * 7 );
		    break;

		case PERIODICITY_MONTHS:
		    g_date_add_months ( end_date, etat.affichage_echeances_perso_nb_libre );
		    break;

		case PERIODICITY_YEARS:
		    g_date_add_years ( end_date, etat.affichage_echeances_perso_nb_libre );
		    break;
	    }
    }
    return end_date;
}



/**
 * called when the selection of the list change
 *
 * \param selection
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_selection_changed ( GtkTreeSelection *selection,
                        gpointer null )
{
    gint tmp_number = 0;
    gint account_number;

    /* wanted to set that function in gsb_scheduler_list_button_press but g_signal_connect_after
     * seems not to work in that case... */

    /* protect last_scheduled_number because when refill the list, set selection to 0 and so last_scheduled_number... */
    tmp_number = gsb_scheduler_list_get_current_scheduled_number ();

    if (tmp_number)
        last_scheduled_number = tmp_number;

    /* if conf.show_transaction_selected_in_form => edit the scheduled transaction */
    if ( tmp_number != 0 && conf.show_transaction_selected_in_form )
            gsb_scheduler_list_edit_transaction (tmp_number);
    else if ( tmp_number == 0 )
    {
        gsb_form_scheduler_clean ( );
        account_number = gsb_data_scheduled_get_account_number (tmp_number);
        gsb_form_clean ( account_number );
    }

    /* sensitive/unsensitive the button execute */
    gtk_widget_set_sensitive ( scheduler_button_execute,
                        (tmp_number > 0)
                        &&
                        !gsb_data_scheduled_get_mother_scheduled_number (tmp_number));

    /* sensitive/unsensitive the button edit */
    gtk_widget_set_sensitive ( scheduler_button_edit, ( tmp_number > 0 ) );

    /* sensitive/unsensitive the button delete */
    gtk_widget_set_sensitive ( scheduler_button_delete, ( tmp_number > 0 ) );

    gsb_menu_set_menus_select_scheduled_sensitive ( tmp_number > 0 );

    return FALSE;
}


/**
 * called when a key is pressed on the scheduled transactions list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_key_press ( GtkWidget *tree_view,
                        GdkEventKey *ev )
{
    gint scheduled_number;

    scheduled_number = gsb_scheduler_list_get_current_scheduled_number ();

    switch ( ev->keyval )
    {
	case GDK_KEY_Return :		/* touches entrée */
	case GDK_KEY_KP_Enter :

	    if ( scheduled_number )
		gsb_scheduler_list_edit_transaction (scheduled_number);
	    break;


	case GDK_KEY_Delete :               /*  del  */

	    if ( scheduled_number > 0 )
		gsb_scheduler_list_delete_scheduled_transaction (scheduled_number, TRUE);
	    break;

	case GDK_KEY_Left:
	    /* if we press left, give back the focus to the tree at left */
	    gtk_widget_grab_focus ( gsb_gui_navigation_get_tree_view ( ) );
	    break;

	case GDK_KEY_space:
	    /* space open/close a split */
	    gsb_scheduler_list_switch_expander (scheduled_number);
	    break;
    }
    return ( FALSE );
}

/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev )
{
	/* show the popup */
	if ( ev -> button == RIGHT_BUTTON )
        popup_scheduled_context_menu ( );
    else if ( ev -> type == GDK_2BUTTON_PRESS )
    {
         /* if double-click => edit the scheduled transaction */
        gint current_scheduled_number;

        current_scheduled_number = gsb_scheduler_list_get_current_scheduled_number ();

        if ( current_scheduled_number )
            gsb_scheduler_list_edit_transaction (current_scheduled_number);
    }
    return FALSE;
}


/**
 * get the current selected transaction and return it
 * if it's a virtual transaction, return 0
 *
 * \param
 *
 * \return the current scheduled transaction number
 * */
gint gsb_scheduler_list_get_current_scheduled_number ( void )
{
    GList *list_tmp;
    GtkTreeModel *model;
    gint scheduled_number;
    gint virtual_transaction;
    GtkTreeIter iter;
    GtkWidget *tree_view;

    tree_view = gsb_scheduler_list_get_tree_view ();
    list_tmp = gtk_tree_selection_get_selected_rows ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
						      &model );

    if ( !list_tmp )
	return 0;

    gtk_tree_model_get_iter ( GTK_TREE_MODEL (model),
			      &iter,
			      list_tmp -> data );
    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number,
			 SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_transaction,
			 -1 );

    g_list_foreach (list_tmp, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (list_tmp);

    if ( virtual_transaction )
	return 0;
    else
	return scheduled_number;
}


/**
 * edit the scheduling transaction given in param
 *
 * \param scheduled_number the number, -1 if new scheduled transaction or < -1 if new child of split
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_edit_transaction ( gint scheduled_number )
{
    devel_debug_int (scheduled_number);
    if ( scheduled_number == 0 )
        gsb_form_fill_by_transaction ( gsb_scheduler_list_get_current_scheduled_number ( ), FALSE, TRUE );
    else
        gsb_form_fill_by_transaction ( scheduled_number, FALSE, TRUE );

    return FALSE;
}



/**
 * delete the current selected transaction, but called by menu
 * just call gsb_scheduler_list_delete_scheduled_transaction with show_warning = TRUE
 * because cannot do that by the signal
 *
 * \param button
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_delete_scheduled_transaction_by_menu ( GtkWidget *button,
                        gpointer null )
{
    gsb_scheduler_list_delete_scheduled_transaction (0, TRUE);
    return FALSE;
}



/**
 * delete the scheduled transaction
 *
 * \param scheduled_number the transaction to delete
 * \param show_warning TRUE to warn, FALSE to delete directly
 * 		!! this don't affect the question to delete only the occurence or the whole scheduled transaction
 * 		it affects only for children of split, and especially deleting the white line child
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_delete_scheduled_transaction ( gint scheduled_number,
                        gboolean show_warning )
{
    gchar *tmpstr;
    gint mother_number = 0;
    gint result;
    gint msg_no = 0;

    devel_debug_int (scheduled_number);

    if ( !scheduled_number )
        scheduled_number = gsb_scheduler_list_get_current_scheduled_number ( );

    /* return for white line only if show_warning is set
     * (means the action is not automatic) */
    if ( scheduled_number <= 0
	 &&
	 show_warning )
        return FALSE;

    mother_number = gsb_data_scheduled_get_mother_scheduled_number ( scheduled_number );

    /* show a warning */
    if (show_warning)
    {
        if ( mother_number )
        {
            /* ask all the time for a child */
            msg_no = question_conditional_yes_no_get_no_struct ( &delete_msg[0],
                        "delete-child-scheduled" );
            tmpstr = g_strdup_printf ( _("Do you really want to delete the child of the "
                        "scheduled transaction with party '%s' ?"),
                        gsb_data_payee_get_name (
                        gsb_data_scheduled_get_party_number (scheduled_number),
                        FALSE ) );
            delete_msg[msg_no].message = tmpstr;
            if ( !question_conditional_yes_no_with_struct ( &delete_msg[msg_no] ) )
            {
                g_free ( tmpstr );
                return FALSE;
            }
            g_free ( tmpstr );
        }
        else
        {
            /* for a normal scheduled, ask only if no frequency, else, it will
             * have another dialog to delete the occurence or the transaction */
            msg_no = question_conditional_yes_no_get_no_struct ( &delete_msg[0],
                        "delete-scheduled" );
            tmpstr = g_strdup_printf ( _("Do you really want to delete the scheduled "
                        "transaction with party '%s' ?"),
                        gsb_data_payee_get_name (
                        gsb_data_scheduled_get_party_number ( scheduled_number ),
                                                 FALSE ) );
            delete_msg[msg_no].message = tmpstr;
            if ( !gsb_data_scheduled_get_frequency (scheduled_number)
             &&
             !question_conditional_yes_no_with_struct ( &delete_msg[msg_no] ) )
            {
                g_free ( tmpstr );
                return FALSE;
            }
            g_free ( tmpstr );
        }
    }

    /* split with child of split or normal scheduled,
     * for a child, we directly delete it, for mother, ask
     * for just that occurrence or the complete transaction */
    if ( mother_number )
    {
        gint white_line_number;

        /* !! important to remove first from the list... */
        gsb_scheduler_list_remove_transaction_from_list ( scheduled_number );
        gsb_data_scheduled_remove_scheduled ( scheduled_number );

        white_line_number = gsb_data_scheduled_get_white_line ( mother_number );
        gsb_scheduler_list_update_white_child ( white_line_number, mother_number );
    }
    else
    {
        /* ask if we want to remove only the current one (so only change the date
         * for the next) or all (so remove the transaction */

        if ( gsb_data_scheduled_get_frequency ( scheduled_number) )
        {
            GtkWidget * vbox, * checkbox, *dialog = NULL;
            gchar *occurrences;

            msg_no = question_conditional_yes_no_get_no_struct ( &delete_msg[0],
                            "delete-scheduled-occurrences" );

            if ( delete_msg[msg_no].hidden )
                result = delete_msg[msg_no].default_answer;
            else
            {
            tmpstr = utils_real_get_string (gsb_data_scheduled_get_amount (scheduled_number));
            occurrences = g_strdup_printf ( _("Do you want to delete just this occurrence or "
                            "the whole scheduled transaction?\n\n%s : %s [%s %s]"),
                            gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)),
                            gsb_data_payee_get_name (
                            gsb_data_scheduled_get_party_number (scheduled_number), FALSE ),
                            tmpstr,
                            gsb_data_currency_get_name (
                            gsb_data_scheduled_get_currency_number (scheduled_number)));
            g_free ( tmpstr );

            dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                            occurrences, _("Delete this scheduled transaction?") );

            gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
                             "gtk-cancel", 2,
                             _("All the occurrences"), 1,
                             _("Only this one"), 0,
                             NULL );

            vbox = gtk_dialog_get_content_area ( GTK_DIALOG ( dialog ) );

            checkbox = gtk_check_button_new_with_label (_("Keep this choice permanently?"));
            g_signal_connect ( G_OBJECT ( checkbox ),
                            "toggled",
                            G_CALLBACK ( dialogue_update_struct_message ),
                            &delete_msg[msg_no] );
            gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, TRUE, TRUE, 6 );
            gtk_widget_show_all ( checkbox );

            result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

            delete_msg[msg_no].default_answer = result;
            g_free (occurrences);
            gtk_widget_destroy ( dialog );
            }
        }
        else
            result = 1;

        switch ( result )
        {
            case 0:
            if ( gsb_scheduler_increase_scheduled (scheduled_number))
                gsb_scheduler_list_update_transaction_in_list (scheduled_number);
            break;

            case 1:
            /* !! important to remove first from the list... */
            gsb_scheduler_list_remove_transaction_from_list ( scheduled_number );
            gsb_data_scheduled_remove_scheduled ( scheduled_number );
            break;
        }
    }

    gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());

    gsb_calendar_update ( );
    run.mise_a_jour_liste_echeances_manuelles_accueil = TRUE;

    gsb_file_set_modified ( TRUE );

    return FALSE;
}


/**
 * called when the user choose a custom periodicity on the toolbar
 *
 * \param
 *
 * \return TRUE : did his choice, FALSE : cancel the choice
 * */
gboolean gsb_scheduler_list_popup_custom_periodicity_dialog (void)
{
    GtkWidget *dialog, *hbox, *hbox2, *paddingbox, *label, *entry, *combobox;
    int i;

    dialog = gtk_dialog_new_with_buttons ( _("Show scheduled transactions"),
					   GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
					   GTK_DIALOG_MODAL,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-apply", GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_pack_start ( GTK_BOX ( dialog_get_content_area ( dialog ) ), hbox, FALSE, FALSE, 0 );
    paddingbox = new_paddingbox_with_title ( hbox, TRUE, _("Scheduler frequency") );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(paddingbox), 6 );

    hbox2 = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox2, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Show transactions for the next: "));
    gtk_box_pack_start ( GTK_BOX(hbox2), label, FALSE, FALSE, 0 );
    entry = gsb_automem_spin_button_new ( &etat.affichage_echeances_perso_nb_libre,
					  NULL, NULL );
    gtk_box_pack_start ( GTK_BOX(hbox2), entry, FALSE, FALSE, 6 );

    /* combobox for userdefined frequency */
    combobox = gtk_combo_box_text_new ();
    gtk_box_pack_start ( GTK_BOX(hbox2), combobox, FALSE, FALSE, 0 );

    for ( i = 0; j_m_a_names[i]; i++ )
    {
        gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT ( combobox ),
                        g_dgettext ( NULL, j_m_a_names[i] ) );
    }
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combobox ), etat.affichage_echeances_perso_j_m_a );

    gtk_widget_show_all ( dialog );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
	case GTK_RESPONSE_OK:
	    etat.affichage_echeances_perso_j_m_a = gtk_combo_box_get_active ( GTK_COMBO_BOX (combobox) );
	    etat.affichage_echeances_perso_nb_libre = utils_str_atoi ( gtk_entry_get_text ( GTK_ENTRY(entry)) );
	    gtk_widget_destroy ( dialog );
	    return TRUE;
    }

    gtk_widget_destroy ( dialog );
    return FALSE;
}


/**
 * switch the expander of the split given in param
 *
 * \param scheduled_number the scheduled split we want to switch
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_switch_expander ( gint scheduled_number )
{
    GtkTreeIter *iter;
    GtkTreePath *path;
    GtkTreePath *path_sorted;

    if ( !gsb_data_scheduled_get_split_of_scheduled (scheduled_number)
	 ||
	 !tree_view_scheduler_list )
	return FALSE;

    iter = gsb_scheduler_list_get_iter_from_scheduled_number (scheduled_number);

    path = gtk_tree_model_get_path ( GTK_TREE_MODEL (tree_model_scheduler_list),
				     iter );
    path_sorted = gtk_tree_model_sort_convert_child_path_to_path ( tree_model_sort_scheduler_list,
								   path );
    if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (tree_view_scheduler_list), path_sorted))
	gtk_tree_view_collapse_row (GTK_TREE_VIEW (tree_view_scheduler_list), path_sorted);
    else
	gtk_tree_view_expand_row (GTK_TREE_VIEW (tree_view_scheduler_list), path_sorted, FALSE);
    gtk_tree_path_free (path);
    gtk_tree_path_free (path_sorted);
    gtk_tree_iter_free (iter);

    return FALSE;
}




/**
 * called when the size of the tree view changed, to keep the same ration
 * between the columns
 *
 * \param tree_view	    the tree view of the scheduled transactions list
 * \param allocation	the new size
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_size_allocate ( GtkWidget *tree_view,
                        GtkAllocation *allocation,
                        gpointer null )
{
    gint i;

    if (allocation -> width == scheduler_current_tree_view_width)
    {
        /* size of the tree view didn't change, but we received an allocated signal
         * it happens several times, and especially when we change the columns,
         * so we update the colums */

        /* sometimes, when the list is not visible, he will set all the columns to 1%...
         * we block that here */
        if (gtk_tree_view_column_get_width (scheduler_list_column[0]) == 1)
            return FALSE;

        for (i=0 ; i<SCHEDULER_COL_VISIBLE_COLUMNS ; i++)
        {
            if (gtk_tree_view_column_get_width (scheduler_list_column[i]))
                scheduler_col_width[i] = (gtk_tree_view_column_get_width (
                        scheduler_list_column[i]) * 100) / allocation -> width + 1;
        }

        return FALSE;
    }

    /* the size of the tree view changed, we keep the ration between the columns,
     * we don't set the size of the last column to avoid the calculate problems,
     * it will take the end of the width alone */
    scheduler_current_tree_view_width = allocation -> width;

    for ( i = 0 ; i < SCHEDULER_COL_VISIBLE_COLUMNS - 1 ; i++ )
    {
        gint width;

        width = (scheduler_col_width[i] * (allocation -> width))/ 100;
        if ( width > 0 )
            gtk_tree_view_column_set_fixed_width ( scheduler_list_column[i],
                               width );
    }
    return FALSE;
}


/**
 * Pop up a menu with several actions to apply to current scheduled.
 *
 * \param
 *
 */
void popup_scheduled_context_menu ( void )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    gint scheduled_number;

    menu = gtk_menu_new ();

    /* Edit transaction */
    scheduled_number = gsb_scheduler_list_get_current_scheduled_number ( );

    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-edit-16.png", _("Edit transaction")));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_scheduler_list_edit_transaction_by_pointer ),
                        GINT_TO_POINTER ( scheduled_number ) );
    if ( scheduled_number < 0 )
        gtk_widget_set_sensitive ( menu_item, FALSE );

    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Clone transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-copy-16.png", _("Clone transaction")));
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_scheduler_list_clone_selected_scheduled ),
                        GINT_TO_POINTER ( scheduled_number ) );
    if ( scheduled_number < 0 )
        gtk_widget_set_sensitive ( menu_item, FALSE );

    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new ( ) );

    /* New transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label  ("gsb-new-transaction-16.png", _("New transaction")));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_scheduler_list_edit_transaction_by_pointer ),
                        GINT_TO_POINTER ( -1 ) );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Delete transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Delete transaction")));
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_scheduler_list_delete_scheduled_transaction_by_menu ),
                        NULL );

    if ( scheduled_number < 0 )
        gtk_widget_set_sensitive ( menu_item, FALSE );

    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new ( ) );

    /* Display/hide notes */
    if ( etat.affichage_commentaire_echeancier )
        menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gsb-comments-16.png", _("Displays notes")));
    else
        menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gsb-comments-16.png", _("Displays Frequency/Mode")));

    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_scheduler_list_show_notes ),
                        scheduler_display_hide_notes );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new ( ) );

    /* Execute transaction */
    menu_item = utils_menu_item_new_from_image_label ("gtk-execute-16.png",_("Execute transaction"));
    g_signal_connect_swapped ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_scheduler_list_execute_transaction ),
                        NULL );
    if ( scheduled_number < 0 )
        gtk_widget_set_sensitive ( menu_item, FALSE );

    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Finish all. */
    gtk_widget_show_all (menu);

#if GTK_CHECK_VERSION (3,22,0)
	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
#else
	gtk_menu_popup ( GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
#endif
}


/**
 * Clone selected transaction if any.  Update user interface as well.
 *
 * \param menu_item
 * \param null
 *
 * \return FALSE
 */
gboolean gsb_scheduler_list_clone_selected_scheduled ( GtkWidget *menu_item,
                        gint *scheduled_number )
{
    gint new_scheduled_number;
    gint tmp_scheduled_number;

    if ( scheduled_number == NULL )
        tmp_scheduled_number = gsb_scheduler_list_get_current_scheduled_number ( );
    else
        tmp_scheduled_number = GPOINTER_TO_INT ( scheduled_number );

    new_scheduled_number = gsb_data_scheduled_new_scheduled ( );

    gsb_data_scheduled_copy_scheduled ( tmp_scheduled_number, new_scheduled_number );

    if ( gsb_data_scheduled_get_split_of_scheduled ( tmp_scheduled_number ) )
    {
        GSList *tmp_list;

        tmp_list = g_slist_copy ( gsb_data_scheduled_get_scheduled_list ( ) );

        while ( tmp_list )
        {
            gint split_scheduled_number;

            split_scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list -> data );

            if ( gsb_data_scheduled_get_mother_scheduled_number (
             split_scheduled_number ) == tmp_scheduled_number )
            {
                gint new_number;

                new_number = gsb_data_scheduled_new_scheduled ( );
                gsb_data_scheduled_copy_scheduled ( split_scheduled_number, new_number );
                gsb_data_scheduled_set_mother_scheduled_number ( new_number, new_scheduled_number );
            }

            tmp_list = tmp_list -> next;
        }
    }

    gsb_scheduler_list_fill_list ( gsb_scheduler_list_get_tree_view ( ) );
    gsb_scheduler_list_set_background_color ( gsb_scheduler_list_get_tree_view ( ) );
    gsb_scheduler_list_select ( new_scheduled_number );

    gsb_file_set_modified ( TRUE );

    return FALSE;
}


/**
 * Called to edit a specific transaction but the number of transaction
 * is passed via a pointer (by g_signal_connect)
 *
 * \param scheduled_number a pointer which is the number of the transaction
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_edit_transaction_by_pointer ( gint *scheduled_number )
{
    devel_debug_int (GPOINTER_TO_INT ( scheduled_number ) );
    gsb_scheduler_list_edit_transaction ( GPOINTER_TO_INT (scheduled_number));
    return FALSE;
}


/**
 *
 *
 *
 *
 **/
gint gsb_scheduler_list_sort_function_by_payee ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gint *column_ptr )
{
    gchar *str_1;
    gchar *str_2;
    gint number_1;
    gint number_2;
    gint virtual_op_1 = 0;
    gint virtual_op_2 = 0;
    gint return_value = 0;

    /* first, we sort by payee (col 0) */
    gtk_tree_model_get ( model,
                        iter_1,
                        COL_NB_PARTY, &str_1,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_1,
                        SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_op_1,
                        -1 );

    gtk_tree_model_get ( model,
                        iter_2,
                        COL_NB_PARTY, &str_2,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_2,
                        SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_op_2,
                        -1 );

    if ( number_1 == -1 )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            return 1;
        else
            return -1;
    }
    else if ( number_2 == -1 )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            return -1;
        else
            return 1;
    }

    if ( sort_type == GTK_SORT_ASCENDING )
        return_value = virtual_op_1 - virtual_op_2;
    else
        return_value = virtual_op_2 - virtual_op_1;

    if ( return_value )
        return return_value;

    if ( str_1 )
    {
        if ( str_2 )
            return_value = g_utf8_collate ( str_1, str_2 );
        else
        {
            g_free ( str_1 );
            return -1;
        }
    }
    else if ( str_2 )
    {
        g_free ( str_2 );
        if ( sort_type == GTK_SORT_ASCENDING )
            return 1;
        else
            return -1;
    }

    if ( return_value == 0 )
        return_value = number_1 - number_2;

    g_free ( str_1 );
    g_free ( str_2 );

    return return_value;
}


gboolean gsb_scheduler_list_sort_column_clicked ( GtkTreeViewColumn *tree_view_column,
                        gint *column_ptr )
{
    gint current_column;
    gint new_column;

    gtk_tree_sortable_get_sort_column_id ( GTK_TREE_SORTABLE (
                        tree_model_sort_scheduler_list ),
                        &current_column, &sort_type );

    new_column = GPOINTER_TO_INT (column_ptr);

    /* if the new column is the same as the old one, we change
     * the sort type */
    if ( new_column == current_column )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            sort_type = GTK_SORT_DESCENDING;
        else
            sort_type = GTK_SORT_ASCENDING;
    }
    else
	/* we sort by another column, so sort type by default is descending */
	sort_type = GTK_SORT_ASCENDING;

    gtk_tree_view_column_set_sort_indicator ( scheduler_list_column[current_column], FALSE );
    gtk_tree_view_column_set_sort_indicator ( scheduler_list_column[new_column], TRUE );

    gtk_tree_view_column_set_sort_order ( scheduler_list_column[new_column], sort_type );
    gtk_tree_sortable_set_sort_column_id (  GTK_TREE_SORTABLE (
                        tree_model_sort_scheduler_list ),
                        new_column, sort_type );

    gsb_scheduler_list_set_background_color ( gsb_scheduler_list_get_tree_view ( ) );

    if ( last_scheduled_number > 0 )
        gsb_scheduler_list_select ( last_scheduled_number );
    else
        gsb_scheduler_list_select ( -1 );

    return FALSE;
}


/**
 *
 *
 *
 *
 **/
gboolean gsb_scheduler_list_set_largeur_col ( void )
{
    gint i;
    gint width;

    for ( i = 0 ; i < SCHEDULER_COL_VISIBLE_COLUMNS ; i++ )
    {
        width = ( scheduler_col_width[i] * ( scheduler_current_tree_view_width ) ) / 100;
        if ( width > 0 )
            gtk_tree_view_column_set_fixed_width ( scheduler_list_column[i], width );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 **/
gint gsb_scheduler_list_sort_function_by_account ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gint *column_ptr )
{
    gchar *str_1;
    gchar *str_2;
    gint number_1;
    gint number_2;
    gint virtual_op_1 = 0;
    gint virtual_op_2 = 0;
    gint return_value = 0;

    /* first, we sort by account (col 0) */
    gtk_tree_model_get ( model,
                        iter_1,
                        COL_NB_ACCOUNT, &str_1,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_1,
                        SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_op_1,
                        -1 );

    gtk_tree_model_get ( model,
                        iter_2,
                        COL_NB_ACCOUNT, &str_2,
                        SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_2,
                        SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_op_2,
                        -1 );

    if ( number_1 == -1 )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            return 1;
        else
            return -1;
    }
    else if ( number_2 == -1 )
    {
        if ( sort_type == GTK_SORT_ASCENDING )
            return -1;
        else
            return 1;
    }

    if ( sort_type == GTK_SORT_ASCENDING )
        return_value = virtual_op_1 - virtual_op_2;
    else
        return_value = virtual_op_2 - virtual_op_1;

    if ( return_value )
        return return_value;

    if ( str_1 )
    {
        if ( str_2 )
            return_value = g_utf8_collate ( str_1, str_2 );
        else
        {
            g_free ( str_1 );
            return -1;
        }
    }
    else if ( str_2 )
    {
        g_free ( str_2 );
        if ( sort_type == GTK_SORT_ASCENDING )
            return 1;
        else
            return -1;
    }

    if ( return_value == 0 )
        return_value = number_1 - number_2;

    g_free ( str_1 );
    g_free ( str_2 );

    return return_value;
}


gboolean gsb_scheduler_list_update_white_child ( gint white_line_number,
                        gint mother_scheduled_number )
{
    GtkTreeIter *iter = NULL;
    GSList *tmp_list;
    gsb_real total_split = null_real;
    gsb_real variance;
    gchar *tmp_str;

    if ( !tree_model_scheduler_list )
        return FALSE;

    iter = gsb_scheduler_list_get_iter_from_scheduled_number ( white_line_number );
    if ( !iter )
        return FALSE;

    tmp_list = gsb_data_scheduled_get_scheduled_list (  );
    while ( tmp_list )
    {
        gint split_scheduled_number;

        split_scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list -> data );

        if ( gsb_data_scheduled_get_mother_scheduled_number (
         split_scheduled_number ) == mother_scheduled_number )
        {
            total_split = gsb_real_add ( total_split,
                        gsb_data_scheduled_get_amount ( split_scheduled_number ) );
        }
        tmp_list = tmp_list -> next;
    }

    variance = gsb_real_sub ( gsb_data_scheduled_get_amount ( mother_scheduled_number ), total_split );

    /* show the variance and sub-total only if different of the scheduled */
    if ( variance.mantissa )
    {
        gchar *amount_string;
        gchar *variance_string;
        gint currency_number;

        currency_number = gsb_data_scheduled_get_currency_number ( mother_scheduled_number );
        amount_string = utils_real_get_string_with_currency ( total_split, currency_number, TRUE);
        variance_string = utils_real_get_string_with_currency (variance, currency_number, TRUE);

        tmp_str = g_strdup_printf ( _("Total: %s (variance : %s)"), amount_string, variance_string );

        g_free ( amount_string );
        g_free ( variance_string );

        gsb_scheduler_list_set_color_of_mother ( mother_scheduled_number, TRUE );
    }
    else
    {
        tmp_str = "";
        gsb_scheduler_list_set_color_of_mother ( mother_scheduled_number, FALSE );
    }

    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model_scheduler_list ), iter, 2, tmp_str, -1 );

    if ( tmp_str && strlen ( tmp_str ) )
        g_free ( tmp_str );

    return TRUE;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_scheduler_list_get_toolbar ( void )
{
    return scheduler_toolbar;
}


/**
 * set the text red if the variance is nonzero
 *
 *
 */
gboolean gsb_scheduler_list_set_color_of_mother ( gint mother_scheduled_number,
                        gboolean is_red )
{
    GtkTreeIter *iter = NULL;
    gchar *color_str = NULL;
    gint i;

    iter = gsb_scheduler_list_get_iter_from_scheduled_number ( mother_scheduled_number );
    if ( !iter )
        return FALSE;

    if ( is_red )
        color_str = "red";

    for ( i = 0 ; i < SCHEDULER_COL_VISIBLE_COLUMNS -1 ; i++ )
    {
        gtk_tree_store_set ( GTK_TREE_STORE ( tree_model_scheduler_list ),
                        iter,
                        SCHEDULER_COL_NB_TEXT_COLOR, color_str,
                        -1 );
    }

    return TRUE;
}


/**
 * remove the orphan transactions
 *
 * \param orphan list
 *
 * \return void
 */
void gsb_scheduler_list_process_orphan_list ( GSList *orphan_scheduled,
                        GDate *end_date )
{
    GSList *tmp_list;
    gchar *string = NULL;
    GArray *garray;

    garray = g_array_new ( FALSE, FALSE, sizeof ( gint ) );
    tmp_list = orphan_scheduled;
    while (tmp_list)
    {
        gint scheduled_number;

        scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list -> data );

        if ( !gsb_scheduler_list_append_new_scheduled ( scheduled_number, end_date ) )
        {
            /* on sauvegarde le numéro de l'opération */
            g_array_append_val ( garray, scheduled_number );
            if ( string == NULL )
                string = utils_str_itoa ( scheduled_number );
            else
                string = g_strconcat ( string, " - ",
                    utils_str_itoa ( scheduled_number ), NULL );
        }

        tmp_list = tmp_list -> next;
    }

    /* if string is not null, there is still some children
     * which didn't find their mother. show them now */
    if ( string )
    {
        gchar *message;
        gint result;

        message = g_strdup_printf ( _("Some scheduled children didn't find their mother in the list, "
                        "this shouldn't happen and there is probably a bug behind that.\n\n"
                        "The concerned children number are:\n %s\n\n"
                        "Do you want to delete it"),
                        string );

        result = question_yes_no (message, _("Remove orphan children"), GTK_RESPONSE_CANCEL );

        if (result == TRUE)
        {
            gint i;

            for ( i = 0; i < (gint) garray->len; i++ )
                gsb_data_scheduled_remove_scheduled ( g_array_index ( garray, gint, i ) );

        }

        g_free ( message );
        g_free ( string );
        g_array_free ( garray, TRUE );
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GSList *gsb_scheduler_list_get_scheduled_transactions_taken (void)
{
	return scheduled_transactions_taken;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GSList *gsb_scheduler_list_get_scheduled_transactions_to_take (void)
{
	return scheduled_transactions_to_take;
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
