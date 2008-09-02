/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger	(cedric@grisbi.org)	      */
/*			2005-2008 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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


/*START_INCLUDE*/
#include "qif.h"
#include "./dialog.h"
#include "./utils_files.h"
#include "./utils_str.h"
#include "./gsb_data_account.h"
#include "./gsb_data_archive_store.h"
#include "./gsb_data_category.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file_util.h"
#include "./gsb_real.h"
#include "./import.h"
#include "./gsb_data_account.h"
#include "./gsb_data_transaction.h"
#include "./include.h"
#include "./import.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern GSList *liste_comptes_importes_error;
extern gsb_real null_real ;
/*END_EXTERN*/



/**
 * \brief Import QIF data.
 *
 * Open a QIF file and fills in data in a struct_compte_importation
 * data structure.
 *
 * \param assistant	Not used.
 * \param imported	A pointer to structure containing name and
 *			format of imported file.
 *
 * \return		TRUE on success.
 */
gboolean recuperation_donnees_qif ( GtkWidget * assistant, struct imported_file * imported )
{
    gchar *pointeur_char;
    gchar **tab_char;
    struct struct_compte_importation *compte;
    gint retour = 0;
    gint format_date;
    GSList *liste_tmp;
    gchar **tab;
    gint pas_le_premier_compte = 0;
    FILE * fichier = utf8_fopen ( imported -> name, "r" );

    if ( ! fichier )
    {
	return FALSE;
    }

    /* fichier pointe sur le fichier qui a été reconnu comme qif */
    rewind ( fichier );

    
    compte = g_malloc0 ( sizeof ( struct struct_compte_importation ));
    compte -> nom_de_compte = unique_imported_name ( _("Invalid QIF file") );
    compte -> filename = my_strdup ( imported -> name );
    compte -> origine = my_strdup ( "QIF" );

    /* save filename */
    compte -> real_filename = my_strdup (imported -> name);

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

	    if ( retour && retour != EOF )
	    {
		pointeur_char = g_convert ( pointeur_char, -1, "UTF-8", 
					    imported -> coding_system, NULL, NULL,
					    NULL );
		if ( ! pointeur_char )
		{
		    g_print ("> convert failed\n");
		}
	    }

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

		/* a new of money : now it sets the name bank (and perhaps the others ??)
		 * in the locale... so i do it here, and keep the non localisation because
		 * there is not only money on earth... pfff... */

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
				       5 )
		     ||
		     !my_strcasecmp ( pointeur_char+6,
				       _("bank"))
		     ||
		     !my_strcasecmp ( pointeur_char+6,
				      _("cash)"))
		     ||
		     !my_strcasecmp ( pointeur_char+6,
				      _("ccard)"))
		     ||
		     !my_strcasecmp ( pointeur_char+6,
				      _("invst)"))
		     ||
		     !my_strcasecmp ( pointeur_char+6,
				      _("oth a)"))
		     ||
		     !my_strcasecmp ( pointeur_char+6,
				      _("oth l)")))
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
		liste_comptes_importes_error = g_slist_append ( liste_comptes_importes_error,
								compte );
		fclose ( fichier );
		return FALSE;
	    }
	    else
	    {
		fclose ( fichier );
		return TRUE;
	    }
	}

	compte = g_malloc0 ( sizeof ( struct struct_compte_importation ));
	compte -> origine = my_strdup ( "QIF" );

	/* save filename */
	compte -> real_filename = my_strdup (imported -> name);


	/* récupération du type de compte */

	/*     on n'accepte que les types bank, cash, ccard, invst(avec warning), oth a, oth l */
	/* 	le reste, on passe */

	if ( !my_strncasecmp ( pointeur_char+6,
			       "bank",
			       4 )
	     ||
	     !my_strcasecmp ( pointeur_char+6,
			      _("bank")))
	    compte -> type_de_compte = 0;
	else
	{
	    if ( !my_strncasecmp ( pointeur_char+6,
				   "invst",
				   5 )
		 ||
		 !my_strcasecmp ( pointeur_char+6,
				  _("invst)")))
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
				       4 )
		     ||
		     !my_strcasecmp ( pointeur_char+6,
				      _("cash)")))
		    compte -> type_de_compte = 7;
		else
		{
		    if ( !my_strncasecmp ( pointeur_char+6,
					   "oth a",
					   5 )
			 ||
			 !my_strcasecmp ( pointeur_char+6,
					  _("oth a)")))
			compte -> type_de_compte = 2;
		    else
		    {
			if ( !my_strncasecmp ( pointeur_char+6,
					       "oth l",
					       5 )
			     ||
			     !my_strcasecmp ( pointeur_char+6,
					      _("oth l)")))
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
		g_free ( pointeur_char );

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
			compte -> solde = gsb_real_get_from_string (pointeur_char + 1);
			g_strfreev ( tab_char );
		    }
		    else
			compte -> solde = gsb_real_get_from_string (pointeur_char + 1);

		    g_strfreev ( tab );

		}


		/* récupération du nom du compte */
		/* 	      parfois, le nom est entre crochet et parfois non ... */

		if ( pointeur_char[0] == 'L' )
		{
		    compte -> nom_de_compte = get_line_from_string ( pointeur_char ) + 1;
		    compte -> filename = my_strdup ( imported -> name );

		    /* on vire les crochets s'ils y sont */

		    if ( g_utf8_validate ( compte -> nom_de_compte,-1,NULL ))
		    {
			compte -> nom_de_compte = g_strdelimit ( compte -> nom_de_compte,
								 "[",
								 ' ' );
			compte -> nom_de_compte =  g_strdelimit ( compte -> nom_de_compte,
								  "]",
								  ' ' );
			compte -> nom_de_compte =  compte -> nom_de_compte;

		    }
		    else
		    {
			compte -> nom_de_compte = latin2utf8 ( g_strdelimit ( compte -> nom_de_compte,
									      "[",
									      ' ' ));
			compte -> nom_de_compte =  latin2utf8 (g_strdelimit ( compte -> nom_de_compte,
									      "]",
									      ' ' ));
			compte -> nom_de_compte =  latin2utf8 (compte -> nom_de_compte);
		    }

		    compte -> nom_de_compte = unique_imported_name ( compte -> nom_de_compte );
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

	    compte -> nom_de_compte = unique_imported_name ( my_strdup ( _("Credit card")) );
	    compte -> filename = my_strdup ( imported -> name );
	    compte -> solde = null_real;
	    retour = 0;
	}

	/* si le compte n'a pas de nom, on en met un ici */

	if ( !compte -> nom_de_compte )
	    compte -> nom_de_compte = unique_imported_name ( my_strdup ( _("Imported QIF account" )) );

	if ( retour == EOF )
	{
	    g_free (compte);
	    if ( !pas_le_premier_compte )
	    {
		liste_comptes_importes_error = g_slist_append ( liste_comptes_importes_error,
								compte );
		fclose ( fichier );
		return (FALSE);
	    }
	    else
	    {
		fclose ( fichier );
		return (TRUE);
	    }
	}


	/* récupération des opérations en brut, on les traitera ensuite */

	do
	{
	    struct struct_ope_importation *operation;
	    struct struct_ope_importation *ventilation;

	    ventilation = NULL;

	    operation = g_malloc0 (sizeof ( struct struct_ope_importation ));

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

		    /* remove the 0A at the end if present */
		    if ( pointeur_char [ strlen (pointeur_char)-1 ] == 10 )
			pointeur_char [ strlen (pointeur_char)-1 ] = 0;

		    /* récupération de la date */
		    /* on la met pour l'instant dans date_tmp, et après avoir récupéré toutes */
		    /* les opés on transformera les dates en gdate */

		    if ( pointeur_char[0] == 'D' )
			operation -> date_tmp = my_strdup ( pointeur_char + 1 );


		    /* récupération du pointage */

		    if ( pointeur_char[0] == 'C' )
		    {
			if ( pointeur_char[1] == '*' )
			    operation -> p_r = OPERATION_POINTEE;
			else
			    operation -> p_r = OPERATION_RAPPROCHEE;
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

		    /* if there is a '.' to separate the decimals, we need to check if there is no ','
		     * to separate the thousands (thanks money...) */
		    if ( pointeur_char[0] == 'T' )
		    {
			tab = g_strsplit ( pointeur_char,
					   ".",
					   2 );

			if( tab[1] )
			{
			    /* we remove the ',' wich split the thousands for money */
			    tab_char = g_strsplit ( pointeur_char,
						    ",",
						    FALSE );
			    pointeur_char = g_strjoinv ( NULL,
							 tab_char );
			    g_strfreev ( tab_char );
			}
			g_strfreev ( tab );

			/* now we are sure there is only 1 ',' or '.' to separate the decimals */
			operation -> montant = gsb_real_get_from_string (pointeur_char + 1);
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
			    operation -> tiers = my_strdup ( pointeur_char + 1 );
			}
			else
			    operation -> tiers = latin2utf8 (my_strdup ( pointeur_char + 1 )); 
		    }


		    /* récupération des catég */

		    if ( pointeur_char[0] == 'L' )
		    {
			if ( g_utf8_validate ( pointeur_char+1,-1,NULL ))
			{
			    operation -> categ = my_strdup ( pointeur_char + 1 );
			}
			else
			    operation -> categ = latin2utf8 (my_strdup ( pointeur_char + 1 )); 
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

			    g_free ( operation );

			    if ( ventilation )
				g_free ( ventilation );

			    operation = NULL;
			    ventilation = NULL;
			}

			/* si une ventilation était en cours, on l'enregistre */

			if ( ventilation )
			    compte -> operations_importees = g_slist_append ( compte -> operations_importees,
									      ventilation );

			ventilation = g_malloc0 ( sizeof ( struct struct_ope_importation ));

			if ( operation )
			{
			    operation -> operation_ventilee = 1;

			    /* récupération des données de l'opération en cours */

			    ventilation -> date_tmp = my_strdup ( operation -> date_tmp );
			    ventilation -> tiers = operation -> tiers;
			    ventilation -> cheque = operation -> cheque;
			    ventilation -> p_r = operation -> p_r;
			    ventilation -> ope_de_ventilation = 1;
			}

			if ( g_utf8_validate ( pointeur_char+1,-1,NULL ))
			{
			    ventilation -> categ = my_strdup ( pointeur_char + 1 );
			}
			else
			    ventilation -> categ = latin2utf8 (my_strdup ( pointeur_char + 1 )); 


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
			    ventilation -> montant = gsb_real_get_from_string (pointeur_char + 1);

			    g_strfreev ( tab_char );
			}
			else
			    ventilation -> montant = gsb_real_get_from_string (pointeur_char + 1);

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

			operation -> date_tmp = my_strdup ( "01/01/1970" );
			if ( operation -> tiers )
			    operation -> tiers = g_strconcat ( operation -> tiers,
							       _(" [Transaction imported without date]"),
							       NULL );
			else
			    operation -> tiers = my_strdup ( _(" [Transaction imported without date]"));
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

			/* si on avait 'nn, en fait ça peut être 'nn ou 'nnnn ... */

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
			liste_comptes_importes_error = g_slist_append ( liste_comptes_importes_error,
									compte );
			fclose ( fichier );
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

		    /* si on avait 'aa, en fait ça peut être 'aa ou 'aaaa ... */

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
		    if ( strchr ( compte -> date_solde_qif, '-' ) )
		    {
			/* le format est aaaa-mm-jj */
			tab_str = g_strsplit ( compte -> date_solde_qif, "-", 3 );

			mois = my_strtod ( tab_str[1], NULL );
			jour = my_strtod ( tab_str[2], NULL );
			if ( strlen ( tab_str[0] ) == 4 )
			    annee = my_strtod ( tab_str[0], NULL );
			else
			{
			    annee = my_strtod ( tab_str[0], NULL );
			    if ( annee < 80 )
				annee = annee + 2000;
			    else
				annee = annee + 1900;
			}
		    }
		    else
		    {
			if ( strchr ( compte -> date_solde_qif, '.' ) )
			{
			    /* le format est aaaa.mm.jj */
			    tab_str = g_strsplit ( compte -> date_solde_qif, ".", 3 );

			    mois = my_strtod ( tab_str[1], NULL );
			    jour = my_strtod ( tab_str[2], NULL );
			    if ( strlen ( tab_str[0] ) == 4 )
				annee = my_strtod ( tab_str[0], NULL );
			    else
			    {
				annee = my_strtod ( tab_str[0], NULL );
				if ( annee < 80 )
				    annee = annee + 2000;
				else
				    annee = annee + 1900;
			    }
			}
			else
			{
			    /* i don't know any other format, if there is new, it's here... */
			    jour = 0;
			    mois = 0;
			    annee = 0;
			}
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

    fclose ( fichier );

    return ( TRUE );
}


