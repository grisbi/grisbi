/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2023 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                           */
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
#include "bet_data.h"
#include "bet_data_finance.h"
#include "bet_finance_ui.h"
#include "bet_hist.h"
#include "bet_tab.h"
#include "categories_onglet.h"
#include "etats_onglet.h"
#include "dialog.h"
#include "gsb_data_form.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_rgba.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "mouse.h"
#include "navigation.h"
#include "tiers_onglet.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_str.h"
#include "utils_widgets.h"
#include "widget_account_property.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct	_GrisbiWin			GrisbiWin;
typedef struct	_GrisbiWinPrivate	GrisbiWinPrivate;

struct _GrisbiWin
{
  GtkApplicationWindow parent;
};


struct _GrisbiWinPrivate
{
	GtkBuilder *		builder;

	/* box principale */
	GtkWidget *			main_box;
	GtkWidget *			stack_box;

	/* page d'accueil affichée si pas de fichier chargé */
	GtkWidget *			no_file_frame;
	GtkWidget *			no_file_grid;
	GtkWidget *			bouton_nouveau;
	GtkWidget *			bouton_ouvrir;
	GtkWidget *			bouton_importer;

	/* widgets si un fichier est chargé */
	GtkWidget *			hpaned_general;
	GtkWidget *			notebook_general;
	GtkWidget *			account_page;					/* account_page est un notebook qui contient les onglets du compte */
	GtkWidget *			scheduler_calendar;
	GtkWidget *			sw_general;
	GtkWidget *			vbox_general;

	GtkWidget *			form_general;
	GtkWidget *			form_frame;						/* frame of form */
	GtkWidget *			form_expander;					/* expander of form */
	GtkWidget *			form_hbox_label;				/* hbox pour le label de l'expander */
	GtkWidget *			form_label_last_statement;		/* label de la partie variable de l'expander */

	/* widgets de la fenêtre des operations */
	GtkWidget *			vbox_transactions_list;			/* vbox contenant le tree_view des opérations */

	/* widget account_property_page */
	GtkWidget *			account_property_page;

	/* nom du fichier associé à la fenêtre */
	gchar *				filename;

	/* titre de la fenêtre */
	gchar *				window_title;

	/* Menus et barres d'outils */
	/* menu move-to-acc */
	GMenu *				menu;
	gboolean			init_move_to_acc;				/* Si VRAI le sous menu des comptes est initialisé */

	/* statusbar */
	GtkWidget *			statusbar;
	GdkWindow *			tracked_window;
	guint				context_id;
	guint				message_id;
	gboolean			wait_state;

	/* headings_bar */
	GtkWidget *			headings_eb;
	GtkWidget *			headings_title;					/* Title for the heading bar. */
	GtkWidget *			headings_suffix;				/* Suffix for the heading bar. */

	/* paned */
	gint 				hpaned_general_width;

	/* form organization */
	gpointer 			form_organization;

	/* variables de configuration de la fenêtre */
	GrisbiWinEtat		*w_etat;

	/* structure run */
	GrisbiWinRun		*w_run;

	/* prefs dialog*/
	GtkWidget *			prefs_dialog;

	/* reconcile panel */
	 GtkWidget *		reconcile_panel;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrisbiWin, grisbi_win, GTK_TYPE_APPLICATION_WINDOW)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/* HEADINGS_EB */
/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
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
	if (button_event->type == GDK_BUTTON_PRESS)
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
	utils_labels_set_alignment (GTK_LABEL (priv->headings_title), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (grid), priv->headings_title, 2,0,1,1);

	priv->headings_suffix = gtk_label_new (NULL);
	utils_labels_set_alignment (GTK_LABEL (priv->headings_suffix), 0.0, 0.5);
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
static void grisbi_win_hpaned_size_allocate (GtkWidget *hpaned_general,
											 GtkAllocation *allocation,
											 GrisbiAppConf *a_conf)
{
	gint position;

	position = gtk_paned_get_position (GTK_PANED (hpaned_general));
	a_conf->panel_width = position;
}

/* FORM_GENERAL */
/**
 * Called when the state of the expander changes
 *
 * \param expander	Expanded that triggered signal.
 * \param null		Not used.
 *
 * \return FALSE
 * */
static gboolean grisbi_win_form_expander_activate (GtkWidget *expander,
												   GrisbiWin *win)
{
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
	a_conf = grisbi_app_get_a_conf ();

	if (gtk_expander_get_expanded (GTK_EXPANDER (expander)))
		gsb_form_expander_is_extanded (expander);
	else
		a_conf->formulaire_toujours_affiche = FALSE;

	gsb_menu_gui_toggle_show_form ();

	return FALSE;
}

/**
 * Create an empty form in an GtkExpander.
 *
 * \param
 *
 * \return Expander that contains form.
 **/
