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


#define START_INCLUDE
#include "gnucash.h"
#include "utils.h"
#define END_INCLUDE

#define START_STATIC
static gchar * child_content ( xmlNodePtr node, gchar * child_name );
static struct struct_compte_importation * find_imported_account_by_uid ( gchar * guid );
static struct gnucash_category * find_imported_categ_by_uid ( gchar * guid );
static xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name );
static gchar * get_currency ( xmlNodePtr currency_node );
static gdouble gnucash_value ( gchar * value );
static gboolean node_strcmp ( xmlNodePtr node, gchar * name );
static xmlDocPtr parse_gnucash_file ( gchar * filename );
static void recuperation_donnees_gnucash_book ( xmlNodePtr book_node );
static void recuperation_donnees_gnucash_categorie ( xmlNodePtr categ_node );
static void recuperation_donnees_gnucash_compte ( xmlNodePtr compte_node );
static void recuperation_donnees_gnucash_transaction ( xmlNodePtr transaction_node );
#define END_STATIC



struct gnucash_category {
  gchar * name;
  enum gnucash_category_type {
    GNUCASH_CATEGORY_INCOME,
    GNUCASH_CATEGORY_EXPENSE,
  } type;
  gchar * guid;
};
GSList * gnucash_categories = NULL;

#define START_EXTERN
extern GSList *liste_comptes_importes;
extern gchar * tempname;
#define END_EXTERN


gboolean recuperation_donnees_gnucash ( gchar * filename )
{
  xmlDocPtr doc;

  doc = parse_gnucash_file ( filename );

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
	  else if ( node_strcmp(root_node, "account") )
	    {
	      gchar * type = child_content(root_node, "type");
	      if ( strcmp(type, "INCOME") && strcmp(type, "EXPENSES") &&
		   strcmp(type, "EQUITY"),  )
		{
		  /* Ce n'est pas un compte de catégories */
		  recuperation_donnees_gnucash_compte ( root_node );
		}
	      else
		{
		  recuperation_donnees_gnucash_categorie ( root_node );
		}
	    }
	  else if ( node_strcmp(root_node, "transaction") )
	    {
	      recuperation_donnees_gnucash_transaction ( root_node );
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
  gchar * type = child_content ( compte_node, "type" );


  compte = calloc ( 1, sizeof ( struct struct_compte_importation ));

  /* Gnucash import */
  compte -> origine = GNUCASH_IMPORT;

  if ( !strcmp(type, "BANK") || !strcmp(type, "CREDIT") ) 
    {
      compte -> type_de_compte = 0; /* Bank */
    }
  else if ( !strcmp(type, "CASH") || !strcmp(type, "CURRENCY") ) 
    {
      compte -> type_de_compte = 1; /* Currency */
    }
  else if ( !strcmp(type, "ASSET") || !strcmp(type, "STOCK") || !strcmp(type, "MUTUAL") ) 
    {
      compte -> type_de_compte = 0; /* Asset */
    }
  else if ( !strcmp(type, "LIABILITY") ) 
    {
      compte -> type_de_compte = 0; /* Liability */
    }

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

  /* Find name, could be tricky if there is a parent. */
  categ -> name = child_content ( categ_node, "name" );
  if ( child_content ( categ_node, "parent" ) )
    {
      gchar * parent_guid = child_content ( categ_node, "parent" );
      GSList * liste_tmp = gnucash_categories;

      while ( liste_tmp )
	{
	  struct gnucash_category * iter = liste_tmp -> data;

	  if ( !strcmp ( iter -> guid, parent_guid ) )
	    {
	      categ -> name = g_strconcat ( iter -> name, " : ", categ -> name, NULL );
	      break;
	    }

	  liste_tmp = liste_tmp -> next;
	}
    }

  categ -> guid = child_content ( categ_node, "id" );
  
  if ( !strcmp ( child_content ( categ_node, "type" ), "INCOME" ) )
    {
      categ -> type = GNUCASH_CATEGORY_INCOME;
    }
  else 
    {
      categ -> type = GNUCASH_CATEGORY_EXPENSE;
    }

  gnucash_categories = g_slist_append ( gnucash_categories, categ );
}



void recuperation_donnees_gnucash_transaction ( xmlNodePtr transaction_node )
{
  struct struct_ope_importation * transaction;
  struct struct_compte_importation * account = NULL;
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
      struct struct_compte_importation * split_account = NULL; 
      if ( node_strcmp ( split_node, "split" ) )
	{
	  split_account = find_imported_account_by_uid (child_content(split_node, "account"));
	  if ( split_account )
	    {			
	      /* This split is about a real account. */
	      if ( account )
		{
		  /* First split was about a real account, this is then a transfer. */
		  struct struct_compte_importation * contra_account;
		  struct struct_ope_importation * contra_transaction;

		  contra_account = split_account;
		  contra_transaction = calloc ( 1, sizeof ( struct struct_ope_importation ));
		  contra_transaction -> montant = gnucash_value ( child_content(split_node, "value") );
		  printf (">>> amount : %f\n", contra_transaction -> montant);
		  contra_transaction -> notes = child_content(split_node, "memo");
		  if ( !strcmp(child_content(split_node, "reconciled-state"), "y") )
		    contra_transaction -> p_r = OPERATION_RAPPROCHEE;

		  transaction -> categ = g_strconcat ( _("Transfer"), " : ", 
						       contra_account -> nom_de_compte, NULL);
		  contra_transaction -> categ = g_strconcat ( _("Transfer"), " : ", 
							      account -> nom_de_compte, NULL);
		  contra_transaction -> date = transaction -> date;

		  contra_account -> operations_importees = g_slist_append ( contra_account -> operations_importees, contra_transaction );
		}
	      else 
		{
		  /* This is the first split */
		  account = split_account;
		  account -> operations_importees = g_slist_append ( account -> operations_importees, transaction );
		  transaction -> montant = gnucash_value ( child_content(split_node, "value") );
		  transaction -> notes = child_content(split_node, "memo");
		  if ( !strcmp(child_content(split_node, "reconciled-state"), "y") )
		    transaction -> p_r = OPERATION_RAPPROCHEE;
		}
	    }
	  else
	    {
	      /* This is a category split */
	      categ = find_imported_categ_by_uid ( child_content(split_node, "account") );
	      if ( categ )
		{
		  /* This is a normal transaction with a category */
		  transaction -> categ = categ -> name;
		}
	      else
		{
		  /* Something is wrong. */
		}
	    }
	}

      split_node = split_node -> next;
    }
}



