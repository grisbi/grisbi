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
#include "structures.h"
#include "variables-extern.c"
#include "qif.h"


#include "devises.h"
#include "dialog.h"
#include "search_glist.h"
#include "utils.h"
#include "utils_files.h"




/** Read QIF file and constructs account/opeartions data to be imported.
 *
 * @param fichier file to read
 * @param first_operation_is_account_information first opeartion found are account information.
 *      When TRUE first operation will be used to set account name and initial amount.
 *      When FALSE all data are used as standard operation. 
 *
 * @note Until we find a solution to detect if the file is used for an account creation
 * the account_creation parameter should always be set to FALSE to avoid to lose operations.
 *
 */
gboolean recuperation_donnees_qif ( FILE *fichier, gboolean first_operation_is_account_information )
{
    gchar *pointeur_char;
    gchar **tab_char;
    struct struct_compte_importation *compte;
    gint retour = 0;
    gint format_date;
    GSList *liste_tmp;
    gchar **tab;
    gint pas_le_premier_compte = 0;
    gchar *type;

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

	    type = NULL;

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

		tab = g_strsplit ( pointeur_char,
				   ":",
				   2 );

		if ( tab[1] )
		    tab[1] = g_strstrip ( tab[1] );

		/* a new of money : now it sets the name bank (and perhaps the others ??)
		 * in the locale... so i do it here, and keep the non localisation because
		 * there is not only money on earth... pfff... */

		if ( !my_strcasecmp ( tab[1],
				      "bank" )
		     ||
		     !my_strcasecmp ( tab[1],
				      "cash" )
		     ||
		     !my_strcasecmp ( tab[1],
				      "ccard" )
		     ||
		     !my_strcasecmp ( tab[1],
				      "invst" )
		     ||
		     !my_strcasecmp ( tab[1],
				      "oth a" )
		     ||
		     !my_strcasecmp ( tab[1],
				      "oth l" )
		     ||
		     !my_strcasecmp ( tab[1],
				      _("bank") )
		     ||
		     !my_strcasecmp ( tab[1],
				      _("cash") )
		     ||
		     !my_strcasecmp ( tab[1],
				      _("ccard") )
		     ||
		     !my_strcasecmp ( tab[1],
				      _("invst") )
		     ||
		     !my_strcasecmp ( tab[1],
				      _("oth a") )
		     ||
		     !my_strcasecmp ( tab[1],
				      _("oth l") ))
		{
		    retour = -2;
		    type = g_strdup ( tab[1] );
		}

		g_strfreev ( tab );
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
		dialogue ( _("This file is empty!") );
		return FALSE;
	    }
	    else
		return TRUE;
	}

	/*     on est sur le début d'opérations d'un compte, on crée un nouveau compte */
	/* 	si on est ici, la variable type contient le type de compte */

	compte = calloc ( 1,
			  sizeof ( struct struct_compte_importation ));

	/* c'est une importation qif */

	compte -> origine = 0;

	/* récupération du type de compte */

	if ( !my_strcasecmp ( type,
			      "bank" )
	     ||
	     !my_strcasecmp ( type,
			      _("bank")))
	    compte -> type_de_compte = 0;
	else
	{
	    if ( !my_strcasecmp ( type,
				  "invst" )
		 ||
		 !my_strcasecmp ( type,
				  _("invst")))
	    {
/* 		on considère le compte d'investissement comme un compte bancaire mais met un */
/* 		    warning car pas implémenté ; aucune idée si ça passe ou pas... */
		compte -> type_de_compte = 0;
		dialogue_warning ( _("Grisbi found an investment account, which is not implemented yet.  Nevertheless, Grisbi will try to import it as a bank account." ));
	    }
	    else
	    {
		if ( !my_strcasecmp ( type,
				      "cash" )
		     ||
		     !my_strcasecmp ( type,
				      _("cash")))
		    compte -> type_de_compte = 7;
		else
		{
		    if ( !my_strcasecmp ( type,
					  "oth a" )
			 ||
			 !my_strcasecmp ( type,
					  _("oth a")))
			 compte -> type_de_compte = 2;
		    else
		    {
			if ( !my_strcasecmp ( type,
					      "oth l" )
			     ||
			     !my_strcasecmp ( type,
					      _("oth l")))
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

	if ( my_strcasecmp ( type,
			      "ccard" )
	     &&
	     my_strcasecmp ( type,
			     _("ccard")))
	{
            /* Getting account information from the first operation is not consistent with all usage of QIF file 
             * It seems that only mony is using thr first operation to set account initial value and name
             * So this feature has been susepnded from Grisbi (version 0.5.9 and next ?) 
             */
            if(first_operation_is_account_information) // should always been FALSE waiting for a way to detects account creation */
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

                        compte -> nom_de_compte = latin2utf8 ( g_strdelimit ( compte -> nom_de_compte,
                                                                              "[",
                                                                              ' ' ));
                        compte -> nom_de_compte =  latin2utf8 (g_strdelimit ( compte -> nom_de_compte,
                                                                              "]",
                                                                              ' ' ));
                        compte -> nom_de_compte =  latin2utf8 (g_strstrip ( compte -> nom_de_compte ));
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
                compte -> solde = 0;
                retour = 0;
            }
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
		dialogue ( _("This file is empty!") );
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
			operation -> tiers = latin2utf8 ( pointeur_char + 1 ); 


		    /* récupération des catég */
		    if ( pointeur_char[0] == 'L' )
			operation -> categ = latin2utf8 ( pointeur_char + 1 );


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

			ventilation -> categ = latin2utf8 (pointeur_char + 1 ); 


		    }


		    /* récupération de la note de ventilation */

		    if ( pointeur_char[0] == 'E'
			 &&
			 ventilation )
		    {
			ventilation -> notes = g_strstrip ( g_strdelimit ( pointeur_char + 1,
									   ";",
									   '/' ));

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

                // #153 Some banks/software are producing qif files using '.' as date separators! 
                g_strdelimit(operation -> date_tmp,".",'/');

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
		free ( liste_tmp -> data );
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
    gint i, resultat;


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

    table = gtk_table_new ( 2, nb_comptes, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 12 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), table, TRUE, TRUE, 0 );
    gtk_widget_show ( table );

    /* on met chaque compte dans la table */
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {

	check_button = gtk_check_button_new_with_label ( NOM_DU_COMPTE );
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
					   g_strdelimit ( g_strdup ( NOM_DU_COMPTE) , " ", '_' ),
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

	p_tab_nom_de_compte_variable++;
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
		if ( ! question_yes_no_hint ( g_strdup_printf (_("File '%s' already exists"),
							       nom_fichier_qif),	     
					      _("This will irreversibly overwrite previous file.  There is no undo for this.")) )
		    goto choix_liste_fichier;
	    }
	    else
	    {
		dialogue ( g_strdup_printf ( _("File name '%s' invalid !"),
					     nom_fichier_qif ));
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
	    dialogue ( g_strdup_printf ( _("Error for the file \"%s\" :\n%s"),
					 nom_fichier_qif, strerror ( errno ) ));
	else
	{
	    GSList *pointeur_tmp;
	    struct structure_operation *operation;

	    p_tab_nom_de_compte_variable = 
		p_tab_nom_de_compte
		+
		GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
							"no_compte" ));

	    /* met le type de compte */

	    if ( TYPE_DE_COMPTE == 1 )
		fprintf ( fichier_qif,
			  "!Type:Cash\n" );
	    else
		if ( TYPE_DE_COMPTE == 2
		     ||
		     TYPE_DE_COMPTE == 3 )
		    fprintf ( fichier_qif,
			      "!Type:Oth L\n" );
		else
		    fprintf ( fichier_qif,
			      "!Type:Bank\n" );


	    if ( LISTE_OPERATIONS )
	    {
		/* met la date de la 1ère opération comme dâte d'ouverture de compte */

		operation = LISTE_OPERATIONS -> data;

		fprintf ( fichier_qif,
			  "D%d/%d/%d\n",
			  operation -> jour,
			  operation -> mois,
			  operation -> annee );



		/* met le solde initial */

		montant_tmp = g_strdup_printf ( "%4.2f",
						SOLDE_INIT );
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
					NOM_DU_COMPTE,
					"]",
					NULL ) );

		/* on met toutes les opérations */

		pointeur_tmp = LISTE_OPERATIONS;

		while ( pointeur_tmp )
		{
		    GSList *pointeur;
		    gdouble montant;
		    struct struct_type_ope *type;

		    operation = pointeur_tmp -> data;


		    /* si c'est une opé de ventilation, on la saute pas elle sera recherchée quand */
		    /* son opé ventilée sera exportée */

		    if ( !operation -> no_operation_ventilee_associee )
		    {
			/* met la date */

			fprintf ( fichier_qif,
				  "D%d/%d/%d\n",
				  operation -> jour,
				  operation -> mois,
				  operation -> annee );


			/* met le pointage */

			if ( operation -> pointe == CHECKED_TRANSACTION ||
			     operation -> pointe == TELECHECKED_TRANSACTION )
			    fprintf ( fichier_qif,
				      "C*\n" );
			else
			    if ( operation -> pointe == RECONCILED_TRANSACTION )
				fprintf ( fichier_qif,
					  "CX\n" );


			/* met les notes */

			if ( operation -> notes )
			    fprintf ( fichier_qif,
				      "M%s\n",
				      operation -> notes );


			/* met le montant, transforme la devise si necessaire */

			montant = calcule_montant_devise_renvoi ( operation -> montant,
								  DEVISE,
								  operation -> devise,
								  operation -> une_devise_compte_egale_x_devise_ope,
								  operation -> taux_change,
								  operation -> frais_change );

			montant_tmp = g_strdup_printf ( "%4.2f",
							montant );
			montant_tmp = g_strdelimit ( montant_tmp,
						     ",",
						     '.' );

			fprintf ( fichier_qif,
				  "T%s\n",
				  montant_tmp );


			/* met le chèque si c'est un type à numérotation automatique */

			pointeur = g_slist_find_custom ( TYPES_OPES,
							 GINT_TO_POINTER ( operation -> type_ope ),
							 (GCompareFunc) recherche_type_ope_par_no );

			if ( pointeur )
			{
			    type = pointeur -> data;

			    if ( type -> numerotation_auto )
				fprintf ( fichier_qif,
					  "N%s\n",
					  operation -> contenu_type );
			}

			/* met le tiers */

			pointeur = g_slist_find_custom ( liste_struct_tiers,
							 GINT_TO_POINTER ( operation -> tiers ),
							 (GCompareFunc) recherche_tiers_par_no );

			if ( pointeur )
			    fprintf ( fichier_qif,
				      "P%s\n",
				      ((struct struct_tiers *)(pointeur -> data )) -> nom_tiers );



			/*  on met soit un virement, soit une ventilation, soit les catégories */

			/* si c'est une ventilation, on recherche toutes les opés de cette ventilation */
			/* et les met à la suite */
			/* la catégorie de l'opé sera celle de la première opé de ventilation */

			if ( operation -> operation_ventilee )
			{
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

				    if ( ope_test -> relation_no_operation )
				    {
					/* c'est un virement */

					gpointer **save_ptab;

					save_ptab = p_tab_nom_de_compte_variable;

					p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_test -> relation_no_compte;

					if ( !categ_ope_mise )
					{
					    fprintf ( fichier_qif,
						      "L%s\n",
						      g_strconcat ( "[",
								    NOM_DU_COMPTE,
								    "]",
								    NULL ));
					    categ_ope_mise = 1;
					}

					fprintf ( fichier_qif,
						  "S%s\n",
						  g_strconcat ( "[",
								NOM_DU_COMPTE,
								"]",
								NULL ));

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
					    if ( pointeur_2 )
					    {
						if ( !categ_ope_mise )
						{
						    fprintf ( fichier_qif,
							      "L%s\n",
							      g_strconcat ( categorie -> nom_categ,
									    ":",
									    ((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ,
									    NULL ));
						    categ_ope_mise = 1;
						}

						fprintf ( fichier_qif,
							  "S%s\n",
							  g_strconcat ( categorie -> nom_categ,
									":",
									((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ,
									NULL ));
					    }
					    else
					    {
						if ( !categ_ope_mise )
						{
						    fprintf ( fichier_qif,
							      "L%s\n",
							      categorie -> nom_categ );
						    categ_ope_mise = 1;
						}

						fprintf ( fichier_qif,
							  "S%s\n",
							  categorie -> nom_categ );
					    }
					}
				    }


				    /* met les notes de la ventilation */

				    if ( ope_test -> notes )
					fprintf ( fichier_qif,
						  "E%s\n",
						  ope_test -> notes );

				    /* met le montant de la ventilation */

				    montant = calcule_montant_devise_renvoi ( ope_test -> montant,
									      DEVISE,
									      operation -> devise,
									      operation -> une_devise_compte_egale_x_devise_ope,
									      operation -> taux_change,
									      operation -> frais_change );

				    montant_tmp = g_strdup_printf ( "%4.2f",
								    montant );
				    montant_tmp = g_strdelimit ( montant_tmp,
								 ",",
								 '.' );

				    fprintf ( fichier_qif,
					      "$%s\n",
					      montant_tmp );

				}

				liste_ventil = liste_ventil -> next;
			    }
			}
			else
			{
			    /* si c'est un virement vers un compte supprimé, ça sera pris comme categ normale vide */

			    if ( operation -> relation_no_operation
				 &&
				 operation -> relation_no_compte >= 0 )
			    {
				/* c'est un virement */

				gpointer **save_ptab;

				save_ptab = p_tab_nom_de_compte_variable;

				p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

				fprintf ( fichier_qif,
					  "L%s\n",
					  g_strconcat ( "[",
							NOM_DU_COMPTE,
							"]",
							NULL ));

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

				    pointeur_2 = g_slist_find_custom ( categorie -> liste_sous_categ,
								       GINT_TO_POINTER ( operation -> sous_categorie ),
								       (GCompareFunc) recherche_sous_categorie_par_no );
				    if ( pointeur_2 )
					fprintf ( fichier_qif,
						  "L%s\n",
						  g_strconcat ( categorie -> nom_categ,
								":",
								((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ,
								NULL ));
				    else
					fprintf ( fichier_qif,
						  "L%s\n",
						  categorie -> nom_categ );
				}
			    }
			}

			fprintf ( fichier_qif,
				  "^\n" );
		    }

		    pointeur_tmp = pointeur_tmp -> next;
		}
	    }
	    else
	    {
		/* le compte n'a aucune opération enregistrée : on ne met pas de date, mais on fait l'ouverture du compte */

		/* met le solde initial */

		montant_tmp = g_strdup_printf ( "%4.2f",
						SOLDE_INIT );
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
					NOM_DU_COMPTE,
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
	free ( entree );
    }

}
/* *******************************************************************************/


void export_qif (GSList* export_entries_list )
{
    gchar *nom_fichier_qif, *montant_tmp;
    GSList *liste_tmp;
    FILE *fichier_qif;

    liste_tmp = export_entries_list;

    while ( liste_tmp )
    {

	/*       ouverture du fichier, si pb, on marque l'erreur et passe au fichier suivant */

	nom_fichier_qif = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( liste_tmp -> data )));

	if ( !( fichier_qif = utf8_fopen ( nom_fichier_qif,
				      "w" ) ))
	    dialogue ( g_strdup_printf ( _("Error for the file \"%s\" :\n%s"),
					 nom_fichier_qif, strerror ( errno ) ));
	else
	{
	    GSList *pointeur_tmp;
	    struct structure_operation *operation;

	    p_tab_nom_de_compte_variable = 
		p_tab_nom_de_compte
		+
		GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
							"no_compte" ));

	    /* met le type de compte */

	    if ( TYPE_DE_COMPTE == 1 )
		fprintf ( fichier_qif,
			  "!Type:Cash\n" );
	    else
		if ( TYPE_DE_COMPTE == 2
		     ||
		     TYPE_DE_COMPTE == 3 )
		    fprintf ( fichier_qif,
			      "!Type:Oth L\n" );
		else
		    fprintf ( fichier_qif,
			      "!Type:Bank\n" );


	    if ( LISTE_OPERATIONS )
	    {
		/* met la date de la 1ère opération comme dâte d'ouverture de compte */

		operation = LISTE_OPERATIONS -> data;

		fprintf ( fichier_qif,
			  "D%d/%d/%d\n",
			  operation -> jour,
			  operation -> mois,
			  operation -> annee );



		/* met le solde initial */

		montant_tmp = g_strdup_printf ( "%4.2f",
						SOLDE_INIT );
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
					NOM_DU_COMPTE,
					"]",
					NULL ) );

		/* on met toutes les opérations */

		pointeur_tmp = LISTE_OPERATIONS;

		while ( pointeur_tmp )
		{
		    GSList *pointeur;
		    gdouble montant;
		    struct struct_type_ope *type;

		    operation = pointeur_tmp -> data;


		    /* si c'est une opé de ventilation, on la saute pas elle sera recherchée quand */
		    /* son opé ventilée sera exportée */

		    if ( !operation -> no_operation_ventilee_associee )
		    {
			/* met la date */

			fprintf ( fichier_qif,
				  "D%d/%d/%d\n",
				  operation -> jour,
				  operation -> mois,
				  operation -> annee );


			/* met le pointage */

			if ( operation -> pointe == CHECKED_TRANSACTION ||
			     operation -> pointe == TELECHECKED_TRANSACTION )
			    fprintf ( fichier_qif,
				      "C*\n" );
			else
			    if ( operation -> pointe == RECONCILED_TRANSACTION )
				fprintf ( fichier_qif,
					  "CX\n" );


			/* met les notes */

			if ( operation -> notes )
			    fprintf ( fichier_qif,
				      "M%s\n",
				      operation -> notes );


			/* met le montant, transforme la devise si necessaire */

			montant = calcule_montant_devise_renvoi ( operation -> montant,
								  DEVISE,
								  operation -> devise,
								  operation -> une_devise_compte_egale_x_devise_ope,
								  operation -> taux_change,
								  operation -> frais_change );

			montant_tmp = g_strdup_printf ( "%4.2f",
							montant );
			montant_tmp = g_strdelimit ( montant_tmp,
						     ",",
						     '.' );

			fprintf ( fichier_qif,
				  "T%s\n",
				  montant_tmp );


			/* met le chèque si c'est un type à numérotation automatique */

			pointeur = g_slist_find_custom ( TYPES_OPES,
							 GINT_TO_POINTER ( operation -> type_ope ),
							 (GCompareFunc) recherche_type_ope_par_no );

			if ( pointeur )
			{
			    type = pointeur -> data;

			    if ( type -> numerotation_auto )
				fprintf ( fichier_qif,
					  "N%s\n",
					  operation -> contenu_type );
			}

			/* met le tiers */

			pointeur = g_slist_find_custom ( liste_struct_tiers,
							 GINT_TO_POINTER ( operation -> tiers ),
							 (GCompareFunc) recherche_tiers_par_no );

			if ( pointeur )
			    fprintf ( fichier_qif,
				      "P%s\n",
				      ((struct struct_tiers *)(pointeur -> data )) -> nom_tiers );



			/*  on met soit un virement, soit une ventilation, soit les catégories */

			/* si c'est une ventilation, on recherche toutes les opés de cette ventilation */
			/* et les met à la suite */
			/* la catégorie de l'opé sera celle de la première opé de ventilation */

			if ( operation -> operation_ventilee )
			{
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

				    if ( ope_test -> relation_no_operation )
				    {
					/* c'est un virement */

					gpointer **save_ptab;

					save_ptab = p_tab_nom_de_compte_variable;

					p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_test -> relation_no_compte;

					if ( !categ_ope_mise )
					{
					    fprintf ( fichier_qif,
						      "L%s\n",
						      g_strconcat ( "[",
								    NOM_DU_COMPTE,
								    "]",
								    NULL ));
					    categ_ope_mise = 1;
					}

					fprintf ( fichier_qif,
						  "S%s\n",
						  g_strconcat ( "[",
								NOM_DU_COMPTE,
								"]",
								NULL ));

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
					    if ( pointeur_2 )
					    {
						if ( !categ_ope_mise )
						{
						    fprintf ( fichier_qif,
							      "L%s\n",
							      g_strconcat ( categorie -> nom_categ,
									    ":",
									    ((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ,
									    NULL ));
						    categ_ope_mise = 1;
						}

						fprintf ( fichier_qif,
							  "S%s\n",
							  g_strconcat ( categorie -> nom_categ,
									":",
									((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ,
									NULL ));
					    }
					    else
					    {
						if ( !categ_ope_mise )
						{
						    fprintf ( fichier_qif,
							      "L%s\n",
							      categorie -> nom_categ );
						    categ_ope_mise = 1;
						}

						fprintf ( fichier_qif,
							  "S%s\n",
							  categorie -> nom_categ );
					    }
					}
				    }


				    /* met les notes de la ventilation */

				    if ( ope_test -> notes )
					fprintf ( fichier_qif,
						  "E%s\n",
						  ope_test -> notes );

				    /* met le montant de la ventilation */

				    montant = calcule_montant_devise_renvoi ( ope_test -> montant,
									      DEVISE,
									      operation -> devise,
									      operation -> une_devise_compte_egale_x_devise_ope,
									      operation -> taux_change,
									      operation -> frais_change );

				    montant_tmp = g_strdup_printf ( "%4.2f",
								    montant );
				    montant_tmp = g_strdelimit ( montant_tmp,
								 ",",
								 '.' );

				    fprintf ( fichier_qif,
					      "$%s\n",
					      montant_tmp );

				}

				liste_ventil = liste_ventil -> next;
			    }
			}
			else
			{
			    /* si c'est un virement vers un compte supprimé, ça sera pris comme categ normale vide */

			    if ( operation -> relation_no_operation
				 &&
				 operation -> relation_no_compte >= 0 )
			    {
				/* c'est un virement */

				gpointer **save_ptab;

				save_ptab = p_tab_nom_de_compte_variable;

				p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

				fprintf ( fichier_qif,
					  "L%s\n",
					  g_strconcat ( "[",
							NOM_DU_COMPTE,
							"]",
							NULL ));

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

				    pointeur_2 = g_slist_find_custom ( categorie -> liste_sous_categ,
								       GINT_TO_POINTER ( operation -> sous_categorie ),
								       (GCompareFunc) recherche_sous_categorie_par_no );
				    if ( pointeur_2 )
					fprintf ( fichier_qif,
						  "L%s\n",
						  g_strconcat ( categorie -> nom_categ,
								":",
								((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ,
								NULL ));
				    else
					fprintf ( fichier_qif,
						  "L%s\n",
						  categorie -> nom_categ );
				}
			    }
			}

			fprintf ( fichier_qif,
				  "^\n" );
		    }

		    pointeur_tmp = pointeur_tmp -> next;
		}
	    }
	    else
	    {
		/* le compte n'a aucune opération enregistrée : on ne met pas de date, mais on fait l'ouverture du compte */

		/* met le solde initial */

		montant_tmp = g_strdup_printf ( "%4.2f",
						SOLDE_INIT );
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
					NOM_DU_COMPTE,
					"]",
					NULL ) );
	    }
	    fclose ( fichier_qif );
	}
	liste_tmp = liste_tmp -> next;
    }

}