/**
 * export an archive given in param
 * it will create 1 file per account exported, containing
 * 	the transactions of the archive
 * 	the name exported will be "filename-name of the account.qif"
 *
 * \param filename a name without the extension
 * \param archive_number
 *
 * \return TRUE ok, FALSE pb
 * */
gboolean gsb_qif_export_archive ( const gchar *filename,
				  gint archive_number )
{
    GSList *tmp_list;
    GSList *name_list = NULL;
    gint error_return = 0;

    if (!gsb_file_util_test_overwrite (filename))
	return FALSE;

    /* the work (split between accounts) is already done with the archive_store,
     * so just use it */
    tmp_list = gsb_data_archive_store_get_archives_list ();
    while (tmp_list)
    {
	gint archive_store_number;

	archive_store_number = gsb_data_archive_store_get_number (tmp_list -> data);

	if (gsb_data_archive_store_get_archive_number (archive_store_number) == archive_number)
	{
	    gchar *new_filename;

	    new_filename = g_strconcat ( filename,
					 "-",
					 gsb_data_account_get_name (gsb_data_archive_store_get_account_number(archive_store_number)),
					 ".qif",
					 NULL );
	    if (qif_export ( new_filename,
			     gsb_data_archive_store_get_account_number(archive_store_number),
			     archive_number ))
		name_list = g_slist_append ( name_list,
					     new_filename );
	    else
		error_return = 1;
	}
	tmp_list = tmp_list -> next;
    }
    
    /* if there is more than 1 file created, we show what files we did */
    if (g_slist_length (name_list) > 1)
    {
	gchar *string;

	string = g_strdup_printf ( _("There were many accounts to export in the archive,\nbut the QIF format only support one file per account,\nso Grisbi created %d files, one per account :\n"),
				     g_slist_length (name_list));

	/* add the name to the string */
	tmp_list = name_list;
	while (tmp_list)
	{
	    gchar *new_string;

	    new_string = g_strconcat ( string,
				       "\n",
				       tmp_list -> data,
				       NULL );
	    g_free (string);
	    string = new_string;
	    tmp_list = tmp_list -> next;
	}
	dialogue (string);
	g_free (string);
    }

    /* free the names */
    tmp_list = name_list;
    while (tmp_list)
    {
	g_free (tmp_list -> data);
	tmp_list = tmp_list -> next;
    }
    g_slist_free (name_list);

    return !error_return;
}



