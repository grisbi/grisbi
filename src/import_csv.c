/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2003 Cédric Auger  (cedric@grisbi.org)           */
/*          2004-2006 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2018 Pierre Biava (grisbi@pierre.biava.name)     */
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

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "import_csv.h"
#include "csv_parse.h"
#include "csv_template_rule.h"
#include "dialog.h"
#include "gsb_automem.h"
#include "gsb_data_import_rule.h"
#include "import.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern GSList *liste_comptes_importes_error;
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/

/** Array of pointers to fields.  */
static gint *csv_fields_config = NULL;

/* Mémorise la première ligne avec max columns */
static gint first_line_with_cols;

/* data for rule */
static gboolean csv_create_rule = FALSE;

/** Contain configuration of CSV fields.  */
/* NAME, ALIGNMENT, VALIDATE FUNCTION, PARSE FUNCTION, ALIAS */
struct csv_field csv_fields[18] = {
	{ N_("Unknown field"),  	0.0, NULL,			     		 NULL		     ,				"" },				/* 0 */
	{ N_("Currency"),	    	0.0, csv_import_validate_string, csv_import_parse_currency, 	"" },				/* 1 */
	{ N_("Date"),	    		0.0, csv_import_validate_date,   csv_import_parse_date, 		"" },				/* 2 */
	{ N_("Value date"),	    	0.0, csv_import_validate_date,   csv_import_parse_value_date, 	"" },				/* 3 */
	{ N_("Payee"),	    		0.0, csv_import_validate_string, csv_import_parse_payee, 		N_("Wording") },	/* 4 */
	{ N_("Notes"),	    		0.0, csv_import_validate_string, csv_import_parse_notes, 		"" },				/* 5 */
	{ N_("Cheque number"), 		0.0, csv_import_validate_number, csv_import_parse_cheque, 		"" },				/* 6 */
	{ N_("Category"),	    	0.0, csv_import_validate_string, csv_import_parse_category, 	"" },				/* 7 */
	{ N_("Sub-categories"),   	0.0, csv_import_validate_string, csv_import_parse_sub_category, "" },				/* 8 */
	{ N_("Budgetary lines"), 	0.0, csv_import_validate_string, csv_import_parse_budget, 		"" },				/* 9 */
	{ N_("Sub-budgetary lines"),0.0, csv_import_validate_string, csv_import_parse_sub_budget,	"" },				/* 10 */
	{ N_("Amount"),        		0.0, csv_import_validate_amount, csv_import_parse_balance, 		"" },				/* 11 */
	{ N_("Credit"), 			0.0, csv_import_validate_amount, csv_import_parse_credit, 		"" },				/* 12 */
	{ N_("Debit (absolute)"),	0.0, csv_import_validate_amount, csv_import_parse_debit, 		N_("Debit") },		/* 13 */
	{ N_("Debit (negative)"),	0.0, csv_import_validate_amount, csv_import_parse_credit, 		"" },				/* 14 */
	{ N_("C/R"),	    		0.0, csv_import_validate_string, csv_import_parse_p_r, 			"" },				/* 15 */
	{ N_("Split"),	    		0.0, csv_import_validate_string, csv_import_parse_split, 		"" },				/* 16 */
	{ NULL, 0.0, NULL, NULL, NULL },
};


