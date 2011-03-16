/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger  (cedric@grisbi.org)           */
/*          2005-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "qif.h"
#include "dialog.h"
#include "utils_files.h"
#include "gsb_data_account.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_transaction.h"
#include "gsb_file_util.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "import.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GDate *gsb_qif_get_date ( gchar *date_string, gint order );
static gchar **gsb_qif_get_date_content ( gchar *date_string );
static gint gsb_qif_get_date_order ( GSList *transactions_list );
static gchar *gsb_qif_get_account_name ( FILE *qif_file, const gchar *coding_system );
static gint gsb_qif_get_account_type ( gchar *header );
static gint gsb_qif_recupere_categories ( FILE *qif_file, const gchar *coding_system );
static gint gsb_qif_recupere_operations_from_account ( FILE *qif_file,
                        const gchar *coding_system,
                        struct struct_compte_importation *imported_account );
/*END_STATIC*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern GSList *liste_comptes_importes_error;
/*END_EXTERN*/

gchar *last_header = NULL;
gboolean mismatch_dates = TRUE;

enum
{
    ORDER_DD_MM_YY = 0,
    ORDER_MM_DD_YY,
    ORDER_YY_MM_DD,
    ORDER_YY_DD_MM,
    ORDER_DD_YY_MM,
    ORDER_MM_YY_DD,

    ORDER_MAX,
};


static gchar *order_names[] = {
    "day-month-year",
    "day-year-month",
    "year-month-day",
    "year-day-month",
    "month-day-year",
    "month-year-day" };


/**
 * \brief Import QIF data.
 *
 * Open a QIF file and fills in data in a struct_compte_importation
 * data structure.
 *
 * \param assistant	Not used.
 * \param imported	A pointer to structure containing name and
 *			format of imported file.
 *
 * \return		TRUE on success.
 */
