/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2003 Benjamin Drieu (bdrieu@april.org)		      */
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

/**
 * \file gsb_currency_config.c
 * contains the part to set the configuration of the currencies
 */

#include "include.h"

/*START_INCLUDE*/
#include "gsb_currency_config.h"
#include "dialog.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "utils_editables.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "structures.h"
#include "gsb_currency_config.h"
#include "gsb_file_config.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void append_currency_to_currency_list ( GtkTreeStore * model,
					gint currency_number );
static gboolean changement_code_entree_devise ( GtkEditable *editable, gchar * text,
					 gint length, gpointer data );
static gboolean changement_iso_code_entree_devise ( GtkEditable *editable, gchar * text,
					     gint length, gpointer data );
static gboolean changement_nom_entree_devise ( GtkEditable *editable, gchar * text,
					gint length, gpointer data );
static gint create_currency ( const gchar *nom_devise,
		       const gchar *code_devise, 
		       const gchar *code_iso4217_devise );
static gint currency_get_selected ( GtkTreeView * view );
static void fill_currency_list ( GtkTreeView * view, gboolean include_obsolete );
static gboolean gsb_gui_select_default_currency ( GtkTreeModel * tree_model, GtkTreePath * path, 
					   GtkTreeIter * iter, GtkTreeView * view );
static GtkWidget * new_currency_option_menu ( gint * value, GCallback hook );
static GtkWidget * new_currency_tree ();
static GtkWidget * new_currency_vbox ();
static gboolean rebuild_currency_list ( GtkWidget * checkbox, GtkTreeView * view );
static void remove_selected_currency_from_currency_view ( GtkTreeView * view );
static void retrait_devise ( GtkWidget *bouton, GtkWidget * view );
static gboolean select_currency_in_iso_list ( GtkTreeSelection *selection, GtkTreeModel *model );
/*END_STATIC*/

/*START_EXTERN*/
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkTreeSelection * selection;
extern GtkWidget *window;
/*END_EXTERN*/

/* xxx !!!!!!!!!!!!!!!!!!!! ici à faire */
/* voir aussi le bug corrigé sur la stable qu'on ne peut pas changer la devise associée */
/* (c'était un pb de history avec directement le no de la devise */

