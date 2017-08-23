/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cedric Auger (cedric@grisbi.org)            */
/*          2002-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
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
 * \file etats_onglet.c
 * Handle all UI actions for the reports.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "etats_onglet.h"
#include "etats_calculs.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "utils_file_selection.h"
#include "gsb_automem.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_file_others.h"
#include "navigation.h"
#include "gsb_report.h"
#include "traitement_variables.h"
#include "utils.h"
#include "etats_config.h"
#include "print_report.h"
#include "utils_files.h"
#include "structures.h"
#include "fenetre_principale.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

static GtkWidget *bouton_export_pdf_etat = NULL;
static GtkWidget *bouton_effacer_etat = NULL;
static GtkWidget *bouton_personnaliser_etat = NULL;
static GtkWidget *bouton_imprimer_etat = NULL;
static GtkWidget *bouton_exporter_etat = NULL;
static GtkWidget *bouton_dupliquer_etat = NULL;
GtkWidget *scrolled_window_etat = NULL;          /* contient l'état en cours */
gint nb_colonnes;
gint nb_lignes;
gint ligne_debut_partie;
GtkWidget *notebook_etats = NULL;
GtkWidget *notebook_config_etat = NULL;
static GtkWidget *reports_toolbar = NULL;
static gboolean maj_reports_list;

/*START_EXTERN*/
extern struct EtatAffichage csv_affichage;
extern struct EtatAffichage html_affichage;
/*END_EXTERN*/

/** Different formats supported.  */
enum report_export_formats {
    REPORT_EGSB, REPORT_HTML, REPORT_CSV,
    REPORT_MAX_FORMATS,
};

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/**
 * Callback triggered when user change the menu of template reports in
 * the "new report" dialog,
 *
 * \param menu_item		Menu item that triggered signal.
 * \param label_description	A GtkLabel to fill with the long
 *				description of template report.
 *
 * \return
 **/
static void etats_onglet_change_choix_nouvel_etat (GtkWidget *combobox,
												   GtkWidget *label_description)
{
    gchar *description;

    switch (gtk_combo_box_get_active (GTK_COMBO_BOX (combobox)))
    {
		case 0:
			/* Last month incomes and outgoings */

			description = _("This report displays totals for last month's transactions sorted by "
							"categories and sub-categories. You just need to select the account(s). "
							"By default, all accounts are selected.");
			break;

		case 1:
			/* Current month incomes and outgoings */

			description = _("This report displays totals of current month's transactions sorted by "
							"categories and sub-categories. You just need to select the account(s). "
							"By default, all accounts are selected.");
			break;

		case 2:
			/* Annual budget */

			description = _("This report displays annual budget. You just need to select the account(s). "
							"By default all accounts are selected.");
			break;

		case 3:
			/* Blank report */

			description = _("This report is an empty one. You need to customize it entirely.");
			break;

		case 4:
			/* Cheques deposit */

			description = _("This report displays the cheques deposit. You just need to select the account(s). "
							"By default all accounts are selected.");
			break;

		case 5:
			/* Monthly outgoings by payee */

			description = _("This report displays current month's outgoings sorted by payees. "
							"You just need to select the account(s). By default all accounts are selected.");
			break;

		case 6:
			/* Search */

			description = _("This report displays all the information for all transactions of all accounts "
							"for the current year. You just have to add the amount, date, payees etc. criteria "
							"that you want. By default the transactions are clickables.");
			break;

		default:

			description = _("No description available");
    }

    gtk_label_set_text (GTK_LABEL (label_description), description);

}

/**
 * Make a copy of current report and update user interface.
 *
 * \param
 *
 * \return
 **/
static void etats_onglet_dupliquer_etat (void)
{
    gint report_number, current_report_number;
    GtkWidget *notebook_general;

    current_report_number = gsb_gui_navigation_get_current_report ();

    notebook_general = gsb_gui_get_general_notebook ();
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

    report_number = gsb_data_report_dup (current_report_number);

    gsb_gui_navigation_add_report (report_number);
    etats_onglet_update_gui_to_report (report_number);

    etats_config_personnalisation_etat ();
    gsb_file_set_modified (TRUE);
}

/**
 * Export current report as a CSV file.  It uses a "benj's meta
 * structure" affichage_etat structure as a backend.
 *
 * \param filename		Filename to save report into.
 *
 * \return
 **/
static void etats_onglet_export_etat_courant_vers_csv (gchar *filename)
{
    GtkWidget *notebook_general;

    notebook_general = gsb_gui_get_general_notebook ();
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

    affichage_etat (gsb_gui_navigation_get_current_report (), &csv_affichage, filename);
}

/**
 * Export current report as a HTML file.  It uses a "benj's meta
 * structure" affichage_etat structure as a backend.
 *
 * \param filename		Filename to save report into.
 *
 * \return
 **/
