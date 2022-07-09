/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2006 Benjamin Drieu (bdrieu@april.org)                */
/*          http://www.grisbi.org/                                            */
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
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/* This define is required to disable openssl's SSLeay support which redefines
 * _(), which obvisouly breaks glib's gettext macros. */
#define OPENSSL_DISABLE_OLD_DES_SUPPORT
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/err.h>

/*START_INCLUDE*/
#include "openssl.h"
#include "grisbi_app.h"
#include "structures.h"
#include "utils.h"
#include "dialog.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
static gchar *gsb_file_util_ask_for_crypt_key ( const gchar * file_name, gchar * additional_message,
                        gboolean encrypt );
static void gsb_file_util_show_hide_passwd ( GtkToggleButton *togglebutton, GtkWidget *entry );

static void gsb_file_util_show_hide_passwd_from_icon (GtkEntry *entry,
													  GtkEntryIconPosition icon_pos,
													  GdkEvent *event,
													  gpointer null)
{
	GdkPixbuf *pixbuf;
	gint visibility;

    visibility = gtk_entry_get_visibility (GTK_ENTRY (entry) );
	if (visibility)
	{
		pixbuf = g_object_get_data (G_OBJECT (entry), "pixbuf_1");
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, pixbuf);
	}
	else
	{
		pixbuf = g_object_get_data (G_OBJECT (entry), "pixbuf_2");
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, pixbuf);
	}
    gtk_entry_set_visibility (GTK_ENTRY (entry), !visibility);

}

/*END_STATIC*/

static gchar *saved_crypt_key = NULL;

#define V1_MARKER "Grisbi encrypted file "
#define V1_MARKER_SIZE (sizeof(V1_MARKER) - 1)
#define V2_MARKER "Grisbi encryption v2: "
#define V2_MARKER_SIZE (sizeof(V2_MARKER) - 1)
#define V3_MARKER "Grisbi encryption v3: "
#define V3_MARKER_SIZE (sizeof(V3_MARKER) - 1)



/**
 *
 */
