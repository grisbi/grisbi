/* Fichier devises.c */
/* s'occupe de tout ce qui concerne les devises */

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
#include "structures.h"
#include "variables-extern.c"
#include "en_tete.h"


GtkWidget *entree_nom, *entree_code, *entree_iso_code;

static struct iso_4217_currency iso_4217_currencies[] = {
  { "ET", "Afrique", "Birr", "Ethiopie", "ETB", NULL },
  { "GH", "Afrique", "Cedi", "Ghana", "GHC", NULL },
  { "GM", "Afrique", "Dalasie", "Gambie", "GMD", NULL },
  { "DZ", "Afrique", "Dinar algérien", "Algérie", "DZD", NULL },
  { "LY", "Afrique", "Dinar Iybien", "Lybie", "LYD", NULL },
  { "TN", "Afrique", "Dinar tunisien", "Tunisie", "TND", NULL },
  { "MA", "Afrique", "Dirham marocain", "Maroc", "MAD", NULL },
  { "ST", "Afrique", "Dobra", "Sâo Tomé - Principe", "STD", NULL },
  { "LR", "Afrique", "Dollar libérien", "Libéria", "LRD", NULL },
  { "GQ", "Afrique", "Ekwele", "Guinée Equatoriale", "GQE", NULL },
  { "NE", "Afrique", "Franc CFA (BCEAO)", "Niger", "XOF", NULL },
  { "SN", "Afrique", "Franc CFA (BCEAO)", "Senegal", "XOF", NULL },
  { "CM", "Afrique", "Franc CFA (BEAC)", "Cameroun", "XAF", NULL },
  { "CG", "Afrique", "Franc CFA (BEAC)", "Congo", "XAF", NULL },
  { "TD", "Afrique", "Franc CFA (BEAC)", "Tchad", "XAF", NULL },
  { "DJ", "Afrique", "Franc de Djibouti", "Djibouti", "DIF", NULL },
  { "KM", "Afrique", "Franc des Comores", "Comores", "KMF", NULL },
  { "BI", "Afrique", "Franc du Burundi", "Burundi", "BIF", NULL },
  { "RW", "Afrique", "Franc du Rwanda", "Rwanda", "RWF", NULL },
  { "MG", "Afrique", "Franc malgache", "Madagascar", "MGF", NULL },
  { "ML", "Afrique", "Franc malien", "Mali", "MLF", NULL },
  { "MW", "Afrique", "Kwacha", "Malawi", "MWK", NULL },
  { "ZM", "Afrique", "Kwacha", "Zambie", "ZMK", NULL },
  { "AO", "Afrique", "Kwanza", "Angola", "AOK", NULL },
  { "SL", "Afrique", "Leone", "Sierra Leone", "SLL", NULL },
  { "SZ", "Afrique", "Lilangeni", "Swaziland", "SZL", NULL },
  { "EG", "Afrique", "Livre égyptienne", "Egypte", "EGP", NULL },
  { "SD", "Afrique", "Livre soudanaise", "Soudan", "SDP", NULL },
  { "MZ", "Afrique", "Metical", "Mozambique", "MZM", NULL },
  { "NG", "Afrique", "Naira", "Nigeria", "NGN", NULL },
  { "MR", "Afrique", "Ouguija", "Mauritanie", "MRO", NULL },
  { "GW", "Afrique", "Peso de Guinée-Bissau", "Guinée-Bissau", "GWP", NULL },
  { "BW", "Afrique", "Pula", "Botswana", "BWP", NULL },
  { "ZA", "Afrique", "Rand", "Afrique du Sud", "ZAR", NULL },
  { "LS", "Afrique", "Rand", "Lesotho", "ZAR", NULL },
  { "NA", "Afrique", "Rand", "Namibie", "ZAR", NULL },
  { "SC", "Afrique", "Roupie des Seychelles", "Seychelles", "SCR", NULL },
  { "SO", "Afrique", "Shilling de Somalie", "Somalie", "SOS", NULL },
  { "TZ", "Afrique", "Shilling de Tanzanie", "Tanzanie", "TZS", NULL },
  { "KE", "Afrique", "Shilling du Kenya", "Kenya", "KES", NULL },
  { "UG", "Afrique", "Shilling ougandais", "Ouganda", "UGS", NULL },
  { "GN", "Afrique", "Syli", "Guinée", "GNS", NULL },
  { "ZW", "Afrique", "Zimbabwe dollar", "Zimbabwe", "ZWD", NULL },
  { "PA", "Amérique centrale", "Balboa", "Panama", "PAB", NULL },
  { "CR", "Amérique centrale", "Colon", "Costa Rica", "CRC", NULL },
  { "NI", "Amérique centrale", "Cordoba", "Nicaragua", "NIC", NULL },
  { "BZ", "Amérique centrale", "Dollar de Belize", "Belize", "BZD", NULL },
  { "HN", "Amérique centrale", "Lempira", "Honduras", "HNL", NULL },
  { "GT", "Amérique centrale", "Quetzal", "Guatemala", "GTQ", NULL },
  { "CA", "Amérique du Nord", "Dollar canadien", "Canada", "CAD", NULL },
  { "BB", "Amérique du Nord", "Dollar de Barbade", "Barbade", "BBD", NULL },
  { "TT", "Amérique du Nord", "Dollar de la Trinité", "Trinité-et-Tobago", "TTD", NULL },
  { "BS", "Amérique du Nord", "Dollar des Bahamas", "Bahamas", "BSD", NULL },
  { "BM", "Amérique du Nord", "Dollar des Bermudes", "Berrnudes", "BMD", NULL },
  { "KY", "Amérique du Nord", "Dollar des Caimanes", "Iles Caimanes", "KYD", NULL },
  { "GD", "Amérique du Nord", "Dollar des Caraïbes", "Grenade", "XCD", NULL },
  { "LC", "Amérique du Nord", "Dollar des Caraïbes", "St. Lucia", "XCD", NULL },
  { "US", "Amérique du Nord", "Dollar des Etats-Unis", "Etats-Unis", "USD", "$" },
  { "JM", "Amérique du Nord", "Dollar jamaicain", "Jamaique", "JMD", NULL },
  { "AN", "Amérique du Nord", "Florin des Antilles néerl.", "Curaçao e.a.", "ANG", NULL },
  { "GP", "Amérique du Nord", "Franc français", "Guadeloupe", "FRF", NULL },
  { "MQ", "Amérique du Nord", "Franc français", "Martinique e.a.", "FRF", NULL },
  { "HT", "Amérique du Nord", "Gourde", "Haïti", "HTG", NULL },
  { "CU", "Amérique du Nord", "Peso cubain", "Cuba", "CUP", NULL },
  { "DO", "Amérique du Nord", "Peso dominicain", "Rép.  Dominicaine", "DOP", NULL },
  { "MX", "Amérique du Nord", "Peso mexicain", "Mexique", "MXP", NULL },
  { "AR", "Amérique du Sud", "Austral", "Argentine", "ARA", NULL },
  { "VE", "Amérique du Sud", "Bolivar", "Vénézuela", "VEB", NULL },
  { "BR", "Amérique du Sud", "Cruzeiro", "Brésil", "BRC", NULL },
  { "GY", "Amérique du Sud", "Dollar de Guyane", "Guyane", "GYD", NULL },
  { "SR", "Amérique du Sud", "Florin de Surinam", "Surinam", "SRG", NULL },
  { "PY", "Amérique du Sud", "Guarani", "Paraguay", "PYG", NULL },
  { "AR", "Amérique du Sud", "Peso argentin", "Argentine", "ARP", NULL },
  { "BO", "Amérique du Sud", "Peso bolivien", "Bolivie", "BOP", NULL },
  { "CL", "Amérique du Sud", "Peso chilien", "Chili", "CLP", NULL },
  { "CO", "Amérique du Sud", "Peso colombien", "Colombie", "COP", NULL },
  { "UY", "Amérique du Sud", "Peso uruguayen", "Uruguay", "UYP", NULL },
  { "PE", "Amérique du Sud", "Sol", "Pérou", "PES", NULL },
  { "EC", "Amérique du Sud", "Sucre", "Equateur", "ECS", NULL },
  { "AF", "Asie", "Afghani", "Afghanistan", "AFA", NULL },
  { "TH", "Asie", "Baht", "Thailande", "THB", NULL },
  { "BH", "Asie", "Dinar de Bahrein", "Bahrein", "BHD", NULL },
  { "IQ", "Asie", "Dinar irakien", "Irak", "IQD", NULL },
  { "JO", "Asie", "Dinar jordanien", "Jordanie", "JOD", NULL },
  { "KW", "Asie", "Dinar koweitien", "Koweit", "KWD", NULL },
  { "AE", "Asie", "Dirham des E A.U.", "Emirats Arabes Unis", "AED", NULL },
  { "BN", "Asie", "Dollar de Brunei", "Brunei", "BND", NULL },
  { "HK", "Asie", "Dollar de Hong Kong", "Hong Kong", "HKD", NULL },
  { "SG", "Asie", "Dollar de Singapour", "Singapour", "SGD", NULL },
  { "TW", "Asie", "Dollar taïwanien", "Taïwan", "TWD", NULL },
  { "VN", "Asie", "Dong", "Viêt-nam", "VND", NULL },
  { "LA", "Asie", "Kip", "Laos", "LAK", NULL },
  { "CY", "Asie", "Livre cypriote", "Chypre", "CYP", NULL },
  { "LB", "Asie", "Livre libanaise", "Liban", "LBP", NULL },
  { "SY", "Asie", "Livre syrienne", "Syrie", "SYP", NULL },
  { "TR", "Asie", "Livre turque", "Turquie", "TRL", NULL },
  { "MO", "Asie", "Pataca", "Macao", "MOP", NULL },
  { "PH", "Asie", "Peso philippin", "Philippines", "PHP", NULL },
  { "QA", "Asie", "Rial du Qatar", "Qatar", "QAR", NULL },
  { "IR", "Asie", "Rial iranien", "Iran", "IRR", NULL },
  { "OM", "Asie", "Rial omani", "Oman", "OMR", NULL },
  { "KH", "Asie", "Riel", "Cambodge", "KHR", NULL },
  { "MY", "Asie", "Ringgit", "Malaisie", "MYR", NULL },
  { "YE", "Asie", "Riyal du Yémen", "Yémen du Nord (Rép. arab.)", "YER", NULL },
  { "SA", "Asie", "Riyal saoudien", "Arabie Saoudite", "SAR", NULL },
  { "RU", "Asie", "Rouble de Russie", "Russie", "RUR", NULL },
  { "LK", "Asie", "Roupie de Sri Lanka", "Sri Lanka", "LKR", NULL },
  { "NP", "Asie", "Roupie du Népal", "Népal", "NPR", NULL },
  { "PK", "Asie", "Roupie du Pakistan", "Pakistan", "PKR", NULL },
  { "BT", "Asie", "Roupie indienne", "Buthan", "INR", NULL },
  { "IN", "Asie", "Roupie indienne", "Inde", "INR", NULL },
  { "ID", "Asie", "Rupiah", "Indonésie", "IDR", NULL },
  { "IL", "Asie", "Shékel", "Israël", "ILS", NULL },
  { "BD", "Asie", "Taka", "Bangladesh", "BDT", NULL },
  { "MN", "Asie", "Tugrik", "Mongolie", "MNT", NULL },
  { "KR", "Asie", "Won", "Corée du Sud", "KRW", NULL },
  { "KP", "Asie", "Won nord-coréen", "Corée du Nord", "KPW", NULL },
  { "JP", "Asie", "Yen", "Japon", "JPY", "¥" },
  { "CN", "Asie", "Yuan Ren-min-bi", "Chine", "CNY", NULL },
  { "DK", "Europe", "Couronne danoise", "Danemark", "DKK", NULL },
  { "IS", "Europe", "Couronne islandaise", "Islande", "ISK", NULL },
  { "NO", "Europe", "Couronne norvégienne", "Norvège", "NOK", NULL },
  { "SK", "Europe", "Couronne slovaque", "Slovaquie", "SKK", NULL },
  { "SE", "Europe", "Couronne suédoise", "Suède", "SEK", NULL },
  { "CZ", "Europe", "Couronne tchèque", "Tchéquie", "CZK", NULL },
  { "DE", "Europe", "Deutsche mark", "Allemagne", "DEM", NULL },
  { "HR", "Europe", "Dinar croate", "Croatie", "HRD", NULL },
  { "GR", "Europe", "Drachme", "Grèce", "GRD", NULL },
  { "PT", "Europe", "Escudo portugais", "Portugal (Açores-Madère)", "PTE", NULL },
  { "NL", "Europe", "Florin néerlandais", "Pays-Bas", "NLG", NULL },
  { "HU", "Europe", "Forint", "Hongrie", "HUF", NULL },
  { "BE", "Europe", "Franc belge", "Belgique", "BEF", NULL },
  { "FR", "Europe", "Franc français", "France", "FRF", NULL },
  { "LU", "Europe", "Franc luxembourgeois", "Luxembourg", "LUF", NULL },
  { "LI", "Europe", "Franc suisse", "Liechtenstein", "CHF", NULL },
  { "CH", "Europe", "Franc suisse", "Suisse", "CHF", NULL },
  { "AL", "Europe", "Lek", "Albanie", "ALL", NULL },
  { "RO", "Europe", "Leu", "Roumanie", "ROL", NULL },
  { "BG", "Europe", "Lev", "Bulgarie", "BGL", NULL },
  { "IT", "Europe", "Lire italienne", "Italie", "ITL", NULL },
  { "SM", "Europe", "Lire italienne", "San Marino", "ITL", NULL },
  { "VA", "Europe", "Lire italienne", "Vatican (St Siège)", "ITL", NULL },
  { "GI", "Europe", "Livre de Gibraltar", "Gibraltar", "GIP", NULL },
  { "IE", "Europe", "Livre irlandaise", "Irlande", "IEP", NULL },
  { "MT", "Europe", "Livre maltaise", "Malte", "MTP", NULL },
  { "GB", "Europe", "Livre sterling", "Royaume-Uni", "GBP", "£" },
  { "FI", "Europe", "Mark finlandais", "Finlande", "FIM", NULL },
  { "SI", "Europe", "Monnaie slovène", "Slovénie", "SIT", NULL },
  { "YU", "Europe", "Nouveau dinar yougoslave", "Yougoslavie", "YUD", NULL },
  { "ES", "Europe", "Peseta espagnole", "Espagne", "ESP", NULL },
  { "AT", "Europe", "Schilling autrichien", "Autriche", "ATS", NULL },
  { "PL", "Europe", "Zloty", "Pologne", "PLZ", NULL },
  { "AU", "Océanie", "Dollar australien", "Australie", "AUD", NULL },
  { "KI", "Océanie", "Dollar australien", "Kiribati", "AUD", NULL },
  { "NR", "Océanie", "Dollar australien", "Nauru", "AUD", NULL },
  { "TV", "Océanie", "Dollar australien", "Tuvalu", "AUD", NULL },
  { "FJ", "Océanie", "Dollar de Fidji", "Fidji", "FJD", NULL },
  { "SB", "Océanie", "Dollar de Salomon", "Iles Salomon", "SBD", NULL },
  { "CK", "Océanie", "Dollar néo-zélandais", "Cook Islands", "NZD", NULL },
  { "NZ", "Océanie", "Dollar néo-zélandais", "Nouvelle-Zélande", "NZD", NULL },
  { "TP", "Océanie", "Escudo de Timor", "Timor", "TPE", NULL },
  { "WF", "Océanie", "Franc CFP", "Iles Wallis &amp; Futuna", "XPF", NULL },
  { "NC", "Océanie", "Franc CFP", "Nouvelle-Calédonie", "XPF", NULL },
  { "PA", "Océanie", "Dollar des Etats-Unis", "Océanie américaine", "USD", "$" },
  { "PF", "Océanie", "Franc CFP", "Polynésie française", "XPF", NULL },
  { "PG", "Océanie", "Kina", "Papouasie - Nouvelle-Guinée", "PGK", NULL },
  { "TO", "Océanie", "Pa'anga", "Tonga", "TOP", NULL },
  { "WS", "Océanie", "Tala", "Samoa occidental", "WST", NULL },
  { "VU", "Océanie", "Vatu", "Vanuatu", "VUV", NULL },
  { NULL },
};




