/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_category.c
 * fuctions to deal with the categories
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_category.h"
#include "./gsb_data_category.h"
#include "./gsb_data_form.h"
#include "./gsb_form_widget.h"
#include "./gtk_combofix.h"
#include "./gsb_data_category.h"
#include "./gtk_combofix.h"
#include "./include.h"
#include "./gsb_data_form.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean gsb_category_assistant_change_choice ( GtkWidget *button, 
						GtkWidget *assistant );
/*static gboolean gsb_category_choose_default_category ( void );*/
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *window ;
/*END_EXTERN*/


/* declarations of the initial general categories */
const gchar *debit_general_category_list [] = {
    N_("Food : Bar"),
    N_("Food : Baker"),
    N_("Food : Canteen"),
    N_("Food : Sweets"),
    N_("Food : Misc"),
    N_("Food : Grocery"),
    N_("Food : Restaurant"),
    N_("Food : Self-service"),
    N_("Food : Supermarket"),
    N_("Pets : Food"),
    N_("Pets : Various supplies"),
    N_("Pets : Grooming"),
    N_("Pets : Veterinary surgeon"),
    N_("Insurance : Car"),
    N_("Insurance : Health"),
    N_("Insurance : House"),
    N_("Insurance : Civil liability"),
    N_("Insurance : Life"),
    N_("Car : Fuel"),
    N_("Car : Repairs"),
    N_("Car : Maintenance"),
    N_("Car : Parking"),
    N_("Car : Fines"),
    N_("Misc."),
    N_("Gifts"),
    N_("Children : Nurse"),
    N_("Children : Misc."),
    N_("Children : Studies"),
    N_("Studies : Lessons"),
    N_("Studies : Scool fees"),
    N_("Studies : Books"),
    N_("Miscelanious : Gifts"),
    N_("Financial expenses : Miscelanious"),
    N_("Financial expenses : Bank charges"),
    N_("Financial expenses : Loan/Mortgage"),
    N_("Financial expenses : Charges"),
    N_("Financial expenses : Refunding"),
    N_("Professionnal expenses : Non refundable"),
    N_("Professionnal expenses : Refundable"),
    N_("Taxes : Miscelanious"),
    N_("Taxes : Income"),
    N_("Taxes : Land"),
    N_("Housing : Hotel"),
    N_("Housing : Rent"),
    N_("Housing : TV"),
    N_("Housing : Furnitures"),
    N_("Housing : Charges"),
    N_("Housing : Heating"),
    N_("Housing : Decoration"),
    N_("Housing : Water"),
    N_("Housing : Electricity"),
    N_("Housing : White products"),
    N_("Housing : Equipment"),
    N_("Housing : Gaz"),
    N_("Housing : Garden"),
    N_("Housing : House keeper"),
    N_("Housing : Phone"),
    N_("Housing : Mobile Phone"),
    N_("Leisures : Snooker"),
    N_("Leisures : Bowling"),
    N_("Leisures : Movies"),
    N_("Leisures : Night club"),
    N_("Leisures : IT"),
    N_("Leisures : Games"),
    N_("Leisures : Books"),
    N_("Leisures : Parks"),
    N_("Leisures : Concerts"),
    N_("Leisures : Sports"),
    N_("Leisures : Video"),
    N_("Leisures : Travels"),
    N_("Leisures : Equipment"),
    N_("Leisures : Museums/Exhibitions"),
    N_("Loan/Mortgage : Capital"),
    N_("Health : Insurance"),
    N_("Health : Dentist"),
    N_("Health : Hospital"),
    N_("Health : Kinesitherapist"),
    N_("Health : Doctor"),
    N_("Health : Ophtalmologist"),
    N_("Health : Osteopath"),
    N_("Health : Chemist"),
    N_("Health : Social security"),
    N_("Care : Hairdresser"),
    N_("Care : Clothing"),
    N_("Transport : Bus"),
    N_("Transport : Metro"),
    N_("Transport : Toll"),
    N_("Transport : Train"),
    N_("Transport : Tramway"),
    N_("Transport : Travels"),
    N_("Transport : Train"),
    N_("Holiday : Housing"),
    N_("Holiday : Visits"),
    N_("Holiday : Travels"),
    NULL };

const gchar *credit_general_category_list [] = {
    N_("Other incomes : Unemployment benefit"),
    N_("Other incomes : Family allowance"),
    N_("Other incomes : Tax credit"),
    N_("Other incomes : Gamble"),
    N_("Other incomes : Mutual insurance"),
    N_("Other incomes : Social security"),
    N_("Retirement : Retirement Fund"),
    N_("Retirement : Pension"),
    N_("Retirement : Supplementary pension"),
    N_("Investment incomes : Dividends"),
    N_("Investment incomes : Interests"),
    N_("Investment incomes : Capital gain"),
    N_("Salary : Overtime"),
    N_("Salary : Leave allowance"),
    N_("Salary : Misc. premiums"),
    N_("Salary : Success fee"),
    N_("Salary : Net salary"),
    N_("Misc. incomes : Gifts"),
    N_("Misc. incomes : Refunds"),
    N_("Misc. incomes : Second hand sales"),
    NULL
};

