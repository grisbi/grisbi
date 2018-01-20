/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2018 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include "config.h"
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "accueil.h"
#include "affichage_liste.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_form.h"
#include "gsb_transactions_list.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

#define GSB_NBRE_CHAR 15
#define GSB_NAMEFILE_TOO_LONG 75				/* 5 lignes dans le bouton */

/*START_EXTERN*/
/*END_EXTERN*/

struct _GrisbiWin
{
  GtkApplicationWindow parent;
};

struct _GrisbiWinClass
{
  GtkApplicationWindowClass parent_class;
};

typedef struct _GrisbiWinPrivate GrisbiWinPrivate;

struct _GrisbiWinPrivate
{
	GtkBuilder *        builder;

    /* box principale */
    GtkWidget *         main_box;
	GtkWidget *         stack_box;

    /* page d'accueil affichée si pas de fichier chargé */
	GtkWidget *			no_file_page;
	GtkWidget *			no_file_sw;
	GtkWidget *			no_file_grid;
	GtkWidget *			bouton_nouveau;
	GtkWidget *			bouton_ouvrir;
	GtkWidget *			bouton_importer;

    /* widgets si un fichier est chargé */
	GtkWidget *			form_general;
    GtkWidget *         hpaned_general;
	GtkWidget *			navigation_sw;
    GtkWidget *         notebook_general;
	GtkWidget *			scheduler_calendar;
    GtkWidget *         vbox_general;

	GtkWidget *			vbox_transactions_list;			/* vbox contenant le tree_view des opérations */

	/* allocations widgets */
	gint 				form_expander_label_width;
	GtkAllocation		navigation_sw_allocation;
	GtkAllocation		notebook_general_allocation;
	GtkAllocation		scheduler_calendar_allocation;

    /* nom du fichier associé à la fenêtre */
    gchar *             filename;

    /* titre de la fenêtre */
    gchar *             window_title;

    /* Menus et barres d'outils */
    /* menu move-to-acc */
    GMenu *             menu;
    gboolean            init_move_to_acc;		/* Si VRAI le sous menu des comptes est initialisé */

    /* statusbar */
    GtkWidget *         statusbar;
    GdkWindow *         tracked_window;
    guint               context_id;
    guint               message_id;

    /* headings_bar */
    GtkWidget *         headings_eb;
    GtkWidget *         headings_title;          /** Title for the heading bar. */
    GtkWidget *         headings_suffix;         /** Suffix for the heading bar.  */

	/* paned */
	gint 				hpaned_general_width;

    /* variables de configuration de la fenêtre */
	GrisbiWinEtat		*w_etat;

    /* structure run */
    GrisbiWinRun     	*w_run;


	/* prefs dialog*/
	GtkWidget *			prefs_dialog;
};

G_DEFINE_TYPE_WITH_PRIVATE(GrisbiWin, grisbi_win, GTK_TYPE_APPLICATION_WINDOW);

/* variables initialisées lors de l'exécution de grisbi PROVISOIRE */
struct _GrisbiWinRun run;

/* global "etat" structure shared in the entire program */
struct _GrisbiWinEtat etat;

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/* STATUS_BAR */
/**
 * initiate the GtkStatusBar to hold various status
 * information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_init_statusbar (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* set status_bar PROVISOIRE */
    priv->context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (priv->statusbar), "Grisbi");
    priv->message_id = G_MAXUINT;
}


/* HEADINGS_EB */
static void grisbi_win_headings_private_update_label_markup (GtkLabel *label,
                                                             const gchar *text,
                                                             gboolean escape_text)
{
    gchar* tmp_str;

    if (escape_text)
        tmp_str = g_markup_printf_escaped ("<b>%s</b>", text);
    else
        tmp_str = g_strconcat ("<b>", text, "</b>", NULL);
    gtk_label_set_markup (label,tmp_str);

    g_free (tmp_str);
}

/**
 * Trigger a callback functions only if button event that triggered it
 * was a simple click.
 *
 * \param button
 * \param event
 * \param callback function
 *
 * \return  TRUE.
 */
static gboolean grisbi_win_headings_simpleclick_event_run (GtkWidget *button,
                        GdkEvent *button_event,
                        GCallback callback)
{
    if (button_event -> type == GDK_BUTTON_PRESS)
    {
        callback ();
    }

    return TRUE;
}

/**
 * retourne headings_eb
 *
 * \param GrisbiWin *win
 *
 * \return priv->headings_eb
 **/
static GtkWidget *grisbi_win_get_headings_eb (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

    if (win == NULL)
        win = grisbi_app_get_active_window (NULL);

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    return priv->headings_eb;
}