/** Contain pre-defined CSV separators */
struct csv_separators {
    gchar *name;		/** Visible name of CSV separator */
    gchar *value; 		/** Real value */
} csv_separators[] =		/* Contains all pre-defined CSV separators. */
{
    { N_("Comma"),		"," },
    { N_("Semi-colon"),	";" },
    { N_("Colon"),		":" },
    { N_("Tabulation"),	"\t" },
    { N_("Other"),		NULL },
};

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
static void csv_import_free_line (GSList *list,
								  gint nbre_cols)
{
	gint i;

	for (i = 0; i < nbre_cols ; i++)
	{
		if (strlen (list->data))
			g_free (list->data);

		list = list->next;
	}
	g_slist_free (list);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void csv_import_free_lines_tab (GArray *lines_tab)
{
    GSList *list;
	gint index = 0;
	gint i;

	list = g_array_index (lines_tab, GSList *, index);
    do
    {
        for (i = 0; csv_fields_config[i] != -1 && list ; i++)
        {
			if (strlen (list->data))
				g_free (list->data);

			list = list->next;
        }
		g_slist_free (list);

		index++;
        list = g_array_index (lines_tab, GSList *, index);
    }
    while (list);

	g_array_unref (lines_tab);
}

/**
 * teste la validité d'un fichier
 *
 * \param const gchar 	filename
 *
 * \return TRUE if OK FALSE other
 **/
static gchar *gsb_file_test_and_load_csv_file (struct ImportFile *imported)
{
	gchar* tmp_str1;
    gchar *contents;
    gsize size;
    gsize bytes_written;
    GError *error;

	/* Open file */
    if (!g_file_get_contents (imported->name, &tmp_str1, &size, &error))
    {
        g_print (_("Unable to read file: %s\n"), error->message);
        g_error_free (error);

		return NULL;
    }

	/*longueur nulle */
	if (size == 0)
	{
		gchar *tmp_str2;

		tmp_str2 = g_path_get_basename (imported->name);
		tmp_str1 = g_strdup_printf ( _("The file %s is empty. Please choose another file."), tmp_str2);
        dialogue_warning_hint (tmp_str1, _("File empty."));

		g_free (tmp_str1);
		g_free (tmp_str2);

		return NULL;
    }

	/* Convert in UTF8 */
    error = NULL;
    contents = g_convert_with_fallback (tmp_str1, -1, "UTF-8",
										imported->coding_system,
										"?", &size, &bytes_written,
										&error);

    if (contents == NULL)
    {
        g_error_free (error);
        error = NULL;
        size = 0;
        bytes_written = 0;

        dialogue_warning_hint (
                            _("If the result does not suit you, try again by selecting the "
                            "correct character set in the window for selecting files."),
                            _("The conversion to utf8 went wrong."));

        contents = g_convert_with_fallback (tmp_str1, -1, "UTF-8",
											"ISO-8859-1",
											"?", &size, &bytes_written,
											&error);
        if (bytes_written == 0)
        {
            g_print (_("Unable to read file: %s\n"), error->message);
            g_error_free (error);
            return FALSE;
        }
    }

    g_free (tmp_str1);

	/* uniformise les sauts de ligne */
	tmp_str1 = contents;
    contents = gsb_string_uniform_new_line ( tmp_str1, (gssize) size);
	g_free (tmp_str1);

	return contents;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gchar *csv_import_button_rule_get_account_id (GArray *lines_tab,
													 gint csv_account_id_col,
													 gint csv_account_id_row)
{
	gchar *account_id = NULL;
	gchar *tmp_str;
	GSList *list;

	list = g_array_index (lines_tab, GSList *, csv_account_id_row-1);
	tmp_str = (gchar*) g_slist_nth_data (list, csv_account_id_col-1);

	if (tmp_str && strlen (tmp_str))
		account_id = g_strstrip (g_strdup (tmp_str));

	return account_id;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GArray *csv_import_button_rule_traite_spec_line (SpecConfData *spec_conf_data,
														GArray *lines_tab,
														gint index)
{
    GSList *list;
	gint action;

	devel_debug (NULL);
	if (!csv_fields_config || !lines_tab)
	{
		return lines_tab;
	}

	/* adaptation du tableau des données */
	action = spec_conf_data->csv_spec_conf_action;

	list = g_array_index (lines_tab, GSList *, index);
    do
    {
		gchar *data_entry;

		data_entry = (gchar*) g_slist_nth_data (list, spec_conf_data->csv_spec_conf_used_data);
		if (g_utf8_collate (data_entry, spec_conf_data->csv_spec_conf_used_text) == 0)
		{
			if (action == 0) 		/* suppression de la ligne */
			{
				g_array_remove_index (lines_tab, index);
				index--;
			}
			else if (action == 1)	/* inversion du montant */
			{
				GSList *link;
				gchar *str_montant;
				gsb_real montant;

				link = g_slist_nth (list, spec_conf_data->csv_spec_conf_action_data);
				str_montant = (gchar*) link->data;
				montant = gsb_real_opposite (utils_real_get_from_string (str_montant));
				str_montant = utils_real_get_string (montant);
				list = g_slist_delete_link (list, link);
				list = g_slist_insert (list, g_strdup (str_montant), spec_conf_data->csv_spec_conf_action_data);
			}
		}

		index++;
        list = g_array_index (lines_tab, GSList *, index);
    }
    while (list);

	return lines_tab;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void csv_import_button_rule_clicked (GtkButton *button,
											GtkWidget *assistant)
{
	CsvTemplateRule *dialog;
	CSVImportRule *csv_import_rule = NULL;

	devel_debug (NULL);

	dialog = csv_template_rule_new (assistant);

	gtk_dialog_run (GTK_DIALOG (dialog));

	csv_import_rule = g_object_get_data (G_OBJECT (dialog), "csv-import-rule");

	if (csv_import_rule)
	{
		GSList *list;
		gint i = 0;
		SpecConfData *spec_conf_data;

		csv_create_rule = TRUE;
		g_object_set_data (G_OBJECT (assistant), "csv-import-rule", csv_import_rule);
		list = csv_import_rule->csv_spec_lines_list;
		if (!list)
		{
			gtk_widget_destroy (GTK_WIDGET (dialog));
			return;
		}

		spec_conf_data = (SpecConfData *) list->data;
		if (spec_conf_data->csv_spec_conf_used_text)
		{
			GArray *lines_tab;

			lines_tab = g_object_get_data (G_OBJECT(assistant), "lines-tab");
			do
			{
				gint action;
				gint index;
				if (i)
					spec_conf_data = (SpecConfData *) list->data;

				/* détermination de la première transaction du fichier */
				action = spec_conf_data->csv_spec_conf_action;
				if (action == 0)
					index = CSV_MAX_TOP_LINES;
				else if (action == 1)
				{
					if (csv_import_rule->csv_headers_present)
						index = csv_import_rule->csv_first_line_data;
					else
						index = csv_import_rule->csv_first_line_data-1;
				}
				else
					index = 0;

				if (index)
					lines_tab = csv_import_button_rule_traite_spec_line (spec_conf_data, lines_tab, index);
				i++;
				list = list->next;
			}
			while (list);

			g_object_set_data (G_OBJECT(assistant), "lines-tab", lines_tab);
		}
		gtk_widget_destroy (GTK_WIDGET (dialog));

		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
	}
}

/**
 * Parse raw CSV text using separator and return a list containing all
 * fields.
 *
 * \param contents	A pointer to raw CSV contents to parse.
 *					Pointer content will be changed to the end of
 *					parsed line.
 * \param separator	Separator to use.
 *
 * \return			Parsed list or NULL upon failure (last line).
 **/
static GSList *csv_get_next_line (gchar **contents,
								  gchar *separator)
{
    GSList *list;

    do
    {
		list = csv_parse_line (contents, separator);
    }
    while (list == GINT_TO_POINTER(-1));

	return list;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GArray *csv_import_init_lines_tab (gchar **contents,
										  gchar *separator)
{
	GArray *lines_tab;
    GSList *list;

	devel_debug (NULL);

	lines_tab = g_array_sized_new (TRUE, FALSE, sizeof (GSList*), CSV_MAX_TOP_LINES);

    do
    {
		list = csv_get_next_line (contents, separator);
		g_array_append_val (lines_tab, list);
    }
	while (list);

	return lines_tab;
}

/**
 * Count number of columns if a raw CSV text were parsed using
 * a separator.
 *
 * \param contents	Raw CSV contents to parse.
 * \param separator	Separator.
 *
 * \return			Number of columns.
 **/
static gint csv_import_count_columns (GArray *lines_tab,
									  gchar *separator)
{
    guint max = 0;
	gint i = 0;
    GSList *list;

    if (!lines_tab)
		return 0;

    do
    {
		list = g_array_index (lines_tab, GSList *, i);

		if (g_slist_length (list) > max)
		{
			max = g_slist_length (list);
			first_line_with_cols = i;
		}

		i++;
    }
    while (list && i < CSV_MAX_TOP_LINES);

    return max;
}

/**
 * Skip n lines in the CSV stream.
 *
 * \param contents	Pointer to CSV data.
 * \param num_lines	Number of lines to skip.
 * \param
 *
 * \return 			Actual number of lines skipped.
 **/
static gint csv_skip_lines (gchar **contents,
							gint num_lines,
							gchar *separator)
{
    GSList *list;
    int i;

    /* g_print ("Skipping %d lines\n", num_lines); */

    for (i = 0; i < num_lines; i ++)
    {
		list = csv_get_next_line (contents, separator);

		if (!list)
		{
			return i;
		}
    }

    return i;
}

/**
 * Safely checks if a string is contained in another one.
 *
 * \param original	String to search substring into.
 * \param substring	Substring to search into original string.
 *
 * \return			TRUE if substring is contained into original.
 **/
static gboolean safe_contains (gchar *original,
							   gchar *substring)
{
	gchar *tmp_original;
	gchar *tmp_substring;
	gint ptr = 0;

    g_return_val_if_fail (original, FALSE);
    g_return_val_if_fail (substring, FALSE);

	tmp_original = g_utf8_strdown (original, -1);
	tmp_substring = g_utf8_strdown (substring, -1);

    ptr = GPOINTER_TO_INT (g_strstr_len (tmp_original, strlen (original), tmp_substring));

	g_free (tmp_original);
	g_free (tmp_substring);

	return ptr;
}

/**
 * Pre-fills the CSV field configuration using various heuristics.
 * Should match most cases thought this is purely statistical.
 *
 * \param contents	Data extracted from CSV file.
 * \param size		Number of columns of data (dependant of separator).
 * \param separator	Columns separator.
 *
 * \return			A newly allocated int array, containg guessed fields.
 **/
static gint *csv_import_guess_fields_config (GArray *lines_tab,
											 gint size,
											 gchar *separator)
{
    gchar *string;
    gint *default_config;
	gint i;
    GSList *list;
	gboolean date_validated = 0;

    default_config = (gint *) g_malloc0 ((size + 1) * sizeof (int));

	/* force skip first lines */
	for (i = 0; i <= first_line_with_cols; i++)
	{
		etat.csv_skipped_lines [i] = 1;
	}
	/* init other lines to 0 */
	for (; i <CSV_MAX_TOP_LINES; i++)
	{
		etat.csv_skipped_lines [i] = 0;
	}

    list = g_array_index (lines_tab, GSList *, first_line_with_cols);

	if (!list)
		return default_config;

    /** First, we try to match first line because it might contains of
     * the fields.  */
    for (i = 0 ; i < size && list ; i ++)
    {
		gint field;

		gchar *value = list->data;

		for (field = 0 ; csv_fields [field].name != NULL ; field ++)
		{
			if (strlen (value) > 1 && strlen (csv_fields [field].name) > 1)
			{
				if (strlen (csv_fields [field].alias) > 1
				 &&
				 (safe_contains (csv_fields [field].alias, value) ||
				   safe_contains (_(csv_fields [field].alias), value) ||
				   safe_contains (csv_fields [field].name, value) ||
				   safe_contains (_(csv_fields [field].name), value)))
				{
					if (!default_config [i])
					{
						default_config [i] = field;
						break;
					}

				}
				else if (safe_contains (csv_fields [field].name, value) ||
				 safe_contains (_(csv_fields [field].name), value))
				{
					if (!default_config [i])
					{
						default_config [i] = field;
						break;
					}
				}
			}
		}

		list = list->next;
    }
	default_config [i] = -1;

    /** Then, we try using heuristics to determine which field is date
     * and which ones contain amounts.  We cannot guess payees or
     * comments so we only auto-detect these fields. */

	list = g_array_index (lines_tab, GSList *, first_line_with_cols + 1);

	if (!list)
		return default_config;

	for (i = 0 ; i < size && list ; i ++)
	{
		string = list->data;

		if (strlen (string))
		{
			if (csv_import_validate_date (string) && !date_validated && !default_config [i])
			{
				default_config [i] = 2; 	/* Date */
				date_validated = TRUE;
			}
			else if (csv_import_validate_amount (string) &&
					 !csv_import_validate_number (string) &&
					 strlen(string) <= 9)	/* This is a hack since most numbers are smaller than 8 chars. */
			{
				if (g_strrstr (string, "-")) /* This is negative */
				{
					if (!default_config [i])
					{
						default_config [i] = 11; /* Negative debit */
					}
					else if (default_config [i] == 12)
					{
						default_config [i] = 11; /* Neutral amount */
					}
				}
				else
				{
					if (!default_config [i])
					{
						default_config [i] = 12; /* Negative debit */
					}
					else if (default_config [i] == 14)
					{
						default_config [i] = 11; /* Neutral amount */
					}
				}
			}
		}
		list = list->next;
	}
	default_config [i] = -1;

    return default_config;
}

/**
 * Update the CSV fields config structure to a new size.  Mainly used
 * when separator changes so that we do the allocation.
 *
 * \param contents	Raw CSV contents, used to guess config if not set already.
 * \param size		New size of allocation.
 * \paran separator	Separator, used to guess config if not set already.
 *
 * \return			A newly allocated integers array.
 **/
static gint *csv_import_update_fields_config (GArray *lines_tab,
											  gint size,
											  gchar *separator)
{
    gint i;
	gint *old_csv_fields_config;

    g_return_val_if_fail (size, NULL);

	old_csv_fields_config = csv_fields_config;

	if (!old_csv_fields_config)
    {
		return csv_import_guess_fields_config (lines_tab, size, separator);
    }

    csv_fields_config = (gint *) g_malloc ((size + 2) * sizeof (gint));

    for (i = 0; i < size && old_csv_fields_config [i] != -1 ; i ++)
    {
		csv_fields_config [i] = old_csv_fields_config [i];
    }

    for (; i < size ; i ++)
    {
		csv_fields_config[i] = 0;
    }

    if (old_csv_fields_config)
    {
		g_free (old_csv_fields_config);
    }
    csv_fields_config [i] = -1;

    return csv_fields_config;
}

/**
 * Find if a specific option is set in CSV field config.
 *
 * \param searched	Field to test presence.
 *
 * \return			TRUE is found.  FALSE otherwise.
 **/
static gboolean csv_find_field_config (gint searched)
{
    gint f;

    for (f = 0 ; csv_fields_config[f] != -1 ; f ++)
    {
		if (csv_fields_config [f] == searched)
		{
			return TRUE;
		}
    }

    return FALSE;
}

/**
 * Perform various checks on csv field configs.
 *
 * \param assistant	A pointer to the GsbAssistant holding CSV configuration.
 *
 * \return
 **/
static void csv_import_update_validity_check (GtkWidget *assistant)
{
    int i, needed[] = { 2, 4, -1 };
    gchar *label = NULL;

    if (!csv_fields_config)
		return;

    /* Check all needed fields.  */
    for (i = 0 ; needed [i] != -1 ; i ++)
    {
		if (!csv_find_field_config (needed [i]))
		{
			if (label)
			{
				label = g_strconcat (label, ", ", _(csv_fields [needed [i]].name), NULL);
			}
			else
			{
				label = _(csv_fields [needed [i]].name);
			}
		}
    }

    /** After checking all required fields, check the conformity of
     * transaction amount, which is somewhat complicated. */
	/* NON ((11 ET NON 12 ET NON 13 ET NON 14) OU (NON 11 ET 12 ET (13 XOR 14))) */
    if (!(																		/* NON (() OU ()) */
		  (csv_find_field_config (11) && !csv_find_field_config (12)
		   && !csv_find_field_config (13) && !csv_find_field_config (14))
		  ||																	/* OU */
		  (!csv_find_field_config (11) && csv_find_field_config (12)
		   &&
		   (csv_find_field_config (13) || csv_find_field_config (14))
		   && !(csv_find_field_config (13) && csv_find_field_config (14))
		   )
		  )
		)
    {
        if (label)
        {
            label = g_strconcat (label, ", ", _("transaction amount"), NULL);
        }
        else
        {
            label = my_strdup (_("transaction amount"));
        }
    }

    /** Then, fill in a GtkLabel containing diagnostic message and
     * show or hide a warning icon.  */
    if (label)
    {
		gtk_label_set_markup (g_object_get_data (G_OBJECT(assistant),
												 "validity_label"),
							  g_markup_printf_escaped (_("<b>The following fields are missing or inconsistent:</b> %s"),
													   label));
		gtk_widget_show (g_object_get_data (G_OBJECT(assistant), "validity_icon"));
		gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant),
													 "button_next"),
								  FALSE);
    }
    else
    {
		gtk_label_set_markup (g_object_get_data (G_OBJECT(assistant), "validity_label"),
							  _("All mandatory fields are filed in."));
		gtk_widget_hide (g_object_get_data (G_OBJECT(assistant), "validity_icon"));
		gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), TRUE);
		gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_rule"), TRUE);
    }
}

