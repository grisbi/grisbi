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


/*START_INCLUDE*/
#include "gnucash.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
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
struct gnucash_split * find_split ( GSList * split_list, gdouble amount, 
				    struct struct_compte_importation * account, 
				    struct gnucash_category * categ );
void update_split ( struct gnucash_split * split, gdouble amount, gchar * account, 
		    gchar * categ );
struct gnucash_split * new_split ( gdouble amount, gchar * account, gchar * categ );
struct struct_compte_importation * find_imported_account_by_uid ( gchar * guid );
struct struct_compte_importation * find_imported_account_by_name ( gchar * name );
struct struct_ope_importation * new_transaction_from_split ( struct gnucash_split * split,
							     gchar * tiers, GDate * date );
/*END_STATIC*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern gchar * tempname;
/*END_EXTERN*/

/* Structures */
struct gnucash_category {
  gchar * name;
  enum gnucash_category_type {
    GNUCASH_CATEGORY_INCOME,
    GNUCASH_CATEGORY_EXPENSE,
  } type;
  gchar * guid;
};

struct gnucash_split {
  gdouble amount;
  gchar * category;
  gchar * account;
  gchar * contra_account;
  gchar * notes;
  enum operation_etat_rapprochement p_r;
};

/* Variables */
GSList * gnucash_categories = NULL;


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
		   strcmp(type, "EQUITY") )
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
  struct gnucash_split * split;
  gchar * date_string, *space, *tiers;
  GDate * date;
  xmlNodePtr splits, split_node, date_node;
  GSList * split_list = NULL;
  gdouble total = 0;

  /* Transaction amount, category, account, etc.. */
  splits = get_child ( transaction_node, "splits" );
  split_node = splits -> children;

  while ( split_node )
    {
      struct struct_compte_importation * split_account = NULL; 
      struct gnucash_category * categ = NULL;
      struct gnucash_split * split;
      enum operation_etat_rapprochement p_r;
      gdouble amount;

      if ( node_strcmp ( split_node, "split" ) )
	{
	  gchar * account_name = NULL, * categ_name = NULL;

	  split_account = find_imported_account_by_uid ( child_content ( split_node, 
									"account" ) );
	  categ = find_imported_categ_by_uid ( child_content ( split_node, "account" ) );
	  amount = gnucash_value ( child_content(split_node, "value") );

	  if ( categ ) 
	    categ_name = categ -> name;
	  if ( split_account )
	    {
	      /* All of this stuff is here since we are dealing with
		 the account split, not the category one */
	      account_name = split_account -> nom_de_compte;
	      total += amount;
	      if ( !strcmp(child_content(split_node, "reconciled-state"), "y") )
		p_r = OPERATION_RAPPROCHEE;
	      else
		p_r = OPERATION_NORMALE;
	    }

	  split = find_split ( split_list, amount, split_account, categ );
	  if ( split )
	    {
	      update_split ( split, amount, account_name, categ_name );
	    }
	  else
	    {
	      split = new_split ( amount, account_name, categ_name );
	      split_list = g_slist_append ( split_list, split );
	      split -> notes = child_content(split_node, "memo");
	      split -> p_r = p_r;
	    }
	}

      split_node = split_node -> next;
    }

  if ( ! split_list )
    return;

  /* Transaction date */
  date_node = get_child ( transaction_node, "date-posted" );
  date_string = child_content (date_node, "date");
  space = strchr ( date_string, ' ' );
  if ( space )
    *space = 0;
  date = g_date_new ();
  g_date_set_parse ( date, date_string );
  if ( !g_date_valid ( date ))
    fprintf ( stderr, "grisbi: Can't parse date %s\n", date_string );

  /* Tiers */
  tiers = child_content ( transaction_node, "description" );

  /* Create transaction */
  split = split_list -> data;
  transaction = new_transaction_from_split ( split, tiers, date );
  transaction -> operation_ventilee = 0;
  transaction -> ope_de_ventilation = 0;
  account = find_imported_account_by_name ( split -> account );
  if ( account )
    account -> operations_importees = g_slist_append ( account -> operations_importees, transaction );

  if ( g_slist_length ( split_list ) > 1 )
    {
      transaction -> operation_ventilee = 1;
      transaction -> montant = total;

      while ( split_list )
	{
	  struct gnucash_split * split = split_list -> data;
	  struct struct_compte_importation * account = NULL; 
	  
	  transaction = new_transaction_from_split ( split, tiers, date );
	  transaction -> ope_de_ventilation = 1;

	  account = find_imported_account_by_name ( split -> account );
	  if ( account )
	    account -> operations_importees = g_slist_append ( account -> operations_importees, transaction );
      
	  split_list = split_list -> next;
	}
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

  if ( ! guid )
    return NULL;

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



struct struct_compte_importation * find_imported_account_by_name ( gchar * name )
{
  GSList * liste_tmp;

  if ( ! name )
    return NULL;

  liste_tmp = liste_comptes_importes;

  while ( liste_tmp )
    {
      struct struct_compte_importation * account = liste_tmp -> data;

      if ( !strcmp ( account -> nom_de_compte, name ))
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
	  gchar * ns[14] = { "gnc", "cd", "book", "act", "trn", "split", "cmdty", 
			     "ts", "slots", "slot", "price", "sx", "fs", NULL };
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



struct gnucash_split * find_split ( GSList * split_list, gdouble amount, 
				    struct struct_compte_importation * account, 
				    struct gnucash_category * categ )
{
  GSList * tmp;
  
  tmp = split_list;
  while ( tmp )
    {
      struct gnucash_split * split = tmp -> data;
      if ( amount == - split -> amount &&
	   ! ( split -> account && split -> category ) &&
	   ! ( split -> category && categ ) )
	{
	  return split;
	}

      tmp = tmp -> next;
    }
  
  return NULL;
}



void update_split ( struct gnucash_split * split, gdouble amount, 
		    gchar * account, gchar * categ )
{
  if ( categ )
    {
      split -> category = g_strdup ( categ );
    }

  if ( account )
    {
      if ( !split -> account )
	{
	  split -> account = g_strdup ( account );
	  split -> amount = amount;
	}
      else
	{
	  split -> contra_account = g_strdup ( account );
	}
    }
}



struct gnucash_split * new_split ( gdouble amount, gchar * account, gchar * categ )
{
  struct gnucash_split * split;

  split = calloc ( 1, sizeof ( struct gnucash_split ));

  split -> amount = amount;
  if ( account )
    split -> account = g_strdup ( account );
  else 
    split -> account = NULL;
  if ( categ )
    split -> category = g_strdup ( categ );
  else 
    split -> category = NULL;

  return split;
}



struct struct_ope_importation * new_transaction_from_split ( struct gnucash_split * split,
							     gchar * tiers, GDate * date )
{
  struct struct_ope_importation * transaction;

  /* Basic properties */
  transaction = calloc ( 1, sizeof ( struct struct_ope_importation ));
  transaction -> montant = split -> amount;
  transaction -> notes = split -> notes;
  transaction -> p_r = split -> p_r;
  transaction -> tiers = tiers;
  transaction -> date = date;

  if ( split -> contra_account )
    {
      /* First split was about a real account, this is then a transfer. */
      struct struct_compte_importation * contra_account;
      struct struct_ope_importation * contra_transaction;
	  
      contra_transaction = calloc ( 1, sizeof ( struct struct_ope_importation ));
      contra_transaction -> montant = -split -> amount;
      contra_transaction -> notes = split -> notes;
      contra_transaction -> tiers = tiers;
      contra_transaction -> date = date;

      transaction -> categ = g_strconcat ( _("Transfer"), " : ",
					   split -> contra_account, NULL);
      contra_transaction -> categ = g_strconcat ( _("Transfer"), " : ",
						  split -> account, NULL);

      contra_account = find_imported_account_by_name ( split -> contra_account );
      if ( contra_account )
	contra_account -> operations_importees = g_slist_append ( contra_account -> operations_importees, contra_transaction );
    }
  else
    {
      transaction -> categ = split -> category;
    }

  return transaction;
}