struct iso_4217_currency;
static struct iso_4217_currency iso_4217_currencies[] = {
    { N_("Africa"), N_("Algerian Dinar"), N_("Algeria"), "DZD", NULL, TRUE, "DZD.png" },
    { N_("Africa"), N_("Botswana Pula"), N_("Botswana"), "BWP", NULL, TRUE, "BWP.png" },
    { N_("Africa"), N_("Burundi Franc"), N_("Burundi"), "BIF", NULL, TRUE, "BIF.png" },
    { N_("Africa"), N_("CFA Franc BCEAO"), N_("Niger"), "XOF", NULL, TRUE, "NIG.png" },
    { N_("Africa"), N_("CFA Franc BCEAO"), N_("Senegal"), "XOF", NULL, TRUE, "SEN.png" },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Cameroon"), "XAF", NULL, TRUE, "CAM.png" },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Chad"), "XAF", NULL, TRUE, "CHA.png" },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Congo"), "XAF", NULL, TRUE, "CON.png" },
    { N_("Africa"), N_("Comoro Franc"), N_("Comoros"), "KMF", NULL, TRUE, "KMF.png" },
    { N_("Africa"), N_("Egyptian Pound"), N_("Egypt"), "EGP", "£", TRUE, "EGP.png" },
    { N_("Africa"), N_("Ethiopian Birr"), N_("Ethiopia"), "ETB", NULL, TRUE, "ETB.png" },
    { N_("Africa"), N_("Gambian Dalasi"), N_("Gambia"), "GMD", NULL, TRUE, "GMD.png" },
    { N_("Africa"), N_("Ghana Cedi"), N_("Ghana"), "GHC", NULL, TRUE, "GHC.png" },
    { N_("Africa"), N_("Guinea-Bissau Peso"), N_("Guinea-Bissau"), "GWP", NULL, TRUE, "GWP.png" },
    { N_("Africa"), N_("Kenyan Shilling"), N_("Kenya"), "KES", NULL, TRUE, "KES.png" },
    { N_("Africa"), N_("Liberian Dollar"), N_("Liberia"), "LRD", "$", TRUE, "LRD.png" },
    { N_("Africa"), N_("Libyan Dinar"), N_("Libyan Arab Jamahiriya"), "LYD", NULL, TRUE, "LYD.png" },
    { N_("Africa"), N_("Malagasy Franc"), N_("Madagascar"), "MGF", NULL, TRUE, "MGF.png" },
    { N_("Africa"), N_("Malawi Kwacha"), N_("Malawi"), "MWK", NULL, TRUE, "MWK.png" },
    { N_("Africa"), N_("Mauritania Ouguiya"), N_("Mauritania"), "MRO", NULL, TRUE, "MRO.png" },
    { N_("Africa"), N_("Moazambique Metical"), N_("Mozambique"), "MZM", NULL, TRUE, "MZM.png" },
    { N_("Africa"), N_("Moroccan Dirham"), N_("Morocco"), "MAD", NULL, TRUE, "MAD.png" },
    { N_("Africa"), N_("Nigerian Naira"), N_("Nigeria"), "NGN", "₦", TRUE, "NGN.png" },
    { N_("Africa"), N_("Rwanda Franc"), N_("Rwanda"), "RWF", NULL, TRUE, "RWF.png" },
    { N_("Africa"), N_("Sao Tome and Principe Dobra"), N_("Sao Tome and Principe"), "STD", NULL, TRUE, "STD.png" },
    { N_("Africa"), N_("Seychelles Rupee"), N_("Seychelles"), "SCR", "₨", TRUE, "SCR.png" },
    { N_("Africa"), N_("Sierra Leonean Leone"), N_("Sierra Leone"), "SLL", NULL, TRUE, "SLL.png" },
    { N_("Africa"), N_("Somali Shilling"), N_("Somalia"), "SOS", NULL, TRUE, "SOS.png" },
    { N_("Africa"), N_("South African Rand"), N_("Lesotho"), "ZAR", "R", TRUE, "LSL.png" },
    { N_("Africa"), N_("South African Rand"), N_("Namibia"), "ZAR", "R", TRUE, "NAD.png" },
    { N_("Africa"), N_("South African Rand"), N_("South Africa"), "ZAR", "R", TRUE, "ZAR.png" },
    { N_("Africa"), N_("Swaziland Lilangeni"), N_("Swaziland"), "SZL", NULL, TRUE, "SZL.png" },
    { N_("Africa"), N_("Tanzanian Shilling"), N_("United Republic of Tanzania"), "TZS", NULL, TRUE, "TZS.png" },
    { N_("Africa"), N_("Tunisian Dinar"), N_("Tunisia"), "TND", NULL, TRUE, "TND.png" },
    { N_("Africa"), N_("Zambian Kwacha"), N_("Zambia"), "ZMK", NULL, TRUE, "ZMK.png" },
    { N_("Africa"), N_("Zimbabwe Dollar"), N_("Zimbabwe"), "ZWD", "$", TRUE, "ZWD.png" },
    { N_("Asia"), N_("Afghani"), N_("Afghanistan"), "AFA", NULL, TRUE, "AFN.png" },
    { N_("Asia"), N_("Bahraini Dinar"), N_("Bahrain"), "BHD", NULL, TRUE, "BHD.png" },
    { N_("Asia"), N_("Bangladesh Taka"), N_("Bangladesh"), "BDT", NULL, TRUE, "BDT.png" },
    { N_("Asia"), N_("Brunei Dollar"), N_("Brunei Darussalam"), "BND", "$", TRUE, "BND.png" },
    { N_("Asia"), N_("Cambodian Riel"), N_("Cambodia"), "KHR", "៛", TRUE, "KHR.png" },
    { N_("Asia"), N_("Cyprus Pound"), N_("Cyprus"), "CYP", "£", TRUE, "CYP.png" },
    { N_("Asia"), N_("Hong Kong Dollar"), N_("Hong Kong"), "HKD", "$", TRUE, "HKD.png" },
    { N_("Asia"), N_("Indian Rupee"), N_("Bhutan"), "INR", "₨", TRUE, "BHU.png" },
    { N_("Asia"), N_("Indian Rupee"), N_("India"), "INR", "₨", TRUE, "INR.png" },
    { N_("Asia"), N_("Indonesian Rupiah"), N_("Indonesia"), "IDR", NULL, TRUE, "IDR.png" },
    { N_("Asia"), N_("Iranian Rial"), N_("Iran"), "IRR", "﷼", TRUE, "IRR.png" },
    { N_("Asia"), N_("Iraqi Dinar"), N_("Iraq"), "IQD", NULL, TRUE, "IQD.png" },
    { N_("Asia"), N_("Japanese Yen"), N_("Japan"), "JPY", "¥", TRUE, "JPY.png" },
    { N_("Asia"), N_("Jordanian Dinar"), N_("Jordan"), "JOD", NULL, TRUE, "JOD.png" },
    { N_("Asia"), N_("Kuwaiti Dinar"), N_("Kuwait"), "KWD", NULL, TRUE, "KWD.png" },
    { N_("Asia"), N_("Lao Kip"), N_("Lao People's Democratic Republic"), "LAK", "₭", TRUE, "LAK.png" },
    { N_("Asia"), N_("Lebanese Pound"), N_("Lebanon"), "LBP", "£", TRUE, "LBP.png" },
    { N_("Asia"), N_("Macau Pataca"), N_("Macao"), "MOP", NULL, TRUE, "MOP.png" },
    { N_("Asia"), N_("Malaysian Ringgit"), N_("Malaysia"), "MYR", NULL, TRUE, "MYR.png" },
    { N_("Asia"), N_("Mongolian Tugrik"), N_("Mongolia"), "MNT", "₮", TRUE, "MNT.png" },
    { N_("Asia"), N_("Nepalese Rupee"), N_("Nepal"), "NPR", "₨", TRUE, "NPR.png" },
    { N_("Asia"), N_("New Israeli Shekel"), N_("Israel"), "ILS", "₪", TRUE, "ILS.png" },
    { N_("Asia"), N_("New Taiwan Dollar"), N_("Taiwan, Province of China"), "TWD", "元", TRUE, "TWD.png" },
    { N_("Asia"), N_("North Korean Won"), N_("Democratic People's Republic of Korea"), "KPW", "₩", TRUE, "KPW.png" },
    { N_("Asia"), N_("Pakistan Rupee"), N_("Pakistan"), "PKR", "₨", TRUE, "PKR.png" },
    { N_("Asia"), N_("Philippine peso"), N_("Philippines"), "PHP", "₱", TRUE, "PHP.png" },
    { N_("Asia"), N_("Qatari Rial"), N_("Qatar"), "QAR", "﷼", TRUE, "QAR.png" },
    { N_("Asia"), N_("Rial Omani"), N_("Oman"), "OMR", "﷼", TRUE, "OMR.png" },
    { N_("Asia"), N_("Russian Ruble"), N_("Russia"), "RUR", "руб", TRUE, "RUB.png" },
    { N_("Asia"), N_("Saudi Riyal"), N_("Saudi Arabia"), "SAR", "﷼", TRUE, "SAR.png" },
    { N_("Asia"), N_("Singapore Dollar"), N_("Singapore"), "SGD", "$", TRUE, "SGD.png" },
    { N_("Asia"), N_("South Korean Won"), N_("Republic of Korea"), "KRW", "₩", TRUE, "KRW.png" },
    { N_("Asia"), N_("Sri Lanka Rupee"), N_("Sri Lanka"), "LKR", "௹", TRUE, "LKR.png" },
    { N_("Asia"), N_("Syrian Pound"), N_("Syrian Arab Republic"), "SYP", "£", TRUE, "SYP.png" },
    { N_("Asia"), N_("Thai Baht"), N_("Thailand"), "THB", "฿", TRUE, "THB.png" },
    { N_("Asia"), N_("Turkish Lira"), N_("Turkey"), "TRL", "₤", TRUE, "TRL.png" },
    { N_("Asia"), N_("United Arab Emirates Dirham"), N_("United Arab Emirates"), "AED", NULL, TRUE, "AED.png" },
    { N_("Asia"), N_("Viet Nam Dong"), N_("Viet Nam"), "VND", "₫", TRUE, "VND.png" },
    { N_("Asia"), N_("Yemeni Rial"), N_("Yemen"), "YER", "﷼", TRUE, "YER.png" },
    { N_("Asia"), N_("Yuan Renminbi"), N_("China"), "CNY", "元", TRUE, "CNY.png" },
    { N_("Central America"), N_("Belize Dollar"), N_("Belize"), "BZD", "$", TRUE, "BZD.png" },
    { N_("Central America"), N_("Costa Rican Colon"), N_("Costa Rica"), "CRC", "₡", TRUE, "CRC.png" },
    { N_("Central America"), N_("Guatemalan Quetzal"), N_("Guatemala"), "GTQ", NULL, TRUE, "GTQ.png" },
    { N_("Central America"), N_("Honduran Lempira"), N_("Honduras"), "HNL", NULL, TRUE, "HNL.png" },
    { N_("Central America"), N_("Mexican Peso"), N_("Mexico"), "MXP", "$", FALSE, "MXN.png" },
    { N_("Central America"), N_("Panama Balboa"), N_("Panama"), "PAB", NULL, TRUE, "PAB.png" },
    { N_("Europe"), N_("Albanian Lek"), N_("Albania"), "ALL", NULL, TRUE, "ALL.png" },
    { N_("Europe"), N_("Austrian Schilling"), N_("Austria"), "ATS", NULL, FALSE, "ATS.png" },
    { N_("Europe"), N_("Belgian Franc"), N_("Belgium"), "BEF", NULL, FALSE, "BEF.png" },
    { N_("Europe"), N_("Bulgarian Lev"), N_("Bulgaria"), "BGL", NULL, FALSE, "BGN.png" },
    { N_("Europe"), N_("Czech Koruna"), N_("Czech Republic"), "CZK", NULL, TRUE, "CZK.png" },
    { N_("Europe"), N_("Danish Krone"), N_("Denmark"), "DKK", NULL, TRUE, "DKK.png" },
    { N_("Europe"), N_("Deutsche Mark"), N_("Germany"), "DEM", NULL, FALSE, "DEM.png" },
    { N_("Europe"), N_("Estonian Kroon"), N_("Estonia"), "EEK", NULL, TRUE, "EEK.png" },
    { N_("Europe"), N_("Euro"), N_("CEE"), "EUR", "€", TRUE, "EUR.png" },
    { N_("Europe"), N_("Finnish Markka"), N_("Finland"), "FIM", NULL, FALSE, "FIM.png" },
    { N_("Europe"), N_("French Franc"), N_("France"), "FRF", "₣", FALSE, "FRF.png" },
    { N_("Europe"), N_("Gibraltar Pound"), N_("Gibraltar"), "GIP", "£", TRUE, "GIP.png" },
    { N_("Europe"), N_("Greek Drachma"), N_("Greece"), "GRD", "₯", FALSE, "GRD.png" },
    { N_("Europe"), N_("Hungarian Forint"), N_("Hungary"), "HUF", NULL, TRUE, "HUF.png" },
    { N_("Europe"), N_("Iceland Krona"), N_("Iceland"), "ISK", NULL, TRUE, "ISK.png" },
    { N_("Europe"), N_("Irish Pound"), N_("Ireland"), "IEP", "£", FALSE, "IEP.png" },
    { N_("Europe"), N_("Italian Lira"), N_("Holy See"), "ITL", "₤", FALSE, "VAT.png" },
    { N_("Europe"), N_("Italian Lira"), N_("Italy"), "ITL", "₤", FALSE, "ITL.png" },
    { N_("Europe"), N_("Italian Lira"), N_("San Marino"), "ITL", "₤", FALSE, "SAN.png" },
    { N_("Europe"), N_("Latvian Lat"), N_("Latvia"), "LVL", NULL, TRUE, "LVL.png" },
    { N_("Europe"), N_("Lithuanian Litas"), N_("Lietuva"), "LTL", NULL, TRUE, "LTL.png" },
    { N_("Europe"), N_("Luxembourg Franc"), N_("Luxembourg"), "LUF", "₣", FALSE, "LUF.png" },
    { N_("Europe"), N_("Netherlands Guilder"), N_("Netherlands"), "NLG", "ƒ", FALSE, "NLG.png" },
    { N_("Europe"), N_("New Yugoslavian Dinar"), N_("Serbia and Montenegro"), "YUD", NULL, FALSE, "YUV.png" },
    { N_("Europe"), N_("Norwegian Krone"), N_("Norway"), "NOK", NULL, TRUE, "NOK.png" },
    { N_("Europe"), N_("Polish Zloty"), N_("Poland"), "PLZ", NULL, TRUE, "PLN.png" },
    { N_("Europe"), N_("Portuguese Escudo"), N_("Portugal"), "PTE", NULL, FALSE, "PTE.png" },
    { N_("Europe"), N_("Pound Sterling"), N_("United Kingdom"), "GBP", "£", TRUE, "GBP.png" },
    { N_("Europe"), N_("Romanian Leu"), N_("Romania"), "ROL", NULL, TRUE, "ROL.png" },
    { N_("Europe"), N_("Slovak Koruna"), N_("Slovakia"), "SKK", NULL, TRUE, "SKK.png" },
    { N_("Europe"), N_("Slovene Tolar"), N_("Slovenia"), "SIT", NULL, TRUE, "SIT.png" },
    { N_("Europe"), N_("Spanish Peseta"), N_("Spain"), "ESP", "₧", FALSE, "ESP.png" },
    { N_("Europe"), N_("Swedish Krona"), N_("Sweden"), "SEK", "kr", TRUE, "SEK.png" },
    { N_("Europe"), N_("Swiss Franc"), N_("Liechtenstein"), "CHF", NULL, TRUE, "LIE.png" },
    { N_("Europe"), N_("Swiss Franc"), N_("Switzerland"), "CHF", NULL, TRUE, "CHF.png" },
    { N_("Europe"), N_("Hryvnia"), N_("Ukraine"), "UAH", NULL, TRUE, "UAH.png" },
    { N_("Northern America"), N_("Bahamian Dollar"), N_("Bahamas"), "BSD", "$", TRUE, "BSD.png" },
    { N_("Northern America"), N_("Barbados Dollar"), N_("Barbados"), "BBD", "$", TRUE, "BBD.png" },
    { N_("Northern America"), N_("Bermuda Dollar"), N_("Bermuda"), "BMD", "$", TRUE, "BMD.png" },
    { N_("Northern America"), N_("Canadian Dollar"), N_("Canada"), "CAD", "$", TRUE, "CAD.png" },
    { N_("Northern America"), N_("Cayman Islands Dollar"), N_("Cayman Islands"), "KYD", NULL, TRUE, "KYD.png" },
    { N_("Northern America"), N_("Cuban Peso"), N_("Cuba"), "CUP", "₱ 	", TRUE, "CUP.png" },
    { N_("Northern America"), N_("Dominican Peso"), N_("Dominican Republic"), "DOP", "₱", TRUE, "DOP.png" },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Grenada"), "XCD", "$", TRUE, "GRE.png" },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Saint Lucia"), "XCD", "$", TRUE, "SLC.png" },
    { N_("Northern America"), N_("Haitian Gourde"), N_("Haiti"), "HTG", NULL, TRUE, "HTG.png" },
    { N_("Northern America"), N_("Jamaican Dollar"), N_("Jamaica"), "JMD", "$", TRUE, "JMD.png" },
    { N_("Northern America"), N_("Netherlands Antillian Guilder"), N_("Netherlands Antilles"), "ANG", "ƒ", TRUE, "ANG.png" },
    { N_("Northern America"), N_("Trinidad and Tobago Dollar"), N_("Trinidad and Tobago"), "TTD", "$ 	", TRUE, "TTD.png" },
    { N_("Northern America"), N_("United States Dollar"), N_("United States"), "USD", "$", TRUE, "USD.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Australia"), "AUD", "$", TRUE, "AUD.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Kiribati"), "AUD", "$", TRUE, "KIR.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Nauru"), "AUD", "$", TRUE, "NAU.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Tuvalu"), "AUD", "$", TRUE, "TUV.png" },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("French Polynesia"), "XPF", NULL, TRUE, "FRF.png" },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("New Caledonia"), "XPF", NULL, TRUE, "FRF.png" },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("Wallis and Futuna"), "XPF", NULL, TRUE, "FRF.png" },
    { N_("Pacific Ocean"), N_("Fiji Dollar"), N_("Fiji"), "FJD", "$", TRUE, "FJD.png" },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("Cook Islands"), "NZD", "$", TRUE, "COO.png" },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("New Zealand"), "NZD", NULL, TRUE, "NZD.png" },
    { N_("Pacific Ocean"), N_("Papua New Guinea Kina"), N_("Papua New Guinea"), "PGK", NULL, TRUE, "PGK.png" },
    { N_("Pacific Ocean"), N_("Samoa Tala"), N_("Samoa"), "WST", NULL, TRUE, "WST.png" },
    { N_("Pacific Ocean"), N_("Solomon Islands Dollar"), N_("Solomon Islands"), "SBD", NULL, TRUE, "SBD.png" },
    { N_("Pacific Ocean"), N_("Timor Escudo"), N_("Timor"), "TPE", NULL, TRUE, "TPE.png" },
    { N_("Pacific Ocean"), N_("Tongan Pa'anga"), N_("Tonga"), "TOP", NULL, TRUE, "TOP.png" },
    { N_("Pacific Ocean"), N_("United States Dollar"), N_("Panama"), "USD", "$", TRUE, "PAB.png" },
    { N_("Pacific Ocean"), N_("Vanuatu Vatu"), N_("Vanuatu"), "VUV", NULL, TRUE, "VUV.png" },
    { N_("Southern America"), N_("Peso"), N_("Argentina"), "ARP", "$", TRUE, "ARP.png" },
    { N_("Southern America"), N_("Boliviano"), N_("Bolivia"), "BOB", "$", TRUE, "BOB.png" },
    { N_("Southern America"), N_("Peso"), N_("Chile"), "CLP", "$", TRUE, "CLP.png" },
    { N_("Southern America"), N_("Peso"), N_("Colombia"), "COP", "₱", TRUE, "COP.png" },
    { N_("Southern America"), N_("Ecuador Sucre"), N_("Ecuador"), "ECS", NULL, FALSE, "ECS.png" },
    { N_("Southern America"), N_("Guyana Dollar"), N_("Guyana"), "GYD", NULL, TRUE, "GYD.png" },
    { N_("Southern America"), N_("Paraguay Guarani"), N_("Paraguay"), "PYG", NULL, TRUE, "PYG.png" },
    { N_("Southern America"), N_("Nuevos Soles"), N_("Peru"), "PEN", "S/.", TRUE, "PEN.png" },
    { N_("Southern America"), N_("Real"), N_("Brazil"), "BRL", "R$", TRUE, "BRL.png" },
    { N_("Southern America"), N_("Suriname Guilder"), N_("Suriname"), "SRG", NULL, TRUE, "SRD.png" },
    { N_("Southern America"), N_("Peso"), N_("Uruguay"), "UYU", "₱", TRUE, "UYU.png" },
    { N_("Southern America"), N_("Venezuelan Bolivar"), N_("Venezuela"), "VEB", NULL, TRUE, "VEB.png" },
    { NULL },
};