/**
 * Event triggered when a pop-up menu associated to a column is
 * triggered and option is selected.  Changes the CSV field config for
 * this column.  Title label of column is changed too.
 *
 * \param item		GtkMenuItem that triggered event.
 * \param no_menu	Position of column that triggered pop up menu.
 *
 * \return			FALSE
 **/
static gboolean csv_import_change_field (GtkWidget *item,
										 gint no_menu)
{
    GtkTreeViewColumn *col;
    GtkWidget *label;
    gchar *name;
    gint field;

    col = g_object_get_data (G_OBJECT(item), "column");
    field = GPOINTER_TO_INT (g_object_get_data (G_OBJECT(item), "field"));

    gtk_tree_view_column_set_title (col, _(csv_fields [no_menu].name));

    if (no_menu > 0)
    {
		name = g_strconcat ("<b><u>", _(csv_fields [no_menu].name), "</u></b>", NULL);
    }
    else
    {
		name = g_strdup (_(csv_fields [no_menu].name));
    }
    label = gtk_label_new (name);
    gtk_label_set_markup (GTK_LABEL(label), name);
    gtk_widget_show (label);
    gtk_tree_view_column_set_widget (col, label);
	g_free (name);

    csv_fields_config [field] = no_menu;

    csv_import_update_validity_check (g_object_get_data (G_OBJECT(col), "assistant"));

    return FALSE;
}

