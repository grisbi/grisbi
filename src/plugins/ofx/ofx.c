/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2006-2006 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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
#include <config.h>
#endif

#include "include.h"

#include <libofx/libofx.h>

/*START_INCLUDE*/
#include "ofx.h"
#include "dialog.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
static int ofx_proc_account_cb(struct OfxAccountData data, void * account_data);;
static int ofx_proc_security_cb(struct OfxSecurityData data);
static int ofx_proc_statement_cb(struct OfxStatementData data, void * statement_data);;
static int ofx_proc_status_cb(struct OfxStatusData data, void * status_data);;
static int ofx_proc_transaction_cb(struct OfxTransactionData data, void * security_data);;
static GSList * recuperation_donnees_ofx ( GtkWidget * assistant, struct imported_file * imported );
/*END_STATIC*/


static struct import_format ofx_format =
{
    "OFX",
    "Open Financial Exchange",
    "ofx",
    (import_function) recuperation_donnees_ofx,
};



#ifndef ENABLE_STATIC
/** Module name. */
G_MODULE_EXPORT const gchar plugin_name[] = "ofx";
#endif



/** Initialization function. */
G_MODULE_EXPORT extern void ofx_plugin_register ( void )
{
    devel_debug ("Initializating ofx plugin");
    register_import_format ( &ofx_format );
}



/** Main function of module. */
G_MODULE_EXPORT extern gpointer ofx_plugin_run ( GtkWidget * assistant,
				    struct imported_file * imported )
{
    return recuperation_donnees_ofx ( assistant, imported );
}



/* on doit mettre le compte en cours d'importation en global pour que
 * la libofx puisse le traiter de plus un fichier ofx peut intégrer
 * plusieurs comptes, donc on crée une liste... */
GSList *liste_comptes_importes_ofx;
struct struct_compte_importation *compte_ofx_importation_en_cours;
gint erreur_import_ofx;
gint  message_erreur_operation;
gchar * ofx_filename;


#ifdef OFX_0_7
LibofxContextPtr ofx_context;
int ofx_proc_status_cb(struct OfxStatusData data, void * status_data);
/* void ofx_proc_security_cb ( LibofxContextPtr, LibofxProcSecurityCallback, void * ); */
int ofx_proc_account_cb(struct OfxAccountData data, void * account_data);
int ofx_proc_transaction_cb(struct OfxTransactionData data, void * security_data);
int ofx_proc_statement_cb(struct OfxStatementData data, void * statement_data);
#endif /* OFX_0_7 */


/**
 *
 *
 *
 *
 */
GSList *recuperation_donnees_ofx ( GtkWidget * assistant, struct imported_file * imported )
{
    GSList *liste_tmp;
    gchar *argv[2] = { "", "" };

    liste_comptes_importes_ofx = NULL;
    compte_ofx_importation_en_cours = NULL;
    erreur_import_ofx = 0;
    message_erreur_operation = 0;
    ofx_filename = imported -> name;

    /* 	la lib ofx ne tient pas compte du 1er argument */
    argv[1] = imported -> name;
    devel_print_str ( imported -> name );

#ifdef OFX_0_7
    ofx_context = libofx_get_new_context();
    ofx_set_status_cb ( ofx_context, ofx_proc_status_cb, NULL );
    /*     ofx_set_security_cb ( ofx_context, sofx_proc_security_cb, NULL ); */
    ofx_set_account_cb ( ofx_context, ofx_proc_account_cb, NULL );
    ofx_set_transaction_cb ( ofx_context, ofx_proc_transaction_cb, NULL );
    ofx_set_statement_cb ( ofx_context, ofx_proc_statement_cb, NULL );
#endif /* OFX_0_7 */
 
#ifdef OFX_0_7
    libofx_proc_file ( ofx_context, ofx_filename, AUTODETECT );
#else /* OFX_0_7 */
    ofx_proc_file ( 2, argv );	/* FIXME: handle < 0.7 */
#endif /* OFX_0_7 */

    /*     le dernier compte n'a pas été ajouté à la liste */
    liste_comptes_importes_ofx = g_slist_append ( liste_comptes_importes_ofx,
						  compte_ofx_importation_en_cours );

    if ( !compte_ofx_importation_en_cours )
    {
        struct struct_compte_importation * account;

        account = g_malloc0 ( sizeof ( struct struct_compte_importation ));
        account -> nom_de_compte = unique_imported_name ( _("Invalid OFX file") );
        account -> filename = g_strdup ( ofx_filename );
        account -> real_filename = g_strdup (ofx_filename);
        account -> origine = "OFX";
        gsb_import_register_account_error ( account );
        devel_print_str ( account -> nom_de_compte );

        return ( FALSE );
    }

    liste_tmp = liste_comptes_importes_ofx;

    while ( liste_tmp )
    {
        if ( !erreur_import_ofx )
        {
            gsb_import_register_account ( liste_tmp -> data );
        }
        else
        {
            gsb_import_register_account_error ( liste_tmp -> data );
        }

	liste_tmp = liste_tmp -> next;
    }

    return ( liste_tmp );
}
/* *******************************************************************************/




