/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/* 			http://www.grisbi.org				                              */
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
 * \file gsb_data_bank.c
 * work with the bank structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_bank.h"
#include "gsb_data_account.h"
#include "utils_str.h"
/*END_INCLUDE*/


/* these two macros are here to reduce
 * amount of code for setters and getters */
#define BANK_GET_OR_RETURN(bank, number, ret) \
    bank = gsb_data_bank_get_structure ( number ); \
    if ( !bank ) \
	return ret;
#define BANK_SET_FIELD(bank, field, value) \
    g_free (bank -> field); \
    bank -> field = my_strdup (value);


/**
 * \struct
 * Describe a bank
 */
typedef struct
{
    /* WARNING : cannot set guint for bank_number because a bug before (and after ?)
     * 0.6 sometimes set -1 to bank_number and big pb after that if we set guint here */
    gint bank_number;
    gchar *bank_name;
    gchar *bank_code;
    gchar *bank_BIC;                /* BIC number */

    gchar *bank_address;
    gchar *bank_tel;
    gchar *bank_mail;
    gchar *bank_web;
    gchar *bank_note;

    gchar *correspondent_name;
    gchar *correspondent_tel;
    gchar *correspondent_mail;
    gchar *correspondent_fax;
} struct_bank;

/*START_STATIC*/
static void _gsb_data_bank_free ( struct_bank* bank);
static gpointer gsb_data_bank_get_structure ( gint bank_number );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of struct_bank */
static GSList *bank_list = NULL;

/** a pointer to the last bank used (to increase the speed) */
static struct_bank *bank_buffer;


/**
 * set the banks global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_bank_init_variables ( void )
{
    if ( bank_list )
    {
        g_slist_free_full ( bank_list, (GDestroyNotify) _gsb_data_bank_free );
        bank_list = NULL;
    }

    bank_buffer = NULL;

    return FALSE;
}


/**
 * find and return the structure of the bank asked
 *
 * \param bank_number number of bank
 *
 * \return the adr of the struct of the bank (NULL if doesn't exit)
 * */
gpointer gsb_data_bank_get_structure ( gint bank_number )
{
    GSList *tmp;

    if (!bank_number)
	return NULL;

    /* before checking all the banks, we check the buffer */
    if ( bank_buffer
	 &&
	 bank_buffer -> bank_number == bank_number )
	return bank_buffer;

    tmp = bank_list;

    while ( tmp )
    {
	struct_bank *bank;

	bank = tmp -> data;

	if ( bank -> bank_number == bank_number )
	{
	    bank_buffer = bank;
	    return bank;
	}
	tmp = tmp -> next;
    }
    return NULL;
}

/**
 * return the number of the bank given in param
 *
 * \param bank_ptr a pointer to the struct of the bank
 *
 * \return the number of the bank, 0 if problem
 * */
gint gsb_data_bank_get_no_bank ( gpointer bank_ptr )
{
    struct_bank *bank;

    if ( !bank_ptr )
	return 0;

    bank = bank_ptr;
    bank_buffer = bank;
    return bank -> bank_number;
}


/**
 * give the g_slist of bank structure
 *
 * \param none
 *
 * \return the g_slist of banks structure
 * */
GSList *gsb_data_bank_get_bank_list ( void )
{
    return bank_list;
}



/**
 * find and return the last number of bank
 *
 * \param none
 *
 * \return last number of bank
 * */
gint gsb_data_bank_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = bank_list;

    while ( tmp )
    {
	struct_bank *bank;

	bank = tmp -> data;

	if ( bank -> bank_number > number_tmp )
	    number_tmp = bank -> bank_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new bank, give it a number, append it to the list
 * and return the number
 *
 * \param name the name of the bank (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new bank
 * */
gint gsb_data_bank_new ( const gchar *name )
{
    struct_bank *bank;

    bank = g_malloc0 ( sizeof ( struct_bank ));
    bank -> bank_number = gsb_data_bank_max_number () + 1;

    if (name)
	bank -> bank_name = my_strdup (name);

    bank_list = g_slist_append ( bank_list, bank );

    return bank -> bank_number;
}

/**
 * This internal function is called to free the memory used by a struct_bank structure
 */
static void _gsb_data_bank_free ( struct_bank* bank)
{
    if ( !bank )
	return;

    /* free string fields */
    g_free ( bank -> bank_name );
    g_free ( bank -> bank_code );
    g_free ( bank -> bank_BIC );
    g_free ( bank -> bank_address );
    g_free ( bank -> bank_web );
    g_free ( bank -> bank_note );
    g_free ( bank -> correspondent_name );
    g_free ( bank -> correspondent_tel );
    g_free ( bank -> correspondent_mail );
    g_free ( bank -> correspondent_fax );

    g_free ( bank );

    if ( bank_buffer == bank )
	bank_buffer = NULL;
}

/**
 * remove a bank, remove also the number in the accounts linked to that bank
 *
 * \param bank_number the bank we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_bank_remove ( gint bank_number )
{
    struct_bank *bank;
    GSList *list_tmp;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    bank_list = g_slist_remove ( bank_list,
				 bank );
    _gsb_data_bank_free ( bank );

    /* remove that bank of the accounts */
    list_tmp = gsb_data_account_get_list_accounts ();

    while (list_tmp)
    {
	gint account_number = gsb_data_account_get_no_account (list_tmp -> data);

	if ( gsb_data_account_get_bank (account_number) == bank_number )
	    gsb_data_account_set_bank ( account_number, 0 );
	list_tmp = list_tmp -> next;
    }

    return TRUE;
}


/**
 * set a new number for the bank
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param bank_number the number of the bank
 * \param new_no_bank the new number of the bank
 *
 * \return the new number or 0 if the bank doen't exist
 * */
gint gsb_data_bank_set_new_number ( gint bank_number,
				    gint new_no_bank )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, 0);
    bank -> bank_number = new_no_bank;
    return new_no_bank;
}


