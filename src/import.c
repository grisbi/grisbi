/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2018 Pierre Biava (grisbi@pierre.biava.name)     */
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


#include "config.h"

#include "include.h"
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <sys/stat.h>
#include <sys/time.h>

/*START_INCLUDE*/
#include "import.h"
#include "accueil.h"
#include "bet_data.h"
#include "dialog.h"
#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>
#else
#include "go-charmap-sel.h"
#endif /* HAVE_GOFFICE */
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_assistant.h"
#include "gsb_assistant_file.h"
#include "gsb_automem.h"
#include "gsb_combo_box.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_form.h"
#include "gsb_data_fyear.h"
#include "gsb_data_import_rule.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_file_util.h"
#include "gsb_form_scheduler.h"
#include "gsb_form_transaction.h"
#include "gsb_form_widget.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "import_csv.h"
#include "menu.h"
#include "navigation.h"
#include "qif.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
#ifdef HAVE_XML2
#include "plugins/gnucash/gnucash.h"
#endif
#ifdef HAVE_OFX
#include "plugins/ofx/ofx.h"
#endif
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** Suppported import formats.  Plugins may register themselves. */
static GSList *		ImportFormats = NULL;

/* set to TRUE if we import some marked R transactions
 * grisbi cannot associate them to a reconcile number, so if TRUE,
 * grisbi will show a dialog to tell people to manually associate them */
static gboolean 	marked_r_transactions_imported;

/** used to keep the number of the mother transaction while importing split transactions */
static gint 		mother_transaction_number;

GSList *			liste_comptes_importes;
GSList *			liste_comptes_importes_error;
static gint 		virements_a_chercher;

gchar *				charmap_imported;

static gboolean		add_csv_page = FALSE;

/* gestion des associations entre un tiers et sa chaine de recherche */
GSList *			liste_associations_tiers = NULL;
struct ImportPayeeAsso *last_added_assoc;

/* nombre de transaction à importer qui affiche une barre de progression */
#define NBRE_TRANSACTION_FOR_PROGRESS_BAR 250

/** Known built-in import formats.  Others are plugins. */
static struct ImportFormat builtin_formats[] =
{
{ "CSV", N_("Comma Separated Values"),     "csv", csv_import_csv_account },
{ "QIF", N_("Quicken Interchange Format"), "qif", recuperation_donnees_qif },
#ifdef HAVE_XML2
{ "Gnucash", N_("Gnucash"),                "gnc", recuperation_donnees_gnucash },
#endif
#ifdef HAVE_OFX
{ "OFX", N_("Open Financial Exchange"),    "ofx", recuperation_donnees_ofx },
#endif
{ NULL,  NULL,              NULL,       NULL },
};

enum ImportFileselColumns
{
    IMPORT_FILESEL_SELECTED = 0,
    IMPORT_FILESEL_TYPENAME,
    IMPORT_FILESEL_FILENAME,
    IMPORT_FILESEL_REALNAME,
    IMPORT_FILESEL_TYPE,
    IMPORT_FILESEL_CODING,
	IMPORT_FILESEL_DATE,
    IMPORT_FILESEL_NUM_COLS
};

/** Page numbering for the import wizard. */
enum ImportPages
{
    IMPORT_STARTUP_PAGE,
    IMPORT_FILESEL_PAGE,
    IMPORT_CSV_PAGE,
    IMPORT_RESUME_PAGE,
    IMPORT_FIRST_ACCOUNT_PAGE
};

/******************************************************************************/
/* Fonctions de configuration pour la gestion des associations des tiers      */
/* ****************************************************************************/
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
static gint gsb_import_associations_find_payee (gchar *imported_tiers)
{
    GSList *tmp_list;

    tmp_list = liste_associations_tiers;
    while (tmp_list)
    {
        struct ImportPayeeAsso *assoc;

        assoc = tmp_list->data;
        if (gsb_string_is_trouve (imported_tiers, assoc->search_str, assoc->ignore_case, assoc->use_regex))
        {
           return assoc->payee_number;
        }
        tmp_list = tmp_list->next;
    }

    return 0;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void gsb_import_associations_free_assoc (struct ImportPayeeAsso *assoc, gpointer data)
{
	g_free (assoc->search_str);
	g_free (assoc);
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
GSList *gsb_import_associations_get_liste_associations (void)
{
	return liste_associations_tiers;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_import_associations_init_variables (void)
{
    if (liste_associations_tiers)
    {
		gsb_import_associations_free_liste ();
        liste_associations_tiers = NULL;
    }

}

/**
 * add a new association
 *
 * \param assoc
 * \param search_string
 *
 * \return
 **/
gboolean gsb_import_associations_add_assoc (gint payee_number,
											const gchar *search_str,
											gint ignore_case,
											gint use_regex)
{
	struct ImportPayeeAsso *assoc;
	gboolean result = FALSE;

	if (!payee_number || !search_str)
	{
		return result;
	}

    /* create new association */
	assoc = g_malloc (sizeof (struct ImportPayeeAsso));
	assoc->payee_number = payee_number;
	assoc->search_str = g_strdup (search_str);
	assoc->ignore_case = ignore_case;
	assoc->use_regex = use_regex;

	last_added_assoc = assoc;

    /* add association in liste_associations_tiers */
    if (g_slist_length (liste_associations_tiers) == 0)
    {
        liste_associations_tiers = g_slist_append (liste_associations_tiers, assoc);
        gsb_data_payee_set_search_string (assoc->payee_number, assoc->search_str);
		gsb_data_payee_set_ignore_case (assoc->payee_number,assoc->ignore_case);
		gsb_data_payee_set_use_regex (assoc->payee_number,assoc->use_regex);

		result = TRUE;
    }
    else
    {
        if (!g_slist_find_custom (liste_associations_tiers,
								  assoc,
								  (GCompareFunc) gsb_import_associations_cmp_assoc))
        {
            liste_associations_tiers = g_slist_insert_sorted (liste_associations_tiers,
															  assoc,
															  (GCompareFunc) gsb_import_associations_cmp_assoc);
            gsb_data_payee_set_search_string (assoc->payee_number, assoc->search_str);
			gsb_data_payee_set_ignore_case (assoc->payee_number,assoc->ignore_case);
			gsb_data_payee_set_use_regex (assoc->payee_number,assoc->use_regex);

			result = TRUE;
        }
    }
	return result;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gint gsb_import_associations_cmp_assoc (struct ImportPayeeAsso *assoc_1,
                                        struct ImportPayeeAsso *assoc_2)
{
    gint num_1, num_2;

    num_1 = assoc_1->payee_number;
    num_2 = assoc_2->payee_number;

    if (num_1 == num_2)
        return 0;
    else
        return g_utf8_collate (gsb_data_payee_get_name (num_1, FALSE),
							   gsb_data_payee_get_name (num_2, FALSE));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_import_associations_remove_assoc (gint payee_number)
{
    if (payee_number > 0)
    {
		GSList *tmp_list;

        gsb_data_payee_set_search_string (payee_number, "");
        tmp_list = liste_associations_tiers;
        while (tmp_list)
        {
            struct ImportPayeeAsso *assoc;

            assoc = tmp_list->data;
            if (assoc->payee_number == payee_number)
            {
				g_free (assoc->search_str);
                liste_associations_tiers = g_slist_remove (liste_associations_tiers, assoc);
                break;
            }
            tmp_list = tmp_list->next;
        }
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gint gsb_import_associations_list_append_assoc (gint payee_number,
												struct ImportPayeeAsso *assoc)
{
     if (!g_slist_find_custom (liste_associations_tiers,
							   assoc,
							  (GCompareFunc) gsb_import_associations_cmp_assoc))
        liste_associations_tiers = g_slist_insert_sorted (liste_associations_tiers,
														  assoc,
														  (GCompareFunc) gsb_import_associations_cmp_assoc);

    return g_slist_length (liste_associations_tiers);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_import_associations_free_liste (void)
{
	if (!liste_associations_tiers)
	{
		return;
	}

	g_slist_foreach (liste_associations_tiers, (GFunc) gsb_import_associations_free_assoc, NULL);
	g_slist_free (liste_associations_tiers);
}

/**
 * retourne le dernier numéro de tiers ajouté à la liste
 * utilisé pour la création d'une nouvelle association
 * dans la page associations des préférences
 *
 * \param
 *
 * \return
 **/
gint gsb_import_associations_get_last_payee_number (void)
{
	return last_added_assoc ? last_added_assoc->payee_number : 0;
}

/******************************************************************************/
/* Fonctions générales d'importation des fichiers                             */
/* ****************************************************************************/
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/*
 * date_sort_function
 * This function is called by the Tree Model to sort the files by date
 *
 * \param	model
 * \param	itera
 * \param	iterb
 * \param	NULL
 *
 * \return	qsort()-style comparison
 **/
static gint gsb_import_date_sort_function (GtkTreeModel *model,
										   GtkTreeIter *itera,
										   GtkTreeIter *iterb,
										   gpointer data)
{
    gchar *date_a_str = NULL;
    gchar *date_b_str = NULL;
	gchar **tab_a;
	gchar **tab_b;
	GDate *date_a;
	GDate *date_b;

    gint result;

    if (itera == NULL)
        return -1;
    if (iterb == NULL)
        return -1;

    /* get first date to compare */
    gtk_tree_model_get (model, itera, IMPORT_FILESEL_DATE, &date_a_str, -1);
	if (date_a_str == NULL)
	{
		return -1;
	}

    /* get second date to compare */
    gtk_tree_model_get (model, iterb, IMPORT_FILESEL_DATE, &date_b_str, -1);
	if (date_b_str == NULL)
	{
		return -1;
	}

	tab_a = g_strsplit (date_a_str, " ", 2);
	date_a = gsb_parse_date_string (tab_a[0]);
	tab_b = g_strsplit (date_b_str, " ", 2);
	date_b = gsb_parse_date_string (tab_b[0]);

    result = g_date_compare (date_a, date_b);
	if (result == 0)
	{
		result = g_strcmp0 (tab_a[1],tab_b[1]);
	}
    g_free (date_a_str);
    g_free (date_b_str);
	g_strfreev (tab_a);
	g_strfreev (tab_b);

    return result;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_account_radiobutton_toggle (GtkWidget *radio,
													 gint action)
{
    struct ImportAccount *compte;

	compte = g_object_get_data (G_OBJECT (radio), "account");
    if (compte->hbox1 && gtk_widget_get_visible (compte->hbox1))
        gtk_widget_set_sensitive (compte->hbox1, FALSE);
    if (compte->hbox2 && gtk_widget_get_visible (compte->hbox2))
        gtk_widget_set_sensitive (compte->hbox2, FALSE);
    if (compte->hbox3 && gtk_widget_get_visible (compte->hbox3))
        gtk_widget_set_sensitive (compte->hbox3, FALSE);
    gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (radio), "associated"), TRUE);

	compte->action = action;

	/* check button rule_name */
	if (g_strcmp0 (compte->origine, "CSV")!= 0)
	{
		gtk_widget_set_sensitive (compte->hbox_rule, action != IMPORT_CREATE_ACCOUNT);
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_clear_entry_name_rule (GtkWidget *check_button,
											  GtkWidget *entry)
{
	if (gtk_entry_get_text_length (GTK_ENTRY (entry)))
		gtk_entry_set_text (GTK_ENTRY (entry), "");
	gtk_widget_grab_focus (entry);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gint gsb_import_add_currency (struct ImportAccount *compte)
{
    GtkWidget *vbox, *checkbox, *dialog;
    gint response;
    gchar *message;
    gchar *hint;
    gint currency_number = 0;

    message = g_strdup_printf (
                        _("The account currency imported %s is %s.\nThis currency "
                        "doesn't exist so you have to create it by selecting OK.\n"
                        "\n"
                        "Do you create it?"),
                        compte->nom_de_compte,
                        compte->devise);
    hint = g_strdup_printf (
                        _("Can't associate ISO 4217 code for currency '%s'."),
                        compte->devise);

    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_QUESTION,
                        GTK_BUTTONS_YES_NO,
                        NULL);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), hint);
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                               "%s", message);
    g_free (message);
    g_free (hint);

    vbox = dialog_get_content_area (dialog);

    checkbox = gtk_check_button_new_with_label (
                        _("Use this currency for totals for the payees categories\n"
                        "and budgetary lines"));
    gtk_box_pack_start (GTK_BOX (vbox), checkbox, TRUE, TRUE, 6);

    gtk_widget_show_all (checkbox);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_YES)
    {
		currency_number = gsb_currency_dialog_list_iso_4217_new (NULL, TRUE);
		if (currency_number && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox)))
        {
			GrisbiWinEtat *w_etat;

			w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
            w_etat->no_devise_totaux_tiers = currency_number;
            w_etat->no_devise_totaux_categ = currency_number;
            w_etat->no_devise_totaux_ib = currency_number;
		}
    }
    gtk_widget_destroy (dialog);

    return currency_number;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *gsb_import_assistant_create_final_page (GtkWidget *assistant)
{
    GtkWidget *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 12);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 12);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_create_tag (buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_insert_with_tags_by_name (buffer,
											  &iter,
											  _("Import settings completed successfully"),
											  -1,
											  "x-large",
											  NULL);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
    gtk_text_buffer_insert (buffer, &iter, _("Press the 'Close' button to finish the import."), -1);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

    return view;
}

/**
 * create a new account with the datas in the imported account given in args
 * don't work with the gui here, all the gui will be done later
 *
 * \param imported_account the account we want to create
 *
 * \return the number of the new account or -1 if problem
 **/
static gint gsb_import_create_imported_account (struct ImportAccount *imported_account)
{
    gint account_number;
    gint kind_account;

    if (!imported_account)
		return -1;

    /* create the new account,
     * for now 3 kind of account, GSB_TYPE_BANK, GSB_TYPE_LIABILITIES or GSB_TYPE_CASH */
    switch (gsb_combo_box_get_index (imported_account->bouton_type_compte))
    {
		case 2:
			kind_account = GSB_TYPE_LIABILITIES;
			break;

		case 1:
			kind_account = GSB_TYPE_CASH;
			break;

		default:
			kind_account = GSB_TYPE_BANK;
    }

    account_number = gsb_data_account_new (kind_account);

    if (account_number == -1)
		return -1;

    /* set the default method of payment */
    gsb_data_payment_create_default (account_number);

    /* set the id and try to find the bank */
    if (imported_account->id_compte)
    {
		gchar **tab_str;

		gsb_data_account_set_id (account_number, imported_account->id_compte);

		/* usually, the id is "bank_number guichet_number account_number key" try to get the datas */

		tab_str = g_strsplit (gsb_data_account_get_id (account_number), " ", 3);
		if (tab_str[1])
		{
			gsb_data_account_set_bank_branch_code (account_number, tab_str[1]);
			if (tab_str[2])
			{
				gchar *temp;

				gsb_data_account_set_bank_account_key (account_number, tab_str[2] + strlen (tab_str[2]) - 1);

				temp = my_strdup (tab_str[2]);
				if (temp && strlen(temp))
				{
					temp[strlen (temp) - 1] = 0;
					gsb_data_account_set_bank_account_number (account_number, temp);
					g_free (temp);
				}
			}
		}
		g_strfreev (tab_str);
    }

    /* set the name (the g_strstrip is VERY important !!!) */
    if (imported_account->nom_de_compte)
		gsb_data_account_set_name (account_number, g_strstrip (imported_account->nom_de_compte));
    else
		gsb_data_account_set_name (account_number, _("Imported account"));

    /* set the currency */
    gsb_data_account_set_currency (account_number,
								   gsb_currency_get_currency_from_combobox (imported_account->bouton_devise));

    /* set the initial balance */
    gsb_data_account_set_init_balance (account_number, imported_account->solde);

    /* Use two lines view by default. */
    gsb_data_account_set_nb_rows (account_number, 2);

    return account_number;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *gsb_import_create_page_recapitulatif (struct ImportAccount *compte)
{
    GtkWidget *button;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *radio_add_account;
    GtkWidget *radio_mark_account;
    GtkWidget *radio_new_account;
    GtkWidget *vbox;
    gchar *short_filename;
    gint size = 0, spacing = 0;
    gint account_number;
    gchar *tmp_str;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);

    if (compte->filename)
		short_filename = g_path_get_basename (compte->filename);
    else
    {
		if (compte->real_filename)
			short_filename = g_path_get_basename (compte->real_filename);
		else
			short_filename = g_strdup (_("file"));
    }

    label = gtk_label_new (NULL);
    utils_labels_set_alignment (GTK_LABEL (label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    tmp_str = g_markup_printf_escaped (_("<span size=\"x-large\">%s</span>\n\n"
										 "What do you want to do with contents from <span "
										 "foreground=\"blue\">%s</span> ?\n"),
									   compte->nom_de_compte,
									   short_filename);
    gtk_label_set_markup (GTK_LABEL (label), tmp_str);
    g_free (tmp_str);
    g_free (short_filename);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    /* New account */
    radio_new_account = gtk_radio_button_new_with_label (NULL, _("Create a new account"));
    gtk_box_pack_start (GTK_BOX (vbox), radio_new_account, FALSE, FALSE, 0);
    gtk_widget_style_get (radio_new_account, "indicator_size", &size, NULL);
    gtk_widget_style_get (radio_new_account, "indicator_spacing", &spacing, NULL);

    compte->hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (vbox), compte->hbox1, FALSE, FALSE, 0);
    label = gtk_label_new (_("Account type: "));
	gtk_widget_set_margin_start (label, 2 * spacing + size);
    gtk_box_pack_start (GTK_BOX (compte->hbox1), label, FALSE, FALSE, 0);

    compte->bouton_type_compte = gsb_combo_box_new_with_index_from_list (gsb_data_bank_get_bank_type_list (),
																		 NULL,
																		 NULL);
    gtk_box_pack_start (GTK_BOX (compte->hbox1), compte->bouton_type_compte, TRUE, TRUE, 0);

    /* at this level imported_account->type_de_compte was filled while importing transactions,
     * in qif.c or ofx.c ; but we have only 4 kind of account for now, so try to place the combobox correctly
     * and it's the user who will choose the kind of account */
    switch (compte->type_de_compte)
    {
		case 3:
			gsb_combo_box_set_index (compte->bouton_type_compte, GSB_TYPE_LIABILITIES);
			break;

		case 7:
			gsb_combo_box_set_index (compte->bouton_type_compte, GSB_TYPE_CASH);
			break;

		default:
			gsb_combo_box_set_index (compte->bouton_type_compte, GSB_TYPE_BANK);
    }

	/* Add to account */
    radio_add_account = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_new_account),
																	 _("Add transactions to an account"));
    gtk_box_pack_start (GTK_BOX (vbox), radio_add_account, FALSE, FALSE, 0);
    if (radio_add_account && gtk_widget_get_visible (radio_add_account))
        gtk_widget_set_sensitive  (radio_add_account, assert_account_loaded ());

    compte->hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (vbox), compte->hbox2, FALSE, FALSE, 0);
    label = gtk_label_new (_("Account name: "));
	gtk_widget_set_margin_start (label, 2 * spacing + size);
    gtk_box_pack_start (GTK_BOX (compte->hbox2), label, FALSE, FALSE, 0);

    compte->bouton_compte_add = gsb_account_create_combo_list (NULL, NULL, FALSE);
    gtk_box_pack_start (GTK_BOX (compte->hbox2), compte->bouton_compte_add, TRUE, TRUE, 0);
    gtk_widget_set_sensitive (compte->hbox2, FALSE);

    /* Mark account */
    radio_mark_account = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio_new_account),
																	  ("Mark transactions of an account"));
    gtk_box_pack_start (GTK_BOX (vbox), radio_mark_account, FALSE, FALSE, 0);
    gtk_widget_set_sensitive  (radio_mark_account, assert_account_loaded ());

    compte->hbox3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (vbox), compte->hbox3, FALSE, FALSE, 0);
    label = gtk_label_new (_("Account name: "));
	gtk_widget_set_margin_start (label, 2 * spacing + size);
    gtk_box_pack_start (GTK_BOX (compte->hbox3), label, FALSE, FALSE, 0);

    compte->bouton_compte_mark = gsb_account_create_combo_list (NULL, NULL, FALSE);
    gtk_box_pack_start (GTK_BOX (compte->hbox3), compte->bouton_compte_mark, TRUE, TRUE, 0);
    gtk_widget_set_sensitive (compte->hbox3, FALSE);

    /* Currency */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    label = gtk_label_new (_("Account currency: "));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    compte->bouton_devise = gsb_currency_make_combobox (TRUE);

    /* create the currency if doesn't exist */
    if (compte->devise)
    {
        gint currency_number;

        /* First, we search currency from ISO4217 code for existing currencies */
        currency_number = gsb_data_currency_get_number_by_code_iso4217 (compte->devise);

		/* Then, by nickname for existing currencies */
        if (!currency_number)
        {
            currency_number = gsb_import_add_currency (compte);
        }

        if (currency_number)
            gsb_currency_set_combobox_history (compte->bouton_devise, currency_number);
    }

    gtk_box_pack_start (GTK_BOX (hbox), compte->bouton_devise, FALSE, FALSE, 0);

    /* invert amount of transactions */
    button = gsb_automem_checkbutton_new (_("Invert the amount of the imported transactions"),
										  &compte->invert_transaction_amount,
										  NULL,
										  NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

    /* propose to create a rule */
	if (g_strcmp0 (compte->origine, "CSV") != 0)
	{
		compte->hbox_rule = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
		gtk_box_pack_start (GTK_BOX (vbox), compte->hbox_rule, FALSE, FALSE, 0);

		/* set entry_name_rule */
		compte->entry_name_rule = gtk_entry_new ();
		gtk_widget_set_sensitive (compte->entry_name_rule, FALSE);

		/* set checkbutton_create_rule */
		button = gsb_automem_checkbutton_new (_("Create a rule for this import. Name of the rule: "),
											  &compte->create_rule,
											  G_CALLBACK (utils_buttons_sensitive_by_checkbutton),
											  compte->entry_name_rule);
		gtk_box_pack_start (GTK_BOX (compte->hbox_rule), button, FALSE, FALSE, 0);

		/* pack entry_name_rule */
		gtk_box_pack_start (GTK_BOX (compte->hbox_rule), compte->entry_name_rule, FALSE, FALSE, 0);

		/* set signal to clear entry_name_rule */
		g_signal_connect_after (G_OBJECT (button),
							    "toggled",
							    G_CALLBACK (gsb_import_clear_entry_name_rule),
							    compte->entry_name_rule);
	}

	/* set objects */
    g_object_set_data (G_OBJECT (radio_add_account), "associated", compte->hbox2);
    g_object_set_data (G_OBJECT (radio_mark_account), "associated", compte->hbox3);
    g_object_set_data (G_OBJECT (radio_new_account), "associated", compte->hbox1);

	g_object_set_data (G_OBJECT (radio_add_account), "account", compte);
    g_object_set_data (G_OBJECT (radio_mark_account), "account", compte);
    g_object_set_data (G_OBJECT (radio_new_account), "account", compte);

	g_object_set_data (G_OBJECT (radio_add_account), "invert_amount", compte);
    g_object_set_data (G_OBJECT (radio_mark_account), "invert_amount", compte);
    g_object_set_data (G_OBJECT (radio_new_account), "invert_amount", compte);

	/* set on the right account */
    account_number = gsb_data_account_get_account_by_id (compte->id_compte);
    if (account_number > 0)
    {
		/* get rule name */
		if (g_strcmp0 (compte->origine, "CSV"))
		{
			GSList	*tmp_list;
			gint number;

			tmp_list = gsb_data_import_rule_get_from_account (account_number);
			number = g_slist_length (tmp_list);
			if (number > 0)
			{
				ImportRule *rule;

				rule = tmp_list->data;
				gtk_entry_set_text (GTK_ENTRY (compte->entry_name_rule), rule->rule_name);				
			}
		}

    	gsb_import_account_radiobutton_toggle (radio_add_account,IMPORT_ADD_TRANSACTIONS);
    	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_add_account), TRUE);

		gsb_account_set_combo_account_number (compte->bouton_compte_add, account_number);
    	gsb_account_set_combo_account_number (compte->bouton_compte_mark, account_number);
    }

	/* set signals */
    g_signal_connect (G_OBJECT (radio_new_account),
					  "toggled",
					  G_CALLBACK (gsb_import_account_radiobutton_toggle),
					  IMPORT_CREATE_ACCOUNT);
    g_signal_connect (G_OBJECT (radio_add_account),
					  "toggled",
					  G_CALLBACK (gsb_import_account_radiobutton_toggle),
					  GINT_TO_POINTER (IMPORT_ADD_TRANSACTIONS));
    g_signal_connect (G_OBJECT (radio_mark_account),
					  "toggled",
					  G_CALLBACK (gsb_import_account_radiobutton_toggle),
					  GINT_TO_POINTER (IMPORT_MARK_TRANSACTIONS));

	return vbox;
}

