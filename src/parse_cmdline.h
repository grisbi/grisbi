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

/*START_DECLARATION*/
#ifndef   __PARSE_CMDLINE_H
#define   __PARSE_CMDLINE_H

struct _cmdline_options 
{

    gint        demande_page;
    gint        page_w;
    
    /* quand page_w correspond à la page des états */
    gint        report_no;            /* no etat. -1 si non spécifié dans les options */
    gint        customization_tab_no; /* onglet de personalisation. -1 si non spécifié dans les options */
    gint        subcustom_tab_no;     /* sous-onglet de personalisation. -1 si non spécifié dans les options */
    
    gchar      *fichier;
    
};
typedef struct _cmdline_options cmdline_options;

/* Parse les paramètres de la ligne de commande et les range dans pOpt             */
/* renvoie true si les paramètres doivent conduire à l'arrêt du programme          */
/* (affichage numéro de version ou erreur de paramètres par exemple), false si     */
/* on continue                                                                     */

gboolean  parse_options(int argc, char **argv, cmdline_options *pOpt);

#endif  /* ifndef __PARSE_CMDLINE_H */
/*END_DECLARATION*/
