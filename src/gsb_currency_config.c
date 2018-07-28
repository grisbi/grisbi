/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2006-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_currency_config.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "parametres.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_locale.h"
#include "traitement_variables.h"
#include "utils.h"
#include "tiers_onglet.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "utils_str.h"
#include "structures.h"
#include "utils_files.h"
#include "erreur.h"
#include "gsb_dirs.h"
#include "utils_prefs.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *delete_devise_button;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_GLOBAL*/
GtkWidget *combo_devise_totaux_tiers;
GtkWidget *combo_devise_totaux_ib;
GtkWidget *combo_devise_totaux_categ;
/*END_GLOBAL*/

/* struct Iso4217Currency; */
struct Iso4217Currency iso_4217_currencies[] =
{
    { N_("Africa"), N_("Algerian Dinar"), N_("Algeria"), "DZD", NULL, TRUE, "DZD.png", 3, 1 },
    { N_("Africa"), N_("Botswana Pula"), N_("Botswana"), "BWP", NULL, TRUE, "BWP.png", 2, 1 },
    { N_("Africa"), N_("Burundi Franc"), N_("Burundi"), "BIF", NULL, TRUE, "BIF.png", 2, 1 },
    { N_("Africa"), N_("CFA Franc BCEAO"), N_("Niger"), "XOF", NULL, TRUE, "NIG.png", 2, 1 },
    { N_("Africa"), N_("CFA Franc BCEAO"), N_("Senegal"), "XOF", NULL, TRUE, "SEN.png", 2, 1 },
	{ N_("Africa"), N_("CFA Franc BCEAO"), N_("Togo"), "XOF", NULL, TRUE, "TOG.png", 2, 1 },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Cameroon"), "XAF", NULL, TRUE, "CAM.png", 2, 1 },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Chad"), "XAF", NULL, TRUE, "CHA.png", 2, 1 },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Congo"), "XAF", NULL, TRUE, "CO_.png", 2, 1 },
    { N_("Africa"), N_("Comoro Franc"), N_("Comoros"), "KMF", NULL, TRUE, "KMF.png", 2, 1 },
    { N_("Africa"), N_("Egyptian Pound"), N_("Egypt"), "EGP", "£", TRUE, "EGP.png", 3, 1 },
    { N_("Africa"), N_("Ethiopian Birr"), N_("Ethiopia"), "ETB", NULL, TRUE, "ETB.png", 0, 1 },
    { N_("Africa"), N_("Gambian Dalasi"), N_("Gambia"), "GMD", NULL, TRUE, "GMD.png", 2, 1 },
    { N_("Africa"), N_("Ghana Cedi"), N_("Ghana"), "GHC", NULL, TRUE, "GHC.png", 2, 1 },
    { N_("Africa"), N_("Guinea-Bissau Peso"), N_("Guinea-Bissau"), "GWP", NULL, TRUE, "GWP.png", 2, 1 },
    { N_("Africa"), N_("Kenyan Shilling"), N_("Kenya"), "KES", NULL, TRUE, "KES.png", 2, 1 },
    { N_("Africa"), N_("Liberian Dollar"), N_("Liberia"), "LRD", "$", TRUE, "LRD.png", 2, 1 },
    { N_("Africa"), N_("Libyan Dinar"), N_("Libyan Arab Jamahiriya"), "LYD", NULL, TRUE, "LYD.png", 3, 1 },
    { N_("Africa"), N_("Malagasy Franc"), N_("Madagascar"), "MGF", NULL, TRUE, "MGF.png", 3, 1 },
    { N_("Africa"), N_("Malawi Kwacha"), N_("Malawi"), "MWK", NULL, TRUE, "MWK.png", 2, 1 },
    { N_("Africa"), N_("Mauritania Ouguiya"), N_("Mauritania"), "MRO", NULL, TRUE, "MRO.png", 2, 1 },
    { N_("Africa"), N_("Moazambique Metical"), N_("Mozambique"), "MZM", NULL, TRUE, "MZM.png", 2, 1 },
    { N_("Africa"), N_("Moroccan Dirham"), N_("Morocco"), "MAD", NULL, TRUE, "MAD.png", 2, 1 },
    { N_("Africa"), N_("Nigerian Naira"), N_("Nigeria"), "NGN", "₦", TRUE, "NGN.png", 2, 1 },
    { N_("Africa"), N_("Rwanda Franc"), N_("Rwanda"), "RWF", NULL, TRUE, "RWF.png", 2, 1 },
    { N_("Africa"), N_("Sao Tome and Principe Dobra"), N_("Sao Tome and Principe"), "STD", NULL, TRUE, "STD.png", 2, 1 },
    { N_("Africa"), N_("Seychelles Rupee"), N_("Seychelles"), "SCR", "₨", TRUE, "SCR.png", 2, 1 },
    { N_("Africa"), N_("Sierra Leonean Leone"), N_("Sierra Leone"), "SLL", NULL, TRUE, "SLL.png", 2, 1 },
    { N_("Africa"), N_("Somali Shilling"), N_("Somalia"), "SOS", NULL, TRUE, "SOS.png", 2, 1 },
    { N_("Africa"), N_("South African Rand"), N_("Lesotho"), "ZAR", "R", TRUE, "LSL.png", 2, 1 },
    { N_("Africa"), N_("South African Rand"), N_("Namibia"), "ZAR", "R", TRUE, "NAD.png", 2, 1 },
    { N_("Africa"), N_("South African Rand"), N_("South Africa"), "ZAR", "R", TRUE, "ZAR.png", 2, 1 },
    { N_("Africa"), N_("Swaziland Lilangeni"), N_("Swaziland"), "SZL", NULL, TRUE, "SZL.png", 2, 1 },
    { N_("Africa"), N_("Tanzanian Shilling"), N_("United Republic of Tanzania"), "TZS", NULL, TRUE, "TZS.png", 2, 1 },
    { N_("Africa"), N_("Tunisian Dinar"), N_("Tunisia"), "TND", NULL, TRUE, "TND.png", 3, 1 },
    { N_("Africa"), N_("Zambian Kwacha"), N_("Zambia"), "ZMK", NULL, TRUE, "ZMK.png", 2, 1 },
    { N_("Africa"), N_("Zimbabwe Dollar"), N_("Zimbabwe"), "ZWD", "$", TRUE, "ZWD.png", 2, 1 },
    { N_("Asia"), N_("Afghani"), N_("Afghanistan"), "AFA", NULL, TRUE, "AFN.png", 2, 1 },
    { N_("Asia"), N_("Bahraini Dinar"), N_("Bahrain"), "BHD", NULL, TRUE, "BHD.png", 3, 1 },
    { N_("Asia"), N_("Bangladesh Taka"), N_("Bangladesh"), "BDT", NULL, TRUE, "BDT.png", 2, 1 },
    { N_("Asia"), N_("Brunei Dollar"), N_("Brunei Darussalam"), "BND", "$", TRUE, "BND.png", 2, 1 },
    { N_("Asia"), N_("Cambodian Riel"), N_("Cambodia"), "KHR", "៛", TRUE, "KHR.png", 2, 1 },
    { N_("Asia"), N_("Cyprus Pound"), N_("Cyprus"), "CYP", "£", FALSE, "CYP.png", 2, 1 },
    { N_("Asia"), N_("Hong Kong Dollar"), N_("Hong Kong"), "HKD", "$", TRUE, "HKD.png", 2, 1 },
    { N_("Asia"), N_("Indian Rupee"), N_("Bhutan"), "INR", "₨", TRUE, "BHU.png", 2, 1 },
    { N_("Asia"), N_("Indian Rupee"), N_("India"), "INR", "₨", TRUE, "INR.png", 2, 1 },
    { N_("Asia"), N_("Indonesian Rupiah"), N_("Indonesia"), "IDR", NULL, TRUE, "IDR.png", 2, 1 },
    { N_("Asia"), N_("Iranian Rial"), N_("Iran"), "IRR", "﷼", TRUE, "IRR.png", 2, 1 },
    { N_("Asia"), N_("Iraqi Dinar"), N_("Iraq"), "IQD", NULL, TRUE, "IQD.png", 3, 1 },
    { N_("Asia"), N_("Japanese Yen"), N_("Japan"), "JPY", "¥", TRUE, "JPY.png", 0, 1 },
    { N_("Asia"), N_("Jordanian Dinar"), N_("Jordan"), "JOD", NULL, TRUE, "JOD.png", 3, 1 },
    { N_("Asia"), N_("Kazakhstan Tenge"), N_("Kazakhstan"), "KZT", NULL, TRUE, "KZT.png", 2, 1 },
    { N_("Asia"), N_("Kuwaiti Dinar"), N_("Kuwait"), "KWD", NULL, TRUE, "KWD.png", 3, 1 },
    { N_("Asia"), N_("Lao Kip"), N_("Lao People's Democratic Republic"), "LAK", "₭", TRUE, "LAK.png", 2, 1 },
    { N_("Asia"), N_("Lebanese Pound"), N_("Lebanon"), "LBP", "£", TRUE, "LBP.png", 3, 1 },
    { N_("Asia"), N_("Macau Pataca"), N_("Macao"), "MOP", NULL, TRUE, "MOP.png", 2, 1 },
    { N_("Asia"), N_("Malaysian Ringgit"), N_("Malaysia"), "MYR", NULL, TRUE, "MYR.png", 2, 1 },
    { N_("Asia"), N_("Mongolian Tugrik"), N_("Mongolia"), "MNT", "₮", TRUE, "MNT.png", 2, 1 },
    { N_("Asia"), N_("Nepalese Rupee"), N_("Nepal"), "NPR", "₨", TRUE, "NPR.png", 2, 1 },
    { N_("Asia"), N_("New Israeli Shekel"), N_("Israel"), "ILS", "₪", TRUE, "ILS.png", 2, 1 },
    { N_("Asia"), N_("New Taiwan Dollar"), N_("Taiwan, Province of China"), "TWD", "元", TRUE, "TWD.png", 2, 1 },
    { N_("Asia"), N_("North Korean Won"), N_("Democratic People's Republic of Korea"), "KPW", "₩", TRUE, "KPW.png", 2, 1 },
    { N_("Asia"), N_("Pakistan Rupee"), N_("Pakistan"), "PKR", "₨", TRUE, "PKR.png", 2, 1 },
    { N_("Asia"), N_("Philippine peso"), N_("Philippines"), "PHP", "₱", TRUE, "PHP.png", 2, 1 },
    { N_("Asia"), N_("Qatari Rial"), N_("Qatar"), "QAR", "﷼", TRUE, "QAR.png", 3, 1 },
    { N_("Asia"), N_("Rial Omani"), N_("Oman"), "OMR", "﷼", TRUE, "OMR.png", 3, 1 },
    { N_("Asia"), N_("Russian Ruble"), N_("Russia"), "RUR", "руб", TRUE, "RUR.png", 2, 1 },
    { N_("Asia"), N_("Saudi Riyal"), N_("Saudi Arabia"), "SAR", "﷼", TRUE, "SAR.png", 2, 1 },
    { N_("Asia"), N_("Singapore Dollar"), N_("Singapore"), "SGD", "$", TRUE, "SGD.png", 2, 1 },
    { N_("Asia"), N_("South Korean Won"), N_("Republic of Korea"), "KRW", "₩", TRUE, "KRW.png", 2, 1 },
    { N_("Asia"), N_("Sri Lanka Rupee"), N_("Sri Lanka"), "LKR", "௹", TRUE, "LKR.png", 2, 1 },
    { N_("Asia"), N_("Syrian Pound"), N_("Syrian Arab Republic"), "SYP", "£", TRUE, "SYP.png", 3, 1 },
    { N_("Asia"), N_("Thai Baht"), N_("Thailand"), "THB", "฿", TRUE, "THB.png", 2, 1 },
    { N_("Asia"), N_("Turkish Lira"), N_("Turkey"), "TRL", "₤", TRUE, "TRL.png", 2, 1 },
    { N_("Asia"), N_("United Arab Emirates Dirham"), N_("United Arab Emirates"), "AED", NULL, TRUE, "AED.png", 3, 1 },
    { N_("Asia"), N_("Viet Nam Dong"), N_("Viet Nam"), "VND", "₫", TRUE, "VND.png", 2, 1 },
    { N_("Asia"), N_("Yemeni Rial"), N_("Yemen"), "YER", "﷼", TRUE, "YER.png", 3, 1 },
    { N_("Asia"), N_("Yuan Renminbi"), N_("China"), "CNY", "元", TRUE, "CNY.png", 2, 1 },
    { N_("Central America"), N_("Belize Dollar"), N_("Belize"), "BZD", "$", TRUE, "BZD.png", 2, 1 },
    { N_("Central America"), N_("Costa Rican Colon"), N_("Costa Rica"), "CRC", "₡", TRUE, "CRC.png", 2, 1 },
    { N_("Central America"), N_("Guatemalan Quetzal"), N_("Guatemala"), "GTQ", NULL, TRUE, "GTQ.png", 2, 1 },
    { N_("Central America"), N_("Honduran Lempira"), N_("Honduras"), "HNL", NULL, TRUE, "HNL.png", 2, 1 },
    { N_("Central America"), N_("Mexican Peso"), N_("Mexico"), "MXP", "$", TRUE, "MXP.png", 2, 1 },
    { N_("Central America"), N_("Panama Balboa"), N_("Panama"), "PAB", NULL, TRUE, "PAB.png", 2, 1 },
    { N_("Europe"), N_("Euro"), N_("Germany"), "EUR", "€", TRUE, "DEM.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Austria"), "EUR", "€", TRUE, "ATS.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Belgium"), "EUR", "€", TRUE, "BEF.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Cyprus"), "EUR", "€", TRUE, "CYP.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Spain"), "EUR", "€", TRUE, "ESP.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Finland"), "EUR", "€", TRUE, "FIM.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("France"), "EUR", "€", TRUE, "FRF.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Greece"), "EUR", "€", TRUE, "GRD.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Ireland"), "EUR", "€", TRUE, "IEP.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Italy"), "EUR", "€", TRUE, "VAT.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Luxembourg"), "EUR", "€", TRUE, "LUF.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Netherlands"), "EUR", "€", TRUE, "NLG.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Portugal"), "EUR", "€", TRUE, "PTE.png", 2, 0 },
    { N_("Europe"), N_("Euro"), N_("Slovenia"), "EUR", "€", TRUE, "SIT.png", 2, 0 },
    { N_("Europe"), N_("Albanian Lek"), N_("Albania"), "ALL", NULL, TRUE, "ALL.png", 3, 1 },
    { N_("Europe"), N_("Austrian Schilling"), N_("Austria"), "ATS", NULL, FALSE, "ATS.png", 2, 1 },
    { N_("Europe"), N_("Belgian Franc"), N_("Belgium"), "BEF", NULL, FALSE, "BEF.png", 2, 1 },
    { N_("Europe"), N_("Bulgarian Lev"), N_("Bulgaria"), "BGL", NULL, FALSE, "BGN.png", 2, 1 },
    { N_("Europe"), N_("Croatian kuna"), N_("Croatia"), "HRK", "kn", TRUE, "HRK.png", 2, 1 },
    { N_("Europe"), N_("Czech Koruna"), N_("Czech Republic"), "CZK", NULL, TRUE, "CZK.png", 2, 1 },
    { N_("Europe"), N_("Danish Krone"), N_("Denmark"), "DKK", NULL, TRUE, "DKK.png", 2, 1 },
    { N_("Europe"), N_("Deutsche Mark"), N_("Germany"), "DEM", NULL, FALSE, "DEM.png", 2, 1 },
    { N_("Europe"), N_("Estonian Kroon"), N_("Estonia"), "EEK", NULL, TRUE, "EEK.png", 2, 1 },
    { N_("Europe"), N_("Euro"), N_("UE (Europe)"), "EUR", "€", TRUE, "EUR.png", 2, 1 },
    { N_("Europe"), N_("Finnish Markka"), N_("Finland"), "FIM", NULL, FALSE, "FIM.png", 2, 1 },
    { N_("Europe"), N_("French Franc"), N_("France"), "FRF", "₣", FALSE, "FRF.png", 2, 1 },
    { N_("Europe"), N_("Gibraltar Pound"), N_("Gibraltar"), "GIP", "£", TRUE, "GIP.png", 2, 1 },
    { N_("Europe"), N_("Greek Drachma"), N_("Greece"), "GRD", "₯", FALSE, "GRD.png", 2, 1 },
    { N_("Europe"), N_("Hungarian Forint"), N_("Hungary"), "HUF", NULL, TRUE, "HUF.png", 2, 1 },
    { N_("Europe"), N_("Iceland Krona"), N_("Iceland"), "ISK", NULL, TRUE, "ISK.png", 0, 1 },
    { N_("Europe"), N_("Irish Pound"), N_("Ireland"), "IEP", "£", FALSE, "IEP.png", 2, 1 },
    { N_("Europe"), N_("Italian Lira"), N_("Holy See"), "ITL", "₤", FALSE, "VAT.png", 2, 1 },
    { N_("Europe"), N_("Italian Lira"), N_("Italy"), "ITL", "₤", FALSE, "ITL.png", 2, 1 },
    { N_("Europe"), N_("Italian Lira"), N_("San Marino"), "ITL", "₤", FALSE, "SAN.png", 2, 1 },
    { N_("Europe"), N_("Latvian Lats"), N_("Latvia"), "LVL", NULL, TRUE, "LVL.png", 2, 1 },
    { N_("Europe"), N_("Lithuanian Litas"), N_("Lietuva"), "LTL", NULL, TRUE, "LTL.png", 2, 1 },
    { N_("Europe"), N_("Luxembourg Franc"), N_("Luxembourg"), "LUF", "₣", FALSE, "LUF.png", 2, 1 },
    { N_("Europe"), N_("Moldovenesc Leu"), N_("Moldova"), "MDL", NULL, TRUE, "MDL.png", 2, 1 },
    { N_("Europe"), N_("Netherlands Guilder"), N_("Netherlands"), "NLG", "ƒ", FALSE, "NLG.png", 2, 1 },
    { N_("Europe"), N_("New Yugoslavian Dinar"), N_("Serbia and Montenegro"), "YUD", NULL, FALSE, "YUV.png", 2, 1 },
    { N_("Europe"), N_("Norwegian Krone"), N_("Norway"), "NOK", NULL, TRUE, "NOK.png", 2, 1 },
    { N_("Europe"), N_("Polish Zloty"), N_("Poland"), "PLN", NULL, TRUE, "PLN.png", 2, 1 },
    { N_("Europe"), N_("Portuguese Escudo"), N_("Portugal"), "PTE", NULL, FALSE, "PTE.png", 2, 1 },
    { N_("Europe"), N_("Pound Sterling"), N_("United Kingdom"), "GBP", "£", TRUE, "GBP.png", 2, 1 },
    { N_("Europe"), N_("Romanian Leu"), N_("Romania"), "ROL", NULL, TRUE, "ROL.png", 2, 1 },
    { N_("Europe"), N_("Slovak Koruna"), N_("Slovakia"), "SKK", NULL, TRUE, "SKK.png", 2, 1 },
    { N_("Europe"), N_("Slovene Tolar"), N_("Slovenia"), "SIT", NULL, TRUE, "SIT.png", 2, 1 },
    { N_("Europe"), N_("Spanish Peseta"), N_("Spain"), "ESP", "₧", FALSE, "ESP.png", 2, 1 },
    { N_("Europe"), N_("Swedish Krona"), N_("Sweden"), "SEK", "kr", TRUE, "SEK.png", 2, 1 },
    { N_("Europe"), N_("Swiss Franc"), N_("Liechtenstein"), "CHF", NULL, TRUE, "LIE.png", 2, 1 },
    { N_("Europe"), N_("Swiss Franc"), N_("Switzerland"), "CHF", NULL, TRUE, "CHF.png", 2, 1 },
    { N_("Europe"), N_("Hryvnia"), N_("Ukraine"), "UAH", NULL, TRUE, "UAH.png", 2, 1 },
    { N_("Northern America"), N_("Bahamian Dollar"), N_("Bahamas"), "BSD", "$", TRUE, "BSD.png", 0, 1 },
    { N_("Northern America"), N_("Barbados Dollar"), N_("Barbados"), "BBD", "$", TRUE, "BBD.png", 2, 1 },
    { N_("Northern America"), N_("Bermuda Dollar"), N_("Bermuda"), "BMD", "$", TRUE, "BMD.png", 2, 1 },
    { N_("Northern America"), N_("Canadian Dollar"), N_("Canada"), "CAD", "$", TRUE, "CAD.png", 2, 1 },
    { N_("Northern America"), N_("Cayman Islands Dollar"), N_("Cayman Islands"), "KYD", NULL, TRUE, "KYD.png", 2, 1 },
    { N_("Northern America"), N_("Cuban Convertible Peso"), N_("Cuba"), "CUC", "$", TRUE, "CUP.png", 2, 1 },
    { N_("Northern America"), N_("Cuban Peso"), N_("Cuba"), "CUP", "₱ 	", TRUE, "CUP.png", 2, 1 },
    { N_("Northern America"), N_("Dominican Peso"), N_("Dominican Republic"), "DOP", "₱", TRUE, "DOP.png", 2, 1 },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Grenada"), "XCD", "$", TRUE, "GRE.png", 2, 1 },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Saint Lucia"), "XCD", "$", TRUE, "SLC.png", 2, 1 },
    { N_("Northern America"), N_("Haitian Gourde"), N_("Haiti"), "HTG", NULL, TRUE, "HTG.png", 2, 1 },
    { N_("Northern America"), N_("Jamaican Dollar"), N_("Jamaica"), "JMD", "$", TRUE, "JMD.png", 2, 1 },
    { N_("Northern America"), N_("Netherlands Antillian Guilder"), N_("Netherlands Antilles"), "ANG", "ƒ", TRUE, "ANG.png", 2, 1 },
    { N_("Northern America"), N_("Trinidad and Tobago Dollar"), N_("Trinidad and Tobago"), "TTD", "$ 	", TRUE, "TTD.png", 2, 1 },
    { N_("Northern America"), N_("United States Dollar"), N_("United States"), "USD", "$", TRUE, "USD.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Australia"), "AUD", "$", TRUE, "AUD.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Kiribati"), "AUD", "$", TRUE, "KIR.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Nauru"), "AUD", "$", TRUE, "NAU.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Tuvalu"), "AUD", "$", TRUE, "TUV.png", 2, 1 },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("French Polynesia"), "XPF", NULL, TRUE, "FRF.png", 2, 1 },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("New Caledonia"), "XPF", NULL, TRUE, "FRF.png", 2, 1 },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("Wallis and Futuna"), "XPF", NULL, TRUE, "FRF.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Fiji Dollar"), N_("Fiji"), "FJD", "$", TRUE, "FJD.png", 2, 1 },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("Cook Islands"), "NZD", "$", TRUE, "COO.png", 2, 1 },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("New Zealand"), "NZD", NULL, TRUE, "NZD.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Papua New Guinea Kina"), N_("Papua New Guinea"), "PGK", NULL, TRUE, "PGK.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Samoa Tala"), N_("Samoa"), "WST", NULL, TRUE, "WST.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Solomon Islands Dollar"), N_("Solomon Islands"), "SBD", NULL, TRUE, "SBD.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Timor Escudo"), N_("Timor"), "TPE", NULL, TRUE, "TPE.png", 2, 1 },
    { N_("Pacific Ocean"), N_("Tongan Pa'anga"), N_("Tonga"), "TOP", NULL, TRUE, "TOP.png", 2, 1 },
    { N_("Pacific Ocean"), N_("United States Dollar"), N_("Panama"), "USD", "$", TRUE, "PAB.png", 2, 0 },
    { N_("Pacific Ocean"), N_("Vanuatu Vatu"), N_("Vanuatu"), "VUV", NULL, TRUE, "VUV.png", 2, 1 },
    { N_("Southern America"), N_("Peso"), N_("Argentina"), "ARP", "$", TRUE, "ARS.png", 2, 1 },
    { N_("Southern America"), N_("Boliviano"), N_("Bolivia"), "BOB", "$", TRUE, "BOB.png", 2, 1 },
    { N_("Southern America"), N_("Peso"), N_("Chile"), "CLP", "$", TRUE, "CLP.png", 2, 1 },
    { N_("Southern America"), N_("Peso"), N_("Colombia"), "COP", "₱", TRUE, "COP.png", 2, 1 },
    { N_("Southern America"), N_("Ecuador Sucre"), N_("Ecuador"), "ECS", NULL, FALSE, "ECS.png", 2, 1 },
    { N_("Southern America"), N_("Guyana Dollar"), N_("Guyana"), "GYD", NULL, TRUE, "GYD.png", 2, 1 },
    { N_("Southern America"), N_("Paraguay Guarani"), N_("Paraguay"), "PYG", NULL, TRUE, "PYG.png", 2, 1 },
    { N_("Southern America"), N_("Nuevos Soles"), N_("Peru"), "PEN", "S/.", TRUE, "PEN.png", 2, 1 },
    { N_("Southern America"), N_("Real"), N_("Brazil"), "BRL", "R$", TRUE, "BRL.png", 2, 1 },
    { N_("Southern America"), N_("Suriname Guilder"), N_("Suriname"), "SRG", NULL, TRUE, "SRD.png", 2, 1 },
    { N_("Southern America"), N_("Peso"), N_("Uruguay"), "UYU", "₱", TRUE, "UYU.png", 2, 1 },
    { N_("Southern America"), N_("Venezuelan Bolivar"), N_("Venezuela"), "VEB", NULL, TRUE, "VEB.png", 2, 1 },
    { NULL, NULL, NULL, NULL, NULL, FALSE, NULL, 0, 0 },
};

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/**
 * append the given currency in the list of known currencies
 * call both for known currencies and word currencies
 *
 * \param model the tree_model of the known currencies
 * \param currency_number
 *
 * \return
 *
 **/
