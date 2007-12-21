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

#include "include.h"

/*START_INCLUDE*/
#include "gsb_plugins.h"
#include "./dialog.h"
#include "./gsb_file_config.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_EXTERN*/
extern G_MODULE_EXPORT const gchar plugin_name[] ;
extern G_MODULE_EXPORT const gchar plugin_name[] ;
extern G_MODULE_EXPORT const gchar plugin_name[] ;
extern G_MODULE_EXPORT const gchar plugin_name[] ;
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/


/** List of registered plugins.  It should contain gsb_plugin structures. */
GSList * plugins = NULL;


/**
 *
 *
 */
void gsb_plugins_scan_dir ( const char *dirname )
{
    GDir * plugin_dir;
    const gchar * filename;
    gchar * plugin_name;

    plugin_dir = g_dir_open ( dirname, 0, NULL );
    if ( ! plugin_dir )
	return;
    
    while ( ( filename = g_dir_read_name ( plugin_dir ) ) != NULL )
    {
	gchar * complete_filename, * tmp;
	gsb_plugin * plugin = g_malloc0 ( sizeof ( gsb_plugin ) );

	if ( strncmp ( filename + ( strlen ( filename ) - 3 ), ".so", 3 ) )
	    continue;

	complete_filename = g_strconcat ( PLUGINS_DIR, C_DIRECTORY_SEPARATOR,
					  filename, NULL );

	if ( ! ( plugin -> handle = 
		 g_module_open (complete_filename, 0 ) ) )
	{
	    dialogue_error ( g_strdup_printf ( "Couldn't load module %s: %s", filename,
					       g_module_error() ) );
	    g_free ( plugin );
	    continue;
	}
	
	if ( ! g_module_symbol ( plugin -> handle, "plugin_name",
				 (gpointer) &plugin_name ) )
	{
	    dialogue_error ( g_strdup_printf ( "Plugin %s has no register symbol", 
					       filename ) );
	    g_free ( plugin );
	    continue;
	}
	plugin -> name = plugin_name;

	tmp = g_strconcat ( plugin_name, "_plugin_register", NULL );
	if ( ! g_module_symbol ( plugin -> handle, tmp,
				 (gpointer)  &( plugin -> plugin_register ) ) )
	{
	    dialogue_error ( g_strdup_printf ( "Plugin %s has no register symbol", 
					       filename ) );
	    g_free ( plugin );
	    continue;
	}
	g_free ( tmp );

	plugin -> plugin_register ();

	tmp = g_strconcat ( plugin_name, "_plugin_run", NULL );
	if ( ! g_module_symbol ( plugin -> handle, tmp,
				 (gpointer) &( plugin -> plugin_run ) ) )
	{
	    dialogue_error ( g_strdup_printf ( "Plugin %s has no run symbol", 
					       filename ) );
	    g_free ( plugin );
	    continue;
	}
	g_free ( tmp );

	tmp = g_strconcat ( plugin_name, "_plugin_release", NULL );
	if ( ! g_module_symbol ( plugin -> handle, tmp,
				 (gpointer) &( plugin -> plugin_release ) ) )
	{
	    dialogue_error ( g_strdup_printf ( "Plugin %s has no release symbol", 
					       filename ) );
	    g_free ( plugin );
	    continue;
	}
	g_free ( tmp );

	plugins = g_slist_append ( plugins, plugin );

    }

    g_dir_close ( plugin_dir );
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



/**
 *
 *
 *
 */
void gsb_plugins_release ( )
{
    GSList * tmp = plugins;

    while ( tmp )
    {
	gsb_plugin * plugin = (gsb_plugin *) tmp -> data;

	plugin -> plugin_release ();

	tmp = tmp -> next;
    }        
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
