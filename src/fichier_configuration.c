/* ce fichier se charge de toutes les opérations relatives à la
   configuration sauvegardée */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2004 Axel Rousseau (axel584@axel584.org) */
/*			2004 Benjamin Drieu (bdrieu@april.org) */
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

#include "fichier_configuration.h"
#include "fichiers_io.h"
#include <libxml/tree.h>


void charge_configuration ( void )
{
  gint nb_fichiers_a_verifier;
  gchar **tab_noms_fichiers;
  gint i;
  gint flag;
  xmlDocPtr doc;
  xmlNodePtr node;
  xmlNodePtr node_1;
  int result;
  struct stat buffer_stat;

  if ( stat ( g_strconcat ( getenv ("HOME"), "/.grisbirc", NULL ),&buffer_stat ) == -1 ) {
    if ( ! gnome_config_get_int( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Width", NULL) )  )
      {
	raz_configuration ();
	return;
      }
    else {
      charge_configuration_ancien();
      return;
    }
  }

  doc = xmlParseFile ( g_strconcat ( getenv ("HOME"), "/.grisbirc", NULL ) );

  /* vérifications d'usage */
  xmlNodePtr root = xmlDocGetRootElement(doc);

  if ( !root
       ||
       !root->name
       ||
       g_strcasecmp ( root->name,
		      "Configuration" ))
    {
      xmlFreeDoc ( doc );
      return;
    }

  /* On se place sur Generalite*/
  node = root -> children;

  while (node) {
    if ( !strcmp ( node -> name, "Geometry" ) )
      {
	xmlNodePtr node_geometry;
	node_geometry = node -> children;
	while (node_geometry) {
	  if ( !strcmp ( node_geometry -> name, "Width" ) ) {
	    largeur_window = atoi(xmlNodeGetContent ( node_geometry));
	  }
	  if ( !strcmp ( node_geometry -> name, "Height" ) ) {
	    hauteur_window = atoi(xmlNodeGetContent ( node_geometry));
	  }
	  node_geometry = node_geometry->next;
	}
      }
    if ( !strcmp ( node -> name, "General" ) )
      {
	xmlNodePtr node_general;
	node_general = node -> children;
	while (node_general) {
	  if ( !strcmp ( node_general -> name, "Modification_operations_rapprochees" ) ) {
	    etat.r_modifiable = atoi(xmlNodeGetContent ( node_general));
	  }
	  if ( !strcmp ( node_general -> name, "Dernier_chemin_de_travail" ) ) {
	    dernier_chemin_de_travail = xmlNodeGetContent ( node_general);
	    if ( !dernier_chemin_de_travail )
	      dernier_chemin_de_travail = g_strconcat ( getenv ("HOME"), "/",NULL );
	  }
	  if ( !strcmp ( node_general -> name, "Affichage_alerte_permission" ) ) {
	    etat.alerte_permission = atoi(xmlNodeGetContent ( node_general));
	  }
	  if ( !strcmp ( node_general -> name, "Force_enregistrement" ) ) {
	    etat.force_enregistrement = atoi(xmlNodeGetContent ( node_general));
	  }
	  if ( !strcmp ( node_general -> name, "Fonction_touche_entree" ) ) {
	    etat.entree = atoi(xmlNodeGetContent ( node_general));
	  }
	  if ( !strcmp ( node_general -> name, "Affichage_messages_alertes" ) ) {
	    etat.alerte_mini = atoi(xmlNodeGetContent ( node_general));
	  }

	  if ( !strcmp ( node_general -> name, "Fonte_des_listes" ) ) {
	    fonte_liste = xmlNodeGetContent ( node_general);
	  }

	  if ( !strcmp ( node_general -> name, "Fonte_generale" ) ) {
	    fonte_general = xmlNodeGetContent ( node_general);
	  }
	  node_general = node_general->next;
	}
      }
    if ( fonte_liste && !strlen( fonte_liste ) ) {
      fonte_liste = NULL;
    }
    if ( fonte_general && !strlen( fonte_general ) ) {
      fonte_general = NULL;
    }

    if ( !strcmp ( node -> name, "IO" ) )
      {
	xmlNodePtr node_io;
	node_io = node -> children;
	while (node_io) {
	  if ( !strcmp ( node_io -> name, "Chargement_auto_dernier_fichier" ) ) {
	    etat.dernier_fichier_auto = atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Nom_dernier_fichier" ) ) {
	    nom_fichier_comptes = xmlNodeGetContent ( node_io);
	  }
	  if ( !strcmp ( node_io -> name, "Enregistrement_automatique" ) ) {
	    etat.sauvegarde_auto = atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Enregistrement_au_demarrage" ) ) {
	    etat.sauvegarde_demarrage = atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Nb_max_derniers_fichiers_ouverts" ) ) {
	    nb_max_derniers_fichiers_ouverts = atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Compression_fichier" ) ) {
	    compression_fichier = atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Compression_backup" ) ) {
	    compression_backup = atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io->name, "Liste_noms_derniers_fichiers_ouverts" ) ) {
	    nb_derniers_fichiers_ouverts = 0;
	    xmlNodePtr node_filename = node_io -> children;
	    tab_noms_derniers_fichiers_ouverts = malloc ( nb_max_derniers_fichiers_ouverts * sizeof(gchar *) );
	    while ( node_filename ) {
	      if ( !strcmp ( node_filename -> name, "fichier" ) ) {
		tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts] = xmlNodeGetContent ( node_filename );
		nb_derniers_fichiers_ouverts++;
	      }
	      node_filename = node_filename->next;
	    }
	  }

	  node_io = node_io->next;
	}
      }
    if ( !strcmp ( node -> name, "Echeances" ) )
      {
	xmlNodePtr node_echeances;
	node_echeances = node -> children;
	while (node_echeances) {
	  if ( !strcmp ( node_echeances -> name, "Delai_rappel_echeances" ) ) {
	    decalage_echeance = atoi(xmlNodeGetContent ( node_echeances));
	  }
	  node_echeances = node_echeances->next;
	}
      }
    /*if ( !strcmp ( node -> name, "Applet" ) )
      {
      xmlNodePtr node_io;
      node_io = node -> children;
      while (node_io) {
      if ( !strcmp ( node_io -> name, "Chargement_auto_dernier_fichier" ) ) {
      etat.dernier_fichier_auto = atoi(xmlNodeGetContent ( node_io));
      }
      if ( !strcmp ( node_io -> name, "Nom_dernier_fichier" ) ) {
      nom_fichier_comptes = xmlNodeGetContent ( node_io);
      }
      if ( !strcmp ( node_io -> name, "Enregistrement_automatique" ) ) {
      etat.sauvegarde_auto = atoi(xmlNodeGetContent ( node_io));
      }
      if ( !strcmp ( node_io -> name, "Enregistrement_au_demarrage" ) ) {
      etat.sauvegarde_demarrage = atoi(xmlNodeGetContent ( node_io));
      }
      if ( !strcmp ( node_io -> name, "Nb_max_derniers_fichiers_ouverts" ) ) {
      nb_max_derniers_fichiers_ouverts = atoi(xmlNodeGetContent ( node_io));
      }
      if ( !strcmp ( node_io -> name, "Compression_fichier" ) ) {
      compression_fichier = atoi(xmlNodeGetContent ( node_io));
      }
      if ( !strcmp ( node_io -> name, "Compression_backup" ) ) {
      variable = xmlNodeGetContent ( node_io);
      }
      // boucler pour avoir la liste des derniers fichiers.
      node_io = node_io->next;
      }
      }*/
    if ( !strcmp ( node -> name, "Affichage" ) )
      {
	xmlNodePtr node_affichage;
	node_affichage = node -> children;
	while (node_affichage) {
	  if ( !strcmp ( node_affichage -> name, "Affichage_formulaire" ) ) {
	    etat.formulaire_toujours_affiche = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Affichage_formulaire_echeancier" ) ) {
	    etat.formulaire_echeancier_toujours_affiche = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Affichage_tous_types" ) ) {
	    etat.affiche_tous_les_types = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Affiche_no_operation" ) ) {
	    etat.affiche_no_operation = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Affiche_date_bancaire" ) ) {
	    etat.affiche_date_bancaire = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Tri_par_date" ) ) {
	    etat.classement_par_date = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Affiche_boutons_valider_annuler" ) ) {
	    etat.affiche_boutons_valider_annuler = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Largeur_auto_colonnes" ) ) {
	    etat.largeur_auto_colonnes = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Caracteristiques_par_compte" ) ) {
	    etat.retient_affichage_par_compte = atoi( xmlNodeGetContent ( node_affichage));
	  }
	  // boucler pour avoir les tailles des différentes colonnes
	  if ( !strcmp ( node_affichage -> name, "taille_largeur_colonne" ) ) {
	    //int numero_colonne;
	    //int largeur_colonne;
	    int numero_colonne = atoi(xmlGetProp ( node_affichage, "No"));
	    int largeur_colonne = atoi(xmlNodeGetContent ( node_affichage));
	    taille_largeur_colonnes[numero_colonne] = largeur_colonne;
	  }
	  if ( !strcmp ( node_affichage -> name, "Affichage_exercice_automatique" ) ) {
	    etat.affichage_exercice_automatique = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  if ( !strcmp ( node_affichage -> name, "Affichage_nb_ecritures" ) ) {
	    etat.affiche_nb_ecritures_listes = atoi(xmlNodeGetContent ( node_affichage));
	  }
	  node_affichage = node_affichage->next;
	}
      }
    if ( !strcmp ( node -> name, "Exercice" ) )
      {
	xmlNodePtr node_exercice;
	node_exercice = node -> children;
	while (node_exercice) {
	  node_exercice = node_exercice->next;
	}
      }
    if ( !strcmp ( node -> name, "Messages" ) )
      {
	xmlNodePtr node_messages;
	node_messages = node -> children;
	while (node_messages) {
	  if ( !strcmp ( node_messages -> name, "display_message_lock_active" ) ) {
	    etat.display_message_lock_active = atoi(xmlNodeGetContent ( node_messages));
	  }
	  if ( !strcmp ( node_messages -> name, "display_message_file_readable" ) ) {
	    etat.display_message_file_readable = atoi(xmlNodeGetContent ( node_messages));
	  }
	  if ( !strcmp ( node_messages -> name, "display_message_minimum_alert" ) ) {
	    etat.display_message_minimum_alert = atoi(xmlNodeGetContent ( node_messages));
	  }
	  node_messages = node_messages->next;
	}
      }
    node = node -> next;
  }
}
/* ***************************************************************************************************** */


/******************************************************************************************************************/
/* Fonction charge_configuration */
/* appelée à l'ouverture de grisbi, charge les préférences */
/******************************************************************************************************************/

void charge_configuration_ancien ( void )
{
  gint nb_fichiers_a_verifier;
  gchar **tab_noms_fichiers;
  gint i;
  gint flag;
  struct stat buffer_stat;

  if ( ! gnome_config_get_int( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Width", NULL) )  )
    {
      raz_configuration ();
      return;
    }

   /*on récupère la taille de la fenêtre à l'arrêt précédent */

  largeur_window = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Width", NULL ));
  hauteur_window = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Geometry/Height", NULL ));

  etat.r_modifiable = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Modification_operations_rapprochees", NULL ));
  dernier_chemin_de_travail = gnome_config_get_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Dernier_chemin_de_travail", NULL ));

  if ( !dernier_chemin_de_travail )
    dernier_chemin_de_travail = g_strconcat ( getenv ("HOME"),
					      "/",
					      NULL );

  etat.entree = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonction_touche_entree", NULL ));

  /*FIXME : do that with list_font_name & list_font_size */
   fonte_liste = gnome_config_get_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonte_des_listes", NULL ));
   fonte_general = gnome_config_get_string ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonte_generale", NULL ));
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

  fichier_a_verifier = NULL;

  for ( i = 0 ; i < nb_fichiers_a_verifier ; i++ )
    fichier_a_verifier = g_slist_append ( fichier_a_verifier,
					  tab_noms_fichiers[i] );


  etat.display_message_lock_active  = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Messages/display_message_lock_active", NULL ));
  etat.display_message_file_readable  = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Messages/display_message_file_readable", NULL ));
  etat.display_message_minimum_alert = gnome_config_get_int ( g_strconcat ( "/", FICHIER_CONF, "/Messages/display_message_minimum_alert", NULL ));


}
/*************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction RAZ_configuration */
/* ***************************************************************************************************** */

void raz_configuration ( void )
{

  largeur_window = 0;
  hauteur_window = 0;

  etat.r_modifiable = 0;       /* on ne peux modifier les opé relevées */
  etat.dernier_fichier_auto = 1;   /*  on n'ouvre pas directement le dernier fichier */
  buffer_dernier_fichier = g_strdup ( "" );
  etat.sauvegarde_auto = 0;    /* on ne sauvegarde pas automatiquement */
  etat.entree = 1;    /* la touche entree provoque l'enregistrement de l'opération */
  decalage_echeance = 3;     /* nb de jours avant l'échéance pour prévenir */
  etat.formulaire_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
  etat.formulaire_echeancier_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
  etat.affichage_exercice_automatique = 1;        /* l'exercice est choisi en fonction de la date */
  fonte_liste = NULL;
  fonte_general = NULL;
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

  /* Messages */
  etat.display_message_lock_active = 0;
  etat.display_message_file_readable = 0;
  etat.display_message_minimum_alert = 0;
}
/* ***************************************************************************************************** */

/* ***************************************************************************************************** */
/* Fonction sauve_configurationXML */
/* Appelée à chaque changement de configuration */
/* ***************************************************************************************************** */

void sauve_configurationXML(void)
{
  GSList *pointeur_fichier_a_verifier;
  gint i, x, y;
  gchar **tab_pointeurs;
	// document XML (voir fichiers_io.c l.4000)
  xmlDocPtr doc;
  xmlNodePtr node;
  xmlNodePtr node_1;
  xmlNodePtr node_2;
  char buff[15];
	// resultat de la sauvegarde
  gint resultat;


  /* creation de l'arbre xml en memoire */

  doc = xmlNewDoc("1.0");

	/* la racine est "configuration" */

  doc->children = xmlNewDocNode ( doc,NULL, "Configuration",NULL );

  /*   sauvegarde de la géométrie */
  if ( GTK_WIDGET ( window) -> window ) 
    {
      gtk_window_get_size (GTK_WINDOW ( window ),
			   &largeur_window,&hauteur_window); // gtk2 ???
    } 
  else 
    {
    largeur_window = 0;
    hauteur_window = 0;
  }

  node = xmlNewChild ( doc->children,NULL, "Geometry",NULL );
  xmlNewChild ( node,NULL, "Width",
		itoa(largeur_window));
  xmlNewChild ( node,NULL, "Height",
		itoa(hauteur_window));

  /* sauvegarde de l'onglet général */
  node = xmlNewChild ( doc->children,NULL, "General",NULL );
  xmlNewChild ( node,NULL, "Modification_operations_rapprochees",
		itoa(etat.r_modifiable));
  xmlNewChild ( node,NULL, "Dernier_chemin_de_travail",dernier_chemin_de_travail);
  xmlNewChild ( node,NULL, "Affichage_alerte_permission",
		itoa(etat.alerte_permission));
  xmlNewChild ( node,NULL, "Force_enregistrement",
		itoa(etat.force_enregistrement));
  xmlNewChild ( node,NULL, "Fonction_touche_entree",
		itoa(etat.entree));
  xmlNewChild ( node,NULL, "Affichage_messages_alertes",
		itoa(etat.alerte_mini));
  xmlNewChild ( node,NULL, "Fonte_des_listes",fonte_liste);
  xmlNewChild ( node,NULL, "Fonte_generale",fonte_general);

  /* sauvegarde de l'onglet I/O */
  node = xmlNewChild ( doc->children,NULL, "IO",NULL );
  xmlNewChild ( node,NULL, "Chargement_auto_dernier_fichier",
		itoa(etat.dernier_fichier_auto));
  xmlNewChild ( node,NULL, "Nom_dernier_fichier",nom_fichier_comptes);
  xmlNewChild ( node,NULL, "Enregistrement_automatique",
		itoa(etat.sauvegarde_auto));
  xmlNewChild ( node,NULL, "Enregistrement_au_demarrage",
		itoa(etat.sauvegarde_demarrage));
  xmlNewChild ( node,NULL, "Nb_max_derniers_fichiers_ouverts",
		itoa(nb_max_derniers_fichiers_ouverts));
  xmlNewChild ( node,NULL, "Compression_fichier",
		itoa(compression_fichier));
  xmlNewChild ( node,NULL, "Compression_backup",
		itoa(compression_backup));
  node_1 = xmlNewChild ( node,NULL, "Liste_noms_derniers_fichiers_ouverts",NULL);
  for (i=0;i<nb_derniers_fichiers_ouverts;i++) {
    // ajout des noeuds de la forme fichier1,fichier2,fichier3...
    //sprintf(buff, "fichier%i",i);
    node_2 = xmlNewChild ( node_1,NULL, "fichier",tab_noms_derniers_fichiers_ouverts[i]);
    xmlSetProp ( node_2, "No", itoa (i));
  }

  /* sauvegarde de l'onglet échéances */
  node = xmlNewChild ( doc->children,NULL, "Echeances",NULL );
  xmlNewChild ( node,NULL, "Delai_rappel_echeances",
		itoa(decalage_echeance));

  /* sauvegarde de l'onglet affichage */
  node = xmlNewChild ( doc->children,NULL, "Affichage",NULL );
  xmlNewChild ( node,NULL, "Affichage_formulaire",
		itoa(etat.formulaire_toujours_affiche));
  xmlNewChild ( node,NULL, "Affichage_formulaire_echeancier",
		itoa(etat.formulaire_echeancier_toujours_affiche));
  xmlNewChild ( node,NULL, "Affichage_tous_types",
		itoa(etat.affiche_tous_les_types));
  xmlNewChild ( node,NULL, "Affiche_no_operation",
		itoa(etat.affiche_no_operation));
  xmlNewChild ( node,NULL, "Affiche_date_bancaire",
		itoa(etat.affiche_date_bancaire));
  xmlNewChild ( node,NULL, "Tri_par_date",
		itoa(etat.classement_par_date));
  xmlNewChild ( node,NULL, "Affiche_boutons_valider_annuler",
		itoa(etat.affiche_boutons_valider_annuler));
  xmlNewChild ( node,NULL, "Largeur_auto_colonnes",
		itoa(etat.largeur_auto_colonnes));
  xmlNewChild ( node,NULL, "Caracteristiques_par_compte",
		itoa(etat.retient_affichage_par_compte));
  for ( i=0 ; i<7 ; i++ ) {
    node_2 = xmlNewChild ( node,NULL, "taille_largeur_colonne",
			   itoa(taille_largeur_colonnes[i]));
    xmlSetProp ( node_2, "No", itoa (i));
  }
  xmlNewChild ( node,NULL, "Affichage_nb_ecritures",
		itoa(etat.affichage_exercice_automatique));
  xmlNewChild ( node,NULL, "Affichage_exercice_automatique",
		itoa(etat.affichage_exercice_automatique));

  /*   sauvegarde de l'onglet d'exercice */
  node = xmlNewChild ( doc->children,NULL, "Exercice",NULL );

  /* sauvegarde des messages */
  node = xmlNewChild ( doc->children,NULL, "Messages",NULL );
  xmlNewChild ( node,NULL, "display_message_lock_active",
		itoa(etat.display_message_lock_active));
  xmlNewChild ( node,NULL, "display_message_file_readable",
		itoa(etat.display_message_file_readable));
  xmlNewChild ( node,NULL, "display_message_minimum_alert",
		itoa(etat.display_message_minimum_alert));

  /* Enregistre dans le ~/.grisbirc */
  resultat = xmlSaveFormatFile ( g_strconcat ( getenv ("HOME"), "/.grisbirc",
					       NULL), doc, 1 );

  /* on libère la memoire */
  xmlFreeDoc ( doc );
  if ( resultat == -1 ) 
    {
      dialogue_error ( g_strdup_printf ( _("Error saving file '%s': %s"), 
					 nom_fichier_comptes, 
					 latin2utf8(strerror(errno)) ));
    }
}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction sauve_configuration */
/* Appelée à chaque changement de configuration */
/* ***************************************************************************************************** */
void sauve_configuration (void)
{

	sauve_configurationXML();
	return;
  GSList *pointeur_fichier_a_verifier;
  gint i;
  gchar **tab_pointeurs;

  /*   sauvegarde de la géométrie */

  if ( GTK_WIDGET ( window) -> window )
    {
      gtk_window_get_size ( GTK_WINDOW(window), &largeur_window, &hauteur_window);
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
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Force_enregistrement", NULL ),
			 etat.force_enregistrement );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/General/Fonction_touche_entree", NULL ),
			 etat.entree );
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

  /* Save messages settings */
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Messages/display_message_lock_active", NULL ),
			 etat.display_message_lock_active );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Messages/display_message_file_readable", NULL ),
			 etat.display_message_file_readable );
  gnome_config_set_int ( g_strconcat ( "/", FICHIER_CONF, "/Messages/display_message_minimum_alert", NULL ),
			 etat.display_message_minimum_alert );


  gnome_config_sync();
}
/* ***************************************************************************************************** */
