/* This file mange cvs export format */

/*     Copyright (C) 2004-  François Terror */
/* 			francois.terrot at grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "csv.h"


#include "devises.h"
#include "dialog.h"
#include "search_glist.h"
#include "utils.h"
#include "utils_files.h"
#include "exercice.h"

#define CSV_CLEAR_FIELD(a)  if (a) { g_free(a);  a = NULL; }
#define CSV_STR_FIELD(f,a)  if (a) { fprintf(f,"\"%s\"%c",g_locale_from_utf8(a,-1,NULL,NULL,NULL),g_csv_field_separator); } else { fprintf(f,"\"\"%c",g_csv_field_separator); }
#define CSV_NUM_FIELD(f,a)  if (a) { fprintf(f,"%s%c",g_locale_from_utf8(a,-1,NULL,NULL,NULL),g_csv_field_separator); } else { fprintf(f,"0%c",g_csv_field_separator); }
#define CSV_END_RECORD(f)  fprintf(f,"\n")

static gboolean g_csv_with_title_line = TRUE;
static gchar    g_csv_field_separator = ';';

gchar*  csv_field_operation  = NULL;
gchar*  csv_field_ventil     = NULL;
gchar*  csv_field_date       = NULL;
gchar*  csv_field_pointage   = NULL;
gchar*  csv_field_tiers      = NULL;
gchar*  csv_field_credit     = NULL;
gchar*  csv_field_debit      = NULL;
gchar*  csv_field_montant    = NULL;
gchar*  csv_field_solde      = NULL;
gchar*  csv_field_categ      = NULL;
gchar*  csv_field_sous_categ = NULL;
gchar*  csv_field_notes      = NULL;
gchar*  csv_field_exercice   = NULL;

static void _csv_clear_fields(gboolean clear_all)
{
    if (clear_all)
    {
        CSV_CLEAR_FIELD(csv_field_date);
        CSV_CLEAR_FIELD(csv_field_pointage);
        CSV_CLEAR_FIELD(csv_field_operation);
        CSV_CLEAR_FIELD(csv_field_tiers);
        CSV_CLEAR_FIELD(csv_field_solde);
    }

    CSV_CLEAR_FIELD(csv_field_notes);
    CSV_CLEAR_FIELD(csv_field_debit);
    CSV_CLEAR_FIELD(csv_field_credit);
    CSV_CLEAR_FIELD(csv_field_montant);
    CSV_CLEAR_FIELD(csv_field_ventil);
    CSV_CLEAR_FIELD(csv_field_categ);
    CSV_CLEAR_FIELD(csv_field_sous_categ);
    CSV_CLEAR_FIELD(csv_field_exercice);
}
/**
 * Append the current csv record value in the given file and clean all fields
 */ 
static void _csv_add_record(FILE* file,gboolean clear_all)
{
    CSV_NUM_FIELD(file,csv_field_operation);
    CSV_STR_FIELD(file,csv_field_ventil);
    CSV_STR_FIELD(file,csv_field_date);
    if (etat.utilise_exercice)
    {
        CSV_STR_FIELD(file,csv_field_exercice);
    }
    CSV_STR_FIELD(file,csv_field_pointage);
    CSV_STR_FIELD(file,csv_field_tiers);
    CSV_NUM_FIELD(file,csv_field_credit);
    CSV_NUM_FIELD(file,csv_field_debit);
    CSV_NUM_FIELD(file,csv_field_montant);
    CSV_NUM_FIELD(file,csv_field_solde);
    CSV_STR_FIELD(file,csv_field_categ);
    CSV_STR_FIELD(file,csv_field_sous_categ);
    CSV_STR_FIELD(file,csv_field_notes);
    CSV_END_RECORD(file);
    _csv_clear_fields(clear_all);
}

/* *******************************************************************************/

#define EMPTY_STR_FIELD fprintf(fichier_csv,"\"\"");
#define EMPTY_NUM_FIELD fprintf(fichier_csv,"0");

