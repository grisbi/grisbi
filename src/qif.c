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
#include "accueil.h"
#include "categories_onglet.h"
#include "comptes_traitements.h"
#include "devises.h"
#include "erreur.h"
#include "fichiers_gestion.h"
#include "fichiers_io.h"
#include "gtkcombofix.h"
#include "operations_comptes.h"
#include "operations_liste.h"
#include "patienter.h"
#include "qif.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "operations_formulaire.h"
#include "type_operations.h"



/* *******************************************************************************/
/* fonction importer_fichier_qif */
/* appelée par le menu importer */
/* *******************************************************************************/

void importer_fichier_qif ( void )
{
  liste_comptes_qif = NULL;

  selection_fichier_qif ();

  update_liste_comptes_accueil();
}
/* *******************************************************************************/



/* *******************************************************************************/
/* Affiche la fenêtre de sélection de fichier */
/* *******************************************************************************/

void selection_fichier_qif ( void )
{
  GtkWidget *fenetre;


  fenetre = gtk_file_selection_new ( _("Select a QIF file") );
  gtk_window_set_transient_for ( GTK_WINDOW ( fenetre ),
				 GTK_WINDOW ( window ));
  gtk_window_set_modal ( GTK_WINDOW ( fenetre ),
			 TRUE );
  gtk_clist_set_selection_mode ( GTK_CLIST ( GTK_FILE_SELECTION( fenetre ) -> file_list ),
				 GTK_SELECTION_EXTENDED );
  gtk_file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre ),
				    dernier_chemin_de_travail );

  gtk_signal_connect_object (GTK_OBJECT ( GTK_FILE_SELECTION( fenetre ) -> ok_button ),
			     "clicked",
			     GTK_SIGNAL_FUNC ( fichier_choisi_importation_qif ),
			     GTK_OBJECT ( fenetre ));
  gtk_signal_connect_object (GTK_OBJECT ( GTK_FILE_SELECTION( fenetre ) -> cancel_button ),
			     "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT ( fenetre ));
  gtk_signal_connect ( GTK_OBJECT ( fenetre ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( traitement_donnees_brutes ),
		       NULL );
  gtk_widget_show ( fenetre );


}
/* *******************************************************************************/





/* *******************************************************************************/
void fichier_choisi_importation_qif ( GtkWidget *fenetre )
{
  gchar *nom_fichier_qif;
  FILE *fichier_qif;
  gchar *pointeur_char;
  gchar **tab_char;
  struct donnees_compte *compte;
  GSList *liste_ope_brut;
  gint retour;
  gchar ** selection, **iter;


  /* on met le répertoire courant dans la variable correspondante */

  dernier_chemin_de_travail = g_strconcat ( GTK_LABEL ( GTK_BIN ( GTK_OPTION_MENU ( GTK_FILE_SELECTION ( fenetre ) -> history_pulldown )) -> child ) -> label,
					    "/",
					    NULL );

  /* on va récupérer tous les fichiers sélectionnés puis proposer d'en importer d'autres */


  selection = gtk_file_selection_get_selections ( GTK_FILE_SELECTION(fenetre) );
  iter = selection;

  while ( iter && *iter )
    {
      gchar *fichier;

/*       separation = g_strsplit ( GTK_LABEL ( GTK_FILE_SELECTION ( fenetre ) -> selection_text ) -> label, */
/* 				":", */
/* 				2 ); */

/*       nom_fichier_qif = g_strconcat ( g_strstrip ( separation[1] ), */
/* 				      "/", */
/* 				      fichier[0], */
/* 				      NULL ); */
/*       g_strfreev ( separation ); */

      nom_fichier_qif = (char*) *iter;

      if ( !( fichier_qif = fopen ( nom_fichier_qif,
				    "r" ) ))
	{
	  dialogue ( g_strconcat ("cannot open ", nom_fichier_qif, ": ",
				  latin2utf8 ( strerror(errno) ), NULL ));
	  return;
	}


      /* récupération des données du compte */

      fscanf ( fichier_qif,
	       "%a[^\n]\n",
	       &pointeur_char );

      if ( strncmp ( pointeur_char,
		     "!Type",
		     5 )
	   &&
	   strncmp ( pointeur_char,
		     "!type",
		     5 )
	   &&
	   strncmp ( pointeur_char,
		     "!Account",
		     8 )
	   &&
	   strncmp ( pointeur_char,
		     "!Option",
		     7 ))
	{
	  dialogue ( g_strdup_printf ( _("The file \"%s\" is not a QIF file"),
				       nom_fichier_qif ));
	  free ( pointeur_char );
	  return;
	}
      else
	gtk_signal_handler_block_by_func ( GTK_OBJECT ( fenetre ),
					   GTK_SIGNAL_FUNC ( traitement_donnees_brutes ),
					   NULL );


      /* c'est bien un fichier qif, on peut commencer */


      /* si ça commence par !Option ou !Account ou !Type:Cat, on avance jusqu'à ce qu'on */
      /* trouve un autre !Type */

    retour:
      if ( !strncmp ( pointeur_char,
		      "!Option",
		      7 )
	   ||
	   !strncmp ( pointeur_char,
		      "!Account",
		      8 )
	   ||
	   !strncmp ( pointeur_char,
		      "!Type:Cat",
		      9 ))
	{
	  do
	    {
	      fscanf ( fichier_qif,
		       "%a[^\n]\n",
		       &pointeur_char );
	    }
	  while ( strncmp ( pointeur_char,
			    "!Type",
			    5 ));
	  goto retour;
	}
	    

      /* rafraichit la fenetre d'attente */

      if ( patience_en_cours )
	while ( g_main_iteration ( FALSE ) );

      compte = calloc ( 1,
			sizeof ( struct donnees_compte ));


      /* ajoute ce compte aux autres comptes importés */
 
      liste_comptes_qif = g_slist_append ( liste_comptes_qif,
					   compte );


      /* récupération du type de compte */

      tab_char = g_strsplit ( pointeur_char,
			      ":",
			      2 );

      if ( strcmp ( g_strstrip (tab_char[1]),
		    "Cash" ) )
	if ( strcmp ( g_strstrip ( tab_char[1]),
		      "Oth L" ))
	  compte -> type_de_compte = 0;
	else
	  compte -> type_de_compte = 2;
      else
	compte -> type_de_compte = 1;



      /* récupère les autres données du compte */

      /*       pour un type CCard, le qif commence directement une opé sans donner les */
      /* 	caractéristiques de départ du compte, on crée donc un compte du nom */
      /* "carte de crédit" avec un solde init de 0 */

      if ( strcmp ( g_strstrip (tab_char[1]),
		    "CCard" ))
	{
	  /* ce n'est pas une ccard, on récupère les infos */

	  do
	    {
	      free ( pointeur_char );

	      retour = fscanf ( fichier_qif,
				"%a[^\n]\n",
				&pointeur_char );


	      /* récupération du solde initial ( on doit virer la , que money met pour séparer les milliers ) */
	      /* on ne vire la , que s'il y a un . */

	      if ( pointeur_char[0] == 'T' )
		{
		  gchar **tab;

		  tab = g_strsplit ( pointeur_char,
				     ".",
				     2 );

		  if( tab[1] )
		    {
		      gchar **tab_char;

		      tab_char = g_strsplit ( pointeur_char,
					      ",",
					      FALSE );

		      pointeur_char = g_strjoinv ( NULL,
						   tab_char );
		      compte -> solde_initial = my_strtod ( pointeur_char + 1,
							   NULL );
		      g_strfreev ( tab_char );
		    }
		  else
		    compte -> solde_initial = my_strtod ( pointeur_char + 1,
							 NULL );

		  g_strfreev ( tab );

		}


	      /* récupération du nom du compte */
	      /* 	      parfois, le nom est entre crochet et parfois non ... */

	      if ( pointeur_char[0] == 'L' )
		{
		  sscanf ( pointeur_char,
			   "L%a[^\n]",
			   &compte -> nom_de_compte );

		  /* on vire les crochets s'ils y sont */

		  compte -> nom_de_compte = latin2utf8 (g_strdelimit ( compte -> nom_de_compte,
								       "[", ' ' ));
		  compte -> nom_de_compte = latin2utf8 (g_strdelimit ( compte -> nom_de_compte,
							   "]", ' ' ));
		  compte -> nom_de_compte = latin2utf8 (g_strstrip ( compte -> nom_de_compte ));

		}
	    }
	  while ( pointeur_char[0] != '^' && retour != EOF );
	}
      else
	{
	  /* c'est un compte ccard */

	  compte -> nom_de_compte = _("Credit card");
	  compte -> solde_initial = 0;
	  retour = 0;
	}

      /* si le compte n'a pas de nom, on en met un ici */

      if ( !compte -> nom_de_compte )
	compte -> nom_de_compte = g_strdup ( _("Imported account with no name"));

      if ( retour == EOF )
	{
	  dialogue ( _("This file is empty!") );
	  return;
	}

      /* récupération des opérations en brut, on les traitera ensuite */

      liste_ope_brut = NULL;

      do
	{
	  struct struct_operation_qif *operation;
	  struct struct_operation_qif *ventilation;

	  ventilation = NULL;

	  operation = calloc ( 1, sizeof ( struct struct_operation_qif ));

	  do
	    {
	      retour = fscanf ( fichier_qif,
				"%a[^\n]\n",
				&pointeur_char );

	      if ( retour != EOF )
		{
		  /* on vire le 0d à la fin de la chaîne s'il y est  */

		  if ( pointeur_char [ strlen (pointeur_char)-1 ] == 13 )
		    pointeur_char [ strlen (pointeur_char)-1 ] = 0;

		  /* récupération de la date */

		  if ( pointeur_char[0] == 'D' )
		    operation -> date = g_strdup ( pointeur_char + 1 );


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
		      operation -> notes = latin2utf8 (g_strstrip ( g_strdelimit ( pointeur_char + 1,
								       ";", '/' )));
		      if ( !strlen ( operation -> notes ))
			operation -> notes = NULL;
		    }


		  /* récupération du montant ( on doit virer la , que money met pour séparer les milliers ) */
		  /* on ne vire la , que s'il y a un . */

		  if ( pointeur_char[0] == 'T' )
		    {
		      gchar **tab;

		      tab = g_strsplit ( pointeur_char,
					 ".",
					 2 );

		      if( tab[1] )
			{
			  gchar **tab_char;

			  tab_char = g_strsplit ( pointeur_char,
						  ",",
						  FALSE );

			  pointeur_char = g_strjoinv ( NULL,
						       tab_char );
			  operation -> montant = my_strtod ( pointeur_char + 1, NULL );
			  g_strfreev ( tab_char );
			}
		      else
			operation -> montant = my_strtod ( pointeur_char + 1, NULL );

		      g_strfreev ( tab );
		    }

		  /* récupération du chèque */

		  if ( pointeur_char[0] == 'N' )
		    operation -> cheque = my_strtod ( pointeur_char + 1, NULL );


		  /* récupération du tiers */
	  
		  if ( pointeur_char[0] == 'P' )
		    operation -> tiers = latin2utf8 (g_strdup ( pointeur_char + 1 ));


		  /* récupération des catég */

		  if ( pointeur_char[0] == 'L' )
		    operation -> categ = latin2utf8 (g_strdup ( pointeur_char + 1 ));


		  /* récupération de la ventilation et de sa categ */

		  if ( pointeur_char[0] == 'S' )
		    {
		      /* on commence une ventilation, si une opé était en cours, on l'enregistre */

		      /* ALAIN-FIXME : je ne comprends pas du tout ce test.
		         D'abord, normalement, on est dans une boucle où
			 retour != EOF.
			 Ensuite, pourquoi tester la date de l'opération ?
			 le bug #180 survient car operation n'existe pas,
			 on a donc un segment fault lorsqu'on essaye d'accéder
			 au membre date */
/*		      if ( retour != EOF && operation -> date ) */
		      if ( retour != EOF && operation )
			{
			  if ( !ventilation )
			    liste_ope_brut = g_slist_append ( liste_ope_brut, operation );
			}
		      else
			{
			  /*c'est la fin du fichier ou l'opé n'est pas
			    valide, donc les ventils ne sont pas
			    valides non plus */
			  free ( operation );

			  if ( ventilation )
			    free ( ventilation );

			  operation = NULL;
			  ventilation = NULL;
			}

		      /* si une ventilation était en cours, on l'enregistre */

		      if ( ventilation )
			liste_ope_brut = g_slist_append ( liste_ope_brut, ventilation );

		      if ( operation )
			{
			  ventilation = calloc ( 1, sizeof ( struct struct_operation_qif ));
			  operation -> operation_ventilee = 1;

			  /* récupération des données de l'opération en cours */
			  ventilation -> date = operation -> date;
			  ventilation -> tiers = operation -> tiers;
			  ventilation -> cheque = operation -> cheque;
			  ventilation -> p_r = operation -> p_r;
			  ventilation -> ope_de_ventilation = 1;

			  ventilation -> categ = latin2utf8 (g_strdup ( pointeur_char + 1 ));
			}
		    }


		  /* récupération de la note de ventilation */

		  if ( pointeur_char[0] == 'E'
		       &&
		       ventilation )
		    {
		      ventilation -> notes = g_strstrip ( g_strdelimit ( pointeur_char + 1,
									 ";",
									 '/' ));

		      if ( !strlen ( ventilation -> notes ))
			ventilation -> notes = NULL;
		    }

		  /* récupération du montant de la ventilation */
		  /* récupération du montant ( on doit virer la , que money met pour séparer les milliers ) */
		  /* on ne vire la , que s'il y a un . */
		  /* comme money enregistre en négatif ou en positif */
		  /* soit c'est du même signe que l'opé et on prend la valeur absolue */
		  /* soit c'est de signe contraire et on prend la vraie valeur */

		  if ( pointeur_char[0] == '$'
		       &&
		       ventilation )
		    {
		      gchar **tab;

		      tab = g_strsplit ( pointeur_char, ".", 2 );

		      if( tab[1] )
			{
			  gchar **tab_char;
			  tab_char = g_strsplit ( pointeur_char, ",", FALSE );
			  pointeur_char = g_strjoinv ( NULL, tab_char );
			  ventilation -> montant = my_strtod ( pointeur_char + 1, NULL );
			  g_strfreev ( tab_char );
			}
		      else
			ventilation -> montant = my_strtod ( pointeur_char + 1, NULL );

		      g_strfreev ( tab );
		    }
		}
	    }
	  while ( pointeur_char[0] != '^' && retour != EOF );


	  /* on n'enregistre l'opération que si elle est datée */


	  if ( ventilation )
	    {
	      liste_ope_brut = g_slist_append ( liste_ope_brut, ventilation );
	      ventilation = NULL;
	    }
	  else
	    {
	      if ( retour != EOF && operation -> date )
		liste_ope_brut = g_slist_append ( liste_ope_brut, operation );
	      else
		free ( operation );
	    }

	}
      while ( retour != EOF );

      fclose ( fichier_qif );


      /* pour l'instant, on met la liste créée dans la liste des opés du compte */

      compte -> gsliste_operations = liste_ope_brut;

      iter++;
    }

  g_strfreev(selection);

  gtk_widget_hide ( fenetre );

  traitement_donnees_brutes();
}
/* *******************************************************************************/