/**
 * Create a menu containing a link to all possible CSV fields.  This
 * is normally an event triggered by a click on a GtkTreeViewColumn.
 *
 * \param col		Column that triggered event.
 * \param field		Position of column that triggered event.
 * \param assistant	Assistant that contains the column.
 *
 * \return			A newly-created GtkMenu.
 **/
static GtkWidget *csv_import_fields_menu (GtkTreeViewColumn *col,
										  gint field,
										  GtkWidget *assistant)
{
    GtkWidget *menu, *item;
    int i;

    menu = gtk_menu_new();

    for (i = 0 ; csv_fields[i].name ; i++)
    {
		item = gtk_menu_item_new_with_label ((gchar *) _(csv_fields[i].name));
		g_object_set_data (G_OBJECT (item), "column", col);
		g_object_set_data (G_OBJECT (item), "field", GINT_TO_POINTER(field - 1));
		g_object_set_data (G_OBJECT (item), "assistant", assistant);
		g_signal_connect (G_OBJECT (item),
						  "activate",
						  G_CALLBACK (csv_import_change_field),
						  GINT_TO_POINTER( i));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    }

    gtk_widget_show_all (menu);
    return menu;
}

/**
 * Triggered when user clicks on a column header.  Create and pop up a
 * menu to allow CSV config change.
 *
 * \param button	Button placed at the title column.
 * \param no_column	Position of the column.
 *
 * \return			FALSE
 **/
static gboolean csv_import_header_on_click (GtkWidget *button,
											gint *no_column)
{
    GtkWidget *menu, *col;

    col = g_object_get_data (G_OBJECT (button), "column");

    menu = csv_import_fields_menu (GTK_TREE_VIEW_COLUMN (col),
								   GPOINTER_TO_INT (no_column),
								   g_object_get_data (G_OBJECT(col), "assistant"));

#if GTK_CHECK_VERSION (3,22,0)
	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
#else
	gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
#endif

    return FALSE;
}

/**
 * Callback triggered when the "skip" attribute of a line in the tree
 * preview is changed (that is, the checkbox associated is changed).
 * It marks this line as skipped.
 *
 * \param cell			Not used.
 * \param path_str		Textual representation of the path of modified checkbox.
 * \param tree_preview	GtkTreeView triggering event.
 *
 * \return
 **/
