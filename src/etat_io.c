/*  Fichier qui s'occupe de l'export et de l'import des états */
/*      etats.c */

/*     Copyright (C) 2000-2002  Cédric Auger */
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




/***********************************************************************************************************/
gboolean charge_etat ( gchar *nom_etat )
{
  xmlDocPtr doc;

  doc = xmlParseFile ( nom_etat );

  if ( doc )
    {
      /* vérifications d'usage */

      if ( !doc->root
	   ||
	   !doc->root->name
	   ||
	   g_strcasecmp ( doc->root->name,
			  _("Grisbi_etat") ))
	{
	  dialogue ( _("Ce fichier n'est pas un état Grisbi") );
	  xmlFreeDoc ( doc );
	  return ( FALSE );
	}

      /* récupère la version de fichier */

      if (( !strcmp (  xmlNodeGetContent ( doc->root->childs->childs ),
		       "0.4.0" )))
	return ( charge_etat_version_0_4_0 ( doc ));

      /* 	à ce niveau, c'est que que la version n'est pas connue de grisbi, on donne alors */
      /* la version nécessaire pour l'ouvrir */

      dialogue ( g_strdup_printf ( _("Pour ouvrir ce fichier, il vous faut la version %s de Grisbi "),
				   xmlNodeGetContent ( doc->root->childs->childs->next )));

      xmlFreeDoc ( doc );

      return ( FALSE );
    }
  else
    {
      dialogue ( _("Fichier d'état invalide.") );
      return ( FALSE );
    }
}
/***********************************************************************************************************/



