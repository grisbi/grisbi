/*  Fichier qui s'occupe du calcul des états */
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



#include "etats_gtktable.h"
#include "etats_gnomeprint.h"


/*****************************************************************************************************/
void affichage_etat ( struct struct_etat *etat, 
		      struct struct_etat_affichage * affichage )
{
  GSList *liste_opes_selectionnees;
  gint i;
  gint no_exercice_recherche;
  struct struct_devise *devise_montant;
  struct struct_devise *devise_operation;


  if ( !etat )
    {
      if ( etat_courant )
	etat = etat_courant;
      else
	return;
    }

  if ( !affichage )
    {
      affichage = &gtktable_affichage;
    }

  /* si on utilise l'exercice courant ou précédent, on cherche ici */
  /* le numéro de l'exercice correspondant */

  no_exercice_recherche = 0;

  if ( etat -> exo_date )
    {
      GSList *liste_tmp;
      GDate *date_jour;
      struct struct_exercice *exo;
      struct struct_exercice *exo_courant;
      struct struct_exercice *exo_precedent;

      liste_tmp = liste_struct_exercices;
      date_jour = g_date_new ();
      g_date_set_time ( date_jour,
			time ( NULL ));


      exo_courant = NULL;
      exo_precedent = NULL;

      switch ( etat -> utilise_detail_exo )
	{
	case 1:
	  /* on recherche l'exo courant */

	  while ( liste_tmp )
	    {
	      exo = liste_tmp -> data;

	      if ( g_date_compare ( date_jour,
				    exo -> date_debut ) >= 0
		   &&
		   g_date_compare ( date_jour,
				    exo -> date_fin ) <= 0 )
		{
		  no_exercice_recherche = exo -> no_exercice;
		  liste_tmp = NULL;
		}
	      else
		liste_tmp = liste_tmp -> next;
	    }
	  break;

	case 2:
	  /* on recherche l'exo précédent */

	  while ( liste_tmp )
	    {
	      struct struct_exercice *exo;

	      exo = liste_tmp -> data;

	      if ( exo_courant )
		{
		  /* exo_courant est forcemment après exo_precedent */
		  /* si l'exo en court est après exo_courant, on met exo_courant */
		  /* dans exo_precedent, et l'exo en court dans exo_courant */
		  /*   sinon, on compare avec exo_precedent */

		  if ( g_date_compare ( exo -> date_debut,
					exo_courant -> date_fin ) >= 0 )
		    {
		      exo_precedent = exo_courant;
		      exo_courant = exo;
		    }
		  else
		    {
		      /* l'exo en cours est avant exo_courant, on le compare à exo_precedent */

		      if ( !exo_precedent
			   ||
			   g_date_compare ( exo -> date_debut,
					    exo_precedent -> date_fin ) >= 0 )
			exo_precedent = exo;
		    }
		}
	      else
		exo_courant = exo;


	      liste_tmp = liste_tmp -> next;
	    }

	  if ( exo_precedent )
	    no_exercice_recherche = exo_precedent -> no_exercice;

	  break;

	default:
	}
    }

  /* on récupère la devise des monants pour les tests de montants */

  devise_montant = g_slist_find_custom ( liste_struct_devises,
					GINT_TO_POINTER ( etat_courant -> choix_devise_montant ),
					( GCompareFunc ) recherche_devise_par_no) -> data;
  devise_operation = NULL;


  /*   selection des opérations */
  /* on va mettre l'adresse des opés sélectionnées dans une liste */

  liste_opes_selectionnees = NULL;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      /* on commence par vérifier que le compte fait partie de l'état */