gint
sort_tree (GtkTreeModel *model,
	   GtkTreeIter *a,
	   GtkTreeIter *b,
	   gpointer user_data)
{
  GValue value1 = {0, };
  GValue value2 = {0, };
  GValue value3 = {0, };
  GValue value4 = {0, };
  gchar * continent1, * continent2, * country1, * country2;

  gtk_tree_model_get_value (model, a, CONTINENT_NAME_COLUMN, &value1);
  gtk_tree_model_get_value (model, b, CONTINENT_NAME_COLUMN, &value2);
  continent1 = (gchar *) g_value_get_string(&value1);
  continent2 = (gchar *) g_value_get_string(&value2);

  gtk_tree_model_get_value (model, a, COUNTRY_NAME_COLUMN, &value3);
  gtk_tree_model_get_value (model, b, COUNTRY_NAME_COLUMN, &value4);
  country1 = (gchar *) g_value_get_string(&value3);
  country2 = (gchar *) g_value_get_string(&value4);

  if (! strcmp(continent1, continent2))
    {
      return strcmp(country1, country2);
    }
  else
    {
      return strcmp(continent1, continent2);
    }
}


gboolean
select_currency_in_iso_list (GtkTreeSelection *selection,
			     GtkTreeModel *model)
{
  GtkTreeIter iter;
  GValue value1 = {0, };
  GValue value2 = {0, };
  GValue value3 = {0, };

  if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
    return;

  gtk_tree_model_get_value (model, &iter, CURRENCY_NAME_COLUMN, &value1);
  gtk_tree_model_get_value (model, &iter, CURRENCY_ISO_CODE_COLUMN, &value2);
  gtk_tree_model_get_value (model, &iter, CURRENCY_NICKNAME_COLUMN, &value3);

  if (g_value_get_string(&value1))
    gtk_entry_set_text ( GTK_ENTRY ( entree_nom ), 
			 g_value_get_string(&value1) );
  if (g_value_get_string(&value2))
    gtk_entry_set_text ( GTK_ENTRY ( entree_iso_code ), 
			 g_value_get_string(&value2) );
  if (g_value_get_string(&value3))
    gtk_entry_set_text ( GTK_ENTRY ( entree_code ), 
			 g_value_get_string(&value3) );

}