/***********************************************************************************************************/
gboolean charge_etat_version_0_4_0 ( xmlDocPtr doc )
{
  xmlNodePtr node;
  struct struct_etat *etat;
		    
  /*   on met à null le log_message, il sera affiché à la fin en cas de pb */

  log_message = NULL;

  etat = calloc ( 1,
		  sizeof ( struct struct_etat ));

  /* on place node sur les generalites */

  node = doc -> root -> childs;

  while ( node )
    {
      if ( !strcmp ( node -> name,
		     "Generalites" ) )
	{
	  xmlNodePtr node_generalites;

	  /* node_generalites va faire le tour des generalites */

	  node_generalites = node -> childs;

	  while ( node_generalites )
	    {
	      if ( !strcmp ( node_generalites -> name,
			     "Nom" ))
		etat -> nom_etat = xmlNodeGetContent ( node_generalites );


	      node_generalites = node_generalites -> next;
	    }
	}

      /* on recupère ici l'etat */

      if ( !strcmp ( node -> name,
		     "Details" ))
	{
	  xmlNodePtr node_detail_etat;

	  node_detail_etat = node -> childs;

	  while ( node_detail_etat )
	    {
	      if ( !strcmp ( node_detail_etat -> name,
			     "Type_classement" )
		   &&
		   xmlNodeGetContent ( node_detail_etat ))
		{
		  gchar **pointeur_char;
		  gint i;

		  pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
					       "/",
					       0 );
		  i=0;

		  while ( pointeur_char[i] )
		    {
		      etat -> type_classement = g_list_append ( etat -> type_classement,
								GINT_TO_POINTER ( atoi ( pointeur_char[i] )));
		      i++;
		    }
		  g_strfreev ( pointeur_char );
		}


	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_r" ))
		etat -> afficher_r = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_ope" ))
		etat -> afficher_opes = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_nb_ope" ))
		etat -> afficher_nb_opes = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_no_ope" ))
		etat -> afficher_no_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_date_ope" ))
		etat -> afficher_date_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_tiers_ope" ))
		etat -> afficher_tiers_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_categ_ope" ))
		etat -> afficher_categ_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_ss_categ_ope" ))
		etat -> afficher_sous_categ_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_type_ope" ))
		etat -> afficher_type_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_ib_ope" ))
		etat -> afficher_ib_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_ss_ib_ope" ))
		etat ->afficher_sous_ib_ope  = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_cheque_ope" ))
		etat -> afficher_cheque_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_notes_ope" ))
		etat -> afficher_notes_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_pc_ope" ))
		etat -> afficher_pc_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_rappr_ope" ))
		etat -> afficher_rappr_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_infobd_ope" ))
		etat -> afficher_infobd_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_exo_ope" ))
		etat -> afficher_exo_ope = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_titres_col" ))
		etat -> afficher_titre_colonnes = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_titres_chgt" ))
		etat -> type_affichage_titres = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Pas_detail_ventil" ))
		etat -> pas_detailler_ventilation = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Sep_rev_dep" ))
		etat -> separer_revenus_depenses = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Devise_gen" ))
		etat -> devise_de_calcul_general = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Incl_tiers" ))
		etat -> inclure_dans_tiers = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Exo_date" ))
		etat -> exo_date = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Detail_exo" ))
		etat -> utilise_detail_exo = atoi ( xmlNodeGetContent ( node_detail_etat ));
							 
	      if ( !strcmp ( node_detail_etat -> name,
			     "Liste_exo" ))
		{
		  xmlNodePtr node_exo;

		  node_exo = node_detail_etat -> childs;

		  /*  on fait le tour des exos */

		  while ( node_exo )
		    {
		      gint no_exo;

		      no_exo = recupere_exo_par_nom_etat ( xmlGetProp ( node_exo,
									"Nom" ));

		      if ( no_exo )
			etat -> no_exercices = g_slist_append ( etat -> no_exercices,
								GINT_TO_POINTER ( no_exo ));

		      node_exo = node_exo -> next;
		    }
		}

	      if ( !strcmp ( node_detail_etat -> name,
			     "Plage_date" ))
		etat -> no_plage_date = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Date_debut" )
		   &&
		   xmlNodeGetContent ( node_detail_etat ))
		{
		  gchar **pointeur_char;

		  pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
					       "/",
					       3 );

		  etat -> date_perso_debut = g_date_new_dmy ( atoi ( pointeur_char[0] ),
							      atoi ( pointeur_char[1] ),
							      atoi ( pointeur_char[2] ));
		  g_strfreev ( pointeur_char );
		}

	      if ( !strcmp ( node_detail_etat -> name,
			     "Date_fin" )
		   &&
		   xmlNodeGetContent ( node_detail_etat ))
		{
		  gchar **pointeur_char;

		  pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
					       "/",
					       3 );

		  etat -> date_perso_fin = g_date_new_dmy ( atoi ( pointeur_char[0] ),
							    atoi ( pointeur_char[1] ),
							    atoi ( pointeur_char[2] ));
		  g_strfreev ( pointeur_char );
		}

	      if ( !strcmp ( node_detail_etat -> name,
			     "Utilise_plages" ))
		etat -> separation_par_plage = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Sep_plages" ))
		etat -> type_separation_plage = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Deb_sem_plages" ))
		etat -> jour_debut_semaine = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Detail_comptes" ))
		etat -> utilise_detail_comptes = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Liste_comptes" ))
		{
		  xmlNodePtr node_compte;

		  node_compte = node_detail_etat -> childs;

		  /*  on fait le tour des comptes */

		  while ( node_compte )
		    {
		      gint no_compte;

		      no_compte = recupere_compte_par_nom_etat ( xmlGetProp ( node_compte,
									      "Nom" ));

		      if ( no_compte != -1 )
			etat -> no_comptes = g_slist_append ( etat -> no_comptes,
							      GINT_TO_POINTER ( no_compte ));

		      node_compte = node_compte -> next;
		    }
		}


	      if ( !strcmp ( node_detail_etat -> name,
			     "Grp_ope_compte" ))
		etat -> regroupe_ope_par_compte = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Total_compte" ))
		etat -> affiche_sous_total_compte = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_nom_compte" ))
		etat -> afficher_nom_compte = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Type_vir" ))
		etat -> type_virement = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Liste_comptes_vir" ))
		{
		  xmlNodePtr node_compte;

		  node_compte = node_detail_etat -> childs;

		  /*  on fait le tour des comptes */

		  while ( node_compte )
		    {
		      gint no_compte;

		      no_compte = recupere_compte_par_nom_etat ( xmlGetProp ( node_compte,
									      "Nom" ));

		      if ( no_compte != -1 )
			etat -> no_comptes_virements = g_slist_append ( etat -> no_comptes_virements,
									GINT_TO_POINTER ( no_compte ));

		      node_compte = node_compte -> next;
		    }
		}


	      if ( !strcmp ( node_detail_etat -> name,
			     "Exclure_non_vir" ))
		etat -> exclure_ope_non_virement = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Categ" ))
		etat -> utilise_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Detail_categ" ))
		etat -> utilise_detail_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Liste_categ" ))
		{
		  xmlNodePtr node_categ;

		  node_categ = node_detail_etat -> childs;

		  /*  on fait le tour des categ */

		  while ( node_categ )
		    {
		      gint no_categ;

		      no_categ = recupere_categ_par_nom_etat ( xmlGetProp ( node_categ,
									    "Nom" ));

		      if ( no_categ )
			etat -> no_categ = g_slist_append ( etat -> no_categ,
							    GINT_TO_POINTER ( no_categ ));

		      node_categ = node_categ -> next;
		    }
		}


	      if ( !strcmp ( node_detail_etat -> name,
			     "Exclut_categ" ))
		etat -> exclure_ope_sans_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Total_categ" ))
		etat -> affiche_sous_total_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_ss_categ" ))
		etat -> afficher_sous_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_pas_ss_categ" ))
		etat -> afficher_pas_de_sous_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Total_ss_categ" ))
		etat -> affiche_sous_total_sous_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Devise_categ" ))
		etat -> devise_de_calcul_categ = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_nom_categ" ))
		etat -> afficher_nom_categ = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "IB" ))
		etat -> utilise_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Detail_ib" ))
		etat -> utilise_detail_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Liste_ib" ))
		{
		  xmlNodePtr node_ib;

		  node_ib = node_detail_etat -> childs;

		  /*  on fait le tour des ib */

		  while ( node_ib )
		    {
		      gint no_ib;

		      no_ib = recupere_ib_par_nom_etat ( xmlGetProp ( node_ib,
									    "Nom" ));

		      if ( no_ib )
			etat -> no_ib = g_slist_append ( etat -> no_ib,
							    GINT_TO_POINTER ( no_ib ));

		      node_ib = node_ib -> next;
		    }
		}

	      if ( !strcmp ( node_detail_etat -> name,
			     "Exclut_ib" ))
		etat -> exclure_ope_sans_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Total_ib" ))
		etat -> affiche_sous_total_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_ss_ib" ))
		etat -> afficher_sous_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_pas_ss_ib" ))
		etat -> afficher_pas_de_sous_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Total_ss_ib" ))
		etat -> affiche_sous_total_sous_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Devise_ib" ))
		etat -> devise_de_calcul_ib = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_nom_ib" ))
		etat -> afficher_nom_ib = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Tiers" ))
		etat -> utilise_tiers = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Detail_tiers" ))
		etat -> utilise_detail_tiers = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Liste_tiers" ))
		{
		  xmlNodePtr node_tiers;

		  node_tiers = node_detail_etat -> childs;

		  /*  on fait le tour des tiers */

		  while ( node_tiers )
		    {
		      gint no_tiers;

		      no_tiers = recupere_tiers_par_nom_etat ( xmlGetProp ( node_tiers,
									    "Nom" ));

		      if ( no_tiers )
			etat -> no_tiers = g_slist_append ( etat -> no_tiers,
							    GINT_TO_POINTER ( no_tiers ));

		      node_tiers = node_tiers -> next;
		    }
		}


	      if ( !strcmp ( node_detail_etat -> name,
			     "Total_tiers" ))
		etat -> affiche_sous_total_tiers = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Devise_tiers" ))
		etat -> devise_de_calcul_tiers = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Aff_nom_tiers" ))
		etat -> afficher_nom_tiers = atoi ( xmlNodeGetContent ( node_detail_etat ));


	      if ( !strcmp ( node_detail_etat -> name,
			     "Texte" ))
		etat -> utilise_texte = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Texte_comp" ))
		{
		  xmlNodePtr node_comp_textes;

		  node_comp_textes = node_detail_etat -> childs;

		  /*  on fait le tour des comparaisons */

		  while ( node_comp_textes )
		    {
		      struct struct_comparaison_textes_etat *comp_textes;

		      comp_textes = calloc ( 1,
					     sizeof ( struct struct_comparaison_textes_etat ));

		      comp_textes -> lien_struct_precedente = atoi ( xmlGetProp ( node_comp_textes,
										  "Lien_struct" ));
		      comp_textes -> champ = atoi ( xmlGetProp ( node_comp_textes,
								 "Champ" ));
		      comp_textes -> operateur = atoi ( xmlGetProp ( node_comp_textes,
								     "Op" ));
		      comp_textes -> texte = xmlGetProp ( node_comp_textes,
							  "Txt" );
		      comp_textes -> utilise_txt = atoi ( xmlGetProp ( node_comp_textes,
									 "Util_txt" ));
		      comp_textes -> comparateur_1 = atoi ( xmlGetProp ( node_comp_textes,
									 "Comp_1" ));
		      comp_textes -> lien_1_2 = atoi ( xmlGetProp ( node_comp_textes,
								    "Lien_1_2" ));
		      comp_textes -> comparateur_2 = atoi ( xmlGetProp ( node_comp_textes,
									 "Comp_2" ));
		      comp_textes -> montant_1 = atoi ( xmlGetProp ( node_comp_textes,
								     "Mont_1" ));
		      comp_textes -> montant_2 = atoi ( xmlGetProp ( node_comp_textes,
								     "Mont_2" ));

		      /* on a fini de remplir le détail de la comparaison, on l'ajoute à la liste */

		      etat -> liste_struct_comparaison_textes = g_slist_append ( etat -> liste_struct_comparaison_textes,
										 comp_textes );
		      node_comp_textes = node_comp_textes -> next;
		    }
		}

	      if ( !strcmp ( node_detail_etat -> name,
			     "Montant" ))
		etat -> utilise_montant = atoi ( xmlNodeGetContent ( node_detail_etat ));

	      if ( !strcmp ( node_detail_etat -> name,
			     "Montant_devise" ))
		etat -> choix_devise_montant = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));


	      if ( !strcmp ( node_detail_etat -> name,
			     "Montant_comp" ))
		{
		  xmlNodePtr node_comp_montants;

		  node_comp_montants = node_detail_etat -> childs;

		  /*  on fait le tour des comparaisons */

		  while ( node_comp_montants )
		    {
		      struct struct_comparaison_montants_etat *comp_montants;

		      comp_montants = calloc ( 1,
					       sizeof ( struct struct_comparaison_montants_etat ));


		      comp_montants -> lien_struct_precedente = atoi ( xmlGetProp ( node_comp_montants,
										    "Lien_struct" ));
		      comp_montants -> comparateur_1 = atoi ( xmlGetProp ( node_comp_montants,
									   "Comp_1" ));
		      comp_montants -> lien_1_2 = atoi ( xmlGetProp ( node_comp_montants,
								      "Lien_1_2" ));
		      comp_montants -> comparateur_2 = atoi ( xmlGetProp ( node_comp_montants,
									   "Comp_2" ));
		      comp_montants -> montant_1 = g_strtod ( xmlGetProp ( node_comp_montants,
									   "Mont_1" ),
							      NULL );
		      comp_montants -> montant_2 = g_strtod ( xmlGetProp ( node_comp_montants,
									   "Mont_2" ),
							      NULL );

		      /* on a fini de remplir le détail de la comparaison, on l'ajoute à la liste */

		      etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
										   comp_montants );
		      node_comp_montants = node_comp_montants -> next;
		    }
		}

	      node_detail_etat = node_detail_etat -> next;
	    }
	  /* on a fini de remplir l'état, on l'ajoute à la liste */

	  liste_struct_etats = g_slist_append ( liste_struct_etats,
						etat );

	}
      node = node -> next;
    }

  /* on libère la memoire */
  
  xmlFreeDoc ( doc );

  /* on rafraichit la liste des états */
 
  remplissage_liste_etats ();

  gtk_widget_set_sensitive ( bouton_effacer_etat,
			     TRUE );

  modification_fichier ( TRUE );

  /* on affiche le log si nécessaire */

  affiche_log_message ();

  return ( TRUE );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* fonction recupere_devise_par_nom_etat */