#ifdef OFX_0_7
int ofx_proc_status_cb(struct OfxStatusData data, void * status_data)
#else /* OFX_0_7 */
int ofx_proc_status_cb(struct OfxStatusData data)
#endif /* OFX_0_7 */
{
/*         printf ( "ofx_proc_status_cb:\n" ); */
/*         printf ( "ofx_element_name_valid %d\n", data . ofx_element_name_valid); */
/*         printf ( "ofx_element_name %s\n", data . ofx_element_name ); */
/*         printf ( "code_valid %d\n", data . code_valid ); */
/*         printf ( "code %d\n", data . code); */
/*         printf ( "name %s\n", data . name); */
/*         printf ( "description %s\n", data . description); */
/*         printf ( "severity_valid %d\n", data . severity_valid ); */
/*         printf ( "severity %d\n", data . severity); */
/*         printf ( "server_message_valid %d\n", data.server_message_valid ); */
/*         printf ( "server_message %s\n\n", data . server_message ); */

    /*     	on vérifie l'état, si c'est un warning, on l'affiche, si c'est une erreur, on vire */

    if ( data.severity_valid )
    {
	switch ( data.severity )
	{
	    case OFX_INFO :
		/* 		pas de pb, on fait rien */
		break;

	    case OFX_WARN :
		if ( data.code_valid )
		    dialogue_warning ( g_strconcat ( _("OFX processing returned following message:\n"),
						     data.name,
						     "\n",
						     data.description,
						     NULL ));
		else
		    dialogue_warning ( _("OFX processing ended in a warning message which is not valid."));
/* 		erreur_import_ofx = 1; */
		break;

	    case OFX_ERROR:
		if ( data.code_valid )
		    dialogue_error ( g_strconcat ( _("OFX processing returned following error message:\n"),
						   data.name,
						   "\n",
						   data.description,
						   NULL ));
		else
		    dialogue_error ( _("OFX processing returned an error message which is not valid."));
		erreur_import_ofx = 1;
		break;
	}		
    }


    return 0;
}
/* *******************************************************************************/






/* *******************************************************************************/
int ofx_proc_security_cb(struct OfxSecurityData data)
{
    dialog_message ( "ofx-security-not-implemented" );
    return 0;
}
/* *******************************************************************************/




/* *******************************************************************************/