/* ***************************************************************************************************** */
/* Fonction creation_devises_de_base */
/* appelé lors de l'ouverture d'un nouveau fichier ou lors */
/* de l'ouverture de la version 0.2.5 */
/* met en mémoire les devises de base : l'euro et le franc */
/* ***************************************************************************************************** */

void creation_devises_de_base ( void )
{
  struct struct_devise *devise;


  liste_struct_devises = NULL;

  /* création de l'euro */

  devise = malloc ( sizeof ( struct struct_devise ));

  devise -> no_devise = 1;
  devise -> nom_devise= g_strdup ( _("Euro") );
  devise -> code_iso4217_devise = g_strdup ( _("EUR") );
  devise -> code_devise = g_strdup ( _("€") );
  devise -> passage_euro = 0;
  devise -> date_dernier_change = NULL;
  devise -> une_devise_1_egale_x_devise_2 = 0;
  devise -> no_devise_en_rapport = 0;
  devise -> change = 0;

  liste_struct_devises = g_slist_append ( liste_struct_devises,
					  devise );

  nb_devises = 1;
  no_derniere_devise = 1;

}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* Fonction creation_option_menu_devises */
/* retourne le menu d'un option menu avec les différentes devises définies */
/* si devise_cachee = -1, met toutes les devises sous forme de leur sigle */
/* sinon, ne met pas la devise correspondant à devise_cachee ( 0 = aucune ) */
/* liste_tmp est la liste utilisée : soit liste_struct_devises dans le cas général, */
/*                       soit liste_struct_devises_tmp dans le cas des paramètres */
/* **************************************************************************************************** */

GtkWidget *creation_option_menu_devises ( gint devise_cachee,
					  GSList *liste_tmp )
{
  GtkWidget *menu;
  GtkWidget *menu_item;


  menu = gtk_menu_new ();

  if ( devise_cachee > 0 )
    {

      /* le 1er item c'est aucune devise */

      menu_item = gtk_menu_item_new_with_label ( _("None") );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "adr_devise",
			    devise_nulle );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "no_devise",
			    GINT_TO_POINTER ( 0 ) );

      gtk_menu_append ( GTK_MENU ( menu ),
			menu_item );

      gtk_widget_show ( menu_item );
    }


  while ( liste_tmp )
    {
      struct struct_devise *devise;

      devise = liste_tmp -> data;

      if ( devise_cachee != devise -> no_devise )
	{
	  if ( devise_cachee == -1 )
	    menu_item = gtk_menu_item_new_with_label ( devise_name ( devise ) );
	  else
	    menu_item = gtk_menu_item_new_with_label ( g_strconcat ( devise -> nom_devise,
								     " ( ",
								     devise_name ( devise ),
								     " )",
								     NULL ));

	  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				"adr_devise",
				devise );
	  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				"no_devise",
				GINT_TO_POINTER ( devise -> no_devise ) );


	  gtk_menu_append ( GTK_MENU ( menu ),
			    menu_item );

	  gtk_widget_show ( menu_item );
	}
      liste_tmp = liste_tmp -> next;
    }

  gtk_widget_show ( menu );

  return ( menu );

}
/* **************************************************************************************************** */


