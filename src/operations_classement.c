/* Fichier classement_liste.c */
/* Contient toutes les fonctions utilisées pour classer la liste des opés */

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


/* ********************************************************************************************************** */
/* Fonction par défaut : par ordre de date */
/* ********************************************************************************************************** */

gint classement_liste_par_date ( GtkWidget *liste,
				 GtkCListRow *ligne_1,
				 GtkCListRow *ligne_2 )
{

  struct structure_operation *operation_1;
  struct structure_operation *operation_2;
  gint retour;

  operation_1 = ligne_1 -> data;
  operation_2 = ligne_2 -> data;


  if ( operation_1 == GINT_TO_POINTER ( -1 ) )
    return ( 1 );

  if ( operation_2 == GINT_TO_POINTER ( -1 ) )
    return ( -1 );

  if ( etat.classement_par_date )
    /* on classe par dates normales */
    retour = g_date_compare ( operation_1 -> date,
			      operation_2 -> date );
  else
    {
      /*       on classe par date bancaire, si elle existe */

      if ( operation_1 -> date_bancaire )
	{
	  if ( operation_2 -> date_bancaire )
	    retour = g_date_compare ( operation_1 -> date_bancaire,
				      operation_2 -> date_bancaire );
	  else
	    retour = g_date_compare ( operation_1 -> date_bancaire,
				      operation_2 -> date );
	}
      else
	{
	  if ( operation_2 -> date_bancaire )
	    retour = g_date_compare ( operation_1 -> date,
				      operation_2 -> date_bancaire );
	  else
	    retour = g_date_compare ( operation_1 -> date,
				      operation_2 -> date );
	}
    }


  if ( retour )
    return ( retour );
  else
    return ( operation_1 -> no_operation - operation_2 -> no_operation );

}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
/* classement par no d'opé (donc d'entrée)  */
/* ********************************************************************************************************** */

gint classement_liste_par_no_ope ( GtkWidget *liste,
				   GtkCListRow *ligne_1,
				   GtkCListRow *ligne_2 )
{
  struct structure_operation *operation_1;
  struct structure_operation *operation_2;

  operation_1 = ligne_1 -> data;
  operation_2 = ligne_2 -> data;


  if ( operation_1 == GINT_TO_POINTER ( -1 ) )
    return ( 1 );

  if ( operation_2 == GINT_TO_POINTER ( -1 ) )
    return ( -1 );

  return (operation_1 -> no_operation - operation_2 -> no_operation);
}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
gint classement_liste_par_tri_courant ( GtkWidget *liste,
					GtkCListRow *ligne_1,
					GtkCListRow *ligne_2 )
{
  struct structure_operation *operation_1;
  struct structure_operation *operation_2;
  gint pos_type_ope_1;
  gint pos_type_ope_2;
  gint buffer;

  operation_1 = ligne_1 -> data;
  operation_2 = ligne_2 -> data;

  if ( operation_1 == GINT_TO_POINTER ( -1 ) )
    return ( 1 );

  if ( operation_2 == GINT_TO_POINTER ( -1 ) )
    return ( -1 );

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  /* si l'opé est négative et que le type est neutre et que les types neutres sont séparés, on lui */
  /* met la position du type négatif */

  if ( operation_1 -> montant < 0
       &&
       ( buffer = g_slist_index ( LISTE_TRI,
				  GINT_TO_POINTER ( -operation_1 -> type_ope ))) != -1 )
    pos_type_ope_1 = buffer;
  else
    pos_type_ope_1 = g_slist_index ( LISTE_TRI,
				     GINT_TO_POINTER ( operation_1 -> type_ope ));

  if ( operation_2 -> montant < 0
       &&
       ( buffer = g_slist_index ( LISTE_TRI,
				  GINT_TO_POINTER ( -operation_2 -> type_ope ))) != -1 )
    pos_type_ope_2 = buffer;
  else
    pos_type_ope_2 = g_slist_index ( LISTE_TRI,
				     GINT_TO_POINTER ( operation_2 -> type_ope ));



  /*   s'ils ont le même type, on classe par date */

  if ( pos_type_ope_1 == pos_type_ope_2 )
    {
      gint retour;

      if ( etat.classement_par_date )
	/* on classe par dates normales */
	retour = g_date_compare ( operation_1 -> date,
				  operation_2 -> date );
      else
	{
	  /*       on classe par date bancaire, si elle existe */

	  if ( operation_1 -> date_bancaire )
	    {
	      if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date_bancaire,
					  operation_2 -> date_bancaire );
	      else
		retour = g_date_compare ( operation_1 -> date_bancaire,
					  operation_2 -> date );
	    }
	  else
	    {
	      if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date,
					  operation_2 -> date_bancaire );
	      else
		retour = g_date_compare ( operation_1 -> date,
					  operation_2 -> date );
	    }
	}

      if ( retour )
	return ( retour );
      else
	return ( operation_1 -> no_operation - operation_2 -> no_operation );
    }


  if ( pos_type_ope_1 < pos_type_ope_2 )
    return ( -1 );
  else
    return ( 1 );

}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
/* Fonction par défaut : par ordre de date */
/* ********************************************************************************************************** */

