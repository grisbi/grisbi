/* ce fichier de la gestion du format csv - version initiale par kik0r */


/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
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


/*START_INCLUDE*/
#include "include.h"
#include "structures.h"
#include "csv.h"
#include "dialog.h"
#include "devises.h"
#include "search_glist.h"
#include "utils_files.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern GSList *liste_struct_categories;
extern GSList *liste_struct_tiers;
extern gint nb_comptes;
extern gchar *nom_fichier_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
/*END_EXTERN*/

/* *******************************************************************************/
gboolean recuperation_donnees_csv ( FILE *fichier )
{
    gchar *pointeur_char;
	struct struct_compte_importation *compte;
	gint retour = 0;
	gchar **tab_lignecourante;
	gint nb_ligne_entete = 0;
	gint nb_ligne_lues = 0;

	/* fichier pointe sur le fichier qui a été reconnu comme csv */
	rewind ( fichier );

	printf(_("DEBUG kik0r : Start CSV IMPORT\n")); // DEBUG

	do
	{
		/* si on est déjà à la fin du fichier, on quitte */
		if ( retour == EOF )
		{
			dialogue ( _("This file is empty!") );
			return FALSE;
		}

		printf(_("DEBUG kik0r : File parsing start\n")); // DEBUG
		
		/* on crée un nouveau compte */
		compte = calloc ( 1, sizeof ( struct struct_compte_importation ));
	
		/* c'est une importation csv */
		compte -> origine = CSV_IMPORT;
	
		/* on place le type de compte a bank */
		compte -> type_de_compte = 0;

		/* récupération du solde initial ( on doit virer la , que money met pour séparer les milliers ) */
		/* on ne vire la , que s'il y a un . */
		/* en csv nous n'avons pas cette info */
		compte -> solde = 0;

		/* récupération du nom du compte */
		/* en csv nous n'avons pas cette info */
		compte -> nom_de_compte = g_strdup ( _("Imported account with no name" ));
		
		/* il n'y a pas d'autres informations sur le compte, il faudra le paramètrer */
		/* plus tard si il s'agit d'un import initial ou non, de plus le csv ne donne */
		/* aucune infos sur le compte */
		
		printf(_("DEBUG kik0r : Begin operation import\n")); // DEBUG
		
		/* récupération des opérations en brut, on les traitera ensuite */
		do
		{
			struct struct_ope_importation *operation;
			operation = calloc ( 1,sizeof ( struct struct_ope_importation ));
			gchar **tab_date;
			gint jour = 0, mois = 0, annee = 0;
			gint erreur_date = 0, erreur_montant = 0;
			
			retour = get_line_from_file ( fichier,&pointeur_char );

			printf(_("\n\n------------------------\nDEBUG kik0r : Find a new line\n")); // DEBUG			
			printf(_("DEBUG kik0r : Line is %s\n"),pointeur_char); // DEBUG
			
			if ( retour != EOF && pointeur_char && g_strrstr ( pointeur_char, "Date;Type;" ) && count_char_from_string(";",pointeur_char)==8 )
			{
				/* c'est une ligne d'en tete alors on passe */
				nb_ligne_entete++;
				
				printf(_("DEBUG kik0r : Skip this line (header)\n")); // DEBUG
			}
			else if ( retour != EOF && !g_strrstr ( pointeur_char, "Date;Type;" ) && count_char_from_string(";",pointeur_char)==8 )
			{
				/* c'est une ligne qui semble ok */
				nb_ligne_lues++;

				printf(_("DEBUG kik0r : Find a new operation\n")); // DEBUG
				
				/* on explose la ligne courante */
				tab_lignecourante=g_strsplit( pointeur_char, ";",0);
				
				/* Date;Type;Tiers;Catégorie;S/catégorie;Notes;Débit;Crédit;P;Solde	*/
				/* 19/12/2000;Solde initial;Création du compte;;;Création le 20/12/2000;;118,53;R */
				/* 12/20/2000;Chèque;CCP;Automobile;Carburant;0266002C;17,49;;R */
				/* 12/25/2000;Chèque;CCP;Noel;Cadeau;C'est un cadeau;;25,00;R */
				/* Date => 0; ok */
				/* Type => 1; */
				/* Tiers => 2; ok */
				/* Catégorie => 3; */
				/* S/catégorie => 4; */
				/* Notes => 5; ok */
				/* Débit => 6; ok */
				/* Crédit => 7; ok */
				/* P => 8; ok */
				
				
				/* récupération de la date au format JJ/MM/AAAA ou JJ/MM/AA */
				/* il faudrait avoir en fonction de la locale une couche d'abstraction pour l'import de date */
				/* pour le moment je controle que la date 12/27/2004 est valide auquel cas on est en locale us */
				/* sinon en locale fr. voir fonction gdate g_date_set_parse() */
				
				printf(_("DEBUG kik0r : Date is %s\n"),tab_lignecourante[0]); // DEBUG
				
				/* le champs date contient t'il le bon nb de caracteres ? */
				if (strlen(tab_lignecourante[0]) == 10 || strlen(tab_lignecourante[0]) == 8)
				{
					/* on split pour récupèrer les valeurs jour, mois et année */
					tab_date = g_strsplit (tab_lignecourante[0],"/",3 );
					
					/* on a les trois valeurs donc on a une date complete probablement */
					if ( tab_date [2] && tab_date [1] && tab_date[0])
					{
						/* on recupere le mois et le jour */
						if ( TRUE || g_date_valid_dmy ( 12,27,2004 ) ) /* si cette date est valide alors on est format us */
						{
							mois = my_strtod ( tab_date[0],NULL );
							jour = my_strtod ( tab_date[1],NULL );
						}
						else /* sinon en format fr */
						{
							jour = my_strtod ( tab_date[0],NULL );
							mois = my_strtod ( tab_date[1],NULL );
						}
	
						/* on recupere l'annee */
						if ( strlen ( tab_date[2] ) == 4 )
						{
							annee = my_strtod ( tab_date[2],NULL );
						}
						else
						{
							annee = my_strtod ( tab_date[2],NULL );
						
							/* on gere les annees a partir de 1981 */
							if ( annee < 80 ) { annee = annee + 2000;	}
							else { annee = annee + 1900; }
						}
					}
			
					printf(_("DEBUG kik0r : Date parsed is jour=%d mois=%d annee=%d\n"),jour,mois,annee); // DEBUG
					
					g_strfreev ( tab_date );
					
					/* on controle la validite de la date */
					if ( !g_date_valid_dmy ( jour,mois,annee ))
					{
						erreur_date = 1; /* date erronee, on place le flag erreur date a 1 */
					}
				}
				else /* date erronee, on place le flag erreur date a 1 */
				{
					erreur_date = 1; /* date erronee, on place le flag erreur date a 1 */
				}
				
				if (erreur_date == 0)
				{				
					operation -> date = g_date_new_dmy ( jour,mois,annee );
				}
				else
				{
					/* une erreur sur la date, j'affiche le dialog d'erreur et on va mettre la date a une valeur bidon */
					dialogue_error_hint ( _("Dates can't be parsed in CSV file."),_("Grisbi automatically tries to parse dates from CSV files using heuristics.  Please double check that they are valid and contact grisbi development team for assistance if needed. Operation will be imported with date set on 01/01/1970") );
					
					/* l'opération n'a pas de date, c'est pas normal. pour éviter de la perdre, on va lui */
					/* donner la date 01/01/1980 et on ajoutera plus tard a la note [opération sans date] */
					operation -> date = g_date_new_dmy 	(01,01,1970);
				}
			
				printf(_("DEBUG kik0r : Trying tiers %s\n"),tab_lignecourante[2]); // DEBUG
				
				/* récupération du tiers */
				operation -> tiers = g_strstrip ( tab_lignecourante[2] );
				if ( !g_utf8_validate ( operation -> tiers ,-1,NULL ))
				{
					operation -> tiers = latin2utf8 (operation -> tiers ); 
				}
				if ( !strlen ( operation -> tiers ))
				{
					operation -> tiers = NULL;
				}
				
				printf(_("DEBUG kik0r : Tiers found %s\n"),operation -> tiers); // DEBUG
				printf(_("DEBUG kik0r : Trying amount debit %s credit %s\n"),tab_lignecourante[6],tab_lignecourante[7]); // DEBUG
				
				/* récupération du montant */
				if ( strlen ( tab_lignecourante[6] )>=4) /* c'est un débit */
				{
					operation -> montant = my_strtod (tab_lignecourante[6],NULL ); 
					operation -> montant = operation -> montant * -1; /* on multiplie par -1 pour indiquer le débit */
				}
				
				if ( strlen ( tab_lignecourante[7] ) >= 4 ) /* c'est un crédit */
				{
					operation -> montant = my_strtod (tab_lignecourante[7],NULL ); 
				}

				if (!operation -> montant)
				{
					/* une erreur sur la date, j'affiche le dialog d'erreur et on va mettre la date a une valeur bidon */
					dialogue_error_hint ( _("Amount can't be parsed in CSV file."),_("Grisbi automatically tries to parse amount from CSV files using heuristics.  Please double check that they are valid and contact grisbi development team for assistance if needed. Operation will be imported with amount set to 0") );
					
					/* l'opération n'a pas de date, c'est pas normal. pour éviter de la perdre, on va lui */
					/* donner la date 01/01/1980 et on ajoutera plus tard a la note [opération sans date] */
					operation -> montant = 0;
				}
				
				printf(_("DEBUG kik0r : Amount found %5.2f\n"),operation -> montant ); // DEBUG
				printf(_("DEBUG kik0r : Trying note %s\n"),tab_lignecourante[5]); // DEBUG
				
				/* récupération de la note */
				operation -> notes = g_strstrip ( tab_lignecourante[5] );
				if ( !g_utf8_validate ( operation -> notes ,-1,NULL ))
				{
					operation -> notes = latin2utf8 (operation -> notes ); 
				}
				if ( !strlen ( operation -> notes ))
				{
					operation -> notes = NULL;
				}
				/* on a eu une erreur de date plus haut donc on l'indique dans la note */
				if ( erreur_date == 1)
				{
					operation -> notes = g_strconcat ( operation -> notes,_(" [Transaction imported without date]"),NULL );
				}
				/* on a eu une erreur de date plus haut donc on l'indique dans la note */
				if ( erreur_montant == 1)
				{
					operation -> notes = g_strconcat ( operation -> notes,_(" [Transaction imported with 0 amount]"),NULL );
				}
				
				printf(_("DEBUG kik0r : Note found %s\n"),operation -> notes); // DEBUG
				printf(_("DEBUG kik0r : Trying pointage %s\n"),tab_lignecourante[8]); // DEBUG
				
				/* récupération du pointage */
				if ( strcmp(g_strstrip ( tab_lignecourante[8] ),"P") )
				{
					operation -> p_r = OPERATION_POINTEE;
				}
				else if ( strcmp(g_strstrip ( tab_lignecourante[8] ),"R") )
				{
					operation -> p_r = OPERATION_RAPPROCHEE;
				}
				else
				{	
					operation -> p_r = OPERATION_NORMALE;
				}
	
				printf(_("DEBUG kik0r : Pointage found %d\n"),operation -> p_r); // DEBUG
				printf(_("DEBUG kik0r : Trying categ %s : %s\n"),tab_lignecourante[3],tab_lignecourante[4]); // DEBUG
				
				/* récupération des catég (concatenation categ:souscateg */
				/* il faudrait peut etre changer ce separateur car cela interdit les ":" dans les noms de categ */
				operation -> categ = g_strconcat ( g_strstrip ( tab_lignecourante[3]),":",g_strstrip ( tab_lignecourante[4]),NULL );
				if ( !g_utf8_validate ( operation -> categ ,-1,NULL ))
				{
					operation -> categ = latin2utf8 (operation -> categ ); 
				}
				if ( !strlen ( operation -> categ ))
				{
					operation -> categ = NULL;
				}				
				
				printf(_("DEBUG kik0r : Categ found %s\n"),operation -> categ); // DEBUG
				
				/* on enregistre l'opé */

				if ( retour != EOF && operation && operation -> date )
				{
					compte -> operations_importees = g_slist_append ( compte -> operations_importees,operation );
				}
				else
				{
					/*c'est la fin du fichier ou l'opé n'est pas valide */
					free ( operation );
					operation = NULL;
				}
			}

			/* 	à ce stade, soit on est à la fin d'une opération, soit à la fin du fichier */
			/* 	en théorie, on a toujours  à la fin d'une opération */
			/*  donc si on en est à eof on n'enregistre pas l'opé */
		
			/*if ( retour != EOF )
			{
				if ( !(operation -> date_tmp && strlen ( g_strstrip (operation -> date_tmp ))))
				{

					compte -> operations_importees = g_slist_append ( compte -> operations_importees,operation );
				}
			}*/
		
		}
		/* on continue à enregistrer les opés jusqu'à la fin du fichier ou jusqu'à un changement de compte */
		while ( retour != EOF	);
	
		/* toutes les opérations du compte ont été récupérées */
		/* ajoute ce compte aux autres comptes importés */
		liste_comptes_importes = g_slist_append ( liste_comptes_importes,compte );
	
	}
	while ( retour != EOF );

	return ( TRUE );
}
/* *******************************************************************************/