static void skip_line_toggled (GtkCellRendererToggle *cell,
							   gchar *path_str,
							   GtkTreeView *tree_preview)
{
    GtkTreeIter iter;
    gboolean toggle_item;
    gint *indices;
    GtkTreePath *path;
    GtkTreeModel *tree_model;

	path = gtk_tree_path_new_from_string (path_str);
	tree_model = gtk_tree_view_get_model (tree_preview);

	/* Get toggled iter */
    gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (tree_model), &iter, 0, &toggle_item, -1);
    gtk_tree_store_set (GTK_TREE_STORE (tree_model), &iter, 0, !toggle_item, -1);

    indices = gtk_tree_path_get_indices (path);
    etat.csv_skipped_lines [indices[0]] = !toggle_item;
}

/**
 * Create the model containing CSV file preview.
 *
 * \param tree_preview		GtkTreeView to associate model to.
 * \param contents			Contents to place into model
 * \param separator
 *
 * \return
 **/
static GtkTreeModel *csv_import_create_model (GtkTreeView *tree_preview,
											  GArray *lines_tab,
											  gchar *separator)
{
    GtkWidget *assistant;
    GtkTreeStore *model;
    GtkTreeViewColumn *col;
    GtkCellRenderer *cell;
    GType *types;
    gint size = 0;
    gint i;
    GList *list;

	size = csv_import_count_columns (lines_tab, separator);
    if (!size || !lines_tab)
	    return NULL;

    csv_fields_config = csv_import_update_fields_config (lines_tab, size, separator);

    assistant = g_object_get_data (G_OBJECT(tree_preview), "assistant");
	g_object_set_data (G_OBJECT (assistant), "csv_first_line_data", GINT_TO_POINTER (first_line_with_cols+1));

    /* Remove previous columns if any. */
    list = gtk_tree_view_get_columns (GTK_TREE_VIEW (tree_preview));

    while (list)
    {
        gtk_tree_view_remove_column (tree_preview, list->data);
        list = list->next;
    }

    types = g_malloc0 ((size + 2) * sizeof (GType *));

    types[0] = G_TYPE_BOOLEAN;
    cell = gtk_cell_renderer_toggle_new ();
    col = gtk_tree_view_column_new_with_attributes (_("Skip"),
													cell,
													"active", 0,
													NULL);
    gtk_tree_view_append_column (tree_preview, col);
    g_object_set_data (G_OBJECT (col), "assistant", assistant);
    g_signal_connect (cell,
                      "toggled",
                      G_CALLBACK (skip_line_toggled),
                      tree_preview);

    for (i = 0 ; i < size ; i ++)
    {
        GtkWidget *label;
        gchar *name;

        types[i+1] = G_TYPE_STRING;

        cell = gtk_cell_renderer_text_new ();
        col = gtk_tree_view_column_new_with_attributes (NULL,
														cell,
														"text", i + 1,
														"strikethrough", 0,
														NULL);

        if (csv_fields_config[i] > 0)
        {
            name = g_strconcat ("<b><u>",
								_(csv_fields [csv_fields_config[i]].name),
								"</u></b>",
								NULL);
        }
        else
        {
            name = my_strdup (_(csv_fields [csv_fields_config[i]].name));
        }

        label = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL(label), name);
        gtk_widget_show (label);
        g_free (name);

        gtk_tree_view_column_set_widget (col, label);
        gtk_tree_view_append_column (tree_preview, col);
        gtk_tree_view_column_set_clickable (col, TRUE);
        g_object_set_data (G_OBJECT (col), "column", col);
        g_object_set_data (G_OBJECT (col), "assistant", assistant);
        g_signal_connect (G_OBJECT (col),
						  "clicked",
						  G_CALLBACK (csv_import_header_on_click),
						  GINT_TO_POINTER (i + 1));
    }

    model =  gtk_tree_store_newv (size + 1, types);

    return (GtkTreeModel *) model;
}

/**
 * Try to match separator against raw CSV contents and see if it would
 * be consistent, see csv_import_guess_separator().
 *
 * \param contents	Raw CSV contents to parse.
 * \param separator	Separator to try.
 *
 * \return			FALSE on failure, number of columns otherwise.
 **/
static gint csv_import_try_separator (gchar *contents,
									  gchar *separator)
{
    GSList *list;
    int cols, i = 0;

    csv_skip_lines (&contents, 3, separator);

    list = csv_get_next_line (&contents, separator);
    cols = g_slist_length (list);
	csv_import_free_line (list, cols);

    //~ g_print ("> I believe first line is %d cols\n", cols);

    do
    {
		list = csv_get_next_line (&contents, separator);

		if (list && (cols != (gint) g_slist_length (list) || cols == 1))
		{
			//~ g_print ("> %d != %d, not %s\n", cols, g_slist_length (list), separator);
			cols = g_slist_length (list);
			csv_import_free_line (list, cols);
			return FALSE;
		}
		csv_import_free_line (list, cols);
		i++;
    }
    while (list && i < CSV_MAX_TOP_LINES);

    //~ g_print ("> I believe separator could be %s\n", separator);
    return cols;
}

/**
 * Based on raw text, try to estimate which character is the CSV
 * separator.  For each separator, try to estimate if this would make
 * consistent lines with the same amount of columns.  Of course, if
 * CSV is broken, it will fail and revert back to comma as a
 * separator, which seems the most used nowadays.
 *
 * This is black magic, believe me !
 *
 * \param contents	Raw CSV text to parse.
 *
 * \return			A newly-allocated string containing estimated
 *					or default CSV separator.
 **/
static gchar *csv_import_guess_separator (gchar *contents)
{
    gchar *separators[5] = { ",", ";", "	", " ", NULL }, *cmax = NULL;
    gint i, max = 0;

    for (i = 0 ; separators[i] ; i++)
    {
		gchar *tmp = contents;
		int n = csv_import_try_separator (tmp, separators[i]);

		if (n > max)
		{
			max = n;
			cmax = separators[i];
		}
    }

    if (cmax)
    {
		return my_strdup (cmax);
    }

    /* Semicolon is the most used separator, so as we are puzzled we try
     * this one. */
    return my_strdup (";");
}

/**
 * Callback triggered when user changed the pre-defined csv separators
 * combobox.  Update the text entry and thus the preview.
 *
 * \param combo		GtkComboBox that triggered event.
 * \param entry		Associated entry to change.
 *
 * \return			FALSE
 **/