gboolean recuperation_donnees_qif ( GtkWidget *assistant, struct imported_file *imported )
{
    gchar *tmp_str;
    struct struct_compte_importation *imported_account;
    gint returned_value = 0;
    gboolean premier_compte = TRUE;
    FILE *qif_file;

    qif_file = utf8_fopen ( imported -> name, "r" );
    if ( ! qif_file )
        return FALSE;

    mismatch_dates = TRUE;

    /* qif_file pointe sur le qif_file qui a été reconnu comme qif */
    rewind ( qif_file );

    imported_account = g_malloc0 ( sizeof ( struct struct_compte_importation ));
    imported_account -> nom_de_compte = unique_imported_name ( _("Invalid QIF file") );
    imported_account -> filename = my_strdup ( imported -> name );
    imported_account -> origine = my_strdup ( "QIF" );

    /* save filename */
    imported_account -> real_filename = my_strdup (imported -> name);

    /* It is positioned on the first line of file */
    returned_value = get_utf8_line_from_file ( qif_file, &tmp_str, imported -> coding_system );

    do
    {
        GSList *tmp_list;
        gchar *account_name = NULL;
        gint order = 0;
        gboolean name_preced = FALSE;
        struct struct_ope_importation *imported_transaction;

        do
        {
            if (  returned_value != EOF
             &&
             tmp_str
             &&
             g_ascii_strncasecmp ( tmp_str, "!Account", 8 ) == 0 )
            {
                /* create and fill the new account */
                imported_account = g_malloc0 ( sizeof ( struct struct_compte_importation ) );
                imported_account -> origine = my_strdup ( "QIF" );

                /* save filename and account_name */
                imported_account -> real_filename = my_strdup ( imported -> name );
                imported_account -> filename = my_strdup ( imported -> name );

                account_name = gsb_qif_get_account_name ( qif_file, imported -> coding_system );
                imported_account -> nom_de_compte = unique_imported_name ( account_name );
                g_free ( account_name );

                name_preced = TRUE;
                premier_compte = FALSE;
                returned_value = get_utf8_line_from_file ( qif_file, &tmp_str, imported -> coding_system );
            }
            else if ( returned_value != EOF
             &&
             tmp_str
             &&
             g_ascii_strncasecmp ( tmp_str, "!Type:Cat", 9 ) == 0 )
            {

                do
                {
                    returned_value = gsb_qif_recupere_categories ( qif_file,
                                        imported -> coding_system );

                    if ( returned_value == 0 )
                        tmp_str = last_header;
                }
                /* continue untill the end of the file or a change of account */
                while ( returned_value != EOF && returned_value != 0 );
            }
            else if ( returned_value != EOF
             &&
             tmp_str
             &&
             g_ascii_strncasecmp ( tmp_str, "!Type", 5 ) == 0 )
            {
                gint account_type;

                account_type = gsb_qif_get_account_type ( tmp_str ) ;
                if ( account_type == -1 )
                {
                    name_preced = FALSE;
                    continue;
                }
                else
                {
                    if ( name_preced == FALSE )
                    {
                        /* create and fill the new account */
                        imported_account = g_malloc0 ( sizeof ( struct struct_compte_importation ) );
                        imported_account -> origine = my_strdup ( "QIF" );

                        /* save filename and account_name */
                        imported_account -> real_filename = my_strdup ( imported -> name );
                        imported_account -> filename = my_strdup ( imported -> name );
                        imported_account -> nom_de_compte = unique_imported_name (
                                                                my_strdup ( _("Imported QIF account" ) ) );

                        premier_compte = FALSE;
                    }

                    if ( account_type == 6 )
                    {
                        /* on considère le imported_account d'investissement comme un imported_account 
                         * bancaire mais met un warning car pas implémenté, aucune idée si ça passe ou pas... */
                        gchar *msg;

                        msg = g_strdup_printf ( _("Grisbi found an investment account:\n%s\n"
                                        "which is not implemented yet.  Nevertheless, Grisbi will try "
                                        "to import it as a bank account." ),
                                        imported -> name );
                        dialogue_warning ( msg );
                        g_free ( msg );

                        account_type = 0;
                    }

                    imported_account -> type_de_compte = account_type;
                    returned_value = -2;
                }
            }
            else
                name_preced = FALSE;
        }
        while ( returned_value != EOF && returned_value != -2 );

        if ( returned_value == EOF )
        {
            if ( premier_compte )
            {
            /* no account already saved, so send an error */
                liste_comptes_importes_error = g_slist_append ( liste_comptes_importes_error,
                                        imported_account );
                fclose ( qif_file );
                return FALSE;
            }
            else
            {
                /* we have at least saved an account before, ok, enough for me */
                fclose ( qif_file );
                return TRUE;
            }
        }

        do
        {
            returned_value = gsb_qif_recupere_operations_from_account ( qif_file,
                                        imported -> coding_system,
                                        imported_account );

            if ( returned_value == 0 )
                tmp_str = last_header;
        }
        /* continue untill the end of the file or a change of account */
        while ( returned_value != EOF && returned_value != 0 );

        /* first, we need to check if the first transaction is an opening balance
         * or a normal transaction
         * update : money sometimes translate Opening balance... */
        if ( g_slist_length ( imported_account -> operations_importees) > 0 )
        {
            imported_transaction = imported_account -> operations_importees -> data;
            if ( imported_transaction -> tiers
             &&
             (!g_ascii_strncasecmp ( imported_transaction -> tiers, "Opening Balance", 15 )
             ||
             !g_ascii_strcasecmp ( imported_transaction -> tiers, _("Opening Balance") ) ) )
            {
                /* ok, we are on an opening balance, we transfer the first transaction
                 * to the initial datas of the account */

                /* get the initial amount */
                imported_account -> solde = imported_transaction -> montant;

                /* get the name of account */
                tmp_str = my_strdelimit (imported_transaction -> categ, "[]", "");
                if ( imported_account -> nom_de_compte )
                    g_free ( imported_account -> nom_de_compte );
                imported_account -> nom_de_compte = unique_imported_name ( tmp_str );
                g_free (tmp_str);

                /* get the date of the file */
                imported_account -> date_solde_qif = my_strdup ( imported_transaction -> date_tmp );

                /* now, we can remove the first imported transaction */
                imported_account -> operations_importees = g_slist_remove ( imported_account -> operations_importees,
                                            imported_transaction );
                g_free (imported_transaction);
            }
            /* now we need to transform the dates of transaction into gdate */

            /* try to understand the order */
            order = gsb_qif_get_date_order ( imported_account -> operations_importees );
            if (order == -1)
                dialogue_error ( _("Grisbi couldn't determine the format of the date into the qif file.\n"
                                        "Please contact the Grisbi team (devel@listes.grisbi.org) to find "
                                        "the problem.\nFor now, all the dates will be imported as 01.01.1970") );
        }

        tmp_list = imported_account -> operations_importees;
        while (tmp_list)
        {
            imported_transaction = tmp_list -> data;
            if (order == -1)
                /* we didn't find the order */
                imported_transaction -> date = g_date_new_dmy ( 1,1,2000 );
            else
                imported_transaction -> date = gsb_qif_get_date ( imported_transaction -> date_tmp, order );

            tmp_list = tmp_list -> next;
        }

        /* set the date of the qif file */
        if ( imported_account -> date_solde_qif )
            imported_account -> date_fin = gsb_qif_get_date (imported_account -> date_solde_qif, order);

        /* add that account to the others */
        liste_comptes_importes = g_slist_append ( liste_comptes_importes, imported_account );
    }
    /* go to the next account */
    while ( returned_value != EOF );

    fclose ( qif_file );

    return ( TRUE );
}


/**
 * this function try to understand in what order are the content of the date,
 * the two order known are d-m-y or y-m-d (hoping Money won't do something like m-y-d...)
 * the only way to know that is to check all the transactions imported and verify the first
 * order, if doesn't work, it's the second
 *
 * \param transactions_list	the list of imported transactions
 *
 * \return -1 for not found, or ORDER_... (see the enum at the begining of file)
 * */
