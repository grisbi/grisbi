/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*     2004-2008 Benjamin Drieu (bdrieu@april.org)                            */
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

/**
 * \file data_import_rule.c
 * work with the importing rule, no gui here
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_import_rule.h"
#include "dialog.h"
#include "import.h"
#include "structures.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of ImportRule */
static GSList *import_rule_list = NULL;

/** a pointer to the last import_rule used (to increase the speed) */
static ImportRule *import_rule_buffer;

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * This internal function is called to free the memory used by an ImportRule structure
 *
 * \param
 *
 * \return
 **/
static void _gsb_data_import_rule_free (ImportRule* import_rule)
{
    if (!import_rule)
		return;

	if (import_rule->charmap)
		g_free (import_rule->charmap);
    if (import_rule->last_file_name)
		g_free (import_rule->last_file_name);
    if (import_rule->rule_name)
		g_free (import_rule->rule_name);

	if (!g_strcmp0 (import_rule->type, "CSV"))
	{
		if (import_rule->csv_fields_str)
			g_free (import_rule->csv_fields_str);
		if (import_rule->csv_separator)
			g_free (import_rule->csv_separator);
		if (import_rule->csv_spec_text_str)
			g_free (import_rule->csv_spec_text_str);
		if (import_rule->csv_spec_cols_name)
			g_free (import_rule->csv_spec_cols_name);
		if (import_rule->csv_spec_lines_list)
			gsb_data_import_rule_free_csv_spec_lines_list (import_rule->import_rule_number);
	}

    if (import_rule->type)
		g_free (import_rule->type);

	if (import_rule_buffer == import_rule)
		import_rule_buffer = NULL;

	g_free (import_rule);

}

/**
 * find and return the structure of the import_rule asked
 *
 * \param import_rule_number 	number of import_rule
 *
 * \return 						the adr of the struct of the import_rule (NULL if doesn't exit)
 **/
static gpointer gsb_data_import_rule_get_structure (gint import_rule_number)
{
    GSList *tmp;

    if (!import_rule_number)
	return NULL;

    /* before checking all the import rule, we check the buffer */
    if (import_rule_buffer
	 &&
	 import_rule_buffer->import_rule_number == import_rule_number)
	return import_rule_buffer;

    tmp = import_rule_list;

    while (tmp)
    {
	ImportRule *import_rule;

	import_rule = tmp->data;

	if (import_rule->import_rule_number == import_rule_number)
	{
	    import_rule_buffer = import_rule;
	    return import_rule;
	}

	tmp = tmp->next;
    }
    return NULL;
}

/**
 * find and return the last number of import_rule
 *
 * \param none
 *
 * \return last number of import_rule
 * */
static gint gsb_data_import_rule_max_number (void)
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = import_rule_list;

    while (tmp)
    {
	ImportRule *import_rule;

	import_rule = tmp->data;

	if (import_rule->import_rule_number > number_tmp)
	    number_tmp = import_rule->import_rule_number;

	tmp = tmp->next;
    }
    return number_tmp;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * set the import rule global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_import_rule_init_variables (void)
{
    if (import_rule_list)
    {
		GSList* tmp_list = import_rule_list;
		while (tmp_list)
		{
			ImportRule *import_rule;

			import_rule = tmp_list->data;
			tmp_list = tmp_list->next;
			_gsb_data_import_rule_free (import_rule);
		}
		g_slist_free (import_rule_list);
    }
    import_rule_list = NULL;
    import_rule_buffer = NULL;

    return FALSE;
}

/**
 * give the g_slist of import rule structure
 * usefull when want to check all import rule
 *
 * \param none
 *
 * \return the g_slist of import rule structure
 * */
GSList *gsb_data_import_rule_get_list (void)
{
    return import_rule_list;
}

/**
 * return the number of the import rule given in param
 *
 * \param rule_ptr 		a pointer to the struct of the import_rule
 *
 * \return 				the number of the import_rule, 0 if problem
 * */
gint gsb_data_import_rule_get_number (gpointer rule_ptr)
{
    ImportRule *import_rule;

    if (!rule_ptr)
	return 0;

    import_rule = rule_ptr;
    import_rule_buffer = import_rule;
    return import_rule->import_rule_number;
}