GtkWidget *entree_nom, *entree_code, *entree_iso_code;

GtkTreeModel * currency_list_model;
GtkTreeView * currency_list_view;




/** Exchange rates cache, used by update_exchange_rate_cache and
  cached_exchange_rates */
GSList * cached_exchange_rates = NULL;

GtkWidget *bouton_supprimer_devise;
GtkWidget *entree_nom_devise_parametres;
GtkWidget *entree_iso_code_devise_parametres;
GtkWidget *entree_code_devise_parametres;


/**
 *
 *
 *
 */
gboolean select_currency_in_iso_list ( GtkTreeSelection *selection, GtkTreeModel *model )
{
    gchar * currency_name, * currency_iso_code, * currency_nickname;
    GtkWidget * entry_name, * entry_iso_code, * entry_code;
    GtkTreeIter iter;

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    entry_name = g_object_get_data ( G_OBJECT(model), "entry_name" );
    entry_iso_code = g_object_get_data ( G_OBJECT(model), "entry_iso_code" );
    entry_code = g_object_get_data ( G_OBJECT(model), "entry_code" );

    gtk_tree_model_get ( model, &iter, 
			 CURRENCY_NAME_COLUMN, &currency_name,
			 CURRENCY_ISO_CODE_COLUMN, &currency_iso_code,
			 CURRENCY_NICKNAME_COLUMN, &currency_nickname, 
			 -1 );

    if ( ! currency_name ) 
	currency_name = "";
    if ( ! currency_nickname ) 
	currency_nickname = "";
    if ( ! currency_iso_code ) 
	currency_iso_code = "";

    gtk_entry_set_text ( GTK_ENTRY ( entry_name ), currency_name );
    gtk_entry_set_text ( GTK_ENTRY ( entry_iso_code ), currency_iso_code );
    gtk_entry_set_text ( GTK_ENTRY ( entry_code ), currency_nickname );

    return ( FALSE );
} 



