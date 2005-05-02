/* Fichier devises.c */
/* s'occupe de tout ce qui concerne les devises */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003 Benjamin Drieu (bdrieu@april.org) */
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
#include "devises.h"
#include "dialog.h"
#include "utils_devises.h"
#include "utils_editables.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "comptes_gestion.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "etats_config.h"
#include "utils_buttons.h"
#include "affichage_formulaire.h"
#include "operations_formulaire.h"
#include "structures.h"
#include "echeancier_formulaire.h"
#include "fichier_configuration.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint bloque_echap_choix_devise ( GtkWidget *dialog,
				 GdkEventKey *key,
				 gpointer null );
static struct cached_exchange_rate *cached_exchange_rate ( struct struct_devise * currency1, 
						    struct struct_devise * currency2 );
static gboolean changement_code_entree_devise ( void );
static gboolean changement_iso_code_entree_devise ( void );
static gboolean changement_nom_entree_devise ( void );
static struct struct_devise *create_currency ( gchar * nom_devise, gchar * code_devise, gchar * code_iso4217_devise );
static gboolean deselection_ligne_devise ( GtkWidget *liste,
				    gint ligne,
				    gint colonne,
				    GdkEventButton *ev,
				    GtkWidget *frame );
static gboolean devise_selectionnee ( GtkWidget *menu_shell, gint origine );
static void fill_currency_list ( GtkTreeView * view, gboolean include_obsolete );
static GtkWidget * new_currency_list ();
static GtkWidget * new_currency_option_menu ( gint * value, GCallback hook );
static gboolean rebuild_currency_list ( GtkWidget * checkbox, GtkTreeView * view );
static void retrait_devise ( GtkWidget *bouton,
		      GtkWidget *liste );
static gboolean select_currency_in_iso_list (GtkTreeSelection *selection,
				      GtkTreeModel *model);
static gboolean selection_ligne_devise ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev,
				  GtkWidget *frame );
static gint sort_tree (GtkTreeModel *model,
		GtkTreeIter *a,
		GtkTreeIter *b,
		gpointer user_data);
static void update_currency_widgets();
static void update_exchange_rate_cache ( struct struct_devise * currency1, 
				  struct struct_devise * currency2,
				  gdouble change, gdouble fees );
