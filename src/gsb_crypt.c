/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2006 Benjamin Drieu (bdrieu@april.org)	      */
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

#ifndef NOSSL
#  include <openssl/des.h>
#endif
#include "include.h"

/*START_INCLUDE*/
#include "gsb_crypt.h"
#include "./dialog.h"
#include "./plugins/openssl/openssl.h"
#include "./utils_str.h"
#include "./structures.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
extern gchar *crypt_key;
extern GtkWidget *window ;
/*END_EXTERN*/

#ifndef NOSSL
/*START_STATIC*/
/*END_STATIC*/
#endif





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
#ifndef NOSSL
    gchar * key;
    des_cblock openssl_key;
    des_key_schedule sched;

    gchar* tmpstr = g_strdup_printf ("gsb_file_util_crypt_file : %d", crypt );
    devel_debug ( tmpstr );
    g_free ( tmpstr );

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

	des_string_to_key ( key, &openssl_key );
	des_set_key_unchecked( &openssl_key, sched );
	DES_set_odd_parity ( &openssl_key );

	/* we create a copy of the file in memory which will begin by
	 * "Grisbi encrypted file " */

	encrypted_file = g_malloc ( (length + 22) * sizeof ( gchar ));
	strncpy ( encrypted_file,
		  "Grisbi encrypted file ",
		  22 );

	des_cbc_encrypt ( (guchar *) (* file_content),
			  (guchar *) (encrypted_file + 22), 
			  (long) length, 
			  sched, 
			  (DES_cblock *) key, 
			  TRUE );

	g_free (*file_content);
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
	    key = gsb_file_util_ask_for_crypt_key ( file_name, FALSE );

	/* if we have no key, we stop the loading */

	if ( !key )
	    return 0;

	des_string_to_key ( key, &openssl_key );
	des_set_key_unchecked( &openssl_key, sched );
	DES_set_odd_parity ( &openssl_key );

	/* we create a copy of the file in memory which will begin by "Grisbi encrypted file " */

	decrypted_file = g_malloc ( length * sizeof ( gchar ));
	
	des_cbc_encrypt ( (guchar *) (* file_content + 22), 
			  (guchar *) decrypted_file, 
			  (long) length, 
			  sched, 
			  (DES_cblock *) key, 
			  FALSE );

	/* before freeing file_content and go back, we check that the password was correct
	 * if not, we free the decrypted_file and ask again for the password */

	if ( strncmp ( decrypted_file,
		       "<?xml version=\"1.0\"?>",
		       18 ))
	{
	    /* it seems that it was not the correct password */

	    g_free ( decrypted_file );

	    dialogue_error ( _( "The password was incorrect ! Please try again..."));
	    crypt_key = NULL;
	    goto return_bad_password;
	}

	g_free (*file_content);
	*file_content = decrypted_file;
	return length;
    }

#else
    gchar* tmpstr = g_strdup_printf ( _("Cannot open encrypted file '%s'"),
					    file_name );
    dialogue_error_hint ( _("This build of Grisbi does not support encryption.\n"
			    "Please recompile Grisbi with OpenSSL encryption enabled."),
			  tmpstr );
    g_free ( tmpstr );
#endif

    return 0;
}


/**
 * ask for the crypting key
 * return the key, and save it in the variable crypt_key if asked
 *
 * \param encrypt : TRUE if comes to encrypt, FALSE to decrypt
 *
 * \return a newly allocated string which is the crypt key or NULL if it was
 * cancelled. */
#ifndef NOSSL
gchar *gsb_file_util_ask_for_crypt_key ( gchar * file_name, gboolean encrypt )
{
    gchar *key = NULL;
    GtkWidget *dialog, *button, *label, *entry, *hbox, *hbox2, *vbox, *icon;
    gint result;

    dialog = gtk_dialog_new_with_buttons ( _("Grisbi password"),
					   GTK_WINDOW ( window ),
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   ( encrypt ? _("Crypt file") : _("Decrypt file") ),
					   GTK_RESPONSE_OK,
					   NULL );
    gtk_dialog_set_default_response ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK );

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

    label = gtk_label_new (NULL);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );

    if ( encrypt )
    {
        gchar* tmpstr = g_strdup_printf ( _( "Please enter password to encrypt file\n'<tt>%s</tt>'" ),
						 file_name );
	gtk_label_set_markup ( GTK_LABEL (label), tmpstr );
        g_free ( tmpstr );
    }
    else
    {
        gchar* tmpstr = g_strdup_printf ( _( "Please enter password to decrypt file\n'<tt>%s</tt>'" ),
						 file_name );
	gtk_label_set_markup ( GTK_LABEL (label), tmpstr );
        g_free ( tmpstr );
    }
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

	    key = my_strdup (gtk_entry_get_text ( GTK_ENTRY ( entry )));

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
#endif




