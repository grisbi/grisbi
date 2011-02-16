/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2006 Benjamin Drieu (bdrieu@april.org)                */
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

/* This define is required to disable openssl's SSLeay support which redefines
 * _(), which obvisouly breaks glib's gettext macros. */
#define OPENSSL_DISABLE_OLD_DES_SUPPORT
#ifdef HAVE_SSL
#  include <openssl/des.h>
#endif

/*START_INCLUDE*/
#include "openssl.h"
#include "dialog.h"
#include "main.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*FIX FOR THE LINKING ERROR WITH WINDOWS MSVC*/
#ifdef _MSC_VER
#define SYMBOL_IMPORT __declspec(dllimport)
#else
#define SYMBOL_IMPORT extern
#endif

/*START_EXTERN*/
SYMBOL_IMPORT GtkWidget *window;
/*END_EXTERN*/

#ifdef HAVE_SSL
/*START_STATIC*/
static gchar *gsb_file_util_ask_for_crypt_key ( gchar * file_name, gchar * additional_message,
                        gboolean encrypt );
static gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
                        gboolean crypt, gulong length );
static void gsb_file_util_show_hide_passwd ( GtkToggleButton *togglebutton, GtkWidget *entry );
/*END_STATIC*/
#endif

static gchar *saved_crypt_key;

#define V1_MARKER "Grisbi encrypted file "
#define V1_MARKER_SIZE (sizeof(V1_MARKER) - 1)
#define V2_MARKER "Grisbi encryption v2: "
#define V2_MARKER_SIZE (sizeof(V2_MARKER) - 1)

#define ALIGN_TO_8_BYTES(l) ((l + 7) & (~7))



#ifdef HAVE_SSL
/**
 *
 */
static gulong
encrypt_v2(gchar *password, gchar **file_content, gulong length)
{
    DES_cblock key;
    DES_key_schedule sched;
    gulong to_encrypt_length, output_length;
    gchar *to_encrypt_content, *output_content;

    /* Create a temporary buffer that will hold data to be encrypted. */
    to_encrypt_length = V2_MARKER_SIZE + length;
    to_encrypt_content = g_malloc ( to_encrypt_length );
    g_memmove ( to_encrypt_content, V2_MARKER, V2_MARKER_SIZE );
    g_memmove ( to_encrypt_content + V2_MARKER_SIZE, *file_content, length );

    /* Allocate the output file and copy the special marker at its beginning.
     * DES_cbc_encrypt output is always a multiple of 8 bytes. Adjust the
     * length of the allocation accordingly. */
    output_length = V2_MARKER_SIZE + ALIGN_TO_8_BYTES ( to_encrypt_length );
    output_content = g_malloc ( output_length );
    g_memmove ( output_content, V2_MARKER , V2_MARKER_SIZE );

    /* Encrypt the data and put it in the right place in the output buffer. */
    DES_string_to_key ( password, &key );
    DES_set_key_unchecked ( &key, &sched );
    DES_set_odd_parity ( &key );

    DES_cbc_encrypt ( (guchar *) to_encrypt_content,
                    (guchar *) (output_content + V2_MARKER_SIZE),
                    to_encrypt_length,
                    &sched,
                    &key,
                    DES_ENCRYPT );

    g_free ( to_encrypt_content );

    *file_content = output_content;
    return output_length;
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
    g_memmove ( &key, password, MIN(sizeof(DES_cblock), strlen(password)) );

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
#endif /* HAVE_SSL */


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
gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
                        gboolean crypt, gulong length )
{
#ifdef HAVE_SSL
    gchar * key, * message = "";

    if ( run.new_crypted_file )
    {
        if ( saved_crypt_key )
            g_free ( saved_crypt_key );
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

        return encrypt_v2 ( key, file_content, length );
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

#else
    /* FIXME: import symbols to be sure we can call this kind of stuff. */
    dialogue_error_hint ( _("This build of Grisbi does not support encryption.\n"
                        "Please recompile Grisbi with OpenSSL encryption enabled."),
                        g_strdup_printf ( _("Cannot open encrypted file '%s'"),
					    file_name ) );
#endif

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
#ifdef HAVE_SSL
gchar *gsb_file_util_ask_for_crypt_key ( gchar * file_name, gchar * additional_message,
                        gboolean encrypt )
{
    gchar *key = NULL;
    GtkWidget *dialog, *button, *label, *entry, *hbox, *hbox2, *vbox, *icon;
    gint result;

    dialog = gtk_dialog_new_with_buttons ( _("Grisbi password"),
                        GTK_WINDOW ( window ),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        ( encrypt ? _("Crypt file") : _("Decrypt file") ), GTK_RESPONSE_OK,
                        NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );
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

    label = gtk_label_new ("");
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );

    if ( encrypt )
        gtk_label_set_markup ( GTK_LABEL (label),
                        g_strdup_printf (
                        _( "%sPlease enter password to encrypt file\n<span "
                        "foreground=\"blue\">%s</span>" ),
                        additional_message, file_name ) );
    else
        gtk_label_set_markup ( GTK_LABEL (label),
                        g_strdup_printf (
                        _( "%sPlease enter password to decrypt file\n<span "
                        "foreground=\"blue\">%s</span>" ),
                        additional_message, file_name ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 6 );

    hbox2 = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox2, FALSE, FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
                        gtk_label_new ( _("Password: ") ),
                        FALSE, FALSE, 0 );

    entry = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry ), TRUE );
    gtk_entry_set_visibility ( GTK_ENTRY ( entry ), FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), entry, TRUE, TRUE, 0 );

    if ( run.new_crypted_file )
    {
        button = gtk_check_button_new_with_label ( _("View password") );
        gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );
        g_signal_connect ( G_OBJECT ( button ),
			            "toggled",
			            G_CALLBACK ( gsb_file_util_show_hide_passwd ),
			            entry );
    }

    button = gtk_check_button_new_with_label ( _("Don't ask password again for this session."));
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );

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

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button )))
            saved_crypt_key = key;
        else
            saved_crypt_key = NULL;

        run.new_crypted_file = FALSE;

        break;

    case GTK_RESPONSE_CANCEL:
            key = NULL;
    }

    gtk_widget_destroy ( dialog );

    return key;
}
#endif


#ifndef ENABLE_STATIC
/** Module name. */
G_MODULE_EXPORT const gchar plugin_name[] = "openssl";
#endif



/** Initialization function. */
G_MODULE_EXPORT extern void openssl_plugin_register ( void )
{
    devel_debug ("Initializating openssl plugin");
    saved_crypt_key = NULL;
}



/** Main function of module. */
G_MODULE_EXPORT extern gpointer openssl_plugin_run ( gchar * file_name, gchar **file_content,
                        gboolean crypt, gulong length )
{
    /* The final size is cast from a gulong to a gpointer. This is 'ok' because
     * a gpointer is always the same size. It is quite ugly though, and a proper
     * fix should be found for this. */
    return (gpointer) gsb_file_util_crypt_file ( file_name, file_content, crypt, length );
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