/**
 * TODO: document this
 *
 */
void fill_currency_list ( GtkTreeView * view, gboolean include_obsolete )
{
    GtkTreeModel * model;
    GtkTreeIter child_iter;
    struct iso_4217_currency * currency = iso_4217_currencies;

    model = gtk_tree_view_get_model ( view );

    while (currency -> country_name )
    {
	if ( include_obsolete || currency -> active )
	{
	    GdkPixbuf * pixbuf;
	    
	    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, 
							     C_DIRECTORY_SEPARATOR,
							     "flags", 
							     C_DIRECTORY_SEPARATOR,
							     currency -> flag_filename, 
							     NULL ),
						NULL );	
	    
	    gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, NULL);
	    gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
				CURRENCY_FLAG_COLUMN, pixbuf,
				COUNTRY_NAME_COLUMN, g_strconcat ( " ", 
								   _(currency -> country_name),
								   NULL ),
				CURRENCY_NAME_COLUMN, _(currency -> currency_name),
				CURRENCY_ISO_CODE_COLUMN, _(currency -> currency_code),
				CURRENCY_NICKNAME_COLUMN, _(currency -> currency_nickname),
				CURRENCY_POINTER_COLUMN, currency,
				CURRENCY_HAS_FLAG, TRUE,
				-1);
	}
	currency++;
    }

    gtk_tree_view_expand_all ( view );
}


