/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          https://www.grisbi.org/                                           */
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
 * \file gsb_data_currency.c
 * work with the currency structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <string.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_data_currency.h"
#include "dialog.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/**
 * \struct
 * Describe a currency
 */
typedef struct _CurrencyStruct		CurrencyStruct;

struct _CurrencyStruct
{									/* correspondance avec struct Iso4217Currency */
    gint currency_number;
    gchar *currency_name;			/* -> currency_name */
    gchar *currency_nickname;		/* -> currency_nickname */
    gchar *currency_code_iso4217;	/* -> currency_code_iso */
    gint currency_floating_point;	/* -> floating_point (number of digits after the point) */
};

/*START_STATIC*/
/* struct Iso4217Currency; */
static Iso4217Currency iso_4217_currencies[] =
{
	/* continent,   currency_name,        country_name,code_iso,nickname .......*/
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
	{ N_("World"), N_("bitcoin"), N_("World"), "XBT", "₿", TRUE, "XBT.png", 3, 0},
    { NULL, NULL, NULL, NULL, NULL, FALSE, NULL, 0, 0 },
};

static void _gsb_data_currency_free ( CurrencyStruct *currency );
static gpointer gsb_data_currency_get_structure ( gint currency_number );
static gboolean gsb_data_currency_set_default_currency ( gint currency_number );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of CurrencyStruct */
static GSList *currency_list = NULL;

/** a pointer to the last currency used (to increase the speed) */
static CurrencyStruct *currency_buffer;

/** the number of the default currency */
static gint default_currency_number;

/**
 * set the currencies global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_currency_init_variables ( void )
{
    if ( currency_list )
    {
        GSList* tmp_list = currency_list;
        while ( tmp_list )
        {
	    CurrencyStruct *currency;
	    currency = tmp_list -> data;
	    tmp_list = tmp_list -> next;
            _gsb_data_currency_free ( currency );
        }
	g_slist_free ( currency_list );
    }
    currency_list = NULL;
    currency_buffer = NULL;
    default_currency_number = 0;
    return FALSE;
}


/**
 * find and return the structure of the currency asked
 *
 * \param currency_number number of currency
 *
 * \return the adr of the struct of the currency (NULL if doesn't exit)
 * */
gpointer gsb_data_currency_get_structure ( gint currency_number )
{
    GSList *tmp;

    if (!currency_number)
	return NULL;

    /* before checking all the currencies, we check the buffer */

    if ( currency_buffer
	 &&
	 currency_buffer -> currency_number == currency_number )
	return currency_buffer;

    tmp = currency_list;

    while ( tmp )
    {
	CurrencyStruct *currency;

	currency = tmp -> data;

	if ( currency -> currency_number == currency_number )
	{
	    currency_buffer = currency;
	    return currency;
	}
	tmp = tmp -> next;
    }
    return NULL;
}

/**
 * return the number of the currency given in param
 *
 * \param currency_ptr a pointer to the struct of the currency
 *
 * \return the number of the currency, 0 if problem
 * */
gint gsb_data_currency_get_no_currency ( gpointer currency_ptr )
{
    CurrencyStruct *currency;

    if ( !currency_ptr )
	return 0;

    currency = currency_ptr;
    currency_buffer = currency;
    return currency -> currency_number;
}


/**
 * give the g_slist of currency structure
 * usefull when want to check all currencies
 *
 * \param none
 *
 * \return the g_slist of currencies structure
 * */
GSList *gsb_data_currency_get_currency_list ( void )
{
    return currency_list;
}



/**
 * find and return the last number of currency
 *
 * \param none
 *
 * \return last number of currency
 * */
gint gsb_data_currency_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = currency_list;

    while ( tmp )
    {
	CurrencyStruct *currency;

	currency = tmp -> data;

	if ( currency -> currency_number > number_tmp )
	    number_tmp = currency -> currency_number;

	tmp = tmp -> next;
    }

    return number_tmp;
}


/**
 * create a new currency, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the currency (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new currency
 * */
gint gsb_data_currency_new ( const gchar *name )
{
    CurrencyStruct *currency;

    currency = g_malloc0 ( sizeof ( CurrencyStruct ));
    currency -> currency_number = gsb_data_currency_max_number () + 1;

    if (name)
	currency -> currency_name = my_strdup (name);
    else
	currency -> currency_name = NULL;

    currency_list = g_slist_append ( currency_list, currency );

    if ( ! gsb_data_currency_get_default_currency () )
	gsb_data_currency_set_default_currency ( currency -> currency_number );

    return currency -> currency_number;
}

/**
 * This internal function is called to free the memory used by a CurrencyStruct structure
 */
