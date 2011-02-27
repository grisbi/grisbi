/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2008 Benjamin Drieu (bdrieu@april.org) 	      */
/*			http://www.grisbi.org   			      */
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
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_import_rule.h"
#include "dialog.h"
#include "utils_str.h"
#include "import.h"
/*END_INCLUDE*/

/** \struct
 * describe a rule
 * */
typedef struct
{
    gint import_rule_number;

    gchar *rule_name;
    gint account_number;

    /* currency used to import the transactions */
    gint currency_number;

    /* if TRUE, all the transactions imported will have their amount inverted */
    gboolean invert_transaction_amount;

    /* charmap du fichier importé   */
    gchar *charmap;
    
    /* last file imported with a rule */
    gchar *last_file_name;

    /* action of the rule : IMPORT_ADD_TRANSACTIONS, IMPORT_MARK_TRANSACTIONS */
    gint action;
} struct_import_rule;


/*START_STATIC*/
static void _gsb_data_import_rule_free ( struct_import_rule* import_rule);
static gpointer gsb_data_import_rule_get_structure ( gint import_rule_number );
static gint gsb_data_import_rule_max_number ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of struct_import_rule */
static GSList *import_rule_list = NULL;

/** a pointer to the last import_rule used (to increase the speed) */
static struct_import_rule *import_rule_buffer;


/**
 * set the import rule global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_import_rule_init_variables ( void )
{
    if ( import_rule_list )
    {
	GSList* tmp_list = import_rule_list;
	while ( tmp_list )
	{
	    struct_import_rule *import_rule;
	    import_rule = tmp_list -> data;
	    tmp_list = tmp_list -> next;
	    _gsb_data_import_rule_free ( import_rule ); 
	}
	g_slist_free ( import_rule_list );
    }
    import_rule_list = NULL;
    import_rule_buffer = NULL;

    return FALSE;
}


/**
 * find and return the structure of the import_rule asked
 *
 * \param import_rule_number number of import_rule
 *
 * \return the adr of the struct of the import_rule (NULL if doesn't exit)
 * */
gpointer gsb_data_import_rule_get_structure ( gint import_rule_number )
{
    GSList *tmp;

    if (!import_rule_number)
	return NULL;

    /* before checking all the import rule, we check the buffer */
    if ( import_rule_buffer
	 &&
	 import_rule_buffer -> import_rule_number == import_rule_number )
	return import_rule_buffer;

    tmp = import_rule_list;

    while ( tmp )
    {
	struct_import_rule *import_rule;

	import_rule = tmp -> data;

	if ( import_rule -> import_rule_number == import_rule_number )
	{
	    import_rule_buffer = import_rule;
	    return import_rule;
	}

	tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of import rule structure
 * usefull when want to check all import rule
 *
 * \param none
 *
 * \return the g_slist of import rule structure
 * */
GSList *gsb_data_import_rule_get_list ( void )
{
    return import_rule_list;
}

/**
 * return the number of the import rule given in param
 *
 * \param rule_ptr a pointer to the struct of the import_rule
 *
 * \return the number of the import_rule, 0 if problem
 * */
gint gsb_data_import_rule_get_number ( gpointer rule_ptr )
{
    struct_import_rule *import_rule;

    if ( !rule_ptr )
	return 0;

    import_rule = rule_ptr;
    import_rule_buffer = import_rule;
    return import_rule -> import_rule_number;
}


/**
 * find and return the last number of import_rule
 * 
 * \param none
 * 
 * \return last number of import_rule
 * */
gint gsb_data_import_rule_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = import_rule_list;

    while ( tmp )
    {
	struct_import_rule *import_rule;

	import_rule = tmp -> data;

	if ( import_rule -> import_rule_number > number_tmp )
	    number_tmp = import_rule -> import_rule_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new import_rule, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the import_rule (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new import_rule
 * */
gint gsb_data_import_rule_new ( const gchar *name )
{
    struct_import_rule *import_rule;

    import_rule = g_malloc0 ( sizeof ( struct_import_rule ));
    if (!import_rule)
    {
	dialogue_error_memory ();
	return 0;
    }
    import_rule -> import_rule_number = gsb_data_import_rule_max_number () + 1;

    if (name)
	import_rule -> rule_name = my_strdup (name);
    else 
	import_rule -> rule_name = NULL;

    import_rule_list = g_slist_append ( import_rule_list, import_rule );
    import_rule_buffer = import_rule;

    return import_rule -> import_rule_number;
}

/**
 * This internal function is called to free the memory used by an struct_import_rule structure
 */
static void _gsb_data_import_rule_free ( struct_import_rule* import_rule)
{
    if ( ! import_rule )
	return;
    if ( import_rule -> rule_name )
	g_free ( import_rule -> rule_name );
    if ( import_rule -> charmap )
    g_free (import_rule -> charmap);
    if ( import_rule -> last_file_name )
	g_free ( import_rule -> last_file_name );
    g_free ( import_rule );
    if ( import_rule_buffer == import_rule )
	import_rule_buffer = NULL;
}



/**
 * remove an import_rule
 *
 * \param import_rule_number the import_rule we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_import_rule_remove ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	    return FALSE;
    
    /* remove the import_rule from the list */
    import_rule_list = g_slist_remove ( import_rule_list,
					import_rule );

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
gint gsb_data_import_rule_set_new_number ( gint import_rule_number,
					   gint new_no_import_rule )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return 0;

    import_rule -> import_rule_number = new_no_import_rule;
    return new_no_import_rule;
}


/**
 * return the name of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the name of the import_rule or NULL if fail
 * */
const gchar *gsb_data_import_rule_get_name ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return NULL;

    return import_rule -> rule_name;
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
gboolean gsb_data_import_rule_set_name ( gint import_rule_number,
					 const gchar *name )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return FALSE;

    /* we free the last name */
    if ( import_rule -> rule_name )
	g_free (import_rule -> rule_name);

    /* and copy the new one */
    import_rule -> rule_name = my_strdup (name);

    return TRUE;
}



/**
 * return the account number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the account number of the import_rule or 0 if fail
 * */
gint gsb_data_import_rule_get_account ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return 0;

    return import_rule -> account_number;
}


/**
 * set the account number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param account_number the account number of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_account ( gint import_rule_number,
					    gint account_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return FALSE;

    import_rule -> account_number = account_number;

    return TRUE;
}



/**
 * return the currency number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the currency number of the import_rule or 0 if fail
 * */
gint gsb_data_import_rule_get_currency ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return 0;

    return import_rule -> currency_number;
}


