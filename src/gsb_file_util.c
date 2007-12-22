/* file gsb_file_util.c
 * used to save the gsb files */
/*     Copyright (C)	2000-2005 Cédric Auger (cedric@grisbi.org) */
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

#include <zlib.h>


/*START_INCLUDE*/
#include "gsb_file_util.h"
#include "./dialog.h"
#include "./gsb_data_account.h"
#include "./gsb_data_transaction.h"
#include "./utils_str.h"
#include "./utils_files.h"
#include "./include.h"
#include "./erreur.h"
#include "./structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *nom_fichier_comptes;
/*END_EXTERN*/


/**
 * check if the file exists, and if so, ask the user if he wants
 * to overwrite it
 * show an error message if necessary
 * return TRUE if the saving action can continue, FALSE to stop it
 *
 * \param filename
 *
 * \return TRUE ok, can continue (no file exists or user wants to overwrite), FALSE stop the action
 * */
gboolean gsb_file_util_test_overwrite ( const gchar *filename )
{
    if (!filename
	||
	!strlen (filename))
    {
	dialogue_error (_("No name to the file !"));
	return FALSE;
    }

    if (g_file_test ( filename,
		      G_FILE_TEST_EXISTS ))
    {
	/* the file exists */
	if (g_file_test (filename,
			 G_FILE_TEST_IS_DIR))
	{
	    gchar* tmpstr = g_strdup_printf ( _("%s is a directory...\nPlease choose another name."),
					       filename );
	    dialogue_error ( tmpstr );
	    g_free ( tmpstr );
	    return FALSE;
	}
	
	gchar* tmpstr = g_strdup_printf (_("Do you want to overwrite file \"%s\"?"), filename);
	gboolean response = question_yes_no_hint (_("File already exists"),
				       tmpstr,
				       GTK_RESPONSE_NO );
	g_free ( tmpstr );
        return response;
    }
    return TRUE;
}


/**
 * compress or uncompress  the string given in the param
 * if the compress is done, the parameter string is freed
 *
 * \param file_content a string which is the file
 * \param length length of the string
 * \param compress TRUE to compress, FALSE to uncompress
 *
 * \return the new length of the string
 * */
gint gsb_file_util_compress_file ( gchar **file_content,
				   gulong length,
				   gboolean compress )
{
    /* FIXME : for zlib, file_content need to be guchar, but g_file_get_contents need
     * a gchar... i don't know if it's a problem to hide that to gcc, but it
     * seems to work... perhaps there is a better way ?
     * so here a gchar comes in, but it's hidden with (guchar *) to zlib... */

    gchar* tmpstr = g_strdup_printf ("gsb_file_util_compress_file : %d", compress );
    devel_debug ( tmpstr );
    g_free ( tmpstr );

    if ( compress )
    {
	/* compress */
	gchar *temp;
	gulong new_length;
	gint result;
	gchar *str_length;
	gint iterator;

	new_length = compressBound (length);

	temp = g_malloc (new_length);

	result = compress2 ( (guchar *) temp, &new_length,
			     (guchar *) *file_content, length,
			     Z_BEST_COMPRESSION );
	
	if ( result != Z_OK )
	{
	    dialogue_error_hint ( ("An error occured while compressing the file : zlib error\nOperation aborted"),
				  _("File compression"));
	    free (*file_content);
	    free (temp);
	    return 0;
	}
	free (*file_content);

	str_length = utils_str_itoa (length);

	*file_content = g_malloc (23 + strlen (str_length) + 1 + new_length);
	
	memcpy ( *file_content, "Grisbi compressed file ", 23 );
	iterator = 23;
	memcpy ( *file_content + iterator, str_length, strlen (str_length));
	iterator = iterator + strlen (str_length);
	memcpy ( *file_content + iterator, "_", 1);
	iterator = iterator + 1;
	memcpy ( *file_content + iterator, temp, new_length);
	iterator = iterator + new_length;

	free (temp);

	return iterator;
    }
    else
    {
	/* decompress */

	gchar *temp;
	gint result;
	gulong new_length;
	gchar *zip_begining;
	gulong zip_size;

	/* get the length of the uncompressed file */

	if ( !sscanf ( *file_content,
		       "Grisbi compressed file %ld_",
		       &new_length ))
	{
	    dialogue_error_hint ( ("An error occured while uncompressing the file : cannot get the size\nOperation aborted"),
				  _("File uncompression"));
	    free (*file_content);
	    return 0;
	}

	zip_begining = memchr ( *file_content, '_', length );

	if (!zip_begining)
	{
	    dialogue_error_hint ( ("An error occured while uncompressing the file : cannot find the begining of the compressed file.\nOperation aborted"),
				  _("File uncompression"));
	    free (*file_content);
	    return 0;
	}

	zip_begining = zip_begining + 1;
	zip_size = length - (zip_begining - *file_content );

	temp = g_malloc ( new_length );
	result = uncompress ( (guchar *) temp, &new_length,
			      (guchar *) zip_begining, zip_size);

	if ( result != Z_OK )
	{
	    dialogue_error_hint ( ("An error occured while uncompressing the file : zlib error\nOperation aborted"),
				  _("File uncompression"));
	    free (*file_content);
	    free (temp);
	    return 0;
	}
	free (*file_content);
	*file_content = temp;
	return new_length;
    }

    /* normally souldn't come here */
    return length;
}