static void _gsb_data_currency_free ( CurrencyStruct *currency )
{
    if ( ! currency )
        return;
    if ( currency -> currency_name )
        g_free ( currency -> currency_name );
    if ( currency -> currency_nickname )
        g_free ( currency -> currency_nickname );
    if ( currency -> currency_code_iso4217 )
        g_free ( currency -> currency_code_iso4217 );
    g_free ( currency );
    if ( currency_buffer == currency )
	currency_buffer = NULL;
}

/**
 * remove a currency
 * set all the currencies of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param currency_number the currency we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_currency_remove ( gint currency_number )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency_list = g_slist_remove ( currency_list,
				     currency );

    _gsb_data_currency_free (currency);

    return TRUE;
}


/**
 * set a new number for the currency
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param currency_number the number of the currency
 * \param new_no_currency the new number of the currency
 *
 * \return the new number or 0 if the currency doen't exist
 * */
gint gsb_data_currency_load_currency (gint new_no_currency)
{
	GSList *tmp_list;
    CurrencyStruct *currency;

	/* test existence de la devise */
    tmp_list = currency_list;

    while (tmp_list)
    {
		CurrencyStruct *tmp_currency;

		tmp_currency = tmp_list->data;
		if (tmp_currency->currency_number == new_no_currency)
		{
			run.file_modification = TRUE;

			return 0;
		}

		tmp_list = tmp_list->next;
    }

    currency = g_malloc0 (sizeof (CurrencyStruct));
    currency->currency_number = new_no_currency;
	currency->currency_name = NULL;
	currency->currency_floating_point = 2;
    currency_list = g_slist_append (currency_list, currency);

    return new_no_currency;
}


/**
 * give the default currency number
 *
 * \param
 *
 * \return the default currency number
 * */
gint gsb_data_currency_get_default_currency (void)
{
    return default_currency_number;
}

/**
 * set the default currency number
 * if the currency corresponding to the number doesn't exists, return FALSE and do nothing
 *
 * \param currency_number
 *
 * \return TRUE ok, FALSE the currency doesn't exist
 * */
gboolean gsb_data_currency_set_default_currency ( gint currency_number )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    default_currency_number = currency_number;
    return TRUE;
}



/**
 * return the name of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the name of the currency or NULL if problem
 * */
gchar *gsb_data_currency_get_name ( gint currency_number )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    return currency -> currency_name;
}


/**
 * set the name of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param name the name of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_name ( gint currency_number,
				      const gchar *name )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    /* we free the last name */
    if ( currency -> currency_name )
	g_free (currency -> currency_name);

    /* and copy the new one */
    if (name)
	currency -> currency_name = my_strdup (name);
    else
	currency -> currency_name = NULL;

    return TRUE;
}


/**
 * return the  of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the currency_nickname of the currency or NULL if problem
 * */
const gchar *gsb_data_currency_get_nickname ( gint currency_number )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    return currency -> currency_nickname;
}


/**
 * set the  of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the  of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_nickname ( gint currency_number,
				      const gchar *currency_nickname )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    /* we free the last  */
    if ( currency -> currency_nickname )
	g_free (currency -> currency_nickname);

    /* and copy the new one */
    if (currency_nickname)
	currency -> currency_nickname = my_strdup (currency_nickname);
    else
	currency -> currency_nickname = NULL;

    return TRUE;
}


/**
 * return the currency_code_iso4217 of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the code_iso4217 of the currency or NULL if problem or not exist
 * */
gchar *gsb_data_currency_get_code_iso4217 ( gint currency_number )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    return currency -> currency_code_iso4217;
}


/**
 * set the currency_code_iso4217 of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the code_iso4217 of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_code_iso4217 ( gint currency_number,
					      const gchar *currency_code_iso4217 )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    /* we free the last  */
    if ( currency -> currency_code_iso4217 )
	g_free (currency ->currency_code_iso4217 );

    /* and copy the new one */
    if (currency_code_iso4217)
	currency -> currency_code_iso4217 = my_strdup (currency_code_iso4217);
    else
	currency -> currency_code_iso4217 = NULL;

    return TRUE;
}


/**
 * return the floating_point of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the floating_point of the currency or NULL if problem
 * */
gint gsb_data_currency_get_floating_point ( gint currency_number )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return 0;

    return currency -> currency_floating_point;
}


/**
 * set the floating_point of the currency
 *
 * \param currency_number the number of the currency
 * \param floating_point the floating_point of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_floating_point ( gint currency_number,
						gint floating_point )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency -> currency_floating_point = floating_point;

    return TRUE;
}


/**
 * return the number of the currency which has the name in param
 * create it if necessary
 *
 * \param name the name of the currency
 *
 * \return the number of the currency or 0 if doesn't exist
 * */
