/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                               */
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

#include "config.h"
#endif

#include "include.h"
#include <gtk/gtk.h>
#include <libintl.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>
#ifdef __APPLE__
#   include <mach-o/dyld.h> // for _NSGetExecutablePath
#endif
#endif /* HAVE_GOFFICE */

#ifdef G_OS_WIN32
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <unistd.h>
#endif /* WIN32 */

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "gsb_locale.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/



#ifdef __APPLE__
static char const *get_program_name(void) {
    gchar *program_name="undefined";

    char pathbuf[PATH_MAX + 1];
    uint32_t bufsize = sizeof(pathbuf);

    if (_NSGetExecutablePath(pathbuf, &bufsize) == 0) {
        program_name = realpath(pathbuf, NULL);
    } else {
        g_warning("get_program_name() - _NSGetExecutablePath failed");
    }

    return program_name;
}

static char const *get_program_dir()
{
    return g_path_get_dirname(get_program_name());
}



static gchar *get_bundle_prefix(void) {
    char const *program_dir = get_program_dir();

    gchar *prefix = g_path_get_dirname(program_dir);
    if (g_str_has_suffix(program_dir, "Contents/MacOS")) {
        prefix = g_build_filename(prefix, "Resources", NULL);
    }

    return prefix;
}

static void my_setenv(gchar *key, gchar *value) {
    printf("SET %s = %s\n", key, value);
    g_setenv(key, value, TRUE);
}

static void set_macos_app_bundle_env(gchar const *program_dir)
{
    // use bundle identifier
    // https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/MacOSXDirectories/MacOSXDirectories.html
    gchar *app_support_dir = g_build_filename(g_getenv("HOME"), "Library/Application Support/Grisbi", NULL);

    gchar *bundle_resources_dir       = get_bundle_prefix();
    gchar *bundle_resources_etc_dir   = g_build_filename(bundle_resources_dir, "etc", NULL);
    //gchar *bundle_resources_bin_dir   = g_build_filename(bundle_resources_dir, "bin", NULL);
    gchar *bundle_resources_lib_dir   = g_build_filename(bundle_resources_dir, "lib", NULL);
    gchar *bundle_resources_share_dir = g_build_filename(bundle_resources_dir, "share", NULL);

    printf("PREFIX=%s\n", bundle_resources_dir);
    // failsafe: Check if the expected content is really there, using GIO modules
    // as an indicator.
    // This is also helpful to developers as it enables the possibility to
    //      1. cmake -DCMAKE_INSTALL_PREFIX=Inkscape.app/Contents/Resources
    //      2. move binary to Inkscape.app/Contents/MacOS and set rpath
    //      3. copy Info.plist
    // to ease up on testing and get correct application behavior (like dock icon).
    if (! g_file_test(g_build_filename(bundle_resources_lib_dir, "goffice", NULL), G_FILE_TEST_EXISTS)) {
        // doesn't look like a standalone bundle
        return;
    }

    // XDG
    // https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    my_setenv("XDG_DATA_HOME",   g_build_filename(app_support_dir, "share", NULL));
    my_setenv("XDG_DATA_DIRS",   bundle_resources_share_dir);
    my_setenv("XDG_CONFIG_HOME", g_build_filename(app_support_dir, "config", NULL));
    my_setenv("XDG_CONFIG_DIRS", g_build_filename(bundle_resources_etc_dir, "xdg", NULL));
    my_setenv("XDG_CACHE_HOME",  g_build_filename(app_support_dir, "cache", NULL));

    // GTK
    // https://developer.gnome.org/gtk3/stable/gtk-running.html
    my_setenv("GTK_EXE_PREFIX",  bundle_resources_dir);
    my_setenv("GTK_DATA_PREFIX", bundle_resources_dir);

    // GDK
    my_setenv("GDK_PIXBUF_MODULE_FILE", g_build_filename(bundle_resources_lib_dir, "gdk-pixbuf-2.0/2.10.0/loaders.cache", NULL));

    // fontconfig
    my_setenv("FONTCONFIG_PATH", g_build_filename(bundle_resources_etc_dir, "fonts", NULL));

    // GIO
    my_setenv("GIO_MODULE_DIR", g_build_filename(bundle_resources_lib_dir, "gio/modules", NULL));

    // GNOME introspection
    my_setenv("GI_TYPELIB_PATH", g_build_filename(bundle_resources_lib_dir, "girepository-1.0", NULL));

    // PATH
    //g_setenv("PATH", bundle_resources_bin_dir + ":" + Glib::getenv("PATH"));

    // DYLD_LIBRARY_PATH
    // This is required to make Python GTK bindings work as they use dlopen()
    // to load libraries.
    //g_setenv("DYLD_LIBRARY_PATH", bundle_resources_lib_dir + ":"
    //        + bundle_resources_lib_dir + "/gdk-pixbuf-2.0/2.10.0/loaders");
}
#endif