GtkWidget *
new_currency_list ()
{ 
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkWidget *sw, *treeview;
  GtkTreeStore *model;
  GtkTreeIter iter, child_iter;
  struct iso_4217_currency * currency= iso_4217_currencies;
  gchar ** continent;
  gchar * continents[] = {
    "Afrique",
    "Amérique centrale",
    "Amérique du Nord",
    "Amérique du Sud",
    "Asie",
    "Europe",
    "Océanie",
    NULL,
  };
  gint col_offset;

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_ALWAYS);
  /* create tree store */
  model = gtk_tree_store_new (NUM_CURRENCIES_COLUMNS,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING);

  for (continent = continents; *continent; continent++)
    {
      gtk_tree_store_append (model, &iter, NULL);
      gtk_tree_store_set (model, &iter,
			  COUNTRY_NAME_COLUMN, *continent,
			  CURRENCY_NAME_COLUMN, FALSE,
			  CURRENCY_ISO_CODE_COLUMN, FALSE,
			  CURRENCY_NICKNAME_COLUMN, FALSE,
			  CONTINENT_NAME_COLUMN, *continent,
			  -1);

      while (currency -> country_code && 
	     !strcmp(currency -> continent, *continent))
	{
	  gtk_tree_store_append (model, &child_iter, &iter);
	  gtk_tree_store_set (model, &child_iter,
			      COUNTRY_NAME_COLUMN, currency -> country_name,
			      CURRENCY_NAME_COLUMN, currency -> currency_name,
			      CURRENCY_ISO_CODE_COLUMN, currency -> currency_code,
			      CURRENCY_NICKNAME_COLUMN, currency -> currency_nickname,
			      CONTINENT_NAME_COLUMN, currency -> continent,
			      -1);
	  currency++;
	}
    }

  /* create tree view */
  treeview = gtk_tree_view_new_with_model (model);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), 
		    "changed", 
		    G_CALLBACK (select_currency_in_iso_list),
		    model);

  cell = gtk_cell_renderer_text_new ();
  col_offset = 
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						 -1, _("Country name"),
						 cell, "text",
						 COUNTRY_NAME_COLUMN,
						 NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  cell = gtk_cell_renderer_text_new ();
  col_offset = 
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						 -1, _("Currency name"),
						 cell, "text",
						 CURRENCY_NAME_COLUMN,
						 NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  cell = gtk_cell_renderer_text_new ();
  col_offset = 
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						 -1, _("ISO Code"), 
						 cell, "text",
						 CURRENCY_ISO_CODE_COLUMN,
						 NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  cell = gtk_cell_renderer_text_new ();
  col_offset = 
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						 -1, _("Sign"), 
						 cell, "text",
						 CURRENCY_NICKNAME_COLUMN,
						 NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  /* Sort columns accordingly */
  gtk_tree_sortable_set_default_sort_func (model, sort_tree, NULL, NULL);
  gtk_tree_sortable_set_sort_func (model, COUNTRY_NAME_COLUMN, sort_tree, NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (model, COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

  /* expand all rows after the treeview widget has been realized */
  g_signal_connect (treeview, "realize",
		    G_CALLBACK (gtk_tree_view_expand_all), NULL);

  gtk_widget_set_usize ( treeview, FALSE, 200 );
  gtk_container_add (GTK_CONTAINER (sw), treeview);
  gtk_container_set_resize_mode (GTK_CONTAINER (sw), GTK_RESIZE_PARENT);

  return sw;
}



/***********************************************************************************************************/
/* Fonction ajout_devise */
/* appelée pour créer une nouvelle devise */
/* le widget est soit un option menu si l'appel vient du chargement de la version 0.2.4, dans ce cas on utilise liste_struct_devises */
/* soit c'est la clist des paramètres, dans ce cas on utilise liste_struct_devises_tmp */
/***********************************************************************************************************/

void ajout_devise ( GtkWidget *bouton,
		    GtkWidget *widget )
{
  GtkWidget *dialog, *label, *hbox, *table;
  GtkWidget *check_bouton, *entree_conversion_euro;
  GtkWidget *label_nom_devise, *list, *paddingbox;
  struct struct_devise *devise;
  gchar *nom_devise, *code_devise, *code_iso4217_devise;
  gint resultat;

  dialog = gnome_dialog_new ( _("Add a currency"),
				GNOME_STOCK_BUTTON_OK,
				GNOME_STOCK_BUTTON_CANCEL,
				NULL );
  gtk_container_set_border_width ( GTK_WIDGET ( dialog ), 10 );
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
		        "destroy" );
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "key-press-event",
		       GTK_SIGNAL_FUNC ( bloque_echap_choix_devise ),
		       NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			    0 );

  paddingbox = 
    paddingbox_new_with_title (GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			       _("ISO 4217 currencies"));
  list = new_currency_list ();
  gtk_box_pack_start ( paddingbox , list,
		       FALSE, FALSE, 5 );

  paddingbox = 
    paddingbox_new_with_title (GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			       _("Currency details"));
  
  /* Create table */
  table = gtk_table_new ( 2, 2, FALSE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ),
		       table,
		       TRUE, TRUE, 0 );

  /* Currency name */
  label = gtk_label_new (COLON(_("Currency name")));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( label, GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 
		     0, 1, 0, 1,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );
  entree_nom = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_nom ));
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_nom, 1, 2, 0, 1,
		     GTK_EXPAND|GTK_FILL, 0,
		     0, 0 );

  /* Currency ISO code */
  label = gtk_label_new (COLON(_("Currency ISO 4217 code")));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 
		     0, 1, 1, 2,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );
  entree_iso_code = gtk_entry_new ();
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_iso_code, 1, 2, 1, 2,
		     GTK_EXPAND|GTK_FILL, 0,
		     0, 0 );

  /* Currency usual sign */
  label = gtk_label_new (COLON(_("Currency sign")));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label,
		     0, 1, 2, 3,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );
  entree_code = gtk_entry_new ();
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_code, 1, 2, 2, 3,
		     GTK_EXPAND|GTK_FILL, 0,
		     0, 0 );

  /* Swith to Euro */
  paddingbox = 
    paddingbox_new_with_title (GTK_WIDGET ( GNOME_DIALOG ( dialog ) -> vbox ),
			       _("Euro zone currency"));

  check_bouton = gtk_check_button_new_with_label ( _("Will switch to euro") );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), check_bouton,
		       TRUE, TRUE, 0 );

  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       TRUE, TRUE, 0 );
  label = gtk_label_new ( POSTSPACIFY(_("One Euro equals")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       TRUE, TRUE, 0 );
  entree_conversion_euro = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_conversion_euro ));
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_conversion_euro,
		       TRUE, TRUE, 0 );

  /* Do not activate the "Switch to euro" stuff */
  gtk_widget_set_sensitive (entree_conversion_euro, FALSE );
  gtk_widget_set_sensitive (label, FALSE );
  gtk_signal_connect ( GTK_OBJECT ( check_bouton ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( passe_a_l_euro ),
		       entree_conversion_euro);
  gtk_signal_connect ( GTK_OBJECT ( check_bouton ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( passe_a_l_euro ),
		       label);

  /* Currency name.  This label will be labeled after the entree_nom
     widget. */
  label_nom_devise = gtk_label_new ( _("...") );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label_nom_devise,
		       FALSE, FALSE, 5 );

  gtk_signal_connect ( GTK_OBJECT ( entree_nom ),
		       "changed",
		       GTK_SIGNAL_FUNC ( nom_nouvelle_devise_defini ),
		       label_nom_devise );

  gtk_widget_grab_focus ( entree_nom );
  gtk_editable_set_position ( GTK_EDITABLE ( entree_nom ),
			   0 );
 reprise_dialog:
  gtk_widget_show_all ( GTK_WIDGET ( dialog ) );
  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));


  switch ( resultat )
    {
    case 0 :

      nom_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom ))));
      code_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_code ))));
      code_iso4217_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code ))));
 
      if ( strlen ( nom_devise ) && 
	   (strlen ( code_devise ) ||
	    strlen ( code_iso4217_devise )))
	{
	  GtkWidget *menu;
	  GtkWidget *item;


	  devise = malloc ( sizeof ( struct struct_devise ));
	  devise -> nom_devise = nom_devise;
	  devise -> code_devise = code_devise;
	  devise -> code_iso4217_devise = code_iso4217_devise;
	  devise -> passage_euro = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_bouton ));
	  devise -> no_devise_en_rapport = 0;
	  devise -> date_dernier_change = NULL;
	  devise -> une_devise_1_egale_x_devise_2 = 0;

	  if ( devise -> passage_euro )
	    devise -> change = g_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion_euro ))),
					  NULL );
	  else
	    devise -> change = 0;


	  /* 	  si le widget est une clist, c'est que l'appel vient du menu de configuration, */
	  /* on met la liste à jour et on ajoute la devise à liste_struct_devises_tmp */
	  /* 	    sinon, c'est que c'est le chargement de la version 0.2.5, on met l'option menu et liste_struct_devises à jour */
	  /* à retirer à partir de la 0.2.7 */

	  if ( GTK_IS_CLIST ( widget ))
	    {
	      gchar *ligne[3];
	      gint ligne_liste;

	      devise -> no_devise = ++no_derniere_devise_tmp;
	      liste_struct_devises_tmp = g_slist_append ( liste_struct_devises_tmp,
							  devise );
	      nb_devises_tmp++;

	      ligne[0] = devise -> nom_devise;
	      ligne[1] = devise -> code_iso4217_devise;
	      ligne[2] = devise -> code_devise;
	      
	      ligne_liste = gtk_clist_append ( GTK_CLIST ( widget ),
					       ligne );
	      gtk_clist_set_row_data  ( GTK_CLIST ( widget ),
					ligne_liste,
					devise );

	      /* dégrise appliquer dans paramètres */

	      activer_bouton_appliquer ( );

	    }
	  else
	    {
	      devise -> no_devise = ++no_derniere_devise;
	      liste_struct_devises = g_slist_append ( liste_struct_devises,
						      devise );
	      nb_devises++;

	      menu = gtk_option_menu_get_menu ( GTK_OPTION_MENU ( widget ));
	      
	      item = gtk_menu_item_new_with_label ( g_strconcat ( devise -> nom_devise,
								  " ( ",
								  devise_name ( devise ),
								  " )",
								  NULL ));
	      gtk_object_set_data ( GTK_OBJECT ( item ),
				    "adr_devise",
				    devise );
	      gtk_object_set_data ( GTK_OBJECT ( item ),
				    "no_devise",
				    GINT_TO_POINTER ( devise -> no_devise ) );
	      gtk_menu_append ( GTK_MENU ( menu ),
				item );
	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
					 menu );

	      gtk_widget_show (item );
	    }

	  modification_fichier ( TRUE );
	}
      else
	{
	  dialogue ( _("All fields are not filled in!") );
	  goto reprise_dialog;
	}
      break;

    }
  gnome_dialog_close ( GNOME_DIALOG ( dialog ));
}
/***********************************************************************************************************/



/***********************************************************************************************************/
gint bloque_echap_choix_devise ( GtkWidget *dialog,
				 GdkEventKey *key,
				 gpointer null )
{

  /* empèche la touche echap de fermer la fenetre */

  if ( key -> keyval == 65307 )
    {
      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( dialog ),
				     "key-press-event" );
      return ( TRUE );
    }

  return ( FALSE );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction nom_nouvelle_devise_defini */
/* appelée lorsque l'utilisateur a défini le nouveau nom de la devise */
/* affiche le nom de la devise derrière l'entrée pour la conversion en euro */
/***********************************************************************************************************/

gboolean nom_nouvelle_devise_defini ( GtkWidget *entree,
				      GtkWidget *label )
{
  gchar *nom_devise;

  nom_devise = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )));
  
  if ( nom_devise || !strlen (nom_devise) )
    {
      gtk_label_set_text ( GTK_LABEL ( label ),
			   nom_devise );
    }
  else 
    {
      gtk_label_set_text ( GTK_LABEL ( label ),
			   _("...") );
    }

  return FALSE;
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* Fonction retrait_devise */
/***********************************************************************************************************/

