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

#include "utils.h"

extern GSList *liste_comptes_importes;
extern gint nb_comptes;


void recuperation_donnees_gnucash_book ( xmlNodePtr book_node );
void recuperation_donnees_gnucash_categorie ( xmlNodePtr categ_node );
void recuperation_donnees_gnucash_compte ( xmlNodePtr compte_node );
void recuperation_donnees_gnucash_transaction ( xmlNodePtr transaction_node );
gboolean node_strcmp ( xmlNodePtr node, gchar * name );
xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name );
gchar * child_content ( xmlNodePtr node, gchar * child_name );
gchar * get_currency ( xmlNodePtr currency_node );
struct struct_compte_importation * find_imported_account_by_uid ( gchar * guid );
struct gnucash_category * find_imported_categ_by_uid ( gchar * guid );
gdouble gnucash_value ( gchar * value );


struct gnucash_category {
  gchar * name;
  enum gnucash_category_type {
    GNUCASH_CATEGORY_INCOME,
    GNUCASH_CATEGORY_EXPENSE,
  } type;
  gchar * guid;
};
GSList * gnucash_categories = NULL;


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

  return ( TRUE );
}


void recuperation_donnees_gnucash_book ( xmlNodePtr book_node )
{
  xmlNodePtr child_node;

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

      if ( node_strcmp(child_node, "transaction") )
	{
	  recuperation_donnees_gnucash_transaction ( child_node );
	}

      child_node = child_node -> next;
    }
}


void recuperation_donnees_gnucash_compte ( xmlNodePtr compte_node )
{
  struct struct_compte_importation *compte;

  compte = calloc ( 1, sizeof ( struct struct_compte_importation ));

  /* Gnucash import */
  compte -> origine = GNUCASH_IMPORT;

  printf (">>> new account: %s, devise %s\n", 
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



void recuperation_donnees_gnucash_categorie ( xmlNodePtr categ_node )
{
  struct gnucash_category * categ;

  categ = calloc ( 1, sizeof ( struct gnucash_category ));

  categ -> name = child_content ( categ_node, "name" );
  categ -> guid = child_content ( categ_node, "id" );
  
  if ( !strcmp ( child_content ( categ_node, "type" ), "INCOME" ) )
    {
      categ -> type = GNUCASH_CATEGORY_INCOME;
    }
  else 
    {
      categ -> type = GNUCASH_CATEGORY_EXPENSE;
    }

  printf (">>> new categ: %s\n", categ -> name );

  gnucash_categories = g_slist_append ( gnucash_categories, categ );
}



void recuperation_donnees_gnucash_transaction ( xmlNodePtr transaction_node )
{
  struct struct_ope_importation * transaction;
  struct struct_compte_importation * account;
  struct gnucash_category * categ;
  gchar * date, *space;
  xmlNodePtr splits, split_node, date_node;

  /* Basic properties */
  transaction = calloc ( 1, sizeof ( struct struct_ope_importation ));
  transaction -> id_operation = child_content ( transaction_node, "id" );
  transaction -> tiers = child_content ( transaction_node, "description" );
  

  /* Transaction date */
  date_node = get_child ( transaction_node, "date-posted" );
  transaction -> date = g_date_new ( );
  date = child_content (date_node, "date");
  space = strchr(date, ' ');
  if ( space )
    *space = 0;
  g_date_set_parse ( transaction -> date, date );
  if ( !g_date_valid ( transaction -> date ))
    fprintf (stderr, "grisbi: Can't parse date %s\n", child_content (date_node, "date"));

  /* Transaction ammount, category, account, etc.. */
  splits = get_child ( transaction_node, "splits" );
  split_node = splits -> children;
  while ( split_node )
    {
      if ( node_strcmp ( split_node, "split") )
	{
	  account = find_imported_account_by_uid ( child_content(split_node, "account") );
	  if ( account )
	    {
	      account -> operations_importees = 
		g_slist_append ( account -> operations_importees, transaction );
	      transaction -> montant = gnucash_value ( child_content(split_node, "value") );
	      transaction -> notes = child_content(split_node, "memo");
	      if ( !strcmp(child_content(split_node, "reconciled-state"), "y") )
		transaction -> p_r = OPERATION_RAPPROCHEE;
	    }
	  else 
	    {
	      /* This is a category, then */
	      categ = find_imported_categ_by_uid ( child_content(split_node, "account") );
	      transaction -> categ = categ -> name;
	    }
	}

      split_node = split_node -> next;
    }
}



gchar * get_currency ( xmlNodePtr currency_node )
{
  /** FIXME: detail how it works */
  return child_content ( currency_node, "id" );
}



gboolean node_strcmp ( xmlNodePtr node, gchar * name )
{
  return node -> name && !strcmp ( node -> name, name );
}



xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name )
{
  xmlNodePtr iter_node = node -> children;

  if (!node) return NULL;

  while ( iter_node )
    {
      if ( node_strcmp(iter_node, child_name) )
	return iter_node;

      iter_node = iter_node -> next;
    }

  return NULL;
}



gchar * child_content ( xmlNodePtr node, gchar * child_name )
{
  xmlNodePtr child_node;

  if (!node) return NULL;

  child_node = get_child ( node, child_name );

  if ( child_node )
    return xmlNodeGetContent ( child_node );
  
  return NULL;
}



struct struct_compte_importation * find_imported_account_by_uid ( gchar * guid )
{
  GSList * liste_tmp;

  liste_tmp = liste_comptes_importes;

  while ( liste_tmp )
    {
      struct struct_compte_importation * account = liste_tmp -> data;

      if ( !strcmp ( account -> guid, guid ))
	{
	  return account;
	}

      liste_tmp = liste_tmp -> next;
    }  

  return NULL;
}



struct gnucash_category * find_imported_categ_by_uid ( gchar * guid )
{
  GSList * liste_tmp;

  liste_tmp = gnucash_categories;

  while ( liste_tmp )
    {
      struct gnucash_category * categ = liste_tmp -> data;

      if ( !strcmp ( categ -> guid, guid ))
	{
	  return categ;
	}

      liste_tmp = liste_tmp -> next;
    }  

  return NULL;
}



gdouble gnucash_value ( gchar * value )
{
  gchar **tab_value;
  gdouble number, mantisse;

  tab_value = g_strsplit ( value, "/", 2 );
  
  number = my_atoi ( tab_value[0] );
  mantisse = my_atoi ( tab_value[1] );

  return number / mantisse;
}