/**
 * this is the summary page of the import assistant
 * show what accounts will be imported in grisbi
 * and create the next pages of the assistant, one per account
 *
 * \param assistant
 *
 * \return  FALSE
 */
static gboolean gsb_import_affichage_recapitulatif_importation (GtkWidget *assistant)
{
    gint page;
    GSList *tmp_list;

    if (!assistant)
		return FALSE;

    /* Initial page is fourth. */
    page = IMPORT_FIRST_ACCOUNT_PAGE;

    /* First, iter to see if we need to create currencies */
    tmp_list = liste_comptes_importes;
    while (tmp_list)
    {
		struct ImportAccount *compte;

		compte = tmp_list->data;

		/* add one page per account */
		gsb_assistant_add_page (assistant,
								gsb_import_create_page_recapitulatif (compte),
								page,
								page - 1,
								page + 1,
								G_CALLBACK (NULL));
		page ++;

		tmp_list = tmp_list->next;
    }

    /* And final page */
    gsb_assistant_add_page (assistant,
							gsb_import_assistant_create_final_page (assistant),
							page, page - 1, -1, G_CALLBACK (NULL));

    /* Replace button. */
    gsb_assistant_change_button_next (assistant, "gtk-go-forward", GTK_RESPONSE_YES);

    return (FALSE);
}

/**
 * try to autodetect the type of imported file
 *
 * \param filename
 * \param pointeur_char	pointer returned by g_file_get_contents
 *          can be NULL, in that case only the extension will be used
 *
 * \return a string : OFX, QIF, Gnucash
 **/
static const gchar *gsb_import_autodetect_file_type (gchar *filename,
													 gchar *pointeur_char)
{
    gchar *extension;
    const gchar *type;

    extension = strrchr (filename, '.');
    if (extension)
    {
    GSList *tmp = ImportFormats;

    while (tmp)
    {
        struct ImportFormat *format = (struct ImportFormat *) tmp->data;

        if (!g_ascii_strcasecmp (extension + 1, format->extension))
        {
        return format->name;
        }

        tmp = tmp->next;
    }
    }

    if (!pointeur_char)
    {
    return _("Unknown");
    }

    /** FIXME: add auto-detection to plugin. */

    if (g_strrstr (pointeur_char, "ofx") || g_strrstr (pointeur_char, "OFX"))
    {
    type = "OFX";
    }
    else if (!my_strncasecmp (pointeur_char, "!Type", 5) ||
        !my_strncasecmp (pointeur_char, "!Account", 8) ||
        !my_strncasecmp (pointeur_char, "!Option", 7))
    {
    type = "QIF";
    }
    else
    {
    if (strstr (pointeur_char, "<gnc-v2"))
    {
        type = "Gnucash";
    }
    else
    {
        type = _("Unknown");
    }
    }

    return type;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_preview_maybe_sensitive_next (GtkWidget *assistant,
													 GtkTreeModel *model)
{
    GtkTreeIter iter;

	/* Don't allow going to next page if no file is selected yet. */
    gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), FALSE);

    if (!gtk_tree_model_get_iter_first (model, &iter))
    {
		return;
    }

    /* Iterate over lines so we check if some are checked. */
    do
    {
		gchar *type;
		gboolean selected;

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
							IMPORT_FILESEL_SELECTED, &selected,
							IMPORT_FILESEL_TYPE, &type,
						-   1);
		if (selected && strcmp (type, _("Unknown")) != 0)
		{
			gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), TRUE);
			g_free (type);

			return;
		}
		g_free (type);
    }
    while (gtk_tree_model_iter_next (model, &iter));
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_import_file_selection_cell_renderer_toggled (GtkCellRendererToggle *cell,
																 gchar *path_str,
																 gpointer model)
{
    GtkWidget *assistant;
	GtkWidget *qif_button;
    GtkTreePath *path;
    GtkTreeIter iter;
	gchar *type;
    gboolean toggle_item;
	GrisbiAppConf *a_conf;

	assistant = g_object_get_data (G_OBJECT (model), "assistant");
	qif_button = g_object_get_data (G_OBJECT (assistant), "qif_button");
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	path = gtk_tree_path_new_from_string (path_str);
    gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        IMPORT_FILESEL_SELECTED, &toggle_item,
						IMPORT_FILESEL_TYPE, &type,
						-1);
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                        IMPORT_FILESEL_SELECTED, !toggle_item, -1);

	if (a_conf->force_import_directory)
	{
		gchar *contents;
		gchar *tmp_contents;
		gchar *charmap;
		gchar *new_charmap = NULL;
		gchar *nom_fichier;

		gtk_tree_model_get (GTK_TREE_MODEL (model),
							&iter,
							IMPORT_FILESEL_REALNAME, &nom_fichier,
							IMPORT_FILESEL_CODING, &charmap,
							-1);
		(void)g_file_get_contents (nom_fichier, &tmp_contents, NULL, NULL);
		contents = g_convert (tmp_contents, -1, "UTF-8", charmap, NULL, NULL, NULL);

		if (contents == NULL)
		{
			gchar *basename = g_path_get_basename (nom_fichier);
			new_charmap = utils_files_create_sel_charset (assistant,
														  tmp_contents,
														  charmap,
														  basename);
			g_free(basename);
			if (new_charmap)
			{
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, IMPORT_FILESEL_CODING, new_charmap, -1);
				g_free (new_charmap);
			}

		}
		else
		{
			g_free (contents);
		}

		g_free (tmp_contents);
		g_free (charmap);
		g_free (nom_fichier);

		if (strcmp (type, "CSV") == 0)
		{
			/* !toggle_item parce que la case à cocher vient de passer de 0 à 1 */
			if (!toggle_item)
			{
				gsb_assistant_set_next (assistant, IMPORT_FILESEL_PAGE, IMPORT_CSV_PAGE);
				gsb_assistant_set_prev (assistant, IMPORT_RESUME_PAGE, IMPORT_CSV_PAGE);
				add_csv_page = TRUE;
			}
			else
			{
				gsb_assistant_set_next (assistant, IMPORT_FILESEL_PAGE, IMPORT_RESUME_PAGE);
				gsb_assistant_set_prev (assistant, IMPORT_RESUME_PAGE, IMPORT_FILESEL_PAGE);
				add_csv_page = FALSE;
			}
		}
	}

	if (strcmp (type, "QIF") == 0)
	{
		gtk_widget_set_sensitive (qif_button, !toggle_item);
	}
	else
	{
		gtk_widget_set_sensitive (qif_button, FALSE);
	}

	gtk_tree_path_free (path);
	g_free (type);

    gsb_import_preview_maybe_sensitive_next (assistant, model);

    return FALSE;
}

/**
 * create an appropriate dialog file chooser
 * for importing files to grisbi and return the selected files
 *
 * \param
 * \param
 *
 * \return a GtkFileChooser
 * */
static GSList *gsb_import_create_file_chooser (const char *enc,
											   GtkWidget *parent)
{
    GtkWidget *dialog, *hbox, *go_charmap_sel;
	GtkWidget *button_cancel;
	GtkWidget *button_open;
	GtkWidget *label;
    GtkFileFilter *filter, *default_filter;
    gchar *files;
    GSList *tmp_list;
    struct ImportFormat *format;
    GSList *filenames = NULL;
    gchar *old_str;
    gchar *tmp_str;
    const gchar *tmp_char;
    gchar *tmp_last_directory;
	GrisbiAppConf *a_conf;

    dialog = gtk_file_chooser_dialog_new (_("Choose files to import."),
										  GTK_WINDOW (parent),
										  GTK_FILE_CHOOSER_ACTION_OPEN,
										  NULL, NULL,
										  NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_CANCEL);

	button_open = gtk_button_new_with_label (_("Open"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_open, GTK_RESPONSE_ACCEPT);
	gtk_widget_set_can_default (button_open, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (a_conf->force_import_directory)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), a_conf->import_directory);
	else
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gsb_file_get_last_path ());

    /* Import filters */
    tmp_list = ImportFormats;
    format = (struct ImportFormat *) tmp_list->data;
    files = g_strconcat ("*.", format->extension, NULL);
    tmp_list = tmp_list->next;

    while (tmp_list)
    {
		format = (struct ImportFormat *) tmp_list->data;
		old_str = files;
		files = g_strconcat (files, ", *.", format->extension, NULL);
		g_free (old_str);
		tmp_list = tmp_list->next;
    }

    default_filter = gtk_file_filter_new ();
    tmp_str = g_strdup_printf (_("Known files (%s)"), files);
	g_free(files);
    gtk_file_filter_set_name (default_filter, tmp_str);
    g_free (tmp_str);

    tmp_list = ImportFormats;
    while (tmp_list)
    {
		GtkFileFilter *format_filter;

		format = (struct ImportFormat *) tmp_list->data;

		format_filter = gtk_file_filter_new ();
		tmp_str = g_strdup_printf (_("%s files (*.%s)"), format->name, format->extension);
		gtk_file_filter_set_name (format_filter, tmp_str);
		g_free (tmp_str);

		/* Make it case insensitive */
		tmp_str = g_strdup ("*.");
		tmp_char = format->extension;
		while(*tmp_char != '\0')
		{
			old_str=tmp_str;
			tmp_str = g_strdup_printf ("%s[%c%c]",
									   tmp_str,
									   (int)g_ascii_toupper(*tmp_char),
									   (int)*tmp_char);
			tmp_char++;
			g_free (old_str);
		}
		gtk_file_filter_add_pattern (format_filter, tmp_str);

		/* Add this pattern to the global filter as well*/
		gtk_file_filter_add_pattern (default_filter, tmp_str);
		g_free (tmp_str);

		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), format_filter);

		tmp_list = tmp_list->next;
    }

    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), default_filter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), default_filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    /* Add encoding preview */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), hbox);
    gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (_("Encoding: ")), FALSE, FALSE, 0);

	go_charmap_sel = go_charmap_sel_new (GO_CHARMAP_SEL_TO_UTF8);
    if (enc && strlen (enc))
        go_charmap_sel_set_encoding ((GOCharmapSel *) go_charmap_sel, enc);
    gtk_box_pack_start (GTK_BOX (hbox), go_charmap_sel, TRUE, TRUE, 0);

	gtk_widget_show_all (dialog);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));

    /* save charmap */
    charmap_imported = g_strdup (go_charmap_sel_get_encoding ((GOCharmapSel *)go_charmap_sel));

	tmp_last_directory = utils_files_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE);
	if (!a_conf->force_import_directory)
		gsb_file_update_last_path (tmp_last_directory);

	/* set label of paddingbox file_selection_page */
	label = g_object_get_data (G_OBJECT (parent), "paddingbox_label");
	if (label)
	{
		gchar *tmp_str2;

		tmp_str = g_strdup_printf (_("Choose file to import in %s"), tmp_last_directory);
		tmp_str2 = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", tmp_str);
		gtk_label_set_markup (GTK_LABEL (label), tmp_str2);
		g_free(tmp_str);
		g_free (tmp_str2);
	}

	g_free (tmp_last_directory);

    gtk_widget_destroy (dialog);

	return filenames;
}

/**
 * décompresse le fichier et le sauvegarde à la place du fichier original
 *
 * \param filename          nom du fichier provisoire
 *
 * \return TRUE si OK FALSE autrement
 **/
static gboolean gsb_import_gunzip_file (gchar *filename)
{
    gchar *file_content;
    gulong length;

    if (gsb_file_util_get_contents (filename, &file_content, &length))
    {
        GError *error = NULL;

        if (g_unlink (filename))
			important_debug("Unable to remove file");
        if (!g_file_set_contents (filename, file_content, length, &error))
        {
            gchar *tmp_str;

			tmp_str = g_strdup_printf (_("Cannot unzip file '%s': %s"),
									   filename,
									   error->message);
            dialogue_error (tmp_str);
			g_free (tmp_str);
            g_free (file_content);
            g_error_free (error);
            return FALSE;
        }
        else
		{
			g_free (file_content);
            return TRUE;
		}
    }
    return FALSE;
}

/**
 * Register a known import format as known.
 *
 * \param format        A pointer to a structure describing
 *                      this import format.
 *
 * \return
 **/
