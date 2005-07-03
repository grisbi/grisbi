/* ce fichier de la gestion du format qif */


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



#include "include.h"


/*START_INCLUDE*/
#include "qif.h"
#include "dialog.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_str.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "utils.h"
#include "utils_categories.h"
#include "search_glist.h"
#include "utils_tiers.h"
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void click_compte_export_qif ( GtkWidget *bouton,
			       GtkWidget *entree );
/*END_STATIC*/


GSList *liste_entrees_exportation;


/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern gchar *nom_fichier_comptes;
extern GtkWidget *window;
/*END_EXTERN*/


/* *******************************************************************************/
gboolean recuperation_donnees_qif ( FILE *fichier )
{
    gchar *pointeur_char;
    gchar **tab_char;
    struct struct_compte_importation *compte;
    gint retour = 0;
    gint format_date;
    GSList *liste_tmp;
    gchar **tab;
    gint pas_le_premier_compte = 0;

    /* fichier pointe sur le fichier qui a été reconnu comme qif */

    rewind ( fichier );
    
/*     on n'accepte que les types bank, cash, ccard, invst(avec warning), oth a, oth l */
/* 	le reste, on passe */

    do
    {
	do
	{
/* 	    si ce n'est pas le premier compte du fichier, pointeur_char est déjà sur la ligne du nouveau compte */
/* 	    tans que pas_le_premier_compte = 1 ; du coup on le met à 2 s'il était à 1 */

	    if ( pas_le_premier_compte != 1 )
	    {
		retour = get_line_from_file ( fichier,
					      &pointeur_char );
	    }
	    else
		pas_le_premier_compte = 2;

	    if ( retour
		 &&
		 retour != EOF
		 &&
		 pointeur_char
		 &&
		 !my_strncasecmp ( pointeur_char,
				   "!Type",
				   5 ))
	    {
		/* 	    à ce niveau on est sur un !type ou !Type, on vérifie maintenant qu'on supporte */
		/* 		bien ce type ; si c'est le cas, on met retour à -1 */

		if ( !my_strncasecmp ( pointeur_char+6,
				       "bank",
				       4 )
		     ||
		     !my_strncasecmp ( pointeur_char+6,
				       "cash",
				       4 )
		     ||
		     !my_strncasecmp ( pointeur_char+6,
				       "ccard",
				       5 )
		     ||
		     !my_strncasecmp ( pointeur_char+6,
				       "invst",
				       5 )
		     ||
		     !my_strncasecmp ( pointeur_char+6,
				       "oth a",
				       5 )
		     ||
		     !my_strncasecmp ( pointeur_char+6,
				       "oth l",
				       5 ))
					   retour = -2;
	    }
	}
	while ( retour != EOF
		&&
		retour != -2 );

	/*     si on est déjà à la fin du fichier,on se barre */

	if ( retour == EOF )
	{
	    if ( !pas_le_premier_compte )
	    {
		return FALSE;
	    }
	    else
		return TRUE;
	}

	/*     on est sur le début d'opérations d'un compte, on crée un nouveau compte */

	compte = calloc ( 1,
			  sizeof ( struct struct_compte_importation ));

	/* c'est une importation qif */

	compte -> origine = QIF_IMPORT;

	/* récupération du type de compte */

	if ( !my_strncasecmp ( pointeur_char+6,
			       "bank",
			       4 ))
	    compte -> type_de_compte = 0;
	else
	{
	    if ( !my_strncasecmp ( pointeur_char+6,
				   "invst",
				   5 ))
	    {
/* 		on considère le compte d'investissement comme un compte bancaire mais met un */
/* 		    warning car pas implémenté ; aucune idée si ça passe ou pas... */
		compte -> type_de_compte = 0;
		dialogue_warning ( _("Grisbi found an investment account, which is not implemented yet.  Nevertheless, Grisbi will try to import it as a bank account." ));
	    }
	    else
	    {
		if ( !my_strncasecmp ( pointeur_char+6,
				       "cash",
				       4 ))
		    compte -> type_de_compte = 7;
		else
		{
		    if ( !my_strncasecmp ( pointeur_char+6,
					   "oth a",
					   5 ))
			compte -> type_de_compte = 2;
		    else
		    {
			if ( !my_strncasecmp ( pointeur_char+6,
					       "oth l",
					       5 ))
			    compte -> type_de_compte = 3;
			else
			    /* CCard */
			    compte -> type_de_compte = 5;
		    }
		}
	    }
	}
    


	/* récupère les autres données du compte */

	/*       pour un type CCard, le qif commence directement une opé sans donner les */
	/* 	caractéristiques de départ du compte, on crée donc un compte du nom */
	/* "carte de crédit" avec un solde init de 0 */

	if ( my_strncasecmp ( pointeur_char+6,
			      "ccard",
			      5 ))
	{
	    /* ce n'est pas une ccard, on récupère les infos */

	    do
	    {
		free ( pointeur_char );

		retour = get_line_from_file ( fichier,
					      &pointeur_char );


		/* récupération du solde initial ( on doit virer la , que money met pour séparer les milliers ) */
		/* on ne vire la , que s'il y a un . */

		if ( pointeur_char[0] == 'T' )
		{
		    tab = g_strsplit ( pointeur_char,
				       ".",
				       2 );

		    if( tab[1] )
		    {
			tab_char = g_strsplit ( pointeur_char,
						",",
						FALSE );

			pointeur_char = g_strjoinv ( NULL,
						     tab_char );
			compte -> solde = my_strtod ( pointeur_char + 1,
						      NULL );
			g_strfreev ( tab_char );
		    }
		    else
			compte -> solde = my_strtod ( pointeur_char + 1,
						      NULL );

		    g_strfreev ( tab );

		}


		/* récupération du nom du compte */
		/* 	      parfois, le nom est entre crochet et parfois non ... */

		if ( pointeur_char[0] == 'L' )
		{
		    compte -> nom_de_compte = get_line_from_string ( pointeur_char ) + 1;

		    /* on vire les crochets s'ils y sont */

		    if ( g_utf8_validate ( compte -> nom_de_compte,-1,NULL ))
		    {
			compte -> nom_de_compte = g_strdelimit ( compte -> nom_de_compte,
								 "[",
								 ' ' );
			compte -> nom_de_compte =  g_strdelimit ( compte -> nom_de_compte,
								  "]",
								  ' ' );
			compte -> nom_de_compte =  g_strstrip ( compte -> nom_de_compte );

		    }
		    else
		    {
			compte -> nom_de_compte = latin2utf8 ( g_strdelimit ( compte -> nom_de_compte,
									      "[",
									      ' ' ));
			compte -> nom_de_compte =  latin2utf8 (g_strdelimit ( compte -> nom_de_compte,
									      "]",
									      ' ' ));
			compte -> nom_de_compte =  latin2utf8 (g_strstrip ( compte -> nom_de_compte ));
		    }
		}

		/* on récupère la date du fichier */
		/*  on ne la traite pas maintenant mais quand on traitera toutes les dates */

		if ( pointeur_char[0] == 'D' )
		    compte -> date_solde_qif = get_line_from_string ( pointeur_char ) + 1;

	    }
	    while ( pointeur_char[0] != '^'
		    &&
		    retour != EOF );
	}
	else
	{
	    /* c'est un compte ccard */

	    compte -> nom_de_compte = g_strdup ( _("Credit card"));
	    compte -> solde = 0;
	    retour = 0;
	}

	/* si le compte n'a pas de nom, on en met un ici */

	if ( !compte -> nom_de_compte )
	    compte -> nom_de_compte = g_strdup ( _("Imported account with no name" ));

	if ( retour == EOF )
	{
	    free (compte);
	    if ( !pas_le_premier_compte )
	    {
		return (FALSE);
	    }
	    else
		return (TRUE);
	}


	/* récupération des opérations en brut, on les traitera ensuite */

	do
	{
	    struct struct_ope_importation *operation;
	    struct struct_ope_importation *ventilation;

	    ventilation = NULL;

	    operation = calloc ( 1,
				 sizeof ( struct struct_ope_importation ));

	    do
	    {
		retour = get_line_from_file ( fichier,
					      &pointeur_char );

		if ( retour != EOF
		     &&
		     pointeur_char[0] != '^'
		     &&
		     pointeur_char[0] != '!' )
		{
		    /* on vire le 0d à la fin de la chaîne s'il y est  */

		    if ( pointeur_char [ strlen (pointeur_char)-1 ] == 13 )
			pointeur_char [ strlen (pointeur_char)-1 ] = 0;

		    /* récupération de la date */
		    /* on la met pour l'instant dans date_tmp, et après avoir récupéré toutes */
		    /* les opés on transformera les dates en gdate */

		    if ( pointeur_char[0] == 'D' )
			operation -> date_tmp = g_strdup ( pointeur_char + 1 );


		    /* récupération du pointage */

		    if ( pointeur_char[0] == 'C' )
		    {
			if ( pointeur_char[1] == '*' )
			    operation -> p_r = 1;
			else
			    operation -> p_r = 2;
		    }


		    /* récupération de la note */

		    if ( pointeur_char[0] == 'M' )
		    {
			operation -> notes = g_strstrip ( g_strdelimit ( pointeur_char + 1,
									 ";",
									 '/' ));

			if ( !g_utf8_validate ( operation -> notes ,-1,NULL ))
			    operation -> notes = latin2utf8 (operation -> notes ); 

			if ( !strlen ( operation -> notes ))
			    operation -> notes = NULL;
		    }


		    /* récupération du montant ( on doit virer la , que money met pour séparer les milliers ) */
		    /* on ne vire la , que s'il y a un . */

		    if ( pointeur_char[0] == 'T' )
		    {
			tab = g_strsplit ( pointeur_char,
					   ".",
					   2 );

			if( tab[1] )
			{
			    tab_char = g_strsplit ( pointeur_char,
						    ",",
						    FALSE );

			    pointeur_char = g_strjoinv ( NULL,
							 tab_char );
			    operation -> montant = my_strtod ( pointeur_char + 1,
							       NULL ); 

			    g_strfreev ( tab_char );
			}
			else
			    operation -> montant = my_strtod ( pointeur_char + 1,
							       NULL );


			g_strfreev ( tab );
		    }

		    /* récupération du chèque */

		    if ( pointeur_char[0] == 'N' )
			operation -> cheque = my_strtod ( pointeur_char + 1,
							  NULL ); 



		    /* récupération du tiers */

		    if ( pointeur_char[0] == 'P' )
		    {
			if ( g_utf8_validate ( pointeur_char+1,-1,NULL ))
			{
			    operation -> tiers = g_strdup ( pointeur_char + 1 );
			}
			else
			    operation -> tiers = latin2utf8 (g_strdup ( pointeur_char + 1 )); 
		    }


		    /* récupération des catég */

		    if ( pointeur_char[0] == 'L' )
		    {
			if ( g_utf8_validate ( pointeur_char+1,-1,NULL ))
			{
			    operation -> categ = g_strdup ( pointeur_char + 1 );
			}
			else
			    operation -> categ = latin2utf8 (g_strdup ( pointeur_char + 1 )); 
		    }



		    /* récupération de la ventilation et de sa categ */

		    if ( pointeur_char[0] == 'S' )
		    {
			/* on commence une ventilation, si une opé était en cours, on l'enregistre */

			if ( retour != EOF && operation && operation -> date_tmp )
			{
			    if ( !ventilation )
				compte -> operations_importees = g_slist_append ( compte -> operations_importees,
										  operation );
			}
			else
			{
			    /*c'est la fin du fichier ou l'opé n'est pas valide, donc les ventils ne sont pas valides non plus */

			    free ( operation );

			    if ( ventilation )
				free ( ventilation );

			    operation = NULL;
			    ventilation = NULL;
			}

			/* si une ventilation était en cours, on l'enregistre */

			if ( ventilation )
			    compte -> operations_importees = g_slist_append ( compte -> operations_importees,
									      ventilation );

			ventilation = calloc ( 1,
					       sizeof ( struct struct_ope_importation ));

			if ( operation )
			{
			    operation -> operation_ventilee = 1;

			    /* récupération des données de l'opération en cours */

			    ventilation -> date_tmp = g_strdup ( operation -> date_tmp );
			    ventilation -> tiers = operation -> tiers;
			    ventilation -> cheque = operation -> cheque;
			    ventilation -> p_r = operation -> p_r;
			    ventilation -> ope_de_ventilation = 1;
			}

			if ( g_utf8_validate ( pointeur_char+1,-1,NULL ))
			{
			    ventilation -> categ = g_strdup ( pointeur_char + 1 );
			}
			else
			    ventilation -> categ = latin2utf8 (g_strdup ( pointeur_char + 1 )); 


		    }


		    /* récupération de la note de ventilation */

		    if ( pointeur_char[0] == 'E'
			 &&
			 ventilation )
		    {
			ventilation -> notes = g_strstrip ( g_strdelimit ( pointeur_char + 1,
									   ";",
									   '/' ));

			if ( !g_utf8_validate ( ventilation -> notes ,-1,NULL ))
			    ventilation -> notes = latin2utf8 (ventilation -> notes ); 

			if ( !strlen ( ventilation -> notes ))
			    ventilation -> notes = NULL;
		    }

		    /* récupération du montant de la ventilation */
		    /* récupération du montant ( on doit virer la , que money met pour séparer les milliers ) */
		    /* on ne vire la , que s'il y a un . */

		    if ( pointeur_char[0] == '$'
			 &&
			 ventilation )
		    {
			tab = g_strsplit ( pointeur_char,
					   ".",
					   2 );

			if( tab[1] )
			{
			    tab_char = g_strsplit ( pointeur_char,
						    ",",
						    FALSE );

			    pointeur_char = g_strjoinv ( NULL,
							 tab_char );
			    ventilation -> montant = my_strtod ( pointeur_char + 1,
								 NULL ); 


			    g_strfreev ( tab_char );
			}
			else
			    ventilation -> montant = my_strtod ( pointeur_char + 1,
								 NULL );


			g_strfreev ( tab );
		    }
		}
	    }
	    while ( pointeur_char[0] != '^'
		    &&
		    retour != EOF
		    &&
		    pointeur_char[0] != '!' );

	    /* 	à ce stade, soit on est à la fin d'une opération, soit à la fin du fichier */

	    /* 	    en théorie, on a toujours ^ à la fin d'une opération */
	    /* 		donc si on en est à eof ou !, on n'enregistre pas l'opé */

	    if ( retour != EOF
		 &&
		 pointeur_char[0] != '!' )
	    {
		if ( ventilation )
		{
		    compte -> operations_importees = g_slist_append ( compte -> operations_importees,
								      ventilation );
		    ventilation = NULL;
		}
		else
		{
		    if ( !(operation -> date_tmp
			   && 
			   strlen ( g_strstrip (operation -> date_tmp ))))
		    {
			/* 	l'opération n'a pas de date, c'est pas normal. pour éviter de la perdre, on va lui */
			/* 	 donner la date 01/01/1970 et on ajoute au tiers [opération sans date] */

			operation -> date_tmp = g_strdup ( "01/01/1970" );
			if ( operation -> tiers )
			    operation -> tiers = g_strconcat ( operation -> tiers,
							       _(" [Transaction imported without date]"),
							       NULL );
			else
			    operation -> tiers = g_strdup ( _(" [Transaction imported without date]"));
		    }
		    compte -> operations_importees = g_slist_append ( compte -> operations_importees,
								      operation );
		}
	    }
	}
	/*     on continue à enregistrer les opés jusqu'à la fin du fichier ou jusqu'à un changement de compte */
	while ( retour != EOF
		&&
		pointeur_char[0] != '!' );

	/* toutes les opérations du compte ont été récupérées */
	/* on peut maintenant transformer la date_tmp en gdate */

	format_date = 0;

changement_format_date:

	liste_tmp = compte -> operations_importees;

	while ( liste_tmp )
	{
	    struct struct_ope_importation *operation;
	    gchar **tab_str;
	    gint jour, mois, annee;

	    operation = liste_tmp -> data;

	    /*   vérification qu'il y a une date, sinon on vire l'opé de toute manière */

	    if ( operation -> date_tmp)
	    {	      
		/* récupération de la date qui est du format jj/mm/aaaa ou jj/mm/aa ou jj/mm'aa à partir de 2000 */
		/* 	      si format_date = 0, c'est sous la forme jjmm sinon mmjj */


		tab_str = g_strsplit ( operation -> date_tmp,
				       "/",
				       3 );

		if ( tab_str [2] && tab_str [1] )
		{
		    /* 		  le format est xx/xx/xx, pas d'apostrophe */

		    if ( format_date )
		    {
			mois = my_strtod ( tab_str[0],
					   NULL );
			jour = my_strtod ( tab_str[1],
					   NULL );
		    }
		    else
		    {
			jour = my_strtod ( tab_str[0],
					   NULL );
			mois = my_strtod ( tab_str[1],
					   NULL );
		    }

		    if ( strlen ( tab_str[2] ) >= 4 )
			annee = my_strtod ( tab_str[2],
					    NULL );
		    else
		    {
			annee = my_strtod ( tab_str[2],
					    NULL );
			if ( annee < 80 )
			    annee = annee + 2000;
			else
			    annee = annee + 1900;
		    }
		}
		else
		{
		    if ( tab_str[1] )
		    {
			/* le format est xx/xx'xx */

			gchar **tab_str2;

			tab_str2 = g_strsplit ( tab_str[1],
						"'",
						2 );

			if ( format_date )
			{
			    mois = my_strtod ( tab_str[0],
					       NULL );
			    jour = my_strtod ( tab_str2[0],
					       NULL );
			}
			else
			{
			    jour = my_strtod ( tab_str[0],
					       NULL );
			    mois = my_strtod ( tab_str2[0],
					       NULL );
			}

			/* si on avait 'xx, en fait ça peut être 'xx ou 'xxxx ... */

			if ( strlen ( tab_str2[1] ) == 2 )
			    annee = my_strtod ( tab_str2[1],
						NULL ) + 2000;
			else
			    annee = my_strtod ( tab_str2[1],
						NULL );
			g_strfreev ( tab_str2 );

		    }
		    else
		    {
			/* le format est aaaa-mm-jj */

			tab_str = g_strsplit ( operation -> date_tmp,
					       "-",
					       3 );

			mois = my_strtod ( tab_str[1],
					   NULL );
			jour = my_strtod ( tab_str[2],
					   NULL );
			if ( strlen ( tab_str[0] ) >= 4 )
			    annee = my_strtod ( tab_str[0],
						NULL );
			else
			{
			    annee = my_strtod ( tab_str[0],
						NULL );
			    if ( annee < 80 )
				annee = annee + 2000;
			    else
				annee = annee + 1900;
			}
		    }
		}

		g_strfreev ( tab_str );

		if ( g_date_valid_dmy ( jour,
					mois,
					annee ))
		    operation -> date = g_date_new_dmy ( jour,
							 mois,
							 annee );
		else
		{
		    if ( format_date )
		    {
			dialogue_error_hint ( _("Dates can't be parsed in QIF file."),
					      _("Grisbi automatically tries to parse dates from QIF files using heuristics.  Please double check that they are valid and contact grisbi development team for assistance if needed") );
			return (FALSE);
		    }

		    format_date = 1;

		    goto changement_format_date;
		}
	    }
	    else
	    {
		/* il n'y a pas de date, on vire l'opé de la liste */

		compte -> operations_importees = g_slist_remove ( compte -> operations_importees,
								  liste_tmp -> data );
	    }
	    liste_tmp = liste_tmp -> next;
	}


	/* récupération de la date du fichier  */
	/* si format_date = 0, c'est sous la forme jjmm sinon mmjj */

	if ( compte -> date_solde_qif )
	{
	    gchar **tab_str;
	    gint jour, mois, annee;

	    tab_str = g_strsplit ( compte -> date_solde_qif,
				   "/",
				   3 );

	    if ( tab_str [2] && tab_str [1] )
	    {
		/* 		  le format est xx/xx/xx, pas d'apostrophe */

		if ( format_date )
		{
		    mois = my_strtod ( tab_str[0],
				       NULL );
		    jour = my_strtod ( tab_str[1],
				       NULL );
		}
		else
		{
		    jour = my_strtod ( tab_str[0],
				       NULL );
		    mois = my_strtod ( tab_str[1],
				       NULL );
		}

		if ( strlen ( tab_str[2] ) == 4 )
		    annee = my_strtod ( tab_str[2],
					NULL );
		else
		{
		    annee = my_strtod ( tab_str[2],
					NULL );
		    if ( annee < 80 )
			annee = annee + 2000;
		    else
			annee = annee + 1900;
		}
	    }
	    else
	    {
		if ( tab_str[1] )
		{
		    /* le format est xx/xx'xx */

		    gchar **tab_str2;

		    tab_str2 = g_strsplit ( tab_str[1],
					    "'",
					    2 );

		    if ( format_date )
		    {
			mois = my_strtod ( tab_str[0],
					   NULL );
			jour = my_strtod ( tab_str2[0],
					   NULL );
		    }
		    else
		    {
			jour = my_strtod ( tab_str[0],
					   NULL );
			mois = my_strtod ( tab_str2[0],
					   NULL );
		    }

		    /* si on avait 'xx, en fait ça peut être 'xx ou 'xxxx ... */

		    if ( strlen ( tab_str2[1] ) == 2 )
			annee = my_strtod ( tab_str2[1],
					    NULL ) + 2000;
		    else
			annee = my_strtod ( tab_str2[1],
					    NULL );
		    g_strfreev ( tab_str2 );

		}
		else
		{
		    /* le format est aaaa-mm-jj */

		    tab_str = g_strsplit ( compte -> date_solde_qif,
					   "-",
					   3 );

		    mois = my_strtod ( tab_str[1],
				       NULL );
		    jour = my_strtod ( tab_str[2],
				       NULL );
		    if ( strlen ( tab_str[0] ) == 4 )
			annee = my_strtod ( tab_str[0],
					    NULL );
		    else
		    {
			annee = my_strtod ( tab_str[0],
					    NULL );
			if ( annee < 80 )
			    annee = annee + 2000;
			else
			    annee = annee + 1900;
		    }
		}
	    }

	    g_strfreev ( tab_str );

	    if ( g_date_valid_dmy ( jour,
				    mois,
				    annee ))
		compte -> date_fin = g_date_new_dmy ( jour,
						      mois,
						      annee );
	}


	/* ajoute ce compte aux autres comptes importés */

	liste_comptes_importes = g_slist_append ( liste_comptes_importes,
						  compte );

	/*     si à la fin des opérations c'était un changement de compte et pas la fin du fichier, */
	/*     on y retourne !!! */

	pas_le_premier_compte = 1;
    }
    while ( retour != EOF );

	    return ( TRUE );
}
/* *******************************************************************************/






