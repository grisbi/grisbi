/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          https://www.grisbi.org                                            */
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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_data_bank.h"
#include "gsb_data_account.h"
#include "utils_str.h"
/*END_INCLUDE*/

/**
 * \struct
 * Describe a bank
 */
typedef struct _BankStruct		BankStruct;

struct _BankStruct
{
	/* WARNING : cannot set guint for bank_number because a bug before (and after ?)
	 * 0.6 sometimes set -1 to bank_number and big pb after that if we set guint here */
	gint 		bank_number;
	gchar *		bank_name;
	gchar *		bank_code;
	gchar *		bank_BIC;				/* BIC number */

	gchar *		bank_address;
	gchar *		bank_tel;
	gchar *		bank_mail;
	gchar *		bank_web;
	gchar *		bank_note;

	gchar *		correspondent_name;
	gchar *		correspondent_tel;
	gchar *		correspondent_mail;
	gchar *		correspondent_fax;
};

/*START_STATIC*/
/** contains the g_slist of BankStruct */
static GSList *bank_list = NULL;

/** a pointer to the last bank used (to increase the speed) */
static BankStruct *bank_buffer;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * This internal function is called to free the memory used by a BankStruct structure
 *
 * \param struct	bank
 *
 * \return
 **/
static void _gsb_data_bank_free (BankStruct* bank)
{
	if (!bank)
		return;

	/* free string fields */
	g_free (bank->bank_name);
	g_free (bank->bank_code);
	g_free (bank->bank_BIC);
	g_free (bank->bank_address);
	g_free (bank->bank_tel);
	g_free (bank->bank_mail);
	g_free (bank->bank_web);
	g_free (bank->bank_note);
	g_free (bank->correspondent_name);
	g_free (bank->correspondent_tel);
	g_free (bank->correspondent_mail);
	g_free (bank->correspondent_fax);

	g_free (bank);

	if (bank_buffer == bank)
		bank_buffer = NULL;
}

/**
 * find and return the structure of the bank asked
 *
 * \param bank_number number of bank
 *
 * \return the adr of the struct of the bank (NULL if doesn't exit)
 **/