static void gsb_import_register_ImportFormat (struct ImportFormat *format)
{
    gchar *tmp_str;

    tmp_str = g_strdup_printf (_("Adding '%s' as an import format"), format->name);
    devel_debug (tmp_str);
    g_free (tmp_str);
    ImportFormats = g_slist_append (ImportFormats, format);
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
static gboolean gsb_import_switch_type (GtkCellRendererText *cell,
								    const gchar *path,
								    const gchar *value,
								    GtkListStore *model)
{
    GtkTreeIter iter;
    GtkWidget *assistant;

    assistant = g_object_get_data (G_OBJECT (model), "assistant");

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
    {
    GSList *tmp_list = ImportFormats;

    while (tmp_list)
    {
        gchar *contents;
        gchar *nom_fichier;
        gchar *tmp_str;
        GError *error = NULL;
        struct ImportFormat *format;

        format = (struct ImportFormat *) tmp_list->data;
        if (!strcmp (value, format->name))
        {
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                        IMPORT_FILESEL_TYPENAME, value,
                        IMPORT_FILESEL_TYPE, format->name,
                        -1);

        /* CSV is special because it needs configuration, so
         * we add a conditional jump there. */
        if (!strcmp (value, "CSV"))
        {
            gsb_assistant_set_next (assistant, IMPORT_FILESEL_PAGE,
                        IMPORT_CSV_PAGE);
            gsb_assistant_set_prev (assistant, IMPORT_RESUME_PAGE,
                        IMPORT_CSV_PAGE);
        }

        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                    IMPORT_FILESEL_REALNAME, &nom_fichier,
                    -1);
        /* get contents of file */
        if (!g_file_get_contents (nom_fichier, &tmp_str, NULL, &error))
        {
            g_print (_("Unable to read file: %s\n"), error->message);
            g_error_free (error);
            return FALSE;
        }

        /* Convert in UTF8 */
        contents = g_convert (tmp_str, -1, "UTF-8", charmap_imported, NULL, NULL, NULL);

        if (contents == NULL)
        {
			gchar *basename = g_path_get_basename (nom_fichier);
            charmap_imported = utils_files_create_sel_charset (assistant, tmp_str,
                        charmap_imported,
						basename);
			g_free(basename);
            gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                        IMPORT_FILESEL_CODING, charmap_imported,
                        -1);
        }
		else
			g_free (contents);
        g_free (tmp_str);

        gsb_import_preview_maybe_sensitive_next (assistant, GTK_TREE_MODEL (model));
        }

        tmp_list = tmp_list->next;
    }
    }

     return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_select_file (GSList *filenames,
									GtkWidget *assistant)
{
	GtkWidget *qif_button;
    GSList *iterator;
    GtkTreeModel *model;
	gboolean selected;
	GrisbiAppConf *a_conf;

	devel_debug (charmap_imported);
    model = g_object_get_data (G_OBJECT (assistant), "model");
	if (!model || !GTK_IS_TREE_STORE (model))
		return;
	else
		gtk_tree_store_clear (GTK_TREE_STORE (model));

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (a_conf->force_import_directory)
	{
		selected = FALSE;
	}
	else
		selected = TRUE;

    iterator = filenames;
    while (iterator && model)
    {
		GtkTreeIter iter;
		const gchar *type;
		gchar *nom_fichier;
		gchar *tmp_contents;
		gchar *contents;
		gchar *charmap;
		gchar *tmp_str;
		gchar *str_last_modif = NULL;
		GError *error = NULL;
		gchar *extension;
		struct stat buf;

		/* Open file */
		if (stat (iterator->data, &buf) == 0)
		{
			struct tm *file_time;

			file_time = localtime ((&buf.st_mtime));
			if (file_time)
			{
				tmp_str = gsb_format_date (file_time->tm_mday,file_time->tm_mon + 1,file_time->tm_year + 1900);
				str_last_modif = g_strdup_printf ("%s %02d:%02d:%02d",
												  tmp_str,
												  file_time->tm_hour,
												  file_time->tm_min,
												  file_time->tm_sec);

				g_free (tmp_str);
			}
			else
			{
				str_last_modif = g_strdup ("");
			}
		}
		else
			str_last_modif = g_strdup ("");

		extension = strrchr (iterator->data, '.');

		/* unzip Gnucash file if necessary */
		if (extension && strcmp (extension, ".gnc") == 0)
			gsb_import_gunzip_file (iterator->data);

		/* get contents of file */
		if (!g_file_get_contents (iterator->data, &tmp_contents, NULL, &error))
		{
			g_free(str_last_modif);
			g_print (_("Unable to read file: %s\n"), error->message);
			g_error_free (error);
			return;
		}

		type = gsb_import_autodetect_file_type (iterator->data, tmp_contents);
		qif_button = g_object_get_data (G_OBJECT (assistant), "qif_button");

		/* passe par un fichier temporaire pour bipasser le bug libofx */
		if (strcmp (type, "OFX") == 0)
		{
			gtk_widget_set_sensitive (qif_button, FALSE);
			charmap = utils_files_get_ofx_charset (tmp_contents);
		}
		else if (strcmp (type, "QIF") == 0)
		{

			gtk_widget_set_sensitive (qif_button, selected);
			charmap = charmap_imported;
		}
		else
		{
			gtk_widget_set_sensitive (qif_button, FALSE);
			charmap = charmap_imported;
		}

		/* CSV is special because it needs configuration, so we
		 * add a conditional jump here when selected = TRUE. */
		if (selected && !strcmp (type, "CSV"))
		{
			gsb_assistant_set_next (assistant, IMPORT_FILESEL_PAGE, IMPORT_CSV_PAGE);
			gsb_assistant_set_prev (assistant, IMPORT_RESUME_PAGE, IMPORT_CSV_PAGE);
		}

 		/* Test Convert to UTF8 */
		if (g_utf8_validate (tmp_contents, -1, NULL))
		{
			charmap = (gchar*)"UTF-8";
		}
		else
		{
			if (charmap && !a_conf->force_import_directory)
			{
				contents = g_convert (tmp_contents, -1, "UTF-8", charmap, NULL, NULL, NULL);

				if (contents == NULL)
				{
					gchar *basename = g_path_get_basename (iterator->data);
					charmap = utils_files_create_sel_charset (assistant,
															  tmp_contents,
															  charmap,
															  basename);
					g_free(basename);
				}
				else
					g_free (contents);
			}
		}

		tmp_str = g_path_get_basename (iterator->data);
		nom_fichier = my_strdup (iterator->data);
		gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
							IMPORT_FILESEL_SELECTED, selected,
							IMPORT_FILESEL_TYPENAME, type,
							IMPORT_FILESEL_FILENAME, tmp_str,
							IMPORT_FILESEL_REALNAME, nom_fichier,
							IMPORT_FILESEL_TYPE, type,
							IMPORT_FILESEL_CODING, charmap,
							IMPORT_FILESEL_DATE, str_last_modif,
							-1);
		g_free (nom_fichier);
		g_free (tmp_contents);
		g_free (tmp_str);
		g_free (str_last_modif);

		if (selected && strcmp (type, _("Unknown")) != 0)
		{
			/* A valid file was selected, so we can now go ahead. */
			gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), TRUE);
		}

		iterator = iterator->next;
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_import_enter_force_dir_page (GtkWidget *assistant)
{
	GFileEnumerator *direnum;
	GFile *dir;
    GSList *filenames = NULL;
	GrisbiAppConf *a_conf;

	devel_debug (charmap_imported);
	if (add_csv_page)
	{
		gsb_assistant_set_next (assistant, IMPORT_FILESEL_PAGE, IMPORT_RESUME_PAGE);
		gsb_assistant_set_prev (assistant, IMPORT_RESUME_PAGE, IMPORT_FILESEL_PAGE);

		add_csv_page = FALSE;
	}

	/* Don't allow going to next page if no file is selected yet. */
    gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), FALSE);

	/* if a_conf->import_directory is not set force a default directory */
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (!a_conf->import_directory)
		a_conf->import_directory = my_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD));

	dir =  g_file_new_for_path (a_conf->import_directory);
	direnum = g_file_enumerate_children (dir,
										"standard::*",
										G_FILE_QUERY_INFO_NONE,
										NULL,
										NULL);
	if (!direnum)
		return FALSE;

	while (TRUE)
	{
		GFileInfo *info = NULL;
		const gchar *filename;
		gchar *extension;
		const gchar *type = NULL;

		if (!g_file_enumerator_iterate (direnum, &info, NULL, NULL, NULL))
			break;

		if (!info)
		  break;

		filename = g_file_info_get_name (info);
		if (filename && strlen (filename))
		{
			extension = strrchr (filename, '.');
		}
		else
		{
			continue;
		}

		if (!extension)
		{
			continue;
		}

		if (g_ascii_strcasecmp (extension+1, "csv") == 0)
			type = "CSV";
		else if (g_ascii_strcasecmp (extension+1, "gnc") == 0)
			type = "Gnucash";
		else if (g_ascii_strcasecmp (extension+1, "qif") == 0)
			type = "QIF";
		else if (g_ascii_strcasecmp (extension+1, "ofx") == 0)
			type = "OFX";
		if (type)
		{
			gchar *tmp_filename;

			tmp_filename = g_build_filename (a_conf->import_directory, G_DIR_SEPARATOR_S, filename, NULL);

			filenames = g_slist_append (filenames, tmp_filename);
		}
	}
	g_object_unref (direnum);
	g_object_unref (dir);

	if (!filenames)
		return FALSE;

	gsb_import_select_file (filenames, assistant);
    g_slist_free_full (filenames, g_free);

    return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_import_select_file_from_chooser (GtkWidget *button,
										GtkWidget *assistant)
{
    GSList *filenames;
	devel_debug (NULL);

    filenames = gsb_import_create_file_chooser (NULL, assistant);
    if (!filenames)
		return FALSE;

	gsb_import_select_file (filenames, assistant);
	g_slist_free_full (filenames, g_free);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *gsb_import_create_file_selection_page (GtkWidget *assistant)
{
	GtkWidget *chooser;
	GtkWidget *label;
    GtkWidget *paddingbox;
	GtkWidget *qif_button;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *vbox;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeModel *model, *list_acc;
    GSList *tmp_list;
	gchar *tmp_str;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (a_conf->force_import_directory)
	{
		tmp_str = g_strdup_printf (_("Choose file to import from the directory: %s"), a_conf->import_directory);
		paddingbox = new_paddingbox_with_title (vbox, TRUE, tmp_str);
		g_free (tmp_str);
	}
	else
		paddingbox = new_paddingbox_with_title (vbox, TRUE, _("Choose file to import"));

	/* save padddingbox label */
	label = g_object_get_data (G_OBJECT (paddingbox), "paddingbox_label");
	g_object_set_data (G_OBJECT (assistant), "paddingbox_label", label);

	/* Add select file button */
	chooser = gtk_button_new_with_label (_("Add file to import..."));
	tmp_str = g_build_filename (gsb_dirs_get_pixmaps_dir (), "gsb-import-32.png", NULL);
	gtk_button_set_always_show_image (GTK_BUTTON (chooser), TRUE);
	gtk_button_set_image (GTK_BUTTON(chooser), gtk_image_new_from_file (tmp_str));
	g_free (tmp_str);
	gtk_box_pack_start (GTK_BOX(paddingbox), chooser, FALSE, FALSE, 6);
	g_signal_connect (G_OBJECT (chooser),
					  "clicked",
					  G_CALLBACK (gsb_import_select_file_from_chooser),
					  assistant);

    /* Scroll for tree view. */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (sw, 480, 120);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX(paddingbox), sw, TRUE, TRUE, 6);

    /* Tree view and model. */
    model = GTK_TREE_MODEL (gtk_tree_store_new (IMPORT_FILESEL_NUM_COLS,
												G_TYPE_BOOLEAN,				/* IMPORT_FILESEL_SELECTED checked*/
												G_TYPE_STRING,				/* IMPORT_FILESEL_TYPENAME OFX, CVS QIF*/
												G_TYPE_STRING,				/* IMPORT_FILESEL_FILENAME */
												G_TYPE_STRING,				/* IMPORT_FILESEL_REALNAME */
												G_TYPE_STRING,				/* IMPORT_FILESEL_TYPE */
												G_TYPE_STRING,				/* IMPORT_FILESEL_CODING */
												G_TYPE_STRING));			/* IMPORT_FILESEL_DATE */

	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_widget_set_name (tree_view, "tree_view");

	/* sort by date */
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (model),
									 IMPORT_FILESEL_DATE,
									 (GtkTreeIterCompareFunc) gsb_import_date_sort_function,
									 NULL,
									 NULL);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
										  IMPORT_FILESEL_DATE,
										  GTK_SORT_DESCENDING);

	gtk_container_add (GTK_CONTAINER (sw), tree_view);

    /* Toggle column. */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled", G_CALLBACK (gsb_import_file_selection_cell_renderer_toggled), model);
    column = gtk_tree_view_column_new_with_attributes (_("Import"), renderer,
                        "active", IMPORT_FILESEL_SELECTED,
                        NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    /* Type column. */
    renderer = gtk_cell_renderer_combo_new ();
    g_signal_connect (G_OBJECT (renderer),
					  "edited",
					  G_CALLBACK (gsb_import_switch_type),
					  model);

    list_acc = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));

    tmp_list = ImportFormats;
    while (tmp_list)
    {
        GtkTreeIter iter;
        struct ImportFormat *format = (struct ImportFormat *) tmp_list->data;

        gtk_list_store_append (GTK_LIST_STORE (list_acc), &iter);
        gtk_list_store_set (GTK_LIST_STORE (list_acc), &iter, 0, format->name, -1);

        tmp_list = tmp_list->next;
    }

    g_object_set (renderer,
                        "model", list_acc,
                        "text-column", 0,
                        "editable", TRUE,
                        "editable-set", FALSE,
                        "has-entry", FALSE,
                        NULL);

    column = gtk_tree_view_column_new_with_attributes (_("Type"), renderer,
                        "text", IMPORT_FILESEL_TYPENAME,
                        NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    /* Name column. */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("File name"), renderer,
                        "text", IMPORT_FILESEL_FILENAME,
                        NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

	/* Date column. */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Date"),
													   renderer,
													   "text", IMPORT_FILESEL_DATE,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    g_object_set_data (G_OBJECT(assistant), "tree_view", tree_view);
    g_object_set_data (G_OBJECT(assistant), "model", model);
    g_object_set_data (G_OBJECT(model), "assistant", assistant);

	/* adding option for import of categories for qif files */
    paddingbox = new_paddingbox_with_title (vbox, TRUE, _("Select options to import"));
	qif_button = gsb_automem_checkbutton_new (_("Don't imported the categories"),
										  &w_etat->qif_no_import_categories,
										  NULL,
										  NULL);
	g_object_set_data (G_OBJECT (assistant), "qif_button", qif_button);
	gtk_widget_set_sensitive (qif_button, FALSE);
    gtk_box_pack_start (GTK_BOX (paddingbox), qif_button, FALSE, FALSE, MARGIN_BOX);
    return vbox;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean gsb_import_enter_file_selection_page (GtkWidget *assistant)
{
    GtkTreeModel *model;
	devel_debug (charmap_imported);

    model = g_object_get_data (G_OBJECT (assistant), "model");
    gsb_import_preview_maybe_sensitive_next (assistant, model);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *gsb_import_create_resume_page (GtkWidget *assistant)
{
    GtkWidget *sw;
    GtkWidget *view;
    GtkTextBuffer *buffer;

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    view = gtk_text_view_new ();
    gtk_container_add (GTK_CONTAINER (sw), view);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 12);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 12);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag (buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag (buffer, "indented", "left-margin", 24, NULL);

    g_object_set_data (G_OBJECT (assistant), "text-buffer", buffer);

    return sw;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean gsb_import_enter_resume_page (GtkWidget *assistant)
{
    GSList *files;
    GSList *list;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    const gchar *error_message = "";
	gchar *tmp_str;
	gboolean import_categories = FALSE;

    liste_comptes_importes_error = NULL;
    liste_comptes_importes = NULL;

    /* fichiers sélectionnés dans le gestionnaire de fichiers */
    files = gsb_import_import_selected_files (assistant);
    while (files)
    {
        struct ImportFile *imported = files->data;
        GSList *tmp_list = ImportFormats;

        while (tmp_list)
        {
            struct ImportFormat *format = (struct ImportFormat *) tmp_list->data;

            if (!strcmp (imported->type, format->name))
            {
				devel_debug (imported->type);
				/* importation du fichier sélectionné */
                format->import (assistant, imported);
				if (imported->import_categories)
					import_categories = TRUE;

                tmp_list = tmp_list->next;
                continue;
            }

            tmp_list = tmp_list->next;
        }

        files = files->next;
    }

    buffer = g_object_get_data (G_OBJECT (assistant), "text-buffer");
    gtk_text_buffer_set_text (buffer, "\n", -1);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);

	if (liste_comptes_importes)
    {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, ("Congratulations!"), -1, "x-large", NULL);
        gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

        gtk_text_buffer_insert (buffer,
								&iter,
								_("You successfully imported files into Grisbi.  The "
								  "following pages will help you set up imported data for "
								  "the following files"),
								-1);
        gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

        list = liste_comptes_importes;
        while (list)
        {
            struct ImportAccount *compte;
            compte = list->data;

            /* Fix account name if needed. */
            if (!compte->nom_de_compte)
            {
                compte->nom_de_compte = _("Unnamed Imported account");
            }
            devel_debug (compte->nom_de_compte);

            tmp_str = g_strconcat ("• ", compte->nom_de_compte, " (", compte->origine, ")\n\n", NULL);
            gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, tmp_str , -1, "indented", NULL);
            g_free (tmp_str);

            list = list->next;
        }

        while (gtk_notebook_get_n_pages (g_object_get_data (G_OBJECT (assistant), "notebook")) > IMPORT_FIRST_ACCOUNT_PAGE)
        {
            gtk_notebook_remove_page (g_object_get_data (G_OBJECT (assistant), "notebook"), -1);
        }
        gsb_import_affichage_recapitulatif_importation (assistant);
    }
	else if (import_categories)
	{
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, _("Congratulations!"), -1, "x-large", NULL);
        gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

        gtk_text_buffer_insert (buffer, &iter, _("You successfully imported categories file into Grisbi."), -1);
        gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
	}
    else
    {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, _("Error!"), -1, "x-large", NULL);
        gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

        gtk_text_buffer_insert (buffer,
								&iter,
								_("No file has been imported, please double check that they are "
								  "valid files.  Please make sure that they are not compressed and "
								  "that their format is valid."),
								-1);
        if (strlen (error_message))
        {
            gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
            gtk_text_buffer_insert (buffer, &iter, error_message, -1);
        }
        gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
    }

    if (liste_comptes_importes_error)
    {
        gtk_text_buffer_insert (buffer, &iter, _("The following files are in error: "), -1);
        gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

        list = liste_comptes_importes_error;
        while (list)
        {
            struct ImportAccount *compte;
            compte = list->data;

            tmp_str = g_strconcat ("• ", compte->nom_de_compte, " (", compte->origine, ")\n\n", NULL);
            gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, tmp_str, -1, "indented", NULL);
            g_free (tmp_str);

            list = list->next;
        }
    }

    return FALSE;
}

/**
 * Lookup the budget and create it if necessary
 *
 * \param budget_str          nom du budget à rechercher
 * \param
 *
 * \return void
 **/
static void gsb_import_lookup_budget (struct ImportTransaction *imported_transaction,
									  gint transaction_number)
{
	gint budget_number;
	gchar **tab_str;

	tab_str = g_strsplit (imported_transaction->budget,
					":",
					2);

	/* get the budget and create it if doesn't exist */
	if (tab_str[0])
		tab_str[0] = g_strstrip (tab_str[0]);
	budget_number = gsb_data_budget_get_number_by_name (tab_str[0],
					TRUE,
					imported_transaction->montant.mantissa < 0);
	gsb_data_transaction_set_budgetary_number (transaction_number,
					budget_number);
	if (tab_str[1])
		tab_str[1] = g_strstrip (tab_str[1]);
	gsb_data_transaction_set_sub_budgetary_number (transaction_number,
					gsb_data_budget_get_sub_budget_number_by_name (budget_number,
					tab_str[1],
					TRUE));
	g_strfreev(tab_str);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *gsb_import_progress_bar_affiche (struct ImportAccount *imported_account)
{
    GtkWidget *assistant;
    GtkWidget *progress;
    GtkWidget *hbox;
    GtkWidget *image;
    GtkWidget *bar;

    progress = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated (GTK_WINDOW (progress), FALSE);

    assistant = g_object_get_data (G_OBJECT (grisbi_app_get_active_window (NULL)), "assistant");
    gtk_window_set_modal (GTK_WINDOW (assistant), FALSE);
    gtk_window_set_transient_for (GTK_WINDOW (progress), GTK_WINDOW (assistant));
    gtk_window_set_modal (GTK_WINDOW (progress), TRUE);
    gtk_window_set_position (GTK_WINDOW (progress), GTK_WIN_POS_CENTER_ALWAYS);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (progress), hbox);

    image = gtk_image_new_from_icon_name ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

    bar = gtk_progress_bar_new ();
	gtk_widget_set_valign (bar, GTK_ALIGN_CENTER);
    gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR (bar), 0.1);
    g_object_set_data (G_OBJECT (progress), "bar", bar);
    gtk_box_pack_start (GTK_BOX (hbox), bar, FALSE, FALSE, 6);

    gtk_widget_show_all (progress);

    return progress;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_progress_bar_pulse (GtkWidget *progress,
										   gint nbre_transaction)
{
    GtkWidget *bar;
    gchar *tmp_text;
    gchar *text;

    bar = g_object_get_data (G_OBJECT (progress), "bar");
    gtk_progress_bar_pulse (GTK_PROGRESS_BAR (bar));

    tmp_text = utils_str_itoa (nbre_transaction);
    text = g_strdup_printf (" reste %s transactions à traiter ", tmp_text);
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR (bar), text);

    g_free (tmp_text);
    g_free (text);
}

/**
 * get an imported transaction structure in arg and create the corresponding transaction
 *
 * \param imported_transaction the transaction to import
 * \param account_number the account where to put the new transaction
 * \param
 *
 * \return the number of the new transaction
 **/
