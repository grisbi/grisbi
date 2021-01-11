/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
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

#import <Foundation/NSUserDefaults.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/NSString.h>
#include <mach-o/dyld.h> // for _NSGetExecutablePath

#include "grisbi_osx.h"
#include "erreur.h"


/** 
 * 
 */
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

static void my_setenv(const gchar *key, const gchar *value) {
    char set_env[255];

    snprintf(set_env, sizeof(set_env), "SET: %s = %s", key, value);
    devel_debug(set_env);
    g_setenv(key, value, TRUE);
}

static gchar *set_macos_app_bundle_env(gchar const *program_dir)
{
    // use bundle identifier
    // https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/MacOSXDirectories/MacOSXDirectories.html
    gchar *app_support_dir = g_build_filename(g_getenv("HOME"), "Library/Application Support/Grisbi", NULL);

    gchar *bundle_resources_dir       = get_bundle_prefix();
    gchar *bundle_resources_etc_dir   = g_build_filename(bundle_resources_dir, "etc", NULL);
    gchar *bundle_resources_bin_dir   = g_build_filename(bundle_resources_dir, "bin", NULL);
    gchar *bundle_resources_lib_dir   = g_build_filename(bundle_resources_dir, "lib", NULL);
    gchar *bundle_resources_share_dir = g_build_filename(bundle_resources_dir, "share", NULL);

    // failsafe: Check if the expected content is really there, using GIO modules
    // as an indicator.
    // This is also helpful to developers as it enables the possibility to
    //      1. cmake -DCMAKE_INSTALL_PREFIX=Inkscape.app/Contents/Resources
    //      2. move binary to Inkscape.app/Contents/MacOS and set rpath
    //      3. copy Info.plist
    // to ease up on testing and get correct application behavior (like dock icon).
    if (! g_file_test(g_build_filename(bundle_resources_lib_dir, "goffice", NULL), G_FILE_TEST_EXISTS)) {
        // doesn't look like a standalone bundle
        return NULL;
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
    my_setenv("PATH", g_build_path(":", bundle_resources_bin_dir, g_getenv("PATH"), NULL));

    // DYLD_LIBRARY_PATH
    // This is required to make Python GTK bindings work as they use dlopen()
    // to load libraries.
    my_setenv("DYLD_LIBRARY_PATH", g_build_path(":", bundle_resources_lib_dir, NULL));

    return bundle_resources_dir;
}

static void set_locale(void) {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSString* lang = [defaults stringForKey:@"AppleLocale"];

	const char *lang_str = [lang UTF8String];
    my_setenv("LANG", lang_str);
}

GSList *grisbi_osx_init(int *argc, char **argv[]) {
    char const *program_dir = get_program_dir();
    GSList *goffice_plugins_dirs = NULL;
    gchar *bundle_resources_dir = NULL;

    devel_debug("MACOSX: Start initialization");
    if (g_str_has_suffix(program_dir, "Contents/MacOS")) {

        // Step 1
        // Remove macOS session identifier from command line arguments.
        // Code adopted from GIMP's app/main.c

        int new_argc = 0;
        for (int i = 0; i < *argc; i++) {
            // Rewrite argv[] without "-psn_..." argument.
            if (!g_str_has_prefix(*argv[i], "-psn_")) {
                *argv[new_argc] = *argv[i];
                new_argc++;
            }
        }
        if (*argc > new_argc) {
            *argv[new_argc] = NULL; // glib expects null-terminated array
            *argc = new_argc;
        }

        // Step 2
        // In the past, a launch script/wrapper was used to setup necessary environment
        // variables to facilitate relocatability for the application bundle. Starting
        // with Catalina, this approach is no longer feasible due to new security checks
        // that get misdirected by using a launcher. The launcher needs to go and the
        // binary needs to setup the environment itself.
        bundle_resources_dir = set_macos_app_bundle_env(program_dir);
    } else {
        devel_debug("Running outside bundle");
    }
    set_locale();


#ifdef HAVE_GOFFICE
    if (bundle_resources_dir) {
        gchar *local_plugins_dir = g_build_filename(bundle_resources_dir,
                                                    "/lib/goffice/0.10.49/plugins/", NULL);
        goffice_plugins_dirs = g_slist_prepend(NULL, local_plugins_dir);
        devel_debug(local_plugins_dir);
    }
#endif

    devel_debug("MACOSX: initialization done.");

    return goffice_plugins_dirs;
}