/**
 * initiate the headings_bar information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_create_headings_eb (GrisbiWin *win)
{
    GtkWidget *grid;
    GtkWidget *arrow_eb;
    GtkWidget *arrow_left;
    GtkWidget *arrow_right;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    priv->headings_eb = gtk_event_box_new ();
    gtk_widget_set_name (priv->headings_eb, "grey_box");
	gtk_widget_set_margin_start (priv->headings_eb, MARGIN_BOX);
	gtk_widget_set_margin_end (priv->headings_eb, MARGIN_BOX);

    grid = gtk_grid_new ();
    gtk_widget_set_margin_end (grid, MARGIN_BOX);
	gtk_grid_set_column_spacing (GTK_GRID (grid), MARGIN_BOX);
	gtk_widget_set_hexpand (grid, TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (grid), MARGIN_BOX);

    /* Create two arrows. */
    arrow_left = gtk_image_new_from_icon_name ("pan-start-symbolic", GTK_ICON_SIZE_BUTTON);
    arrow_eb = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (arrow_eb), arrow_left);
    g_signal_connect (G_OBJECT (arrow_eb), "button-press-event",
                        G_CALLBACK (grisbi_win_headings_simpleclick_event_run),
                        gsb_gui_navigation_select_prev);
	gtk_grid_attach (GTK_GRID (grid), arrow_eb, 0,0,1,1);

    arrow_right = gtk_image_new_from_icon_name ("pan-end-symbolic", GTK_ICON_SIZE_BUTTON);
    arrow_eb = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (arrow_eb), arrow_right);
    g_signal_connect (G_OBJECT (arrow_eb), "button-press-event",
                      G_CALLBACK (grisbi_win_headings_simpleclick_event_run),
                      gsb_gui_navigation_select_next);
	gtk_grid_attach (GTK_GRID (grid), arrow_eb, 1,0,1,1);

    /* Define labels. */
    priv->headings_title = gtk_label_new (NULL);
    gtk_label_set_justify (GTK_LABEL(priv->headings_title), GTK_JUSTIFY_LEFT);
    utils_labels_set_alignement (GTK_LABEL (priv->headings_title), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (grid), priv->headings_title, 2,0,1,1);

    priv->headings_suffix = gtk_label_new (NULL);
	utils_labels_set_alignement (GTK_LABEL (priv->headings_suffix), 0.0, 0.5);
	gtk_widget_set_hexpand (priv->headings_suffix, TRUE);
	gtk_widget_set_halign (priv->headings_suffix, GTK_ALIGN_END);
    gtk_grid_attach (GTK_GRID (grid), priv->headings_suffix, 3,0,1,1);

    gtk_container_add (GTK_CONTAINER (priv->headings_eb), grid);
}

/* HPANED_GENERAL */
/**
 * save hpahed width
 *
 * \param GtkWidget			hpaned
 * \param GtkAllocation 	allocation
 * \param gpointer			NULL
 *
 * \return FALSE
 */
static gboolean grisbi_win_hpaned_size_allocate (GtkWidget *hpaned_general,
                                          GtkAllocation *allocation,
                                          gpointer data)
{
	/* probleme avec la maximisation de la fenêtre */
#if 0

	GrisbiWinPrivate *priv = (GrisbiWinPrivate *) data;
	gint position;

	if (priv->hpaned_general_width == 0)
	{
		priv->hpaned_general_width = allocation->width;
		conf.panel_width = position;

		return FALSE;
	}

    position = gtk_paned_get_position (GTK_PANED (hpaned_general));

	if (allocation->width == priv->hpaned_general_width)
	{
		/* on modifie et mémorise la largeur du panel droit */
		conf.panel_width = position;
	}
	else
	{
		/* on conserve la largeur du panel droit */
		gtk_paned_set_position (GTK_PANED (hpaned_general), conf.panel_width);
	}
#endif

	return FALSE;
}

/* FORM_GENERAL */
/**
 * utile pour gérer la dimmension du formulaire
 *
 * \param GtkWidget			form
 * \param GtkAllocation 	allocation
 * \param gpointer			NULL
 *
 * \return FALSE
 */
static gboolean grisbi_win_form_size_allocate (GtkWidget *form_general,
                                               GtkAllocation *allocation,
                                               gpointer data)
{
	GtkWidget *form_expander;

	form_expander = g_object_get_data (G_OBJECT (form_general), "form_expander");
	if (form_expander)
	{
		GtkWidget *expander_label;

		conf.form_expander_label_width = 0.95*allocation->width;

		expander_label = gtk_expander_get_label_widget (GTK_EXPANDER(form_expander));
		gtk_widget_set_size_request (expander_label, conf.form_expander_label_width, -1);
	}

    return FALSE;
}

static gboolean grisbi_win_expander_label_set_initial_width (GtkWidget *form_general)
{
	GtkWidget *form_expander;
	GtkWidget *expander_label;


	form_expander = g_object_get_data (G_OBJECT (form_general), "form_expander");
	if (form_expander)
	{
		expander_label = gtk_expander_get_label_widget (GTK_EXPANDER(form_expander));
		gtk_widget_set_size_request (expander_label, conf.form_expander_label_width, -1);
	}

    return FALSE;
}
/* NAVIGATION PANE */
/**
 * utile pour gérer la dimmension de navigation_sw
 *
 * \param GtkWidget			navigation_sw
 * \param GtkAllocation 	allocation
 * \param gpointer			NULL
 *
 * \return FALSE
 */
static gboolean grisbi_win_navigation_sw_size_allocate (GtkWidget *navigation_sw,
                                          GtkAllocation *allocation,
                                          gpointer data)
{
	GrisbiWinPrivate *priv = (GrisbiWinPrivate *) data;

	priv->navigation_sw_allocation.width = allocation->width;
	priv->navigation_sw_allocation.height = allocation->height;
    return FALSE;
}

/**
 * utile pour gérer la dimmension du scheduler_calendar
 *
 * \param GtkWidget			scheduler_calendar
 * \param GtkAllocation 	allocation
 * \param gpointer			NULL
 *
 * \return FALSE
 */
static gboolean grisbi_win_scheduler_calendar_size_allocate (GtkWidget *scheduler_calendar,
                                          GtkAllocation *allocation,
                                          gpointer data)
{
	GrisbiWinPrivate *priv = (GrisbiWinPrivate *) data;

	priv->scheduler_calendar_allocation.width = allocation->width;
	priv->scheduler_calendar_allocation.height = allocation->height;
    return FALSE;
}

/* NOTEBOOK_GENERAL */
/**
 * utile pour gérer la dimmension du notebook general
 *
 * \param GtkWidget			notebook
 * \param GtkAllocation 	allocation
 * \param gpointer			NULL
 *
 * \return FALSE
 */
static gboolean grisbi_win_notebook_size_allocate (GtkWidget *notebook_general,
                                          GtkAllocation *allocation,
                                          gpointer data)
{
	GrisbiWinPrivate *priv = (GrisbiWinPrivate *) data;

	priv->notebook_general_allocation.width = allocation->width;
	priv->notebook_general_allocation.height = allocation->height;
    return FALSE;
}