static gint gsb_import_create_transaction (struct ImportTransaction *imported_transaction,
										   gint account_number,
										   gchar *origine)
{
    gchar **tab_str;
    gint transaction_number;
    gint payee_number = 0;
    gint fyear = 0;
    gint last_transaction_number;
    gint div_number;
    gint payment_number = 0;
    gchar *tmp_str = NULL;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    if (w_etat->fusion_import_transactions && imported_transaction->ope_correspondante > 0)
        transaction_number = imported_transaction->ope_correspondante;
    else
        /* we create the new transaction */
        transaction_number = gsb_data_transaction_new_transaction (account_number);

    /* get the id if exists */
    if (imported_transaction->id_operation)
        gsb_data_transaction_set_transaction_id (transaction_number, imported_transaction->id_operation);

    /* get the date */
    gsb_data_transaction_set_date (transaction_number, imported_transaction->date);

    /* récupération de la date de valeur */
    if (imported_transaction->date_de_valeur)
    {
		gint contra_number;

        gsb_data_transaction_set_value_date (transaction_number, imported_transaction->date_de_valeur);

        /* set the financial year according to the date or value date */
        if (w_etat->get_fyear_by_value_date)
            fyear = gsb_data_fyear_get_from_date (imported_transaction->date_de_valeur);

		/* set value_date if transfert */
		contra_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);
		if (contra_number)
		{
			gsb_data_transaction_set_value_date (transaction_number, imported_transaction->date_de_valeur);
			gsb_transactions_list_update_transaction (contra_number);

		}
		/* on regarde aussi dans les opérations filles */
		else if (gsb_data_transaction_get_split_of_transaction (transaction_number))
		{
			GSList *children;
			gint tmp_number;

			children = gsb_data_transaction_get_children (transaction_number, TRUE);
			while (children)
			{
				tmp_number = GPOINTER_TO_INT (children->data);

				if (tmp_number > 0)
				{
					contra_number = gsb_data_transaction_get_contra_transaction_number (tmp_number);
					if (contra_number)
					{
						gsb_data_transaction_set_value_date (contra_number, imported_transaction->date_de_valeur);
						gsb_transactions_list_update_transaction (contra_number);
					}
				}

				children = children->next;
			}
			g_slist_free (children);
		}
    }
	/* On force eventuellement la date de valeur des fichiers CSV avec la date */
	else if (origine && g_ascii_strcasecmp (origine, "CSV") == 0)
	{
		if (w_etat->csv_force_date_valeur_with_date)
		{
			gsb_data_transaction_set_value_date (transaction_number, imported_transaction->date);

			/* set the financial year according to the date or value date */
			if (w_etat->get_fyear_by_value_date)
			{
				fyear = gsb_data_fyear_get_from_date (imported_transaction->date_de_valeur);
			}
		}
	}

    /* if no fyear found, get from the date */
	if (fyear <= 0)
		fyear = gsb_data_fyear_get_from_date (imported_transaction->date);
	if (fyear > 0)
		gsb_data_transaction_set_financial_year_number (transaction_number, fyear);

    /* Recovery of payee.
     * we routinely backup imported payee. May be replaced later if
     * notes exist to transactions imported */
	if (imported_transaction->tiers && strlen (imported_transaction->tiers))
	{
		/* Before leaving, we retrieve the data from payee */
		if (w_etat->copy_payee_in_note)
		{
			if (gsb_data_transaction_get_notes (transaction_number) == NULL)
				gsb_data_transaction_set_notes (transaction_number, imported_transaction->tiers);
		}
		if (!imported_transaction->cheque && w_etat->extract_number_for_check)
		{
			gchar *str_to_free = NULL;

			str_to_free = utils_str_my_case_strstr (imported_transaction->tiers, _("Check"));
			if (str_to_free)
			{
				tmp_str = gsb_string_extract_int (imported_transaction->tiers);
				if (tmp_str && strlen (tmp_str) > 0)
				{
					imported_transaction->cheque = tmp_str;
				}
				else if (tmp_str && strlen (tmp_str) == 0)
				{
					g_free (tmp_str);
				}
				g_free (str_to_free);
			}
		}

    	/* on sort de la fonction si on a fusionné des opérations */
		if (w_etat->fusion_import_transactions && imported_transaction->ope_correspondante > 0)
		{
			if (imported_transaction->cheque)
			{
				payment_number = gsb_data_payment_get_number_by_name (_("Check"), account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number, payment_number);
				gsb_data_transaction_set_method_of_payment_content (transaction_number, imported_transaction->cheque);
			}
			return transaction_number;
		}
		else
		{
			payee_number = gsb_import_associations_find_payee (imported_transaction->tiers);
			if (payee_number == 0)
			{
				payee_number = gsb_data_payee_get_number_by_name (imported_transaction->tiers, TRUE);
			}
        	gsb_data_transaction_set_payee_number (transaction_number, payee_number);
		}
	}

	/* récupération du montant */
    gsb_data_transaction_set_amount (transaction_number, imported_transaction->montant);

    /* récupération de la devise */
    gsb_data_transaction_set_currency_number (transaction_number, imported_transaction->devise);

    /* checking if split, otherwise recovery categories */
    if (imported_transaction->operation_ventilee)
    {
        /* transaction is splitted */
        gsb_data_transaction_set_split_of_transaction (transaction_number, 1);
    }
    else
    {
        if (imported_transaction->categ && strlen (imported_transaction->categ))
        {
            /* Fill budget if existing */
            if (imported_transaction->budget && strlen (imported_transaction->budget))
            {
                gsb_import_lookup_budget (imported_transaction, transaction_number);
            }

            if (imported_transaction->categ[0] == '[')
            {
				/* it's a transfer,
				 * we will try to make the link later, for now, we keep the name of the contra account into
				 * the bank references (never used for import)
				 * and set contra_transaction_number to -1 to search the link later */

				gsb_data_transaction_set_bank_references (transaction_number,
														  imported_transaction->categ);
				gsb_data_transaction_set_contra_transaction_number (transaction_number, -1);
				virements_a_chercher = 1;
            }
            else
            {
                /* it's a normal category */
                gint category_number;

                tab_str = g_strsplit (imported_transaction->categ, ":", 2);

                /* get the category and create it if doesn't exist */
                if (tab_str[0])
                    tab_str[0] = g_strstrip (tab_str[0]);
                category_number = gsb_data_category_get_number_by_name (tab_str[0],
																		TRUE,
																		imported_transaction->montant.mantissa < 0);
                gsb_data_transaction_set_category_number (transaction_number, category_number);
                if (tab_str[1])
                {
                    tab_str[1] = g_strstrip (tab_str[1]);
                    gsb_data_transaction_set_sub_category_number (transaction_number,
                                                                  gsb_data_category_get_sub_category_number_by_name
																  (category_number,
																   tab_str[1],
																   TRUE));
                }
                g_strfreev(tab_str);
            }
        }
        else if (payee_number && w_etat->associate_categorie_for_payee &&  !imported_transaction->cheque)
        {
            /* associate the class and the budgetary line to the payee except checks */
            last_transaction_number = gsb_form_transactions_look_for_last_party (payee_number,
																				 transaction_number,
																				 account_number);
            div_number = gsb_data_transaction_get_category_number (last_transaction_number);
            if (div_number != -1)
                gsb_data_transaction_set_category_number (transaction_number, div_number);

            div_number = gsb_data_transaction_get_sub_category_number (last_transaction_number);
            if (div_number != -1)
                gsb_data_transaction_set_sub_category_number (transaction_number, div_number);

            div_number = gsb_data_transaction_get_budgetary_number (last_transaction_number);
            if (div_number != -1)
                gsb_data_transaction_set_budgetary_number (transaction_number, div_number);

            div_number = gsb_data_transaction_get_sub_budgetary_number (last_transaction_number);
            if (div_number != -1)
                gsb_data_transaction_set_sub_budgetary_number (transaction_number, div_number);
        }
        else
        {
            gsb_data_transaction_set_category_number (transaction_number, 0);
            gsb_data_transaction_set_sub_category_number (transaction_number, 0);

            /* Fill budget if existing */
            if (imported_transaction->budget && strlen (imported_transaction->budget))
            {
                gsb_import_lookup_budget(imported_transaction, transaction_number);
            }
        }
    }

    /* récupération des notes */
    if (imported_transaction->notes && strlen (imported_transaction->notes))
    {
        gsb_data_transaction_set_notes (transaction_number, imported_transaction->notes);
    }

    /* définition des moyens de payement des fichiers OFX et QIF*/
    if (origine
		&& (g_ascii_strcasecmp (origine, "OFX") == 0
			|| (g_ascii_strcasecmp (origine, "QIF") == 0 && w_etat->qif_use_field_extract_method_payment)))
    {
        switch (imported_transaction->type_de_transaction)
        {
            case GSB_CHECK:
				/* Check = Chèque */
				payment_number = gsb_data_payment_get_number_by_name (_("Check"), account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				/* we get the check number */
				gsb_data_transaction_set_method_of_payment_content (transaction_number,
																	imported_transaction->cheque);

				break;
            /* case GSB_INT:
				break;
            case GSB_DIV:
				break;
            case GSB_SRVCHG:
				break;
            case GSB_FEE:
				break; */
            case GSB_DEP:
				/* Deposit = Dépôt */
				payment_number = gsb_data_payment_get_number_by_name (_("Deposit"),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				break;
            case GSB_ATM:
				/* Retrait dans un distributeur de billets replacé par carte de crédit */
				payment_number = gsb_data_payment_get_number_by_name (_("Credit card"),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				break;
            case GSB_POS:
				/* Point of sale = Point de vente remplacé par carte de crédit */
				payment_number = gsb_data_payment_get_number_by_name (_("Credit card"),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				break;
            case GSB_XFER:
				/* Transfer = Virement */
				payment_number = gsb_data_payment_get_number_by_name (_("Transfer"),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				break;
            case GSB_PAYMENT:
				/* Electronic payment remplacé par Direct debit = Prélèvement */
				payment_number = gsb_data_payment_get_number_by_name (_("Direct debit"),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				break;
            case GSB_CASH:
				/* Cash withdrawal = retrait en liquide */
				payment_number = gsb_data_payment_get_number_by_name (_("Cash withdrawal"),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				break;
            case GSB_DIRECTDEP:
				/* Direct deposit remplacé par Transfert = Virement */
				payment_number = gsb_data_payment_get_number_by_name (_("Transfer"), account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				/* we get the number */
				gsb_data_transaction_set_method_of_payment_content (transaction_number,
																	imported_transaction->cheque);
				break;
            case GSB_DIRECTDEBIT:
				/* Merchant initiated debit remplacé par Direct debit = Prélèvement */
				payment_number = gsb_data_payment_get_number_by_name (_("Direct debit"),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
				break;
            /* case GSB_REPEATPMT:
            break; */
            case GSB_DEBIT:
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   gsb_data_account_get_default_debit
																   (account_number));
				break;
            case GSB_CREDIT:
                gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   gsb_data_account_get_default_credit
																   (account_number));
				break;
            case GSB_OTHER:
             /* on met sur le type par défaut en fonction du signe du montant*/
            if (gsb_data_transaction_get_amount (transaction_number).mantissa < 0)
                gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   gsb_data_account_get_default_debit
																   (account_number));
            else
                gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   gsb_data_account_get_default_credit
																   (account_number));
           break;
        }
    }
    else	/* définition des moyens de payement des autres fichiers CSV ... */
    {
        /* récupération du chèque et mise en forme du type d'opération */
        if (imported_transaction->cheque)
        {
            /* it's a cheque, try to find a method of payment with automatic increment, if don't find
             * set in default method of payment */
            gint sign;

            if (gsb_data_transaction_get_amount (transaction_number).mantissa < 0)
            {
                sign = GSB_PAYMENT_DEBIT;
                payment_number = gsb_data_account_get_default_debit (account_number);
            }
            else
            {
                sign = GSB_PAYMENT_CREDIT;
                payment_number = gsb_data_account_get_default_credit (account_number);
            }

            if (!gsb_data_payment_get_automatic_numbering (payment_number))
            {
                /* the default method is not an automatic numbering, try to find one */
				/* in the method of payment of this account */
                GSList *tmp_list;

                tmp_list = gsb_data_payment_get_payments_list ();
                while (tmp_list)
                {
                    gint payment_number_tmp;

                    payment_number_tmp = gsb_data_payment_get_number (tmp_list->data);

                    if (gsb_data_payment_get_account_number (payment_number_tmp) == account_number
						&& gsb_data_payment_get_automatic_numbering (payment_number_tmp)
                        && (gsb_data_payment_get_sign (payment_number_tmp) == sign
                           	||
                           	gsb_data_payment_get_sign (payment_number_tmp) == GSB_PAYMENT_NEUTRAL))
                    {
                        payment_number = payment_number_tmp;
                        break;
                    }
                    tmp_list = tmp_list->next;
                }
            }
            /* now, either payment_number is an automatic numbering method of payment,
             * either it's the default method of payment,
             * first save it */
            gsb_data_transaction_set_method_of_payment_number (transaction_number, payment_number);

            if (gsb_data_payment_get_automatic_numbering (payment_number))
            {
                /* we are on the default payment_number, save just the cheque number */
                gsb_data_transaction_set_method_of_payment_content (transaction_number,
																	imported_transaction->cheque);
            }
            else
            {
                /* we are on a automatic numbering payment, we will save the cheque only
                 * if it's not used before, else we show an warning message */
                if (gsb_data_transaction_check_content_payment (payment_number,
																imported_transaction->cheque))
                {
                    tmp_str = g_strdup_printf (_("Warning: the cheque number %s is already used.\n"
												 "We skip it"),
											   imported_transaction->cheque);
                    dialogue_warning (tmp_str);
                    g_free (tmp_str);
                }
                else
                gsb_data_transaction_set_method_of_payment_content (transaction_number,
																	imported_transaction->cheque);
            }
        }
        else /* on regarde si on a un moyen de paiement */
		{
			if (imported_transaction->payment_method)
			{
				payment_number = gsb_data_payment_get_number_by_name (_(imported_transaction->payment_method),
																	  account_number);
				gsb_data_transaction_set_method_of_payment_number (transaction_number,
																   payment_number);
			}
			else
			{
				/* comme on en pas, on met le type par défaut */
				if (gsb_data_transaction_get_amount (transaction_number).mantissa < 0)
					gsb_data_transaction_set_method_of_payment_number (transaction_number,
																	   gsb_data_account_get_default_debit
																	   (account_number));
				else
					gsb_data_transaction_set_method_of_payment_number (transaction_number,
																	   gsb_data_account_get_default_credit
																	   (account_number));
			}
		}
    }

    /* récupération du pointé */
    gsb_data_transaction_set_marked_transaction (transaction_number, imported_transaction->p_r);
	if (imported_transaction->p_r == OPERATION_RAPPROCHEE)
    	marked_r_transactions_imported = TRUE;

    /* si c'est une ope de ventilation, lui ajoute le no de l'opération précédente */
	if (imported_transaction->ope_de_ventilation)
	{
		gsb_data_transaction_set_split_of_transaction (mother_transaction_number, 1);
		gsb_data_transaction_set_mother_transaction_number (transaction_number,
															mother_transaction_number);
	}
	else
		mother_transaction_number  = transaction_number;

    return (transaction_number);
}

/**
 * check or uncheck the operation found
 *
 * \param widget to test
 * \param data for check or uncheck
 *
 * \return void
 **/
static void gsb_import_check_ope_import (GtkWidget *widget,
										 gpointer data)
{
    gint result = GPOINTER_TO_INT (data);

    if (GTK_IS_BOX (widget))
    {
        gtk_container_foreach (GTK_CONTAINER (widget),
                        (GtkCallback) gsb_import_check_ope_import,
                        data);
    }

    if (GTK_IS_TOGGLE_BUTTON (widget))
    {
        GtkDialog *dialog;

        dialog = g_object_get_data (G_OBJECT (widget), "dialog");
        if (result == -12)
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
            gtk_dialog_set_response_sensitive   (dialog, -12, FALSE);
            gtk_dialog_set_response_sensitive   (dialog, -13, TRUE);
        }
        else
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
            gtk_dialog_set_response_sensitive   (dialog, -12, TRUE);
            gtk_dialog_set_response_sensitive   (dialog, -13, FALSE);
        }
    }
}

/**
 * returns false if at least 1 check_button is different of test
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_import_ope_import_test_toggled (GtkWidget *vbox,
													gboolean test)
{
    GList *children;

    children = gtk_container_get_children (GTK_CONTAINER (vbox));

    while (children)
    {
        GtkWidget *widget;

        widget = children->data;
        if (GTK_IS_BOX (widget))
        {
            GList *list;

            list = gtk_container_get_children (GTK_CONTAINER (widget));
            while (list)
            {
                widget = list->data;
                if (GTK_IS_TOGGLE_BUTTON (widget))
                {
                    g_list_free (list);
                    break;
                }
                list = list->next;
            }
        }
        if (GTK_IS_TOGGLE_BUTTON (widget))
        {
            if (test != gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
                return FALSE;
        }
        children = children->next;
    }
    g_list_free (children);

    return TRUE;
}

/**
 * select or unselect buttons of the dialog
 *
 * \param toggle_button
 * \param dialog
 *
 * \return void
 **/
static void gsb_import_ope_import_toggled (GtkWidget *button,
										   GtkWidget *vbox)
{
    gboolean toggle;

    toggle = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

    if (gsb_import_ope_import_test_toggled (vbox, toggle))
    {
        GtkDialog *dialog;

        dialog = g_object_get_data (G_OBJECT (button), "dialog");
        if (toggle == 1)
        {
            gtk_dialog_set_response_sensitive   (dialog, -12, FALSE);
            gtk_dialog_set_response_sensitive   (dialog, -13, TRUE);
        }
        else
        {
            gtk_dialog_set_response_sensitive   (dialog, -12, TRUE);
            gtk_dialog_set_response_sensitive   (dialog, -13, FALSE);
        }
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_correct_opes_import_button_ope_clicked (GtkWidget *button,
                                                               gpointer data)
{
	GtkWidget *dialog;
	gint id;

	id = GPOINTER_TO_INT (data);
	dialog = g_object_get_data (G_OBJECT (button), "dialog");
	gtk_dialog_response (GTK_DIALOG (dialog), id);
}

/**
 * edite les opérations à importer avec problème
 *
 * \param button
 * \param dialog
 *
 * \return void
 **/
static gint gsb_import_correct_opes_find_multiples_ope_msg (GtkWidget *parent,
                                                            GSList *list_resultats,
                                                            const gchar *tiers)
{
    GtkWidget *dialog;
    GtkWidget *frame;
    GtkWidget *scrolled_window;
	GtkWidget *separator;
    GtkWidget *vbox;
    GSList *tmp_list;
    gchar *tmp_str2;
	gint result;
	gint tmp_number;

	tmp_str2 = g_strdup_printf (_("Choose from the list below that which will be selected \n"
	                              "for the payee: \"%s\""),
	                            tiers);
	dialog = dialogue_special_no_run (GTK_MESSAGE_INFO,
	                                  GTK_BUTTONS_NONE,
	                                  tmp_str2,
	                                  _("Several transactions have been found"));
	g_free (tmp_str2);

    gtk_window_set_default_size (GTK_WINDOW (dialog), 770, 500);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER(dialog), BOX_BORDER_WIDTH);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));

    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), frame, TRUE, TRUE, 0);
    gtk_widget_show (frame);

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                        GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (frame), scrolled_window);
    gtk_widget_show (scrolled_window);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_add (GTK_CONTAINER (scrolled_window), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BOX_BORDER_WIDTH);
    gtk_widget_show (vbox);

	/* traitement des opérations trouvées qui ont le même montant*/
	tmp_number = GPOINTER_TO_INT (list_resultats->data);
	tmp_str2 = utils_real_get_string (gsb_data_transaction_get_amount (tmp_number));

	tmp_list = list_resultats;
	while (tmp_list)
    {
		GtkWidget *button;
    	gchar *tmp_str;
		gchar *tmp_date;
		const gchar *payee;
		gint transaction_number;

		transaction_number = GPOINTER_TO_INT (tmp_list->data);
		payee = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number (transaction_number),
										 FALSE);
		tmp_date = gsb_format_gdate (gsb_data_transaction_get_date (transaction_number));
		tmp_str = g_strdup_printf (_("Transaction N° %d found: %s ; %s ; %s"),
		                           transaction_number,
								   tmp_date,
		                           payee,
		                           tmp_str2);
		g_free(tmp_date);
		button = gtk_button_new_with_label (tmp_str);
		g_object_set_data (G_OBJECT (button), "dialog", dialog);
		gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
		gtk_widget_show (button);
		g_free (tmp_str);

		/* Add a separator */
		separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
		gtk_widget_show (separator);

		/* set signals */
		g_signal_connect (G_OBJECT (button),
						  "clicked",
						  G_CALLBACK (gsb_import_correct_opes_import_button_ope_clicked),
						  GINT_TO_POINTER (transaction_number));

		tmp_list = tmp_list->next;
	}
	g_free (tmp_str2);
	gtk_widget_show (dialog);

	result = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return result;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_correct_opes_import_button_find_clicked (GtkWidget *button,
                                                                struct ImportTransaction *ope_import)
{
	GSList *list_ope_retenues;
	GSList *list_resultats = NULL;
	GSList *tmp_list_ope_retenues;
	GDate *date_debut_comparaison;
	GDate *date_fin_comparaison;
	GDateDay day;
	GDateMonth month;
	GDateYear year;
	gint account_number;
	gint nbre_resultat = 0;
	GrisbiWinEtat *w_etat;