static void etats_onglet_export_etat_courant_vers_html (gchar *filename)
{
    GtkWidget *notebook_general;

    notebook_general = gsb_gui_get_general_notebook ();
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

    affichage_etat (gsb_gui_navigation_get_current_report (), &html_affichage, filename);
}

/**
 * Callback of the file format menu in the export report dialog.  It
 * is responsible to change the "format" object property of the
 * dialog, which is used when it is closed to determine format.
 *
 * It is also responsible to change the default value of filename in
 * the selector.  For that, it uses the "basename" property set in the
 * chosser creation.  The "basename" property is the report name.
 *
 * \param combo		Combo box that triggered event.
 * \param selector	GtkFileChooser containing combo.
 *
 * \return FALSE
 */
static gboolean etats_onglet_report_export_change_format (GtkWidget *combo,
														  GtkWidget *selector)
{
    gchar *name;
    gchar *extension;
    gchar *tmp_str;

    g_object_set_data (G_OBJECT(selector), "format",
			GINT_TO_POINTER (gtk_combo_box_get_active (GTK_COMBO_BOX(combo))));

    name = utils_files_safe_file_name (g_object_get_data (G_OBJECT (selector), "basename"));
    switch (gtk_combo_box_get_active (GTK_COMBO_BOX(combo)))
    {
	    case REPORT_EGSB:		/* EGSB */
		extension = "egsb";
		break;

	    case REPORT_HTML:		/* HTML */
		extension = "html";
		break;

	    case REPORT_CSV:		/* CSV */
		extension = "csv";
		break;

	    default :
		extension = NULL;
		break;
    }

	tmp_str = g_strconcat (name, ".", extension, NULL);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(selector), tmp_str);
	g_free (tmp_str);

    return FALSE;
}

/**
 * This is responsible of exporting current report.  This function
 * will ask a filename, a format, and call the appropriate benj's meta
 * backend to do the job.
 *
 * \param
 *
 * \return
 **/
static void etats_onglet_exporter_etat (void)
{
    GtkWidget *fenetre_nom, *hbox, *combo;
    gint resultat;
    gchar *nom_etat;
    GtkWidget *notebook_general;
    gchar *tmp_last_directory;

    notebook_general = gsb_gui_get_general_notebook ();
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

    fenetre_nom = gtk_file_chooser_dialog_new (_("Export report"),
					   GTK_WINDOW (grisbi_app_get_active_window (NULL)),
					   GTK_FILE_CHOOSER_ACTION_SAVE,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-save", GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fenetre_nom), gsb_file_get_last_path ());
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (fenetre_nom), TRUE);
    gtk_window_set_position (GTK_WINDOW (fenetre_nom), GTK_WIN_POS_CENTER_ON_PARENT);

    g_object_set_data (G_OBJECT(fenetre_nom), "basename",
			 gsb_data_report_get_report_name (gsb_gui_navigation_get_current_report ()));

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX(hbox), gtk_label_new (_("File format: ")),
			 FALSE, FALSE, 0);

    combo = gtk_combo_box_text_new ();
    gtk_box_pack_start (GTK_BOX(hbox), combo, TRUE, TRUE, 0);
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), _("Grisbi report file (egsb file)"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), _("HTML file"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), _("CSV file"));

    /* Set initial format. */
    gtk_combo_box_set_active (GTK_COMBO_BOX(combo), REPORT_HTML);
    etats_onglet_report_export_change_format (combo, fenetre_nom);

    g_signal_connect (G_OBJECT(combo), "changed",
		       G_CALLBACK (etats_onglet_report_export_change_format),
		       fenetre_nom);
    gtk_widget_show_all (hbox);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(fenetre_nom), hbox);

    resultat = gtk_dialog_run (GTK_DIALOG (fenetre_nom));
    if (resultat == GTK_RESPONSE_OK)
    {
		nom_etat = file_selection_get_filename (GTK_FILE_CHOOSER (fenetre_nom));

		grisbi_win_status_bar_message (_("Exporting report..."));

		grisbi_win_status_bar_wait (TRUE);

		switch (GPOINTER_TO_INT(g_object_get_data (G_OBJECT(fenetre_nom), "format")))
		{
			case REPORT_EGSB:		/* EGSB */
			gsb_file_others_save_report (nom_etat);
			break;

			case REPORT_HTML:		/* HTML */
			etats_onglet_export_etat_courant_vers_html (nom_etat);
			break;

			case REPORT_CSV:		/* CSV */
			etats_onglet_export_etat_courant_vers_csv (nom_etat);
			break;

			default :
			break;
		}

		g_free (nom_etat);

		grisbi_win_status_bar_stop_wait (TRUE);
		grisbi_win_status_bar_message (_("Done"));
    }
    tmp_last_directory = file_selection_get_last_directory (GTK_FILE_CHOOSER (fenetre_nom), TRUE);
    gsb_file_update_last_path (tmp_last_directory);
    g_free (tmp_last_directory);
    gtk_widget_destroy (GTK_WIDGET (fenetre_nom));
}