/* *******************************************************************************/
/* Affiche la fenêtre de sélection de fichier pour exporter en qif */
/* *******************************************************************************/

void exporter_fichier_qif ( void )
{
    GtkWidget *dialog, *table, *entree, *check_button, *paddingbox;
    gchar *nom_fichier_qif, *montant_tmp;
    GSList *liste_tmp;
    FILE *fichier_qif;
    gint resultat;
    GSList *list_tmp;


    if ( !nom_fichier_comptes )
    {
	dialogue_error ( _("Your file must have a name (saved) to be exported.") );
	return;
    }


    dialogue_conditional_info_hint ( _("QIF format does not define currencies."), 
				     _("All transactions will be converted into currency of their account."),
				     &etat.display_message_qif_export_currency ); 

    dialog = gtk_dialog_new_with_buttons ( _("Export QIF files"),
					   GTK_WINDOW(window),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_signal_connect ( GTK_OBJECT ( dialog ), "destroy",
			 GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ), "destroy" );

    paddingbox = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, FALSE,
					     _("Select accounts to export") );
    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog)->vbox), 6 );

    table = gtk_table_new ( 2, gsb_account_get_accounts_amount (), FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 12 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), table, TRUE, TRUE, 0 );
    gtk_widget_show ( table );

    /* on met chaque compte dans la table */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	check_button = gtk_check_button_new_with_label ( gsb_account_get_name (i) );
	gtk_table_attach ( GTK_TABLE ( table ),
			   check_button,
			   0, 1,
			   i, i+1,
			   GTK_SHRINK | GTK_FILL,
			   GTK_SHRINK | GTK_FILL,
			   0, 0 );
	gtk_widget_show ( check_button );

	entree = gtk_entry_new ();
	gtk_entry_set_text ( GTK_ENTRY ( entree ),
			     g_strconcat ( nom_fichier_comptes,
					   "_",
					   g_strdelimit ( g_strdup ( gsb_account_get_name (i)) , " ", '_' ),
					   ".qif",
					   NULL ));
	gtk_widget_set_sensitive ( entree,
				   FALSE );
	gtk_object_set_data ( GTK_OBJECT ( entree ),
			      "no_compte",
			      GINT_TO_POINTER ( i ));
	gtk_table_attach ( GTK_TABLE ( table ),
			   entree,
			   1, 2,
			   i, i+1,
			   GTK_EXPAND | GTK_FILL,
			   GTK_SHRINK | GTK_FILL,
			   0, 0 );
	gtk_widget_show ( entree );


	/*       si on clique sur le check bouton, ça rend éditable l'entrée */

	gtk_signal_connect ( GTK_OBJECT ( check_button ),
			     "toggled",
			     GTK_SIGNAL_FUNC ( click_compte_export_qif ),
			     entree );


	list_tmp = list_tmp -> next;
    }


    liste_entrees_exportation = NULL;
    gtk_widget_show_all ( dialog );

