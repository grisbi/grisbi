/*  Fichier qui s'occupe de l'export et de l'import des états */
/*      etats.c */

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
#include "etat_io.h"
#include "erreur.h"
#include "utils_categories.h"
#include "utils_devises.h"
#include "dialog.h"
#include "utils_exercices.h"
#include "gsb_account.h"
#include "utils_ib.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils_tiers.h"
#include "utils_files.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean charge_etat_version_0_4_0 ( xmlDocPtr doc );
static gint recupere_categ_par_nom_etat ( gchar *nom );
static gint recupere_compte_par_nom_etat ( gchar *nom );
static gint recupere_devise_par_nom_etat ( gchar *nom );
static gint recupere_exo_par_nom_etat ( gchar *nom );
static gint recupere_ib_par_nom_etat ( gchar *nom );
static gint recupere_tiers_par_nom_etat ( gchar *nom );
/*END_STATIC*/


gchar *log_message;        /* utilisé comme log lors de l'import d'un état */


/*START_EXTERN*/
extern GtkWidget *bouton_effacer_etat;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_etats;
/*END_EXTERN*/


/***********************************************************************************************************/
gboolean charge_etat ( gchar *nom_etat )
{
    xmlDocPtr doc;

    doc = utf8_xmlParseFile ( nom_etat );

    if ( doc )
    {
	/* vérifications d'usage */
	xmlNodePtr root = xmlDocGetRootElement(doc);

	if ( !root
	     ||
	     !root->name
	     ||
	     g_strcasecmp ( root->name,
			    "Grisbi_etat" ))
	{
	    dialogue_error_hint ( _("Grisbi is unable to parse this file as a report file.  Be sure it is valid."), 
				  _("Invalid Grisbi report file") );
	    xmlFreeDoc ( doc );
	    return ( FALSE );
	}

	/* récupère la version de fichier */

	if (( strcmp (  xmlNodeGetContent ( root->children->next->children->next ), VERSION )))
	{
	    dialogue_warning_hint ( g_strdup_printf (_("This report has been produced with grisbi version %s, Grisbi will nevertheless try to import it."), xmlNodeGetContent ( root->children->next->children->next )),
				    _("Version mismatch") );
	}
	return ( charge_etat_version_0_4_0 ( doc ));
    }
    else
    {
	dialogue_error_hint ( _("Grisbi is unable to parse this file as a report file.  Be sure it is valid."), 
			      _("Invalid Grisbi report file") );
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

    node = doc -> children -> children;

    while ( node )
    {
	if ( !strcmp ( node -> name,
		       "Generalites" ) )
	{
	    xmlNodePtr node_generalites;

	    /* node_generalites va faire le tour des generalites */

	    node_generalites = node -> children;

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

	    node_detail_etat = node -> children;

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
								  GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
			i++;
		    }
		    g_strfreev ( pointeur_char );
		}


		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_r" ))
		    etat -> afficher_r = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_ope" ))
		    etat -> afficher_opes = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_nb_ope" ))
		    etat -> afficher_nb_opes = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_no_ope" ))
		    etat -> afficher_no_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_date_ope" ))
		    etat -> afficher_date_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_tiers_ope" ))
		    etat -> afficher_tiers_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_categ_ope" ))
		    etat -> afficher_categ_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_ss_categ_ope" ))
		    etat -> afficher_sous_categ_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_type_ope" ))
		    etat -> afficher_type_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_ib_ope" ))
		    etat -> afficher_ib_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_ss_ib_ope" ))
		    etat ->afficher_sous_ib_ope  = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_cheque_ope" ))
		    etat -> afficher_cheque_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_notes_ope" ))
		    etat -> afficher_notes_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_pc_ope" ))
		    etat -> afficher_pc_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_rappr_ope" ))
		    etat -> afficher_rappr_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_infobd_ope" ))
		    etat -> afficher_infobd_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_exo_ope" ))
		    etat -> afficher_exo_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Class_ope" ))
		    etat -> type_classement_ope = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_titres_col" ))
		    etat -> afficher_titre_colonnes = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_titres_chgt" ))
		    etat -> type_affichage_titres = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Pas_detail_ventil" ))
		    etat -> pas_detailler_ventilation = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Sep_rev_dep" ))
		    etat -> separer_revenus_depenses = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Devise_gen" ))
		    etat -> devise_de_calcul_general = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Incl_tiers" ))
		    etat -> inclure_dans_tiers = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Ope_click" ))
		    etat -> ope_clickables = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Exo_date" ))
		    etat -> exo_date = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Detail_exo" ))
		    etat -> utilise_detail_exo = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Liste_exo" ))
		{
		    xmlNodePtr node_exo;

		    node_exo = node_detail_etat -> children;

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
		    etat -> no_plage_date = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Date_debut" )
		     &&
		     xmlNodeGetContent ( node_detail_etat ))
		{
		    if ( xmlNodeGetContent (node_detail_etat) &&  
			 strlen (xmlNodeGetContent (node_detail_etat)) )
		      {
			gchar **pointeur_char;
		    
			pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ), "/", 3 );
			etat -> date_perso_debut = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
								    utils_str_atoi ( pointeur_char[1] ),
								    utils_str_atoi ( pointeur_char[2] ));
			g_strfreev ( pointeur_char );
		      }
		}

		if ( !strcmp ( node_detail_etat -> name,
			       "Date_fin" )
		     &&
		     xmlNodeGetContent ( node_detail_etat ))
		{
		    if ( xmlNodeGetContent (node_detail_etat) &&  
			 strlen (xmlNodeGetContent (node_detail_etat)) )
		      {
			gchar **pointeur_char;
			
			pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ), "/", 3 ); 
			etat -> date_perso_fin = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
								  utils_str_atoi ( pointeur_char[1] ),
								  utils_str_atoi ( pointeur_char[2] ));
			g_strfreev ( pointeur_char );
		      }
		}

		if ( !strcmp ( node_detail_etat -> name,
			       "Utilise_plages" ))
		    etat -> separation_par_plage = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Sep_plages" ))
		    etat -> type_separation_plage = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Deb_sem_plages" ))
		    etat -> jour_debut_semaine = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Detail_comptes" ))
		    etat -> utilise_detail_comptes = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Liste_comptes" ))
		{
		    xmlNodePtr node_compte;

		    node_compte = node_detail_etat -> children;

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
		    etat -> regroupe_ope_par_compte = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Total_compte" ))
		    etat -> affiche_sous_total_compte = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_nom_compte" ))
		    etat -> afficher_nom_compte = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Type_vir" ))
		    etat -> type_virement = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Liste_comptes_vir" ))
		{
		    xmlNodePtr node_compte;

		    node_compte = node_detail_etat -> children;

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
		    etat -> exclure_ope_non_virement = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Categ" ))
		    etat -> utilise_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Detail_categ" ))
		    etat -> utilise_detail_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Liste_categ" ))
		{
		    xmlNodePtr node_categ;

		    node_categ = node_detail_etat -> children;

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
		    etat -> exclure_ope_sans_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Total_categ" ))
		    etat -> affiche_sous_total_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_ss_categ" ))
		    etat -> afficher_sous_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_pas_ss_categ" ))
		    etat -> afficher_pas_de_sous_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Total_ss_categ" ))
		    etat -> affiche_sous_total_sous_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Devise_categ" ))
		    etat -> devise_de_calcul_categ = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_nom_categ" ))
		    etat -> afficher_nom_categ = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "IB" ))
		    etat -> utilise_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Detail_ib" ))
		    etat -> utilise_detail_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Liste_ib" ))
		{
		    xmlNodePtr node_ib;

		    node_ib = node_detail_etat -> children;

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
		    etat -> exclure_ope_sans_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Total_ib" ))
		    etat -> affiche_sous_total_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_ss_ib" ))
		    etat -> afficher_sous_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_pas_ss_ib" ))
		    etat -> afficher_pas_de_sous_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Total_ss_ib" ))
		    etat -> affiche_sous_total_sous_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Devise_ib" ))
		    etat -> devise_de_calcul_ib = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_nom_ib" ))
		    etat -> afficher_nom_ib = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Tiers" ))
		    etat -> utilise_tiers = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Detail_tiers" ))
		    etat -> utilise_detail_tiers = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Liste_tiers" ))
		{
		    xmlNodePtr node_tiers;

		    node_tiers = node_detail_etat -> children;

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
		    etat -> affiche_sous_total_tiers = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Devise_tiers" ))
		    etat -> devise_de_calcul_tiers = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Aff_nom_tiers" ))
		    etat -> afficher_nom_tiers = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));


		if ( !strcmp ( node_detail_etat -> name,
			       "Texte" ))
		    etat -> utilise_texte = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Texte_comp" ))
		{
		    xmlNodePtr node_comp_textes;

		    node_comp_textes = node_detail_etat -> children;

		    /*  on fait le tour des comparaisons */

		    while ( node_comp_textes )
		    {
			struct struct_comparaison_textes_etat *comp_textes;

			comp_textes = calloc ( 1,
					       sizeof ( struct struct_comparaison_textes_etat ));

			comp_textes -> lien_struct_precedente = utils_str_atoi ( xmlGetProp ( node_comp_textes,
										    "Lien_struct" ));
			comp_textes -> champ = utils_str_atoi ( xmlGetProp ( node_comp_textes,
								   "Champ" ));
			comp_textes -> operateur = utils_str_atoi ( xmlGetProp ( node_comp_textes,
								       "Op" ));
			comp_textes -> texte = xmlGetProp ( node_comp_textes,
							    "Txt" );
			comp_textes -> utilise_txt = utils_str_atoi ( xmlGetProp ( node_comp_textes,
									 "Util_txt" ));
			comp_textes -> comparateur_1 = utils_str_atoi ( xmlGetProp ( node_comp_textes,
									   "Comp_1" ));
			comp_textes -> lien_1_2 = utils_str_atoi ( xmlGetProp ( node_comp_textes,
								      "Lien_1_2" ));
			comp_textes -> comparateur_2 = utils_str_atoi ( xmlGetProp ( node_comp_textes,
									   "Comp_2" ));
			comp_textes -> montant_1 = utils_str_atoi ( xmlGetProp ( node_comp_textes,
								       "Mont_1" ));
			comp_textes -> montant_2 = utils_str_atoi ( xmlGetProp ( node_comp_textes,
								       "Mont_2" ));

			/* on a fini de remplir le détail de la comparaison, on l'ajoute à la liste */

			etat -> liste_struct_comparaison_textes = g_slist_append ( etat -> liste_struct_comparaison_textes,
										   comp_textes );
			node_comp_textes = node_comp_textes -> next;
		    }
		}

		if ( !strcmp ( node_detail_etat -> name,
			       "Montant" ))
		    etat -> utilise_montant = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Montant_devise" ))
		    etat -> choix_devise_montant = recupere_devise_par_nom_etat ( xmlNodeGetContent ( node_detail_etat ));


		if ( !strcmp ( node_detail_etat -> name,
			       "Montant_comp" ))
		{
		    xmlNodePtr node_comp_montants;

		    node_comp_montants = node_detail_etat -> children;

		    /*  on fait le tour des comparaisons */

		    while ( node_comp_montants )
		    {
			struct struct_comparaison_montants_etat *comp_montants;

			comp_montants = calloc ( 1,
						 sizeof ( struct struct_comparaison_montants_etat ));


			comp_montants -> lien_struct_precedente = utils_str_atoi ( xmlGetProp ( node_comp_montants,
										      "Lien_struct" ));
			comp_montants -> comparateur_1 = utils_str_atoi ( xmlGetProp ( node_comp_montants,
									     "Comp_1" ));
			comp_montants -> lien_1_2 = utils_str_atoi ( xmlGetProp ( node_comp_montants,
									"Lien_1_2" ));
			comp_montants -> comparateur_2 = utils_str_atoi ( xmlGetProp ( node_comp_montants,
									     "Comp_2" ));
			comp_montants -> montant_1 = my_strtod ( xmlGetProp ( node_comp_montants,
									      "Mont_1" ),
								 NULL );
			comp_montants -> montant_2 = my_strtod ( xmlGetProp ( node_comp_montants,
									      "Mont_2" ),
								 NULL );

			/* on a fini de remplir le détail de la comparaison, on l'ajoute à la liste */

			etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
										     comp_montants );
			node_comp_montants = node_comp_montants -> next;
		    }
		}

		if ( !strcmp ( node_detail_etat -> name,
			       "Excl_nul" ))
		    etat -> exclure_montants_nuls = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Detail_mod_paie" ))
		    etat -> utilise_mode_paiement = utils_str_atoi ( xmlNodeGetContent ( node_detail_etat ));

		if ( !strcmp ( node_detail_etat -> name,
			       "Liste_mod_paie" ))
		{
		    xmlNodePtr node_mode_paiement;

		    node_mode_paiement = node_detail_etat -> children;

		    /*  on fait le tour des modes de paiement */

		    while ( node_mode_paiement )
		    {
			etat -> noms_modes_paiement = g_slist_append ( etat -> noms_modes_paiement,
								       xmlGetProp ( node_mode_paiement,
										    "Nom" ));
			node_mode_paiement = node_mode_paiement -> next;
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

    /* TODO, update with navigation list */
/*     remplissage_liste_etats (); */

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
    struct struct_devise *devise;

    devise = devise_par_nom ( nom );

    if ( !devise )
    {
	devise = liste_struct_devises -> data;

	if ( log_message )
	    log_message = g_strdup_printf ( _("%sCurrency %s not found.\n"),
					    log_message, nom );
	else
	    log_message = g_strdup_printf ( _("Currency %s not found.\n"),
					    nom );
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
    struct struct_exercice *exo;

    exo = exercice_par_nom ( nom );

    if ( exo )
	no_exo = exo -> no_exercice;
    else
    {
	no_exo = 0;

	if ( log_message )
	    log_message = g_strdup_printf ( _("%sFinancial year %s not found.\n"),
					    log_message, nom );
	else
	    log_message = g_strdup_printf ( _("Financial year %s not found.\n"),
					    nom );
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

    no_compte = gsb_account_get_no_account_by_name ( nom );

    if ( no_compte == -1 )
    {
	if ( log_message )
	    log_message = g_strdup_printf ( _("%sAccount %s not found.\n"),
					    log_message, nom );
	else
	    log_message = g_strdup_printf ( _("Account %s not found.\n"),
					    nom );
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
    struct struct_categ *categ;


    categ = categ_par_nom ( nom,
			    0,
			    0,
			    0 );

    if ( categ )
	no_categ = categ -> no_categ;
    else
    {
	no_categ = 0;

	if ( log_message )
	    log_message = g_strdup_printf ( _("%sCategory %s not found.\n"),
					    log_message, nom );
	else
	    log_message = g_strdup_printf ( _("Category %s not found.\n"),
					    nom );
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
    struct struct_imputation *imputation;

    imputation = imputation_par_nom ( nom,
				      0,
				      0,
				      0 );

    if ( imputation )
	no_ib = imputation -> no_imputation;
    else
    {
	no_ib = 0;

	if ( log_message )
	    log_message = g_strdup_printf ( _("%sBudget line %s not found.\n"),
					    log_message, nom );
	else
	    log_message = g_strdup_printf ( _("Budget line %s not found.\n"),
					    nom );
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
    struct struct_tiers *tiers;

    tiers = tiers_par_nom ( nom,
			    0 );

    if ( tiers )
	no_tiers = tiers -> no_tiers;
    else
    {
	no_tiers = 0;

	if ( log_message )
	    log_message = g_strdup_printf ( _("%sPayee %s not found.\n"),
					    log_message, nom );
	else
	    log_message = g_strdup_printf ( _("Payee %s not found.\n"),
					    nom );
    }

    return ( no_tiers );
}
/***********************************************************************************************************/





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
