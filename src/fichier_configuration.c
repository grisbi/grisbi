/* ce fichier se charge de toutes les opérations relative à la configuration sauvegardée */

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
#include "en_tete.h"


/******************************************************************************************************************/
/* Fonction charge_configuration */
/* appelée à l'ouverture de grisbi, charge les préférences */
/******************************************************************************************************************/

void charge_configuration ( void )
{
  gint nb_fichiers_a_verifier;
  gchar **tab_noms_fichiers;
  gint i;
  gint flag;
  struct stat buffer_stat;

  /*   on vérifie que le fichier de conf existe bien ; dans le cas contraire, on charge */
  /* la conf par défaut */

  if ( stat ( g_strconcat ( getenv ("HOME"), "/.gnome2/", FICHIER_CONF, NULL ),
	      &buffer_stat ) == -1 )
    {
      raz_configuration ();
      return;
    }

  /* on récupère la taille de la fenêtre à l'arrêt précédent */

  largeur_window = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Width", NULL ));
  hauteur_window = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Height", NULL ));

  etat.r_modifiable = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Modification_operations_rapprochees", NULL ));
  dernier_chemin_de_travail = gnome_config_get_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Dernier_chemin_de_travail", NULL ));

  if ( !dernier_chemin_de_travail )
    dernier_chemin_de_travail = g_strconcat ( getenv ("HOME"),
					      "/",
					      NULL );

  etat.entree = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonction_touche_entree", NULL ));
  etat.alerte_mini = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Affichage_messages_alertes", NULL ));

  /* FIXME : do that with list_font_name & list_font_size */
/*   fonte_liste = gnome_config_get_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonte_des_listes", NULL )); */
/*   fonte_general = gnome_config_get_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonte_generale", NULL )); */
  etat.alerte_permission = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Affichage_alerte_permission", NULL ));
  etat.force_enregistrement = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Force_enregistrement", NULL ));

  if ( fonte_liste && !strlen( fonte_liste ) )
    fonte_liste = NULL;
  if ( fonte_general && !strlen( fonte_general ) )
    fonte_general = NULL;

  etat.dernier_fichier_auto = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Chargement_auto_dernier_fichier", NULL ));
  nom_fichier_comptes = gnome_config_get_string ( g_strconcat ( "/", FICHIER_CONF, "/IO/Nom_dernier_fichier", NULL ));
  etat.sauvegarde_auto = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Enregistrement_automatique", NULL ));
  etat.sauvegarde_demarrage = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Enregistrement_au_demarrage", NULL ));

  nb_max_derniers_fichiers_ouverts = gnome_config_get_int_with_default ( g_strconcat ( "/", FICHIER_CONF, "/IO/Nb_max_derniers_fichiers_ouverts", NULL ),
									 &flag );
  if ( flag )
    nb_max_derniers_fichiers_ouverts = 3;

  compression_fichier = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Compression_fichier", NULL ));
  compression_backup = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Compression_backup", NULL ));
  xmlSetCompressMode ( compression_fichier );
  
  gnome_config_get_vector ( g_strconcat ( "/", FICHIER_CONF, "/IO/Liste_noms_derniers_fichiers_ouverts", NULL ),
			    &nb_derniers_fichiers_ouverts,
			    &tab_noms_derniers_fichiers_ouverts );

  /* quand il n'y a aucun dernier fichier ouvert, il en récupère quand même un vide */
  /* ... on le vire ... */

  if ( nb_derniers_fichiers_ouverts == 1
       &&
       !strlen ( tab_noms_derniers_fichiers_ouverts[0]))
    {
      tab_noms_derniers_fichiers_ouverts[0] = NULL;
      nb_derniers_fichiers_ouverts = 0;
    }

  decalage_echeance = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Echeances/Delai_rappel_echeances", NULL ));

  gnome_config_get_vector ( g_strconcat ( "/", FICHIER_CONF, "/Applet/Fichiers_a_verifier", NULL ),
			    &nb_fichiers_a_verifier,
			    &tab_noms_fichiers );

  etat.formulaire_toujours_affiche = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affichage_formulaire", NULL ));
  etat.formulaire_echeancier_toujours_affiche  = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affichage_formulaire_echeancier", NULL ));

  etat.affiche_tous_les_types = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affichage_tous_types", NULL ));
  etat.affiche_no_operation = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affiche_no_operation", NULL ));
  etat.affiche_date_bancaire = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affiche_date_bancaire", NULL ));
  etat.classement_par_date = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Tri_par_date", NULL ));
  etat.affiche_boutons_valider_annuler = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affiche_boutons_valider_annuler", NULL ));
  etat.largeur_auto_colonnes  = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Largeur_auto_colonnes", NULL ));
  etat.retient_affichage_par_compte  = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Caracteristiques_par_compte", NULL ));



  etat.affichage_exercice_automatique  = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Exercice/Affichage_exercice_automatique", NULL ));
  etat.affiche_nb_ecritures_listes  = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Exercice/Affichage_nb_ecritures", NULL ));

  for ( i=0 ; i<7 ; i++ )
    taille_largeur_colonnes[i] = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Exercice/taille_largeur_colonne", itoa(i), NULL ));

  /* remplissage de la liste des fichiers à vérifier */

  fichier_a_verifier = NULL;

  for ( i = 0 ; i < nb_fichiers_a_verifier ; i++ )
    fichier_a_verifier = g_slist_append ( fichier_a_verifier,
					  tab_noms_fichiers[i] );

}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction RAZ_configuration */
/* ***************************************************************************************************** */