      if ( !etat -> utilise_detail_comptes
	   ||
	   g_slist_index ( etat -> no_comptes,
			   GINT_TO_POINTER ( i )) != -1 )
	{
	  /* 	  le compte est bon, passe à la suite de la sélection */


	  /* on va faire le tour de toutes les opés du compte */

	  GSList *pointeur_tmp;

	  pointeur_tmp = LISTE_OPERATIONS;

	  while ( pointeur_tmp )
	    {
	      struct structure_operation *operation;

	      operation = pointeur_tmp -> data;

	      /* si c'est une opé ventilée, dépend de la conf */

	      if ( operation -> operation_ventilee
		   &&
		   !etat -> pas_detailler_ventilation )
		goto operation_refusee;

	      if ( operation -> no_operation_ventilee_associee
		   &&
		   etat -> pas_detailler_ventilation )
		goto operation_refusee;


	      /* on vérifie ensuite si un texte est recherché */

	      if ( etat -> texte
		   &&
		   (
		    !operation -> notes
		    ||
		    !g_strcasecmp ( operation -> notes,
				    etat -> texte )))
		goto operation_refusee;


	      /* 	  on va vérifier les montant est demandé */

	      /* 	      si les montants nuls sont exclus, c'est ici */

	      if ( etat -> choix_montant_nul == 1
		   &&
		   fabs (operation -> montant ) < 0.01 )
		goto operation_refusee;


	      /* vérifie le positif/nég */

	      if ( etat -> utilise_montant_neg_pos
		   &&
		   ( (( etat -> type_neg_pos
			 &&
			 operation -> montant < 0 )
		       ||
		       ( etat -> type_neg_pos
			 &&
			 fabs (operation -> montant) < 0.01
			 &&
			 etat -> choix_montant_nul == 2 ))
		     ||
		     (( !etat -> type_neg_pos
			&&
			operation -> montant > 0 )
		      ||
		      ( !etat -> type_neg_pos
			&&
			fabs (operation -> montant) < 0.01
			&&
			etat -> choix_montant_nul == 3 ))))
		goto operation_refusee;

	      /* vérifie la valeur du montant */

	      if ( etat -> utilise_valeur )
		{
		  switch ( etat -> type_operateur_valeur )
		    {
		    case 0 :
		      /* = */

		      if ( fabs ( operation -> montant - etat -> montant_valeur ) > 0.01 )
			goto operation_refusee;
		      break;

		    case 1 :
		      /* < */

		      if ( operation -> devise == devise_montant -> no_devise )
			{
			  if ( operation -> montant >= etat -> montant_valeur )
			    goto operation_refusee;
			}
		      else
			{
			  gfloat montant;

			  if ( !devise_operation
			       ||
			       operation -> devise != devise_operation -> no_devise )
			    devise_operation = g_slist_find_custom ( liste_struct_devises,
								     GINT_TO_POINTER ( operation -> devise ),
								     ( GCompareFunc ) recherche_devise_par_no ) -> data;

			  if ( devise_montant -> passage_euro
			       &&
			       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
			    montant = operation -> montant * devise_montant -> change;
			  else
			    if ( devise_operation -> passage_euro
				 &&
				 !strcmp ( devise_montant -> nom_devise, _("Euro") ))
			      montant = operation -> montant / devise_operation -> change;
			    else
			      if ( operation -> une_devise_compte_egale_x_devise_ope )
				montant = operation -> montant / operation -> taux_change - operation -> frais_change;
			      else
				montant = operation -> montant * operation -> taux_change - operation -> frais_change;

			  montant = ( rint (montant * 100 )) / 100;

			  if ( montant >= etat -> montant_valeur )
			    goto operation_refusee;
			}

		      break;
		    case 2 :
		      /* <= */

		      if ( operation -> devise == devise_montant -> no_devise )
			{
			  if ( operation -> montant > etat -> montant_valeur )
			    goto operation_refusee;
			}
		      else
			{
			  gfloat montant;

			  if ( !devise_operation
			       ||
			       operation -> devise != devise_operation -> no_devise )
			    devise_operation = g_slist_find_custom ( liste_struct_devises,
								     GINT_TO_POINTER ( operation -> devise ),
								     ( GCompareFunc ) recherche_devise_par_no ) -> data;

			  if ( devise_montant -> passage_euro
			       &&
			       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
			    montant = operation -> montant * devise_montant -> change;
			  else
			    if ( devise_operation -> passage_euro
				 &&
				 !strcmp ( devise_montant -> nom_devise, _("Euro") ))
			      montant = operation -> montant / devise_operation -> change;
			    else
			      if ( operation -> une_devise_compte_egale_x_devise_ope )
				montant = operation -> montant / operation -> taux_change - operation -> frais_change;
			      else
				montant = operation -> montant * operation -> taux_change - operation -> frais_change;

			  montant = ( rint (montant * 100 )) / 100;

			  if ( montant > etat -> montant_valeur )
			    goto operation_refusee;
			}

		      break;
		    case 3 :
		      /* > */

		      if ( operation -> devise == devise_montant -> no_devise )
			{
			  if ( operation -> montant <= etat -> montant_valeur )
			    goto operation_refusee;
			}
		      else
			{
			  gfloat montant;

			  if ( !devise_operation
			       ||
			       operation -> devise != devise_operation -> no_devise )
			    devise_operation = g_slist_find_custom ( liste_struct_devises,
								     GINT_TO_POINTER ( operation -> devise ),
								     ( GCompareFunc ) recherche_devise_par_no ) -> data;

			  if ( devise_montant -> passage_euro
			       &&
			       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
			    montant = operation -> montant * devise_montant -> change;
			  else
			    if ( devise_operation -> passage_euro
				 &&
				 !strcmp ( devise_montant -> nom_devise, _("Euro") ))
			      montant = operation -> montant / devise_operation -> change;
			    else
			      if ( operation -> une_devise_compte_egale_x_devise_ope )
				montant = operation -> montant / operation -> taux_change - operation -> frais_change;
			      else
				montant = operation -> montant * operation -> taux_change - operation -> frais_change;

			  montant = ( rint (montant * 100 )) / 100;

			  if ( montant <= etat -> montant_valeur )
			    goto operation_refusee;
			}

		      break;
		    case 4 :
		      /* >= */

		      if ( operation -> devise == devise_montant -> no_devise )
			{
			  if ( operation -> montant < etat -> montant_valeur )
			    goto operation_refusee;
			}
		      else
			{
			  gfloat montant;

			  if ( !devise_operation
			       ||
			       operation -> devise != devise_operation -> no_devise )
			    devise_operation = g_slist_find_custom ( liste_struct_devises,
								     GINT_TO_POINTER ( operation -> devise ),
								     ( GCompareFunc ) recherche_devise_par_no ) -> data;

			  if ( devise_montant -> passage_euro
			       &&
			       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
			    montant = operation -> montant * devise_montant -> change;
			  else
			    if ( devise_operation -> passage_euro
				 &&
				 !strcmp ( devise_montant -> nom_devise, _("Euro") ))
			      montant = operation -> montant / devise_operation -> change;
			    else
			      if ( operation -> une_devise_compte_egale_x_devise_ope )
				montant = operation -> montant / operation -> taux_change - operation -> frais_change;
			      else
				montant = operation -> montant * operation -> taux_change - operation -> frais_change;

			  montant = ( rint (montant * 100 )) / 100;

			  if ( montant < etat -> montant_valeur )
			    goto operation_refusee;
			}

		      break;
		    }
		}

	      /* vérifie l'inclusion du montant */

	      if ( etat -> utilise_inclusion )
		{
		  /* on calcule le montant de l'opé dans la devise demandée pour faire les comparaisons */

		  gfloat montant;


		  if ( operation -> devise == devise_montant -> no_devise )
		    montant = operation -> montant;
		  else
		    {
		      if ( !devise_operation
			   ||
			   operation -> devise != devise_operation -> no_devise )
			devise_operation = g_slist_find_custom ( liste_struct_devises,
								 GINT_TO_POINTER ( operation -> devise ),
								 ( GCompareFunc ) recherche_devise_par_no ) -> data;

		      if ( devise_montant -> passage_euro
			   &&
			   !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
			montant = operation -> montant * devise_montant -> change;
		      else
			if ( devise_operation -> passage_euro
			     &&
			     !strcmp ( devise_montant -> nom_devise, _("Euro") ))
			  montant = operation -> montant / devise_operation -> change;
			else
			  if ( operation -> une_devise_compte_egale_x_devise_ope )
			    montant = operation -> montant / operation -> taux_change - operation -> frais_change;
			  else
			    montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		      montant = ( rint (montant * 100 )) / 100;
		    }

		  if ( ( etat -> type_operateur_inf_inclusion
			 &&
			 montant < etat -> montant_inclusion_inf )
		       ||
		       ( !etat -> type_operateur_inf_inclusion
			 &&
			 montant <= etat -> montant_inclusion_inf ))
		    goto operation_refusee;

		  if ( ( etat -> type_operateur_sup_inclusion
			 &&
			 montant > etat -> montant_inclusion_sup )
		       ||
		       ( !etat -> type_operateur_sup_inclusion
			 &&
			 montant >= etat -> montant_inclusion_sup ))
		    goto operation_refusee;
		}


	      /* on vérifie les R */

	      if ( etat -> afficher_r )
		{
		  if ( ( etat -> afficher_r == 1
			 &&
			 operation -> pointe == 2 )
		       ||
		       ( etat -> afficher_r == 2
			 &&
			 operation -> pointe != 2 ))
		    goto operation_refusee;
		}

	      /* 	      on vérifie les virements */

	      if ( operation -> relation_no_operation )
		{
		  if ( !etat -> type_virement )
		    goto operation_refusee;

		  if ( etat -> type_virement == 1 )
		    {
		      /* on inclue l'opé que si le compte de virement */
		      /* est un compte de passif ou d'actif */

		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		      /* pour l'instant on n'a que le compte passif */

		      if ( TYPE_DE_COMPTE != 2 )
			goto operation_refusee;

		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i; 
		    }
		  else
		    {
		      if ( etat -> type_virement == 2 )
			{
			  /* on inclut l'opé que si le compte de virement n'est */
			  /* pas présent dans l'état */
			  
			  if ( g_slist_index ( etat -> no_comptes,
					       GINT_TO_POINTER ( operation -> relation_no_compte )) != -1 )
			    goto operation_refusee;
			}
		      else
			{
			  /* on inclut l'opé que si le compte de virement est dans la liste */

			  if ( g_slist_index ( etat -> no_comptes_virements,
					       GINT_TO_POINTER ( operation -> relation_no_compte )) == -1 )
			    goto operation_refusee;

			}
		    }
		}
	      else
		{
		  /* 		  l'opé n'est pas un virement, si on doit exclure les non virement, c'est ici */

		  if ( etat -> type_virement
		       &&
		       etat -> exclure_ope_non_virement )
		    goto operation_refusee;
		}


	      if ( etat -> utilise_categ )
		{
		  /* on va maintenant vérifier que les catég sont bonnes */
 
		  if ((( etat -> utilise_detail_categ
			 &&
			 g_slist_index ( etat -> no_categ,
					 GINT_TO_POINTER ( operation -> categorie )) == -1 )
		       ||
		       ( etat -> exclure_ope_sans_categ
			 &&
			 !operation -> categorie ))
		      &&
		      !operation -> operation_ventilee )
		    goto operation_refusee;
		}

	      /* vérification de l'imputation budgétaire */

	      if (( etat -> utilise_ib
		    &&
		    etat -> utilise_detail_ib
		    &&
		    g_slist_index ( etat -> no_ib,
				    GINT_TO_POINTER ( operation -> imputation )) == -1 )
		  ||
		  ( etat -> exclure_ope_sans_ib
		    &&
		    !operation -> imputation ))
		goto operation_refusee;

	      /* vérification du tiers */

	      if ( etat -> utilise_tiers
		   &&
		   etat -> utilise_detail_tiers
		   &&
		   g_slist_index ( etat -> no_tiers,
				   GINT_TO_POINTER ( operation -> tiers )) == -1 )
		goto operation_refusee;


	      /* vérifie la plage de date */

	      if ( etat -> exo_date )
		{
		  /* on utilise l'exercice */

		  if ( (( etat -> utilise_detail_exo == 1
			  ||
			  etat -> utilise_detail_exo == 2 )
			&&
			( operation -> no_exercice != no_exercice_recherche
			  ||
			  !operation -> no_exercice ))
		       ||
		       ( etat -> utilise_detail_exo == 3
			 &&
			 ( g_slist_index ( etat -> no_exercices,
					   GINT_TO_POINTER ( operation -> no_exercice )) == -1
			   ||
			   !operation -> no_exercice )))
		    goto operation_refusee;
		}
	      else
		{
		  /* on utilise une plage de dates */

		  GDate *date_jour;
		  GDate *date_tmp;

		  date_jour = g_date_new ();
		  g_date_set_time ( date_jour,
				    time ( NULL ));


		  switch ( etat -> no_plage_date )
		    {
		    case 0:
		      /* toutes */

		      break;

		    case 1:
		      /* plage perso */

		      if ( !etat -> date_perso_debut
			   ||
			   !etat -> date_perso_fin
			   ||
			   g_date_compare ( etat -> date_perso_debut,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( etat -> date_perso_fin,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 2:
		      /* cumul à ce jour */

		      if ( g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 3:
		      /* mois en cours */

		      if ( g_date_month ( date_jour ) != g_date_month ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 4:
		      /* année en cours */

		      if ( g_date_year ( date_jour ) != g_date_year ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 5:
		      /* cumul mensuel */

		      if ( g_date_month ( date_jour ) != g_date_month ( operation -> date )
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 6:
		      /* cumul annuel */

		      if ( g_date_year ( date_jour ) != g_date_year ( operation -> date )
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 7:
		      /* mois précédent */

		      g_date_subtract_months ( date_jour,
					       1 );

		      if ( g_date_month ( date_jour ) != g_date_month ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 8:
		      /* année précédente */

		      g_date_subtract_years ( date_jour,
					      1 );

		      if ( g_date_year ( date_jour ) != g_date_year ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 9:
		      /* 30 derniers jours */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_days ( date_tmp,
					     30 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 10:
		      /* 3 derniers mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_months ( date_tmp,
					       3 );
		      g_date_subtract_months ( date_jour,
					       1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 11:
		      /* 6 derniers mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_months ( date_tmp,
					       6 );
		      g_date_subtract_months ( date_jour,
					       1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 12:
		      /* 12 derniers mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_months ( date_tmp,
					       12 );
		      g_date_subtract_months ( date_jour,
					       1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 13:
		      /* 30 prochains jours */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_days ( date_tmp,
					30 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 14:
		      /* 3 prochains mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_months ( date_tmp,
					  3 );
		      g_date_add_months ( date_jour,
					  1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 15:
		      /* 6 prochains mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_months ( date_tmp,
					  6 );
		      g_date_add_months ( date_jour,
					  1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 16:
		      /* 12 prochains mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_months ( date_tmp,
					  12 );
		      g_date_add_months ( date_jour,
					  1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		    }
		}


	      liste_opes_selectionnees = g_slist_append ( liste_opes_selectionnees,
							  operation );

	    operation_refusee:
	      pointeur_tmp = pointeur_tmp -> next;
	    }
	}
      p_tab_nom_de_compte_variable++;
    }

  /*   à ce niveau, on a récupéré toutes les opés qui entreront dans */
  /* l'état ; reste plus qu'à les classer et les afficher */


  /* on classe la liste et l'affiche en fonction du choix du type de classement */

  etape_finale_affichage_etat ( liste_opes_selectionnees, affichage );


}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Fonction d'impression de l'état */
/*****************************************************************************************************/
void impression_etat ( struct struct_etat *etat )
{
  affichage_etat ( etat, &gnomeprint_affichage );
}


/*****************************************************************************************************/
/* Fonction de rafraichissement de l'état */
/*****************************************************************************************************/
void rafraichissement_etat ( struct struct_etat *etat )
{
  affichage_etat ( etat, &gtktable_affichage );
}

/*****************************************************************************************************/
/* Fonction de classement de la liste */
/* en fonction du choix du type de classement */
/*****************************************************************************************************/

gint classement_liste_opes_etat ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 )
{
  GList *pointeur;

  pointeur = etat_courant -> type_classement;

 classement_suivant:

  /*   si pointeur est nul, on a fait le tour du classement, les opés sont identiques */
  /* on classe par date, et si pareil, par no d'opé */

  if ( !pointeur )
    {
      if ( g_date_compare ( operation_1 -> date,
			    operation_2 -> date ))
	return ( g_date_compare ( operation_1 -> date,
				  operation_2 -> date ));

      /*       les dates sont identiques, on classe par no d'opé */

      return ( operation_1 -> no_operation - operation_2 -> no_operation );
    }


  switch ( GPOINTER_TO_INT ( pointeur -> data ))
    {
      /* classement des catégories */

    case 1:

      if ( etat_courant -> utilise_categ )
	{
	  if ( operation_1 -> categorie != operation_2 -> categorie )
	    return ( operation_1 -> categorie - operation_2 -> categorie );

	  /*     si  les catégories sont nulles, on doit départager entre virements, pas */
	  /* de categ ou opé ventilée */
	  /* on met en 1er les opés sans categ, ensuite les ventilations et enfin les virements */

	  if ( !operation_1 -> categorie )
	    {
	      if ( operation_1 -> operation_ventilee )
		{
		  if ( operation_2 -> relation_no_operation )
		    return ( -1 );
		  else
		    if ( !operation_2 -> operation_ventilee )
		      return ( 1 );
		}
	      else
		{
		  if ( operation_1 -> relation_no_operation )
		    {
		      if ( !operation_2 -> relation_no_operation )
			return ( 1 );
		    }
		  else
		    if ( operation_2 -> relation_no_operation
			 ||
			 operation_2 -> operation_ventilee )
		      return ( -1 );
		}
	    }
	}

      /*       les catégories sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;

      /* classement des sous catégories */

    case 2:

      if ( etat_courant -> utilise_categ
	   &&
	   etat_courant -> afficher_sous_categ )
	{
	  if ( operation_1 -> sous_categorie != operation_2 -> sous_categorie )
	    return ( operation_1 -> sous_categorie - operation_2 -> sous_categorie );
	}

      /*       les ss-catégories sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;

      /* classement des ib */

    case 3:

      if ( etat_courant -> utilise_ib )
	{
	  if ( operation_1 -> imputation != operation_2 -> imputation )
	    return ( operation_1 -> imputation - operation_2 -> imputation );
	}

      /*       les ib sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;

      /* classement des sous ib */

    case 4:

      if ( etat_courant -> utilise_ib
	   &&
	   etat_courant -> afficher_sous_ib )
	{
	  if ( operation_1 -> sous_imputation != operation_2 -> sous_imputation )
	    return ( operation_1 -> sous_imputation - operation_2 -> sous_imputation );
	}

      /*       les ib sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;


      /* classement des comptes */

    case 5:

      if ( etat_courant -> regroupe_ope_par_compte )
	{
	  if ( operation_1 -> no_compte != operation_2 -> no_compte )
	    return ( operation_1 ->no_compte  - operation_2 -> no_compte );
	}

      /*       les comptes sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;
      break;


      /* classement des tiers */

    case 6:

      if ( etat_courant -> utilise_tiers )
	{
	  if ( operation_1 -> tiers != operation_2 -> tiers )
	    return ( operation_1 ->tiers  - operation_2 -> tiers );
	}

      /*       les tiers sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;
      break;
    }
  return (0);
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etape_finale_affichage_etat ( GSList *ope_selectionnees,
				   struct struct_etat_affichage *affichage)
{
  GSList *liste_ope_revenus;
  GSList *liste_ope_depenses;
  GSList *pointeur_tmp;
  gint i;
  gint ligne;
  gdouble total_partie;
  gdouble total_general;
  gchar *decalage_base;
  gchar *decalage_categ;
  gchar *decalage_sous_categ;
  gchar *decalage_ib;
  gchar *decalage_sous_ib;
  gchar *decalage_compte;
  gchar *decalage_tiers;
  GList *pointeur_glist;


  /* on commence par séparer la liste revenus et de dépenses */
  /*   si le classement racine est la catégorie, on sépare par catégorie */
  /* de revenu ou de dépense */
  /* si c'est un autre, on sépare par montant positif ou négatif */

  pointeur_glist = etat_courant -> type_classement;

  liste_ope_revenus = NULL;
  liste_ope_depenses = NULL;
  pointeur_tmp = ope_selectionnees;

  while ( pointeur_tmp )
    {
      struct structure_operation *operation;

      operation = pointeur_tmp -> data;

      if ( GPOINTER_TO_INT ( pointeur_glist -> data ) == 1 )
	{
	  /* le classement racine est la catégorie */
	  /* s'il n'y a pas de catég, c'est un virement ou une ventilation */
	  /*       dans ce cas, on classe en fonction du montant */

	  if ( operation -> categorie )
	    {
	      struct struct_categ *categ;

	      categ = g_slist_find_custom ( liste_struct_categories,
					    GINT_TO_POINTER ( operation -> categorie ),
					    (GCompareFunc) recherche_categorie_par_no ) -> data;

	      if ( categ -> type_categ )
		liste_ope_depenses = g_slist_append ( liste_ope_depenses,
						      operation );
	      else
		liste_ope_revenus = g_slist_append ( liste_ope_revenus,
						     operation );
	    }
	  else
	    {
	      if ( operation -> montant < 0
	       ||
	       ( fabs ( operation -> montant ) < 0.01
		 &&
		 etat_courant -> choix_montant_nul == 2 ))
		liste_ope_depenses = g_slist_append ( liste_ope_depenses,
						      operation );
	      else
		liste_ope_revenus = g_slist_append ( liste_ope_revenus,
						     operation );
	    }
	}
      else
	{
	  /* le classement racine n'est pas la catég, on sépare en fonction du montant */

	  if ( operation -> montant < 0
	       ||
	       ( fabs ( operation -> montant ) < 0.01
		 &&
		 etat_courant -> choix_montant_nul == 2 ))
	    liste_ope_depenses = g_slist_append ( liste_ope_depenses,
						  operation );
	  else
	    liste_ope_revenus = g_slist_append ( liste_ope_revenus,
						 operation );
	}
      pointeur_tmp = pointeur_tmp -> next;
    }

  /* on va maintenant classer ces 2 listes dans l'ordre adéquat */

  liste_ope_depenses = g_slist_sort ( liste_ope_depenses,
				      (GCompareFunc) classement_liste_opes_etat );
  liste_ope_revenus = g_slist_sort ( liste_ope_revenus,
				     (GCompareFunc) classement_liste_opes_etat );

  
  /* calcul du décalage pour chaque classement */
  /* c'est une chaine vide qu'on ajoute devant le nom du */
  /*   classement ( tiers, ib ...) */
 
  /* on met 2 espaces par décalage */
  /*   normalement, pointeur_glist est déjà positionné */

  decalage_base = "";

  /*   pour éviter le warning lors de la compilation, on met */
  /* toutes les var char à "" */

  decalage_categ = "";
  decalage_sous_categ = "";
  decalage_ib = "";
  decalage_sous_ib = "";
  decalage_compte = "";
  decalage_tiers = "";

  while ( pointeur_glist )
    {
      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	  /* décalage de la catégorie */

	case 1:
	  if ( etat_courant -> utilise_categ )
	    decalage_categ = g_strconcat ( decalage_base,
					   "    ",
					   NULL );
	  else
	    {
	      decalage_categ = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage de la ss-catégorie */

	case 2:
	  if ( etat_courant -> utilise_categ
	       &&
	       etat_courant -> afficher_sous_categ )
	    decalage_sous_categ = g_strconcat ( decalage_base,
						"    ",
						NULL );
	  else
	    {
	      decalage_sous_categ = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage de l'ib */

	case 3:
	  if ( etat_courant -> utilise_ib )
	    decalage_ib = g_strconcat ( decalage_base,
					"    ",
					NULL );
	  else
	    {
	      decalage_ib = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage de la ss-ib */

	case 4:
	  if ( etat_courant -> utilise_ib
	       &&
	       etat_courant -> afficher_sous_ib )
	    decalage_sous_ib = g_strconcat ( decalage_base,
					     "    ",
					     NULL );
	  else
	    {
	      decalage_sous_ib = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage du compte */

	case 5:
	  if ( etat_courant -> regroupe_ope_par_compte )
	    decalage_compte = g_strconcat ( decalage_base,
					    "    ",
					    NULL );
	  else
	    {
	      decalage_compte = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage du tiers */

	case 6:
	  if ( etat_courant -> utilise_tiers )
	    decalage_tiers = g_strconcat ( decalage_base,
					   "    ",
					   NULL );
	  else
	    {
	      decalage_tiers = decalage_base;
	      goto pas_decalage;
	    }
	  break;
	}

      decalage_base = g_strconcat ( decalage_base,
				    "    ",
				    NULL );

    pas_decalage:
      pointeur_glist = pointeur_glist -> next;
    }



  /*   calcul du nb de colonnes : */
  /* 1ère pour les titres de structure */
  /* la dernière pour les montants */
  /* et entre les 2 pour l'affichage des opés -> variable */

  nb_colonnes = 0;

  if ( etat_courant -> afficher_opes )
    {
      nb_colonnes = nb_colonnes + etat_courant -> afficher_date_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_tiers_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_categ_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_ib_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_notes_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_pc_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_infobd_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_no_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_type_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_cheque_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_rappr_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_exo_ope;

      /* on ajoute les séparations */

      nb_colonnes = 2 * nb_colonnes;
    }

  nb_colonnes = nb_colonnes + 3;
  ligne_debut_partie = -1;

  nom_categ_en_cours = NULL;
  nom_ss_categ_en_cours = NULL;
  nom_ib_en_cours = NULL;
  nom_ss_ib_en_cours = NULL;
  nom_compte_en_cours = NULL;
  nom_tiers_en_cours = NULL;

  if (! affichage -> init ())
    {
      return;
    }

  /* on commence à remplir le tableau */

  /* on met le titre */

  total_general = 0;
  ligne = affichage -> afficher_titre ( 0 );

  /* séparation */
  ligne = affichage -> afficher_separateur (ligne);

  /*   si nécessaire, on met les titres des colonnes */

  if ( etat_courant -> afficher_opes
       &&
       etat_courant -> afficher_titre_colonnes
       &&
       !etat_courant -> type_affichage_titres )
    ligne = affichage -> affiche_titres_colonnes ( ligne );

  for ( i=0 ; i<2 ; i++ )
    {
      ancienne_categ_etat = -1;
      ancienne_categ_speciale_etat = 0;
      ancienne_sous_categ_etat = -1;
      ancienne_ib_etat = -1;
      ancienne_sous_ib_etat = -1;
      ancien_compte_etat = -1;
      ancien_tiers_etat = -1;

      montant_categ_etat = 0;
      montant_sous_categ_etat = 0;
      montant_ib_etat = 0;
      montant_sous_ib_etat = 0;
      montant_compte_etat = 0;
      montant_tiers_etat = 0;
      montant_periode_etat = 0;
      total_partie = 0;
      date_debut_periode = NULL;

      changement_de_groupe_etat = 0;
      debut_affichage_etat = 1;
      devise_compte_en_cours_etat = NULL;

      /*       on met directement les adr des devises de categ, ib et tiers en global pour */
      /* gagner de la vitesse */

      devise_categ_etat = g_slist_find_custom ( liste_struct_devises,
						GINT_TO_POINTER ( etat_courant -> devise_de_calcul_categ ),
						( GCompareFunc ) recherche_devise_par_no) -> data;

      devise_ib_etat = g_slist_find_custom ( liste_struct_devises,
						GINT_TO_POINTER ( etat_courant -> devise_de_calcul_ib ),
						( GCompareFunc ) recherche_devise_par_no) -> data;

      devise_tiers_etat = g_slist_find_custom ( liste_struct_devises,
						GINT_TO_POINTER ( etat_courant -> devise_de_calcul_tiers ),
						( GCompareFunc ) recherche_devise_par_no) -> data;

      devise_generale_etat = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( etat_courant -> devise_de_calcul_general ),
						   ( GCompareFunc ) recherche_devise_par_no) -> data;

      /* on met ici le pointeur sur les revenus ou sur les dépenses */
      /* en vérifiant qu'il y en a */

      if ( !i )
	{
	  /* on met le pointeur sur les revenus */

	  if ( liste_ope_revenus )
	    {
	      pointeur_tmp = liste_ope_revenus;

	      ligne = affichage -> affiche_titre_revenus_etat ( ligne );
	    }
	  else
	    {
	      /* il n'y a pas de revenus, on saute directement aux dépenses */

	      i++;
	      pointeur_tmp = liste_ope_depenses;

	      /* 	      s'il n'y a pas de dépenses non plus, on sort de la boucle */
 
	      if ( !liste_ope_depenses )
		goto fin_boucle_affichage_etat;

	      ligne = affichage -> affiche_titre_depenses_etat ( ligne );
	    }
	}
      else
	{
	  /* on met le pointeur sur les dépenses */

	  if ( liste_ope_depenses )
	    {
	      /* séparation */
	      ligne = affichage -> afficher_separateur (ligne);
	      pointeur_tmp = liste_ope_depenses;

	      ligne = affichage -> affiche_titre_depenses_etat ( ligne );
	    }
	  else
	    goto fin_boucle_affichage_etat;
	}


      /* on commence la boucle qui fait le tour de chaque opé */

      while ( pointeur_tmp )
	{
	  struct structure_operation *operation;
	  struct struct_devise *devise_operation;
	  gdouble montant;

	  operation = pointeur_tmp -> data;

	  pointeur_glist = etat_courant -> type_classement;

	  while ( pointeur_glist )
	    {
	      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
		{
		case 1:
		  ligne = affichage -> affiche_categ_etat ( operation,
							    decalage_categ,
							    ligne );
		  break;

		case 2:
		  ligne = affichage -> affiche_sous_categ_etat ( operation,
								 decalage_sous_categ,
								 ligne );

		  break;

		case 3:
		  ligne = affichage -> affiche_ib_etat ( operation,
							 decalage_ib,
							 ligne );

		  break;

		case 4:
		  ligne = affichage -> affiche_sous_ib_etat ( operation,
							      decalage_sous_ib,
							      ligne );

		  break;

		case 5:
		  ligne = affichage -> affiche_compte_etat ( operation,
							     decalage_compte,
							     ligne );

		  break;

		case 6:
		  ligne = affichage -> affiche_tiers_etat ( operation,
							    decalage_tiers,
							    ligne );
		}

	      pointeur_glist = pointeur_glist -> next;
	    }


	  /* on affiche si nécessaire le total de la période */

	  ligne = gtktable_affiche_total_periode ( operation,
						   ligne,
						   0 );


	  ligne = affichage -> affichage_ligne_ope ( operation,
						     ligne );

	  /* on ajoute les montants que pour ceux affichés */

	  /* calcule le montant de la categ */

	  if ( etat_courant -> utilise_categ )
	    {
	      if ( operation -> devise == devise_categ_etat -> no_devise )
		montant = operation -> montant;
	      else
		{
		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_categ_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		    montant = operation -> montant * devise_categ_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_categ_etat -> nom_devise, _("Euro") ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_categ_etat = montant_categ_etat + montant;
	      montant_sous_categ_etat = montant_sous_categ_etat + montant;
	    }

	  /* calcule le montant de l'ib */
	  
	  if ( etat_courant -> utilise_ib )
	    {
	      if ( operation -> devise == devise_ib_etat -> no_devise )
		montant = operation -> montant;
	      else
		{
		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_ib_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		    montant = operation -> montant * devise_ib_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_ib_etat -> nom_devise, _("Euro") ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_ib_etat = montant_ib_etat + montant;
	      montant_sous_ib_etat = montant_sous_ib_etat + montant;
	    }

	  /* calcule le montant du tiers */

	  if ( etat_courant -> utilise_tiers )
	    {
	      if ( operation -> devise == devise_tiers_etat -> no_devise )
		montant = operation -> montant;
	      else
		{
		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_tiers_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		    montant = operation -> montant * devise_tiers_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_tiers_etat -> nom_devise, _("Euro") ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_tiers_etat = montant_tiers_etat + montant;
	    }

	  /* calcule le montant du compte */

	  if ( etat_courant -> affiche_sous_total_compte )
	    {
	      /* on modifie le montant s'il n'est pas de la devise du compte en cours */

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	      if ( !devise_compte_en_cours_etat
		   ||
		   DEVISE != devise_compte_en_cours_etat -> no_devise )
		devise_compte_en_cours_etat = g_slist_find_custom ( liste_struct_devises,
								    GINT_TO_POINTER ( DEVISE ),
								    ( GCompareFunc ) recherche_devise_par_no) -> data;

	      if ( operation -> devise == DEVISE )
		montant = operation -> montant;
	      else
		{
		  /* ce n'est pas la devise du compte, si le compte passe à l'euro et que la devise est l'euro, */
		  /* utilise la conversion du compte, */
		  /* si c'est une devise qui passe à l'euro et que la devise du compte est l'euro, utilise la conversion du compte */
		  /* sinon utilise la conversion stockée dans l'opé */

		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_compte_en_cours_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		    montant = operation -> montant * devise_compte_en_cours_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_compte_en_cours_etat -> nom_devise, _("Euro") ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_compte_etat = montant_compte_etat + montant;
	    }

	  /* calcule les montants totaux */

	  if ( operation -> devise == devise_generale_etat -> no_devise )
	    montant = operation -> montant;
	  else
	    {
	      devise_operation = g_slist_find_custom ( liste_struct_devises,
						       GINT_TO_POINTER ( operation -> devise ),
						       ( GCompareFunc ) recherche_devise_par_no ) -> data;

	      if ( devise_generale_etat -> passage_euro
		   &&
		   !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		montant = operation -> montant * devise_generale_etat -> change;
	      else
		if ( devise_operation -> passage_euro
		     &&
		     !strcmp ( devise_generale_etat -> nom_devise, _("Euro") ))
		  montant = operation -> montant / devise_operation -> change;
		else
		  if ( operation -> une_devise_compte_egale_x_devise_ope )
		    montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		  else
		    montant = operation -> montant * operation -> taux_change - operation -> frais_change;

	      montant = ( rint (montant * 100 )) / 100;
	    }


	  /* calcule le montant de la periode */

	  if ( etat_courant -> separation_par_plage )
	    {
	      if ( operation -> devise == devise_categ_etat -> no_devise )
		montant = operation -> montant;
	      else
		{
		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_categ_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		    montant = operation -> montant * devise_categ_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_categ_etat -> nom_devise, _("Euro") ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_periode_etat = montant_periode_etat + montant;
	    }

	  total_partie = total_partie + montant;
	  total_general = total_general + montant;

	  changement_de_groupe_etat = 0;

	  pointeur_tmp = pointeur_tmp -> next;
	}


      /*   à la fin, on affiche les totaux des dernières lignes */

      /* on affiche le total de la période en le forçant */

      ligne = gtktable_affiche_total_periode ( NULL,
					       ligne,
					       1 );

      ligne = affichage -> affiche_totaux_sous_jaccent ( GPOINTER_TO_INT ( etat_courant -> type_classement -> data ),
					    ligne );

					    
      /* on ajoute le total de la structure racine */


       switch ( GPOINTER_TO_INT ( etat_courant -> type_classement -> data ))
	{
	case 1:
	  ligne = affichage -> affiche_total_categories ( ligne );
	  break;

	case 2:
	  ligne = affichage -> affiche_total_sous_categ ( ligne );
	  break;

	case 3:
	  ligne = affichage -> affiche_total_ib ( ligne );
	  break;

	case 4:
	  ligne = affichage -> affiche_total_sous_ib ( ligne );
	  break;

	case 5:
	  ligne = affichage -> affiche_total_compte ( ligne );
	  break;

	case 6:
	  ligne = affichage -> affiche_total_tiers ( ligne );
	  break;
	}

      /* on affiche le total de la partie en cours */

      ligne = affichage -> affiche_total_partiel ( total_partie,
				      ligne,
				      i );

    fin_boucle_affichage_etat:
    }

  /* on affiche maintenant le total général */

  ligne = affichage -> affiche_total_general ( total_general,
				  ligne );

  affichage -> finish ();
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* met tous les anciens_x_etat sous jaccents à l'origine à -1 */
/*****************************************************************************************************/

void denote_struct_sous_jaccentes ( gint origine )
{
  GList *pointeur_glist;

  /* on peut partir du bout de la liste pour revenir vers la structure demandée */
  /* gros vulgaire copier coller de la fonction précédente */

  pointeur_glist = g_list_last ( etat_courant -> type_classement );


  while ( GPOINTER_TO_INT ( pointeur_glist -> data ) != origine )
    {
      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	case 1:
	  ancienne_categ_etat = -1;
	  ancienne_categ_speciale_etat = 0;
	  break;

	case 2:
	  ancienne_sous_categ_etat = -1;
	  break;

	case 3:
	  ancienne_ib_etat = -1;
	  break;

	case 4:
	  ancienne_sous_ib_etat = -1;
	  break;

	case 5:
	  ancien_compte_etat = -1;
	  break;

	case 6:
	  ancien_tiers_etat = -1;
	  break;
	}
      pointeur_glist = pointeur_glist -> prev;
    }
}
/*****************************************************************************************************/