void retrait_devise ( GtkWidget *bouton,
		      GtkWidget *liste )
{
  gint devise_trouvee;
  gint i;
  struct struct_devise *devise;

  if ( ligne_selection_devise == -1 )
    return;


  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );

  /*   recherche dans tous les comptes, les opés et les échéances si la devise n'est pas utilisée */
  /* si elle l'est, empêche sa suppression */

  devise_trouvee = 0;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      if ( DEVISE == devise -> no_devise )
	{
	  devise_trouvee = 1;
	  i = nb_comptes;
	}
      else
	{
	  GSList *liste_tmp;

	  liste_tmp = LISTE_OPERATIONS;

	  while ( liste_tmp )
	    {
	      if ( ((struct structure_operation *)(liste_tmp -> data )) -> devise == devise -> no_devise )
		{
		  devise_trouvee = 1;
		  i = nb_comptes;
		  liste_tmp = NULL;
		}
	      else
		liste_tmp = liste_tmp -> next;
	    }

	  if ( !devise )
	    {
	      liste_tmp = gsliste_echeances;

	      while ( liste_tmp )
		{
		  if ( ((struct operation_echeance *)(liste_tmp -> data )) -> devise == devise -> no_devise )
		    {
		      devise_trouvee = 1;
		      i = nb_comptes;
		      liste_tmp = NULL;
		    }
		  else
		    liste_tmp = liste_tmp -> next;
		}
	    }
	}
      p_tab_nom_de_compte_variable++;
    }


  /* si c'est l'euro qu'on veut supprimer, on interdit */

  if ( !strcmp ( devise -> nom_devise,
		 _("Euro") ))
    devise_trouvee = 1;



  if ( devise_trouvee )
    {
      dialogue ( g_strdup_printf ( _("The %s currency is used in the current account.\nYou can't delete it."),
				   devise -> nom_devise ) ) ;
      return;
    }


  gtk_clist_remove ( GTK_CLIST ( liste ),
		     ligne_selection_devise );

  liste_struct_devises_tmp = g_slist_remove ( liste_struct_devises_tmp,
					      devise );
  nb_devises_tmp--;

  activer_bouton_appliquer ( );

}
/***********************************************************************************************************/




/* ************************************************************************************************************ */
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom )
{

  return ( g_strcasecmp ( g_strstrip ( devise -> nom_devise ),
			  nom ) );

}
/* ************************************************************************************************************ */


/***********************************************************************************************************/
/* Fonction recherche_devise_par_no */
/* appelée par un g_slist_find_custom */
/***********************************************************************************************************/

gint recherche_devise_par_no ( struct struct_devise *devise,
			       gint *no_devise )
{

  return ( devise -> no_devise != GPOINTER_TO_INT ( no_devise ));

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction selection_devise */
/* permet de choisir une devise pour un compte */
/* utilisée quand la devise d'un compte est supprimée, pour le réaffecter */
/* et pour l'importation d'un fichier qif */
/***********************************************************************************************************/

gint selection_devise ( gchar *nom_du_compte )
{
  GtkWidget *dialogue;
  GtkWidget *label;
  GtkWidget *option_menu;
  GtkWidget *bouton;
  GtkWidget *hbox;
  gint resultat;


  dialogue = gnome_dialog_new ( _("Select a currency"),
				GNOME_STOCK_BUTTON_OK,
				NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				 GTK_WINDOW ( window ) );
  gtk_signal_connect ( GTK_OBJECT ( dialogue ),
		       "delete_event",
		       GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( dialogue ),
		       "key-press-event",
		       GTK_SIGNAL_FUNC ( bloque_echap_choix_devise ),
		       NULL );

  label = gtk_label_new ( g_strdup_printf ( _("Please choose a currency for account \"%s\":\n"),
					    nom_du_compte ) );
  
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       label,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  /* met la liste des devises sans "Aucunes" */

  option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ) );

  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       option_menu,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( option_menu );

  bouton = gtk_button_new_with_label ( _("Add a currency") );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( ajout_devise ),
		       option_menu );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  

  gnome_dialog_run ( GNOME_DIALOG ( dialogue ));

  resultat = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
						     "no_devise" ) );
  gnome_dialog_close ( GNOME_DIALOG ( dialogue ));

  return ( resultat );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
gboolean passe_a_l_euro ( GtkWidget *toggle_bouton,
		      GtkWidget *hbox )
{

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( toggle_bouton )))
    gtk_widget_set_sensitive ( hbox,
			       TRUE );
  else
    gtk_widget_set_sensitive ( hbox,
			       FALSE );

  return FALSE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction demande_taux_de_change : */
/* affiche une fenetre permettant d'entrer le taux de change entre la devise du compte et la devise demandée */
/* renvoie ce taux de change */
/* le taux renvoyé est <0 si une_devise_compte_egale_x_devise_ope = 1, > 0 sinon */
/***********************************************************************************************************/

void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise ,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *entree;
  GtkWidget *menu;
  GtkWidget *item;
  GtkWidget *hbox;
  gint resultat;
  GtkWidget *entree_frais;
  

  dialog = gnome_dialog_new ( _("Entry of the exchange rate"),
			      GNOME_STOCK_BUTTON_OK,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gtk_signal_connect ( GTK_OBJECT ( dialog),
		       "delete_event",
		       GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
		       NULL );




  label = gtk_label_new ( COLON(_("Please enter the exchange rate")) );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       20 );
  gtk_widget_show ( label );

  /* création de la ligne du change */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox );

  label = gtk_label_new ( POSTSPACIFY(_("A")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  option_menu_devise_1= gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       option_menu_devise_1,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( option_menu_devise_1 );

  label = gtk_label_new ( SPACIFY(_("equals")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( label );

  entree = gtk_entry_new ();
  gtk_widget_set_usize ( entree,
			 100,
			 FALSE );
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( entree );

  option_menu_devise_2 = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       option_menu_devise_2,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( option_menu_devise_2 );

      /* création du menu de la 1ère devise ( le menu comporte la devise courante et celle associée ) */

  menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ( devise -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise_compte );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_1 ),
			     menu );
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_1 )-> menu ),
		       "selection_done",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       NULL );
  gtk_widget_show ( menu );
	  


      /* création du menu de la 2ème devise ( le menu comporte la devise courante et celle associée ) */

  menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ( devise -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"adr_devise",
			devise_compte );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_2 ),
			     menu );
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_2 ) -> menu ),
		       "selection_done",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       GINT_TO_POINTER ( 1 ));
  gtk_widget_show ( menu );
 




  /* création de la ligne des frais de change */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( COLON(_("Exchange fees")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );


  entree_frais = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree_frais ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_frais,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( entree_frais );

  label = gtk_label_new ( devise_compte -> nom_devise );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );


      /* choix des 1ère et 2ème devise */

  if ( taux_change
       ||
       frais_change )
    {

      if ( une_devise_compte_egale_x_devise_ope )
	{
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					1 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					0 );
	}
      else
	{
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					0 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					1 );
	}
      gtk_entry_set_text ( GTK_ENTRY ( entree ),
			   g_strdup_printf ( "%f",
					     taux_change ));
      gtk_entry_set_text ( GTK_ENTRY ( entree_frais ),
			   g_strdup_printf ( "%4.2f",
					     fabs ( frais_change )));

    }
  else
    {
      /* vérifie s'il y a déjà une association entre la devise du compte et la devise de l'opération */

      if ( devise_compte -> no_devise_en_rapport == devise -> no_devise )
	{
	  /* il y a une association de la devise du compte vers la devise de l'opération */

	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					devise_compte -> une_devise_1_egale_x_devise_2 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					!( devise_compte -> une_devise_1_egale_x_devise_2 ));

	  /*       si un change est déjà entré, on l'affiche */
	  
	  if ( devise_compte -> date_dernier_change )
	    gtk_entry_set_text ( GTK_ENTRY ( entree ),
				 g_strdup_printf ( "%f",
						   devise_compte -> change ));
	}
      else
	if ( devise -> no_devise_en_rapport == devise_compte -> no_devise )
	  {
	    /* il y a une association de la devise de l'opération vers la devise du compte */

	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					  !(devise -> une_devise_1_egale_x_devise_2 ));
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					  devise -> une_devise_1_egale_x_devise_2 );

	    /*       si un change est déjà entré, on l'affiche */
	  
	    if ( devise -> date_dernier_change )
	      gtk_entry_set_text ( GTK_ENTRY ( entree ),
				   g_strdup_printf ( "%f",
						     devise -> change ));
	  }
      else
	{
	  /* il n'y a aucun rapport établi entre les 2 devises */

	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					1 );
	  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					0 );
	}
    }