static void gsb_currency_append_currency_to_list (GtkListStore *model,
												  gint currency_number)
{
    GdkPixbuf *pixbuf;
    GtkTreeIter iter;
    gchar *string;

    string = g_strconcat (gsb_dirs_get_pixmaps_dir (),
						  G_DIR_SEPARATOR_S,
						  "flags",
						  G_DIR_SEPARATOR_S,
						  gsb_data_currency_get_code_iso4217 (currency_number),
						  ".png",
						  NULL);
    pixbuf = gdk_pixbuf_new_from_file (string, NULL);
    g_free (string);

    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model),
						&iter,
						CURRENCY_FLAG_COLUMN, pixbuf,
						CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
						CURRENCY_ISO_CODE_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
						CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_code (currency_number),
						CURRENCY_FLOATING_COLUMN, gsb_data_currency_get_floating_point (currency_number),
						CURRENCY_NUMBER_COLUMN, currency_number,
						CURRENCY_HAS_FLAG, TRUE,
						-1);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_currency_config_change_selection (GtkTreeSelection *selection,
													  gpointer null)
{
    GSList *tmp_list;
    gint nbre_devises;

    tmp_list = gsb_data_currency_get_currency_list ();
    nbre_devises = g_slist_length (tmp_list);
    if (delete_devise_button)
    {
        if (nbre_devises > 1)
            gtk_widget_set_sensitive (delete_devise_button, TRUE);
        else
            gtk_widget_set_sensitive (delete_devise_button, FALSE);
    }
    return FALSE;
}