/**
 * for a grisbi file before 0.5.1, switch the R and T transactions because T appears
 * called only for a file before 0.5.1
 *
 * \param
 *
 * \return
 * */
void switch_t_r ( void )
{
    /* cette fonction fait le tour des opérations et change le marquage T et R des opés */
    /*     R devient pointe=3 */
    /*     T devient pointe=2 */
    /*     à n'appeler que pour une version antérieure à 0.5.1 */

    GSList *list_tmp_transactions;

    if ( !gsb_data_account_get_accounts_amount () )
	return;

    devel_debug ( "switch_t_r");


    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	switch ( gsb_data_transaction_get_marked_transaction (transaction_number_tmp))
	{
	    case 2 :
		gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
							      3 );
		break;
	    case 3:
		gsb_data_transaction_set_marked_transaction ( transaction_number_tmp,
							      2 );
		break;
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }
}



/**
 * create or delete a file ".name_of_file.swp" to check if the file is opened
 * already or not
 *
 * \param create_swp if we create or delete it
 *
 * \return TRUE if ok
 * */
gboolean gsb_file_util_modify_lock ( gboolean create_swp )
{
    gchar *lock_filename;
    gchar **tab_str;
    gint i;

    /* if the file was already opened and we don't force the saving, we do nothing */
    if ( (etat.fichier_deja_ouvert
	  &&
	  !etat.force_enregistrement)
	 ||
	 !nom_fichier_comptes
	 ||
	 !strlen(nom_fichier_comptes) )
	return TRUE;

    /* check if nom_fichier_comptes exists */
    if (!g_file_test ( nom_fichier_comptes,
		       G_FILE_TEST_EXISTS ))
    {
	gchar* tmpstr = g_strdup_printf (_("Cannot open file '%s' to mark it as used: %s"),
					 nom_fichier_comptes,
					 latin2utf8 (strerror(errno)));
	dialogue_error (tmpstr );
	g_free ( tmpstr );
	return FALSE;
    }


    /* create the name of the lock file */

    tab_str = g_strsplit ( nom_fichier_comptes,
			   G_DIR_SEPARATOR_S,
			   0 );

    i=0;

    while ( tab_str[i+1] )
	i++;

    tab_str[i] = g_strconcat ( 
#ifndef _WIN32
                              ".",
#endif
			       tab_str[i],
			       ".swp",
			       NULL );
    lock_filename = g_strjoinv ( G_DIR_SEPARATOR_S,
				 tab_str );
    g_strfreev ( tab_str );

    if ( create_swp )
    {
	/* now we create the lock file */

	FILE *fichier;

	/* check if the file lock exists */
	if (g_file_test ( lock_filename,
			  G_FILE_TEST_EXISTS ))
	{
	    if ( ! etat.force_enregistrement )
		dialog_message ( "account-already-opened", nom_fichier_comptes );

	    /* the lock is already created, return TRUE */
	    etat.fichier_deja_ouvert = 1;
	    return TRUE;
	}

	etat.fichier_deja_ouvert = 0;

	fichier = utf8_fopen ( lock_filename, "w" );

	if ( !fichier )
	{
	    gchar* tmpstr = g_strdup_printf (_("Cannot write lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno)));
	    dialogue_error ( tmpstr );
	    g_free ( tmpstr );
	    return FALSE;
	}

	fclose ( fichier );
	return TRUE;
    }
    else
    {
	/* delete the lock file */
	gint result;

	etat.fichier_deja_ouvert = 0;

	/* check if it exits, if not, just go away */
	if (!g_file_test ( lock_filename,
			   G_FILE_TEST_EXISTS ))
	    return TRUE;

	result = utf8_remove ( lock_filename );

	if ( result == -1 )
	{
	    gchar* tmpstr = g_strdup_printf (_("Cannot erase lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno)));
	    dialogue_error ( tmpstr );
	    g_free ( tmpstr );
	    return FALSE;
	}
	return TRUE;
    }
}


/**
 *
 * called when loading a file, if the permissions are not set only for the user
 * propose to change the permissions
 *
 * \param
 *
 * \return
 * */
void gsb_file_util_change_permissions ( void )
{
    /* On Windows, the chmod feature does not work: FAT does not
     * have right access permission notions , on NTFS it to
     * complicated to implement => the feature is removed from the
     * Windows version : for that the corresponding parameter
     * check box is not displayed and the paramater is forced to
     * not display msg. */
#ifndef _WIN32
    if ( question_conditional_yes_no ( "account-file-readable" ) == TRUE )
    {
	chmod ( nom_fichier_comptes, S_IRUSR | S_IWUSR );
    }

#endif /* _WIN32 */
}