/**
 * Import a report.
 *
 * \param
 *
 * \return
 **/
static void etats_onglet_importer_etat (void)
{
    GtkWidget *fenetre_nom;
    GtkWidget *notebook_general;
    gint resultat;
    gchar *nom_etat;
    GtkFileFilter *filter;
    gchar *tmp_last_directory;

    notebook_general = gsb_gui_get_general_notebook ();
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
	gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

    fenetre_nom = gtk_file_chooser_dialog_new (_("Import a report"),
					   GTK_WINDOW (grisbi_app_get_active_window (NULL)),
					   GTK_FILE_CHOOSER_ACTION_OPEN,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-open", GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fenetre_nom), gsb_file_get_last_path ());
    gtk_window_set_position (GTK_WINDOW (fenetre_nom), GTK_WIN_POS_CENTER_ON_PARENT);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("Grisbi report files (*.egsb)"));
    gtk_file_filter_add_pattern (filter, "*.egsb");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fenetre_nom), filter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fenetre_nom), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fenetre_nom), filter);

    resultat = gtk_dialog_run (GTK_DIALOG (fenetre_nom));

    switch (resultat)
    {
		case GTK_RESPONSE_OK :
			nom_etat =file_selection_get_filename (GTK_FILE_CHOOSER (fenetre_nom));
			tmp_last_directory = file_selection_get_last_directory (GTK_FILE_CHOOSER (fenetre_nom), TRUE);
			gsb_file_update_last_path (tmp_last_directory);
			g_free (tmp_last_directory);
			gtk_widget_destroy (GTK_WIDGET (fenetre_nom));

			/* la vérification que c'est possible a été faite par la boite de selection*/
			if (!gsb_file_others_load_report (nom_etat))
			{
				g_free (nom_etat);
				return;
			}
			g_free (nom_etat);
			break;

		default :
			gtk_widget_destroy (GTK_WIDGET (fenetre_nom));
			return;
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void etats_onglet_bouton_export_pdf_etat_clicked (void)
{
	gint report_number;
	gchar *export_pdf_name;
	gchar *filename;

	devel_debug (NULL);
	report_number = gsb_gui_navigation_get_current_report ();
	export_pdf_name = gsb_data_report_get_export_pdf_name (report_number);
	filename = g_build_filename (gsb_dirs_get_default_dir (), export_pdf_name, NULL);
	print_report_export_pdf (filename);
	g_free (filename);
}

/**
 * Create a toolbar containing all necessary controls on reports tab.
 *
 * \param
 *
 * \return a newly-allocated hbox
 **/
static GtkWidget *etats_onglet_create_reports_toolbar (void)
{
    GtkWidget *toolbar;
    GtkToolItem *item;

    toolbar = gtk_toolbar_new ();

    /* New report button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-new-report-24.png", _("New report"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Create a new report"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (etats_onglet_ajoute_etat),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* Import button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-import-24.png", _("Import"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Import a Grisbi report file (.egsb)"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (etats_onglet_importer_etat),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* Export button */
    bouton_exporter_etat = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gsb-export-24.png", _("Export")));
    gtk_widget_set_tooltip_text (GTK_WIDGET (bouton_exporter_etat),
                        _("Export selected report to egsb, HTML, Tex, CSV, PostScript"));
    g_signal_connect (G_OBJECT (bouton_exporter_etat),
                        "clicked",
                        G_CALLBACK (etats_onglet_exporter_etat),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (bouton_exporter_etat), -1);

    /* print button */
    bouton_imprimer_etat = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-print-24.png", _("Print")));
    gtk_widget_set_tooltip_text (GTK_WIDGET (bouton_imprimer_etat), _("Print selected report"));
    g_signal_connect (G_OBJECT (bouton_imprimer_etat),
                        "clicked",
                        G_CALLBACK (print_report),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (bouton_imprimer_etat), -1);

    /* delete button */
    bouton_effacer_etat = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-delete-24.png", _("Delete")));
    gtk_widget_set_tooltip_text (GTK_WIDGET (bouton_effacer_etat),
                        _("Delete selected report"));
    g_signal_connect (G_OBJECT (bouton_effacer_etat),
                        "clicked",
                        G_CALLBACK (etats_onglet_efface_etat),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (bouton_effacer_etat), -1);

    /* edit button */
    bouton_personnaliser_etat = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-properties-24.png", _("Properties")));
    gtk_widget_set_tooltip_text (GTK_WIDGET (bouton_personnaliser_etat),
                        _("Edit selected report"));
    g_signal_connect (G_OBJECT (bouton_personnaliser_etat),
                        "clicked",
                        G_CALLBACK (etats_config_personnalisation_etat),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (bouton_personnaliser_etat), -1);

    /* clone button */
    bouton_dupliquer_etat = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-copy-24.png", _("Clone")));
    gtk_widget_set_tooltip_text (GTK_WIDGET (bouton_dupliquer_etat),
                        _("Clone selected report"));
    g_signal_connect (G_OBJECT (bouton_dupliquer_etat),
                        "clicked",
                        G_CALLBACK (etats_onglet_dupliquer_etat),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (bouton_dupliquer_etat), -1);

    /* archive button */
    bouton_export_pdf_etat = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gsb-pdf-24.png", _("Create pdf file")));
    gtk_widget_set_tooltip_text (bouton_export_pdf_etat, _("Creates a pdf file of the report in user directory"));
    g_signal_connect (G_OBJECT (bouton_export_pdf_etat),
					  "clicked",
					  G_CALLBACK (etats_onglet_bouton_export_pdf_etat_clicked),
					  NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (bouton_export_pdf_etat), -1);

    etats_onglet_unsensitive_reports_widgets ();

    return toolbar;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean etats_onglet_click_on_line_report (GtkTreeSelection *selection,
												   GtkWidget *tree_view)
{
    GtkTreeIter iter;
	GtkTreeModel *model;
	gint report_number;

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
		gtk_tree_model_get (model, &iter, 1, &report_number, -1);
		gsb_gui_navigation_set_selection (GSB_REPORTS_PAGE, -1, report_number);
	}

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void etats_onglet_fill_reports_list_model (GtkListStore *list_store)
{
	GSList *tmp_list;
    GtkTreeIter iter;

	tmp_list = gsb_data_report_get_report_list ();
    gtk_list_store_clear (GTK_LIST_STORE (list_store));

	while (tmp_list)
	{
		gint report_number;
		gchar *report_name;

		report_number = gsb_data_report_get_report_number (tmp_list->data);
		report_name = gsb_data_report_get_report_name (report_number);

        gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
        gtk_list_store_set (GTK_LIST_STORE (list_store),
							&iter,
							0, report_name,
							1, report_number,
							-1);

		tmp_list = tmp_list->next;
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_onglet_create_reports_list (void)
{
	GtkWidget *vbox;
	GtkWidget *sw;
	GtkWidget *tree_view;
	GtkListStore *model;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection *selection;

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
	tree_view = gtk_tree_view_new ();
	model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);					/* report_name, report_number */
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL(model));
    g_object_unref (model);
    gtk_container_add (GTK_CONTAINER(sw), tree_view);
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	g_object_set_data (G_OBJECT (vbox), "tree_view", tree_view);

	/* Add column */
    renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(_("Report name"), renderer, "text", 0, NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

	/* Fill the model */
	etats_onglet_fill_reports_list_model (GTK_LIST_STORE (model));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	g_signal_connect (selection,
                      "changed",
					  G_CALLBACK (etats_onglet_click_on_line_report),
					  tree_view);

	gtk_widget_show_all (vbox);

	return vbox;
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void etats_onglet_notebook_switch_page (GtkNotebook *notebook,
											   GtkWidget   *page,
											   guint        page_num,
											   gpointer     user_data)
{
	devel_debug_int (page_num);
	if (page_num == 1)
	{
		GtkWidget *tree_view;

		//~ g_signal_handlers_block_by_func (G_OBJECT (notebook),
										 //~ G_CALLBACK (etats_onglet_notebook_switch_page),
										 //~ NULL);
		tree_view = g_object_get_data (G_OBJECT (page), "tree_view");
		if (GTK_TREE_VIEW (tree_view) && maj_reports_list)
		{
			GtkTreeModel *model;

			model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
			etats_onglet_fill_reports_list_model (GTK_LIST_STORE (model));
			maj_reports_list = FALSE;
		}

		//~ g_signal_handlers_unblock_by_func (G_OBJECT (notebook),
										   //~ G_CALLBACK (etats_onglet_notebook_switch_page),
										   //~ NULL);
	}
}

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/**
 * Ask for a report type for a template list, create the report and
 * update user interface.
 *
 * \param
 *
 * \return	FALSE
 **/
gboolean etats_onglet_ajoute_etat (void)
{
    gint report_number, amount_comparison_number, resultat;
    GtkWidget *dialog, *frame, *combobox, *label_description;
    GtkWidget *scrolled_window;
    GtkWidget *notebook_general;

    notebook_general = gsb_gui_get_general_notebook ();
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
	gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

    dialog = dialogue_special_no_run (GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_OK_CANCEL,
				       _("You are about to create a new report. For "
					 "convenience, you can choose between the "
					 "following templates.  Reports may be "
					 "customized later."),
				       _("Choose template for new report"));

    frame = new_paddingbox_with_title (dialog_get_content_area (dialog), FALSE,
					_("Report type"));

    /* combobox for predefined reports */
    combobox = gtk_combo_box_text_new ();
    gtk_box_pack_start (GTK_BOX(frame), combobox, FALSE, FALSE, 0);

    /* on ajoute maintenant la frame */
    frame = new_paddingbox_with_title (dialog_get_content_area (dialog), TRUE,
					_("Description"));

    /* on met le label dans une scrolled window */
    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX(frame), scrolled_window, TRUE, TRUE, 6);

    /* on ajoute maintenant le label */
    label_description = gtk_label_new (NULL);
    utils_labels_set_alignement (GTK_LABEL (label_description), 0, 0);
    gtk_label_set_line_wrap (GTK_LABEL (label_description), TRUE);
    gtk_container_add (GTK_CONTAINER (scrolled_window), label_description);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_NONE);

	/* fill combobox */
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), _("Last month incomes and outgoings"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), _("Current month incomes and outgoings"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), _("Annual budget"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), _("Blank report"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), _("Cheques deposit"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), _("Monthly outgoings by payee"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), _("Search"));


	/* set first entry and description */
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
	etats_onglet_change_choix_nouvel_etat (combobox, label_description);

	/* update description based on selection */
	g_signal_connect (G_OBJECT (combobox), "changed",
			G_CALLBACK (etats_onglet_change_choix_nouvel_etat),
			G_OBJECT (label_description));

    gtk_box_set_spacing (GTK_BOX(dialog_get_content_area (dialog)), 6);
    gtk_widget_show_all (dialog);

    /* on attend le choix de l'utilisateur */
    resultat = gtk_dialog_run (GTK_DIALOG (dialog));

    if (resultat != GTK_RESPONSE_OK)
    {
	gtk_widget_destroy (dialog);
	return FALSE;
    }

	/* get the wanted report */
	resultat = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));
	gtk_widget_destroy (GTK_WIDGET (dialog));

    /* create and fill the new report */

    switch (resultat)
    {
	case 0:
	    /*  Last month incomes and outgoings  */

	    report_number = gsb_data_report_new (_("Last month incomes and outgoings"));

	    gsb_data_report_set_split_credit_debit (report_number,
						     1);
	    gsb_data_report_set_date_type (report_number, 7);

	    /*   le classement de base est 1-2-3-4-5-6 */

	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (1)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (2)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (3)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (4)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (5)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (6)));

	    gsb_data_report_set_transfer_choice (report_number,
						  2);
	    gsb_data_report_set_category_used (report_number,
						1);
	    gsb_data_report_set_category_show_sub_category (report_number,
							     1);
	    gsb_data_report_set_category_show_category_amount (report_number,
								1);
	    gsb_data_report_set_category_show_sub_category_amount (report_number,
								    1);
	    gsb_data_report_set_category_show_without_category (report_number,
								 1);
	    gsb_data_report_set_category_show_name (report_number,
						     1);

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general (report_number,
						   1);
	    gsb_data_report_set_category_currency (report_number,
						    1);
	    gsb_data_report_set_budget_currency (report_number,
						  1);
	    gsb_data_report_set_payee_currency (report_number,
						 1);
	    gsb_data_report_set_amount_comparison_currency (report_number,
							     1);

	    break;

	case 1:
	    /*  Current month incomes and outgoings */

	    report_number = gsb_data_report_new (_("Current month incomes and outgoings"));

	    gsb_data_report_set_split_credit_debit (report_number,
						     1);
	    gsb_data_report_set_date_type (report_number,
					    3);


	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (1)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (2)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (3)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (4)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (5)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (6)));


	    gsb_data_report_set_transfer_choice (report_number,
						  2);
	    gsb_data_report_set_category_used (report_number,
						1);
	    gsb_data_report_set_category_show_sub_category (report_number,
							     1);
	    gsb_data_report_set_category_show_category_amount (report_number,
								1);
	    gsb_data_report_set_category_show_sub_category_amount (report_number,
								    1);
	    gsb_data_report_set_category_show_without_category (report_number,
								 1);
	    gsb_data_report_set_category_show_name (report_number,
						     1);

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general (report_number,
						   1);
	    gsb_data_report_set_category_currency (report_number,
						    1);
	    gsb_data_report_set_budget_currency (report_number,
						  1);
	    gsb_data_report_set_payee_currency (report_number,
						 1);
	    gsb_data_report_set_amount_comparison_currency (report_number,
							     1);

	    break;


	case 2:

	    /* Annual budget */

	    report_number = gsb_data_report_new (_("Annual budget"));

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (1)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (2)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (3)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (4)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (5)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (6)));


	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general (report_number,
						   1);
	    gsb_data_report_set_category_currency (report_number,
						    1);
	    gsb_data_report_set_budget_currency (report_number,
						  1);
	    gsb_data_report_set_payee_currency (report_number,
						 1);
	    gsb_data_report_set_amount_comparison_currency (report_number,
							     1);

	    gsb_data_report_set_split_credit_debit (report_number,
						     1);
	    gsb_data_report_set_date_type (report_number,
					    4);
	    gsb_data_report_set_category_used (report_number,
						1);
	    gsb_data_report_set_category_show_category_amount (report_number,
								1);
	    gsb_data_report_set_category_show_sub_category (report_number,
							     1);
	    gsb_data_report_set_category_show_without_category (report_number,
								 1);
	    gsb_data_report_set_category_show_sub_category_amount (report_number,
								    1);
	    gsb_data_report_set_category_show_name (report_number,
						     1);
	    gsb_data_report_set_amount_comparison_only_report_non_null (report_number,
									 1);

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;



	case 3:

	    /* Blank report */

	    report_number = gsb_data_report_new (_("Blank report"));

	    /*   le classement de base est 1-2-3-4-5-6  */

	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (1)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (2)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (3)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (4)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (5)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (6)));

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general (report_number,
						   1);
	    gsb_data_report_set_category_currency (report_number,
						    1);
	    gsb_data_report_set_budget_currency (report_number,
						  1);
	    gsb_data_report_set_payee_currency (report_number,
						 1);

	    gsb_data_report_set_amount_comparison_currency (report_number,
							     1);