/**
 * Init the main notebook :
 * a notebook which contains the pages : main page, accounts, scheduler... and
 * the form on the bottom, the form will be showed only for accounts page and
 * scheduler page
 *
 * \param GrisbiWin		active window
 *
 * \return GtkWidget	general_notebook
 */
static GtkWidget *grisbi_win_create_general_notebook (GrisbiWin *win)
{
	GtkWidget *grid;
	GtkWidget *sw;
	GrisbiWinPrivate *priv;

    devel_debug ("create_main_notebook");
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* the main right page is a grid with a notebook on the top and the form on the bottom */
	grid = gtk_grid_new ();

	/* adding a scrolled window for low resolution */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

  	gtk_grid_attach (GTK_GRID (grid), sw, 0,0,1,1);

	/* append the notebook */
    priv->notebook_general = gtk_notebook_new ();
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (priv->notebook_general), FALSE);
    gtk_notebook_set_show_border (GTK_NOTEBOOK (priv->notebook_general), FALSE);
	gtk_container_add (GTK_CONTAINER(sw), priv->notebook_general);

    gtk_widget_show (priv->notebook_general);
    g_signal_connect (G_OBJECT (priv->notebook_general),
                      "size_allocate",
                      G_CALLBACK (grisbi_win_notebook_size_allocate),
                      priv);

    /* append the form */
    priv->form_general = gsb_form_new ();
	gtk_grid_attach (GTK_GRID (grid), priv->form_general, 0,1,1,1);
	grisbi_win_expander_label_set_initial_width (priv->form_general);
    gtk_widget_hide (priv->form_general);
    g_signal_connect (G_OBJECT (priv->form_general),
                      "size_allocate",
                      G_CALLBACK (grisbi_win_form_size_allocate),
                      NULL);

	gtk_widget_show (sw);
    gtk_widget_show (grid);

    return grid;
}

/* VBOX_GENERAL */
/**
 *
 *
 * \param GrisbiWin		active window
 *
 * \return
 **/
