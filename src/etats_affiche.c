/* ************************************************************************** */
/* Fichier qui s'occupe d'afficher les états                                  */
/*                                                                            */
/*                            etats_affiche.c                                 */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2008 Benjamin Drieu (bdrieu@april.org)	      */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "etats_affiche.h"
#include "etats_calculs.h"
#include "etats_support.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_report.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "navigation.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "structures.h"
#include "etats_config.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void etat_affiche_attach_hsep ( int x, int x2, int y, int y2);
static void etat_affiche_attach_label ( gchar * text, gint properties, int x, int x2, int y, int y2, 
				 enum alignement align, gint transaction_number );
static void etat_affiche_attach_vsep ( int x, int x2, int y, int y2);
/*END_STATIC*/




static gint ancien_tiers_etat;

/*START_EXTERN*/
extern gint ancien_compte_etat;
extern gint ancien_tiers_etat;
extern gint ancienne_categ_etat;
extern gint ancienne_categ_speciale_etat;
extern gint ancienne_ib_etat;
extern gint ancienne_sous_categ_etat;
extern gint ancienne_sous_ib_etat;
extern gint changement_de_groupe_etat;
extern GDate *date_debut_periode;
extern gint debut_affichage_etat;
extern gint devise_categ_etat;
extern gint devise_compte_en_cours_etat;
extern gint devise_generale_etat;
extern gint devise_ib_etat;
extern gint devise_tiers_etat;
extern struct struct_etat_affichage * etat_affichage_output;
extern gint exo_en_cours_etat;
extern gint ligne_debut_partie;
extern gsb_real montant_categ_etat;
extern gsb_real montant_compte_etat;
extern gsb_real montant_exo_etat;
extern gsb_real montant_ib_etat;
extern gsb_real montant_periode_etat;
extern gsb_real montant_sous_categ_etat;
extern gsb_real montant_sous_ib_etat;
extern gsb_real montant_tiers_etat;
extern gint nb_colonnes;
extern gint nb_ope_categ_etat;
extern gint nb_ope_compte_etat;
extern gint nb_ope_exo_etat;
extern gint nb_ope_general_etat;
extern gint nb_ope_ib_etat;
extern gint nb_ope_partie_etat;
extern gint nb_ope_periode_etat;
extern gint nb_ope_sous_categ_etat;
extern gint nb_ope_sous_ib_etat;
extern gint nb_ope_tiers_etat;
extern const gchar *nom_categ_en_cours;
extern const gchar *nom_compte_en_cours;
extern const gchar *nom_ib_en_cours;
extern const gchar *nom_ss_categ_en_cours;
extern const gchar *nom_ss_ib_en_cours;
extern const gchar *nom_tiers_en_cours;
extern gsb_real null_real;
extern gint titres_affiches;
/*END_EXTERN*/