/* *******************************************************************************/
void traitement_donnees_brutes ( void )
{
  gint retour;
  GtkWidget *dialog;
  GtkWidget *label;
  GSList *liste_tmp;
  GtkWidget *frame;
  GtkWidget *vbox;
  GSList *liste_comptes_tmp;
  gint nb_comptes_ajoutes;
  gint nouveau_fichier;
  GtkWidget *scrolled_window;

  if ( !liste_comptes_qif )
    return;


  /* on permet ici de récupérer d'autres comptes */


  dialog = gnome_dialog_new ( _("List of the imported accounts"),
			      GNOME_STOCK_BUTTON_YES,
			      GNOME_STOCK_BUTTON_NO,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			     0 );

  if ( g_slist_length ( liste_comptes_qif ) == 1 )
    label = gtk_label_new ( _("Account to import:\n") );
  else
    label = gtk_label_new ( _("Accounts to import:\n") );

  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ), label,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ), frame,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( frame );

  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      scrolled_window );
  gtk_widget_show ( scrolled_window );


  vbox = gtk_vbox_new ( FALSE, 10 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  vbox );
  gtk_widget_set_usize ( scrolled_window, 200, 200 );
  gtk_widget_show ( vbox );


  liste_tmp = liste_comptes_qif;

  while ( liste_tmp )
    {
      label = gtk_label_new ( (( struct donnees_compte * )( liste_tmp -> data )) -> nom_de_compte );
      gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5 );
      gtk_box_pack_start ( GTK_BOX ( vbox ), label,
			   FALSE, FALSE, 0 );
      gtk_widget_show ( label );

      liste_tmp = liste_tmp -> next;
    }

  label = gtk_label_new ( _("Import another account?") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ), label,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( label );

  retour = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));


  /*   si on veut importer un autre compte, on renvoie sur selection_fichier_qif */

  if ( !retour )
    {
      selection_fichier_qif();
      return;
    }

  /*   si on annule : on s'en va */

  if ( retour == 2 )
    return;


  /* on réalise maintenant les transformations pour importer les fichiers */