gint gsb_data_currency_get_number_by_name ( const gchar *name )
{
    GSList *list_tmp;

    if (!name)
	return FALSE;

    list_tmp = currency_list;

    while (list_tmp)
    {
	CurrencyStruct *currency;

	currency = list_tmp -> data;

	if (!strcmp (currency -> currency_name,
		     name ))
	    return (currency -> currency_number);
	list_tmp = list_tmp -> next;
    }

    return FALSE;
}

/**
 * return the number of the currency which has the code iso4217 in param
 * create it if necessary
 *
 * \param code the iso code of the currency
 *
 * \return the number of the currency or 0 if doesn't exist
 * */
gint gsb_data_currency_get_number_by_code_iso4217 ( const gchar *code )
{
    GSList *list_tmp;

    if (!code)
	return FALSE;

    list_tmp = currency_list;

    while (list_tmp)
    {
	CurrencyStruct *currency;

	currency = list_tmp -> data;

	if (currency -> currency_code_iso4217
	    &&
	    !strcmp (currency -> currency_code_iso4217,
		     code ))
	    return (currency -> currency_number);
	list_tmp = list_tmp -> next;
    }

    return FALSE;
}


/**
 * return the code, and if the currency has no code, return the isocode
 *
 * \param currency_number
 *
 * \return the code, isocode or NULL
 * */
gchar *gsb_data_currency_get_nickname_or_code_iso ( gint currency_number )
{
    CurrencyStruct *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    if (currency -> currency_nickname)
	return currency -> currency_nickname;

    return currency -> currency_code_iso4217;
}

/**
 * look for a currency with its code name in the iso list
 * and create it
 *
 * \param currency_name the currency code we look for
 *
 * \return the number of the new currency or FALSE if not found
 **/
gint gsb_data_currency_new_from_iso4217_list (const gchar *currency_name)
{
    Iso4217Currency *currency = iso_4217_currencies;
    gchar *tmp = g_strdup (currency_name);
    g_strchomp (tmp);

    while (currency->country_name)
    {
	if (!strcmp (currency->currency_code_iso, tmp) && currency->main_currency)
	{
	    g_free (tmp);
	    return gsb_data_currency_new_with_data (currency->currency_name,
							 currency->currency_nickname,
							 currency->currency_code_iso,
							 currency->floating_point);
	}
	currency++;
    }

    g_free (tmp);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
Iso4217Currency *gsb_data_currency_get_tab_iso_4217_currencies (void)
{
	Iso4217Currency *pointer;

	pointer = iso_4217_currencies;

	return pointer;
}

/**
 * create a new currency according to the param
 *
 * \param currency_name
 * \param currency_nickname
 * \param currency_isocode
 * \param floating_point
 *
 * \return the number of the new currency
 **/
gint gsb_data_currency_new_with_data (const gchar *currency_name,
									  const gchar *currency_nickname,
									  const gchar *currency_isocode,
									  gint floating_point)
{
    gint currency_number;

    currency_number = gsb_data_currency_new (currency_name);
    gsb_data_currency_set_nickname (currency_number, currency_nickname);
    gsb_data_currency_set_code_iso4217 (currency_number, currency_isocode);
    gsb_data_currency_set_floating_point (currency_number, floating_point);

	return currency_number;
}

/**
 * set the name of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param name the name of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_check_and_remove_duplicate (gboolean display_message)
{
	GSList *tmp_list;
	GSList *used = NULL;

	devel_debug (NULL);

	if (g_slist_length (currency_list) == 1)
	{
		return FALSE;
	}
	else
	{
		tmp_list = currency_list;
		while (tmp_list)
		{
			CurrencyStruct *tmp_currency;

			tmp_currency = tmp_list->data;

			if (g_slist_find_custom (used, tmp_currency->currency_name, (GCompareFunc)my_strcasecmp))
			{
				if (display_message)
				{
					gchar *msg;

					msg = g_strdup_printf ( _("The currency '%s' was duplicated. It has been deleted."),
										   tmp_currency->currency_name);
					dialogue_warning_hint (msg, _("Duplicate currency"));
				}

				currency_list = g_slist_remove (currency_list, tmp_currency);
				g_slist_free_full (used, (GDestroyNotify) g_free);
    			_gsb_data_currency_free (tmp_currency);

				return TRUE;
			}
			else
			{
				used = g_slist_append (used, g_strdup (tmp_currency->currency_name));
			}

			tmp_list = tmp_list->next;
		}
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
