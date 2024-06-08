/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2011 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                            */
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


#include "config.h"

#include <string.h>
#include <glib/gi18n.h>

 /*START_INCLUDE*/
#include "gsb_regex.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GHashTable *regex_store = NULL;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/**
 * Initialize the hash map which is used to store
 * compile GRegex during the whole exeuction of Grisbi.
 *
 * This function is called on Grisbi startup and each time
 * of a new file is opened.
 *
 * \return void
 */
void gsb_regex_init_variables ( void )
{
    if ( regex_store )
    {
        g_hash_table_destroy ( regex_store );
        regex_store = NULL;
    }

    regex_store = g_hash_table_new_full ( g_str_hash,
                                          g_str_equal,
                                          (GDestroyNotify) g_free,
                                          (GDestroyNotify) g_regex_unref );
}


/**
 * Add a new regex to the map.
 *
 * \param key the key to retrieve the regex
 * \param pattern the pattern of the GRegex
 * \param compile_options options passed to g_regex_new, can be NULL
 * \param match_options options passed to g_regex_new, can be NULL
 *
 * \return Pointer value on success, NULL otherwise
 */
GRegex *gsb_regex_insert ( const gchar *key,
                            const gchar *pattern,
                            GRegexCompileFlags compile_options,
                            GRegexMatchFlags match_options )
{
    GRegex *regex;
    GError *error = NULL;

    //~ devel_debug (pattern);
    if ( ! regex_store )
        return NULL;

    if ( ! key || ! strlen ( key )
         || ! pattern || ! strlen ( pattern ) )
        return NULL;

    regex = g_regex_new ( pattern, compile_options, match_options, &error );
    if ( ! regex )
    {
        g_print ( _("Unable to compile regex: %s\n"), error -> message);
        g_error_free ( error );
        return NULL;
    }

    g_hash_table_insert ( regex_store,
                          (gpointer) g_strdup ( key ),
                          regex );

    return regex;
}


/**
 * Get a GRegex stored in the map.
 *
 * \param key
 *
 * \return Pointer on the regex, NULL if it can't be found
 */
GRegex *gsb_regex_lookup ( const gchar *key )
{
    return g_hash_table_lookup ( regex_store, key );
}


/**
 * Destroy the hash map. Free keys and values properly.
 *
 * This function is called once on Grisbi shutdown.
 *
 * \return void
 */
void gsb_regex_destroy ( void )
{
    if ( regex_store )
	{
        g_hash_table_destroy ( regex_store );
        regex_store = NULL;
	}
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