static gboolean csv_import_combo_changed (GtkComboBox *combo,
										  GtkEntry *entry)
{
    gint active = gtk_combo_box_get_active (combo);

    if (csv_separators [active].value)
    {
		gtk_entry_set_text (entry, csv_separators [active].value);
    }
    else
    {
		gtk_entry_set_text (entry, "");
    }

    return FALSE;
}

/**
 * Update tree view containing preview of CSV import.
 *
 * \param assistant	GsbAssistant containg all UI.
 *
 * \return			FALSE
 **/
static gboolean csv_import_update_preview (GtkWidget *assistant)
{
	GArray *lines_tab;
    gchar *separator;
    GtkTreeModel *model;
    GtkTreeView *tree_preview;
    GSList *list;
    gint line = 0;

	devel_debug (NULL);
    separator = g_object_get_data (G_OBJECT(assistant), "separator");
    tree_preview = g_object_get_data (G_OBJECT(assistant), "tree_preview");
	lines_tab = g_object_get_data (G_OBJECT(assistant), "lines-tab");

	if (!lines_tab || !tree_preview || !separator)
	{
		return FALSE;
	}

	assistant = g_object_get_data (G_OBJECT(tree_preview), "assistant");
    model = csv_import_create_model (tree_preview, lines_tab, separator);
    if (model)
    {
        gtk_tree_view_set_model (GTK_TREE_VIEW(tree_preview), model);
        g_object_unref (G_OBJECT(model));
    }
	else
	{
		return FALSE;
	}

    while (line < CSV_MAX_TOP_LINES)
    {
        GtkTreeIter iter;
        gint col = 1;

        list = g_array_index (lines_tab, GSList *, line);

        if (!list)
        {
            return FALSE;
        }

        gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
        while (list)
        {
			gchar *tmp_str;

			tmp_str = gsb_string_truncate (list->data);
            gtk_tree_store_set (GTK_TREE_STORE (model),
								&iter,
								col, tmp_str,
								-1);
			g_free (tmp_str);
            col++;
            list = list->next;
        }

        if (etat.csv_skipped_lines [line])
        {
            gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 0, TRUE, -1);
        }

        line++;
    }

    csv_import_update_validity_check (assistant);

    return FALSE;
}

/**
 * Callback triggered when separator is changed in the GtkEntry
 * containing it.
 *
 * \param entry		Entry that triggered event.
 * \param position	Position of the change (not used).
 *
 * \return			FALSE
 **/