static GtkWidget *grisbi_win_form_new (GrisbiWin *win)
{
	GtkWidget *label;
	gchar *tmp_str;
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	devel_debug (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	a_conf = grisbi_app_get_a_conf ();

	/* Create the form frame */
	priv->form_frame = gtk_frame_new ("");

	/* Create the expander */
	priv->form_expander = gtk_expander_new ("");
	utils_widget_set_padding (priv->form_expander, MARGIN_BOX, MARGIN_BOX);

	gtk_expander_set_expanded (GTK_EXPANDER (priv->form_expander), a_conf->formulaire_toujours_affiche);
	gtk_container_add (GTK_CONTAINER (priv->form_frame), priv->form_expander);

	g_object_set_data (G_OBJECT (priv->form_frame), "form_expander", priv->form_expander);
	g_signal_connect_after (G_OBJECT(priv->form_expander),
							"activate",
							G_CALLBACK (grisbi_win_form_expander_activate),
							win);

	/* create the label of expander */
	/* Expander has a composite label */
	priv->form_hbox_label = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_expander_set_label_widget (GTK_EXPANDER(priv->form_expander), priv->form_hbox_label);
	gtk_expander_set_label_fill (GTK_EXPANDER(priv->form_expander), FALSE);
	gtk_expander_set_resize_toplevel (GTK_EXPANDER(priv->form_expander), FALSE);

	/* set the label transaction form */
	label = gtk_label_new (NULL);
	tmp_str = dialogue_make_pango_attribut ("weight=\"bold\"", _("Transaction/Scheduled _form"));
	gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), tmp_str);
	g_free (tmp_str);
	gtk_box_pack_start (GTK_BOX (priv->form_hbox_label), label, FALSE, FALSE, 0);

	/* set the last statement label */
	priv->form_label_last_statement = gtk_label_new (NULL);
	gtk_box_pack_end (GTK_BOX (priv->form_hbox_label), priv->form_label_last_statement, FALSE, FALSE, 0);

	gtk_widget_show_all (priv->form_hbox_label);

	gsb_form_create_widgets ();

	gtk_widget_hide (priv->form_frame);

	return priv->form_frame;
}

/* NAVIGATION PANE */
/* NOTEBOOK_GENERAL */
/**
 * fill the notebook given in param
 *
 * \param notebook a notebook (usually the main_notebook...)
 *
 * \return FALSE
 * */
static gboolean grisbi_win_fill_general_notebook (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	devel_debug (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	/* append the main page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  creation_onglet_accueil(),
							  gtk_label_new (_("Main page")));

	/* append the account page : a notebook with the account configuration
	 * the bet pages and transactions page */
	priv->account_page = gtk_notebook_new ();
	gtk_notebook_set_show_border (GTK_NOTEBOOK(priv->account_page), TRUE);
	gtk_widget_show (priv->account_page);

	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  priv->account_page,
							  gtk_label_new (_("Accounts")));

	gtk_notebook_append_page (GTK_NOTEBOOK (priv->account_page),
							  gsb_transactions_list_creation_fenetre_operations (),
							  gtk_label_new (_("Transactions")));

	/* append the balance estimate pages */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->account_page),
							  bet_array_create_page (),
							  gtk_label_new (_("Forecast")));

	gtk_notebook_append_page (GTK_NOTEBOOK (priv->account_page),
							  bet_hist_create_page (),
							  gtk_label_new (_("Historical data")));

	/* append the amortization page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->account_page),
							  bet_finance_ui_create_account_amortization_page (),
							  gtk_label_new (_("Amortization array")));

	/* append account_property_page */
	priv->account_property_page = GTK_WIDGET (widget_account_property_new ());
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->account_page),
							  priv->account_property_page,
							  gtk_label_new (_("Properties")));

	g_signal_connect (G_OBJECT (priv->account_page),
					  "switch-page",
					  G_CALLBACK (grisbi_win_on_account_switch_page),
					  NULL);

	/* append the scheduled transactions page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  gsb_scheduler_list_create_list (),
							  gtk_label_new (_("Scheduler")));

	/* append the payee page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  payees_create_list (),
							  gtk_label_new (_("Payee")));

	/* append the financial page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  bet_finance_ui_create_loan_simulator (),
							  gtk_label_new (_("Credits simulator")));

	/* append the categories page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  categories_create_list (),
							  gtk_label_new (_("Categories")));

	/* append the budget page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  budgetary_lines_create_list (),
							  gtk_label_new (_("Budgetary lines")));

	/* append the reports page */
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_general),
							  etats_onglet_create_reports_tab (),
							  gtk_label_new (_("Reports")));

	/* Set the signal for the navigation treeview selection when the notebook_general is filled */
	gsb_gui_navigation_tree_view_selection_changed ();

	/* update toolbars */
	grisbi_win_update_all_toolbars ();

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
	GtkWidget *sw_general;
	GtkWidget *grid;
	GrisbiWinPrivate *priv;

	devel_debug ("create_main_notebook");
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	sw_general = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw_general),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);

	/* the main right page is a grid with a notebook on the top and the form on the bottom */
	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER(sw_general), grid);
	gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);

	/* append the notebook */
	priv->notebook_general = gtk_notebook_new ();
	gtk_widget_set_margin_end (priv->notebook_general, MARGIN_END);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (priv->notebook_general), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (priv->notebook_general), FALSE);
  	gtk_grid_attach (GTK_GRID (grid), priv->notebook_general, 0,0,1,1);


	/* append the form */
	priv->form_general = grisbi_win_form_new (win);
	gtk_widget_set_margin_end (priv->form_general, MARGIN_END);
	gtk_grid_attach (GTK_GRID (grid), priv->form_general, 0,1,1,1);
	gtk_widget_hide (priv->form_general);

	/* show widgets */
	gtk_widget_show (sw_general);
	gtk_widget_show (grid);
	gtk_widget_show (priv->notebook_general);

	return sw_general;
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