static gint gsb_qif_get_date_order ( GSList *transactions_list )
{
    GSList *tmp_list;
    gint order = 0;
    gchar *date_wrong[ORDER_MAX];

    /* to find the good order of the content of the date, we check all possible orders
     * and check if all the dates are possible with that order. if one day 2 different order
     * can be good with that check, we should implement a second check : the transactions
     * are sorted normally, either in ascending, sometimes in descending order.
     * so check for the valids order if the transactions are sorted, normally, only one we be correct */

    tmp_list = transactions_list;
    while ( tmp_list )
    {
        struct struct_ope_importation *transaction = tmp_list -> data;
        gchar **array;
        gint year = 0, month = 0, day = 0;

        if ( !transaction -> date_tmp )
            continue;

        array = gsb_qif_get_date_content ( transaction -> date_tmp );

        /* if array still contains /, there is a problem (more than 2 / in the first entry) */
        if ( memchr ( array[2], '/', strlen ( array[2] ) ) )
        {
            gchar *string = g_strdup_printf ( _("The date %s seems contains more than 2 separators.\n"
                                            "This shouldn't happen. Please contact the Grisbi team to try to "
                                            "add your strange format into Grisbi"),
                                            transaction -> date_tmp );
            dialogue_error ( string );
            g_free ( string );
            return -1;
        }

        /* get the day, month and year according to the order */
        switch (order)
        {
            case ORDER_DD_MM_YY:
            day = atoi (array[0]);
            month = atoi (array[1]);
            year = atoi (array[2]);
            break;

            case ORDER_MM_DD_YY:
            day = atoi (array[1]);
            month = atoi (array[0]);
            year = atoi (array[2]);
            break;

            case ORDER_YY_MM_DD:
            day = atoi (array[2]);
            month = atoi (array[1]);
            year = atoi (array[0]);
            break;

            case ORDER_YY_DD_MM:
            day = atoi (array[1]);
            month = atoi (array[2]);
            year = atoi (array[0]);
            break;

            case ORDER_DD_YY_MM:
            day = atoi (array[0]);
            month = atoi (array[2]);
            year = atoi (array[1]);
            break;

            case ORDER_MM_YY_DD:
            day = atoi (array[2]);
            month = atoi (array[0]);
            year = atoi (array[1]);
            break;
        }

        /* the year can be yy or yyyy, we change that here */
        if ( year < 100 )
        {
            if ( year < 80 )
                year = year + 2000;
            else
                year = year + 1900;
        }

        if ( g_date_valid_dmy ( day, month, year ) )
            /* the date is valid, go to the next date */
            tmp_list = tmp_list -> next;
        else
        {
            /* the date is not valid, change the order or go away */
            date_wrong[order] = transaction -> date_tmp;
            order++;

            if ( order < ORDER_MAX )
                /* we try again with the new order */
                tmp_list = transactions_list;
            else
            {
                /* the order was already changed for all the formats, we show the problem and leave */
                gint i;
                gchar *string = my_strdup ( _("The order cannot be determined,\n") );

                for ( i = 0; i < ORDER_MAX; i++ )
                {
                    gchar *tmp_str;
                    tmp_str = g_strconcat ( string,_("Date wrong for the order "),
                                order_names[i], " : ",
                                date_wrong[i], "\n", NULL );
                    g_free (string);
                    string = tmp_str;
                }

                dialogue_error (string);
                g_free (string);
                g_strfreev (array);
                return -1;
            }
        }
        g_strfreev (array);
    }

    return order;
}


/**
 * get a string representing a date in qif format and return
 * a newly-allocated NULL-terminated array of strings.
 * * the order in the array is the same as in the string
 * 	known formats :
 * 		dd/mm/yyyy
 * 		dd/mm/yy
 * 		yyyy/mm/dd
 * 		dd/mm'yy
 * 		dd/mm'yyyy
 * 		dd-mm-yy
 *
 * \param date_string	a qif formatted (so randomed...) string
 *
 * \return a newly-allocated NULL-terminated array of 3 strings. Use g_strfreev() to free it.
 * */
static gchar **gsb_qif_get_date_content ( gchar *date_string )
{
    gchar *pointer;
    gchar **array;
    gint i;
    gchar *tmp_str;
    gint number_of_slash = 0;

    if ( !date_string )
        return NULL;

    date_string = my_strdup ( date_string );

    /* some software set a space in the format to annoy us... */
    tmp_str = my_strdelimit ( date_string, " ", "" );
    g_free ( date_string );
    date_string = tmp_str;

    /* as the format is risky, we will not check only / ' and -
     * we will remove all wich is not a number */
    pointer = date_string;
    for ( i = 0; i < strlen ( date_string ); i++ )
    {
        if ( !isalnum ( pointer[i] ) )
            pointer[i] = '/';
    }

    /* some qif have some text at the end of the date... i don't know what to do with
     * that, so i remove all the text after the 2nd / */
    pointer = date_string;
    for ( i = 0; i < strlen ( date_string ); i++ )
    {
        if ( !isalnum ( pointer [i] ) )
        {
            /* we are on a /, only 2 will survive */
            if ( number_of_slash < 2 )
                number_of_slash++;
            else
            /* sorry, end game for you*/
                pointer[i] = 0;
        }
    }

    array = g_strsplit (date_string, "/", 3);
    if ( mismatch_dates && strlen ( array[0] ) == 2 && strlen ( array[1] ) == 2 && strlen ( array[2] ) == 2 )
    {
        gchar *tmp_str;

        tmp_str = g_strdup ( _("Warning the date has three fields of two numbers. "
                        "In these circumstances the date might be wrong.") );

        dialogue_warning ( tmp_str );
        g_free ( tmp_str );
        mismatch_dates = FALSE;
    }

    g_free (date_string);

    return array;
}