/**
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
int main (int argc, char **argv)
{
    GrisbiApp *app;
	gint status;

	/* On force l'utilisation de X11 en attendant que grisbi fonctionne correctement sous wayland */
/*#ifdef GDK_WINDOWING_WAYLAND
	#ifdef GDK_WINDOWING_X11
		gdk_set_allowed_backends ("x11");
	#else
		return (1);
	#endif
#endif
*/

#ifdef __APPLE__
    char const *program_dir = get_program_dir();
    printf("program_dir = %s\n", program_dir);
    if (g_str_has_suffix(program_dir, "Contents/MacOS")) {

        // Step 1
        // Remove macOS session identifier from command line arguments.
        // Code adopted from GIMP's app/main.c

        int new_argc = 0;
        for (int i = 0; i < argc; i++) {
            // Rewrite argv[] without "-psn_..." argument.
            if (!g_str_has_prefix(argv[i], "-psn_")) {
                argv[new_argc] = argv[i];
                new_argc++;
            }
        }
        if (argc > new_argc) {
            argv[new_argc] = NULL; // glib expects null-terminated array
            argc = new_argc;
        }

        // Step 2
        // In the past, a launch script/wrapper was used to setup necessary environment
        // variables to facilitate relocatability for the application bundle. Starting
        // with Catalina, this approach is no longer feasible due to new security checks
        // that get misdirected by using a launcher. The launcher needs to go and the
        // binary needs to setup the environment itself.

        set_macos_app_bundle_env(program_dir);
    }
#endif

	/* On commence par initialiser les répertoires */
    gsb_dirs_init (argv[0]);

    /* Setup locale/gettext */
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, gsb_dirs_get_locale_dir ());
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);



#ifdef HAVE_GOFFICE
    /* initialisation libgoffice */
    libgoffice_init ();
    /* Initialize plugins manager */

#ifdef __APPLE__
    gchar *local_plugins_dir = g_build_filename(g_path_get_dirname(argv[0]), "/../Resources/lib/goffice/0.10.49/plugins/", NULL);
    GSList *plugins_dirs = g_slist_prepend(NULL, local_plugins_dir);
    go_plugins_init (NULL, NULL, NULL, plugins_dirs, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
#else
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
#endif
#endif /* HAVE_GOFFICE */

    app = g_object_new (GRISBI_APP_TYPE,
                        "application-id",   "org.gtk.grisbi",
                        "flags",            G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_NON_UNIQUE,
                        "register-session", TRUE,
                        NULL);

    /* option en attente : G_APPLICATION_HANDLES_OPEN,voir GApplicationFlags pour détails */

    status = g_application_run (G_APPLICATION (app), argc, argv);

    g_object_unref (app);

#ifdef HAVE_GOFFICE
    /* liberation libgoffice */
    libgoffice_shutdown ();
#ifdef __APPLE__
    g_slist_free(plugins_dirs);
#endif
#endif /* HAVE_GOFFICE */

    return status;
}


/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