/* 	    gsb_data_report_set_show_report_transactions (report_number,
 * 							   1);
 * 	    gsb_data_report_set_show_report_date (report_number,
 * 						   1);
 * 	    gsb_data_report_set_show_report_value_date (report_number,
 * 							 1);
 * 	    gsb_data_report_set_show_report_payee (report_number,
 * 						    1);
 * 	    gsb_data_report_set_show_report_category (report_number,
 * 						       1);
 * 	    gsb_data_report_set_split_credit_debit (report_number,
 * 						     1);
 * 	    gsb_data_report_set_transfer_choice (report_number,
 * 						  2);
 * 	    gsb_data_report_set_category_used (report_number,
 * 						1);
 * 	    gsb_data_report_set_category_show_category_amount (report_number,
 * 								1);
 * 	    gsb_data_report_set_category_show_sub_category (report_number,
 * 							     1);
 * 	    gsb_data_report_set_category_show_sub_category_amount (report_number,
 * 								    1);
 * 	    gsb_data_report_set_category_show_name (report_number,
 * 						     1);
 * 	    gsb_data_report_set_amount_comparison_only_report_non_null (report_number,
 * 									 1);
 */


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 4:

	    /* Cheques deposit */

	    report_number = gsb_data_report_new (_("Cheques deposit"));

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (1)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (2)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (3)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (4)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (5)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (6)));

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general (report_number,
						   1);
	    gsb_data_report_set_category_currency (report_number,
						    1);
	    gsb_data_report_set_budget_currency (report_number,
						  1);
	    gsb_data_report_set_payee_currency (report_number,
						 1);
	    gsb_data_report_set_amount_comparison_currency (report_number,
							     1);


	    gsb_data_report_set_show_report_transactions (report_number,
							   1);
	    gsb_data_report_set_show_report_transaction_amount (report_number,
								 1);
	    gsb_data_report_set_show_report_payee (report_number,
						    1);
	    gsb_data_report_set_show_report_bank_references (report_number,
							      1);
	    gsb_data_report_set_transfer_choice (report_number,
						  2);
	    gsb_data_report_set_amount_comparison_only_report_non_null (report_number,
									 1);
	    gsb_data_report_set_amount_comparison_used (report_number,
							 1);

	    /* on doit créer une structure de montant qui dit que ça va être positif */

	    amount_comparison_number = gsb_data_report_amount_comparison_new (0);
	    gsb_data_report_amount_comparison_set_report_number (amount_comparison_number,
								  report_number);
	    gsb_data_report_amount_comparison_set_link_to_last_amount_comparison (amount_comparison_number,
										   -1);
	    gsb_data_report_amount_comparison_set_first_comparison (amount_comparison_number,
								     8);
	    gsb_data_report_amount_comparison_set_link_first_to_second_part (amount_comparison_number,
									      3);

	    gsb_data_report_set_amount_comparison_list (report_number,
							 g_slist_append (gsb_data_report_get_amount_comparison_list (report_number),
									  GINT_TO_POINTER (amount_comparison_number)));

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 5:

	    /* Monthly outgoings by payee */

	    report_number = gsb_data_report_new (_("Monthly outgoings by payee"));

	    /*   le classement de base est 1-2-3-4-5-6  */

	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (6)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (1)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (2)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (3)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (4)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (5)));


	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general (report_number,
						   1);
	    gsb_data_report_set_category_currency (report_number,
						    1);
	    gsb_data_report_set_budget_currency (report_number,
						  1);
	    gsb_data_report_set_payee_currency (report_number,
						 1);
	    gsb_data_report_set_amount_comparison_currency (report_number,
							     1);


	    gsb_data_report_set_show_report_transactions (report_number,
							   1);
	    gsb_data_report_set_show_report_transaction_amount (report_number,
								 1);
	    gsb_data_report_set_show_report_payee (report_number,
						    1);
	    gsb_data_report_set_sorting_report (report_number,
						 2);
	    gsb_data_report_set_column_title_show (report_number,
						    1);
	    gsb_data_report_set_date_type (report_number,
					    7);
	    gsb_data_report_set_category_used (report_number,
						1);
	    gsb_data_report_set_category_show_category_amount (report_number,
								1);
	    gsb_data_report_set_category_show_sub_category (report_number,
							     1);
	    gsb_data_report_set_category_show_name (report_number,
						     1);

	    gsb_data_report_set_show_report_date (report_number,
						   1);
	    gsb_data_report_set_show_report_value_date (report_number,
							 1);
	    gsb_data_report_set_show_report_category (report_number,
						       1);
	    gsb_data_report_set_split_credit_debit (report_number,
						     1);
	    gsb_data_report_set_transfer_choice (report_number,
						  2);
	    gsb_data_report_set_category_show_sub_category_amount (report_number,
								    1);
	    gsb_data_report_set_amount_comparison_only_report_non_null (report_number,
									 1);
	    gsb_data_report_set_payee_used (report_number,
					     1);
	    gsb_data_report_set_payee_show_payee_amount (report_number,
							  1);
	    gsb_data_report_set_payee_show_name (report_number,
						  1);


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 6:

	    /* Search */

	    report_number = gsb_data_report_new (_("Search"));

	    /*   le classement de base est 1-2-3-4-5-6  */

	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (1)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (2)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (3)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (4)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (5)));
	    gsb_data_report_set_sorting_type_list (report_number,
					       g_slist_append (gsb_data_report_get_sorting_type_list (report_number),
								GINT_TO_POINTER (6)));

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general (report_number,
						   1);
	    gsb_data_report_set_category_currency (report_number,
						    1);
	    gsb_data_report_set_budget_currency (report_number,
						  1);
	    gsb_data_report_set_payee_currency (report_number,
						 1);
	    gsb_data_report_set_amount_comparison_currency (report_number,
							     1);