gchar * get_currency ( xmlNodePtr currency_node )
{
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


xmlDocPtr parse_gnucash_file ( gchar * filename )
{
  gchar buffer[1024], *tempname;
  FILE * filein, * tempfile;
  xmlDocPtr doc;

  filein = fopen ( filename, "r" );
  if ( ! filein )
    {
      fprintf (stderr, "ERROR");
      return NULL;
    }

  tempname = g_strdup_printf ( "gsbgnc%05d", g_random_int_range (0,99999) );
  tempfile = fopen ( tempname, "w+x" );
  if ( ! tempfile )
    {
      fprintf (stderr, "ERROR");
      return NULL;
    }

  while ( fgets ( buffer, 1024, filein ) )
    {
      gchar * tag;
      tag = g_strrstr ( buffer, "<gnc-v2>" );
      
      if ( tag )
	{
	  gchar * ns[12] = { "gnc", "cd", "book", "act", "trn", "split", "cmdty", 
			     "ts", "slots", "slot", "price", NULL };
	  gchar ** iter;
	  /* We need to fix the file */
	  tag += 7;
	  *tag = 0;
	  tag++;

	  fputs ( buffer, tempfile );
	  for ( iter = ns ; *iter != NULL ; iter++ )
	    {
	      fputs ( g_strdup_printf ( "  xmlns:%s=\"http://www.gnucash.org/lxr/gnucash/source/src/doc/xml/%s-v1.dtd#%s\"\n", 
					*iter, *iter, *iter ),
		      tempfile );
	    }
	  fputs ( ">\n", tempfile );
	}
      else
	{
	  fputs ( buffer, tempfile );
	}
    }
  fclose ( filein );
  fclose ( tempfile );

  doc = xmlParseFile ( tempname );

  printf ("unlink ( tempname );\n");
  
  return doc;
}