	devel_debug (NULL);
	w_etat = grisbi_win_get_w_etat ();
	account_number = gsb_data_transaction_get_account_number (ope_import->ope_correspondante);

	/* on calcule l'intervalle de recherche */
	day = g_date_get_day (ope_import->date);
	month = g_date_get_month (ope_import->date);
	year = g_date_get_year (ope_import->date);

	date_debut_comparaison = g_date_new_dmy (day, month, year);
	g_date_subtract_days (date_debut_comparaison, w_etat->import_files_nb_days);

	date_fin_comparaison = g_date_new_dmy (day, month, year);
	g_date_add_days (date_fin_comparaison, w_etat->import_files_nb_days);

	/* on récupère la liste des opérations à partir de l'opération correspondante trouvée */
	list_ope_retenues = gsb_data_transaction_get_list_for_import (account_number, date_debut_comparaison);

	tmp_list_ope_retenues = list_ope_retenues;
	while (tmp_list_ope_retenues)
	{
		gint transaction_number;

		transaction_number = GPOINTER_TO_INT (tmp_list_ope_retenues->data);

		tmp_list_ope_retenues = tmp_list_ope_retenues->next;

		if (transaction_number == ope_import->ope_correspondante)
			continue;
		else if (gsb_data_transaction_get_account_number (transaction_number) == account_number)
		{
            if (!gsb_real_cmp (gsb_data_transaction_get_amount (
             transaction_number), ope_import->montant)
             &&
             (g_date_compare (gsb_data_transaction_get_date (
               transaction_number), date_debut_comparaison) >= 0)
             &&
             (g_date_compare (gsb_data_transaction_get_date (
               transaction_number), date_fin_comparaison) <= 0)
             &&
             !ope_import->ope_de_ventilation
             &&
             (!w_etat->fusion_import_transactions
             ||
             !gsb_data_transaction_get_id (transaction_number)))
            {
				nbre_resultat++;
				list_resultats = g_slist_append (list_resultats, GINT_TO_POINTER (transaction_number));
			}
		}
	}
	if (list_ope_retenues)
		g_slist_free (list_ope_retenues);

	if (nbre_resultat == 0)
	{
		gboolean toggle;

    	toggle = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ope_import->bouton));

		if (toggle)
		{
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ope_import->bouton), FALSE);
		}

		dialogue_warning (_("No other operation was found.\n"
		                    "In this case it is a new operation and\n"
							"the associated box has been unchecked"));

		return;
	}
	else
	{
		const gchar *tiers;
		gchar *tmp_str;
		gchar *tmp_str2;
		gchar *tmp_date;

		if (nbre_resultat == 1)
		{
			ope_import->ope_correspondante = GPOINTER_TO_INT (list_resultats->data);
			dialogue_warning (_("Only one operation was found. \n"
								"The correction has been made."));
		}
		else if (nbre_resultat > 1)
		{
			GtkWidget *parent;
			gint result;

			parent = g_object_get_data (G_OBJECT (ope_import->bouton), "dialog");
			result = gsb_import_correct_opes_find_multiples_ope_msg (parent, list_resultats, ope_import->tiers);
			ope_import->ope_correspondante = result;
		}

		tiers = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number (ope_import->ope_correspondante), FALSE);
		if (gsb_data_transaction_get_notes (ope_import->ope_correspondante))
		{
			tmp_str2 = utils_real_get_string (gsb_data_transaction_get_amount (
								ope_import->ope_correspondante));
			tmp_date = gsb_format_gdate (gsb_data_transaction_get_date (ope_import->ope_correspondante));
			tmp_str = g_strdup_printf (_("Transaction found: %s ; %s ; %s ; %s"),
						tmp_date,
						tiers,
						tmp_str2,
						gsb_data_transaction_get_notes (ope_import->ope_correspondante));
			g_free(tmp_date);
			g_free (tmp_str2);
			gtk_label_set_text (GTK_LABEL (ope_import->label_ope_find), tmp_str);
			g_free (tmp_str);
		}
		else
		{
			tmp_str2 = utils_real_get_string (gsb_data_transaction_get_amount (
								ope_import->ope_correspondante));
			tmp_date = gsb_format_gdate (gsb_data_transaction_get_date (ope_import->ope_correspondante));
			tmp_str = g_strdup_printf (_("Transaction found: %s ; %s ; %s"),
						tmp_date,
						tiers,
						tmp_str2);
			g_free(tmp_date);
			g_free (tmp_str2);
			gtk_label_set_text (GTK_LABEL (ope_import->label_ope_find), tmp_str);
			g_free (tmp_str);
		}
	}
}

/**
 * création de la boite pour le traitement des doublons
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static GtkWidget *gsb_import_correct_opes_import_create_box_doublons (GtkWidget *vbox,
                                                                      GSList *list_ope_importees,
                                                                      gint transaction_number)
{
	GtkWidget *button_change;
	GtkWidget *hbox;
    GtkWidget *label;
	GSList *tmp_list_ope;
    gchar *tmp_str2;
	gchar *tmp_date;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

	/* traitement des opérations */
	tmp_str2 = utils_real_get_string (gsb_data_transaction_get_amount (transaction_number));

	tmp_list_ope = list_ope_importees;
	while (tmp_list_ope)
	{
		gchar *tmp_str;
		struct ImportTransaction *ope_import;

		ope_import = tmp_list_ope->data;
		if (transaction_number == ope_import->ope_correspondante)
		{
			const gchar *tiers;

			/* Opération existante */
			tiers = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number (ope_import->ope_correspondante),
											 FALSE);
			tmp_date = gsb_format_gdate (gsb_data_transaction_get_date
										 (ope_import->ope_correspondante));
			tmp_str = g_strdup_printf (_("Transaction found: %s ; %s ; %s"),
									   tmp_date,
									   tiers,
									   tmp_str2);
			g_free(tmp_date);
			label = gtk_label_new (tmp_str);
			utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
			g_free (tmp_str);
			gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
			gtk_widget_show (label);

			/* Opération importée */
			tmp_date = gsb_format_gdate (ope_import->date);
			if (w_etat->fusion_import_transactions)
				tmp_str = g_strdup_printf (_("Transaction to be merged: %s ; %s ; %s"),
										   tmp_date,
										   ope_import->tiers,
										   tmp_str2);
			else
				tmp_str = g_strdup_printf (_("Transaction to import: %s ; %s ; %s"),
										   tmp_date,
										   ope_import->tiers,
										   tmp_str2);
			g_free(tmp_date);
			label = gtk_label_new (tmp_str);
			utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
			g_free (tmp_str);
			gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
			gtk_widget_show (label);

			/* Boite de traitement */
			hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
			gtk_widget_show (hbox);

			/* Ajout du bouton de traitement */
			button_change = gtk_button_new_with_label (_("Find other transaction"));
			gtk_box_pack_start (GTK_BOX (hbox), button_change, TRUE, TRUE, 0);
			gtk_widget_show (button_change);
			g_signal_connect (G_OBJECT (button_change),
							  "clicked",
							  G_CALLBACK (gsb_import_correct_opes_import_button_find_clicked),
							  ope_import);
		}
		tmp_list_ope = tmp_list_ope->next;
	}
	g_free (tmp_str2);

	return vbox;
}

/**
 * called if we are not sure about some transactions to import
 * ask here to the user
 *
 * \param imported_account
 * \param
 *
 * \return
 **/
static void gsb_import_confirmation_enregistrement_ope_import (struct ImportAccount *imported_account,
															   gint account_number,
															   GtkWindow *parent)
{
    GtkWidget *dialog;
	GtkWidget *button_OK;
	GtkWidget *button_select_all;
	GtkWidget *button_unselect_all;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *scrolled_window;
    GtkWidget *label;
    GtkWidget *frame;
    GtkWidget *frame_alert;
	GtkWidget *hbox_alert;
	GtkWidget *label_alert;
	GtkWidget *vbox_alert;
    GtkWidget *vbox_alert_lignes;
	GtkWidget *image;
	GSList *list_ope_correspondantes = NULL;
	GSList *list_ope_doublons = NULL;
    GSList *tmp_list;
    gchar *tmp_str;
    gchar *tmp_str2;
    gint return_exponent;
    gint action_derniere_ventilation;
    gint result;
    gboolean ope_visible = FALSE;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

    /* pbiava the 03/17/2009 modifications pour la fusion des opérations */
    if (w_etat->fusion_import_transactions)
	{
        tmp_str = g_strdup_printf (_("Confirmation of transactions to be merged in: %s"),
								   gsb_data_account_get_name (account_number));
	}
    else
	{
        tmp_str = g_strdup_printf (_("Confirmation of importation of transactions in: %s"),
								   gsb_data_account_get_name (account_number));
	}

	if (parent == NULL)
		parent = GTK_WINDOW (grisbi_app_get_active_window (NULL));


    dialog = gtk_dialog_new_with_buttons (tmp_str,
										  GTK_WINDOW (parent),
										  GTK_DIALOG_MODAL,
										  NULL, NULL,
										  NULL);
    g_free (tmp_str);

	button_select_all = gtk_button_new_with_label (_("Select all"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_select_all, -12);

	button_unselect_all = gtk_button_new_with_label (_("Unselect all"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_unselect_all, -13);

	button_OK = gtk_button_new_with_label (_("Validate"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_OK, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_OK, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    gtk_window_set_default_size (GTK_WINDOW (dialog), 770, 500);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER(dialog), BOX_BORDER_WIDTH);

    if (w_etat->fusion_import_transactions)
    {
        gtk_dialog_set_response_sensitive   (GTK_DIALOG (dialog), -12, FALSE);
        tmp_str = g_strdup ( _("Please select the transactions to be merged"));
    }
    else
    {
        gtk_dialog_set_response_sensitive   (GTK_DIALOG (dialog), -13, FALSE);
        tmp_str = g_strdup (
                        _("Some imported transactions seem to be already saved."
                        "Please select the transactions to import."));
    }
    label = gtk_label_new (tmp_str);
    utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
    gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), label, FALSE, FALSE, BOX_BORDER_WIDTH);
    g_free (tmp_str);

    /* set the decoration */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), frame, TRUE, TRUE, 0);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_add (GTK_CONTAINER (scrolled_window), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BOX_BORDER_WIDTH);

	/* On crée la boite d'alerte quand plusieurs transactions sont fusionnées avec une transaction existante */
    frame_alert = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_alert), GTK_SHADOW_OUT);
    gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), frame_alert, FALSE, FALSE, MARGIN_BOX);
	gtk_widget_set_no_show_all (frame_alert, TRUE);

	vbox_alert = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
	gtk_container_add (GTK_CONTAINER (frame_alert), vbox_alert);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_alert), BOX_BORDER_WIDTH);

	hbox_alert = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_box_pack_start (GTK_BOX (vbox_alert), hbox_alert, TRUE, TRUE, 0);

	image = gtk_image_new_from_icon_name ("gtk-dialog-warning", GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_box_pack_start (GTK_BOX (hbox_alert), image, FALSE, FALSE, 0);

	label_alert = gtk_label_new (NULL);
	tmp_str = dialogue_make_pango_attribut ("weight=\"bold\"", _("Duplicate transactions"));
    gtk_label_set_markup_with_mnemonic (GTK_LABEL (label_alert), tmp_str);
    g_free (tmp_str);

	gtk_label_set_xalign (GTK_LABEL (label_alert), GSB_CENTER);
	gtk_box_pack_start (GTK_BOX (hbox_alert), label_alert, TRUE, TRUE, 0);

	scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (vbox_alert), scrolled_window);
	gtk_widget_set_size_request (scrolled_window, -1, SW_MIN_HEIGHT);

    vbox_alert_lignes = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_add (GTK_CONTAINER (scrolled_window), vbox_alert_lignes);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_alert_lignes), BOX_BORDER_WIDTH);

    /* on fait maintenant le tour des opés importées et affichent celles à problème */
    tmp_list = imported_account->operations_importees;
    while (tmp_list)
    {
		struct ImportTransaction *ope_import;

		ope_import = tmp_list->data;

		/* on n'affiche pas si c'est des opés de ventil, si la mère est cochée, les filles
		 * seront alors cochées on ne teste pas ici car ça a été testé avant */
		if (ope_import->action == IMPORT_TRANSACTION_ASK_FOR_TRANSACTION
			 &&
			 !ope_import->ope_de_ventilation)
		{
			const gchar *tiers;
			gchar *tmp_date;

			/* on est certain d'afficher une opération au moins */
			ope_visible = TRUE;

			hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

			ope_import->bouton = gtk_check_button_new ();
			if (w_etat->fusion_import_transactions)
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ope_import->bouton), TRUE);
			g_object_set_data (G_OBJECT (ope_import->bouton), "dialog", dialog);
			g_signal_connect (ope_import->bouton,
							  "toggled",
							  G_CALLBACK (gsb_import_ope_import_toggled),
							  vbox);
			gtk_box_pack_start (GTK_BOX (hbox), ope_import->bouton, FALSE, FALSE, 0);

			return_exponent = gsb_data_account_get_currency_floating_point (account_number);
			tmp_str2 = utils_real_get_string (gsb_real_adjust_exponent (ope_import->montant, return_exponent));
			tmp_date = gsb_format_gdate (ope_import->date);
			if (w_etat->fusion_import_transactions)
				tmp_str = g_strdup_printf (_("Transaction to be merged: %s ; %s ; %s"),
							tmp_date,
							ope_import->tiers,
							tmp_str2);
			else
				tmp_str = g_strdup_printf (_("Transaction to import: %s ; %s ; %s"),
							tmp_date,
							ope_import->tiers,
							tmp_str2);
			g_free(tmp_date);
			g_free (tmp_str2);
			label = gtk_label_new (tmp_str);
			g_free (tmp_str);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

			hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

			label = gtk_label_new ("       ");
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

			/* on garde trace des opérations correspondantes pour avertir l'utilisateur d'un problème de fusion */
			if (g_slist_find (list_ope_correspondantes, GINT_TO_POINTER (ope_import->ope_correspondante)))
			{
				if (!g_slist_find (list_ope_doublons, GINT_TO_POINTER (ope_import->ope_correspondante)))
				{
					gchar *tmp_str3;

					list_ope_doublons = g_slist_append (list_ope_doublons,
														GINT_TO_POINTER (ope_import->ope_correspondante));

					/* on ajoute le widget de traitement de la ligne */
					gsb_import_correct_opes_import_create_box_doublons (vbox_alert_lignes,
																		imported_account->operations_importees,
																		ope_import->ope_correspondante);
					tmp_str2 = g_strdup_printf (" : %d", 2*g_slist_length (list_ope_doublons));
					tmp_str3 = g_strconcat (_("Duplicate transactions"), tmp_str2, NULL);
					tmp_str = dialogue_make_pango_attribut ("weight=\"bold\"", tmp_str3);
					gtk_label_set_markup_with_mnemonic (GTK_LABEL (label_alert), tmp_str);
					g_free (tmp_str);
					g_free (tmp_str2);
					g_free (tmp_str3);
					gtk_widget_set_no_show_all (frame_alert, FALSE);
				}
			}
			else
			{
				list_ope_correspondantes = g_slist_append (list_ope_correspondantes,
				                                           GINT_TO_POINTER (ope_import->ope_correspondante));
			}

			tiers = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number (
							ope_import->ope_correspondante), FALSE);
			if (gsb_data_transaction_get_notes (ope_import->ope_correspondante))
			{
				tmp_str2 = utils_real_get_string (gsb_data_transaction_get_amount (
									ope_import->ope_correspondante));
				tmp_date = gsb_format_gdate (gsb_data_transaction_get_date (
																			ope_import->ope_correspondante));
				tmp_str = g_strdup_printf (_("Transaction found: %s ; %s ; %s ; %s"),
							tmp_date,
							tiers,
							tmp_str2,
							gsb_data_transaction_get_notes (ope_import->ope_correspondante));
				g_free(tmp_date);
				g_free (tmp_str2);
				ope_import->label_ope_find = gtk_label_new (tmp_str);
				g_free (tmp_str);
			}
			else
			{
				tmp_str2 = utils_real_get_string (gsb_data_transaction_get_amount (
									ope_import->ope_correspondante));
				tmp_date = gsb_format_gdate (gsb_data_transaction_get_date (
																			ope_import->ope_correspondante));
				tmp_str = g_strdup_printf (_("Transaction found: %s ; %s ; %s"),
							tmp_date,
							tiers,
							tmp_str2);
				g_free(tmp_date);
				g_free (tmp_str2);
				ope_import->label_ope_find = gtk_label_new (tmp_str);
				g_free (tmp_str);
			}

			gtk_box_pack_start (GTK_BOX (hbox), ope_import->label_ope_find, FALSE, FALSE, 0);
		}
		tmp_list = tmp_list->next;
    }

    /* si on n'a rien a afficher on sort */
    if (!ope_visible)
    {
        gtk_widget_destroy (dialog);
        return;
    }
	else
		gtk_widget_show_all (dialog);

	dialog_return:
    result = gtk_dialog_run (GTK_DIALOG (dialog));

    if (result <= -12)
    {
        gtk_container_foreach (GTK_CONTAINER (vbox),
                        (GtkCallback) gsb_import_check_ope_import,
                        GINT_TO_POINTER (result));
        goto dialog_return;
    }

    /* on fait maintenant le tour des check buttons pour voir ce qu'on importe */
    tmp_list = imported_account->operations_importees;
    action_derniere_ventilation = 1;

    while (tmp_list)
    {
        struct ImportTransaction *ope_import;

        ope_import = tmp_list->data;

        /* on ne fait rien pour une opération existante */
        if (ope_import->action == IMPORT_TRANSACTION_LEAVE_TRANSACTION)
        {
            tmp_list = tmp_list->next;
            continue;
        }

        /* si c'est une opé de ventil, elle n'était pas affichée, dans ce cas si l'action de la
           dernière ventil était 0, on fait de même pour les filles */

        if (ope_import->ope_de_ventilation)
        {
            if (ope_import->action)
            ope_import->action = action_derniere_ventilation;
        }
        else
            action_derniere_ventilation = 1;

        if (ope_import->bouton
             &&
             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ope_import->bouton)))
        {
            ope_import->action = 0;

            /* si c'était une ventil on met l'action de la dernière ventil à 0 */

            if (ope_import->operation_ventilee)
                action_derniere_ventilation = 0;
        }
        else if (w_etat->fusion_import_transactions &&
                        ope_import->ope_correspondante > 0)
        {
            ope_import->action = 0;
            if (ope_import->operation_ventilee)
                action_derniere_ventilation = 0;
            ope_import->ope_correspondante = 0;
        }
        tmp_list = tmp_list->next;
    }
	if (list_ope_correspondantes)
		g_slist_free (list_ope_correspondantes);

	if (list_ope_doublons)
		g_slist_free (list_ope_doublons);

    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * return
 **/