/**
 * create a new import_rule, give him a number, append it to the list
 * and return the number
 *
 * \param name 		the name of the import_rule (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new import_rule
 * */
gint gsb_data_import_rule_new (const gchar *name)
{
    ImportRule *import_rule;

    import_rule = g_malloc0 (sizeof (ImportRule));
    if (!import_rule)
    {
		dialogue_error_memory ();
		return 0;
    }
    import_rule->import_rule_number = gsb_data_import_rule_max_number () + 1;

    if (name)
		import_rule->rule_name = my_strdup (name);
    else
		import_rule->rule_name = NULL;

	import_rule->csv_spec_lines_list = NULL;

    import_rule_list = g_slist_append (import_rule_list, import_rule);
    import_rule_buffer = import_rule;

    return import_rule->import_rule_number;
}

/**
 * remove an import_rule
 *
 * \param import_rule_number the import_rule we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_import_rule_remove (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	    return FALSE;

    /* remove the import_rule from the list */
    import_rule_list = g_slist_remove (import_rule_list,
					import_rule);

    _gsb_data_import_rule_free (import_rule);

    return TRUE;
}


/**
 * set a new number for the import_rule
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param import_rule_number the number of the import_rule
 * \param new_no_import_rule the new number of the import_rule
 *
 * \return the new number or 0 if the import_rule doen't exist
 * */
gint gsb_data_import_rule_set_new_number (gint import_rule_number,
										  gint new_no_import_rule)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return 0;

    import_rule->import_rule_number = new_no_import_rule;
    return new_no_import_rule;
}


/**
 * return the name of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the name of the import_rule or NULL if fail
 * */
const gchar *gsb_data_import_rule_get_name (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return NULL;

    return import_rule->rule_name;
}


/**
 * set the name of the import_rule
 * the value is dupplicate in memory
 *
 * \param import_rule_number the number of the import_rule
 * \param name the name of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_name (gint import_rule_number,
										const gchar *name)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return FALSE;

    /* we free the last name */
    if (import_rule->rule_name)
	g_free (import_rule->rule_name);

    /* and copy the new one */
    import_rule->rule_name = my_strdup (name);

    return TRUE;
}



/**
 * return the account number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the account number of the import_rule or 0 if fail
 * */
gint gsb_data_import_rule_get_account (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return 0;

    return import_rule->account_number;
}


/**
 * set the account number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param account_number the account number of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_account (gint import_rule_number,
										   gint account_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return FALSE;

    import_rule->account_number = account_number;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_import_rule_get_csv_account_id_col (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return 0;

    return import_rule->csv_account_id_col;
}

/**
 * set the account Id col of the import_rule
 *
 * \param import_rule_number	the number of the import_rule
 * \param account_id_col 		the account column of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_csv_account_id_col (gint import_rule_number,
													  gint csv_account_id_col)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    import_rule->csv_account_id_col = csv_account_id_col;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_import_rule_get_csv_account_id_row (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return 0;

    return import_rule->csv_account_id_row;
}

/**
 * set the account row of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param account_row the account row of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_csv_account_id_row (gint import_rule_number,
													  gint csv_account_id_row)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    import_rule->csv_account_id_row = csv_account_id_row;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_import_rule_get_csv_fields_str (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return NULL;

    return import_rule->csv_fields_str;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_import_rule_set_csv_fields_str (gint import_rule_number,
												  const gchar *csv_fields_str)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    import_rule->csv_fields_str = g_strdup (csv_fields_str);

    return TRUE;
}

/**
 * get the first line data of the CSV file for the import_rule
 *
 * \param import_rule_number 	the number of the import_rule
 *
 * \return the first line of data or 0 if problem
 * */
gint gsb_data_import_rule_get_csv_first_line_data (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return 0;

    return import_rule->csv_first_line_data;
}

/**
 * set the first line data of the CSV file for the import_rule
 *
 * \param import_rule_number 	the number of the import_rule
 * \param first_line_data		the first line of data
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_csv_first_line_data	(gint import_rule_number,
														 gint csv_first_line_data)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    import_rule->csv_first_line_data = csv_first_line_data;

    return TRUE;
}

/**
 * get if headers of cols are present for the import_rule
 *
 * \param import_rule_number 	the number of the import_rule
 *
 * \return TRUE if headers of cols are present or 0 if problem
 * */