void raz_configuration ( void )
{

  largeur_window = 0;
  hauteur_window = 0;

  etat.alerte_permission = 1;
  etat.alerte_mini = 1;
  etat.r_modifiable = 0;       /* on ne peux modifier les opé relevées */
  etat.dernier_fichier_auto = 1;   /*  on n'ouvre pas directement le dernier fichier */
  buffer_dernier_fichier = g_strdup ( "" );
  etat.sauvegarde_auto = 0;    /* on ne sauvegarde pas automatiquement */
  etat.entree = 1;    /* la touche entree provoque l'enregistrement de l'opération */
  decalage_echeance = 3;     /* nb de jours avant l'échéance pour prévenir */
  etat.alerte_mini = 1;     /* alerte si dépassement des seuils mini définis */
  etat.formulaire_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
  etat.formulaire_echeancier_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
  etat.affichage_exercice_automatique = 1;        /* l'exercice est choisi en fonction de la date */
  fonte_liste = NULL;
  fonte_general = NULL;
  etat.alerte_permission = 1;       /* par défaut, on prévient quand le fichier n'est pas à 600 */
  etat.force_enregistrement = 0;     /* par défaut, on ne force pas l'enregistrement */
  etat.affiche_tous_les_types = 0;   /* par défaut, on n'affiche ds le formulaire que les types du débit ou crédit */
  etat.classement_par_date = 1;  /* par défaut, on tri la liste des opés par les dates */
  etat.affiche_boutons_valider_annuler = 1;
  etat.classement_par_date = 1;
  dernier_chemin_de_travail = g_strconcat ( getenv ("HOME"),
					    "/",
					    NULL );
  nb_derniers_fichiers_ouverts = 0;
  nb_max_derniers_fichiers_ouverts = 3;
  tab_noms_derniers_fichiers_ouverts = NULL;
  compression_fichier = 0;     /* pas de compression par défaut */
  compression_backup = 0;
  etat.largeur_auto_colonnes = 1;
  etat.retient_affichage_par_compte = 0;
}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction sauve_configuration */
/* Appelée à chaque changement de configuration */
/* ***************************************************************************************************** */