/* prend le nom d'une devise en argument et essaie */
/* de retrouver son numéro */
/* si rien n'est trouvé, renvoie le no de la 1ère devise et ajoute un message */
/* dans le log */
/***********************************************************************************************************/

gint recupere_devise_par_nom_etat ( gchar *nom )
{
  GSList *liste_tmp;
  struct struct_devise *devise;

  liste_tmp = g_slist_find_custom ( liste_struct_devises,
				    g_strstrip ( nom ),
				    (GCompareFunc) recherche_devise_par_nom  );

  if ( liste_tmp )
    devise = liste_tmp -> data;
  else
    {
      devise = liste_struct_devises -> data;

      if ( log_message )
	log_message = g_strconcat ( log_message,
				    _("Devise "),
				    nom,
				    _(" non trouvée.\n"),
				    NULL );
      else
	log_message = g_strconcat ( _("Devise "),
				    nom,
				    _(" non trouvée.\n"),
				    NULL );
    }

  return ( devise -> no_devise );
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* fonction recupere_exo_par_nom_etat */
/* prend le nom de l'exo en argument et essaie */
/* de retrouver son numéro */
/* si rien n'est trouvé, renvoie 0 et ajoute un message */
/* dans le log */
/***********************************************************************************************************/

gint recupere_exo_par_nom_etat ( gchar *nom )
{
  gint no_exo;
  GSList *liste_tmp;
  struct struct_exercice *exo;


  liste_tmp = g_slist_find_custom ( liste_struct_exercices,
				    g_strstrip ( nom ),
				    (GCompareFunc) recherche_exercice_par_nom  );

  if ( liste_tmp )
    {
      exo = liste_tmp -> data;
      no_exo = exo -> no_exercice;
    }
  else
    {
      no_exo = 0;

      if ( log_message )
	log_message = g_strconcat ( log_message,
				    _("Exercice "),
				    nom,
				    _(" non trouvé.\n"),
				    NULL );
      else
	log_message = g_strconcat ( _("Exercice "),
				    nom,
				    _(" non trouvé.\n"),
				    NULL );
    }

  return ( no_exo );
}
/***********************************************************************************************************/






/***********************************************************************************************************/
/* fonction recupere_compte_par_nom_etat */
/* prend le nom du compte en argument et essaie */
/* de retrouver son numéro */
/* si rien n'est trouvé, renvoie -1 et ajoute un message */
/* dans le log */
/***********************************************************************************************************/

gint recupere_compte_par_nom_etat ( gchar *nom )
{
  gint no_compte;
  gint i;

  no_compte = -1;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

      if ( !g_strcasecmp ( NOM_DU_COMPTE,
			   nom ))
	{
	  no_compte = i;
	  i=nb_comptes;
	}
    }

  if ( no_compte == -1 )
    {
      if ( log_message )
	log_message = g_strconcat ( log_message,
				    _("Compte "),
				    nom,
				    _(" non trouvé.\n"),
				    NULL );
      else
	log_message = g_strconcat ( _("Compte "),
				    nom,
				    _(" non trouvé.\n"),
				    NULL );
    }

  return ( no_compte );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* fonction recupere_categ_par_nom_etat */
/* prend le nom de la categ en argument et essaie */
/* de retrouver son numéro */
/* si rien n'est trouvé, renvoie 0 et ajoute un message */
/* dans le log */
/***********************************************************************************************************/

gint recupere_categ_par_nom_etat ( gchar *nom )
{
  gint no_categ;
  GSList *liste_tmp;
  struct struct_categ *categ;


  liste_tmp = g_slist_find_custom ( liste_struct_categories,
				    g_strstrip ( nom ),
				    (GCompareFunc) recherche_categorie_par_nom  );

  if ( liste_tmp )
    {
      categ = liste_tmp -> data;
      no_categ = categ -> no_categ;
    }
  else
    {
      no_categ = 0;

      if ( log_message )
	log_message = g_strconcat ( log_message,
				    _("Catégorie "),
				    nom,
				    _(" non trouvée.\n"),
				    NULL );
      else
	log_message = g_strconcat ( _("Catégorie "),
				    nom,
				    _(" non trouvée.\n"),
				    NULL );
    }

  return ( no_categ );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* fonction recupere_ib_par_nom_etat */
/* prend le nom de l'ib en argument et essaie */
/* de retrouver son numéro */
/* si rien n'est trouvé, renvoie 0 et ajoute un message */
/* dans le log */
/***********************************************************************************************************/

gint recupere_ib_par_nom_etat ( gchar *nom )
{
  gint no_ib;
  GSList *liste_tmp;
  struct struct_imputation *imputation;


  liste_tmp = g_slist_find_custom ( liste_struct_imputation,
				    g_strstrip ( nom ),
				    (GCompareFunc) recherche_imputation_par_nom  );

  if ( liste_tmp )
    {
      imputation = liste_tmp -> data;
      no_ib = imputation -> no_imputation;
    }
  else
    {
      no_ib = 0;

      if ( log_message )
	log_message = g_strconcat ( log_message,
				    _("Imputation "),
				    nom,
				    _(" non trouvée.\n"),
				    NULL );
      else
	log_message = g_strconcat ( _("Imputation "),
				    nom,
				    _(" non trouvée.\n"),
				    NULL );
    }

  return ( no_ib );
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* fonction recupere_tiers_par_nom_etat */
/* prend le nom du tiers en argument et essaie */
/* de retrouver son numéro */
/* si rien n'est trouvé, renvoie 0 et ajoute un message */
/* dans le log */
/***********************************************************************************************************/

gint recupere_tiers_par_nom_etat ( gchar *nom )
{
  gint no_tiers;
  GSList *liste_tmp;
  struct struct_tiers *tiers;


  liste_tmp = g_slist_find_custom ( liste_struct_tiers,
				    g_strstrip ( nom ),
				    (GCompareFunc) recherche_tiers_par_nom  );

  if ( liste_tmp )
    {
      tiers = liste_tmp -> data;
      no_tiers = tiers -> no_tiers;
    }
  else
    {
      no_tiers = 0;

      if ( log_message )
	log_message = g_strconcat ( log_message,
				    _("Tiers "),
				    nom,
				    _(" non trouvé.\n"),
				    NULL );
      else
	log_message = g_strconcat ( _("Tiers "),
				    nom,
				    _(" non trouvé.\n"),
				    NULL );
    }

  return ( no_tiers );
}
/***********************************************************************************************************/





/***********************************************************************************************************/
gboolean enregistre_etat ( gchar *nom_etat )
{
  xmlDocPtr doc;
  xmlNodePtr node;
  xmlNodePtr node_2;
  gint resultat;
  gchar *pointeur_char;
  GSList *pointeur_liste;
  GList *pointeur_list;

  /* creation de l'arbre xml en memoire */

  doc = xmlNewDoc("1.0");

  /* la racine est grisbi */

  doc->root = xmlNewDocNode ( doc,
			      NULL,
			      "Grisbi_etat",
			      NULL );

  /* on commence à ajouter les generalites */

  node = xmlNewChild ( doc->root,
		       NULL,
		       "Generalites",
		       NULL );
  xmlNewTextChild ( node,
		    NULL,
		    "Version_fichier_etat",
		    VERSION_FICHIER_ETAT );

  xmlNewTextChild ( node,
		    NULL,
		    "Version_grisbi",
		    VERSION );

  xmlNewTextChild ( node,
		    NULL,
		    "Nom",
		    etat_courant -> nom_etat );


  node = xmlNewChild ( doc->root,
		       NULL,
		       "Details",
		       NULL );

  pointeur_list = etat_courant -> type_classement;
  pointeur_char = NULL;

  while ( pointeur_list )
    {
      if ( pointeur_char )
	pointeur_char = g_strconcat ( pointeur_char,
				      "/",
				      itoa ( GPOINTER_TO_INT ( pointeur_list -> data )),
				      NULL );
      else
	pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_list -> data ));

      pointeur_list = pointeur_list -> next;
    }

  xmlNewTextChild ( node,
		    NULL,
	       "Type_classement",
	       pointeur_char );


  xmlNewTextChild ( node,
		    NULL,
	       "Aff_r",
	       itoa ( etat_courant -> afficher_r ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_ope",
	       itoa ( etat_courant -> afficher_opes ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_nb_ope",
	       itoa ( etat_courant -> afficher_nb_opes ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_no_ope",
	       itoa ( etat_courant -> afficher_no_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_date_ope",
	       itoa ( etat_courant -> afficher_date_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_tiers_ope",
	       itoa ( etat_courant -> afficher_tiers_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_categ_ope",
	       itoa ( etat_courant -> afficher_categ_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_ss_categ_ope",
	       itoa ( etat_courant -> afficher_sous_categ_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_type_ope",
	       itoa ( etat_courant -> afficher_type_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_ib_ope",
	       itoa ( etat_courant -> afficher_ib_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_ss_ib_ope",
	       itoa ( etat_courant -> afficher_sous_ib_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_cheque_ope",
	       itoa ( etat_courant -> afficher_cheque_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_notes_ope",
	       itoa ( etat_courant -> afficher_notes_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_pc_ope",
	       itoa ( etat_courant -> afficher_pc_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_rappr_ope",
	       itoa ( etat_courant -> afficher_rappr_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_infobd_ope",
	       itoa ( etat_courant -> afficher_infobd_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_exo_ope",
	       itoa ( etat_courant -> afficher_exo_ope ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_titres_col",
	       itoa ( etat_courant -> afficher_titre_colonnes ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_titres_chgt",
	       itoa ( etat_courant -> type_affichage_titres ));

  xmlNewTextChild ( node,
		    NULL,
	       "Pas_detail_ventil",
	       itoa ( etat_courant -> pas_detailler_ventilation ));

  xmlNewTextChild ( node,
		    NULL,
	       "Sep_rev_dep",
	       itoa ( etat_courant -> separer_revenus_depenses ));

  xmlNewTextChild ( node,
		    NULL,
		    "Devise_gen",
		    ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
								       GINT_TO_POINTER ( etat_courant -> devise_de_calcul_general ),
								       (GCompareFunc) recherche_devise_par_no ) -> data ))->nom_devise);

  xmlNewTextChild ( node,
		    NULL,
	       "Incl_tiers",
	       itoa ( etat_courant -> inclure_dans_tiers ));


  xmlNewTextChild ( node,
		    NULL,
		    "Exo_date",
		    itoa ( etat_courant -> exo_date ));

  xmlNewTextChild ( node,
		    NULL,
		    "Detail_exo",
		    itoa ( etat_courant -> utilise_detail_exo ));

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Liste_exo",
			 NULL );

  pointeur_liste = etat_courant -> no_exercices;
  pointeur_char = NULL;

  while ( pointeur_liste )
    {
      xmlNodePtr node_3;
      struct struct_exercice *exo;

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Exo",
			     NULL );

      exo = g_slist_find_custom ( liste_struct_exercices,
				  pointeur_liste -> data,
				  (GCompareFunc) recherche_exercice_par_no ) -> data;

      xmlSetProp ( node_3,
		   "Nom",
		   exo -> nom_exercice );


      pointeur_liste = pointeur_liste -> next;
    }

  xmlNewTextChild ( node,
		    NULL,
	       "Plage_date",
	       itoa ( etat_courant -> no_plage_date ));


  if ( etat_courant->date_perso_debut )
    xmlNewTextChild ( node,
		    NULL,
		 "Date_debut",
		 g_strdup_printf ( "%d/%d/%d",
				   g_date_day ( etat_courant->date_perso_debut ),
				   g_date_month ( etat_courant->date_perso_debut ),
				   g_date_year ( etat_courant->date_perso_debut )));
  else
    xmlNewTextChild ( node,
		    NULL,
		 "Date_debut",
		 NULL );

  if ( etat_courant->date_perso_fin )
    xmlNewTextChild ( node,
		    NULL,
		 "Date_fin",
		 g_strdup_printf ( "%d/%d/%d",
				   g_date_day ( etat_courant->date_perso_fin ),
				   g_date_month ( etat_courant->date_perso_fin ),
				   g_date_year ( etat_courant->date_perso_fin )));
  else
    xmlNewTextChild ( node,
		    NULL,
		 "Date_fin",
		 NULL );

  xmlNewTextChild ( node,
		    NULL,
	       "Utilise_plages",
	       itoa ( etat_courant -> separation_par_plage ));

  xmlNewTextChild ( node,
		    NULL,
	       "Sep_plages",
	       itoa ( etat_courant -> type_separation_plage ));

  xmlNewTextChild ( node,
		    NULL,
	       "Deb_sem_plages",
	       itoa ( etat_courant -> jour_debut_semaine ));

  xmlNewTextChild ( node,
		    NULL,
	       "Detail_comptes",
	       itoa ( etat_courant -> utilise_detail_comptes ));

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Liste_comptes",
			 NULL );

  pointeur_liste = etat_courant -> no_comptes;
  pointeur_char = NULL;

  while ( pointeur_liste )
    {
      xmlNodePtr node_3;
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( pointeur_liste -> data );

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Compte",
			     NULL );

      xmlSetProp ( node_3,
		   "Nom",
		   NOM_DU_COMPTE );


      pointeur_liste = pointeur_liste -> next;
    }


  xmlNewTextChild ( node,
		    NULL,
	       "Grp_ope_compte",
	       itoa ( etat_courant -> regroupe_ope_par_compte ));

  xmlNewTextChild ( node,
		    NULL,
	       "Total_compte",
	       itoa ( etat_courant -> affiche_sous_total_compte ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_nom_compte",
	       itoa ( etat_courant -> afficher_nom_compte ));

  xmlNewTextChild ( node,
		    NULL,
	       "Type_vir",
	       itoa ( etat_courant -> type_virement ));

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Liste_comptes_vir",
			 NULL );

  pointeur_liste = etat_courant -> no_comptes_virements;
  pointeur_char = NULL;

  while ( pointeur_liste )
    {
      xmlNodePtr node_3;
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( pointeur_liste -> data );

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Compte",
			     NULL );

      xmlSetProp ( node_3,
		   "Nom",
		   NOM_DU_COMPTE );


      pointeur_liste = pointeur_liste -> next;
    }



  xmlNewTextChild ( node,
		    NULL,
	       "Exclure_non_vir",
	       itoa ( etat_courant -> exclure_ope_non_virement ));

  xmlNewTextChild ( node,
		    NULL,
	       "Categ",
	       itoa ( etat_courant -> utilise_categ ));

  xmlNewTextChild ( node,
		    NULL,
	       "Detail_categ",
	       itoa ( etat_courant -> utilise_detail_categ ));

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Liste_categ",
			 NULL );

  pointeur_liste = etat_courant -> no_categ;
  pointeur_char = NULL;

  while ( pointeur_liste )
    {
      xmlNodePtr node_3;
      struct struct_categ *categ;

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Categ",
			     NULL );

      categ = g_slist_find_custom ( liste_struct_categories,
				    pointeur_liste -> data,
				    (GCompareFunc) recherche_categorie_par_no ) -> data;

      xmlSetProp ( node_3,
		   "Nom",
		   categ -> nom_categ );

      pointeur_liste = pointeur_liste -> next;
    }

  xmlNewTextChild ( node,
		    NULL,
	       "Exclut_categ",
	       itoa ( etat_courant -> exclure_ope_sans_categ ));

  xmlNewTextChild ( node,
		    NULL,
	       "Total_categ",
	       itoa ( etat_courant -> affiche_sous_total_categ ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_ss_categ",
	       itoa ( etat_courant -> afficher_sous_categ ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_pas_ss_categ",
	       itoa ( etat_courant -> afficher_pas_de_sous_categ ));

  xmlNewTextChild ( node,
		    NULL,
	       "Total_ss_categ",
	       itoa ( etat_courant -> affiche_sous_total_sous_categ ));

  xmlNewTextChild ( node,
		    NULL,
		    "Devise_categ",
		    ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
								       GINT_TO_POINTER ( etat_courant -> devise_de_calcul_categ ),
								       (GCompareFunc) recherche_devise_par_no ) -> data ))->nom_devise);

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_nom_categ",
	       itoa ( etat_courant -> afficher_nom_categ ));



  xmlNewTextChild ( node,
		    NULL,
	       "IB",
	       itoa ( etat_courant -> utilise_ib ));

  xmlNewTextChild ( node,
		    NULL,
	       "Detail_ib",
	       itoa ( etat_courant -> utilise_detail_ib ));

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Liste_ib",
			 NULL );

  pointeur_liste = etat_courant -> no_ib;
  pointeur_char = NULL;

  while ( pointeur_liste )
    {
      xmlNodePtr node_3;
      struct struct_imputation *ib;

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Ib",
			     NULL );

      ib = g_slist_find_custom ( liste_struct_imputation,
				    pointeur_liste -> data,
				    (GCompareFunc) recherche_imputation_par_no ) -> data;

      xmlSetProp ( node_3,
		   "Nom",
		   ib -> nom_imputation );

      pointeur_liste = pointeur_liste -> next;
    }


  xmlNewTextChild ( node,
		    NULL,
	       "Exclut_ib",
	       itoa ( etat_courant -> exclure_ope_sans_ib ));

  xmlNewTextChild ( node,
		    NULL,
	       "Total_ib",
	       itoa ( etat_courant -> affiche_sous_total_ib ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_ss_ib",
	       itoa ( etat_courant -> afficher_sous_ib ));

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_pas_ss_ib",
	       itoa ( etat_courant -> afficher_pas_de_sous_ib ));

  xmlNewTextChild ( node,
		    NULL,
	       "Total_ss_ib",
	       itoa ( etat_courant -> affiche_sous_total_sous_ib ));

  xmlNewTextChild ( node,
		    NULL,
		    "Devise_ib",
		    ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
								       GINT_TO_POINTER ( etat_courant -> devise_de_calcul_ib ),
								       (GCompareFunc) recherche_devise_par_no ) -> data ))->nom_devise);

  xmlNewTextChild ( node,
		    NULL,
	       "Aff_nom_ib",
	       itoa ( etat_courant -> afficher_nom_ib ));

  xmlNewTextChild ( node,
		    NULL,
	       "Tiers",
	       itoa ( etat_courant -> utilise_tiers ));

  xmlNewTextChild ( node,
		    NULL,
	       "Detail_tiers",
	       itoa ( etat_courant -> utilise_detail_tiers ));

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Liste_tiers",
			 NULL );

  pointeur_liste = etat_courant -> no_tiers;
  pointeur_char = NULL;

  while ( pointeur_liste )
    {
      xmlNodePtr node_3;
      struct struct_tiers *tiers;

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Tiers",
			     NULL );

      tiers = g_slist_find_custom ( liste_struct_tiers,
				    pointeur_liste -> data,
				    (GCompareFunc) recherche_tiers_par_no ) -> data;

      xmlSetProp ( node_3,
		   "Nom",
		   tiers -> nom_tiers );

      pointeur_liste = pointeur_liste -> next;
    }


  xmlNewTextChild ( node,
		    NULL,
	       "Total_tiers",
	       itoa ( etat_courant -> affiche_sous_total_tiers ));

  xmlNewTextChild ( node,
		    NULL,
		    "Devise_tiers",
		    ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
								  GINT_TO_POINTER ( etat_courant -> devise_de_calcul_tiers ),
								  (GCompareFunc) recherche_devise_par_no ) -> data ))->nom_devise);
  xmlNewTextChild ( node,
		    NULL,
		    "Aff_nom_tiers",
		    itoa ( etat_courant -> afficher_nom_tiers ));

  xmlNewTextChild ( node,
		    NULL,
		    "Texte",
		    itoa ( etat_courant -> utilise_texte ));

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Texte_comp",
			 NULL );

  pointeur_liste = etat_courant -> liste_struct_comparaison_textes;

  while ( pointeur_liste )
    {
      struct struct_comparaison_textes_etat *textes_comp;
      xmlNodePtr node_3;

      textes_comp = pointeur_liste -> data;

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Comp",
			     NULL );

      xmlSetProp ( node_3,
		   "Lien_struct",
		   itoa ( textes_comp -> lien_struct_precedente ));
      xmlSetProp ( node_3,
		   "Champ",
		   itoa ( textes_comp -> champ ));
      xmlSetProp ( node_3,
		   "Op",
		   itoa ( textes_comp -> operateur ));
      xmlSetProp ( node_3,
		   "Txt",
		   textes_comp -> texte );
      xmlSetProp ( node_3,
		   "Util_txt",
		   itoa ( textes_comp -> utilise_txt ));
      xmlSetProp ( node_3,
		   "Comp_1",
		   itoa ( textes_comp -> comparateur_1 ));
      xmlSetProp ( node_3,
		   "Lien_1_2",
		   itoa ( textes_comp -> lien_1_2 ));
      xmlSetProp ( node_3,
		   "Comp_2",
		   itoa ( textes_comp -> comparateur_2 ));
      xmlSetProp ( node_3,
		   "Mont_1",
		   itoa ( textes_comp -> montant_1 ));
      xmlSetProp ( node_3,
		   "Mont_2",
		   itoa ( textes_comp -> montant_2 ));
      pointeur_liste = pointeur_liste -> next;
    }


  xmlNewTextChild ( node,
		    NULL,
	       "Montant",
	       itoa ( etat_courant -> utilise_montant ));
  xmlNewTextChild ( node,
		    NULL,
		    "Montant_devise",
		    ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
								       GINT_TO_POINTER ( etat_courant -> choix_devise_montant ),
								       (GCompareFunc) recherche_devise_par_no ) -> data ))->nom_devise);

  node_2 = xmlNewChild ( node,
			 NULL,
			 "Montant_comp",
			 NULL );

  pointeur_liste = etat_courant -> liste_struct_comparaison_montants;

  while ( pointeur_liste )
    {
      struct struct_comparaison_montants_etat *montants_comp;
      xmlNodePtr node_3;

      montants_comp = pointeur_liste -> data;

      node_3 = xmlNewChild ( node_2,
			     NULL,
			     "Comp",
			     NULL );
      xmlSetProp ( node_3,
		   "Lien_struct",
		   itoa ( montants_comp -> lien_struct_precedente ));
      xmlSetProp ( node_3,
		   "Comp_1",
		   itoa ( montants_comp -> comparateur_1 ));
      xmlSetProp ( node_3,
		   "Lien_1_2",
		   itoa ( montants_comp -> lien_1_2 ));
      xmlSetProp ( node_3,
		   "Comp_2",
		   itoa ( montants_comp -> comparateur_2 ));
      xmlSetProp ( node_3,
		   "Mont_1",
		   g_strdup_printf ( "%4.2f",
				     montants_comp -> montant_1 ));
      xmlSetProp ( node_3,
		   "Mont_2",
		   g_strdup_printf ( "%4.2f",
				     montants_comp -> montant_2 ));
      pointeur_liste = pointeur_liste -> next;
    }


  /* l'arbre est fait, on sauvegarde */

  resultat = xmlSaveFile ( nom_etat,
			   doc );

  /* on libère la memoire */

  xmlFreeDoc ( doc );


  if ( resultat == -1 )
    {
      dialogue ( g_strconcat ( _("Erreur dans l'enregistrement du fichier :\n\n"),
			       nom_etat,
			       _("\n\nErreur :\n"),
			       strerror ( errno ),
			       NULL ));
      return ( FALSE );
    }


  return ( TRUE );
}
/***********************************************************************************************************/