/**
 * create the tree which contains the currency list
 * used both for the currency known list, and the currency to add list
 *
 * \param
 *
 * \return a GtkTreeView
 **/
static GtkWidget *gsb_currency_config_create_list (void)
{
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkListStore *model;
    GtkWidget *tree_view;
    gint col_offset;

    /* Create tree store
       CURRENCY_FLAG_COLUMN,
       CURRENCY_HAS_FLAG,
       COUNTRY_NAME_COLUMN,
       CURRENCY_NAME_COLUMN,
       CURRENCY_ISO_CODE_COLUMN,
       CURRENCY_NICKNAME_COLUMN,
       CURRENCY_FLOATING_COLUMN,
       CURRENCY_NUMBER_COLUMN,
       CURRENCY_MAIN_CURRENCY_COLUMN,
	   CURRENCY_BACKGROUND_COLOR */

	model = gtk_list_store_new (NUM_CURRENCIES_COLUMNS,
								GDK_TYPE_PIXBUF,
								G_TYPE_BOOLEAN,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_INT,
								G_TYPE_INT,
								G_TYPE_INT,
								GDK_TYPE_RGBA);

    /* Create tree tree_view */
    tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
 	gtk_widget_set_name (tree_view, "tree_view");
    g_object_unref (G_OBJECT(model));

    /* connect the selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);

    /* Flag */
    cell = gtk_cell_renderer_pixbuf_new ();
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("Country name"),
															  cell,
															  "pixbuf", CURRENCY_FLAG_COLUMN,
															  "visible", CURRENCY_HAS_FLAG,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);

	/* Country name */
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN(column), cell, FALSE);
    gtk_tree_view_column_set_attributes (GTK_TREE_VIEW_COLUMN(column),
										 cell,
										 "text", COUNTRY_NAME_COLUMN,
										 "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
										 NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Currency name */
    cell = gtk_cell_renderer_text_new ();
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("Currency name"),
															  cell,
															  "text", CURRENCY_NAME_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

	/* ISO Code */
    cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("ISO Code"),
															  cell,
															  "text", CURRENCY_ISO_CODE_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, GSB_CENTER);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

	/* Sign */
    cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("Sign"),
															  cell,
															  "text", CURRENCY_NICKNAME_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, GSB_CENTER);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model),
					  COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

    /* Set signal */
    g_signal_connect (G_OBJECT (selection),
                      "changed",
                      G_CALLBACK (gsb_currency_config_change_selection),
                      NULL);

    return tree_view;
}