/**
 * create vbox_general
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_create_general_widgets (GrisbiWin *win)
{
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	a_conf = grisbi_app_get_a_conf ();

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
	if (a_conf->show_headings_bar)
		gtk_widget_show_all (priv->headings_eb);
	else
		gtk_widget_hide (priv->headings_eb);

	/* Then create the main hpaned. */
	priv->hpaned_general = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	g_signal_connect (G_OBJECT (priv->hpaned_general),
					  "size-allocate",
					  G_CALLBACK (grisbi_win_hpaned_size_allocate),
					  a_conf);
	gtk_box_pack_start (GTK_BOX (priv->vbox_general), priv->hpaned_general, TRUE, TRUE, 0);

	/* fill the main hpaned. */
	gtk_paned_pack1 (GTK_PANED (priv->hpaned_general),
					 gsb_gui_navigation_create_navigation_pane (),
					 TRUE,
					 FALSE);

	priv->sw_general = grisbi_win_create_general_notebook (win);
	gtk_paned_pack2 (GTK_PANED (priv->hpaned_general),
					 priv->sw_general,
					 TRUE,
					 FALSE);
	gtk_widget_set_margin_bottom (priv->hpaned_general, MARGIN_BOTTOM);
	gtk_widget_set_margin_start (priv->hpaned_general, MARGIN_START);
	gtk_widget_set_margin_top (priv->hpaned_general, MARGIN_TOP);

	if (a_conf->panel_width > PANEL_MIN_WIDTH)
		gtk_paned_set_position (GTK_PANED (priv->hpaned_general), a_conf->panel_width);
	else
	{
		gint width, height;

		gtk_window_get_size (GTK_WINDOW (win), &width, &height);
		gtk_paned_set_position (GTK_PANED (priv->hpaned_general), (gint) width / 4);
	}

	/* show the widgets */
	gtk_widget_show (priv->hpaned_general);
	gtk_widget_show (priv->vbox_general);
}

/* NO_FILE_PAGE */
/**
 * suppression des boutons avant update de la vue
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_win_no_file_page_remove_buttons (GtkWidget *button,
													gpointer null)
{
	const gchar *data;

	data = g_object_get_data (G_OBJECT (button), "button_fixed");

	/* suppression des boutons avec data != "button_fixed" */
	if (g_strcmp0 (data, "button_fixed"))
	{
		gtk_widget_destroy (button);
	}
}

/**
 * called for a key-press-event on the account_button
 *
 * \param button
 * \param ev
 * \param filename
 *
 * \return FALSE or TRUE, depends if need to block the signal
 **/