/* 	    gsb_data_report_set_show_report_transactions (report_number,
 * 							   1);
 * 	    gsb_data_report_set_show_report_date (report_number,
 * 						   1);
 * 	    gsb_data_report_set_show_report_value_date (report_number,
 * 							 1);
 * 	    gsb_data_report_set_show_report_payee (report_number,
 * 						    1);
 * 	    gsb_data_report_set_show_report_category (report_number,
 * 						       1);
 * 	    gsb_data_report_set_show_report_sub_category (report_number,
 * 							   1);
 * 	    gsb_data_report_set_show_report_method_of_payment (report_number,
 * 								1);
 * 	    gsb_data_report_set_show_report_budget (report_number,
 * 						     1);
 * 	    gsb_data_report_set_show_report_sub_budget (report_number,
 * 							 1);
 * 	    gsb_data_report_set_show_report_method_of_payment_content (report_number,
 * 									1);
 * 	    gsb_data_report_set_show_report_note (report_number,
 * 						   1);
 * 	    gsb_data_report_set_show_report_voucher (report_number,
 * 						      1);
 * 	    gsb_data_report_set_show_report_marked (report_number,
 * 						     1);
 * 	    gsb_data_report_set_show_report_bank_references (report_number,
 * 							      1);
 * 	    gsb_data_report_set_show_report_financial_year (report_number,
 * 							     1);
 */