/**
 * get the date from the qif formated string
 *
 * \param date_string	the string into the qif
 * \param order		the value retrieved by gsb_qif_get_date_order
 *
 * \return a newly allocated GDate
 * */
static GDate *gsb_qif_get_date ( gchar *date_string, gint order )
{
    gchar **array;
    GDate *date;
    gint year = 0, month = 0, day = 0;

    array = gsb_qif_get_date_content ( date_string );
    if ( !array )
        return NULL;

    /* get the day, month and year according to the order */
    switch (order)
    {
            case ORDER_DD_MM_YY:
            day = atoi (array[0]);
            month = atoi (array[1]);
            year = atoi (array[2]);
            break;

            case ORDER_MM_DD_YY:
            day = atoi (array[1]);
            month = atoi (array[0]);
            year = atoi (array[2]);
            break;

            case ORDER_YY_MM_DD:
            day = atoi (array[2]);
            month = atoi (array[1]);
            year = atoi (array[0]);
            break;

            case ORDER_YY_DD_MM:
            day = atoi (array[1]);
            month = atoi (array[2]);
            year = atoi (array[0]);
            break;

            case ORDER_DD_YY_MM:
            day = atoi (array[0]);
            month = atoi (array[2]);
            year = atoi (array[1]);
            break;

            case ORDER_MM_YY_DD:
            day = atoi (array[2]);
            month = atoi (array[0]);
            year = atoi (array[1]);
            break;
    }

    /* the year can be yy or yyyy, we change that here */
    if ( year < 100 )
    {
        if ( year < 80 )
            year = year + 2000;
        else
            year = year + 1900;
    }

    date = g_date_new_dmy ( day, month, year );

    g_strfreev ( array );

    if ( !date || !g_date_valid ( date ) )
        return NULL;
    else
        return date;
}


/**
 * export an archive given in param
 * it will create 1 file per account exported, containing
 * 	the transactions of the archive
 * 	the name exported will be "filename-name of the account.qif"
 *
 * \param filename a name without the extension
 * \param archive_number
 *
 * \return TRUE ok, FALSE pb
 * */
gboolean gsb_qif_export_archive ( const gchar *filename,
                        gint archive_number )
{
    GSList *tmp_list;
    GSList *name_list = NULL;
    gint error_return = 0;

    if (!gsb_file_util_test_overwrite (filename))
	return FALSE;

    /* the work (split between accounts) is already done with the archive_store,
     * so just use it */
    tmp_list = gsb_data_archive_store_get_archives_list ();
    while (tmp_list)
    {
	gint archive_store_number;

	archive_store_number = gsb_data_archive_store_get_number (tmp_list -> data);

	if (gsb_data_archive_store_get_archive_number (archive_store_number) == archive_number)
	{
	    gchar *new_filename;

	    new_filename = g_strconcat ( filename,
					 "-",
					 gsb_data_account_get_name (gsb_data_archive_store_get_account_number(archive_store_number)),
					 ".qif",
					 NULL );
	    if (qif_export ( new_filename,
			     gsb_data_archive_store_get_account_number(archive_store_number),
			     archive_number ))
		name_list = g_slist_append ( name_list,
					     new_filename );
	    else
		error_return = 1;
	}
	tmp_list = tmp_list -> next;
    }
    
    /* if there is more than 1 file created, we show what files we did */
    if (g_slist_length (name_list) > 1)
    {
	gchar *string;

	string = g_strdup_printf ( _("There were many accounts to export in the archive,\nbut the QIF format only support one file per account,\nso Grisbi created %d files, one per account :\n"),
				     g_slist_length (name_list));

	/* add the name to the string */
	tmp_list = name_list;
	while (tmp_list)
	{
	    gchar *new_string;

	    new_string = g_strconcat ( string,
				       "\n",
				       tmp_list -> data,
				       NULL );
	    g_free (string);
	    string = new_string;
	    tmp_list = tmp_list -> next;
	}
	dialogue (string);
	g_free (string);
    }

    /* free the names */
    tmp_list = name_list;
    while (tmp_list)
    {
	g_free (tmp_list -> data);
	tmp_list = tmp_list -> next;
    }
    g_slist_free (name_list);

    return !error_return;
}