/* fait le nécessaire si aucun compte n'est ouvert */

  if ( nb_comptes )
      nouveau_fichier = 0;
  else
    {
      init_variables ( FALSE );
      init_variables ( TRUE );
      creation_liste_categories ();
      creation_devises_de_base ();

      nom_fichier_comptes = NULL;
      nouveau_fichier = 1;
      affiche_titre_fenetre();

      /*   la taille des colonnes est automatique au départ, on y met les rapports de base */

      etat.largeur_auto_colonnes = 1;
      rapport_largeur_colonnes[0] = 11;
      rapport_largeur_colonnes[1] = 13;
      rapport_largeur_colonnes[2] = 30;
      rapport_largeur_colonnes[3] = 3;
      rapport_largeur_colonnes[4] = 11;
      rapport_largeur_colonnes[5] = 11;
      rapport_largeur_colonnes[6] = 11;
    }


  /* on commence par enregistrer les comptes */

  nb_comptes_ajoutes = 0;
  liste_comptes_tmp = liste_comptes_qif;

  while ( liste_comptes_tmp )
    {
      struct donnees_compte *compte;
      GtkWidget *bouton;
      gint compte_visible;

      compte = liste_comptes_tmp -> data;

      /* enregistre le compte */

      compte_visible = p_tab_nom_de_compte_courant - p_tab_nom_de_compte;

      p_tab_nom_de_compte = realloc ( p_tab_nom_de_compte, ( nb_comptes + 1 )* sizeof ( gpointer ) );
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + nb_comptes;
      nb_comptes++;
      *p_tab_nom_de_compte_variable = (gpointer) compte;

      p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_visible; 


      /* choix de la devise du compte */

      compte -> devise = selection_devise ( compte -> nom_de_compte );

      /* met le no de compte */

      compte -> no_compte = nb_comptes - 1;

      ordre_comptes = g_slist_append ( ordre_comptes,
				       GINT_TO_POINTER ( nb_comptes - 1 ));

      /* on crée les types d'opé par défaut */

      creation_types_par_defaut ( nb_comptes - 1,
				  0);
      if ( !nouveau_fichier )
	{
	  /* crée le nouveau bouton du compte et l'ajoute à la liste des comptes */

	  bouton = comptes_appel(nb_comptes - 1 );
	  gtk_box_pack_start (GTK_BOX (vbox_liste_comptes),
			      bouton,
			      FALSE,
			      FALSE,
			      0);
	  gtk_widget_show (bouton);
	}

      nb_comptes_ajoutes++;

      liste_comptes_tmp = liste_comptes_tmp -> next;
    }

  p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;


  /* on fait maintenant le tour des opés et cette fois on les convertit */


  liste_comptes_tmp = liste_comptes_qif;

  while ( liste_comptes_tmp )
    {
      struct donnees_compte *compte;
      gdouble solde_courant;
      struct structure_operation *operation;
      gint format_date;
      GSList *save_gsliste;
      gint derniere_operation;
      GSList *liste_ope_brut;

      derniere_operation = 0;
      compte = liste_comptes_tmp -> data;

      /*       au début, on considère que la date est sous forme jjmmaa, si pb, on passe à mmjjaa */

      format_date = 0;
      liste_ope_brut = compte -> gsliste_operations;

      changement_format_date :

      compte -> nb_operations = 0;
      liste_tmp = liste_ope_brut;
      save_gsliste = liste_tmp;

      compte -> gsliste_operations = NULL;
      solde_courant = compte -> solde_initial;

      while ( liste_tmp )
	{
	  struct struct_operation_qif *operation_qif;
	  gchar **tab_str;
	  GSList *pointeur_tmp;

	  operation_qif = liste_tmp -> data;

	  /* 	  vérification qu'il y a une date, sinon ce n'est pas une opé, on prend pas */

	  if ( operation_qif -> date )
	    {
	      operation = calloc ( 1, sizeof ( struct structure_operation ));


	      /* récupération du no de l'opé */

	      operation -> no_operation = ++no_derniere_operation;
	      operation -> type_ope = 0;

	      /* récupération de la date qui est du format jj/mm/aaaa ou jj/mm/aa ou jj/mm'aa à partir de 2000 */
	      /* 	      si format_date = 0, c'est sous la forme jjmm sinon mmjj */


	      tab_str = g_strsplit ( operation_qif -> date, "/", 3 );

	      if ( tab_str [2] && tab_str [1] )
		{
		  /* 		  le format est xx/xx/xx, pas d'apostrophe */

		  if ( format_date )
		    {
		      operation -> mois = my_strtod ( tab_str[0], NULL );
		      operation -> jour = my_strtod ( tab_str[1], NULL );
		    }
		  else
		    {
		      operation -> jour = my_strtod ( tab_str[0], NULL );
		      operation -> mois = my_strtod ( tab_str[1], NULL );
		    }

		  if ( strlen ( tab_str[2] ) == 4 )
		    operation -> annee = my_strtod ( tab_str[2], NULL );
		  else
		    {
		      operation -> annee = my_strtod ( tab_str[2], NULL );
		      if ( operation -> annee < 80 )
			operation -> annee = operation -> annee + 2000;
		      else
			operation -> annee = operation -> annee + 1900;
		    }
		}
	      else
		{
		  if ( tab_str[1] )
		    {
		      /* le format est xx/xx'xx */

		      gchar **tab_str2;

		      tab_str2 = g_strsplit ( tab_str[1], "'", 2 );

		      if ( format_date )
			{
			  operation -> mois = my_strtod ( tab_str[0], NULL );
			  operation -> jour = my_strtod ( tab_str2[0], NULL );
			}
		      else
			{
			  operation -> jour = my_strtod ( tab_str[0], NULL );
			  operation -> mois = my_strtod ( tab_str2[0], NULL );
			}

		      /* si on avait 'xx, en fait ça peut être 'xx ou 'xxxx ... */

		      if ( strlen ( tab_str2[1] ) == 2 )
			operation -> annee = my_strtod ( tab_str2[1], NULL ) + 2000;
		      else
			operation -> annee = my_strtod ( tab_str2[1], NULL );
		      g_strfreev ( tab_str2 );

		    }
		  else
		    {
		      /* le format est aaaa-mm-jj */

		      tab_str = g_strsplit ( operation_qif -> date, "-", 3 );

		      operation -> mois = my_strtod ( tab_str[1], NULL );
		      operation -> jour = my_strtod ( tab_str[2], NULL );
		      if ( strlen ( tab_str[0] ) == 4 )
			operation -> annee = my_strtod ( tab_str[0], NULL );
		      else
			{
			  operation -> annee = my_strtod ( tab_str[0], NULL );
			if ( operation -> annee < 80 )
			  operation -> annee = operation -> annee + 2000;
			else
			  operation -> annee = operation -> annee + 1900;
			}

		    }
		}

	      g_strfreev ( tab_str );

	      if ( g_date_valid_dmy ( operation -> jour,
				      operation -> mois,
				      operation -> annee ))
		operation -> date = g_date_new_dmy ( operation -> jour,
						     operation -> mois,
						     operation -> annee );
	      else
		{
		  if ( format_date )
		    {
		      dialogue ( _("Problem recovering the dates in the QIF file. Import canceled.") );
		      return;
		    }

		  format_date = 1;

		  goto changement_format_date;
		}

	      /* récupération du no de compte ( stocké pour l'instant dans no_derniere_operation ) */

	      operation -> no_compte = compte->no_compte;


	      /* récupération du montant */

	      operation -> montant = operation_qif -> montant;
	      solde_courant = solde_courant + operation_qif -> montant;
	      operation -> devise = compte -> devise;

	      /* récupération du tiers */

	      if ( operation_qif -> tiers )
		{
		  pointeur_tmp = g_slist_find_custom ( liste_struct_tiers,
						       g_strstrip ( operation_qif -> tiers ),
						       (GCompareFunc) recherche_tiers_par_nom );

		  /* si le tiers n'existait pas, on le crée */

		  if ( pointeur_tmp )
		    operation -> tiers = ((struct struct_tiers *)(pointeur_tmp -> data)) -> no_tiers;
		  else
		    operation -> tiers = (( struct struct_tiers * )( ajoute_nouveau_tiers ( operation_qif -> tiers ))) -> no_tiers;

		}


	      /* vérification si c'est ventilé, sinon récupération des catégories */


	      if ( operation_qif -> operation_ventilee )
		{
		  /* l'opération est ventilée */

		  operation -> operation_ventilee = 1;
		}
	      else
		{
		  /* vérification que ce n'est pas un virement */

		  if ( operation_qif -> categ && strlen ( g_strstrip (operation_qif -> categ)) )
		    {
		      if ( operation_qif -> categ[0] == '[' )
			{
			  /* recherche du compte en relation ( on cherchera l'opé ensuite, une fois */
			  /* que toutes les opés auront été importées */

			  gint i;

			  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

			  for ( i = 0 ; i < nb_comptes ; i++ )
			    {
			      if ( !g_strcasecmp ( g_strconcat ( "[",
								 NOM_DU_COMPTE,
								 "]",
								 NULL ),
						   operation_qif -> categ ))
				{
				  /* on a trouvé le compte, met la relation de l'opé à -1 pour dire qu'il faut la chercher */
				  operation -> relation_no_compte = i;
				  operation -> relation_no_operation = -1;
				  operation -> type_ope = 1;
				}
			      p_tab_nom_de_compte_variable++;
			    }
			}
		      else
			{
			  struct struct_categ *categ;
			  struct struct_sous_categ *sous_categ ;


			  tab_str = g_strsplit ( operation_qif -> categ, ":", 2 );


			  /* récupération ou création de la catégorie */

			  if ( !g_slist_find_custom ( liste_struct_categories,
						      g_strstrip ( tab_str[0] ),
						      (GCompareFunc) recherche_categorie_par_nom ))
			    {
			      categ = calloc ( 1, sizeof ( struct struct_categ ));

			      categ -> no_categ = ++no_derniere_categorie;
			      categ -> nom_categ = g_strdup ( g_strstrip ( tab_str[0] ) );

			      if ( operation_qif -> montant < 0 )
				categ -> type_categ = 1;
			      else
				categ -> type_categ = 0;

			      nb_enregistrements_categories++;

			      liste_struct_categories = g_slist_append ( liste_struct_categories,
									 categ );
			    }
			  else
			    categ = g_slist_find_custom ( liste_struct_categories,
							  g_strstrip ( tab_str[0] ),
							  (GCompareFunc) recherche_categorie_par_nom ) -> data;


			  operation -> categorie = categ -> no_categ;


			  /* récupération ou création de la sous-catégorie */

			  if ( tab_str[1] )
			    {
			      if ( !g_slist_find_custom ( categ -> liste_sous_categ,
							  g_strstrip ( tab_str[1] ),
							  (GCompareFunc) recherche_sous_categorie_par_nom ))
				{
				  sous_categ = calloc ( 1, sizeof ( struct struct_sous_categ ));

				  sous_categ -> no_sous_categ = ++( categ -> no_derniere_sous_categ );
				  sous_categ -> nom_sous_categ = g_strdup ( g_strstrip ( tab_str[1] ));

				  categ -> liste_sous_categ = g_slist_append ( categ -> liste_sous_categ,
									       sous_categ );
				}
			      else
				sous_categ = g_slist_find_custom ( categ -> liste_sous_categ,
								   g_strstrip ( tab_str[1] ),
								   (GCompareFunc) recherche_sous_categorie_par_nom ) -> data;

			      operation -> sous_categorie = sous_categ -> no_sous_categ;

			    }

			  g_strfreev ( tab_str );

			}
		    }
		}

	      /* récupération des notes */

	      operation -> notes = operation_qif -> notes;


	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	      /* récupération du chèque et mise en forme du type d'opération */

	      if ( operation_qif -> cheque )
		{
		  /* c'est un chèque, on va rechercher un type à incrémentation automatique et mettre l'opé sous ce type */
		  /* si un tel type n'existe pas, on met quand même le no dans contenu_type et on met le type par défaut */

		  struct struct_type_ope *type_choisi;
		  GSList *liste_tmp;

		  if ( operation -> montant < 0 )
		    operation -> type_ope = TYPE_DEFAUT_DEBIT;
		  else
		    operation -> type_ope = TYPE_DEFAUT_CREDIT;

		  operation -> contenu_type = itoa ( operation_qif -> cheque );

		  liste_tmp = TYPES_OPES;
		  type_choisi = NULL;

		  while ( liste_tmp )
		    {
		      struct struct_type_ope *type;

		      type = liste_tmp -> data;

		      /* si l'opé on trouve un type à incrémentation
			 automatique et que le signe du type est bon,
			 on l'enregistre et on arrête la recherche,
			 sinon, on l'enregistre mais on continue la
			 recherche dans l'espoir de mieux */

		      if ( type -> numerotation_auto )
			{
			  if ( !type -> signe_type
			       ||
			       ( type -> signe_type == 1 && operation -> montant < 0 )
			       ||
			       ( type -> signe_type == 2 && operation -> montant > 0 ))
			    {
			      operation -> type_ope = type -> no_type;
			      type_choisi = type;
			      liste_tmp = NULL;
			    }
			  else
			    {
			      operation -> type_ope = type -> no_type;
			      type_choisi = type;
			      liste_tmp = liste_tmp -> next;
			    }
			}
		      else
			liste_tmp = liste_tmp -> next;
		    }

		  /* type_choisi contient l'adr du type qui a été utilisé, on peut y mettre le dernier no de chèque */

		  if ( type_choisi )
		    type_choisi -> no_en_cours = MAX ( operation_qif -> cheque,
						       type_choisi -> no_en_cours );
		}
	      else
		{
		  /* comme ce n'est pas un chèque, on met sur le type par défaut */
		  if ( operation -> type_ope != 1 )
		    {
		      if ( operation -> montant < 0 )
			operation -> type_ope = TYPE_DEFAUT_DEBIT;
		      else
			operation -> type_ope = TYPE_DEFAUT_CREDIT;
		    }
		}

	      /* récupération du pointé */

	      operation -> pointe = operation_qif -> p_r;


	      /* si c'est une ope de ventilation, lui ajoute le no de l'opération précédente */

	      if ( operation_qif -> ope_de_ventilation )
		operation -> no_operation_ventilee_associee = derniere_operation;
	      else
		derniere_operation = operation -> no_operation;


	      /* ajoute l'opération dans la liste des opés du compte */

	      compte -> gsliste_operations = g_slist_append ( compte -> gsliste_operations,
							      operation );
	      compte -> nb_operations++;
	    }

	  liste_tmp = liste_tmp -> next;
	}

      compte -> nb_lignes_ope = 3;
      compte -> solde_courant = solde_courant;
      compte -> date_releve = NULL;
      compte -> operation_selectionnee = GINT_TO_POINTER ( -1 );

      liste_comptes_tmp = liste_comptes_tmp -> next;
    }



  /* on fait maintenant le lien entre les différents virements : */
  /* on fait le tour de toutes les opés des comptes ajoutés, */
  /* si une opé à un relation_no_operation à -1 , */
  /* on recherche dans le compte associé une opé avec aussi une relation_no_operation à -1  */
  /* le même jour avec le même tiers */


  liste_comptes_tmp = liste_comptes_qif;

  while ( liste_comptes_tmp )
    {
      struct donnees_compte *compte;
      struct structure_operation *operation;


      compte = liste_comptes_tmp -> data;

      liste_tmp = compte -> gsliste_operations;

      while ( liste_tmp )
	{

	  operation = liste_tmp -> data;


	  if ( operation -> relation_no_operation == -1 )
	    {
	      GSList *pointeur_tmp;

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	      pointeur_tmp = LISTE_OPERATIONS;

	      while ( pointeur_tmp )
		{
		  struct structure_operation *operation_2;

		  operation_2 = pointeur_tmp -> data;

		  if ( operation_2 -> relation_no_operation == -1 )
		    {
		      if ( !g_date_compare ( operation -> date, operation_2 -> date )
			   &&
			   ( operation -> tiers == operation_2 -> tiers )
			   &&
			   ( fabs ( operation -> montant ) == fabs ( operation_2 -> montant ) ))
			{
			  operation -> relation_no_operation = operation_2 -> no_operation;
			  operation_2 -> relation_no_operation = operation -> no_operation;
			}
		    }

		  pointeur_tmp = pointeur_tmp -> next;
		}

	    }
	  liste_tmp = liste_tmp -> next;
	}

      liste_comptes_tmp = liste_comptes_tmp -> next;
    }



  /* création des listes d'opé */

  mise_en_route_attente ( _("Processing the information can be long...") );

  if ( nouveau_fichier )
    {
      creation_fenetre_principale();

      creation_listes_operations ();
      changement_compte ( GINT_TO_POINTER ( compte_courant ) );
      gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 0 );
    }
  else
    while ( nb_comptes_ajoutes )
      {
	/* trie la liste */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + nb_comptes - nb_comptes_ajoutes;

	LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
					  ( GCompareFunc ) comparaison_date_list_ope );

	ajoute_nouvelle_liste_operation( nb_comptes - nb_comptes_ajoutes );
	nb_comptes_ajoutes--;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;
	MISE_A_JOUR = 1;
      }


  /* on recrée les combofix des tiers et des catégories */

  mise_a_jour_tiers ();
  mise_a_jour_categ();