#ifdef OFX_0_7
int ofx_proc_account_cb(struct OfxAccountData data, void * account_data)
#else /* OFX_0_7 */
int ofx_proc_account_cb(struct OfxAccountData data)
#endif /* OFX_0_7 */
{
/*         printf ( "ofx_proc_account_cb\n" ); */
/*         printf ( "account_id_valid %d\n", data.account_id_valid ); */
/*         printf ( "account_id %s\n", data.account_id ); */
/*         printf ( "account_name %s\n", data.account_name ); */
/*         printf ( "account_type_valid %d\n", data.account_type_valid ); */
/*         printf ( "account_type %d\n", data.account_type ); */
/*         printf ( "currency_valid %d\n", data.currency_valid ); */
/*         printf ( "currency %s\n", data. currency); */

    /*     si on revient ici et qu'un compte était en cours, c'est qu'il est fini et qu'on passe au compte */
    /* 	suivant... */

    if ( compte_ofx_importation_en_cours )
	liste_comptes_importes_ofx = g_slist_append ( liste_comptes_importes_ofx,
						      compte_ofx_importation_en_cours );

    compte_ofx_importation_en_cours = g_malloc0 ( sizeof ( struct struct_compte_importation ));

    if ( data.account_id_valid )
    {
	compte_ofx_importation_en_cours -> id_compte = latin2utf8 ( data.account_id );
	compte_ofx_importation_en_cours -> nom_de_compte = unique_imported_name ( latin2utf8 ( data.account_name ) );
	compte_ofx_importation_en_cours -> filename = ofx_filename;
    }

	compte_ofx_importation_en_cours -> real_filename = g_strdup (ofx_filename);
	compte_ofx_importation_en_cours -> origine = "OFX";

    if ( data.account_type_valid )
	compte_ofx_importation_en_cours -> type_de_compte = data.account_type;

    if ( data.currency_valid )
	compte_ofx_importation_en_cours -> devise = latin2utf8 ( data.currency );


    return 0;
}
/* *******************************************************************************/