/**
 * set the currency number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param currency_number the currency number of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_currency ( gint import_rule_number,
					     gint currency_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return FALSE;

    import_rule -> currency_number = currency_number;

    return TRUE;
}


/**
 * return the invert number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the invert number of the import_rule or 0 if fail
 * */
gboolean gsb_data_import_rule_get_invert ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return 0;

    return import_rule -> invert_transaction_amount;
}


/**
 * set the invert number of the import_rule
 *
 * \param import_rule_number 	the number of the import_rule
 * \param invert		TRUE to invert the amount of transactions 
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_invert ( gint import_rule_number,
					   gboolean invert )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return FALSE;

    import_rule -> invert_transaction_amount = invert;

    return TRUE;
}


/**
 * return the last_file_name of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the last_file_name of the import_rule or NULL if fail
 * */
const gchar *gsb_data_import_rule_get_charmap ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return NULL;

    return import_rule -> charmap;
}


/**
 * set the charmap of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param charmap the charmap of the import_file
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_charmap ( gint import_rule_number,
					 const gchar *charmap )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return FALSE;

    /* we free the last charmap */
    if ( import_rule -> charmap )
	g_free (import_rule -> charmap);

    /* and copy the new one */
    import_rule -> charmap = my_strdup (charmap);

    return TRUE;
}


/**
 * return the last_file_name of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the last_file_name of the import_rule or NULL if fail
 * */
const gchar *gsb_data_import_rule_get_last_file_name ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return NULL;

    return import_rule -> last_file_name;
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
gboolean gsb_data_import_rule_set_last_file_name ( gint import_rule_number,
						   const gchar *last_file_name )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return FALSE;

    /* we free the last last_file_name */
    if ( import_rule -> last_file_name )
	g_free (import_rule -> last_file_name);

    /* and copy the new one */
    import_rule -> last_file_name = my_strdup (last_file_name);

    return TRUE;
}


/**
 * return the action of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 *
 * \return the action of the import_rule or 0 if fail
 * */
gint gsb_data_import_rule_get_action ( gint import_rule_number )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return 0;

    return import_rule -> action;
}


/**
 * set the action number of the import_rule
 *
 * \param import_rule_number the number of the import_rule
 * \param action the action of the import_rule (IMPORT_ADD_TRANSACTIONS, IMPORT_MARK_TRANSACTIONS)
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_import_rule_set_action ( gint import_rule_number,
					   gint action )
{
    struct_import_rule *import_rule;

    import_rule = gsb_data_import_rule_get_structure ( import_rule_number );

    if (!import_rule)
	return FALSE;

    import_rule -> action = action;

    return TRUE;
}



/**
 * return TRUE or FALSE depending if there are some rules for the account
 *
 * \param account_number
 *
 * \return TRUE : there is at least 1 rule, FALSE : no rules
 * */
gboolean gsb_data_import_rule_account_has_rule ( gint account_number )
{
    GSList *tmp_list;

    if (account_number < 0)
	return FALSE;

    tmp_list = import_rule_list;
    while (tmp_list)
    {
	struct_import_rule *import_rule;

	import_rule = tmp_list -> data;

	if (import_rule -> account_number == account_number)
	    return TRUE;

	tmp_list = tmp_list -> next;
    }
    return FALSE;
}


/**
 * return a list of rules structure according to the account number
 *
 * \param account_number
 *
 * \return a GSList of struct_import_rule or NULL if none
 * */
GSList *gsb_data_import_rule_get_from_account ( gint account_number )
{
    GSList *tmp_list;
    GSList *returned_list = NULL;

    tmp_list = import_rule_list;
    while (tmp_list)
    {
	struct_import_rule *import_rule;

	import_rule = tmp_list -> data;

	if (import_rule -> account_number == account_number)
	    returned_list = g_slist_append (returned_list, import_rule);

	tmp_list = tmp_list -> next;
    }
    return returned_list;
}