/**
 * create a qif export, according to the filename, the account
 * and eventually, limit the export to the archive if exists
 * this will export all the transactions of the account (except if we ask for an archive...)
 * 	including the archived transactions
 *
 * \param filename that file will be checked and ask to overwrite if needed
 * \param account_nb
 * \param archive_number if 0, just export in qif the account transactions ; if non 0, export just the transactions for that archive and account
 *
 * \return TRUE ok, FALSE pb
 */
gboolean qif_export ( const gchar *filename,
		      gint account_nb,
		      gint archive_number )
{
    FILE * fichier_qif;
    GSList *list_tmp_transactions;
    gint begining;
    gint floating_point;

    if (!gsb_file_util_test_overwrite (filename))
	return FALSE;

    if ( !( fichier_qif = utf8_fopen ( filename, "w" ) ))
    {
	dialogue_error_hint ( latin2utf8 ( strerror(errno) ),
			      g_strdup_printf ( _("Error opening file '%s'"),
						filename ) );
	return FALSE;
    }

    /* get the floating point of the currency of the amount,
     * ie the number of digits after the . */
    floating_point = gsb_data_currency_get_floating_point (gsb_data_account_get_currency (account_nb));

    /* kind of account */
    if ( gsb_data_account_get_kind (account_nb) == GSB_TYPE_CASH )
	fprintf ( fichier_qif,
		  "!Type:Cash\n" );
    else
	if ( gsb_data_account_get_kind (account_nb) == GSB_TYPE_LIABILITIES
	     ||
	     gsb_data_account_get_kind (account_nb) == GSB_TYPE_ASSET )
	    fprintf ( fichier_qif,
		      "!Type:Oth L\n" );
	else
	    fprintf ( fichier_qif,
		      "!Type:Bank\n" );


    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    begining = 1;

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_nb
	     &&
	     (!archive_number
	      ||
	      gsb_data_transaction_get_archive_number (transaction_number_tmp) == archive_number))
	{
	    if ( begining )
	    {
		/* this is the begining of the qif file, we set some beginings things */
		fprintf ( fichier_qif,
			  "D%d/%d/%d\n",
			  g_date_day (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_month (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_year (gsb_data_transaction_get_date (transaction_number_tmp)));

		/* met le solde initial */

		gchar* tmpstr = gsb_real_get_string (gsb_data_account_get_init_balance (account_nb, -1));
		fprintf ( fichier_qif,
			  "T%s\n",
			  tmpstr);
		g_free ( tmpstr );

		fprintf ( fichier_qif,
			  "CX\nPOpening Balance\n" );

		/* met le nom du compte */

		fprintf ( fichier_qif,
			  "L%s\n^\n",
			  g_strconcat ( "[",
					gsb_data_account_get_name (account_nb),
					"]",
					NULL ) );
		begining = 0;
	    }

	    /* si c'est une opé de ventilation, on la saute pas elle sera recherchée quand */
	    /* son opé ventilée sera exportée */

	    if ( !gsb_data_transaction_get_mother_transaction_number ( transaction_number_tmp))
	    {
		/* met la date */

		fprintf ( fichier_qif,
			  "D%d/%d/%d\n",
			  g_date_day (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_month (gsb_data_transaction_get_date (transaction_number_tmp)),
			  g_date_year (gsb_data_transaction_get_date (transaction_number_tmp)));

		/* met le pointage */

		if ( gsb_data_transaction_get_marked_transaction ( transaction_number_tmp)== OPERATION_POINTEE
		     ||
		     gsb_data_transaction_get_marked_transaction ( transaction_number_tmp)== OPERATION_TELERAPPROCHEE )
		    fprintf ( fichier_qif,
			      "C*\n" );
		else
		    if ( gsb_data_transaction_get_marked_transaction ( transaction_number_tmp)== OPERATION_RAPPROCHEE )
			fprintf ( fichier_qif,
				  "CX\n" );


		/* met les notes */

		if ( gsb_data_transaction_get_notes ( transaction_number_tmp))
		    fprintf ( fichier_qif,
			      "M%s\n",
			      gsb_data_transaction_get_notes ( transaction_number_tmp));


		/* met le montant, transforme la devise si necessaire */
		gchar* tmpstr = gsb_real_get_string (gsb_data_transaction_get_adjusted_amount ( transaction_number_tmp, floating_point));
		fprintf ( fichier_qif,
			  "T%s\n",
			  tmpstr);
		g_free ( tmpstr );

		/* met le chèque si c'est un type à numérotation automatique */
		if ( gsb_data_payment_get_automatic_numbering (gsb_data_transaction_get_method_of_payment_number (transaction_number_tmp)))
		    fprintf ( fichier_qif,
			      "N%s\n",
			      gsb_data_transaction_get_method_of_payment_content ( transaction_number_tmp));

		/* met le tiers */

		fprintf ( fichier_qif,
			  "P%s\n",
			  gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number_tmp),
						    FALSE ));

		/*  on met soit un virement, soit une ventilation, soit les catégories */

		/* si c'est une ventilation, on recherche toutes les opés de cette ventilation */
		/* et les met à la suite */
		/* la catégorie de l'opé sera celle de la première opé de ventilation */

		if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number_tmp))
		{
		    /* it's a breakdown of transactions, look for the children and append them */

		    gint mother_transaction_category_written;
		    GSList *list_tmp_transactions_2;

		    mother_transaction_category_written = 0;
		    list_tmp_transactions_2 = gsb_data_transaction_get_transactions_list ();

		    while ( list_tmp_transactions_2 )
		    {
			gint transaction_number_tmp_2;
			transaction_number_tmp_2 = gsb_data_transaction_get_transaction_number (list_tmp_transactions_2 -> data);

			if (gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp_2) == transaction_number_tmp)
			{
			    /* we are on a child, for the first one, we set the mother category */
			    /*  the child can only be a normal category or a transfer */

			    if ( gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp_2))
			    {
				/* the child is a transfer */

				if ( !mother_transaction_category_written )
				{
				    fprintf ( fichier_qif,
					      "L%s\n",
					      g_strconcat ( "[",
							    gsb_data_account_get_name (gsb_data_transaction_get_contra_transaction_account (transaction_number_tmp_2)),
							    "]",
							    NULL ));
				    mother_transaction_category_written = 1;
				}
				fprintf ( fichier_qif,
					  "S%s\n",
					  g_strconcat ( "[",
							gsb_data_account_get_name (gsb_data_transaction_get_contra_transaction_account ( transaction_number_tmp_2)),
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
					      gsb_data_category_get_name (gsb_data_transaction_get_category_number (transaction_number_tmp_2),
									  gsb_data_transaction_get_sub_category_number (transaction_number_tmp_2),
									  _("No category defined")));
				    mother_transaction_category_written = 1;
				}
				fprintf ( fichier_qif,
					  "S%s\n",
					  gsb_data_category_get_name (gsb_data_transaction_get_category_number (transaction_number_tmp_2),
								      gsb_data_transaction_get_sub_category_number (transaction_number_tmp_2),
								      _("No category defined")));
			    }

			    /* set the notes of the breakdown child */

			    if ( gsb_data_transaction_get_notes (transaction_number_tmp_2))
				fprintf ( fichier_qif,
					  "E%s\n",
					  gsb_data_transaction_get_notes (transaction_number_tmp_2));

			    /* set the amount of the breakdown child */

			    gchar* tmpstr = gsb_real_get_string (gsb_data_transaction_get_adjusted_amount (transaction_number_tmp_2, floating_point));
			    fprintf ( fichier_qif,
				      "$%s\n",
				      tmpstr);
			    g_free ( tmpstr );
			}
			list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
		    }
		}
		else
		{
		    /* if it's a transfer, the contra-account must exist, else we do
		     * as for a normal category */

		    if ( gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp)
			 &&
			 gsb_data_transaction_get_contra_transaction_account (transaction_number_tmp)>= 0 )
		    {
			/* it's a transfer */

			fprintf ( fichier_qif,
				  "L%s\n",
				  g_strconcat ( "[",
						gsb_data_account_get_name (gsb_data_transaction_get_contra_transaction_account ( transaction_number_tmp)),
						"]",
						NULL ));
		    }
		    else
		    {
			/* it's a normal category */

			fprintf ( fichier_qif,
				  "L%s\n",
				  gsb_data_category_get_name (gsb_data_transaction_get_category_number (transaction_number_tmp),
							      gsb_data_transaction_get_sub_category_number (transaction_number_tmp),
							      FALSE ));
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

	gchar* tmpstr = gsb_real_get_string (gsb_data_account_get_init_balance (account_nb, -1));
	fprintf ( fichier_qif,
		  "T%s\n",
		  tmpstr);
	g_free ( tmpstr );

	fprintf ( fichier_qif,
		  "CX\nPOpening Balance\n" );

	/* met le nom du compte */

	fprintf ( fichier_qif,
		  "L%s\n^\n",
		  g_strconcat ( "[",
				gsb_data_account_get_name (account_nb),
				"]",
				NULL ) );
    }
    fclose ( fichier_qif );
    return TRUE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