/**
 * TODO: document this
 *
 */
gboolean rebuild_currency_list ( GtkWidget * checkbox, GtkTreeView * view )
{
    GtkTreeModel * model;

    model = gtk_tree_view_get_model ( view );
    gtk_tree_store_clear ( GTK_TREE_STORE (model) );
    fill_currency_list ( view, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) );
    return FALSE;
}


/**
 *
 *
 */
GtkWidget * new_currency_tree ()
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeStore * model;
    GtkWidget * treeview;
    gint col_offset;

    /* Create tree store */
    model = gtk_tree_store_new (NUM_CURRENCIES_COLUMNS,
				GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_INT );

    /* Create tree view */
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

    /* Flag */
    cell = gtk_cell_renderer_pixbuf_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Country name"),
						     cell, "pixbuf",
						     CURRENCY_FLAG_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), cell,
				       "visible", CURRENCY_HAS_FLAG);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Country name */
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), cell, TRUE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), cell, 
				       "text", COUNTRY_NAME_COLUMN);

    /* Currency name */
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
    gtk_tree_view_column_set_alignment ( column, 0.5 );
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
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), 
					  COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

    return treeview;
}


/**
 * TODO: document this
 *
 */
GtkWidget * new_currency_vbox ()
{ 
    GtkWidget * sw, * treeview, * vbox, * checkbox;
    GtkTreeModel * model;

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_ALWAYS);

    treeview = new_currency_tree ();
    gtk_widget_set_usize ( treeview, FALSE, 200 );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW(treeview) );
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW ( treeview ) ), 
		       "changed", G_CALLBACK ( select_currency_in_iso_list ), 
		       model );

    gtk_container_add (GTK_CONTAINER (sw), treeview);
    gtk_container_set_resize_mode (GTK_CONTAINER (sw), GTK_RESIZE_PARENT);

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(vbox), sw,
			 TRUE, TRUE, 0 );

    checkbox = gtk_check_button_new_with_label ( _("Include obsolete currencies"));
    gtk_box_pack_start ( GTK_BOX(vbox), checkbox,
			 FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT(checkbox), "toggled",
		       (GCallback) rebuild_currency_list, treeview );

    fill_currency_list ( GTK_TREE_VIEW(treeview), FALSE );

    g_object_set_data ( G_OBJECT(vbox), "model", model );
    g_object_set_data ( G_OBJECT(vbox), "treeview", treeview );

    return vbox;
}




/**
 * Pop up a dialog to create a new currency, do some sanity checks and
 * call the create_currency() function to do the grunt work.
 * 
 * \param widget	GtkButton that triggered event.
 * 
 * \return TRUE if currency has been created.
 */
