/* ************************************************************************** */
/* Fichier qui s'occupe d'afficher les états                                  */
/*                                                                            */
/*                            etats_affiche.c                                 */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/*			2005 Alain Portal (aportal@univ-montp2.fr)	      */
/*			http://www.grisbi.org				      */
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
#include "structures.h"
#include "variables-extern.c"
#include "etats_affiche.h"

#include "devises.h"
#include "etats_calculs.h"
#include "etats.h"
#include "etats_support.h"
#include "search_glist.h"
#include "utils.h"
#include "exercice.h"





/*****************************************************************************************************/
gint etat_affiche_affiche_titre ( gint ligne )
{
    gchar *titre;

    titre = etats_titre () ;

    etat_affiche_attach_label ( titre, TEXT_BOLD | TEXT_HUGE, 0, nb_colonnes,
				ligne, ligne + 1, LEFT, NULL );

    return 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affiche_separateur ( gint ligne )
{
    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    return ligne + 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les catégories sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_categories ( gint ligne )
{
    char * text;

    if ( etat_courant -> utilise_categ
	 &&
	 etat_courant -> affiche_sous_total_categ )
    {
	/* si rien n'est affiché en dessous de la catég, on */
	/* met le résultat sur la ligne de la catég */
	/* sinon on fait une barre et on met le résultat */

	if ( etat_courant -> afficher_sous_categ
	     ||
	     etat_courant -> utilise_ib
	     ||
	     etat_courant -> regroupe_ope_par_compte
	     ||
	     etat_courant -> utilise_tiers
	     ||
	     etat_courant -> afficher_opes )
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes -1, ligne, ligne + 1, LEFT, NULL );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_categ_en_cours )
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_categ_etat <= 1 )
			text = g_strdup_printf ( _("Total %s (%d transaction)"), nom_categ_en_cours, nb_ope_categ_etat );
		    else
			text = g_strdup_printf ( _("Total %s (%d transactions)"), nom_categ_en_cours, nb_ope_categ_etat );
		}
		else
		    text =g_strconcat ( POSTSPACIFY(_("Total")), nom_categ_en_cours, NULL );
	    }
	    else
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_categ_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Category total (%d transaction)")), nb_ope_categ_etat);
		    else
			text = g_strdup_printf ( COLON(_("Category total (%d transactions)")), nb_ope_categ_etat);
		}
		else
		    text = COLON(_("Category total"));
	    }
	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1,
					ligne, ligne + 1, LEFT, NULL );

	    text = g_strdup_printf ( _("%4.2f %s"), montant_categ_etat, devise_name ( devise_categ_etat ) );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes,
					ligne, ligne + 1, RIGHT, NULL );

	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes -1, ligne, ligne + 1, LEFT, NULL );

	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_categ_etat <= 1 )
		    text = g_strdup_printf ( _("%4.2f %s (%d transaction)"), montant_categ_etat,
					     devise_name ( devise_categ_etat ), nb_ope_categ_etat );
		else
		    text = g_strdup_printf ( _("%4.2f %s (%d transactions)"), montant_categ_etat,
					     devise_name ( devise_categ_etat ), nb_ope_categ_etat );
	    }
	    else
		text =g_strdup_printf ( _("%4.2f %s"), montant_categ_etat, devise_name ( devise_categ_etat ) );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes,
					ligne, ligne + 1, RIGHT, NULL );

	    ligne++;
	}

    }

    montant_categ_etat = 0;
    nom_categ_en_cours = NULL;
    titres_affiches = 0;
    nb_ope_categ_etat = 0;

    return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_categ sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_sous_categ ( gint ligne )
{
    gchar * text;

    if ( etat_courant -> utilise_categ
	 &&
	 etat_courant -> afficher_sous_categ
	 &&
	 etat_courant -> affiche_sous_total_sous_categ )
    {
	/* si rien n'est affiché en dessous de la sous_categ, on */
	/* met le résultat sur la ligne de la ss categ */
	/* sinon on fait une barre et on met le résultat */

	if ( etat_courant -> utilise_ib
	     ||
	     etat_courant -> regroupe_ope_par_compte
	     ||
	     etat_courant -> utilise_tiers
	     ||
	     etat_courant -> afficher_opes )
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_categ_en_cours
		 &&
		 nom_ss_categ_en_cours )
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_sous_categ_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Total %s: %s (%d transaction)")), nom_categ_en_cours,
						 nom_ss_categ_en_cours, nb_ope_sous_categ_etat );
		    else
			text = g_strdup_printf ( COLON(_("Total %s: %s (%d transactions)")), nom_categ_en_cours,
						 nom_ss_categ_en_cours, nb_ope_sous_categ_etat );
		}
		else
		    text = g_strdup_printf ( _("Total %s: %s"), nom_categ_en_cours, nom_ss_categ_en_cours );
	    }
	    else
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_sous_categ_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Sub-categories total (%d transaction)")),
						 nb_ope_sous_categ_etat );
		    else
			text = g_strdup_printf ( COLON(_("Sub-categories total (%d transactions)")),
						 nb_ope_sous_categ_etat );
		}
		else
		    text = COLON(_("Sub-categories total"));
	    }
	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

	    text = g_strdup_printf ( _("%4.2f %s"), montant_sous_categ_etat, devise_name ( devise_categ_etat ) );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_sous_categ_etat <= 1 )
		    text = g_strdup_printf ( _("%4.2f %s (%d transaction)"), montant_sous_categ_etat,
					     devise_name ( devise_categ_etat ), nb_ope_sous_categ_etat );
		else
		    text = g_strdup_printf ( _("%4.2f %s (%d transactions)"), montant_sous_categ_etat,
					     devise_name ( devise_categ_etat ), nb_ope_sous_categ_etat );
	    }
	    else
		text = g_strdup_printf ( _("%4.2f %s"), montant_sous_categ_etat, devise_name ( devise_categ_etat ) );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;
	}
    }

    montant_sous_categ_etat = 0;
    nom_ss_categ_en_cours = NULL;
    titres_affiches = 0;
    nb_ope_sous_categ_etat = 0;

    return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_ib ( gint ligne )
{
    gchar * text;

    if ( etat_courant -> utilise_ib
	 &&
	 etat_courant -> affiche_sous_total_ib )
    {
	/* si rien n'est affiché en dessous de la ib, on */
	/* met le résultat sur la ligne de l'ib */
	/* sinon on fait une barre et on met le résultat */

	if ( etat_courant -> afficher_sous_ib
	     ||
	     etat_courant -> regroupe_ope_par_compte
	     ||
	     etat_courant -> utilise_tiers
	     ||
	     etat_courant -> afficher_opes )
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_ib_en_cours )
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_ib_etat <= 1 )
			text = g_strdup_printf ( _("Total %s (%d transaction)"), nom_ib_en_cours, nb_ope_ib_etat );
		    else
			text = g_strdup_printf ( _("Total %s (%d transactions)"), nom_ib_en_cours, nb_ope_ib_etat );
		}
		else
		    text = g_strconcat ( POSTSPACIFY(_("Total")), nom_ib_en_cours, NULL );
	    }
	    else
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_ib_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Budgetary lines total: (%d transaction)")),
						 nb_ope_ib_etat );
		    else
			text = g_strdup_printf ( COLON(_("Budgetary lines total: (%d transactions)")),
						 nb_ope_ib_etat );
		}
		else
		    text = COLON(_("Budgetary lines total"));
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

	    text = g_strdup_printf ( _("%4.2f %s"), montant_ib_etat, devise_name ( devise_ib_etat ) );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_ib_etat <= 1 )
		    text = g_strdup_printf ( _("%4.2f %s (%d transaction)"), montant_ib_etat,
					     devise_name ( devise_ib_etat ), nb_ope_ib_etat );
		else
		    text = g_strdup_printf ( _("%4.2f %s (%d transactions)"), montant_ib_etat,
					     devise_name ( devise_ib_etat ), nb_ope_ib_etat );
	    }
	    else
		text = g_strdup_printf ( _("%4.2f %s"), montant_ib_etat, devise_name ( devise_ib_etat ) );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;
	}
    }

    montant_ib_etat = 0;
    nom_ib_en_cours = NULL;
    titres_affiches = 0;
    nb_ope_ib_etat = 0;

    return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_sous_ib ( gint ligne )
{
    gchar * text;

    if ( etat_courant -> utilise_ib
	 &&
	 etat_courant -> afficher_sous_ib
	 &&
	 etat_courant -> affiche_sous_total_sous_ib )
    {
	/* si rien n'est affiché en dessous de la sous ib, on */
	/* met le résultat sur la ligne de la sous ib */
	/* sinon on fait une barre et on met le résultat */

	if ( etat_courant -> regroupe_ope_par_compte
	     ||
	     etat_courant -> utilise_tiers
	     ||
	     etat_courant -> afficher_opes )
	{

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_ib_en_cours
		 &&
		 nom_ss_ib_en_cours )
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_sous_ib_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Total %s: %s (%d transaction)")), nom_ib_en_cours,
						 nom_ss_ib_en_cours, nb_ope_sous_ib_etat );
		    else
			text = g_strdup_printf ( COLON(_("Total %s: %s (%d transactions)")), nom_ib_en_cours,
						 nom_ss_ib_en_cours, nb_ope_sous_ib_etat );
		}
		else
		    text = g_strdup_printf ( _("Total %s: %s"), nom_ib_en_cours, nom_ss_ib_en_cours );
	    }
	    else
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_sous_ib_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Sub-budgetary lines total: (%d transaction)")),
						 nb_ope_sous_ib_etat );
		    else
			text = g_strdup_printf ( COLON(_("Sub-budgetary lines total: (%d transactions)")),
						 nb_ope_sous_ib_etat );
		}
		else
		    text = COLON(_("Sub-budgetary lines total"));
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

	    text = g_strdup_printf ( _("%4.2f %s"), montant_sous_ib_etat, devise_name ( devise_ib_etat ) );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_sous_ib_etat <= 1 )
		    text = g_strdup_printf ( _("%4.2f %s (%d transaction)"), montant_sous_ib_etat,
					     devise_name ( devise_ib_etat ), nb_ope_sous_ib_etat );
		else
		    text = g_strdup_printf ( _("%4.2f %s (%d transactions)"), montant_sous_ib_etat,
					     devise_name ( devise_ib_etat ), nb_ope_sous_ib_etat );
	    }
	    else
		text = g_strdup_printf ( _("%4.2f %s"), montant_sous_ib_etat, devise_name ( devise_ib_etat )) ;

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;
	}
    }

    montant_sous_ib_etat = 0;
    nom_ss_ib_en_cours = NULL;
    titres_affiches = 0;
    nb_ope_sous_ib_etat = 0;

    return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les compte sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_compte ( gint ligne )
{
    gchar * text;

    if ( etat_courant -> regroupe_ope_par_compte
	 &&
	 etat_courant -> affiche_sous_total_compte )
    {
	/* si rien n'est affiché en dessous du compte, on */
	/* met le résultat sur la ligne du compte */
	/* sinon on fait une barre et on met le résultat */

	if ( etat_courant -> utilise_tiers
	     ||
	     etat_courant -> afficher_opes )
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_compte_en_cours )
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_compte_etat <= 1 )
			text = g_strdup_printf ( _("Total %s (%d transaction)"), nom_compte_en_cours, nb_ope_compte_etat );
		    else
			text = g_strdup_printf ( _("Total %s (%d transactions)"), nom_compte_en_cours, nb_ope_compte_etat );
		}
		else
		    text = g_strconcat ( POSTSPACIFY(_("Total")), nom_compte_en_cours, NULL );
	    }
	    else
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_compte_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Account total: (%d transaction)")), nb_ope_compte_etat );
		    else
			text = g_strdup_printf ( COLON(_("Account total: (%d transactions)")), nb_ope_compte_etat );
		}
		else
		    text = COLON(_("Account total"));
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

	    text = g_strdup_printf ( _("%4.2f %s"), montant_compte_etat, devise_name ( devise_compte_en_cours_etat ) );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_compte_etat <= 1 )
		    text = g_strdup_printf ( _("%4.2f %s (%d transaction)"), montant_compte_etat,
					     devise_name ( devise_compte_en_cours_etat ), nb_ope_compte_etat );
		else
		    text = g_strdup_printf ( _("%4.2f %s (%d transactions)"), montant_compte_etat,
					     devise_name ( devise_compte_en_cours_etat ), nb_ope_compte_etat );
	    }
	    else
		text = g_strdup_printf ( _("%4.2f %s"), montant_compte_etat, devise_name (devise_compte_en_cours_etat) );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );  
	    ligne++;
	}
    }

    montant_compte_etat = 0;
    nom_compte_en_cours = NULL;
    titres_affiches = 0;
    nb_ope_compte_etat = 0;

    return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les tiers sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_tiers ( gint ligne )
{
    gchar * text;

    if ( etat_courant -> utilise_tiers
	 &&
	 etat_courant -> affiche_sous_total_tiers )
    {
	/* si rien n'est affiché en dessous du tiers, on */
	/* met le résultat sur la ligne du tiers */
	/* sinon on fait une barre et on met le résultat */

	if ( etat_courant -> afficher_opes )
	{

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_tiers_en_cours )
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_tiers_etat <= 1 )
			text = g_strdup_printf ( _("Total %s (%d transaction)"), nom_tiers_en_cours, nb_ope_tiers_etat );
		    else
			text = g_strdup_printf ( _("Total %s (%d transactions)"), nom_tiers_en_cours, nb_ope_tiers_etat );
		}
		else
		    text = g_strdup_printf ( _("Total %s"), nom_tiers_en_cours );
	    }
	    else
	    {
		if ( etat_courant -> afficher_nb_opes )
		{
		    if ( nb_ope_tiers_etat <= 1 )
			text = g_strdup_printf ( COLON(_("Third party total: (%d transaction)")), nb_ope_tiers_etat );
		    else
			text = g_strdup_printf ( COLON(_("Third party total: (%d transactions)")), nb_ope_tiers_etat );
		}
		else
		    text = COLON(_("Third party total"));
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

	    text = g_strdup_printf ( _("%4.2f %s"), montant_tiers_etat, devise_name ( devise_tiers_etat ) );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_tiers_etat <= 1 )
		    text = g_strdup_printf ( _("%4.2f %s (%d transaction)"), montant_tiers_etat,
					     devise_name ( devise_tiers_etat ), nb_ope_tiers_etat );
		else
		    text = g_strdup_printf ( _("%4.2f %s (%d transactions)"), montant_tiers_etat,
					     devise_name ( devise_tiers_etat ), nb_ope_tiers_etat );
	    }
	    else
		text = g_strdup_printf ( _("%4.2f %s"), montant_tiers_etat, devise_name ( devise_tiers_etat ) );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	    ligne++;
	}
    }

    montant_tiers_etat = 0;
    nom_tiers_en_cours = NULL;
    titres_affiches = 0;
    nb_ope_tiers_etat = 0;

    return (ligne );
}
/*****************************************************************************************************/