static gboolean grisbi_win_account_button_press_event (GtkWidget *button,
													   GdkEventButton *ev,
													   const gchar *filename)
{
	if (ev->button == LEFT_BUTTON)
		return FALSE;
	else if (ev->button == RIGHT_BUTTON)
	{
		GtkWidget *menu;
		GtkWidget *menu_item;
		menu = gtk_menu_new ();
		menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Delete button")));
		g_signal_connect_swapped (G_OBJECT(menu_item),
								  "activate",
								  G_CALLBACK (utils_files_remove_name_to_recent_array),
								  g_strdup (filename));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Delete file")));
		g_signal_connect_swapped (G_OBJECT(menu_item),
								  "activate",
								  G_CALLBACK (gsb_file_remove_account_file),
								  g_strdup (filename));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		gtk_widget_show_all (menu);
		gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);

		return TRUE;
	}
	else
		return TRUE;
}

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
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	devel_debug (NULL);
	a_conf = grisbi_app_get_a_conf ();
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	gtk_container_set_border_width (GTK_CONTAINER (priv->no_file_frame), MARGIN_BOX);

	/* set grid properties */
	gtk_grid_set_column_spacing (GTK_GRID (priv->no_file_grid), MARGIN_PADDING_BOX);
	gtk_grid_set_column_homogeneous (GTK_GRID (priv->no_file_grid), TRUE);
	gtk_grid_set_row_spacing (GTK_GRID (priv->no_file_grid), MARGIN_PADDING_BOX);
	gtk_grid_set_row_homogeneous (GTK_GRID (priv->no_file_grid), TRUE);
	gtk_widget_set_hexpand (priv->no_file_grid, TRUE);
	gtk_widget_set_vexpand (priv->no_file_grid, TRUE);

	/* set bouton nouveau */
	priv->bouton_nouveau = utils_buttons_button_new_from_image ("gsb-new-file-24.png");
	gtk_button_set_label (GTK_BUTTON (priv->bouton_nouveau), _("New"));
	gtk_button_set_image_position (GTK_BUTTON (priv->bouton_nouveau), GTK_POS_TOP);
	gtk_widget_set_tooltip_text (priv->bouton_nouveau, _("Create a new account file"));
	gtk_widget_set_size_request (priv->bouton_nouveau, 150, 150);
	gtk_widget_set_halign (priv->bouton_nouveau, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (priv->bouton_nouveau, GTK_ALIGN_CENTER);
	gtk_actionable_set_action_name (GTK_ACTIONABLE (priv->bouton_nouveau), "win.new-acc-file");
	g_object_set_data_full (G_OBJECT (priv->bouton_nouveau), "button_fixed", g_strdup ("button_fixed"), g_free);
	gtk_grid_attach (GTK_GRID (priv->no_file_grid), priv->bouton_nouveau, 0,0,1,1);
	gtk_widget_show (priv->bouton_nouveau);

	/* set bouton ouvrir */
	priv->bouton_ouvrir = utils_buttons_button_new_from_image ("gtk-open-24.png");
	gtk_button_set_label (GTK_BUTTON (priv->bouton_ouvrir), _("Open"));
	gtk_button_set_image_position (GTK_BUTTON (priv->bouton_ouvrir), GTK_POS_TOP);
	gtk_widget_set_tooltip_text (priv->bouton_ouvrir, _("Open an existing account file"));
	gtk_widget_set_size_request (priv->bouton_ouvrir, 150, 150);
	gtk_widget_set_halign (priv->bouton_ouvrir, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (priv->bouton_ouvrir, GTK_ALIGN_CENTER);
	gtk_actionable_set_action_name (GTK_ACTIONABLE (priv->bouton_ouvrir), "win.open-file");
	g_object_set_data_full (G_OBJECT (priv->bouton_ouvrir), "button_fixed", g_strdup ("button_fixed"), g_free);
	gtk_grid_attach (GTK_GRID (priv->no_file_grid), priv->bouton_ouvrir, 1,0,1,1);
	gtk_widget_show (priv->bouton_ouvrir);

	/* set the button "import" */
	priv->bouton_importer = utils_buttons_button_new_from_image ("gsb-convert-24.png");
	gtk_button_set_label (GTK_BUTTON (priv->bouton_importer), _("Import"));
	gtk_button_set_image_position (GTK_BUTTON (priv->bouton_importer), GTK_POS_TOP);
	gtk_widget_set_tooltip_text (priv->bouton_importer, _("Import a CSV, QIF, OFX file ..."));
	gtk_widget_set_size_request (priv->bouton_importer, 150, 150);
	gtk_widget_set_halign (priv->bouton_importer, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (priv->bouton_importer, GTK_ALIGN_CENTER);
	gtk_actionable_set_action_name (GTK_ACTIONABLE (priv->bouton_importer), "win.import-file");
	g_object_set_data_full (G_OBJECT (priv->bouton_importer), "button_fixed", g_strdup ("button_fixed"), g_free);
	gtk_grid_attach (GTK_GRID (priv->no_file_grid), priv->bouton_importer, 2,0,1,1);
	gtk_widget_show (priv->bouton_importer);

	/* set the recent files buttons */
	recent_files_array = grisbi_app_get_recent_files_array ();
	for (i = 0; i < a_conf->nb_derniers_fichiers_ouverts; i++)
	{
		if (g_file_test (recent_files_array[i], G_FILE_TEST_EXISTS))
		{

			GtkWidget *bouton;
			gchar *tmp_str;
			gchar *target_value;
			gchar *basename;

			basename = g_path_get_basename (recent_files_array[i]);
			tmp_str = utils_str_break_filename (basename, GSB_NBRE_CHAR_TRUNC);
			g_free (basename);

			bouton = utils_buttons_button_new_from_image ("gtk-open-24.png");
			gtk_button_set_label (GTK_BUTTON (bouton), tmp_str);
			gtk_widget_set_tooltip_text (bouton, recent_files_array[i]);
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

			g_signal_connect (bouton,
							  "button-press-event",
							  G_CALLBACK (grisbi_win_account_button_press_event),
							  recent_files_array[i]);

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
	gtk_widget_show_all (priv->no_file_frame);
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
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();

	/* en premier on fixe a_conf->maximize_screen */
	if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
	{
		gboolean show;

		show = !(event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED);
		a_conf->maximize_screen = !show;
	}

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

	/* on libère la mémoire utilisée par etat */
	free_variables ();

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

	g_free (w_run->prefs_selected_row);
	g_free (w_run->import_format_date);
	w_run->import_format_date = NULL;

	/* free reconcile data */
	if (w_run->reconcile_final_balance)
		g_free (w_run->reconcile_final_balance);
	if (w_run->reconcile_new_date)
		g_date_free (w_run->reconcile_new_date);

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
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	/* initialisation des variables chaînes */
	priv->filename = NULL;
	priv->window_title = NULL;

	/* initialisations des widgets liés à grisbi_win_new_file_gui */
	priv->vbox_general = NULL;
	priv->notebook_general = NULL;
	priv->form_expander = NULL;
	priv->vbox_transactions_list = NULL;

	priv->account_property_page = NULL;

	/* creation et initialisation de la structure w_run */
	priv->w_run = g_malloc0 (sizeof (GrisbiWinRun));
	(priv->w_run)->account_number_is_0 = FALSE;
	(priv->w_run)->display_one_line = 0;					/* fixes bug 1875 */
	(priv->w_run)->new_account_file = FALSE;
	(priv->w_run)->prefs_expand_tree = TRUE;
	(priv->w_run)->prefs_selected_row = g_strdup ("0:0");

	/* initialisation de la variable w_etat */
	priv->w_etat = g_malloc0 (sizeof (GrisbiWinEtat));
	(priv->w_etat)->metatree_add_archive_in_totals = TRUE;	/* add the archived transactions by default */
	(priv->w_etat)->export_quote_dates = TRUE;				/* "cite les dates" TRUE par défaut */

	/* init widgets in grisbi_win.ui */
	gtk_widget_init_template (GTK_WIDGET (win));

	/* set font size of statusbar */
	gtk_widget_set_name (priv->statusbar, "global_statusbar");

	/* adding accueil_page */
	grisbi_win_no_file_page_new (win);
	if ((a_conf->dernier_fichier_auto && a_conf->last_open_file))
	{
		grisbi_win_init_general_widgets (win);
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->vbox_general, "file_page");
	}
	else
	{
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->no_file_frame, "accueil_page");
		(priv->w_run)->file_is_loading = FALSE;
	}

	/* initialisation de la barre d'état */
	if (!a_conf->low_definition_screen)
		grisbi_win_status_bar_init (GRISBI_WIN (win));

	/* initialisation du format de la date */
	(priv->w_etat)->date_format = gsb_date_initialise_format_date ();
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_dispose (GObject *object)
{
	G_OBJECT_CLASS (grisbi_win_parent_class)->dispose (object);
}


/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_class_init (GrisbiWinClass *class)
{
	G_OBJECT_CLASS (class)->dispose = grisbi_win_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
												 "/org/gtk/grisbi/ui/grisbi_win.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GrisbiWin, main_box);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GrisbiWin, stack_box);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GrisbiWin, statusbar);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GrisbiWin, no_file_frame);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GrisbiWin, no_file_grid);

	/* signaux */
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), grisbi_win_change_state_window);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * unused
 *
 * \param
 *
 * \return
 **/
void grisbi_win_open (GrisbiWin *win,
					  GFile *file)
{
	gchar *filename;

	devel_debug (NULL);
	filename = g_file_get_path (file);

	if (gsb_file_open_file (filename))
	{
		GrisbiWinPrivate *priv;
		GrisbiAppConf *a_conf;

		a_conf = grisbi_app_get_a_conf ();
		priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

		utils_files_append_name_to_recent_array (filename);
		(priv->w_run)->file_is_loading = TRUE;

		/* Si sauvegarde automatique on la lance ici */
		if (a_conf->make_backup_every_minutes
			&& a_conf->make_backup_nb_minutes)
			gsb_file_automatic_backup_start (NULL, NULL);

		grisbi_app_set_has_started_true ();
	}
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

	return w_run->file_is_loading;
}

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
 * retourne account_page
 *
 * \param
 *
 * \return account_page
 **/
GtkWidget *grisbi_win_get_account_page (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->account_page;
}

/**
 * retourne account_property_page
 *
 * \param
 *
 * \return account_page
 **/
GtkWidget *grisbi_win_get_account_property_page (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->account_property_page;
}

/**
 * retourne form_expander
 *
 * \param
 *
 * \return form_expander
 **/
GtkWidget *grisbi_win_get_form_expander (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->form_expander;
}

/**
 * retourne label_last_statement
 *
 * \param
 *
 * \return form_label_last_statement
 **/
GtkWidget *grisbi_win_get_label_last_statement (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->form_label_last_statement;
}

/**
 * retourne notebook_general
 *
 * \param GrisbiWin *win
 *
 * \return notebook_general
 **/
GtkWidget *grisbi_win_get_notebook_general (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
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
	if (win)
	{
		priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
		priv->prefs_dialog = prefs_dialog;
	}
}

/**
 * return reconcile panel
 *
 * \param
 *
 * \return
 **/
GtkWidget *grisbi_win_get_reconcile_panel (GrisbiWin *win)
{
	GrisbiWinPrivate *priv = NULL;

	if (!win)
		win = grisbi_app_get_active_window (NULL);

	if (win)
		priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	if (priv)
		return priv->reconcile_panel;
	else
		return NULL;
}

/**
 * set reconcile_panel
 *
 * \param
 * \param
 *
 * \return
 **/
void grisbi_win_set_reconcile_panel (GrisbiWin *win,
									 GtkWidget *reconcile_panel)
{
	GrisbiWinPrivate *priv;

	if (!win)
		win = grisbi_app_get_active_window (NULL);
	if (win)
	{
		priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
		priv->reconcile_panel = reconcile_panel;
	}
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
	const gchar * items[] = {
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
	const gchar **tmp = items;
	gboolean has_app_menu;
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();

	/* initialisations sub menus */
	action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-form");
	g_action_change_state (G_ACTION (action), g_variant_new_boolean (a_conf->formulaire_toujours_affiche));
	action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-closed-acc");
	g_action_change_state (G_ACTION (action), g_variant_new_boolean (a_conf->show_closed_accounts));

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

		i = gsb_data_account_get_no_account (tmp_list->data);

		if (!gsb_data_account_get_closed_account (i))
		{
			gchar *tmp_name;

			tmp_name = g_strdup_printf ("move-to-acc%d", i);
			g_action_map_remove_action (G_ACTION_MAP (win), tmp_name);

			g_free (tmp_name);
		}
		tmp_list = tmp_list->next;
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

		i = gsb_data_account_get_no_account (tmp_list->data);

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

		tmp_list = tmp_list->next;
	}

	menu_item = g_menu_item_new_submenu (_("Move transaction to another account"), (GMenuModel*) submenu);
	g_menu_item_set_detailed_action (menu_item, "win.move-to-acc");

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

	win = grisbi_app_get_active_window (NULL);

	tmp_list = gsb_data_account_get_list_accounts ();
	while (tmp_list)
	{
		gint i;

		i = gsb_data_account_get_no_account (tmp_list->data);

		if (!gsb_data_account_get_closed_account (i))
		{
			gchar *tmp_name;

			tmp_name = g_strdup_printf ("move-to-acc%d", i);
			action = g_action_map_lookup_action (G_ACTION_MAP (win), tmp_name);

			if (gsb_gui_navigation_get_current_account () == i)
			{
				g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);
				tmp_list = tmp_list->next;
				continue;
			}
			if (active)
				g_simple_action_set_enabled (G_SIMPLE_ACTION (action), TRUE);
			else
				g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

			g_free (tmp_name);
		}
		tmp_list = tmp_list->next;
	}
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

	devel_debug (page_name);
	if (win == NULL)
		win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	if (gtk_stack_get_child_by_name (GTK_STACK (priv->stack_box), page_name))
		gtk_stack_set_visible_child_name (GTK_STACK (priv->stack_box), page_name);
	else
	{
		gtk_stack_add_named (GTK_STACK (priv->stack_box), priv->no_file_frame, page_name);
		gtk_stack_set_visible_child_name (GTK_STACK (priv->stack_box), page_name);
	}
	if (strcmp (page_name, "accueil_page") == 0)
	{
		(priv->w_run)->file_is_loading = FALSE;
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
		gchar *titre = NULL;
		gint tmp_number;
		GrisbiAppConf *a_conf;
		GrisbiWinEtat *w_etat;

		devel_debug_int (account_number);
		a_conf = grisbi_app_get_a_conf ();
		w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

		switch (a_conf->display_window_title)
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
			return_value = TRUE;
		}
		else
		{
			titre_grisbi = g_strconcat ("<", _("unnamed"), ">", NULL);
			return_value = FALSE;
		}
		g_free (titre);
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
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();

	/* set the size of the window */
	if (a_conf->main_width && a_conf->main_height)
		gtk_window_set_default_size (GTK_WINDOW (win), a_conf->main_width, a_conf->main_height);
	else
		gtk_window_set_default_size (GTK_WINDOW (win), WIN_MIN_WIDTH, WIN_MIN_HEIGHT);

	/* display window at position */
	gtk_window_move (GTK_WINDOW (win), a_conf->x_position, a_conf->y_position);

	/* set the full screen if necessary */
	if (a_conf->full_screen)
		gtk_window_fullscreen (GTK_WINDOW (win));

	/* put up the screen if necessary */
	if (a_conf->maximize_screen)
		gtk_window_maximize (GTK_WINDOW (win));
}

/**
 * Remplace la fonction gtk_window_close () en libérant la mémoire utilisée
 * par la fenêtre quand on quitte l'application par le menu quitter.
 *
 * \param
 *
 * \return
 **/
void grisbi_win_close_window (GtkWindow *win)
{
	GrisbiWinPrivate *priv;

	devel_debug (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	g_free (priv->filename);
	priv->filename = NULL;

	g_free (priv->window_title);
	priv->window_title = NULL;

	g_free (priv->form_organization);

	g_clear_object (&priv->builder);
	g_clear_object (&priv->menu);

	/* libération de la mémoiré utilisée par w_etat */
	grisbi_win_free_w_etat (priv->w_etat);

	/* libération mémoire de la structure run */
	grisbi_win_free_w_run (priv->w_run);
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
	GtkWidget *bouton;
	gchar **recent_files_array;
	gint i;
	gint col = 0;
	gint row = 1;
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	devel_debug (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	a_conf = grisbi_app_get_a_conf ();

	if ((priv->w_run)->file_is_loading == TRUE)
	{
		grisbi_win_stack_box_show (win, "accueil_page");
	}

	recent_files_array = grisbi_app_get_recent_files_array ();
	if (!a_conf->nb_derniers_fichiers_ouverts)
	{
		bouton = gtk_grid_get_child_at (GTK_GRID (priv->no_file_grid), col, row);
		if (bouton)
			gtk_widget_destroy (bouton);

		return;
	}

	/* Suppression des boutons */
	gtk_container_foreach (GTK_CONTAINER (priv->no_file_grid),
						   (GtkCallback) grisbi_win_no_file_page_remove_buttons,
						   NULL);

	/* ajout des boutons restants */
	for (i = 0; i < a_conf->nb_derniers_fichiers_ouverts; i++)
	{
		gchar *tmp_str;
		gchar *target_value;
		gchar *basename;

		bouton = utils_buttons_button_new_from_icon_name ("gtk-open", _("Open"));
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

		g_signal_connect (bouton,
						  "button-press-event",
						  G_CALLBACK (grisbi_win_account_button_press_event),
						  recent_files_array[i]);

		g_free (target_value);

		basename = g_path_get_basename (recent_files_array[i]);
		tmp_str = utils_str_break_filename (basename, GSB_NBRE_CHAR_TRUNC);
		g_free (basename);

		gtk_button_set_label (GTK_BUTTON (bouton), tmp_str);
		gtk_widget_set_tooltip_text (bouton, recent_files_array[i]);

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
 * Set the main notebook page.
 *
 * \param page		Page to set.
 *
 * \return
 **/
void grisbi_win_general_notebook_set_page (gint page)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_general), page);
}

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

/* ACCOUNT_PAGE */
/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_free_account_property_page (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	priv->account_property_page = NULL;
}

/**
 * called when the account notebook changed page between
 * transactions list and account description
 *
 * \param notebook	Widget that triggered event.
 * \param page		Not used.
 * \param page_number	Page set.
 * \param null		Not used.
 *
 * \return		FALSE
 */
gboolean grisbi_win_on_account_switch_page (GtkNotebook *notebook,
											gpointer page,
											guint page_number,
											gpointer null)
{
	gint account_number;

	grisbi_win_menu_move_to_acc_update  (FALSE);
	if (page_number != GSB_TRANSACTIONS_PAGE)
	{
		gsb_menu_set_menus_view_account_sensitive (FALSE);
		gsb_menu_gui_sensitive_win_menu_item ("new-ope", FALSE);
	}

	switch (page_number)
	{
	case GSB_TRANSACTIONS_PAGE:
		grisbi_win_set_form_expander_visible (TRUE, TRUE);
		gsb_menu_set_menus_view_account_sensitive (TRUE);
		gsb_menu_gui_sensitive_win_menu_item ("new-ope", TRUE);
		break;
	case GSB_ESTIMATE_PAGE:
		grisbi_win_set_form_expander_visible (FALSE, FALSE);
		account_number = gsb_gui_navigation_get_current_account ();
		if (gsb_data_account_get_bet_maj (account_number))
			bet_data_update_bet_module (account_number, GSB_ESTIMATE_PAGE);
		gsb_menu_gui_sensitive_win_menu_item ("reset-width-col", TRUE);
		break;
	case GSB_HISTORICAL_PAGE:
		grisbi_win_set_form_expander_visible (FALSE, FALSE);
		account_number = gsb_gui_navigation_get_current_account ();
		if (gsb_data_account_get_bet_maj (account_number))
			bet_data_update_bet_module (account_number, GSB_HISTORICAL_PAGE);
		bet_hist_set_page_title (account_number);
		break;
	case GSB_FINANCE_PAGE:
		grisbi_win_set_form_expander_visible (FALSE, FALSE);
		account_number = gsb_gui_navigation_get_current_account ();
		bet_finance_update_amortization_tab (account_number);
		/* FALLTHRU */
	case GSB_PROPERTIES_PAGE:
		grisbi_win_set_form_expander_visible (FALSE, FALSE);
		break;
	}

	return (FALSE);
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
	GtkWidget *sw_transaction_list;
	GtkWidget *vbox_transactions_list;
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	/* dégrise les menus nécessaire */
	gsb_menu_set_menus_with_file_sensitive (TRUE);

	/* récupère l'organisation des colonnes */
	gsb_transactions_list_set_titles_tips_col_list_ope ();

	/* Create main widget. */
	grisbi_win_status_bar_message (_("Creating main window"));

	/* création de grid_general */
	grisbi_win_create_general_widgets (GRISBI_WIN (win));
	grisbi_win_stack_box_show (win, "file_page");

	/* fill the general notebook */
	grisbi_win_fill_general_notebook (GRISBI_WIN (win));

	/* create the model */
	if (!transaction_list_create ())
	{
		dialogue_error (_("The model of the list couldn't be created... "
						  "Bad things will happen very soon..."));
		return;
	}

	/* Create transaction list. */
	sw_transaction_list = gsb_transactions_list_make_gui_list ();
	vbox_transactions_list = grisbi_win_get_vbox_transactions_list (win);
	gtk_box_pack_start (GTK_BOX (vbox_transactions_list), sw_transaction_list, TRUE, TRUE, 0);
	gtk_widget_show (sw_transaction_list);

	/* Display accounts in menus */
	grisbi_win_menu_move_to_acc_delete ();
	grisbi_win_menu_move_to_acc_new ();

	gtk_notebook_set_current_page (GTK_NOTEBOOK(priv->notebook_general), GSB_HOME_PAGE);

	gtk_widget_show (priv->notebook_general);
}

/* FORM_GENERAL */
/**
 * hide the frame
 *
 * \param
 *
 * \return
 **/
void grisbi_win_form_expander_hide_frame (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	gtk_widget_hide (priv->form_frame);
}

/**
 * Check if transactions form is visible.
 *
 * \param
 *
 * \return TRUE if transactions forms is expanded, FALSE otherwise.
 */
gboolean grisbi_win_form_expander_is_expanded (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return gtk_expander_get_expanded (GTK_EXPANDER (priv->form_expander));
}
/**
 * show the frame
 *
 * \param
 *
 * \return
 **/
void grisbi_win_form_expander_show_frame (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	gtk_widget_show (priv->form_frame);
}

/**
 * Positionne le label des rapprochements à la fin du label de l'expander
 * utile juste pour les comptes.
 *
 * \param GtkAllocation 	allocation
 *
 * \return
 **/
void grisbi_win_form_label_align_right (GtkAllocation *allocation)
{
	GtkWidget *expander_label;
	gint new_width;
	gint sw_general_width;
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	sw_general_width = gtk_widget_get_allocated_width (priv->sw_general);
	new_width = sw_general_width - EXPANDER_LABEL_GAP;

	expander_label = gtk_expander_get_label_widget (GTK_EXPANDER(priv->form_expander));
	gtk_widget_set_size_request (expander_label, new_width, -1);
}

/**
 * get the form_organization
 *
 * \param
 *
 * \return form_organization
 **/
gpointer grisbi_win_get_form_organization (void)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->form_organization;
}

/**
 * show or hide the expander
 *
 * \param visible TRUE or FALSE
 * \param transactions_list TRUE if we are on transactions, FALSE if we are on scheduler
 *
 * return FALSE
 * */
gboolean grisbi_win_set_form_expander_visible (gboolean visible,
											   gboolean transactions_list)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	if (visible)
	{
		gtk_widget_show (priv->form_expander);
		gtk_widget_show (priv->form_frame);

		if (transactions_list)
			gtk_widget_show (priv->form_label_last_statement);
		else
			gtk_widget_hide (priv->form_label_last_statement);
	}
	else
	{
		gtk_widget_hide (priv->form_expander);
		gtk_widget_hide (priv->form_frame);
	}

	return FALSE;
}