static void grisbi_win_init_general_widgets (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	devel_debug (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* création de vbox_general */
    priv->vbox_general = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (priv->vbox_general);
}

/* NO_FILE_PAGE */
/**
 * page d'accueil si on ne charge pas un fichier automatiquement
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_no_file_page_new (GrisbiWin *win)
{
	gchar **recent_files_array;
	gint i;
    gint col = 0;
    gint row = 1;
	GrisbiWinPrivate *priv;

    devel_debug (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	gtk_container_set_border_width (GTK_CONTAINER (priv->no_file_page), MARGIN_BOX);

	/* set grid properties */
	gtk_grid_set_column_spacing (GTK_GRID (priv->no_file_grid), MARGIN_PADDING_BOX);
	gtk_grid_set_column_homogeneous (GTK_GRID (priv->no_file_grid), TRUE);
	gtk_grid_set_row_spacing (GTK_GRID (priv->no_file_grid), MARGIN_PADDING_BOX);
	gtk_grid_set_row_homogeneous (GTK_GRID (priv->no_file_grid), TRUE);
	gtk_widget_set_hexpand (priv->no_file_grid, TRUE);
	gtk_widget_set_vexpand (priv->no_file_grid, TRUE);

	gtk_container_add (GTK_CONTAINER (priv->no_file_sw), priv->no_file_grid);

	/* set action of fixed buttons */
	gtk_actionable_set_action_name (GTK_ACTIONABLE (priv->bouton_nouveau), "win.new-acc-file");
	gtk_actionable_set_action_name (GTK_ACTIONABLE (priv->bouton_ouvrir), "win.open-file");

	/* set the button "import" */
	priv->bouton_importer = utils_buttons_button_new_from_stock ("gtk-convert", _("Import"));
	gtk_button_set_image_position (GTK_BUTTON (priv->bouton_importer), GTK_POS_TOP);
	gtk_widget_set_size_request (priv->bouton_importer, 150, 150);
	gtk_widget_set_halign (priv->bouton_importer, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (priv->bouton_importer, GTK_ALIGN_CENTER);
	gtk_actionable_set_action_name (GTK_ACTIONABLE (priv->bouton_importer), "win.import-file");

	gtk_grid_attach (GTK_GRID (priv->no_file_grid), priv->bouton_importer, 2,0,1,1);
	gtk_widget_show (priv->bouton_importer);

	/* set the recent files buttons */
	recent_files_array = grisbi_app_get_recent_files_array ();
	for (i = 0; i < conf.nb_derniers_fichiers_ouverts; i++)
    {
		if (g_file_test (recent_files_array[i], G_FILE_TEST_EXISTS))
		{

			GtkWidget *bouton;
			gchar *tmp_str;
			gchar *target_value;

			if (g_utf8_strlen (recent_files_array[i], -1) > GSB_NAMEFILE_TOO_LONG)
			{
				gchar *basename;

				basename = g_path_get_basename (recent_files_array[i]);
				tmp_str = utils_str_break_filename (basename, GSB_NBRE_CHAR);
				g_free (basename);
			}
			else
				tmp_str = utils_str_break_filename (recent_files_array[i], GSB_NBRE_CHAR);

			bouton = utils_buttons_button_new_from_stock ("gtk-open", tmp_str);
			gtk_button_set_image_position (GTK_BUTTON (bouton), GTK_POS_TOP);
			gtk_widget_set_size_request (bouton, 150, 150);
			gtk_widget_set_halign (bouton, GTK_ALIGN_CENTER);
			gtk_widget_set_valign (bouton, GTK_ALIGN_CENTER);

			gtk_grid_attach (GTK_GRID (priv->no_file_grid), bouton, col,row,1,1);
			gtk_widget_show (bouton);

			/* set action */
			target_value = g_strdup_printf ("%d", i+1);
			gtk_actionable_set_action_target_value (GTK_ACTIONABLE (bouton), g_variant_new_string (target_value));
			gtk_actionable_set_action_name (GTK_ACTIONABLE (bouton), "win.direct-open-file");

			g_free (target_value);

			col++;
			if ((col % 3) == 0)
			{
				col = 0;
				row++;
			}

			g_free (tmp_str);
		}
	}

	/* finalisation de no_file_page */
	gtk_widget_show (priv->no_file_page);
}

/* WIN CALLBACK */
/**
 * check on any change on the main window
 * for now, only to check if we set/unset the full-screen
 *
 * \param window
 * \param event
 * \param null
 *
 * \return FALSE
 * */
static gboolean grisbi_win_change_state_window (GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null)
{
    gboolean show;

    if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
    {
        show = !(event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED);
        conf.maximize_screen = !show;
    }

    /* return value */
    return FALSE;
}

/* FREE STRUCTURES */
/**
 * libère la mémoire utilisée par w_etat
 *
 * \param object
 *
 * \return
 **/
static void grisbi_win_free_w_etat (GrisbiWinEtat *w_etat)
{
    devel_debug (NULL);

	/* variables generales */
	if (w_etat->accounting_entity)
		g_free (w_etat->accounting_entity);
	if (w_etat->adr_common)
		g_free (w_etat->adr_common);
	if (w_etat->adr_secondary)
		g_free (w_etat->adr_secondary);


    g_free (w_etat);
}

/**
 * libère la mémoire utilisée par w_run
 *
 * \param object
 *
 * \return
 **/
static void grisbi_win_free_w_run (GrisbiWinRun *w_run)
{

    devel_debug (NULL);

    g_free (w_run);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 *
 *
 * \param GrisbiWin *win
 *
 * \return
 **/
static void grisbi_win_init (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	/* initialisation des variables chaînes */
    priv->filename = NULL;
    priv->window_title = NULL;

    /* initialisations des widgets liés à grisbi_win_new_file_gui */
    priv->vbox_general = NULL;
    priv->notebook_general = NULL;
	priv->vbox_transactions_list = NULL;

    /* creation de la structure w_run */
    priv->w_run = g_malloc0 (sizeof (GrisbiWinRun));

    /* initialisation de la variable w_etat */
    priv->w_etat = g_malloc0 ( sizeof (GrisbiWinEtat));

    /* init widgets in grisbi_win.ui */
	gtk_widget_init_template (GTK_WIDGET (win));

	/* creation du widget stack_box->GtkStack */
	priv->stack_box = gtk_stack_new ();
	gtk_stack_set_transition_type (GTK_STACK (priv->stack_box), GTK_STACK_TRANSITION_TYPE_NONE);
	gtk_stack_set_vhomogeneous (GTK_STACK (priv->stack_box), TRUE);
	gtk_widget_show (priv->stack_box);
	gtk_box_pack_start (GTK_BOX (priv->main_box), priv->stack_box, TRUE, TRUE, 0);

	/* adding accueil_page */
	grisbi_win_no_file_page_new (win);
	if ((conf.dernier_fichier_auto && conf.last_open_file))
	{
		grisbi_win_init_general_widgets (win);
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->vbox_general, "file_page");
		(priv->w_run)->is_loading = TRUE;
	}
	else
	{
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->no_file_page, "accueil_page");
		(priv->w_run)->is_loading = FALSE;
	}

	/* initialisation de la barre d'état */
	grisbi_win_init_statusbar (GRISBI_WIN (win));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_finalize (GObject *object)
{
	GrisbiWinPrivate *priv;

    devel_debug (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (object));

    g_free (priv->filename);
    priv->filename = NULL;

    g_free (priv->window_title);
    priv->window_title = NULL;

    g_clear_object (&priv->builder);
    g_clear_object (&priv->menu);

    /* libération de la mémoiré utilisée par w_etat */
	grisbi_win_free_w_etat (priv->w_etat);

	/* libération mémoire de la structure run */
	grisbi_win_free_w_run (priv->w_run);

	G_OBJECT_CLASS (grisbi_win_parent_class)->finalize (object);
}


/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_class_init (GrisbiWinClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = grisbi_win_finalize;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
                                                 "/org/gtk/grisbi/ui/grisbi_win.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, main_box);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, statusbar);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, no_file_page);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, no_file_sw);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, no_file_grid);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, bouton_nouveau);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, bouton_ouvrir);

    /* signaux */
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_win_change_state_window);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *grisbi_win_get_filename (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;
	gchar *filename = NULL;

	if (win == NULL)
		win = grisbi_app_get_active_window (NULL);

	if (win)
	{
		priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
		filename = priv->filename;
	}

	return filename;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean grisbi_win_file_is_loading (void)
{
    GrisbiWin *win;
	GrisbiWinPrivate *priv;
	GrisbiWinRun *w_run;

	win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	w_run = priv->w_run;

	return w_run->is_loading;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_set_filename (GrisbiWin *win,
							  const gchar *filename)
{
	GrisbiWinPrivate *priv;

	if (!win)
        win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	if (priv->filename)
		g_free (priv->filename);

	if (filename)
	{
		priv->filename = g_strdup (filename);
	}
	else
	{
		priv->filename = NULL;
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gpointer grisbi_win_get_w_etat (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->w_etat;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gpointer grisbi_win_get_w_run (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->w_run;
}

/* GET WIDGET */
/**
 * retourne notebook_general
 *
 * \param GrisbiWin *win
 *
 * \return notebook_general
 **/
GtkWidget *grisbi_win_get_notebook_general (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    return priv->notebook_general;
}

/**
 * retourne prefs object
 *
 * \param GrisbiWin *win
 *
 * \return prefs_dialog
 **/
GtkWidget *grisbi_win_get_prefs_dialog (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	if (!win)
        win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    return priv->prefs_dialog;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void grisbi_win_set_prefs_dialog (GrisbiWin *win,
								  GtkWidget *prefs_dialog)
{
	GrisbiWinPrivate *priv;

	if (!win)
        win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	priv->prefs_dialog = prefs_dialog;
}

/**
 * retourne stack_box
 *
 * \param GrisbiWin *win
 *
 * \return stack_box
 **/
GtkWidget *grisbi_win_get_stack_box (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    return priv->stack_box;
}

/**
 * retourne vbox_transactions_list (ex tree_view_vbox)
 *
 * \param GrisbiWin *win
 *
 * \return vbox_transactions_list
 **/
GtkWidget *grisbi_win_get_vbox_transactions_list (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	if (win == NULL)
		win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    return priv->vbox_transactions_list;
}

/**
 * set vbox_transactions_list (ex tree_view_vbox)
 *
 * \param GrisbiWin *win
 * \param vbox
 *
 * \return
 **/
gboolean grisbi_win_set_vbox_transactions_list (GrisbiWin *win,
												GtkWidget *vbox)
{
	GrisbiWinPrivate *priv;

	if (win == NULL)
		win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	priv->vbox_transactions_list = vbox;

	return TRUE;
}

/* WIN_MENU */
/**
 *
 *
 * \param GrisbiWin 	win
 * \param GMenuModel  	menubar
 *
 * \return
 **/
void grisbi_win_init_menubar (GrisbiWin *win,
							  gpointer app)
{
	GAction *action;
    gchar * items[] = {
        "save",
        "save-as",
        "export-accounts",
        "create-archive",
        "export-archive",
        "debug-acc-file",
        "obf-acc-file",
        "debug-mode",
        "file-close",
        "edit-ope",
        "new-ope",
        "remove-ope",
        "template-ope",
        "clone-ope",
        "convert-ope",
        "new-acc",
        "remove-acc",
        "show-form",
        "show-reconciled",
        "show-archived",
        "show-closed-acc",
        "show-ope",
        "reset-width-col",
        NULL
    };
    gchar **tmp = items;
    gboolean has_app_menu;

	/* initialisations sub menus */
	action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-form");
    g_action_change_state (G_ACTION (action), g_variant_new_boolean (conf.formulaire_toujours_affiche));
	action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-closed-acc");
    g_action_change_state (G_ACTION (action), g_variant_new_boolean (conf.show_closed_accounts));

	/* disabled menus */
    while (*tmp)
    {
        gsb_menu_gui_sensitive_win_menu_item (*tmp, FALSE);

        tmp++;
    }

    /* sensibilise le menu new-window PROVISOIRE*/
    has_app_menu = grisbi_app_get_has_app_menu (GRISBI_APP (app));
	if (!has_app_menu)
		gsb_menu_gui_sensitive_win_menu_item ("new-window", FALSE);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_delete (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;
    GMenu *menu;
    GSList *tmp_list;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    if (priv->init_move_to_acc == FALSE)
        return;

    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
        gint i;

        i = gsb_data_account_get_no_account (tmp_list -> data);

        if (!gsb_data_account_get_closed_account (i))
        {
            gchar *tmp_name;

            tmp_name = g_strdup_printf ("move-to-acc%d", i);
            g_action_map_remove_action (G_ACTION_MAP (win), tmp_name);

            g_free (tmp_name);
        }
        tmp_list = tmp_list -> next;
    }

    menu = grisbi_app_get_menu_edit ();

    g_menu_remove (menu, 3);
    priv->init_move_to_acc = FALSE;
}

/**
 *
 *
 * \param GrisbiWin 	win
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_new (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;
    GAction *action;
    GMenu *menu;
    GMenu *submenu;
    GMenuItem *menu_item;
    GSList *tmp_list;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    menu = grisbi_app_get_menu_edit ();

    submenu = g_menu_new ();

    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
        gint i;

        i = gsb_data_account_get_no_account (tmp_list -> data);

        if (!gsb_data_account_get_closed_account (i))
        {
            gchar *tmp_name;
            gchar *account_name;
            gchar *action_name;

            tmp_name = g_strdup_printf ("move-to-acc%d", i);
            account_name = gsb_data_account_get_name (i);
            if (!account_name)
                account_name = _("Unnamed account");

            action = (GAction *) g_simple_action_new (tmp_name, NULL);
            g_signal_connect (action,
                        "activate",
                        G_CALLBACK (grisbi_cmd_move_to_account_menu),
                        GINT_TO_POINTER (i));
            g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

            g_action_map_add_action (G_ACTION_MAP (grisbi_app_get_active_window (NULL)), action);
            g_object_unref (G_OBJECT (action));

            action_name = g_strconcat ("win.", tmp_name, NULL);
            g_menu_append (submenu, account_name, action_name);

            g_free (tmp_name);
            g_free (action_name);
        }

        tmp_list = tmp_list -> next;
    }

    menu_item = g_menu_item_new_submenu (_("Move transaction to another account"), (GMenuModel*) submenu);
    g_menu_item_set_detailed_action (menu_item, "win.move-to-acc");
    action = g_action_map_lookup_action (G_ACTION_MAP (win), "move-to-acc");
    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

    g_menu_insert_item (G_MENU (menu), 3, menu_item);
    g_object_unref (menu_item);
    g_object_unref (submenu);
    priv->init_move_to_acc = TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_update (gboolean active)
{
    GrisbiWin *win;
    GAction *action;
    GSList *tmp_list;
    static gboolean flag_active = FALSE;

    if (flag_active == active)
        return;

    win = grisbi_app_get_active_window (NULL);

    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
        gint i;

        i = gsb_data_account_get_no_account (tmp_list -> data);

        if (!gsb_data_account_get_closed_account (i))
        {
            gchar *tmp_name;

            tmp_name = g_strdup_printf ("move-to-acc%d", i);
            action = g_action_map_lookup_action (G_ACTION_MAP (win), tmp_name);

            if (gsb_gui_navigation_get_current_account () == i)
            {
                g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);
                tmp_list = tmp_list -> next;
                continue;
            }
            if (active)
                g_simple_action_set_enabled (G_SIMPLE_ACTION (action), TRUE);
            else
                g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

            g_free (tmp_name);
        }
        tmp_list = tmp_list -> next;
    }
    flag_active = active;
}

/* MAIN WINDOW */
/**
 * Active la vue accueil ou la vue fichier
 *
 * \param GrisbiWin		active window
 * \param gchar 		nom de la page
 *
 * \return
 **/
void grisbi_win_stack_box_show (GrisbiWin *win,
							    const gchar *page_name)
{
	GrisbiWinPrivate *priv;

	if (win == NULL)
		win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	if (gtk_stack_get_child_by_name (GTK_STACK (priv->stack_box), page_name))
		gtk_stack_set_visible_child_name (GTK_STACK (priv->stack_box), page_name);
	else
	{
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->no_file_page, page_name);
		gtk_stack_set_visible_child_name (GTK_STACK (priv->stack_box), page_name);
	}
	if (strcmp (page_name, "accueil_page") == 0)
	{
		(priv->w_run)->is_loading = FALSE;
	}
	else
	{
		(priv->w_run)->is_loading = TRUE;
	}
}

/**
 * set the title of the window
 *
 * \param gint 		account_number
 *
 * \return			TRUE if OK, FALSE otherwise
 * */
gboolean grisbi_win_set_window_title (gint account_number)
{
	const gchar *filename;
	gchar *titre_grisbi = NULL;
	gboolean return_value;

	filename = grisbi_win_get_filename (NULL);
	if (filename == NULL)
    {
        titre_grisbi = g_strdup (_("Grisbi"));
        return_value = TRUE;
    }
    else
    {
		GrisbiWinEtat *w_etat;
		gchar *titre = NULL;
		gint tmp_number;

		devel_debug_int (account_number);
		w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

        switch (conf.display_window_title)
        {
            case GSB_ACCOUNT_ENTITY:
                if (w_etat->accounting_entity && strlen (w_etat->accounting_entity))
                    titre = g_strdup (w_etat->accounting_entity);
            break;
            case GSB_ACCOUNT_HOLDER:
            {
                if (account_number == -1)
                    tmp_number = gsb_data_account_first_number ();
                else
                    tmp_number = account_number;

                if (tmp_number == -1)
                {
                    if (w_etat->accounting_entity && strlen (w_etat->accounting_entity))
                        titre = g_strdup (w_etat->accounting_entity);
                }
                else
                {
                    titre = g_strdup (gsb_data_account_get_holder_name (tmp_number));

                    if (titre == NULL)
                        titre = g_strdup (gsb_data_account_get_name (tmp_number));
                }
				break;
            }
            case GSB_ACCOUNT_FILENAME:
                if (filename && strlen (filename))
                    titre = g_path_get_basename (filename);
				break;
        }

        if (titre && strlen (titre) > 0)
        {
            titre_grisbi = g_strconcat (titre, " - ", _("Grisbi"), NULL);
            g_free (titre);

            return_value = TRUE;
        }
        else
        {
            titre_grisbi = g_strconcat ("<", _("unnamed"), ">", NULL);
            return_value = FALSE;
        }
    }
    gtk_window_set_title (GTK_WINDOW (grisbi_app_get_active_window (NULL)), titre_grisbi);

    if (titre_grisbi && strlen (titre_grisbi) > 0)
    {
        gsb_main_page_update_homepage_title (titre_grisbi);
        g_free (titre_grisbi);
    }

    return return_value;
}

/**
 * set size and position of the main window of grisbi.
 *
 * \param GrisbiWin *win
 *
 * \return
 * */
void grisbi_win_set_size_and_position (GtkWindow *win)
{
	GrisbiWinPrivate *priv;
    GdkGeometry size_hints = {
    1200, 600, -1, -1, 1240, 640, 10, 10, 1.5, 1.5, GDK_GRAVITY_NORTH_WEST
  };

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* on fixe la taille mini remplacer main_box par plus pertinent.*/
    gtk_window_set_geometry_hints (GTK_WINDOW (win),
                                   priv->main_box,
                                   &size_hints,
                                   GDK_HINT_RESIZE_INC |
                                   GDK_HINT_MIN_SIZE |
                                   GDK_HINT_BASE_SIZE);

    /* set the size of the window */
    if (conf.main_width && conf.main_height)
        gtk_window_set_default_size (GTK_WINDOW (win), conf.main_width, conf.main_height);
    else
        gtk_window_set_default_size (GTK_WINDOW (win), 900, 600);

    /* display window at position */
    gtk_window_move (GTK_WINDOW (win), conf.x_position, conf.y_position);

    /* set the full screen if necessary */
    if (conf.full_screen)
        gtk_window_fullscreen (GTK_WINDOW (win));

    /* put up the screen if necessary */
    if (conf.maximize_screen)
        gtk_window_maximize (GTK_WINDOW (win));
}

/* NO_FILE_PAGE */
/**
 * update page d'accueil si on ferme le fichier
 *
 * \param
 *
 * \return
 **/
void grisbi_win_no_file_page_update (GrisbiWin *win)
{
	gchar **recent_files_array;
	gint i;
    gint col = 0;
    gint row = 1;
	GrisbiWinPrivate *priv;

    devel_debug (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	if ((priv->w_run)->is_loading == TRUE)
	{
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->no_file_page, "accueil_page");
		(priv->w_run)->is_loading = FALSE;
	}

	recent_files_array = grisbi_app_get_recent_files_array ();
	for (i = 0; i < conf.nb_derniers_fichiers_ouverts; i++)
	{
		GtkWidget *bouton;
        gchar *tmp_str;
		gchar *target_value;

		bouton = gtk_grid_get_child_at (GTK_GRID (priv->no_file_grid), col, row);
		if (bouton == NULL)
		{
			bouton = utils_buttons_button_new_from_stock ("gtk-open", _("Open"));
			gtk_button_set_image_position (GTK_BUTTON (bouton), GTK_POS_TOP);
			gtk_widget_set_size_request (bouton, 150, 150);
			gtk_widget_set_halign (bouton, GTK_ALIGN_CENTER);
			gtk_widget_set_valign (bouton, GTK_ALIGN_CENTER);

			gtk_grid_attach (GTK_GRID (priv->no_file_grid), bouton, col,row,1,1);
			gtk_widget_show (bouton);

			/* set action */
			target_value = g_strdup_printf ("%d", i+1);
			gtk_actionable_set_action_target_value (GTK_ACTIONABLE (bouton), g_variant_new_string (target_value));
			gtk_actionable_set_action_name (GTK_ACTIONABLE (bouton), "win.direct-open-file");

			g_free (target_value);
		}

		if (g_utf8_strlen (recent_files_array[i], -1) > GSB_NAMEFILE_TOO_LONG)
		{
			gchar *basename;

			basename = g_path_get_basename (recent_files_array[i]);
			tmp_str = utils_str_break_filename (basename, GSB_NBRE_CHAR);
			g_free (basename);
		}
		else
			tmp_str = utils_str_break_filename (recent_files_array[i], GSB_NBRE_CHAR);

		gtk_button_set_label (GTK_BUTTON (bouton), tmp_str);

		col++;
		if ((col % 3) == 0)
		{
			col = 0;
			row++;
		}
		g_free (tmp_str);
	}
}

/* VBOX_GENERAL */
/**
 * create vbox_general
 *
 * \param
 *
 * \return
 **/
GtkWidget *grisbi_win_create_general_widgets (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;
	GtkWidget *navigation_pane;

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* création de vbox_general */
	if (priv->vbox_general == NULL)
	{
		grisbi_win_init_general_widgets (win);
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->vbox_general, "file_page");
	}

    /* chargement de headings_eb */
    /* initialisation de headings_eb */
    grisbi_win_create_headings_eb (GRISBI_WIN (win));
    gtk_box_pack_start (GTK_BOX (priv->vbox_general), priv->headings_eb, FALSE, FALSE, 0);
    if (conf.show_headings_bar)
        gtk_widget_show_all (priv->headings_eb);
    else
        gtk_widget_hide (priv->headings_eb);

    /* Then create the main hpaned. */
    priv->hpaned_general = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	/* probleme avec la maximisation de la fenêtre */
    g_signal_connect (G_OBJECT (priv->hpaned_general),
                      "size_allocate",
                      G_CALLBACK (grisbi_win_hpaned_size_allocate),
                      priv);
    gtk_box_pack_start (GTK_BOX (priv->vbox_general), priv->hpaned_general, TRUE, TRUE, 0);

	/* recuperation des enfants pour gestion dimmension */
	navigation_pane = gsb_gui_navigation_create_navigation_pane ();
	priv->navigation_sw = gtk_grid_get_child_at (GTK_GRID (navigation_pane), 0, 0);
    g_signal_connect (G_OBJECT (priv->navigation_sw),
                      "size_allocate",
                      G_CALLBACK (grisbi_win_navigation_sw_size_allocate),
                      priv);
	priv->scheduler_calendar = gtk_grid_get_child_at (GTK_GRID (navigation_pane), 0, 1);
    g_signal_connect (G_OBJECT (priv->scheduler_calendar),
                      "size_allocate",
                      G_CALLBACK (grisbi_win_scheduler_calendar_size_allocate),
                      priv);

    /* fill the main hpaned. */
	gtk_paned_pack1 (GTK_PANED (priv->hpaned_general),
					 navigation_pane,
					 TRUE,
					 FALSE);
    gtk_paned_pack2 (GTK_PANED (priv->hpaned_general),
					 grisbi_win_create_general_notebook (win),
					 TRUE,
					 TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (priv->hpaned_general), MARGIN_BOX);

    if (conf.panel_width > 250)
        gtk_paned_set_position (GTK_PANED (priv->hpaned_general), conf.panel_width);
    else
    {
        gint width, height;

        gtk_window_get_size (GTK_WINDOW (win), &width, &height);
        gtk_paned_set_position (GTK_PANED (priv->hpaned_general), (gint) width / 4);
    }

    /* show the widgets */
    gtk_widget_show (priv->hpaned_general);
    gtk_widget_show (priv->vbox_general);

	/* set visible statusbar */
	if (!gtk_widget_get_visible (priv->statusbar))
		gtk_widget_show (priv->statusbar);

    return priv->vbox_general;
}

/**
 * free vbox_general
 *
 * \param
 *
 * \return
 **/
void grisbi_win_free_general_vbox (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    if (priv->vbox_general)
    {
        gtk_widget_destroy (priv->vbox_general);
        priv->vbox_general = NULL;
    }
}

/* NOTEBOOK_GENERAL */
/**
 * free notebook_general
 *
 * \param
 *
 * \return
 **/
void grisbi_win_free_general_notebook (void)
{
    GrisbiWin *win = NULL;
    GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    if (win == NULL)
        return;

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    if (priv->notebook_general)
        priv->notebook_general = NULL;
}

/* FILE_GUI */
/**
 * Initialize user interface part when a new accounts file is created.
 *
 * \param
 *
 * \return
 **/
void grisbi_win_new_file_gui (void)
{
    GrisbiWin *win;
    GtkWidget *tree_view_widget;
    GtkWidget *notebook_general;
	GtkWidget *vbox_transactions_list;

    win = grisbi_app_get_active_window (NULL);

    /* dégrise les menus nécessaire */
    gsb_menu_set_menus_with_file_sensitive (TRUE);

    /* récupère l'organisation des colonnes */
    recuperation_noms_colonnes_et_tips ();

    /* Create main widget. */
    grisbi_win_status_bar_message (_("Creating main window"));

    /* création de grid_general */
    grisbi_win_create_general_widgets (GRISBI_WIN (win));
	grisbi_win_stack_box_show (win, "file_page");

    /* fill the general notebook */
    notebook_general = grisbi_win_get_notebook_general (win);
    gsb_gui_fill_general_notebook (notebook_general);

    /* create the model */
    if (!transaction_list_create ())
    {
		dialogue_error (_("The model of the list couldn't be created... "
						  "Bad things will happen very soon..."));
		return;
    }

    /* Create transaction list. */
    tree_view_widget = gsb_transactions_list_make_gui_list ();
	vbox_transactions_list = grisbi_win_get_vbox_transactions_list (win);
    gtk_box_pack_start (GTK_BOX (vbox_transactions_list), tree_view_widget, TRUE, TRUE, 0);
    gtk_widget_show (tree_view_widget);

    /* Display accounts in menus */
	grisbi_win_menu_move_to_acc_delete ();
	grisbi_win_menu_move_to_acc_new ();

    gtk_notebook_set_current_page (GTK_NOTEBOOK(notebook_general), GSB_HOME_PAGE);

    gtk_widget_show (notebook_general);
}

/* HEADINGS */
/**
 * sensitive or unsensitive the headings
 *
 * \param sensitive TRUE to sensitive
 *
 * \return
 * */
void grisbi_win_headings_sensitive_headings (gboolean sensitive)
{
    GtkWidget *headings_eb;

    headings_eb = grisbi_win_get_headings_eb (NULL);
    gtk_widget_set_sensitive (headings_eb, sensitive);
}

/**
 * Display or hide the headings bar depending on configuration.
 *
 * \param
 *
 * \return		FALSE
 */
gboolean grisbi_win_headings_update_show_headings (void)
{
    GtkWidget *headings_eb;

    headings_eb = grisbi_win_get_headings_eb (NULL);
    if (conf.show_headings_bar)
    {
        gtk_widget_show_all (headings_eb);
    }
    else
    {
        gtk_widget_hide (headings_eb);
    }
    return FALSE;
}

/**
 * Update headings bar with a new title.
 *
 * \param title		String to display as a title in headings bar.
 *
 */
void grisbi_win_headings_update_title (gchar *title)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

    grisbi_win_headings_private_update_label_markup (GTK_LABEL (priv->headings_title), title, TRUE);
}

/**
 * Update headings bar with a new suffix.
 *
 * \param suffix	String to display as a suffix in headings bar.
 *
 */
void grisbi_win_headings_update_suffix (gchar *suffix)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));
    grisbi_win_headings_private_update_label_markup (GTK_LABEL (priv->headings_suffix), suffix, FALSE);
}

/* STATUS_BAR */
/**
 * Clear any message in the status bar.
 *
 * \param
 *
 * \return
 **/
void grisbi_win_status_bar_clear (void)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

    if (!priv->statusbar || !GTK_IS_STATUSBAR (priv->statusbar))
        return;

    if (priv->message_id < G_MAXUINT)
    {
        gtk_statusbar_remove (GTK_STATUSBAR (priv->statusbar), priv->context_id, priv->message_id);
        priv->message_id = G_MAXUINT;
    }

    /* force status message to be displayed NOW */
    update_gui ();
}