gint classement_sliste_par_date ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 )
{
  gint retour;

  if ( etat.classement_par_date )
    /* on classe par dates normales */
    retour = g_date_compare ( operation_1 -> date,
			      operation_2 -> date );
  else
    {
      /*       on classe par date de valeur, si elle existe */

      if ( operation_1 -> date_bancaire )
	{
	  if ( operation_2 -> date_bancaire )
	    retour = g_date_compare ( operation_1 -> date_bancaire,
				      operation_2 -> date_bancaire );
	  else
	    retour = g_date_compare ( operation_1 -> date_bancaire,
				      operation_2 -> date );
	}
      else
	{
	  if ( operation_2 -> date_bancaire )
	    retour = g_date_compare ( operation_1 -> date,
				      operation_2 -> date_bancaire );
	  else
	    retour = g_date_compare ( operation_1 -> date,
				      operation_2 -> date );
	}
    }


  if ( retour )
    return ( retour );
  else
    return ( operation_1 -> no_operation - operation_2 -> no_operation );
}
/* ********************************************************************************************************** */



/* ********************************************************************************************************** */
/* identique à classement_liste_par_tri_courant sauf que classe */
/* une slist */
/* ********************************************************************************************************** */

gint classement_sliste_par_tri_courant ( struct structure_operation *operation_1,
					 struct structure_operation *operation_2 )
{
  gint pos_type_ope_1;
  gint pos_type_ope_2;
  gint buffer;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  /* si l'opé est négative et que le type est neutre et que les types neutres sont séparés, on lui */
  /* met la position du type négatif */

  if ( operation_1 -> montant < 0
       &&
       ( buffer = g_slist_index ( LISTE_TRI,
				  GINT_TO_POINTER ( -operation_1 -> type_ope ))) != -1 )
    pos_type_ope_1 = buffer;
  else
    pos_type_ope_1 = g_slist_index ( LISTE_TRI,
				     GINT_TO_POINTER ( operation_1 -> type_ope ));

  if ( operation_2 -> montant < 0
       &&
       ( buffer = g_slist_index ( LISTE_TRI,
				  GINT_TO_POINTER ( -operation_2 -> type_ope ))) != -1 )
    pos_type_ope_2 = buffer;
  else
    pos_type_ope_2 = g_slist_index ( LISTE_TRI,
				     GINT_TO_POINTER ( operation_2 -> type_ope ));



  /*   s'ils ont le même type, on classe par date */

  if ( pos_type_ope_1 == pos_type_ope_2 )
    {
      gint retour;

      if ( etat.classement_par_date )
	/* on classe par dates normales */
	retour = g_date_compare ( operation_1 -> date,
				  operation_2 -> date );
      else
	{
	  /*       on classe par date bancaire, si elle existe */

	  if ( operation_1 -> date_bancaire )
	    {
	      if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date_bancaire,
					  operation_2 -> date_bancaire );
	      else
		retour = g_date_compare ( operation_1 -> date_bancaire,
					  operation_2 -> date );
	    }
	  else
	    {
	      if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date,
					  operation_2 -> date_bancaire );
	      else
		retour = g_date_compare ( operation_1 -> date,
					  operation_2 -> date );
	    }
	}

      if ( retour )
	return ( retour );
      else
	return ( operation_1 -> no_operation - operation_2 -> no_operation );
    }


  if ( pos_type_ope_1 < pos_type_ope_2 )
    return ( -1 );
  else
    return ( 1 );

}
/* ********************************************************************************************************** */