void sauve_configuration (void)
{
  GSList *pointeur_fichier_a_verifier;
  gint i;
  gchar **tab_pointeurs;

  /*   sauvegarde de la géométrie */

  if ( GTK_WIDGET ( window) -> window )
    {
      gtk_window_get_size ( GTK_WIDGET(window), 
			    &largeur_window, &hauteur_window);
    }
  else
    {
      largeur_window = 0;
      hauteur_window = 0;
    }

  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Width", NULL ),
			 largeur_window );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Height", NULL ),
			 hauteur_window );

  /* sauvegarde de l'onglet général */
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Modification_operations_rapprochees", NULL ),
			 etat.r_modifiable );
  gnome_config_set_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Dernier_chemin_de_travail", NULL ),
			    dernier_chemin_de_travail );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Affichage_alerte_permission", NULL ),
			 etat.alerte_permission );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Force_enregistrement", NULL ),
			 etat.force_enregistrement );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonction_touche_entree", NULL ),
			 etat.entree );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Affichage_messages_alertes", NULL ),
			 etat.alerte_mini );
  gnome_config_set_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonte_des_listes", NULL ),
			    fonte_liste );
  gnome_config_set_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonte_generale", NULL ),
			    fonte_general );


  /* sauvegarde de l'onglet I/O */

  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Chargement_auto_dernier_fichier", NULL ),
			 etat.dernier_fichier_auto );
  gnome_config_set_string ( g_strconcat ( "/", FICHIER_CONF, "/IO/Nom_dernier_fichier", NULL ),
			    nom_fichier_comptes );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Enregistrement_automatique", NULL ),
			 etat.sauvegarde_auto );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Enregistrement_au_demarrage", NULL ),
			 etat.sauvegarde_demarrage );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Nb_max_derniers_fichiers_ouverts", NULL ),
			 nb_max_derniers_fichiers_ouverts );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Compression_fichier", NULL ),
			 compression_fichier );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/IO/Compression_backup", NULL ),
			 compression_backup );
  gnome_config_set_vector ( g_strconcat ( "/", FICHIER_CONF, "/IO/Liste_noms_derniers_fichiers_ouverts", NULL ),
			    nb_derniers_fichiers_ouverts,
			    (const char **) tab_noms_derniers_fichiers_ouverts );

  /* sauvegarde de l'onglet échéances */

  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Echeances/Delai_rappel_echeances", NULL ),
			 decalage_echeance );


  /* sauvegarde des fichiers à vérifier par l'applet */

  tab_pointeurs = malloc ( g_slist_length ( fichier_a_verifier ) * sizeof ( gchar * ) );

  pointeur_fichier_a_verifier = fichier_a_verifier;
  i = 0;
  
  while ( pointeur_fichier_a_verifier )
    {
      tab_pointeurs[i] = pointeur_fichier_a_verifier -> data;
      i++;
      pointeur_fichier_a_verifier = pointeur_fichier_a_verifier -> next;
    }


  gnome_config_set_vector ( g_strconcat ( "/", FICHIER_CONF, "/Applet/Fichiers_a_verifier", NULL ),
			    g_slist_length ( fichier_a_verifier ),
			    (const char **) tab_pointeurs );


  /*   sauvegarde de l'onglet affichage */

  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affichage_formulaire", NULL ),
			 etat.formulaire_toujours_affiche );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affichage_formulaire_echeancier", NULL ),
			 etat.formulaire_echeancier_toujours_affiche );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affichage_tous_types", NULL ),
			 etat.affiche_tous_les_types );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affiche_no_operation", NULL ),
			 etat.affiche_no_operation );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affiche_date_bancaire", NULL ),
			 etat.affiche_date_bancaire );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Tri_par_date", NULL ),
			 etat.classement_par_date );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Affiche_boutons_valider_annuler", NULL ),
			 etat.affiche_boutons_valider_annuler );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Largeur_auto_colonnes", NULL ),
			 etat.largeur_auto_colonnes );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Affichage/Caracteristiques_par_compte", NULL ),
			 etat.retient_affichage_par_compte );

  for ( i=0 ; i<7 ; i++ )
    gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Exercice/taille_largeur_colonne", itoa(i), NULL ),
			   taille_largeur_colonnes[i] );


  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Exercice/Affichage_nb_ecritures", NULL ),
			 etat.affiche_nb_ecritures_listes );

  /* sauvegarde de l'onglet exercice */

  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Exercice/Affichage_exercice_automatique", NULL ),
			 etat.affichage_exercice_automatique );

  gnome_config_sync();

}
/* ***************************************************************************************************** */