const gchar *association_category_list [] = {
    N_("10. Fonds associatifs et réserves : 1020. Fonds associatifs sans droit de reprise"),
    N_("10. Fonds associatifs et réserves : 1021. Valeur du patrimoine intégré"),
    N_("10. Fonds associatifs et réserves : 1022. Fonds statutaires (à déclarer en fonction des statuts)"),
    N_("10. Fonds associatifs et réserves : 1024. Apports sans droit de reprise"),
    N_("10. Fonds associatifs et réserves : 1025. Lges et donations avec contrepartie d'actifs immobilisés"),
    N_("10. Fonds associatifs et réserves : 1026. Subventions d'investissement affectées à des biens renouvelables"),
    N_("10. Fonds associatifs et réserves : 1030. Fonfs associatifs avec droit de reprise"),
    N_("10. Fonds associatifs et réserves : 1034. Apports avec droit de reprise"),
    N_("10. Fonds associatifs et réserves : 1035. Legs et donations avec contrepartie d'actifs immobilisés assorties d'une obligation ou d'une condition"),
    N_("10. Fonds associatifs et réserves : 1036. Subventions d'investissement affectées à des biens renouvelables"),
    N_("10. Fonds associatifs et réserves : 1050. Ecarts et réévaluation"),
    N_("10. Fonds associatifs et réserves : 1051. Ecarts et réévaluations sur des biens sans droit de reprise"),
    N_("10. Fonds associatifs et réserves : 1052. Ecarts et réévaluations sur des biens avec droit de reprise"),
    N_("10. Fonds associatifs et réserves : 1060. Réserves"),
    N_("10. Fonds associatifs et réserves : 1062. Réserves indisponibles"),
    N_("10. Fonds associatifs et réserves : 1063. Réserves statutaires ou contractuelles"),
    N_("10. Fonds associatifs et réserves : 1064. Réserves réglementées"),
    N_("10. Fonds associatifs et réserves : 1068. Autres réserves (dont réserves pour projet associatif)"),
    N_("11. Eléments en instance d'affectation : 110. Report à nouveau"),
    N_("11. Eléments en instance d'affectation : 115. Résultats sous contrôle de tiers financeurs"),
    N_("12. Résultat net de l'exercice (exédent ou déficit) : 120. Résultat des l'exercice (exédent)"),
    N_("12. Résultat net de l'exercice (exédent ou déficit) : 129. Résultat de l'exercice (déficit)"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1310. Subventions d'investissements (renouvelable)"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1311. Etat"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1312. Régions"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1313. Départements"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1314. Communes"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1315. Collectivités publiques"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1316. Entreprise publiques"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1317. Entreprises et organismes privés"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1318. Autres"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1380. Autres subventions d'investissement"),
    N_("13. Subventions d'investissement affectées à des biens non renouvelables : 1390. Subventions d'investissement inscrites au compte de résultat"),
    N_("14. Provisions réglementées : 1420. Provisions réglementées relatives aux immobilisations"),
    N_("14. Provisions réglementées : 1430. Provisions réglementées relatives aux stocks"),
    N_("14. Provisions réglementées : 1424. Provisions pour investissement (participation des salariés)"),
    N_("14. Provisions réglementées : 1431. Hausse des prix"),
    N_("14. Provisions réglementées : 1432. Fluctuations des cours"),
    N_("14. Provisions réglementées : 1440. Provisions réglementées relatives aux autres éléments de l'actif"),
    N_("14. Provisions réglementées : 1450. Ammortissements dérogatoires"),
    N_("14. Provisions réglementées : 1460. Provisions spéciale de réévaluation"),
    N_("14. Provisions réglementées : 1480. Autres provisions réglementées"),
    N_("15. Provisions pour risques et charges : 1510. Provisions pour risques"),
    N_("15. Provisions pour risques et charges : 1511. Provisions pour litiges"),
    N_("15. Provisions pour risques et charges : 1514. Provisions pour amendes et pénalités"),
    N_("15. Provisions pour risques et charges : 1515. Provisions pour pertes de changes"),
    N_("15. Provisions pour risques et charges : 1516. Provisions pour risque d'emploi"),
    N_("15. Provisions pour risques et charges : 1518. Autres provisions pour risques"),
    N_("15. Provisions pour risques et charges : 1530. Provisions pour pensions et obligations similaires"),
    N_("15. Provisions pour risques et charges : 1550. Provisions pour impôts"),
    N_("15. Provisions pour risques et charges : 1570. Provisions pour charges à répartir sur plusieurs exercice"),
    N_("15. Provisions pour risques et charges : 1572. Provisions pour grosses réparations"),
    N_("15. Provisions pour risques et charges : 1580. Autres provisions pour charges"),
    N_("15. Provisions pour risques et charges : 1582. Provisions pour charges sociales et fiscales sur congés payer"),
    N_("16. Emprunt et dettes assimilées : 1640. Emprunts auprès des établissements de crédit"),
    N_("16. Emprunt et dettes assimilées : 1641. Emprunt (à détailler)"),
    N_("16. Emprunt et dettes assimilées : 1650. Dépôts et cautionnements reçus"),
    N_("16. Emprunt et dettes assimilées : 1651. Dépôts"),
    N_("16. Emprunt et dettes assimilées : 1655. Cautionnements"),
    N_("16. Emprunt et dettes assimilées : 1660. Participation des salariés aux résultats"),
    N_("16. Emprunt et dettes assimilées : 1661. Comptes bloqués"),
    N_("16. Emprunt et dettes assimilées : 1662. Fonds de participation"),
    N_("16. Emprunt et dettes assimilées : 1670. Emprunts et dettes assorties de conditions particulières"),
    N_("16. Emprunt et dettes assimilées : 1672. Titre associatif"),
    N_("16. Emprunt et dettes assimilées : 1680. Autres emprunts et dettes assimilées"),
    N_("16. Emprunt et dettes assimilées : 1681. Autres emprunts (à détailler)"),
    N_("16. Emprunt et dettes assimilées : 1685. Rents viagères capitalisées"),
    N_("16. Emprunt et dettes assimilées : 1687. Autres dettes (à détailler)"),
    N_("16. Emprunt et dettes assimilées : 1688. Intérêts courus ( à détailler)"),
    N_("18. Comptes de liaisons des établissements (avec le siège social ou entre eux) : 1810. Apports permanents entre siège social et établissement"),
    N_("18. Comptes de liaisons des établissements (avec le siège social ou entre eux) : 1850. Biens et prestations de services échangés entre établissements et le siège social"),
    N_("18. Comptes de liaisons des établissements (avec le siège social ou entre eux) : 1860. Biens et prestations de services échangés entre établissement (charges)"),
    N_("18. Comptes de liaisons des établissements (avec le siège social ou entre eux) : 1670. Biens et prestations de services échangés entre établissements (produits)"),
    N_("19. Fonds dédiés : 1940. Fonds dédiés sur subventions de fonctionnement"),
    N_("19. Fonds dédiés : 1950. Fonds dédiés sur dons manuels affectés"),
    N_("19. Fonds dédiés : 1970. Fons dédiés sur legs et dons affectés"),
    N_("20. Immobilisations incorporelles : 2010. Frais d'établissement"),
    N_("20. Immobilisations incorporelles : 2012. Frais de premier établissement"),
    N_("20. Immobilisations incorporelles : 2060. Droit au bail"),
    N_("20. Immobilisations incorporelles : 2080. Autres immobilisations incorporelles"),
    N_("21. Immoblilisations corporelles : 2110. Terrains"),
    N_("21. Immoblilisations corporelles : 2120. Agencements et aménagements de terrains"),
    N_("21. Immoblilisations corporelles : 2130. Constructions"),
    N_("21. Immoblilisations corporelles : 2131. Bâtiments"),
    N_("21. Immoblilisations corporelles : 2135. Installations générales, agencements, aménagements des constructions"),
    N_("21. Immoblilisations corporelles : 2140. Constructions sur sol d'autrui"),
    N_("21. Immoblilisations corporelles : 2150. Installations techniques, matériels et outillages industriels"),
    N_("21. Immoblilisations corporelles : 2151. Installations complexes spécialisées"),
    N_("21. Immoblilisations corporelles : 2154. Matériels industriel"),
    N_("21. Immoblilisations corporelles : 2155. Outillages industriel"),
    N_("21. Immoblilisations corporelles : 2180. Autres immobilisations incorporelles"),
    N_("21. Immoblilisations corporelles : 2181. Installations générales, agencements, aménagements divers"),
    N_("21. Immoblilisations corporelles : 2182. Matériels de transport"),
    N_("21. Immoblilisations corporelles : 2183. Matériels de bureau et matériels informatique"),
    N_("21. Immoblilisations corporelles : 2184. Mobilier"),
    N_("21. Immoblilisations corporelles : 2185. Cheptel"),
    N_("22. Immobilisations mises en concession : 2280. Immobilisations grevées de droits"),
    N_("22. Immobilisations mises en concession : 2290. Droits des propriétaires"),
    N_("23. Immobilisations en cours : 2310.Immobilisations corporelles en cours"),
    N_("23. Immobilisations en cours : 2312. Terrain"),
    N_("23. Immobilisations en cours : 2313. Constructions"),
    N_("23. Immobilisations en cours : 2315. Installations techniques, matériels et outillages industriels"),
    N_("23. Immobilisations en cours : 2318. Autres immobilisations corporelles"),
    N_("23. Immobilisations en cours : 2320. Immobilisations incorporelles en cours"),
    N_("23. Immobilisations en cours : 2380. Avances et acomptes versés sur commandes d'immobilisations corporelles"),
    N_("26. Participations et créances rattachées à des participations : 2610. Titres de participations"),
    N_("26. Participations et créances rattachées à des participations : 2660. Autres formes de participation"),
    N_("26. Participations et créances rattachées à des participations : 2670. Créances rattachées à des particiations"),
    N_("26. Participations et créances rattachées à des participations : 2690. Versements restant à effectuer sur titres de participation non libérés"),
    N_("27. Autres immobilisations financières : 2710. Titres immobilisés (droit de propiété)"),
    N_("27. Autres immobilisations financières : 2711. Actions"),
    N_("27. Autres immobilisations financières : 2720. Titres immobilisés (droit decréance)"),
    N_("27. Autres immobilisations financières : 2721. Obligations"),
    N_("27. Autres immobilisations financières : 2722. Bons"),
    N_("27. Autres immobilisations financières : 2728. Autres"),
    N_("27. Autres immobilisations financières : 2730. Titres immobilisés de l'activité de portefeuille"),
    N_("27. Autres immobilisations financières : 2740. Prêts"),
    N_("27. Autres immobilisations financières : 2743. Prêts au personnel"),
    N_("27. Autres immobilisations financières : 2748. Autres prêts"),
    N_("27. Autres immobilisations financières : 2750. Dépôts et cautionnements versés"),
    N_("27. Autres immobilisations financières : 2751. Dépots"),
    N_("27. Autres immobilisations financières : 2755. Cautionnements"),
    N_("27. Autres immobilisations financières : 2760. AUtres créances immobilisées"),
    N_("27. Autres immobilisations financières : 2761. Créances divers"),
    N_("27. Autres immobilisations financières : 2768. Intérêts courus (à détailler)"),
    N_("27. Autres immobilisations financières : 2790. Versements restant à effectuer sur titres immobilisés non libérés"),
    N_("28. Ammortissements des immobiloisations : 2800. Ammortissements des immobilisations incorporelles"),
    N_("28. Ammortissements des immobiloisations : 2801. Frais d'établissement (même ventilation que celle du compte 201)"),
    N_("28. Ammortissements des immobiloisations : 2808. Autres immobilisations incorporelles"),
    N_("28. Ammortissements des immobiloisations : 2810. Ammortissements des immobilisations corporelles"),
    N_("28. Ammortissements des immobiloisations : 2812. Agencememnts, aménagements des terrains (même ventilations que celle du compte 212)"),
    N_("28. Ammortissements des immobiloisations : 2813. Constructions (même ventilation que celle du compte 213)"),
    N_("28. Ammortissements des immobiloisations : 2814. Constructions sur sol d'autrui (même ventilations que celles du comptes 214)"),
    N_("28. Ammortissements des immobiloisations : 2815. Installtions techniques, matériels et outillages industriels (même ventilation que celle du compte 215)"),
    N_("28. Ammortissements des immobiloisations : 2818. Autres immobilisations corporelles (même ventilation que celle du compte 218)"),
    N_("29. Provision pour dépréciation des immobilisation : 2900. Provisions pour dépréciations des immobilisations incorporelles"),
    N_("29. Provision pour dépréciation des immobilisation : 2906. Droit au bail"),
    N_("29. Provision pour dépréciation des immobilisation : 2908. Autres immobilisations incorporelles"),
    N_("29. Provision pour dépréciation des immobilisation : 2910. Provisions pour dépréciation des immobilisations corporelles"),
    N_("29. Provision pour dépréciation des immobilisation : 2911. Terrain"),
    N_("29. Provision pour dépréciation des immobilisation : 2960. Provisions pour dépréciations des participations et créances rattachées à des participations"),
    N_("29. Provision pour dépréciation des immobilisation : 2961. Tires de participations"),
    N_("29. Provision pour dépréciation des immobilisation : 2966. Autres formes de participations"),
    N_("29. Provision pour dépréciation des immobilisation : 2967. Créances rattachées à des participations (même ventilations que celles du compte 267)"),
    N_("29. Provision pour dépréciation des immobilisation : 2970. Provisions pour dépréciations des autres immobilisations financières"),
    N_("29. Provision pour dépréciation des immobilisation : 2971. Tires immobilisés (droit de propriété) (même ventilation que celle du compte 271)"),
    N_("29. Provision pour dépréciation des immobilisation : 2972. Titres immobilisés (droit de créances) ( même ventilations que celle compte 272)"),
    N_("29. Provision pour dépréciation des immobilisation : 2974. Prêts (même ventilation que celle du compte 274)"),
    N_("29. Provision pour dépréciation des immobilisation : 2975. Dépôts et cautionnements versés (même ventilation que celle du compte 275)"),
    N_("29. Provision pour dépréciation des immobilisation : 2976. Autres créances immobilisées ( même ventilation que celle du compte 276)"),
    N_("31. Matières premières et fournitures : 3110. Matières premières"),
    N_("31. Matières premières et fournitures : 3170. Fournitures"),
    N_("32. Autres approvisionnements : 3210. Matières consommables"),
    N_("32. Autres approvisionnements : 3220. Fournitures consommables"),
    N_("32. Autres approvisionnements : 3260. Emballages"),
    N_("33. En-cours de production de biens : 3310. Produit en cours"),
    N_("33. En-cours de production de biens : 3350. Travaux en cours"),
    N_("34. En-cours de production de services : 3410. Produit en cours"),
    N_("34. En-cours de production de services : 3450. Prestations de servevices en cours"),
    N_("35. Stocks de produits : 3510. Produits intermédiaires"),
    N_("35. Stocks de produits : 3550. Produit finis"),
    N_("39. Provisions pour dépréciations des stocks et en-cours : 3910. Provision pour dépréciations des matiéres premières et fournitures"),
    N_("39. Provisions pour dépréciations des stocks et en-cours : 3920. Provision pour dépréciation  des autres approvisionnement"),
    N_("39. Provisions pour dépréciations des stocks et en-cours : 3930. Provision pour dépréciation des en-cours de production de biens"),
    N_("39. Provisions pour dépréciations des stocks et en-cours : 3940. Provision pour dépréciation des en-cours de production de services"),
    N_("39. Provisions pour dépréciations des stocks et en-cours : 3950. Provision pour dépréciation des stocks de produits"),
    N_("39. Provisions pour dépréciations des stocks et en-cours : 3970. Provision pour dépréciation des stocks de marchandises"),
    N_("40. Fournisseurs et comptes rattachés : 4010. Fournisseurs"),
    N_("40. Fournisseurs et comptes rattachés : 4011. Forunisseurs. Achats de biens ou de prestations de services"),
    N_("40. Fournisseurs et comptes rattachés : 4040. Fournisseurs d'immobilisations"),
    N_("40. Fournisseurs et comptes rattachés : 4041. Fournisseurs. Achats d'immobilisations"),
    N_("40. Fournisseurs et comptes rattachés : 4047. Fournisseurs d'immobilisations. Retenues de garantie"),
    N_("40. Fournisseurs et comptes rattachés : 4080. Fournisseurs. Factures non parvenues"),
    N_("40. Fournisseurs et comptes rattachés : 4081. Fournisseurs. Achats de biens ou prestations de services"),
    N_("40. Fournisseurs et comptes rattachés : 4084. Fournisseurs. Achats d'immobilisations"),
    N_("40. Fournisseurs et comptes rattachés : 4090. Fournisseurs débiteurs"),
    N_("40. Fournisseurs et comptes rattachés : 4091. Fournisseurs. Avances et acomptes versés sur commandes"),
    N_("40. Fournisseurs et comptes rattachés : 4096. Fournisserus. Créances pour embalage et matériel à rendre"),
    N_("41. Usagers et comptes rattachés : 4110. Usagers ( et organismes de prise en charges)"),
    N_("41. Usagers et comptes rattachés : 4116. Créances douteuses ou litigieuses"),
    N_("41. Usagers et comptes rattachés : 4118. Usagers. Produitd non encore facturés"),
    N_("41. Usagers et comptes rattachés : 4119. Usagers créditeurs"),
    N_("42. Personnel et comptes rattachés : 4210. Personnel. Rémunération dues"),
    N_("42. Personnel et comptes rattachés : 4220. Comités d'entreprise, d'établissement"),
    N_("42. Personnel et comptes rattachés : 4250. Personnel. Avances et acomptes"),
    N_("42. Personnel et comptes rattachés : 4270. Personnel. Opposition"),
    N_("42. Personnel et comptes rattachés : 4280. Personnel. Charges à payer et produits à recevoir"),
    N_("42. Personnel et comptes rattachés : 4282. Dettes provisionnées pour congés à payer"),
    N_("42. Personnel et comptes rattachés : 4286. Autres charges à payer"),
    N_("42. Personnel et comptes rattachés : 4287. Produit à recevoir"),
    N_("43 Sécurité social et autres organismes sociaux : 4310. Sécurité sociale"),
    N_("43 Sécurité social et autres organismes sociaux : 4370. Autres organismes sociaux"),
    N_("43 Sécurité social et autres organismes sociaux : 4372. Mutuelles"),
    N_("43 Sécurité social et autres organismes sociaux : 4373. Caisses de retraite et de prévoyance"),
    N_("43 Sécurité social et autres organismes sociaux : 4374. Caisse d'allocation chômage. ASSEDIC"),
    N_("43 Sécurité social et autres organismes sociaux : 4378. Autres organismes sociaux. Divers"),
    N_("43 Sécurité social et autres organismes sociaux : 4380. Organismes sociaux. Charges à payer et produits à recevoir"),
    N_("43 Sécurité social et autres organismes sociaux : 4382. Charges sociales sur congés à payer"),
    N_("43 Sécurité social et autres organismes sociaux : 4386. Autres charges à payer"),
    N_("43 Sécurité social et autres organismes sociaux : 4387. Produits à recevoir"),
    N_("44. Etat et autres collectivités publiques : 4410. Etat. Subvention à recevoir"),
    N_("44. Etat et autres collectivités publiques : 4411. Subventions d'investissement"),
    N_("44. Etat et autres collectivités publiques : 4417. Subvention d'exploitation"),
    N_("44. Etat et autres collectivités publiques : 4419. Avances sur subventions"),
    N_("44. Etat et autres collectivités publiques : 4430. Opérations particulières avec l'état, les collectivités publiques, les organismes internationnaux"),
    N_("44. Etat et autres collectivités publiques : 4440. Etat. Impôts sur les bénéfices"),
    N_("44. Etat et autres collectivités publiques : 4445. Etat. Impôts sur les sociétés (organismes sans but lucratif)"),
    N_("44. Etat et autres collectivités publiques : 4450. Etat. taxes sur le chiffre d'affaires"),
    N_("44. Etat et autres collectivités publiques : 4452. TVA due intercommunautaire"),
    N_("44. Etat et autres collectivités publiques : 4455. Taxes sur le chiffre d'affaires à décaisser"),
    N_("44. Etat et autres collectivités publiques : 4456. Taxes sur le chiffres d'affaires déductible"),
    N_("44. Etat et autres collectivités publiques : 44562. TVA sur immobilisation"),
    N_("44. Etat et autres collectivités publiques : 44566. TVA sur autres biens et services"),
    N_("44. Etat et autres collectivités publiques : 44567. Crédit de TVA à reporter"),
    N_("44. Etat et autres collectivités publiques : 4457. Taxes sur le chiffre d'affaires collectés par l'association"),
    N_("44. Etat et autres collectivités publiques : 4458. Taxes sur le chiffre d'affaires à régulariser ou en attente"),
    N_("44. Etat et autres collectivités publiques : 44586. Taxes sur le chiffre d'affaires sur factures non parvenues"),
    N_("44. Etat et autres collectivités publiques : 44587. Taxes sur le chiffre d'affaires sur factures à établir"),
    N_("44. Etat et autres collectivités publiques : 4470. Autres impôts et taxes et versements assimilés"),
    N_("44. Etat et autres collectivités publiques : 4471. Impôts, taxes et versements assimilés sur émunérations ( administration des impôts)"),
    N_("44. Etat et autres collectivités publiques : 44711. Taxes sur les salaires"),
    N_("44. Etat et autres collectivités publiques : 44713. Participation des employeurs à la formation professionnelle continue"),
    N_("44. Etat et autres collectivités publiques : 44714. Cotisation pour défaut d'investissement dans la construction"),
    N_("44. Etat et autres collectivités publiques : 44718. Autres impôts, taxes et versements assimilés"),
    N_("44. Etat et autres collectivités publiques : 4473. Impôts, taxes et versements assimilés sur rémunérations (autres organisme)"),
    N_("44. Etat et autres collectivités publiques : 44733. Participation des employeurs à la formation professionnelle continue"),
    N_("44. Etat et autres collectivités publiques : 44734. Participation des employeurs à l'effort de construction (versement à fond perdu)"),
    N_("44. Etat et autres collectivités publiques : 4475. Autres impôts, taxes et versements assimilés (administration des impôts)"),
    N_("44. Etat et autres collectivités publiques : 4477. AUtres impôts, taxes et versements assimilés (autres organismes)"),
    N_("44. Etat et autres collectivités publiques : 4480. Etat. Charges à payer et produits à recevoir"),
    N_("44. Etat et autres collectivités publiques : 4482. Charges fiscales sur congés à payer"),
    N_("44. Etat et autres collectivités publiques : 4486. Autres charges à payer"),
    N_("44. Etat et autres collectivités publiques : 4487. Produits à recevoir"),
    N_("45. Confédération, fédération, union, association affiliées : 4510. Confédération, fédération, associations affiliées. Compte courant"),
    N_("45. Confédération, fédération, union, association affiliées : 4550. Sociétaires. Comptes courants"),
    N_("46. Débiteurs divers et créditeurs divers : 4670. Autres comptes débiteurs ou créditeurs"),
    N_("46. Débiteurs divers et créditeurs divers : 4680. Divers. Charges à payer et produits à recevoir"),
    N_("46. Débiteurs divers et créditeurs divers : 4686. Charges à payer"),
    N_("46. Débiteurs divers et créditeurs divers : 4687. Produits à recevoir"),
    N_("47. Comptes d'attente : 4710. Recettes à classer"),
    N_("47. Comptes d'attente : 4720. Dépences à classer et régulariser"),
    N_("47. Comptes d'attente : 4750. Legs et donnations en cours de réalisation"),
    N_("48. Comptes de régularisation : 4810. Charges à répartir sur plusieurs exercices"),
    N_("48. Comptes de régularisation : 4812. Frais d'acquisition des immobilisation"),
    N_("48. Comptes de régularisation : 4818. Charges à étaler"),
    N_("48. Comptes de régularisation : 4860. Charges constaté d'avance"),
    N_("48. Comptes de régularisation : 4870. Produits constatés d'avance"),
    N_("49. Provision pour dépréciation des comptes de tiers : 4910. Provision pour dépréciation des comptes d'usagers (et organisme de prise en charge)"),
    N_("49. Provision pour dépréciation des comptes de tiers : 4960. Provision pour dépréciation des comptes de débiteurs divers"),
    N_("50. Valeurs mobilières de placement : 5030. Actions"),
    N_("50. Valeurs mobilières de placement : 5031. Titres cotés"),
    N_("50. Valeurs mobilières de placement : 5035. Titres non cotés"),
    N_("50. Valeurs mobilières de placement : 5060. Obligations"),
    N_("50. Valeurs mobilières de placement : 5061. Titres cotés"),
    N_("50. Valeurs mobilières de placement : 5065. Titres non cotés"),
    N_("50. Valeurs mobilières de placement : 5070. Bons du trésor et bons de caisse à court terme"),
    N_("50. Valeurs mobilières de placement : 5080. Autres valeurs mobilières et créances assimilées"),
    N_("50. Valeurs mobilières de placement : 5088. Intérêts courus sur obligations, bons et valeurs assimilés"),
    N_("51. Banques, établissements financiers et assimilés : 5110. Valeurs à l'encaissement"),
    N_("51. Banques, établissements financiers et assimilés : 5120. Banques"),
    N_("51. Banques, établissements financiers et assimilés : 5130. Caisse des dépôts et consignation"),
    N_("51. Banques, établissements financiers et assimilés : 5140. Chéques postaux"),
    N_("51. Banques, établissements financiers et assimilés : 5150. Caisse du trésor et des établissement publics"),
    N_("51. Banques, établissements financiers et assimilés : 5170. Autres organismes financiers"),
    N_("51. Banques, établissements financiers et assimilés : 5171. Caisse d'épargne"),
    N_("51. Banques, établissements financiers et assimilés : 5180. Intérêts courus"),
    N_("51. Banques, établissements financiers et assimilés : 5186. Intérêts couris à payer"),
    N_("51. Banques, établissements financiers et assimilés : 5187. Intérêts courus à recevoir"),
    N_("51. Banques, établissements financiers et assimilés : 5190. Concours bancaires courants"),
    N_("51. Banques, établissements financiers et assimilés : 5198. Intérêts courus sur concours bancaires courants"),
    N_("53. Caisse : 5310. Caisse du siège"),
    N_("53. Caisse : 5320. Caisse des leiux d'activités"),
    N_("54 Régies d'avances et accréditifs : 5410. Régies d'avances"),
    N_("54 Régies d'avances et accréditifs : 5420. Accréditifs"),
    N_("58.Virements internes : 5810. Virements de fonds"),
    N_("59. Provision pour dépréciation des comptes financiers : 5900. Provision pour dépréciation des valeurs mobilières de placement"),
    N_("60. Achat (sauf 603) : 6010. Achats stockés. Matières premières et fournitures"),
    N_("60. Achat (sauf 603) : 6020. Achats stockés. Autres approvisionnements"),
    N_("60. Achat (sauf 603) : 6021. Matières consommables"),
    N_("60. Achat (sauf 603) : 6022. Fournitures consommables"),
    N_("60. Achat (sauf 603) : 60221. Combustibles"),
    N_("60. Achat (sauf 603) : 60222. Produits d'entretien"),
    N_("60. Achat (sauf 603) : 60223. Fournitures d'atelier"),
    N_("60. Achat (sauf 603) : 60224. Fournitures de magasin"),
    N_("60. Achat (sauf 603) : 60225. Fournitures de bureau"),
    N_("60. Achat (sauf 603) : 6030. Variations de stocks (approvisionnements et marchandises)"),
    N_("60. Achat (sauf 603) : 6031. Variations de stocks de matières premières et fournitures"),
    N_("60. Achat (sauf 603) : 6032. Variations de stocks des autres approvisionnements"),
    N_("60. Achat (sauf 603) : 6037. Variations des stocks de marchandises"),
    N_("60. Achat (sauf 603) : 6040. Achats d'études et de prestation de services"),
    N_("60. Achat (sauf 603) : 6060. Achats non stockés de matières et fournitures"),
    N_("60. Achat (sauf 603) : 6061. Fournitures non stockables (eau, électricité...)"),
    N_("60. Achat (sauf 603) : 6063. Fournitures d'entretiens et de petit équipement"),
    N_("60. Achat (sauf 603) : 6064. Fournitures administrative"),
    N_("60. Achat (sauf 603) : 6068. Autres matières et fournitures"),
    N_("60. Achat (sauf 603) : 6070. Achats de marchandises"),
    N_("60. Achat (sauf 603) : 6071. MArchandises  A"),
    N_("60. Achat (sauf 603) : 6072. Marchandise B"),
    N_("60. Achat (sauf 603) : 6090. Rabais, remises et ristournes obtenus sur achats"),
    N_("61. Service exterieurs : 6110. Sous-traitance général"),
    N_("61. Service exterieurs : 6120. Redevances de crédit-bail"),
    N_("61. Service exterieurs : 6122. Crédit-bail mobilier"),
    N_("61. Service exterieurs : 6125. Crédit-bail immobilier"),
    N_("61. Service exterieurs : 6130. Locations"),
    N_("61. Service exterieurs : 6132. Locations immobilières"),
    N_("61. Service exterieurs : 6135. Locations mobilières"),
    N_("61. Service exterieurs : 6140. Charges locatives et de copropriétè"),
    N_("61. Service exterieurs : 6150. Entretiens et réparations"),
    N_("61. Service exterieurs : 6152. SUr biens immobiliers"),
    N_("61. Service exterieurs : 6155. Sur biens mobiliers"),
    N_("61. Service exterieurs : 6156. Maintenance"),
    N_("61. Service exterieurs : 6160. Primes d'assurance"),
    N_("61. Service exterieurs : 6161. Multirisque"),
    N_("61. Service exterieurs : 6162. Assurance obligatoire dommage-construction"),
    N_("61. Service exterieurs : 6168. Autres assurances"),
    N_("61. Service exterieurs : 6170. Etudes et recherches"),
    N_("61. Service exterieurs : 6180 Divers"),
    N_("61. Service exterieurs : 6181. Docummentations générale"),
    N_("61. Service exterieurs : 6183. Documantations technique"),
    N_("61. Service exterieurs : 6185. Frais de colloques, séminaires, conférences"),
    N_("61. Service exterieurs : 6190. Rabais, remises, ristournes obtenus sur services exterieurs"),
    N_("62. Autres services exterieurs : 6210. Personnel exterieur à l'association"),
    N_("62. Autres services exterieurs : 6211. Personnel intérimaire"),
    N_("62. Autres services exterieurs : 6214. Personnel détaché ou prêté à l'association"),
    N_("62. Autres services exterieurs : 6220. Rémunérations d'intérimaires et honoraires"),
    N_("62. Autres services exterieurs : 6226. Honoraires"),
    N_("62. Autres services exterieurs : 6227. Frais d'actes et de contencieux"),
    N_("62. Autres services exterieurs : 6230. Publicité, publication, relations publiques"),
    N_("62. Autres services exterieurs : 6231. Annonces et insertions"),
    N_("62. Autres services exterieurs : 6233. Foires et expositions"),
    N_("62. Autres services exterieurs : 6236. Catalogues et imprimés"),
    N_("62. Autres services exterieurs : 6237. Publications"),
    N_("62. Autres services exterieurs : 6238. Divers (pourboires, dons courant...)"),
    N_("62. Autres services exterieurs : 6240. Trasports de biens et transports colletifs de personnels"),
    N_("62. Autres services exterieurs : 6241. Transports sur achats"),
    N_("62. Autres services exterieurs : 6242. Transports sur ventes"),
    N_("62. Autres services exterieurs : 6243. Transports entre établissements"),
    N_("62. Autres services exterieurs : 6248. Divers"),
    N_("62. Autres services exterieurs : 6250. Déplacements, missions et réceptions"),
    N_("62. Autres services exterieurs : 6251. Voyages et déplacements"),
    N_("62. Autres services exterieurs : 6256. Missions"),
    N_("62. Autres services exterieurs : 6257. Réceptions"),
    N_("62. Autres services exterieurs : 6260. Frais postaux et frais de télécommunications"),
    N_("62. Autres services exterieurs : 6261. Liaisons informatiques ou spécialisées"),
    N_("62. Autres services exterieurs : 6263. Affranchissements"),
    N_("62. Autres services exterieurs : 6265. Téléphone"),
    N_("62. Autres services exterieurs : 6270. Services banquaires et assimilés"),
    N_("62. Autres services exterieurs : 6280. Divers"),
    N_("62. Autres services exterieurs : 6281. Cotisations (liées à l'activités économique)"),
    N_("62. Autres services exterieurs : 6284. Frais de recrutement du personnel"),
    N_("62. Autres services exterieurs : 6290. Rabais, remies, ristournes obtenus sur autres services exterieurs"),
    N_("63. Impôts, taxes et versements assimilés : 6310. Impôts, taxes et versements assimilées sur rémunérations (administration des impôts)"),
    N_("63. Impôts, taxes et versements assimilés : 6311. Taxes sur salaires"),
    N_("63. Impôts, taxes et versements assimilés : 6312. Taxe d'apprentissage"),
    N_("63. Impôts, taxes et versements assimilés : 6313. Participation des employeurs à la formation professionelle continue"),
    N_("63. Impôts, taxes et versements assimilés : 6314. Cotisation pour défaut d'investissement obligatoire dans la construction"),
    N_("63. Impôts, taxes et versements assimilés : 6330. Impôts, taxes et versements assimilés sur rémunérations ( autres organismes)"),
    N_("63. Impôts, taxes et versements assimilés : 6331. Versements de transports"),
    N_("63. Impôts, taxes et versements assimilés : 6333. Participations des employeurs à la formations professionnellescontinue"),
    N_("63. Impôts, taxes et versements assimilés : 6334. Participation des employeurs à l'effort de construction (versement à fonds perdu)"),
    N_("63. Impôts, taxes et versements assimilés : 6335. Versements libératoire donnant droit à l'exonération de la taxe d'apprentissage"),
    N_("63. Impôts, taxes et versements assimilés : 6350. Autres impôts, taxes et versements assimilés (administrations des impôts)"),
    N_("63. Impôts, taxes et versements assimilés : 6351. Impôts directs (sauf impôt sur les bénéfices)"),
    N_("63. Impôts, taxes et versements assimilés : 63512. Taxes foncières"),
    N_("63. Impôts, taxes et versements assimilés : 62413. Autres impôts locaux"),
    N_("63. Impôts, taxes et versements assimilés : 63518.Autres impôts directs"),
    N_("63. Impôts, taxes et versements assimilés : 6352. Taxes sur les chiffres d'affaires non récupérables"),
    N_("63. Impôts, taxes et versements assimilés : 6353. Impôts indirects"),
    N_("63. Impôts, taxes et versements assimilés : 6354. Droits d'enregistrement et de timbre"),
    N_("63. Impôts, taxes et versements assimilés : 6358. Autres droits"),
    N_("63. Impôts, taxes et versements assimilés : 6370. Autres impôts, taxes et versements assimilés (autres organismes)"),
    N_("64. Charges de personnel : 6410. Rémunérations du personnel"),
    N_("64. Charges de personnel : 6411. Salaires et appointements"),
    N_("64. Charges de personnel : 6412. Congés payés"),
    N_("64. Charges de personnel : 6413. Primes et gratifications"),
    N_("64. Charges de personnel : 6414. Indemnités et avantages divers"),
    N_("64. Charges de personnel : 6415. Suppléments familial"),
    N_("64. Charges de personnel : 6450. Charges de sécurité sociale et de prévoyance"),
    N_("64. Charges de personnel : 6451. Cotisations de l'URSSAF"),
    N_("64. Charges de personnel : 6452. Cotisations aux mutuelles"),
    N_("64. Charges de personnel : 6453. Cotisations aux caisses de retraite et de prévoyance"),
    N_("64. Charges de personnel : 6454. Cotisations aux ASSEDIC"),
    N_("64. Charges de personnel : 6458. Cotisations aux autres organismes sociaux"),
    N_("64. Charges de personnel : 6470. Autres charges sociales"),
    N_("64. Charges de personnel : 6472. Versements aux comités d'entreprise et d'établissement"),
    N_("64. Charges de personnel : 6475. Médecine du travail, pharmacie"),
    N_("64. Charges de personnel : 6480. Autres charges de personnel"),
    N_("65. Autres charges de gestions courante : 6510. Redevances pour concessions, brevet, licences, marques, procédés"),
    N_("65. Autres charges de gestions courante : 6516. Droits d'auteur et de reproduction (SACEM)"),
    N_("65. Autres charges de gestions courante : 6518. Autres droits et valeurs similaires"),
    N_("65. Autres charges de gestions courante : 6540. Pertes sur créances irrécouvrables"),
    N_("65. Autres charges de gestions courante : 6541. Créances de l'exercice"),
    N_("65. Autres charges de gestions courante : 6544. Créances des exercices antérieurs"),
    N_("65. Autres charges de gestions courante : 6550. Qoute-part de résultat sur opération faites en commun"),
    N_("65. Autres charges de gestions courante : 6570. Subventions versées par l'association"),
    N_("65. Autres charges de gestions courante : 6571. Bourses accordées aux usagers"),
    N_("65. Autres charges de gestions courante : 6580. Charges divers de gestion courante"),
    N_("65. Autres charges de gestions courante : 6586. Cotisations (liées à la vie statutaire)"),
    N_("66. Charges financières : 6610. Charges d'intérêts"),
    N_("66. Charges financières : 6611. Intérêts des emprunts et dettes"),
    N_("66. Charges financières : 6616. Intérêts banquaires"),
    N_("66. Charges financières : 6618. Intérêts des autres dettes"),
    N_("66. Charges financières : 6660. Pertes de change"),
    N_("66. Charges financières : 6670. Charges nettes sur cessions de valeurs immobilières de placement"),
    N_("66. Charges financières : 6680. Autres chages financières"),
    N_("67. Charges exeptionnelles : 6710. Charges exeptionnelles sur opérations de gestion"),
    N_("67. Charges exeptionnelles : 6712. Pénalités et amendes fiscales ou pénales"),
    N_("67. Charges exeptionnelles : 6713. Dons, libéralités"),
    N_("67. Charges exeptionnelles : 6714. Créances devenues irrécouvrables dans l'exercice"),
    N_("67. Charges exeptionnelles : 6717. Rappel d'impôts (autres qu'impôts sur les bénéfices)"),
    N_("67. Charges exeptionnelles : 6718. Autres charges exeptionnelles sur opérations de gestion"),
    N_("67. Charges exeptionnelles : 6720. Charges sur exercices antérieurs ( à reclasser)"),
    N_("67. Charges exeptionnelles : 6750. Valeurs comptables des éléments d'actif cédés"),
    N_("67. Charges exeptionnelles : 6751. Immobilisations incorporelles"),
    N_("67. Charges exeptionnelles : 6752. Immobilisations corporelles"),
    N_("67. Charges exeptionnelles : 6756. Immobilisations financières"),
    N_("67. Charges exeptionnelles : 6780. Autres charges exeptionnelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6810. Dotations aux ammortissements et aux provisions. Charges d'exploitation"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6811. Dotations aux ammortissements des immobilisations incorporelles et corporelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 68111. Immobilisations incorporelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 68112. Immobilisations corporelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6812. Dotations aux ammortissements des charges d'exploitation à répartir"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6815. Dotations aux provisions, pour risque et charges d'exploitation"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6816. Dotationsaux provisions pour dépréciation des immobilisations incorporelles et corporelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 68161. Immobilisations incorporelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 68162. Immobilisations corporelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6817. Dotations aux provisions pour dépréciation des actifs circulants"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6860. Dotations aux ammortissements et aux provisions. Charges financières"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6866. Dotation aux provisions pour dépréciations des éléments financiers"),
    N_("68. Dotations aux ammortissements, provision et engagements : 68662. Immobilisations financières"),
    N_("68. Dotations aux ammortissements, provision et engagements : 68665. Valeurs mobilières de placement"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6870. Dotations aux ammortissements et aux provisions. Charges exeptionnelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6871. Dotations aux ammortissements exceptionnelles des immobilisations"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6876. Dotations aux ammortissements pour dépréciation exceptionnelles"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6890. Engagements à réaliser sur ressources affectées"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6894. Engagements à réaliser sur subventions attribuées"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6895. Engagements à réaliser sur dons manuels affectés"),
    N_("68. Dotations aux ammortissements, provision et engagements : 6897. Engagements à réaliser sur legs et donations affectés"),
    N_("69. Impôts sur les bénéfices : 6950. Impôts sur les bénéfices"),
    N_("69. Impôts sur les bénéfices : 6910. Participations des salariès aux résultats"),
    N_("69. Impôts sur les bénéfices : 69511. Impôts sur les sociétés des personnes morales non lucratives"),
    N_("69. Impôts sur les bénéfices : 69512. Impôts sur les sociétés de droit commun"),
    N_("69. Impôts sur les bénéfices : 6952. Contributions additionnelles à l'impôts sur les bénéfices"),
    N_("69. Impôts sur les bénéfices : 6970. Imposition forfaitaire et annuelle (IFA)"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7010. Ventes de produits finis"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7050. Etudes"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7060. Prestations de services"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7070. Ventes de marchandises"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7080. Produits des activités annexes"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7081. Produits des prestations fournies au personnel"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7083. Locations divers"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7084. Mise à disposition de personnel facturée"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7088. Autres produits d'activités annexes"),
    N_("70. Ventes de produits finis, prestations de services, marchandises : 7090. Rabais, remises et ristournes accordées par l'association"),
    N_("71. Production stockée (ou destockage) : 7130. Variations de stock (en-cours de production, produits)"),
    N_("71. Production stockée (ou destockage) : 7133. Variations des en-cours de production de biens"),
    N_("71. Production stockée (ou destockage) : 7134. Variations des en-cours de productions de services"),
    N_("71. Production stockée (ou destockage) : 7135. Variations des stocks de produits"),
    N_("75. Autres produits de gestions courante : 7510. Redevances pour concessions, brevets, licences, marques, procédés, droits et valeurs similaires"),
    N_("75. Autres produits de gestions courante : 7520. Revenues des immeubles non affectés aux activités de l'association"),
    N_("75. Autres produits de gestions courante : 7540. Collectes"),
    N_("75. Autres produits de gestions courante : 7560. cotisations"),
    N_("75. Autres produits de gestions courante : 7570. Quote-part d'éléments du fond associatif virée au compte de résultat"),
    N_("75. Autres produits de gestions courante : 7571. Quote part de subventions d'investissement (renouvelables) virée au compte de résultat"),
    N_("75. Autres produits de gestions courante : 7573. Quote-part des apports virés au compte de résultat"),
    N_("75. Autres produits de gestions courante : 7580. Produits divers de gestion courante"),
    N_("75. Autres produits de gestions courante : 7581. Dons manuels non affectés"),
    N_("75. Autres produits de gestions courante : 7582. Dons manuels affectés"),
    N_("75. Autres produits de gestions courante : 7585. Legs et donations non affectés"),
    N_("75. Autres produits de gestions courante : 7586. Legs et donations affectés"),
    N_("75. Autres produits de gestions courante : 7587. Ventes de dons en nature"),
    N_("75. Autres produits de gestions courante : 7588. Autres produits de la générosité public"),
    N_("76. Produits financiers : 7610. Produits des participations"),
    N_("76. Produits financiers : 7620. Produits des autres immobilisations financières"),
    N_("76. Produits financiers : 7621. Revenus des titres immobilisés"),
    N_("76. Produits financiers : 7624. Revenus des prêts"),
    N_("76. Produits financiers : 7640. Revenues des valeurs mobilières de placements"),
    N_("76. Produits financiers : 7650. Escomptes obtenus"),
    N_("76. Produits financiers : 7660. Gains de change"),
    N_("76. Produits financiers : 7670. Produits nets sur cession de valeurs mobilières de placement"),
    N_("76. Produits financiers : 7680. Autres produits financiers"),
    N_("76. Produits financiers : 7681. Intérêts des comptes financiers débiteurs"),
    N_("77. Produits exceptionnels : 7710. Produits exceptionnels sur opération de gestion"),
    N_("77. Produits exceptionnels : 7713. Libérabilités perçues"),
    N_("77. Produits exceptionnels : 7714. Rentrées sur créances amorties"),
    N_("77. Produits exceptionnels : 7715. Subvention d'équilibre"),
    N_("77. Produits exceptionnels : 7717. Dégrèvements d'impôts (autres qu'impôts sue les bénéfices)"),
    N_("77. Produits exceptionnels : 7718. Autres produits excepetionnels sur opérations de gestion"),
    N_("77. Produits exceptionnels : 7720. Produits sur exercices antérieurs (à reclaser)"),
    N_("77. Produits exceptionnels : 7750. Produits de cessions d'éléments d'actif"),
    N_("77. Produits exceptionnels : 7751. Immobilisations incorporelles"),
    N_("77. Produits exceptionnels : 7752. Immobilisations corporelles"),
    N_("77. Produits exceptionnels : 7756. Immobilisations financières"),
    N_("77. Produits exceptionnels : 7770. Quote-part des subventionsd'investissement virée au résultat de l'exercice"),
    N_("77. Produits exceptionnels : 7780. Autres produits exceptionnels"),
    N_("78. Reprise sur amortissements et provisions : 7810. Reprise sur amoratissements et provisions (à inscrire dans les produits d'exploitation)"),
    N_("78. Reprise sur amortissements et provisions : 7811. Reprise sur amortissement et provisions des immobilisations incorporelles et corporelles"),
    N_("78. Reprise sur amortissements et provisions : 7815. Reprise surprovision pour risques et charges d'exploitation"),
    N_("78. Reprise sur amortissements et provisions : 7816. Reprise sur provisions pour dépréciation des immobilisations incorporelles et corporelles"),
    N_("78. Reprise sur amortissements et provisions : 78161. Immobilisations incorporelles"),
    N_("78. Reprise sur amortissements et provisions : 78162. Immobilisation corporelles"),
    N_("78. Reprise sur amortissements et provisions : 7817. Reprise sur provisions pour dépréciation des actifs circulants"),
    N_("78. Reprise sur amortissements et provisions : 7860. Reprise sur provisions ( à inscrire dans les produits exceptionnels)"),
    N_("78. Reprise sur amortissements et provisions : 7866. Reprise sur provision pour dépréciation des éléments financiers"),
    N_("78. Reprise sur amortissements et provisions : 78662. Immobilisations financières"),
    N_("78. Reprise sur amortissements et provisions : 78665. Valeurs mobilières de placement"),
    N_("78. Reprise sur amortissements et provisions : 7870. Reprise sur provisions ( à inscrire dans les produits exceptionnels)"),
    N_("78. Reprise sur amortissements et provisions : 7876. Reprise sur provision pour dépréciation exceptionnelles"),
    N_("78. Reprise sur amortissements et provisions : 7890. Report des ressources non utilisées des exercices antérieurs"),
    N_("78. Reprise sur amortissements et provisions : 7894. Reports des ressources non utilisées sur subventions attribées"),
    N_("78. Reprise sur amortissements et provisions : 7895. Reports des ressources non utilisées sur dons manuels reçus"),
    N_("78. Reprise sur amortissements et provisions : 7897. Report des ressources non utilisées sur legs et donations affectées"),
    N_("79. Transferts de charges : 7910. Transferts de charges d'exploitation"),
    N_("79. Transferts de charges : 7960. Transferts des charges financières"),
    N_("79. Transferts de charges : 7970. Transferts de charges exceptionnelles"),
    N_("86. Répartition par nature de charges : 8600. Secours en nature"),
    N_("86. Répartition par nature de charges : 8601. Alimentaires"),
    N_("86. Répartition par nature de charges : 8602. Vestimentaires"),
    N_("86. Répartition par nature de charges : 8610. Mise à disposition gratuite de biens"),
    N_("86. Répartition par nature de charges : 8611. Locaux"),
    N_("86. Répartition par nature de charges : 8612. Matériels"),
    N_("86. Répartition par nature de charges : 8620. Prestations"),
    N_("86. Répartition par nature de charges : 8640. Personnel bénévole"),
    N_("87. Répartition par nautre des ressources : 8700. Bénévolat"),
    N_("87. Répartition par nautre des ressources : 8710. Prestation en nature"),
    N_("87. Répartition par nautre des ressources : 8750. Dons en nature"),
    NULL
};

const gchar *liberal_category_list [] = {
    N_("Petit outillage"),
    N_("Impots et taxes : Taxe sur la valeur ajoutée"),
    N_("Impots et taxes : Taxe professionnelle"),
    N_("Impots et taxes : Autres impôts"),
    N_("Achats"),
    N_("P.T.T."),
    N_("Autres frais de déplacements"),
    N_("Frais de véhicules"),
    N_("Réception, Représentation, Congrès"),
    N_("Entretien et réparations"),
    N_("Cotisations professionnelles et syndicales"),
    N_("Autres frais divers de gestion"),
    N_("Location de matériel ou de mobilier"),
    N_("Fournitures de bureau et documentation"),
    N_("Frais financiers"),
    N_("Rétrocession d'honoraires"),
    N_("Honoraires ne constituant pas des rétrocessions"),
    N_("Charges sociales du praticien"),
    NULL
};

const gchar *category_choice_list [] = {
    N_("Don't create any list, i will do it by myself or it will be imported"),
    N_("General category list (most common choice)"),
    N_("Association category list (in french for now, special categories to associations)"),
    N_("Liberal category list (in french for now, to use Grisbi for a small business)"),
    NULL
};


/**
 * create a window wich ask what kind of category we want
 * this will be replaced by a nice assistant
 *
 * \param
 *
 * \return 0 : default category choosen, 1 : association category, -1 if problem
 * */
/*gboolean gsb_category_choose_default_category ( void )
{
    GtkWidget *dialog;
    GtkWidget *page;

    // FIXME : for now, just a dialog, but will be include in a nice assistant
    dialog = gtk_dialog_new_with_buttons (_("Choose the categories"),
					    GTK_WINDOW (window),
					    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_OK,
					    NULL );

    page = gsb_category_assistant_create_choice_page (dialog);
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 page,
			 FALSE, FALSE,
			 0 );
    gtk_widget_show (page);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gsb_category_assistant_create_categories (dialog);
    gtk_widget_destroy (dialog);

    return TRUE;
}*/


/**
 * create a widget to include in an assistant
 * it gives the choice between the kind of categories list we can create
 * later, to create the categories according to the choice,
 * 	call gsb_category_assistant_create_categories
 *
 * \param assistant the assistant wich that page will be added to
 * 	the choice will be set into the assistant widget with the key "choice_value"
 *		CATEGORY_CHOICE_NONE
 *		CATEGORY_CHOICE_DEFAULT
 *		CATEGORY_CHOICE_ASSOCIATION
 *		CATEGORY_CHOICE_LIBERAL
 *
 * \return a box to include in an assistant
 * */
GtkWidget *gsb_category_assistant_create_choice_page ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *button = NULL;
    gint i;

    page = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(page), 12 );

    /* add the buttons choice */
    i=0;
    while (category_choice_list [i])
    {
	button = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON (button),
							       category_choice_list[i]);
	/* see category_choice_values */
	g_object_set_data ( G_OBJECT (button),
			    "choice_value", GINT_TO_POINTER (i));
	g_signal_connect ( G_OBJECT (button),
			   "toggled",
			   G_CALLBACK (gsb_category_assistant_change_choice),
			   assistant );
	gtk_box_pack_start ( GTK_BOX (page),
			     button,
			     FALSE, FALSE,
			     0 );
	/* set the default category */
	if (i == CATEGORY_CHOICE_DEFAULT)
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (button),
					   TRUE );
	i++;
    }
    gtk_widget_show_all (page);
    return page;
}

/**
 * get the choice in the assistant page created by gsb_category_assistant_create_choice_page
 * and create the category list
 *
 * \param assistant
 *
 * \return FALSE
 * */
gboolean gsb_category_assistant_create_categories ( GtkWidget *assistant )
{
    gint value;

    value = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT (assistant),
						 "choice_value"));
    gsb_data_category_create_default_category_list (value);
    return FALSE;
}


/**
 * callback when toggle the choice of category list
 *
 * \param button the radio button
 * \param assistant the assistant containing the chosen page
 *
 * \return FALSE
 * */
gboolean gsb_category_assistant_change_choice ( GtkWidget *button, 
						GtkWidget *assistant )
{
    g_object_set_data ( G_OBJECT (assistant),
			"choice_value", g_object_get_data ( G_OBJECT (button), "choice_value"));
    return FALSE;
}


/**
 * update the form's combofix category
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_category_update_combofix ( void )
{
    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( gsb_form_widget_get_widget (TRANSACTION_FORM_CATEGORY)),
				gsb_data_category_get_name_list ( TRUE,
								  TRUE,
								  TRUE,
								  TRUE ));
    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