static gboolean gsb_import_define_action (struct ImportAccount *imported_account,
										  gint account_number,
										  GDate *first_date_import)
{
    GSList *list_ope_retenues;
    GSList *tmp_list;
    gint demande_confirmation = FALSE;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

    /* on récupère la liste des opérations dans l'intervalle de recherche pour l'import */
    list_ope_retenues = gsb_data_transaction_get_list_for_import (account_number, first_date_import);

    tmp_list = imported_account->operations_importees;

    while (tmp_list)
    {
        GSList *tmp_list_ope_retenues;
        struct ImportTransaction *imported_transaction;

        imported_transaction = tmp_list->data;

        tmp_list_ope_retenues = list_ope_retenues;
        while (tmp_list_ope_retenues)
        {
            gint transaction_number;
            const gchar *tmp_str;
            GDate *date_debut_comparaison;
            GDate *date_fin_comparaison;
            GDateDay day;
            GDateMonth month;
            GDateYear year;

            transaction_number = GPOINTER_TO_INT (tmp_list_ope_retenues->data);

            tmp_list_ope_retenues = tmp_list_ope_retenues->next;

            /* first check the id */
            if (imported_transaction->id_operation)
            {
                tmp_str = gsb_data_transaction_get_id (transaction_number);
                if (tmp_str && strcmp (imported_transaction->id_operation, tmp_str) == 0)
                {
                    imported_transaction->action = IMPORT_TRANSACTION_LEAVE_TRANSACTION;
                    break;
                }
            }

            /* if no id, check the cheque */
            if (imported_transaction->cheque)
            {
                tmp_str = gsb_data_transaction_get_method_of_payment_content (transaction_number);
                if (tmp_str && strcmp (imported_transaction->cheque, tmp_str) == 0)
                {
                    if (w_etat->fusion_import_transactions)
                    {
                        imported_transaction->action = IMPORT_TRANSACTION_ASK_FOR_TRANSACTION;
                        imported_transaction->ope_correspondante = transaction_number;
                        demande_confirmation = TRUE;
                    }
                    else
                    {
                        /* found the cheque, forget that transaction */
                        imported_transaction->action = IMPORT_TRANSACTION_LEAVE_TRANSACTION;
                    }
                    break;
                }
                if (tmp_str)
                    continue;
            }

            /* no id, no cheque, try to find the transaction */
            day = g_date_get_day (imported_transaction->date);
            month = g_date_get_month (imported_transaction->date);
            year = g_date_get_year (imported_transaction->date);

            date_debut_comparaison = g_date_new_dmy (day, month, year);
            g_date_subtract_days (date_debut_comparaison, w_etat->import_files_nb_days);

            date_fin_comparaison = g_date_new_dmy (day, month, year);
            g_date_add_days (date_fin_comparaison, w_etat->import_files_nb_days);

            if (!gsb_real_cmp (gsb_data_transaction_get_amount (
             transaction_number), imported_transaction->montant)
             &&
             (g_date_compare (gsb_data_transaction_get_date (
               transaction_number), date_debut_comparaison) >= 0)
             &&
             (g_date_compare (gsb_data_transaction_get_date (
               transaction_number), date_fin_comparaison) <= 0)
             &&
             !imported_transaction->ope_de_ventilation
             &&
             (!w_etat->fusion_import_transactions
             ||
             !gsb_data_transaction_get_id (transaction_number)))
            {
                /* the imported transaction has the same date and same amount,
                 * will ask the user */
                imported_transaction->action = IMPORT_TRANSACTION_ASK_FOR_TRANSACTION;
                imported_transaction->ope_correspondante = transaction_number;
                demande_confirmation = TRUE;
            }
            g_date_free (date_debut_comparaison);
            g_date_free (date_fin_comparaison);
        }
        tmp_list = tmp_list->next;
    }

    if (list_ope_retenues)
        g_slist_free (list_ope_retenues);

    return demande_confirmation;
}

/**
 * get first date of the imported file
 *
 *
 * \param
 *
 * return
 **/
static GDate *gsb_import_get_first_date (GSList *import_list)
{
    GSList *tmp_list;
    GDate *first_date = NULL;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

    tmp_list = import_list;

    while (tmp_list)
    {
        struct ImportTransaction *imported_transaction;

        imported_transaction = tmp_list->data;

        if (!first_date
         ||
         g_date_compare (imported_transaction->date, first_date) < 0)
            first_date = imported_transaction->date;

        tmp_list = tmp_list->next;
    }

    first_date = gsb_date_copy (first_date);
    g_date_subtract_days (first_date, w_etat->import_files_nb_days);

    return first_date;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_import_set_id_compte (gint account_nb,
										  gchar *imported_id)
{
    gchar *account_id;

    account_id = gsb_data_account_get_id (account_nb);
    if (account_id)
	{
	    if (g_ascii_strcasecmp (account_id, imported_id))
	    {
			gchar *tmp_str;
			gchar *tmp_str2;

		/* l'id du compte choisi et l'id du compte importé sont différents */
		/* on propose encore d'arrêter... */
			tmp_str2 = my_strdup (imported_id);
			tmp_str = g_strdup_printf (_("Perhaps you choose a wrong account?\n"
										 "If you choose to continue, the id of the account will be changed\n"
										 " for: %s\n"
										 "If not, the account will be ignored.\n\n"
										 "Do you want to continue?"), tmp_str2);
            if (dialogue_yes_no (tmp_str,
								 _("The id of the imported and chosen accounts are different"),
								 GTK_RESPONSE_NO))
			{
                gsb_data_account_set_id (account_nb, tmp_str2);
				g_free (tmp_str);
				g_free (tmp_str2);
			}
            else
			{
				g_free (tmp_str);
				g_free (tmp_str2);
                return FALSE;
			}
	    }
	}
	else
	    gsb_data_account_set_id (account_nb, my_strdup (imported_id));

    return TRUE;
}

/**
 * import the transactions in an existent account
 * check the id of the account and if the transaction already exists
 * add too the transactions to the gui
 *
 * \param imported_account an imported structure account which contains the transactions
 * \param account_number the number of account where we want to put the new transations
 *
 * \return
 **/
static void gsb_import_add_imported_transactions (struct ImportAccount *imported_account,
												  gint account_number,
												  GtkWindow *parent)
{
    GSList *tmp_list;
    GDate *first_date_import = NULL;
    gint demande_confirmation;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

    /* check the imported account id, and set it in the grisbi account if it doesn't
     * exist or if it's wrong */
    if (imported_account->id_compte)
    {
        if (!gsb_import_set_id_compte (account_number, imported_account->id_compte))
            return;
    }

    /* on fait un premier tour de la liste des opés pour repérer celles qui sont déjà entrées
     * si on n'importe que du ofx, c'est facile, chaque opé est repérée par une id
     * donc si l'opé importée a une id, il suffit de rechercher l'id dans le compte, si elle
     * n'y est pas l'opé est à enregistrer
     * si on importe du qif, il n'y a pas d'id. donc soit on retrouve une opé semblable
     * (cad même montant et même date, on ne fait pas joujou avec le tiers car l'utilisateur
     * a pu le changer), et on demande à l'utilisateur quoi faire, sinon on enregistre l'opé
     */

    /* pour gagner en rapidité, on va récupérer la date de la première
     * opération qui est dans le fichier importé */
    first_date_import = gsb_import_get_first_date (imported_account->operations_importees);

    /* ok, now first_date_import contains the first transaction date used in that account,
     * can check the imported transactions */
    demande_confirmation = gsb_import_define_action (imported_account, account_number, first_date_import);

    g_date_free (first_date_import);

    /* if we are not sure about some transactions, ask now */
    if (demande_confirmation)
        gsb_import_confirmation_enregistrement_ope_import (imported_account, account_number, parent);

    /* ok, now we know what to do for each transactions, can import to the account */
    mother_transaction_number = 0;

    tmp_list = imported_account->operations_importees;

    while (tmp_list)
    {
		struct ImportTransaction *imported_transaction;

		imported_transaction = tmp_list->data;

		if (imported_transaction->action == IMPORT_TRANSACTION_GET_TRANSACTION)
		{
			gint transaction_number;

			/* set the account currency to the transaction and create the transaction */
			if (imported_account->bouton_devise)
				imported_transaction->devise = gsb_currency_get_currency_from_combobox (
							imported_account->bouton_devise);
			else
				imported_transaction->devise = gsb_data_currency_get_number_by_code_iso4217 (
                        imported_account->devise);

	    transaction_number = gsb_import_create_transaction (imported_transaction,
																account_number,
																imported_account->origine);

			if (w_etat->fusion_import_transactions
				&& imported_transaction->ope_correspondante > 0)
        {
            gsb_transactions_list_update_transaction (transaction_number);
            tmp_list = tmp_list->next;
				continue;
			}

	    /* invert the amount of the transaction if asked */
	    if (imported_account->invert_transaction_amount)
            gsb_data_transaction_set_amount (transaction_number,
												 gsb_real_opposite
												 (gsb_data_transaction_get_amount (transaction_number)));

	    /* we need to add the transaction now to the tree model here
	     * to avoid to write again all the account */
			gsb_transactions_list_append_new_transaction (transaction_number, FALSE);
		}
		tmp_list = tmp_list->next;
    }

    /* if we are on the current account, we need to update the tree_view */
    if (gsb_gui_navigation_get_current_account () == account_number)
    {
        gsb_transactions_list_update_tree_view (account_number, TRUE);
        gsb_data_account_colorize_current_balance (account_number);
    }
}

/**
 * check if the transaction 2 is the link of the transaction 1
 * called after importing to create the link
 *
 * \param transaction_number
 * \param tested_transaction
 *
 * \return TRUE : the transactions are linked, FALSE : the transactions are different
 **/
static gboolean gsb_import_check_transaction_link (gint transaction_number,
												   gint tested_transaction)
{
    gchar *contra_account_name;
    gint contra_account_number;
    GsbReal amount_1, amount_2;

    /* we check first the easy and quick things, and if all ok, comes after the check
     * which need some time */

    /* check if the contra transaction number of the tested transaction is -1 (ie imported transfer) */
    if (gsb_data_transaction_get_contra_transaction_number (tested_transaction) != -1)
	return FALSE;

    /* check if there is a contra account name */
    if (!gsb_data_transaction_get_bank_references (tested_transaction))
	return FALSE;

    /* check same payee */
    if (gsb_data_transaction_get_payee_number (transaction_number) != gsb_data_transaction_get_payee_number (tested_transaction))
	return FALSE;

    /* check same date */
    if (g_date_compare (gsb_data_transaction_get_date (transaction_number),
			 gsb_data_transaction_get_date (tested_transaction)))
	return FALSE;

    /* check if same amount (but opposite) */
    amount_1 = gsb_real_abs (gsb_data_transaction_get_amount (transaction_number));
    amount_2 = gsb_real_abs (gsb_data_transaction_get_adjusted_amount_for_currency (tested_transaction,
										     gsb_data_transaction_get_currency_number (transaction_number),
										     -1));
    if (gsb_real_cmp (amount_1, amount_2))
	return FALSE;

    /* check if the contra account name of the tested transaction is the account of the transaction */
    contra_account_name = my_strdelimit (gsb_data_transaction_get_bank_references (tested_transaction), "[]", "");
    contra_account_number = gsb_data_account_get_no_account_by_name (contra_account_name);
    g_free (contra_account_name);

    if (gsb_data_transaction_get_account_number (transaction_number) != contra_account_number)
	return FALSE;

    /* all seems the same, can return TRUE */
    return TRUE;
}

/**
 * called at the end of an import, check all the transactions which need a link
 * to another transaction.
 * we can find that transactions because they have contra_transaction_number to -1
 * and the bank_references is the name of the contra account, with [ ]
 *
 * that function try to find the contra-transaction and create the link
 *
 * \param
 *
 * \return
 **/
static void gsb_import_cree_liens_virements_ope_import (void)
{
    GSList *tmp_list_transactions;

	tmp_list_transactions = gsb_data_transaction_get_transactions_list ();
    while (tmp_list_transactions)
    {
		gint transaction_number_tmp;

		transaction_number_tmp = gsb_data_transaction_get_transaction_number (tmp_list_transactions->data);

		/* if the contra transaction number is -1, it's a transfer, */
		/* in that case, the name of the contra account is in the bank_references */
		if (gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp)== -1
			 && gsb_data_transaction_get_bank_references (transaction_number_tmp))
		{
			/* the name of the contra account is in the bank references with [and] */
			gchar *contra_account_name;
			gint contra_account_number;

			contra_account_name = my_strdelimit (gsb_data_transaction_get_bank_references (transaction_number_tmp),
												 "[]", "");
			contra_account_number = gsb_data_account_get_no_account_by_name (contra_account_name);

			/* now we needn't the bank reference anymore */
			gsb_data_transaction_set_bank_references (transaction_number_tmp, NULL);

			if (contra_account_number == -1)
			{
				gchar *tmp_str;

				/* we have not found the contra-account */
				gsb_data_transaction_set_contra_transaction_number (transaction_number_tmp, 0);
				tmp_str = g_strdup_printf ( _("You have imported transaction of transfer to an "
											  "inexistent account: '%s'"),
										   contra_account_name);
				dialogue_warning_hint (tmp_str, _("Transfer transaction in error"));
				g_free (tmp_str);
				g_free (contra_account_name);
			}
			else
			{
				/* we have found the contra-account, we look for the contra-transaction */
				GSList *tmp_list_transactions_2;
				gint transaction_account;

				transaction_account = gsb_data_transaction_get_account_number (transaction_number_tmp);
				tmp_list_transactions_2 = gsb_data_transaction_get_transactions_list ();
				while (tmp_list_transactions_2)
				{
					gint contra_transaction_number_tmp;
					gint contra_transaction_account;

					contra_transaction_number_tmp = gsb_data_transaction_get_transaction_number (tmp_list_transactions_2->data);
					contra_transaction_account = gsb_data_transaction_get_account_number (contra_transaction_number_tmp);

					if (contra_account_number == contra_transaction_account
						&& gsb_import_check_transaction_link (transaction_number_tmp, contra_transaction_number_tmp))
					{
						/* we have found the contra transaction, set all the values */
						gint payment_number;

						gsb_data_transaction_set_contra_transaction_number (transaction_number_tmp,
																			contra_transaction_number_tmp);
						gsb_data_transaction_set_contra_transaction_number (contra_transaction_number_tmp,
																			transaction_number_tmp);

						/* unset the reference of the contra transaction */
						gsb_data_transaction_set_bank_references (contra_transaction_number_tmp, NULL);

						/* try to set the good method of payment to transfer */
						payment_number = gsb_data_payment_get_transfer_payment_number (transaction_account);
						if (payment_number)
							gsb_data_transaction_set_method_of_payment_number (transaction_number_tmp, payment_number);

						payment_number = gsb_data_payment_get_transfer_payment_number (contra_transaction_account);
						if (payment_number)
							gsb_data_transaction_set_method_of_payment_number (contra_transaction_number_tmp, payment_number);
					}
					tmp_list_transactions_2 = tmp_list_transactions_2->next;
				}

				/* if no contra-transaction, that transaction becomes normal */
				/* on retouve notre virement dans la deuxième liste */
				if (gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) == -1)
				{
					gint contra_transaction_number = 0;
					gint mother_number;
					gint payment_number;

					/* on crée une transaction dans le compte destinataire */
					contra_transaction_number = gsb_data_transaction_new_transaction (contra_account_number);
					gsb_data_transaction_copy_transaction (transaction_number_tmp, contra_transaction_number, TRUE);

					/* fixe les données si le virement est une opération fille */
					mother_number = gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp);
					if (mother_number)
					{
						gint payee_number;

						gsb_data_transaction_set_mother_transaction_number (contra_transaction_number, 0);
						payee_number = gsb_data_transaction_get_payee_number (mother_number);
						gsb_data_transaction_set_payee_number (contra_transaction_number, payee_number);

					}

					/* we have to change the amount by the opposite */
					gsb_data_transaction_set_amount (contra_transaction_number,
													 gsb_real_opposite (gsb_data_transaction_get_amount
																		(transaction_number_tmp)));

					/* we have to check the change */
					gsb_currency_check_for_change (contra_transaction_number);

					/* set default payment number of contra */
					payment_number = gsb_data_payment_get_transfer_payment_number (contra_account_number);
					if (payment_number)
						gsb_data_transaction_set_method_of_payment_number (contra_transaction_number,
																		   payment_number);

					/* set the link between the transactions */
					gsb_data_transaction_set_contra_transaction_number (transaction_number_tmp,
																		contra_transaction_number);
					gsb_data_transaction_set_contra_transaction_number (contra_transaction_number,
																		transaction_number_tmp);

					gsb_transactions_list_append_new_transaction (contra_transaction_number, TRUE);
				}
				g_free (contra_account_name);
			}
		}

		tmp_list_transactions = tmp_list_transactions->next;
    }

	/* the transactions were already set in the list,
     * and the transfer was not written, we need to update the categories values
     * in the lists */
    transaction_list_update_element (ELEMENT_CATEGORY);
    transaction_list_update_element (ELEMENT_PAYMENT_TYPE);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_import_click_dialog_ope_orphelines (GtkWidget *dialog,
														gint result,
														GtkWidget *liste_ope_celibataires)
{
    GSList *list_ope_import_celibataires;
    GSList *tmp_list;
    GtkTreeIter iter;
    GtkTreeModel *model;
	GrisbiWinRun *w_run;

    switch (result)
    {
        case GTK_RESPONSE_ACCEPT:
			/* on coche toutes les cases des opérations */
			model = gtk_tree_view_get_model (GTK_TREE_VIEW (liste_ope_celibataires));
			if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
			{
				do
				{
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, TRUE, -1);
				}
				while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
			}
			gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),GTK_RESPONSE_ACCEPT, FALSE);
			break;

		case GTK_RESPONSE_OK:
			/* on ajoute la ou les opés marquées à la liste d'opés en les pointant d'un T
			   puis on les retire de la liste des orphelines
			   s'il ne reste plus d'opés orphelines, on ferme la boite de dialogue */
			list_ope_import_celibataires = g_object_get_data (G_OBJECT (liste_ope_celibataires), "liste_ope");
			model = gtk_tree_view_get_model (GTK_TREE_VIEW (liste_ope_celibataires));
			gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter);

			/* normalement, pas besoin de mettre ça à 0 car normalement pas de ventilations à ce stade... */
			mother_transaction_number = 0;

			tmp_list = list_ope_import_celibataires;
			while (tmp_list)
			{
				gboolean enregistre;
				GSList *last_item;

					gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &enregistre, -1);

				if (enregistre)
				{
					/* à ce niveau, l'opé a été cochée donc on l'enregistre en la marquant T	 */

					struct ImportTransaction *ope_import;
					gint transaction_number;

					ope_import = tmp_list->data;

					transaction_number = gsb_import_create_transaction (ope_import, ope_import->no_compte, NULL);
					gsb_data_transaction_set_marked_transaction (transaction_number, OPERATION_TELEPOINTEE);

					/* we need to add the transaction now to the tree model and update the tree_view */
					gsb_transactions_list_append_new_transaction (transaction_number, FALSE);
					gsb_transactions_list_update_tree_view (ope_import->no_compte, FALSE);
					gsb_data_account_colorize_current_balance (ope_import->no_compte);

					/* on a enregistré l'opé, on la retire maintenant de la liste et de la sliste */
					last_item = tmp_list;
					tmp_list = tmp_list->next;
					list_ope_import_celibataires = g_slist_remove_link (list_ope_import_celibataires, last_item);

					/* on retire la ligne qu'on vient d'enregistrer, cela met l'iter directement sur la suite */
					gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
				}
				else
				{
					gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter);
					tmp_list = tmp_list->next;
				}
			}

			/* on enregistre la nouvelle liste d'opé pour la retrouver plus tard */
			g_object_set_data (G_OBJECT (liste_ope_celibataires), "liste_ope", list_ope_import_celibataires);

			/* il est possible que les opés importées soient des virements, il faut faire les relations ici */
			if (virements_a_chercher)
				gsb_import_cree_liens_virements_ope_import ();

			/* mise à jour de l'accueil */
			w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
			w_run->mise_a_jour_liste_comptes_accueil = TRUE;
			w_run->mise_a_jour_soldes_minimaux = TRUE;

            gsb_file_set_modified (TRUE);

		/* FALLTHRU */
		default:
			gtk_widget_destroy (dialog);
			break;
    }

    return (FALSE);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_import_click_sur_liste_opes_orphelines (GtkCellRendererToggle *renderer,
															gchar *ligne,
															GtkTreeModel *store)
{
    GtkTreeIter iter;
    gboolean valeur;

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, ligne))
    {
        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &valeur, -1);
        gtk_list_store_set (GTK_LIST_STORE (store), &iter, 0, 1 - valeur, -1);
    }
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
    {
        gboolean all_true = TRUE;
        GtkDialog *dialog;

        dialog = g_object_get_data (G_OBJECT (store), "dialog");
        do
        {
            gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &valeur, -1);
            if (valeur == FALSE)
                all_true = FALSE;
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));

        if (all_true == TRUE)
            gtk_dialog_set_response_sensitive (dialog,GTK_RESPONSE_ACCEPT, FALSE);
        else
            gtk_dialog_set_response_sensitive (dialog,GTK_RESPONSE_ACCEPT, TRUE);
    }
    return (FALSE);
}