/*END_STATIC*/



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
    { N_("Africa"), N_("Egyptian Pound"), N_("Egypt"), "EGP", NULL, TRUE, "EGP.png" },
    { N_("Africa"), N_("Ethiopian Birr"), N_("Ethiopia"), "ETB", NULL, TRUE, "ETB.png" },
    { N_("Africa"), N_("Gambian Dalasi"), N_("Gambia"), "GMD", NULL, TRUE, "GMD.png" },
    { N_("Africa"), N_("Ghana Cedi"), N_("Ghana"), "GHC", NULL, TRUE, "GHC.png" },
    { N_("Africa"), N_("Guinea-Bissau Peso"), N_("Guinea-Bissau"), "GWP", NULL, TRUE, "GWP.png" },
    { N_("Africa"), N_("Kenyan Shilling"), N_("Kenya"), "KES", NULL, TRUE, "KES.png" },
    { N_("Africa"), N_("Liberian Dollar"), N_("Liberia"), "LRD", NULL, TRUE, "LRD.png" },
    { N_("Africa"), N_("Libyan Dinar"), N_("Libyan Arab Jamahiriya"), "LYD", NULL, TRUE, "LYD.png" },
    { N_("Africa"), N_("Malagasy Franc"), N_("Madagascar"), "MGF", NULL, TRUE, "MGF.png" },
    { N_("Africa"), N_("Malawi Kwacha"), N_("Malawi"), "MWK", NULL, TRUE, "MWK.png" },
    { N_("Africa"), N_("Mauritania Ouguiya"), N_("Mauritania"), "MRO", NULL, TRUE, "MRO.png" },
    { N_("Africa"), N_("Moazambique Metical"), N_("Mozambique"), "MZM", NULL, TRUE, "MZM.png" },
    { N_("Africa"), N_("Moroccan Dirham"), N_("Morocco"), "MAD", NULL, TRUE, "MAD.png" },
    { N_("Africa"), N_("Nigerian Naira"), N_("Nigeria"), "NGN", "₦", TRUE, "NGN.png" },
    { N_("Africa"), N_("Rwanda Franc"), N_("Rwanda"), "RWF", NULL, TRUE, "RWF.png" },
    { N_("Africa"), N_("Sao Tome and Principe Dobra"), N_("Sao Tome and Principe"), "STD", NULL, TRUE, "STD.png" },
    { N_("Africa"), N_("Seychelles Rupee"), N_("Seychelles"), "SCR", NULL, TRUE, "SCR.png" },
    { N_("Africa"), N_("Sierra Leonean Leone"), N_("Sierra Leone"), "SLL", NULL, TRUE, "SLL.png" },
    { N_("Africa"), N_("Somali Shilling"), N_("Somalia"), "SOS", NULL, TRUE, "SOS.png" },
    { N_("Africa"), N_("South African Rand"), N_("Lesotho"), "ZAR", NULL, TRUE, "LSL.png" },
    { N_("Africa"), N_("South African Rand"), N_("Namibia"), "ZAR", NULL, TRUE, "NAD.png" },
    { N_("Africa"), N_("South African Rand"), N_("South Africa"), "ZAR", NULL, TRUE, "ZAR.png" },
    { N_("Africa"), N_("Swaziland Lilangeni"), N_("Swaziland"), "SZL", NULL, TRUE, "SZL.png" },
    { N_("Africa"), N_("Tanzanian Shilling"), N_("United Republic of Tanzania"), "TZS", NULL, TRUE, "TZS.png" },
    { N_("Africa"), N_("Tunisian Dinar"), N_("Tunisia"), "TND", NULL, TRUE, "TND.png" },
    { N_("Africa"), N_("Zambian Kwacha"), N_("Zambia"), "ZMK", NULL, TRUE, "ZMK.png" },
    { N_("Africa"), N_("Zimbabwe Dollar"), N_("Zimbabwe"), "ZWD", NULL, TRUE, "ZWD.png" },
    { N_("Asia"), N_("Afghani"), N_("Afghanistan"), "AFA", NULL, TRUE, "AFN.png" },
    { N_("Asia"), N_("Bahraini Dinar"), N_("Bahrain"), "BHD", NULL, TRUE, "BHD.png" },
    { N_("Asia"), N_("Bangladesh Taka"), N_("Bangladesh"), "BDT", NULL, TRUE, "BDT.png" },
    { N_("Asia"), N_("Brunei Dollar"), N_("Brunei Darussalam"), "BND", NULL, TRUE, "BND.png" },
    { N_("Asia"), N_("Cambodian Riel"), N_("Cambodia"), "KHR", NULL, TRUE, "KHR.png" },
    { N_("Asia"), N_("Cyprus Pound"), N_("Cyprus"), "CYP", NULL, TRUE, "CYP.png" },
    { N_("Asia"), N_("Hong Kong Dollar"), N_("Hong Kong"), "HKD", NULL, TRUE, "HKD.png" },
    { N_("Asia"), N_("Indian Rupee"), N_("Bhutan"), "INR", "₨", TRUE, "BHU.png" },
    { N_("Asia"), N_("Indian Rupee"), N_("India"), "INR", "₨", TRUE, "INR.png" },
    { N_("Asia"), N_("Indonesian Rupiah"), N_("Indonesia"), "IDR", NULL, TRUE, "IDR.png" },
    { N_("Asia"), N_("Iranian Rial"), N_("Iran"), "IRR", NULL, TRUE, "IRR.png" },
    { N_("Asia"), N_("Iraqi Dinar"), N_("Iraq"), "IQD", NULL, TRUE, "IQD.png" },
    { N_("Asia"), N_("Japanese Yen"), N_("Japan"), "JPY", "¥", TRUE, "JPY.png" },
    { N_("Asia"), N_("Jordanian Dinar"), N_("Jordan"), "JOD", NULL, TRUE, "JOD.png" },
    { N_("Asia"), N_("Kuwaiti Dinar"), N_("Kuwait"), "KWD", NULL, TRUE, "KWD.png" },
    { N_("Asia"), N_("Lao Kip"), N_("Lao People's Democratic Republic"), "LAK", "₭", TRUE, "LAK.png" },
    { N_("Asia"), N_("Lebanese Pound"), N_("Lebanon"), "LBP", NULL, TRUE, "LBP.png" },
    { N_("Asia"), N_("Macau Pataca"), N_("Macao"), "MOP", NULL, TRUE, "MOP.png" },
    { N_("Asia"), N_("Malaysian Ringgit"), N_("Malaysia"), "MYR", NULL, TRUE, "MYR.png" },
    { N_("Asia"), N_("Mongolian Tugrik"), N_("Mongolia"), "MNT", "₮", TRUE, "MNT.png" },
    { N_("Asia"), N_("Nepalese Rupee"), N_("Nepal"), "NPR", NULL, TRUE, "NPR.png" },
    { N_("Asia"), N_("New Israeli Shekel"), N_("Israel"), "ILS", "₪", TRUE, "ILS.png" },
    { N_("Asia"), N_("New Taiwan Dollar"), N_("Taiwan, Province of China"), "TWD", NULL, TRUE, "TWD.png" },
    { N_("Asia"), N_("North Korean Won"), N_("Democratic People's Republic of Korea"), "KPW", "₩", TRUE, "KPW.png" },
    { N_("Asia"), N_("Pakistan Rupee"), N_("Pakistan"), "PKR", NULL, TRUE, "PKR.png" },
    { N_("Asia"), N_("Philippine peso"), N_("Philippines"), "PHP", NULL, TRUE, "PHP.png" },
    { N_("Asia"), N_("Qatari Rial"), N_("Qatar"), "QAR", NULL, TRUE, "QAR.png" },
    { N_("Asia"), N_("Rial Omani"), N_("Oman"), "OMR", NULL, TRUE, "OMR.png" },
    { N_("Asia"), N_("Russian Ruble"), N_("Russia"), "RUR", NULL, TRUE, "RUB.png" },
    { N_("Asia"), N_("Saudi Riyal"), N_("Saudi Arabia"), "SAR", NULL, TRUE, "SAR.png" },
    { N_("Asia"), N_("Singapore Dollar"), N_("Singapore"), "SGD", NULL, TRUE, "SGD.png" },
    { N_("Asia"), N_("South Korean Won"), N_("Republic of Korea"), "KRW", "₩", TRUE, "KRW.png" },
    { N_("Asia"), N_("Sri Lanka Rupee"), N_("Sri Lanka"), "LKR", NULL, TRUE, "LKR.png" },
    { N_("Asia"), N_("Syrian Pound"), N_("Syrian Arab Republic"), "SYP", NULL, TRUE, "SYP.png" },
    { N_("Asia"), N_("Thai Baht"), N_("Thailand"), "THB", NULL, TRUE, "THB.png" },
    { N_("Asia"), N_("Turkish Lira"), N_("Turkey"), "TRL", "₤", TRUE, "TRL.png" },
    { N_("Asia"), N_("United Arab Emirates Dirham"), N_("United Arab Emirates"), "AED", NULL, TRUE, "AED.png" },
    { N_("Asia"), N_("Viet Nam Dong"), N_("Viet Nam"), "VND", "₫", TRUE, "VND.png" },
    { N_("Asia"), N_("Yemeni Rial"), N_("Yemen"), "YER", NULL, TRUE, "YER.png" },
    { N_("Asia"), N_("Yuan Renminbi"), N_("China"), "CNY", NULL, TRUE, "CNY.png" },
    { N_("Central America"), N_("Belize Dollar"), N_("Belize"), "BZD", NULL, TRUE, "BZD.png" },
    { N_("Central America"), N_("Costa Rican Colon"), N_("Costa Rica"), "CRC", "₡", TRUE, "CRC.png" },
    { N_("Central America"), N_("Guatemalan Quetzal"), N_("Guatemala"), "GTQ", NULL, TRUE, "GTQ.png" },
    { N_("Central America"), N_("Honduran Lempira"), N_("Honduras"), "HNL", NULL, TRUE, "HNL.png" },
    { N_("Central America"), N_("Mexican Peso"), N_("Mexico"), "MXP", NULL, FALSE, "MXN.png" },
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
    { N_("Europe"), N_("French Franc"), N_("France"), "FRF", NULL, FALSE, "FRF.png" },
    { N_("Europe"), N_("Gibraltar Pound"), N_("Gibraltar"), "GIP", NULL, TRUE, "GIP.png" },
    { N_("Europe"), N_("Greek Drachma"), N_("Greece"), "GRD", "₯", FALSE, "GRD.png" },
    { N_("Europe"), N_("Hungarian Forint"), N_("Hungary"), "HUF", NULL, TRUE, "HUF.png" },
    { N_("Europe"), N_("Iceland Krona"), N_("Iceland"), "ISK", NULL, TRUE, "ISK.png" },
    { N_("Europe"), N_("Irish Pound"), N_("Ireland"), "IEP", NULL, FALSE, "IEP.png" },
    { N_("Europe"), N_("Italian Lira"), N_("Holy See"), "ITL", "₤", FALSE, "VAT.png" },
    { N_("Europe"), N_("Italian Lira"), N_("Italy"), "ITL", "₤", FALSE, "ITL.png" },
    { N_("Europe"), N_("Italian Lira"), N_("San Marino"), "ITL", "₤", FALSE, "SAN.png" },
    { N_("Europe"), N_("Latvian Lat"), N_("Latvia"), "LVL", NULL, TRUE, "LVL.png" },
    { N_("Europe"), N_("Lithuanian Litas"), N_("Lietuva"), "LTL", NULL, TRUE, "LTL.png" },
    { N_("Europe"), N_("Luxembourg Franc"), N_("Luxembourg"), "LUF", NULL, FALSE, "LUF.png" },
    { N_("Europe"), N_("Netherlands Guilder"), N_("Netherlands"), "NLG", NULL, FALSE, "NLG.png" },
    { N_("Europe"), N_("New Yugoslavian Dinar"), N_("Serbia and Montenegro"), "YUD", NULL, FALSE, "YUV.png" },
    { N_("Europe"), N_("Norwegian Krone"), N_("Norway"), "NOK", NULL, TRUE, "NOK.png" },
    { N_("Europe"), N_("Polish Zloty"), N_("Poland"), "PLZ", NULL, FALSE, "PLN.png" },
    { N_("Europe"), N_("Portuguese Escudo"), N_("Portugal"), "PTE", NULL, FALSE, "PTE.png" },
    { N_("Europe"), N_("Pound Sterling"), N_("United Kingdom"), "GBP", "£", TRUE, "GBP.png" },
    { N_("Europe"), N_("Romanian Leu"), N_("Romania"), "ROL", NULL, TRUE, "ROL.png" },
    { N_("Europe"), N_("Slovak Koruna"), N_("Slovakia"), "SKK", NULL, TRUE, "SKK.png" },
    { N_("Europe"), N_("Slovene Tolar"), N_("Slovenia"), "SIT", NULL, TRUE, "SIT.png" },
    { N_("Europe"), N_("Spanish Peseta"), N_("Spain"), "ESP", "₧", FALSE, "ESP.png" },
    { N_("Europe"), N_("Swedish Krona"), N_("Sweden"), "SEK", NULL, TRUE, "SEK.png" },
    { N_("Europe"), N_("Swiss Franc"), N_("Liechtenstein"), "CHF", NULL, TRUE, "LIE.png" },
    { N_("Europe"), N_("Swiss Franc"), N_("Switzerland"), "CHF", NULL, TRUE, "CHF.png" },
    { N_("Europe"), N_("Hryvnia"), N_("Ukraine"), "UAH", NULL, TRUE, "UAH.png" },
    { N_("Northern America"), N_("Bahamian Dollar"), N_("Bahamas"), "BSD", NULL, TRUE, "BSD.png" },
    { N_("Northern America"), N_("Barbados Dollar"), N_("Barbados"), "BBD", NULL, TRUE, "BBD.png" },
    { N_("Northern America"), N_("Bermuda Dollar"), N_("Bermuda"), "BMD", NULL, TRUE, "BMD.png" },
    { N_("Northern America"), N_("Canadian Dollar"), N_("Canada"), "CAD", NULL, TRUE, "CAD.png" },
    { N_("Northern America"), N_("Cayman Islands Dollar"), N_("Cayman Islands"), "KYD", NULL, TRUE, "KYD.png" },
    { N_("Northern America"), N_("Cuban Peso"), N_("Cuba"), "CUP", NULL, TRUE, "CUP.png" },
    { N_("Northern America"), N_("Dominican Peso"), N_("Dominican Republic"), "DOP", NULL, TRUE, "DOP.png" },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Grenada"), "XCD", NULL, TRUE, "GRE.png" },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Saint Lucia"), "XCD", NULL, TRUE, "SLC.png" },
    { N_("Northern America"), N_("Haitian Gourde"), N_("Haiti"), "HTG", NULL, TRUE, "HTG.png" },
    { N_("Northern America"), N_("Jamaican Dollar"), N_("Jamaica"), "JMD", NULL, TRUE, "JMD.png" },
    { N_("Northern America"), N_("Netherlands Antillian Guilder"), N_("Netherlands Antilles"), "ANG", NULL, TRUE, "ANG.png" },
    { N_("Northern America"), N_("Trinidad and Tobago Dollar"), N_("Trinidad and Tobago"), "TTD", NULL, TRUE, "TTD.png" },
    { N_("Northern America"), N_("United States Dollar"), N_("United States"), "USD", "$", TRUE, "USD.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Australia"), "AUD", NULL, TRUE, "AUD.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Kiribati"), "AUD", NULL, TRUE, "KIR.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Nauru"), "AUD", NULL, TRUE, "NAU.png" },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Tuvalu"), "AUD", NULL, TRUE, "TUV.png" },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("French Polynesia"), "XPF", NULL, TRUE, "FRF.png" },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("New Caledonia"), "XPF", NULL, TRUE, "FRF.png" },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("Wallis and Futuna"), "XPF", NULL, TRUE, "FRF.png" },
    { N_("Pacific Ocean"), N_("Fiji Dollar"), N_("Fiji"), "FJD", NULL, TRUE, "FJD.png" },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("Cook Islands"), "NZD", NULL, TRUE, "COO.png" },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("New Zealand"), "NZD", NULL, TRUE, "NZD.png" },
    { N_("Pacific Ocean"), N_("Papua New Guinea Kina"), N_("Papua New Guinea"), "PGK", NULL, TRUE, "PGK.png" },
    { N_("Pacific Ocean"), N_("Samoa Tala"), N_("Samoa"), "WST", NULL, TRUE, "WST.png" },
    { N_("Pacific Ocean"), N_("Solomon Islands Dollar"), N_("Solomon Islands"), "SBD", NULL, TRUE, "SBD.png" },
    { N_("Pacific Ocean"), N_("Timor Escudo"), N_("Timor"), "TPE", NULL, TRUE, "TPE.png" },
    { N_("Pacific Ocean"), N_("Tongan Pa'anga"), N_("Tonga"), "TOP", NULL, TRUE, "TOP.png" },
    { N_("Pacific Ocean"), N_("United States Dollar"), N_("Panama"), "USD", "$", TRUE, "PAB.png" },
    { N_("Pacific Ocean"), N_("Vanuatu Vatu"), N_("Vanuatu"), "VUV", NULL, TRUE, "VUV.png" },
    { N_("Southern America"), N_("Chilean Peso"), N_("Chile"), "CLP", NULL, TRUE, "CLP.png" },
    { N_("Southern America"), N_("Colombian Peso"), N_("Colombia"), "COP", NULL, TRUE, "COP.png" },
    { N_("Southern America"), N_("Ecuador Sucre"), N_("Ecuador"), "ECS", NULL, FALSE, "ECS.png" },
    { N_("Southern America"), N_("Guyana Dollar"), N_("Guyana"), "GYD", NULL, TRUE, "GYD.png" },
    { N_("Southern America"), N_("Paraguay Guarani"), N_("Paraguay"), "PYG", NULL, TRUE, "PYG.png" },
    { N_("Southern America"), N_("Real"), N_("Brazil"), "BRL", "R$", TRUE, "BRL.png" },
    { N_("Southern America"), N_("Suriname Guilder"), N_("Suriname"), "SRG", NULL, TRUE, "SRD.png" },
    { N_("Southern America"), N_("Venezuelan Bolivar"), N_("Venezuela"), "VEB", NULL, TRUE, "VEB.png" },
    { NULL },
};