/**
 * set the form_organization
 *
 * \param form_organization form_organization to set
 *
 * \return TRUE
 * */
gboolean grisbi_win_set_form_organization (gpointer form_organization)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	priv->form_organization = form_organization;

	return TRUE;
}

/**
 * show/hide the form according to the expander,
 * this will emit a signal as if the user changed it by himself
 *
 * \param
 *
 * \return FALSE
 * */
gboolean grisbi_win_switch_form_expander (void)
{
	gboolean is_visible;
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	if (!priv->form_expander)
		return FALSE;

	is_visible = gtk_expander_get_expanded (GTK_EXPANDER (priv->form_expander));
	gtk_expander_set_expanded (GTK_EXPANDER (priv->form_expander), !is_visible);

	return FALSE;
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
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();
	headings_eb = grisbi_win_get_headings_eb (NULL);
	if (a_conf->show_headings_bar)
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
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	a_conf = grisbi_app_get_a_conf ();
	if (!a_conf->show_headings_bar)
		return;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

	grisbi_win_headings_private_update_label_markup (GTK_LABEL (priv->headings_title), title, TRUE);
}

/**
 * Update headings bar with a new suffix.
 *
 * \param suffix	String to display as a suffix in headings bar.
 *
 */
void grisbi_win_headings_update_suffix (const gchar *suffix)
{
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	a_conf = grisbi_app_get_a_conf ();
	if (!a_conf->show_headings_bar)
		return;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));
	grisbi_win_headings_private_update_label_markup (GTK_LABEL (priv->headings_suffix), suffix, FALSE);
}