/* on lance la fenetre */

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( !resultat )
    {
      struct struct_devise *devise_tmp;

      taux_de_change[0] = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )),
				  NULL );
      taux_de_change[1] = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_frais )),
				  NULL );

      /* FIXME: use gtk_menu_get_active + gtk_option_menu_get_menu ? */
      devise_tmp = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_1 ) -> menu_item ),
					 "adr_devise" );


      if ( devise_tmp -> no_devise != devise -> no_devise )
	taux_de_change[0] = -taux_de_change[0];

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
    }
  else
    {
      taux_de_change[0] = 0;
      taux_de_change[1] = 0;
    }
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Fonction devise_selectionnee */
/* appelée lorsqu'on selectionne une des 2 devises de la fenetre de demande du taux de change */
/* change automatiquement le 2ème option_menu */
/* entrée : origine = 0 : c'est le 1er option menu qui a été changé */
/*              origine = 1 sinon */
/***********************************************************************************************************/

gboolean devise_selectionnee ( GtkWidget *menu_shell,
			       gint origine )
{
  gint position;

  if ( origine )
    {
      /* le 2ème option menu a été changé */

      position = g_list_index ( GTK_MENU_SHELL ( menu_shell ) -> children,
				GTK_OPTION_MENU ( option_menu_devise_2 ) -> menu_item );

      gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
				    1 - position );
    }
  else
    {
      /* le 1er option menu a été changé */

      position = g_list_index ( GTK_MENU_SHELL ( menu_shell ) -> children,
				GTK_OPTION_MENU ( option_menu_devise_1 ) -> menu_item );

      gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
				    1 - position );
    }

  return FALSE;
}
/***********************************************************************************************************/





/**
 * Creates the currency list and associated form to configure them.
 *
 * \returns A newly created vbox
 */
GtkWidget *onglet_devises ( void )
{
  GtkWidget *hbox_pref, *vbox_pref, *separateur, *label, *frame, *paddingbox;
  GtkWidget *scrolled_window, *vbox;
  GSList *liste_tmp;
  gchar *titres_devise [3] = { _("Currency"),
			       _("Code ISO"),
			       _("Sign") };
  GtkWidget *bouton;
  GtkWidget *hbox;

  vbox_pref = new_vbox_with_title_and_icon ( _("Currencies"),
					     "pixmaps/currencies.png" );

  paddingbox = paddingbox_new_with_title (vbox_pref, 
					  _("Known currencies"));
  
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);
  gtk_widget_show ( hbox );

  /* Currency list */
  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
		       FALSE, FALSE, 0);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize ( GTK_WIDGET ( scrolled_window ), FALSE, 120 );
  gtk_widget_show ( scrolled_window );

  clist_devises_parametres = gtk_clist_new_with_titles ( 3, titres_devise );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_devises_parametres ) ,
				      0, TRUE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_devises_parametres ) ,
				      1, TRUE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_devises_parametres ) ,
				      2, TRUE );
  gtk_clist_column_titles_passive ( GTK_CLIST ( clist_devises_parametres ));
  gtk_clist_set_column_justification ( GTK_CLIST ( clist_devises_parametres ),
				       1, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( clist_devises_parametres ),
				       2, GTK_JUSTIFY_CENTER);
  gtk_signal_connect_object  ( GTK_OBJECT ( fenetre_preferences ),
			       "apply",
			       GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
			       GTK_OBJECT ( clist_devises_parametres ));
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      clist_devises_parametres );
  gtk_widget_show ( clist_devises_parametres );


  /*   s'il n'y a pas de fichier ouvert, on grise */
  if ( !nb_comptes )
    gtk_widget_set_sensitive ( vbox, FALSE );
  else
    {
      /* on crée la liste_struct_devises_tmp qui est un copie de liste_struct_devises originale */
      /* avec laquelle on travaillera dans les parametres */

      liste_struct_devises_tmp = NULL;
      liste_tmp = liste_struct_devises;

      while ( liste_tmp )
	{
	  struct struct_devise *devise;
	  struct struct_devise *copie_devise;

	  devise = liste_tmp -> data;
	  copie_devise = malloc ( sizeof ( struct struct_devise ));

	  copie_devise -> no_devise = devise -> no_devise;
	  copie_devise -> nom_devise = g_strdup ( devise -> nom_devise );
	  copie_devise -> code_devise = g_strdup ( devise -> code_devise );
	  copie_devise -> code_iso4217_devise = g_strdup ( devise -> code_iso4217_devise );
	  copie_devise -> passage_euro = devise -> passage_euro;
	  copie_devise -> une_devise_1_egale_x_devise_2 = devise -> une_devise_1_egale_x_devise_2;
	  copie_devise -> no_devise_en_rapport = devise -> no_devise_en_rapport;
	  copie_devise -> change = devise -> change;

	  if ( devise -> date_dernier_change )
	    copie_devise -> date_dernier_change = g_date_new_dmy ( devise -> date_dernier_change -> day,
								   devise -> date_dernier_change -> month,
								   devise -> date_dernier_change -> year );
	  else
	    copie_devise -> date_dernier_change = NULL;

	  liste_struct_devises_tmp = g_slist_append ( liste_struct_devises_tmp,
						      copie_devise );
	  liste_tmp = liste_tmp -> next;
	}

      no_derniere_devise_tmp = no_derniere_devise;
      nb_devises_tmp = nb_devises;


      /* remplissage de la liste avec les devises temporaires */

      liste_tmp = liste_struct_devises_tmp;

      while ( liste_tmp )
	{
	  struct struct_devise *devise;
	  gchar *ligne[3];
	  gint ligne_insert;

	  devise = liste_tmp -> data;

	  ligne[0] = devise -> nom_devise;
	  ligne[1] = devise -> code_iso4217_devise;
	  ligne[2] = devise -> code_devise;

	  ligne_insert = gtk_clist_append ( GTK_CLIST ( clist_devises_parametres ),
					    ligne );

	  /* on associe à la ligne la struct de la devise */

	  gtk_clist_set_row_data ( GTK_CLIST ( clist_devises_parametres ),
				   ligne_insert,
				   devise );

	  liste_tmp = liste_tmp -> next;
	}

    }

  /* Create Add/Remove buttons */
  vbox = gtk_vbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
		       FALSE, FALSE, 0 );
  gtk_widget_show ( vbox );

  /* Button "Add" */
  bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC  ( ajout_devise ),
		       clist_devises_parametres );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
		       FALSE, FALSE, 5 );
  gtk_widget_show ( bouton );

  /* Button "Remove" */
  bouton_supprimer_devise = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_devise ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_supprimer_devise, FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_devise ),
		       "clicked",
		       GTK_SIGNAL_FUNC  ( retrait_devise ),
		       clist_devises_parametres );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_devise,
		       FALSE, FALSE, 5 );
  gtk_widget_show ( bouton_supprimer_devise );


  /* Input form for currencies */
  paddingbox = paddingbox_new_with_title (vbox_pref, 
					  _("Currency properties"));

  /* Selecting a currency activates this form */
  gtk_signal_connect ( GTK_OBJECT ( clist_devises_parametres ),
		       "select-row",
		       GTK_SIGNAL_FUNC ( selection_ligne_devise ),
		       paddingbox );
  gtk_signal_connect ( GTK_OBJECT ( clist_devises_parametres ),
		       "unselect-row",
		       GTK_SIGNAL_FUNC ( deselection_ligne_devise ),
		       paddingbox );

  /* Create currency name entry */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);
  label = gtk_label_new ( COLON(_("Name")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );
  entree_nom_devise_parametres = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_nom_devise_parametres ),
		       "changed",
		       GTK_SIGNAL_FUNC ( changement_nom_entree_devise ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( entree_nom_devise_parametres ),
		       "changed",
		       activer_bouton_appliquer,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_nom_devise_parametres,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_nom_devise_parametres );

  /* Create code entry */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);
  label = gtk_label_new ( COLON(_("Sign")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );
  entree_code_devise_parametres = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_code_devise_parametres ),
		       "changed",
		       GTK_SIGNAL_FUNC ( changement_code_entree_devise ),
		       NULL );
  gtk_signal_connect  ( GTK_OBJECT ( entree_code_devise_parametres ),
			"changed",
			activer_bouton_appliquer,
			NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_code_devise_parametres,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_code_devise_parametres );

  /* Create code entry */
  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0);
  label = gtk_label_new ( COLON(_("ISO code")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );
  entree_iso_code_devise_parametres = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_iso_code_devise_parametres ),
		       "changed",
		       GTK_SIGNAL_FUNC ( changement_iso_code_entree_devise ),
		       NULL );
  gtk_signal_connect  ( GTK_OBJECT ( entree_iso_code_devise_parametres ),
			"changed",
			activer_bouton_appliquer,
			NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), entree_iso_code_devise_parametres,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_iso_code_devise_parametres );

  /* Will switch to Euro? */
  check_button_euro = gtk_check_button_new_with_label ( _("Will switch to Euro") );
  gtk_signal_connect ( GTK_OBJECT ( check_button_euro ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( change_passera_euro ),
		       clist_devises_parametres );
  gtk_signal_connect ( GTK_OBJECT ( check_button_euro ),
		       "toggled",
		       activer_bouton_appliquer,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), check_button_euro,
		       FALSE, FALSE, 0);
  gtk_widget_show ( check_button_euro );


  /* Création de la ligne devise associée */
  hbox_devise_associee = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox_devise_associee,
		       FALSE, FALSE, 0);
  gtk_widget_show ( hbox_devise_associee );

  label = gtk_label_new ( COLON(_("Associated currency")) );
  gtk_box_pack_start ( GTK_BOX ( hbox_devise_associee ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );

  option_menu_devises = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_devise_associee ), option_menu_devises,
		       FALSE, FALSE, 0);
  gtk_widget_show ( option_menu_devises );


  /* Création de la ligne de change entre les devises */
  label_date_dernier_change = gtk_label_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), label_date_dernier_change,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label_date_dernier_change );

  /* Création de la ligne du change */
  hbox_ligne_change = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox_ligne_change,
		       FALSE, FALSE, 0);
  gtk_widget_show ( hbox_ligne_change );

  label = gtk_label_new ( POSTSPACIFY(_("One")) );
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );

  devise_1 = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), devise_1,
		       FALSE, FALSE, 0);
  gtk_widget_show ( devise_1 );

  label = gtk_label_new ( SPACIFY(_("costs")) );
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), label,
		       FALSE, FALSE, 0);
  gtk_widget_show ( label );

  entree_conversion = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( entree_conversion ),
		       "changed",
		       activer_bouton_appliquer,
		       NULL );
  gtk_widget_set_usize ( entree_conversion,
			 100,
			 FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), entree_conversion,
		       FALSE, FALSE, 0);
  gtk_widget_show ( entree_conversion );

  devise_2 = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_ligne_change ), devise_2,
		       FALSE, FALSE, 0);
  gtk_widget_show ( devise_2 );

  return ( vbox_pref );

}
/* ************************************************************************************************************** */




