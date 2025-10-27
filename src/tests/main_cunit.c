/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*                      https://www.grisbi.org/                                   */
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
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/


#include "config.h"

#include "include.h"

/*START_INCLUDE*/
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <gtk/gtk.h>
#include "gsb_data_account_cunit.h"
#include "gsb_real_cunit.h"
#include "utils_dates_cunit.h"
#include "utils_real_cunit.h"
#include "structures.h"
/*END_INCLUDE*/


/* Following declarations avoid link errors.
 * Everything should be removed as soon as core functions and GUI functions
 * are properly separated and a libgrisbi_core.a library is generated.
 */
GtkWidget *window = NULL;
#if 0
gboolean grisbi_app_quit( void )
{
	return FALSE;
}
gboolean gsb_main_set_grisbi_title ( gint account_number )
{
	return FALSE;
}
gchar *gsb_main_get_print_locale_var(void) { return NULL; }
gchar *gsb_main_get_print_dir_var(void) { return NULL; }
#endif

/* End of unnecessary things */


int main(int argc, char** argv)
{
	CU_initialize_registry();

	utils_real_cunit_create_suite();
	utils_dates_cunit_create_suite();
	gsb_data_account_cunit_create_suite();
	gsb_real_cunit_create_suite();

	CU_basic_run_tests();

	CU_cleanup_registry();
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