/* STATUS_BAR */
/**
 * initiate the GtkStatusBar to hold various status
 * information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
void grisbi_win_status_bar_init (GrisbiWin *win)
{
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	if (!win)
		win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	a_conf = grisbi_app_get_a_conf ();

	/* set wait_state */
	priv->wait_state = FALSE;

	/* set status_bar PROVISOIRE */
	priv->context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (priv->statusbar), "Grisbi");
	priv->message_id = G_MAXUINT;

	/* set visible statusbar if necessary */
	if (a_conf->low_definition_screen)
		gtk_widget_hide (priv->statusbar);
	else
		gtk_widget_show (priv->statusbar);
}


/**
 * Clear any message in the status bar.
 *
 * \param
 *
 * \return
 **/
void grisbi_win_status_bar_clear (void)
{
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));
	a_conf = grisbi_app_get_a_conf ();
	if (a_conf->low_definition_screen || !priv->statusbar || !GTK_IS_STATUSBAR (priv->statusbar))
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
	GrisbiAppConf *a_conf;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));
	a_conf = grisbi_app_get_a_conf ();
	if (a_conf->low_definition_screen || !priv->statusbar || !GTK_IS_STATUSBAR (priv->statusbar))
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
 *							is updated.  May cause trouble if
 *							called from some signal handlers.
 *
 * \return
 **/