/**
 * Display a message in the status bar.
 *
 * \param message	Message to display.
 *
 * \return
 **/
void grisbi_win_status_bar_message (gchar *message)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

    if (!priv->statusbar || !GTK_IS_STATUSBAR (priv->statusbar))
        return;

    if (priv->message_id < G_MAXUINT)
        gtk_statusbar_remove (GTK_STATUSBAR (priv->statusbar), priv->context_id, priv->message_id);

    priv->message_id = gtk_statusbar_push (GTK_STATUSBAR (priv->statusbar), priv->context_id, message);

    /* force status message to be displayed NOW */
    update_gui ();
}

/**
 * Change current cursor to a animated watch (if animation supported
 * by environment).
 *
 * \param force_update		Call a gtk iteration to ensure cursor
 *			                is updated.  May cause trouble if
 *				            called from some signal handlers.
 *
 * \return
 **/
void grisbi_win_status_bar_wait (gboolean force_update)
{
    GrisbiWin *win;
	GrisbiWinPrivate *priv;
    GdkDisplay *display;
    GdkDevice *device;
    GdkWindow *current_window;
    GdkWindow *run_window;

    win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    run_window = gtk_widget_get_window (GTK_WIDGET (win));
    display = gdk_window_get_display (run_window);

#if GTK_CHECK_VERSION (3,20,0)
	GdkSeat *default_seat;

	default_seat = gdk_display_get_default_seat (display);
	device = gdk_seat_get_pointer (default_seat);
#else
    GdkDeviceManager *device_manager;

	device_manager = gdk_display_get_device_manager (display);
    device = gdk_device_manager_get_client_pointer (device_manager);
#endif

    gdk_window_set_cursor (run_window, gdk_cursor_new_for_display (display, GDK_WATCH));

    current_window = gdk_device_get_window_at_position (device, NULL, NULL);

    if (current_window && GDK_IS_WINDOW (current_window) && current_window != run_window)
    {
        GdkWindow *parent = gdk_window_get_toplevel (current_window);

        if (parent && parent != current_window)
        {
            current_window = parent;
        }

        gdk_window_set_cursor (current_window,
                               gdk_cursor_new_for_display (gdk_display_get_default (),
                                                           GDK_WATCH));

        priv->tracked_window = current_window;
    }

    if (force_update)
        update_gui ();
}


/**
 * Change current cursor to default cursor.
 *
 * \param force_update		Call a gtk iteration to ensure cursor
 *				            is updated.  May cause trouble if
 *				            called from some signal handlers.
 *
 * \return
 **/
void grisbi_win_status_bar_stop_wait (gboolean force_update)
{
    GrisbiWin *win;
	GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    if (!win)
        return;

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    if (priv->tracked_window && gdk_window_is_visible (priv->tracked_window))
    {
        gdk_window_set_cursor (priv->tracked_window, NULL);
        priv->tracked_window = NULL;
    }

	gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (win)), NULL);

    if (force_update)
        update_gui ();
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
