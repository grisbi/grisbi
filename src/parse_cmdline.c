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

#include "include.h"

/*START_INCLUDE*/
#include "parse_cmdline.h"
#include "main.h"
#include "utils_str.h"
#include "include.h"
/*END_INCLUDE*/

/* Definitions des options (à ne pas traduire)                               */

#define  LONG_OPT_ONGLET     "--onglet="
#define  LONG_OPT_HELP       "--help"
#define  LONG_OPT_VERSION    "--version"

#define  SHORT_OPT_ONGLET    "-o"
#define  SHORT_OPT_HELP      "-?"
#define  SHORT_OPT_VERSION   "-v"

/*START_STATIC*/
static gboolean  is_option(gchar *argv, gchar *o);
static gboolean  is_option_with_arg(gchar *argv, gchar *o);
static gboolean  is_valid_window_number(gint w);
static gboolean  parse_onglet(char **argv, int argc, int *no, cmdline_options* pOpt);
static gboolean   show_help();
static gboolean   show_usage();
static gboolean   show_version();
/*END_STATIC*/



/*****************************************************************************/
/* Parse les options 									                     */
/*                   													     */
/* - si les options comportent --version, on affiche la version et on sort   */
/*   (paramètres restant ignorés)                                            */
/* - si les options comportent --help, on affiche l'aide et on sort          */
/*   (paramètres restant ignorés)                                            */
/*****************************************************************************/

gboolean  parse_options(int argc, char **argv, cmdline_options *pOpt)
{
	gint i;
	
	/* init param de retour */
	pOpt->demande_page         = 0;
	pOpt->fichier              = NULL;
	pOpt->report_no            = -1;
	pOpt->customization_tab_no = -1;
	pOpt->subcustom_tab_no     = -1;

	for (i = 1 ; i < argc ; i++ ) 
	{
		if (argv[i][0] == '-' ) 
		{
			/* Traite les options (ie parametres qui commencent par '-') */
			
			if ( is_option(argv[i], LONG_OPT_HELP) || is_option(argv[i], SHORT_OPT_HELP) )
			{
				return show_help();
			} 
			else if ( is_option(argv[i], LONG_OPT_VERSION) || is_option(argv[i], SHORT_OPT_VERSION) ) 
			{
				return show_version();
			} 
			else if ( is_option(argv[i], SHORT_OPT_ONGLET) || is_option_with_arg(argv[i], LONG_OPT_ONGLET) )
			{
				if (!parse_onglet(argv, argc, &i, pOpt))
				{
					printf(N_("Bad '--onglet' options"));
					return show_usage();
				}
			}
			else
			{
				/* option non reconnue */
				printf(N_("Unrecognized option: %s\n"), argv[i]);
				return show_usage();
			}
		} 
		else 
		{
			/* Ca doit être le nom du fichier (forcement le dernier paramètre, sinon erreur) */
			if (i != argc-1 )
			{
				return show_usage();
			}
			pOpt->fichier = argv[i];
		}
		
	}
	
	return FALSE;

}


/*****************************************************************************/
gboolean   show_version()
{
	printf(N_("grisbi version %s\n"), VERSION);	
	printf(N_("Personal finance tracking program based on GTK.\n\n"));
	return TRUE;
}


/*****************************************************************************/
#define   HELP_STRING  				                                           \
 N_("gribi [options] [file]\n"                                                 \
    "Personal finance tracking program based on GTK.\n"                        \
    "  -?, --help                    Show this help message and exit\n"        \
    "  -v, --version                 Show version number and exit\n"           \
    "  -o, --onglet=w[,x[,y[,z]]]    Open file and show specified tab\n"       \
    "              w=-1              Show configuration window\n"              \
    "              w=[0-6]           Show tab no <w>\n"                        \
    "              w=7,x[,y[,z]]]    Show report pane\n"                       \
    "                                    x: report number to show\n"           \
    "                                    y: show main customization tab\n"     \
    "                                       number <y> of report\n"            \
    "                                    z: show custmization tab number <z>\n"\
    "                                       of report\n\n"                     \
 )

gboolean   show_help()
{
	printf(HELP_STRING);
	return TRUE;
}


/*****************************************************************************/
#define   USAGE_STRING  		        						\
 N_("Usage: gribi [--version | -v] [--help | -?] " 				\
	"[--onglet=w[,x[,y[,z]]] | -o w[,x[,y[,z]]]] [file]\n"      \
	"Type grisbi --help for details\n\n"                        \
 )

gboolean   show_usage()
{
	printf(USAGE_STRING);
	return TRUE;
}


/*****************************************************************************/
gboolean  parse_onglet(char **argv, int argc, int *no, cmdline_options* pOpt)
{
	gchar **split_chiffres;
	gint    w, x, y, z;
	char   *arg0 = argv[*no];
	
	w = x = y = z = -1;

	/* se positionne sur les données significatives */
	if (arg0[1] == '-') 
	{
		/* version longue, se place après le "=" */
		arg0 += strlen(LONG_OPT_ONGLET);
	}
	else 
	{
		/* version courte */
		if (*no == argc - 1)
		{
			/* erreur si "-o" est le dernier argument */
			return FALSE;
		}
		(*no)++; /* on se place sur l'argument suivant */
		arg0 = argv[*no];
	}

	/* arg0 pointe sur le début des donnees de l'option */
	
	split_chiffres = g_strsplit ( arg0, ",", 0 );
	
	if (split_chiffres[0] == NULL)
	{
		/* on s'attend à avoir au moins un numero d'onglet */
		return FALSE;
	}
	
	w = utils_str_atoi(split_chiffres[0]); 
	if ( !is_valid_window_number(w) )
	{
		return FALSE;
	}

	if (w == 7)
	{
		/* Fenetre des etats. Lit parametres restant si il y en a. */
		if (split_chiffres[1]) 
		{
			x = utils_str_atoi(split_chiffres[1]);
		
			if (split_chiffres[2]) 
			{
				y = utils_str_atoi(split_chiffres[2]);
				if (split_chiffres[3]) 
				{
					z = utils_str_atoi(split_chiffres[3]);
				}
			}
		}
	}
	

	/* Tout c'est bien passé, on peut ranger les valeurs dans la var. de retour */
	pOpt->demande_page         = 1;
	
	pOpt->page_w               = w;
	pOpt->report_no            = x;
	pOpt->customization_tab_no = y;
	pOpt->subcustom_tab_no     = z;
	
	return TRUE;
	
}


/*****************************************************************************/
#define  NB_MAX_ONGLET    8 

gboolean  is_valid_window_number(gint w)
{
	/* Valeurs valides:
	 * 1)  -1: fenêtre de configuration
	 * 2)  0..NB_MAX_ONGLET-1: onglet
	 */
	return ((w >= -1) && (w < NB_MAX_ONGLET));
}

/*****************************************************************************/
/* teste si argv est l'option 'o' (sans argument) */
gboolean  is_option(gchar *argv, gchar *o)
{
	return strcmp(argv, o) == 0;
}

/* teste si argv est l'option 'o' avec des arguments (c'est-à-dire <option>=<args>) */
gboolean  is_option_with_arg(gchar *argv, gchar *o)
{
	return (strncmp(argv, o, strlen(o)) == 0) && (argv[strlen(o)-1] == '=');
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
