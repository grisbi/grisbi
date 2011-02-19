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

/*START_INCLUDE*/
#include "gsb_plugins.h"
#include "dialog.h"
#ifdef ENABLE_STATIC
#include "plugins/gnucash/gnucash.h"
#include "plugins/ofx/ofx.h"
#include "plugins/openssl/openssl.h"
#endif /* ENABLE_STATIC */
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/


/** List of registered plugins.  It should contain gsb_plugin structures. */
static GSList * plugins = NULL;

/**
 *
 *
 */
void gsb_plugins_scan_dir ( const char *dirname )
{
#ifdef ENABLE_STATIC
    gsb_plugin *plugin = NULL;

    plugin = g_malloc0 ( sizeof ( gsb_plugin ) );
    plugin -> name = "gnucash";
    plugin -> plugin_register = &gnucash_plugin_register;
    plugin -> plugin_run =      &gnucash_plugin_run;
    plugin -> plugin_register ();
    plugins = g_slist_append ( plugins, plugin );

#ifndef NOOFX
    plugin = g_malloc0 ( sizeof ( gsb_plugin ) );
    plugin -> name = "ofx";
    plugin -> plugin_register = &ofx_plugin_register;
    plugin -> plugin_run =      &ofx_plugin_run;
    plugin -> plugin_register ();
    plugins = g_slist_append ( plugins, plugin );
#endif /* NOODX */

    plugin = g_malloc0 ( sizeof ( gsb_plugin ) );
    plugin -> name = "openssl";
    plugin -> plugin_register = &openssl_plugin_register;
    plugin -> plugin_run =      &openssl_plugin_run;
    plugin -> plugin_register ();
    plugins = g_slist_append ( plugins, plugin );
#else /* ENABLE_STATIC */
    GDir * plugin_dir;
    const gchar * filename;
    gchar * plugin_name;

    plugin_dir = g_dir_open ( dirname, 0, NULL );
    if ( ! plugin_dir )
	return;
    
    while ( ( filename = g_dir_read_name ( plugin_dir ) ) != NULL )
    {
	gchar * complete_filename, * tmp;
	gchar ** split_filename;
	gsb_plugin * plugin = g_malloc0 ( sizeof ( gsb_plugin ) );

	split_filename = g_strsplit(filename, ".", 2);

	if (!split_filename[1])
		continue;

	if ( strncmp ( split_filename[1], G_MODULE_SUFFIX, strlen(G_MODULE_SUFFIX) ) )
	    continue;

	complete_filename = g_build_filename ( dirname, filename, NULL );

	if ( ! ( plugin -> handle = 
		 g_module_open (complete_filename, 0 ) ) )
	{
	    gchar* tmpstr = g_strdup_printf ( "Couldn't load module %s: %s", filename,
					       g_module_error() );
	    dialogue_error ( tmpstr );
	    g_free ( tmpstr );
	    g_free ( plugin );
	    g_free ( complete_filename);
	    continue;
	}
	
	g_free (complete_filename);

	if ( ! g_module_symbol ( plugin -> handle, "plugin_name",
				 (gpointer) &plugin_name ) )
	{
	    gchar* tmpstr = g_strdup_printf ( "Plugin %s has no register symbol", 
					       filename );
	    dialogue_error ( tmpstr );
	    g_free ( tmpstr );
	    g_free ( plugin );
	    continue;
	}
	plugin -> name = plugin_name;

	tmp = g_strconcat ( plugin_name, "_plugin_register", NULL );
	if ( ! g_module_symbol ( plugin -> handle, tmp,
				 (gpointer)  &( plugin -> plugin_register ) ) )
	{
	    gchar* tmpstr = g_strdup_printf ( "Plugin %s has no register symbol", 
					       filename );
	    dialogue_error ( tmpstr );
	    g_free ( tmpstr );
	    g_free ( plugin );
	    continue;
	}
	g_free ( tmp );

	plugin -> plugin_register ();

	tmp = g_strconcat ( plugin_name, "_plugin_run", NULL );
	if ( ! g_module_symbol ( plugin -> handle, tmp,
				 (gpointer) &( plugin -> plugin_run ) ) )
	{
	    gchar* tmpstr = g_strdup_printf ( "Plugin %s has no run symbol", 
					       filename );
	    dialogue_error ( tmpstr );
	    g_free ( tmpstr );
	    g_free ( plugin );
	    continue;
	}
	g_free ( tmp );

	plugins = g_slist_append ( plugins, plugin );
    }

    g_dir_close ( plugin_dir );
#endif /* ENABLE_STATIC */
}



/**
 *
 *
 *
 */
gsb_plugin * gsb_plugin_find ( gchar * plugin_name )
{
    GSList * tmp = plugins;

    g_return_val_if_fail ( plugin_name, NULL );
    
    while ( tmp )
    {
	gsb_plugin * plugin = (gsb_plugin *) tmp -> data;

	if ( ! strcmp ( plugin_name, plugin -> name ) )
	{
	    return plugin;
	}

	tmp = tmp -> next;
    }

    return NULL;
}



/**
 * Get activated plugins.
 *
 * \return A newly-allocated string representing activated plugins.
 */
gchar * gsb_plugin_get_list ()
{
    gchar * list = NULL;
    gchar * old_list = NULL;
    GSList * tmp = plugins;

    while ( tmp )
    {
	gsb_plugin * plugin = (gsb_plugin *) tmp -> data;

	if ( ! list )
	{
	    list = g_strconcat ( _("with plugins"), " ", plugin -> name, NULL );
	}
	else
	{
	    old_list = list;
	    list = g_strconcat ( list, ", ", plugin -> name, NULL );
	    g_free(old_list);
	}

	tmp = tmp -> next;
    }
    if (! list)
    	list = g_strdup("no plugin");

    return list;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