GtkWidget *entree_nom, *entree_code, *entree_iso_code;



/** Exchange rates cache, used by update_exchange_rate_cache and
  cached_exchange_rates */
GSList * cached_exchange_rates = NULL;

/** This structure holds informations needed for exchange rates
  cache. */
struct cached_exchange_rate {
    struct struct_devise * currency1;	/** First currency */
    struct struct_devise * currency2;	/** Second currency */
    gdouble rate;				/** Exchange rate betweend
						  currency1 and currency 2 */
    gdouble fees;				/** Fees associated with
						  exchange rate */
};


GSList *liste_struct_devises;
gint nb_devises;
gint no_derniere_devise;
struct struct_devise *devise_nulle;
gdouble taux_de_change[2];
GtkWidget *option_menu_devise_1;
GtkWidget *option_menu_devise_2;


GtkWidget *clist_devises_parametres;
GtkWidget *bouton_supprimer_devise;
GtkWidget *entree_nom_devise_parametres;
GtkWidget *entree_iso_code_devise_parametres;
GtkWidget *entree_code_devise_parametres;

gint ligne_selection_devise;               /* contient la ligne sélectionnée dans la liste des devises */
struct struct_devise *devise_compte;
struct struct_devise *devise_operation;




/*START_EXTERN*/
extern GtkWidget *bouton_devise_categ_etat;
extern GtkWidget *bouton_devise_ib_etat;
extern GtkWidget *bouton_devise_montant_etat;
extern GtkWidget *bouton_devise_tiers_etat;
extern GtkWidget *detail_devise_compte;
extern GtkWidget *hbox_boutons_modif;
extern GSList *liste_struct_echeances;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern GtkTreeStore *model;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkWidget *onglet_config_etat;
extern GtkTreeSelection * selection;
extern GtkWidget *treeview;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/*END_EXTERN*/