/**
 * create a qif export, according to the filename, the account
 * and eventually, limit the export to the archive if exists
 * this will export all the transactions of the account (except if we ask for an archive...)
 * 	including the archived transactions
 *
 * \param filename that file will be checked and ask to overwrite if needed
 * \param account_nb
 * \param archive_number if 0, just export in qif the account transactions ; if non 0, export just the transactions for that archive and account
 *
 * \return TRUE ok, FALSE pb
 */
gboolean qif_export ( const gchar *filename,
                        gint account_nb,
                        gint archive_number )
{
    FILE * fichier_qif;
    GSList *list_tmp_transactions;
    gint beginning;
    gint floating_point;
	gchar* tmpstr;

    if (!gsb_file_util_test_overwrite (filename))
	return FALSE;

    if ( !( fichier_qif = utf8_fopen ( filename, "w" ) ))
    {
	dialogue_error_hint ( g_strerror(errno),
			      g_strdup_printf ( _("Error opening file '%s'"),
						filename ) );
	return FALSE;
    }

    /* get the floating point of the currency of the amount,
     * ie the number of digits after the . */
    floating_point = gsb_data_currency_get_floating_point (gsb_data_account_get_currency (account_nb));

    /* kind of account */
    if ( gsb_data_account_get_kind (account_nb) == GSB_TYPE_CASH )
	fprintf ( fichier_qif,
		  "!Type:Cash\n" );
    else
	if ( gsb_data_account_get_kind (account_nb) == GSB_TYPE_LIABILITIES
	     ||
	     gsb_data_account_get_kind (account_nb) == GSB_TYPE_ASSET )
	    fprintf ( fichier_qif,
		      "!Type:Oth L\n" );
	else
	    fprintf ( fichier_qif,
		      "!Type:Bank\n" );


    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    beginning = 1;

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_nb
	     &&
	     (!archive_number
	      ||
	      gsb_data_transaction_get_archive_number (transaction_number_tmp) == archive_number))
	{
	    if ( beginning )
	    {
		/* this is the beginning of the qif file, we set some beginnings things */
		fprintf ( fichier_qif,
			  "D%d/%d/%d\n",
			  g_date_get_day (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_get_month (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_get_year (gsb_data_transaction_get_date (transaction_number_tmp)));

		/* met le solde initial */
		tmpstr = gsb_real_get_string (gsb_data_account_get_init_balance (account_nb, -1));
		fprintf ( fichier_qif,
			  "T%s\n",
			  tmpstr);
		g_free ( tmpstr );

		fprintf ( fichier_qif,
			  "CX\nPOpening Balance\n" );

		/* met le nom du imported_account */

		fprintf ( fichier_qif,
			  "L%s\n^\n",
			  g_strconcat ( "[",
					gsb_data_account_get_name (account_nb),
					"]",
					NULL ) );
		beginning = 0;
	    }

	    /* si c'est une opé de ventil, on la saute pas elle sera recherchée quand */
	    /* son opé ventilée sera exportée */

	    if ( !gsb_data_transaction_get_mother_transaction_number ( transaction_number_tmp))
	    {
		/* met la date */

		fprintf ( fichier_qif,
			  "D%d/%d/%d\n",
			  g_date_get_day (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_get_month (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_get_year (gsb_data_transaction_get_date (transaction_number_tmp)));

		/* met le pointage */

		if ( gsb_data_transaction_get_marked_transaction ( transaction_number_tmp)== OPERATION_POINTEE
		     ||
		     gsb_data_transaction_get_marked_transaction ( transaction_number_tmp)== OPERATION_TELERAPPROCHEE )
		    fprintf ( fichier_qif,
			      "C*\n" );
		else
		    if ( gsb_data_transaction_get_marked_transaction ( transaction_number_tmp)== OPERATION_RAPPROCHEE )
			fprintf ( fichier_qif,
				  "CX\n" );


		/* met les notes */

		if ( gsb_data_transaction_get_notes ( transaction_number_tmp))
		    fprintf ( fichier_qif,
			      "M%s\n",
			      gsb_data_transaction_get_notes ( transaction_number_tmp));


		/* met le montant, transforme la devise si necessaire */
		tmpstr = gsb_real_get_string (gsb_data_transaction_get_adjusted_amount ( transaction_number_tmp, floating_point));
		fprintf ( fichier_qif,
			  "T%s\n",
			  tmpstr);
		g_free ( tmpstr );

		/* met le chèque si c'est un type à numérotation automatique */
		if ( gsb_data_payment_get_automatic_numbering (gsb_data_transaction_get_method_of_payment_number (transaction_number_tmp)))
		    fprintf ( fichier_qif,
			      "N%s\n",
			      gsb_data_transaction_get_method_of_payment_content ( transaction_number_tmp));

		/* met le tiers */

		fprintf ( fichier_qif,
			  "P%s\n",
			  gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number_tmp),
						    FALSE ));

		/*  on met soit un virement, soit une ventil, soit les catégories */

		/* si c'est une imported_splitted, on recherche toutes les opés de cette imported_splitted */
		/* et les met à la suite */
		/* la catégorie de l'opé sera celle de la première opé de imported_splitted */

		if ( gsb_data_transaction_get_split_of_transaction ( transaction_number_tmp))
		{
		    /* it's a split of transactions, look for the children and append them */

		    gint mother_transaction_category_written;
		    GSList *list_tmp_transactions_2;

		    mother_transaction_category_written = 0;
		    list_tmp_transactions_2 = gsb_data_transaction_get_transactions_list ();

		    while ( list_tmp_transactions_2 )
		    {
			gint transaction_number_tmp_2;
			transaction_number_tmp_2 = gsb_data_transaction_get_transaction_number (list_tmp_transactions_2 -> data);

			if (gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp_2) == transaction_number_tmp)
			{
			    /* we are on a child, for the first one, we set the mother category */
			    /*  the child can only be a normal category or a transfer */

			    if ( gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp_2) > 0)
			    {
				/* the child is a transfer */

				if ( !mother_transaction_category_written )
				{
				    fprintf ( fichier_qif,
					      "L%s\n",
					      g_strconcat ( "[",
							    gsb_data_account_get_name (gsb_data_transaction_get_contra_transaction_account (transaction_number_tmp_2)),
							    "]",
							    NULL ));
				    mother_transaction_category_written = 1;
				}
				fprintf ( fichier_qif,
					  "S%s\n",
					  g_strconcat ( "[",
							gsb_data_account_get_name (gsb_data_transaction_get_contra_transaction_account ( transaction_number_tmp_2)),
							"]",
							NULL ));
			    }
			    else
			    {
				/* it's a category : sub-category */

				if ( !mother_transaction_category_written )
				{
				    fprintf ( fichier_qif,
					      "L%s\n",
					      gsb_data_category_get_name (gsb_data_transaction_get_category_number (transaction_number_tmp_2),
									  gsb_data_transaction_get_sub_category_number (transaction_number_tmp_2),
									  _("No category defined")));
				    mother_transaction_category_written = 1;
				}
				fprintf ( fichier_qif,
					  "S%s\n",
					  gsb_data_category_get_name (gsb_data_transaction_get_category_number (transaction_number_tmp_2),
								      gsb_data_transaction_get_sub_category_number (transaction_number_tmp_2),
								      _("No category defined")));
			    }

			    /* set the notes of the split child */

			    if ( gsb_data_transaction_get_notes (transaction_number_tmp_2))
				fprintf ( fichier_qif,
					  "E%s\n",
					  gsb_data_transaction_get_notes (transaction_number_tmp_2));

			    /* set the amount of the split child */

			    tmpstr = gsb_real_get_string (gsb_data_transaction_get_adjusted_amount (transaction_number_tmp_2, floating_point));
			    fprintf ( fichier_qif,
				      "$%s\n",
				      tmpstr);
			    g_free ( tmpstr );
			}
			list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
		    }
		}
		else
		{
		    /* if it's a transfer, the contra-account must exist, else we do
		     * as for a normal category */

		    if ( gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) > 0 )
		    {
			/* it's a transfer */

			fprintf ( fichier_qif,
				  "L%s\n",
				  g_strconcat ( "[",
						gsb_data_account_get_name (gsb_data_transaction_get_contra_transaction_account ( transaction_number_tmp)),
						"]",
						NULL ));
		    }
		    else
		    {
			/* it's a normal category */

			fprintf ( fichier_qif,
				  "L%s\n",
				  gsb_data_category_get_name (gsb_data_transaction_get_category_number (transaction_number_tmp),
							      gsb_data_transaction_get_sub_category_number (transaction_number_tmp),
							      FALSE ));
		    }
		}
		fprintf ( fichier_qif,
			  "^\n" );
	    }
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }

    if ( beginning )
    {
	/* there is no transaction in the account, so do the opening of the account, bug no date */
	/* met le solde initial */

	gchar* tmpstr = gsb_real_get_string (gsb_data_account_get_init_balance (account_nb, -1));
	fprintf ( fichier_qif,
		  "T%s\n",
		  tmpstr);
	g_free ( tmpstr );

	fprintf ( fichier_qif,
		  "CX\nPOpening Balance\n" );

	/* met le nom du imported_account */

	fprintf ( fichier_qif,
		  "L%s\n^\n",
		  g_strconcat ( "[",
				gsb_data_account_get_name (account_nb),
				"]",
				NULL ) );
    }
    fclose ( fichier_qif );
    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gchar *gsb_qif_get_account_name ( FILE *qif_file, const gchar *coding_system )
{
    gchar *tmp_str;
    gchar *name = NULL;
    gint returned_value = 0;

    do
    {
        returned_value = get_utf8_line_from_file ( qif_file, &tmp_str, coding_system );

        if ( tmp_str[0] == 'N' )
            name = my_strdup ( tmp_str + 1 );
    }
    while ( tmp_str[0] != '^' && returned_value != EOF && tmp_str[0] != '!' );

    return name;
}