/**
 * Obtain selected currency from currency tree.
 *
 * \param tree_view		GtkTreeView to remove currency from.
 *
 * \return		the number of the selected currency.
 **/
static gint gsb_currency_config_get_selected (GtkTreeView *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *tree_model;
    gint currency_number;

	selection = gtk_tree_view_get_selection (tree_view);
    if (!selection || !gtk_tree_selection_get_selected (selection, &tree_model, &iter))
		return(FALSE);

    gtk_tree_model_get (tree_model,
						&iter,
						CURRENCY_NUMBER_COLUMN, &currency_number,
						-1);

	return currency_number;
}

/**
 * called when user change something in an entry
 * set the content of the entry for in the tree_view
 * (changes in memory done automatickly by gsb_autofunc_entry_new)
 *
 * \param entry
 * \param tree_view
 *
 * \return FALSE
 */
static gboolean gsb_currency_config_entry_changed (GtkWidget *entry,
												   GtkWidget *tree_view)
{
    gint currency_number;
    GtkTreeSelection *selection;
    GtkTreeModel *tree_model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    currency_number = gsb_currency_config_get_selected (GTK_TREE_VIEW (tree_view));

	if (!selection || !gtk_tree_selection_get_selected (selection, &tree_model, &iter))
		return(FALSE);

    gtk_list_store_set (GTK_LIST_STORE (tree_model),
						&iter,
						CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
						CURRENCY_ISO_CODE_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
						CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_code (currency_number),
						CURRENCY_FLOATING_COLUMN,  gsb_data_currency_get_floating_point (currency_number),
						-1);
    gsb_currency_update_combobox_currency_list ();
    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * fill the currency list in the popup
 * with all the world currencies
 *
 * \param tree_view the tree tree_view to fill in
 * \param include_obsolete TRUE to fill with the obsolete curencies
 *
 * \return
 **/
static void gsb_currency_config_fill_popup_list (GtkTreeView *tree_view,
												 gboolean include_obsolete)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    struct Iso4217Currency *currency = iso_4217_currencies;

    model = gtk_tree_view_get_model (tree_view);

    while (currency->country_name)
    {
		if (include_obsolete || currency->active)
		{
			GdkPixbuf *pixbuf;
			gchar *string;

			string = g_build_filename (gsb_dirs_get_pixmaps_dir (), "flags", currency->flag_filename, NULL);
			pixbuf = gdk_pixbuf_new_from_file (string, NULL);
			g_free (string);

			string = g_strconcat (" ", _(currency->country_name), NULL);
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model),
								&iter,
								CURRENCY_FLAG_COLUMN, pixbuf,
								COUNTRY_NAME_COLUMN, string,
								CURRENCY_NAME_COLUMN, _(currency->currency_name),
								CURRENCY_ISO_CODE_COLUMN, _(currency->currency_code),
								CURRENCY_NICKNAME_COLUMN, _(currency->currency_nickname),
								CURRENCY_FLOATING_COLUMN, currency->floating_point,
								CURRENCY_NUMBER_COLUMN, currency,
								CURRENCY_HAS_FLAG, TRUE,
								CURRENCY_MAIN_CURRENCY_COLUMN, currency->main_currency,
								-1);
			g_free (string);
		}
		currency++;
    }
}