static gulong
encrypt_v3(gchar *password, gchar **file_content, int length)
{
    int to_encrypt_length, output_length, tmp_length;
	unsigned char *to_encrypt_content, *output_content, *encrypted_content;
	unsigned char hash[EVP_MAX_MD_SIZE];
	unsigned char iv[] = "1234567887654321";
	unsigned int hash_len;
	EVP_MD_CTX *md_ctx;
	EVP_CIPHER_CTX *cipher_ctx;

    /* Create a temporary buffer that will hold data to be encrypted. */
    to_encrypt_length = V3_MARKER_SIZE + length;
    to_encrypt_content = g_malloc ( to_encrypt_length );

	/* copy V3 marker */
    memcpy ( to_encrypt_content, V3_MARKER, V3_MARKER_SIZE );
	/* copy data */
    memcpy ( to_encrypt_content + V3_MARKER_SIZE, *file_content, length );

    /* Allocate the output file and copy the special marker at its beginning. */
    output_length = V3_MARKER_SIZE + to_encrypt_length + EVP_MAX_BLOCK_LENGTH;
    output_content = g_malloc ( output_length );
	/* add V3 marker to output (in clear) */
    memcpy ( output_content, V3_MARKER , V3_MARKER_SIZE );

	/* sip the clear marker */
	encrypted_content = output_content + V3_MARKER_SIZE;

	/* hash the password to generate a key */
	md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit(md_ctx, EVP_sha256());
	EVP_DigestUpdate(md_ctx, password, strlen(password));
	EVP_DigestFinal_ex(md_ctx, hash, &hash_len);
	EVP_MD_CTX_free(md_ctx);

    /* Encrypt the data and put it in the right place in the output buffer. */
	cipher_ctx = EVP_CIPHER_CTX_new();
	EVP_CipherInit_ex(cipher_ctx, EVP_aes_128_cbc(), NULL, hash, iv, 1 /* encrypt */);
	if (!EVP_CipherUpdate(cipher_ctx, encrypted_content, &output_length, to_encrypt_content, to_encrypt_length))
	{
		/* Error */
		EVP_CIPHER_CTX_free(cipher_ctx);
		alert_debug(ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	if (!EVP_CipherFinal_ex(cipher_ctx, encrypted_content + output_length, &tmp_length))
	{
		/* Error */
		EVP_CIPHER_CTX_free(cipher_ctx);
		alert_debug(ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	output_length += tmp_length;
	EVP_CIPHER_CTX_free(cipher_ctx);

	g_free ( to_encrypt_content );

    *file_content = (gchar *)output_content;
    return output_length + V3_MARKER_SIZE;
}


/**
 *
 */
static gulong
decrypt_v3(gchar *password, gchar **file_content, int length)
{
	int encrypted_len, decrypted_len, output_len, tmp_len;
	unsigned char *encrypted_buf, *decrypted_buf;
	gchar *output_buf;
	unsigned char hash[EVP_MAX_MD_SIZE];
	unsigned char iv[] = "1234567887654321";
	unsigned int hash_len;
	EVP_MD_CTX *md_ctx;
	EVP_CIPHER_CTX *cipher_ctx;

	/* skip the marker */
	encrypted_buf = (unsigned char *)*file_content + V3_MARKER_SIZE;
	encrypted_len = length - V3_MARKER_SIZE;

	/* Create a temporary buffer that will hold the decrypted data without the
	 * first marker. */
	decrypted_len = encrypted_len + EVP_MAX_BLOCK_LENGTH;
	decrypted_buf = g_malloc ( decrypted_len );
	
	/* clean the buffer to avoid problems with an incomplete last block */
	memset(decrypted_buf, 0, decrypted_len);

	/* hash the password to generate a key */
	md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit(md_ctx, EVP_sha256());
	EVP_DigestUpdate(md_ctx, password, strlen(password));
	EVP_DigestFinal_ex(md_ctx, hash, &hash_len);
	EVP_MD_CTX_free(md_ctx);

	cipher_ctx = EVP_CIPHER_CTX_new();
	EVP_CipherInit_ex(cipher_ctx, EVP_aes_128_cbc(), NULL, hash, iv, 0 /* decrypt */);
	if (!EVP_CipherUpdate(cipher_ctx, decrypted_buf, &output_len, encrypted_buf, encrypted_len))
	{
		/* Error */
		EVP_CIPHER_CTX_free(cipher_ctx);
		alert_debug(ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	if (!EVP_CipherFinal_ex(cipher_ctx, decrypted_buf + output_len, &tmp_len))
	{
		/* Error */
		EVP_CIPHER_CTX_free(cipher_ctx);
		alert_debug(ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	output_len += tmp_len;
	EVP_CIPHER_CTX_free(cipher_ctx);

	/* If the password was correct, the second marker should appear in the first
	 * few bytes of the decrypted content. */
	if ( strncmp ( (const char *)decrypted_buf, V3_MARKER, V3_MARKER_SIZE ) )
	{
		g_free ( decrypted_buf );
		return 0;
	}

	/* Copy the decrypted data to a final buffer, leaving out the second
	 * marker. g_strndup() is used to add a trailing null byte. */
	output_len = decrypted_len - V3_MARKER_SIZE;
	output_buf = g_strndup ( (const gchar *)decrypted_buf + V3_MARKER_SIZE, output_len );

	g_free ( decrypted_buf );

	*file_content = output_buf;
	return output_len;
}


/**
 *
 */
static gulong
decrypt_v2(gchar *password, gchar **file_content, gulong length)
{
    DES_cblock key;
    DES_key_schedule sched;
    gulong decrypted_len, output_len;
    gchar *decrypted_buf, *output_buf;

    /* Create a temporary buffer that will hold the decrypted data without the
     * first marker. */
    decrypted_len = length - V2_MARKER_SIZE;
    decrypted_buf = g_malloc ( decrypted_len );

    DES_string_to_key ( password, &key );
    DES_set_key_unchecked( &key, &sched );
    DES_set_odd_parity ( &key );

    DES_cbc_encrypt ( (guchar *) (* file_content + V2_MARKER_SIZE),
              (guchar *) decrypted_buf,
              (long) decrypted_len,
              &sched,
              &key,
              DES_DECRYPT );

    /* If the password was correct, the second marker should appear in the first
     * few bytes of the decrypted content. */
    if ( strncmp ( decrypted_buf, V2_MARKER, V2_MARKER_SIZE ) )
    {
        g_free ( decrypted_buf );
        return 0;
    }

    /* Copy the decrypted data to a final buffer, leaving out the second
     * marker. g_strndup() is used to add a trailing null byte. */
    output_len = decrypted_len - V2_MARKER_SIZE;
    output_buf = g_strndup ( decrypted_buf + V2_MARKER_SIZE, output_len );

    g_free ( decrypted_buf );

    *file_content = output_buf;
    return output_len;
}


/**
 *
 */
static gulong
decrypt_v1(gchar *password, gchar **file_content, gulong length)
{
    DES_cblock key;
    DES_key_schedule sched;
    gulong decrypted_len;
    gchar *decrypted_buf;

    /* Create a temporary buffer that will hold the decrypted data without the
     * marker. A trailing null byte is also added. */
    decrypted_len = length - V1_MARKER_SIZE;
    decrypted_buf = g_malloc ( decrypted_len + 1 );
    decrypted_buf[decrypted_len] = 0;

    DES_string_to_key ( password, &key );
    DES_set_key_unchecked( &key, &sched );
    DES_set_odd_parity ( &key );
    /* Set the DES key the WRONG AND BROKEN way. DO NOT REUSE THIS CODE EVER! */
    memset ( &key, 0, sizeof ( DES_cblock ) );
    memmove ( &key, password, MIN(sizeof(DES_cblock), strlen(password)) );

    DES_cbc_encrypt ( (guchar *) (* file_content + V1_MARKER_SIZE),
        (guchar *) decrypted_buf,
        (long) decrypted_len,
        &sched,
        &key,
        DES_DECRYPT );

    /* If the password was correct, the first few bytes of the decrypted
     * content should contain the following strings. */
    if ( strncmp ( decrypted_buf, "<?xml version=\"1.0\"?>", 18 )
         &&
         strncmp ( decrypted_buf, "Grisbi compressed file ", 23 ) )
    {
        g_free ( decrypted_buf );
        return 0;
    }

    *file_content = decrypted_buf;
    return decrypted_len;
}


/**
 * Crypt or decrypt string given in the param
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
gulong gsb_file_util_crypt_file ( const gchar * file_name, gchar **file_content,
                        gboolean crypt, gulong length )
{
    gchar * key, * message = NULL;
	GrisbiWinRun *w_run;

	w_run = grisbi_win_get_w_run ();
    if (w_run->new_crypted_file )
    {
        if ( saved_crypt_key )
		{
            g_free ( saved_crypt_key );
		}
	    saved_crypt_key = NULL;
    }
    if ( crypt )
    {
        /* now, if we know here a key to crypt, we use it, else, we ask for it */

        if ( saved_crypt_key )
            key = saved_crypt_key;
        else
            key = gsb_file_util_ask_for_crypt_key ( file_name, message, TRUE);

        /* if we have no key, we will no crypt that file */

        if ( !key )
            return 0;

        return encrypt_v3 ( key, file_content, (int)length );
    }
    else
    {
        gulong returned_length;

return_bad_password:

        /* now, if we know here a key to crypt, we use it, else, we ask for it */

        if ( saved_crypt_key )
            key = saved_crypt_key;
        else
            key = gsb_file_util_ask_for_crypt_key ( file_name, message, FALSE );

        /* if we have no key, we stop the loading */

        if ( !key )
            return 0;

		returned_length = decrypt_v3 ( key, file_content, (int)length );

		if ( returned_length == 0 )
			returned_length = decrypt_v2 ( key, file_content, length );

        if ( returned_length == 0 )
            returned_length = decrypt_v1 ( key, file_content, length );

        if ( returned_length == 0 )
        {
            /* it seems that it was not the correct password */

            message = _( "<span weight=\"bold\" foreground=\"red\">Password is incorrect!</span>\n\n");
            g_free ( saved_crypt_key );
            saved_crypt_key = NULL;
            goto return_bad_password;
        }

        return returned_length;
    }

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
gchar *gsb_file_util_ask_for_crypt_key ( const gchar * file_name, gchar * additional_message,
                        gboolean encrypt )
{
	GdkPixbuf *pixbuf_1;
	GdkPixbuf *pixbuf_2;
    gchar *key = NULL;
    GtkWidget *dialog, *button = NULL, *label, *entry, *hbox, *hbox2, *vbox, *icon;
	gchar *tmp_msg;
    gint result;
	GrisbiWinRun *w_run;

	w_run = grisbi_win_get_w_run ();

    dialog = gtk_dialog_new_with_buttons ( _("Grisbi password"),
                        GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        "gtk-cancel", GTK_RESPONSE_CANCEL,
                        ( encrypt ? _("Crypt file") : _("Decrypt file") ), GTK_RESPONSE_OK,
                        NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );
    gtk_dialog_set_default_response ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( gtk_dialog_get_content_area ( GTK_DIALOG ( dialog ) ) ), hbox, TRUE, TRUE, 6 );

    /* Ugly dance to force alignement. */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, FALSE, FALSE, 6 );
    icon = gtk_image_new_from_icon_name ( "gtk-dialog-authentication", GTK_ICON_SIZE_DIALOG );
    gtk_box_pack_start ( GTK_BOX ( vbox ), icon, FALSE, FALSE, 6 );

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, TRUE, TRUE, 6 );

    label = gtk_label_new ("");
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
    utils_labels_set_alignment ( GTK_LABEL (label), 0, 0);
    gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );

    if ( encrypt )
	{
		if (additional_message && strlen (additional_message))
			tmp_msg = g_strdup_printf (_("%sPlease enter password to encrypt file\n<span "
										   "foreground=\"blue\">%s</span>"),
									   additional_message,
									   file_name);
		else
			tmp_msg = g_strdup_printf (_("Please enter password to encrypt file\n<span "
										   "foreground=\"blue\">%s</span>"),
									   file_name);
	}
    else
	{
		if (additional_message && strlen (additional_message))
			tmp_msg = g_strdup_printf (_("%sPlease enter password to decrypt file\n<span "
										 "foreground=\"blue\">%s</span>"),
									   additional_message,
									   file_name);
		else
			tmp_msg = g_strdup_printf (_("Please enter password to decrypt file\n<span "
										 "foreground=\"blue\">%s</span>"),
									   file_name);
	}

	gtk_label_set_markup ( GTK_LABEL (label), tmp_msg);
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 6 );
	g_free (tmp_msg);

    hbox2 = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox2, FALSE, FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
                        gtk_label_new ( _("Password: ") ),
                        FALSE, FALSE, 0 );

    entry = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry ), TRUE );
    gtk_entry_set_visibility ( GTK_ENTRY ( entry ), FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), entry, TRUE, TRUE, 0 );
	pixbuf_1 = gdk_pixbuf_new_from_resource ("/org/gtk/grisbi/images/gtk-eye-not-looking.svg", NULL);
	pixbuf_2 = gdk_pixbuf_new_from_resource ("/org/gtk/grisbi/images/gtk-eye-looking.svg", NULL);

	if (pixbuf_1 && pixbuf_2)
	{
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, pixbuf_1);
		g_object_set_data_full (G_OBJECT (entry), "pixbuf_1", pixbuf_1, g_object_unref);
		g_object_set_data_full (G_OBJECT (entry), "pixbuf_2", pixbuf_2, g_object_unref);

        g_signal_connect (G_OBJECT (entry),
						  "icon-press",
						  G_CALLBACK (gsb_file_util_show_hide_passwd_from_icon),
						  NULL);
	}
	else if (w_run->new_crypted_file)
    {
        button = gtk_check_button_new_with_label ( _("View password") );
        gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );
        g_signal_connect ( G_OBJECT ( button ),
			            "toggled",
			            G_CALLBACK ( gsb_file_util_show_hide_passwd ),
			            entry );
    }
	if (!encrypt)
	{
		button = gtk_check_button_new_with_label ( _("Don't ask password again for this session."));
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
		gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );
	}

    gtk_widget_show_all ( dialog );