void grisbi_win_status_bar_wait (gboolean force_update)
{
	GdkCursor *cursor;
	GdkDevice *device;
	GdkDisplay *display;
	GdkSeat *default_seat;
	GdkWindow *current_window;
	GdkWindow *run_window;
	GrisbiAppConf *a_conf;
	GrisbiWin *win;
	GrisbiWinPrivate *priv;

	a_conf = grisbi_app_get_a_conf ();
	if (a_conf->low_definition_screen)
		return;

	win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	/* set wait_state */
	priv->wait_state = TRUE;

	run_window = gtk_widget_get_window (GTK_WIDGET (win));
	display = gdk_window_get_display (run_window);
	cursor = gdk_cursor_new_for_display (display, GDK_HAND2);
	gdk_window_set_cursor (run_window, cursor);

	default_seat = gdk_display_get_default_seat (display);
	device = gdk_seat_get_pointer (default_seat);

	current_window = gdk_device_get_window_at_position (device, NULL, NULL);
	if (current_window && GDK_IS_WINDOW (current_window) && current_window != run_window)
	{
		GdkWindow *parent;

		parent = gdk_window_get_toplevel (current_window);
		if (parent && parent != current_window)
		{
			current_window = parent;
		}
		gdk_window_set_cursor (current_window, cursor);

		priv->tracked_window = current_window;
	}

	if (force_update)
		update_gui ();
}