gboolean ajout_devise ( GtkWidget *widget )
{
    GtkWidget *dialog, *label, *table, *model, *list, *paddingbox;
    const gchar *nom_devise, *code_devise, *code_iso4217_devise;
    gint currency_number;
    gint resultat;

    dialog = gtk_dialog_new_with_buttons ( _("Add a currency"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL,0,
					   GTK_STOCK_OK,1,
					   NULL );

    gtk_container_set_border_width ( GTK_CONTAINER ( dialog ), 12 );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   TRUE, _("ISO 4217 currencies"));

    /* Create list */
    list = new_currency_vbox ();
    model = g_object_get_data ( G_OBJECT(list), "model" );

    gtk_box_pack_start ( GTK_BOX(paddingbox) , list, TRUE, TRUE, 5 );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   FALSE, _("Currency details"));

    /* Create table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Currency name */
    label = gtk_label_new (COLON(_("Currency name")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_nom = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree_nom ), TRUE );
    gtk_table_attach ( GTK_TABLE ( table ), entree_nom, 1, 2, 0, 1,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_name", entree_nom );

    /* Currency ISO code */
    label = gtk_label_new (COLON(_("Currency ISO 4217 code")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_iso_code = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entree_iso_code, 1, 2, 1, 2,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_iso_code", entree_iso_code );

    /* Currency usual sign */
    label = gtk_label_new (COLON(_("Currency sign")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_code = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entree_code, 1, 2, 2, 3,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_code", entree_code );

    /* Select default currency. */
    gtk_tree_model_foreach ( GTK_TREE_MODEL(model), 
			     (GtkTreeModelForeachFunc) gsb_gui_select_default_currency, 
			     g_object_get_data ( G_OBJECT(list), "treeview" ) );

  reprise_dialog:
    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case 1 :

	    nom_devise = gtk_entry_get_text ( GTK_ENTRY ( entree_nom ));
	    code_devise = gtk_entry_get_text ( GTK_ENTRY ( entree_code ));
	    code_iso4217_devise = gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code ));

	    if ( strlen ( nom_devise ) && 
		 (strlen ( code_devise ) ||
		  strlen ( code_iso4217_devise )))
	    {
		/* 		recherche si la devise existe dÃ©jÃ , si c'est le cas, on refuse */

		if ( gsb_data_currency_get_number_by_name ( nom_devise )
		     ||
		     gsb_data_currency_get_number_by_code_iso4217 ( code_iso4217_devise ))
		{
		    dialogue_error_hint ( _("Currency names or iso 4217 codes should be unique.  Please choose a new name for the currency."),
					  g_strdup_printf ( _("Currency '%s' already exists." ), nom_devise ));
		    goto reprise_dialog;
		}

		currency_number = create_currency ( nom_devise, code_devise, code_iso4217_devise);

		if ( widget )
		{
		    append_currency_to_currency_list ( GTK_TREE_STORE ( currency_list_model ),
						       currency_number );
		    gsb_currency_update_currency_list ();
		    modification_fichier ( TRUE );
		    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
		    return TRUE;
		}
	    }
	    else
	    {
		dialogue_warning_hint ( _("Currency name and either currency ISO4217 code or currency nickname should be set."),
					_("All fields are not filled in") );
		goto reprise_dialog;
	    }
	    break;

	default:
	    /* 	    si on veut partir, mais qu'aucune devise n'existe, on met un message et on retourne au */
	    /* 		choix de la devise */

	    if ( !gsb_data_currency_get_currency_list () )
	    {
		gtk_widget_destroy ( GTK_WIDGET ( dialog ));
		return FALSE;
	    }
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
    return TRUE;
}


/**
 * Remove selected currency from currency list.  First, be sure there
 * is no use for it.
 *
 * \param bouton	Widget that triggered event.
 * \param view		GtkTreeView that contains selected currency.
 */
void retrait_devise ( GtkWidget *bouton, GtkWidget * view )
{
    gint devise_trouvee = 0;
    GSList *list_tmp_transactions;
    gint currency_number;

    currency_number = currency_get_selected ( GTK_TREE_VIEW ( view ) );
    if ( !currency_number )
    {
	return;
    }

    /* we look for that currency in all the transactions and scheduler,
     * if we find it, we cannot delete it */

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number;
	transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_currency_number (transaction_number) == currency_number )
	{
	    devise_trouvee = 1;
	    list_tmp_transactions = NULL;
	}
	else
	    list_tmp_transactions = list_tmp_transactions -> next;
    }

    if ( !devise_trouvee )
    {
	GSList *list_tmp;

	list_tmp = gsb_data_scheduled_get_scheduled_list ();

	while ( list_tmp )
	{
	    gint scheduled_number;

	    scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp -> data);
	    if ( gsb_data_scheduled_get_currency_number (scheduled_number) == currency_number )
	    {
		devise_trouvee = 1;
		list_tmp = NULL;
	    }
	    else
		list_tmp = list_tmp -> next;
	}
    }

    if ( devise_trouvee )
    {
	dialogue_error_hint ( g_strdup_printf ( _("Currency '%s' is used in current account.  Grisbi can't delete it."),
						 gsb_data_currency_get_name (currency_number)),
			      g_strdup_printf ( _("Impossible to remove currency '%s'"), 
						gsb_data_currency_get_name (currency_number) ));
	return;
    }

    remove_selected_currency_from_currency_view ( GTK_TREE_VIEW(view) );
    gsb_data_currency_remove (currency_number);
}


/**
 * Creates the currency list and associated form to configure them.
 *
 * \returns A newly created vbox
 */