gint sort_tree (GtkTreeModel *model,
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



gboolean select_currency_in_iso_list (GtkTreeSelection *selection,
				      GtkTreeModel *model)
{
    GtkTreeIter iter;
    GValue value1 = {0, };
    GValue value2 = {0, };
    GValue value3 = {0, };

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    gtk_tree_model_get_value (model, &iter, CURRENCY_NAME_COLUMN, &value1);
    gtk_tree_model_get_value (model, &iter, CURRENCY_ISO_CODE_COLUMN, &value2);
    gtk_tree_model_get_value (model, &iter, CURRENCY_NICKNAME_COLUMN, &value3);

    if (g_value_get_string(&value1))
	gtk_entry_set_text ( GTK_ENTRY ( entree_nom ), 
			     g_value_get_string(&value1) );
    else
	gtk_entry_set_text ( GTK_ENTRY ( entree_nom ), 
			     "" );

    if (g_value_get_string(&value2))
	gtk_entry_set_text ( GTK_ENTRY ( entree_iso_code ), 
			     g_value_get_string(&value2) );
    else
	gtk_entry_set_text ( GTK_ENTRY ( entree_iso_code ), 
			     "" );

    if (g_value_get_string(&value3))
	gtk_entry_set_text ( GTK_ENTRY ( entree_code ), 
			     g_value_get_string(&value3) );
    else
	gtk_entry_set_text ( GTK_ENTRY ( entree_code ), 
			     "" );
    return ( FALSE );
} 

/**
 * Update various widgets related to currencies
 */
void update_currency_widgets()
{
    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_DEVISE ))
    {
	gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_DEVISE) ) -> menu );
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_DEVISE) ),
				   creation_option_menu_devises ( -1,
								  liste_struct_devises ));
    }

    gtk_widget_destroy ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ) -> menu );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
			       creation_option_menu_devises ( -1,
							      liste_struct_devises ));

    /* on modifie la liste des devises de l'option menu du detail des comptes */

    gtk_widget_destroy ( GTK_OPTION_MENU ( detail_devise_compte ) -> menu );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_devise_compte ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte  ) ),
				"changed",
				GTK_SIGNAL_FUNC ( modif_detail_compte ),
				GTK_OBJECT ( hbox_boutons_modif ) );
    gtk_option_menu_set_history ( GTK_OPTION_MENU (  detail_devise_compte),
				  g_slist_index ( liste_struct_devises,
						  devise_par_no ( gsb_account_get_currency (gsb_account_get_current_account ()) ))); 


    /* on recrée les boutons de devises dans la conf de l'état */

    if ( onglet_config_etat )
    {
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_montant_etat ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));

	selectionne_devise_categ_etat_courant ();
	selectionne_devise_ib_etat_courant ();
	selectionne_devise_tiers_etat_courant ();
    }

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_echeances_auto_accueil = 1;
}



/* **************************************************************************************************** */
/* Fonction creation_option_menu_devises */
/* retourne le menu d'un option menu avec les différentes devises définies */
/* si devise_cachee = -1, met toutes les devises sous forme de leur sigle */
/* sinon, ne met pas la devise correspondant à devise_cachee ( 0 = aucune ) */
/* liste_tmp est la liste utilisée : soit liste_struct_devises dans le cas général, */
/*                       soit liste_struct_devises dans le cas des paramètres */
/* **************************************************************************************************** */