/* *******************************************************************************/
#ifdef OFX_0_7
int ofx_proc_transaction_cb(struct OfxTransactionData data, void * security_data)
#else /* OFX_0_7 */
int ofx_proc_transaction_cb(struct OfxTransactionData data)
#endif /* OFX_0_7 */
{
    struct struct_ope_importation *ope_import;
    GDate *date;

/*         printf ( "ofx_proc_transaction_cb\n" ); */
/*         printf ( "account_id_valid : %d  \n", data.account_id_valid ); */
/*         printf ( "account_id : %s  \n", data.account_id ); */
/*         printf ( "transactiontype_valid : %d  \n", data.transactiontype_valid ); */
/*         printf ( "transactiontype : %d  \n", data.transactiontype ); */
/*         printf ( "invtransactiontype_valid : %d  \n", data.invtransactiontype_valid ); */
/*         printf ( "invtransactiontype : %d  \n", data.invtransactiontype ); */
/*         printf ( "units_valid : %d  \n", data. units_valid); */
/*         printf ( "units : %f  \n", data.units ); */
/*         printf ( "unitprice_valid : %d  \n", data.unitprice_valid ); */
/*         printf ( "unitprice : %f  \n", data.unitprice ); */
/*         printf ( "amount_valid : %d  \n", data.amount_valid ); */
/*         printf ( "amount : %f  \n", data.amount ); */
/*         printf ( "fi_id_valid : %d  \n", data.fi_id_valid ); */
/*         printf ( "fi_id : %s  \n", data.fi_id ); */
/*         printf ( "unique_id_valid : %d  \n", data.unique_id_valid ); */
/*         printf ( "unique_id : %s  \n", data.unique_id ); */
/*         printf ( "unique_id_type_valid : %d  \n", data.unique_id_type_valid ); */
/*         printf ( "unique_id_type : %s  \n", data.unique_id_type ); */
/*         printf ( "security_data_valid : %d  \n", data.security_data_valid ); */
/*         printf ( "security_data_ptr : %s  \n", data.security_data_ptr ); */
/*         printf ( "date_posted_valid : %d  \n", data.date_posted_valid ); */
/*         printf ( "date_posted : %s  \n", ctime ( &data.date_posted )); */
/*         printf ( "date_initiated_valid : %d  \n", data.date_initiated_valid ); */
/*         printf ( "date_initiated : %s  \n", ctime ( &data.date_initiated )); */
/*         printf ( "date_funds_available_valid : %d  \n", data.date_funds_available_valid ); */
/*         printf ( "date_funds_available : %s  \n", ctime ( &data.date_funds_available )); */
/*         printf ( "fi_id_corrected_valid : %d  \n", data.fi_id_corrected_valid ); */
/*         printf ( "fi_id_corrected : %s  \n", data.fi_id_corrected ); */
/*         printf ( "fi_id_correction_action_valid : %d  \n", data.fi_id_correction_action_valid ); */
/*         printf ( "fi_id_correction_action : %d  \n", data.fi_id_correction_action ); */
/*         printf ( "server_transaction_id_valid : %d  \n", data.server_transaction_id_valid ); */
/*         printf ( "server_transaction_id : %s  \n", data.server_transaction_id ); */
/*         printf ( "check_number_valid : %d  \n", data.check_number_valid ); */
/*         printf ( "check_number : %s  \n", data.check_number ); */
/*         printf ( "reference_number_valid : %d  \n", data.reference_number_valid ); */
/*         printf ( "reference_number : %s  \n", data.reference_number ); */
/*         printf ( "standard_industrial_code_valid : %d  \n", data.standard_industrial_code_valid ); */
/*         printf ( "standard_industrial_code : %s  \n", data.standard_industrial_code ); */
/*         printf ( "payee_id_valid : %d  \n", data.payee_id_valid ); */
/*         printf ( "payee_id : %s  \n", data.payee_id ); */
/*         printf ( "name_valid : %d  \n", data.name_valid ); */
/*         printf ( "name : %s  \n", data.name ); */
/*         printf ( "memo_valid : %d  \n", data.memo_valid ); */
/*         printf ( "memo : %s  \n\n\n\n", data.memo ); */

    /* si à ce niveau le comtpe n'est pas créé, c'est qu'il y a un pb... */

    if ( !compte_ofx_importation_en_cours )
    {
	if ( !message_erreur_operation )
	{
	    dialogue_error ( _("A transaction try to be saved but no account was created...\n"));
	    message_erreur_operation = 1;
	    erreur_import_ofx = 1;
	}
	return 0;
    }

    /*     c'est parti, on crée et remplit l'opération */

    ope_import = g_malloc0 ( sizeof ( struct struct_ope_importation ));

    if ( data.fi_id_valid )
	ope_import -> id_operation = latin2utf8 ( data.fi_id );

    date = g_date_new ();
    if ( data.date_posted_valid )
    {
	g_date_set_time_t ( date,
			  data.date_posted );
	if ( g_date_valid ( date ))
	    ope_import -> date_de_valeur = date;
	else
	    ope_import -> date_de_valeur = NULL;
    }

    if ( data.date_initiated_valid )
    {
	g_date_set_time_t ( date,
			  data.date_initiated );
	if ( g_date_valid ( date ))
	    ope_import -> date = date;
	else
	    ope_import -> date = ope_import -> date_de_valeur;
    }
    else
	ope_import -> date = ope_import -> date_de_valeur;

    if ( data.name_valid )
	ope_import -> tiers = latin2utf8 ( data.name );

    if ( data.memo_valid )
	ope_import -> notes = latin2utf8 ( data.memo );

    if ( data.check_number_valid )
	ope_import -> cheque = latin2utf8 ( data.check_number );

    if ( data.amount_valid )
	ope_import -> montant = gsb_real_double_to_real (data.amount);

    if ( data.transactiontype_valid )
	ope_import -> type_de_transaction = data.transactiontype;

    /*     on peut faire ici des ptites modifs en fonction du type de transaction, */
    /*     mais tout n'est pas utilisé par les banques... */

    if ( data.transactiontype_valid )
    {
	switch ( data.transactiontype )
	{
	    case OFX_CHECK:
		/* 		   si c'est un chèque, svt ya pas de tiers, on va mettre chèque...  */
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Check"));
		break;
	    case OFX_INT:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Interest"));
		break;
	    case OFX_DIV:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Dividend"));
		break;
	    case OFX_SRVCHG:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Service charge"));
		break;
	    case OFX_FEE:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Fee"));
		break;
	    case OFX_DEP:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Deposit"));
		break;
	    case OFX_ATM:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Cash dispenser"));
		break;
	    case OFX_POS:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Point of sale"));
		break;
	    case OFX_XFER:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Transfer"));
		break;
	    case OFX_PAYMENT:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Electronic payment"));
		break;
	    case OFX_CASH:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Cash"));
		break;
	    case OFX_DIRECTDEP:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Direct deposit"));
		break;
	    case OFX_DIRECTDEBIT:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Merchant initiated debit"));
		break;
	    case OFX_REPEATPMT:
		if ( !ope_import -> tiers )
		    ope_import -> tiers = g_strdup ( _("Repeating payment/standing order"));
		break;

	    case OFX_DEBIT:
	    case OFX_CREDIT:
	    case OFX_OTHER:
		break;
	}
    }
    /*     on ajoute l'opé à son compte */

    compte_ofx_importation_en_cours -> operations_importees = g_slist_append ( compte_ofx_importation_en_cours -> operations_importees,
									       ope_import );


    return 0; 
}
/* *******************************************************************************/



