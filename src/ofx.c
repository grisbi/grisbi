/* ce fichier de la gestion du format ofx */


/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "ofx.h"
/*#include <libofx/libofx.h>*/


/* *******************************************************************************/
gboolean recuperation_donnees_ofx ( gchar *nom_fichier )
{
	gchar *argv[1];

	argv[0] = nom_fichier;

	printf ( "début procédure\n" );
	/*printf ( "%d\n",
			ofx_proc_file ( 1,
			&nom_fichier ));*/
	printf ( "fin procédure\n" );



  return ( TRUE );
}
/* *******************************************************************************/



/* *******************************************************************************/
 /* int ofx_proc_status_cb(struct OfxStatusData data)
{
	printf ( "ofx_proc_status_cb\n" );
return 0;
}*/
/* *******************************************************************************/
 





/* *******************************************************************************/
 /* int ofx_proc_security_cb(struct OfxSecurityData data)
{
	printf ( "ofx_proc_security_cb\n" );
return 0;
} */
/* *******************************************************************************/
 




/* *******************************************************************************/
 /* int ofx_proc_transaction_cb(struct OfxTransactionData data)
{
	printf ( "ofx_proc_transaction_cb\n" );
 return 0; 
 
}*/
/* *******************************************************************************/



/* *******************************************************************************/
/*int ofx_proc_statement_cb(struct OfxStatementData data)
{
	printf ( "ofx_proc_statement_cb\n" );
  return 0;
}*/
/* *******************************************************************************/




/* *******************************************************************************/

/*int ofx_proc_account_cb(struct OfxAccountData data)
{
	printf ( "ofx_proc_account_cb\n" );
 return 0;
}*/
/* *******************************************************************************/