#ifdef __APPLE__
return_bad_password:
#endif /* __APPLE__ */
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch (result)
    {
    case GTK_RESPONSE_OK:
        key = g_strdup (gtk_entry_get_text ( GTK_ENTRY ( entry )));

        if (!strlen ( key ) )
        {
            g_free ( key );
            key = NULL;
        }
#ifdef __APPLE__
        else if ( g_utf8_strlen ( key, -1 ) < 7 )
        {
            dialogue_warning_hint ( _("The password must contain at least 7 characters"),
                                   _("Password too short" ) );
            gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );

            goto return_bad_password;
        }
#endif /* __APPLE__ */

        if ( button && gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button )))
            saved_crypt_key = key;
        else
            saved_crypt_key = NULL;
        w_run->new_crypted_file = FALSE;

        break;

    case GTK_RESPONSE_CANCEL:
            key = NULL;
    }

    gtk_widget_destroy ( dialog );

    return key;
}

void gsb_file_util_show_hide_passwd ( GtkToggleButton *togglebutton, GtkWidget *entry )
{
    gint visibility;

    visibility = gtk_entry_get_visibility ( GTK_ENTRY ( entry ) );
    if ( visibility )
        gtk_button_set_label ( GTK_BUTTON ( togglebutton ), _("View password") );
    else
        gtk_button_set_label ( GTK_BUTTON ( togglebutton ), _("Hide password") );

    gtk_entry_set_visibility ( GTK_ENTRY ( entry ), !visibility );
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