/**
 * fill the currencies list with all the known currencies
 *
 * \param model
 *
 * \return FALSE
 **/
static gboolean gsb_currency_config_fill_tree (GtkTreeModel *model)
{
    GSList *tmp_list;

    if (!model)
		return FALSE;

    tmp_list = gsb_data_currency_get_currency_list ();

    while (tmp_list)
    {
		gint currency_number;

		currency_number = gsb_data_currency_get_no_currency (tmp_list->data);

		gsb_currency_append_currency_to_list (GTK_LIST_STORE (model), currency_number);
		tmp_list = tmp_list->next;
    }

    return FALSE;
}

/**
 * Remove selected currency from tree.  In fact, this is a generic
 * function that could be used for any purpose (and could be then
 * renamed).
 *
 * \param tree_view		GtkTreeView to remove selected entry from.
 *
 * \return
 **/
static void gsb_currency_config_remove_selected_from_view (GtkTreeView *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *tree_model;

	selection = gtk_tree_view_get_selection (tree_view);
    if (!selection || !gtk_tree_selection_get_selected (selection, &tree_model, &iter))
		return;

    gtk_list_store_remove (GTK_LIST_STORE (tree_model), &iter);
}

/**
 * Remove selected currency from currency list.  First, be sure there
 * is no use for it.
 *
 * \param button		Widget that triggered event.
 * \param tree_view		GtkTreeView that contains selected currency.
 *
 * \return
 **/
static void gsb_currency_config_remove_currency (GtkWidget *button,
												 GtkWidget *tree_view)
{
    GtkWidget *entry_code;
    GtkWidget *entry_floating_point;
    GtkWidget *entry_iso_code;
    GtkWidget *entry_name;
    GtkTreeModel *model;
    GSList *list_tmp;
    gint currency_number;

    devel_debug (NULL);
    currency_number = gsb_currency_config_get_selected (GTK_TREE_VIEW (tree_view));
    if (!currency_number)
		return;

    /* we look for that currency in all the transactions,
     * if we find it, we cannot delete it */

    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while (list_tmp)
    {
		gint transaction_number;
		transaction_number = gsb_data_transaction_get_transaction_number (list_tmp->data);

		if (gsb_data_transaction_get_currency_number (transaction_number) == currency_number)
		{
			gchar *tmp_str1;
			gchar *tmp_str2;

			tmp_str1 = g_strdup_printf (_("Currency '%s' is used in current "
										  "file.  Grisbi can't delete it."),
										gsb_data_currency_get_name (currency_number));
			tmp_str2 = g_strdup_printf (_("Impossible to remove currency '%s'"),
										gsb_data_currency_get_name (currency_number));
			dialogue_error_hint (tmp_str1, tmp_str2);
			g_free (tmp_str1);
			g_free (tmp_str2);

			return;
		}
		else
			list_tmp = list_tmp->next;
    }

    /* check the currency in the scheduled transactions */
    list_tmp = gsb_data_scheduled_get_scheduled_list ();

    while (list_tmp)
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp->data);
	if (gsb_data_scheduled_get_currency_number (scheduled_number) == currency_number)
	{
	    gchar *tmp_str1 = g_strdup_printf (_("Currency '%s' is used in current "
                        "file. Grisbi can't delete it."),
                        gsb_data_currency_get_name (currency_number));
	    gchar *tmp_str2 = g_strdup_printf (_("Impossible to remove currency '%s'"),
                        gsb_data_currency_get_name (currency_number));
	    dialogue_error_hint (tmp_str1, tmp_str2);
	    g_free (tmp_str1);
	    g_free (tmp_str2);
	    return;
	}
	else
	    list_tmp = list_tmp->next;
    }

    /*  pbiava the 02/22/09 erase the entries */
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    entry_name = g_object_get_data (G_OBJECT(model), "entry_name");
    gsb_autofunc_int_erase_entry (entry_name);
    entry_iso_code = g_object_get_data (G_OBJECT(model), "entry_iso_code");
    gsb_autofunc_int_erase_entry (entry_iso_code);
    entry_code = g_object_get_data (G_OBJECT(model), "entry_code");
    gsb_autofunc_int_erase_entry (entry_code);
    entry_floating_point = g_object_get_data (G_OBJECT(model), "entry_floating_point");
    gsb_autofunc_int_erase_entry (entry_floating_point);

    gsb_data_currency_remove (currency_number);
    gsb_currency_config_remove_selected_from_view (GTK_TREE_VIEW(tree_view));
    gsb_currency_update_combobox_currency_list ();
}

/**
 * called when select a currency in the known list of currencies
 * fill the corresponding entries to append it
 *
 * \param selection
 * \param model the tree_model
 *
 * \return
 **/
