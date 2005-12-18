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