/*****************************************************************************************************/
gint etat_affiche_affiche_titre ( gint ligne )
{
    gchar *titre;

    titre = etats_titre (gsb_gui_navigation_get_current_report ()) ;

    etat_affiche_attach_label ( titre, TEXT_BOLD | TEXT_HUGE, 0, nb_colonnes,
				ligne, ligne + 1, ALIGN_LEFT, 0 );

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
/* retourne la ligne suivante de la table */
/*****************************************************************************************************/
gint etat_affiche_affiche_total_categories ( gint ligne )
{
    char * text;
    gint current_report_number;
	gchar* tmpstr;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( gsb_data_report_get_category_used (current_report_number)
	 &&
	 gsb_data_report_get_category_show_category_amount (current_report_number))
    {
	/* si rien n'est affiché en dessous de la catég, on */
	/* met le résultat sur la ligne de la catég */
	/* sinon on fait une barre et on met le résultat */

	if ( gsb_data_report_get_category_show_sub_category (current_report_number)
	     ||
	     gsb_data_report_get_budget_used (current_report_number)
	     ||
	     gsb_data_report_get_account_group_reports (current_report_number)
	     ||
	     gsb_data_report_get_payee_used (current_report_number)
	     ||
	     gsb_data_report_get_show_report_transactions (current_report_number) )
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes -1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_categ_en_cours )
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_categ_etat <= 1 )
			fmtstr = _("Total %s (%d transaction)");
		    else
			fmtstr = _("Total %s (%d transactions)");
		    text = g_strdup_printf ( fmtstr, nom_categ_en_cours, nb_ope_categ_etat );
		}
		else
		    text = g_strconcat ( _("Total "), nom_categ_en_cours, NULL );
	    }
	    else
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_categ_etat <= 1 )
			fmtstr = _("Category total (%d transaction): ");
		    else
			fmtstr = _("Category total (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nb_ope_categ_etat);
		}
		else
		    text = g_strdup ( _("Category total: ") );
	    }
	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1,
					ligne, ligne + 1, ALIGN_LEFT, 0 );
            g_free ( text );

	    text = gsb_real_get_string_with_currency (montant_categ_etat, devise_categ_etat, TRUE );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes,
					ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes -1, ligne, ligne + 1, ALIGN_LEFT, 0 );

	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr; 
		if ( nb_ope_categ_etat <= 1 )
		    fmtstr = _("%s (%d transaction)");
		else
		    fmtstr = _("%s (%d transactions)");

		tmpstr = gsb_real_get_string_with_currency ( montant_categ_etat,
										 devise_categ_etat, TRUE  );
	        text = g_strdup_printf ( fmtstr , tmpstr, nb_ope_categ_etat );
		g_free ( tmpstr );
	    }
	    else
		text = gsb_real_get_string_with_currency ( montant_categ_etat,
							   devise_categ_etat, TRUE );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes,
					ligne, ligne + 1, ALIGN_RIGHT, 0 );
            g_free ( text );

	    ligne++;
	}
    }

    montant_categ_etat = null_real;
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
    gint current_report_number;
	gchar* tmpstr;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( gsb_data_report_get_category_used (current_report_number)
	 &&
	 gsb_data_report_get_category_show_sub_category (current_report_number)
	 &&
	 gsb_data_report_get_category_show_sub_category_amount (current_report_number))
    {
	/* si rien n'est affiché en dessous de la sous_categ, on */
	/* met le résultat sur la ligne de la ss categ */
	/* sinon on fait une barre et on met le résultat */

	if ( gsb_data_report_get_budget_used (current_report_number)
	     ||
	     gsb_data_report_get_account_group_reports (current_report_number)
	     ||
	     gsb_data_report_get_payee_used (current_report_number)
	     ||
	     gsb_data_report_get_show_report_transactions (current_report_number))
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_categ_en_cours
		 &&
		 nom_ss_categ_en_cours )
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_sous_categ_etat <= 1 )
			fmtstr = _("Total %s: %s (%d transaction): ");
		    else
			fmtstr = _("Total %s: %s (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nom_categ_en_cours,
						 nom_ss_categ_en_cours, nb_ope_sous_categ_etat );
		}
		else
		    text = g_strdup_printf ( _("Total %s: %s"), nom_categ_en_cours, nom_ss_categ_en_cours );
	    }
	    else
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_sous_categ_etat <= 1 )
			fmtstr = _("Sub-categories total (%d transaction): ");
		    else
			fmtstr = _("Sub-categories total (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nb_ope_sous_categ_etat );
		}
		else
		    text = g_strdup( _("Sub-categories total: ") );
	    }
	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( text );

	    text = gsb_real_get_string_with_currency (montant_sous_categ_etat, devise_categ_etat, TRUE);
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_sous_categ_etat <= 1 )
		    fmtstr = _("%s (%d transaction)"); 
		else
		    fmtstr = _("%s (%d transactions)");
		tmpstr = gsb_real_get_string_with_currency ( montant_sous_categ_etat, devise_categ_etat, TRUE  );
	        text = g_strdup_printf ( fmtstr, tmpstr, 
				nb_ope_sous_categ_etat );
	        g_free ( tmpstr );
	    }
	    else
		text = gsb_real_get_string_with_currency ( montant_sous_categ_etat,
							   devise_categ_etat, TRUE );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;
	}
    }

    montant_sous_categ_etat = null_real;
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
    gint current_report_number;
	gchar* tmpstr2;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( gsb_data_report_get_budget_used (current_report_number)
	 &&
	 gsb_data_report_get_budget_show_budget_amount (current_report_number))
    {
	/* si rien n'est affiché en dessous de la ib, on */
	/* met le résultat sur la ligne de l'ib */
	/* sinon on fait une barre et on met le résultat */

	if ( gsb_data_report_get_budget_show_sub_budget (current_report_number)
	     ||
	     gsb_data_report_get_account_group_reports (current_report_number)
	     ||
	     gsb_data_report_get_payee_used (current_report_number)
	     ||
	     gsb_data_report_get_show_report_transactions (current_report_number))
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_ib_en_cours )
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr; 
		    if ( nb_ope_ib_etat <= 1 )
			fmtstr = _("Total %s (%d transaction)");
		    else
			fmtstr = _("Total %s (%d transactions)");
	   	    text = g_strdup_printf ( fmtstr, nom_ib_en_cours, nb_ope_ib_etat );
		}
		else
		    text = g_strconcat ( _("Total "), nom_ib_en_cours, NULL );
	    }
	    else
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr; 
		    if ( nb_ope_ib_etat <= 1 )
			fmtstr = _("Budgetary lines total: (%d transaction): ");
		    else
			fmtstr = _("Budgetary lines total: (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nb_ope_ib_etat );
		}
		else
		    text = g_strdup( _("Budgetary lines total: ") );
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( text );

	    text = gsb_real_get_string_with_currency ( montant_ib_etat, devise_ib_etat, TRUE );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_ib_etat <= 1 )
		    fmtstr = _("%s (%d transaction)"); 
		else
		    fmtstr = _("%s (%d transactions)"); 
		tmpstr2 = gsb_real_get_string_with_currency (montant_ib_etat, devise_ib_etat, TRUE  );
		text = g_strdup_printf ( fmtstr, tmpstr2 , nb_ope_ib_etat );
		g_free (tmpstr2);
	    }
	    else
		text = gsb_real_get_string_with_currency ( montant_ib_etat, devise_ib_etat, TRUE );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;
	}
    }

    montant_ib_etat = null_real;
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
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( gsb_data_report_get_budget_used (current_report_number)
	 &&
	 gsb_data_report_get_budget_show_sub_budget (current_report_number)
	 &&
	 gsb_data_report_get_budget_show_sub_budget_amount (current_report_number))
    {
	/* si rien n'est affiché en dessous de la sous ib, on */
	/* met le résultat sur la ligne de la sous ib */
	/* sinon on fait une barre et on met le résultat */

	if ( gsb_data_report_get_account_group_reports (current_report_number)
	     ||
	     gsb_data_report_get_payee_used (current_report_number)
	     ||
	     gsb_data_report_get_show_report_transactions (current_report_number))
	{

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_ib_en_cours
		 &&
		 nom_ss_ib_en_cours )
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr; 
		    if ( nb_ope_sous_ib_etat <= 1 )
			fmtstr = _("Total %s: %s (%d transaction): ");
		    else
			fmtstr = _("Total %s: %s (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nom_ib_en_cours,
						 nom_ss_ib_en_cours, nb_ope_sous_ib_etat );
		}
		else
		    text = g_strdup_printf ( _("Total %s: %s"), nom_ib_en_cours, nom_ss_ib_en_cours );
	    }
	    else
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_sous_ib_etat <= 1 )
			fmtstr = _("Sub-budgetary lines total: (%d transaction): ");
		    else
			fmtstr = _("Sub-budgetary lines total: (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nb_ope_sous_ib_etat );
		}
		else
		    text = g_strdup( _("Sub-budgetary lines total: ") );
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( text );

	    text = gsb_real_get_string_with_currency (montant_sous_ib_etat, devise_ib_etat, TRUE );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
	        gchar* tmpstr;

		if ( nb_ope_sous_ib_etat <= 1 )
		    fmtstr = _("%s (%d transaction)"); 
		else
		    fmtstr = _("%s (%d transactions)"); 
		tmpstr = gsb_real_get_string_with_currency (montant_sous_ib_etat, 
							    devise_ib_etat, TRUE  );
		text = g_strdup_printf ( fmtstr, tmpstr, 
					 nb_ope_sous_ib_etat );
		g_free ( tmpstr );
	    }
	    else
		text = gsb_real_get_string_with_currency ( montant_sous_ib_etat, devise_ib_etat, TRUE );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;
	}
    }

    montant_sous_ib_etat = null_real;
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
    gint current_report_number;
	gchar* tmpstr;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( gsb_data_report_get_account_group_reports (current_report_number)
	 &&
	 gsb_data_report_get_account_show_amount (current_report_number))
    {
	/* si rien n'est affiché en dessous du compte, on */
	/* met le résultat sur la ligne du compte */
	/* sinon on fait une barre et on met le rÃ©sultat */

	if ( gsb_data_report_get_payee_used (current_report_number)
	     ||
	     gsb_data_report_get_show_report_transactions (current_report_number))
	{
	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_compte_en_cours )
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_compte_etat <= 1 )
			fmtstr = _("Total %s (%d transaction)");
		    else
			fmtstr = _("Total %s (%d transactions)");
		    text = g_strdup_printf ( fmtstr, nom_compte_en_cours, nb_ope_compte_etat );
		}
		else
		    text = g_strconcat ( _("Total "), nom_compte_en_cours, NULL );
	    }
	    else
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_compte_etat <= 1 )
			fmtstr = _("Account total: (%d transaction): ");
		    else
			fmtstr = _("Account total: (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nb_ope_compte_etat );
		}
		else
		    text = g_strdup( _("Account total: ") );
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( text );

	    text = gsb_real_get_string_with_currency (montant_compte_etat, devise_compte_en_cours_etat, TRUE );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_compte_etat <= 1 )
		    fmtstr = _("%s (%d transaction)"); 
		else
		    fmtstr = _("%s (%d transactions)"); 
		tmpstr = gsb_real_get_string_with_currency ( montant_compte_etat,
				devise_compte_en_cours_etat, TRUE  );
		text = g_strdup_printf ( fmtstr, tmpstr, 
					     nb_ope_compte_etat );
	        g_free ( tmpstr );
	    }
	    else
		text = gsb_real_get_string_with_currency ( montant_compte_etat, devise_compte_en_cours_etat, TRUE );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );  
	    g_free ( text );
	    ligne++;
	}
    }

    montant_compte_etat = null_real;
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
    gint current_report_number;
	gchar* tmpstr;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( gsb_data_report_get_payee_used (current_report_number)
	 &&
	 gsb_data_report_get_payee_show_payee_amount (current_report_number))
    {
	/* si rien n'est affiché en dessous du tiers, on */
	/* met le résultat sur la ligne du tiers */
	/* sinon on fait une barre et on met le résultat */

	if ( gsb_data_report_get_show_report_transactions (current_report_number))
	{

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	    ligne++;

	    etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	    ligne++;

	    if ( nom_tiers_en_cours )
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_tiers_etat <= 1 )
			fmtstr = _("Total %s (%d transaction)");
		    else
			fmtstr = _("Total %s (%d transactions)");
		    text = g_strdup_printf ( fmtstr, nom_tiers_en_cours, nb_ope_tiers_etat );
		}
		else
		    text = g_strdup_printf ( _("Total %s"), nom_tiers_en_cours );
	    }
	    else
	    {
		if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
		{
		    gchar* fmtstr;
		    if ( nb_ope_tiers_etat <= 1 )
			fmtstr = _("Payee total: (%d transaction): ");
		    else
			fmtstr = _("Payee total: (%d transactions): ");
		    text = g_strdup_printf ( fmtstr, nb_ope_tiers_etat );
		}
		else
		    text = g_strdup( _("Payee total: ") );
	    }

	    etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( text );

	    text = gsb_real_get_string_with_currency (montant_tiers_etat, devise_tiers_etat, TRUE );
	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;

	    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	    ligne++;
	}
	else
	{
	    ligne--;

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_tiers_etat <= 1 )
		    fmtstr = _("%s (%d transaction)"); 
		else
		    fmtstr = _("%s (%d transactions)"); 
		tmpstr = gsb_real_get_string_with_currency (montant_tiers_etat,
										devise_tiers_etat, TRUE  );
		text = g_strdup_printf ( fmtstr, tmpstr, nb_ope_tiers_etat );
	        g_free ( tmpstr );
	    }
	    else
		text = gsb_real_get_string_with_currency (montant_tiers_etat, devise_tiers_etat, TRUE );

	    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	    g_free ( text );
	    ligne++;
	}
    }

    montant_tiers_etat = null_real;
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
gint etat_affiche_affiche_total_periode ( gint transaction_number, 
					  gint ligne,
					  gint force )
{
    gint current_report_number;
	gchar* dtstr1;
	gchar* dtstr2;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( gsb_data_report_get_period_split (current_report_number))
    {
	gchar *text = NULL;
	const GDate *date;
    gint jour_debut_semaine;

	date = gsb_data_transaction_get_date (transaction_number);

    /* on récupère ici le premier jour de la semaine */
    jour_debut_semaine = gsb_data_report_get_period_split_day ( current_report_number ) + 1;

	/* si la date de début de période est nulle, on la met au début de la période la date de l'opération */
	if ( !date_debut_periode )
	{
	    if ( transaction_number )
	    {
		/*  il y a une opération, on va rechercher le début de la période qui la contient */
		/* ça peut être le début de la semaine, du mois ou de l'année de l'opé */

		switch ( gsb_data_report_get_period_split_type (current_report_number))
		{
		    case 0:
			/* split by day, set the day of the transaction */
			date_debut_periode = g_date_new_dmy ( g_date_get_day ( date),
							      g_date_get_month ( date),
							      g_date_get_year ( date));
			break;

		    case 1:
			/* séparation par semaine : on recherche le début de la semaine qui contient l'opé */
			date_debut_periode = g_date_new_dmy ( g_date_get_day ( date),
							      g_date_get_month ( date),
							      g_date_get_year ( date));

            if ( g_date_get_weekday ( date_debut_periode )  != jour_debut_semaine )
			{
                do 
                {
                    g_date_subtract_days ( date_debut_periode, 1 );
                }
                while ( g_date_get_weekday ( date_debut_periode )  != jour_debut_semaine );
			}
			break;

		    case 2:
			/* séparation par mois */

			date_debut_periode = g_date_new_dmy ( 1,
							      g_date_get_month ( date),
							      g_date_get_year ( date));
			break;

		    case 3:
			/* séparation par an */

			date_debut_periode = g_date_new_dmy ( 1,
							      1,
							      g_date_get_year ( date));
			break;
		}
	    }
	    else
		date_debut_periode = NULL;
	    return ( ligne );
	}

	/* on vérifie maintenant s'il faut afficher un total ou pas */
	switch ( gsb_data_report_get_period_split_type (current_report_number))
	{
	    gchar buffer[60];
	    GDate *date_tmp;

	    case 0:
	    /* split by day, we do nothing only if it's the same day as the transaction before */
	    if ( !force
		 &&
		 !g_date_compare ( date,
				   date_debut_periode ))
		return ( ligne );

	    /* ok, not the same day, we show a separation */
	    g_date_strftime ( buffer,
			      59,
			      "%A %d %B %Y",
			      date_debut_periode );

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_periode_etat <= 1 )
		    fmtstr = _("Result of %s (%d transaction): ");
		else
		    fmtstr = _("Result of %s (%d transactions): ");
		text = g_strdup_printf ( fmtstr, buffer, nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( _("Result of %s: "), buffer );


	    break;

	    case 1:
	    /* séparation par semaine */

	    /* 	  si c'est le même jour que l'opé précédente, on fait rien */

	    if ( !force
		 &&
		 !g_date_compare ( date,
				   date_debut_periode ))
		return ( ligne );

	    /* 	  si on est en début de semaine, on met un sous total */

	    date_tmp = g_date_new_dmy ( g_date_get_day ( date_debut_periode ),
					g_date_get_month ( date_debut_periode ),
					g_date_get_year ( date_debut_periode ));
	    g_date_add_days ( date_tmp,
			      7 );

	    if ( !force
		 &&
		 ( g_date_get_weekday ( date)  != (gsb_data_report_get_period_split_day (current_report_number)+ 1 )
		   &&
		   g_date_compare ( date,
				    date_tmp ) < 0 ))
		return ( ligne );

	    /* on doit retrouver la date du début de semaine et y ajouter 6j pour afficher la période */

	    if ( g_date_get_weekday ( date_debut_periode )  != jour_debut_semaine )
			{
                do 
                {
                    g_date_subtract_days ( date_debut_periode, 1 );
                }
                while ( g_date_get_weekday ( date_debut_periode )  != jour_debut_semaine );
			}


	    g_date_free ( date_tmp );

	    date_tmp = g_date_new_dmy ( g_date_get_day ( date_debut_periode ),
					g_date_get_month ( date_debut_periode ),
					g_date_get_year ( date_debut_periode ));
	    g_date_add_days ( date_tmp,
			      6 );
	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_periode_etat <= 1 )
		    fmtstr = _("Result from %s to %s (%d transaction): ");
		else
		    fmtstr = _("Result from %s to %s (%d transactions): ");
		dtstr1 = gsb_format_gdate ( date_debut_periode );
		dtstr2 = gsb_format_gdate ( date_tmp );
		text = g_strdup_printf ( fmtstr, dtstr1, dtstr2, nb_ope_periode_etat );
	        g_free ( dtstr1 );
	        g_free ( dtstr2 );
	    }
	    else
	    {
	        gchar* dtstr1 = gsb_format_gdate ( date_debut_periode );
	        gchar* dtstr2 = gsb_format_gdate ( date_tmp );
		text = g_strdup_printf ( _("Result from %s to %s: "), dtstr1, dtstr2);
	        g_free ( dtstr1 );
	        g_free ( dtstr2 );
	    }

	    break;

	    case 2:
	    /* séparation par mois */

	    if ( !force
		 &&
		 g_date_get_month ( date) == g_date_get_month ( date_debut_periode ) )
		return ( ligne );

	    g_date_strftime ( buffer,
			      59,
			      "%B %Y",
			      date_debut_periode );

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_periode_etat <= 1 )
		    fmtstr = _("Result of %s (%d transaction): ");
		else
		    fmtstr = _("Result of %s (%d transactions): ");
		text = g_strdup_printf ( fmtstr, buffer, nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( _("Result of %s: "), buffer );

	    break;

	    case 3:
	    /* séparation par an */

	    if ( !force
		 &&
		 g_date_get_year ( date) == g_date_get_year ( date_debut_periode ) )
		return ( ligne );

	    g_date_strftime ( buffer,
			      59,
			      "%Y",
			      date_debut_periode );

	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_periode_etat <= 1 )
		    fmtstr = _("Result for %s (%d transaction): ");
		else
		    fmtstr = _("Result for %s (%d transactions): ");
		text = g_strdup_printf ( fmtstr, buffer, nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( _("Result for %s: "), buffer );
	    break;
	}

	/*       si on arrive ici, c'est qu'il y a un chgt de période ou que c'est forcé */

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	ligne++;

	etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	ligne++;

	etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	g_free ( text );

	text = gsb_real_get_string_with_currency (montant_periode_etat, devise_generale_etat, TRUE );
	etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	g_free ( text );
	ligne++;

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	ligne++;

	montant_periode_etat = null_real;
	nb_ope_periode_etat = 0;

	/* comme il y a un changement de période, on remet la date_debut_periode à celle du début de la période */
	/* de l'opération  en cours */


	if ( transaction_number )
	{
	    /*  il y a une opération, on va rechercher le début de la période qui la contient */
	    /* ça peut être le début de la semaine, du mois ou de l'année de l'opé */
	    switch ( gsb_data_report_get_period_split_type (current_report_number))
	    {
		case 0:
		    /* split by day, set on the transactions day */
		    date_debut_periode = g_date_new_dmy ( g_date_get_day ( date),
							  g_date_get_month ( date),
							  g_date_get_year ( date));
		    break;
		case 1:
		    /* séparation par semaine : on recherche le début de la semaine qui contient l'opé */

		    date_debut_periode = g_date_new_dmy ( g_date_get_day ( date),
							  g_date_get_month ( date),
							  g_date_get_year ( date));

		    if ( g_date_get_weekday ( date_debut_periode )  != jour_debut_semaine )
			{
                do 
                {
                    g_date_subtract_days ( date_debut_periode, 1 );
                }
                while ( g_date_get_weekday ( date_debut_periode )  != jour_debut_semaine );
			}
		    break;

		case 2:
		    /* séparation par mois */

		    date_debut_periode = g_date_new_dmy ( 1,
							  g_date_get_month ( date),
							  g_date_get_year ( date));
		    break;

		case 3:
		    /* séparation par an */

		    date_debut_periode = g_date_new_dmy ( 1,
							  1,
							  g_date_get_year ( date));
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
gint etat_affiche_affiche_total_exercice ( gint transaction_number,
					   gint ligne,
					   gint force )
{
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( gsb_data_report_get_financial_year_split (current_report_number))
    {
	gchar *text;

	text = NULL;

	/* 	si l'exo précédent est -1, on le met à l'exo de l'opé */
	/* 	utilise ça car des opés peuvent ne pas avoir d'exo */

	if ( exo_en_cours_etat == -1 )
	{
	    exo_en_cours_etat = gsb_data_transaction_get_financial_year_number (transaction_number);
	    return ligne;
	}

	/* on vérifie maintenant s'il faut afficher un total ou pas */

	if ( !force
	     &&
	     gsb_data_transaction_get_financial_year_number (transaction_number) == exo_en_cours_etat )
	    return ligne;

	if ( exo_en_cours_etat )
	{
	    /* 	    les opés ont un exo */
	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_periode_etat <= 1 )
		    fmtstr = _("Result of %s (%d transaction): ");
		else
		    fmtstr = _("Result of %s (%d transactions): ");
		text = g_strdup_printf ( fmtstr,
					     gsb_data_fyear_get_name ( exo_en_cours_etat ),
					     nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( _("Result of %s: "),
					 gsb_data_fyear_get_name ( exo_en_cours_etat ));
	}
	else
	{
	    /* 	    les opés n'ont pas d'exo */
	    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	    {
	        gchar* fmtstr;
		if ( nb_ope_periode_etat <= 1 )
		    fmtstr = _("Result without financial year (%d transaction): ");
		else
		    fmtstr = _("Result without financial year (%d transactions): ");
		text = g_strdup_printf ( fmtstr, nb_ope_periode_etat );
	    }
	    else
		text = g_strdup_printf ( "%s", _("Result without financial year: ") );
	}



	/*       si on arrive ici, c'est qu'il y a un chgt de période ou que c'est forcé */

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	ligne++;

	etat_affiche_attach_hsep ( 1, nb_colonnes, ligne, ligne + 1 );
	ligne++;

	etat_affiche_attach_label ( text, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	g_free ( text );

	text = gsb_real_get_string_with_currency (montant_exo_etat, devise_generale_etat, TRUE );
	etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	g_free ( text );
	ligne++;

	etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	ligne++;

	montant_exo_etat = null_real;
	nb_ope_exo_etat = 0;

	/* comme il y a un changement d'état, on remet exo_en_cours_etat à celle de l'opé en cours */

	if (transaction_number)
	    exo_en_cours_etat = gsb_data_transaction_get_financial_year_number (transaction_number);
	else
	    date_debut_periode = NULL;
    }

    return (ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affichage_ligne_ope ( gint transaction_number,
					gint ligne )
{
    gint colonne;
    gchar *text;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    /* on met tous les labels dans un event_box pour aller directement sur l'opé si elle est clickée */

    if ( gsb_data_report_get_show_report_transactions (current_report_number))
    {
	/* on affiche ce qui est demandé pour les opés */
	/* si les titres ne sont pas affichés et qu'il faut le faire, c'est ici */

	if ( !titres_affiches
	     &&
	     gsb_data_report_get_column_title_show (current_report_number)
	     &&
	     gsb_data_report_get_column_title_type (current_report_number))
	    ligne = etat_affiche_affiche_titres_colonnes ( ligne );

	colonne = 1;

	/*       pour chaque info, on vérifie si on l'opé doit être clickable */
	/* si c'est le cas, on place le label dans un event_box */

	if ( gsb_data_report_get_show_report_transaction_number (current_report_number))
	{
	    text = utils_str_itoa ( transaction_number);

	    if ( gsb_data_report_get_report_can_click (current_report_number))
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
	    }
	    else
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	    g_free (text);
	}

	if ( gsb_data_report_get_show_report_date (current_report_number))
	{
	    text = gsb_format_gdate ( gsb_data_transaction_get_date (transaction_number));

	    if ( gsb_data_report_get_report_can_click (current_report_number))
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
	    }
	    else
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	    if (text)
		g_free (text);
	}

	if ( gsb_data_report_get_show_report_value_date (current_report_number))
	{
	    text = gsb_format_gdate ( gsb_data_transaction_get_value_date (transaction_number));

	    if ( gsb_data_report_get_report_can_click (current_report_number))
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
	    }
	    else
	    {
		etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	    if (text)
		g_free (text);
	}

	if ( gsb_data_report_get_show_report_financial_year (current_report_number))
	{
	    if ( gsb_data_transaction_get_financial_year_number ( transaction_number))
	    {
		text = my_strdup (gsb_data_fyear_get_name ( gsb_data_transaction_get_financial_year_number ( transaction_number)));

		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		if (text)
		    g_free (text);
	    }
	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( gsb_data_report_get_show_report_payee (current_report_number))
	{
	    if ( gsb_data_transaction_get_party_number ( transaction_number))
	    {
		text = my_strdup (gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number), TRUE ));

		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		if (text)
		    g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( gsb_data_report_get_show_report_category (current_report_number))
	{
	    if ( gsb_data_transaction_get_category_number (transaction_number))
		text = my_strdup (gsb_data_category_get_name ( gsb_data_transaction_get_category_number (transaction_number),
								  gsb_data_transaction_get_sub_category_number (transaction_number),
								  NULL ));
	    else
	    {
		/* si c'est un virement, on le marque, sinon c'est qu'il n'y a pas de categ */
		/* ou que c'est une opé ventilée, et on marque rien */

		if ( gsb_data_transaction_get_contra_transaction_number ( transaction_number) > 0)
		{
		    /* c'est un virement */

		    gchar* fmtstr;
		    if ( gsb_data_transaction_get_amount ( transaction_number).mantissa < 0 )
			fmtstr = _("Transfer to %s");
		    else
			fmtstr =  _("Transfer from %s");
		    text = g_strdup_printf ( fmtstr, gsb_data_account_get_name (
		    	gsb_data_transaction_get_contra_transaction_account ( transaction_number)) );
		}
		else
		    text = NULL;
	    }

	    if ( text )
	    {
		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( gsb_data_report_get_show_report_budget (current_report_number))
	{
	    if ( gsb_data_transaction_get_budgetary_number ( transaction_number))
	    {
		text = my_strdup (gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( transaction_number),
								 gsb_data_transaction_get_sub_budgetary_number ( transaction_number),
								NULL ));

		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		if (text)
		    g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( gsb_data_report_get_show_report_note (current_report_number))
	{
	    if ( gsb_data_transaction_get_notes ( transaction_number))
	    {
		text = my_strdup (gsb_data_transaction_get_notes ( transaction_number));

		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		if (text)
		    g_free (text);
	    }
	    else
	    {
		etat_affiche_attach_label ( NULL, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( gsb_data_report_get_show_report_method_of_payment (current_report_number))
	{
	    text = my_strdup (gsb_data_payment_get_name (gsb_data_transaction_get_method_of_payment_number ( transaction_number)));
	    if (text)
	    {
		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( gsb_data_report_get_show_report_method_of_payment_content (current_report_number))
	{
	    /* Si l'opération est une opération de ventilation, il faut rechercher
	       l'opération mère pour pouvoir récupérer le n° du chèque */
	    if ( gsb_data_transaction_get_mother_transaction_number ( transaction_number))
	    {
		GSList *list_tmp_transactions;
		gboolean found = FALSE;

		/* On récupère donc la liste des opérations du compte et on en fait
		   le tour jusqu'à ce qu'on trouve l'opération mère */

		list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

		while ( list_tmp_transactions )
		{
		    gint transaction_number_tmp;
		    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (transaction_number))
		    {
			if ( gsb_data_transaction_get_split_of_transaction (transaction_number_tmp)
			     &&
			     transaction_number_tmp == gsb_data_transaction_get_mother_transaction_number ( transaction_number)
			     &&
			     gsb_data_transaction_get_method_of_payment_content (transaction_number_tmp))
			{
			    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
										 gsb_data_transaction_get_method_of_payment_content (transaction_number_tmp));
			    found = TRUE;
			}
		    }
		    list_tmp_transactions = list_tmp_transactions -> next;
		}
	    }

	    if ( gsb_data_transaction_get_method_of_payment_content ( transaction_number))
	    {
		text = my_strdup (gsb_data_transaction_get_method_of_payment_content ( transaction_number));

		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		if (text)
		    g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	if ( gsb_data_report_get_show_report_voucher (current_report_number))
	{
	    if ( gsb_data_transaction_get_voucher ( transaction_number))
	    {
		text = my_strdup (gsb_data_transaction_get_voucher ( transaction_number));

		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		if (text)
		    g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}



	if ( gsb_data_report_get_show_report_bank_references (current_report_number))
	{
	    if ( gsb_data_transaction_get_bank_references ( transaction_number))
	    {
		text = my_strdup (gsb_data_transaction_get_bank_references ( transaction_number));

		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		if (text)
		    g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}

	if ( gsb_data_report_get_show_report_marked (current_report_number))
	{
	    text = my_strdup (gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( transaction_number)));

	    if ( text )
	    {
		if ( gsb_data_report_get_report_can_click (current_report_number))
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, transaction_number );
		}
		else
		{
		    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
		}
		g_free (text);
	    }

	    etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	    colonne = colonne + 2;
	}


	/* on affiche le montant au bout de la ligne */

	text = gsb_real_get_string_with_currency (gsb_data_transaction_get_amount (transaction_number),
						  gsb_data_transaction_get_currency_number (transaction_number), TRUE );

	if ( gsb_data_report_get_report_can_click (current_report_number))
	{
	    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_RIGHT, transaction_number );
	}
	else
	{
	    etat_affiche_attach_label ( text, TEXT_NORMAL, colonne, colonne + 1, ligne, ligne + 1, ALIGN_RIGHT, 0 );
	}
	if (text)
	    g_free (text);


	if ( ligne_debut_partie == -1 )
	    ligne_debut_partie = ligne;

	ligne++;
    }
    return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affiche_total_partiel ( gsb_real total_partie,
					  gint ligne,
					  gint type )
{
    gchar * text;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    if ( type )
    {
	if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	{
	    gchar* fmtstr;
	    if ( nb_ope_partie_etat <= 1 )
		fmtstr = _("Total expenses (%d transaction): ");
	    else
		fmtstr = _("Total expenses (%d transactions): ");
	    text = g_strdup_printf ( fmtstr, nb_ope_partie_etat );
	}
	else
	    text = g_strdup( _("Total expenses: ") );
    }
    else
    {
	if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
	{
	    gchar* fmtstr;
	    if ( nb_ope_partie_etat <= 1 )
		fmtstr = _("Total income (%d transaction): ");
	    else
		fmtstr = _("Total income (%d transactions): ");
	    text = g_strdup_printf ( fmtstr, nb_ope_partie_etat );
	}
	else
	    text = g_strdup( _("Total income: ") );
    }

    etat_affiche_attach_label ( text, TEXT_NORMAL, 0, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
    g_free ( text );

    text = gsb_real_get_string_with_currency (total_partie, devise_generale_etat, TRUE );
    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
    g_free ( text );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes - 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
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
gint etat_affiche_affiche_total_general ( gsb_real total_general,
					  gint ligne )
{
    gchar * text;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes, ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    if ( gsb_data_report_get_show_report_transaction_amount (current_report_number))
    {
        gchar* fmtstr;
	if ( nb_ope_general_etat <= 1 )
	    fmtstr = _("General total (%d transaction): ");
	else
	    fmtstr = _("General total (%d transactions): ");
	text = g_strdup_printf ( fmtstr, nb_ope_general_etat );
    }
    else
	text = g_strdup( _("General total: ") );

    etat_affiche_attach_label ( text, TEXT_NORMAL, 0, nb_colonnes - 1, ligne, ligne + 1, ALIGN_LEFT, 0 );
    g_free ( text );

    text = gsb_real_get_string_with_currency (total_general, devise_generale_etat, TRUE );
    etat_affiche_attach_label ( text, TEXT_NORMAL, nb_colonnes - 1, nb_colonnes, ligne, ligne + 1, ALIGN_RIGHT, 0 );
    g_free ( text );
    ligne++;

    etat_affiche_attach_hsep ( 0, nb_colonnes, ligne, ligne + 1 );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 1, nb_colonnes, ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint etat_affiche_affiche_categ_etat ( gint transaction_number,
				       gchar *decalage_categ,
				       gint ligne )
{
    gchar *pointeur_char;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    /* vérifie qu'il y a un changement de catégorie */
    /* ça peut être aussi chgt pour virement, ventilation ou pas de categ */

    if ( gsb_data_report_get_category_used (current_report_number)
	 &&
	 ( gsb_data_transaction_get_category_number ( transaction_number)!= ancienne_categ_etat
	   ||
	   ( ancienne_categ_speciale_etat == 1
	     &&
	     gsb_data_transaction_get_contra_transaction_number ( transaction_number) == 0)
	   ||
	   ( ancienne_categ_speciale_etat == 2
	     &&
	     !gsb_data_transaction_get_split_of_transaction ( transaction_number))
	   ||
	   ( ancienne_categ_speciale_etat == 3
	     &&
	     ( gsb_data_transaction_get_split_of_transaction ( transaction_number)
	       ||
	       gsb_data_transaction_get_contra_transaction_number ( transaction_number) > 0))))
    {

	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( transaction_number,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 1,
							       ligne );

	    /* on ajoute le total de la categ */

	    ligne = etat_affiche_affiche_total_categories ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des catég, on saute la partie suivante */

	if ( gsb_data_report_get_category_show_name (current_report_number))
	{
	    if ( gsb_data_transaction_get_category_number ( transaction_number))
	    {
		nom_categ_en_cours = gsb_data_category_get_name ( gsb_data_transaction_get_category_number (transaction_number),
								  0,
								  NULL );
		pointeur_char = g_strconcat ( decalage_categ,
					      nom_categ_en_cours,
					      NULL );
		ancienne_categ_speciale_etat = 0;
	    }
	    else
	    {
		if ( gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0)
		{
		    pointeur_char = g_strconcat ( decalage_categ,
						  _("Transfer"),
						  NULL );
		    ancienne_categ_speciale_etat = 1;
		}
		else
		{
		    if ( gsb_data_transaction_get_split_of_transaction (transaction_number))
		    {
			pointeur_char = g_strconcat ( decalage_categ,
						      _("Split of transaction"),
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
					ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( pointeur_char );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 1 );

	ancienne_categ_etat = gsb_data_transaction_get_category_number (transaction_number);

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint etat_affiche_affiche_sous_categ_etat ( gint transaction_number,
					    gchar *decalage_sous_categ,
					    gint ligne )
{
    gchar *pointeur_char;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( gsb_data_report_get_category_used (current_report_number)
	 &&
	 gsb_data_report_get_category_show_sub_category (current_report_number)
	 &&
	 gsb_data_transaction_get_category_number (transaction_number)
	 &&
	 gsb_data_transaction_get_sub_category_number (transaction_number) != ancienne_sous_categ_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( transaction_number,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière la sous catégorie */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 2,
							       ligne );

	    /* on ajoute le total de la sous categ */

	    ligne = etat_affiche_affiche_total_sous_categ ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des ss-catég, on saute la partie suivante */

	if ( gsb_data_report_get_category_show_name (current_report_number))
	{
	    gchar *sub_categ = gsb_data_category_get_sub_category_name ( gsb_data_transaction_get_category_number (transaction_number),
									 gsb_data_transaction_get_sub_category_number (transaction_number),
									 0 );
	    if (sub_categ)
	    {
		pointeur_char = g_strconcat ( decalage_sous_categ,
					      sub_categ,
					      NULL );
	    }
	    else
	    {
		if ( gsb_data_report_get_category_show_without_category (current_report_number))
		    pointeur_char = g_strconcat ( decalage_sous_categ,
						  _("No subcategory"),
						  NULL );
		else
		    pointeur_char = my_strdup ("");
	    }

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free (pointeur_char);
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 2 );

	ancienne_sous_categ_etat = gsb_data_transaction_get_sub_category_number (transaction_number );

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_ib_etat ( gint transaction_number,
				    gchar *decalage_ib,
				    gint ligne )
{
    gchar *pointeur_char;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    /* mise en place de l'ib */


    if ( gsb_data_report_get_budget_used (current_report_number)
	 &&
	 gsb_data_transaction_get_budgetary_number (transaction_number) != ancienne_ib_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( transaction_number,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière l'ib */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 3,
							       ligne );

	    /* on ajoute le total de l'ib */

	    ligne = etat_affiche_affiche_total_ib ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des ib, on saute la partie suivante */

	if ( gsb_data_report_get_budget_show_name (current_report_number))
	{
	    if ( gsb_data_transaction_get_budgetary_number (transaction_number ))
	    {
		nom_ib_en_cours = gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number (transaction_number),
							     0,
							     NULL );

		pointeur_char = g_strconcat ( decalage_ib,
					      nom_ib_en_cours,
					      NULL );
	    }
	    else
		pointeur_char = g_strconcat ( decalage_ib,
					      _("No budgetary line"),
					      NULL );

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( pointeur_char );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 3 );

	ancienne_ib_etat = gsb_data_transaction_get_budgetary_number (transaction_number);

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_sous_ib_etat ( gint transaction_number,
					 gchar *decalage_sous_ib,
					 gint ligne )
{
    gchar *pointeur_char;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();



    /* mise en place de la sous_ib */


    if ( gsb_data_report_get_budget_used (current_report_number)
	 &&
	 gsb_data_report_get_budget_show_sub_budget (current_report_number)
	 &&
	 gsb_data_transaction_get_budgetary_number (transaction_number)
	 &&
	 gsb_data_transaction_get_sub_budgetary_number (transaction_number) != ancienne_sous_ib_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( transaction_number,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière la sous ib */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 4,
							       ligne );

	    /* on ajoute le total de la sous ib */

	    ligne = etat_affiche_affiche_total_sous_ib ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des ss-ib, on saute la partie suivante */

	if ( gsb_data_report_get_budget_show_name (current_report_number))
	{
	    nom_ss_ib_en_cours = gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_budgetary_number (transaction_number),
								       gsb_data_transaction_get_sub_budgetary_number (transaction_number),
								       0 );
	    if ( nom_ss_ib_en_cours )
		pointeur_char = g_strconcat ( decalage_sous_ib,
					      nom_ss_ib_en_cours,
					      NULL );
	    else
	    {
		if ( gsb_data_report_get_budget_show_without_budget (current_report_number))
		    pointeur_char = g_strconcat ( decalage_sous_ib,
						  _("No sub-budgetary line"),
						  NULL );
		else
		    pointeur_char = g_strdup("");
	    }

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( pointeur_char );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 4 );

	ancienne_sous_ib_etat = gsb_data_transaction_get_sub_budgetary_number (transaction_number);

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_compte_etat ( gint transaction_number,
					gchar *decalage_compte,
					gint ligne )
{
    gchar *pointeur_char;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    /* mise en place du compte */

    if ( gsb_data_report_get_account_group_reports (current_report_number)
	 &&
	 gsb_data_transaction_get_account_number (transaction_number) != ancien_compte_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( transaction_number,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière le compte */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 5,
							       ligne );

	    /* on ajoute le total du compte */

	    ligne = etat_affiche_affiche_total_compte ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des comptes, on saute la partie suivante */

	if ( gsb_data_report_get_account_show_name (current_report_number))
	{
	    pointeur_char = g_strconcat ( decalage_compte,
					  gsb_data_account_get_name (gsb_data_transaction_get_account_number (transaction_number)),
					  NULL );
	    nom_compte_en_cours = gsb_data_account_get_name (gsb_data_transaction_get_account_number (transaction_number));

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( pointeur_char );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 5 );

	ancien_compte_etat = gsb_data_transaction_get_account_number (transaction_number);

	debut_affichage_etat = 0;
	changement_de_groupe_etat = 1;
    }

    return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint etat_affiche_affiche_tiers_etat ( gint transaction_number,
				       gchar *decalage_tiers,
				       gint ligne )
{
    gchar *pointeur_char;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    /* affiche le tiers */

    if ( gsb_data_report_get_payee_used (current_report_number)
	 &&
	 gsb_data_transaction_get_party_number (transaction_number) != ancien_tiers_etat )
    {
	/* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

	if ( !debut_affichage_etat
	     &&
	     !changement_de_groupe_etat )
	{
	    /* on affiche le total de la période en le forçant */

	    ligne = etat_affiche_affiche_total_periode ( transaction_number,
							 ligne,
							 1 );

	    /* on ajoute les totaux de tout ce qu'il y a derrière le tiers */

	    ligne = etat_affiche_affiche_totaux_sous_jaccent ( 6,
							       ligne );

	    /* on ajoute le total du tiers */

	    ligne = etat_affiche_affiche_total_tiers ( ligne );
	}

	/*       si on a demandé de ne pas afficher les noms des tiers, on saute la partie suivante */

	if ( gsb_data_report_get_payee_show_name (current_report_number))
	{
	    if ( gsb_data_transaction_get_party_number (transaction_number))
	    {
		nom_tiers_en_cours = gsb_data_payee_get_name ( gsb_data_transaction_get_party_number (transaction_number), TRUE );

		pointeur_char = g_strconcat ( decalage_tiers,
					      nom_tiers_en_cours,
					      NULL );
	    }
	    else
		pointeur_char = g_strconcat ( decalage_tiers,
					      _("No payee"),
					      NULL );

	    etat_affiche_attach_label ( pointeur_char, TEXT_NORMAL, 0, nb_colonnes-1, 
					ligne, ligne + 1, ALIGN_LEFT, 0 );
	    g_free ( pointeur_char );
	    ligne++;
	}

	ligne_debut_partie = ligne;
	denote_struct_sous_jaccentes ( 6 );

	ancien_tiers_etat = gsb_data_transaction_get_party_number (transaction_number);

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
				ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    etat_affiche_attach_label ( _("Incomes"), TEXT_LARGE, 0, nb_colonnes-1, 
				ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 0, 1, 
				ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint etat_affiche_affiche_titre_depenses_etat ( gint ligne )
{
    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 0, 1, 
				ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    etat_affiche_attach_label ( _("Outgoings"), TEXT_LARGE, 0, nb_colonnes-1, 
				ligne, ligne + 1, ALIGN_CENTER, 0 );
    ligne++;

    etat_affiche_attach_label ( NULL, TEXT_NORMAL, 0, 1, 
				ligne, ligne + 1, ALIGN_CENTER, 0 );
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
    GSList *pointeur_glist;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    /* on doit partir du bout de la liste pour revenir vers la structure demandée */
    
    pointeur_glist = g_slist_reverse (g_slist_copy ( gsb_data_report_get_sorting_type (current_report_number)));

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
	pointeur_glist = pointeur_glist -> next;
    }

    g_slist_free (pointeur_glist);
    return ( ligne );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint etat_affiche_affiche_titres_colonnes ( gint ligne )
{
    gint colonne;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    colonne = 1;

    if ( gsb_data_report_get_show_report_transaction_number (current_report_number))
    {
	etat_affiche_attach_label ( _("Number"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_date (current_report_number))
    {
	etat_affiche_attach_label ( _("Date"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_value_date (current_report_number))
    {
	etat_affiche_attach_label ( _("Value date"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_financial_year (current_report_number))
    {
	etat_affiche_attach_label ( _("Financial year"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_payee (current_report_number))
    {
	etat_affiche_attach_label ( _("Payee"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_category (current_report_number))
    {
	etat_affiche_attach_label ( _("Category"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_budget (current_report_number))
    {
	etat_affiche_attach_label ( _("Budgetary line"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_note (current_report_number))
    {
	etat_affiche_attach_label ( _("Notes"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_method_of_payment (current_report_number))
    {
	etat_affiche_attach_label ( _("Payment methods"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_method_of_payment_content (current_report_number))
    {
	etat_affiche_attach_label ( _("Cheque"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_voucher (current_report_number))
    {
	etat_affiche_attach_label ( _("Voucher"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_bank_references (current_report_number))
    {
	etat_affiche_attach_label ( _("Bank references"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
	etat_affiche_attach_vsep ( colonne + 1, colonne + 2, ligne, ligne + 1 );
	colonne = colonne + 2;
    }

    if ( gsb_data_report_get_show_report_marked (current_report_number))
    {
	etat_affiche_attach_label ( _("Statement"), TEXT_BOLD, colonne, colonne + 1, ligne, ligne + 1, ALIGN_CENTER, 0 );
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



void etat_affiche_attach_label ( gchar * text, gint properties, int x, int x2, int y, int y2, 
				 enum alignement align, gint transaction_number )
{
    etat_affichage_output -> attach_label ( text, properties, x, x2, y, y2, align, transaction_number );
}



gint etat_affiche_initialise (GSList * opes_selectionnees, gchar * filename )
{
    return etat_affichage_output -> initialise ( opes_selectionnees, filename );
}



gint etat_affiche_finish ()
{
    return etat_affichage_output -> finish ();
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
