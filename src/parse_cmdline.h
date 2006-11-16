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
#ifndef   __PARSE_CMDLINE_H
#define   __PARSE_CMDLINE_H (1)
/*START_INCLUDE*/
#include <glib.h>
/*END_INCLUDE*/

typedef struct _cmdline_options 
{

    gint        demande_page;
    gint        page_w;
    
    /* quand page_w correspond à la page des états */
    gint        report_no;            /* no etat. -1 si non spécifié dans les options */
    gint        customization_tab_no; /* onglet de personalisation. -1 si non spécifié dans les options */
    gint        subcustom_tab_no;     /* sous-onglet de personalisation. -1 si non spécifié dans les options */
    
    gchar      *fichier;
    
} cmdline_options;

typedef enum
{
    CMDLINE_SYNTAX_OK = 0,
    CMDLINE_MISSING_PARAMETER,
    CMDLINE_SYNTAX_ERROR,
    CMDLINE_UNKNOWN_OPTION,
    CMDLINE_TOO_MANY_PARAMETERS,
    CMDLINE_TAB_ID_OUT_OF_RANGE,
    CMDLINE_GETOPT_CONFIGURATION_ERROR,
    CMDLINE_OTHER
} CMDLINE_ERRNO;
#define CMDLINE_ERROR(err)  (gint)(-err)


/*START_DECLARATION*/
gboolean  parse_options(int argc, char **argv, cmdline_options *pOpt, gint* pErrval);
/*END_DECLARATION*/


#endif  /* ifndef __PARSE_CMDLINE_H */