/**
 * retourne le type de compte en fonction du header du QIF file
 *
 *
 * \return -1 si un type non traité else 0 to 7
 * */
gint gsb_qif_get_account_type ( gchar *header )
{
    gint account_type = -1;
    gchar *ptr;

    ptr = g_utf8_strchr ( header, -1, ':' );
    ptr++;

    if ( g_ascii_strncasecmp ( ptr, "bank", 4 ) == 0 || my_strcasecmp ( ptr, _("bank") ) == 0 )
        account_type = 0;
    else if ( g_ascii_strncasecmp ( ptr, "invst", 5 ) == 0  || my_strcasecmp ( ptr, _("invst)") ) == 0 )
        account_type = 6;
    else if ( g_ascii_strncasecmp ( ptr, "cash", 4 ) == 0  || my_strcasecmp ( ptr, _("cash)") ) == 0 )
        account_type = 7;
    else if ( g_ascii_strncasecmp ( ptr, "oth a", 5 ) == 0  || my_strcasecmp ( ptr, _("oth a)") ) == 0 )
        account_type = 2;
    else if ( g_ascii_strncasecmp ( ptr, "oth l", 5 ) == 0  || my_strcasecmp ( ptr, _("oth l)") ) == 0 )
        account_type = 3;
    else if ( g_ascii_strncasecmp ( ptr, "ccard", 5 ) == 0  || my_strcasecmp ( ptr, _("ccard)") ) == 0 )
        account_type = 5;
    else
        account_type = -1;

    return account_type;
}


