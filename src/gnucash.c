/* ************************************************************************** */
/* Ce fichier s'occupe de la gestion du format gnucash			      */
/*                                                                            */
/*     Copyright (C)	2004      Benjamin Drieu (bdrieu@april.org)	      */
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
#include "structures.h"


extern GSList *liste_comptes_importes;
extern gint nb_comptes;


void recuperation_donnees_gnucash_book ( xmlDocPtr book_node );
void recuperation_donnees_gnucash_categorie ( xmlDocPtr categ_node );
void recuperation_donnees_gnucash_compte ( xmlDocPtr compte_node );
gboolean node_strcmp ( xmlDocPtr node, gchar * name );
xmlDocPtr get_child ( xmlDocPtr node, gchar * child_name );
gchar * child_content ( xmlDocPtr node, gchar * child_name );
gchar * get_currency ( xmlDocPtr currency_node );


gboolean recuperation_donnees_gnucash ( gchar * filename )
{
  xmlDocPtr doc;

  doc = xmlParseFile ( filename );

  if ( doc )
    {
      xmlNodePtr root = xmlDocGetRootElement(doc);
      xmlNodePtr root_node = root -> children;

      while ( root_node )
	{
	  if ( node_strcmp(root_node, "book") )
	    {
	      recuperation_donnees_gnucash_book ( root_node );
	    }
	  root_node = root_node -> next;
	}
    }

  printf ("fini\n");

/* 	/\* ajoute le ou les compte aux autres comptes importés *\/ */
/* 	while ( liste_tmp ) */
/* 	{ */
/* 	    liste_comptes_importes = g_slist_append ( liste_comptes_importes, */
/* 						      liste_tmp -> data ); */
/* 	    liste_tmp = liste_tmp -> next; */
/* 	} */
/*     } */

    return ( TRUE );
}


void recuperation_donnees_gnucash_book ( xmlDocPtr book_node )
{
  xmlDocPtr child_node;

  child_node = book_node -> children;

  while ( child_node )
    {
      if ( node_strcmp(child_node, "account") )
	{
	  if ( !strcmp(child_content(child_node, "type"), "BANK") )
	    {
	      recuperation_donnees_gnucash_compte ( child_node );
	    }
	  else
	    {
	      recuperation_donnees_gnucash_categorie ( child_node );
	    }
	}

      child_node = child_node -> next;
    }
}


void recuperation_donnees_gnucash_compte ( xmlDocPtr compte_node )
{
  struct struct_compte_importation *compte;

  compte = calloc ( 1, sizeof ( struct struct_compte_importation ));

  /* Gnucash import */
  compte -> origine = GNUCASH_IMPORT;

  printf ("> New account: %s, devise %s\n", 
	  child_content ( compte_node, "name" ),
	  get_currency ( get_child(compte_node, "commodity") ) );

  /** FIXME: be sure there are no other sorts + write enum */
  compte -> type_de_compte = 0;
  compte -> nom_de_compte = child_content ( compte_node, "name" );
  compte -> solde = 0;
  compte -> devise = get_currency ( get_child(compte_node, "commodity") );
  compte -> guid = child_content ( compte_node, "id" );
  compte -> operations_importees = NULL;
  
  liste_comptes_importes = g_slist_append ( liste_comptes_importes, compte );
}


void recuperation_donnees_gnucash_categorie ( xmlDocPtr categ_node )
{

}


gchar * get_currency ( xmlDocPtr currency_node )
{
  /** FIXME: detail how it works */
  return child_content ( currency_node, "id" );
}


gboolean node_strcmp ( xmlDocPtr node, gchar * name )
{
  return node -> name && !strcmp ( node -> name, name );
}


xmlDocPtr get_child ( xmlDocPtr node, gchar * child_name )
{
  xmlDocPtr iter_node = node -> children;

  if (!node) return NULL;

  while ( iter_node )
    {
      if ( node_strcmp(iter_node, child_name) )
	return iter_node;

      iter_node = iter_node -> next;
    }

  return NULL;
}


gchar * child_content ( xmlDocPtr node, gchar * child_name )
{
  xmlDocPtr child_node;

  if (!node) return NULL;

  child_node = get_child ( node, child_name );

  if ( child_node )
    return xmlNodeGetContent ( child_node );
  
  return NULL;
}