/* 	    gsb_data_report_set_report_can_click (report_number,
 * 						   1);
 * 	    gsb_data_report_set_date_type (report_number,
 * 					    4);
 * 	    gsb_data_report_set_period_split (report_number,
 * 					       1);
 * 	    gsb_data_report_set_period_split_type (report_number,
 * 						    3);
 * 	    gsb_data_report_set_transfer_choice (report_number,
 * 						  2);
 */

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;


	default :
	    dialogue_error (_("Unknown report type, creation cancelled"));
	    return FALSE;
    }

    /* Add an entry in navigation pane. */
    gsb_gui_navigation_add_report (report_number);
    etats_onglet_update_gui_to_report (report_number);

    etats_config_personnalisation_etat ();
    gsb_file_set_modified (TRUE);
	maj_reports_list = TRUE;

    return FALSE;
}

/**
 * Create the report tab widgets.
 *
 * \param
 *
 * \return	 A newly allocated vbox.
 **/
GtkWidget *etats_onglet_create_reports_tab (void)
{
    GtkWidget *tab, *vbox;
    GtkWidget *frame;

    tab = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    /* frame pour la barre d'outils */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (tab), frame, FALSE, FALSE, 0);

    /* création du notebook contenant l'état et la config */
    notebook_etats = gtk_notebook_new ();
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook_etats), FALSE);
    gtk_notebook_set_show_border (GTK_NOTEBOOK(notebook_etats), FALSE);
    gtk_box_pack_start (GTK_BOX (tab), notebook_etats, TRUE, TRUE, 0);

    /* création de la partie droite */
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook_etats), vbox, gtk_label_new (_("Reports")));

    /* On met une scrolled window qui sera remplit par l'état */
    scrolled_window_etat = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrolled_window_etat),
					  GTK_SHADOW_NONE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window_etat),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrolled_window_etat, TRUE, TRUE, 0);

	/* affichage de la liste des états */
	vbox = etats_onglet_create_reports_list ();
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook_etats), vbox, NULL);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_etats), 1);

	g_signal_connect (G_OBJECT (notebook_etats),
					  "switch-page",
					  G_CALLBACK (etats_onglet_notebook_switch_page),
					  NULL);

    /* création de la barre d'outils */
    reports_toolbar = etats_onglet_create_reports_toolbar ();
    gtk_container_add (GTK_CONTAINER (frame), reports_toolbar);

    gtk_widget_show_all (tab);
	maj_reports_list = FALSE;

    return (tab);
}