static gboolean gsb_currency_config_select_currency (GtkTreeSelection *selection,
													 gpointer null)
{
    GtkWidget *entry_code;
    GtkWidget *entry_floating_point;
    GtkWidget *entry_iso_code;
    GtkWidget *entry_name;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *currency_iso_code;
    gchar *currency_name;
    gchar *currency_nickname;
    gint currency_floating;
    gint currency_number;

    if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter))
		return(FALSE);

    /* get the entries */
    entry_name = g_object_get_data (G_OBJECT(model), "entry_name");
    entry_iso_code = g_object_get_data (G_OBJECT(model), "entry_iso_code");
    entry_code = g_object_get_data (G_OBJECT(model), "entry_code");
    entry_floating_point = g_object_get_data (G_OBJECT(model), "entry_floating_point");

    /* get the contents of the selected line */
    gtk_tree_model_get (model,
						&iter,
						CURRENCY_NAME_COLUMN, &currency_name,
						CURRENCY_ISO_CODE_COLUMN, &currency_iso_code,
						CURRENCY_NICKNAME_COLUMN, &currency_nickname,
						CURRENCY_FLOATING_COLUMN, &currency_floating,
						CURRENCY_NUMBER_COLUMN, &currency_number,
						-1);

    gsb_autofunc_entry_set_value (entry_name, currency_name, currency_number);
    gsb_autofunc_entry_set_value (entry_iso_code, currency_iso_code, currency_number);
    gsb_autofunc_entry_set_value (entry_code, currency_nickname, currency_number);
    gsb_autofunc_int_set_value (entry_floating_point, currency_floating, currency_number);

    return (FALSE);
}

/**
 * called when select a currency in the whole world list in the add popup
 * fill the corresponding entries to append it
 *
 * \param selection
 * \param model 		the tree_model
 *
 * \return
 **/
static gboolean gsb_currency_config_select_currency_popup (GtkTreeSelection *selection,
														   GtkTreeModel *model)
{
    GtkWidget *entry_code;
    GtkWidget *entry_floating_point;
    GtkWidget *entry_iso_code;
    GtkWidget *entry_name;
    GtkTreeIter iter;
    gchar *currency_iso_code;
    gchar *currency_name;
    gchar *currency_nickname;
	gchar *tmp_str;
    gint currency_floating;

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
		return(FALSE);

    entry_name = g_object_get_data (G_OBJECT(model), "entry_name");
    entry_iso_code = g_object_get_data (G_OBJECT(model), "entry_iso_code");
    entry_code = g_object_get_data (G_OBJECT(model), "entry_code");
    entry_floating_point = g_object_get_data (G_OBJECT(model), "entry_floating_point");


    gtk_tree_model_get (model,
						&iter,
						CURRENCY_NAME_COLUMN, &currency_name,
						CURRENCY_ISO_CODE_COLUMN, &currency_iso_code,
						CURRENCY_NICKNAME_COLUMN, &currency_nickname,
						CURRENCY_FLOATING_COLUMN, &currency_floating,
						-1);

    if (!currency_name)
		currency_name = "";
    if (!currency_nickname)
		currency_nickname = "";
    if (!currency_iso_code)
		currency_iso_code = "";

    gtk_entry_set_text (GTK_ENTRY (entry_name), currency_name);
    gtk_entry_set_text (GTK_ENTRY (entry_iso_code), currency_iso_code);
    gtk_entry_set_text (GTK_ENTRY (entry_code), currency_nickname);
    tmp_str = utils_str_itoa (currency_floating);
    gtk_entry_set_text (GTK_ENTRY (entry_floating_point), tmp_str);
    g_free (tmp_str);

	return (FALSE);
}

/**
 * Set an integer to the value of a menu.  Normally called via a GTK
 * "changed" signal handler.
 *
 * \param menu 		a pointer to a menu widget.
 * \param dummy 	unused
 *
 * \return
 **/
static gboolean gsb_currency_config_set_int_from_combobox (GtkWidget *combobox,
														   gint *dummy)
{
    gint *data;

    data = g_object_get_data (G_OBJECT(combobox), "pointer");

    if (data)
    {
		*data = gsb_currency_get_currency_from_combobox (combobox);
    }

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return (FALSE);
}

/**
 * called when toggle the button show/hide the obselete currencies
 * in the popup
 * fill the list according that button
 *
 * \param checkbox 		contains what the user want
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean gsb_currency_config_update_list (GtkWidget *checkbox,
												 GtkTreeView *tree_view)
{
    GtkTreeModel *model;

    model = gtk_tree_view_get_model (tree_view);
    gtk_list_store_clear (GTK_LIST_STORE (model));
    gsb_currency_config_fill_popup_list (tree_view,
										 gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)));

    /* re-select the default currency
     * TODO : should use the GtkTreeModelFilter to show or not the obsoletes currencies,
     * so the selection shouldn't be lost */
    gtk_tree_model_foreach (GTK_TREE_MODEL(model),
							(GtkTreeModelForeachFunc) gsb_currency_config_select_default,
							tree_view);

	utils_set_list_store_background_color (GTK_WIDGET (tree_view), CURRENCY_BACKGROUND_COLOR);

    return FALSE;
}

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/**
 * Creates the currency list and associated form to configure them.
 *
 * \param
 *
 * \return A newly created vbox
 **/