gint gsb_data_import_rule_get_csv_headers_present (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return 0;

    return import_rule->csv_headers_present;
}

/**
 * set if headers of cols are present for the import_rule
 *
 * \param import_rule_number 	the number of the import_rule
 * \param headers_present		TRUE if headers are present
 *
 * \return TRUE if ok or FALSE if problem
 * */
gint gsb_data_import_rule_set_csv_headers_present	(gint import_rule_number,
													 gboolean csv_headers_present)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    import_rule->csv_headers_present = csv_headers_present;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_import_rule_get_csv_separator (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return NULL;

    return import_rule->csv_separator;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_import_rule_set_csv_separator (gint import_rule_number,
												 const gchar *csv_separator)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    import_rule->csv_separator = g_strdup (csv_separator);

    return TRUE;
}
/**
 * return the currency number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the currency number of the import_rule or 0 if fail
 * */
gint gsb_data_import_rule_get_currency (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return 0;

    return import_rule->currency_number;
}


/**
 * set the currency number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param currency_number the currency number of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_currency (gint import_rule_number,
											gint currency_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return FALSE;

    import_rule->currency_number = currency_number;

    return TRUE;
}


/**
 * return the invert number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the invert number of the import_rule or 0 if fail
 * */
gboolean gsb_data_import_rule_get_invert (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return 0;

    return import_rule->invert_transaction_amount;
}


/**
 * set the invert number of the import_rule
 *
 * \param import_rule_number 	the number of the import_rule
 * \param invert		TRUE to invert the amount of transactions
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_invert (gint import_rule_number,
										  gboolean invert)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return FALSE;

    import_rule->invert_transaction_amount = invert;

    return TRUE;
}


/**
 * return the last_file_name of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the last_file_name of the import_rule or NULL if fail
 * */
const gchar *gsb_data_import_rule_get_charmap (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return NULL;

    return import_rule->charmap;
}


/**
 * set the charmap of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param charmap the charmap of the import_file
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_charmap (gint import_rule_number,
										   const gchar *charmap)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return FALSE;

    /* we free the last charmap */
    if (import_rule->charmap)
	g_free (import_rule->charmap);

    /* and copy the new one */
    import_rule->charmap = my_strdup (charmap);

    return TRUE;
}


/**
 * return the last_file_name of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the last_file_name of the import_rule or NULL if fail
 * */
const gchar *gsb_data_import_rule_get_last_file_name (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return NULL;

    return import_rule->last_file_name;
}


/**
 * set the last_file_name of the import_rule
 * the value is dupplicate in memory
 *
 * \param import_rule_number the number of the import_rule
 * \param last_file_name the last_file_name of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_last_file_name (gint import_rule_number,
												  const gchar *last_file_name)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return FALSE;

    /* we free the last last_file_name */
    if (import_rule->last_file_name)
	g_free (import_rule->last_file_name);

    /* and copy the new one */
    import_rule->last_file_name = my_strdup (last_file_name);

    return TRUE;
}


/**
 * return the action of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the action of the import_rule or 0 if fail
 * */
gint gsb_data_import_rule_get_action (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return 0;

    return import_rule->action;
}


/**
 * set the action number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param action the action of the import_rule (IMPORT_ADD_TRANSACTIONS, IMPORT_MARK_TRANSACTIONS)
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_action (gint import_rule_number,
										  gint action)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	return FALSE;

    import_rule->action = action;

    return TRUE;
}



/**
 * return TRUE or FALSE depending if there are some rules for the account
 *
 * \param account_number
 *
 * \return TRUE : there is at least 1 rule, FALSE : no rules
 * */
gboolean gsb_data_import_rule_account_has_rule (gint account_number)
{
    GSList *tmp_list;

    if (account_number < 0)
	return FALSE;

    tmp_list = import_rule_list;
    while (tmp_list)
    {
	ImportRule *import_rule;

	import_rule = tmp_list->data;

	if (import_rule->account_number == account_number)
	    return TRUE;

	tmp_list = tmp_list->next;
    }
    return FALSE;
}


