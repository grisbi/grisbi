/*  Fonctions utilitaires à destination des plugins d'état  */
/*      etats.c */

/*     Copyright (C) 2002  Benjamin Drieu */
/* 			benj@april.org */
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

gchar * etats_titre ()
{
  gchar *titre;
  GDate *date_jour;

  date_jour = g_date_new ();
  g_date_set_time ( date_jour,
		    time ( NULL ));
  
  titre = etat_courant -> nom_etat;

  if ( etat_courant -> exo_date )
    {
      GSList *liste_tmp;
      struct struct_exercice *exo;
      struct struct_exercice *exo_courant;
      struct struct_exercice *exo_precedent;

      /* initialise les variables pour la recherche d'exo */

      liste_tmp = liste_struct_exercices;

      exo_courant = NULL;
      exo_precedent = NULL;
      exo = NULL;

      switch ( etat_courant -> utilise_detail_exo )
	{
	case 0:
	  /* tous les exos */

	  titre = g_strconcat ( titre,
				", ", _("tous les exercices"),
				NULL );
	  break;

	case 1:
	  /* exercice courant */

	  /* on recherche l'exo courant */

	  while ( liste_tmp )
	    {
	      exo = liste_tmp -> data;

	      if ( g_date_compare ( date_jour,
				    exo -> date_debut ) >= 0
		   &&
		   g_date_compare ( date_jour,
				    exo -> date_fin ) <= 0 )
		liste_tmp = NULL;
	      else
		liste_tmp = liste_tmp -> next;
	    }

	  /* 	  à ce niveau, exo contient l'exercice courant ou NULL */

	  if ( exo )
	    titre = g_strconcat ( titre,
				  ", ", _("exercice courant") , " (",
				  exo -> nom_exercice,
				  ")",
				  NULL );
	  else
	    titre = g_strconcat ( titre,
				  ", ", _("exercice courant"),
				  NULL );
	  break;

	case 2:
	  /* exo précédent */
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

	  /* 	  à ce niveau, exo_precedent contient l'exercice précédent ou NULL */

	  if ( exo_precedent )
	    titre = g_strconcat ( titre,
				  ", ", _("exercice précédent") , " (",
				  exo_precedent -> nom_exercice,
				  ")",
				  NULL );
	  else
	    titre = g_strconcat ( titre,
				  ", ", _("exercice précédent"),
				  NULL );
	  break;

	case 3:
	  /* exos perso */
	  /* 	  un ou plusieurs exos ont été sélectionnés, on récupère le nom de chacuns */

	  liste_tmp = etat_courant -> no_exercices;

	  if ( g_slist_length ( liste_tmp ) > 1 )
	    titre = g_strconcat ( titre,
				  ", ", _("exercices"), " ",
				  NULL );
	  else
	    titre = g_strconcat ( titre,
				  ", ", _("exercice"), " ",
				  NULL );

	  while ( liste_tmp )
	    {
	      exo = g_slist_find_custom ( liste_struct_exercices,
					  liste_tmp -> data,
					  (GCompareFunc) recherche_exercice_par_no ) -> data;

	      if ( liste_tmp == g_slist_last ( etat_courant -> no_exercices ))
		titre = g_strconcat ( titre,
				      exo -> nom_exercice,
				      NULL );
	      else
		titre = g_strconcat ( titre,
				      exo -> nom_exercice,
				      ", ",
				      NULL );
	      liste_tmp = liste_tmp -> next;
	    }



	  break;
	}
    }     
  else
    {
      /* c'est une plage de dates qui a été entrée */

      gchar buffer_date[15];
      gchar buffer_date_2[15];
      GDate *date_tmp;

      switch ( etat_courant -> no_plage_date )
	{
	case 0:
	  /* toutes */

	  titre = g_strconcat ( titre,
				", ",
				_("toutes les dates"),
				NULL );
	  break;

	case 1:
	  /* plage perso */

	  if ( etat_courant -> date_perso_debut
	       &&
	       etat_courant -> date_perso_fin )
	    titre = g_strconcat ( titre,
				  ", ",
				  g_strdup_printf ( _("du %d/%d/%d au %d/%d/%d"),
						    g_date_day ( etat_courant -> date_perso_debut ),
						    g_date_month ( etat_courant -> date_perso_debut ),
						    g_date_year ( etat_courant -> date_perso_debut ),
						    g_date_day ( etat_courant -> date_perso_fin ),
						    g_date_month ( etat_courant -> date_perso_fin ),
						    g_date_year ( etat_courant -> date_perso_fin )),
				  NULL );
	  else
	    titre = g_strconcat ( titre,
				  ", ", _("plages perso non remplies"),
				  NULL );
	  break;

	case 2:
	  /* cumul à ce jour */

	  titre = g_strconcat ( titre,
				", ", g_strdup_printf ( _("cumul au %d/%d/%d"),
						  g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 3:
	  /* mois en cours */

	  g_date_strftime ( buffer_date,
			    14,
			    "%B",
			    date_jour );

	  titre = g_strconcat ( titre,
				", ", 
				g_strdup_printf ( _("%s %d"),
						  buffer_date,
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 4:
	  /* année en cours */

	  titre = g_strconcat ( titre,
				", ", g_strdup_printf ( _("année %d"),
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 5:
	  /* cumul mensuel */

	  titre = g_strconcat ( titre,
				", ", g_strdup_printf ( _("cumul mensuel au %d/%d/%d"),
						  g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 6:
	  /* cumul annuel */

	  titre = g_strconcat ( titre,
				", ", g_strdup_printf ( _("cumul annuel au %d/%d/%d"),
						  g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 7:
	  /* mois précédent */

	  g_date_subtract_months ( date_jour,
				   1 );
	  g_date_strftime ( buffer_date,
			    14,
			    "%B",
			    date_jour );

	  titre = g_strconcat ( titre,
				", ", 
				g_strdup_printf ( _("%s %d"),
						  buffer_date,
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 8:
	  /* année précédente */

	  titre = g_strconcat ( titre,
				", ", g_strdup_printf ( _("année %d"),
						  g_date_year ( date_jour ) - 1),
				NULL );
	  break;

	case 9:
	  /* 30 derniers jours */

	  date_tmp = g_date_new ();
	  g_date_set_time ( date_tmp,
			    time ( NULL ));

	  g_date_subtract_days ( date_tmp,
				 30 );

	  titre = g_strconcat ( titre,
				", ", g_strdup_printf ( _("du %d/%d/%d au %d/%d/%d"),
						  g_date_day ( date_tmp ),
						  g_date_month ( date_tmp ),
						  g_date_year (date_tmp  ),
						  g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 10:
	  /* 3 derniers mois */

	  date_tmp = g_date_new ();
	  g_date_set_time ( date_tmp,
			    time ( NULL ));
	  g_date_subtract_months ( date_tmp,
				   3 );
	  g_date_strftime ( buffer_date_2,
			    14,
			    "%B",
			    date_tmp );
	  g_date_strftime ( buffer_date,
			    14,
			    "%B",
			    date_jour );

	  titre = g_strconcat ( titre,
				", ", 
				g_strdup_printf ( _("de %s %d"),
						  buffer_date_2,
						  g_date_year ( date_tmp )),
				" ",
				g_strdup_printf ( _("à %s %d"),
						  buffer_date,
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 11:
	  /* 6 derniers mois */

	  date_tmp = g_date_new ();
	  g_date_set_time ( date_tmp,
			    time ( NULL ));
	  g_date_subtract_months ( date_tmp,
				   6 );
	  g_date_strftime ( buffer_date_2,
			    14,
			    "%B",
			    date_tmp );
	  g_date_strftime ( buffer_date,
			    14,
			    "%B",
			    date_jour );


	  titre = g_strconcat ( titre,
				", ", 
				g_strdup_printf ( _("de %s %d"),
						  buffer_date_2,
						  g_date_year ( date_tmp )),
				" ",
				g_strdup_printf ( _("à %s %d"),
						  buffer_date,
						  g_date_year ( date_jour )),
				NULL );
	  break;

	case 12:
	  /* 12 derniers mois */

	  date_tmp = g_date_new ();
	  g_date_set_time ( date_tmp,
			    time ( NULL ));
	  g_date_subtract_months ( date_tmp,
				   12 );
	  g_date_strftime ( buffer_date_2,
			    14,
			    "%B",
			    date_tmp );
	  g_date_strftime ( buffer_date,
			    14,
			    "%B",
			    date_jour );

	  titre = g_strconcat ( titre,
				", ", 
				g_strdup_printf ( _("de %s %d"),
						  buffer_date_2,
						  g_date_year ( date_tmp )),
				" ",
				g_strdup_printf ( _("à %s %d"),
						  buffer_date,
						  g_date_year ( date_jour )),
				NULL );
	  break;
	}
    }

  return titre;
}