GtkWidget *gsb_currency_config_create_page (void)
{
    GtkWidget *vbox_pref;
    GtkWidget *paddinggrid;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *currency_list_view;
    GtkTreeModel *currency_tree_model;

    vbox_pref = new_vbox_with_title_and_icon (_("Currencies"), "gsb-currencies-32.png");
    paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, _("Known currencies"));

    /* Currency list */
    scrolled_window = utils_prefs_scrolled_window_new (NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_PG, 200);
    gtk_grid_attach (GTK_GRID (paddinggrid), scrolled_window, 0, 0, 2, 3);

    /* Create it. */
    currency_list_view = gsb_currency_config_create_list ();
    currency_tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (currency_list_view));
    gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET(currency_list_view));
    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (currency_list_view)),
					  "changed",
					  G_CALLBACK (gsb_currency_config_select_currency),
					  NULL);

    /* if nothing opened, all is unsensitive */
    if (!gsb_data_account_get_accounts_amount ())
		gtk_widget_set_sensitive (vbox_pref, FALSE);
    else
    {
		/* fill the list with the known currencies */
		gsb_currency_config_fill_tree (currency_tree_model);
    }

    /* Create Add/Remove buttons */

    /* Button "Add" */
    button = utils_buttons_button_new_from_stock ("gtk-add", _("Add"));
    gtk_widget_set_margin_end (button, MARGIN_END);
    utils_widget_set_padding (button, 0, MARGIN_TOP);
    g_signal_connect (G_OBJECT (button),
					  "clicked",
					  G_CALLBACK  (gsb_currency_config_add_currency),
					  currency_list_view);
    gtk_grid_attach (GTK_GRID (paddinggrid), button, 0, 3, 1, 1);

    /* Button "Remove" */
    delete_devise_button = utils_buttons_button_new_from_stock ("gtk-remove", _("Remove"));
    utils_widget_set_padding (delete_devise_button, 0, MARGIN_TOP);
    gtk_widget_set_sensitive (delete_devise_button, FALSE);
    g_signal_connect (G_OBJECT (delete_devise_button),
					  "clicked",
					  G_CALLBACK (gsb_currency_config_remove_currency),
					  currency_list_view);
    gtk_grid_attach (GTK_GRID (paddinggrid), delete_devise_button, 1, 3, 1, 1);

    /* Input form for currencies */
    paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, _("Currency properties"));

    /* Create currency name entry */
    label = gtk_label_new (_("Name: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_grid_attach (GTK_GRID (paddinggrid), label, 0, 0, 1, 1);
    entry = gsb_autofunc_entry_new (NULL,
									G_CALLBACK (gsb_currency_config_entry_changed),
									currency_list_view,
									G_CALLBACK (gsb_data_currency_set_name),
									0);
    gtk_grid_attach (GTK_GRID (paddinggrid), entry, 1, 0, 1, 1);
    g_object_set_data (G_OBJECT(currency_tree_model), "entry_name", entry);

    /* Create Sign entry */
    label = gtk_label_new (_("Sign: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_grid_attach (GTK_GRID (paddinggrid), label, 0, 1, 1, 1);
    entry = gsb_autofunc_entry_new (NULL,
									G_CALLBACK (gsb_currency_config_entry_changed),
									currency_list_view,
									G_CALLBACK (gsb_data_currency_set_code),
									0);
    gtk_grid_attach (GTK_GRID (paddinggrid), entry, 1, 1, 1, 1);
    g_object_set_data (G_OBJECT(currency_tree_model), "entry_code", entry);

    /* Create ISO code entry */
    label = gtk_label_new (_("ISO code: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_grid_attach (GTK_GRID (paddinggrid), label, 0, 2, 1, 1);
    entry = gsb_autofunc_entry_new (NULL,
									G_CALLBACK (gsb_currency_config_entry_changed),
									currency_list_view,
									G_CALLBACK (gsb_data_currency_set_code_iso4217),
									0);
    gtk_grid_attach (GTK_GRID (paddinggrid), entry, 1, 2, 1, 1);
    g_object_set_data (G_OBJECT(currency_tree_model), "entry_iso_code", entry);

    /* Create floating point entry */
    label = gtk_label_new (_("Floating point: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_grid_attach (GTK_GRID (paddinggrid), label, 0, 3, 1, 1);
    entry = gsb_autofunc_int_new (0,
								  G_CALLBACK (gsb_currency_config_entry_changed),
								  currency_list_view,
								  G_CALLBACK (gsb_data_currency_set_floating_point),
								  0);
    gtk_grid_attach (GTK_GRID (paddinggrid), entry, 1, 3, 1, 1);
    g_object_set_data (G_OBJECT(currency_tree_model), "entry_floating_point", entry);

	utils_set_list_store_background_color (currency_list_view, CURRENCY_BACKGROUND_COLOR);

    /* for now we want nothing in the entry of floating point */
    gsb_autofunc_int_erase_entry (entry);

	return (vbox_pref);
}

/**
 * show combo_box with the currencies to choose the currency
 * for totals of lists (payee, categories, budgets
 *
 * \param
 *
 * \return the widget to set in the conf
 */
GtkWidget *gsb_currency_config_create_totals_page (void)
{
    GtkWidget *table, *label;

    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), 5);
    gtk_grid_set_row_spacing (GTK_GRID (table), 5);

    label = gtk_label_new (_("Currency for payees tree: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    combo_devise_totaux_tiers = gsb_currency_config_new_combobox (&etat.no_devise_totaux_tiers,
																  payees_fill_list);
    gtk_grid_attach (GTK_GRID (table), combo_devise_totaux_tiers, 1, 0, 1, 1);

    label = gtk_label_new (_("Currency for categories tree: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1,1);
    combo_devise_totaux_categ = gsb_currency_config_new_combobox (&etat.no_devise_totaux_categ,
																  categories_fill_list);
    gtk_grid_attach (GTK_GRID (table), combo_devise_totaux_categ, 1, 1, 1, 1);

    label = gtk_label_new (_("Currency for budgetary lines tree: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);
    combo_devise_totaux_ib = gsb_currency_config_new_combobox (&etat.no_devise_totaux_ib,
															   budgetary_lines_fill_list);
    gtk_grid_attach (GTK_GRID (table), combo_devise_totaux_ib, 1, 2, 1, 1);

    return (table);
}

/**
 * Create a new GtkComboBox with a pointer to an integer that will be
 * modified according to the entry's value.
 *
 * \param value		A pointer to a gint which contains the currency number
 * \param hook 		An optional function to execute as a handler
 *
 * \return A newly allocated option menu.
 */
GtkWidget *gsb_currency_config_new_combobox (gint *value,
											 GCallback hook)
{
    GtkWidget *combo_box;

    combo_box = gsb_currency_make_combobox (FALSE);

    if (value && *value)
		gsb_currency_set_combobox_history (combo_box, *value);

    g_signal_connect (G_OBJECT (combo_box),
					  "changed",
					  (GCallback) gsb_currency_config_set_int_from_combobox,
					  value);
    g_object_set_data (G_OBJECT (combo_box), "pointer", value);

    if (hook)
		g_object_set_data (G_OBJECT (combo_box),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(combo_box),
																	 "changed",
																	 G_CALLBACK (hook),
																	 value)));

    return combo_box;
}




/* ********************************************************************************************************************************** */
/* NEXT PART about the list to add a new currency */
/* ********************************************************************************************************************************** */
/**
 * Pop up a dialog to create a new currency, do some sanity checks and
 * call the gsb_currency_config_create_currency() function to do the grunt work.
 *
 * \param button				GtkButton that triggered event.
 * \param currency_tree_model 	the treemodel to add the new currency (can be NULL)
 *
 * \return TRUE if currency has been created.
 **/
gboolean gsb_currency_config_add_currency (GtkWidget *button,
										   GtkWidget *tree_view)
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *list;
    GtkWidget *main_vbox;
    GtkWidget *paddingbox;
    GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *entry_code;
    GtkWidget *entry_floating_point;
    GtkWidget *entry_iso_code;
    GtkWidget *entry_name;
    GtkTreeModel *model;
	GtkTreeModel *currency_tree_model;
    const gchar *currency_code;
    const gchar *currency_iso_code;
    const gchar *currency_name;
    gint currency_number;
    gint floating_point;
    gint result;

	dialog = gtk_dialog_new_with_buttons (_("Add a currency"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL,
										  "gtk-close", 1,
										  NULL);

    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

    main_vbox = new_vbox_with_title_and_icon (_("Select base currency for your account"),
											  "gsb-currencies-32.png");
    gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), main_vbox, TRUE, TRUE, 0);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (main_vbox), vbox, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BOX_BORDER_WIDTH);

    paddingbox = new_paddingbox_with_title (vbox, TRUE, _("World currencies"));

    /* Create list */
    list = gsb_currency_config_create_box_popup (G_CALLBACK (gsb_currency_config_select_currency_popup));
    model = g_object_get_data (G_OBJECT(list), "model");

    gtk_box_pack_start (GTK_BOX(paddingbox), list, TRUE, TRUE, 5);

    paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Currency details"));

    /* Create table */
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), 5);
    gtk_grid_set_row_spacing (GTK_GRID (table), 5);
    gtk_box_pack_start (GTK_BOX (paddingbox), table, TRUE, TRUE, 0);

    /* Currency name */
    label = gtk_label_new (_("Currency name: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    entry_name = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entry_name), TRUE);
    gtk_grid_attach (GTK_GRID (table), entry_name, 1, 0, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_name", entry_name);

    /* Currency ISO code */
    label = gtk_label_new (_("Currency international code: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    entry_iso_code = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), entry_iso_code, 1, 1, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_iso_code", entry_iso_code);

    /* Currency usual sign */
    label = gtk_label_new (_("Currency sign: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);
    entry_code = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), entry_code, 1, 2, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_code", entry_code);

    /* Create floating point entry */
    label = gtk_label_new (_("Floating point: "));
    utils_labels_set_alignement (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 3, 1, 1);
    entry_floating_point = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), entry_floating_point, 1, 3, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_floating_point", entry_floating_point);

    /* Select default currency. */
    gtk_tree_model_foreach (GTK_TREE_MODEL(model),
							(GtkTreeModelForeachFunc) gsb_currency_config_select_default,
							g_object_get_data (G_OBJECT(list), "tree_view"));

dialog_return:
    gtk_widget_show_all (GTK_WIDGET (dialog));
    result = gtk_dialog_run (GTK_DIALOG (dialog));

    if (result)
    {
		currency_name = gtk_entry_get_text (GTK_ENTRY (entry_name));
		currency_code = gtk_entry_get_text (GTK_ENTRY (entry_code));
		currency_iso_code = gtk_entry_get_text (GTK_ENTRY (entry_iso_code));
		floating_point = utils_str_atoi (gtk_entry_get_text (GTK_ENTRY (entry_floating_point)));

		if (strlen (currency_name)
			&& (strlen (currency_code) || strlen (currency_iso_code)))
		{
			/* check if the currency exists si la devise existe on ne fait rien */

			if (!gsb_data_currency_get_number_by_name (currency_name)
				&& !gsb_data_currency_get_number_by_code_iso4217 (currency_iso_code))
			{
				currency_number = gsb_currency_config_create_currency (currency_name,
																	   currency_code,
																	   currency_iso_code,
																	   floating_point);

				/* update the currencies list in account properties */
				gsb_currency_update_combobox_currency_list ();

				currency_tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
				if (currency_tree_model && currency_number > 0)
				{
					gsb_currency_append_currency_to_list (GTK_LIST_STORE (currency_tree_model), currency_number);
					gtk_widget_destroy (GTK_WIDGET (dialog));
					gsb_file_set_modified (TRUE);
					utils_set_list_store_background_color (tree_view, CURRENCY_BACKGROUND_COLOR);

					return TRUE;
				}
			}
		}
		else
		{
			dialogue_warning_hint (_("Currency name and either international "
									 "currency code or currency nickname should be set."),
								   _("All fields are not filled in"));
			goto dialog_return;
		}
    }
    gtk_widget_destroy (GTK_WIDGET (dialog));

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_currency_config_add_currency_set_combobox (GtkWidget *button,
														GtkWidget *combobox)
{
    gsb_currency_config_add_currency (button, FALSE);
    gsb_currency_set_combobox_history (combobox, gsb_data_currency_max_number ());

    return FALSE;
}


/**
 * create a new currency according to the param
 *
 * \param currency_name
 * \param currency_code
 * \param currency_isocode
 * \param floating_point
 *
 * \return the number of the new currency
 **/
gint gsb_currency_config_create_currency (const gchar *currency_name,
										  const gchar *currency_code,
										  const gchar *currency_isocode,
										  gint floating_point)
{
    gint currency_number;

    currency_number = gsb_data_currency_new (currency_name);
    gsb_data_currency_set_code (currency_number, currency_code);
    gsb_data_currency_set_code_iso4217 (currency_number, currency_isocode);
    gsb_data_currency_set_floating_point (currency_number, floating_point);

	return currency_number;
}


/**
 * look for a currency with its code name in the iso list
 * and create it
 *
 * \param currency_name the currency code we look for
 *
 * \return the number of the new currency or FALSE if not found
 **/
gint gsb_currency_config_create_currency_from_iso4217list (gchar *currency_name)
{
    struct Iso4217Currency *currency = iso_4217_currencies;
    gchar *tmp = g_strdup (currency_name);
    g_strchomp (tmp);

    while (currency->country_name)
    {
	if (!strcmp (currency->currency_code, tmp) && currency->main_currency)
	{
	    g_free (tmp);
	    return gsb_currency_config_create_currency (currency->currency_name,
							 currency->currency_nickname,
							 currency->currency_code,
							 currency->floating_point);
	}
	currency++;
    }

    g_free (tmp);

    return FALSE;
}



/**
 * create the box which contains the world currencies list for the
 * add currency popup
 * the tree view and model are saved into the box with the keys :
 * 	"model" and "tree_view"
 *
 * \param select_callback callback to call when a line is selected
 * 		callback (GtkTreeSelection *, GtkTreeModel *)
 *
 * \return a vbox
 **/
GtkWidget *gsb_currency_config_create_box_popup (GCallback select_callback)
{
    GtkWidget *checkbox;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *vbox;
    GtkTreeModel *model;

    sw = utils_prefs_scrolled_window_new (NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_SW, 200);

    tree_view = gsb_currency_config_create_list ();
    gtk_widget_set_size_request (tree_view, -1, 200);
    model = gtk_tree_view_get_model (GTK_TREE_VIEW(tree_view));
    if (select_callback)
		g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
						  "changed",
						  G_CALLBACK (select_callback),
						  model);

    gtk_container_add (GTK_CONTAINER (sw), tree_view);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX(vbox), sw, TRUE, TRUE, 0);

    checkbox = gtk_check_button_new_with_label (_("Include obsolete currencies"));
    gtk_box_pack_start (GTK_BOX(vbox), checkbox, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT(checkbox),
                      "toggled",
                      (GCallback) gsb_currency_config_update_list,
                      tree_view);

    gsb_currency_config_fill_popup_list (GTK_TREE_VIEW(tree_view), FALSE);

    g_object_set_data (G_OBJECT(vbox), "model", model);
    g_object_set_data (G_OBJECT(vbox), "tree_view", tree_view);

	utils_set_list_store_background_color (tree_view, CURRENCY_BACKGROUND_COLOR);
    gtk_widget_show_all (GTK_WIDGET (vbox));

    return vbox;
}

/**
 * foreach function on the tree_view to find the default currency
 * according to the current locale
 *
 * \param tree_model
 * \param path
 * \param iter
 * \param tree_view
 *
 * \return TRUE if found
 **/
gboolean gsb_currency_config_select_default (GtkTreeModel *tree_model,
											 GtkTreePath *path,
											 GtkTreeIter *iter,
											 GtkTreeView *tree_view)
{
    struct lconv *locale;
    gchar *code;
    gchar *country;
    gchar *symbol;
    gboolean good = FALSE;
    gboolean main_currency;

	locale = gsb_locale_get_locale ();
    gtk_tree_model_get (GTK_TREE_MODEL (tree_model),
						iter,
						CURRENCY_ISO_CODE_COLUMN, &code,
						COUNTRY_NAME_COLUMN, &country,
						CURRENCY_MAIN_CURRENCY_COLUMN, &main_currency,
						-1);
    if (locale && locale->int_curr_symbol && strlen (locale->int_curr_symbol))
    {
		symbol = g_strdup (locale->int_curr_symbol);
		g_strstrip (symbol);
		/* When matching, weed out currencies that are not "main"
		 * currency, that is for example, do not match USD on Panama
		 * or our US folks will be prompted to use USD as in Panama by
		 * default.  --benj */
		if (!strcmp (code, symbol) && main_currency)
		{
			good = TRUE;
		}
    }
    else
    {
		symbol = country;
		if (!strcmp (symbol, _("United States")))
		{
			symbol = g_strdup ("USD");
			good = TRUE;
		}
    }

    if (good)
    {
		gchar*tmp_str;

		tmp_str = g_strdup_printf ("found '%s'", symbol);
		devel_debug (tmp_str);
		g_free (tmp_str);
		g_free (symbol);
		gtk_tree_selection_select_path (gtk_tree_view_get_selection (tree_view), path);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, TRUE, GSB_CENTER, 0);

		return TRUE;
    }

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