GtkWidget *creation_option_menu_devises ( gint devise_cachee, GSList *liste_tmp )
{
    GtkWidget *menu;
    GtkWidget *menu_item;


    menu = gtk_menu_new ();

    if ( devise_cachee > 0 )
    {

	/* le 1er item c'est aucune devise */

	menu_item = gtk_menu_item_new_with_label ( _("No associated currency") );
	g_object_set_data ( G_OBJECT ( menu_item ),
			    "adr_devise",
			    devise_nulle );
	g_object_set_data ( G_OBJECT ( menu_item ),
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
		menu_item = gtk_menu_item_new_with_label ( devise_code ( devise ) );
	    else
		menu_item = gtk_menu_item_new_with_label ( g_strconcat ( devise -> nom_devise,
									 " ( ",
									 devise_code ( devise ),
									 " )",
									 NULL ));

	    g_object_set_data ( G_OBJECT ( menu_item ),
				"adr_devise",
				devise );
	    g_object_set_data ( G_OBJECT ( menu_item ),
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



/**
 * TODO: document this
 *
 */
void fill_currency_list ( GtkTreeView * view, gboolean include_obsolete )
{
    GtkTreeModel * model;
    GtkTreeIter iter, child_iter;
    gchar ** continent;
    struct iso_4217_currency * currency = iso_4217_currencies;
    gchar * continents[] = {
	N_("Africa"),
	N_("Asia"),
	N_("Central America"),
	N_("Europe"),
	N_("Northern America"),
	N_("Pacific Ocean"),
	N_("Southern America"),
	NULL,
    };

    model = gtk_tree_view_get_model ( view );

    for (continent = continents; *continent; continent++)
    {
	gtk_tree_store_append (GTK_TREE_STORE(model), &iter, NULL);
	gtk_tree_store_set (GTK_TREE_STORE(model), &iter,
			    COUNTRY_NAME_COLUMN, _(*continent),
			    CURRENCY_NAME_COLUMN, FALSE,
			    CURRENCY_ISO_CODE_COLUMN, FALSE,
			    CURRENCY_NICKNAME_COLUMN, FALSE,
			    CONTINENT_NAME_COLUMN, _(*continent),
			    -1);

	while (currency -> country_name && 
	       /* No need to translate ;-P */
	       !strcmp(currency -> continent, *continent)) 
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
		
		gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, &iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
				    CURRENCY_FLAG_COLUMN, pixbuf,
				    COUNTRY_NAME_COLUMN, g_strconcat ( " ", 
								       _(currency -> country_name),
								       NULL ),
				    CURRENCY_NAME_COLUMN, _(currency -> currency_name),
				    CURRENCY_ISO_CODE_COLUMN, _(currency -> currency_code),
				    CURRENCY_NICKNAME_COLUMN, _(currency -> currency_nickname),
				    CONTINENT_NAME_COLUMN, _(currency -> continent),
				    CURRENCY_HAS_FLAG, TRUE,
				    -1);
	    }
	    currency++;
	}
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
    fill_currency_list ( view, 
			 gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) );
    return FALSE;
}



/**
 * TODO: document this
 *
 */
GtkWidget * new_currency_list ()
{ 
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkWidget *sw, *treeview, *vbox, *checkbox;
    GtkTreeStore *model;
    gint col_offset;

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_ALWAYS);
    /* Create tree store */
    model = gtk_tree_store_new (NUM_CURRENCIES_COLUMNS,
				GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_STRING );

    /* Create tree view */
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), 
		      "changed", 
		      G_CALLBACK (select_currency_in_iso_list),
		      model);

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
    gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE(model), 
					     sort_tree, NULL, NULL);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE(model), COUNTRY_NAME_COLUMN, 
				     sort_tree, NULL, NULL);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), 
					  COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

    /*     mis sur collapse, trouve ça plus pratique à discuter si nécessaire [cedric] */
    /* expand all rows after the treeview widget has been realized */
    /*     g_signal_connect (treeview, "realize", */
    /* 		      G_CALLBACK (gtk_tree_view_expand_all), NULL); */
    g_signal_connect (treeview, "realize",
		      G_CALLBACK (gtk_tree_view_collapse_all), NULL);


    gtk_widget_set_usize ( treeview, FALSE, 200 );
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

    return vbox;
}



/***********************************************************************************************************/
/* Fonction ajout_devise */
/* appelée pour créer une nouvelle devise */
/* soit c'est la clist des paramètres */
/* soit il est NULL, quand il provient de l'import ou nouveau fichier */
/* dans tous les cas, on ne repart pas s'il n'y a aucune devise définie */
/***********************************************************************************************************/

gboolean ajout_devise ( GtkWidget *widget )
{
    GtkWidget *dialog, *label, *table;
    GtkWidget *list, *paddingbox;
    struct struct_devise *devise;
    gchar *nom_devise, *code_devise, *code_iso4217_devise;
    gint resultat;

    dialog = gtk_dialog_new_with_buttons ( _("Add a currency"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL,0,
					   GTK_STOCK_OK,1,
					   NULL );

    gtk_container_set_border_width ( GTK_CONTAINER ( dialog ), 10 );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
			 "destroy",
			 GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
			 "destroy" );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( bloque_echap_choix_devise ),
			 NULL );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   TRUE, _("ISO 4217 currencies"));
    list = new_currency_list ();
    gtk_box_pack_start ( GTK_BOX(paddingbox) , list, TRUE, TRUE, 5 );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   FALSE, _("Currency details"));

    /* Create table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 table,
			 TRUE, TRUE, 0 );

    /* Currency name */
    label = gtk_label_new (COLON(_("Currency name")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 
		       0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    entree_nom = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree_nom ),
				      TRUE );
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