choix_liste_fichier:
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat != GTK_RESPONSE_OK || !liste_entrees_exportation )
    {
	if ( liste_entrees_exportation )
	    g_slist_free ( liste_entrees_exportation );

	gtk_widget_destroy ( dialog );
	return;
    }

    /* vérification que tous les fichiers sont enregistrables */

    liste_tmp = liste_entrees_exportation;

    while ( liste_tmp )
    {
	struct stat test_fichier;


	nom_fichier_qif = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( liste_tmp -> data )));


	if ( utf8_stat ( nom_fichier_qif, &test_fichier ) != -1 )
	{
	    if ( S_ISREG ( test_fichier.st_mode ) )
	    {
		if ( ! question_yes_no_hint ( g_strdup_printf (_("File '%s' already exists."),
							       nom_fichier_qif),
					      _("Do you want to overwrite it?")) )
		    goto choix_liste_fichier;
	    }
	    else
	    {
		dialogue_error_hint ( g_strdup_printf ( _("File \"%s\" exists and is not a regular file."),
							nom_fichier_qif),
				      g_strdup_printf ( _("Error saving file '%s'." ), nom_fichier_qif ) );
		goto choix_liste_fichier;
	    }
	}


	liste_tmp = liste_tmp -> next;
    }



    /* on est sûr de l'enregistrement, c'est parti ... */


    liste_tmp = liste_entrees_exportation;

    while ( liste_tmp )
    {
	/*       ouverture du fichier, si pb, on marque l'erreur et passe au fichier suivant */

	nom_fichier_qif = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( liste_tmp -> data )));

	if ( !( fichier_qif = utf8_fopen ( nom_fichier_qif,
				      "w" ) ))

	    dialogue_error_hint ( latin2utf8 ( strerror(errno) ),
				  g_strdup_printf ( _("Error opening file '%s'"),
						    nom_fichier_qif ) );
	else
	{
	    gint no_compte;
	    GSList *list_tmp_transactions;
	    gint begining;

	    no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
								"no_compte" ));

	    /* met le type de compte */

	    if ( gsb_account_get_kind (no_compte) == GSB_TYPE_CASH )
		fprintf ( fichier_qif,
			  "!Type:Cash\n" );
	    else
		if ( gsb_account_get_kind (no_compte) == GSB_TYPE_LIABILITIES
		     ||
		     gsb_account_get_kind (no_compte) == GSB_TYPE_ASSET )
		    fprintf ( fichier_qif,
			      "!Type:Oth L\n" );
		else
		    fprintf ( fichier_qif,
			      "!Type:Bank\n" );


	    list_tmp_transactions = gsb_transaction_data_get_transactions_list ();
	    begining = 1;

	    while ( list_tmp_transactions )
	    {
		gint transaction_number_tmp;
		transaction_number_tmp = gsb_transaction_data_get_transaction_number (list_tmp_transactions -> data);

		if ( gsb_transaction_data_get_account_number (transaction_number_tmp) == no_compte )
		{
		    GSList *pointeur;
		    gdouble montant;
		    struct struct_type_ope *type;

		    if ( begining )
		    {
			/* this is the begining of the qif file, we set some beginings things */
			fprintf ( fichier_qif,
				  "D%d/%d/%d\n",
				  g_date_day (gsb_transaction_data_get_date (transaction_number_tmp)),
				  g_date_month (gsb_transaction_data_get_date (transaction_number_tmp)),
				  g_date_year (gsb_transaction_data_get_date (transaction_number_tmp)));

			/* met le solde initial */

			montant_tmp = g_strdup_printf ( "%4.2f",
							gsb_account_get_init_balance (no_compte) );
			montant_tmp = g_strdelimit ( montant_tmp,
						     ",",
						     '.' );
			fprintf ( fichier_qif,
				  "T%s\n",
				  montant_tmp );

			fprintf ( fichier_qif,
				  "CX\nPOpening Balance\n" );

			/* met le nom du compte */

			fprintf ( fichier_qif,
				  "L%s\n^\n",
				  g_strconcat ( "[",
						gsb_account_get_name (no_compte),
						"]",
						NULL ) );
			begining = 0;
		    }

		    /* si c'est une opé de ventilation, on la saute pas elle sera recherchée quand */
		    /* son opé ventilée sera exportée */

		    if ( !gsb_transaction_data_get_mother_transaction_number ( transaction_number_tmp))
		    {
			/* met la date */

			fprintf ( fichier_qif,
				  "D%d/%d/%d\n",
				  g_date_day (gsb_transaction_data_get_date (transaction_number_tmp)),
				  g_date_month (gsb_transaction_data_get_date (transaction_number_tmp)),
				  g_date_year (gsb_transaction_data_get_date (transaction_number_tmp)));

			/* met le pointage */

			if ( gsb_transaction_data_get_marked_transaction ( transaction_number_tmp)== OPERATION_POINTEE
			     ||
			     gsb_transaction_data_get_marked_transaction ( transaction_number_tmp)== OPERATION_TELERAPPROCHEE )
			    fprintf ( fichier_qif,
				      "C*\n" );
			else
			    if ( gsb_transaction_data_get_marked_transaction ( transaction_number_tmp)== OPERATION_RAPPROCHEE )
				fprintf ( fichier_qif,
					  "CX\n" );


			/* met les notes */

			if ( gsb_transaction_data_get_notes ( transaction_number_tmp))
			    fprintf ( fichier_qif,
				      "M%s\n",
				      gsb_transaction_data_get_notes ( transaction_number_tmp));


			/* met le montant, transforme la devise si necessaire */

			montant = gsb_transaction_data_get_adjusted_amount ( transaction_number_tmp);

			montant_tmp = g_strdup_printf ( "%4.2f",
							montant );
			montant_tmp = g_strdelimit ( montant_tmp,
						     ",",
						     '.' );

			fprintf ( fichier_qif,
				  "T%s\n",
				  montant_tmp );

			/* met le chèque si c'est un type à numérotation automatique */

			pointeur = g_slist_find_custom ( gsb_account_get_method_payment_list (no_compte),
							 GINT_TO_POINTER ( gsb_transaction_data_get_method_of_payment_number ( transaction_number_tmp)),
							 (GCompareFunc) recherche_type_ope_par_no );

			if ( pointeur )
			{
			    type = pointeur -> data;

			    if ( type -> numerotation_auto )
				fprintf ( fichier_qif,
					  "N%s\n",
					  gsb_transaction_data_get_method_of_payment_content ( transaction_number_tmp));
			}
			
			/* met le tiers */
			
			fprintf ( fichier_qif,
				  "P%s\n",
				  tiers_name_by_no ( gsb_transaction_data_get_party_number ( transaction_number_tmp),
						     FALSE ));

			/*  on met soit un virement, soit une ventilation, soit les catégories */

			/* si c'est une ventilation, on recherche toutes les opés de cette ventilation */
			/* et les met à la suite */
			/* la catégorie de l'opé sera celle de la première opé de ventilation */

			if ( gsb_transaction_data_get_breakdown_of_transaction ( transaction_number_tmp))
			{
			    /* it's a breakdown of transactions, look for the children and append them */

			    gint mother_transaction_category_written;
			    GSList *list_tmp_transactions_2;

			    mother_transaction_category_written = 0;
			    list_tmp_transactions_2 = gsb_transaction_data_get_transactions_list ();

			    while ( list_tmp_transactions_2 )
			    {
				gint transaction_number_tmp_2;
				transaction_number_tmp_2 = gsb_transaction_data_get_transaction_number (list_tmp_transactions_2 -> data);

				if (gsb_transaction_data_get_mother_transaction_number (transaction_number_tmp_2) == transaction_number_tmp)
				{
				    /* we are on a child, for the first one, we set the mother category */
				    /*  the child can only be a normal category or a transfer */

				    if ( gsb_transaction_data_get_transaction_number_transfer (transaction_number_tmp_2))
				    {
					/* the child is a transfer */

					if ( !mother_transaction_category_written )
					{
					    fprintf ( fichier_qif,
						      "L%s\n",
						      g_strconcat ( "[",
								    gsb_account_get_name (gsb_transaction_data_get_account_number_transfer (transaction_number_tmp_2)),
								    "]",
								    NULL ));
					    mother_transaction_category_written = 1;
					}
					fprintf ( fichier_qif,
						  "S%s\n",
						  g_strconcat ( "[",
								gsb_account_get_name (gsb_transaction_data_get_account_number_transfer ( transaction_number_tmp_2)),
								"]",
								NULL ));
				    }
				    else
				    {
					/* it's a category : sub-category */

					if ( !mother_transaction_category_written )
					{
					    fprintf ( fichier_qif,
						      "L%s\n",
						      nom_categ_par_no (gsb_transaction_data_get_category_number (transaction_number_tmp_2),
									gsb_transaction_data_get_sub_category_number (transaction_number_tmp_2)));
					    mother_transaction_category_written = 1;
					}
					fprintf ( fichier_qif,
						  "S%s\n",
						  nom_categ_par_no (gsb_transaction_data_get_category_number (transaction_number_tmp_2),
								    gsb_transaction_data_get_sub_category_number (transaction_number_tmp_2)));
				    }

				    /* set the notes of the breakdown child */

				    if ( gsb_transaction_data_get_notes (transaction_number_tmp_2))
					fprintf ( fichier_qif,
						  "E%s\n",
						  gsb_transaction_data_get_notes (transaction_number_tmp_2));

				    /* set the amount of the breakdown child */

				    fprintf ( fichier_qif,
					      "$%s\n",
					      g_strdelimit ( g_strdup_printf ( "%4.2f",
									       gsb_transaction_data_get_adjusted_amount (transaction_number_tmp_2) ),
							     ",",
							     '.' ));
				}
				list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
			    }
			}
			else
			{
			    /* if it's a transfer, the contra-account must exist, else we do
			     * as for a normal category */
			    
			    if ( gsb_transaction_data_get_transaction_number_transfer (transaction_number_tmp)
				 &&
				 gsb_transaction_data_get_account_number_transfer (transaction_number_tmp)>= 0 )
			    {
				/* it's a transfer */

				fprintf ( fichier_qif,
					  "L%s\n",
					  g_strconcat ( "[",
							gsb_account_get_name (gsb_transaction_data_get_account_number_transfer ( transaction_number_tmp)),
							"]",
							NULL ));
			    }
			    else
			    {
				/* it's a normal category */

				fprintf ( fichier_qif,
					  "L%s\n",
					  nom_categ_par_no (gsb_transaction_data_get_category_number (transaction_number_tmp),
							    gsb_transaction_data_get_sub_category_number (transaction_number_tmp)));
			    }
			}
			fprintf ( fichier_qif,
				  "^\n" );
		    }
		}
		list_tmp_transactions = list_tmp_transactions -> next;
	    }

	    if ( begining )
	    {
		/* there is no transaction in the account, so do the opening of the account, bug no date */
		/* met le solde initial */

		fprintf ( fichier_qif,
			  "T%s\n",
			  g_strdelimit ( g_strdup_printf ( "%4.2f",
							   gsb_account_get_init_balance (no_compte) ),
					 ",",
					 '.' ));

		fprintf ( fichier_qif,
			  "CX\nPOpening Balance\n" );

		/* met le nom du compte */

		fprintf ( fichier_qif,
			  "L%s\n^\n",
			  g_strconcat ( "[",
					gsb_account_get_name (no_compte),
					"]",
					NULL ) );
	    }
	    fclose ( fichier_qif );
	}
	liste_tmp = liste_tmp -> next;
    }

    gtk_widget_destroy ( dialog );
}
/* *******************************************************************************/





/* *******************************************************************************/
void click_compte_export_qif ( GtkWidget *bouton,
			       GtkWidget *entree )
{

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton ) ) )
    {
	gtk_widget_set_sensitive ( entree,
				   TRUE );
	liste_entrees_exportation = g_slist_append ( liste_entrees_exportation,
						     entree);
    }
    else
    {
	gtk_widget_set_sensitive ( entree,
				   FALSE );
	liste_entrees_exportation = g_slist_remove ( liste_entrees_exportation,
						     entree);
    }

}
/* *******************************************************************************/


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