/**
 * return a list of rules structure according to the account number
 *
 * \param account_number
 *
 * \return a GSList of ImportRule or NULL if none
 * */
GSList *gsb_data_import_rule_get_from_account (gint account_number)
{
    GSList *tmp_list;
    GSList *returned_list = NULL;

    tmp_list = import_rule_list;
    while (tmp_list)
    {
		ImportRule *import_rule;

		import_rule = tmp_list->data;

		if (import_rule->account_number == account_number)
			returned_list = g_slist_append (returned_list, import_rule);

		tmp_list = tmp_list->next;
    }
    return returned_list;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_import_rule_get_type (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
	{
		return NULL;
	}

	if (import_rule->type)
	{
		return import_rule->type;
	}
	else
	{
		return NULL;
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
gboolean gsb_data_import_rule_set_type (gint import_rule_number,
										const gchar *type)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    /* we free the last type */
    if (import_rule->type)
		g_free (import_rule->type);

    /* and copy the new one */
    import_rule->type = my_strdup (type);

    return TRUE;
}
/**
 *
 *
 * \param
 * \param
 *
 * \return rule_number
 **/
gint gsb_data_import_rule_account_has_rule_name (gint account_number,
												 const gchar *rule_name)
{
    GSList *tmp_list;

    if (account_number < 0)
		return 0;

    tmp_list = import_rule_list;
    while (tmp_list)
    {
		ImportRule *import_rule;

		import_rule = tmp_list->data;

		if (import_rule->account_number == account_number)
		{
			if (!g_utf8_collate (rule_name, import_rule->rule_name))
			{
				return import_rule->import_rule_number;
			}
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
const gchar *gsb_data_import_rule_get_csv_spec_cols_name (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return NULL;

    return import_rule->csv_spec_cols_name;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_import_rule_set_csv_spec_cols_name (gint import_rule_number,
													  const gchar *csv_spec_cols_name)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

    /* we free the last csv_spec_cols_name string */
    if (import_rule->csv_spec_cols_name)
		g_free (import_rule->csv_spec_cols_name);

    /* and copy the new one */
    import_rule->csv_spec_cols_name = my_strdup (csv_spec_cols_name);

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GSList *gsb_data_import_rule_get_csv_spec_lines_list (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return NULL;

    return import_rule->csv_spec_lines_list;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_import_rule_set_csv_spec_lines_list (gint import_rule_number,
													   GSList *csv_spec_lines_list)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

	import_rule->csv_spec_lines_list = csv_spec_lines_list;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_import_rule_get_csv_spec_nbre_lines (gint import_rule_number)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return 0;

    return import_rule->csv_spec_nbre_lines;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_import_rule_set_csv_spec_nbre_lines (gint import_rule_number,
													   gint csv_spec_nbre_lines)
{
    ImportRule *import_rule;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return FALSE;

	import_rule->csv_spec_nbre_lines = csv_spec_nbre_lines;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_data_import_rule_free_csv_spec_lines_list (gint import_rule_number)
{
    ImportRule *import_rule;
    GSList *tmp_list;

    import_rule = gsb_data_import_rule_get_structure (import_rule_number);

    if (!import_rule)
		return;

	tmp_list = import_rule->csv_spec_lines_list;
    while (tmp_list)
    {
		SpecConfData *spec_conf_data;

		spec_conf_data = (SpecConfData *) tmp_list->data;
		if (spec_conf_data->csv_spec_conf_used_text)
			g_free (spec_conf_data->csv_spec_conf_used_text);
		g_free (spec_conf_data);

		tmp_list = tmp_list->next;
    }
	g_slist_free (tmp_list);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_data_import_rule_free_list (void)
{
    if (import_rule_list)
    {
		GSList* tmp_list;

		tmp_list = import_rule_list;
		while (tmp_list)
		{
			ImportRule *import_rule;

			import_rule = tmp_list->data;
			tmp_list = tmp_list->next;
			_gsb_data_import_rule_free (import_rule);
		}
		g_slist_free (import_rule_list);
    }
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


