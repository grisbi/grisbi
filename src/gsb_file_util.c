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

#include <rpc/des_crypt.h>
#include <zlib.h>


/*START_INCLUDE*/
#include "gsb_file_util.h"
#include "dialog.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "utils_str.h"
#include "utils_buttons.h"
#include "utils_files.h"
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gchar *gsb_file_util_ask_for_crypt_key ( gchar * file_name, gboolean encrypt );
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *nom_fichier_comptes;
extern GtkWidget *window;
/*END_EXTERN*/

/* filled when we ask to keep the password in memory */
gchar *crypt_key;

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

    if (DEBUG)
	printf ( "gsb_file_util_compress_file : %d\n",
		 compress );

    if ( compress )
    {
	/* compress */
	gchar *temp;
	gulong new_length;
	gint result;
	gchar *str_length;
	gint iterator;

	new_length = compressBound (length);

	temp = malloc (new_length);

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

	*file_content = malloc (23 + strlen (str_length) + 1 + new_length);
	
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

	temp = malloc ( new_length );
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
 * crypt if necessary the string given in the param
 * if the string is crypted, the parameter string is freed
 *
 * \param file_name	File name, used to 
 * \param file_content	A string which is the file
 * \param crypt		TRUE to crypt, FALSE to uncrypt
 * \param length	The length of the grisbi data,
 *                      without "Grisbi encrypted file " if comes to crypt
 *                      with "Grisbi encrypted file " if comes to decrypt 
 *
 * \return the length of the new file_content or 0 if problem
 */
gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
				gboolean crypt, gulong length )
{
    gint result;
    gchar *key;
    gulong position;

    if ( DEBUG )
	printf ( "gsb_file_util_crypt_file : %d\n", crypt );

    if ( crypt )
    {
	/* we want to encrypt the file */

	gchar *encrypted_file;

	/* check first if we want to encrypt that file */

	if ( !etat.crypt_file )
	    return length;

	/* now, if we know here a key to crypt, we use it, else, we ask for it */

	if ( crypt_key )
	    key = crypt_key;
	else
	    key = gsb_file_util_ask_for_crypt_key ( file_name, TRUE);

	/* if we have no key, we will no crypt that file */

	if ( !key )
	    return length;

	/* we create a copy of the file in memory which will begin by "Grisbi encrypted file " */

	encrypted_file = malloc ( (length + 22) * sizeof ( gchar ));
	strncpy ( encrypted_file,
		  "Grisbi encrypted file ",
		  22 );
	memmove ( encrypted_file + 22,
		  *file_content,
		  length );
	
	position = 0;

	while ( position < length)
	{
	    gulong size;

	    if ( (length - position ) < 8192 )
	    {
		size = length - position;

		/* the encryption need to be by block of 8, so the last bytes of the
		 * file won't be encrypted... i don't think it's annoying, if it is,
		 * we can change here to increase the lenght of the file */
		size = size / 8 * 8;
	    }
	    else
		size = 8192;

	    result = ecb_crypt ( key,
				 encrypted_file + 22 + position,
				 size,
				 DES_ENCRYPT );

	    if ( result == DESERR_BADPARAM )
	    {
		dialogue_error ( _("Error while crypting the file, the file saved won't be crypted."));
		return length;
	    }

	    position = position + 8192;
	}
	free (*file_content);
	*file_content = encrypted_file;

	/* the actual length is the initial + 22 (size of Grisbi encrypted file */
	return length + 22;
    }
    else
    {
	/* we want to decrypt the file */

	gchar *decrypted_file;

	/* we set the length on the rigt size */

	length = length - 22;
	
return_bad_password:

	/* now, if we know here a key to crypt, we use it, else, we ask for it */

	if ( crypt_key )
	    key = crypt_key;
	else
	    key = gsb_file_util_ask_for_crypt_key ( file_name, FALSE);

	/* if we have no key, we stop the loading */

	if ( !key )
	    return 0;

	/* we create a copy of the file in memory which will begin by "Grisbi encrypted file " */

	decrypted_file = malloc ( length * sizeof ( gchar ));
	memmove ( decrypted_file,
		  *file_content + 22,
		  length );
	
	position = 0;

	while ( position < length)
	{
	    gint size;

	    if ( (length - position ) < 8192 )
	    {
		size = length - position;

		/* the encryption need to be by block of 8, so the last bytes of the
		 * file won't be encrypted... i don't think it's annoying, if it is,
		 * we can change here to increase the lenght of the file */
		size = size / 8 * 8;
	    }
	    else
		size = 8192;

	    result = ecb_crypt ( key,
				 decrypted_file + position,
				 size,
				 DES_DECRYPT );

	    if ( result == DESERR_BADPARAM )
	    {
		dialogue_error ( _("Error while decrypting the file, the file cannot be opened."));
		return 0;
	    }

	    position = position + 8192;
	}

	/* befor freeing file_content and go back, we check that the password was correct
	 * if not, we free the decrypted_file and ask again for the password */

	if ( strncmp ( decrypted_file,
		       "<?xml version=\"1.0\"?>",
		       18 )
	     &&
	     strncmp ( decrypted_file,
		       "Grisbi compressed file ",
		       23 ))
	{
	    /* it seems that it was not the correct password */

	    free ( decrypted_file );

	    dialogue_error ( _( "The password was incorrect ! Please try again..."));
	    crypt_key = NULL;
	    goto return_bad_password;
	}

	free (*file_content);
	*file_content = decrypted_file;
	return length;
    }

    /* normally never come here */
    return 0;
}