/**
 *
 *
 *
 *
 * */
gint gsb_qif_recupere_operations_from_account ( FILE *qif_file,
                        const gchar *coding_system,
                        struct struct_compte_importation *imported_account )
{
    gchar *string;
    gint returned_value;
    struct struct_ope_importation *imported_splitted = NULL;
    struct struct_ope_importation *imported_transaction;

    imported_transaction = g_malloc0 ( sizeof ( struct struct_ope_importation ) );
    do
    {
        returned_value = get_utf8_line_from_file ( qif_file, &string, coding_system );

        /* a transaction never begin with ^ and ! */
        if ( strlen ( string )
         &&
         returned_value != EOF
         &&
         string[0] != '^'
         &&
         string[0] != '!' )
        {
            /* set the date into date_tmp */
            if ( string[0] == 'D' )
                imported_transaction -> date_tmp = my_strdup ( string + 1 );

            /* récupération du pointage */
            if ( string[0] == 'C' )
            {
                if ( string[1] == '*' )
                    imported_transaction -> p_r = OPERATION_POINTEE;
                else
                    imported_transaction -> p_r = OPERATION_RAPPROCHEE;
            }

            /* récupération de la note */
            if ( string[0] == 'M' )
            {
                imported_transaction -> notes = g_strstrip ( g_strdelimit ( string + 1, ";", '/' ) );
                if ( imported_transaction -> notes && strlen ( imported_transaction -> notes ) == 0 )
                    imported_transaction -> notes = NULL;
            }

            if ( string[0] == 'T' )
            {
                gchar *new_str;

                new_str = utils_str_localise_decimal_point_from_string ( string + 1 );
                imported_transaction -> montant = gsb_real_get_from_string ( new_str );

                g_free ( new_str );
            }

            /* récupération du chèque */
            if ( string[0] == 'N' )
                imported_transaction -> cheque = my_strdup ( string + 1 ); 

            /* récupération du tiers */
            if ( string[0] == 'P' )
                imported_transaction -> tiers = my_strdup ( string + 1 );

            /* récupération des catég */
            if ( string[0] == 'L' )
                imported_transaction -> categ = my_strdup ( string + 1 );

            /* get the splitted transaction */
            if ( string[0] == 'S' )
            {
                /* begin a splitted transaction, if we were on a transaction, we save it */
                if ( returned_value != EOF && imported_transaction && imported_transaction -> date_tmp )
                {
                    
                    if ( imported_splitted == NULL )
                    imported_account -> operations_importees = g_slist_append (
                                        imported_account -> operations_importees, imported_transaction );
                }
                else
                {
                    /* it's the end of file or the transaction is not valid, so the children are not valid too */
                    g_free ( imported_transaction );

                    if ( imported_splitted )
                        g_free ( imported_splitted );

                    imported_transaction = NULL;
                    imported_splitted = NULL;
                }

                /* if we were on a splitted transaction, we save it */
                if ( imported_splitted )
                    imported_account -> operations_importees = g_slist_append (
                                        imported_account -> operations_importees, imported_splitted );

                imported_splitted = g_malloc0 ( sizeof ( struct struct_ope_importation ) );

                if ( imported_transaction )
                {
                    imported_transaction -> operation_ventilee = 1;

                    /* get the data of the transaction */
                    imported_splitted -> date_tmp = my_strdup ( imported_transaction -> date_tmp );
                    imported_splitted -> tiers = imported_transaction -> tiers;
                    imported_splitted -> cheque = imported_transaction -> cheque;
                    imported_splitted -> p_r = imported_transaction -> p_r;
                    imported_splitted -> ope_de_ventilation = 1;
                }

                imported_splitted -> categ = my_strdup ( string + 1 );
            }

            /* récupération de la note de imported_splitted */
            if ( string[0] == 'E' && imported_splitted )
            {
                imported_splitted -> notes = g_strstrip ( g_strdelimit ( string + 1, ";", '/' ) );
                if ( imported_splitted -> notes && strlen ( imported_splitted -> notes ) == 0 )
                    imported_splitted -> notes = NULL;
            }

            /* récupération du montant de la imported_splitted */
            /* récupération du montant ( on doit virer la , que money met pour séparer les milliers ) */
            /* on ne vire la , que s'il y a un . */
            if ( string[0] == '$' && imported_splitted )
            {
                gchar *new_str;

                new_str = utils_str_localise_decimal_point_from_string ( string + 1 );
                imported_splitted -> montant = gsb_real_get_from_string ( new_str );

                g_free ( new_str );
            }
        }
    }
    while ( string[0] != '^' && returned_value != EOF && string[0] != '!' );

