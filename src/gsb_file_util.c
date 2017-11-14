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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <errno.h>
#include <glib/gi18n.h>
#include <zlib.h>

/*START_INCLUDE*/
#include "gsb_file_util.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "utils_str.h"
#include "utils_files.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern struct ConditionalMessage messages[];
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
	gchar* tmpstr;
	gboolean response;

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

	tmpstr = g_strdup_printf (_("Do you want to overwrite file \"%s\"?"), filename);
	response = question_yes_no (tmpstr, _("File already exists"),
				       GTK_RESPONSE_NO );
	g_free ( tmpstr );
        return response;
    }
    return TRUE;
}


/**
 * this function do the same as g_file_get_contents
 * but can also open a compressed file with zlib
 *
 * \param filename the name of file to open
 * \param file_content a gchar pointer to fill with the adress of the content of the file
 * \param length a gulong pointer to file with the length of the loaded file
 *
 * \return TRUE all is ok, FALSE a problem occured
 * */
gboolean gsb_file_util_get_contents ( const gchar *filename,
				      gchar **file_content,
				      gulong *length )
{
    gzFile file;
    struct stat stat_buf;
    gulong alloc_size;
    gulong orig_size;
    gchar *content;
    int iterator = 0;
    gboolean eof = 0;
	gchar *os_filename;

#ifdef G_OS_WIN32
	os_filename = g_locale_from_utf8(filename, -1, NULL, NULL, NULL);
#else
	os_filename = g_strdup(filename);
#endif /* G_OS_WIN32 */

    file = gzopen (os_filename, "rb");
    if (!file)
	return FALSE;

    /* use stat to get the size of the file, windows ? */
    if (stat (os_filename, &stat_buf))
    {
	gchar *tmpstr = g_strdup_printf ( _("Grisbi cannot stat file %s, please check the file."),
					  os_filename);
	dialogue_error (tmpstr);
	g_free (tmpstr);
	return FALSE;
    }

    orig_size = stat_buf.st_size;
#ifndef G_OS_WIN32
    if (gzdirect (file))
	/* the file is not compressed, keep the original size */
	alloc_size = orig_size + 1;
    else
#endif /*_G_OS_WIN32 */
	/* the file is compressed, the final size should be about 20x more
	 *  this is not completly true, if the file is compressed AND crypted,
	 * the size doesn't really change. but i can't know here if the file is crypted
	 * because encrypt before compressed. perhaps forbid to compress a crypted file ?
	 * but i don't think it's a big thing because that allocated memory will be freed
	 * after loading the file ...*/
	alloc_size = 20 * orig_size;

    content = g_malloc0 (alloc_size);
    if (!content)
    {
	dialogue_error_memory ();
	return FALSE;
    }

    /* we should be able to get directly the orig_size
     * for most of files it's enough, if the file is compressed,
     * we continue */
    iterator = gzread (file, content, orig_size);

    if (iterator < 0)
    {
	int save_errno = errno;
	gchar *tmpstr;

	g_free (content);
	tmpstr = g_strdup_printf ( _("Failed to read from file '%s': %s"),
				   os_filename, g_strerror (save_errno));
	dialogue_error (tmpstr);
	g_free (tmpstr);
	return FALSE;
    }

    /* ok, now add caracter by caracter untill the end of the file */
    do
    {
	gchar c;

	c = gzgetc (file);

	eof = gzeof (file);
	if (!eof)
	{
	    content[iterator] = c;
	    iterator++;

	    if (iterator >= (gint) alloc_size)
	    {
		/* we need more space, should be rare,
		 * show a warning to prevent and correct if necessary */
		devel_debug ("Realloc is needed, if this message comes often, please contact the Grisbi team to improve the software ;-)");
		alloc_size = alloc_size + orig_size;
		content = g_realloc (content, alloc_size);

		if (!content)
		{
		    dialogue_error_memory ();
		    return FALSE;
		}
	    }
	}
    }
    while (!eof);

    content[iterator] = '\0';

    /* fill the returned values */
    *length = iterator;
    *file_content = content;

    gzclose (file);
	g_free(os_filename);
    return TRUE;
}

/**
 * create or delete a file ".name_of_file.lock" to check if the file is opened
 * already or not
 *
 * \param create_lock if we create or delete it
 *
 * \return TRUE if ok
 * */
gboolean gsb_file_util_modify_lock (const gchar *filename,
									gboolean create_lock )
{
    gchar *lock_filename;
	gchar *tmp_str;

    devel_debug_int ( create_lock );
	if (!filename)
		return TRUE;

    /* if the file was already opened we do nothing */
    if ((etat.fichier_deja_ouvert) || strlen (filename) == 0)
        return TRUE;

    /* Check if filename exists.  If not, this is a new
     * file so don't try to lock it. */
    if ( !g_file_test (filename, G_FILE_TEST_EXISTS ) )
        return FALSE;

    /* Create the name of the lock file */
	tmp_str = g_path_get_basename (filename);
    lock_filename = g_strconcat (g_get_tmp_dir (),
								 G_DIR_SEPARATOR_S,
#ifndef G_OS_WIN32
								 ".",
#endif /* G_OS_WIN32 */
								 tmp_str,
								 ".lock",
								 NULL);
    g_free (tmp_str);

    if ( create_lock )
    {
        /* now we create the lock file */

        FILE *fichier;

        /* check if the file lock exists */
        if ( g_file_test ( lock_filename, G_FILE_TEST_EXISTS ) )
        {
            dialog_message ( "account-already-opened", filename);

            /* the lock is already created, return TRUE */
            etat.fichier_deja_ouvert = 1;
            return TRUE;
        }

        etat.fichier_deja_ouvert = 0;

        fichier = utils_files_utf8_fopen ( lock_filename, "w" );

        if ( !fichier )
        {
            tmp_str = g_strdup_printf ( _("Cannot write lock file: '%s': %s"),
									   filename,
									   g_strerror ( errno ) );
            dialogue_error ( tmp_str );
            g_free ( tmp_str );

            return FALSE;
        }

        fclose ( fichier );
        return TRUE;
    }
    else
	{
		if ( etat.fichier_deja_ouvert == 0 )
		{
			/* delete the lock file */
			gint result;

			/* check if it exits, if not, just go away */
			if ( !g_file_test ( lock_filename, G_FILE_TEST_EXISTS ) )
				return TRUE;

			result = utils_files_utf8_remove ( lock_filename );

			if ( result == -1 )
			{
				gchar* tmp_str;

				tmp_str = g_strdup_printf (_("Cannot erase lock file: '%s': %s"),
										   filename,
										   g_strerror ( errno ) );
				dialogue_error ( tmp_str );
				g_free ( tmp_str );

				return FALSE;
			}
			return TRUE;
		}
		else
		{
			return TRUE;
		}
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
    devel_debug (NULL);

#ifndef G_OS_WIN32
    if ( question_conditional_yes_no ( "account-file-readable" ) == TRUE )
    {
		const gchar *filename;

		filename = grisbi_win_get_filename (NULL);
        chmod (filename, S_IRUSR | S_IWUSR );
    }

#endif /* G_OS_WIN32 */
}


/**
 *
 * called when loading a file, if the permissions are not set only for the user
 * display a conditional warning
 *
 * \param
 *
 * \return
 * */
void gsb_file_util_display_warning_permissions ( void )
{
    gint msg_no = 0;

    devel_debug (NULL);

    msg_no = question_conditional_yes_no_get_no_struct ( &messages[0], "account-file-readable" );

    dialogue_conditional_hint ( _(messages[msg_no].hint), _(messages[msg_no].message), "account-file-readable" );
}