/**
 * ask for the crypting key
 * return the key, and save it in the variable crypt_key if asked
 *
 * \param encrypt : TRUE if comes to encrypt, FALSE to decrypt
 *
 * \return a string which is the crypt key or NULL if it was
 * cancelled. */
gchar *gsb_file_util_ask_for_crypt_key ( gchar * file_name, gboolean encrypt )
{
    gchar *key = NULL;
    GtkWidget *dialog, *button, *label, *entry, *hbox, *hbox2, *vbox, *icon;
    gint result;

    dialog = gtk_dialog_new_with_buttons ( _("Grisbi password"),
					   GTK_WINDOW ( window ),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   ( encrypt ? _("Crypt file") : _("Decrypt file") ),
					   GTK_RESPONSE_OK,
					   NULL );
    gtk_dialog_set_default_response ( GTK_DIALOG ( dialog ),
				      GTK_RESPONSE_OK );

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox), hbox, TRUE, TRUE, 6 );

    /* Ugly dance to force alignement. */
    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, FALSE, FALSE, 6 );
    icon = gtk_image_new_from_stock ( GTK_STOCK_DIALOG_AUTHENTICATION,
				      GTK_ICON_SIZE_DIALOG );
    gtk_box_pack_start ( GTK_BOX ( vbox ), icon, FALSE, FALSE, 6 );
    
    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, TRUE, TRUE, 6 );

    label = gtk_label_new ("");
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );

    if ( encrypt )
	gtk_label_set_markup ( GTK_LABEL (label),
			       g_strdup_printf ( _( "Please enter password to encrypt file\n'<tt>%s</tt>'" ),
						 file_name ) );
    else
	gtk_label_set_markup ( GTK_LABEL (label), 
			       g_strdup_printf ( _( "Please enter password to decrypt file\n'<tt>%s</tt>'" ),
						 file_name ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 6 );

    hbox2 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox2, FALSE, FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 gtk_label_new ( COLON(_("Password")) ), 
			 FALSE, FALSE, 0 );

    entry = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry ), TRUE );
    gtk_entry_set_visibility ( GTK_ENTRY ( entry ), FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), entry, TRUE, TRUE, 0 );

    button = gtk_check_button_new_with_label ( _("Don't ask password again for this session."));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );

    gtk_widget_show_all ( dialog );
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch (result)
    {	
	case GTK_RESPONSE_OK:

	    key = g_strdup (gtk_entry_get_text ( GTK_ENTRY ( entry )));

	    if (!strlen (key))
		key = NULL;

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button )))
		crypt_key = key;
	    else
		crypt_key = NULL;
	    break;

	case GTK_RESPONSE_CANCEL:
	    key = NULL;
    }

    gtk_widget_destroy ( dialog );

    return key;
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

    if ( !gsb_account_get_accounts_amount () )
	return;

    if ( DEBUG )
	printf ( "switch_t_r\n");


    list_tmp_transactions = gsb_transaction_data_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_transaction_data_get_transaction_number (list_tmp_transactions -> data);

	switch ( gsb_transaction_data_get_marked_transaction (transaction_number_tmp))
	{
	    case 2 :
		gsb_transaction_data_set_marked_transaction ( transaction_number_tmp,
							      3 );
		break;
	    case 3:
		gsb_transaction_data_set_marked_transaction ( transaction_number_tmp,
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
    struct stat buffer_stat;
    int result;
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

    result = utf8_stat ( nom_fichier_comptes, &buffer_stat);

    if ( result == -1 )
    {
	dialogue_error (g_strdup_printf (_("Cannot open file '%s' to mark it as used: %s"),
					 nom_fichier_comptes,
					 latin2utf8 (strerror(errno))));
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

	result = utf8_stat ( lock_filename, &buffer_stat);

	if ( result != -1 )
	{
	    /* the file lock exists, if we want create a new one, we must force in the parameters */

	    dialogue_conditional_hint ( g_strdup_printf( _("File \"%s\" is already opened"),
							 nom_fichier_comptes),
					_("Either this file is already opened by another user or it wasn't closed correctly (maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the \"Force saving locked files\" option in setup."),
					(gchar *) &(etat.display_message_lock_active) );

	    /* the lock is already created, return TRUE */

	    etat.fichier_deja_ouvert = 1;
	    return TRUE;
	}

	etat.fichier_deja_ouvert = 0;

	fichier = utf8_fopen ( lock_filename, "w" );

	if ( !fichier )
	{
	    dialogue_error (g_strdup_printf (_("Cannot write lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno))));
	    return FALSE;
	}

	fclose ( fichier );
	return TRUE;
    }
    else
    {
	/* delete the lock file */

	etat.fichier_deja_ouvert = 0;

	/* check if it exits, if not, just go away */

	result = utf8_stat ( lock_filename, &buffer_stat);

	if ( result == -1 )
	    return TRUE;

	result = utf8_remove ( lock_filename );

	if ( result == -1 )
	{
	    dialogue_error (g_strdup_printf (_("Cannot erase lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno))));
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
    GtkWidget *dialog, *vbox, *checkbox;
    gint resultat;

    dialog = gtk_message_dialog_new ( GTK_WINDOW ( window ),
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_QUESTION,
				      GTK_BUTTONS_YES_NO,
				      " ");

    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), 
			   make_hint ( _("Account file is world readable."),
				       _("Your account file should not be readable by anybody else, but it is. You should change its permissions.\nShould this be fixed now?")));
    

    vbox = GTK_DIALOG(dialog) -> vbox;
    checkbox = new_checkbox_with_title ( _("Do not show this message again"),
					 &(etat.display_message_file_readable), NULL);
    gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, FALSE, FALSE, 6 );
    gtk_widget_show_all ( dialog );

    resultat = gtk_dialog_run ( GTK_DIALOG(dialog) );

    if ( resultat == GTK_RESPONSE_YES )
    {
	chmod ( nom_fichier_comptes, S_IRUSR | S_IWUSR );
    }

    gtk_widget_destroy ( dialog );
}