/*****************************************************************************************************/
/* affiche le total de la période à l'endroit donné de la table */
/* retourne le ligne suivante de la table */
/* si force = 0, vérifie les dates et affiche si nécessaire */
/*   si force = 1, affiche le total (chgt de categ, ib ...) */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_periode ( struct structure_operation *operation, gint ligne, gint force )
{
    if ( etat_courant -> separation_par_plage )
    {
	gchar *text;

	text = NULL;

	/* si la date de début de période est nulle, on la met au début de la période la date de l'opération */

	if ( !date_debut_periode )
	{
	    if ( operation )
	    {
		/*  il y a une opération, on va rechercher le début de la période qui la contient */
		/* ça peut être le début de la semaine, du mois ou de l'année de l'opé */

		switch ( etat_courant -> type_separation_plage )
		{
		    case 0:
			/* séparation par semaine : on recherche le début de la semaine qui contient l'opé */

			date_debut_periode = g_date_new_dmy ( g_date_day ( operation -> date ),
							      g_date_month ( operation -> date ),
							      g_date_year ( operation -> date ));

			if ( g_date_weekday ( date_debut_periode )  != (etat_courant -> jour_debut_semaine + 1 ))
			{
			    if ( g_date_weekday ( date_debut_periode ) < (etat_courant -> jour_debut_semaine + 1 ))
				g_date_subtract_days ( date_debut_periode,
						       g_date_weekday ( date_debut_periode ) + etat_courant -> jour_debut_semaine - 2 );
			    else
				g_date_subtract_days ( date_debut_periode,
						       g_date_weekday ( date_debut_periode ) - etat_courant -> jour_debut_semaine - 1 );
			}
			break;

		    case 1:
			/* séparation par mois */

			date_debut_periode = g_date_new_dmy ( 1,
							      g_date_month ( operation -> date ),
							      g_date_year ( operation -> date ));
			break;

		    case 2:
			/* séparation par an */

			date_debut_periode = g_date_new_dmy ( 1,
							      1,
							      g_date_year ( operation -> date ));
			break;
		}
	    }
	    else
		date_debut_periode = NULL;
	    return ( ligne );
	}

	/* on vérifie maintenant s'il faut afficher un total ou pas */

	switch ( etat_courant -> type_separation_plage )
	{
	    gchar buffer[30];
	    GDate *date_tmp;

	    case 0:
	    /* séparation par semaine */

	    /* 	  si c'est le même jour que l'opé précédente, on fait rien */

	    if ( !force
		 &&
		 !g_date_compare ( operation -> date,
				   date_debut_periode ))
		return ( ligne );

	    /* 	  si on est en début de semaine, on met un sous total */

	    date_tmp = g_date_new_dmy ( g_date_day ( date_debut_periode ),
					g_date_month ( date_debut_periode ),
					g_date_year ( date_debut_periode ));
	    g_date_add_days ( date_tmp,
			      7 );

	    if ( !force
		 &&
		 ( g_date_weekday ( operation -> date )  != (etat_courant -> jour_debut_semaine + 1 )
		   &&
		   g_date_compare ( operation -> date,
				    date_tmp ) < 0 ))
		return ( ligne );

	    /* on doit retrouver la date du début de semaine et y ajouter 6j pour afficher la période */

	    if ( g_date_weekday ( date_debut_periode )  != (etat_courant -> jour_debut_semaine + 1 ))
	    {
		if ( g_date_weekday ( date_debut_periode ) < (etat_courant -> jour_debut_semaine + 1 ))
		    g_date_subtract_days ( date_debut_periode,
					   g_date_weekday ( date_debut_periode ) + etat_courant -> jour_debut_semaine - 2 );
		else
		    g_date_subtract_days ( date_debut_periode,
					   g_date_weekday ( date_debut_periode ) - etat_courant -> jour_debut_semaine - 1 );
	    }


	    g_date_free ( date_tmp );

	    date_tmp = g_date_new_dmy ( g_date_day ( date_debut_periode ),
					g_date_month ( date_debut_periode ),
					g_date_year ( date_debut_periode ));
	    g_date_add_days ( date_tmp,
			      6 );
	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_periode_etat <= 1 )
		    text = g_strdup_printf ( _("Result from %d/%d/%d to %d/%d/%d (%d transaction):"),
					     g_date_day ( date_debut_periode ),
					     g_date_month ( date_debut_periode ),
					     g_date_year ( date_debut_periode ),
					     g_date_day ( date_tmp ),
					     g_date_month ( date_tmp ),
					     g_date_year ( date_tmp ),
					     nb_ope_periode_etat );
		else
		    text = g_strdup_printf ( _("Result from %d/%d/%d to %d/%d/%d (%d transactions):"),
					     g_date_day ( date_debut_periode ),
					     g_date_month ( date_debut_periode ),
					     g_date_year ( date_debut_periode ),
					     g_date_day ( date_tmp ),
					     g_date_month ( date_tmp ),
					     g_date_year ( date_tmp ),
					     nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( _("Result from %d/%d/%d to %d/%d/%d:"),
					 g_date_day ( date_debut_periode ),
					 g_date_month ( date_debut_periode ),
					 g_date_year ( date_debut_periode ),
					 g_date_day ( date_tmp ),
					 g_date_month ( date_tmp ),
					 g_date_year ( date_tmp ));

	    break;

	    case 1:
	    /* séparation par mois */

	    if ( !force
		 &&
		 operation -> mois == g_date_month ( date_debut_periode ) )
		return ( ligne );

	    g_date_strftime ( buffer,
			      29,
			      "%B %Y",
			      date_debut_periode );

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_periode_etat <= 1 )
		    text = g_strdup_printf ( COLON(_("Result of %s (%d transaction)")),
					     buffer, nb_ope_periode_etat );
		else
		    text = g_strdup_printf ( COLON(_("Result of %s (%d transactions)")),
					     buffer, nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( COLON(_("Result of %s")),
					 buffer );

	    break;

	    case 2:
	    /* séparation par an */

	    if ( !force
		 &&
		 operation -> annee == g_date_year ( date_debut_periode ) )
		return ( ligne );

	    g_date_strftime ( buffer,
			      29,
			      "%Y",
			      date_debut_periode );

	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_periode_etat <= 1 )
		    text = g_strdup_printf ( COLON(_("Result for %s (%d transaction)")),
					     buffer, nb_ope_periode_etat );
		else
		    text = g_strdup_printf ( COLON(_("Result for %s (%d transactions)")),
					     buffer, nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( COLON(_("Result for %s")), buffer );
	    break;
	}

	/*       si on arrive ici, c'est qu'il y a un chgt de période ou que c'est forcé */

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	ligne++;

	etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	ligne++;

	etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

	text = g_strdup_printf ( _("%4.2f %s"), montant_periode_etat, devise_name ( devise_generale_etat ) );
	etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	ligne++;

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	ligne++;

	montant_periode_etat = 0;
	nb_ope_periode_etat = 0;

	/* comme il y a un changement de période, on remet la date_debut_periode à celle du début de la période */
	/* de l'opération  en cours */


	if ( operation )
	{
	    /*  il y a une opération, on va rechercher le début de la période qui la contient */
	    /* ça peut être le début de la semaine, du mois ou de l'année de l'opé */

	    switch ( etat_courant -> type_separation_plage )
	    {
		case 0:
		    /* séparation par semaine : on recherche le début de la semaine qui contient l'opé */

		    date_debut_periode = g_date_new_dmy ( g_date_day ( operation -> date ),
							  g_date_month ( operation -> date ),
							  g_date_year ( operation -> date ));

		    if ( g_date_weekday ( date_debut_periode )  != (etat_courant -> jour_debut_semaine + 1 ))
		    {
			if ( g_date_weekday ( date_debut_periode ) < (etat_courant -> jour_debut_semaine + 1 ))
			    g_date_subtract_days ( date_debut_periode,
						   g_date_weekday ( date_debut_periode ) + etat_courant -> jour_debut_semaine - 2 );
			else
			    g_date_subtract_days ( date_debut_periode,
						   g_date_weekday ( date_debut_periode ) - etat_courant -> jour_debut_semaine - 1 );
		    }
		    break;

		case 1:
		    /* séparation par mois */

		    date_debut_periode = g_date_new_dmy ( 1,
							  g_date_month ( operation -> date ),
							  g_date_year ( operation -> date ));
		    break;

		case 2:
		    /* séparation par an */

		    date_debut_periode = g_date_new_dmy ( 1,
							  1,
							  g_date_year ( operation -> date ));
		    break;
	    }
	}
	else
	    date_debut_periode = NULL;
    }

    return (ligne );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total de l'exo  à l'endroit donné de la table */
/* retourne le ligne suivante de la table */
/* si force = 0, vérifie les dates et affiche si nécessaire */
/*   si force = 1, affiche le total (chgt de categ, ib ...) */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_exercice ( struct structure_operation *operation, gint ligne, gint force )
{
    if ( etat_courant -> separation_par_exo )
    {
	gchar *text;

	text = NULL;

	/* 	si l'exo précédent est -1, on le met à l'exo de l'opé */
	/* 	utilise ça car des opés peuvent ne pas avoir d'exo */

	if ( exo_en_cours_etat == -1 )
	{
	    exo_en_cours_etat = operation -> no_exercice;
	    return ligne;
	}
	
	/* on vérifie maintenant s'il faut afficher un total ou pas */

	if ( !force
	     &&
	     operation -> no_exercice == exo_en_cours_etat )
	    return ligne;

	if ( exo_en_cours_etat )
	{
	    /* 	    les opés ont un exo */
	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_periode_etat <= 1 )
		    text = g_strdup_printf ( COLON(_("Result of %s (%d transaction)")),
					     exercice_name_by_no ( exo_en_cours_etat ),
					     nb_ope_periode_etat );
		else
		    text = g_strdup_printf ( COLON(_("Result of %s (%d transactions)")),
					     exercice_name_by_no ( exo_en_cours_etat ),
					     nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( COLON(_("Result of %s")),
					 exercice_name_by_no ( exo_en_cours_etat ));
	}
	else
	{
	    /* 	    les opés n'ont pas d'exo */
	    if ( etat_courant -> afficher_nb_opes )
	    {
		if ( nb_ope_periode_etat <= 1 )
		    text = g_strdup_printf ( COLON(_("Result without financial year (%d transaction)")),
					     nb_ope_periode_etat );
		else
		    text = g_strdup_printf ( COLON(_("Result without financial year (%d transactions)")),
					     nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( COLON(_("Result without financial year")));
	}

    
    
    /*       si on arrive ici, c'est qu'il y a un chgt de période ou que c'est forcé */

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	ligne++;

	etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	ligne++;

	etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

	text = g_strdup_printf ( _("%4.2f %s"), montant_exo_etat, devise_name ( devise_generale_etat ) );
	etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
	ligne++;

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
	ligne++;

	montant_exo_etat = 0;
	nb_ope_exo_etat = 0;

	/* comme il y a un changement d'état, on remet exo_en_cours_etat à celle de l'opé en cours */

	if ( operation )
	    exo_en_cours_etat = operation -> no_exercice;
	else
	    date_debut_periode = NULL;
    }

    return (ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affichage_ligne_ope ( struct structure_operation *operation,
					gint ligne )
{
    gint colonne;
    gchar * text;

    /* on met tous les labels dans un event_box pour aller directement sur l'opé si elle est clickée */


    if ( etat_courant -> afficher_opes )
    {
	/* on affiche ce qui est demandé pour les opés */


	/* si les titres ne sont pas affichés et qu'il faut le faire, c'est ici */

	if ( !titres_affiches
	     &&
	     etat_courant -> afficher_titre_colonnes
	     &&
	     etat_courant -> type_affichage_titres )
	    ligne = etat_affiche_affiche_titres_colonnes ( ligne );

	colonne = 1;

	/*       pour chaque info, on vérifie si on l'opé doit être clickable */
	/* si c'est le cas, on place le label dans un event_box */

	if ( etat_courant -> afficher_no_ope )
	{
	    text = itoa ( operation -> no_operation );

	    if ( etat_courant -> ope_clickables )
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
	    }
	    else
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( etat_courant -> afficher_date_ope )
	{
	    text = g_strdup_printf  ( "%.2d/%.2d/%d", operation -> jour, operation -> mois, operation -> annee );
	    if ( etat_courant -> ope_clickables )
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
	    }
	    else
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( etat_courant -> afficher_exo_ope )
	{
	    if ( operation -> no_exercice )
	    {
		text = ((struct struct_exercice *)(g_slist_find_custom ( liste_struct_exercices,
									 GINT_TO_POINTER ( operation -> no_exercice ),
									 (GCompareFunc) recherche_exercice_par_no )->data)) -> nom_exercice;

		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }
	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( etat_courant -> afficher_tiers_ope )
	{
	    if ( operation -> tiers )
	    {
		text = ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
								      GINT_TO_POINTER ( operation -> tiers ),
								      (GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers;
		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( etat_courant -> afficher_categ_ope )
	{
	    gchar *pointeur;

	    pointeur = NULL;

	    if ( operation -> categorie )
	    {
		struct struct_categ *categ;

		categ = g_slist_find_custom ( liste_struct_categories,
					      GINT_TO_POINTER ( operation -> categorie ),
					      (GCompareFunc) recherche_categorie_par_no ) -> data;
		pointeur = categ -> nom_categ;

		if ( operation -> sous_categorie
		     &&
		     etat_courant -> afficher_sous_categ_ope )
		    pointeur = g_strconcat ( pointeur,
					     " : ",
					     ((struct struct_sous_categ *)(g_slist_find_custom ( categ -> liste_sous_categ,
												 GINT_TO_POINTER ( operation -> sous_categorie ),
												 (GCompareFunc) recherche_sous_categorie_par_no ) -> data )) -> nom_sous_categ,
					     NULL );
	    }
	    else
	    {
		/* si c'est un virement, on le marque, sinon c'est qu'il n'y a pas de categ */
		/* ou que c'est une opé ventilée, et on marque rien */

		if ( operation -> relation_no_operation )
		{
		    /* c'est un virement */

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		    if ( operation -> montant < 0 )
			pointeur = g_strdup_printf ( _("Transfer to %s"), NOM_DU_COMPTE );
		    else
			pointeur = g_strdup_printf ( _("Transfer from %s"), NOM_DU_COMPTE );
		}
	    }

	    if ( pointeur )
	    {
		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( pointeur, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( pointeur, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( etat_courant -> afficher_ib_ope )
	{
	    if ( operation -> imputation )
	    {
		struct struct_imputation *ib;
		gchar *pointeur;

		ib = g_slist_find_custom ( liste_struct_imputation,
					   GINT_TO_POINTER ( operation -> imputation ),
					   (GCompareFunc) recherche_imputation_par_no ) -> data;
		pointeur = ib -> nom_imputation;

		if ( operation -> sous_imputation
		     &&
		     etat_courant -> afficher_sous_ib_ope )
		    pointeur = g_strconcat ( pointeur,
					     " : ",
					     ((struct struct_sous_imputation *)(g_slist_find_custom ( ib -> liste_sous_imputation,
												      GINT_TO_POINTER ( operation -> sous_imputation ),
												      (GCompareFunc) recherche_sous_imputation_par_no ) -> data )) -> nom_sous_imputation,
					     NULL );

		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( pointeur, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( pointeur, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( etat_courant -> afficher_notes_ope )
	{
	    if ( operation -> notes )
	    {
		text =  operation -> notes;

		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }
	    else
	    {
		etat_affiche_attach_label ( NULL, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( etat_courant -> afficher_type_ope )
	{
	    GSList *pointeur;

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	    pointeur = g_slist_find_custom ( TYPES_OPES,
					     GINT_TO_POINTER ( operation -> type_ope ),
					     (GCompareFunc) recherche_type_ope_par_no );

	    if ( pointeur )
	    {
		struct struct_type_ope *type;

		type = pointeur -> data;

		text = type -> nom_type;

		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( etat_courant -> afficher_cheque_ope )
	{
	    /* Si l'opération est une opération de ventilation, il faut rechercher
	       l'opération mère pour pouvoir récupérer le n° du chèque */
	    if ( operation -> no_operation_ventilee_associee )
	    {
		gpointer **save_ptab;
		GSList *pTransactionList;
		gboolean found = FALSE;

		/* On récupère donc la liste des opérations du compte et on en fait
		   le tour jusqu'à ce qu'on trouve l'opération mère */
		save_ptab = p_tab_nom_de_compte_variable;
		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;
		pTransactionList = LISTE_OPERATIONS;
		while ( pTransactionList && !found )
		{
		    struct structure_operation *pTransaction;

		    pTransaction = pTransactionList -> data;
		    
		    if ( pTransaction -> operation_ventilee &&
			 pTransaction -> no_operation == operation -> no_operation_ventilee_associee &&
			 pTransaction -> contenu_type )
		    {
			operation -> contenu_type = pTransaction -> contenu_type;
			found = TRUE;
		    }
		    pTransactionList = pTransactionList -> next;
		}
		p_tab_nom_de_compte_variable = save_ptab;
	    }
	    
	    if ( operation -> contenu_type )
	    {
		text = operation -> contenu_type;

		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( etat_courant -> afficher_pc_ope )
	{
	    if ( operation -> no_piece_comptable )
	    {
		text = operation -> no_piece_comptable;

		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}



	if ( etat_courant -> afficher_infobd_ope )
	{
	    if ( operation -> info_banque_guichet )
	    {
		text = operation -> info_banque_guichet;

		if ( etat_courant -> ope_clickables )
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( etat_courant -> afficher_verif_ope )
	{
	    
	    switch ( operation -> pointe )
	    {
		case CHECKED_TRANSACTION :
		
		    text = g_strdup ( _("C") );
		    break;

		case RECONCILED_TRANSACTION :
		
		    text = g_strdup ( _("R") );
		    break;

		case TELECHECKED_TRANSACTION :
		
		    text = g_strdup ( _("C") );
		    break;

		case UNCHECKED_TRANSACTION :
		default :
		
		    text = "";
		    break;

	    }

	    if ( etat_courant -> ope_clickables )
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
	    }
	    else
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( etat_courant -> afficher_rappr_ope )
	{
	    GSList *pointeur;

	    pointeur = g_slist_find_custom ( liste_no_rapprochements,
					     GINT_TO_POINTER ( operation -> no_rapprochement ),
					     (GCompareFunc) recherche_no_rapprochement_par_no );

	    if ( pointeur )
	    {
		struct struct_no_rapprochement *rapprochement;

		rapprochement = pointeur -> data;

		if (rapprochement)
		{
		    text = rapprochement -> nom_rapprochement;

		    if ( etat_courant -> ope_clickables )
		    {
			etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, operation );
		    }
		    else
		    {
			etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, LEFT, NULL );
		    }
		}
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	/* on affiche le montant au bout de la ligne */

	if ( devise_compte_en_cours_etat
	     &&
	     operation -> devise == devise_compte_en_cours_etat -> no_devise )
	    text = g_strdup_printf  ( _("%4.2f %s"), operation -> montant, devise_name ( devise_compte_en_cours_etat ) );
	else
	    text = g_strdup_printf  ( _("%4.2f %s"), operation -> montant, devise_name_by_no ( operation -> devise ) );

	if ( etat_courant -> ope_clickables )
	{
	    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, RIGHT, operation );
	}
	else
	{
	    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, RIGHT, NULL );
	}


	if ( ligne_debut_partie == -1 )
	    ligne_debut_partie = ligne;

	ligne++;
    }
    return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affiche_total_partiel ( gdouble total_partie,
					  gint ligne,
					  gint type )
{
    gchar * text;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    if ( type )
    {
	if ( etat_courant -> afficher_nb_opes )
	{
	    if ( nb_ope_partie_etat <= 1 )
		text = g_strdup_printf ( COLON(_("Total expenses (%d transaction)")), nb_ope_partie_etat );
	    else
		text = g_strdup_printf ( COLON(_("Total expenses (%d transactions)")), nb_ope_partie_etat );
	}
	else
	    text = COLON(_("Total expenses"));
    }
    else
    {
	if ( etat_courant -> afficher_nb_opes )
	{
	    if ( nb_ope_partie_etat <= 1 )
		text = g_strdup_printf ( COLON(_("Total income (%d transaction)")), nb_ope_partie_etat );
	    else
		text = g_strdup_printf ( COLON(_("Total income (%d transactions)")), nb_ope_partie_etat );
	}
	else
	    text = COLON(_("Total income"));
    }

    etat_affiche_attach_label ( text, TEXT_NORMAL, 0, nb_colonnes - 1, ligne, ligne + 1, LEFT, NULL );

    text = g_strdup_printf ( _("%4.2f %s"), total_partie, devise_name ( devise_generale_etat ) );
    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, CENTER, NULL );
    ligne++;

    nom_categ_en_cours = NULL;
    nom_ss_categ_en_cours = NULL;
    nom_ib_en_cours = NULL;
    nom_ss_ib_en_cours = NULL;
    nom_compte_en_cours = NULL;
    nom_tiers_en_cours = NULL;


    return ( ligne );
}
/*****************************************************************************************************/

/*****************************************************************************************************/
gint etat_affiche_affiche_total_general ( gdouble total_general,
					  gint ligne )
{
    gchar * text;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes, ligne, ligne + 1, CENTER, NULL );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    if ( etat_courant -> afficher_nb_opes )
    {
	if ( nb_ope_general_etat <= 1 )
	    text = g_strdup_printf ( COLON(_("General total (%d transaction)")), nb_ope_general_etat );
	else
	    text = g_strdup_printf ( COLON(_("General total (%d transactions)")), nb_ope_general_etat );
    }
    else
	text = COLON(_("General total"));

    etat_affiche_attach_label ( text, TEXT_NORMAL, 0, nb_colonnes - 2, ligne, ligne + 1, LEFT, NULL );

    text = g_strdup_printf ( _("%4.2f %s"), total_general, devise_name ( devise_generale_etat ) );
    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, RIGHT, NULL );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes, ligne, ligne + 1, CENTER, NULL );
    ligne++;

    return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint etat_affiche_affiche_categ_etat ( struct structure_operation *operation,
				       gchar *decalage_categ,
				       gint ligne )
{
    gchar *pointeur_char;

    /* vérifie qu'il y a un changement de catégorie */
    /* ça peut être aussi chgt pour virement, ventilation ou pas de categ */

    if ( etat_courant -> utilise_categ
	 &&
	 ( operation -> categorie != ancienne_categ_etat
	   ||
	   ( ancienne_categ_speciale_etat == 1
	     &&
	     !operation -> relation_no_operation )
	   ||
	   ( ancienne_categ_speciale_etat == 2
	     &&
	     !operation -> operation_ventilee )
	   ||
	   ( ancienne_categ_speciale_etat == 3
	     &&
	     ( operation -> operation_ventilee
	       ||
	       operation -> relation_no_operation ))))
    {

	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( operation,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 1,
							       ligne );

	    /* on ajoute le total de la categ */

	    ligne = etat_affiche_affiche_total_categories ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des catég, on saute la partie suivante */

	if ( etat_courant -> afficher_nom_categ )
	{
	    if ( operation -> categorie )
	    {
		nom_categ_en_cours = ((struct struct_categ *)(g_slist_find_custom ( liste_struct_categories,
										    GINT_TO_POINTER ( operation -> categorie ),
										    (GCompareFunc) recherche_categorie_par_no ) -> data )) -> nom_categ;
		pointeur_char = g_strconcat ( decalage_categ,
					      nom_categ_en_cours,
					      NULL );
		ancienne_categ_speciale_etat = 0;
	    }
	    else
	    {
		if ( operation -> relation_no_operation )
		{
		    pointeur_char = g_strconcat ( decalage_categ,
						  _("Transfert"),
						  NULL );
		    ancienne_categ_speciale_etat = 1;
		}
		else
		{
		    if ( operation -> operation_ventilee )
		    {
			pointeur_char = g_strconcat ( decalage_categ,
						      _("Breakdown of transaction"),
						      NULL );
			ancienne_categ_speciale_etat = 2;
		    }
		    else
		    {
			pointeur_char = g_strconcat ( decalage_categ,
						      _("No category"),
						      NULL );
			ancienne_categ_speciale_etat = 3;
		    }
		}
	    }

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1,
					ligne, ligne + 1, LEFT, NULL );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 1 );

	ancienne_categ_etat = operation -> categorie;

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint etat_affiche_affiche_sous_categ_etat ( struct structure_operation *operation,
					    gchar *decalage_sous_categ,
					    gint ligne )
{
    gchar *pointeur_char;

    if ( etat_courant -> utilise_categ
	 &&
	 etat_courant -> afficher_sous_categ
	 &&
	 operation -> categorie
	 &&
	 operation -> sous_categorie != ancienne_sous_categ_etat )
    {
	struct struct_categ *categ;

	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( operation,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière la sous catégorie */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 2,
							       ligne );

	    /* on ajoute le total de la sous categ */

	    ligne = etat_affiche_affiche_total_sous_categ ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des ss-catég, on saute la partie suivante */

	if ( etat_courant -> afficher_nom_categ )
	{
	    categ = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( operation -> categorie ),
					  (GCompareFunc) recherche_categorie_par_no ) -> data;

	    if ( operation -> sous_categorie )
	    {
		nom_ss_categ_en_cours = ((struct struct_sous_categ *)(g_slist_find_custom ( categ->liste_sous_categ,
											    GINT_TO_POINTER ( operation -> sous_categorie ),
											    (GCompareFunc) recherche_sous_categorie_par_no ) -> data )) -> nom_sous_categ;
		pointeur_char = g_strconcat ( decalage_sous_categ,
					      nom_ss_categ_en_cours,
					      NULL );
	    }
	    else
	    {
		if ( etat_courant -> afficher_pas_de_sous_categ )
		    pointeur_char = g_strconcat ( decalage_sous_categ,
						  _("No subcategory"),
						  NULL );
		else
		    pointeur_char = "";
	    }

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, LEFT, NULL );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 2 );

	ancienne_sous_categ_etat = operation -> sous_categorie;

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_ib_etat ( struct structure_operation *operation,
				    gchar *decalage_ib,
				    gint ligne )
{
    gchar *pointeur_char;

    /* mise en place de l'ib */


    if ( etat_courant -> utilise_ib
	 &&
	 operation -> imputation != ancienne_ib_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( operation,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière l'ib */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 3,
							       ligne );

	    /* on ajoute le total de l'ib */

	    ligne = etat_affiche_affiche_total_ib ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des ib, on saute la partie suivante */

	if ( etat_courant -> afficher_nom_ib )
	{
	    if ( operation -> imputation )
	    {
		nom_ib_en_cours = ((struct struct_imputation *)(g_slist_find_custom ( liste_struct_imputation,
										      GINT_TO_POINTER ( operation -> imputation ),
										      (GCompareFunc) recherche_imputation_par_no ) -> data )) -> nom_imputation;
		pointeur_char = g_strconcat ( decalage_ib,
					      nom_ib_en_cours,
					      NULL );
	    }
	    else
		pointeur_char = g_strconcat ( decalage_ib,
					      _("No budgetary line"),
					      NULL );

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, LEFT, NULL );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 3 );

	ancienne_ib_etat = operation -> imputation;

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_sous_ib_etat ( struct structure_operation *operation,
					 gchar *decalage_sous_ib,
					 gint ligne )
{
    gchar *pointeur_char;


    /* mise en place de la sous_ib */


    if ( etat_courant -> utilise_ib
	 &&
	 etat_courant -> afficher_sous_ib
	 &&
	 operation -> imputation
	 &&
	 operation -> sous_imputation != ancienne_sous_ib_etat )
    {
	struct struct_imputation *imputation;

	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( operation,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière la sous ib */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 4,
							       ligne );

	    /* on ajoute le total de la sous ib */

	    ligne = etat_affiche_affiche_total_sous_ib ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des ss-ib, on saute la partie suivante */

	if ( etat_courant -> afficher_nom_ib )
	{
	    imputation = g_slist_find_custom ( liste_struct_imputation,
					       GINT_TO_POINTER ( operation -> imputation ),
					       (GCompareFunc) recherche_imputation_par_no ) -> data;

	    if ( operation -> sous_imputation )
	    {
		nom_ss_ib_en_cours = ((struct struct_sous_imputation *)(g_slist_find_custom ( imputation->liste_sous_imputation,
											      GINT_TO_POINTER ( operation -> sous_imputation ),
											      (GCompareFunc) recherche_sous_imputation_par_no ) -> data )) -> nom_sous_imputation;
		pointeur_char = g_strconcat ( decalage_sous_ib,
					      nom_ss_ib_en_cours,
					      NULL );
	    }
	    else
	    {
		if ( etat_courant -> afficher_pas_de_sous_ib )
		    pointeur_char = g_strconcat ( decalage_sous_ib,
						  _("No sub-budgetary line"),
						  NULL );
		else
		    pointeur_char = "";
	    }

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, LEFT, NULL );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 4 );

	ancienne_sous_ib_etat = operation -> sous_imputation;

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_compte_etat ( struct structure_operation *operation,
					gchar *decalage_compte,
					gint ligne )
{
    gchar *pointeur_char;

    /* mise en place du compte */

    if ( etat_courant -> regroupe_ope_par_compte
	 &&
	 operation -> no_compte != ancien_compte_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( operation,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière le compte */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 5,
							       ligne );

	    /* on ajoute le total du compte */

	    ligne = etat_affiche_affiche_total_compte ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des comptes, on saute la partie suivante */

	if ( etat_courant -> afficher_nom_compte )
	{
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	    pointeur_char = g_strconcat ( decalage_compte,
					  NOM_DU_COMPTE,
					  NULL );
	    nom_compte_en_cours = NOM_DU_COMPTE;

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, LEFT, NULL );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 5 );

	ancien_compte_etat = operation -> no_compte;

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_tiers_etat ( struct structure_operation *operation,
				       gchar *decalage_tiers,
				       gint ligne )
{
    gchar *pointeur_char;

    /* affiche le tiers */

    if ( etat_courant -> utilise_tiers
	 &&
	 operation -> tiers != ancien_tiers_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( operation,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière le tiers */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 6,
							       ligne );

	    /* on ajoute le total du tiers */

	    ligne = etat_affiche_affiche_total_tiers ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des tiers, on saute la partie suivante */

	if ( etat_courant -> afficher_nom_tiers )
	{
	    if ( operation -> tiers )
	    {
		nom_tiers_en_cours = ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
										    GINT_TO_POINTER ( operation -> tiers ),
										    (GCompareFunc) recherche_tiers_par_no ) -> data )) -> nom_tiers;
		pointeur_char = g_strconcat ( decalage_tiers,
					      nom_tiers_en_cours,
					      NULL );
	    }
	    else
		pointeur_char = g_strconcat ( decalage_tiers,
					      _("No third party"),
					      NULL );

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, LEFT, NULL );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 6 );

	ancien_tiers_etat = operation -> tiers;

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }
    return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affiche_titre_revenus_etat ( gint ligne )
{
    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 0, 1, 
				ligne, ligne + 1, CENTER, NULL );
    ligne++;

    etat_affiche_attach_label ( _("Incomes"), TEXT_LARGE, 0, nb_colonnes-1, 
				ligne, ligne + 1, CENTER, NULL );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 0, 1, 
				ligne, ligne + 1, CENTER, NULL );
    ligne++;

    return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affiche_titre_depenses_etat ( gint ligne )
{
    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 0, 1, 
				ligne, ligne + 1, CENTER, NULL );
    ligne++;

    etat_affiche_attach_label ( _("Outgoings"), TEXT_LARGE, 0, nb_colonnes-1, 
				ligne, ligne + 1, CENTER, NULL );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 0, 1, 
				ligne, ligne + 1, CENTER, NULL );
    ligne++;

    return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* appelée lors de l'affichage d'une structure ( catég, ib ... ) */
/* affiche le total de toutes les structures sous jaccentes */
/*****************************************************************************************************/

gint etat_affiche_affiche_totaux_sous_jaccent ( gint origine,
						gint ligne )
{
    GList *pointeur_glist;


    /* on doit partir du bout de la liste pour revenir vers la structure demandée */

    pointeur_glist = g_list_last ( etat_courant -> type_classement );


    while ( GPOINTER_TO_INT ( pointeur_glist -> data ) != origine )
    {
	switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	    case 1:
		ligne = etat_affiche_affiche_total_categories ( ligne );
		break;

	    case 2:
		ligne = etat_affiche_affiche_total_sous_categ ( ligne );
		break;

	    case 3:
		ligne = etat_affiche_affiche_total_ib ( ligne );
		break;

	    case 4:
		ligne = etat_affiche_affiche_total_sous_ib ( ligne );
		break;

	    case 5:
		ligne = etat_affiche_affiche_total_compte ( ligne );
		break;

	    case 6:
		ligne = etat_affiche_affiche_total_tiers ( ligne );
		break;
	}
	pointeur_glist = pointeur_glist -> prev;
    }

    return ( ligne );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint etat_affiche_affiche_titres_colonnes ( gint ligne )
{
    gint colonne;

    colonne = 1;

    if ( etat_courant -> afficher_no_ope )
    {
	etat_affiche_attach_label ( _("Number"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_date_ope )
    {
	etat_affiche_attach_label ( _("Date"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_exo_ope )
    {
	etat_affiche_attach_label ( _("Financial year"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_tiers_ope )
    {
	etat_affiche_attach_label ( _("Third party"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_categ_ope )
    {
	etat_affiche_attach_label ( _("Category"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_ib_ope )
    {
	etat_affiche_attach_label ( _("Budgetary line"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_notes_ope )
    {
	etat_affiche_attach_label ( _("Notes"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_type_ope )
    {
	etat_affiche_attach_label ( _("Payment methods"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_cheque_ope )
    {
	etat_affiche_attach_label ( _("Cheque"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_pc_ope )
    {
	etat_affiche_attach_label ( _("Voucher"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_infobd_ope )
    {
	etat_affiche_attach_label ( _("Bank references"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_verif_ope )
    {
	etat_affiche_attach_label ( _("Check"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( etat_courant -> afficher_rappr_ope )
    {
	etat_affiche_attach_label ( _("Statement"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, CENTER, NULL );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    ligne++;

    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    titres_affiches = 1;

    return ( ligne );
}
/*****************************************************************************************************/



void etat_affiche_attach_hsep ( int x, int x2, int y, int y2)
{
    etat_affichage_output -> attach_hsep ( x, x2, y, y2);
}



void etat_affiche_attach_vsep ( int x, int x2, int y, int y2)
{
    etat_affichage_output -> attach_vsep ( x, x2, y, y2);
}



void etat_affiche_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
				 enum alignement align, struct structure_operation * ope )
{
    etat_affichage_output -> attach_label ( text, properties, x, x2, y, y2, align, ope );
}



gint etat_affiche_initialise (GSList * opes_selectionnees)
{
    return etat_affichage_output -> initialise (opes_selectionnees);
}



gint etat_affiche_finish ()
{
    return etat_affichage_output -> finish ();
}
