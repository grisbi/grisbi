/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                            */
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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */

/**
 * \file data_form.c
 * work with the form structure, no GUI here
 */


#include "config.h"

#include "include.h"
#include <string.h>

/*START_INCLUDE*/
#include "gsb_data_form.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_data_account.h"
#include "gsb_form.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/** \struct
 * describe the form organization
 * each account has its own form_organization
 **/

/* struct organisation_formulaire */
typedef struct _FormOrganization	FormOrganization;

struct _FormOrganization
 {
    /* 1 to 6 columns */
    gint columns;

    /* 1 to 4 rows */
    gint rows;

    /* form's filled by : */
    /* 	1: TRANSACTION_FORM_DATE (always set) */
    /* 	2: TRANSACTION_FORM_DEBIT (always set) */
    /* 	3: TRANSACTION_FORM_CREDIT (always set) */
    /* 	4: TRANSACTION_FORM_VALUE_DATE */
    /* 	5: TRANSACTION_FORM_EXERCICE */
    /* 	6: TRANSACTION_FORM_PARTY */
    /* 	7: TRANSACTION_FORM_CATEGORY */
    /* 	8: TRANSACTION_FORM_FREE */
    /* 	9: TRANSACTION_FORM_BUDGET */
    /* 	10: TRANSACTION_FORM_NOTES */
    /* 	11: TRANSACTION_FORM_TYPE */
    /* 	12: TRANSACTION_FORM_CHEQUE */
    /* 	13: TRANSACTION_FORM_DEVISE */
    /* 	14: TRANSACTION_FORM_CHANGE */
    /* 	15: TRANSACTION_FORM_VOUCHER */
    /* 	16: TRANSACTION_FORM_BANK */
    /* 	17: TRANSACTION_FORM_CONTRA */
    /*  18: TRANSACTION_FORM_OP_NB */
    /*  19: TRANSACTION_FORM_MODE */
    /*  20: TRANSACTION_FORM_WIDGET_NB */

    gint form_table[MAX_HEIGHT][MAX_WIDTH];
};


/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * create a new form organization
 * the form struct is set to 0 and has to be filled
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_data_form_new_organization (void)
{
    FormOrganization *new_form;

    new_form = g_malloc0 (sizeof (FormOrganization));

    if (!new_form)
    {
		dialogue_error_memory ();
		return FALSE;
    }

    grisbi_win_set_form_organization (new_form);

	return FALSE;
}

/**
 * create a default new form organization
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_data_form_set_default_organization (void)
{
    FormOrganization *form;
    gint tab[MAX_HEIGHT][MAX_WIDTH] = {
	{ TRANSACTION_FORM_DATE, TRANSACTION_FORM_PARTY, TRANSACTION_FORM_DEBIT, TRANSACTION_FORM_CREDIT, 0, 0 },
	{ 0, TRANSACTION_FORM_CATEGORY, TRANSACTION_FORM_TYPE, TRANSACTION_FORM_CHEQUE, 0, 0 },
	{ 0, TRANSACTION_FORM_NOTES, TRANSACTION_FORM_CONTRA, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0 }
    };
    gint i, j;

	form = grisbi_win_get_form_organization ();

    if (!form)
		return FALSE;

    /* by default, 4 columns and 3 rows */
    form->columns = 4;
    form->rows = 3;

    for (i = 0 ; i<MAX_HEIGHT ; i++)
	{
		for (j = 0 ; j<MAX_WIDTH ; j++)
		{
			form->form_table[i][j] = tab[i][j];
		}
	}

    return FALSE;
}

/**
 * get the number of columns of the form
 *
 * \param
 *
 * \return the number of columns or 0 if problem
 **/
gint gsb_data_form_get_nb_columns (void)
{
    FormOrganization *form;

    form = grisbi_win_get_form_organization ();

    if (!form)
		return FALSE;

    return form->columns;
}

/**
 * set the number of columns of the form
 * if we decrease the number of culumns, the content
 * of the last column will be deleted
 *
 * \param columns
 *
 * \return FALSE if problem, TRUE if ok
 **/
gboolean gsb_data_form_set_nb_columns (gint columns)
{
    FormOrganization *form;

    form = grisbi_win_get_form_organization ();

    if (!form)
		return FALSE;

    form->columns = columns;

	return TRUE;
}

/**
 * get the number of rows of the form
 *
 * \param
 *
 * \return the number of rows or 0 if problem
 **/
gint gsb_data_form_get_nb_rows (void)
{
    FormOrganization *form;

    form = grisbi_win_get_form_organization ();

    if (!form)
		return FALSE;

    return form->rows;
}

/**
 * set the number of rows of the form
 *
 * \param rows
 *
 * \return FALSE if problem, TRUE if ok
 **/
gboolean gsb_data_form_set_nb_rows (gint rows)
{
    FormOrganization *form;

    form = grisbi_win_get_form_organization ();

    if (!form)
		return FALSE;

    form->rows = rows;

	return TRUE;
}

/**
 * get a value in the table of the form
 *
 * \param column
 * \param row
 *
 * \return the value, 0 if no value at this place or -1 if problem
 **/
gint gsb_data_form_get_value (gint column,
							  gint row)
{
    FormOrganization *form;

    form = grisbi_win_get_form_organization ();

    if (!form || column >= MAX_WIDTH || row >= MAX_HEIGHT)
		return -1;

    return form->form_table[row][column];
}

/**
 * set a value in the table of the form
 *
 * \param column
 * \param row
 * \param value
 *
 * \return TRUE if ok, FALSE if problem
 **/
gboolean gsb_data_form_set_value (gint column,
								  gint row,
								  gint value)
{
    FormOrganization *form;

	form = grisbi_win_get_form_organization ();

	if (!form || column >= MAX_WIDTH || row >= MAX_HEIGHT)
		return FALSE;

    form->form_table[row][column] = value;

	return TRUE;
}

/**
 * look for the value given in param in the form
 * if found, fill row and column
 * if not found, return FALSE
 *
 * \param value the searched value
 * \param *return_row a pointer to gint or NULL
 * \param *return_column a pointer to gint or NULL
 *
 * \return TRUE if value found, FALSE else
 **/
gboolean gsb_data_form_look_for_value (gint value,
									   gint *return_row,
									   gint *return_column)
{
    FormOrganization *form;
    gint row, column;

    form = grisbi_win_get_form_organization ();

    if (!form)
		return FALSE;

    for (row=0 ; row < form->rows ; row++)
	{
		for (column=0 ; column < form->columns ; column++)
		{
			if (form->form_table[row][column] == value)
			{
				if (return_row)
					*return_row = row;
				if (return_column)
					*return_column = column;
				return TRUE;
			}
		}
	}
    return FALSE;
}

/**
 * check if the given value exists in the form of the CURRENT account
 *
 * \param value
 *
 * \return FALSE or TRUE
 **/
gboolean gsb_data_form_check_for_value (gint value)
{
    return gsb_data_form_look_for_value (value, NULL, NULL);
}


/**
 * get the number of values in the table of the form
 *
 * \param account_number
 *
 * \return the number of values or -1 if problem
 **/
gint gsb_data_form_get_values_total (void)
{
    FormOrganization *form;
    gint row, column;
    gint values = 0;

    form = grisbi_win_get_form_organization ();

    if (!form)
		return -1;

    for (row=0 ; row<MAX_HEIGHT ; row++)
	for (column=0 ; column<MAX_WIDTH ; column++)
	    if (form->form_table[row][column])
		values++;

    return values;
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