GtkWidget *onglet_devises ( void )
{
    GtkWidget *vbox_pref, *label, *paddingbox, *bouton, *hbox;
    GtkWidget *scrolled_window, *vbox, *table;
    GSList *liste_tmp;
    
    vbox_pref = new_vbox_with_title_and_icon ( _("Currencies"), "currencies.png" ); 
    paddingbox = new_paddingbox_with_title (vbox_pref, TRUE, _("Known currencies"));
    
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, TRUE, TRUE, 0);

    /* Currency list */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    /* Create it. */
    currency_list_view = GTK_TREE_VIEW ( new_currency_tree () );
    currency_list_model = gtk_tree_view_get_model ( currency_list_view );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), GTK_WIDGET(currency_list_view) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window, TRUE, TRUE, 0);
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW ( currency_list_view ) ), 
		       "changed", G_CALLBACK ( select_currency_in_iso_list ), 
		       currency_list_model );

    /*   s'il n'y a pas de fichier ouvert, on grise */
    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    else
    {
	/* remplissage de la liste avec les devises temporaires */

	liste_tmp = gsb_data_currency_get_currency_list ();

	while ( liste_tmp )
	{
	    gint currency_number;

	    currency_number = gsb_data_currency_get_no_currency (liste_tmp -> data);

	    append_currency_to_currency_list ( GTK_TREE_STORE ( currency_list_model ),
					       currency_number );
	    liste_tmp = liste_tmp -> next;
	}

    }

    /* Create Add/Remove buttons */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, FALSE, FALSE, 0 );

    /* Button "Add" */
    bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ), "clicked",
				GTK_SIGNAL_FUNC  ( ajout_devise ),
				currency_list_model );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton, FALSE, FALSE, 5 );

    /* Button "Remove" */
    bouton_supprimer_devise = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    g_object_set_data ( G_OBJECT(bouton_supprimer_devise), "view", currency_list_view );
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_devise ), "clicked",
			 GTK_SIGNAL_FUNC  ( retrait_devise ), currency_list_view );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_devise, FALSE, FALSE, 5 );

    /* Input form for currencies */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Currency properties"));

    /* Create table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Create currency name entry */
    label = gtk_label_new (COLON(_("Name")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_nom_devise_parametres = new_text_entry ( NULL, (GCallback) changement_nom_entree_devise, NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entree_nom_devise_parametres, 1, 2, 0, 1, 
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_list_model), "entry_name", entree_nom_devise_parametres );
    g_object_set_data ( G_OBJECT(entree_nom_devise_parametres), "view", currency_list_view );

    /* Create code entry */
    label = gtk_label_new (COLON(_("Sign")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_code_devise_parametres = new_text_entry ( NULL, (GCallback) changement_code_entree_devise, NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entree_code_devise_parametres, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_list_model), "entry_code", entree_code_devise_parametres );
    g_object_set_data ( G_OBJECT(entree_code_devise_parametres), "view", currency_list_view );

    /* Create code entry */
    label = gtk_label_new ( COLON(_("ISO code")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entree_iso_code_devise_parametres = new_text_entry ( NULL, (GCallback) changement_iso_code_entree_devise, 
							 NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entree_iso_code_devise_parametres, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_list_model), "entry_iso_code", entree_iso_code_devise_parametres );
    g_object_set_data ( G_OBJECT(entree_iso_code_devise_parametres), "view", currency_list_view );

    return ( vbox_pref );
}



/**
 *
 *
 *
 */
void append_currency_to_currency_list ( GtkTreeStore * model,
					gint currency_number )
{
    GdkPixbuf * pixbuf;
    GtkTreeIter iter;

    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "flags", C_DIRECTORY_SEPARATOR,
						     gsb_data_currency_get_code_iso4217 (currency_number),
						     ".png", NULL ),
					NULL );	

    gtk_tree_store_append (GTK_TREE_STORE(model), &iter, NULL);
    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
			 CURRENCY_FLAG_COLUMN, pixbuf,
			 COUNTRY_NAME_COLUMN, "",
			 CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
			 CURRENCY_ISO_CODE_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
			 CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_code (currency_number),
			 CURRENCY_POINTER_COLUMN, currency_number,
			 CURRENCY_HAS_FLAG, TRUE,
			 -1);
}



/**
 *
 *
 */
GtkWidget *tab_display_totals ( void )
{
    GtkWidget *vbox_pref, *currency_list, *table, *label;

    vbox_pref = new_vbox_with_title_and_icon ( _("Totals currencies"),
					       "currencies.png" );

    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );

    label = gtk_label_new (_("Currency for payees tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 
		       0, 1, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    currency_list = new_currency_option_menu ( &no_devise_totaux_tiers, 
					       remplit_arbre_tiers );
    gtk_table_attach ( GTK_TABLE ( table ), currency_list,
		       1, 2, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    label = gtk_label_new (_("Currency for categories tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    currency_list = new_currency_option_menu ( &no_devise_totaux_categ, 
					       remplit_arbre_categ );
    gtk_table_attach ( GTK_TABLE ( table ), currency_list,
		       1, 2, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    label = gtk_label_new (_("Currency for budgetary lines tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 2, 3, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    currency_list = new_currency_option_menu ( &no_devise_totaux_ib, 
					       remplit_arbre_imputation );
    gtk_table_attach ( GTK_TABLE ( table ), currency_list,
		       1, 2, 2, 3, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), table, TRUE, TRUE, 0);

    return ( vbox_pref );
}


/**
 * Create a new GtkOptionMenu with a pointer to an integerthat will be
 * modified according to the entry's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler
 *
 * \return A newly allocated option menu.
 */
GtkWidget * new_currency_option_menu ( gint * value, GCallback hook )
{
    GtkWidget * currency_list;

    currency_list = gsb_currency_make_combobox (FALSE);
    /* FIXME : vérifier que value est bien un pointer vers le no de devise */
    if (value && *value)
	gsb_currency_set_combobox_history ( currency_list,
					    *value );

/*     g_signal_connect ( GTK_OBJECT (currency_list), "changed", (GCallback) set_int_from_menu, value ); */
    g_signal_connect ( GTK_OBJECT (currency_list), "changed", (GCallback) hook, value );
    g_object_set_data ( G_OBJECT ( currency_list ), "pointer", value);

    return currency_list;
}
/**
 * Set an integer to the value of a menu.  Normally called via a GTK
 * "changed" signal handler.
 * 
 * \param menu a pointer to a menu widget.
 * \param dummy unused
 */
/* gboolean set_int_from_menu ( GtkWidget *combobox, gint * dummy) */
/* { */
/*     gint *data; */
/*      */
/*     data = g_object_get_data ( G_OBJECT(menu), "pointer" ); */
/*  */
/*     if ( data ) */
/*     { */
/* 	*data = gsb_currency_get_currency_from_combobox (combobox); */
/*     } */

    /* Mark file as modified */
/*     modification_fichier ( TRUE ); */
/*     return (FALSE); */
/* } */



/**
 * Obtain selected currency from currency tree.
 *
 * \param view		GtkTreeView to remove currency from.
 * 
 * \return		the number of the selected currency.
 */
gint currency_get_selected ( GtkTreeView * view )
{
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( view );
    GtkTreeIter iter;
    GtkTreeModel * tree_model;
    gint currency_number;

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gtk_tree_model_get ( tree_model, &iter, 
			 CURRENCY_POINTER_COLUMN, &currency_number,
			 -1 );

    return currency_number;
}



/**
 * Remove selected currency from tree.  In fact, this is a generic
 * function that could be used for any purpose (and could be then
 * renamed).
 *
 * \param view	GtkTreeView to remove selected entry from.
 */
void remove_selected_currency_from_currency_view ( GtkTreeView * view )
{
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( view );
    GtkTreeIter iter;
    GtkTreeModel * tree_model;

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return;

    gtk_tree_store_remove ( GTK_TREE_STORE (tree_model), &iter );
}



/**
 *
 *
 */
gboolean changement_nom_entree_devise ( GtkEditable *editable, gchar * text,
					gint length, gpointer data )
{
    gint currency_number;
    GtkTreeSelection * selection;
    GtkTreeModel * tree_model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "view" ) );
    currency_number = currency_get_selected ( g_object_get_data ( G_OBJECT (editable), "view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gsb_data_currency_set_name ( currency_number,
				 gtk_entry_get_text ( GTK_ENTRY ( entree_nom_devise_parametres )));

    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ), &iter,
			 CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
			 -1);
    gsb_currency_update_currency_list ();

    return FALSE;
}



gboolean changement_code_entree_devise ( GtkEditable *editable, gchar * text,
					 gint length, gpointer data )
{
    gint currency_number;
    GtkTreeModel * tree_model;
    GtkTreeSelection * selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "view" ) );
    currency_number = currency_get_selected ( g_object_get_data ( G_OBJECT (editable), "view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gsb_data_currency_set_code ( currency_number,
				 gtk_entry_get_text ( GTK_ENTRY ( entree_code_devise_parametres )));

    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ), &iter,
			 CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_code (currency_number),
			 -1);
    gsb_currency_update_currency_list ();

    return FALSE;
}