/* **************************************************************************************************************************** */
/* Fonction selection_ligne_devise */
/* appelée lorsqu'on sélectionne une devise dans la liste */
/* **************************************************************************************************************************** */

gboolean selection_ligne_devise ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev,
				  GtkWidget *frame )
{
  struct struct_devise *devise;

  ligne_selection_devise = ligne;
  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );


  /* met le nom et le code de la devise */

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				     GTK_SIGNAL_FUNC ( changement_nom_entree_devise ),
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_devise_parametres ),
		       devise -> nom_devise );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				       GTK_SIGNAL_FUNC ( changement_nom_entree_devise ),
				       NULL );

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				     GTK_SIGNAL_FUNC ( changement_code_entree_devise ),
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_code_devise_parametres ),
		       devise -> code_devise );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				       GTK_SIGNAL_FUNC ( changement_code_entree_devise ),
				       NULL );


  /* crée le menu des devises en enlevant la devise courante */

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devises ),
			     creation_option_menu_devises ( devise -> no_devise,
							    liste_struct_devises_tmp ));
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu ),
		       "selection-done",
		       GTK_SIGNAL_FUNC ( changement_devise_associee ),
		       liste );
  gtk_signal_connect ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises) -> menu ),
		       "selection-done",
		       activer_bouton_appliquer,
		       NULL );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( check_button_euro ),
				 devise -> passage_euro );

  change_passera_euro ( check_button_euro,
			liste );

  gtk_widget_set_sensitive ( frame, TRUE );
  gtk_widget_set_sensitive ( bouton_supprimer_devise, TRUE );

  if ( !strcmp ( devise -> nom_devise,
		 _("Euro") ) )
    {
      gtk_widget_set_sensitive ( check_button_euro,
				 FALSE );
      gtk_widget_set_sensitive ( entree_nom_devise_parametres,
				 FALSE );
    }
  else
    gtk_widget_set_sensitive ( entree_nom_devise_parametres,
			       TRUE );

  return FALSE;
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
/* Fonction deselection_ligne_devise */
/* appelée lorsqu'on désélectionne une devise dans la liste */
/* **************************************************************************************************************************** */

gboolean deselection_ligne_devise ( GtkWidget *liste,
				    gint ligne,
				    gint colonne,
				    GdkEventButton *ev,
				    GtkWidget *frame )
{
  struct struct_devise *devise;

  ligne_selection_devise = -1;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne );

  /* retire le nom et le code de la devise */
  
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				     GTK_SIGNAL_FUNC ( changement_nom_entree_devise ),
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_devise_parametres ),
		       "" );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_devise_parametres ),
				       GTK_SIGNAL_FUNC ( changement_nom_entree_devise ),
				       NULL );

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				     GTK_SIGNAL_FUNC ( changement_code_entree_devise ),
				     NULL );
  gtk_entry_set_text ( GTK_ENTRY ( entree_code_devise_parametres ),
		       "" );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_code_devise_parametres ),
				       GTK_SIGNAL_FUNC ( changement_code_entree_devise ),
				       NULL );


  if ( ( devise -> passage_euro = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button_euro ) )) )
    {
      /* c'est une devise qui passera à l'euro */

      devise -> une_devise_1_egale_x_devise_2 = 0;
      devise -> no_devise_en_rapport = 1;
      devise -> change = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion )),
				    NULL );
      devise -> date_dernier_change = NULL;

    }
  else
    {
      /*       si le change a changé, c'est qu'il y a une mise à jours */

      if ( g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion )),
		      NULL )
	   !=
	   devise -> change )
	{
	  devise -> date_dernier_change = g_date_new ();
	  g_date_set_time ( devise -> date_dernier_change,
			    time (NULL));
	}


      /* qu'il y ait un changement dans le change ou pas, on récupère toutes les autres valeurs */

      devise -> no_devise_en_rapport =  ((struct struct_devise *)(gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
											"adr_devise" ))) -> no_devise;

      if ( devise -> no_devise_en_rapport
	   &&
	   ( ((struct struct_devise *)(gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( devise_1 ) -> menu_item ),
							     "adr_devise" ))) -> no_devise_en_rapport )
	   ==
	   devise -> no_devise_en_rapport )
	devise -> une_devise_1_egale_x_devise_2 = 1;
      else
	devise -> une_devise_1_egale_x_devise_2 = 0;
      
      devise -> change = g_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_conversion )),
				    NULL );
    }
	    


  gtk_widget_set_sensitive ( check_button_euro, TRUE );
  gtk_widget_set_sensitive ( frame, FALSE );
  gtk_widget_set_sensitive ( bouton_supprimer_devise, FALSE );

  return FALSE;
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
/* Fonction change_passera_euro */
/* appelée quand appuie sur le bouton Passera à l'euro */
/* **************************************************************************************************************************** */