/**
 * Delete current report, after a confirmation dialog.
 *
 * \param
 *
 * \return
 **/
void etats_onglet_efface_etat (void)
{
    gint current_report_number;
    GtkWidget *notebook_general;
    gchar *hint;
    gboolean answer;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if (!current_report_number)
	return;

    notebook_general = gsb_gui_get_general_notebook ();
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

    hint = g_strdup_printf (_("Delete report \"%s\"?"),
                             gsb_data_report_get_report_name (current_report_number));
    answer = question_yes_no (_("This will irreversibly remove this report.  "
                                 "There is no undo for this."),
                               hint, GTK_RESPONSE_NO);
    g_free (hint);

    if (! answer)
        return;

   /* remove the report */
    /* First update reports list in navigation. */
    gsb_gui_navigation_remove_report (current_report_number);

    gsb_data_report_remove (current_report_number);

    etats_onglet_unsensitive_reports_widgets ();

    gsb_file_set_modified (TRUE);
	maj_reports_list = TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *etats_onglet_get_notebook_etats (void)
{
    return notebook_etats;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void etats_onglet_reports_toolbar_set_style (gint toolbar_style)
{
    gtk_toolbar_set_style (GTK_TOOLBAR (reports_toolbar), toolbar_style);
}

/**
 * Set widgets associated to active report unsensitive.  For instance
 * when there is no selected report.
 *
 * \param
 *
 * \return
 **/
void etats_onglet_unsensitive_reports_widgets (void)
{
    gtk_widget_set_sensitive (bouton_personnaliser_etat, FALSE);
    gtk_widget_set_sensitive (bouton_imprimer_etat, FALSE);
    gtk_widget_set_sensitive (bouton_exporter_etat, FALSE);
    gtk_widget_set_sensitive (bouton_dupliquer_etat, FALSE);
    gtk_widget_set_sensitive (bouton_effacer_etat, FALSE);
	gtk_widget_set_sensitive (bouton_export_pdf_etat, FALSE);
}

/**
 * Set widgets associated to active report sensitive.  For instance
 * when a report has just been selected.
 *
 * \param report_number		Report to display.
 *
 * \return
 **/
void etats_onglet_update_gui_to_report (gint report_number)
{
    gtk_widget_set_sensitive (bouton_personnaliser_etat, TRUE);
    gtk_widget_set_sensitive (bouton_imprimer_etat, TRUE);
    gtk_widget_set_sensitive (bouton_exporter_etat, TRUE);
    gtk_widget_set_sensitive (bouton_dupliquer_etat, TRUE);
    gtk_widget_set_sensitive (bouton_effacer_etat, TRUE);
	gtk_widget_set_sensitive (bouton_export_pdf_etat, TRUE);

    if (gsb_report_get_current () != report_number)
    {
		rafraichissement_etat (report_number);
		gsb_report_set_current (report_number);
    }
    else
        gtk_widget_show (gtk_bin_get_child (GTK_BIN (scrolled_window_etat)));
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