/**
 * Change current cursor to default cursor.
 *
 * \param force_update		Call a gtk iteration to ensure cursor
 *							is updated.  May cause trouble if
 *							called from some signal handlers.
 *
 * \return
 **/
void grisbi_win_status_bar_stop_wait (gboolean force_update)
{
	GrisbiWin *win;
	GrisbiWinPrivate *priv;
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();
	if (a_conf->low_definition_screen)
		return;

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

	/* set wait_state */
	priv->wait_state = FALSE;

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
gboolean grisbi_win_status_bar_get_wait_state (void)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

	return priv->wait_state;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_status_bar_set_font_size (gint font_size)
{
	gchar *data = NULL;
	gchar *font_size_str;
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
	a_conf = grisbi_app_get_a_conf ();
	if (a_conf->low_definition_screen)
		return;

	font_size_str = utils_str_itoa (font_size);
	data = g_strconcat ("#global_statusbar {\n\tfont-size: ", font_size_str, "px;\n}\n",  NULL);
	gsb_css_set_property_from_name ("#global_statusbar", data);

	g_free (font_size_str);
	g_free (data);
}

/* TOOLBARS */
/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_update_all_toolbars (void)
{
	gint toolbar_style = 0;
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();
	switch (a_conf->display_toolbar)
	{
		case GTK_TOOLBAR_TEXT:
			toolbar_style = GTK_TOOLBAR_TEXT;
			break;
		case GTK_TOOLBAR_ICONS:
			toolbar_style = GTK_TOOLBAR_ICONS;
			break;
		case GTK_TOOLBAR_BOTH:
			toolbar_style = GTK_TOOLBAR_BOTH;
			break;
		case GTK_TOOLBAR_BOTH_HORIZ:
			toolbar_style = GTK_TOOLBAR_BOTH_HORIZ;
			break;
	}

	gsb_gui_transaction_toolbar_set_style (toolbar_style);
	gsb_gui_scheduler_toolbar_set_style (toolbar_style);
	gsb_gui_payees_toolbar_set_style (toolbar_style);
	gsb_gui_categories_toolbar_set_style (toolbar_style);
	gsb_gui_budgetary_lines_toolbar_set_style (toolbar_style);
	etats_onglet_reports_toolbar_set_style (toolbar_style);
	bet_array_update_toolbar (toolbar_style);
	bet_hist_update_toolbar (toolbar_style);
	bet_finance_ui_update_all_finance_toolbars (toolbar_style);
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