/**
 * Setters and getters are defined just after.
 *
 * Each setter takes a string as parameter. So the field of the structure is
 * first freed and the value is duplicated. It is freed when the bank is
 * destroyed (by _gsb_data_bank_free).
 * Setters return TRUE on success, FALSE otherwise
 *
 * Each getter returns a pointer on a string, which must not be freed.
 * Setters return the pointer on success, NULL otherwise
 */


/**
 * Getter for the bank_name
 */
const gchar *gsb_data_bank_get_name ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_name;
}
/**
 * Setter for the bank_name
 */
gboolean gsb_data_bank_set_name ( gint bank_number,
				  const gchar *name )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_name, name);
    return TRUE;
}


/**
 * Getter for the bank_code
 */
const gchar *gsb_data_bank_get_code ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_code;
}
/**
 * Setter for the bank_code
 */
gboolean gsb_data_bank_set_code ( gint bank_number,
				  const gchar *bank_code )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_code, bank_code);
    return TRUE;
}


/**
 * Getter for the bank_address
 */
const gchar *gsb_data_bank_get_bank_address ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_address;
}
/**
 * Setter for the bank_address
 */
gboolean gsb_data_bank_set_bank_address ( gint bank_number,
					  const gchar *bank_address )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_address, bank_address);
    return TRUE;
}


/**
 * Getter for the bank_tel
 */
const gchar *gsb_data_bank_get_bank_tel ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_tel;
}
/**
 * Setter for the bank_tel
 */
gboolean gsb_data_bank_set_bank_tel ( gint bank_number,
				      const gchar *bank_tel )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_tel, bank_tel);
    return TRUE;
}


/**
 * Getter for the bank_mail
 */
const gchar *gsb_data_bank_get_bank_mail ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_mail;
}
/**
 * Setter for the bank_mail
 */
gboolean gsb_data_bank_set_bank_mail ( gint bank_number,
				       const gchar *bank_mail )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_mail, bank_mail);
    return TRUE;
}


/**
 * Getter for the bank_web
 */
const gchar *gsb_data_bank_get_bank_web ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_web;
}
/**
 * Setter for the bank_web
 */
gboolean gsb_data_bank_set_bank_web ( gint bank_number,
				      const gchar *bank_web )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_web, bank_web);
    return TRUE;
}


/**
 * Getter for the bank_note
 */
const gchar *gsb_data_bank_get_bank_note ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_note;
}
/**
 * Setter for the bank_note
 */
gboolean gsb_data_bank_set_bank_note ( gint bank_number,
				       const gchar *bank_note )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_note, bank_note);
    return TRUE;
}


/**
 * Getter for the correspondent_name
 */
const gchar *gsb_data_bank_get_correspondent_name ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> correspondent_name;
}
/**
 * Setter for the correspondent_name
 */
gboolean gsb_data_bank_set_correspondent_name ( gint bank_number,
						const gchar *correspondent_name )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, correspondent_name, correspondent_name);
    return TRUE;
}


/**
 * Getter for the correspondent_tel
 */
const gchar *gsb_data_bank_get_correspondent_tel ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> correspondent_tel;
}
/**
 * Setter for the correspondent_tel
 */
gboolean gsb_data_bank_set_correspondent_tel ( gint bank_number,
					       const gchar *correspondent_tel )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, correspondent_tel, correspondent_tel);
    return TRUE;
}


/**
 * Getter for the correspondent_mail
 */
const gchar *gsb_data_bank_get_correspondent_mail ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> correspondent_mail;
}
/**
 * Setter for the correspondent_mail
 */
gboolean gsb_data_bank_set_correspondent_mail ( gint bank_number,
						const gchar *correspondent_mail )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, correspondent_mail, correspondent_mail);
    return TRUE;
}


/**
 * Getter for the correspondent_fax
 */
const gchar *gsb_data_bank_get_correspondent_fax ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> correspondent_fax;
}
/**
 * Setter for the correspondent_fax
 */
gboolean gsb_data_bank_set_correspondent_fax ( gint bank_number,
					       const gchar *correspondent_fax )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, correspondent_fax, correspondent_fax);
    return TRUE;
}


/**
 * Getter for BIC code
 */
const gchar *gsb_data_bank_get_bic ( gint bank_number )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, NULL);
    return bank -> bank_BIC;
}
/**
 * Setter for BIC code
 */
gboolean gsb_data_bank_set_bic ( gint bank_number, const gchar *bank_BIC )
{
    struct_bank *bank;
    BANK_GET_OR_RETURN(bank, bank_number, FALSE);
    BANK_SET_FIELD(bank, bank_BIC, bank_BIC);
    return TRUE;
}