/**
 * import the transactions in a new account.
 *
 * \param imported_account an imported structure account which contains the transactions
 * \param account_number the number of account where we want to put the new transations
 *
 * \return
 **/
static void gsb_import_create_imported_transactions (struct ImportAccount *imported_account,
													 gint account_number)
{
    GtkWidget *progress = NULL;
    GSList *tmp_list;
    gint nbre_transaction;
    gint devise;

    mother_transaction_number = 0;

    tmp_list = imported_account->operations_importees;
    nbre_transaction = g_slist_length (tmp_list);

    if (nbre_transaction > NBRE_TRANSACTION_FOR_PROGRESS_BAR)
        progress = gsb_import_progress_bar_affiche (imported_account);

    /* set the account currency to the transaction and create the transaction */
    if (imported_account->bouton_devise)
        devise = gsb_currency_get_currency_from_combobox (imported_account->bouton_devise);
    else
        devise = gsb_data_currency_get_number_by_code_iso4217 (imported_account->devise);

    while (tmp_list)
    {
        struct ImportTransaction *imported_transaction;
        gint transaction_number;

        if (nbre_transaction > NBRE_TRANSACTION_FOR_PROGRESS_BAR)
        {
            gsb_import_progress_bar_pulse (progress, nbre_transaction);
            /* update progress bar NOW */
            update_gui ();
            nbre_transaction --;
        }

        imported_transaction = tmp_list->data;

        imported_transaction->devise = devise;

        transaction_number = gsb_import_create_transaction (imported_transaction,
                        account_number, imported_account->origine);

        /* invert the amount of the transaction if asked */
        if (imported_account->invert_transaction_amount)
            gsb_data_transaction_set_amount (transaction_number,
                        gsb_real_opposite (gsb_data_transaction_get_amount (
                        transaction_number)));

        /* we need to add the transaction now to the tree model here
         * to avoid to write again all the account */
        gsb_transactions_list_append_new_transaction (transaction_number, FALSE);

        tmp_list = tmp_list->next;
    }

    if (progress)
        gtk_widget_destroy (progress);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_show_orphan_transactions (GSList *orphan_list,
                        gint account_number)
{
	GtkWidget *liste_ope_celibataires, *dialog, *label, *scrolled_window;
    GtkWidget *button_cancel;
    GtkWidget *button_OK;
	GtkWidget *button_select_all;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
    GSList *tmp_list;
    gchar *tmp_str;

    tmp_str = g_strdup_printf (_("Orphaned transactions for: %s"),
							   gsb_data_account_get_name (account_number));

    dialog = gtk_dialog_new_with_buttons (tmp_str,
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										  NULL, NULL,
										  NULL);

    g_free (tmp_str);

	button_select_all = gtk_button_new_with_label (_("Select all"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_select_all, GTK_RESPONSE_ACCEPT);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_CANCEL);

	button_OK = gtk_button_new_with_label (_("Validate"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_OK, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_OK, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    gtk_window_set_default_size (GTK_WINDOW (dialog), 770, 412);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER(dialog), BOX_BORDER_WIDTH);

	label = gtk_label_new (_("Mark transactions you want to add to the list and click the "
                              "OK button"));
    utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
	gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)),
                        label,
                        FALSE,
                        FALSE,
                        10);
	gtk_widget_show (label);

    /* on crée le model et on y associe le dialogue */
	store = gtk_list_store_new (4,
                        G_TYPE_BOOLEAN,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING);
    g_object_set_data (G_OBJECT (store), "dialog", dialog);

	/* on remplit la liste */
	tmp_list = orphan_list;

	while (tmp_list)
	{
	    struct ImportTransaction *ope_import;
	    GtkTreeIter iter;
		gchar *tmp_date;

	    ope_import = tmp_list->data;

	    gtk_list_store_append (store, &iter);
		tmp_date = gsb_format_gdate (ope_import->date);
	    gtk_list_store_set (store,
                        &iter,
                        0, FALSE,
                        1, tmp_date,
                        2, ope_import->tiers,
                        3, utils_real_get_string_with_currency (ope_import->montant,
                        ope_import->devise, TRUE),
                        -1);

	    tmp_list = tmp_list->next;
	}

	/* on crée la liste des opés célibataires et on y associe la gslist */
	liste_ope_celibataires = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_widget_set_name (liste_ope_celibataires, "tree_view");
	g_object_set_data (G_OBJECT (liste_ope_celibataires), "liste_ope", orphan_list);

	scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
	gtk_widget_set_size_request (scrolled_window, -1, 300);
	gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)),
                        scrolled_window,
                        TRUE,
                        TRUE,
                        0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                        GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled_window), liste_ope_celibataires);
	gtk_widget_show_all (scrolled_window);

	/* on affiche les colonnes */
	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (renderer,
                        "toggled",
                        G_CALLBACK (gsb_import_click_sur_liste_opes_orphelines),
                        store);
	column = gtk_tree_view_column_new_with_attributes (_("Mark"),
                        renderer,
                        "active", 0,
                        NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Date"),
                        renderer,
                        "text", 1,
                        NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Payee"),
                        renderer,
                        "text", 2,
                        NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Amount"),
                        renderer,
                        "text", 3,
                        NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	g_signal_connect (G_OBJECT (dialog),
                        "response",
                        G_CALLBACK (gsb_import_click_dialog_ope_orphelines),
                        liste_ope_celibataires);

	gtk_widget_show_all (dialog);
}

/**
 * Pointe les opérations télérelevées.
 * En cas d'opérations comparables multiples , on pointe la première opé comparable rencontrée
 * les suivantes ne seront pas concernées.
 * Seules les opérations non pointées seront modifiées ou fusionnées
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_import_pointe_opes_importees (struct ImportAccount *imported_account,
											  gint account_number)
{
    GSList *list_ope_import_celibataires;
    GSList *list_ope_retenues;
    GSList *tmp_list_ope_importees;
    GDate *first_date_import = NULL;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

    /* si le compte importé a une id, on la vérifie ici */
    /* si elle est absente, on met celle importée */
    /* si elle est différente, on demande si on la remplace */
    if (imported_account->id_compte)
    {
        if (!gsb_import_set_id_compte (account_number, imported_account->id_compte))
            return;
    }

    /* pour gagner en rapidité, on va récupérer la date de la première */
    /* opération qui est dans le fichier importé */
    first_date_import = gsb_import_get_first_date (imported_account->operations_importees);

    /* on récupère la liste des opérations dans l'intervalle de recherche pour l'import */
    list_ope_retenues = gsb_data_transaction_get_list_for_import (account_number, first_date_import);

    g_date_free (first_date_import);

	/* on initialise la liste des opé célibataires */
    list_ope_import_celibataires = NULL;

    /* on fait le tour des opés importées et recherche dans la liste d'opé s'il y a la correspondance */
    tmp_list_ope_importees = imported_account->operations_importees;
    while (tmp_list_ope_importees)
    {
        GSList *tmp_list_ope_retenues;
        struct ImportTransaction *ope_import;
		gboolean ope_import_trouve = FALSE;

        ope_import = tmp_list_ope_importees->data;

        /* set now the account number of the transaction */
        ope_import->no_compte = account_number;

        tmp_list_ope_retenues = list_ope_retenues;
        while (tmp_list_ope_retenues)
        {
            gint transaction_number;
            const gchar *tmp_str;
            GDate *date_debut_comparaison;
            GDate *date_fin_comparaison;
			const GDate *date_transaction;
            GDateDay day;
            GDateMonth month;
            GDateYear year;

            transaction_number = GPOINTER_TO_INT (tmp_list_ope_retenues->data);

			/* on met la devise de l'opération si plus tard on l'enregistre */
			if (imported_account->bouton_devise)
				ope_import->devise = gsb_currency_get_currency_from_combobox (imported_account->bouton_devise);
			else
				ope_import->devise = gsb_data_currency_get_number_by_code_iso4217 (imported_account->devise);

			/* si l'opé d'import a une id, on recherche dans la liste d'opé pour trouver
             * une id comparable */
            if (ope_import->id_operation)
            {
                tmp_str = gsb_data_transaction_get_id (transaction_number);
                if (tmp_str && strcmp (ope_import->id_operation, tmp_str) == 0)
                {
					/* on a trouvé une opé comparable on initialise ope_import->ope_correspondante */
					ope_import->ope_correspondante = transaction_number;
					ope_import_trouve = TRUE;

					/* on peut supprimer l'opé de la liste des ope retenues car c'est un cas unique */
					list_ope_retenues = g_slist_remove (list_ope_retenues, tmp_list_ope_retenues->data);

                    break;
                }
            }

            /* si on n'a rien trouvé par id, */
            /* on fait le tour de la liste d'opés pour trouver des opés comparable */
            /* cad même date avec + ou - une échelle et même montant et pas une opé de ventilation */
            day = g_date_get_day (ope_import->date);
            month = g_date_get_month (ope_import->date);
            year = g_date_get_year (ope_import->date);

            date_debut_comparaison = g_date_new_dmy (day, month, year);
            g_date_subtract_days (date_debut_comparaison, w_etat->import_files_nb_days);

            date_fin_comparaison = g_date_new_dmy (day, month, year);
            g_date_add_days (date_fin_comparaison, w_etat->import_files_nb_days);

			/* si on doit inverser le montant on le fait maintenant */
            if (imported_account->invert_transaction_amount)
                ope_import->montant =  gsb_real_opposite (ope_import->montant);

			/* on recherche des operations ayant le meme montant et dont */
			/* la date est dans l'interval de recherche */
			date_transaction = gsb_data_transaction_get_date (transaction_number);
            if (!gsb_real_cmp (gsb_data_transaction_get_amount (transaction_number), ope_import->montant)
				&& (g_date_compare (date_transaction, date_debut_comparaison) >= 0)
				&& (g_date_compare (date_transaction, date_fin_comparaison) <= 0)
				&& !gsb_data_transaction_get_mother_transaction_number (transaction_number))
            {
                /* on a touvé une opé comparable on initialise ope_import->ope_correspondante */
				ope_import->ope_correspondante = transaction_number;
				ope_import_trouve = TRUE;

				/* on récupère la date de valeur si besoin */
				if (ope_import->date_de_valeur == NULL
					&& w_etat->csv_force_date_valeur_with_date
					&& imported_account->origine
					&& g_ascii_strcasecmp (imported_account->origine, "CSV") == 0)
				{
					ope_import->date_de_valeur = gsb_date_copy (ope_import->date);
				}

				/* on supprime l'opé de la liste des ope retenues car c'est le premier cas rencontré */
				list_ope_retenues = g_slist_remove (list_ope_retenues, tmp_list_ope_retenues->data);

                break;
            }
            tmp_list_ope_retenues = tmp_list_ope_retenues->next;
        }

        /* à ce stade, si il existe une opé comparable ope_import_trouve = TRUE */
		if (ope_import_trouve == TRUE)
			{
				gint transaction_number;

				transaction_number = ope_import->ope_correspondante;

				/* si elle est déjà pointée ou télé relevée, on ne fait rien */
				if (!gsb_data_transaction_get_marked_transaction (transaction_number))
				{
					/* si l'opé d'import a une id et pas l'opé, on marque l'id dans l'opé */
					if (strlen (gsb_data_transaction_get_transaction_id (transaction_number)) == 0
						&& ope_import->id_operation)
						gsb_data_transaction_set_transaction_id (transaction_number, ope_import->id_operation);

					gsb_data_transaction_set_marked_transaction (transaction_number, OPERATION_TELEPOINTEE);

					/* on traite la fusion des opérations si besoin */
					if (w_etat->fusion_import_transactions && ope_import->ope_correspondante > 0)
					{
						transaction_number = gsb_import_create_transaction (ope_import,
																			account_number,
																			imported_account->origine);
						gsb_transactions_list_update_transaction (transaction_number);
					}
				}
			}
			else
			{
				/* récupération de la date de valeur si besoin */
				if (ope_import->date_de_valeur == NULL
					&& w_etat->csv_force_date_valeur_with_date
					&& imported_account->origine
					&& g_ascii_strcasecmp (imported_account->origine, "CSV") == 0)
				{
					ope_import->date_de_valeur = gsb_date_copy (ope_import->date);
				}

			/* on ajoute l'ope dans la liste des operations celibataires */
			list_ope_import_celibataires = g_slist_append (list_ope_import_celibataires, ope_import);
		}
        tmp_list_ope_importees = tmp_list_ope_importees->next;
    }

	if (list_ope_retenues)
		g_slist_free (list_ope_retenues);

    /* a ce niveau, list_ope_import_celibataires contient les opés d'import dont on */
    /* n'a pas retrouvé l'opé correspondante */
    /* on les affiche dans une liste en proposant de les ajouter à la liste  des transactions */
    if (list_ope_import_celibataires)
        gsb_import_show_orphan_transactions (list_ope_import_celibataires, account_number);
}

/**
 * finish the import
 * go throw all the imported accounts and does what the user asked with
 * each of them
 *
 * \param
 *
 * \return
 * */
