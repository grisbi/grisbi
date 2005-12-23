/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org) 	      */
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
 * \file gsb_form.c
 * all that you need for the form is here !!!
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_form.h"
#include "erreur.h"
#include "include.h"
#include "gsb_data_form.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_fom_fill_transaction ( gint transaction_number );
static gboolean gsb_form_hide ( void );
static gboolean gsb_form_show ( void );
static gboolean gsb_form_valid ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/**
 * show the form, detect automatickly what we need to show, even for transactions,
 * scheduled_transactions and the buttons valid/cancel
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_show ( void )
{


    return FALSE;
}


/** 
 * hide the form, for now destroy it
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_hide ( void )
{


    return FALSE;
}

/**
 * check the values in the form and valid them
 *
 * \param
 *
 * \return TRUE if the form is ok, FALSE else
 * */
gboolean gsb_form_valid ( void )
{

    return TRUE;
}


/**
 * get the values in the form and fill the given transaction
 *
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_fom_fill_transaction ( gint transaction_number )
{


    return FALSE;
}



/**
 * get an element number and return its name
 *
 * \param element_number
 *
 * \return the name of the element
 * */
gchar *gsb_form_get_element_name ( gint element_number )
{
    switch ( element_number )
    {
	case -1:
	    /* that value shouldn't be there, it shows that a gsb_data_form_... returns
	     * an error value */
	    warning_debug ( "gsb_form_get_element_name : a value in the form is -1 wich should not happen.\nA gsb_data_form_... function must have returned an error value..." );
	    return ("");
	    break;

	case TRANSACTION_FORM_DATE:
	    return (N_("Date"));
	    break;

	case TRANSACTION_FORM_DEBIT:
	    return (N_("Debit"));
	    break;

	case TRANSACTION_FORM_CREDIT:
	    return (N_("Credit"));
	    break;

	case TRANSACTION_FORM_VALUE_DATE:
	    return (N_("Value date"));
	    break;

	case TRANSACTION_FORM_EXERCICE:
	    return (N_("Financial year"));
	    break;

	case TRANSACTION_FORM_PARTY:
	    return (N_("Payee"));
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    return (N_("Categories"));
	    break;

	case TRANSACTION_FORM_FREE:
	    return (N_("Free"));
	    break;

	case TRANSACTION_FORM_BUDGET:
	    return (N_("Budgetary line"));
	    break;

	case TRANSACTION_FORM_NOTES:
	    return (N_("Notes"));
	    break;

	case TRANSACTION_FORM_TYPE:
	    return (N_("Method of payment"));
	    break;

	case TRANSACTION_FORM_CHEQUE:
	    return (N_("Cheque/Transfer number"));
	    break;

	case TRANSACTION_FORM_DEVISE:
	    return (N_("Currency"));
	    break;

	case TRANSACTION_FORM_CHANGE:
	    return (N_("Change"));
	    break;

	case TRANSACTION_FORM_VOUCHER:
	    return (N_("Voucher"));
	    break;

	case TRANSACTION_FORM_BANK:
	    return (N_("Bank references"));
	    break;

	case TRANSACTION_FORM_CONTRA:
	    return (N_("Contra-transaction method of payment"));
	    break;

	case TRANSACTION_FORM_OP_NB:
	    return (N_("Transaction number"));
	    break;

	case TRANSACTION_FORM_MODE:
	    return (N_("Automatic/Manual"));
	    break;
    }
    return NULL;
}