reprise_dialog:
    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case 1 :

	    nom_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom ))));
	    code_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_code ))));
	    code_iso4217_devise = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code ))));

	    if ( strlen ( nom_devise ) && 
		 (strlen ( code_devise ) ||
		  strlen ( code_iso4217_devise )))
	    {
		/* 		recherche si la devise existe déjà, si c'est le cas, on refuse */

		if ( devise_par_nom ( nom_devise )
		     ||
		     devise_par_code_iso ( code_iso4217_devise ))
		{
		    dialogue_error_hint ( _("Currency names or iso 4217 codes should be unique.  Please choose a new name for the currency."),
					  g_strdup_printf ( _("Currency '%s' already exists." ), nom_devise ));
		    goto reprise_dialog;
		}

		devise = create_currency ( nom_devise, code_devise, code_iso4217_devise);

		if ( widget )
		{
		    gchar *ligne[3];
		    gint ligne_liste;

		    ligne[0] = devise -> nom_devise;
		    ligne[1] = devise -> code_iso4217_devise;
		    ligne[2] = devise -> code_devise;

		    ligne_liste = gtk_clist_append ( GTK_CLIST ( widget ),
						     ligne );
		    gtk_clist_set_row_data  ( GTK_CLIST ( widget ),
					      ligne_liste,
					      devise );
		    gtk_clist_select_row ( GTK_CLIST ( widget ),
					   ligne_liste, 0 );
		    update_currency_widgets();
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

	    if ( !liste_struct_devises )
	    {
		gtk_widget_destroy ( GTK_WIDGET ( dialog ));
		return FALSE;
	    }
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
    return TRUE;
}
/***********************************************************************************************************/



/***********************************************************************************************************/
gint bloque_echap_choix_devise ( GtkWidget *dialog,
				 GdkEventKey *key,
				 gpointer null )
{

    /* empÃšche la touche echap de fermer la fenetre */

    if ( key -> keyval == GDK_Escape )
    {
	gtk_signal_emit_stop_by_name ( GTK_OBJECT ( dialog ),
				       "key-press-event" );
	return ( TRUE );
    }

    return ( FALSE );

}
/***********************************************************************************************************/






/***********************************************************************************************************/
/* Fonction retrait_devise */
/***********************************************************************************************************/

void retrait_devise ( GtkWidget *bouton,
		      GtkWidget *liste )
{
    gint devise_trouvee;
    struct struct_devise *devise;
    GSList *list_tmp;


    if ( ligne_selection_devise == -1 )
	return;


    devise = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				      ligne_selection_devise );

    /*   recherche dans tous les comptes, les opÃ©s et les Ã©chÃ©ances si la devise n'est pas utilisÃ©e */
    /* si elle l'est, empÃªche sa suppression */

    devise_trouvee = 0;

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( gsb_account_get_currency (i) == devise -> no_devise )
	{
	    devise_trouvee = 1;
	    i = gsb_account_get_accounts_amount ();
	}
	else
	{
	    GSList *liste_tmp;

	    liste_tmp = gsb_account_get_transactions_list (i);

	    while ( liste_tmp )
	    {
		if ( gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (liste_tmp -> data)) == devise -> no_devise )
		{
		    devise_trouvee = 1;
		    i = gsb_account_get_accounts_amount ();
		    liste_tmp = NULL;
		}
		else
		    liste_tmp = liste_tmp -> next;
	    }

	    if ( !devise )
	    {
		liste_tmp = liste_struct_echeances;

		while ( liste_tmp )
		{
		    if ( ((struct operation_echeance *)(liste_tmp -> data )) -> devise == devise -> no_devise )
		    {
			devise_trouvee = 1;
			i = gsb_account_get_accounts_amount ();
			liste_tmp = NULL;
		    }
		    else
			liste_tmp = liste_tmp -> next;
		}
	    }
	}

	list_tmp = list_tmp -> next;
    }

    if ( devise_trouvee )
    {
	dialogue_error_hint ( g_strdup_printf ( _("Currency '%s' is used in current account.  Grisbi can't delete it."),
						devise -> nom_devise ),
			      g_strdup_printf ( _("Impossible to remove currency '%s'"), 
						devise -> nom_devise) );
	return;
    }


    gtk_clist_remove ( GTK_CLIST ( liste ), ligne_selection_devise );

    liste_struct_devises = g_slist_remove ( liste_struct_devises, devise );
    nb_devises--;

}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Fonction demande_taux_de_change : */
/* affiche une fenetre permettant d'entrer le taux de change entre la devise du compte et la devise demandÃ©e */
/* renvoie ce taux de change */
/* le taux renvoyÃ© est <0 si une_devise_compte_egale_x_devise_ope = 1, > 0 sinon */
/***********************************************************************************************************/

void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise ,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change,
			      gboolean force )
{
    GtkWidget *dialog, *label, *entree, *menu, *item, *hbox, *entree_frais, *paddingbox;
    struct cached_exchange_rate * cache;
    gint resultat;

    if ( !force && 
	 (cache = cached_exchange_rate ( devise_compte, devise )) )
    {
	taux_de_change[0] = cache -> rate;
	taux_de_change[1] = cache -> fees;
	return;
    }

    dialog = gtk_dialog_new_with_buttons ( _("Enter exchange rate"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK, 0,
					   NULL );
    gtk_signal_connect ( GTK_OBJECT ( dialog), "delete_event",
			 GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
			 NULL );

    paddingbox = new_paddingbox_with_title ( GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ), FALSE, 
					     g_strdup_printf( _("Please enter exchange rate for %s"), 
							      devise->nom_devise) );
    gtk_container_set_border_width ( GTK_CONTAINER ( paddingbox ), 6 );
    gtk_box_set_spacing ( GTK_BOX ( GTK_DIALOG (dialog)->vbox ), 6 );
    

    /* crÃ©ation de la ligne du change */

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( POSTSPACIFY(_("A")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    option_menu_devise_1= gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), option_menu_devise_1,
			 TRUE, TRUE, 0);

    label = gtk_label_new ( SPACIFY(_("equals")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    entree = gtk_entry_new ();
    gtk_widget_set_usize ( entree, 100, FALSE );
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree ),
				      TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), entree,
			 TRUE, TRUE, 0);

    option_menu_devise_2 = gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), option_menu_devise_2,
			 TRUE, TRUE, 0);

    /* crÃ©ation du menu de la 1Ãšre devise ( le menu comporte la devise
       courante et celle associÃ©e ) */
    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label ( devise -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise_compte );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_1 ), menu );
    g_signal_connect ( G_OBJECT ( option_menu_devise_1 ),
		       "changed",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       NULL );
    g_object_set_data ( G_OBJECT (option_menu_devise_1), "associate",
			option_menu_devise_2 );
    g_object_set_data ( G_OBJECT (option_menu_devise_1), "currency", devise );

    /* crÃ©ation du menu de la 2Ãšme devise ( le menu comporte la devise
       courante et celle associÃ©e ) */
    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label ( devise -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    item = gtk_menu_item_new_with_label ( devise_compte -> nom_devise );
    g_object_set_data ( G_OBJECT ( item ), "adr_devise", devise_compte );
    gtk_menu_append ( GTK_MENU ( menu ), item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_devise_2 ), menu );
    g_signal_connect ( G_OBJECT ( option_menu_devise_2 ),
		       "changed",
		       GTK_SIGNAL_FUNC ( devise_selectionnee ),
		       GINT_TO_POINTER ( 1 ));
    g_object_set_data ( G_OBJECT (option_menu_devise_2), "associate",
			option_menu_devise_1 );
    g_object_set_data ( G_OBJECT (option_menu_devise_2), "currency", devise );

    /* crÃ©ation de la ligne des frais de change */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 5 );

    label = gtk_label_new ( COLON(_("Exchange fees")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 5 );

    entree_frais = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree_frais ),
				      TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), entree_frais,
			 TRUE, TRUE, 5 );

    label = gtk_label_new ( devise_compte -> nom_devise );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 5 );

    gtk_widget_show_all ( dialog );

    /* choix des 1Ãšre et 2Ãšme devise */
    if ( taux_change || frais_change )
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
			     g_strdup_printf ( "%f", taux_change ));
	gtk_entry_set_text ( GTK_ENTRY ( entree_frais ),
			     g_strdup_printf ( "%4.2f", fabs ( frais_change )));

    }
    else
    {
	/* vÃ©rifie s'il y a dÃ©jÃ  une association entre la devise du
	   compte et la devise de l'opÃ©ration */
	if ( devise_compte -> no_devise_en_rapport == devise -> no_devise )
	{
	    /* il y a une association de la devise du compte vers la
	       devise de l'opÃ©ration */
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					  devise_compte -> une_devise_1_egale_x_devise_2 );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					  !( devise_compte -> une_devise_1_egale_x_devise_2 ));

	    /* si un change est dÃ©jÃ  entrÃ©, on l'affiche */
	    if ( devise_compte -> date_dernier_change )
		gtk_entry_set_text ( GTK_ENTRY ( entree ),
				     g_strdup_printf ( "%f",
						       devise_compte -> change ));
	}
	else
	    if ( devise -> no_devise_en_rapport == devise_compte -> no_devise )
	    {
		/* il y a une association de la devise de l'opÃ©ration
		   vers la devise du compte */
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					      !(devise -> une_devise_1_egale_x_devise_2 ));
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					      devise -> une_devise_1_egale_x_devise_2 );
		/* si un change est dÃ©jÃ  entrÃ©, on l'affiche */
		if ( devise -> date_dernier_change )
		    gtk_entry_set_text ( GTK_ENTRY ( entree ),
					 g_strdup_printf ( "%f",
							   devise -> change ));
	    }
	    else
	    {
		/* il n'y a aucun rapport Ã©tabli entre les 2 devises */
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_1 ),
					      1 );
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_devise_2 ),
					      0 );
	    }
    }

    /* on lance la fenetre */
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( !resultat )
    {
	struct struct_devise *devise_tmp;

	taux_de_change[0] = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )),
					NULL );
	taux_de_change[1] = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_frais )),
					NULL );

	/* FIXME: use gtk_menu_get_active + gtk_option_menu_get_menu ? */
	devise_tmp = g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( option_menu_devise_1 ) -> menu_item ),
					 "adr_devise" );


	if ( devise_tmp -> no_devise != devise -> no_devise )
	    taux_de_change[0] = -taux_de_change[0];

	update_exchange_rate_cache ( devise_compte, devise, 
				     taux_de_change[0], taux_de_change[1] );

    }
    else
    {
	taux_de_change[0] = 0;
	taux_de_change[1] = 0;
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
}
/***********************************************************************************************************/