/* on met à jour l'option menu du formulaire des échéances */

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[5] ),
			     creation_option_menu_comptes (changement_choix_compte_echeancier, TRUE) );

  /* mise à jour de l'accueil */

  update_liste_comptes_accueil ();
  mise_a_jour_soldes_minimaux ();
  /* actualise la liste des comptes dans l'onglet accueil
     ferme le bogue #185 */
  reaffiche_liste_comptes_onglet();
  affiche_titre_fenetre ();
  annulation_attente();

  gnome_app_set_contents ( GNOME_APP ( window ), notebook_general );
  modification_fichier ( TRUE );
}
/* *******************************************************************************/




/* *******************************************************************************/
/* Affiche la fenêtre de sélection de fichier pour exporter en qif */
/* *******************************************************************************/

void exporter_fichier_qif ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *entree;
  GtkWidget *check_button;
  gint i;
  gint resultat;
  GSList *liste_tmp;
  gchar *nom_fichier_qif;
  FILE *fichier_qif;
  gchar *montant_tmp;
  

  if ( !nom_fichier_comptes )
    {
      dialogue ( _("You must name (save) your file to be able to export it.") );
      return;
    }


  dialogue_warning ( _("QIF files don't use currencies.\nAll transactions will be converted to appropriate account's currency.") );

  dialog = gnome_dialog_new ( _("Export QIF files"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
		        "destroy" );

  label = gtk_label_new ( COLON(_("Select the accounts to export")) );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( frame );


  table = gtk_table_new ( 3,
			  nb_comptes,
			  FALSE );
  gtk_container_set_border_width ( GTK_CONTAINER ( table ),
				   10 );
  gtk_table_set_col_spacings ( GTK_TABLE ( table ),
			       10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      table );
  gtk_widget_show ( table );



  /* on met chaque compte dans la table */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {

      check_button = gtk_check_button_new ();
      gtk_table_attach ( GTK_TABLE ( table ),
			 check_button,
			 0, 1,
			 i, i+1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( check_button );

      label = gtk_label_new ( NOM_DU_COMPTE );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table ),
			 label,
			 1, 2,
			 i, i+1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      entree = gnome_file_entry_new ("qif", _("Export a QIF file"));
      gtk_widget_set_usize ( GTK_WIDGET ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( entree ) )),
			     300,
			     FALSE );
      gnome_file_entry_set_default_path ( GNOME_FILE_ENTRY ( entree ),
					  dernier_chemin_de_travail );
      gtk_entry_set_text ( GTK_ENTRY (gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( entree ) ) ),
			   g_strconcat ( nom_fichier_comptes,
					 "-",
					 g_strdelimit ( g_strdup ( NOM_DU_COMPTE) , " ", '_' ),
					 ".qif",
					 NULL ));
      gtk_widget_set_sensitive ( entree,
				 FALSE );
      gtk_object_set_data ( GTK_OBJECT ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( entree ) ) ),
			    "no_compte",
			    GINT_TO_POINTER ( i ));
      gtk_table_attach ( GTK_TABLE ( table ),
			 entree,
			 2, 3,
			 i, i+1,
			 GTK_SHRINK | GTK_FILL,
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

 choix_liste_fichier:
  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( resultat || !liste_entrees_exportation )
    {
      if ( liste_entrees_exportation )
	g_slist_free ( liste_entrees_exportation );

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }

  /* vérification que tous les fichiers sont enregistrables */

  liste_tmp = liste_entrees_exportation;

  while ( liste_tmp )
    {
      struct stat test_fichier;


      nom_fichier_qif = (gchar *) gtk_entry_get_text ( GTK_ENTRY ( liste_tmp -> data ));


      if ( stat ( nom_fichier_qif,
		  &test_fichier ) != -1 )
	{
	  if ( S_ISREG ( test_fichier.st_mode ) )
	    {
	      GtkWidget *etes_vous_sur;

	      etes_vous_sur = gnome_dialog_new ( _("Save file"),
						 GNOME_STOCK_BUTTON_YES,
						 GNOME_STOCK_BUTTON_NO,
						 NULL );
	      label = gtk_label_new ( _("File exists. Do you want to overwrite it?") );
	      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( etes_vous_sur ) -> vbox ),
				   label,
				   TRUE,
				   TRUE,
				   5 );
	      gtk_widget_show ( label );

	      gnome_dialog_set_default ( GNOME_DIALOG ( etes_vous_sur ),
					 1 );
	      gtk_window_set_transient_for ( GTK_WINDOW ( etes_vous_sur ),
					     GTK_WINDOW ( dialog ));

	      if ( gnome_dialog_run_and_close ( GNOME_DIALOG ( etes_vous_sur ) ) )
		goto choix_liste_fichier;
	    }
	  else
	    {
	      dialogue ( g_strdup_printf ( _("Invalid filename \"%s\"!"),
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

      nom_fichier_qif = (gchar *) gtk_entry_get_text ( GTK_ENTRY ( liste_tmp -> data ));

      if ( !( fichier_qif = fopen ( nom_fichier_qif,
				    "w" ) ))
	dialogue ( g_strdup_printf ( _("The following error occured for the file \"%s\":\n%s"),
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

		      if ( operation -> pointe == 1 )
			fprintf ( fichier_qif,
				  "C*\n" );
		      else
			if ( operation -> pointe == 2 )
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

				/* met le montant de la ventilation
				  transforme la devise si necessaire */

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



  gnome_dialog_close ( GNOME_DIALOG ( dialog ));

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
						   gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( entree)) );
    }
  else
    {
      gtk_widget_set_sensitive ( entree,
				 FALSE );
      liste_entrees_exportation = g_slist_remove ( liste_entrees_exportation,
						   gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( entree)) );
    }

}
/* *******************************************************************************/