static void traitement_operations_importees (GtkWindow *parent)
{
    GSList *tmp_list;
    gint new_file;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;

    devel_debug (NULL);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_etat = grisbi_win_get_w_etat ();

	/* when come here, all the currencies are already created
     * and init_variables is already called
     * (see affichage_recapitulatif_importation) */

    /* if new file, init grisbi */
    if (gsb_data_account_get_number_of_accounts ())
    	new_file = 0;
    else
    {
		/* Create initial lists. */
		new_file = 1;
    }

    /* for now, no marked transactions imported */
    marked_r_transactions_imported = FALSE;

    /* go throw the accounts and do what is asked */
    tmp_list = liste_comptes_importes;

    while (tmp_list)
    {
		struct ImportAccount *compte;
		gint account_number = 0;

		compte = tmp_list->data;

		switch (compte->action)
		{
			case IMPORT_CREATE_ACCOUNT:
			account_number = gsb_import_create_imported_account (compte);

			if (account_number == -1)
			{
				gchar *tmp_str = g_strdup_printf (_("An error occurred while creating the new "
													"account %s,\nWe try to continue to import "
													"but bad things can happen..."),
													compte->nom_de_compte);
				dialogue_error (tmp_str);
				g_free (tmp_str);
				continue;
			}

			/* the next functions will add the transaction to the tree view list
			 * so if we create a new file, we need to finish the gui here to append
			 * the transactions */
			if (new_file)
			{
				/* this should be the same as the end of gsb_file_new */
				/* init the gui */
				grisbi_win_new_file_gui ();

				new_file = 0;
			}
			else
				gsb_gui_navigation_add_account (account_number, FALSE);

			gsb_import_create_imported_transactions (compte, account_number);
			break;

			case IMPORT_ADD_TRANSACTIONS:
			account_number = gsb_account_get_combo_account_number (compte->bouton_compte_add);
			gsb_import_add_imported_transactions (compte,account_number, parent);

			break;

			case IMPORT_MARK_TRANSACTIONS:
			account_number = gsb_account_get_combo_account_number (compte->bouton_compte_mark);
			gsb_import_pointe_opes_importees (compte, account_number);
			transaction_list_update_element (ELEMENT_MARK);
			break;
		}

		/* MAJ du solde du compte nécessaire suivant date des opérations existantes */
		if (a_conf->balances_with_scheduled == FALSE)
			gsb_data_account_set_balances_are_dirty (account_number);

		/* MAJ des données du module bet */
		gsb_data_account_set_bet_maj (account_number, BET_MAJ_ALL);

		/* first, we create the rule if asked */
		if (compte->create_rule
			&& (compte->action != IMPORT_CREATE_ACCOUNT || !strcmp (compte->origine, "CSV")))
		{
			/* ok, we create the rule */
			gchar *name;
			gint rule_number = 0;
			gint csv_spec_nbre_lines = 0;

			name = (gchar *) gtk_entry_get_text (GTK_ENTRY (compte->entry_name_rule));
			if (!strlen (name))
			{
				/* the user didn't enter a name, propose now */
				gchar *tmp_str;

				tmp_str = g_strdup_printf (_("You want to create an import rule for the account %s "
											 "but didn't give a name to that rule. Please set a "
											 "name or let it empty to cancel the rule creation."),
											gsb_data_account_get_name (account_number));
				name = dialogue_hint_with_entry (tmp_str, _("No name for the import rule"),
														 _("Name of the rule: "));
				g_free (tmp_str);

				if (!strlen (name))
					break;
			}
			else
				name = g_strdup (name);

			rule_number = gsb_data_import_rule_new (name);
			g_free (name);
			gsb_data_import_rule_set_account (rule_number, account_number);
			gsb_data_import_rule_set_currency (rule_number, gsb_currency_get_currency_from_combobox (compte->bouton_devise));
			gsb_data_import_rule_set_invert (rule_number, compte->invert_transaction_amount);
			gsb_data_import_rule_set_charmap (rule_number, charmap_imported);
			gsb_data_import_rule_set_last_file_name (rule_number, compte->real_filename);
			gsb_data_import_rule_set_action (rule_number, compte->action);
			gsb_data_import_rule_set_type (rule_number, compte->origine);
			if (!strcmp (compte->origine, "CSV"))
			{
				gsb_data_import_rule_set_csv_account_id_col (rule_number, compte->csv_account_id_col);
				gsb_data_import_rule_set_csv_account_id_row (rule_number, compte->csv_account_id_row);
				gsb_data_import_rule_set_csv_fields_str (rule_number, compte->csv_fields_str);
				gsb_data_import_rule_set_csv_first_line_data (rule_number, compte->csv_first_line_data);
				gsb_data_import_rule_set_csv_headers_present (rule_number, compte->csv_headers_present);
				gsb_data_import_rule_set_csv_separator (rule_number, w_etat->csv_separator);
				gsb_data_import_rule_set_csv_spec_lines_list (rule_number, compte->csv_spec_lines_list);
				csv_spec_nbre_lines = g_slist_length (compte->csv_spec_lines_list);
				if (csv_spec_nbre_lines)
					gsb_data_import_rule_set_csv_spec_nbre_lines (rule_number, csv_spec_nbre_lines);
				gsb_data_import_rule_set_csv_spec_cols_name (rule_number, compte->csv_spec_cols_name);
			}
		}
		tmp_list = tmp_list->next;
	}

    /* if no account created, there is a problem
     * show an error and go away */
    if (!gsb_data_account_get_number_of_accounts ())
    {
		dialogue_error (_("No account in memory now, this is bad...\nBetter to leave "
						  "the import before a crash.\n\nPlease contact the Grisbi team "
						  "to find the problem."));
		return;
    }

    /* create the links between transactions (transfer) */
    if (virements_a_chercher)
	    gsb_import_cree_liens_virements_ope_import ();

    grisbi_win_status_bar_message (_("Please wait"));

    /* update the name of accounts in scheduler form */
    gsb_account_update_combo_list (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
								   FALSE);

    /* set the rule button if necessary */
	gsb_transactions_list_show_menu_import_rule (gsb_gui_navigation_get_current_account ());

    /* show the account list */
	grisbi_win_menu_move_to_acc_delete ();
	grisbi_win_menu_move_to_acc_new ();

    /* update main page */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;
    w_run->mise_a_jour_soldes_minimaux = TRUE;
    mise_a_jour_accueil (FALSE);

    grisbi_win_status_bar_clear();

    /* if some R marked transactions are imported, show a message */
    if (marked_r_transactions_imported)
		dialogue (_("You have just imported reconciled transactions but they not associated "
					 "with any reconcile number yet.  You may associate them with a reconcilation "
					 "later via the preferences windows."));

	/* force le tri des opérations */
	gsb_transactions_list_update_tree_view (gsb_gui_navigation_get_current_account (), TRUE);

    gsb_file_set_modified (TRUE);
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
void gsb_import_assistant_importer_fichier (void)
{
    GtkWidget *assistant;
	gchar *tmp_str;
    gchar *format_str;
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
	/* if nothing opened, we need to create a new file to set up all the variables */
    if (!gsb_data_currency_get_currency_list ())
    {
        init_variables ();
        gsb_assistant_file_run (FALSE, TRUE);
        return;
    }

    liste_comptes_importes = NULL;
    liste_comptes_importes_error = NULL;
    virements_a_chercher = 0;

    format_str = gsb_ImportFormats_get_list_formats_to_string ();
    tmp_str = g_strconcat (_("This assistant will help you import one or several "
                    "files into Grisbi."
                    "\n\n"
                    "Grisbi will try to do its best to guess which format are imported, "
                    "but you may have to manually set them in the list of next page.  "
                    "So far, the following formats are supported:"
                    "\n\n"),
                    format_str, NULL);
    assistant = gsb_assistant_new (_("Importing transactions into Grisbi"),
                        tmp_str,
                        "gsb-import-32.png",
                        NULL);
    g_free (format_str);
    g_free (tmp_str);

	/* Set the charmap_imported variable to the local character set. */
	if (charmap_imported && strlen (charmap_imported) > 0)
		g_free (charmap_imported);
	charmap_imported = g_get_codeset ();

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (a_conf->force_import_directory)
	{
		gsb_assistant_add_page (assistant,
							gsb_import_create_file_selection_page (assistant),
							IMPORT_FILESEL_PAGE,
							IMPORT_STARTUP_PAGE,
							IMPORT_RESUME_PAGE,
							G_CALLBACK (gsb_import_enter_force_dir_page));
	}
	else
	{
		gsb_assistant_add_page (assistant,
							gsb_import_create_file_selection_page (assistant),
							IMPORT_FILESEL_PAGE,
							IMPORT_STARTUP_PAGE,
							IMPORT_RESUME_PAGE,
							G_CALLBACK (gsb_import_enter_file_selection_page));
	}
    gsb_assistant_add_page (assistant,
							import_create_csv_preview_page (assistant),
							IMPORT_CSV_PAGE,
							IMPORT_FILESEL_PAGE,
							IMPORT_RESUME_PAGE,
							G_CALLBACK (import_enter_csv_preview_page));
    gsb_assistant_add_page (assistant,
							gsb_import_create_resume_page (assistant),
							IMPORT_RESUME_PAGE,
							IMPORT_FILESEL_PAGE,
							IMPORT_FIRST_ACCOUNT_PAGE,
							G_CALLBACK (gsb_import_enter_resume_page));

    if (gsb_assistant_run (assistant) == GTK_RESPONSE_APPLY)
    {

		grisbi_win_status_bar_wait (TRUE);
        traitement_operations_importees (GTK_WINDOW (assistant));
        gtk_widget_destroy (assistant);
        grisbi_win_status_bar_stop_wait (TRUE);
    }
    else
    {
        gtk_widget_destroy (assistant);
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *gsb_ImportFormats_get_list_formats_to_string (void)
{
    GSList *tmp_list = ImportFormats;
    gchar *format_str = NULL;

    while (tmp_list)
    {
        gchar *tmp_str;
        struct ImportFormat *format;


        format = (struct ImportFormat *) tmp_list->data;
        tmp_str = g_strdup_printf ("	• %s (%s)\n", _(format->complete_name), format->name);

        if (format_str == NULL)
            format_str = tmp_str;
        else
        {
            gchar *old_str = format_str;

            format_str = g_strconcat (format_str, tmp_str,  NULL);
            g_free (tmp_str);
            g_free (old_str);
        }

        tmp_list = tmp_list->next;
    }

    return format_str;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GSList *gsb_import_import_selected_files (GtkWidget *assistant)
{
    GSList *list = NULL;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;

    model = g_object_get_data (G_OBJECT (assistant), "model");
    g_return_val_if_fail (model, NULL);

    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid)
    {
        struct ImportFile *imported;
        gboolean selected;

        imported = g_malloc0 (sizeof (struct ImportFile));
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        IMPORT_FILESEL_SELECTED, &selected,
                        IMPORT_FILESEL_REALNAME, &(imported->name),
                        IMPORT_FILESEL_TYPE, &(imported->type),
                        IMPORT_FILESEL_CODING, &(imported->coding_system),
                        -1);

        if (selected)
        {
            list = g_slist_append (list, imported);
        }
        valid = gtk_tree_model_iter_next (model, &iter);
    }

    return list;
}

/**
 * Add an imported account to the list of imported accounts.
 *
 * \param account   Account to register.
 *
 * \return
 **/
void gsb_import_register_account (struct ImportAccount *account)
{
    liste_comptes_importes = g_slist_append (liste_comptes_importes, account);
}

/**
 * Add an imported account to the list of imported accounts in error.
 *
 * \param account   Account to register.
 *
 * \return
 **/
void gsb_import_register_account_error (struct ImportAccount *account)
{
    liste_comptes_importes_error = g_slist_append (liste_comptes_importes, account);
}

/**
 * Register built-in import formats as known.
 *
 * \param
 *
 * \return
 **/
void gsb_import_register_import_formats (void)
{
    gint i;

    for (i = 0; builtin_formats [i] . name != NULL ; i ++)
    {
        gsb_import_register_ImportFormat (&builtin_formats [i]);
    }
}

/**
 * check if the name of the imported account in param is not already
 * used, if yes, modify it
 *
 * \param account_name	name to check
 *
 * return a newly allocated string or NULL
 **/
gchar *gsb_import_unique_imported_name (const gchar *account_name)
{
    GSList *tmp_list;
    gchar *basename;
    gint iter = 1;

    tmp_list = liste_comptes_importes;
    basename = my_strdup (account_name);
    if (!liste_comptes_importes)
        return basename;

    do
    {
        struct ImportAccount *tmp_account;

        tmp_account = (struct ImportAccount *) tmp_list->data;

        if (tmp_account->nom_de_compte == NULL)
            tmp_account->nom_de_compte = g_strdup (basename);

        if (!strcmp (basename, tmp_account->nom_de_compte))
        {
            tmp_list = liste_comptes_importes;

            g_free (basename);
            basename = g_strdup_printf (_("%s #%d"), account_name, ++iter);
        }
        else
            tmp_list = tmp_list->next;
    }
    while (tmp_list);

    return basename;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_import_free_transaction (struct ImportTransaction *transaction)
{
	if (transaction->id_operation)
		g_free (transaction->id_operation);

	if (transaction->date)
		g_date_free (transaction->date);
	if (transaction->date_de_valeur)
		g_date_free (transaction->date_de_valeur);
	if (transaction->date_tmp)
		g_free (transaction->date_tmp);

	if (transaction->tiers)
		g_free (transaction->tiers);
	if (transaction->notes)
		g_free (transaction->notes);
	if (transaction->cheque)
		g_free (transaction->cheque);

	if (transaction->categ)
		g_free (transaction->categ);
	if (transaction->budget)
		g_free (transaction->budget);

	if (transaction->dest_account_name)
		g_free (transaction->dest_account_name);

	if (transaction->guid)
		g_free (transaction->guid);

	g_free (transaction);
}

/******************************************************************************/
/* Fonctions de gestion des règles d'importation directe                      */
/******************************************************************************/
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * callback when user click on the button to open a file by file chooser
 *
 * \param button	the button to open the file chooser
 * \param entry		the entry to fill with the new filename
 *
 * \return FALSE
 **/
static gboolean gsb_import_by_rule_get_file (GtkWidget *button,
											 GtkWidget *entry)
{
	GtkWidget *dialog;
    GSList *filenames;
    GSList *tmp_list;
    gchar *string = NULL;

	filenames = gsb_import_create_file_chooser (charmap_imported, GTK_WIDGET (grisbi_app_get_active_window (NULL)));
    if (!filenames)
		return FALSE;

    /* separate all the files by ; */
    tmp_list = filenames;
    while (tmp_list)
    {
    if (string)
    {
        gchar *last_string = string;

        string = g_strconcat (string, ";", tmp_list->data, NULL);
        g_free (last_string);
    }
    else
        string = my_strdup (tmp_list->data);
    tmp_list = tmp_list->next;
    }
    g_slist_free (filenames);
    gtk_entry_set_text (GTK_ENTRY (entry), string);
    gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
    g_free (string);

	/* active le bouton Valider */
	dialog = g_object_get_data (G_OBJECT(button), "dialog");
	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT, TRUE);

    return FALSE;
}

/**
 * show a popup asking the name of the file to import with a rule
 *
 * \param rule
 *
 * \return a newly-allocated NULL-terminated array of strings of filenames, or NULL
 * 		use g_strfreev to free it
 **/
static gchar **gsb_import_by_rule_ask_filename (gint rule,
												GrisbiAppConf *a_conf)
{
    GtkWidget *dialog, *paddingbox, *table;
	GtkWidget *button_cancel;
	GtkWidget *button_OK;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *check_button;
    GtkWidget *entry;
    GtkWidget *hbox;
    gchar **array = NULL;
	const gchar *filename = NULL;
    gchar *tmp_str;
    gchar *tmp_str2;
	gchar *tmp_str3;

    if (!rule)
    return NULL;

    dialog = gtk_dialog_new_with_buttons (_("Import a file with a rule"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										  NULL, NULL,
										  NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_REJECT);

	button_OK = gtk_button_new_with_label (_("Validate"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_OK, GTK_RESPONSE_ACCEPT);

    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

    /* text for paddingbox */
    tmp_str = g_strdup_printf (_("Properties of the rule: %s\n"),
                  gsb_data_import_rule_get_name (rule));

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), hbox, FALSE, FALSE, 0);
    paddingbox = new_paddingbox_with_title (hbox, TRUE, tmp_str);
    gtk_container_set_border_width (GTK_CONTAINER(hbox), BOX_BORDER_WIDTH);
    gtk_container_set_border_width (GTK_CONTAINER(paddingbox), BOX_BORDER_WIDTH);
    g_free (tmp_str);

    /* table for layout */
    table = gtk_grid_new ();
    gtk_box_pack_start (GTK_BOX (paddingbox), table, FALSE, FALSE, 6);
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);

    /* textstring 1 */
    if (gsb_data_import_rule_get_action (rule) == IMPORT_ADD_TRANSACTIONS)
    tmp_str = g_strdup_printf (_("Imported transactions will be added to the account %s.\n"),
                        gsb_data_account_get_name (gsb_data_import_rule_get_account (rule)));
    else
    tmp_str = g_strdup_printf (_("Imported transactions will mark transactions in the account "
                                "%s.\n"),
                        gsb_data_account_get_name (gsb_data_import_rule_get_account (rule)));

    /* textstring 2 */
	tmp_str3 = g_strdup_printf (_("Currency to import is %s.\n"),
								gsb_data_currency_get_name (gsb_data_import_rule_get_currency (rule)));
    tmp_str2 = g_strconcat(tmp_str, tmp_str3, NULL);
    g_free (tmp_str);
	g_free (tmp_str3);
	tmp_str = tmp_str2;

    /* textstring 3 */
    if (gsb_data_import_rule_get_invert (rule))
    {
		tmp_str2 = g_strdup_printf (_("Amounts of the transactions will be "
									  "inverted.\n"));
        tmp_str3 = g_strconcat(tmp_str, tmp_str2, NULL);
        g_free (tmp_str);
		g_free(tmp_str2);
	    tmp_str = tmp_str3;
    }

    label = gtk_label_new (tmp_str);
    utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 3, 1);
    g_free (tmp_str);

    /* label filename */
    label = gtk_label_new (_("Name of the file to import: "));
    utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

    /* i tried to use gtk_file_chooser_button, but the name of the file is showed only sometimes
     * so go back to the old method with a gtkentry */
    entry = gtk_entry_new ();
    gtk_widget_set_size_request (entry, 200, -1);
    gtk_grid_attach (GTK_GRID (table), entry, 1, 1, 1, 1);

	filename = gsb_data_import_rule_get_last_file_name (rule);
    if (filename && g_file_test (filename, G_FILE_TEST_EXISTS))
    {
		gtk_entry_set_text (GTK_ENTRY (entry), filename);
    }
	else
	{
		gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Select a new file"));
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT, FALSE);

	}

    button = gtk_button_new_with_label ("...");
	g_object_set_data (G_OBJECT(button), "dialog", dialog);
    g_signal_connect (G_OBJECT (button), "clicked",
                        G_CALLBACK (gsb_import_by_rule_get_file), entry);
    gtk_grid_attach (GTK_GRID (table), button, 2, 1, 1, 1);

	/* Add check_button to remove the file */
	check_button = gtk_check_button_new_with_label (_("Remove the imported file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), a_conf->import_remove_file);
	gtk_grid_attach (GTK_GRID (table), check_button, 0, 2, 3, 1);

    g_object_set_data (G_OBJECT(entry), "rule", GINT_TO_POINTER (rule));

    gtk_widget_show_all (dialog);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		array = g_strsplit (gtk_entry_get_text (GTK_ENTRY (entry)), ";", 0);
		a_conf->import_remove_file = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button));
	}

    gtk_widget_destroy (dialog);
    return array;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * import a file with a rule
 *
 * \param rule	the number of rule to use
 *
 * \return TRUE : ok, FALSE : nothing done
 **/
void gsb_import_by_rule (gint rule)
{
    gint account_number;
    gchar **array;
    gint i=0;
	GrisbiAppConf *a_conf;
	GrisbiWinRun *w_run;

    devel_debug (NULL);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
    charmap_imported = my_strdup (gsb_data_import_rule_get_charmap (rule));
    array = gsb_import_by_rule_ask_filename (rule, a_conf);
    if (!array)
		return;

	account_number = gsb_data_import_rule_get_account (rule);
    while (array[i])
    {
        gchar *filename = array[i];
        const gchar *type;
        gchar *nom_fichier;
        struct ImportFile imported;
        GSList *tmp_list = ImportFormats;

        /* check if we are on ofx or qif or CSV file */
        type = gsb_import_autodetect_file_type (filename, NULL);
        if (strcmp (type, "OFX") && strcmp (type, "QIF") && strcmp (type, "CSV"))
        {
            gchar *tmp_str = g_path_get_basename (filename);
            gchar *tmp_str2 = g_strdup_printf (_("%s is neither an OFX file, neither a QIF file. "
                            "Nothing will be done for that file."),
                            tmp_str);
            dialogue_error (tmp_str2);
            g_free (tmp_str);
            g_free (tmp_str2);
            i++;
            continue;
        }
        else
            nom_fichier = my_strdup (filename);

        /* get the transactions */
        imported.name = nom_fichier;
        imported.coding_system =  charmap_imported;
        imported.type = type;

        liste_comptes_importes_error = NULL;
        liste_comptes_importes = NULL;

		if (!strcmp (type, "CSV"))
		{
			csv_import_file_by_rule (rule, &imported);
		}
		else
		{
			while (tmp_list)
			{
				struct ImportFormat *format = (struct ImportFormat *) tmp_list->data;

				if (!strcmp (imported.type, format->name))
				{
					format->import (NULL, &imported);
					tmp_list = tmp_list->next;
						continue;
				}
				tmp_list = tmp_list->next;
			}
		}

        /* now liste_comptes_importes contains the account structure of imported transactions */
        if (liste_comptes_importes_error)
        {
            gchar *tmp_str = g_path_get_basename (filename);
            gchar *tmp_str2 = g_strdup_printf (_("%s was not imported successfully. An error occurred while getting the transactions."),
                            tmp_str);
            dialogue_error (tmp_str2);
            g_free (tmp_str);
            g_free (tmp_str2);
            i++;
            continue;
        }

        while (liste_comptes_importes)
        {
            struct ImportAccount *account;

            account = liste_comptes_importes->data;
            account->invert_transaction_amount = gsb_data_import_rule_get_invert (rule);

            /* on fixe la devise pour les fichiers QIF et CSV */
            if (strcmp (type, "QIF") == 0 || strcmp (type, "CSV") == 0)
            {
                account->devise = g_strdup (gsb_data_currency_get_code_iso4217 (
                        gsb_data_import_rule_get_currency (rule)));
            }

            switch (gsb_data_import_rule_get_action (rule))
            {
                case IMPORT_ADD_TRANSACTIONS:
                gsb_import_add_imported_transactions (account, account_number, NULL);

                break;

                case IMPORT_MARK_TRANSACTIONS:
                gsb_import_pointe_opes_importees (account, account_number);
                transaction_list_update_element (ELEMENT_MARK);
                break;
            }
            g_slist_free (account->operations_importees);
            g_free (account);

            liste_comptes_importes = liste_comptes_importes->next;
        }

        /* save the charmap for the last file used */
        gsb_data_import_rule_set_charmap (rule, charmap_imported);

        /* save the last file used */
        gsb_data_import_rule_set_last_file_name (rule, filename);
		if (a_conf->import_remove_file)
		{
			g_remove (filename);
		}

        g_slist_free (liste_comptes_importes);
        g_free (nom_fichier);
        i++;
    }
    g_strfreev (array);

    /* update main page */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;
    w_run->mise_a_jour_soldes_minimaux = TRUE;
    mise_a_jour_accueil (FALSE);

    /* MAJ du solde du compte nécessaire suivant date des opérations existantes */
    if (a_conf->balances_with_scheduled == FALSE)
        gsb_data_account_set_balances_are_dirty (account_number);

    /* force the update module budget */
    gsb_data_account_set_bet_maj (account_number, BET_MAJ_ALL);

    gsb_file_set_modified (TRUE);
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