/* 
 * Handler that change the second option menu of a window that ask for
 * change.
 *
 * \param menu_shell The menu that triggered this handler.
 * \param origine Position of selected item in menu
 */
gboolean devise_selectionnee ( GtkWidget *menu_shell, gint origine )
{
    GtkWidget * associate;
    gint position;
    struct struct_devise *devise;

    devise = g_object_get_data( G_OBJECT(menu_shell), "currency");

    if ( devise )
    {
	position = gtk_option_menu_get_history ( GTK_OPTION_MENU(menu_shell) );
	devise -> une_devise_1_egale_x_devise_2 = (position != origine);
	associate = g_object_get_data ( G_OBJECT(menu_shell), "associate" );
	g_signal_handlers_block_by_func ( G_OBJECT(associate),
					  G_CALLBACK (devise_selectionnee), 
					  (gpointer) !origine );
	if (associate)
	    gtk_option_menu_set_history ( GTK_OPTION_MENU(associate), 1 - position );
	g_signal_handlers_unblock_by_func ( G_OBJECT(associate),
					    G_CALLBACK (devise_selectionnee), 
					    (gpointer) !origine );
    }

    return FALSE;
}



/**
 * Creates the currency list and associated form to configure them.
 *
 * \returns A newly created vbox
 */
GtkWidget *onglet_devises ( void )
{
    GtkWidget *vbox_pref, *label, *paddingbox;
    GtkWidget *scrolled_window, *vbox, *table;
    GSList *liste_tmp;
    gchar *titres_devise [3] = { _("Currency"),
	_("ISO Code"),
	_("Sign") };
	GtkWidget *bouton;
	GtkWidget *hbox;

	vbox_pref = new_vbox_with_title_and_icon ( _("Currencies"),
						   "currencies.png" );

	paddingbox = new_paddingbox_with_title (vbox_pref, TRUE,
						_("Known currencies"));

	hbox = gtk_hbox_new ( FALSE, 5 );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			     TRUE, TRUE, 0);

	/* Currency list */
	scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

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
	gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			    clist_devises_parametres );
	gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			     TRUE, TRUE, 0);

	/*   s'il n'y a pas de fichier ouvert, on grise */
	if ( !gsb_account_get_accounts_amount () )
	    gtk_widget_set_sensitive ( vbox_pref, FALSE );
	else
	{
	    /* remplissage de la liste avec les devises temporaires */

	    liste_tmp = liste_struct_devises;

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

		/* on associe Ã  la ligne la struct de la devise */

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

	/* Button "Add" */
	bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
	gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				    "clicked",
				    GTK_SIGNAL_FUNC  ( ajout_devise ),
				    GTK_OBJECT (clist_devises_parametres ));
	gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
			     FALSE, FALSE, 5 );

	/* Button "Remove" */
	bouton_supprimer_devise = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_devise ),
			     "clicked",
			     GTK_SIGNAL_FUNC  ( retrait_devise ),
			     clist_devises_parametres );
	gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_devise,
			     FALSE, FALSE, 5 );


	/* Input form for currencies */
	paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
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


	/* Create table */
	table = gtk_table_new ( 2, 2, FALSE );
	gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
	gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			     TRUE, TRUE, 0 );

	/* Create currency name entry */
	label = gtk_label_new (COLON(_("Name")));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
	gtk_table_attach ( GTK_TABLE ( table ),
			   label, 0, 1, 0, 1,
			   GTK_SHRINK | GTK_FILL, 0,
			   0, 0 );
	entree_nom_devise_parametres = new_text_entry ( NULL, (GCallback) changement_nom_entree_devise );
	gtk_table_attach ( GTK_TABLE ( table ),
			   entree_nom_devise_parametres, 
			   1, 2, 0, 1, 
			   GTK_EXPAND | GTK_FILL, 0,
			   0, 0 );

	/* Create code entry */
	label = gtk_label_new (COLON(_("Sign")));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
	gtk_table_attach ( GTK_TABLE ( table ),
			   label, 
			   0, 1, 1, 2,
			   GTK_SHRINK | GTK_FILL, 0,
			   0, 0 );
	entree_code_devise_parametres = new_text_entry ( NULL, (GCallback) changement_code_entree_devise );
	gtk_table_attach ( GTK_TABLE ( table ),
			   entree_code_devise_parametres, 
			   1, 2, 1, 2,
			   GTK_EXPAND | GTK_FILL, 0,
			   0, 0 );

	/* Create code entry */
	label = gtk_label_new ( COLON(_("ISO code")) );
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
	gtk_table_attach ( GTK_TABLE ( table ),
			   label,
			   0, 1, 2, 3,
			   GTK_SHRINK | GTK_FILL, 0,
			   0, 0 );
	entree_iso_code_devise_parametres = new_text_entry ( NULL, (GCallback) changement_iso_code_entree_devise );
	gtk_table_attach ( GTK_TABLE ( table ),
			   entree_iso_code_devise_parametres, 
			   1, 2, 2, 3,
			   GTK_EXPAND | GTK_FILL, 0,
			   0, 0 );

	/* Select first entry if applicable */
	gtk_clist_select_row ( GTK_CLIST(clist_devises_parametres), 0, 0 ); 

	return ( vbox_pref );

}
/* ************************************************************************************************************** */