gboolean changement_iso_code_entree_devise ( GtkEditable *editable, gchar * text,
					     gint length, gpointer data )
{
    gint currency_number;
    GtkTreeModel * tree_model;
    GtkTreeSelection * selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "view" ) );
    currency_number = currency_get_selected ( g_object_get_data ( G_OBJECT (editable), "view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gsb_data_currency_set_code_iso4217 ( currency_number,
					 gtk_entry_get_text ( GTK_ENTRY (entree_iso_code_devise_parametres)));


    gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ), &iter,
			 CURRENCY_ISO_CODE_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
			 -1);
    gsb_currency_update_currency_list ();

    return FALSE;
}
/* **************************************************************************************************************************** */



/**
 * Find whether echange rate between two currencies is known.  If so,
 * returns a cached_exchange_rate structure with exchange rate
 * information.
 *
 * \param currency1 First currency
 * \param currency2 Second currency
 *
 * \return FALSE on failure, a pointer to a cached_exchange_rate
 * structure on success.
 */
struct cached_exchange_rate *cached_exchange_rate ( gint currency1_number, 
						    gint currency2_number )
{
    GSList * liste_tmp = cached_exchange_rates;
    struct cached_exchange_rate * tmp;

    while ( liste_tmp )
    {
	tmp = liste_tmp -> data;
	if ( currency1_number == tmp -> currency1_number && currency2_number == tmp -> currency2_number )
	    return tmp;

	liste_tmp = liste_tmp -> next;
    }

    return NULL;
}


/**
 * Update exchange rate cache according to arguments.
 *
 * \param currency1 First currency.
 * \param currency2 Second currency.
 * \param change    Exchange rate between two currencies.
 * \param fees      Fees of transaction.
 */
void update_exchange_rate_cache ( gint currency1_number, 
				  gint currency2_number,
				  gdouble change, gdouble fees )
{
    struct cached_exchange_rate * tmp;

    tmp = (struct cached_exchange_rate *) g_malloc(sizeof(struct cached_exchange_rate));

    tmp -> currency1_number = currency1_number;
    tmp -> currency2_number = currency2_number;
    tmp -> rate = change;
    tmp -> fees = fees;

    cached_exchange_rates = g_slist_append ( cached_exchange_rates, tmp );
}
/* ***************************************************************************************** */



/* ***************************************************************************************** */
gint create_currency ( const gchar *nom_devise,
		       const gchar *code_devise, 
		       const gchar *code_iso4217_devise )
{
    gint currency_number;

    currency_number = gsb_data_currency_new (nom_devise);
    gsb_data_currency_set_code ( currency_number,
				 code_devise );
    gsb_data_currency_set_code_iso4217 ( currency_number,
					 code_iso4217_devise );
    return currency_number;
}



/**
 *
 *
 *
 */
gint find_currency_from_iso4217_list ( gchar * currency_name )
{
  struct iso_4217_currency * currency = iso_4217_currencies;

  while ( currency -> country_name )
    {
      if ( !strcmp ( currency -> currency_code, currency_name ) )
	return create_currency ( currency -> currency_name, 
				 currency -> currency_nickname, 
				 currency -> currency_code );
      currency++;
    }

  return FALSE;
}


/**
 *
 *
 */
gboolean gsb_gui_select_default_currency ( GtkTreeModel * tree_model, GtkTreePath * path, 
					   GtkTreeIter * iter, GtkTreeView * view )
{
    struct lconv * conv = localeconv();
    gchar * code, * symbol, * country;
    gboolean good = FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 CURRENCY_ISO_CODE_COLUMN, &code, 
			 COUNTRY_NAME_COLUMN, &country, 
			 -1 );
    if ( conv && conv -> int_curr_symbol && strlen ( conv -> int_curr_symbol ) )
    {
	symbol = g_strstrip ( my_strdup ( conv -> int_curr_symbol ));
	if ( ! strcmp ( code, symbol ) )
	{
	    good = TRUE;
	}
	free ( symbol );
    }
    else
    {
	symbol = g_strstrip ( my_strdup ( country ) );
	if ( ! strcmp ( symbol, _("United States") ) )
	{
	    good = TRUE;
	}
	free ( symbol );
    }

    if ( good )
    {
	gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( view ), path );
	gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (view), path, NULL, TRUE, 0.5, 0 );
	return TRUE;
    }
    
    return FALSE;
}