gboolean change_passera_euro ( GtkWidget *bouton,
			       GtkWidget *liste )
{
  struct struct_devise *devise;
  GtkWidget *menu;
  GtkWidget *item;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton ) ))
    {
      gtk_widget_hide ( hbox_devise_associee );
      gtk_widget_hide ( label_date_dernier_change );
      gtk_widget_show ( hbox_ligne_change );

      /* on met l'euro dans le 1er menu */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( _("Euro") );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_1 ),
				 menu );
      gtk_widget_show ( menu );


      /*       on met la devise dans le 2ème menu */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( devise -> nom_devise );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_2 ),
				 menu );
      gtk_widget_show ( menu );

      gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			   g_strdup_printf ( "%f",
					     devise -> change ));
      /* on rend le tout sensitif */

      gtk_widget_set_sensitive ( hbox_ligne_change,
				 TRUE );
      gtk_widget_set_sensitive ( entree_conversion,
				 TRUE );
      gtk_widget_set_sensitive ( devise_2,
				 FALSE );
      gtk_widget_set_sensitive ( devise_1,
				 FALSE );

    }
  else
    {
/*       magouille car l'option menu ne contient pas le nom de la devise courante */

      if ( devise -> no_devise > devise -> no_devise_en_rapport )
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devises ),
				      g_slist_position ( liste_struct_devises_tmp,
							 g_slist_find_custom ( liste_struct_devises_tmp,
									       GINT_TO_POINTER ( devise -> no_devise_en_rapport ),
									       ( GCompareFunc ) recherche_devise_par_no )) + 1);
      else
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devises ),
				      g_slist_position ( liste_struct_devises_tmp,
							 g_slist_find_custom ( liste_struct_devises_tmp,
									       GINT_TO_POINTER ( devise -> no_devise_en_rapport ),
									       ( GCompareFunc ) recherche_devise_par_no ))  );

      gtk_widget_show ( hbox_devise_associee );

      if ( devise -> date_dernier_change )
	{
	  gchar date[11];

	  g_date_strftime ( date,
			    11,
			    "%d/%m/%Y",
			    devise -> date_dernier_change );

	  gtk_label_set_text ( GTK_LABEL ( label_date_dernier_change ),
			       date );
	  /* mise en place du change courant */

	  gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			       g_strdup_printf ( "%f",
						 devise -> change ));
	}
      else
	{
	  gtk_label_set_text ( GTK_LABEL ( label_date_dernier_change ),
			       _("No exchange rate defined")  );
	  /* mise en place du change courant */
	  
	  gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			       "" );
	}

      gtk_widget_show ( label_date_dernier_change );

      /* on rend le tout sensitif */

      gtk_widget_set_sensitive ( hbox_ligne_change,
				 TRUE );
      gtk_widget_set_sensitive ( entree_conversion,
				 TRUE );
      gtk_widget_set_sensitive ( devise_2,
				  TRUE);
      gtk_widget_set_sensitive ( devise_1,
				 TRUE );

      changement_devise_associee ( GTK_OPTION_MENU ( option_menu_devises ) -> menu,
				   liste );
      
      gtk_widget_show ( hbox_ligne_change );

    }

  return FALSE;
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
/* Fonction changement_devise_associee */
/* appelée lorsqu'on change la devise comparée */
/* **************************************************************************************************************************** */

gboolean changement_devise_associee ( GtkWidget *menu_devises,
				      GtkWidget *liste )
{
  struct struct_devise *devise;
  struct struct_devise *devise_associee;
  GtkWidget *menu;
  GtkWidget *item;


  devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				    ligne_selection_devise );

  devise_associee = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
					  "adr_devise" );
  if ( devise_associee &&
       devise_associee != devise_nulle &&
       devise_associee -> no_devise )
    {
      gtk_widget_set_sensitive ( hbox_ligne_change,
				 TRUE );

      /* création du menu de la 1ère devise ( le menu comporte la devise courante et celle associée ) */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( devise -> nom_devise );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    devise );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      item = gtk_menu_item_new_with_label ( NOM_DEVISE_ASSOCIEE );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
						  "adr_devise" ));
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );


      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_1 ),
				 menu );
      gtk_signal_connect ( GTK_OBJECT ( menu ),
			   "selection-done",
			   activer_bouton_appliquer,
			   NULL );

      gtk_widget_show ( menu );
	  


      /* création du menu de la 2ème devise ( le menu comporte la devise courante et celle associée ) */

      menu = gtk_menu_new ();

      item = gtk_menu_item_new_with_label ( devise -> nom_devise );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    devise );
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );

      item = gtk_menu_item_new_with_label ( NOM_DEVISE_ASSOCIEE );
      gtk_object_set_data ( GTK_OBJECT ( item ),
			    "adr_devise",
			    gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu_devises ) -> menu_item ),
						  "adr_devise" ));
      gtk_menu_append ( GTK_MENU ( menu ),
			item );
      gtk_widget_show ( item );


      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_2 ),
				 menu );
      gtk_signal_connect ( GTK_OBJECT ( menu ),
			   "selection-done",
			   activer_bouton_appliquer,
			   NULL );

      gtk_widget_show ( menu );
 

      /* choix des 1ère et 2ème devise */

      gtk_option_menu_set_history ( GTK_OPTION_MENU ( devise_1 ),
				    !( devise -> une_devise_1_egale_x_devise_2 ));
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( devise_2 ),
				    devise -> une_devise_1_egale_x_devise_2 );
    }
  else
    {
      menu = gtk_menu_new ();
     gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_1 ),
				menu );

     menu = gtk_menu_new ();
     gtk_option_menu_set_menu ( GTK_OPTION_MENU ( devise_2 ),
				menu );

      gtk_entry_set_text ( GTK_ENTRY ( entree_conversion ),
			   "" );
      gtk_widget_set_sensitive ( hbox_ligne_change,
				 FALSE );
    }

  return FALSE;
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
gboolean changement_nom_entree_devise ( void )
{
  struct struct_devise *devise;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( clist_devises_parametres ),
				    ligne_selection_devise );

  devise -> nom_devise = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_devise_parametres ))));

  gtk_clist_set_text ( GTK_CLIST ( clist_devises_parametres ),
		       ligne_selection_devise,
		       0,
		       devise -> nom_devise );
  return FALSE;
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
gboolean changement_code_entree_devise ( void )
{
  struct struct_devise *devise;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( clist_devises_parametres ),
				    ligne_selection_devise );

  devise -> code_devise = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_code_devise_parametres ))));

  gtk_clist_set_text ( GTK_CLIST ( clist_devises_parametres ),
		       ligne_selection_devise,
		       1,
		       devise -> code_devise );
  return FALSE;
}
/* **************************************************************************************************************************** */



gboolean changement_iso_code_entree_devise ( void )
{
  struct struct_devise *devise;

  devise = gtk_clist_get_row_data ( GTK_CLIST ( clist_devises_parametres ),
				    ligne_selection_devise );

  devise -> code_iso4217_devise = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code_devise_parametres ))));

  gtk_clist_set_text ( GTK_CLIST ( clist_devises_parametres ),
		       ligne_selection_devise,
		       2,
		       devise -> code_iso4217_devise );
  return FALSE;
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
/* cette fonction prend en argument un montant, la devise de renvoi (en général la devise du compte) */
/*      et la devise du montant donné en argument */
/* elle renvoie le montant de l'opération dans la devise de renvoi */
/* **************************************************************************************************************************** */

gdouble calcule_montant_devise_renvoi ( gdouble montant_init,
					gint no_devise_renvoi,
					gint no_devise_montant,
					gint une_devise_compte_egale_x_devise_ope,
					gdouble taux_change,
					gdouble frais_change )
{
  gdouble montant;

  /* tout d'abord, si les 2 devises sont les mêmes, on renvoie le montant directement */

  if ( no_devise_renvoi == no_devise_montant )
    return ( montant_init );

  /*   il faut faire une transformation du montant */
  /* on utilise les variables globales devise_compte et devise_operation pour */
  /* gagner du temps */

  /* récupère la devise du compte si nécessaire */

  if ( !devise_compte
       ||
       devise_compte -> no_devise != no_devise_renvoi )
    devise_compte = g_slist_find_custom ( liste_struct_devises,
					  GINT_TO_POINTER ( no_devise_renvoi ),
					  ( GCompareFunc ) recherche_devise_par_no) -> data;

  /* récupère la devise de l'opération si nécessaire */

  if ( !devise_operation
       ||
       devise_operation -> no_devise != no_devise_montant )
    devise_operation = g_slist_find_custom ( liste_struct_devises,
					     GINT_TO_POINTER ( no_devise_montant ),
					     ( GCompareFunc ) recherche_devise_par_no) -> data;

  /* on a maintenant les 2 devises, on peut faire les calculs */

  if ( devise_compte -> passage_euro
       &&
       !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
    montant = montant_init * devise_compte -> change;
  else
    if ( devise_operation -> passage_euro
	 &&
	 !strcmp ( devise_compte -> nom_devise, _("Euro") ))
      montant = montant_init / devise_operation -> change;
    else
      if ( une_devise_compte_egale_x_devise_ope )
	montant = montant_init / taux_change - frais_change;
      else
	montant = montant_init * taux_change - frais_change;

  montant = ( rint (montant * 100 )) / 100;

  return ( montant);
}
/* **************************************************************************************************************************** */



gchar * devise_name ( struct struct_devise * devise )
{
  if (devise -> code_devise)
    return devise -> code_devise;

  return devise -> code_iso4217_devise;
}
