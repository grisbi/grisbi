/* *******************************************************************************/
/*                                 GRISBI                                        */
/* Programme de gestion financière personnelle                                   */
/*           	  license : GPL                                                  */
/*                                                                               */
/*     Copyright (C)    2000-2006 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2006 Benjamin Drieu (bdrieu@april.org)              */
/*                      http://www.grisbi.org                                    */
/*      Version : 0.6.0                                                          */
/*                                                                               */
/* *******************************************************************************/

/* *******************************************************************************/
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


/* Fichier de base contenant la procédure main */


#include "include.h"

/*START_INCLUDE*/
#include "main.h"
#include "menu.h"
#include "dialog.h"
#include "tip.h"
#include "erreur.h"
#include "gsb_file_config.h"
#include "fichiers_gestion.h"
#include "gsb_status.h"
#include "gsb_plugins.h"
#include "traitement_variables.h"
#include "comptes_traitements.h"
#include "parse_cmdline.h"
#include "import.h"
#include "utils_files.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/




/* vbox ajoutée dans la fenetre de base, contient le menu et la fenetre d'utilisation */

GtkWidget *window;
GtkWidget *window_vbox_principale;
GtkItemFactory *item_factory_menu_general;
gint id_fonction_idle;


/*START_EXTERN*/
extern gint hauteur_window;
extern gint largeur_window;
extern gchar *nom_fichier_comptes;
/*END_EXTERN*/


/**                                                                              
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
int main (int argc, char *argv[])
{
    GtkWidget * statusbar;
    gboolean first_use = FALSE;
    gsb_plugin * plugin;

    initialize_debugging();

#ifndef _WIN32
    struct sigaction sig_sev;
#endif
    struct stat buffer_stat;
    cmdline_options  opt;


#ifdef _WIN32
    /* Retrieve exception information and store them under grisbi.rpt file!
     * see http://jrfonseca.dyndns.org/projects/gnu-win32/software/drmingw/index.html for more information */
    LoadLibrary("exchndl.dll");

    /* we store the path of the running file to use it for pixmaps, help and locales .... */
    win32_set_app_path(argv[0]);
    /* needed to be able to use the "common" installation of GTK libraries */
    win32_make_sure_the_gtk2_dlls_path_is_in_PATH(); 
#endif

    bindtextdomain ("grisbi-tips", LOCALEDIR);
    bind_textdomain_codeset ("grisbi-tips", "UTF-8");

    bindtextdomain (PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain (PACKAGE);

    if ( setlocale ( LC_MONETARY, getenv ( "LC_MONETARY" ) ) == NULL )
    {
	setlocale ( LC_MONETARY, getenv ( "LANG" ) );
    }

    gtk_init(&argc, &argv);

    /* on commence par détourner le signal SIGSEGV */
#ifndef _WIN32
    /* sauf sous Windows*/
    memset ( &sig_sev, 0, sizeof ( struct sigaction ));
    sig_sev.sa_handler = traitement_sigsegv;
    sig_sev.sa_flags = 0;
    sigemptyset (&(sig_sev.sa_mask));

    if ( sigaction ( SIGSEGV, &sig_sev, NULL ))
	printf (_("Error on sigaction: SIGSEGV won't be trapped\n"));
#endif

    /* parse command line parameter, exit with correct error code when needed */
    {
        CMDLINE_ERRNO status = CMDLINE_SYNTAX_OK;/* be optimistic ;-) */
        if (!parse_options(argc, argv, &opt,&status))
        {
            exit(status);
        }
    }

#ifdef HAVE_PLUGINS
    gsb_plugins_scan_dir ( PLUGINS_DIR );
#endif

    /* create the icon of grisbi (set in the panel of gnome or other) */

    if ( utf8_stat ( g_strconcat ( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR, "grisbi.png", NULL ),&buffer_stat ) != -1 )
	gtk_window_set_default_icon_from_file ( g_strconcat(PIXMAPS_DIR,
							    C_DIRECTORY_SEPARATOR,
							    "grisbi.png",
							    NULL),
						NULL );

    /* initialisation of the variables */
    init_variables ();
    register_import_formats ();

    if ( ! gsb_file_config_load_config () )
    {
	first_use = TRUE;
    }
    initialisation_couleurs_listes ();

    /* create the toplevel window */

    window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
    gtk_signal_connect ( GTK_OBJECT (window),
			 "delete_event",
			 GTK_SIGNAL_FUNC ( fermeture_grisbi ),
			 NULL );
    gtk_window_set_policy ( GTK_WINDOW ( window ),
			    TRUE,
			    TRUE,
			    FALSE );

    /* create the main window : a vbox */
    window_vbox_principale = gtk_vbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( window ),
			window_vbox_principale );
    gtk_widget_show ( window_vbox_principale );

    /* We create the statusbar first. */
    statusbar = gsb_new_statusbar ();
    gtk_box_pack_end ( GTK_BOX(window_vbox_principale),
		       statusbar,
		       FALSE, FALSE, 0 );

    /* create the menus */
    init_menus ( window_vbox_principale );

    /* unsensitive the necessaries menus */
    menus_sensitifs ( FALSE );

    /* set the last opened files */
    affiche_derniers_fichiers_ouverts ();

    /* set the size of the window */
    if ( largeur_window && hauteur_window )
	gtk_window_set_default_size ( GTK_WINDOW ( window ),
				      largeur_window, hauteur_window );
    else
	gtk_window_set_default_size ( GTK_WINDOW ( window ),
				      800, 600 );

    gtk_widget_show ( window );

    /* check the command line, if there is something to open */

    if (opt.fichier) 
    {
	nom_fichier_comptes = opt.fichier;

	if (!gsb_file_open_file(nom_fichier_comptes))
	    nom_fichier_comptes = NULL;
    }
    else 
    {
	/* open the last file if needed, nom_fichier_comptes was filled while loading the configuration */
	if ( etat.dernier_fichier_auto
	     &&
	     nom_fichier_comptes )
	    if (!gsb_file_open_file(nom_fichier_comptes))
		nom_fichier_comptes = NULL;
    }

#ifdef IS_DEVELOPMENT_VERSION
    
dialogue_hint("Warning, please be aware that the version you run is a DEVELOPMENT version.\n \
In any case you do work with this version on your original accounting files.\n \
(File format may change and set the files incompatible with previous version)\n ",VERSION);
#endif

    if ( first_use )
    {
	dialogue_hint ( _("You can now create your account file ... blah blah.  This will be replaced by a nice assistant."),
			_("Welcome to Grisbi!"));
	new_account ();
    }
    else
    {
	display_tip ( FALSE );
    }
    gtk_main ();

    gsb_plugins_release ( );
    exit(0);
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