/**
 * Creates a new GtkOptionMenu with a pointer to an integerthat will
 * be modified according to the entry's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler
 *
 * \return A newly allocated option menu.
 */
GtkWidget * new_currency_option_menu ( gint * value, GCallback hook )
{
    GtkWidget * currency_list, *currency_menu;

    currency_list = gtk_option_menu_new ();
    currency_menu = creation_option_menu_devises ( 0, liste_struct_devises );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( currency_list ), currency_menu );
    if (value && *value)
	gtk_option_menu_set_history ( GTK_OPTION_MENU(currency_list), *value - 1 );

    g_signal_connect ( GTK_OBJECT (currency_list), "changed", (GCallback) set_int_from_menu, value );
    g_signal_connect ( GTK_OBJECT (currency_list), "changed", (GCallback) hook, value );
    g_object_set_data ( G_OBJECT ( currency_list ), "pointer", value);

    return currency_list;
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

    label = gtk_label_new (_("Currency for third parties tree"));
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



/* **************************************************************************************************************************** */
/* Fonction selection_ligne_devise */
/* appelÃ©e lorsqu'on sÃ©lectionne une devise dans la liste */
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
    entry_set_value(entree_nom_devise_parametres, &(devise->nom_devise));
    entry_set_value(entree_code_devise_parametres, &(devise->code_devise));
    entry_set_value(entree_iso_code_devise_parametres, &(devise->code_iso4217_devise));

    /*     nécessaire d'interdire le changement du nom de l'euro car tous les tests se font sur son nom */

    if ( strcmp ( devise->nom_devise, "Euro" ))
	gtk_widget_set_sensitive ( entree_nom_devise_parametres,
				   TRUE );
    else
	gtk_widget_set_sensitive ( entree_nom_devise_parametres,
				   FALSE );

    return FALSE;
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
/* Fonction deselection_ligne_devise */
/* appelÃ©e lorsqu'on dÃ©sÃÂ©lectionne une devise dans la liste */
/* **************************************************************************************************************************** */

gboolean deselection_ligne_devise ( GtkWidget *liste,
				    gint ligne,
				    gint colonne,
				    GdkEventButton *ev,
				    GtkWidget *frame )
{
    ligne_selection_devise = -1;

    /* retire le nom et le code de la devise */
    entry_set_value(entree_nom_devise_parametres, NULL);
    entry_set_value(entree_code_devise_parametres, NULL);
    entry_set_value(entree_iso_code_devise_parametres, NULL);

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
			 2,
			 devise -> code_devise );

    return FALSE;
}
/* **************************************************************************************************************************** */

/* **************************************************************************************************************************** */
gboolean changement_iso_code_entree_devise ( void )
{
    struct struct_devise *devise;

    devise = gtk_clist_get_row_data ( GTK_CLIST ( clist_devises_parametres ),
				      ligne_selection_devise );

    devise -> code_iso4217_devise = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_iso_code_devise_parametres ))));

    gtk_clist_set_text ( GTK_CLIST ( clist_devises_parametres ),
			 ligne_selection_devise,
			 1,
			 devise -> code_iso4217_devise );

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
struct cached_exchange_rate *cached_exchange_rate ( struct struct_devise * currency1, 
						    struct struct_devise * currency2 )
{
    GSList * liste_tmp = cached_exchange_rates;
    struct cached_exchange_rate * tmp;

    while ( liste_tmp )
    {
	tmp = liste_tmp -> data;
	if ( currency1 == tmp -> currency1 && currency2 == tmp -> currency2 )
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
void update_exchange_rate_cache ( struct struct_devise * currency1, 
				  struct struct_devise * currency2,
				  gdouble change, gdouble fees )
{
    struct cached_exchange_rate * tmp;

    tmp = (struct cached_exchange_rate *) malloc(sizeof(struct cached_exchange_rate));

    tmp -> currency1 = currency1;
    tmp -> currency2 = currency2;
    tmp -> rate = change;
    tmp -> fees = fees;

    cached_exchange_rates = g_slist_append ( cached_exchange_rates, tmp );
}
/* ***************************************************************************************** */






/* ***************************************************************************************** */
struct struct_devise *create_currency ( gchar * nom_devise, gchar * code_devise, gchar * code_iso4217_devise )
{
  struct struct_devise * devise;
  
  devise = malloc ( sizeof ( struct struct_devise ));
  devise -> nom_devise = nom_devise;
  devise -> code_devise = code_devise;
  devise -> code_iso4217_devise = code_iso4217_devise;
  devise -> passage_euro = 0;
  devise -> no_devise_en_rapport = 0;
  devise -> date_dernier_change = NULL;
  devise -> une_devise_1_egale_x_devise_2 = 0;

  devise -> change = 0;


  /* 	  si le widget n'est pas nul, c'est une clist, c'est que l'appel vient du menu de configuration, */
  /* on met la liste à  jour et on ajoute la devise à  liste_struct_devises */

  devise -> no_devise = ++no_derniere_devise;
  liste_struct_devises = g_slist_append ( liste_struct_devises, devise );
  nb_devises++;

  return devise;
}


struct struct_devise * find_currency_from_iso4217_list ( gchar * currency_name )
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

  return NULL;
}
/* ***************************************************************************************** */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