    /* either we are at the end of a transaction, either at the end of the file */
    /* sometimes we have ^ and EOF, so we need in that case to take the transaction */
    if ( strlen ( string )
     &&
     ( returned_value != EOF || ( string && string[0]=='^') )
     &&
     string[0] != '!' )
    {
        if ( imported_splitted )
        {
            imported_account -> operations_importees = g_slist_append (
                                        imported_account -> operations_importees, imported_splitted );
            imported_splitted = NULL;
        }
        else
        {
            if ( imported_transaction -> date_tmp == NULL
             || 
             strlen ( g_strstrip ( imported_transaction -> date_tmp ) ) == 0 )
            {
                /* l'opération n'a pas de date, c'est pas normal. pour éviter de la perdre, on va lui */
                /* donner la date 01/01/1970 et on ajoute au tiers [opération sans date] */

                imported_transaction -> date_tmp = my_strdup ( "01/01/2000" );
                if ( imported_transaction -> tiers )
                    imported_transaction -> tiers = g_strconcat ( imported_transaction -> tiers,
                                        _(" [Transaction imported without date]"),
                                        NULL );
                else
                    imported_transaction -> tiers = my_strdup ( _(" [Transaction imported without date]") );
            }
            
            imported_account -> operations_importees = g_slist_append (
                                        imported_account -> operations_importees, imported_transaction );
        }
    } 

    if ( string[0] == '!' )
    {
        if ( last_header && strlen ( last_header ) )
            g_free ( last_header );
        last_header = g_strdup ( string );
    }

    if ( returned_value != EOF  && string[0] != '!' )
        return 1;
    else if ( returned_value == EOF )
        return EOF;
    else
        return 0;
}


/**
 *
 *
 *
 *
 * */
gint gsb_qif_recupere_categories ( FILE *qif_file, const gchar *coding_system )
{
    gchar *string;
    gint returned_value;

    do
    {
        returned_value = get_utf8_line_from_file ( qif_file, &string, coding_system );

        /* a category never begin with ^ and ! */
        if ( strlen ( string )
         &&
         returned_value != EOF
         &&
         string[0] != '^'
         &&
         string[0] != '!' )
        {
            gint category_number;
            gint type_category = 1;
            gchar **tab_str = NULL;

            tab_str = g_strsplit ( string + 1, ":", 2 );
            g_free ( string );

            do
            {
                returned_value = get_utf8_line_from_file ( qif_file,
                            &string, coding_system );
                if ( strlen ( string )
                 &&
                 returned_value != EOF
                 &&
                 string[0] != '^'
                 &&
                 string[0] != '!' )
                {
                    if ( strcmp ( string, "I" ) == 0 )
                        type_category = 0;
                    g_free ( string );
                }
            }
            while ( string[0] != '^' && returned_value != EOF && string[0] != '!' );

            /* get the category and create it if doesn't exist */
            if (tab_str[0])
            {
                tab_str[0] = g_strstrip ( tab_str[0] );
                category_number = gsb_data_category_get_number_by_name ( tab_str[0],
                                    TRUE,
                                    type_category );

                if (tab_str[1])
                {
                    tab_str[1] = g_strstrip ( tab_str[1] );
            
                    gsb_data_category_get_sub_category_number_by_name (
                                category_number,
                                tab_str[1],
                                TRUE );
                }
            }
            
            g_strfreev(tab_str);
        }
    }
    while ( string[0] != '^' && returned_value != EOF && string[0] != '!' );

    if ( string[0] == '!' )
    {
        if ( last_header && strlen ( last_header ) )
            g_free ( last_header );
        last_header = g_strdup ( string );
    }

    if ( returned_value != EOF  && string[0] != '!' )
        return 1;
    else if ( returned_value == EOF )
        return EOF;
    else
        return 0;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