void export_accounts_to_csv (GSList* export_entries_list )
{
    gchar *nom_fichier_csv, *montant_tmp;
    GSList *liste_tmp;
    FILE *fichier_csv;
    gdouble solde = 0;

    liste_tmp = export_entries_list;

    while ( liste_tmp )
    {

	/*       ouverture du fichier, si pb, on marque l'erreur et passe au fichier suivant */
        solde = 0.0;
	nom_fichier_csv = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( liste_tmp -> data )));

	if ( !( fichier_csv = utf8_fopen ( nom_fichier_csv,
				      "w" ) ))
	    dialogue ( g_strdup_printf ( _("Error for the file \"%s\" :\n%s"),
					 nom_fichier_csv, strerror ( errno ) ));
	else
	{
	    GSList *pointeur_tmp;
	    struct structure_operation *operation;

	    p_tab_nom_de_compte_variable = 
		p_tab_nom_de_compte
		+
		GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
							"no_compte" ));

            if (g_csv_with_title_line)
            {
                csv_field_operation  = g_strdup(_("Transactions"));
                csv_field_ventil     = g_strdup(_("Breakdown"));
                csv_field_date       = g_strdup(_("Date"));
                if (etat.utilise_exercice)
                {
                    csv_field_exercice   = g_strdup(_("Financial year"));
                }
                csv_field_pointage   = g_strdup(_("C/R"));
                csv_field_tiers      = g_strdup(_("Third party"));
                csv_field_credit     = g_strdup(_("Credit"));
                csv_field_debit      = g_strdup(_("Debit"));
                csv_field_montant    = g_strdup(_("Amount"));
                csv_field_solde      = g_strdup(_("Balance"));
                csv_field_categ      = g_strdup(_("Category"));
                csv_field_sous_categ = g_strdup(_("Sub-categories"));
                csv_field_notes      = g_strdup(_("Notes"));
            }

            _csv_add_record(fichier_csv,TRUE);


            //tiers
            csv_field_tiers = g_strdup_printf ( g_strconcat (_("Initial balance") , " [", NOM_DU_COMPTE, "]", NULL ) );

            /* met le solde initial */
            montant_tmp = g_strdup_printf ( "%4.2f", SOLDE_INIT );
            montant_tmp = g_strdelimit ( montant_tmp, ",", '.' );

            solde           = SOLDE_INIT;
            csv_field_solde = g_strdup_printf ( "%4.2f", solde );
            if (SOLDE_INIT >= 0)
            {
                csv_field_credit = g_strdup ( montant_tmp );
            }
            else
            {
                csv_field_debit = g_strdup( montant_tmp );
            }


            _csv_add_record(fichier_csv,TRUE);

		/* on met toutes les opérations */
	    if ( LISTE_OPERATIONS )
	    {
		pointeur_tmp = LISTE_OPERATIONS;

		while ( pointeur_tmp )
		{
		    GSList *pointeur;
		    gdouble montant;

		    operation = pointeur_tmp -> data;


		    /* si c'est une opé de ventilation, on la saute pas elle sera recherchée quand */
		    /* son opé ventilée sera exportée */

		    if ( !operation -> no_operation_ventilee_associee )
		    {
			/* met la date */
			csv_field_date = g_strdup_printf ("%d/%d/%d", operation -> jour, operation -> mois, operation -> annee );

			/* met le pointage */
			if      ( operation -> pointe == 1 ) csv_field_pointage = g_strdup(_("P"));
			else if ( operation -> pointe == 2 ) csv_field_pointage = g_strdup(_("R"));
                        else if ( operation -> pointe == 3 ) csv_field_pointage = g_strdup(_("T"));

			/* met les notes */
                        CSV_CLEAR_FIELD(csv_field_notes);
			if ( operation -> notes )
                            csv_field_notes = g_strdup(operation -> notes );

			/* met le tiers */
			pointeur = g_slist_find_custom ( liste_struct_tiers,
							 GINT_TO_POINTER ( operation -> tiers ),
							 (GCompareFunc) recherche_tiers_par_no );

                        CSV_CLEAR_FIELD(csv_field_tiers);
			if ( pointeur )
			    csv_field_tiers = g_strdup ( ((struct struct_tiers *)(pointeur -> data )) -> nom_tiers );


			/* met le montant, transforme la devise si necessaire */

			montant = calcule_montant_devise_renvoi ( operation -> montant,
								  DEVISE,
								  operation -> devise,
								  operation -> une_devise_compte_egale_x_devise_ope,
								  operation -> taux_change,
								  operation -> frais_change );

			montant_tmp = g_strdup_printf ( "%4.2f", montant );
			montant_tmp = g_strdelimit ( montant_tmp, ",", '.' );

                        if (montant > -0.0 )
                        {
                            csv_field_credit = g_strdup(montant_tmp );
                        }
                        else
                        {
                            csv_field_debit  = g_strdup( montant_tmp );
                        }

			/* met le chèque si c'est un type à numérotation automatique */
                        /*

			pointeur = g_slist_find_custom ( TYPES_OPES,
							 GINT_TO_POINTER ( operation -> type_ope ),
							 (GCompareFunc) recherche_type_ope_par_no );

			if ( pointeur )
			{
			    type = pointeur -> data;

			    if ( type -> numerotation_auto )
				fprintf ( fichier_csv,
					  "N%s\n",
					  operation -> contenu_type );
			}

                        */
                        solde += montant;
                        csv_field_solde = g_strdup_printf ( "%4.2f", solde );


                        csv_field_operation = g_strdup_printf("%d",operation -> no_operation);
                        
                        /* Financial Year */
                        if (etat.utilise_exercice)
                        {
                            csv_field_exercice  = g_strdup(exercice_name_by_no(operation -> no_exercice));
                        }
			/*  on met soit un virement, soit une ventilation, soit les catégories */

			/* si c'est une ventilation, on recherche toutes les opés de cette ventilation */
			/* et les met à la suite */
			/* la catégorie de l'opé sera celle de la première opé de ventilation */
			if ( operation -> operation_ventilee )
			{

                            _csv_add_record(fichier_csv,FALSE);
                            
			    GSList *liste_ventil;
			    gint categ_ope_mise;

			    categ_ope_mise = 0;
			    liste_ventil = LISTE_OPERATIONS;

			    while ( liste_ventil )
			    {
				struct structure_operation *ope_test;

				ope_test = liste_ventil -> data;

				if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation
				     &&
				     ( ope_test -> categorie
				       ||
				       ope_test -> relation_no_operation ))
				{
				    /* on commence par mettre la catég et sous categ de l'opé et de l'opé de ventilation */
                                    csv_field_ventil  =  g_strdup("V"); // -> mark 

				    if ( ope_test -> relation_no_operation )
				    {
					/* c'est un virement */

                                        csv_field_categ      = g_strdup(_("Transfer"));
					gpointer **save_ptab;

					save_ptab = p_tab_nom_de_compte_variable;

					p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_test -> relation_no_compte;

                                        csv_field_sous_categ = g_strdup(g_strconcat ( "[", NOM_DU_COMPTE, "]", NULL ));

					p_tab_nom_de_compte_variable = save_ptab;
				    }
				    else
				    {
					/* c'est du type categ : sous categ */

					pointeur = g_slist_find_custom ( liste_struct_categories,
									 GINT_TO_POINTER ( ope_test -> categorie ),
									 (GCompareFunc) recherche_categorie_par_no );

					if ( pointeur )
					{
					    GSList *pointeur_2;
					    struct struct_categ *categorie;

					    categorie = pointeur -> data;

					    pointeur_2 = g_slist_find_custom ( categorie -> liste_sous_categ,
									       GINT_TO_POINTER ( ope_test -> sous_categorie ),
									       (GCompareFunc) recherche_sous_categorie_par_no );
                                            csv_field_categ      = g_strdup(categorie -> nom_categ);
					    if ( pointeur_2 )
					    {
                                                csv_field_sous_categ = g_strdup(((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ);
					    }
					}
				    }


				    /* met les notes de la ventilation */

				    if ( ope_test -> notes )
                                        csv_field_notes = g_strdup(ope_test -> notes);

				    /* met le montant de la ventilation */

				    montant = calcule_montant_devise_renvoi ( ope_test -> montant,
									      DEVISE,
									      operation -> devise,
									      operation -> une_devise_compte_egale_x_devise_ope,
									      operation -> taux_change,
									      operation -> frais_change );

				    montant_tmp = g_strdup_printf ( "%4.2f", montant );
				    montant_tmp = g_strdelimit ( montant_tmp, ",", '.' );
                                    csv_field_montant = g_strdup ( montant_tmp );

                                    _csv_add_record(fichier_csv,FALSE);
				}

				liste_ventil = liste_ventil -> next;
			    }
                            _csv_clear_fields(TRUE);
			}
			else
			{

			    /* si c'est un virement vers un compte supprimé, ça sera pris comme categ normale vide */

			    if ( operation -> relation_no_operation
				 &&
				 operation -> relation_no_compte >= 0 )
			    {
				/* c'est un virement */

                                csv_field_categ      = g_strdup(_("Transfer"));
				gpointer **save_ptab;

				save_ptab = p_tab_nom_de_compte_variable;

				p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

                                csv_field_sous_categ = g_strdup(g_strconcat ( "[", NOM_DU_COMPTE, "]", NULL ));

				p_tab_nom_de_compte_variable = save_ptab;
			    }
			    else
			    {
				/* c'est du type categ : sous-categ */

				pointeur = g_slist_find_custom ( liste_struct_categories,
								 GINT_TO_POINTER ( operation -> categorie ),
								 (GCompareFunc) recherche_categorie_par_no );

				if ( pointeur )
				{
				    GSList *pointeur_2;
				    struct struct_categ *categorie;

				    categorie = pointeur -> data;
                                    csv_field_categ      = g_strdup(categorie -> nom_categ);

				    pointeur_2 = g_slist_find_custom ( categorie -> liste_sous_categ,
								       GINT_TO_POINTER ( operation -> sous_categorie ),
								       (GCompareFunc) recherche_sous_categorie_par_no );
				    if ( pointeur_2 )
                                        csv_field_sous_categ = g_strdup(((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ);
				}
			    }
                            _csv_add_record(fichier_csv,TRUE);
			}

                        
		    }

		    pointeur_tmp = pointeur_tmp -> next;
		}
	    }
	   
	    fclose ( fichier_csv );
	}
	liste_tmp = liste_tmp -> next;
    }

}