static gboolean csv_import_change_separator (GtkEntry *entry,
											 GtkWidget *assistant)
{
    GtkWidget *combobox;
    gchar *separator;
    int i = 0;

	devel_debug (NULL);
    combobox = g_object_get_data (G_OBJECT(entry), "combobox");
    separator = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
    if (strlen (separator) > 0)
    {
        g_object_set_data (G_OBJECT(assistant), "separator", separator);
        csv_import_update_preview (assistant);
		if (etat.csv_separator)
			g_free (etat.csv_separator);
        etat.csv_separator = separator;
    }
    else
    {
		if (etat.csv_separator)
			g_free (etat.csv_separator);
        etat.csv_separator = "";
        g_object_set_data (G_OBJECT(assistant), "separator", NULL);
    }

    /* Update combobox if we can. */
    while (csv_separators [i].value)
    {
        if (strcmp (csv_separators [i].value, separator) == 0)
        {
            break;
        }
        i ++ ;
    }
    g_signal_handlers_block_by_func (combobox, csv_import_combo_changed, entry);
    gtk_combo_box_set_active (GTK_COMBO_BOX(combobox), i);
    g_signal_handlers_unblock_by_func (combobox, csv_import_combo_changed, entry);

	return FALSE;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *	prepare l'importation du fichier csv
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_file_by_rule (gint rule,
								  struct ImportFile *imported)
{
    struct ImportAccount *compte;
	GArray *lines_tab;
    GSList *list;
	gchar **pointeur_char;
    gchar *contents;
	const gchar *csv_fields_str;
	gint line = 0;
	guint count;
	gint index = 0;

	devel_debug (imported->name);
	compte = g_malloc0 (sizeof (struct ImportAccount));
    compte->nom_de_compte = gsb_import_unique_imported_name (my_strdup (_("Imported CSV account")));
    compte->origine = my_strdup ("CSV");
    compte->real_filename = my_strdup (imported->name);

	contents = gsb_file_test_and_load_csv_file (imported);
	if (!contents)
		return FALSE;

	lines_tab = csv_import_init_lines_tab (&contents,
										   (gchar *) gsb_data_import_rule_get_csv_separator (rule));

	/* définitions des colonnes utiles pour Grisbi */
	csv_fields_str = gsb_data_import_rule_get_csv_fields_str (rule);
	if (!csv_fields_str)
		return FALSE;
	pointeur_char = g_strsplit (csv_fields_str, "-", 0);
	count = g_strv_length (pointeur_char);
	csv_fields_config = (gint *) g_malloc ((count + 2) * sizeof (gint));

	while (pointeur_char[line])
	{
		csv_fields_config[line] = utils_str_atoi (pointeur_char[line]);
		line ++;
	}
	csv_fields_config[line] = -1;
	g_strfreev ( pointeur_char );

	/* détermination de la première transaction du fichier */
	if (gsb_data_import_rule_get_csv_headers_present (rule))
		index = gsb_data_import_rule_get_csv_first_line_data (rule);
	else
		index = gsb_data_import_rule_get_csv_first_line_data (rule)-1;

	/* on regarde si il y a un traitement spécial */
	if (gsb_data_import_rule_get_csv_spec_nbre_lines (rule))
	{
		SpecConfData *spec_conf_data;

		list = gsb_data_import_rule_get_csv_spec_lines_list (rule);
		spec_conf_data = (SpecConfData *) list->data;
		while (list)
		{
			spec_conf_data = (SpecConfData *) list->data;
			lines_tab = csv_import_button_rule_traite_spec_line (spec_conf_data, lines_tab, index);
			list = list->next;
		};
	}

    list = g_array_index (lines_tab, GSList *, index);
    do
    {
        struct ImportTransaction *ope;
        gint i;

		ope = g_malloc0 (sizeof (struct ImportTransaction));
        ope->date = gdate_today ();
        ope->date_tmp = my_strdup ("");
        ope->tiers = my_strdup ("");
        ope->notes = my_strdup ("");
        ope->categ = my_strdup ("");
        ope->guid = my_strdup ("");

        for (i = 0; csv_fields_config[i] != -1 && list ; i++)
        {
            struct csv_field *field;

			field = &csv_fields [csv_fields_config[i]];
			if (field->parse)
            {
				if (field->validate)
				{
					if (field->validate (list->data))
					{
						if (csv_fields_config[i] == 16)
						{
							if (field->parse (ope, list->data))
							{
								gint nbre_element = g_slist_length (compte->operations_importees);
								struct ImportTransaction *ope_tmp;

								ope_tmp = (struct ImportTransaction *) g_slist_nth_data  (compte->operations_importees,
																						  nbre_element -1);
								if (ope_tmp->operation_ventilee == 0 && ope_tmp->ope_de_ventilation == 0)
									ope_tmp->operation_ventilee = 1;
								ope->ope_de_ventilation = 1;
							}
						}
						else if (!field->parse (ope, list->data))
						{
							/* g_print ("%s", "(failed)"); */
						}
					}
					else
						{
						/* g_print ("%s", "(invalid)"); */
						}
				}
            }
            list = list->next;
        }

        /* g_print (">> Appending new transaction %p\n", ope); */
        compte->operations_importees = g_slist_append (compte->operations_importees, ope);

		index++;
        list = g_array_index (lines_tab, GSList *, index);
    }
    while (list);

    if (compte->operations_importees)
    {
        /* Finally, we register it. */
        liste_comptes_importes = g_slist_append (liste_comptes_importes, compte);
    }
    else
    {
        /* ... or not, if no transaction was imported (implement sanitizing). */
        liste_comptes_importes_error = g_slist_append (liste_comptes_importes_error, compte);
    }

	csv_import_free_lines_tab (lines_tab);

    return FALSE;
}

/**
 * Actually do the grunt work, that is, parse the CSV file and create
 * importation structures in memory.
 *
 * \param assistant	The assistant that contains configuration for
 *					import.
 * \param imported	A pointer to the structure representing file
 *					import.
 *
 * \return			FALSE
 **/
gboolean csv_import_csv_account (GtkWidget *assistant,
								 struct ImportFile *imported)
{
    struct ImportAccount *compte;
	GArray *lines_tab;
    GSList *list;
    int index = 0;

	compte = g_malloc0 (sizeof (struct ImportAccount));
    compte->nom_de_compte = gsb_import_unique_imported_name (_("Imported CSV account"));
    compte->origine = my_strdup ("CSV");
    compte->real_filename = my_strdup (imported->name);

	lines_tab = g_object_get_data (G_OBJECT(assistant), "lines-tab");

    if (!csv_fields_config || !lines_tab)
    {
		liste_comptes_importes_error = g_slist_append (liste_comptes_importes_error, compte);
		return FALSE;
    }

	/* On complète les données du compte */
	compte->create_rule = csv_create_rule;
	if (csv_create_rule)
	{
		GSList *list;
		gchar *first_string_to_free;
		gchar *second_string_to_free;
		gchar *csv_fields_str;
		gint i;
		CSVImportRule *csv_import_rule;
		SpecConfData *spec_conf_data;

		csv_import_rule = g_object_get_data (G_OBJECT (assistant), "csv-import-rule");
		compte->csv_rule_name = csv_import_rule->csv_rule_name;
		compte->csv_first_line_data = csv_import_rule->csv_first_line_data;
		compte->csv_headers_present = csv_import_rule->csv_headers_present;
		compte->csv_account_id_col = csv_import_rule->csv_account_id_col;
		compte->csv_account_id_row = csv_import_rule->csv_account_id_row;

		/* get the account Id */
		if (csv_import_rule->csv_account_id_col > 0 && csv_import_rule->csv_account_id_row > 0)
		{
			compte->id_compte = csv_import_button_rule_get_account_id (lines_tab,
																	   csv_import_rule->csv_account_id_col,
																	   csv_import_rule->csv_account_id_row);
		}

		/* save the columns organization */
		csv_fields_str = utils_str_itoa (csv_fields_config[0]);
		for (i = 1; csv_fields_config[i] != -1 ; i++)
		{
			csv_fields_str = g_strconcat (first_string_to_free = csv_fields_str,
										  "-",
										  second_string_to_free = utils_str_itoa (csv_fields_config[i]),
										  NULL );

			g_free (first_string_to_free);
			g_free (second_string_to_free);
		}

		compte->csv_fields_str = csv_fields_str;

		/* save the columns names */
		compte->csv_spec_cols_name = csv_import_rule->csv_cols_name;

		/* saves the line of special configuration */
		list = csv_import_rule->csv_spec_lines_list;
		if (list)
		{
			spec_conf_data = (SpecConfData *) list->data;
			if (spec_conf_data->csv_spec_conf_used_text)
			{
				compte->csv_spec_lines_list = g_slist_copy_deep (list,
																 (GCopyFunc) csv_template_rule_spec_conf_data_struct_copy,
																 NULL);
			}
		}
	}

	/* détermination de la première transaction du fichier */
	if (compte->csv_headers_present)
		index = compte->csv_first_line_data;
	else
		index = compte->csv_first_line_data-1;

    list = g_array_index (lines_tab, GSList *, index);
    do
    {
        struct ImportTransaction *ope;
        gint i;

        /* Check if this line was specified as to be skipped earlier. */
        if (index < CSV_MAX_TOP_LINES && etat.csv_skipped_lines [index])
        {
            /* g_print ("Skipping line %d\n", index ); */
            index++;
			list = g_array_index (lines_tab, GSList *, index);

            continue;
        }

		ope = g_malloc0 (sizeof (struct ImportTransaction));
        ope->date = gdate_today ();
        ope->date_tmp = my_strdup ("");
        ope->tiers = my_strdup ("");
        ope->notes = my_strdup ("");
        ope->categ = my_strdup ("");
        ope->guid = my_strdup ("");

        for (i = 0; csv_fields_config[i] != -1 && list ; i++)
        {
            struct csv_field *field;

			field = &csv_fields [csv_fields_config[i]];
			if (field->parse)
            {
				if (field->validate)
				{
					if (field->validate (list->data))
					{
						if (csv_fields_config[i] == 16)
						{
							if (field->parse (ope, list->data))
							{
								gint nbre_element = g_slist_length (compte->operations_importees);
								struct ImportTransaction *ope_tmp;

								ope_tmp = (struct ImportTransaction *) g_slist_nth_data  (compte->operations_importees,
																						  nbre_element -1);
								if (ope_tmp->operation_ventilee == 0 && ope_tmp->ope_de_ventilation == 0)
									ope_tmp->operation_ventilee = 1;
								ope->ope_de_ventilation = 1;
							}
						}
						else if (!field->parse (ope, list->data))
						{
							/* g_print ("%s", "(failed)"); */
						}
					}
					else
						{
						/* g_print ("%s", "(invalid)"); */
						}
				}
            }
            list = list->next;
        }

        /* g_print (">> Appending new transaction %p\n", ope); */
        compte->operations_importees = g_slist_append (compte->operations_importees, ope);

		index++;
        list = g_array_index (lines_tab, GSList *, index);
    }
    while (list);

    if (compte->operations_importees)
    {
        /* Finally, we register it. */
        liste_comptes_importes = g_slist_append (liste_comptes_importes, compte);
    }
    else
    {
        /* ... or not, if no transaction was imported (implement sanitizing). */
        liste_comptes_importes_error = g_slist_append (liste_comptes_importes_error,
                                compte);
    }

	csv_import_free_lines_tab (lines_tab);

	return FALSE;
}

/**
 * Create the preview page of the import assistant.
 *
 * \param assistant	GsbAssistant to add page into.
 *
 * \return			A newly-allocated GtkVBox.
 **/
GtkWidget *import_create_csv_preview_page (GtkWidget *assistant)
{
    GtkWidget *vbox, *paddingbox, *tree_preview, *entry, *sw, *validity_label;
    GtkWidget *warn, *hbox, *combobox;
	GtkWidget *button;
    int i = 0;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);

    paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Choose CSV separator"));

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0);

    combobox = gtk_combo_box_text_new ();
    do
    {
		gchar *complete_name;

		complete_name = g_strdup_printf ("%s : \"%s\"",
										 _(csv_separators [i].name),
										 (csv_separators [i].value ?
										 csv_separators [i].value : ""));
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combobox), complete_name);
		g_free (complete_name);

    }
    while (csv_separators [i ++].value);

    gtk_box_pack_start (GTK_BOX(hbox), combobox, TRUE, TRUE, 0);

    entry = gsb_automem_entry_new (NULL,
								   G_CALLBACK (csv_import_change_separator),
								   assistant);
    g_object_set_data (G_OBJECT(entry), "assistant", assistant);
    g_object_set_data (G_OBJECT(entry), "combobox", combobox);
    g_object_set_data (G_OBJECT(assistant), "entry", entry);
    gtk_box_pack_start (GTK_BOX(hbox), entry, FALSE, FALSE, 0);

    g_signal_connect (G_OBJECT (combobox),
					  "changed",
					  G_CALLBACK (csv_import_combo_changed),
					  entry);

    paddingbox = new_paddingbox_with_title (vbox, TRUE, _("Select CSV fields"));

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (sw, 480, 120);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX(paddingbox), sw, TRUE, TRUE, 6);

    tree_preview = gtk_tree_view_new ();
	gtk_widget_set_name (tree_preview, "tree_view");
    g_object_set_data (G_OBJECT(assistant), "tree_preview", tree_preview);
    g_object_set_data (G_OBJECT(tree_preview), "assistant", assistant);
    gtk_container_add (GTK_CONTAINER (sw), tree_preview);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0);

    warn = gtk_image_new_from_icon_name ("gtk-dialog-warning", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start (GTK_BOX(hbox), warn, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT(assistant), "validity_icon", warn);

    validity_label = gtk_label_new (NULL);
    utils_labels_set_alignement (GTK_LABEL (validity_label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (validity_label), GTK_JUSTIFY_LEFT);
    g_object_set_data (G_OBJECT(assistant), "validity_label", validity_label);
    gtk_box_pack_start (GTK_BOX(hbox), validity_label, TRUE, TRUE, 0);

	button = gtk_button_new_with_label (_("Create a rule for this import."));
	g_object_set_data (G_OBJECT(assistant), "button_rule", button);
	g_signal_connect (G_OBJECT (button),
                      "clicked",
		              G_CALLBACK (csv_import_button_rule_clicked),
                      assistant);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_box_pack_start (GTK_BOX(paddingbox), button, FALSE, FALSE, 0);

    return vbox;
}

/**
 * Fill in all graphical elements of preview page with data from a CSV
 * file.
 *
 * \param assistant	Assistant that contains the page.
 *
 * \return		FALSE
 **/
gboolean import_enter_csv_preview_page (GtkWidget *assistant)
{
    GtkWidget *entry;
	GArray *lines_tab;
    GSList *files;
    gchar *contents;
    gchar *filename = NULL;
    struct ImportFile *imported = NULL;

	/* Find first CSV to import. */
    files = gsb_import_import_selected_files (assistant);
    while (files)
    {
        imported = files->data;

        if (!strcmp (imported->type, "CSV"))
        {
            filename = imported->name;
            break;
        }
        files = files->next;
    }
    g_return_val_if_fail (filename, FALSE);

	contents = gsb_file_test_and_load_csv_file (imported);
	if (!contents)
		return FALSE;

	g_object_set_data_full (G_OBJECT(assistant), "contents", contents, g_free);

	/* itialisation de separator */
	if (etat.csv_separator)
		g_free (etat.csv_separator);
	etat.csv_separator = csv_import_guess_separator (contents);

	/* On initialise le tableau avec les lignes du fichier */
	lines_tab = csv_import_init_lines_tab (&contents, etat.csv_separator);
	g_object_set_data (G_OBJECT(assistant), "lines-tab", lines_tab);
	first_line_with_cols = 0;

    entry = g_object_get_data (G_OBJECT(assistant), "entry");
	gtk_entry_set_text (GTK_ENTRY (entry), etat.csv_separator);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GSList *csv_import_get_columns_list	(GtkWidget *assistant)
{
	GArray *lines_tab;
	GSList *list;
	GSList *tmp_list;

	lines_tab = g_object_get_data (G_OBJECT(assistant), "lines-tab");
	tmp_list = g_array_index (lines_tab, GSList *, first_line_with_cols);
	list = g_slist_copy_deep (tmp_list, (GCopyFunc) g_strdup, NULL);

	return list;
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