static gpointer gsb_data_bank_get_structure (gint bank_number)
{
	GSList *tmp;

	if (!bank_number)
		return NULL;

	/* before checking all the banks, we check the buffer */
	if (bank_buffer && bank_buffer->bank_number == bank_number)
		return bank_buffer;

	tmp = bank_list;
	while (tmp)
	{
		BankStruct *bank;

		bank = tmp->data;
		if (bank->bank_number == bank_number)
		{
			bank_buffer = bank;

			return bank;
		}
		tmp = tmp->next;
	}

	return NULL;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * set the banks global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 **/
gboolean gsb_data_bank_init_variables (void)
{
	if (bank_list)
	{
		g_slist_free_full (bank_list, (GDestroyNotify) _gsb_data_bank_free);
		bank_list = NULL;
	}

	bank_buffer = NULL;

	return FALSE;
}

/**
 * return the number of the bank given in param
 *
 * \param bank_ptr a pointer to the struct of the bank
 *
 * \return the number of the bank, 0 if problem
 **/
gint gsb_data_bank_get_no_bank (gpointer bank_ptr)
{
	BankStruct *bank;

	if (!bank_ptr)
		return 0;

	bank = bank_ptr;
	bank_buffer = bank;

	return bank->bank_number;
}

/**
 * give the g_slist of bank structure
 *
 * \param none
 *
 * \return the g_slist of banks structure
 **/
GSList *gsb_data_bank_get_bank_list (void)
{
	return bank_list;
}

/**
 * find and return the last number of bank
 *
 * \param none
 *
 * \return last number of bank
 **/
gint gsb_data_bank_max_number (void)
{
	GSList *tmp;
	gint number_tmp = 0;

	tmp = bank_list;

	while (tmp)
	{
		BankStruct *bank;

		bank = tmp->data;

		if (bank->bank_number > number_tmp)
			number_tmp = bank->bank_number;

		tmp = tmp->next;
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
 **/
gint gsb_data_bank_new (const gchar *name)
{
	BankStruct *bank;

	bank = g_malloc0 (sizeof (BankStruct));
	bank->bank_number = gsb_data_bank_max_number () + 1;

	if (name)
		bank->bank_name = my_strdup (name);

	bank_list = g_slist_append (bank_list, bank);

	return bank->bank_number;
}

/**
 * remove a bank, remove also the number in the accounts linked to that bank
 *
 * \param bank_number the bank we want to remove
 *
 * \return TRUE ok
 **/
gboolean gsb_data_bank_remove (gint bank_number)
{
	GSList *list_tmp;
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);

	if (!bank)
		return FALSE;

	bank_list = g_slist_remove (bank_list, bank);
	_gsb_data_bank_free (bank);

	/* remove that bank of the accounts */
	list_tmp = gsb_data_account_get_list_accounts ();
	while (list_tmp)
	{
		gint account_number;

		account_number = gsb_data_account_get_no_account (list_tmp->data);
		if (gsb_data_account_get_bank (account_number) == bank_number)
			gsb_data_account_set_bank (account_number, 0);
		list_tmp = list_tmp->next;
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
gint gsb_data_bank_set_new_number (gint bank_number,
								   gint new_no_bank)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return 0;

	bank->bank_number = new_no_bank;

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
 **/

/**
 * Getter for the bank_name
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_name (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_name;
}

/**
 * Setter for the bank_name
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_name (gint bank_number,
								 const gchar *bank_name)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	g_free (bank->bank_name);
	bank->bank_name = my_strdup (bank_name);

	return TRUE;
}

/**
 * Getter for the bank_code
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_code (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_code;
}

/**
 * Setter for the bank_code
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_code (gint bank_number,
								 const gchar *bank_code)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	g_free (bank->bank_code);
	bank->bank_code = my_strdup (bank_code);

	return TRUE;
}

/**
 * Getter for the bank_address
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_bank_address (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_address;
}

/**
 * Setter for the bank_address
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_bank_address (gint bank_number,
										 const gchar *bank_address)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->bank_address)
		g_free (bank->bank_address);
	bank->bank_address = my_strdup (bank_address);

	return TRUE;
}

/**
 * Getter for the bank_tel
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_bank_tel (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_tel;
}

/**
 * Setter for the bank_tel
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_bank_tel (gint bank_number,
					  				 const gchar *bank_tel)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->bank_tel)
		g_free (bank->bank_tel);
	bank->bank_tel = my_strdup (bank_tel);

	return TRUE;
}

/**
 * Getter for the bank_mail
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_bank_mail (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_mail;
}

/**
 * Setter for the bank_mail
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_bank_mail (gint bank_number,
									  const gchar *bank_mail)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->bank_mail)
		g_free (bank->bank_mail);
	bank->bank_mail = my_strdup (bank_mail);

	return TRUE;
}

/**
 * Getter for the bank_web
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_bank_web (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_web;
}

/**
 * Setter for the bank_web
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_bank_web (gint bank_number,
									 const gchar *bank_web)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->bank_web)
		g_free (bank->bank_web);
	bank->bank_web = my_strdup (bank_web);

	return TRUE;
}

/**
 * Getter for the bank_note
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_bank_note (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_note;
}

/**
 * Setter for the bank_note
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_bank_note (gint bank_number,
									  const gchar *bank_note)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->bank_note)
		g_free (bank->bank_note);
	bank->bank_note = my_strdup (bank_note);

	return TRUE;
}

/**
 * Getter for the correspondent_name
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_correspondent_name (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->correspondent_name;
}

/**
 * Setter for the correspondent_name
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_correspondent_name (gint bank_number,
											   const gchar *correspondent_name)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->correspondent_name)
		g_free (bank->correspondent_name);
	bank->correspondent_name = my_strdup (correspondent_name);

	return TRUE;
}

/**
 * Getter for the correspondent_tel
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_correspondent_tel (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->correspondent_tel;
}

/**
 * Setter for the correspondent_tel
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_correspondent_tel (gint bank_number,
											  const gchar *correspondent_tel)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->correspondent_tel)
		g_free (bank->correspondent_tel);
	bank->correspondent_tel = my_strdup (correspondent_tel);

	return TRUE;
}

/**
 * Getter for the correspondent_mail
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_correspondent_mail (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->correspondent_mail;
}

/**
 * Setter for the correspondent_mail
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_correspondent_mail (gint bank_number,
											   const gchar *correspondent_mail)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->correspondent_mail)
		g_free (bank->correspondent_mail);
	bank->correspondent_mail = my_strdup (correspondent_mail);

	return TRUE;
}

/**
 * Getter for the correspondent_fax
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_correspondent_fax (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->correspondent_fax;
}

/**
 * Setter for the correspondent_fax
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_correspondent_fax (gint bank_number,
											  const gchar *correspondent_fax)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->correspondent_fax)
		g_free (bank->correspondent_fax);
	bank->correspondent_fax = my_strdup (correspondent_fax);

	return TRUE;
}

/**
 * Getter for BIC code
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_data_bank_get_bic (gint bank_number)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return NULL;

	return bank->bank_BIC;
}

/**
 * Setter for BIC code
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_bank_set_bic (gint bank_number,
								const gchar *bank_BIC)
{
	BankStruct *bank;

	bank = gsb_data_bank_get_structure (bank_number);
	if (!bank)
		return FALSE;

	if (bank->bank_BIC)
		g_free (bank->bank_BIC);
	bank->bank_BIC = my_strdup (bank_BIC);

	return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GSList *gsb_data_bank_get_bank_type_list (void)
{
	GSList *list = NULL;
	gchar *text [] = {_("Bank account"), _("Cash account"), _("Liabilities account"), _("Assets account"), NULL};
	gint i = 0;

	while (text[i])
	{
		list = g_slist_append (list, text[i]);
		list = g_slist_append (list, GINT_TO_POINTER (i));
		i++;
	}

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