/* *******************************************************************************/
#ifdef OFX_0_7
int ofx_proc_statement_cb(struct OfxStatementData data, void * statement_data)
#else /* OFX_0_7 */
int ofx_proc_statement_cb(struct OfxStatementData data)
#endif /* OFX_0_7 */
{
    GDate *date;

/*         printf ( "ofx_proc_statement_cb\n" ); */
/*         printf ( "currency_valid : %d\n", data.currency_valid ); */
/*         printf ( "currency : %s\n", data.currency ); */
/*         printf ( "account_id_valid : %d\n", data.account_id_valid ); */
/*         printf ( "account_id : %s\n", data.account_id ); */
/*         printf ( "ledger_balance_valid : %d\n", data.ledger_balance_valid ); */
/*         printf ( "ledger_balance : %f\n", data.ledger_balance ); */
/*         printf ( "ledger_balance_date_valid : %d\n", data.ledger_balance_date_valid ); */
/*         printf ( "ledger_balance_date : %s\n", ctime ( &data.ledger_balance_date)); */
/*         printf ( "available_balance_valid : %d\n", data.available_balance_valid ); */
/*         printf ( "available_balance : %f\n", data.available_balance ); */
/*         printf ( "available_balance_date_valid : %d\n", data.available_balance_date_valid ); */
/*         printf ( "available_balance_date : %s\n", ctime ( &data.available_balance_date )); */
/*         printf ( "date_start_valid : %d\n", data.date_start_valid ); */
/*         printf ( "date_start : %s\n", ctime ( &data.date_start )); */
/*         printf ( "date_end_valid : %d\n", data.date_end_valid ); */
/*         printf ( "date_end : %s\n", ctime ( &data.date_end )); */
/*         printf ( "marketing_info_valid : %d\n", data.marketing_info_valid ); */
/*         printf ( "marketing_info : %s\n", data.marketing_info ); */

    if ( data.date_start_valid )
    {
	date = g_date_new ();

	g_date_set_time_t ( date,
			  data.date_start );
	if ( g_date_valid ( date ))
	    compte_ofx_importation_en_cours -> date_depart = date;
    }

    if ( data.date_end_valid )
    {
	date = g_date_new ();

	g_date_set_time_t ( date,
			  data.date_end );
	if ( g_date_valid ( date ))
	    compte_ofx_importation_en_cours -> date_fin = date;
    }

    return 0;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
